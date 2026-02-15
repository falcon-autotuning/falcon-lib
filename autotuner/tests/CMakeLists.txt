find_package(GTest CONFIG REQUIRED)

# Unit tests
add_executable(falcon_dsl_unit_tests
  unit/test_parameter_map.cpp
  unit/test_lexer.cpp
  unit/test_parser.cpp
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
  integration/test_full_compilation.cpp
)

target_link_libraries(falcon_dsl_integration_tests
  PRIVATE
    falcon-dsl
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME falcon_dsl_integration_tests
         COMMAND falcon_dsl_integration_tests)
