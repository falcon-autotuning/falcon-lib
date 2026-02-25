#include "falcon-autotuner/ExprEvaluator.hpp"
#include <cmath>
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

  // TODO: Implement member access for structured types
  // For now, this is a placeholder
  throw EvaluationError("Member access not yet implemented: " + expr.member);
}

RuntimeValue ExprEvaluator::eval_method_call(const atc::MethodCallExpr &expr) {
  // Evaluate object
  auto object = evaluate(*expr.object);

  // Determine object's type
  std::string type_name = get_runtime_type_name(object);

  // Look up method for this type
  auto *method = types_->lookup_method(type_name, expr.method_name);
  if (!method) {
    throw EvaluationError("Unknown method '" + expr.method_name +
                          "' for type '" + type_name + "'");
  }

  // Evaluate arguments and build parameter map
  ParameterMap params;
  // TODO: finsih evaluating method calls
  // for (size_t i = 0; i < expr.args.size(); ++i) {
  //   std::ostringstream key;
  //   key << "arg" << i;
  //   params.at(key.str(), evaluate(*expr.args[i]),
  //             atc::ParamType::Int); // Type will be deduced
  // }
  //
  // Call method with object as 'this'
  auto result = (*method)(object, params);

  // Handle return value
  if (result.size() == 0) {
    // void return - return nil or some sentinel
    throw EvaluationError("Method returned no value");
  }

  // Get first result (for single return value)
  auto size = result.size();
  if (size != 0u) {
    throw EvaluationError("Method returned empty result");
  }

  return 0;
}

RuntimeValue ExprEvaluator::eval_index(const atc::IndexExpr &expr) {
  auto object = evaluate(*expr.object);
  auto index = evaluate(*expr.index);

  // TODO: Implement indexing for array-like types (Connections, etc.)
  throw EvaluationError("Indexing not yet implemented");
}

RuntimeValue ExprEvaluator::eval_call(const atc::CallExpr &expr) {
  // Look up function
  auto *func = functions_->lookup(expr.name);
  if (func == nullptr) {
    throw EvaluationError("Unknown function: " + expr.name);
  }

  // Get signature to understand return type
  const atc::BuiltinSignature *sig = functions_->get_signature(expr.name);
  if (!sig) {
    throw EvaluationError("No signature found for function: " + expr.name);
  }

  ParameterMap params;
  for (size_t i = 0; i < expr.arguments.size(); ++i) {
    const auto &arg = expr.arguments[i];
    if (arg.name.has_value()) {
      params[arg.name.value()] = evaluate(*arg.value);
    } else {
      params["arg" + std::to_string(i)] = evaluate(*arg.value);
    }
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
  return TupleValue(result);
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
