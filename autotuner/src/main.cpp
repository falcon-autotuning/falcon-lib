#include "falcon-autotuner/FalconParser.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.fal> [output_dir]\n";
    return 1;
  }

  std::string input_file = argv[1];
  std::string output_dir = (argc >= 3) ? argv[2] : "generated";

  try {
    falcon::autotuner::dsl::CompilerOptions opts;
    opts.output_dir = output_dir;
    opts.generate_documentation = true;
    opts.validate_dependencies = true;

    falcon::autotuner::dsl::FalconCompiler compiler(opts);
    compiler.compile_file(input_file);

    std::cout << "✓ Compilation successful\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
