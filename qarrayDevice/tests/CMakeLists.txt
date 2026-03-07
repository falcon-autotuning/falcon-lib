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
gtest_discover_tests(falcon_qarrayDevice_tests
  # Inject environment variables so the embedded Python can find:
  #   1. quantum_dot_device.py  (FALCON_QARRAY_PYTHON_PATH → src/)
  #   2. qarray/numpy           (FALCON_QARRAY_VENV_SITE_PACKAGES → venv site-packages)
  # These are set at cmake configure time from the detected venv Python.
  PROPERTIES
    ENVIRONMENT
      "FALCON_QARRAY_PYTHON_PATH=${FALCON_DEV_PYTHON_PATH};FALCON_QARRAY_VENV_SITE_PACKAGES=${FALCON_DEV_SITE_PACKAGES}"
)
