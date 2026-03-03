#pragma once

#include "FalconDocument.hpp"
#include <falcon-atc/AST.hpp>
#include <vector>

namespace falcon::lsp {

class TypeChecker {
public:
  std::vector<Symbol> symbols;

  void analyze(const falcon::atc::Program &program);

private:
  void analyze_autotuner(const falcon::atc::AutotunerDecl &at);
  void analyze_state(const falcon::atc::StateDecl &state,
                     const std::string &autotuner_name);
  void analyze_stmt(const falcon::atc::Stmt &stmt,
                    const std::string &autotuner_name,
                    const std::string &state_name);
};

} // namespace falcon::lsp
