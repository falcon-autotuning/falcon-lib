find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(lsp CONFIG REQUIRED)
set_target_properties(lsp::lsp PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "/opt/falcon/include/lsp"
  IMPORTED_LOCATION_RELEASE "/opt/falcon/lib/liblsp.a"
  IMPORTED_LOCATION_DEBUG "/opt/falcon/lib/liblspd.a"
)

add_library(falcon-lsp-lib
    src/FalconDocument.cpp
    src/TypeChecker.cpp
    src/HoverProvider.cpp
    src/CompletionProvider.cpp
    src/DiagnosticsProvider.cpp
    src/DefinitionProvider.cpp
    src/FalconServer.cpp
)
set_target_properties(falcon-lsp-lib PROPERTIES OUTPUT_NAME "falcon-lsp")
target_include_directories(falcon-lsp-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(falcon-lsp-lib PUBLIC
    falcon-atc-core
    lsp::lsp
)
set_target_properties(falcon-lsp-lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(falcon-lsp-lib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>  # For generated headers
    $<INSTALL_INTERFACE:include>
)

add_executable(falcon-lsp src/main.cpp)
target_link_libraries(falcon-lsp PRIVATE falcon-lsp-lib)

install(TARGETS falcon-lsp
  EXPORT falcon-autotuner-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
