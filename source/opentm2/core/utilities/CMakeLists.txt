cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Util)

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
        SystemFunctionsWrapper.cpp
        Property.cpp
        EQFSETUP.C
#EQFUTDLG.CPP
#EQFPROPS.CPP
#OptionsDialog.CPP
        UtlLangCP.C
        UtlRegistry.C
#EQFUTCLB.C
        #unzip.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${XERCESC_INCLUDE_DIR})
add_library(Util ${Util_Source_Files})
target_link_libraries(Util libxerces-c-3.2.so)
