#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "falcon::falcon-comms" for configuration "Debug"
set_property(TARGET falcon::falcon-comms APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(falcon::falcon-comms PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libfalcon-comms.so"
  IMPORTED_SONAME_DEBUG "libfalcon-comms.so"
  )

list(APPEND _cmake_import_check_targets falcon::falcon-comms )
list(APPEND _cmake_import_check_files_for_falcon::falcon-comms "${_IMPORT_PREFIX}/lib/libfalcon-comms.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
