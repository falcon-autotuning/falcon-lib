cmake_minimum_required(VERSION 3.20)
project(
  falcon-qarrayDevice
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Build Options ─────────────────────────────────────────────────────────────
option(BUILD_TESTS  "Build tests"                          ON)
option(USE_CCACHE   "Enable ccache or sccache if available" ON)
option(ENABLE_PCH   "Enable precompiled headers"           ON)

# ── Windows ───────────────────────────────────────────────────────────────────
if(WIN32)
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

# ── Compiler cache ────────────────────────────────────────────────────────────
if(USE_CCACHE)
  find_program(SCCACHE_PROGRAM sccache)
  if(SCCACHE_PROGRAM)
    message(STATUS "Using sccache: ${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER   ${SCCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
  else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
      set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    else()
      message(STATUS "No ccache/sccache found — building normally.")
    endif()
  endif()
endif()

# ── Find Python ───────────────────────────────────────────────────────────────
# We need the Python headers and libraries to embed the interpreter.
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python3 interpreter : ${Python3_EXECUTABLE}")
message(STATUS "Python3 include dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3 libraries   : ${Python3_LIBRARIES}")

# ── Find pybind11 (via vcpkg) ─────────────────────────────────────────────────
find_package(pybind11 CONFIG REQUIRED)
message(STATUS "Found pybind11: ${pybind11_VERSION}")

# ── Determine where the Python module will live at install time ───────────────
# We install quantum_dot_device.py to <prefix>/share/qarrayDevice/.
# This path is compiled into the library so it can find the module at runtime.
# It can be overridden at runtime by FALCON_QARRAY_PYTHON_PATH env var.
set(PYTHON_MODULE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/qarrayDevice")

# ── Generate a small header with the install path ────────────────────────────
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/PythonModulePath.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/PythonModulePath.hpp"
  @ONLY
)

# ── Core library ─────────────────────────────────────────────────────────────
add_library(falcon-qarrayDevice SHARED
  src/Device.cpp
)

target_include_directories(falcon-qarrayDevice
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(falcon-qarrayDevice
  PUBLIC
    pybind11::embed      # brings in Python headers + libpython
)

# Compiler warnings
if(MSVC)
  target_compile_options(falcon-qarrayDevice PRIVATE /W4)
else()
  target_compile_options(falcon-qarrayDevice PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_PCH)
  target_precompile_headers(falcon-qarrayDevice
    PRIVATE
      <string>
      <vector>
      <map>
      <memory>
      <pybind11/embed.h>
      <pybind11/numpy.h>
      <pybind11/stl.h>
  )
endif()

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS falcon-qarrayDevice
  EXPORT   falcon-qarrayDeviceTargets
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the Python module so the embedded interpreter can find it
install(FILES src/device.py
  DESTINATION share/qarrayDevice
)

# Export cmake targets
install(EXPORT falcon-qarrayDeviceTargets
  FILE      falcon-qarrayDeviceTargets.cmake
  NAMESPACE falcon::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/falcon-qarrayDevice
)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
