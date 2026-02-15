#include "falcon-autotuner/FalconParser.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::dsl;

class FullCompilationTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_dir_ = std::filesystem::temp_directory_path() / "falcon_test";
    std::filesystem::create_directories(test_dir_);
  }

  void TearDown() override { std::filesystem::remove_all(test_dir_); }

  void write_fal_file(const std::string &filename, const std::string &content) {
    std::ofstream file(test_dir_ / filename);
    file << content;
    file.close();
  }

  std::filesystem::path test_dir_;
};

TEST_F(FullCompilationTest, CompileSimpleAutotuner) {
  std::string source = R"(
        autotuner SimpleTest {
            requires: [];
            
            params {
                int counter = 0;
            }
            
            start -> init;
            
            state init {
                temp {
                    bool success;
                }
                
                measurement: initialize();
                
                if (success == true) -> done;
                else -> error;
            }
            
            state done {
                terminal;
            }
            
            state error {
                terminal;
            }
        }
    )";

  write_fal_file("simple.fal", source);

  // Compile
  FalconCompiler::CompilerOptions options;
  options.output_dir = test_dir_ / "generated";

  FalconCompiler compiler(options);

  ASSERT_NO_THROW(compiler.compile_file(test_dir_ / "simple.fal"));

  // Check generated files exist
  EXPECT_TRUE(std::filesystem::exists(test_dir_ / "generated" /
                                      "GeneratedAutotuners.hpp"));
  EXPECT_TRUE(std::filesystem::exists(test_dir_ / "generated" /
                                      "GeneratedAutotuners.cpp"));

  // Check required implementations
  auto required = compiler.get_required_implementations();
  EXPECT_EQ(required.size(), 1);
  EXPECT_EQ(required[0], "SimpleTest::initialize");
}

TEST_F(FullCompilationTest, CompileMultipleAutotuners) {
  std::string source = R"(
        autotuner First {
            start -> init;
            state init {
                measurement: func1();
                terminal;
            }
        }
        
        autotuner Second {
            start -> init;
            state init {
                measurement: func2();
                terminal;
            }
        }
    )";

  write_fal_file("multi.fal", source);

  FalconCompiler::CompilerOptions options;
  options.output_dir = test_dir_ / "generated";

  FalconCompiler compiler(options);
  compiler.compile_file(test_dir_ / "multi.fal");

  auto required = compiler.get_required_implementations();
  EXPECT_EQ(required.size(), 2);
}

TEST_F(FullCompilationTest, InvalidSyntax) {
  std::string source = R"(
        autotuner Broken {
            // Missing closing brace
            start -> init
        }
    )";

  write_fal_file("broken.fal", source);

  FalconCompiler::CompilerOptions options;
  options.output_dir = test_dir_ / "generated";

  FalconCompiler compiler(options);

  EXPECT_THROW(compiler.compile_file(test_dir_ / "broken.fal"),
               std::runtime_error);
}
