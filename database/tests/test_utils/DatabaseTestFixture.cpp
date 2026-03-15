#include "DatabaseTestFixture.hpp"
#include <cstdlib>

namespace falcon::database::test {

void DatabaseTestFixture::SetUp() {
  // Use TEST_DATABASE_URL environment variable for tests
  // This takes precedence over FALCON_DATABASE_URL
  const char *test_db_url = std::getenv("TEST_DATABASE_URL");
  std::string conn_string;

  if (test_db_url != nullptr && strlen(test_db_url) > 0) {
    conn_string = test_db_url;
  } else {
    // Fallback to default test database
    conn_string = "postgresql://127.0.0.1/falcon_test";
  }

  // Create admin connection with explicit connection string
  // This bypasses FALCON_DATABASE_URL to ensure tests use test database
  db_ = std::make_shared<AdminDatabaseConnection>(conn_string);
  db_->initialize_schema();
  db_->clear_all(); // Clean slate for each test
}

void DatabaseTestFixture::TearDown() {
  if (db_) {
    db_->clear_all();
  }
}

} // namespace falcon::database::test
