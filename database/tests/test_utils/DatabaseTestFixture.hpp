#pragma once

#include "falcon-database/DeviceCharacteristic.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace falcon {
namespace database {
namespace test {

class DatabaseTestFixture : public ::testing::Test {
protected:
  void SetUp() override;
  void TearDown() override;

  std::unique_ptr<DatabaseConnection> db_;
  std::string test_db_name_;
};

} // namespace test
} // namespace database
} // namespace falcon
