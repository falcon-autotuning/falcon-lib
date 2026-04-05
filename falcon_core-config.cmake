set(falcon_core_INCLUDE_DIRS "/opt/falcon/include")
set(falcon_core_LIBRARIES "/opt/falcon/lib/libfalcon_core_cpp.so")

if(NOT TARGET falcon_core_cpp)
    add_library(falcon_core_cpp SHARED IMPORTED)
    set_target_properties(falcon_core_cpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/opt/falcon/include"
        IMPORTED_LOCATION "/opt/falcon/lib/libfalcon_core_cpp.so"
    )
endif()

if(NOT TARGET falcon_core_c_api)
    add_library(falcon_core_c_api SHARED IMPORTED)
    set_target_properties(falcon_core_c_api PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/opt/falcon/include"
        IMPORTED_LOCATION "/opt/falcon/lib/libfalcon_core_c_api.so"
    )
endif()

set(falcon_core_FOUND TRUE)
