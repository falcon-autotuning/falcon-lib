find_package(GTest CONFIG REQUIRED)

# Unit tests
add_executable(falcon_autotuner_unit_tests
  unit/test_parameter_map.cpp
  unit/test_lexer.cpp
  unit/test_parser.cpp
  unit/test_state_node.cpp
)

target_link_libraries(falcon_autotuner_unit_tests
  PRIVATE
    falcon-autotuner-core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME falcon_autotuner_unit_tests
         COMMAND falcon_autotuner_unit_tests)

# Integration tests
add_executable(falcon_autotuner_integration_tests
  integration/test_full_compilation.cpp
  integration/test_runtime_execution.cpp
)

target_link_libraries(falcon_autotuner_integration_tests
  PRIVATE
    falcon-autotuner-core
    GTest::gtest
    GTest::gtest_main
    Boost::system
    Boost::filesystem
)

add_test(NAME falcon_autotuner_integration_tests
         COMMAND falcon_autotuner_integration_tests)
