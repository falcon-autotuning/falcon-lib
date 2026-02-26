#include "falcon-lsp/FalconDocument.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using namespace falcon::lsp;

// TEST_AUTOTUNERS_DIR is set by CMakeLists.txt via configure_file /
// add_definitions
#ifndef TEST_AUTOTUNERS_DIR
#define TEST_AUTOTUNERS_DIR "test-autotuners"
#endif

static FalconDocument parse_file(const std::string &rel_path) {
  namespace fs = std::filesystem;
  std::string path = std::string(TEST_AUTOTUNERS_DIR) + "/" + rel_path;
  EXPECT_TRUE(fs::exists(path)) << "Test file not found: " << path;

  std::ifstream f(path);
  std::string src((std::istreambuf_iterator<char>(f)),
                  std::istreambuf_iterator<char>());

  FalconDocumentParser parser;
  return parser.parse("file://" + path, src);
}

// ---- basic_features ----

TEST(FalFilesIntegration, Calculator) {
  auto doc = parse_file("basic_features/calculator.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());

  bool found_sum = false;
  bool found_product = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "sum")
      found_sum = true;
    if (s.name == "product")
      found_product = true;
  }
  EXPECT_TRUE(found_sum);
  EXPECT_TRUE(found_product);
}

TEST(FalFilesIntegration, SimpleTransition) {
  auto doc = parse_file("basic_features/simple-transition.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, TempVars) {
  auto doc = parse_file("basic_features/temp-vars.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, ConditionalBranch) {
  auto doc = parse_file("basic_features/conditional-branch.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, Terminal) {
  auto doc = parse_file("basic_features/terminal.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, NoTransition) {
  auto doc = parse_file("basic_features/no-transition.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

// ---- control_flow ----

TEST(FalFilesIntegration, SimpleSweep) {
  auto doc = parse_file("control_flow/simple-sweep.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, ConditionalChain) {
  auto doc = parse_file("control_flow/conditional-chain.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, SequentialTest) {
  auto doc = parse_file("control_flow/sequential-test.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, IterationTest) {
  auto doc = parse_file("control_flow/iteration-test.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}

TEST(FalFilesIntegration, MultiStageWorkflow) {
  auto doc = parse_file("control_flow/multi-stage-workflow.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty());
}
