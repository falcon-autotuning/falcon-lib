#include "falcon-atc/Compiler.hpp"
#include "parser.tab.hpp"

extern std::unique_ptr<falcon::atc::Program> program_root;
extern FILE *yyin;

namespace falcon::atc {

std::unique_ptr<Program> Compiler::parse_file(const std::string &filename) {
  yyin = fopen(filename.c_str(), "r");
  if (!yyin) {
    throw std::runtime_error("Could not open: " + filename);
  }

  Parser parser;
  int result = parser.parse();

  fclose(yyin);

  if (result != 0) {
    throw std::runtime_error("Parse failed");
  }

  return std::move(program_root);
}

} // namespace falcon::atc
