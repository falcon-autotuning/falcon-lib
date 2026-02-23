#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <memory>
#include <optional>
#include <string>

namespace falcon::autotuner {

struct ControlFlow {
  enum class Type { Continue, Transition, Terminal };

  Type type = Type::Continue;
  std::string target_state;
  std::optional<RuntimeValue> parameter;

  static ControlFlow continue_flow() {
    return ControlFlow{Type::Continue, "", std::nullopt};
  }

  static ControlFlow
  transition(const std::string &state,
             std::optional<RuntimeValue> param = std::nullopt) {
    return ControlFlow{Type::Transition, state, std::move(param)};
  }

  static ControlFlow terminal() {
    return ControlFlow{Type::Terminal, "", std::nullopt};
  }
};

class StmtExecutor {
public:
  /**
   * @brief Construct executor with variable environment, function registry, and
   * type registry.
   */
  StmtExecutor(ParameterMap &variables,
               std::shared_ptr<FunctionRegistry> functions,
               std::shared_ptr<TypeRegistry> types);

  ControlFlow execute(const atc::Stmt &stmt);
  ControlFlow
  execute_block(const std::vector<std::unique_ptr<atc::Stmt>> &stmts);

private:
  ControlFlow exec_var_decl(const atc::VarDeclStmt &stmt);
  ControlFlow exec_assign(const atc::AssignStmt &stmt);
  ControlFlow exec_expr(const atc::ExprStmt &stmt);
  ControlFlow exec_if(const atc::IfStmt &stmt);
  ControlFlow exec_transition(const atc::TransitionStmt &stmt);
  ControlFlow exec_terminal(const atc::TerminalStmt &stmt);

  ParameterMap &variables_;
  std::shared_ptr<FunctionRegistry> functions_;
  std::shared_ptr<TypeRegistry> types_; // Added TypeRegistry
  ExprEvaluator evaluator_;
};

} // namespace falcon::autotuner
