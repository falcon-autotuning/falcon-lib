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

// ---- foreign_function ----
// These tests verify that the LSP parser (which reuses the bison grammar)
// correctly handles ffimport, struct declarations, and module-qualified names
// without emitting any parse errors.  They point at the same .fal files used
// by the runtime integration tests in autotuner/tests/test-autotuners/.

TEST(FalFilesIntegration, ForeignFunctionQuantityStruct) {
  auto doc = parse_file("foreign_function/quantity-struct.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty())
      << "Parse errors in quantity-struct.fal: "
      << (doc.parse_errors.empty() ? "" : doc.parse_errors[0].message);

  // The file declares a struct named "Quantity" — verify the LSP indexes it.
  bool found_quantity = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "Quantity")
      found_quantity = true;
  }
  EXPECT_TRUE(found_quantity) << "Expected symbol 'Quantity' in document index";
}

TEST(FalFilesIntegration, ForeignFunctionConnectionBinding) {
  auto doc = parse_file("foreign_function/connection-binding.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty())
      << "Parse errors in connection-binding.fal: "
      << (doc.parse_errors.empty() ? "" : doc.parse_errors[0].message);

  // The file declares a struct named "Connection".
  bool found_connection = false;
  for (const auto &s : doc.symbols) {
    if (s.name == "Connection")
      found_connection = true;
  }
  EXPECT_TRUE(found_connection)
      << "Expected symbol 'Connection' in document index";
}

TEST(FalFilesIntegration, ForeignFunctionRoutine) {
  // routine.fal uses only plain FFI routine declarations (no struct), which
  // the runtime already passes.  Verify the LSP parses it cleanly too.
  auto doc = parse_file("foreign_function/routine.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty())
      << "Parse errors in routine.fal: "
      << (doc.parse_errors.empty() ? "" : doc.parse_errors[0].message);
}

// ---- type coercion ----

TEST(FalFilesIntegration, TypeCoercion) {
  auto doc = parse_file("expressions/coercion-test.fal");
  EXPECT_NE(doc.program, nullptr);
  EXPECT_TRUE(doc.parse_errors.empty())
      << "Parse errors in coercion-test.fal: "
      << (doc.parse_errors.empty() ? "" : doc.parse_errors[0].message);
}
