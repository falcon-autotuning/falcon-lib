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

# ── Compiler cache ──────────��─────────────────────────────────────────────────
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
# IMPORTANT: We must find the SAME Python that has qarray and numpy installed
# (i.e., the uv-managed venv), NOT vcpkg's stripped-down Python headers.
#
# The caller (Makefile) is responsible for passing:
#   -DPython3_EXECUTABLE=/path/to/uv/venv/bin/python
#   -DPython3_ROOT_DIR=/path/to/uv/venv
#
# This is detected automatically by `make configure` using `uv run which python`.
# You can also override it manually:
#   make configure PYTHON_EXECUTABLE=/my/python
#
# pybind11::embed will link against the libpython of this exact interpreter,
# so the embedded interpreter at runtime will find its full stdlib + site-packages.

# Prevent cmake from finding vcpkg's Python or the system Python ahead of ours.
# cmake's FindPython3 respects Python3_EXECUTABLE if set via -D on the command line.
if(DEFINED Python3_EXECUTABLE)
  message(STATUS "Python3_EXECUTABLE explicitly set to: ${Python3_EXECUTABLE}")
  # Force cmake to use only this interpreter — do not search elsewhere.
  set(Python3_FIND_STRATEGY LOCATION)
  set(Python3_FIND_REGISTRY NEVER)
  set(Python3_FIND_FRAMEWORK NEVER)
endif()

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Python3 interpreter : ${Python3_EXECUTABLE}")
message(STATUS "Python3 version     : ${Python3_VERSION}")
message(STATUS "Python3 include dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3 libraries   : ${Python3_LIBRARIES}")
message(STATUS "Python3 stdlib dir  : ${Python3_STDLIB}")

# Sanity check: confirm the interpreter has the standard library.
# If this fails, you are pointing at an incomplete Python (e.g. vcpkg's headers-only Python).
if(Python3_STDLIB STREQUAL "")
  message(WARNING
    "Python3_STDLIB is empty — the found Python may not have a standard library. "
    "Pass -DPython3_EXECUTABLE=<full-python> to cmake, or run `make configure` "
    "which auto-detects the uv venv Python.")
endif()

# ── Find pybind11 (via vcpkg) ─────────────────────────────────────────────────
find_package(pybind11 CONFIG REQUIRED)
message(STATUS "Found pybind11: ${pybind11_VERSION}")

# ── Store the venv site-packages path so Device.cpp can add it to sys.path ───
# This embeds the path at compile time so the library knows where qarray lives.
# Override at runtime with FALCON_QARRAY_PYTHON_PATH env var.
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c
    "import sys; print([p for p in sys.path if 'site-packages' in p][0])"
  OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE _site_result
)
if(NOT _site_result EQUAL 0 OR PYTHON_SITE_PACKAGES STREQUAL "")
  message(WARNING "Could not detect Python site-packages dir from ${Python3_EXECUTABLE}")
  set(PYTHON_SITE_PACKAGES "")
endif()
message(STATUS "Python3 site-packages: ${PYTHON_SITE_PACKAGES}")

# ── Install path for the device.py module ────────────────────────
set(PYTHON_MODULE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/qarrayDevice")

# ── Generate headers ──────────────────────────────────────────────────────────
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
    pybind11::embed
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

install(FILES src/device.py
  DESTINATION share/qarrayDevice
)

install(EXPORT falcon-qarrayDeviceTargets
  FILE      falcon-qarrayDeviceTargets.cmake
  NAMESPACE falcon::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/falcon-qarrayDevice
)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
  enable_testing()

  # ── Pass Python paths to the test executable via CTest environment ──────────
  # During development (non-installed build), quantum_dot_device.py lives in src/.
  # We bake this path into the CTest environment so `make test-debug` just works
  # without needing to set any env vars manually.
  set(FALCON_DEV_PYTHON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(FALCON_DEV_SITE_PACKAGES "${PYTHON_SITE_PACKAGES}")

  add_subdirectory(tests)
endif()
