# Example plugin
add_library(example_plugin SHARED
  example_plugin.cpp
)

target_link_libraries(example_plugin
  PRIVATE
    falcon-autotuner-core
)

set_target_properties(example_plugin PROPERTIES
  PREFIX ""
  POSITION_INDEPENDENT_CODE ON
)

# Install plugin
install(TARGETS example_plugin
  LIBRARY DESTINATION lib/falcon-autotuner/plugins
  RUNTIME DESTINATION bin
)
