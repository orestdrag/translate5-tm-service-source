cmake_minimum_required(VERSION 3.10)
include(GNUInstallDirs)

project(t5core)

set(OPENTM2_MRI_DIR     "${ROOT}/mri")
set(ZLIB_INCLUDE_DIR    "${ROOT}/Packages/zlib")
set(MINIZIP_INCLUDE_DIR "${ZLIB_INCLUDE_DIR}/contrib/minizip")

include_directories(${OPENTM2_INCLUDE_DIR}
                    ${OPENTM2_MRI_DIR}
)

add_subdirectory(utilities/zip)

set(core_source_files
#pluginmanager
    pluginmanager/PluginManager.cpp
    pluginmanager/PluginManagerImpl.cpp
#tagtable
    tagtable/EQFTOKEN.C
    tagtable/MarkupPluginMapper.cpp
#morph
    morph/MorphFactory.cpp
    morph/OtmMorphICU.cpp 
    morph/OtmMorphICUPlugin.cpp 
    morph/SpellFactory.cpp

#Util_Source_Files
    utilities/LanguageFactory.CPP
    utilities/LogWriter.cpp
    utilities/OtmProposal.cpp
    utilities/UtlAlloc.C
    utilities/UtlMATVal.C
    utilities/UtlMisc.cpp
    utilities/UtlString.C
    utilities/EQFUTERR.C
    utilities/EQFPRO00.C
    utilities/EQFUTFIL.C
    utilities/EQFUTDOS.C
    utilities/EQFOBJ00.C
    utilities/EQFUTLNG.CPP
    utilities/EQFOSWIN.C
    utilities/EQFHASH.C
    utilities/Property.cpp
    utilities/EncodingHelper.cpp
    utilities/FilesystemHelper.cpp
    utilities/FilesystemWrapper.cpp
    utilities/LogWrapper.cpp
    utilities/ThreadingWrapper.cpp
    utilities/OSWrapper.cpp
    utilities/EQFSETUP.C
    utilities/UtlLangCP.C
    utilities/UtlRegistry.C
    utilities/EQFDICT.CPP
    utilities/EqfPluginWrapper.cpp
    utilities/EQFPRO00.C
    utilities/EQFMORPW.C
    utilities/EQFMORPH.CPP
    utilities/OTMGLOBM.CPP
    utilities/EQFTADIT.CPP
    utilities/EQFMORPH.CPP
    utilities/ZipHelper.cpp 
    utilities/EQFMAIN.C

    utilities/DocumentPluginMapper.cpp
    utilities/CXMLWRITER.CPP 
    utilities/InitPlugins.cpp
    utilities/EQFTATAG.C

    #EqfMemoryPlugin
    EqfMemoryPlugin/EqfMemory.cpp
    EqfMemoryPlugin/EqfMemoryPlugin.cpp
    EqfMemoryPlugin/EQFNTCR.C
    EqfMemoryPlugin/EQFNTEXT.C
    EqfMemoryPlugin/EQFNTPUT.C
    EqfMemoryPlugin/EQFNTUTL.C
    EqfMemoryPlugin/EQFNTMDB.C
    EqfMemoryPlugin/EQFNTM.C
    EqfMemoryPlugin/EQFNTOP.C
    EqfMemoryPlugin/EQFMEMUT.C
    EqfMemoryPlugin/EQFNTUTL.C
    EqfMemoryPlugin/OTMTMXIE.CPP
    EqfMemoryPlugin/EQFNTDEL.C
    EqfMemoryPlugin/EQFNTGET.CPP
    EqfMemoryPlugin/EQFFUZZ.cpp

    #from MEMORY folder
    EqfMemoryPlugin/TMPluginWrapper.cpp
    EqfMemoryPlugin/MemoryFactory.cpp
    EqfMemoryPlugin/EQFMEMLP.CPP
    EqfMemoryPlugin/EQFMEMEP.CPP
    EqfMemoryPlugin/EQFMEMCD.CPP
    EqfMemoryPlugin/EQFMEM00.CPP
    EqfMemoryPlugin/GenericTagReplace.cpp
    ../EQFX_API.C
    ../OTMFUNC.C
)

set_source_files_properties(${core_source_files} PROPERTIES LANGUAGE CXX )

add_library(t5core ${core_source_files})

target_include_directories(t5core PRIVATE
    utilities
    pluginmanager
    Tagtable
    morph
    EqfMemoryPlugin
)

set_target_properties(t5core PROPERTIES POSITION_INDEPENDENT_CODE ON)
    
find_package(ICU 66.0 COMPONENTS uc i18n REQUIRED)

target_include_directories(t5core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "/opt/local/include"
    ${ZLIB_INCLUDE_DIR}
    ${MINIZIP_INCLUDE_DIR}
    )
target_link_libraries(t5core PRIVATE libxerces-c-3.2.so  zip   ICU::uc ICU::i18n )


