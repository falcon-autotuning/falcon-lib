#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-dsl/StmtExecutor.hpp"
#include "falcon-dsl/log.hpp"
#include "falcon-pm/PackageManager.hpp"
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
    // For each ffimport, compile the wrapper .cpp (relative to the .fal file)
    // into a shared library, then dlopen it and register every routine/struct
    // method found in the program that has an empty body.
    for (const auto &ffi : program->ff_imports) {
      if (!process_ff_import(ffi, abs_path, *program)) {
        log::error("Failed to process ffimport: " + ffi.wrapper_file);
        return false;
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
bool AutotunerEngine::process_ff_import(const atc::FFImportDecl &ffi,
                                        const std::filesystem::path &fal_dir,
                                        const atc::Program &program) {
  namespace fs = std::filesystem;

  // ── 1. Locate the wrapper source file ──────────────────────────────────
  fs::path wrapper_src = fal_dir.parent_path() / ffi.wrapper_file;
  if (!fs::exists(wrapper_src)) {
    log::error("FFI wrapper not found: " + wrapper_src.string());
    return false;
  }

  // ── 2. Determine the cache directory and output .so path ───────────────
  fs::path cache_dir = fal_dir.parent_path() / ".falcon" / "cache";
  fs::create_directories(cache_dir);

  std::string src_hash = compute_file_hash(wrapper_src);
  fs::path so_path = cache_dir / (wrapper_src.stem().string() + "_" +
                                  src_hash.substr(0, 16) + ".so");

  // ── 3. Compile the wrapper if the cached .so is stale ──────────────────
  if (!fs::exists(so_path)) {
    std::string includes;
    for (const auto &inc : ffi.imports) {
      includes += " " + inc; // Do NOT add extra -I
    }

    std::string libs;
    for (const auto &lib : ffi.build_libs) {
      libs += " " + lib;
    }

    std::string cmd = "clang++ -std=c++17 -fPIC -shared -O2"
                      " -o \"" +
                      so_path.string() + "\"" + includes + " \"" +
                      wrapper_src.string() + "\"" + libs;

    log::debug("Compiling FFI wrapper: " + wrapper_src.string());
    log::debug("FFI compile command: " + cmd);

    int ret = std::system(cmd.c_str());
    if (ret != 0) {
      log::error("Failed to compile FFI wrapper (exit " + std::to_string(ret) +
                 "): " + wrapper_src.string());
      return false;
    }
  }

  // ── 4. dlopen the compiled .so ─────────────────────────────────────────
  void *dl_handle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!dl_handle) {
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
