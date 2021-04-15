cmake_minimum_required (VERSION 3.10)
project(OpenTM2_PluginManager)

set(PlgMng_Source_Files
        PluginManager.cpp
        PluginManagerImpl.cpp
)

set(CURL_INDLUDE_DIR "${ROOT}/Packages/curl/include/curl")
set(ZLIB_INDLUDE_DIR "${ROOT}/Packages/zlib")
set(MINIZIP_INDLUDE_DIR "${ROOT}/Packages/zlib/contrib/minizip")

include_directories(${CURL_INDLUDE_DIR}
                    ${ZLIB_INDLUDE_DIR}
                    ${MINIZIP_INDLUDE_DIR})

add_library(PluginManager ${PlgMng_Source_Files})

target_link_libraries(PluginManager libcurl.so libz.so)
