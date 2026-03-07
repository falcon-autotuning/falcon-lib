cmake_minimum_required(VERSION 3.20)

find_package(GTest REQUIRED CONFIG)

set(FAL_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/test-autotuners")
set(FAL_TEST_DEST_DIR   "${CMAKE_CURRENT_BINARY_DIR}/test-autotuners")

# Copy .fal test files to build directory
file(GLOB_RECURSE TEST_FAL_FILES "${FAL_TEST_SOURCE_DIR}/*.fal")
foreach(DATA_FILE ${TEST_FAL_FILES})
  file(RELATIVE_PATH REL_PATH "${FAL_TEST_SOURCE_DIR}" "${DATA_FILE}")
  configure_file(
        "${DATA_FILE}"
        "${FAL_TEST_DEST_DIR}/${REL_PATH}"
        COPYONLY
    )
endforeach()

# ---- unit tests ----------------------------------------------------------
add_executable(falcon_lsp_unit_tests
    unit/test_document.cpp
    unit/test_type_checker.cpp
    unit/test_hover.cpp
    unit/test_completion.cpp
    unit/test_diagnostics.cpp
    unit/test_import_resolver.cpp
    unit/test_semantic_diagnostics.cpp
)

target_link_libraries(falcon_lsp_unit_tests
    PRIVATE
        falcon-lsp-lib
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME falcon_lsp_unit_tests COMMAND falcon_lsp_unit_tests)

set_tests_properties(falcon_lsp_unit_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# ---- integration tests ---------------------------------------------------
add_executable(falcon_lsp_integration_tests
    integration/test_fal_files.cpp
)

target_compile_definitions(falcon_lsp_integration_tests PRIVATE
    TEST_AUTOTUNERS_DIR="${FAL_TEST_DEST_DIR}"
    FALCON_LSP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_link_libraries(falcon_lsp_integration_tests
    PRIVATE
        falcon-lsp-lib
        GTest::gtest
        GTest::gtest_main
)

add_test(NAME falcon_lsp_integration_tests COMMAND falcon_lsp_integration_tests)

