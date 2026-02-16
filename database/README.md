# Falcon Database - C++ PostgreSQL Implementation

A modern C++ implementation of the Falcon device characteristics database using libpqxx.

## Features

- **DeviceCharacteristic** storage with extended metadata (gates, scope, uncertainty)
- **CRUD operations**: Insert, query by name/hash, delete, clear
- **Batch operations**: Get many characteristics, hash range queries
- **JSON snapshotting**: Export/import database state for deployment
- **JSON Schema**: Documented snapshot format
- **Comprehensive tests**: Unit and integration tests with Google Test

## Dependencies

- CMake >= 3.20
- C++17 compiler
- PostgreSQL (libpqxx)
- nlohmann_json >= 3.2.0
- Google Test (for tests)

## Building

```bash
# From the falcon repository root
cd database

# First time setup
make deps

# Set up test database
export TEST_DATABASE_URL="postgresql://localhost/falcon_test"

# Development workflow
make dev              # Build debug + run tests

# Release workflow
make release          # Build release + run tests

# Or step by step
make build-release
make test

# Clean rebuild
make clean
make build-release
```

Or run test executables directly:

```bash
./build/tests/falcon_db_unit_tests
./build/tests/falcon_db_integration_tests
```

## Environment Variables

- `FALCON_DATABASE_URL`: PostgreSQL connection string used by default
  - Example: `postgresql://user:password@127.0.0.1:5432/dbname`
  - Can be overridden by passing explicit connection string to constructors
  - **Connection is lazy** - database connection happens on first operation, not at construction

### Connection String Priority

1. **Explicit constructor parameter** (highest priority)
2. **FALCON_DATABASE_URL environment variable**
3. **Error** if neither is provided

### Examples

```cpp
// Use FALCON_DATABASE_URL environment variable
auto db = falcon::database::ReadOnlyDatabaseConnection();

// Override with explicit connection string
auto db = falcon::database::ReadOnlyDatabaseConnection(
    "postgresql://user:pass@host:port/db");

// Connection is lazy - happens on first operation
auto result = db.get_by_name("device1"); // Connection established here
```

### Testing

For tests, use `TEST_DATABASE_URL` environment variable which the test fixtures prioritize:

```bash
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
make test
```
