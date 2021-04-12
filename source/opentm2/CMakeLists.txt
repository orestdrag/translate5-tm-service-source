cmake_minimum_required (VERSION 3.10)
project(OpenTM2)

set(ROOT "../..")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(OPENTM2_MRI_DIR "${ROOT}/mri")
set(ZLIB_INCLUDE_DIR    "${ROOT}/Packages/zlib")
set(MINIZIP_INCLUDE_DIR "${ROOT}/Packages/zlib/contrib/minizip")

set(API_Source_Files OTMFUNC.C)

include_directories(${OPENTM2_INCLUDE_DIR}
                    ${OPENTM2_MRI_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    ${MINIZIP_INCLUDE_DIR})

add_subdirectory(core)
add_subdirectory(tools)

add_library(OTMFUNC ${API_Source_Files})

target_link_libraries(OTMFUNC
                        Memory
                        PluginManager
                        Util
                        Workbench
                        Common)
