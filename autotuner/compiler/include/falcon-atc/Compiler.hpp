#pragma once

#include "AST.hpp"
#include "ParseError.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace falcon::atc {

class Compiler {
public:
  std::unique_ptr<Program> parse_file(const std::string &filename);

  /**
   * @brief Pre-register struct type names from imported modules.
   *
   * Call this BEFORE parse_file() when you have already loaded imported
   * .fal files and know what struct type names they export.  These names
   * are merged into the parser's struct_known_types table so that
   * type_spec accepts "ImportedType" references without raising
   * "Unknown type".
   *
   * Both bare names ("Quantity") and qualified names ("Quantity::quantity")
   * should be passed — the parser's type_spec checks both.
   *
   * The hints are cleared after each parse_file() call so they do not
   * bleed across independent parse sessions.
   */
  void set_known_struct_hints(const std::set<std::string> &hints);

  // Get last parsing errors
  [[nodiscard]] const std::vector<ParseError> &get_errors() const {
    return errors_;
  }

private:
  std::vector<ParseError> errors_;
  std::set<std::string> struct_hints_;
};

} // namespace falcon::atc
