find_package(GTest REQUIRED)
include(GoogleTest)

# Test utilities
add_library(falcon-db-test-utils STATIC
  test_utils/DatabaseTestFixture.cpp
)

target_link_libraries(falcon-db-test-utils
  PUBLIC
    falcon-database-core
    GTest::gtest
)

target_include_directories(falcon-db-test-utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils
)

# Unit tests
add_executable(falcon_db_unit_tests
  unit/test_device_characteristic.cpp
  unit/test_json_primitive.cpp
  unit/test_database_operations.cpp
)

target_link_libraries(falcon_db_unit_tests
  PRIVATE
    falcon-database-core
    falcon-db-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_db_unit_tests COMMAND falcon_db_unit_tests)
else()
  gtest_discover_tests(falcon_db_unit_tests)
endif()

# Integration tests
add_executable(falcon_db_integration_tests
  integration/test_snapshot_manager.cpp
  integration/test_full_workflow.cpp
)

target_link_libraries(falcon_db_integration_tests
  PRIVATE
    falcon-database-core
    falcon-db-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_db_integration_tests COMMAND falcon_db_integration_tests)
else()
  gtest_discover_tests(falcon_db_integration_tests)
endif()
