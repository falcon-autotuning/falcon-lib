cmake_minimum_required(VERSION 3.20)
project(voltage_sweep_example)

set(CMAKE_CXX_STANDARD 17)

# Find falc compiler
find_program(FALC_COMPILER falc REQUIRED)

# Compile .fal file
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.cpp
    COMMAND ${FALC_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/voltage_sweep.fal
        -o ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS voltage_sweep.fal
    COMMENT "Compiling voltage_sweep.fal"
)

# Build shared library
add_library(voltage_sweep_autotuner SHARED
    voltage_sweep_impl.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/GeneratedAutotuners.cpp
)

target_include_directories(voltage_sweep_autotuner
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(voltage_sweep_autotuner
    PRIVATE falcon::autotuner-core
)

# Set RPATH for runtime
if(APPLE)
  set_target_properties(voltage_sweep_autotuner PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(UNIX)
  set_target_properties(voltage_sweep_autotuner PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
