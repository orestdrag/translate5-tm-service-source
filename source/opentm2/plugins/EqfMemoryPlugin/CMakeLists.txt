cmake_minimum_required (VERSION 3.10)
project(EqfMemoryPlugin)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../core)

set(EqfMemoryPlugin_Source_Files
    EqfMemory.cpp
    EqfMemoryPlugin.cpp
   # EQFNTOP.C
    EQFNTCR.C
   # EQFNTM.C
    EQFNTEXT.C
   # EQFNTMDB.C
   # EQFNTEXT.C
   EQFNTPUT.C
   EQFNTUTL.C
   EQFNTMDB.C
   EQFNTM.C
   EQFNTOP.C
   EQFMEMUT.C
   EQFNTUTL.C
   OTMTMXIE.CPP
   OTMXLFMT.CPP
   EQFNTDEL.C
   EQFNTGET.CPP
)

include(GNUInstallDirs)

add_library(EqfMemoryPlugin SHARED ${EqfMemoryPlugin_Source_Files})

set_target_properties(EqfMemoryPlugin PROPERTIES
    PUBLIC_HEADER EqfMemory.h
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(EqfMemoryPlugin PRIVATE
    ${CORE_DIR}/utilities
    ${CORE_DIR}/pluginmanager
    ${CORE_DIR}/memory
)

target_link_libraries(EqfMemoryPlugin ${CMAKE_DL_LIBS} PluginManager Util Tools Analysis Memory Tagtable )

configure_file(OpenTM2_EqfMemoryPlugin.pc @ONLY)

install(TARGETS EqfMemoryPlugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    ${CMAKE_BINARY_DIR}/OpenTM2_EqfMemoryPlugin.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)
