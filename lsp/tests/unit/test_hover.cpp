#include "FalconDocument.hpp"
#include "HoverProvider.hpp"
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
    // "myvar" starts at line 0, col 20 (0-indexed) in "autotuner Calc (int myvar)..."
    lsp::Position pos{0, 20};
    auto result = hp.hover(doc, pos);
    ASSERT_TRUE(result.has_value());
    // Check content contains type info
    auto& mc = std::get<lsp::MarkupContent>(result->contents);
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

TEST(HoverProvider, HoverOnBuiltin) {
    FalconDocumentParser parser;
    const std::string src = "autotuner T () -> () { start -> s; state s { terminal; } }";
    auto doc = parser.parse("file:///t.fal", src);

    HoverProvider hp;
    // Manually check hover on "logInfo" by looking up the word directly
    // We create a doc with the word in text
    FalconDocument fdoc;
    fdoc.text = "logInfo(\"hello\")";
    fdoc.uri = "file:///t.fal";

    lsp::Position pos{0, 3};
    auto result = hp.hover(fdoc, pos);
    ASSERT_TRUE(result.has_value());
    auto& mc = std::get<lsp::MarkupContent>(result->contents);
    EXPECT_NE(mc.value.find("builtin"), std::string::npos);
}
