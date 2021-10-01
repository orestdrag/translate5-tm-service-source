cmake_minimum_required(VERSION 3.10)
project(OpenTM2)

set(OPENTM2_MRI_DIR     "${ROOT}/mri")
set(ZLIB_INCLUDE_DIR    "${ROOT}/Packages/zlib")
set(MINIZIP_INCLUDE_DIR "${ZLIB_INCLUDE_DIR}/contrib/minizip")

set(API_Source_Files OTMFUNC.C)

include_directories(${OPENTM2_INCLUDE_DIR}
                    ${OPENTM2_MRI_DIR}
)

add_subdirectory(core)
add_subdirectory(tools)
add_subdirectory(plugins)

add_library(OTMFUNC ${API_Source_Files})

set_target_properties(OTMFUNC PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(OTMFUNC PRIVATE
    ${ZLIB_INCLUDE_DIR}
    ${MINIZIP_INCLUDE_DIR}
)

target_link_libraries(OTMFUNC PRIVATE Memory
                                      PluginManager
                                      Util
                                      Workbench
                                      Common
                                      Analysis
                                      #Counting
                                      Document
                                      #Lists
                                      #Services
                                      Tagtable
                                      #Ling
                                      Morph
                                      Spell
                                      EqfMemoryPlugin
)
