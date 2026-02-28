#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-autotuner/StmtExecutor.hpp"
#include "falcon-autotuner/log.hpp"
#include <cmath>
#include <fmt/format.h>
#include <optional>
#include <sstream>

namespace falcon::autotuner {

ExprEvaluator::ExprEvaluator(ParameterMap &variables,
                             std::shared_ptr<FunctionRegistry> functions,
                             std::shared_ptr<TypeRegistry> types)
    : variables_(variables), functions_(std::move(functions)),
      types_(std::move(types)) {}

RuntimeValue ExprEvaluator::evaluate(const atc::Expr &expr) {
  // Dispatch based on actual expression type
  if (const auto *lit = dynamic_cast<const atc::LiteralExpr *>(&expr)) {
    return eval_literal(*lit);
  }
  if (const auto *nil = dynamic_cast<const atc::NilLiteralExpr *>(&expr)) {
    return eval_nil_literal(*nil);
  }
  if (const auto *var = dynamic_cast<const atc::VarExpr *>(&expr)) {
    return eval_variable(*var);
  }
  if (const auto *bin = dynamic_cast<const atc::BinaryExpr *>(&expr)) {
    return eval_binary(*bin);
  }
  if (const auto *un = dynamic_cast<const atc::UnaryExpr *>(&expr)) {
    return eval_unary(*un);
  }
  if (const auto *mem = dynamic_cast<const atc::MemberExpr *>(&expr)) {
    return eval_member(*mem);
  }
  if (const auto *method = dynamic_cast<const atc::MethodCallExpr *>(&expr)) {
    return eval_method_call(*method);
  }
  if (const auto *idx = dynamic_cast<const atc::IndexExpr *>(&expr)) {
    return eval_index(*idx);
  }
  if (const auto *call = dynamic_cast<const atc::CallExpr *>(&expr)) {
    return eval_call(*call);
  }
  throw EvaluationError("Unknown expression type");
}

std::vector<RuntimeValue> ExprEvaluator::evaluate_list(
    const std::vector<std::unique_ptr<atc::Expr>> &exprs) {
  std::vector<RuntimeValue> results;
  results.reserve(exprs.size());
  for (const auto &expr : exprs) {
    results.push_back(evaluate(*expr));
  }
  return results;
}

RuntimeValue ExprEvaluator::eval_literal(const atc::LiteralExpr &expr) {
  // LiteralExpr already stores the value in the correct variant format
  return std::visit(
      [](auto &&val) -> RuntimeValue { return RuntimeValue(val); }, expr.value);
}

RuntimeValue
ExprEvaluator::eval_nil_literal(const atc::NilLiteralExpr & /*expr*/) {
  return nullptr; // nil is represented as nullptr_t
}

RuntimeValue ExprEvaluator::eval_variable(const atc::VarExpr &expr) {
  auto it = variables_.find(expr.name);
  if (it == variables_.end()) {
    throw EvaluationError("Undefined variable: " + expr.name);
  }
  return it->second;
}

RuntimeValue ExprEvaluator::eval_binary(const atc::BinaryExpr &expr) {
  auto left = evaluate(*expr.left);
  auto right = evaluate(*expr.right);
  return apply_binary_op(expr.op, left, right);
}

RuntimeValue ExprEvaluator::eval_unary(const atc::UnaryExpr &expr) {
  auto operand = evaluate(*expr.operand);
  return apply_unary_op(expr.op, operand);
}

RuntimeValue ExprEvaluator::eval_member(const atc::MemberExpr &expr) {
  auto object = evaluate(*expr.object);

  // User-defined struct field access: q.a_
  // User-defined struct field access: q.a_
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(object)) {
    const auto &instancePtr = std::get<std::shared_ptr<StructInstance>>(object);
    if (!instancePtr) {
      throw EvaluationError("StructInstance is nullptr");
    }
    return instancePtr->get_field(expr.member);
  }
  // TODO: falcon-core type field access (config.plunger_gates, etc.)
  throw EvaluationError("Member access not yet implemented for type '" +
                        get_runtime_type_name(object) + "': " + expr.member);
}

RuntimeValue ExprEvaluator::eval_method_call(const atc::MethodCallExpr &expr) {
  auto object = evaluate(*expr.object);

  // User-defined struct routine call: q.Value()  or  q.NewWithB(a, b)
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(object)) {
    const auto &instancePtr = std::get<std::shared_ptr<StructInstance>>(object);
    if (!instancePtr) {
      throw EvaluationError("StructInstance is nullptr");
    }
    // Look up the StructDecl in the type registry
    const atc::StructDecl *struct_decl =
        types_->lookup_struct(instancePtr->type_name);
    if (!struct_decl) {
      throw EvaluationError("No struct declaration found for type: " +
                            instancePtr->type_name);
    }
    const atc::RoutineDecl *routine =
        struct_decl->find_routine(expr.method_name);
    if (!routine) {
      throw EvaluationError("Unknown struct routine '" + expr.method_name +
                            "' on type '" + instancePtr->type_name + "'");
    }

    // Build a sub-environment for the routine.
    ParameterMap routine_env;
    routine_env["self"] = object; // copy

    // Bind input params
    for (size_t i = 0; i < routine->input_params.size(); ++i) {
      if (i < expr.args.size()) {
        routine_env[routine->input_params[i]->name] = evaluate(*expr.args[i]);
      } else if (routine->input_params[i]->default_value.has_value()) {
        ExprEvaluator sub_eval(routine_env, functions_, types_);
        routine_env[routine->input_params[i]->name] =
            sub_eval.evaluate(*routine->input_params[i]->default_value.value());
      } else {
        throw EvaluationError("Missing argument for struct routine '" +
                              expr.method_name +
                              "' parameter: " + routine->input_params[i]->name);
      }
    }

    // Initialize output params (struct-typed outputs get a fresh instance)
    for (const auto &out : routine->output_params) {
      if (out->type.is_struct()) {
        routine_env[out->name] =
            std::make_shared<StructInstance>(out->type.struct_name);
        // Populate with default field values
        const atc::StructDecl *out_decl =
            types_->lookup_struct(out->type.struct_name);
        if (out_decl) {
          ExprEvaluator sub_eval(routine_env, functions_, types_);
          for (const auto &field : out_decl->fields) {
            if (field.initializer.has_value()) {
              std::get<std::shared_ptr<StructInstance>>(routine_env[out->name])
                  ->set_field(field.name,
                              sub_eval.evaluate(*field.initializer.value()));
            } else {
              // Default-initialize primitive fields
              RuntimeValue def;
              switch (field.type.base_type) {
              case atc::ParamType::Int:
                def = int64_t(0);
                break;
              case atc::ParamType::Float:
                def = 0.0;
                break;
              case atc::ParamType::Bool:
                def = false;
                break;
              case atc::ParamType::String:
                def = std::string("");
                break;
              default:
                def = nullptr;
                break;
              }
              std::get<std::shared_ptr<StructInstance>>(routine_env[out->name])
                  ->set_field(field.name, def);
            }
          }
        }
      } else if (out->default_value.has_value()) {
        ExprEvaluator sub_eval(routine_env, functions_, types_);
        routine_env[out->name] = sub_eval.evaluate(*out->default_value.value());
      }
    }

    // Execute routine body
    StmtExecutor sub_exec(routine_env, functions_, types_);
    sub_exec.execute_block(routine->body);

    // Collect output values
    FunctionResult outputs;
    for (const auto &out : routine->output_params) {
      auto it = routine_env.find(out->name);
      if (it == routine_env.end()) {
        throw EvaluationError("Struct routine '" + expr.method_name +
                              "' did not set output: " + out->name);
      }
      outputs.push_back(it->second);
    }

    if (outputs.empty()) {
      return nullptr;
    }
    if (outputs.size() == 1) {
      return outputs[0];
    }
    return std::make_shared<TupleValue>(outputs);
  }
  // Existing falcon-core type method dispatch (unchanged)
  std::string type_name = get_runtime_type_name(object);
  auto *method = types_->lookup_method(type_name, expr.method_name);
  if (method == nullptr) {
    throw EvaluationError("Unknown method '" + expr.method_name +
                          "' for type '" + type_name + "'");
  }
  ParameterMap params;
  auto result = (*method)(object, params);
  if (result.empty()) {
    throw EvaluationError("Method returned no value");
  }
  if (result.size() == 1) {
    return result[0];
  }
  std::vector<RuntimeValue> values;
  values.reserve(result.size());
  for (const auto &kv : result) {
    values.push_back(kv.second);
  }
  return std::make_shared<TupleValue>(values);
}

RuntimeValue ExprEvaluator::eval_index(const atc::IndexExpr &expr) {
  auto object = evaluate(*expr.object);
  auto index = evaluate(*expr.index);

  // TODO: Implement indexing for array-like types (Connections, etc.)
  throw EvaluationError("Indexing not yet implemented");
}

RuntimeValue ExprEvaluator::eval_call(const atc::CallExpr &expr) {
  log::debug(fmt::format("Calling the func {}", expr.name));
  // Look up function
  auto *func = functions_->lookup(expr.name);
  if (func == nullptr) {
    throw EvaluationError("Unknown function: " + expr.name);
  }

  // Get signature to understand return type
  const atc::BuiltinSignature *sig = functions_->get_signature(expr.name);
  if (sig == nullptr) {
    throw EvaluationError("No signature found for function: " + expr.name);
  }
  log::debug("Found signature");
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->qualified_name));
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->supports_named_args));
  log::debug(fmt::format("Searching for the arguments in the signature {}",
                         sig->to_json().dump()));

  // ParameterMap params;
  // for (size_t i = 0; i < expr.arguments.size(); ++i) {
  //   const auto &arg = expr.arguments[i];
  //   if (arg.name.has_value()) {
  //     params[arg.name.value()] = evaluate(*arg.value);
  //   } else {
  //     params["arg" + std::to_string(i)] = evaluate(*arg.value);
  //   }
  // }
  // // Find matching input params, names always take precidence
  ParameterMap params;
  std::vector<bool> assigned(expr.arguments.size(), false);

  // First: handle required parameters
  for (const auto &param_spec : sig->parameters) {
    if (!param_spec.required) {
      continue; // skip non-required for now
    }

    // Try to find a matching named argument
    auto named_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                                 [&](const atc::CallArg &arg) {
                                   return arg.name.has_value() &&
                                          arg.name.value() == param_spec.name;
                                 });

    if (named_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), named_it);
      params[param_spec.name] = evaluate(*named_it->value);
      assigned[idx] = true;
      continue;
    }

    // Otherwise, use the first unassigned positional argument
    auto pos_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                               [&](const atc::CallArg &arg) {
                                 size_t idx = &arg - expr.arguments.data();
                                 return !arg.name.has_value() && !assigned[idx];
                               });

    if (pos_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), pos_it);
      params[param_spec.name] = evaluate(*pos_it->value);
      assigned[idx] = true;
      continue;
    }

    // If no argument found, error
    throw EvaluationError("Missing required argument: " + param_spec.name);
  }

  // Second: handle optional parameters
  for (const auto &param_spec : sig->parameters) {
    if (param_spec.required) {
      continue; // already handled
    }

    // Try to find a matching named argument
    auto named_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                                 [&](const atc::CallArg &arg) {
                                   return arg.name.has_value() &&
                                          arg.name.value() == param_spec.name;
                                 });

    if (named_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), named_it);
      params[param_spec.name] = evaluate(*named_it->value);
      assigned[idx] = true;
      continue;
    }

    // Otherwise, use the first unassigned positional argument
    auto pos_it = std::find_if(expr.arguments.begin(), expr.arguments.end(),
                               [&](const atc::CallArg &arg) {
                                 size_t idx = &arg - expr.arguments.data();
                                 return !arg.name.has_value() && !assigned[idx];
                               });

    if (pos_it != expr.arguments.end()) {
      size_t idx = std::distance(expr.arguments.begin(), pos_it);
      params[param_spec.name] = evaluate(*pos_it->value);
      assigned[idx] = true;
      continue;
    }

    // If no argument found, supply std::nullopt
    params[param_spec.name] = nullptr;
  }

  // Call function
  FunctionResult result = (*func)(params);

  // Handle return value based on signature
  if (result.empty()) {
    return nullptr; // void return
  }

  if (result.size() == 1) {
    // Single return value
    return result[0];
  }

  // Tuple return - wrap for later destructuring
  return std::make_shared<TupleValue>(result);
}

// ============================================================================
// OPERATORS
// ============================================================================

RuntimeValue ExprEvaluator::apply_binary_op(const std::string &op,
                                            const RuntimeValue &left,
                                            const RuntimeValue &right) {
  // Arithmetic operators
  if (op == "+") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) + std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l + r;
    }
    if (std::holds_alternative<std::string>(left) &&
        std::holds_alternative<std::string>(right)) {
      return std::get<std::string>(left) + std::get<std::string>(right);
    }
  }

  if (op == "-") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) - std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l - r;
    }
  }

  if (op == "*") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) * std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l * r;
    }
  }

  if (op == "/") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      auto r = std::get<int64_t>(right);
      if (r == 0)
        throw EvaluationError("Division by zero");
      return std::get<int64_t>(left) / r;
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      if (r == 0.0)
        throw EvaluationError("Division by zero");
      return l / r;
    }
  }

  // Comparison operators
  if (op == "==") {
    return left == right;
  }
  if (op == "!=") {
    return left != right;
  }
  if (op == "<") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) < std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l < r;
    }
  }
  if (op == ">") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) > std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l > r;
    }
  }
  if (op == "<=") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) <= std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l <= r;
    }
  }
  if (op == ">=") {
    if (std::holds_alternative<int64_t>(left) &&
        std::holds_alternative<int64_t>(right)) {
      return std::get<int64_t>(left) >= std::get<int64_t>(right);
    }
    if (std::holds_alternative<double>(left) ||
        std::holds_alternative<double>(right)) {
      double l = std::holds_alternative<double>(left) ? std::get<double>(left)
                                                      : std::get<int64_t>(left);
      double r = std::holds_alternative<double>(right)
                     ? std::get<double>(right)
                     : std::get<int64_t>(right);
      return l >= r;
    }
  }

  // Logical operators
  if (op == "&&") {
    return std::get<bool>(left) && std::get<bool>(right);
  }
  if (op == "||") {
    return std::get<bool>(left) || std::get<bool>(right);
  }

  throw EvaluationError("Unknown or incompatible binary operator: " + op);
}

RuntimeValue ExprEvaluator::apply_unary_op(const std::string &op,
                                           const RuntimeValue &operand) {
  if (op == "!") {
    return !std::get<bool>(operand);
  }
  if (op == "-") {
    if (std::holds_alternative<int64_t>(operand)) {
      return -std::get<int64_t>(operand);
    }
    if (std::holds_alternative<double>(operand)) {
      return -std::get<double>(operand);
    }
  }

  throw EvaluationError("Unknown or incompatible unary operator: " + op);
}

} // namespace falcon::autotuner
