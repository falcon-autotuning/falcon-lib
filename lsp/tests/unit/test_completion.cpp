#include "CompletionProvider.hpp"
#include "FalconDocument.hpp"
#include <gtest/gtest.h>

using namespace falcon::lsp;

TEST(CompletionProvider, ContainsKeywords) {
    FalconDocumentParser parser;
    const std::string src = "autotuner T () -> () { start -> s; state s { terminal; } }";
    auto doc = parser.parse("file:///t.fal", src);

    CompletionProvider cp;
    lsp::Position pos{0, 0};
    auto items = cp.complete(doc, pos);
    EXPECT_FALSE(items.empty());

    bool found_autotuner = false;
    bool found_state = false;
    for (const auto& item : items) {
        if (item.label == "autotuner") found_autotuner = true;
        if (item.label == "state") found_state = true;
    }
    EXPECT_TRUE(found_autotuner);
    EXPECT_TRUE(found_state);
}

TEST(CompletionProvider, ContainsDocumentSymbols) {
    FalconDocumentParser parser;
    const std::string src = R"(
autotuner MyAT (int myInput) -> (int myOutput) {
  myOutput = 0;
  start -> s;
  state s { terminal; }
}
)";
    auto doc = parser.parse("file:///t.fal", src);
    ASSERT_NE(doc.program, nullptr);

    CompletionProvider cp;
    lsp::Position pos{0, 0};
    auto items = cp.complete(doc, pos);

    bool found = false;
    for (const auto& item : items) {
        if (item.label == "myInput") { found = true; break; }
    }
    EXPECT_TRUE(found);
}

TEST(CompletionProvider, ContainsBuiltins) {
    FalconDocumentParser parser;
    const std::string src = "autotuner T () -> () { start -> s; state s { terminal; } }";
    auto doc = parser.parse("file:///t.fal", src);

    CompletionProvider cp;
    lsp::Position pos{0, 0};
    auto items = cp.complete(doc, pos);

    bool found = false;
    for (const auto& item : items) {
        if (item.label == "logInfo") { found = true; break; }
    }
    EXPECT_TRUE(found);
}
