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

// ---------------------------------------------------------------------------
// Helper: execute one struct routine in its own sub-environment.
//
// - receiver_instance: the StructInstance the routine is called on (may be a
//   freshly-created one for constructor routines).
// - struct_decl:       the StructDecl that owns the routine.
// - routine:           the RoutineDecl to execute.
// - call_args:         already-evaluated positional arguments from the call
// site.
//
// Returns the FunctionResult (ordered output param values).
// After execution the receiver_instance is mutated in-place so that any
// field assignments done inside the routine body are visible to the caller.
// ---------------------------------------------------------------------------
FunctionResult ExprEvaluator::exec_struct_routine(
    std::shared_ptr<StructInstance> receiver_instance,
    const atc::StructDecl &struct_decl, const atc::RoutineDecl &routine,
    const std::vector<RuntimeValue> &call_args) {

  // ---- Build the sub-environment ----------------------------------------
  ParameterMap routine_env;

  // Bind and "this" to the receiver so that:
  //   this.a   (MemberExpr on VarExpr("this"))
  // both work inside the routine body.
  routine_env["this"] = receiver_instance;

  // Flatten all struct fields into the routine environment so that bare
  // field names (e.g. `a_`, `b_`) resolve directly without qualifying with.
  // This mirrors the "implicit self" semantics described in the AST
  // comment for StructDecl routines.
  for (const auto &field : struct_decl.fields) {
    auto field_it = receiver_instance->fields->find(field.name);
    if (field_it != receiver_instance->fields->end()) {
      routine_env[field.name] = field_it->second;
    } else if (field.initializer.has_value()) {
      // Field has a default initializer — evaluate it now.
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[field.name] = sub_eval.evaluate(*field.initializer.value());
    } else {
      // Default-initialise by type.
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
      routine_env[field.name] = def;
    }
  }

  // ---- Bind input parameters --------------------------------------------
  for (size_t i = 0; i < routine.input_params.size(); ++i) {
    if (i < call_args.size()) {
      routine_env[routine.input_params[i]->name] = call_args[i];
    } else if (routine.input_params[i]->default_value.has_value()) {
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[routine.input_params[i]->name] =
          sub_eval.evaluate(*routine.input_params[i]->default_value.value());
    } else {
      throw EvaluationError("Missing argument for struct routine '" +
                            routine.name +
                            "' parameter: " + routine.input_params[i]->name);
    }
  }

  // ---- Initialise output parameters -------------------------------------
  for (const auto &out : routine.output_params) {
    if (out->type.is_struct()) {
      // Create a fresh struct instance for struct-typed outputs.
      auto fresh = std::make_shared<StructInstance>(out->type.struct_name);
      const atc::StructDecl *out_decl =
          types_->lookup_struct(out->type.struct_name);
      if (out_decl) {
        ExprEvaluator sub_eval(routine_env, functions_, types_);
        for (const auto &field : out_decl->fields) {
          RuntimeValue fval;
          if (field.initializer.has_value()) {
            fval = sub_eval.evaluate(*field.initializer.value());
          } else {
            switch (field.type.base_type) {
            case atc::ParamType::Int:
              fval = int64_t(0);
              break;
            case atc::ParamType::Float:
              fval = 0.0;
              break;
            case atc::ParamType::Bool:
              fval = false;
              break;
            case atc::ParamType::String:
              fval = std::string("");
              break;
            default:
              fval = nullptr;
              break;
            }
          }
          fresh->set_field(field.name, fval);
        }
      }
      routine_env[out->name] = fresh;
    } else if (out->default_value.has_value()) {
      ExprEvaluator sub_eval(routine_env, functions_, types_);
      routine_env[out->name] = sub_eval.evaluate(*out->default_value.value());
    }
    // Primitive output params without a default are left uninitialised;
    // the routine body must assign them before they are read.
  }

  // ---- Execute the routine body -----------------------------------------
  StmtExecutor sub_exec(routine_env, functions_, types_);
  sub_exec.execute_block(routine.body);

  // ---- Write flattened field variables back into receiver_instance -------
  // Any assignment to a bare field name (a_, b_, …) inside the routine
  // body mutated the flat variable in routine_env.  We need to sync those
  // changes back to the StructInstance so the caller sees them.
  for (const auto &field : struct_decl.fields) {
    auto it = routine_env.find(field.name);
    if (it != routine_env.end()) {
      receiver_instance->set_field(field.name, it->second);
    }
  }

  // Also sync "self"/"this" back — in case the routine body did
  // `self.a_ = …` or `this.a_ = …` directly on the StructInstance pointer.
  // Those mutations already landed on receiver_instance (shared_ptr), so no
  // extra copy is needed here.

  // ---- Collect and return output values ----------------------------------
  FunctionResult outputs;
  for (const auto &out : routine.output_params) {
    auto it = routine_env.find(out->name);
    if (it == routine_env.end()) {
      throw EvaluationError("Struct routine '" + routine.name +
                            "' did not set output: " + out->name);
    }
    outputs.push_back(it->second);
  }
  return outputs;
}

RuntimeValue ExprEvaluator::eval_method_call(const atc::MethodCallExpr &expr) {
  // ------------------------------------------------------------------
  // Check first whether the object expression is a bare type name that
  // refers to a known struct — i.e. a STATIC / CONSTRUCTOR call like
  //   Quantity.New(a)
  // The parser emits this as MethodCallExpr{ object=VarExpr("Quantity"),
  //   method_name="New", args=[a] }.
  // We must intercept this BEFORE evaluating the object as a variable,
  // because "Quantity" is a type name, not a runtime variable.
  // ------------------------------------------------------------------
  if (const auto *type_var =
          dynamic_cast<const atc::VarExpr *>(expr.object.get())) {
    const atc::StructDecl *struct_decl = types_->lookup_struct(type_var->name);

    if (struct_decl != nullptr &&
        variables_.find(type_var->name) == variables_.end()) {
      // It IS a struct type name used as a static receiver.
      // Look up the routine on the struct.
      const atc::RoutineDecl *routine =
          struct_decl->find_routine(expr.method_name);
      if (!routine) {
        throw EvaluationError("Unknown struct routine '" + expr.method_name +
                              "' on type '" + type_var->name + "'");
      }

      // Build a fresh receiver instance (the constructor will populate it).
      auto receiver = std::make_shared<StructInstance>(struct_decl->name);
      // Default-initialise all fields on the fresh instance.
      for (const auto &field : struct_decl->fields) {
        RuntimeValue fval;
        if (field.initializer.has_value()) {
          ExprEvaluator tmp(variables_, functions_, types_);
          fval = tmp.evaluate(*field.initializer.value());
        } else {
          switch (field.type.base_type) {
          case atc::ParamType::Int:
            fval = int64_t(0);
            break;
          case atc::ParamType::Float:
            fval = 0.0;
            break;
          case atc::ParamType::Bool:
            fval = false;
            break;
          case atc::ParamType::String:
            fval = std::string("");
            break;
          default:
            fval = nullptr;
            break;
          }
        }
        receiver->set_field(field.name, fval);
      }

      // Evaluate call arguments in the CALLER'S environment.
      std::vector<RuntimeValue> call_args;
      call_args.reserve(expr.args.size());
      for (const auto &arg : expr.args) {
        call_args.push_back(evaluate(*arg));
      }

      FunctionResult outputs =
          exec_struct_routine(receiver, *struct_decl, *routine, call_args);

      if (outputs.empty()) {
        return nullptr;
      }
      if (outputs.size() == 1) {
        return outputs[0];
      }
      return std::make_shared<TupleValue>(outputs);
    }
  }

  // ------------------------------------------------------------------
  // Normal instance method call: q.Value(), q.ValueWithB(), etc.
  // The object must evaluate to a live StructInstance (or a falcon-core
  // built-in type).
  // ------------------------------------------------------------------
  auto object = evaluate(*expr.object);

  // User-defined struct instance method call
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(object)) {
    const auto &instancePtr = std::get<std::shared_ptr<StructInstance>>(object);
    if (!instancePtr) {
      throw EvaluationError("StructInstance is nullptr");
    }

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

    // Evaluate call arguments in the CALLER'S environment.
    std::vector<RuntimeValue> call_args;
    call_args.reserve(expr.args.size());
    for (const auto &arg : expr.args) {
      call_args.push_back(evaluate(*arg));
    }

    FunctionResult outputs =
        exec_struct_routine(instancePtr, *struct_decl, *routine, call_args);

    if (outputs.empty()) {
      return nullptr;
    }
    if (outputs.size() == 1) {
      return outputs[0];
    }
    return std::make_shared<TupleValue>(outputs);
  }

  // ------------------------------------------------------------------
  // Existing falcon-core built-in type method dispatch (unchanged).
  // ------------------------------------------------------------------
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

  ParameterMap params;
  std::vector<bool> assigned(expr.arguments.size(), false);

  // First: handle required parameters
  for (const auto &param_spec : sig->parameters) {
    if (!param_spec.required) {
      continue;
    }

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

    throw EvaluationError("Missing required argument: " + param_spec.name);
  }

  // Second: handle optional parameters
  for (const auto &param_spec : sig->parameters) {
    if (param_spec.required) {
      continue;
    }

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

    params[param_spec.name] = nullptr;
  }

  // Call function
  FunctionResult result = (*func)(params);

  if (result.empty()) {
    return nullptr;
  }
  if (result.size() == 1) {
    return result[0];
  }
  return std::make_shared<TupleValue>(result);
}

// ============================================================================
// OPERATORS
// ============================================================================

RuntimeValue ExprEvaluator::apply_binary_op(const std::string &op,
                                            const RuntimeValue &left,
                                            const RuntimeValue &right) {
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
