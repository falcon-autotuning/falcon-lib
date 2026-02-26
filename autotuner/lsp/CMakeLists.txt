cmake_minimum_required(VERSION 3.20)
project(falcon-lsp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build tools" ON)

# ---- falcon-atc-core (compiler) ----------------------------------------
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../autotuner/compiler
    ${CMAKE_CURRENT_BINARY_DIR}/atc
)

# ---- lsp-framework -------------------------------------------------------
find_package(lsp REQUIRED CONFIG PATHS /opt/falcon/lib/cmake/lsp NO_DEFAULT_PATH)

# ---- LSP server library --------------------------------------------------
add_library(falcon-lsp-lib
    src/FalconDocument.cpp
    src/TypeChecker.cpp
    src/HoverProvider.cpp
    src/CompletionProvider.cpp
    src/DiagnosticsProvider.cpp
    src/DefinitionProvider.cpp
    src/FalconServer.cpp
)

target_include_directories(falcon-lsp-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(falcon-lsp-lib PUBLIC
    falcon-atc-core
    lsp::lsp
)

set_target_properties(falcon-lsp-lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---- falcon-lsp executable -----------------------------------------------
add_executable(falcon-lsp src/main.cpp)
target_link_libraries(falcon-lsp PRIVATE falcon-lsp-lib)

install(TARGETS falcon-lsp RUNTIME DESTINATION bin)

# ---- Sub-directories -----------------------------------------------------
if(BUILD_TOOLS)
    add_subdirectory(tools/falcon-lint)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
