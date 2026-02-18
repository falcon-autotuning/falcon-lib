#include "falcon-atc/ASTHelpers.hpp"

using namespace falcon::atc;
Program *make_program(std::vector<AutotunerDecl> *autotuners) {
  if (autotuners == nullptr) {
    return new Program{};
  }
  auto *res = new Program{std::move(*autotuners)};
  delete autotuners;
  return res;
}

std::vector<AutotunerDecl> *make_autotuner_list() {
  return new std::vector<AutotunerDecl>();
}

std::vector<AutotunerDecl> *append_autotuner(std::vector<AutotunerDecl> *list,
                                             AutotunerDecl *decl) {
  list->push_back(std::move(*decl));
  delete decl;
  return list;
}
