#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/DeviceCharacteristic.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/device_structures/Connections.hpp"
#include <cmath>
#include <stdexcept>

namespace falcon::autotuner {

struct ToDouble {
  double operator()(int64_t i) const { return static_cast<double>(i); }
  double operator()(double d) const { return d; }
  double operator()(bool b) const { return b ? 1.0 : 0.0; }
  double operator()(const DeviceCharacteristic &) const {
    throw std::runtime_error("Cannot convert DeviceCharacteristic to double");
  }
  template <typename T> double operator()(const T &) const {
    throw std::runtime_error("Cannot convert to double");
  }
};

struct ToBool {
  bool operator()(bool b) const { return b; }
  bool operator()(int64_t i) const { return i != 0; }
  bool operator()(double d) const { return d != 0.0; }
  bool operator()(const DeviceCharacteristic &) const {
    return true; // Not null
  }
  template <typename T> bool operator()(const T &) const {
    throw std::runtime_error("Cannot convert to bool");
  }
};

ExprEvaluator::Value ExprEvaluator::evaluate(const atc::Expr *e) {
  if (!e)
    return ExprEvaluator::Value(true);

  if (auto c = dynamic_cast<const atc::ConstExpr *>(e)) {
    return std::visit(
        [](auto &&arg) -> ExprEvaluator::Value {
          return ExprEvaluator::Value(arg);
        },
        c->value);
  }

  if (auto v = dynamic_cast<const atc::VarExpr *>(e)) {
    if (v->name == "config") {
      return ExprEvaluator::Value(true);
    }
    return params_.get<ExprEvaluator::Value>(v->name);
  }

  if (auto m = dynamic_cast<const atc::MemberExpr *>(e)) {
    if (auto obj_var = dynamic_cast<const atc::VarExpr *>(m->object.get())) {
      if (obj_var->name == "config") {
        if (m->member == "plunger_gates") {
          return ExprEvaluator::Value(config_.plunger_gates());
        }
        if (m->member == "barrier_gates") {
          return ExprEvaluator::Value(config_.barrier_gates());
        }
      }
    }

    // Handle Member access for variables in ParameterMap
    auto obj_val = evaluate(m->object.get());
    if (std::holds_alternative<DeviceCharacteristic>(obj_val)) {
      const auto &dc = std::get<DeviceCharacteristic>(obj_val);
      if (m->member == "name")
        return ExprEvaluator::Value(dc.name);
      if (m->member == "uncertainty")
        return ExprEvaluator::Value(dc.uncertainty);
      if (m->member == "hash")
        return ExprEvaluator::Value(dc.hash);
      if (m->member == "record_time")
        return ExprEvaluator::Value(dc.record_time);
      if (m->member == "device_state")
        return ExprEvaluator::Value(dc.device_state);
      if (m->member == "unit_name")
        return ExprEvaluator::Value(dc.unit_name);
    }

    throw std::runtime_error("Unsupported member access: " + m->member);
  }

  if (auto b = dynamic_cast<const atc::BinaryExpr *>(e)) {
    auto left = evaluate(b->left.get());
    auto right = evaluate(b->right.get());

    if (b->op == ">") {
      return std::visit(ToDouble{}, left) > std::visit(ToDouble{}, right);
    }
    if (b->op == "<") {
      return std::visit(ToDouble{}, left) < std::visit(ToDouble{}, right);
    }
    if (b->op == ">=") {
      return std::visit(ToDouble{}, left) >= std::visit(ToDouble{}, right);
    }
    if (b->op == "<=") {
      return std::visit(ToDouble{}, left) <= std::visit(ToDouble{}, right);
    }
    if (b->op == "==") {
      return left == right;
    }
    if (b->op == "!=") {
      return left != right;
    }
    if (b->op == "+") {
      return std::visit(ToDouble{}, left) + std::visit(ToDouble{}, right);
    }
    if (b->op == "-") {
      return std::visit(ToDouble{}, left) - std::visit(ToDouble{}, right);
    }
    if (b->op == "*") {
      return std::visit(ToDouble{}, left) * std::visit(ToDouble{}, right);
    }
    if (b->op == "/") {
      return std::visit(ToDouble{}, left) / std::visit(ToDouble{}, right);
    }
  }

  if (auto u = dynamic_cast<const atc::UnaryExpr *>(e)) {
    auto val = evaluate(u->expr.get());
    if (u->op == "!") {
      return !std::visit(ToBool{}, val);
    }
    if (u->op == "-") {
      return -std::visit(ToDouble{}, val);
    }
  }

  if (auto c = dynamic_cast<const atc::CallExpr *>(e)) {
    if (!builtin_handler_) {
      throw std::runtime_error("Built-in handler not set for CallExpr: " +
                               c->name);
    }
    std::vector<ExprEvaluator::Value> args;
    for (const auto &arg : c->args) {
      args.push_back(evaluate(arg.get()));
    }
    return builtin_handler_(c->name, args);
  }

  return ExprEvaluator::Value(true);
}

} // namespace falcon::autotuner
