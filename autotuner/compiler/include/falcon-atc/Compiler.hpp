#pragma once

#include "AST.hpp"
#include <memory>
#include <string>

namespace falcon::atc {

class Compiler {
public:
  std::unique_ptr<Program> parse_file(const std::string &filename);
};

} // namespace falcon::atc
