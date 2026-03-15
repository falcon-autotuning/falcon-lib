#include "falcon-lsp/FalconDocument.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

TEST(FalconDocument, ParseSimpleAutotuner) {
  FalconDocumentParser parser;
  const std::string src = R"(
autotuner Calculator (int a, int b) -> (int sum) {
  sum = 0;
  start -> calc;
  state calc {
    sum = a + b;
    terminal;
  }
}
)";
  auto doc = parser.parse("file:///test.fal", src);
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalconDocument, ParseErrorsCollected) {
  FalconDocumentParser parser;
  const std::string src = "autotuner Broken {{{";
  auto doc = parser.parse("file:///broken.fal", src);
  EXPECT_FALSE(doc.parse_errors.empty());
}

TEST(FalconDocument, SymbolsBuiltFromAST) {
  FalconDocumentParser parser;
  const std::string src = R"(
autotuner Calc (int a) -> (int result) {
  result = 0;
  start -> run;
  state run {
    result = a;
    terminal;
  }
}
)";
  auto doc = parser.parse("file:///calc.fal", src);
  ASSERT_NE(doc.program, nullptr);

  bool found_autotuner = false;
  bool found_param = false;
  for (const auto &sym : doc.symbols) {
    if (sym.name == "Calc" && sym.kind == "autotuner")
      found_autotuner = true;
    if (sym.name == "a" && sym.kind == "input_param")
      found_param = true;
  }
  EXPECT_TRUE(found_autotuner);
  EXPECT_TRUE(found_param);
}

TEST(FalconDocument, ParseRoutine) {
  FalconDocumentParser parser;
  // routines must follow autotuner declarations; empty output is `-> ` (no
  // parens)
  const std::string src = R"(
autotuner T (int x) -> (int y) {
  y = x;
  start -> s;
  state s { terminal; }
}
routine Adder (int a, int b) -> (int sum, Error err)
)";
  auto doc = parser.parse("file:///routine.fal", src);
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
  bool found_routine = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "Adder" && s.kind == "routine") {
      found_routine = true;
    }
  }
  EXPECT_TRUE(found_routine);
}
