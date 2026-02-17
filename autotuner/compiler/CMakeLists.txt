cmake_minimum_required(VERSION 3.20)

# This is now a sub-component, not a standalone project
# project() call removed - parent handles it

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate Parser
bison_target(AutotunerParser
  src/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.hpp
)

# Generate Lexer
flex_target(AutotunerLexer
  src/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cpp
)

# Ensure lexer depends on parser (needs parser tokens)
add_flex_bison_dependency(AutotunerLexer AutotunerParser)

# Compiler Core Library
add_library(falcon-atc-core
  ${BISON_AutotunerParser_OUTPUTS}
  ${FLEX_AutotunerLexer_OUTPUTS}
  src/CompilerLib.cpp
)

target_include_directories(falcon-atc-core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>  # For generated headers
    $<INSTALL_INTERFACE:include>
)

# Suppress warnings from generated code
if(MSVC)
  target_compile_options(falcon-atc-core PRIVATE /W0)
else()
  target_compile_options(falcon-atc-core PRIVATE -w)
endif()

# Make sure generated files are built first
set_source_files_properties(
  ${BISON_AutotunerParser_OUTPUTS}
  ${FLEX_AutotunerLexer_OUTPUTS}
  PROPERTIES GENERATED TRUE
)

install(TARGETS falcon-atc-core
  EXPORT falcon-autotuner-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
