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
