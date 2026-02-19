#include "falcon-atc/ParseError.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

// Access to Flex variables
extern int yylineno;
extern int yycolumn;

namespace falcon::atc {

// Define the global variables
std::string current_filename;
std::vector<ParseError> current_errors;
std::vector<std::string> current_source_lines;

void reset_lexer_state() {
  yylineno = 1;
  yycolumn = 1;
}

std::string ParseError::format_with_context(
    const std::vector<std::string> &source_lines) const {
  // ... (keep existing implementation)
  std::ostringstream oss;

  // Basic error info
  oss << "Line " << first_line << ":" << first_column << " - " << message
      << "\n";

  // Check if we have valid source lines
  if (source_lines.empty()) {
    oss << "(Source file content not available)\n";
    return oss.str();
  }

  if (first_line < 1 || first_line > static_cast<int>(source_lines.size())) {
    oss << "(Error location outside source file bounds: line " << first_line
        << " but file has " << source_lines.size() << " lines)\n";
    return oss.str();
  }

  oss << "\n";

  // Show 2 lines before for context (if available)
  int context_start = std::max(1, first_line - 2);
  int context_end = first_line;

  for (int line_num = context_start; line_num <= context_end; ++line_num) {
    int idx = line_num - 1; // Convert to 0-indexed
    if (idx >= 0 && idx < static_cast<int>(source_lines.size())) {
      // Line number with padding (for up to 9999 lines)
      oss << "  ";
      if (line_num < 10)
        oss << "   ";
      else if (line_num < 100)
        oss << "  ";
      else if (line_num < 1000)
        oss << " ";
      oss << line_num << " | ";

      // The actual line content
      oss << source_lines[idx] << "\n";

      // Add caret pointer on the error line
      if (line_num == first_line) {
        // Indent to match the line number prefix
        if (first_line < 10)
          oss << "       | ";
        else if (first_line < 100)
          oss << "      | ";
        else if (first_line < 1000)
          oss << "     | ";
        else
          oss << "    | ";

        // Add spaces to reach the error column (accounting for 1-indexed
        // columns)
        int col = first_column;
        for (int i = 1; i < col; ++i) {
          // Preserve tabs if they exist in the original line
          if (i - 1 < static_cast<int>(source_lines[idx].length()) &&
              source_lines[idx][i - 1] == '\t') {
            oss << '\t';
          } else {
            oss << ' ';
          }
        }

        // Add caret(s) to highlight the error
        int error_length = 1;
        if (last_line == first_line && last_column > first_column) {
          error_length = last_column - first_column + 1;
        }

        oss << "^";
        for (int i = 1; i < error_length && i < 40; ++i) { // Limit to 40 chars
          oss << "~";
        }
        oss << "\n";
      }
    }
  }

  return oss.str();
}

} // namespace falcon::atc
