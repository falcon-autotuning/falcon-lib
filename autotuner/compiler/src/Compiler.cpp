#include "falcon-atc/Compiler.hpp"
#include "parser.tab.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

extern std::unique_ptr<falcon::atc::Program> program_root;
extern FILE *yyin;
extern void yyrestart(FILE *input_file);

namespace falcon::atc {

void Compiler::set_known_struct_hints(const std::set<std::string> &hints) {
  struct_hints_ = hints;
}

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
  yyrestart(yyin);
  current_filename = filename;
  current_errors.clear();

  // Inject any imported struct type names so type_spec accepts them.
  // struct_known_types is the parser's session-global set (defined in
  // parser.y's %code block); we clear it first to avoid stale state
  // from a previous parse, then insert the hints.
  struct_known_types.clear();
  for (const auto &hint : struct_hints_) {
    struct_known_types.insert(hint);
  }

  Parser parser;
  int result = parser.parse();

  fclose(yyin);

  // Clear hints after parsing so they don't bleed into future parse sessions
  // on this Compiler instance.
  struct_known_types.clear();
  struct_hints_.clear();

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
