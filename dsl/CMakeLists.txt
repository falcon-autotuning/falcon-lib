cmake_minimum_required(VERSION 3.20)
project(falcon-dsl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS  "Build tests"                         ON)
option(USE_CCACHE   "Enable ccache or sccache if available" ON)
option(ENABLE_PCH   "Enable target_precompile_headers"    ON)

if(WIN32)
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${SCCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
  endif()
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog        CONFIG REQUIRED)
find_package(cereal        CONFIG REQUIRED)
find_package(xtensor       CONFIG REQUIRED)
find_package(xtl           CONFIG REQUIRED)
find_package(yaml-cpp      CONFIG REQUIRED)
find_package(OpenSSL               REQUIRED)
find_package(cnats         CONFIG REQUIRED)

# 1. Compiler sub-project
add_subdirectory(compiler)

if(NOT TARGET falcon::falcon-database)
  find_package(falcon-database CONFIG REQUIRED)
endif()
if(NOT TARGET falcon::falcon-comms)
  find_package(falcon-comms CONFIG REQUIRED)
endif()
include_directories(/opt/falcon/include)
find_package(falcon_core CONFIG REQUIRED)

# falcon-typing: provides PrimitiveTypes (RuntimeValue/ParameterMap) and FFI ABI.
# First try in-tree (when building from the falcon-lib root), then installed.
if(NOT TARGET falcon::falcon-typing)
  find_package(falcon-typing CONFIG REQUIRED
    PATHS
      "${CMAKE_CURRENT_SOURCE_DIR}/../typing/build/release"
      /opt/falcon/lib/cmake/falcon-typing
  )
endif()

# 2. Package manager sub-project (falcon-pm)
add_subdirectory(package-manager)

# 3. DSL Runtime Library (falcon-dsl)
add_library(falcon-dsl SHARED
  src/AutotunerEngine.cpp
  src/FunctionRegistry.cpp
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
    falcon::falcon-typing
  PRIVATE
    falcon-pm
)

# Pass the in-tree include directories so process_ff_import can compile wrappers
# even when falcon-dsl/ falcon-typing are not yet installed to /opt/falcon.
target_compile_definitions(falcon-dsl PRIVATE
    FALCON_AUTOTUNER_INCLUDE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/include"
    FALCON_TYPING_INCLUDE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../typing/include"
)

if(MSVC)
  target_compile_options(falcon-dsl PRIVATE /W4)
else()
  target_compile_options(falcon-dsl PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-dsl
    PRIVATE
      <string> <vector> <memory> <variant> <optional>
      <nlohmann/json.hpp>
  )
endif()

# 4. LSP
add_subdirectory(lsp)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

install(TARGETS falcon-dsl
  EXPORT falcon-dsl-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-dsl-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
install(EXPORT falcon-dsl-targets
  FILE falcon-dsl-targets.cmake
  NAMESPACE falcon::
  DESTINATION lib/cmake/falcon-dsl
)
configure_file(cmake/falcon-dsl-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-dsl-config.cmake" @ONLY
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-dsl-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-dsl-config-version.cmake"
  DESTINATION lib/cmake/falcon-dsl
)
