cmake_minimum_required (VERSION 3.10)
project(OpenTM2)

set(ROOT "../..")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
set(OPENTM2_MRI_DIR "${ROOT}/mri")
set(ZLIB_INCLUDE_DIR    "${ROOT}/Packages/zlib")
set(MINIZIP_INCLUDE_DIR "${ROOT}/Packages/zlib/contrib/minizip")

include_directories(${OPENTM2_INCLUDE_DIR}
                    ${OPENTM2_MRI_DIR}
                    ${ZLIB_INCLUDE_DIR}
                    ${MINIZIP_INCLUDE_DIR})

add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(tools)
