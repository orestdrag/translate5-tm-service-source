cmake_minimum_required (VERSION 3.10)

project(OtmMemoryService VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ROOT                "..")
set(RESTBED_PATH        "${ROOT}/Packages/restbed")
set(RESTBED_INCLUDE_DIR "${RESTBED_PATH}/distribution/include")
set(RESTBED_LIBRARIES   "${RESTBED_PATH}/distribution/library")
set(XERCESC_INCLUDE_DIR "/usr/local/include/xercesc")
set(LIBRARIES           "/usr/local/lib")
set(MINIZIP_INCLUDE_DIR "${ROOT}/Packages/zlib/contrib/minizip")
set(ZLIB_INCLUDE_DIR    "${ROOT}/Packages/zlib")

set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(OPENTM2_MRI_DIR     "${ROOT}/mri")
set(OPENTM2_API_DIR     "opentm2/api")
set(OPENTM2_MEM_DIR     "opentm2/core/memory")
set(OPENTM2_UTIL_DIR    "opentm2/core/utilities")
set(OPENTM2_WBN_DIR     "opentm2/core/workbench")
set(OPENTM2_TLCMN_DIR   "opentm2/tools/common")
set(OPENTM2_PLGMNG_DIR  "opentm2/core/pluginmanager")
set(OPENTM2_DICT_DIR    "opentm2/core/dictionary")

include_directories(${RESTBED_INCLUDE_DIR}
                    ${XERCESC_INCLUDE_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    ${MINIZIP_INCLUDE_DIR}
                    ${OPENTM2_INCLUDE_DIR}
                    ${OPENTM2_MRI_DIR})

link_directories(${RESTBED_LIBRARIES})

add_library(UtilModule
                    ${OPENTM2_UTIL_DIR}/LanguageFactory.CPP
                    ${OPENTM2_UTIL_DIR}/LogWriter.cpp
                    ${OPENTM2_UTIL_DIR}/OtmProposal.cpp
                    ${OPENTM2_UTIL_DIR}/UtlAlloc.C
                    ${OPENTM2_UTIL_DIR}/UtlMATVal.c
                    ${OPENTM2_UTIL_DIR}/UtlMisc.cpp
                    ${OPENTM2_UTIL_DIR}/UtlString.C
                    ${OPENTM2_UTIL_DIR}/EQFUTERR.C
                    ${OPENTM2_UTIL_DIR}/EQFPRO00.C
                    ${OPENTM2_UTIL_DIR}/EQFUTFIL.C
                    ${OPENTM2_UTIL_DIR}/EQFUTDOS.C
                    ${OPENTM2_UTIL_DIR}/EQFOBJ00.C
)

add_library(MemModule
                    ${OPENTM2_MEM_DIR}/MemoryFactory.cpp
                    ${OPENTM2_MEM_DIR}/EQFMEMLP.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEMEP.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEMCD.CPP
                    ${OPENTM2_MEM_DIR}/EQFMEM00.CPP
)

add_library(WorkbenchModule
                    ${OPENTM2_WBN_DIR}/EQFMAIN.C
                    ${OPENTM2_WBN_DIR}/EQFPLGMG.CPP
)

add_library(CommonModule
                    ${OPENTM2_TLCMN_DIR}/InitPlugins.cpp
)

add_library(DictModule
            ${OPENTM2_DICT_DIR}/DictPluginWrapper.cpp
)

add_library(PluginManagerModule
                    ${OPENTM2_PLGMNG_DIR}/PluginManager.cpp
                    ${OPENTM2_PLGMNG_DIR}/PluginManagerImpl.cpp
)

add_library(OpenTM2Module
                    ${OPENTM2_API_DIR}/OTMFUNC.C
)

add_executable(OtmMemoryService
               OtmMemoryService.cpp
               OtmMemoryServiceWorker.cpp
               OtmMemoryServiceTester.cpp
               OTMMSJSONFactory.cpp)

target_link_libraries(UtilModule libxerces-c-3.2.so)

target_link_libraries(OpenTM2Module UtilModule)
target_link_libraries(OpenTM2Module MemModule)
target_link_libraries(OpenTM2Module WorkbenchModule)
target_link_libraries(OpenTM2Module DictModule)
target_link_libraries(OpenTM2Module PluginManagerModule)
target_link_libraries(OpenTM2Module CommonModule)

target_link_libraries(OtmMemoryService librestbed.so)
target_link_libraries(OtmMemoryService OpenTM2Module)
