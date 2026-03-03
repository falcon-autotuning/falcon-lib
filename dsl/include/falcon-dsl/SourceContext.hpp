#pragma once

#include <string>
#include <vector>

namespace falcon::dsl {

struct SourceLocation {
  std::string filename;
  int line;
  int column;

  SourceLocation(std::string file, int ln, int col)
      : filename(std::move(file)), line(ln), column(col) {}
};

class SourceContext {
public:
  // Read source file lines (cached)
  static std::vector<std::string>
  read_source_lines(const std::string &filename);

  // Format error with context showing 2 lines before and after
  static std::string
  format_error_with_context(const SourceLocation &loc,
                            const std::string &error_message);
};

} // namespace falcon::dsl
