cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Util)

add_subdirectory(zip)

set(Util_Source_Files
    LanguageFactory.CPP
    LogWriter.cpp
    OtmProposal.cpp
    UtlAlloc.C
    UtlMATVal.C
    UtlMisc.cpp
    UtlString.C
    EQFUTERR.C
    EQFPRO00.C
    EQFUTFIL.C
    EQFUTDOS.C
    EQFOBJ00.C
    EQFUTLNG.CPP
    EQFOSWIN.C
    EQFHASH.C
    Property.cpp
    EncodingHelper.cpp
    FilesystemHelper.cpp
    FilesystemWrapper.cpp
    LogWrapper.cpp
    ThreadingWrapper.cpp
    OSWrapper.cpp
    EQFSETUP.C
    UtlLangCP.C
    UtlRegistry.C
    EQFDICT.CPP
    EqfPluginWrapper.cpp
    EQFPRO00.C
    EQFMORPW.C
    EQFMORPH.CPP
    OTMGLOBM.CPP
    EQFTADIT.CPP
    EQFMORPH.CPP
    ZipHelper.cpp 
)

set_source_files_properties(${Util_Source_Files} PROPERTIES LANGUAGE CXX )

add_library(Util ${Util_Source_Files})

set_target_properties(Util PROPERTIES POSITION_INDEPENDENT_CODE ON)
    
target_include_directories(Util PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "/opt/local/include")
target_link_libraries(Util PRIVATE libxerces-c-3.2.so EqfMemoryPlugin Morph zip ) #Util_zip)
