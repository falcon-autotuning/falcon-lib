#pragma once

#include <cstdint>
#include <memory>
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
  Group,
  Connection
};

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

struct Param {
  std::string name;
  std::unique_ptr<Expr> default_value;
};

struct ParamDecl : public Param {
  ParamType type;
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
  std::string name;
  std::string
      parameter; // e.g. "plunger_gate" in "state start_sweep [plunger_gate]"
  std::vector<ParamDecl> params;
  std::vector<ParamDecl> temps;
  std::unique_ptr<Expr> measurement;
  bool is_terminal = false;
  std::vector<Transition> transitions;

  StateDecl(std::string n, std::string p_name, std::vector<ParamDecl> p,
            std::vector<ParamDecl> t, std::unique_ptr<Expr> m, bool terminal,
            std::vector<Transition> trans)
      : name(std::move(n)), parameter(std::move(p_name)), params(std::move(p)),
        temps(std::move(t)), measurement(std::move(m)), is_terminal(terminal),
        transitions(std::move(trans)) {}

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
  std::string name;
  std::vector<ParamDecl> inputs;
  std::vector<ParamDecl> outputs;
  std::vector<std::string> generic_params;
  std::vector<SpecDecl> spec_inputs;
  std::vector<SpecDecl> spec_outputs;
  std::vector<std::string> requirements;
  std::vector<ParamDecl> params;
  std::string entry_state;
  std::vector<StateDecl> states;
  std::vector<ForLoop> loops;

  AutotunerDecl(std::string n, std::vector<ParamDecl> in,
                std::vector<ParamDecl> out, std::vector<std::string> gp,
                std::vector<SpecDecl> si, std::vector<SpecDecl> so,
                std::vector<std::string> req, std::vector<ParamDecl> p,
                std::string entry, std::vector<StateDecl> s,
                std::vector<ForLoop> l)
      : name(std::move(n)), inputs(std::move(in)), outputs(std::move(out)),
        generic_params(std::move(gp)), spec_inputs(std::move(si)),
        spec_outputs(std::move(so)), requirements(std::move(req)),
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
