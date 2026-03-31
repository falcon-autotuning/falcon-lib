#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-dsl/StmtExecutor.hpp"
#include "falcon-dsl/log.hpp"
#include "falcon-pm/PackageManager.hpp"
#include "falcon-pm/PackageManifest.hpp"
#include <dlfcn.h>
#include <falcon-typing/FFIHelpers.hpp>
#include <falcon-typing/falcon_ffi.h>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <sstream>

namespace falcon::dsl {
std::string
AutotunerEngine::compute_file_hash(const std::filesystem::path &path) {
  // Reuse the PackageCache sha256 if accessible, otherwise inline:
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return "00000000000000000000000000000000";
  std::ostringstream ss;
  ss << std::hex;
  // Simple djb2-style hash for cache keying (not cryptographic, just stable)
  uint64_t h = 5381;
  char c;
  while (f.get(c)) {
    h = ((h << 5) + h) + static_cast<unsigned char>(c);
  }
  ss << std::setw(16) << std::setfill('0') << h;
  return ss.str();
}

AutotunerEngine::AutotunerEngine() {
  function_registry_ = FunctionRegistry::create_default();
  type_registry_ = TypeRegistry::create_default();
  interpreter_ =
      std::make_shared<Interpreter>(function_registry_, type_registry_);
}

// ---------------------------------------------------------------------------
// Helper: strip module qualifier from a symbol name.
//   "Adder::adder" → "adder"
//   "adder"        → "adder"
// ---------------------------------------------------------------------------
static std::string strip_module(const std::string &qualified) {
  auto pos = qualified.find("::");
  if (pos == std::string::npos)
    return qualified;
  return qualified.substr(pos + 2);
}

bool AutotunerEngine::load_fal_file(const std::string &fal_file_path) {
  try {
    if (!std::filesystem::exists(fal_file_path)) {
      log::error("File not found: " + fal_file_path);
      return false;
    }
    auto abs_path = std::filesystem::weakly_canonical(fal_file_path);

    // ── Package manager: resolve imports ──────────────────────────────────
    falcon::pm::PackageManager pm(abs_path);

    // Pre-scan for imports before parsing, so type names are available.
    std::vector<std::string> raw_imports = extract_imports_from_file(abs_path);

    if (!raw_imports.empty()) {
      // TODO: need to break each import into its own bucket in the cache
      auto resolved_imports = pm.resolve_imports(abs_path, raw_imports);

      for (const auto &imp : resolved_imports) {
        // Recursively load imported files, avoiding infinite recursion.
        if (!loaded_paths_.contains(imp.absolute_path)) {
          if (!load_fal_file(imp.absolute_path.string())) {
            log::error("Failed to load import: " + imp.absolute_path.string());
            return false;
          }
        }

        // Register imported struct names for type validation.
        auto loaded_prog_it = loaded_programs_by_path_.find(imp.absolute_path);
        if (loaded_prog_it != loaded_programs_by_path_.end()) {
          const auto *prog = loaded_prog_it->second;
          for (const auto &struct_decl : prog->structs) {
            import_struct_hints_.insert(struct_decl.name);
            import_struct_hints_.insert(imp.module_name +
                                        "::" + struct_decl.name);
          }
        }

        // Store the package root for FFI wrapper path resolution.
        package_roots_by_file_[imp.absolute_path] = imp.package_root;
      }
    }

    // ── Parse the main file ───────────────────────────────────────────────
    atc::Compiler compiler;
    compiler.set_known_struct_hints(import_struct_hints_);

    auto program = compiler.parse_file(abs_path.string());
    if (!program) {
      log::error("Failed to parse: " + fal_file_path);
      return false;
    }

    log::info("Loaded .fal file: " + fal_file_path);
    log::info(fmt::format("Found {} autotuner(s), {} routine(s)",
                          program->autotuners.size(),
                          program->routines.size()));

    // ── Register structs ──────────────────────────────────────────────────
    for (auto &struct_decl : program->structs) {
      struct_decl.module_name = program->module_name;
      type_registry_->register_struct(&struct_decl);
    }

    // ── Process ffimport declarations ─────────────────────────────────────
    // For each ffimport, compile the wrapper.cpp (relative to the .fal file)
    // into a shared library, then dlopen it and register every routine/struct
    // method found in the program that has an empty body only if not already
    // packaged. If already packaged, use the package manager
    std::filesystem::path fal_file = fal_file_path;
    std::optional<std::filesystem::path> manifest_path =
        pm.find_package_manifest(fal_file.parent_path());
    pm::PackageManifest manifest;
    if (manifest_path.has_value() &&
        std::filesystem::exists(manifest_path.value())) {
      manifest = pm::PackageManifest::load(manifest_path.value());
    }
    if (manifest_path.has_value() && (std::size(manifest.ffi) != 0U)) {
      for (const auto &ffi : manifest.ffi) {
        // process_ff_import without 1 - 3 and pass in the.so from the bucket
        std::filesystem::path object = fal_file.parent_path() / (ffi.first);
        if (!process_ff_import(ffi.first, abs_path, *program, object)) {
          log::error("Failed to process loaded object: " + fal_file_path);
          return false;
        }
      }
    } else {
      for (const auto &ffi : program->ff_imports) {
        if (!process_ff_import(ffi, abs_path, *program)) {
          log::error("Failed to process ffimport: " + ffi.wrapper_file);
          return false;
        }
      }
    }

    // ── Register autotuners ───────────────────────────────────────────────
    for (auto &autotuner : program->autotuners) {
      autotuner.module_name = program->module_name;
      log::info("  - Autotuner: " + autotuner.name);
      std::string qname = autotuner.module_name.empty()
                              ? autotuner.name
                              : autotuner.module_name + "::" + autotuner.name;
      loaded_autotuners_.erase(qname);
      loaded_autotuners_.insert({qname, std::move(autotuner)});
      auto it = loaded_autotuners_.find(qname);
      if (it != loaded_autotuners_.end()) {
        register_autotuner_as_function(it->second);
      }
    }

    // ── Register inline routines ──────────────────────────────────────────
    for (auto &routine : program->routines) {
      routine.module_name = program->module_name;
      log::info("  - Routine: " + routine.name);
      if (!routine.body.empty()) {
        register_inline_routine(routine);
      }
      std::string qname = routine.module_name.empty()
                              ? routine.name
                              : routine.module_name + "::" + routine.name;
      routine_declarations_.erase(qname);
      routine_declarations_.insert({qname, std::move(routine)});
    }

    // ── Keep program alive; track by path ─────────────────────────────────
    loaded_paths_.insert(abs_path);
    loaded_programs_by_path_[abs_path] = program.get();
    loaded_programs_.push_back(std::move(program));

    return true;
  } catch (const std::exception &e) {
    log::error(std::string("Error loading file: ") + e.what());
    return false;
  }
}

// ---------------------------------------------------------------------------
// Lightweight import extractor — reads the top of a .fal file and collects
// all strings inside import(...) without running the full parser.
// ---------------------------------------------------------------------------
std::vector<std::string>
AutotunerEngine::extract_imports_from_file(const std::filesystem::path &path) {
  std::vector<std::string> imports;
  std::ifstream f(path);
  if (!f.is_open())
    return imports;

  std::string line;
  bool in_import_block = false;
  while (std::getline(f, line)) {
    // Trim leading whitespace
    auto start = line.find_first_not_of(" \t\r");
    if (start == std::string::npos)
      continue;
    std::string trimmed = line.substr(start);

    if (trimmed.rfind("import", 0) == 0) {
      // Single-line: import "path";
      auto q1 = trimmed.find('"');
      auto q2 = trimmed.rfind('"');
      if (q1 != std::string::npos && q2 != q1)
        imports.push_back(trimmed.substr(q1 + 1, q2 - q1 - 1));
      if (trimmed.find('(') != std::string::npos)
        in_import_block = true;
      continue;
    }
    if (in_import_block) {
      if (trimmed.find(')') != std::string::npos) {
        in_import_block = false;
        continue;
      }
      auto q1 = trimmed.find('"');
      auto q2 = trimmed.rfind('"');
      if (q1 != std::string::npos && q2 != q1)
        imports.push_back(trimmed.substr(q1 + 1, q2 - q1 - 1));
      continue;
    }
    // Stop once we see a non-import keyword at the start
    if (!trimmed.empty() && trimmed[0] != '/' && trimmed[0] != '#')
      break;
  }
  return imports;
}

bool AutotunerEngine::load_fal_compiled(const std::string &compiled_path) {
  try {
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
      std::ostringstream oss;
      oss << "Failed to open compiled file: " << compiled_path;
      log::error(oss.str());
      return false;
    }
    log::warn("Compiled .fal loading not yet implemented, use load_fal_file()");
    return false;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error loading compiled file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::save_fal_compiled(const std::string &output_path) {
  try {
    std::ofstream file(output_path, std::ios::binary);
    if (!file.is_open()) {
      std::ostringstream oss;
      oss << "Failed to create compiled file: " << output_path;
      log::error(oss.str());
      return false;
    }
    log::warn("Compiled .fal saving not yet implemented");
    return false;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error saving compiled file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::load_routine_library(const RoutineConfig &info) {
  auto decl_it = routine_declarations_.find(info.name);
  if (decl_it == routine_declarations_.end()) {
    std::ostringstream oss;
    oss << "Routine '" << info.name << "' not declared in any loaded .fal file";
    log::error(oss.str());
    std::ostringstream list_oss;
    list_oss << "Available routines: ";
    for (const auto &r : routine_declarations_) {
      list_oss << r.first << " ";
    }
    log::info(list_oss.str());
    return false;
  }

  void *handle = dlopen(info.library_path.c_str(), RTLD_LAZY);
  if (handle == nullptr) {
    std::ostringstream oss;
    oss << "Failed to load library: " << info.library_path << " (" << dlerror()
        << ")";
    log::error(oss.str());
    return false;
  }

  std::string symbol =
      info.name_space.empty() ? info.name : info.name_space + "::" + info.name;
  dlerror();
  void *sym = dlsym(handle, symbol.c_str());
  const char *dlsym_error = dlerror();
  if (dlsym_error != nullptr) {
    std::ostringstream oss;
    oss << "Symbol '" << symbol << "' not found in " << info.library_path
        << ": " << dlsym_error;
    log::error(oss.str());
    return false;
  }

  // Cast to the C-ABI FFI function type (no C++ types cross the boundary)
  auto ffi_ptr = reinterpret_cast<FalconFFIFunc>(sym);

  // Wrap the C-ABI call in an ExternalFunction adapter that handles
  // ParameterMap ↔ FalconParamEntry[] and FunctionResult ↔ FalconResultSlot[]
  ExternalFunction ext_func =
      [ffi_ptr](typing::ParameterMap &params) -> typing::FunctionResult {
    // Pack inputs
    auto packed = typing::ffi::engine::pack_params(params);
    // Pre-allocate output slots (generous upper bound)
    constexpr int32_t MAX_OUTPUTS = 16;
    FalconResultSlot out_slots[MAX_OUTPUTS] = {};
    int32_t out_count = 0;

    // Call through C ABI — no C++ types cross here
    ffi_ptr(packed.entries.data(), (int32_t)packed.entries.size(), out_slots,
            &out_count);

    // Unpack outputs
    return typing::ffi::engine::unpack_results(out_slots, out_count);
  };

  const auto &decl = decl_it->second;
  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(decl.input_params.size());
  for (const auto &p : decl.input_params) {
    params.emplace_back(p->name, p->type, true);
  }
  returns.reserve(decl.output_params.size());
  for (const auto &p : decl.output_params) {
    returns.emplace_back(p->name, p->type, true);
  }
  atc::BuiltinSignature sig(decl.name, std::move(params), std::move(returns));

  RoutineInfo routine_info{info.name, info.library_path, ext_func, sig};
  function_registry_->register_routine(routine_info);

  log::debug(fmt::format("Registered FFI routine: {}", info.name));
  return true;
}

typing::FunctionResult
AutotunerEngine::run_autotuner(const std::string &autotuner_name,
                               typing::ParameterMap &inputs) {
  auto it = loaded_autotuners_.find(autotuner_name);
  if (it == loaded_autotuners_.end()) {
    // Try bare name fallback
    for (auto iter = loaded_autotuners_.begin();
         iter != loaded_autotuners_.end(); ++iter) {
      if (strip_module(iter->first) == autotuner_name) {
        it = iter;
        break;
      }
    }
  }

  if (it == loaded_autotuners_.end()) {
    throw std::runtime_error("Autotuner not loaded: " + autotuner_name);
  }

  const auto &autotuner = it->second;
  std::ostringstream oss;
  oss << "Running autotuner: " << autotuner_name;
  log::info(oss.str());

  return interpreter_->run(autotuner, inputs);
}

typing::FunctionResult
AutotunerEngine::run_routine(const std::string &routine_name,
                             typing::ParameterMap &inputs) {
  auto *func = function_registry_->lookup(routine_name);
  if (!func) {
    throw std::runtime_error("Routine not found: " + routine_name);
  }
  return (*func)(inputs);
}

std::vector<std::string> AutotunerEngine::get_loaded_autotuners() const {
  std::vector<std::string> names;
  names.reserve(loaded_autotuners_.size());
  for (const auto &pair : loaded_autotuners_) {
    names.push_back(pair.first);
  }
  return names;
}

std::vector<std::string> AutotunerEngine::get_loaded_routines() const {
  std::vector<std::string> names;
  for (const auto &pair : routine_declarations_) {
    if (function_registry_->has_function(pair.first)) {
      names.push_back(pair.first);
    }
  }
  return names;
}

std::vector<std::string> AutotunerEngine::get_declared_routines() const {
  std::vector<std::string> names;
  names.reserve(routine_declarations_.size());
  for (const auto &pair : routine_declarations_) {
    names.push_back(pair.first);
  }
  return names;
}

bool AutotunerEngine::has_autotuner(const std::string &name) const {
  if (loaded_autotuners_.count(name) > 0)
    return true;
  for (const auto &pair : loaded_autotuners_) {
    if (strip_module(pair.first) == name)
      return true;
  }
  return false;
}

const atc::AutotunerDecl *
AutotunerEngine::get_autotuner(const std::string &name) const {
  auto iter = loaded_autotuners_.find(name);
  if (iter != loaded_autotuners_.end())
    return &iter->second;

  for (const auto &pair : loaded_autotuners_) {
    if (strip_module(pair.first) == name)
      return &pair.second;
  }
  return nullptr;
}

void AutotunerEngine::register_autotuner_as_function(
    const atc::AutotunerDecl &autotuner) {
  std::string qname = autotuner.module_name.empty()
                          ? autotuner.name
                          : autotuner.module_name + "::" + autotuner.name;

  auto func = [this,
               qname](typing::ParameterMap &inputs) -> typing::FunctionResult {
    auto iter = loaded_autotuners_.find(qname);
    if (iter == loaded_autotuners_.end()) {
      throw std::runtime_error("Autotuner not found for execution: " + qname);
    }
    return interpreter_->run(iter->second, inputs);
  };

  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(autotuner.input_params.size());
  for (const auto &param : autotuner.input_params) {
    params.emplace_back(param->name, param->type, true);
  }
  returns.reserve(autotuner.output_params.size());
  for (const auto &param : autotuner.output_params) {
    returns.emplace_back(param->name, param->type, true);
  }

  // Register bare name
  atc::BuiltinSignature bare_sig(autotuner.name, params, returns);
  function_registry_->register_autotuner(bare_sig, func);

  // Register qualified name if module is known
  if (!autotuner.module_name.empty()) {
    std::string qname = autotuner.module_name + "::" + autotuner.name;
    atc::BuiltinSignature q_sig(qname, std::move(params), std::move(returns));
    function_registry_->register_autotuner(q_sig, func);
    log::debug(fmt::format("Loaded autotuner: {}", qname));
  } else {
    log::debug(fmt::format("Loaded autotuner: {}", autotuner.name));
  }
}

void AutotunerEngine::register_inline_routine(const atc::RoutineDecl &routine) {
  // Capture everything the lambda needs by value/shared_ptr so it remains
  // valid even after the RoutineDecl is moved into routine_declarations_.
  // We capture the routine name and rely on routine_declarations_ (via the
  // engine pointer) to find the body at call time — this is safe because
  // routine_declarations_ entries are never erased after insertion.
  std::string qname = routine.module_name.empty()
                          ? routine.name
                          : routine.module_name + "::" + routine.name;

  auto func = [this, routine_name = qname](
                  typing::ParameterMap &inputs) -> typing::FunctionResult {
    // Look up the stored RoutineDecl (guaranteed to exist after load_fal_file).
    auto decl_it = routine_declarations_.find(routine_name);
    if (decl_it == routine_declarations_.end()) {
      throw std::runtime_error("Inline routine not found in declarations: " +
                               routine_name);
    }
    const atc::RoutineDecl &decl = decl_it->second;

    // Build the execution environment: bind all input params by name.
    typing::ParameterMap env;
    for (const auto &param : decl.input_params) {
      auto it = inputs.find(param->name);
      if (it != inputs.end()) {
        env[param->name] = it->second;
      } else if (param->default_value.has_value()) {
        // Default-value evaluation needs an evaluator — bootstrap with empty
        // env; default values must be literals or already-bound names.
        // For now leave unset; StmtExecutor will error if the body reads it.
      }
      // Primitive output params are default-initialized below.
    }

    // Default-initialize output params so the body can assign into them.
    for (const auto &out : decl.output_params) {
      switch (out->type.base_type) {
      case atc::ParamType::Int:
        env[out->name] = int64_t(0);
        break;
      case atc::ParamType::Float:
        env[out->name] = 0.0;
        break;
      case atc::ParamType::Bool:
        env[out->name] = false;
        break;
      case atc::ParamType::String:
        env[out->name] = std::string("");
        break;
      default:
        env[out->name] = nullptr;
        break;
      }
    }

    // Execute the routine body.
    StmtExecutor executor(env, function_registry_, type_registry_);
    executor.execute_block(decl.body);

    // Collect outputs in declaration order.
    typing::FunctionResult result;
    result.reserve(decl.output_params.size());
    for (const auto &out : decl.output_params) {
      auto it = env.find(out->name);
      if (it == env.end()) {
        throw std::runtime_error("Inline routine '" + routine_name +
                                 "' did not set output: " + out->name);
      }
      result.push_back(it->second);
    }
    return result;
  };

  // Build the BuiltinSignature so the FunctionRegistry and call-site
  // argument-matching logic can work with it.
  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(routine.input_params.size());
  for (const auto &p : routine.input_params) {
    params.emplace_back(p->name, p->type, true);
  }
  returns.reserve(routine.output_params.size());
  for (const auto &p : routine.output_params) {
    returns.emplace_back(p->name, p->type, true);
  }
  // Register bare name
  atc::BuiltinSignature bare_sig(routine.name, params, returns);
  RoutineInfo bare_info{.name = routine.name,
                        .library_path = "<inline>",
                        .function = ExternalFunction(func),
                        .signature = bare_sig};
  function_registry_->register_routine(bare_info);

  // Register qualified name if module is known
  if (!routine.module_name.empty()) {
    std::string qname = routine.module_name + "::" + routine.name;
    atc::BuiltinSignature q_sig(qname, std::move(params), std::move(returns));
    RoutineInfo q_info{.name = qname,
                       .library_path = "<inline>",
                       .function = ExternalFunction(func),
                       .signature = q_sig};
    function_registry_->register_routine(q_info);
    log::debug(fmt::format("Registered inline routine: {}", qname));
  } else {
    log::debug(fmt::format("Registered inline routine: {}", routine.name));
  }
}
// ---------------------------------------------------------------------------
// process_ff_import: compile wrapper .cpp → .so, dlopen, register symbols.
//
// Naming convention expected in wrapper files:
//   Plain routines:      <RoutineName>(ParameterMap)
//   Struct constructors: STRUCT<StructName><RoutineName>(ParameterMap)
//   Struct methods:      STRUCT<StructName><RoutineName>(ParameterMap)
//                        where params["this"] = shared_ptr<NativeType>
//
// All wrapper functions MUST be marked extern "C".
// ---------------------------------------------------------------------------
bool AutotunerEngine::process_ff_import(
    const atc::FFImportDecl &ffi, const std::filesystem::path &fal_dir,
    const atc::Program &program,
    const std::optional<std::filesystem::path> &so_path_opt) {
  namespace fs = std::filesystem;

  fs::path so_path;
  fs::path cpp_path;

  if (so_path_opt.has_value()) {
    so_path = *so_path_opt;
  } else {
    std::string wrapper_name = ffi.wrapper_file;
    cpp_path = fal_dir.parent_path() / wrapper_name;
    if (wrapper_name.ends_with(".cpp") || wrapper_name.ends_with(".c")) {
      wrapper_name =
          wrapper_name.substr(0, wrapper_name.find_last_of('.')) + ".so";
    }
    so_path = fal_dir.parent_path() / wrapper_name;
  }

  // Construct the flags defined by the developer in the ffimport statement
  std::string extra_flags;
  for (const auto &inc : ffi.imports) {
    extra_flags += " " + inc;
  }
  for (const auto &lib : ffi.build_libs) {
    extra_flags += " " + lib;
  }

  // Smart rebuild: missing, or cpp is newer than so
  bool needs_build = !fs::exists(so_path);
  if (!needs_build && !cpp_path.empty() && fs::exists(cpp_path)) {
    if (fs::last_write_time(cpp_path) > fs::last_write_time(so_path)) {
      needs_build = true;
    }
  }

  if (needs_build) {
    log::info("FFI library missing or out of date. Auto-building...");
    try {
      falcon::pm::PackageManager pm(fal_dir.parent_path());
      auto manifest_opt = pm.find_package_manifest(pm.project_root());
      std::string so_name = cpp_path.stem().string() + ".so";
      auto manifest_path = fal_dir.parent_path() / "falcon.yml";

      if (!manifest_opt) {
        // No manifest: create one
        std::string package_name = cpp_path.stem().string();
        falcon::pm::PackageManager::init(fal_dir.parent_path(), package_name);
        log::info("Created falcon.yml for package: " + package_name);
      }

      // Always ensure the .so is listed in ffi
      auto manifest = pm::PackageManifest::load(manifest_path);
      if (manifest.ffi.find(so_name) == manifest.ffi.end()) {
        manifest.ffi[so_name] = ""; // or initial hash if you want
        manifest.save(manifest_path);
        log::info("Added missing FFI entry to falcon.yml: " + so_name);
      }

      pm.build(pm.project_root(), extra_flags); // Now always safe to build
    } catch (const std::exception &e) {
      log::error(std::string("Auto-build failed: ") + e.what());
      return false;
    }
  }

  if (!fs::exists(so_path)) {
    log::error("FFI library not found after build attempt: " +
               so_path.string());
    return false;
  }

  void *dl_handle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (dl_handle == nullptr) {
    log::error("Failed to dlopen FFI wrapper: " + std::string(dlerror()));
    return false;
  }
  ffi_handles_.push_back(dl_handle);

  // ── 5. Register every struct routine whose body is empty ───────────────
  // Convention: symbol = "STRUCT<TypeName><RoutineName>"
  // All wrapper functions now use the new C-ABI: FalconFFIFunc
  for (const auto &s : program.structs) {
    for (const auto &routine : s.routines) {
      if (!routine.body.empty()) {
        continue;
      }

      std::string sym_name = "STRUCT" + s.name + routine.name;
      dlerror();
      void *sym = dlsym(dl_handle, sym_name.c_str());
      if (!sym) {
        continue;
      }

      auto ffi_ptr = reinterpret_cast<FalconFFIFunc>(sym);

      // Build a TypeMethod that goes through the full C-ABI translation.
      // The params ParameterMap already contains "this" and all input args
      // (assembled by exec_struct_routine in ExprEvaluator.cpp).
      typing::TypeMethod method =
          [ffi_ptr](
              const typing::RuntimeValue & /*receiver*/,
              const typing::ParameterMap &params) -> typing::FunctionResult {
        auto packed = typing::ffi::engine::pack_params(params);
        constexpr int32_t MAX_OUTPUTS = 16;
        FalconResultSlot out_slots[MAX_OUTPUTS] = {};
        int32_t out_count = 0;
        ffi_ptr(packed.entries.data(), (int32_t)packed.entries.size(),
                out_slots, &out_count);
        return typing::ffi::engine::unpack_results(out_slots, out_count);
      };

      type_registry_->register_ffi_method(s.name, routine.name,
                                          std::move(method));
      log::debug("Registered FFI method: " + s.name + "." + routine.name +
                 " → " + sym_name);
    }
  }

  // ── 6. Register top-level routines (non-struct FFI functions) ──────────
  for (const auto &routine : program.routines) {
    if (!routine.body.empty()) {
      continue;
    }

    std::string sym_name = routine.name;
    dlerror();
    void *sym = dlsym(dl_handle, sym_name.c_str());
    if (!sym) {
      log::warn("FFI routine symbol not found: " + sym_name);
      continue;
    }

    auto ffi_ptr = reinterpret_cast<FalconFFIFunc>(sym);

    ExternalFunction ext_func =
        [ffi_ptr](typing::ParameterMap &params) -> typing::FunctionResult {
      auto packed = typing::ffi::engine::pack_params(params);
      constexpr int32_t MAX_OUTPUTS = 16;
      FalconResultSlot out_slots[MAX_OUTPUTS] = {};
      int32_t out_count = 0;
      ffi_ptr(packed.entries.data(), (int32_t)packed.entries.size(), out_slots,
              &out_count);
      return typing::ffi::engine::unpack_results(out_slots, out_count);
    };

    std::vector<atc::BuiltinSignature::ParamSpec> params, returns;
    for (const auto &p : routine.input_params)
      params.emplace_back(p->name, p->type, true);
    for (const auto &p : routine.output_params)
      returns.emplace_back(p->name, p->type, false);

    atc::BuiltinSignature sig(routine.name, std::move(params),
                              std::move(returns));
    RoutineInfo info{routine.name, so_path.string(), ext_func, sig};
    function_registry_->register_routine(info);
    log::debug("Registered FFI routine: " + routine.name);
  }

  return true;
}

} // namespace falcon::dsl
