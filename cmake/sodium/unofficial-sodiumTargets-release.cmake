#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial-sodium::sodium" for configuration "Release"
set_property(TARGET unofficial-sodium::sodium APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial-sodium::sodium PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;C"
  IMPORTED_LOCATION_RELEASE "$/opt/local/lib/libsodium.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS unofficial-sodium::sodium )
list(APPEND _IMPORT_CHECK_FILES_FOR_unofficial-sodium::sodium "/opt/local/lib/libsodium.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
