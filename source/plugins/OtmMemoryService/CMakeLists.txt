cmake_minimum_required (VERSION 3.10)

project(OtmMemoryService VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(RESTBED_PATH        "../../../Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/distribution/library")
set(XERCESC_INCLUDE_DIR "/usr/local/include/xercesc")
set(XERCESC_LIBRARIES   "/usr/local/lib")

set(OPENTM2_INCLUDE_DIR "../../../include")
set(OPENTM2_API_DIR     "../../api")
set(OPENTM2_MEM_DIR     "../../core/memory")
set(OPENTM2_UTIL_DIR    "../../core/utilities")
set(OPENTM2_WBN_DIR     "../../core/workbench")
set(OPENTM2_TLCMN_DIR   "../../tools/common")
set(OPENTM2_PLGMNG_DIR  "../../core/pluginmanager")

include_directories(${RESTBED_INCLUDE_DIR}
                    ${XERCESC_INCLUDE_DIR}
                    ${OPENTM2_INCLUDE_DIR})

link_directories(${RESTBED_LIBRARIES})

add_library(LogModule
                    ${OPENTM2_UTIL_DIR}/LogWriter.cpp)

add_library(LanguageFactoryModule
                    ${OPENTM2_UTIL_DIR}/LanguageFactory.CPP)

add_library(UtilModule
                    ${OPENTM2_UTIL_DIR}/UtlString.C
                    ${OPENTM2_UTIL_DIR}/UtlAlloc.C
                    ${OPENTM2_UTIL_DIR}/UtlMATVal.c
                    ${OPENTM2_UTIL_DIR}/UtlMisc.cpp
                    ${OPENTM2_UTIL_DIR}/EQFUTFIL.C
                    ${OPENTM2_UTIL_DIR}/EQFUTERR.C)

add_library(MemoryFactoryModule
                    ${OPENTM2_UTIL_DIR}/OtmProposal.cpp
                    ${OPENTM2_MEM_DIR}/MemoryFactory.cpp)

add_library(OpenTM2Module
                    ${OPENTM2_MEM_DIR}/EQFMEMLP.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEMEP.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEMCD.CPP
                    #${OPENTM2_UTIL_DIR}/UtlAlloc.C
                    #${OPENTM2_UTIL_DIR}/UtlMATVal.c
                    #${OPENTM2_UTIL_DIR}/UtlMisc.cpp
                    #${OPENTM2_UTIL_DIR}/EQFUTERR.C
                    ${OPENTM2_MEM_DIR}/EQFMEM00.CPP
                    ${OPENTM2_WBN_DIR}/EQFMAIN.C
                    ${OPENTM2_UTIL_DIR}/EQFPRO00.C
                    #${OPENTM2_UTIL_DIR}/EQFUTFIL.C
                    ${OPENTM2_UTIL_DIR}/EQFUTDOS.C
                    ${OPENTM2_TLCMN_DIR}/InitPlugins.cpp #nowinapi
                    ${OPENTM2_PLGMNG_DIR}/PluginManager.cpp #nowinapi
                    ${OPENTM2_UTIL_DIR}/EQFOBJ00.C
                    #${OPENTM2_MEM_DIR}/MemoryFactory.cpp
                    #${OPENTM2_UTIL_DIR}/LanguageFactory.CPP
                    ${OPENTM2_API_DIR}/OTMFUNC.C)

add_executable(OtmMemoryService
               OtmMemoryService.cpp
               OtmMemoryServiceWorker.cpp
               OtmMemoryServiceTester.cpp
               OTMMSJSONFactory.cpp)

# Language Factory
target_link_libraries(LanguageFactoryModule libxerces-c-3.2.so)
target_link_libraries(LanguageFactoryModule LogModule)

# Memory Factory
target_link_libraries(MemoryFactoryModule UtilModule)
target_link_libraries(MemoryFactoryModule LogModule)

# OpenTM2
target_link_libraries(OpenTM2Module LanguageFactoryModule)
target_link_libraries(OpenTM2Module UtilModule)
target_link_libraries(OpenTM2Module MemoryFactoryModule)

# OTM Memory Service
target_link_libraries(OtmMemoryService librestbed.so)
target_link_libraries(OtmMemoryService libxerces-c-3.2.so)

target_link_libraries(OtmMemoryService LanguageFactoryModule)
target_link_libraries(OtmMemoryService UtilModule)
target_link_libraries(OtmMemoryService MemoryFactoryModule)
target_link_libraries(OtmMemoryService OpenTM2Module)
