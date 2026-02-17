#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon_core/physics/config/core/Config.hpp"
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0]
              << " <input.fal> [autotuner_name] [nats_url]\n";
    return 1;
  }

  std::string input_file = argv[1];
  std::string at_name = (argc >= 3) ? argv[2] : "";
  std::string nats_url = (argc >= 4) ? argv[3] : "nats://localhost:4222";

  try {
    std::cout << "Loading: " << input_file << std::endl;

    // 1. Parse the program
    falcon::atc::Compiler compiler;
    auto program = compiler.parse_file(input_file);
    if (!program) {
      std::cerr << "Failed to parse program." << std::endl;
      return 1;
    }

    std::cout << "✓ Parsed " << program->autotuners.size() << " autotuners."
              << std::endl;

    // 2. Setup environment (Config and Params)
    // For demonstration, we'll use a default config
    falcon_core::physics::config::core::ConfigSP config;

    // 3. Initialize Interpreter
    falcon::autotuner::Interpreter interpreter(*program);

    if (!at_name.empty()) {
      std::cout << "Running autotuner: " << at_name << std::endl;
      falcon::autotuner::ParameterMap params;

      if (interpreter.run(at_name, params)) {
        std::cout << "✓ Execution successful." << std::endl;
        std::cout << "Final Parameters:\n"
                  << params.to_json().dump(2) << std::endl;
      } else {
        std::cerr << "✗ Execution failed." << std::endl;
        return 1;
      }
    } else {
      std::cout << "No autotuner specified, finished parsing." << std::endl;
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
