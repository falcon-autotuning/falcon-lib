#include "falcon-lsp/DiagnosticsProvider.hpp"

namespace falcon::lsp {

std::vector<::lsp::Diagnostic>
DiagnosticsProvider::diagnostics(const FalconDocument &doc) {

  std::vector<::lsp::Diagnostic> result;

  for (const auto &err : doc.parse_errors) {
    ::lsp::Diagnostic diag;

    // AST uses 1-indexed; LSP uses 0-indexed
    unsigned start_line =
        err.first_line > 0 ? static_cast<unsigned>(err.first_line - 1) : 0u;
    unsigned start_col =
        err.first_column > 0 ? static_cast<unsigned>(err.first_column - 1) : 0u;
    unsigned end_line = err.last_line > 0
                            ? static_cast<unsigned>(err.last_line - 1)
                            : start_line;
    unsigned end_col = err.last_column > 0
                           ? static_cast<unsigned>(err.last_column)
                           : start_col + 1;

    diag.range = ::lsp::Range{::lsp::Position{start_line, start_col},
                              ::lsp::Position{end_line, end_col}};
    diag.message = err.message;
    diag.severity =
        ::lsp::DiagnosticSeverityEnum{::lsp::DiagnosticSeverity::Error};
    diag.source = std::string{"falcon-lsp"};

    result.push_back(std::move(diag));
  }

  return result;
}

} // namespace falcon::lsp
