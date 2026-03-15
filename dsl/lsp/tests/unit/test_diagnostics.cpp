#include "falcon-lsp/DiagnosticsProvider.hpp"
#include "falcon-lsp/FalconDocument.hpp"
#include "falcon-lsp/ImportResolver.hpp"
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

// ---------------------------------------------------------------------------
// ImportResolver tests
// ---------------------------------------------------------------------------

TEST(ImportResolver, NoWarningForResolvedImport) {
  FalconDocument doc;
  doc.text = R"(import (
"./Adder.fal"
)
autotuner T () -> () { start -> s; state s { terminal; } }
)";
  doc.import_paths = {"./Adder.fal"};

  ImportResolver resolver;
  resolver.resolve("./Adder.fal");

  auto diags = resolver.check_imports(doc);
  EXPECT_TRUE(diags.empty());
}

TEST(ImportResolver, WarningForUnresolvedImport) {
  FalconDocument doc;
  doc.text = R"(import (
"./Missing.fal"
)
autotuner T () -> () { start -> s; state s { terminal; } }
)";
  doc.import_paths = {"./Missing.fal"};

  ImportResolver resolver; // nothing resolved

  auto diags = resolver.check_imports(doc);
  ASSERT_FALSE(diags.empty());
  EXPECT_EQ(diags[0].severity->index(), lsp::DiagnosticSeverity::Warning);
  EXPECT_NE(diags[0].message.find("./Missing.fal"), std::string::npos);
}

TEST(ImportResolver, WarningRangeCoversQuotedPath) {
  FalconDocument doc;
  doc.text = "ffimport \"MyWrapper.cpp\" () ()\n";
  doc.ffimport_paths = {"MyWrapper.cpp"};

  ImportResolver resolver; // nothing resolved

  auto diags = resolver.check_imports(doc);
  ASSERT_FALSE(diags.empty());
  // Range should be on line 0 and cover the quoted string
  EXPECT_EQ(diags[0].range.start.line, 0u);
  EXPECT_EQ(diags[0].range.start.character, 9u); // position of '"'
}

TEST(ImportResolver, IsResolvedReturnsFalseByDefault) {
  ImportResolver resolver;
  EXPECT_FALSE(resolver.is_resolved("./foo.fal"));
}

TEST(ImportResolver, IsResolvedAfterResolve) {
  ImportResolver resolver;
  resolver.resolve("./foo.fal");
  EXPECT_TRUE(resolver.is_resolved("./foo.fal"));
}

#include "falcon-lsp/SemanticAnalyzer.hpp"

TEST(SemanticAnalyzer, UndefinedCall) {
  FalconDocumentParser parser;
  const std::string src = R"(
autotuner T () -> () {
  start -> s;
  state s {
    TotallyUnknownFn(1, 2, 3);
    terminal;
  }
}
)";
  auto doc = parser.parse("file:///t.fal", src);
  ASSERT_NE(doc.program, nullptr);

  SemanticAnalyzer sa;
  auto diags = sa.analyze(doc);
  bool found = false;
  for (const auto &d : diags) {
    if (d.message.find("Undefined") != std::string::npos &&
        d.message.find("TotallyUnknownFn") != std::string::npos) {
      found = true;
    }
  }
  EXPECT_TRUE(found) << "Expected 'Undefined: TotallyUnknownFn' diagnostic";
}
