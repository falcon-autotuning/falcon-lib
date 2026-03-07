#include "falcon-lsp/FalconDocument.hpp"
#include "falcon-lsp/HoverProvider.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

static const std::string SRC = R"(autotuner Calc (int myvar) -> (int result) {
  result = 0;
  start -> run;
  state run {
    result = myvar;
    terminal;
  }
}
)";

TEST(HoverProvider, HoverOnKnownParam) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", SRC);
  ASSERT_NE(doc.program, nullptr);

  HoverProvider hp;
  // "myvar" starts at line 0, col 20 (0-indexed) in "autotuner Calc (int
  // myvar)..."
  lsp::Position pos{0, 20};
  auto result = hp.hover(doc, pos);
  ASSERT_TRUE(result.has_value());
  // Check content contains type info
  auto &mc = std::get<lsp::MarkupContent>(result->contents);
  EXPECT_NE(mc.value.find("int"), std::string::npos);
}

TEST(HoverProvider, HoverOnUnknownWord) {
  FalconDocumentParser parser;
  auto doc = parser.parse("file:///calc.fal", SRC);

  HoverProvider hp;
  lsp::Position pos{0, 0}; // 'a' in 'autotuner'
  auto result = hp.hover(doc, pos);
  // autotuner is not in symbol table but doesn't crash
  (void)result;
}

TEST(HoverProvider, AutotunerSignatureInHover) {
  FalconDocumentParser parser;
  const std::string src = R"(
autotuner Calc (int a, int b) -> (int result) {
  result = 0;
  start -> s;
  state s { terminal; }
}
)";
  auto doc = parser.parse("file:///calc.fal", src);
  ASSERT_NE(doc.program, nullptr);

  HoverProvider hp;
  // Hover over "Calc" on line 1 (0-indexed), col 10
  lsp::Position pos{1, 10};
  auto result = hp.hover(doc, pos);
  ASSERT_TRUE(result.has_value());
  auto &mc = std::get<lsp::MarkupContent>(result->contents);
  // Should contain parameter info
  EXPECT_NE(mc.value.find("int a"), std::string::npos)
      << "Hover should show input params. Got: " << mc.value;
}

TEST(HoverProvider, ImportedSymbolNote) {
  // Manually inject a from_import symbol into a document
  FalconDocument doc;
  doc.text = "Adder(1, 2)";
  doc.uri = "file:///test.fal";

  Symbol s;
  s.name = "Adder";
  s.kind = "autotuner";
  s.type_str = "autotuner";
  s.from_import = true;
  s.param_types = {"int a", "int b"};
  s.return_types = {"int result"};
  doc.symbols.push_back(std::move(s));

  HoverProvider hp;
  lsp::Position pos{0, 3}; // over "Adder"
  auto result = hp.hover(doc, pos);
  ASSERT_TRUE(result.has_value());
  auto &mc = std::get<lsp::MarkupContent>(result->contents);
  EXPECT_NE(mc.value.find("from import"), std::string::npos)
      << "Hover for imported symbol should mention 'from import'. Got: "
      << mc.value;
}
