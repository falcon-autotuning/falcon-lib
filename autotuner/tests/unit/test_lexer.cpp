#include "falcon-autotuner//FalconParser.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner::dsl;

TEST(LexerTest, Keywords) {
  std::string source = "autotuner state params if else";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 6); // 5 keywords + EOF
  EXPECT_EQ(tokens[0].type, TokenType::Autotuner);
  EXPECT_EQ(tokens[1].type, TokenType::State);
  EXPECT_EQ(tokens[2].type, TokenType::Params);
  EXPECT_EQ(tokens[3].type, TokenType::If);
  EXPECT_EQ(tokens[4].type, TokenType::Else);
}

TEST(LexerTest, Identifiers) {
  std::string source = "my_autotuner state1 _var";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 4); // 3 identifiers + EOF
  EXPECT_EQ(tokens[0].type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].value, "my_autotuner");
  EXPECT_EQ(tokens[1].type, TokenType::Identifier);
  EXPECT_EQ(tokens[1].value, "state1");
  EXPECT_EQ(tokens[2].type, TokenType::Identifier);
  EXPECT_EQ(tokens[2].value, "_var");
}

TEST(LexerTest, Numbers) {
  std::string source = "42 3.14 -5 1e-6";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 5); // 4 numbers + EOF
  EXPECT_EQ(tokens[0].type, TokenType::IntLiteral);
  EXPECT_EQ(tokens[0].value, "42");
  EXPECT_EQ(tokens[1].type, TokenType::FloatLiteral);
  EXPECT_EQ(tokens[1].value, "3.14");
  EXPECT_EQ(tokens[2].type, TokenType::IntLiteral);
  EXPECT_EQ(tokens[2].value, "-5");
  EXPECT_EQ(tokens[3].type, TokenType::FloatLiteral);
  EXPECT_EQ(tokens[3].value, "1e-6");
}

TEST(LexerTest, Strings) {
  std::string source = R"("hello" "world")";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 3); // 2 strings + EOF
  EXPECT_EQ(tokens[0].type, TokenType::StringLiteral);
  EXPECT_EQ(tokens[0].value, "hello");
  EXPECT_EQ(tokens[1].type, TokenType::StringLiteral);
  EXPECT_EQ(tokens[1].value, "world");
}

TEST(LexerTest, Operators) {
  std::string source = "-> == != < > <= >= && ||";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 10); // 9 operators + EOF
  EXPECT_EQ(tokens[0].type, TokenType::Arrow);
  EXPECT_EQ(tokens[1].type, TokenType::Equal);
  EXPECT_EQ(tokens[2].type, TokenType::NotEqual);
  EXPECT_EQ(tokens[3].type, TokenType::Less);
  EXPECT_EQ(tokens[4].type, TokenType::Greater);
  EXPECT_EQ(tokens[5].type, TokenType::LessEqual);
  EXPECT_EQ(tokens[6].type, TokenType::GreaterEqual);
  EXPECT_EQ(tokens[7].type, TokenType::And);
  EXPECT_EQ(tokens[8].type, TokenType::Or);
}

TEST(LexerTest, Punctuation) {
  std::string source = "{ } ( ) [ ] ; , : .";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  ASSERT_EQ(tokens.size(), 11); // 10 punctuation + EOF
  EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
  EXPECT_EQ(tokens[1].type, TokenType::RightBrace);
  EXPECT_EQ(tokens[2].type, TokenType::LeftParen);
  EXPECT_EQ(tokens[3].type, TokenType::RightParen);
  EXPECT_EQ(tokens[4].type, TokenType::LeftBracket);
  EXPECT_EQ(tokens[5].type, TokenType::RightBracket);
  EXPECT_EQ(tokens[6].type, TokenType::Semicolon);
  EXPECT_EQ(tokens[7].type, TokenType::Comma);
  EXPECT_EQ(tokens[8].type, TokenType::Colon);
  EXPECT_EQ(tokens[9].type, TokenType::Dot);
}

TEST(LexerTest, Comments) {
  std::string source = R"(
        // This is a comment
        autotuner Test {
            // Another comment
        }
    )";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  // Comments should be skipped
  bool has_comment = false;
  for (const auto &token : tokens) {
    if (token.value.find("comment") != std::string::npos) {
      has_comment = true;
    }
  }
  EXPECT_FALSE(has_comment);
}

TEST(LexerTest, LineNumbers) {
  std::string source = "line1\nline2\nline3";
  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  EXPECT_EQ(tokens[0].line, 1);
  EXPECT_EQ(tokens[1].line, 2);
  EXPECT_EQ(tokens[2].line, 3);
}
