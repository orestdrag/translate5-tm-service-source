cmake_minimum_required(VERSION 3.10)
project(EqfMemoryPlugin)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../core)

set(OPENTM2_DIR ${ROOT}/source/opentm2)

set(EqfMemoryPlugin_Source_Files
    EqfMemory.cpp
    EqfMemoryPlugin.cpp
    #EQFNTOP.C
    EQFNTCR.C
    #EQFNTM.C
    EQFNTEXT.C
    #EQFNTMDB.C
    #EQFNTEXT.C
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
    EQFFUZZ.cpp

    #from MEMORY folder
    TMPluginWrapper.cpp
    MemoryFactory.cpp
    EQFMEMLP.CPP
    EQFMEMEP.CPP
    EQFMEMCD.CPP
    EQFMEM00.CPP
    #EQFMEMIP.CPP
    #EQFMEMRP.CPP
    #EQFMEMMP.CPP
    #EQFTMM.CPP
    #EQFMEMLD.CPP
    GenericTagReplace.cpp
    ${OPENTM2_DIR}/EQFX_API.C
)

include(GNUInstallDirs)

add_library(EqfMemoryPlugin ${EqfMemoryPlugin_Source_Files})

set_target_properties(EqfMemoryPlugin PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(EqfMemoryPlugin PRIVATE
    ${CORE_DIR}/utilities
    ${CORE_DIR}/pluginmanager
    #${CORE_DIR}/memory
)

target_link_libraries(EqfMemoryPlugin PRIVATE
    Util
    Document
    Tagtable
)

# Contains dlopen, dlclose
target_link_libraries(EqfMemoryPlugin PUBLIC ${CMAKE_DL_LIBS})
