#pragma once
#include "falcon-atc/AST.hpp"
/**
 * @brief Helpers for using the AST with Bison.
 */

using namespace falcon::atc;

/**
 * @brief an if style transition in a state
 */
std::vector<Transition> *make_if_transition(Expr *cond,
                                            std::vector<Transition> *then_part);

/**
 * @brief an if -else  style transition in a state
 */
std::vector<Transition> *
make_if_else_transition(Expr *cond, std::vector<Transition> *then_part,
                        std::vector<Transition> *else_part);

enum class SimpleTransitionKind { Success, Fail, Terminal };
std::vector<Transition> *make_simple_transition(SimpleTransitionKind kind,
                                                std::string *msg = nullptr);
std::vector<Transition> *make_arrow_transition(TransitionTarget *target);
std::vector<Transition> *
make_assignment_arrow_transition(std::vector<Assignment> *assigns,
                                 TransitionTarget *target);
std::vector<Transition> *make_assignment_transition(Assignment *asgn);

/**
 * @brief append an item to an existing list
 */
// For value types (e.g., ParamDecl, Mapping)
template <typename T>
std::vector<T> *append_item(std::vector<T> *list, T *decl) {
  list->push_back(std::move(*decl));
  delete decl;
  return list;
}

// For pointer types (e.g., Expr*)
template <typename T>
std::vector<T *> *append_item(std::vector<T *> *list, T *item) {
  list->push_back(item);
  return list;
}
/**
 * @brief Merges two lists together
 */
template <typename T>
std::vector<T> *merge_lists(std::vector<T> *a, std::vector<T> *b) {
  a->insert(a->end(), std::make_move_iterator(b->begin()),
            std::make_move_iterator(b->end()));
  delete b;
  return a;
}
/**
 * @brief make a list of values
 */
template <typename T> std::vector<T> *make_list() {
  return new std::vector<T>();
}
