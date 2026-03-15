#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-dsl/FunctionRegistry.hpp"
#include "falcon-dsl/TypeRegistry.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

namespace falcon::dsl {

class ExprEvaluator {
public:
  /**
   * @brief Construct evaluator with variable environment, function registry,
   * and type registry.
   */
  ExprEvaluator(typing::ParameterMap &variables,
                std::shared_ptr<FunctionRegistry> functions,
                std::shared_ptr<TypeRegistry> types);

  typing::RuntimeValue evaluate(const atc::Expr &expr);
  std::vector<typing::RuntimeValue>
  evaluate_list(const std::vector<std::unique_ptr<atc::Expr>> &exprs);

private:
  static typing::RuntimeValue eval_literal(const atc::LiteralExpr &expr);
  static typing::RuntimeValue eval_nil_literal(const atc::NilLiteralExpr &expr);
  typing::RuntimeValue eval_variable(const atc::VarExpr &expr);
  typing::RuntimeValue eval_binary(const atc::BinaryExpr &expr);
  typing::RuntimeValue eval_unary(const atc::UnaryExpr &expr);
  typing::RuntimeValue eval_member(const atc::MemberExpr &expr);
  typing::RuntimeValue eval_method_call(const atc::MethodCallExpr &expr);
  typing::RuntimeValue eval_index(const atc::IndexExpr &expr);
  typing::RuntimeValue eval_call(const atc::CallExpr &expr);

  typing::RuntimeValue apply_binary_op(const std::string &op,
                                       const typing::RuntimeValue &left,
                                       const typing::RuntimeValue &right);
  typing::RuntimeValue apply_unary_op(const std::string &op,
                                      const typing::RuntimeValue &operand);

  /**
   * @brief Execute a struct routine (constructor or instance method) in its
   *        own sub-environment and return the ordered output values.
   *
   * Handles:
   *  - Binding "self" and "this" to receiver_instance
   *  - Flattening struct fields into the routine env (implicit self)
   *  - Writing flat field mutations back to receiver_instance after execution
   */
  typing::FunctionResult
  exec_struct_routine(std::shared_ptr<typing::StructInstance> receiver_instance,
                      const atc::StructDecl &struct_decl,
                      const atc::RoutineDecl &routine,
                      const std::vector<typing::RuntimeValue> &call_args);

  typing::ParameterMap &variables_;
  std::shared_ptr<FunctionRegistry> functions_;
  std::shared_ptr<TypeRegistry> types_;
};

class EvaluationError : public std::runtime_error {
public:
  explicit EvaluationError(const std::string &msg) : std::runtime_error(msg) {}
};

} // namespace falcon::dsl
