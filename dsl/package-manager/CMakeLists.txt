cmake_minimum_required(VERSION 3.20)
project(falcon-pm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# ─── Library ─────────────────────────────────────────────────────────────────
add_library(falcon-pm STATIC
  src/PackageManifest.cpp
  src/PackageCache.cpp
  src/PackageResolver.cpp
  src/PackageManager.cpp
)

# Required because falcon-dsl is a SHARED library and links this statically
set_target_properties(falcon-pm PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(falcon-pm
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(falcon-pm
  PUBLIC
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
    OpenSSL::Crypto
)

# ─── CLI binary ──────────────────────────────────────────────────────────────
add_executable(falcon-pm-cli src/main_cli.cpp)
target_link_libraries(falcon-pm-cli PRIVATE falcon-pm)
set_target_properties(falcon-pm-cli PROPERTIES OUTPUT_NAME falcon-pm)

# ─── Unit tests ──────────────────────────────────────────────────────────────
if(BUILD_TESTS)
  find_package(GTest CONFIG REQUIRED)
  enable_testing()

  add_executable(falcon-pm-tests
    tests/test_manifest.cpp
    tests/test_cache.cpp
    tests/test_resolver.cpp
    tests/test_package_manager.cpp
  )
  target_link_libraries(falcon-pm-tests
    PRIVATE falcon-pm GTest::gtest GTest::gtest_main
  )
  add_test(NAME falcon-pm-tests COMMAND falcon-pm-tests)
endif()

# ─── Install ─────────────────────────────────────────────────────────────────
install(TARGETS falcon-pm falcon-pm-cli
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/falcon-pm DESTINATION include)
