cmake_minimum_required(VERSION 3.20)

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
  src/Compiler.cpp
  src/ParseError.cpp
  src/BuiltinRegistry.cpp
)

set_target_properties(falcon-atc-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(falcon-atc-core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>  # For generated headers
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(falcon-atc-core PUBLIC nlohmann_json::nlohmann_json)

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
  EXPORT falcon-dsl-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
