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
    GTest::gtest        # NOT GTest::gtest_main — we define our own main()
    pybind11::embed     # needed for scoped_interpreter in test_device.cpp
)

include(GoogleTest)
gtest_discover_tests(falcon_qarrayDevice_tests
  PROPERTIES
    ENVIRONMENT
      "FALCON_QARRAY_PYTHON_PATH=${FALCON_DEV_PYTHON_PATH};FALCON_QARRAY_VENV_SITE_PACKAGES=${FALCON_DEV_SITE_PACKAGES}"
)
