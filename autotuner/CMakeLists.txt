cmake_minimum_required(VERSION 3.20)
project(
  falcon-autotuner
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build Options
option(BUILD_TESTS "Build tests" ON)
option(USE_CCACHE "Enable ccache or sccache if available" ON)
option(ENABLE_PCH "Enable target_precompile_headers to speed up builds" ON)


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
# Find dependencies (via vcpkg or system)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(xtensor CONFIG REQUIRED)
find_package(xtl CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cnats CONFIG REQUIRED)

# 1. Build Compiler Sub-project (falcon-atc-core)
add_subdirectory(compiler)

if(NOT TARGET falcon::falcon-database)
  find_package(falcon-database CONFIG REQUIRED)
endif()
if(NOT TARGET falcon::falcon-comms)
  find_package(falcon-comms CONFIG REQUIRED)
endif()
# After finding falcon_core_cpp
find_package(falcon_core CONFIG REQUIRED)
# FIX: necessary until better cmake-config is created
include_directories(/opt/falcon/include)

# 2. Build DSL Runtime Library (falcon-dsl)
add_library(falcon-dsl SHARED
  src/AutotunerEngine.cpp
  src/FunctionRegistry.cpp
  src/RuntimeValue.cpp
  src/StmtExecutor.cpp
  src/TypeRegistry.cpp
  src/ExprEvaluator.cpp
  src/Interpreter.cpp
  src/SourceContext.cpp
  src/log.cpp
)

target_include_directories(falcon-dsl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(falcon-dsl
  PUBLIC
    nlohmann_json::nlohmann_json
    falcon::falcon-database
    falcon::falcon-comms
    falcon-atc-core
    cereal::cereal
    xtensor
    xtl
    yaml-cpp::yaml-cpp
    cnats::nats
    falcon_core_cpp
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-dsl PRIVATE /W4)
else()
  target_compile_options(falcon-dsl PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Precompiled headers
if(ENABLE_PCH)
  target_precompile_headers(falcon-dsl
    PRIVATE
      <string>
      <vector>
      <memory>
      <variant>
      <optional>
      <nlohmann/json.hpp>
  )
endif()


# 3. Build LSP (falcon-lsp)
add_subdirectory(lsp)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Installation
install(TARGETS falcon-dsl
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
