#include "falcon-autotuner/StmtExecutor.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/SourceContext.hpp"

namespace falcon::autotuner {

StmtExecutor::StmtExecutor(ParameterMap &variables,
                           std::shared_ptr<FunctionRegistry> functions,
                           std::shared_ptr<TypeRegistry> types)
    : variables_(variables), functions_(functions), types_(types),
      evaluator_(variables_, functions_, types_) {}

// Template helper to wrap execution with error context
template <typename Func>
ControlFlow StmtExecutor::execute_with_context(const atc::Stmt &stmt,
                                               Func func) {
  try {
    return func();
  } catch (const EvaluationError &e) {
    // Already an EvaluationError, enhance it with source context
    if (!stmt.filename.empty() && stmt.line > 0) {
      SourceLocation loc(stmt.filename, stmt.line, stmt.column);
      std::string enhanced_error =
          SourceContext::format_error_with_context(loc, e.what());
      throw EvaluationError(enhanced_error);
    }
    throw; // Re-throw as-is if no source location
  } catch (const std::exception &e) {
    // Wrap any other exception with source context
    if (!stmt.filename.empty() && stmt.line > 0) {
      SourceLocation loc(stmt.filename, stmt.line, stmt.column);
      std::string enhanced_error =
          SourceContext::format_error_with_context(loc, e.what());
      throw EvaluationError(enhanced_error);
    }
    // Re-throw wrapped in EvaluationError
    throw EvaluationError(std::string("Unexpected error: ") + e.what());
  }
}

ControlFlow StmtExecutor::execute(const atc::Stmt &stmt) {
  // Dispatch based on statement type, wrapped with error context
  if (auto *var_decl = dynamic_cast<const atc::VarDeclStmt *>(&stmt)) {
    return execute_with_context(stmt,
                                [&]() { return exec_var_decl(*var_decl); });
  } else if (auto *assign = dynamic_cast<const atc::AssignStmt *>(&stmt)) {
    return execute_with_context(stmt, [&]() { return exec_assign(*assign); });
  } else if (auto *expr_stmt = dynamic_cast<const atc::ExprStmt *>(&stmt)) {
    return execute_with_context(stmt, [&]() { return exec_expr(*expr_stmt); });
  } else if (auto *if_stmt = dynamic_cast<const atc::IfStmt *>(&stmt)) {
    return execute_with_context(stmt, [&]() { return exec_if(*if_stmt); });
  } else if (auto *trans = dynamic_cast<const atc::TransitionStmt *>(&stmt)) {
    return execute_with_context(stmt,
                                [&]() { return exec_transition(*trans); });
  } else if (auto *term = dynamic_cast<const atc::TerminalStmt *>(&stmt)) {
    return execute_with_context(stmt, [&]() { return exec_terminal(*term); });
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
  // Evaluate RHS
  auto value = evaluator_.evaluate(*stmt.value);

  if (stmt.is_tuple_assignment()) {
    // Multi-target assignment: a, b = func()
    // Value must be a TupleValue
    if (!std::holds_alternative<TupleValue>(value)) {
      throw EvaluationError(
          "Tuple assignment requires function to return tuple, got " +
          get_runtime_type_name(value));
    }

    const auto &tuple = std::get<TupleValue>(value);

    if (tuple.size() != stmt.targets.size()) {
      throw EvaluationError("Tuple assignment size mismatch: expected " +
                            std::to_string(stmt.targets.size()) +
                            " values, got " + std::to_string(tuple.size()));
    }

    // Assign each value
    for (size_t i = 0; i < stmt.targets.size(); ++i) {
      variables_[stmt.targets[i]] = tuple[i];
    }
  } else {
    // Single assignment: x = value
    variables_[stmt.targets[0]] = value;
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
  }
  if (stmt.has_else()) {
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

ControlFlow StmtExecutor::exec_terminal(const atc::TerminalStmt & /*stmt*/) {
  return ControlFlow::terminal();
}

} // namespace falcon::autotuner
