cmake_minimum_required(VERSION 3.20)
project(falcon-atc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate Parser
bison_target(AutotunerParser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.hpp)

# Generate Lexer
flex_target(AutotunerLexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cpp)
add_flex_bison_dependency(AutotunerLexer AutotunerParser)

# Compiler Binary
add_executable(falcon-atc
    src/compiler.cpp
    ${BISON_AutotunerParser_OUTPUTS}
    ${FLEX_AutotunerLexer_OUTPUTS}
)

target_include_directories(falcon-atc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/falcon-atc
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link rules
target_link_libraries(falcon-atc PRIVATE)
