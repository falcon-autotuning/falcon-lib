#pragma once

#include "falcon-database/DatabaseConnection.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace falcon::database::test {

class DatabaseTestFixture : public ::testing::Test {
protected:
  void SetUp() override;
  void TearDown() override;
  std::shared_ptr<DatabaseConnection> db_;
  std::string test_db_name_;
};

} // namespace falcon::database::test
