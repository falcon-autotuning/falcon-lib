#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon_core/physics/config/core/Config.hpp"
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector> // Added for std::vector in BuiltinFunc

namespace falcon::atc {
class Expr;
}

namespace falcon::autotuner {

/**
 * @brief Evaluates AST expressions at runtime.
 */
class ExprEvaluator {
public:
  using Value = ParameterMap::Value;
  using BuiltinFunc =
      std::function<Value(const std::string &, const std::vector<Value> &)>;

  ExprEvaluator(const ParameterMap &params,
                const falcon_core::physics::config::core::Config &config,
                BuiltinFunc builtin_handler = nullptr)
      : params_(params), config_(config), builtin_handler_(builtin_handler) {}

  Value evaluate(const atc::Expr *e);

private:
  const ParameterMap &params_;
  const falcon_core::physics::config::core::Config &config_;
  BuiltinFunc builtin_handler_;
};

} // namespace falcon::autotuner
