find_package(GTest REQUIRED)
include(GoogleTest)

# Test utilities
add_library(falcon-comms-test-utils STATIC
  test_utils/TestFixture.hpp
)

target_link_libraries(falcon-comms-test-utils
  PUBLIC
    falcon-comms
    GTest::gtest
)

target_include_directories(falcon-comms-test-utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils
)

set_target_properties(falcon-comms-test-utils PROPERTIES
  LINKER_LANGUAGE CXX
)

# Unit tests
add_executable(falcon_comms_unit_tests
  unit/test_manager.cpp
  unit/test_autotuner_comms.cpp
  unit/test_routine_comms.cpp
  unit/test_runtime_comms.cpp
)

target_link_libraries(falcon_comms_unit_tests
  PRIVATE
    falcon-comms
    falcon-comms-test-utils
    GTest::gtest
    GTest::gtest_main
)

if(WIN32)
  add_test(NAME falcon_comms_unit_tests COMMAND falcon_comms_unit_tests)
else()
  gtest_discover_tests(falcon_comms_unit_tests)
endif()

