cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(falcon-typing-tests
  test_primitive_types.cpp
)

target_link_libraries(falcon-typing-tests
  PRIVATE
    falcon-typing
    GTest::gtest_main
    GTest::gmock
)

target_include_directories(falcon-typing-tests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

gtest_discover_tests(falcon-typing-tests)
