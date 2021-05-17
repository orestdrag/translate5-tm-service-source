cmake_minimum_required (VERSION 3.10)
project(EqfMemoryPlugin)

set(OPENTM2_INCLUDE_DIR "${ROOT}/include")

set(EqfMemoryPlugin_Source_Files
    EqfMemory.cpp
    EqfMemoryPlugin.cpp
   # EQFNTOP.C
   # EQFNTCR.C
   # EQFNTM.C
   # EQFNTEXT.C
   # EQFNTMDB.C
   # EQFNTEXT.C
   # EQFNTPUT.C
)

include(GNUInstallDirs)

add_library(EqfMemoryPlugin SHARED ${EqfMemoryPlugin_Source_Files})

set_target_properties(EqfMemoryPlugin PROPERTIES PUBLIC_HEADER EqfMemory.h)

target_link_libraries(EqfMemoryPlugin ${CMAKE_DL_LIBS} PluginManager Util)

configure_file(OpenTM2_EqfMemoryPlugin.pc @ONLY)

install(TARGETS EqfMemoryPlugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    ${CMAKE_BINARY_DIR}/OpenTM2_EqfMemoryPlugin.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)
