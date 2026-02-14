find_package(GTest REQUIRED)
include(GoogleTest)

# Test utilities
add_library(falcon-autotuner-test-utils STATIC
  test_utils/TestMeasurements.cpp
)

target_link_libraries(falcon-autotuner-test-utils
  PUBLIC
    falcon-autotuner-core
    GTest::gtest
)

target_include_directories(falcon-autotuner-test-utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils
)

# Unit tests
add_executable(falcon_autotuner_unit_tests
  unit/test_parameter_map.cpp
  unit/test_state.cpp
  unit/test_state_iterators.cpp
  unit/test_measurement_routine.cpp
)

target_link_libraries(falcon_autotuner_unit_tests
  PRIVATE
    falcon-autotuner-core
    falcon-autotuner-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_autotuner_unit_tests COMMAND falcon_autotuner_unit_tests)
else()
  gtest_discover_tests(falcon_autotuner_unit_tests)
endif()

# Integration tests
add_executable(falcon_autotuner_integration_tests
  integration/test_autotuner_execution.cpp
  integration/test_nested_autotuners.cpp
  integration/test_graph_analyzer.cpp
  integration/test_plugin_loader.cpp
)

target_link_libraries(falcon_autotuner_integration_tests
  PRIVATE
    falcon-autotuner-core
    falcon-autotuner-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_autotuner_integration_tests COMMAND falcon_autotuner_integration_tests)
else()
  gtest_discover_tests(falcon_autotuner_integration_tests)
endif()
