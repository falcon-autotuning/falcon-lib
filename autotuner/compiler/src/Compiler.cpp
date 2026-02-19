#include "falcon-atc/Compiler.hpp"
#include "parser.tab.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

extern std::unique_ptr<falcon::atc::Program> program_root;
extern FILE *yyin;

namespace falcon::atc {

std::unique_ptr<Program> Compiler::parse_file(const std::string &filename) {
  // First, read the entire file into memory for error reporting
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open: " + filename);
  }

  current_source_lines.clear();
  std::string line;
  while (std::getline(file, line)) {
    current_source_lines.push_back(line);
  }
  file.close();

  // Now open for parsing
  yyin = fopen(filename.c_str(), "r");
  if (!yyin) {
    throw std::runtime_error("Could not open: " + filename);
  }

  reset_lexer_state();
  current_filename = filename;
  current_errors.clear();

  Parser parser;
  int result = parser.parse();

  fclose(yyin);

  if (result != 0) {
    // Build detailed error message with context
    std::ostringstream error_details;
    error_details << "Parse failed in " << filename << ":\n\n";

    for (const auto &err : current_errors) {
      if (!current_source_lines.empty()) {
        error_details << err.format_with_context(current_source_lines);
      } else {
        error_details << err.format() << "\n";
        error_details << "(Source context unavailable)\n";
      }
      error_details << "\n";
    }

    // Store errors for later inspection
    errors_ = current_errors;

    throw std::runtime_error(error_details.str());
  }

  return std::move(program_root);
}

} // namespace falcon::atc
