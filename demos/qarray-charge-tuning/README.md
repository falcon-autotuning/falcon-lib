# QArray Charge Tuning Demo & Tests

A modern C++ demo for QArray charge tuning, using the Falcon device characteristics database (libpqxx backend).

## Features

- **DeviceCharacteristic** storage with extended metadata (gates, scope, uncertainty)
- **CRUD operations**: Insert, query by name/hash, delete, clear
- **Batch operations**: Get many characteristics, hash range queries
- **JSON snapshotting**: Export/import database state for deployment
- **JSON Schema**: Documented snapshot format
- **Charge tuning demo**: Run QArray charge tuning and generate stability diagrams
- **Comprehensive tests**: Unit and integration tests for demo and database

## Dependencies

- CMake >= 3.20
- C++17 compiler
- PostgreSQL (libpqxx)
- nlohmann_json >= 3.2.0
- matplotplusplus (for plotting)
- Google Test (for tests)

## Setup

### 1. Establish the database

Ensure PostgreSQL is running and create the Falcon test database:

```bash
export TEST_DATABASE_URL="postgresql://falcon_test:falcon_test_password@127.0.0.1:5432/falcon_test"
```

### 2. Build the demo

```bash
# From the falcon repository root
cd demos/qarray-charge-tuning

# First time setup
make deps

# Build demo and tests
make dev              # Build debug + run tests
make release          # Build release + run tests

# Or step by step
make build-release
make test

# Clean rebuild
make clean
make build-release
```

### 3. Run the demo

```bash
./build/qarray_charge_tuning_demo
```

### 4. Run tests

```bash
./build/tests/qarray_charge_tuning_tests
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
