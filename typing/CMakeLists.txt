cmake_minimum_required(VERSION 3.20)
project(falcon-typing VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build tests"                          ON)
option(USE_CCACHE  "Enable ccache or sccache if available" ON)
option(ENABLE_PCH  "Enable target_precompile_headers"     ON)

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

# ── Main shared library ───────────────────────────────────────────────────────
add_library(falcon-typing SHARED
  src/PrimitiveTypes.cpp
)

set_target_properties(falcon-typing PROPERTIES
  VERSION   ${PROJECT_VERSION}
  SOVERSION 1
)

target_include_directories(falcon-typing
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
  target_compile_options(falcon-typing PRIVATE /W4)
else()
  target_compile_options(falcon-typing PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-typing
    PRIVATE
      <string> <vector> <memory> <variant> <optional> <map>
  )
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS falcon-typing
  EXPORT falcon-typing-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/falcon-typing
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-typing-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT falcon-typing-targets
  FILE      falcon-typing-targets.cmake
  NAMESPACE falcon::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/falcon-typing
)

configure_package_config_file(
  cmake/falcon-typing-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-typing-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/falcon-typing
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-typing-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/falcon-typing-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/falcon-typing
)
