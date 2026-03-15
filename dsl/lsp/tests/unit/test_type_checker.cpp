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

// ---------------------------------------------------------------------------
// Struct symbols
// ---------------------------------------------------------------------------

static const std::string STRUCT_SRC = R"(
struct Quantity {
    int a_;
    routine New (int a) -> (Quantity q) {
        q.a_ = a;
    }
    routine Value -> (int value) {
        value = a_;
    }
}
autotuner QTest (int x) -> (int result) {
  result = 0;
  start -> s;
  state s { terminal; }
}
)";

TEST(TypeChecker, StructSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///struct.fal", STRUCT_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "Quantity" && s.kind == "struct") {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(TypeChecker, StructFieldSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///struct.fal", STRUCT_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "a_" && s.kind == "struct_field") {
      EXPECT_EQ(s.type_str, "int");
      EXPECT_EQ(s.autotuner_name, "Quantity");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST(TypeChecker, StructRoutineSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///struct.fal", STRUCT_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "New" && s.kind == "struct_routine") {
      EXPECT_EQ(s.autotuner_name, "Quantity");
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

// ---------------------------------------------------------------------------
// Import symbols
// ---------------------------------------------------------------------------

static const std::string IMPORT_SRC = R"(
import (
"./Adder.fal"
)
autotuner ImportTest () -> () {
  start -> s;
  state s { terminal; }
}
)";

TEST(TypeChecker, ImportSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///import.fal", IMPORT_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found_sym = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "./Adder.fal" && s.kind == "import") {
      found_sym = true;
    }
  }
  EXPECT_TRUE(found_sym);

  bool found_path = false;
  for (const auto &p : doc.import_paths) {
    if (p == "./Adder.fal") {
      found_path = true;
    }
  }
  EXPECT_TRUE(found_path);
}

// ---------------------------------------------------------------------------
// FFImport symbols
// ---------------------------------------------------------------------------

static const std::string FFIMPORT_SRC = R"(
ffimport "QuantityW.cpp" () ()
autotuner FFTest () -> () {
  start -> s;
  state s { terminal; }
}
)";

TEST(TypeChecker, FFImportSymbol) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///ffimport.fal", FFIMPORT_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found_sym = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "QuantityW.cpp" && s.kind == "ffimport") {
      found_sym = true;
    }
  }
  EXPECT_TRUE(found_sym);

  bool found_path = false;
  for (const auto &p : doc.ffimport_paths) {
    if (p == "QuantityW.cpp") {
      found_path = true;
    }
  }
  EXPECT_TRUE(found_path);
}

// ---------------------------------------------------------------------------
// Routine body vars
// ---------------------------------------------------------------------------

static const std::string ROUTINE_SRC = R"(
routine adder (int a, int b) -> (int add) {
    add = a + b;
}
)";

TEST(TypeChecker, RoutineBodyParamsTracked) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///routine.fal", ROUTINE_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found_routine = false;
  bool found_param = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "adder" && s.kind == "routine") {
      found_routine = true;
    }
    if (s.name == "a" && s.kind == "input_param" &&
        s.autotuner_name == "adder") {
      found_param = true;
    }
  }
  EXPECT_TRUE(found_routine);
  EXPECT_TRUE(found_param);
}

static const std::string ROUTINE_WITH_VAR_SRC = R"(
routine compute (int x) -> (int result) {
    int tmp = x;
    result = tmp;
}
)";

TEST(TypeChecker, RoutineBodyVarsTracked) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///routine_var.fal", ROUTINE_WITH_VAR_SRC);
  ASSERT_NE(doc.program, nullptr);

  bool found_var = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "tmp" && s.kind == "var" &&
        s.autotuner_name == "compute") {
      EXPECT_EQ(s.type_str, "int");
      found_var = true;
    }
  }
  EXPECT_TRUE(found_var);
}
