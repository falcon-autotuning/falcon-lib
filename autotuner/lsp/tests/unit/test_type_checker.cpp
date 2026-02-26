#include "falcon-lsp/FalconDocument.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

static const std::string CALC_SRC = R"(
autotuner Calc (int a, int b) -> (int sum, int product) {
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

TEST(TypeChecker, BuildsSymbolTable) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", CALC_SRC);
  ASSERT_NE(doc.program, nullptr);
  EXPECT_FALSE(doc.symbols.empty());

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "Calc" && s.kind == "autotuner") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(TypeChecker, InputParamHasType) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", CALC_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "a" && s.kind == "input_param") {
      EXPECT_EQ(s.type_str, "int");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(TypeChecker, OutputParamHasType) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", CALC_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "sum" && s.kind == "output_param") {
      EXPECT_EQ(s.type_str, "int");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(TypeChecker, StateSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", CALC_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "calculate" && s.kind == "state") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}
