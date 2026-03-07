#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "falcon::falcon-qarrayDevice" for configuration "Debug"
set_property(TARGET falcon::falcon-qarrayDevice APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(falcon::falcon-qarrayDevice PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libfalcon-qarrayDevice.so"
  IMPORTED_SONAME_DEBUG "libfalcon-qarrayDevice.so"
  )

list(APPEND _cmake_import_check_targets falcon::falcon-qarrayDevice )
list(APPEND _cmake_import_check_files_for_falcon::falcon-qarrayDevice "${_IMPORT_PREFIX}/lib/libfalcon-qarrayDevice.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
