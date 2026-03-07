find_package(GTest CONFIG REQUIRED)

add_executable(falcon_qarrayDevice_tests
  test_device.cpp
)

target_include_directories(falcon_qarrayDevice_tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(falcon_qarrayDevice_tests
  PRIVATE
    falcon-qarrayDevice
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(falcon_qarrayDevice_tests)
