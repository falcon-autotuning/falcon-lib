# autotuner/tests/CMakeLists.txt
find_package(GTest CONFIG REQUIRED)


# Integration tests
add_executable(falcon_dsl_integration_tests
  integration/test_basic_features.cpp
  integration/test_expressions.cpp
  integration/test_control_flow.cpp
  integration/dsl_test_base.cpp
  integration/test_config.cpp
  integration/test_database_access.cpp
  integration/test_nest.cpp
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

# Copy test .fal and .json files to build directory
file(GLOB_RECURSE TEST_FAL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test-autotuners/*.fal")
file(GLOB_RECURSE TEST_JSON_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test-autotuners/*.json")
set(TEST_DATA_FILES ${TEST_FAL_FILES} ${TEST_JSON_FILES})

foreach(DATA_FILE ${TEST_DATA_FILES})
  file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test-autotuners" "${DATA_FILE}")
  configure_file(
    "${DATA_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/test-autotuners/${REL_PATH}"
    COPYONLY
  )
endforeach()
