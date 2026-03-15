cmake_minimum_required(VERSION 3.20)
project(
  falcon-database
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" ON)
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH "Enable target_precompile_headers to speed up builds" ON)

# Windows-specific definitions
if(WIN32)
  add_compile_definitions(NOMINMAX)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Try to auto-detect a compiler launcher (prefer sccache if present)
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
    else()
      message(STATUS "No ccache/sccache found (fine, builds will just be normal).")
    endif()
  endif()
endif()

# Find dependencies (via vcpkg)
find_package(libpqxx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

message(STATUS "Found libpqxx: ${libpqxx_VERSION}")
message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")

# Embed the JSON schema file
set(SNAPSHOT_SCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/schema/snapshot.schema.json")
if(NOT EXISTS "${SNAPSHOT_SCHEMA_FILE}")
  message(FATAL_ERROR "Schema file not found: ${SNAPSHOT_SCHEMA_FILE}")
endif()

# Read the schema file content
file(READ "${SNAPSHOT_SCHEMA_FILE}" SNAPSHOT_SCHEMA_CONTENT)

# Configure the header file with embedded schema
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SnapshotSchemaData.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/SnapshotSchemaData.hpp"
  @ONLY
)

# Core library
add_library(falcon-database SHARED
  src/DeviceCharacteristic.cpp
  src/DatabaseConnection.cpp
  src/SnapshotManager.cpp
)

target_include_directories(falcon-database
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated  # Add generated headers
)

target_link_libraries(falcon-database
  PUBLIC
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-database PRIVATE /W4)
else()
  target_compile_options(falcon-database PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-database
    PRIVATE
      <string>
      <vector>
      <memory>
      <optional>
      <pqxx/pqxx>
      <nlohmann/json.hpp>
  )
endif()

# Tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# CLI Executable
add_executable(falcon-db-cli src/cli.cpp)
target_link_libraries(falcon-db-cli PRIVATE falcon-database)
if(ENABLE_PCH)
  target_precompile_headers(falcon-db-cli
    PRIVATE
      <string>
      <vector>
      <iostream>
  )
endif()

# Installation
install(TARGETS falcon-database falcon-db-cli
  EXPORT falcon-database-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/falcon-database
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)

# Install the generated header (optional, if you want it available to users)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/generated/SnapshotSchemaData.hpp"
  DESTINATION include/falcon-database/internal
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-databaseConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT falcon-database-targets
  FILE falcon-database-targets.cmake
  NAMESPACE falcon::
  DESTINATION lib/cmake/falcon-database
)

configure_file(cmake/falcon-databaseConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-databaseConfig.cmake"
  @ONLY
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-databaseConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-databaseConfigVersion.cmake"
  DESTINATION lib/cmake/falcon-database
)
