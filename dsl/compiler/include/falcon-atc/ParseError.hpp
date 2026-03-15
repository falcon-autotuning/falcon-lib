#pragma once

#include <set>
#include <string>
#include <vector>

namespace falcon::atc {

struct ParseError {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  std::string message;

  [[nodiscard]] std::string format() const {
    return "Line " + std::to_string(first_line) + ":" +
           std::to_string(first_column) + " - " + message;
  }

  // Format with source context
  [[nodiscard]] std::string
  format_with_context(const std::vector<std::string> &source_lines) const;
};

// Global error collection (used by parser and compiler)
extern std::string current_filename;
extern std::vector<ParseError> current_errors;
extern std::vector<std::string> current_source_lines;

// Struct type names known to the parser for the current parse session.
// Populated by struct declarations within the file being parsed.
// AutotunerEngine pre-populates this with names from imported files
// before calling parse_file(), so that imported struct types pass
// the type_spec validation check.
extern std::set<std::string> struct_known_types;

// Lexer state reset function (to be called before parsing each file)
void reset_lexer_state();

} // namespace falcon::atc

// Flex/Bison globals that need to be accessible
extern int yylineno;
extern int yycolumn;
