#include "falcon-lsp/DiagnosticsProvider.hpp"
#include "falcon-lsp/FalconDocument.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

// Helper: parse src and run full diagnostics (parse + semantic).
static std::vector<::lsp::Diagnostic> diags_for(const std::string &src) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///test.fal", src);
  DiagnosticsProvider dp;
  return dp.diagnostics(doc);
}

TEST(SemanticDiagnostics, CorrectArgCountUserAutotuner_NoDiag) {
  // Calc takes 2 inputs; calling with 2 is fine.
  const std::string src = R"(
autotuner Calc (int a, int b) -> (int sum) {
  sum = a + b;
  start -> s;
  state s { terminal; }
}
autotuner Driver () -> () {
  start -> run;
  state run {
    int result = 0;
    result = Calc(1, 2);
    terminal;
  }
}
)";
  // Parse errors might exist because we don't handle tuple return assignment
  // here; we care only that NO arity diagnostic is emitted for Calc.
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///t.fal", src);
  DiagnosticsProvider dp;
  auto d = dp.diagnostics(doc);
  for (const auto &diag : d) {
    if (diag.message.find("Calc") != std::string::npos &&
        diag.message.find("argument") != std::string::npos) {
      FAIL() << "Unexpected arity error for Calc: " << diag.message;
    }
  }
}

TEST(SemanticDiagnostics, WrongArgCountUserAutotuner_Error) {
  const std::string src = R"(
autotuner Calc (int a, int b) -> (int sum) {
  sum = a + b;
  start -> s;
  state s { terminal; }
}
autotuner Driver () -> () {
  start -> run;
  state run {
    int result = 0;
    result = Calc(1);
    terminal;
  }
}
)";
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///t.fal", src);
  DiagnosticsProvider dp;
  auto d = dp.diagnostics(doc);
  bool found = false;
  for (const auto &diag : d) {
    if (diag.message.find("Calc") != std::string::npos &&
        diag.message.find("2") != std::string::npos) {
      found = true;
    }
  }
  EXPECT_TRUE(found) << "Expected arity error for Calc called with 1 arg";
}

// --------------------------------------------------------------------------
// Diagnostic positions use expression location (not just statement location)
// --------------------------------------------------------------------------

TEST(SemanticDiagnostics, DiagnosticPointsToCallSite) {
  // logInfo("a", "b") – the call site is on line 5 (1-indexed).
  const std::string src = "autotuner T () -> () {\n"
                          "  start -> s;\n"
                          "  state s {\n"
                          "    logInfo(\"a\", \"b\");\n" // line 4
                          "    terminal;\n"
                          "  }\n"
                          "}\n";
  auto d = diags_for(src);
  bool found = false;
  for (const auto &diag : d) {
    if (diag.message.find("logInfo") != std::string::npos) {
      // The diagnostic should point to line 3 (0-indexed = line 4, 1-indexed)
      EXPECT_EQ(diag.range.start.line, 3u)
          << "Diagnostic should be on line 4 (0-indexed=3)";
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

// --------------------------------------------------------------------------
// No spurious diagnostics on clean files
// --------------------------------------------------------------------------

TEST(SemanticDiagnostics, CleanFileHasNoSemanticDiags) {
  const std::string src = R"(
autotuner Calculator (int a, int b) -> (int sum, int product) {
  sum = 0;
  product = 0;
  start -> calculate;
  state calculate {
    sum = a + b;
    product = a * b;
    -> done;
  }
  state done {
    terminal;
  }
}
)";
  auto d = diags_for(src);
  EXPECT_TRUE(d.empty()) << "Expected no diagnostics; got: " << d[0].message;
}
