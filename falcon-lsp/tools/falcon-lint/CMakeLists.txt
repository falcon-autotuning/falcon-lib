add_executable(falcon-lint main.cpp)

target_link_libraries(falcon-lint PRIVATE falcon-atc-core)

install(TARGETS falcon-lint RUNTIME DESTINATION bin)
