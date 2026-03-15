find_package(GTest CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PLPLOT REQUIRED IMPORTED_TARGET plplot)

add_executable(falcon_qarrayDevice_tests
  test_device.cpp
  test_visual.cpp
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
    PkgConfig::PLPLOT
)

include(GoogleTest)
gtest_discover_tests(falcon_qarrayDevice_tests
  PROPERTIES
    ENVIRONMENT
      "FALCON_QARRAY_PYTHON_PATH=${FALCON_DEV_PYTHON_PATH};FALCON_QARRAY_VENV_SITE_PACKAGES=${FALCON_DEV_SITE_PACKAGES}"
)
