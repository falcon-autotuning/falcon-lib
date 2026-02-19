#include "falcon-atc/ASTHelpers.hpp"

using namespace falcon::atc;

std::vector<Transition> *
make_if_transition(Expr *cond, std::vector<Transition> *then_part) {
  for (auto &t : *then_part) {
    if (t.condition) {
      t.condition = std::make_unique<BinaryExpr>("&&", cond->clone(),
                                                 std::move(t.condition));
    } else {
      t.condition = cond->clone();
    }
  }
  delete cond;
  return then_part;
}

std::vector<Transition> *
make_if_else_transition(Expr *cond, std::vector<Transition> *then_part,
                        std::vector<Transition> *else_part) {
  for (auto &t : *then_part) {
    if (t.condition) {
      t.condition = std::make_unique<BinaryExpr>("&&", cond->clone(),
                                                 std::move(t.condition));
    } else {
      t.condition = cond->clone();
    }
  }
  auto inv_cond = std::make_unique<UnaryExpr>("!", cond->clone());
  for (auto &t : *else_part) {
    if (t.condition) {
      t.condition = std::make_unique<BinaryExpr>("&&", inv_cond->clone(),
                                                 std::move(t.condition));
    } else {
      t.condition = inv_cond->clone();
    }
  }
  then_part->insert(then_part->end(),
                    std::make_move_iterator(else_part->begin()),
                    std::make_move_iterator(else_part->end()));
  delete else_part;
  delete cond;
  return then_part;
}

std::vector<Transition> *make_simple_transition(SimpleTransitionKind kind,
                                                std::string *msg) {
  auto *v = new std::vector<Transition>();
  Transition t;
  switch (kind) {
  case SimpleTransitionKind::Success:
    t.is_success = true;
    break;
  case SimpleTransitionKind::Fail:
    t.is_fail = true;
    if (msg) {
      t.error_message = std::move(*msg);
      delete msg;
    }
    break;
  case SimpleTransitionKind::Terminal:
    t.target.state_name = "_TERMINAL_";
    break;
  }
  v->push_back(std::move(t));
  return v;
}
std::vector<Transition> *make_arrow_transition(TransitionTarget *target) {
  auto *v = new std::vector<Transition>();
  v->emplace_back(nullptr, std::move(*target), std::vector<Assignment>{});
  delete target;
  return v;
}
std::vector<Transition> *
make_assignment_arrow_transition(std::vector<Assignment> *assigns,
                                 TransitionTarget *target) {
  auto *v = new std::vector<Transition>();
  v->emplace_back(nullptr, std::move(*target), std::move(*assigns));
  delete assigns;
  delete target;
  return v;
}
std::vector<Transition> *make_assignment_transition(Assignment *asgn) {
  auto *v = new std::vector<Transition>();
  std::vector<Assignment> asgns;
  asgns.push_back(std::move(*asgn));
  v->emplace_back(nullptr, TransitionTarget("", "", nullptr, {}),
                  std::move(asgns));
  delete asgn;
  return v;
}
