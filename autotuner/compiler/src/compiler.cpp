#include "falcon-atc/Compiler.hpp"
#include <iostream>

using namespace falcon::atc;

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input.fal>\n";
    return 1;
  }

  try {
    Compiler compiler;
    auto prog = compiler.parse_file(argv[1]);
    std::cout << "Successfully parsed: " << argv[1] << "\n";
    // TODO: Verify semantics (types, etc.)
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
