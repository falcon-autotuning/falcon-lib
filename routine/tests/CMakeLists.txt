find_package(GTest REQUIRED)
include(GoogleTest)

# Test utilities
add_library(falcon-routine-test-utils STATIC
  test_utils/TestFixture.hpp
)

target_link_libraries(falcon-routine-test-utils
  PUBLIC
    falcon-routine
    GTest::gtest
)

target_include_directories(falcon-routine-test-utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils
)

set_target_properties(falcon-routine-test-utils PROPERTIES
  LINKER_LANGUAGE CXX
)

# Unit tests
add_executable(falcon_routine_unit_tests
  unit/test_database.cpp
  unit/test_hub.cpp
  unit/test_log.cpp
)

target_link_libraries(falcon_routine_unit_tests
  PRIVATE
    falcon-routine
    falcon-routine-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_routine_unit_tests COMMAND falcon_routine_unit_tests)
else()
  gtest_discover_tests(falcon_routine_unit_tests)
endif()

# Integration tests
add_executable(falcon_routine_integration_tests
  integration/test_full_workflow.cpp
)

target_link_libraries(falcon_routine_integration_tests
  PRIVATE
    falcon-routine
    falcon-routine-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_routine_integration_tests COMMAND falcon_routine_integration_tests)
else()
  gtest_discover_tests(falcon_routine_integration_tests)
endif()
