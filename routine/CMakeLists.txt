cmake_minimum_required(VERSION 3.20)
project(
  falcon-routine
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

# Find falcon-database (either in build tree or installed)
if(NOT TARGET falcon::falcon-database)
  find_package(falcon-database CONFIG REQUIRED)
endif()
if(NOT TARGET falcon::falcon-comms)
  find_package(falcon-comms CONFIG REQUIRED)
endif()
# After finding falcon_core_cpp
find_package(falcon_core CONFIG REQUIRED)

message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")
message(STATUS "Found spdlog: ${spdlog_VERSION}")
message(STATUS "Found NATS: ${nats_VERSION}")

# Core library
add_library(falcon-routine SHARED
  src/hub.cpp
  src/log.cpp
)

target_include_directories(falcon-routine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(falcon-routine
  PUBLIC
    falcon::falcon-database
    falcon::falcon-comms
    falcon_core_cpp
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    cnats::nats
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-routine PRIVATE /W4)
else()
  target_compile_options(falcon-routine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-routine
    PRIVATE
      <string>
      <vector>
      <memory>
      <optional>
      <mutex>
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
install(TARGETS falcon-routine
  EXPORT falcon-routine-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/falcon-routine
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-routine-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT falcon-routine-targets
  FILE falcon-routine-targets.cmake
  NAMESPACE falcon::
  DESTINATION lib/cmake/falcon-routine
)

configure_file(cmake/falcon-routine-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-routine-config.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-routine-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-routine-config-version.cmake"
  DESTINATION lib/cmake/falcon-routine
)
