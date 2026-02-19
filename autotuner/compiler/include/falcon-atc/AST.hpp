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

struct ParamDecl {
  std::string name;
  ParamType type;
  std::shared_ptr<Value> default_value;
  static ParamDecl *make(int type, std::string *name) {
    auto *decl = new ParamDecl{std::move(*name), (ParamType)type, nullptr};
    delete name;
    return decl;
  }
};

struct SpecDecl {
  ParamType type;
  std::string name;
  std::string
      parameter; // e.g. "plunger_gate" in "Quantity pinchoff [plunger_gate]"
  SpecDecl(ParamType t, std::string n, std::string p = "")
      : type(t), name(std::move(n)), parameter(std::move(p)) {}

  static SpecDecl *make(int type, std::string *name,
                        std::string *param = nullptr) {
    SpecDecl *s;
    if (param)
      s = new SpecDecl((ParamType)type, std::move(*name), std::move(*param));
    else
      s = new SpecDecl((ParamType)type, std::move(*name), "");
    delete name;
    if (param)
      delete param;
    return s;
  }
};

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

  static ConstExpr *from_int(std::string *v) {
    auto *c = new ConstExpr(std::stoll(*v));
    delete v;
    return c;
  }
  static ConstExpr *from_double(std::string *v) {
    auto *c = new ConstExpr(std::stod(*v));
    delete v;
    return c;
  }
  static ConstExpr *from_string(std::string *v) {
    auto *c = new ConstExpr(std::move(*v));
    delete v;
    return c;
  }
  static ConstExpr *from_bool(bool b) { return new ConstExpr(b); }
};

class VarExpr : public Expr {
public:
  std::string name;
  VarExpr(std::string n) : name(std::move(n)) {}
  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<VarExpr>(name);
  }

  static VarExpr *make(std::string *n) {
    auto *v = new VarExpr(std::move(*n));
    delete n;
    return v;
  }
  static VarExpr *make_config() { return new VarExpr("config"); }
  static VarExpr *make_next(std::string *n) {
    auto *v = new VarExpr("next " + *n);
    delete n;
    return v;
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

  static BinaryExpr *make(std::string op, Expr *left, Expr *right) {
    return new BinaryExpr(std::move(op), std::unique_ptr<Expr>(left),
                          std::unique_ptr<Expr>(right));
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

  static UnaryExpr *make(std::string op, Expr *expr) {
    return new UnaryExpr(std::move(op), std::unique_ptr<Expr>(expr));
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
  static std::vector<std::unique_ptr<Expr>>
  to_unique_vec(std::vector<Expr *> *v) {
    std::vector<std::unique_ptr<Expr>> res;
    if (v) {
      for (auto e : *v)
        res.push_back(std::unique_ptr<Expr>(e));
    }
    return res;
  }

  static CallExpr *make(std::string *name, std::vector<Expr *> *args) {
    auto *c = new CallExpr(std::move(*name), to_unique_vec(args));
    delete name;
    delete args;
    return c;
  }
};

struct Assignment {
  std::vector<std::string> targets; // Support multiple targets: x, y = func();
  std::unique_ptr<Expr> expression;

  Assignment(std::vector<std::string> t, std::unique_ptr<Expr> expr)
      : targets(std::move(t)), expression(std::move(expr)) {}

  static Assignment *make(std::vector<std::string> *idents,
                          falcon::atc::Expr *expr) {
    auto *a = new Assignment{std::move(*idents), std::unique_ptr<Expr>(expr)};
    delete idents;
    return a;
  }
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

  static Mapping *make(std::string *src, std::string *dst = nullptr) {
    Mapping *m;
    if (dst)
      m = new Mapping(std::move(*src), std::move(*dst));
    else
      m = new Mapping(std::move(*src), *src); // dst = src if not provided
    delete src;
    if (dst)
      delete dst;
    return m;
  }
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

  // Static factory methods for Bison
  static TransitionTarget *
  from_state_expr_mappings(std::string *state, Expr *expr,
                           std::vector<Mapping> *mappings) {
    auto *t =
        new TransitionTarget{"", std::move(*state), std::unique_ptr<Expr>(expr),
                             std::move(*mappings)};
    delete state;
    delete mappings;
    return t;
  }
  static TransitionTarget *from_state_mappings(std::string *state,
                                               std::vector<Mapping> *mappings) {
    auto *t = new TransitionTarget{"", std::move(*state), nullptr,
                                   std::move(*mappings)};
    delete state;
    delete mappings;
    return t;
  }
  static TransitionTarget *
  from_ns_state_expr_mappings(std::string *ns, std::string *state, Expr *expr,
                              std::vector<Mapping> *mappings) {
    auto *t =
        new TransitionTarget{std::move(*ns), std::move(*state),
                             std::unique_ptr<Expr>(expr), std::move(*mappings)};
    delete ns;
    delete state;
    delete mappings;
    return t;
  }
  static TransitionTarget *
  from_ns_state_mappings(std::string *ns, std::string *state,
                         std::vector<Mapping> *mappings) {
    auto *t = new TransitionTarget{std::move(*ns), std::move(*state), nullptr,
                                   std::move(*mappings)};
    delete ns;
    delete state;
    delete mappings;
    return t;
  }
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
  static StateDecl *make(std::string *name, std::vector<ParamDecl> *params,
                         std::vector<ParamDecl> *temps, Expr *measurement,
                         std::vector<Transition> *transitions,
                         std::string *param = nullptr) {
    auto param_val = param ? std::move(*param) : "";
    auto *s = new StateDecl(std::move(*name), std::move(param_val),
                            std::move(*params), std::move(*temps),
                            std::unique_ptr<Expr>(measurement), false,
                            std::move(*transitions));
    delete name;
    delete params;
    delete temps;
    delete transitions;
    if (param)
      delete param;
    return s;
  }
};

struct ForLoop {
  std::string variable;
  std::unique_ptr<Expr> iterable;
  std::vector<StateDecl> states;

  ForLoop(std::string var, std::unique_ptr<Expr> iter,
          std::vector<StateDecl> sts)
      : variable(std::move(var)), iterable(std::move(iter)),
        states(std::move(sts)) {}

  static ForLoop *make(std::string *var, Expr *iter,
                       std::vector<StateDecl> *sts) {
    auto *f = new ForLoop(std::move(*var), std::unique_ptr<Expr>(iter),
                          std::move(*sts));
    delete var;
    delete sts;
    return f;
  }
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

  static AutotunerDecl *
  make(std::string *name, std::vector<ParamDecl> *input_params,
       std::vector<ParamDecl> *output_params,
       std::vector<std::string> *generic_params,
       std::vector<SpecDecl> *spec_inputs, std::vector<SpecDecl> *spec_outputs,
       std::vector<std::string> *requires_clause,
       std::vector<ParamDecl> *params_block, std::string *entry_clause,
       std::vector<ForLoop> *loop_list, std::vector<StateDecl> *states) {
    auto *decl = new AutotunerDecl{std::move(*name),
                                   std::move(*input_params),
                                   std::move(*output_params),
                                   std::move(*generic_params),
                                   std::move(*spec_inputs),
                                   std::move(*spec_outputs),
                                   std::move(*requires_clause),
                                   std::move(*params_block),
                                   std::move(*entry_clause),
                                   std::move(*states),
                                   std::move(*loop_list)};
    delete name;
    delete input_params;
    delete output_params;
    delete generic_params;
    delete spec_inputs;
    delete spec_outputs;
    delete requires_clause;
    delete params_block;
    delete entry_clause;
    delete loop_list;
    delete states;
    return decl;
  }
  AutotunerDecl(AutotunerDecl &&) noexcept = default;
  AutotunerDecl &operator=(AutotunerDecl &&) noexcept = default;
  AutotunerDecl(const AutotunerDecl &) = delete;
  AutotunerDecl &operator=(const AutotunerDecl &) = delete;
};

struct Program {
  std::vector<AutotunerDecl> autotuners;
  /**
   * @brief Makes the entire program from a list of Autotuners.
   */
  static Program *make(std::vector<AutotunerDecl> *autotuners) {
    if (autotuners == nullptr) {
      return new Program{};
    }
    auto *res = new Program{std::move(*autotuners)};
    delete autotuners;
    return res;
  }
};

} // namespace falcon::atc
