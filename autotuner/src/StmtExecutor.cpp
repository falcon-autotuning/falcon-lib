#include "falcon-autotuner/StmtExecutor.hpp"

namespace falcon::autotuner {

StmtExecutor::StmtExecutor(ParameterMap &variables,
                           std::shared_ptr<FunctionRegistry> functions,
                           std::shared_ptr<TypeRegistry> types)
    : variables_(variables), functions_(functions), types_(types),
      evaluator_(variables_, functions_, types_) {}

ControlFlow StmtExecutor::execute(const atc::Stmt &stmt) {
  // Dispatch based on statement type
  if (auto *var_decl = dynamic_cast<const atc::VarDeclStmt *>(&stmt)) {
    return exec_var_decl(*var_decl);
  } else if (auto *assign = dynamic_cast<const atc::AssignStmt *>(&stmt)) {
    return exec_assign(*assign);
  } else if (auto *expr_stmt = dynamic_cast<const atc::ExprStmt *>(&stmt)) {
    return exec_expr(*expr_stmt);
  } else if (auto *if_stmt = dynamic_cast<const atc::IfStmt *>(&stmt)) {
    return exec_if(*if_stmt);
  } else if (auto *trans = dynamic_cast<const atc::TransitionStmt *>(&stmt)) {
    return exec_transition(*trans);
  } else if (auto *term = dynamic_cast<const atc::TerminalStmt *>(&stmt)) {
    return exec_terminal(*term);
  } else {
    throw EvaluationError("Unknown statement type");
  }
}

ControlFlow StmtExecutor::execute_block(
    const std::vector<std::unique_ptr<atc::Stmt>> &stmts) {
  for (const auto &stmt : stmts) {
    auto flow = execute(*stmt);

    // If we hit a control flow change, stop executing and return it
    if (flow.type != ControlFlow::Type::None) {
      return flow;
    }
  }

  // All statements executed normally
  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_var_decl(const atc::VarDeclStmt &stmt) {
  RuntimeValue initial_value;

  if (stmt.initializer.has_value()) {
    // Evaluate initializer
    initial_value = evaluator_.evaluate(*stmt.initializer.value());
  } else {
    // Default initialization based on type
    switch (stmt.type.base_type) {
    case atc::ParamType::Int:
      initial_value = static_cast<int64_t>(0);
      break;
    case atc::ParamType::Float:
      initial_value = 0.0;
      break;
    case atc::ParamType::Bool:
      initial_value = false;
      break;
    case atc::ParamType::String:
      initial_value = std::string("");
      break;
    case atc::ParamType::Error:
      initial_value = nullptr; // nil
      break;
    default:
      // Other types don't have default values
      throw EvaluationError("Type requires explicit initialization: " +
                            stmt.name);
    }
  }

  // Add to variable environment
  variables_[stmt.name] = initial_value;

  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_assign(const atc::AssignStmt &stmt) {
  if (stmt.is_tuple_assignment()) {
    // Multiple assignment: a, b = expr
    // The expression must return a tuple (implemented as ParameterMap with
    // multiple values)
    auto result = evaluator_.evaluate(*stmt.value);

    // TODO: Implement tuple destructuring
    // For now, this is a placeholder
    throw EvaluationError("Tuple assignment not yet implemented");

  } else {
    // Single assignment
    auto value = evaluator_.evaluate(*stmt.value);

    for (const auto &target : stmt.targets) {
      variables_[target] = value;
    }
  }

  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_expr(const atc::ExprStmt &stmt) {
  // Execute expression for side effects (e.g., log::info(...))
  evaluator_.evaluate(*stmt.expression);

  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_if(const atc::IfStmt &stmt) {
  // Evaluate condition
  auto condition = evaluator_.evaluate(*stmt.condition);

  if (!std::holds_alternative<bool>(condition)) {
    throw EvaluationError("If condition must evaluate to boolean");
  }

  if (std::get<bool>(condition)) {
    // Execute then branch
    return execute_block(stmt.then_body);
  } else if (stmt.has_else()) {
    // Execute else branch
    return execute_block(stmt.else_body);
  }

  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_transition(const atc::TransitionStmt &stmt) {
  std::vector<RuntimeValue> params;
  if (stmt.has_parameters()) {
    for (const auto &expr : stmt.parameters) {
      params.push_back(evaluator_.evaluate(*expr));
    }
  }
  return ControlFlow::transition(stmt.target_state, std::move(params));
}

ControlFlow StmtExecutor::exec_terminal(const atc::TerminalStmt &stmt) {
  return ControlFlow::terminal();
}

} // namespace falcon::autotuner
