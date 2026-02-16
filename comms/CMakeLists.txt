cmake_minimum_required(VERSION 3.20)
project(
  falcon-comms
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" ON)
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH "Enable precompiled headers" ON)

# Windows-specific
if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Compiler cache
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
find_package(spdlog CONFIG REQUIRED)
find_package(cnats CONFIG REQUIRED)
find_package(OpenSSL CONFIG REQUIRED)

message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")
message(STATUS "Found spdlog: ${spdlog_VERSION}")

# Core library
add_library(falcon-comms SHARED
  src/natsManager.cpp
  src/autotuner_comms.cpp
  src/routine_comms.cpp
  src/runtime_comms.cpp
)

target_include_directories(falcon-comms
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(falcon-comms
  PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cnats::nats
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-comms PRIVATE /W4)
else()
  target_compile_options(falcon-comms PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-comms
    PRIVATE
      <string>
      <vector>
      <memory>
      <optional>
      <mutex>
      <functional>
      <nlohmann/json.hpp>
      <spdlog/spdlog.h>
  )
endif()

# Tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Installation
install(TARGETS falcon-comms
  EXPORT falcon-comms-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/falcon-comms
  DESTINATION include
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-comms-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT falcon-comms-targets
  FILE falcon-comms-targets.cmake
  NAMESPACE falcon::
  DESTINATION lib/cmake/falcon-comms
)

configure_file(cmake/falcon-comms-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-comms-config.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-comms-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-comms-config-version.cmake"
  DESTINATION lib/cmake/falcon-comms
)
