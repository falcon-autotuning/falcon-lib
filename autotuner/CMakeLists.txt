cmake_minimum_required(VERSION 3.20)
project(
  falcon-dsl
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" ON)
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH "Enable target_precompile_headers to speed up builds" ON)

# Find dependencies (via vcpkg)
find_package(nlohmann_json CONFIG REQUIRED)

# DSL Compiler Library
add_library(falcon-dsl
  src/ParameterMap.cpp
  src/FalconLexer.cpp
  src/FalconParser.cpp
  src/FalconCodeGenerator.cpp
  src/FalconCompiler.cpp
)

target_include_directories(falcon-dsl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(falcon-dsl
  PUBLIC
    nlohmann_json::nlohmann_json
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-dsl PRIVATE /W4)
else()
  target_compile_options(falcon-dsl PRIVATE -Wall -Wextra -Wpedantic)
endif()

# DSL Compiler Executable
add_executable(falcon-dsl-compiler
  src/main.cpp
)

target_link_libraries(falcon-dsl-compiler
  PRIVATE
    falcon-dsl
)

# Tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Installation
install(TARGETS falcon-dsl falcon-dsl-compiler
  EXPORT falcon-dsl-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/falcon-autotuner
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)
