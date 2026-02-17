# autotuner/tests/CMakeLists.txt
find_package(GTest CONFIG REQUIRED)

# Unit tests
add_executable(falcon_dsl_unit_tests
  unit/test_parameter_map.cpp
)

target_link_libraries(falcon_dsl_unit_tests
  PRIVATE
    falcon-dsl
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME falcon_dsl_unit_tests
         COMMAND falcon_dsl_unit_tests)

# Integration tests
add_executable(falcon_dsl_integration_tests
  integration/test_basic_features.cpp
  integration/test_expressions.cpp
  integration/test_control_flow.cpp
  integration/test_error_handling.cpp
  integration/test_control_errors_expressions.cpp
  integration/dsl_test_base.cpp
)

target_link_libraries(falcon_dsl_integration_tests
  PRIVATE
    falcon-dsl
    falcon-atc-core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME falcon_dsl_integration_tests
         COMMAND falcon_dsl_integration_tests)

target_include_directories(falcon_dsl_integration_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copy test .fal files to build directory
file(GLOB_RECURSE TEST_FAL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_cases/*.fal")
foreach(FAL_FILE ${TEST_FAL_FILES})
  file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test_cases" "${FAL_FILE}")
  configure_file(
        "${FAL_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/test_cases/${REL_PATH}"
        COPYONLY
    )
endforeach()

# Also copy v2_test.fal
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/v2_test.fal"
    "${CMAKE_CURRENT_BINARY_DIR}/v2_test.fal"
    COPYONLY
)
