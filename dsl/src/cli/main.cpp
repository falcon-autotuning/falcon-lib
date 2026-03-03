// falcon-run: User-facing CLI for the Falcon AutotunerEngine.
//
// Usage:
//   falcon-run <autotuner-name> <file1.fal> [file2.fal ...] [options]
//
// Options:
//   --list          List all autotuners loaded from the given .fal files
//   --param k=v     Pass an initial parameter to the autotuner (repeatable)
//   --log-level L   Set log level: trace|debug|info|warn|error (default: info)
//   --help          Show this help message
//
// Exit codes:
//   0  success (autotuner ran and returned results)
//   1  usage or argument error
//   2  file load error
//   3  runtime error

#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-dsl/log.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static void print_usage(const char *argv0) {
  std::cout << "Usage: " << argv0
            << " <autotuner-name> <file.fal> [more.fal ...] [options]\n"
               "\n"
               "Options:\n"
               "  --list            List autotuners discovered in the loaded "
               ".fal files\n"
               "  --param k=v       Set an input parameter "
               "(int/float/bool/string inferred)\n"
               "  --log-level L     Log level: trace|debug|info|warn|error  "
               "(default: info)\n"
               "  --help            Show this help\n"
               "\n"
               "Examples:\n"
               "  # Run the MyAutotuner autotuner defined in autotuner.fal\n"
               "  falcon-run MyAutotuner autotuner.fal\n"
               "\n"
               "  # Run with initial parameters\n"
               "  falcon-run VoltageSweep sweep.fal --param min_voltage=0.0 "
               "--param max_voltage=1.0\n"
               "\n"
               "  # Discover what autotuners are in a file\n"
               "  falcon-run --list autotuners.fal\n";
}

// Attempt to parse a string as int64, double, bool, then fall back to string.
static falcon::typing::RuntimeValue parse_value(const std::string &s) {
  // bool
  if (s == "true")
    return true;
  if (s == "false")
    return false;

  // int
  try {
    std::size_t pos = 0;
    int64_t iv = std::stoll(s, &pos);
    if (pos == s.size())
      return iv;
  } catch (...) {
  }

  // float
  try {
    std::size_t pos = 0;
    double dv = std::stod(s, &pos);
    if (pos == s.size())
      return dv;
  } catch (...) {
  }

  // string
  return s;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string autotuner_name;
  std::vector<std::string> fal_files;
  std::vector<std::pair<std::string, std::string>> raw_params; // k=v strings
  std::string log_level = "info";
  bool list_mode = false;

  // ── Parse arguments ───────────────────────────────────────────────────────
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      print_usage(argv[0]);
      return 0;
    }

    if (arg == "--list") {
      list_mode = true;
      continue;
    }

    if (arg == "--log-level") {
      if (i + 1 >= argc) {
        std::cerr << "error: --log-level requires an argument\n";
        return 1;
      }
      log_level = argv[++i];
      continue;
    }

    if (arg == "--param") {
      if (i + 1 >= argc) {
        std::cerr << "error: --param requires a k=v argument\n";
        return 1;
      }
      std::string kv = argv[++i];
      auto eq = kv.find('=');
      if (eq == std::string::npos) {
        std::cerr << "error: --param value must be in k=v format, got: " << kv
                  << "\n";
        return 1;
      }
      raw_params.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
      continue;
    }

    if (arg.size() > 4 && arg.substr(arg.size() - 4) == ".fal") {
      fal_files.push_back(arg);
      continue;
    }

    // Otherwise treat as the autotuner name (first non-flag, non-fal arg)
    if (autotuner_name.empty() && arg[0] != '-') {
      autotuner_name = arg;
      continue;
    }

    std::cerr << "error: unrecognised argument: " << arg << "\n";
    print_usage(argv[0]);
    return 1;
  }

  // ── Apply log level ───────────────────────────────────────────────────────
  setenv("LOG_LEVEL", log_level.c_str(), 1);

  // ── Validate ──────────────────────────────────────────────────────────────
  if (fal_files.empty()) {
    std::cerr << "error: no .fal files specified\n";
    print_usage(argv[0]);
    return 1;
  }

  if (!list_mode && autotuner_name.empty()) {
    std::cerr << "error: no autotuner name specified\n";
    print_usage(argv[0]);
    return 1;
  }

  // ── Load files ────────────────────────────────────────────────────────────
  falcon::dsl::AutotunerEngine engine;

  for (const auto &path : fal_files) {
    if (!engine.load_fal_file(path)) {
      std::cerr << "error: failed to load: " << path << "\n";
      return 2;
    }
  }

  // ── List mode ─────────────────────────────────────────────────────────────
  if (list_mode) {
    auto names = engine.get_loaded_autotuners();
    if (names.empty()) {
      std::cout << "No autotuners found in the loaded files.\n";
    } else {
      std::cout << "Loaded autotuners:\n";
      for (const auto &n : names) {
        std::cout << "  " << n << "\n";
      }
    }
    return 0;
  }

  // ── Build input ParameterMap ──────────────────────────────────────────────
  falcon::typing::ParameterMap inputs;
  for (const auto &[k, v] : raw_params) {
    inputs[k] = parse_value(v);
  }

  // ── Run ───────────────────────────────────────────────────────────────────
  try {
    auto results = engine.run_autotuner(autotuner_name, inputs);

    std::cout << "Autotuner '" << autotuner_name << "' completed.\n";
    if (!results.empty()) {
      std::cout << "Results (" << results.size() << "):\n";
      for (std::size_t i = 0; i < results.size(); ++i) {
        std::cout << "  [" << i << "] ";
        std::visit(
            [](const auto &val) {
              using T = std::decay_t<decltype(val)>;
              if constexpr (std::is_same_v<T, bool>)
                std::cout << (val ? "true" : "false");
              else if constexpr (std::is_same_v<T, int64_t>)
                std::cout << val;
              else if constexpr (std::is_same_v<T, double>)
                std::cout << val;
              else if constexpr (std::is_same_v<T, std::string>)
                std::cout << std::quoted(val);
              else
                std::cout << "<object>";
            },
            results[i]);
        std::cout << "\n";
      }
    }

    return 0;

  } catch (const std::exception &e) {
    std::cerr << "runtime error: " << e.what() << "\n";
    return 3;
  }
}
