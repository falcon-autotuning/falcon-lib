#include "DatabaseTestFixture.hpp"
#include <cstdlib>
namespace falcon::database::test {
void DatabaseTestFixture::SetUp() {
  // Use environment variable or default test database
  const char *db_conn = std::getenv("TEST_DATABASE_URL");
  std::string conn_string;
  if (db_conn != nullptr) {
    conn_string = db_conn;
  } else {
    // Default test database connection
    conn_string = "postgresql://localhost/falcon_test";
  }
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
