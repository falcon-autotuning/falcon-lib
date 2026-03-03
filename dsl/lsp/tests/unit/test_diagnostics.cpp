#include "falcon-lsp/DiagnosticsProvider.hpp"
#include "falcon-lsp/FalconDocument.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

TEST(DiagnosticsProvider, NoDiagnosticsOnValidFile) {
  FalconDocumentParser parser;
  const std::string src = R"(
autotuner T (int a) -> (int b) {
  b = a;
  start -> s;
  state s { terminal; }
}
)";
  auto doc = parser.parse("file:///t.fal", src);
  ASSERT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());

  DiagnosticsProvider dp;
  auto diags = dp.diagnostics(doc);
  EXPECT_TRUE(diags.empty());
}

TEST(DiagnosticsProvider, DiagnosticsOnInvalidFile) {
  FalconDocumentParser parser;
  const std::string src = "autotuner BROKEN {{{";
  auto doc = parser.parse("file:///broken.fal", src);
  EXPECT_FALSE(doc.parse_errors.empty());

  DiagnosticsProvider dp;
  auto diags = dp.diagnostics(doc);
  EXPECT_FALSE(diags.empty());
  EXPECT_EQ(diags[0].severity->index(), lsp::DiagnosticSeverity::Error);
}

TEST(DiagnosticsProvider, DiagnosticPositionIsZeroIndexed) {
  FalconDocumentParser parser;
  const std::string src = "autotuner BROKEN {{{";
  auto doc = parser.parse("file:///broken.fal", src);
  ASSERT_FALSE(doc.parse_errors.empty());
  ASSERT_FALSE(doc.parse_errors[0].first_line < 1);

  DiagnosticsProvider dp;
  auto diags = dp.diagnostics(doc);
  ASSERT_FALSE(diags.empty());
  // LSP line is 0-indexed; AST line is 1-indexed
  EXPECT_EQ(diags[0].range.start.line,
            static_cast<unsigned>(doc.parse_errors[0].first_line - 1));
}
