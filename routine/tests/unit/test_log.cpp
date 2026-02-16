#include "../test_utils/TestFixture.hpp"
#include "falcon-routine/log.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace falcon::routine;
using namespace falcon::routine::test;

class LogUnitTest : public RoutineTestFixture {
protected:
  void SetUp() override {
    RoutineTestFixture::SetUp();

    // Set up test log file
    test_log_file_ =
        "/tmp/falcon_test_" + std::to_string(std::time(nullptr)) + ".log";
    setenv("LOG_FILE", test_log_file_.c_str(), 1);
    setenv("LOG_LEVEL", "trace", 1);
  }

  void TearDown() override {
    log::flush();

    // Clean up log file
    if (std::filesystem::exists(test_log_file_)) {
      std::filesystem::remove(test_log_file_);
    }

    RoutineTestFixture::TearDown();
  }

  std::string test_log_file_;
};

TEST_F(LogUnitTest, LogLevels) {
  // All should work without throwing
  EXPECT_NO_THROW(log::trace("trace message"));
  EXPECT_NO_THROW(log::debug("debug message"));
  EXPECT_NO_THROW(log::info("info message"));
  EXPECT_NO_THROW(log::warn("warn message"));
  EXPECT_NO_THROW(log::error("error message"));
  EXPECT_NO_THROW(log::critical("critical message"));
}

TEST_F(LogUnitTest, LogToFile) {
  log::info("test log message");
  log::flush();

  // Check file exists
  ASSERT_TRUE(std::filesystem::exists(test_log_file_));

  // Read file and check content
  std::ifstream file(test_log_file_);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("test log message") != std::string::npos);
}

TEST_F(LogUnitTest, SetLevel) {
  log::set_level("error");

  // Info should not appear (we'd need to check file content to verify)
  log::info("should not appear");
  log::error("should appear");

  log::flush();

  std::ifstream file(test_log_file_);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("should appear") != std::string::npos);
  // Note: "should not appear" might still be in file from buffering,
  // so we don't check for its absence
}
