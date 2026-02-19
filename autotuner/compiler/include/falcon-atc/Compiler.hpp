#pragma once

#include "AST.hpp"
#include "ParseError.hpp"
#include <memory>
#include <string>
#include <vector>

namespace falcon::atc {

class Compiler {
public:
  std::unique_ptr<Program> parse_file(const std::string &filename);

  // Get last parsing errors
  [[nodiscard]] const std::vector<ParseError> &get_errors() const {
    return errors_;
  }

private:
  std::vector<ParseError> errors_;
};

} // namespace falcon::atc
