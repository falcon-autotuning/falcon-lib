// autotuner/tests/test_utils.hpp
#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace falcon::autotuner::test {

struct TestResult {
  std::string test_name;
  bool passed;
  std::string message;
  double duration_ms;
};

class TestCase {
public:
  TestCase(std::string name, std::string fal_file)
      : name_(std::move(name)), fal_file_(std::move(fal_file)) {}

  virtual ~TestCase() = default;

  // Override this to define test behavior
  virtual TestResult run() = 0;

  std::string name() const { return name_; }
  std::string fal_file() const { return fal_file_; }

protected:
  std::string name_;
  std::string fal_file_;
};

class TestSuite {
public:
  void add_test(std::unique_ptr<TestCase> test) {
    tests_.push_back(std::move(test));
  }

  std::vector<TestResult> run_all() {
    std::vector<TestResult> results;

    std::cout << "\n=== Running Test Suite ===\n";
    std::cout << "Total tests: " << tests_.size() << "\n\n";

    for (auto &test : tests_) {
      std::cout << "Running: " << test->name() << "... ";
      std::cout.flush();

      auto result = test->run();
      results.push_back(result);

      if (result.passed) {
        std::cout << "✓ PASS";
      } else {
        std::cout << "✗ FAIL";
      }
      std::cout << " (" << result.duration_ms << " ms)\n";

      if (!result.passed && !result.message.empty()) {
        std::cout << "  → " << result.message << "\n";
      }
    }

    return results;
  }

  void print_summary(const std::vector<TestResult> &results) {
    int passed = 0;
    int failed = 0;
    double total_time = 0.0;

    for (const auto &r : results) {
      if (r.passed)
        passed++;
      else
        failed++;
      total_time += r.duration_ms;
    }

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << passed << "/" << results.size() << "\n";
    std::cout << "Failed: " << failed << "/" << results.size() << "\n";
    std::cout << "Total time: " << total_time << " ms\n";

    if (failed > 0) {
      std::cout << "\nFailed tests:\n";
      for (const auto &r : results) {
        if (!r.passed) {
          std::cout << "  - " << r.test_name << ": " << r.message << "\n";
        }
      }
    }
  }

private:
  std::vector<std::unique_ptr<TestCase>> tests_;
};

} // namespace falcon::autotuner::test
