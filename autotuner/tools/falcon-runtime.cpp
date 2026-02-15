#include "falcon-autotuner/Autotuner.hpp"
#include "falcon-autotuner/GraphAnalyzer.hpp"
#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace falcon::autotuner;
using json = nlohmann::json;

void print_usage(const char *prog) {
  std::cout << "Falcon Autotuner Runtime\n\n";
  std::cout << "Usage: " << prog
            << " <library.so> --entry <name> --snapshot <file.json>\n\n";
  std::cout << "Options:\n";
  std::cout << "  --entry <name>          Entry autotuner name (required)\n";
  std::cout << "  --snapshot <file.json>  Snapshot JSON file for initial "
               "params (required)\n";
  std::cout
      << "  --analyze               Analyze state machine before running\n";
  std::cout << "  --verbose               Verbose output\n";
  std::cout << "  --help                  Show this help\n";
}

int main(int argc, char **argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string library_path;
  std::string entry_name;
  std::string snapshot_file;
  bool analyze = false;
  bool verbose = false;

  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help") {
      print_usage(argv[0]);
      return 0;
    } else if (arg == "--entry" && i + 1 < argc) {
      entry_name = argv[++i];
    } else if (arg == "--snapshot" && i + 1 < argc) {
      snapshot_file = argv[++i];
    } else if (arg == "--analyze") {
      analyze = true;
    } else if (arg == "--verbose") {
      verbose = true;
    } else if (library_path.empty()) {
      library_path = arg;
    }
  }

  // Validate required arguments
  if (library_path.empty()) {
    std::cerr << "Error: Library path required\n";
    print_usage(argv[0]);
    return 1;
  }

  if (entry_name.empty()) {
    std::cerr << "Error: --entry <name> required\n";
    print_usage(argv[0]);
    return 1;
  }

  if (snapshot_file.empty()) {
    std::cerr << "Error: --snapshot <file.json> required\n";
    print_usage(argv[0]);
    return 1;
  }

  try {
    // Load shared library
    std::cout << "Loading library: " << library_path << "\n";
    boost::dll::shared_library lib(library_path);

    // Get registration function
    auto register_func = boost::dll::import_alias<void(AutotunerRegistry &)>(
        lib, "register_autotuners");

    // Register autotuners
    register_func(AutotunerRegistry::instance());

    // Get library info
    if (lib.has("get_library_info")) {
      auto info_func =
          boost::dll::import_alias<const char *()>(lib, "get_library_info");
      std::cout << "Library info: " << info_func() << "\n";
    }

    std::cout << "\n";

    // Get autotuner
    auto autotuner = AutotunerRegistry::instance().get_autotuner(entry_name);
    if (!autotuner) {
      std::cerr << "Error: Autotuner '" << entry_name
                << "' not found in library\n";
      return 1;
    }

    std::cout << "Entry autotuner: " << entry_name << "\n";

    // Analyze if requested
    if (analyze) {
      std::cout << "\n=== State Machine Analysis ===\n";
      auto analysis = GraphAnalyzer::analyze(*autotuner);

      std::cout << "Total states: " << analysis.total_states << "\n";
      std::cout << "Total transitions: " << analysis.total_transitions << "\n";
      std::cout << "Terminal states: " << analysis.terminal_states.size()
                << "\n";

      if (analysis.has_cycles) {
        std::cerr << "\nWARNING: Cycle detected!\n";
        std::cerr << "Path:\n";
        for (const auto &state : analysis.cycle_path) {
          std::cerr << "  " << state << "\n";
        }
        std::cerr << "\nExiting due to invalid state machine.\n";
        return 1;
      }

      if (!analysis.unreachable_states.empty()) {
        std::cout << "\nNote: Found " << analysis.unreachable_states.size()
                  << " unreachable state(s)\n";
        if (verbose) {
          for (const auto &state : analysis.unreachable_states) {
            std::cout << "  " << state.full_name() << "\n";
          }
        }
      }

      if (analysis.is_valid()) {
        std::cout << "\n✓ State machine is valid\n";
      }
    }

    // Load snapshot
    std::cout << "\nLoading snapshot: " << snapshot_file << "\n";
    std::ifstream snapshot_stream(snapshot_file);
    if (!snapshot_stream.is_open()) {
      std::cerr << "Error: Could not open snapshot file: " << snapshot_file
                << "\n";
      return 1;
    }

    json snapshot_json;
    snapshot_stream >> snapshot_json;

    ParameterMap initial_params = ParameterMap::from_json(snapshot_json);

    std::cout << "Loaded " << initial_params.size() << " parameter(s)\n";

    if (verbose) {
      std::cout << "Initial parameters:\n";
      for (const auto &key : initial_params.keys()) {
        std::cout << "  " << key << " = ";
        if (auto v = initial_params.try_get<int64_t>(key)) {
          std::cout << *v;
        } else if (auto v = initial_params.try_get<double>(key)) {
          std::cout << *v;
        } else if (auto v = initial_params.try_get<bool>(key)) {
          std::cout << (*v ? "true" : "false");
        } else if (auto v = initial_params.try_get<std::string>(key)) {
          std::cout << "\"" << *v << "\"";
        }
        std::cout << "\n";
      }
    }

    // Run autotuner
    std::cout << "\n=== Running Autotuner ===\n\n";
    auto result = autotuner->run(initial_params);

    // Display results
    std::cout << "\n=== Results ===\n";
    if (result.success) {
      std::cout << "✓ Success!\n";
      std::cout << "Final state: " << result.final_state.full_name() << "\n";
      std::cout << "Transitions: " << result.transition_count << "\n";

      if (verbose) {
        std::cout << "\nFinal parameters:\n";
        for (const auto &key : result.final_params.keys()) {
          std::cout << "  " << key << " = ";
          if (auto v = result.final_params.try_get<int64_t>(key)) {
            std::cout << *v;
          } else if (auto v = result.final_params.try_get<double>(key)) {
            std::cout << *v;
          } else if (auto v = result.final_params.try_get<bool>(key)) {
            std::cout << (*v ? "true" : "false");
          } else if (auto v = result.final_params.try_get<std::string>(key)) {
            std::cout << "\"" << *v << "\"";
          }
          std::cout << "\n";
        }
      }

      return 0;
    } else {
      std::cerr << "✗ Failed!\n";
      std::cerr << "Error: " << result.error_message << "\n";
      std::cerr << "Final state: " << result.final_state.full_name() << "\n";
      std::cerr << "Transitions: " << result.transition_count << "\n";
      return 1;
    }

  } catch (const boost::system::system_error &e) {
    std::cerr << "Library loading error: " << e.what() << "\n";
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
