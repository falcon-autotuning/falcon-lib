#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon-database/DeviceCharacteristic.hpp"
#include <falcon-autotuner/log.hpp>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

namespace falcon::autotuner {

struct ToDouble {
  double operator()(int64_t i) const { return static_cast<double>(i); }
  double operator()(double d) const { return d; }
  double operator()(bool b) const { return b ? 1.0 : 0.0; }
  template <typename T> double operator()(const T &) const {
    throw std::runtime_error("Cannot convert to double");
  }
};

struct ToBool {
  bool operator()(bool b) const { return b; }
  bool operator()(int64_t i) const { return i != 0; }
  bool operator()(double d) const { return d != 0.0; }
  bool operator()(const database::DeviceCharacteristic &) const {
    return true; // Not null
  }
  template <typename T> bool operator()(const T &) const {
    throw std::runtime_error("Cannot convert to bool");
  }
};

static ExprEvaluator::Value perform_math(const atc::BinaryExpr *b,
                                         const ExprEvaluator::Value &left,
                                         const ExprEvaluator::Value &right) {
  auto both_int = [&]() {
    return std::holds_alternative<int64_t>(left) &&
           std::holds_alternative<int64_t>(right);
  };
  auto as_int = [&]() {
    return std::make_pair(std::get<int64_t>(left), std::get<int64_t>(right));
  };
  auto as_double = [&]() {
    return std::make_pair(std::visit(ToDouble{}, left),
                          std::visit(ToDouble{}, right));
  };

  const std::string &op = b->op;

  // Equality
  if (op == "==") {
    return left == right;
  }
  if (op == "!=") {
    return left != right;
  }

  if (op == ">" || op == "<" || op == ">=" || op == "<=" || op == "+" ||
      op == "-" || op == "*") {
    if (both_int()) {
      auto [left, right] = as_int();
      if (op == ">") {
        return left > right;
      }
      if (op == "<") {
        return left < right;
      }
      if (op == ">=") {
        return left >= right;
      }
      if (op == "<=") {
        return left <= right;
      }
      if (op == "+") {
        return left + right;
      }
      if (op == "-") {
        return left - right;
      }
      if (op == "*") {
        return left * right;
      }
    } else {
      auto [left, right] = as_double();
      if (op == ">") {
        return left > right;
      }
      if (op == "<") {
        return left < right;
      }
      if (op == ">=") {
        return left >= right;
      }
      if (op == "<=") {
        return left <= right;
      }
      if (op == "+") {
        return left + right;
      }
      if (op == "-") {
        return left - right;
      }
      if (op == "*") {
        return left * right;
      }
    }
  }

  // Division
  if (op == "/") {
    if (both_int()) {
      auto [left, right] = as_int();
      if (right == 0) {
        throw std::runtime_error("Division by zero");
      }
      if (left % right == 0) {
        return left / right;
      }
      // else fall through to double
    }
    auto [left, right] = as_double();
    if (right == 0.0) {
      throw std::runtime_error("Division by zero");
    }
    return left / right;
  }

  throw std::runtime_error(
      fmt::format("Invalid math to perform. The operation was to be {} {} {}",
                  to_string(ParameterMap::deduce_type(left)), op,
                  to_string(ParameterMap::deduce_type(right))));
}

EvalResult ExprEvaluator::evaluate(const std::unique_ptr<atc::Expr> &e) {
  if (!e) {
    return {true, atc::ParamType::Bool};
  }
  if (const auto *c = dynamic_cast<const atc::ConstExpr *>(e.get())) {
    auto val = std::visit(
        [](auto &&arg) -> ExprEvaluator::Value {
          return ExprEvaluator::Value(arg);
        },
        c->value);
    return {val, ParameterMap::deduce_type(val)};
  }
  if (const auto *v = dynamic_cast<const atc::VarExpr *>(e.get())) {
    log::debug(fmt::format("Evaluating a variable expression for {}", v->name));
    if (v->name == "config") {
      return {true, atc::ParamType::Bool};
    }
    auto val = params_.get<ExprEvaluator::Value>(v->name);
    return {val, params_.get_type(v->name)};
  }
  if (const auto *m = dynamic_cast<const atc::MemberExpr *>(e.get())) {
    if (const auto *obj_var =
            dynamic_cast<const atc::VarExpr *>(m->object.get())) {
      if (obj_var->name == "config") {
        if (m->member == "plunger_gates") {
          auto val = config_->plunger_gates();
          return {val, atc::ParamType::Connections};
        }
        if (m->member == "barrier_gates") {
          auto val = config_->barrier_gates();
          return {val, atc::ParamType::Connections};
        }
      }
    }
    auto obj_res = evaluate(m->object->clone());
    if (std::holds_alternative<database::DeviceCharacteristic>(obj_res.value)) {
      const auto &dchar =
          std::get<database::DeviceCharacteristic>(obj_res.value);
      if (m->member == "name")
        return {dchar.name, atc::ParamType::String};
      if (m->member == "uncertainty")
        return {dchar.uncertainty.value_or(0.0), atc::ParamType::Float};
      if (m->member == "hash")
        return {dchar.hash.value_or(""), atc::ParamType::String};
      if (m->member == "record_time")
        return {dchar.time.value_or(0), atc::ParamType::Int};
      if (m->member == "device_state")
        return {dchar.state.value_or(""), atc::ParamType::String};
      if (m->member == "unit_name")
        return {dchar.unit_name.value_or(""), atc::ParamType::String};
    }
    throw std::runtime_error("Unsupported member access: " + m->member);
  }
  if (auto b = dynamic_cast<const atc::BinaryExpr *>(e.get())) {
    auto left = evaluate(b->left->clone());
    auto right = evaluate(b->right->clone());
    auto val = perform_math(b, left.value, right.value);
    return {val, ParameterMap::deduce_type(val)};
  }
  if (auto u = dynamic_cast<const atc::UnaryExpr *>(e.get())) {
    auto val = evaluate(u->expr->clone());
    if (u->op == "!") {
      return {!std::visit(ToBool{}, val.value), atc::ParamType::Bool};
    }
    if (u->op == "-") {
      return {-std::visit(ToDouble{}, val.value), atc::ParamType::Float};
    }
  }
  if (auto c = dynamic_cast<const atc::CallExpr *>(e.get())) {
    if (!builtin_handler_) {
      throw std::runtime_error("Built-in handler not set for CallExpr: " +
                               c->name);
    }
    std::vector<ExprEvaluator::Value> args;
    args.reserve(c->args.size());
    for (const auto &arg : c->args) {
      args.push_back(evaluate(arg->clone()).value);
    }
    auto val = builtin_handler_(c->name, args);
    return {val, params_.get_type(c->name)};
  }
  return {true, atc::ParamType::Bool};
}

} // namespace falcon::autotuner
