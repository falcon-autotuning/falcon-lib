#include "falcon-autotuner/ExprEvaluator.hpp"
#include "falcon-atc/AST.hpp"
#include "falcon-database/DeviceCharacteristic.hpp"
#include <stdexcept>

namespace falcon::autotuner {

struct ToDouble {
  double operator()(int64_t i) const { return static_cast<double>(i); }
  double operator()(double d) const { return d; }
  double operator()(bool b) const { return b ? 1.0 : 0.0; }
  double operator()(const database::DeviceCharacteristic &) const {
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
  bool operator()(const database::DeviceCharacteristic &) const {
    return true; // Not null
  }
  template <typename T> bool operator()(const T &) const {
    throw std::runtime_error("Cannot convert to bool");
  }
};

ExprEvaluator::Value ExprEvaluator::evaluate(const atc::Expr *e) {
  if (e == nullptr) {
    return true;
  }

  if (const auto *c = dynamic_cast<const atc::ConstExpr *>(e)) {
    return std::visit(
        [](auto &&arg) -> ExprEvaluator::Value {
          return ExprEvaluator::Value(arg);
        },
        c->value);
  }

  if (const auto *v = dynamic_cast<const atc::VarExpr *>(e)) {
    if (v->name == "config") {
      return true;
    }
    return params_.get<ExprEvaluator::Value>(v->name);
  }

  if (const auto *m = dynamic_cast<const atc::MemberExpr *>(e)) {
    if (const auto *obj_var =
            dynamic_cast<const atc::VarExpr *>(m->object.get())) {
      if (obj_var->name == "config") {
        if (m->member == "plunger_gates") {
          return config_->plunger_gates();
        }
        if (m->member == "barrier_gates") {
          return config_->barrier_gates();
        }
      }
    }

    // Handle Member access for variables in ParameterMap
    auto obj_val = evaluate(m->object.get());
    if (std::holds_alternative<database::DeviceCharacteristic>(obj_val)) {
      const auto &dchar = std::get<database::DeviceCharacteristic>(obj_val);
      if (m->member == "name") {
        return dchar.name;
      }
      if (m->member == "uncertainty") {
        return dchar.uncertainty.value_or(0.0);
      }
      if (m->member == "hash") {
        return dchar.hash.value_or("");
      }
      if (m->member == "record_time") {
        return dchar.time.value_or(0);
      }
      if (m->member == "device_state") {
        return dchar.state.value_or("");
      }
      if (m->member == "unit_name") {
        return dchar.unit_name.value_or("");
      }
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

  return true;
}

} // namespace falcon::autotuner
