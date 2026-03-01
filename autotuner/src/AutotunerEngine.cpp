#include "falcon-autotuner/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/StmtExecutor.hpp"
#include "falcon-autotuner/log.hpp"
#include "falcon-pm/PackageManager.hpp"
#include <dlfcn.h>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>

namespace falcon::autotuner {

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

    // ── Package manager: resolve imports ─────────────��────────────────────
    falcon::pm::PackageManager pm(abs_path);

    // We need a quick pre-scan of imports *before* we parse the main file,
    // because the parser validates type names at parse time.
    // Strategy: do a lightweight text-scan of `import "..."` lines, resolve
    // them, pre-load them, and inject their exported struct names into the
    // parser's module_known_types table via the Compiler hint API.
    //
    // We achieve this by loading each imported file first (recursively),
    // then loading the main file.  Imported programs' struct names are
    // registered in the type registry and will be available.

    // Read imports from file without full parse (look for import strings)
    std::vector<std::string> raw_imports = extract_imports_from_file(abs_path);

    if (!raw_imports.empty()) {
      auto resolved_imports = pm.resolve_imports(abs_path, raw_imports);

      for (const auto &imp : resolved_imports) {
        // Load the imported file (recursive) — this registers its structs
        // in the type registry and its routines in the function registry.
        // We track which modules we've loaded to avoid infinite recursion.
        if (loaded_paths_.find(imp.absolute_path) == loaded_paths_.end()) {
          if (!load_fal_file(imp.absolute_path.string())) {
            log::error("Failed to load import: " + imp.absolute_path.string());
            return false;
          }
        }

        // Tell the compiler that "ModuleName::symbolName"-style type
        // references are valid.  We inject the bare symbol names from the
        // imported program's structs into the compiler's known-types hint.
        auto loaded_prog_it = loaded_programs_by_path_.find(imp.absolute_path);
        if (loaded_prog_it != loaded_programs_by_path_.end()) {
          const auto *prog = loaded_prog_it->second;
          for (const auto &s : prog->structs) {
            // Register both the bare name AND the qualified name
            import_struct_hints_.insert(s.name);
            import_struct_hints_.insert(imp.module_name + "::" + s.name);
          }
        }
      }
    }

    // ── Parse the main file ───────────────────────────────────────────────
    atc::Compiler compiler;
    // Pass any imported struct hints so the parser accepts them as types
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
    for (const auto &struct_decl : program->structs) {
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
      log::info("  - Autotuner: " + autotuner.name);
      for (const auto &req : autotuner.required_autotuners) {
        log::debug("    requires: " + req);
        // Strip qualifier for dependency check — "Adder::adder" → "adder"
        auto bare = strip_module(req);
        if (!has_autotuner(bare) && !function_registry_->has_function(bare)) {
          log::warn("    Required '" + bare +
                    "' not yet loaded (must be loaded before running)");
        }
      }
      std::string name = autotuner.name;
      loaded_autotuners_.erase(name);
      loaded_autotuners_.insert({name, std::move(autotuner)});
      auto it = loaded_autotuners_.find(name);
      if (it != loaded_autotuners_.end())
        register_autotuner_as_function(it->second);
    }

    // ── Register inline routines ──────────────────────────────────────────
    for (auto &routine : program->routines) {
      log::info("  - Routine: " + routine.name);
      if (!routine.body.empty())
        register_inline_routine(routine);
      std::string name = routine.name;
      routine_declarations_.erase(name);
      routine_declarations_.insert({name, std::move(routine)});
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
  if (dlsym_error != nullptr) { /* error handling */
  }

  auto func_ptr = reinterpret_cast<FunctionResult (*)(ParameterMap &)>(sym);
  ExternalFunction ext_func = [func_ptr](ParameterMap &params) {
    return func_ptr(params);
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

  log::debug(fmt::format("Loaded routine: {} from {} (symbol: {})", info.name,
                         info.library_path, symbol));
  return true;
}

FunctionResult AutotunerEngine::run_autotuner(const std::string &autotuner_name,
                                              ParameterMap &inputs) {
  auto it = loaded_autotuners_.find(autotuner_name);
  if (it == loaded_autotuners_.end()) {
    throw std::runtime_error("Autotuner not loaded: " + autotuner_name);
  }

  const auto &autotuner = it->second;
  for (const auto &required : autotuner.required_autotuners) {
    // The `uses` clause stores qualified names like "Adder::adder".
    // Routines are registered under their bare name ("adder"), so strip
    // the module prefix before checking.
    auto bare = strip_module(required);
    if (!function_registry_->has_function(bare) &&
        !function_registry_->has_function(required)) {
      throw std::runtime_error("Required dependency '" + required +
                               "' not loaded for autotuner '" + autotuner_name +
                               "'");
    }
  }

  std::ostringstream oss;
  oss << "Running autotuner: " << autotuner_name;
  log::info(oss.str());

  return interpreter_->run(autotuner, inputs);
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
  return loaded_autotuners_.find(name) != loaded_autotuners_.end();
}

const atc::AutotunerDecl *
AutotunerEngine::get_autotuner(const std::string &name) const {
  auto iter = loaded_autotuners_.find(name);
  return iter != loaded_autotuners_.end() ? &iter->second : nullptr;
}

void AutotunerEngine::register_autotuner_as_function(
    const atc::AutotunerDecl &autotuner) {
  auto func = [this,
               name = autotuner.name](ParameterMap &inputs) -> FunctionResult {
    auto iter = loaded_autotuners_.find(name);
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
  const atc::BuiltinSignature sig(autotuner.name, std::move(params),
                                  std::move(returns));

  function_registry_->register_autotuner(sig, func);
  log::debug(fmt::format("Loaded autotuner: {}", autotuner.name));
}

void AutotunerEngine::register_inline_routine(const atc::RoutineDecl &routine) {
  // Capture everything the lambda needs by value/shared_ptr so it remains
  // valid even after the RoutineDecl is moved into routine_declarations_.
  // We capture the routine name and rely on routine_declarations_ (via the
  // engine pointer) to find the body at call time — this is safe because
  // routine_declarations_ entries are never erased after insertion.
  const std::string routine_name = routine.name;

  auto func = [this, routine_name](ParameterMap &inputs) -> FunctionResult {
    // Look up the stored RoutineDecl (guaranteed to exist after load_fal_file).
    auto decl_it = routine_declarations_.find(routine_name);
    if (decl_it == routine_declarations_.end()) {
      throw std::runtime_error("Inline routine not found in declarations: " +
                               routine_name);
    }
    const atc::RoutineDecl &decl = decl_it->second;

    // Build the execution environment: bind all input params by name.
    ParameterMap env;
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
    FunctionResult result;
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
  atc::BuiltinSignature sig(routine.name, std::move(params),
                            std::move(returns));

  // Register using the same path as external routines so has_function() and
  // the call-site lookup both find it.
  RoutineInfo routine_info{routine.name, /*library_path=*/"<inline>",
                           ExternalFunction(func), sig};
  function_registry_->register_routine(routine_info);

  log::debug(fmt::format("Registered inline routine: {}", routine.name));
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
  // ── Locate the wrapper source ─────────────────────────────────────────
  auto wrapper_src = fal_dir.parent_path() / ffi.wrapper_file;
  if (!std::filesystem::exists(wrapper_src)) {
    log::error("FFImport wrapper not found: " + wrapper_src.string());
    return false;
  }

  // ── Build cache path via SHA-256 of source ────────────────────────────
  falcon::pm::PackageManager pm(fal_dir);
  // The cache lives at <project_root>/.falcon/cache/
  auto cache_dir = pm.project_root() / ".falcon" / "cache";
  std::filesystem::create_directories(cache_dir);

  std::string src_hash = falcon::pm::PackageCache::sha256_file(wrapper_src);
  auto so_path = cache_dir / (wrapper_src.stem().string() + "_" +
                              src_hash.substr(0, 16) + ".so");

  // ── Compile if not cached ─────────────────────────────────────────────
  if (!std::filesystem::exists(so_path)) {
    log::debug("Compiling FFI wrapper: " + wrapper_src.string());

    // Build include flags
    std::string includes;
    for (const auto &inc : ffi.imports) {
      includes += " -I" + inc;
    }
    // Always include the falcon-autotuner headers
    includes += " -I/opt/falcon/include";

    // Build library flags
    std::string libs;
    for (const auto &lib : ffi.build_libs) {
      if (lib.rfind(".so") != std::string::npos ||
          lib.rfind(".a") != std::string::npos) {
        libs += " " + lib;
      }
    }

    std::string cmd = "clang++ -std=c++17 -fPIC -shared"
                      " -O2"
                      " -o " +
                      so_path.string() + includes + " " + wrapper_src.string() +
                      libs;

    log::debug("FFI compile command: " + cmd);
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
      log::error("Failed to compile FFI wrapper (exit " + std::to_string(rc) +
                 "): " + wrapper_src.string());
      return false;
    }
    log::debug("Compiled FFI wrapper → " + so_path.string());
  } else {
    log::debug("FFI wrapper cached: " + so_path.string());
  }

  // ── dlopen ────────────────────────────────────────────────────────────
  void *handle = dlopen(so_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (handle == nullptr) {
    log::error("dlopen failed for " + so_path.string() + ": " +
               std::string(dlerror()));
    return false;
  }
  // Note: intentionally not calling dlclose — the library must stay loaded
  // for the lifetime of the engine since function pointers into it remain live.
  ff_handles_.push_back(handle);

  // ── Register plain routines (empty-body routine decls in program) ─────
  for (const auto &routine : program.routines) {
    if (!routine.body.empty())
      continue; // has a body — interpreted, not native

    dlerror();
    void *sym = dlsym(handle, routine.name.c_str());
    if (dlerror() != nullptr || sym == nullptr) {
      log::warn("FFI symbol not found for routine: " + routine.name);
      continue;
    }

    // Cast to by-value signature (matches extern "C" wrapper convention).
    // ExternalFunction passes ParameterMap& so we copy into the call.
    using NativeFn = FunctionResult (*)(ParameterMap);
    auto func_ptr = reinterpret_cast<NativeFn>(sym);

    ExternalFunction ext_func =
        [func_ptr](ParameterMap &params) -> FunctionResult {
      ParameterMap copy = params; // wrappers take by value
      return func_ptr(copy);
    };

    std::vector<atc::BuiltinSignature::ParamSpec> params, returns;
    for (const auto &p : routine.input_params)
      params.emplace_back(p->name, p->type, true);
    for (const auto &p : routine.output_params)
      returns.emplace_back(p->name, p->type, true);
    atc::BuiltinSignature sig(routine.name, std::move(params),
                              std::move(returns));

    RoutineInfo info{routine.name, so_path.string(), ext_func, sig};
    function_registry_->register_routine(info);
    log::debug("Registered FFI routine: " + routine.name);
  }

  // ── Register struct methods ───────────────────────────────────────────
  for (const auto &struct_decl : program.structs) {
    for (const auto &routine : struct_decl.routines) {
      if (!routine.body.empty())
        continue; // interpreted body — skip

      // Naming convention: STRUCT<StructName><RoutineName>
      std::string sym_name = "STRUCT" + struct_decl.name + routine.name;

      dlerror();
      void *sym = dlsym(handle, sym_name.c_str());
      if (dlerror() != nullptr || sym == nullptr) {
        log::warn("FFI symbol not found: " + sym_name);
        continue;
      }

      using NativeFn = FunctionResult (*)(ParameterMap);
      auto func_ptr = reinterpret_cast<NativeFn>(sym);

      type_registry_->register_native_struct_method(
          struct_decl.name, routine.name,
          [func_ptr](ParameterMap &params) -> FunctionResult {
            ParameterMap copy = params;
            return func_ptr(copy);
          });

      log::debug("Registered FFI struct method: " + struct_decl.name + "." +
                 routine.name);
    }
  }

  return true;
}

} // namespace falcon::autotuner
