cmake_minimum_required(VERSION 3.20)
project(
  falcon-autotuner
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build example autotuners" ON)
option(BUILD_TOOLS "Build compiler and runtime tools" ON)
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH "Enable target_precompile_headers to speed up builds" ON)

# Windows-specific definitions
if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Compiler launcher
if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
  endif()
endif()

# Find dependencies
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS graph)
find_package(fmt CONFIG REQUIRED)

message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")
message(STATUS "Found Boost: ${Boost_VERSION}")
message(STATUS "Found fmt: ${fmt_VERSION}")

# Core library
add_library(falcon-autotuner-core STATIC
  src/ParameterMap.cpp
  src/dsl/FalconLexer.cpp
  src/dsl/FalconParser.cpp
  src/dsl/FalconCodeGenerator.cpp
  src/dsl/FalconCompiler.cpp
)

target_include_directories(falcon-autotuner-core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(falcon-autotuner-core
  PUBLIC
    nlohmann_json::nlohmann_json
    Boost::graph
    fmt::fmt
    ${CMAKE_DL_LIBS}
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-autotuner-core PRIVATE /W4)
else()
  target_compile_options(falcon-autotuner-core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-autotuner-core
    PRIVATE
      <memory>
      <string>
      <vector>
      <map>
      <set>
      <functional>
      <nlohmann/json.hpp>
  )
endif()

# Tools
if(BUILD_TOOLS)
  # Compiler tool (falc)
  add_executable(falc tools/falc.cpp)

  target_link_libraries(falc
    PRIVATE falcon-autotuner-core
  )

  install(TARGETS falc
    RUNTIME DESTINATION bin
  )

  # Runtime tool (falcon-runtime)
  find_package(Boost REQUIRED COMPONENTS system filesystem)

  add_executable(falcon-runtime tools/falcon-runtime.cpp)

  target_link_libraries(falcon-runtime
    PRIVATE
      falcon-autotuner-core
      Boost::system
      Boost::filesystem
      ${CMAKE_DL_LIBS}
  )

  install(TARGETS falcon-runtime
    RUNTIME DESTINATION bin
  )
endif()

# Tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Installation
install(TARGETS falcon-autotuner-core
  EXPORT falcon-autotuner-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/falcon-autotuner
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-autotuner-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT falcon-autotuner-targets
  FILE falcon-autotuner-targets.cmake
  NAMESPACE falcon::
  DESTINATION lib/cmake/falcon-autotuner
)

configure_file(cmake/falcon-autotuner-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-autotuner-config.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-autotuner-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-autotuner-config-version.cmake"
  DESTINATION lib/cmake/falcon-autotuner
)
