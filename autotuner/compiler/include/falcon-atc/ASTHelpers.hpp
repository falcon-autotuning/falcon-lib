#pragma once
#include "falcon-atc/AST.hpp"
/**
 * @brief Helpers for using the AST with Bison.
 */

using namespace falcon::atc;

/**
 * @brief Makes the entire program from a list of Autotuners.
 */
Program *make_program(std::vector<AutotunerDecl> *autotuners);

std::vector<AutotunerDecl> *make_autotuner_list();
std::vector<AutotunerDecl> *append_autotuner(std::vector<AutotunerDecl> *list,
                                             AutotunerDecl *decl);
