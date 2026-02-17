// autotuner/tests/test_runner.cpp
#include "falcon-atc/compiler.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon_core/physics/config/core/Config.hpp"
#include "test_utils.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

// Base test case for DSL tests
class DSLTestCase : public TestCase {
public:
  DSLTestCase(std::string name, std::string fal_file,
              std::string autotuner_name, ParameterMap initial_params,
              std::function<bool(const ParameterMap &)> validator)
      : TestCase(std::move(name), std::move(fal_file)),
        autotuner_name_(std::move(autotuner_name)),
        initial_params_(std::move(initial_params)),
        validator_(std::move(validator)) {}

  TestResult run() override {
    auto start = std::chrono::high_resolution_clock::now();
    TestResult result;
    result.test_name = name_;
    result.passed = false;

    try {
      // Parse the file
      falcon::atc::Compiler compiler;
      auto program = compiler.parse_file(fal_file_);

      if (!program) {
        result.message = "Failed to parse " + fal_file_;
        auto end = std::chrono::high_resolution_clock::now();
        result.duration_ms =
            std::chrono::duration<double, std::milli>(end - start).count();
        return result;
      }

      // Create config and interpreter
      falcon_core::physics::config::core::Config config;
      Interpreter interpreter(*program, config, ""); // No NATS for tests

      // Run the autotuner
      ParameterMap params = initial_params_;
      bool success = interpreter.run(autotuner_name_, params);

      if (!success) {
        result.message = "Interpreter execution failed";
        auto end = std::chrono::high_resolution_clock::now();
        result.duration_ms =
            std::chrono::duration<double, std::milli>(end - start).count();
        return result;
      }

      // Validate results
      if (validator_) {
        if (!validator_(params)) {
          result.message = "Validation failed for output parameters";
          auto end = std::chrono::high_resolution_clock::now();
          result.duration_ms =
              std::chrono::duration<double, std::milli>(end - start).count();
          return result;
        }
      }

      result.passed = true;
      result.message = "Success";

    } catch (const std::exception &e) {
      result.message = std::string("Exception: ") + e.what();
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    return result;
  }

private:
  std::string autotuner_name_;
  ParameterMap initial_params_;
  std::function<bool(const ParameterMap &)> validator_;
};

// Parse-only test case
class ParseTestCase : public TestCase {
public:
  ParseTestCase(std::string name, std::string fal_file,
                bool should_succeed = true)
      : TestCase(std::move(name), std::move(fal_file)),
        should_succeed_(should_succeed) {}

  TestResult run() override {
    auto start = std::chrono::high_resolution_clock::now();
    TestResult result;
    result.test_name = name_;
    result.passed = false;

    try {
      falcon::atc::Compiler compiler;
      auto program = compiler.parse_file(fal_file_);

      bool parse_succeeded = (program != nullptr);

      if (should_succeed_) {
        result.passed = parse_succeeded;
        result.message =
            parse_succeeded ? "Parsed successfully" : "Parse failed";
      } else {
        result.passed = !parse_succeeded;
        result.message = parse_succeeded ? "Should have failed to parse"
                                         : "Failed as expected";
      }

    } catch (const std::exception &e) {
      if (should_succeed_) {
        result.message = std::string("Unexpected exception: ") + e.what();
        result.passed = false;
      } else {
        result.message = "Failed as expected (exception)";
        result.passed = true;
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    return result;
  }

private:
  bool should_succeed_;
};

void add_basic_tests(TestSuite &suite, const std::string &base_path) {
  // Simple transition test
  {
    ParameterMap params;
    auto validator = [](const ParameterMap &p) {
      return true; // Just check it completes
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Basic: Simple Transition", base_path + "/basic/simple_transition.fal",
        "SimpleTransition", params, validator));
  }

  // Conditional branching test
  {
    ParameterMap params;
    params.set("value", 50);

    auto validator = [](const ParameterMap &p) {
      return p.has("result") && p.get<std::string>("result") == "high";
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Basic: Conditional Branching (value=50)",
        base_path + "/basic/conditional_branching.fal", "ConditionalBranch",
        params, validator));
  }

  // Parameter assignment test
  {
    ParameterMap params;
    params.set("a", static_cast<int64_t>(10));
    params.set("b", static_cast<int64_t>(20));

    auto validator = [](const ParameterMap &p) {
      return p.has("sum") && p.get<int64_t>("sum") == 30;
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Basic: Parameter Assignment",
        base_path + "/basic/parameter_assignment.fal", "ParameterAssignment",
        params, validator));
  }

  // Terminal state test
  {
    ParameterMap params;

    suite.add_test(std::make_unique<DSLTestCase>(
        "Basic: Terminal State", base_path + "/basic/terminal_state.fal",
        "TerminalTest", params, nullptr));
  }
}

void add_expression_tests(TestSuite &suite, const std::string &base_path) {
  // Arithmetic test
  {
    ParameterMap params;
    params.set("x", 10.0);
    params.set("y", 3.0);

    auto validator = [](const ParameterMap &p) {
      if (!p.has("add") || !p.has("sub") || !p.has("mul") || !p.has("div")) {
        return false;
      }
      return std::abs(p.get<double>("add") - 13.0) < 0.001 &&
             std::abs(p.get<double>("sub") - 7.0) < 0.001 &&
             std::abs(p.get<double>("mul") - 30.0) < 0.001 &&
             std::abs(p.get<double>("div") - 3.333) < 0.01;
    };

    suite.add_test(
        std::make_unique<DSLTestCase>("Expression: Arithmetic Operations",
                                      base_path + "/expressions/arithmetic.fal",
                                      "ArithmeticTest", params, validator));
  }

  // Comparison test
  {
    ParameterMap params;
    params.set("a", static_cast<int64_t>(10));
    params.set("b", static_cast<int64_t>(20));

    auto validator = [](const ParameterMap &p) {
      return p.has("is_greater") && p.get<bool>("is_greater") == false &&
             p.has("is_less") && p.get<bool>("is_less") == true;
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Expression: Comparison Operations",
        base_path + "/expressions/comparisons.fal", "ComparisonTest", params,
        validator));
  }

  // Logical operations test
  {
    ParameterMap params;
    params.set("flag1", true);
    params.set("flag2", false);

    auto validator = [](const ParameterMap &p) {
      return p.has("result") && p.get<std::string>("result") == "expected";
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Expression: Logical Operations",
        base_path + "/expressions/logical_ops.fal", "LogicalTest", params,
        validator));
  }
}

void add_error_handling_tests(TestSuite &suite, const std::string &base_path) {
  // Error state test
  {
    ParameterMap params;
    params.set("trigger_error", true);

    // This should fail (reach error state)
    auto validator = [](const ParameterMap &p) {
      // If we reach here, autotuner completed, check for error marker
      return p.has("error");
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Error: Error State Handling",
        base_path + "/error_handling/error_state.fal", "ErrorTest", params,
        validator));
  }

  // Conditional error test
  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(-5));

    auto validator = [](const ParameterMap &p) {
      return p.has("error") && p.get<std::string>("error") == "negative_value";
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Error: Conditional Error",
        base_path + "/error_handling/conditional_error.fal", "ConditionalError",
        params, validator));
  }
}

void add_integration_tests(TestSuite &suite, const std::string &base_path) {
  // Simple sweep test
  {
    ParameterMap params;
    params.set("start", 0.0);
    params.set("end", 1.0);
    params.set("step", 0.1);
    params.set("count", static_cast<int64_t>(0));

    auto validator = [](const ParameterMap &p) {
      return p.has("count") && p.get<int64_t>("count") == 11;
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Integration: Simple Sweep",
        base_path + "/integration/simple_sweep.fal", "SimpleSweep", params,
        validator));
  }

  // Multi-state workflow test
  {
    ParameterMap params;
    params.set("initialized", false);

    auto validator = [](const ParameterMap &p) {
      return p.has("stage") && p.get<std::string>("stage") == "completed";
    };

    suite.add_test(std::make_unique<DSLTestCase>(
        "Integration: Multi-State Workflow",
        base_path + "/integration/multi_state_workflow.fal",
        "MultiStateWorkflow", params, validator));
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <test_cases_directory>\n";
    return 1;
  }

  std::string test_base_path = argv[1];

  TestSuite suite;

  // Add all test categories
  add_basic_tests(suite, test_base_path);
  add_expression_tests(suite, test_base_path);
  add_error_handling_tests(suite, test_base_path);
  add_integration_tests(suite, test_base_path);

  // Run all tests
  auto results = suite.run_all();

  // Print summary
  suite.print_summary(results);

  // Return non-zero if any test failed
  for (const auto &r : results) {
    if (!r.passed) {
      return 1;
    }
  }

  return 0;
}
