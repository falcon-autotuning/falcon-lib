#include "falcon-atc/Compiler.hpp"
#include <cstdio>
#include <iostream>
#include <stdexcept>

// Flex/Bison globals
extern falcon::atc::Program *program_root;
extern FILE *yyin;
extern int yyparse();

namespace falcon::atc {

std::unique_ptr<Program> Compiler::parse_file(const std::string &filename) {
  yyin = fopen(filename.c_str(), "r");
  if (!yyin) {
    throw std::runtime_error("Could not open input file: " + filename);
  }

  // Parse
  if (yyparse() != 0) {
    if (yyin)
      fclose(yyin);
    throw std::runtime_error("Parsing failed for file: " + filename);
  }

  if (yyin)
    fclose(yyin);

  // Take ownership of the global program_root
  return std::unique_ptr<Program>(program_root);
}

} // namespace falcon::atc
