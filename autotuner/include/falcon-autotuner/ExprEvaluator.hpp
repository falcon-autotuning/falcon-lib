#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <memory>
#include <stdexcept>
#include <vector>

namespace falcon::autotuner {

class ExprEvaluator {
public:
  /**
   * @brief Construct evaluator with variable environment, function registry,
   * and type registry.
   */
  ExprEvaluator(ParameterMap &variables,
                std::shared_ptr<FunctionRegistry> functions,
                std::shared_ptr<TypeRegistry> types);

  RuntimeValue evaluate(const atc::Expr &expr);
  std::vector<RuntimeValue>
  evaluate_list(const std::vector<std::unique_ptr<atc::Expr>> &exprs);

private:
  RuntimeValue eval_literal(const atc::LiteralExpr &expr);
  RuntimeValue eval_nil_literal(const atc::NilLiteralExpr &expr);
  RuntimeValue eval_variable(const atc::VarExpr &expr);
  RuntimeValue eval_binary(const atc::BinaryExpr &expr);
  RuntimeValue eval_unary(const atc::UnaryExpr &expr);
  RuntimeValue eval_member(const atc::MemberExpr &expr);
  RuntimeValue eval_method_call(const atc::MethodCallExpr &expr);
  RuntimeValue eval_index(const atc::IndexExpr &expr);
  RuntimeValue eval_call(const atc::CallExpr &expr);
  RuntimeValue eval_qualified_call(const atc::QualifiedCallExpr &expr);

  RuntimeValue apply_binary_op(const std::string &op, const RuntimeValue &left,
                               const RuntimeValue &right);
  RuntimeValue apply_unary_op(const std::string &op,
                              const RuntimeValue &operand);

  ParameterMap &variables_;
  std::shared_ptr<FunctionRegistry> functions_;
  std::shared_ptr<TypeRegistry> types_;
};

class EvaluationError : public std::runtime_error {
public:
  explicit EvaluationError(const std::string &msg) : std::runtime_error(msg) {}
};

} // namespace falcon::autotuner
