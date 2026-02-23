#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace falcon::atc {

enum class ParamType : std::uint8_t {
  Int,
  Float,
  Bool,
  String,
  Quantity,
  Config,
  Connection,
  Connections,
  DeviceCharacteristic,
  DeviceCharacteristicQuery
};
inline std::string to_string(ParamType type) {
  switch (type) {
  case ParamType::Int:
    return "Int";
  case ParamType::Float:
    return "Float";
  case ParamType::Bool:
    return "Bool";
  case ParamType::String:
    return "String";
  case ParamType::Quantity:
    return "Quantity";
  case ParamType::Config:
    return "Config";
  case ParamType::Connection:
    return "Connection";
  case ParamType::Connections:
    return "Connections";
  case ParamType::DeviceCharacteristic:
    return "DeviceCharacteristic";
  case ParamType::DeviceCharacteristicQuery:
    return "DeviceCharacteristicQuery";
  default:
    return "<unknown>";
  }
}

using Value = std::variant<int64_t, double, bool, std::string>;

class Expr {
public:
  virtual ~Expr() = default;
  virtual std::unique_ptr<Expr> clone() const = 0;
};

class ConstExpr : public Expr {
public:
  std::variant<int64_t, double, bool, std::string> value;

  ConstExpr(std::variant<int64_t, double, bool, std::string> v)
      : value(std::move(v)) {}

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<ConstExpr>(value);
  }
};

class VarExpr : public Expr {
public:
  std::string name;

  VarExpr(std::string n) : name(std::move(n)) {}

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<VarExpr>(name);
  }
};

class MemberExpr : public Expr {
public:
  std::unique_ptr<Expr> object;
  std::string member;

  MemberExpr(std::unique_ptr<Expr> obj, std::string mem)
      : object(std::move(obj)), member(std::move(mem)) {}

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<MemberExpr>(object->clone(), member);
  }
};

class BinaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::string o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
      : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<BinaryExpr>(op, left->clone(), right->clone());
  }
};

class UnaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> expr;

  UnaryExpr(std::string o, std::unique_ptr<Expr> e)
      : op(std::move(o)), expr(std::move(e)) {}

  [[nodiscard]] std::unique_ptr<Expr> clone() const override {
    return std::make_unique<UnaryExpr>(op, expr->clone());
  }
};

class CallExpr : public Expr {
public:
  std::string name;
  std::vector<std::unique_ptr<Expr>> args;

  CallExpr(std::string n, std::vector<std::unique_ptr<Expr>> a)
      : name(std::move(n)), args(std::move(a)) {}

  [[nodiscard]] std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> cloned_args;
    for (const auto &arg : args)
      cloned_args.push_back(arg->clone());
    return std::make_unique<CallExpr>(name, std::move(cloned_args));
  }
};

/**
 * @brief A param only needs a name.
 * And there are two optional arguments <default_value and type>
 * atleast one of them must be included.
 * If a type is included this is a type declaration.
 */
struct Param {
  std::string name;
  std::optional<std::unique_ptr<Expr>> default_value;
  std::optional<ParamType> type;
};

struct SpecDecl {
  ParamType type;
  std::string name;
  std::string
      parameter; // e.g. "plunger_gate" in "Quantity pinchoff [plunger_gate]"

  SpecDecl(ParamType t, std::string n, std::string p = "")
      : type(t), name(std::move(n)), parameter(std::move(p)) {}
};

struct Assignment {
  std::vector<std::string> targets; // Support multiple targets: x, y = func();
  std::unique_ptr<Expr> expression;

  Assignment(std::vector<std::string> t, std::unique_ptr<Expr> expr)
      : targets(std::move(t)), expression(std::move(expr)) {}

  Assignment(Assignment &&) noexcept = default;
  Assignment &operator=(Assignment &&) noexcept = default;
  Assignment(const Assignment &) = delete;
  Assignment &operator=(const Assignment &) = delete;
};

struct Mapping {
  std::string src;
  std::string dst;

  Mapping(std::string s, std::string d)
      : src(std::move(s)), dst(std::move(d)) {}
};

struct TransitionTarget {
  std::string autotuner_name;
  std::string state_name;
  std::unique_ptr<Expr> parameter;
  std::vector<Mapping> mappings;

  TransitionTarget(std::string at, std::string st, std::unique_ptr<Expr> p,
                   std::vector<Mapping> m)
      : autotuner_name(std::move(at)), state_name(std::move(st)),
        parameter(std::move(p)), mappings(std::move(m)) {}
};

struct Transition {
  std::unique_ptr<Expr> condition;
  TransitionTarget target;
  std::vector<Assignment> assignments;
  bool is_success = false;
  bool is_fail = false;
  std::string error_message;

  Transition(std::unique_ptr<Expr> cond, TransitionTarget tgt,
             std::vector<Assignment> asgn)
      : condition(std::move(cond)), target(std::move(tgt)),
        assignments(std::move(asgn)) {}

  Transition()
      : condition(nullptr), target("", "", nullptr, {}), assignments() {}

  Transition(Transition &&) noexcept = default;
  Transition &operator=(Transition &&) noexcept = default;
  Transition(const Transition &) = delete;
  Transition &operator=(const Transition &) = delete;
};

struct StateDecl {
  using ParamPtrVec = std::vector<std::unique_ptr<Param>>;
  std::string name;
  std::string
      generic; // e.g. "plunger_gate" in "state start_sweep [plunger_gate]"
  ParamPtrVec params;
  std::unique_ptr<Expr> measurement;
  bool is_terminal = false;
  std::vector<Transition> transitions;

  StateDecl(std::string n, std::string p_name, ParamPtrVec params,
            std::unique_ptr<Expr> m, bool terminal,
            std::vector<Transition> trans)
      : name(std::move(n)), generic(std::move(p_name)),
        params(std::move(params)), measurement(std::move(m)),
        is_terminal(terminal), transitions(std::move(trans)) {}

  StateDecl(StateDecl &&) noexcept = default;
  StateDecl &operator=(StateDecl &&) noexcept = default;
  StateDecl(const StateDecl &) = delete;
  StateDecl &operator=(const StateDecl &) = delete;
};

struct ForLoop {
  std::string variable;
  std::unique_ptr<Expr> iterable;
  std::vector<StateDecl> states;

  ForLoop(std::string var, std::unique_ptr<Expr> iter,
          std::vector<StateDecl> sts)
      : variable(std::move(var)), iterable(std::move(iter)),
        states(std::move(sts)) {}

  ForLoop(ForLoop &&) noexcept = default;
  ForLoop &operator=(ForLoop &&) noexcept = default;
  ForLoop(const ForLoop &) = delete;
  ForLoop &operator=(const ForLoop &) = delete;
};

struct AutotunerDecl {
  using ParamPtrVec = std::vector<std::unique_ptr<Param>>;
  using StringVec = std::vector<std::string>;
  using StateDeclVec = std::vector<StateDecl>;
  using ForLoopVec = std::vector<ForLoop>;

  std::string name;
  ParamPtrVec inputs;
  ParamPtrVec outputs;
  StringVec generic_params;
  StringVec requirements;
  ParamPtrVec params;
  std::string entry_state;
  StateDeclVec states;
  ForLoopVec loops;

  AutotunerDecl(std::string n, ParamPtrVec in, ParamPtrVec out, StringVec gp,
                StringVec req, ParamPtrVec p, std::string entry, StateDeclVec s,
                ForLoopVec l)
      : name(std::move(n)), inputs(std::move(in)), outputs(std::move(out)),
        generic_params(std::move(gp)), requirements(std::move(req)),
        params(std::move(p)), entry_state(std::move(entry)),
        states(std::move(s)), loops(std::move(l)) {}

  AutotunerDecl(AutotunerDecl &&) noexcept = default;
  AutotunerDecl &operator=(AutotunerDecl &&) noexcept = default;
  AutotunerDecl(const AutotunerDecl &) = delete;
  AutotunerDecl &operator=(const AutotunerDecl &) = delete;
};

struct Program {
  std::vector<AutotunerDecl> autotuners;
};

} // namespace falcon::atc
