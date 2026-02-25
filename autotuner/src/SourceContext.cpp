#include "falcon-autotuner/SourceContext.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <unordered_map>

namespace falcon::autotuner {

std::vector<std::string>
SourceContext::read_source_lines(const std::string &filename) {
  // Cache to avoid re-reading files
  static std::unordered_map<std::string, std::vector<std::string>> cache;

  auto it = cache.find(filename);
  if (it != cache.end()) {
    return it->second;
  }

  std::vector<std::string> lines;
  std::ifstream file(filename);
  if (!file.is_open()) {
    return lines; // Return empty if can't open
  }

  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }

  cache[filename] = lines;
  return lines;
}

std::string
SourceContext::format_error_with_context(const SourceLocation &loc,
                                         const std::string &error_message) {

  std::ostringstream oss;

  // Header with error message
  oss << "\n╔══════════════════════════════════════════════════════════════\n";
  oss << "║ Runtime Error in Autotuner Execution\n";
  oss << "╠══════════════════════════════════════════════════════════════\n";
  oss << "║ File: " << loc.filename << "\n";
  oss << "║ Line: " << loc.line << ", Column: " << loc.column << "\n";
  oss << "║ Error: " << error_message << "\n";
  oss << "╠══════════════════════════════════════════════════════════════\n";

  // Try to load source context
  auto source_lines = read_source_lines(loc.filename);

  if (source_lines.empty() || loc.line < 1 ||
      loc.line > static_cast<int>(source_lines.size())) {
    oss << "║ (Source context not available)\n";
    oss << "╚══════════════════════════════════════════════════════════════\n"
           "\n";
    return oss.str();
  }

  // Show 2 lines before, the error line, and 2 lines after
  int context_start = std::max(1, loc.line - 2);
  int context_end =
      std::min(static_cast<int>(source_lines.size()), loc.line + 2);

  oss << "║\n";

  for (int line_num = context_start; line_num <= context_end; ++line_num) {
    int idx = line_num - 1; // Convert to 0-indexed

    // Line number with indicator for error line
    if (line_num == loc.line) {
      oss << "║ >>> " << std::setw(4) << std::right << line_num << " | ";
    } else {
      oss << "║     " << std::setw(4) << std::right << line_num << " | ";
    }

    // The actual line content
    oss << source_lines[idx] << "\n";

    // Add caret pointer on the error line
    if (line_num == loc.line) {
      oss << "║          | ";

      // Add spaces to reach the error column
      for (int i = 1; i < loc.column; ++i) {
        if (i - 1 < static_cast<int>(source_lines[idx].length()) &&
            source_lines[idx][i - 1] == '\t') {
          oss << '\t';
        } else {
          oss << ' ';
        }
      }
      oss << "^\n";
    }
  }

  oss << "╚══════════════════════════════════════════════════════════════\n\n";

  return oss.str();
}

} // namespace falcon::autotuner
