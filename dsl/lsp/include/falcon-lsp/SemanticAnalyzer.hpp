#pragma once

#include "FalconDocument.hpp"
#include <lsp/types.h>
#include <vector>

namespace falcon::lsp {

/**
 * @brief Walks the AST of a parsed FalconDocument and emits semantic
 *        diagnostics beyond pure parse errors:
 *
 *   - Calls to undefined functions / autotuners / routines.
 *   - Calls with the wrong number of arguments.
 *
 *  Does NOT modify the document; call after FalconDocumentParser::parse()
 *  and ImportResolver::resolve_document().
 */
class SemanticAnalyzer {
public:
  [[nodiscard]] std::vector<::lsp::Diagnostic>
  analyze(const FalconDocument &doc);
};

} // namespace falcon::lsp
