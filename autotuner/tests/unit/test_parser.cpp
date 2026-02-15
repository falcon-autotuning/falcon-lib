#include "falcon-autotuner/dsl/FalconParser.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner::dsl;

TEST(ParserTest, EmptyAutotuner) {
  std::string source = R"(
        autotuner Test {
            requires: [];
            start -> init;
            state init {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 1);
  EXPECT_EQ(ast[0]->name, "Test");
}

TEST(ParserTest, Parameters) {
  std::string source = R"(
        autotuner Test {
            params {
                int counter = 0;
                float voltage = 1.5;
                bool success = true;
                string name = "test";
            }
            start -> init;
            state init {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 1);
  ASSERT_EQ(ast[0]->parameters.size(), 4);

  EXPECT_EQ(ast[0]->parameters[0]->name, "counter");
  EXPECT_EQ(ast[0]->parameters[0]->type, "int");

  EXPECT_EQ(ast[0]->parameters[1]->name, "voltage");
  EXPECT_EQ(ast[0]->parameters[1]->type, "float");

  EXPECT_EQ(ast[0]->parameters[2]->name, "success");
  EXPECT_EQ(ast[0]->parameters[2]->type, "bool");

  EXPECT_EQ(ast[0]->parameters[3]->name, "name");
  EXPECT_EQ(ast[0]->parameters[3]->type, "string");
}

TEST(ParserTest, BasicState) {
  std::string source = R"(
        autotuner Test {
            start -> measure;
            state measure {
                measurement: do_measurement();
                -> done;
            }
            state done {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 1);
  EXPECT_EQ(ast[0]->entry_state, "measure");
}

TEST(ParserTest, ConditionalTransition) {
  std::string source = R"(
        autotuner Test {
            start -> check;
            state check {
                measurement: check_value();
                if (value > 10) -> high;
                else -> low;
            }
            state high {
                terminal;
            }
            state low {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 1);
  // Check that states were parsed
  EXPECT_GT(ast[0]->statements.size(), 0);
}

TEST(ParserTest, TempParameters) {
  std::string source = R"(
        autotuner Test {
            start -> measure;
            state measure {
                temp {
                    float result;
                    bool valid;
                }
                measurement: measure_value();
                -> done;
            }
            state done {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 1);
  // Verify parsing succeeded
  EXPECT_EQ(ast[0]->name, "Test");
}

TEST(ParserTest, MultipleAutotuners) {
  std::string source = R"(
        autotuner First {
            start -> init;
            state init {
                terminal;
            }
        }
        
        autotuner Second {
            start -> init;
            state init {
                terminal;
            }
        }
    )";

  FalconLexer lexer(source);
  auto tokens = lexer.tokenize();

  FalconParser parser(std::move(tokens));
  auto ast = parser.parse();

  ASSERT_EQ(ast.size(), 2);
  EXPECT_EQ(ast[0]->name, "First");
  EXPECT_EQ(ast[1]->name, "Second");
}
