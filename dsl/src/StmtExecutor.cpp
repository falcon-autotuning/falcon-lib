#include "falcon-dsl/StmtExecutor.hpp"
#include "falcon-dsl/SourceContext.hpp"
#include <falcon-dsl/log.hpp>
#include <falcon-typing/PrimitiveTypes.hpp>
#include <utility>

namespace falcon::dsl {

StmtExecutor::StmtExecutor(typing::ParameterMap &variables,
                           std::shared_ptr<FunctionRegistry> functions,
                           std::shared_ptr<TypeRegistry> types)
    : variables_(variables), functions_(std::move(functions)),
      types_(std::move(types)), evaluator_(variables_, functions_, types_) {}

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
  auto try_exec = [&](auto *ptr, auto exec_fn) -> std::optional<ControlFlow> {
    if (ptr) {
      return execute_with_context(stmt, [&] { return exec_fn(*ptr); });
    }
    return std::nullopt;
  };

  if (auto result = try_exec(dynamic_cast<const atc::VarDeclStmt *>(&stmt),
                             [&](const auto &x) { return exec_var_decl(x); })) {
    return *result;
  }
  if (auto result = try_exec(dynamic_cast<const atc::AssignStmt *>(&stmt),
                             [&](const auto &x) { return exec_assign(x); })) {
    return *result;
  }
  if (auto result = try_exec(
          dynamic_cast<const atc::StructFieldAssignStmt *>(&stmt),
          [&](const auto &x) { return exec_struct_field_assign(x); })) {
    return *result;
  }
  if (auto result = try_exec(dynamic_cast<const atc::ExprStmt *>(&stmt),
                             [&](const auto &x) { return exec_expr(x); })) {
    return *result;
  }
  if (auto result = try_exec(dynamic_cast<const atc::IfStmt *>(&stmt),
                             [&](const auto &x) { return exec_if(x); })) {
    return *result;
  }
  if (auto result =
          try_exec(dynamic_cast<const atc::TransitionStmt *>(&stmt),
                   [&](const auto &x) { return exec_transition(x); })) {
    return *result;
  }
  if (auto result = try_exec(dynamic_cast<const atc::TerminalStmt *>(&stmt),
                             [&](const auto &x) { return exec_terminal(x); })) {
    return *result;
  }

  throw EvaluationError("Unknown statement type");
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
  typing::RuntimeValue initial_value;

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
    case atc::ParamType::Array: {
      std::string elem_type_name =
          stmt.type.element_type ? stmt.type.element_type->to_string() : "";
      initial_value =
          std::make_shared<typing::ArrayValue>(std::move(elem_type_name));
      break;
    }
    default:
      // Other types don't have default values
      throw EvaluationError("Type requires explicit initialization: " +
                            stmt.name);
    }
  }

  // Add to variable environment
  log::debug("exec_var_decl storing '" + stmt.name +
             "' = " + runtime_value_to_string(initial_value) +
             " (type: " + get_runtime_type_name(initial_value) + ")");
  variables_[stmt.name] = initial_value;

  return ControlFlow::none();
}

ControlFlow StmtExecutor::exec_assign(const atc::AssignStmt &stmt) {
  auto value = evaluator_.evaluate(*stmt.value);

  if (stmt.is_tuple_assignment()) {
    if (!std::holds_alternative<std::shared_ptr<typing::TupleValue>>(value)) {
      throw EvaluationError(
          "Tuple assignment requires function to return tuple, got " +
          get_runtime_type_name(value));
    }
    const auto &tuplePtr = std::get<std::shared_ptr<typing::TupleValue>>(value);
    if (!tuplePtr) {
      throw EvaluationError("Tuple assignment returned nullptr");
    }
    if (tuplePtr->size() != stmt.targets.size()) {
      throw EvaluationError("Tuple assignment size mismatch: expected " +
                            std::to_string(stmt.targets.size()) +
                            " values, got " + std::to_string(tuplePtr->size()));
    }
    for (size_t i = 0; i < stmt.targets.size(); ++i) {
      const auto &target = stmt.targets[i];
      if (target.is_field_target()) {
        // Assign into struct field: q.a_ = tuple[i]
        auto obj = evaluator_.evaluate(*target.object);
        if (!std::holds_alternative<std::shared_ptr<typing::StructInstance>>(
                obj)) {
          throw EvaluationError("Field assignment target is not a struct");
        }
        // Mutate in place via the variable map
        const auto *var_expr =
            dynamic_cast<const atc::VarExpr *>(target.object.get());
        if (var_expr != nullptr) {
          auto structIter = variables_.find(var_expr->name);
          if (structIter == variables_.end()) {
            throw EvaluationError("Undefined struct variable: " +
                                  var_expr->name);
          }
          auto &structPtr = std::get<std::shared_ptr<typing::StructInstance>>(
              structIter->second);
          if (!structPtr) {
            throw EvaluationError("Struct variable '" + var_expr->name +
                                  "' is nullptr");
          }
          structPtr->set_field(target.field_name, tuplePtr->values[i]);
        } else {
          throw EvaluationError("Complex struct field assignment targets not "
                                "yet supported in tuple context");
        }
      } else {
        variables_[target.variable] = tuplePtr->values[i];
      }
    }
  } else {
    // Single assignment
    const auto &target = stmt.targets[0];
    if (target.is_field_target()) {
      // Should not normally reach here — parser emits StructFieldAssignStmt
      // for standalone dot-assigns — but handle it defensively.
      const auto *var_expr =
          dynamic_cast<const atc::VarExpr *>(target.object.get());
      if (var_expr == nullptr) {
        throw EvaluationError("Complex struct field assignment not supported");
      }
      auto structIter = variables_.find(var_expr->name);
      if (structIter == variables_.end()) {
        throw EvaluationError("Undefined struct variable: " + var_expr->name);
      }
      auto &structPtr =
          std::get<std::shared_ptr<typing::StructInstance>>(structIter->second);
      if (!structPtr) {
        throw EvaluationError("Struct variable '" + var_expr->name +
                              "' is nullptr");
      }
      structPtr->set_field(target.field_name, value);
    } else {
      variables_[target.variable] = value;
    }
  }
  return ControlFlow::none();
}
ControlFlow
StmtExecutor::exec_struct_field_assign(const atc::StructFieldAssignStmt &stmt) {
  auto val = evaluator_.evaluate(*stmt.value);
  // The object must be a VarExpr referring to a StructInstance in variables_.
  const auto *var_expr = dynamic_cast<const atc::VarExpr *>(stmt.object.get());
  if (var_expr == nullptr) {
    throw EvaluationError(
        "Struct field assignment requires a variable as the object");
  }
  auto structIter = variables_.find(var_expr->name);
  if (structIter == variables_.end()) {
    throw EvaluationError("Undefined variable in struct field assignment: " +
                          var_expr->name);
  }
  if (!std::holds_alternative<std::shared_ptr<typing::StructInstance>>(
          structIter->second)) {
    throw EvaluationError("Variable '" + var_expr->name +
                          "' is not a struct instance");
  }
  auto &structPtr =
      std::get<std::shared_ptr<typing::StructInstance>>(structIter->second);
  if (!structPtr) {
    throw EvaluationError("Struct variable '" + var_expr->name +
                          "' is nullptr");
  }
  structPtr->set_field(stmt.field, std::move(val));
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
  std::vector<typing::RuntimeValue> params;
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

} // namespace falcon::dsl
