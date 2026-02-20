#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include <falcon_core/physics/config/core/Config.hpp>
#include <functional>
#include <string>
#include <utility>
#include <vector> // Added for std::vector in BuiltinFunc

namespace falcon::atc {
class Expr;
}

namespace falcon::autotuner {

using Value = ParameterMap::Value;
struct EvalResult {
  Value value;
  atc::ParamType type;
};
/**
 * @brief Evaluates AST expressions at runtime.
 */
class ExprEvaluator {
public:
  using Value = ParameterMap::Value;
  using BuiltinFunc =
      std::function<Value(const std::string &, const std::vector<Value> &)>;

  ExprEvaluator(ParameterMap &params,
                falcon_core::physics::config::core::ConfigSP config,
                BuiltinFunc builtin_handler = nullptr)
      : params_(params), config_(std::move(config)),
        builtin_handler_(std::move(builtin_handler)) {}

  EvalResult evaluate(const std::unique_ptr<atc::Expr> &e);

private:
  ParameterMap &params_;
  falcon_core::physics::config::core::ConfigSP config_;
  BuiltinFunc builtin_handler_;
};

} // namespace falcon::autotuner
