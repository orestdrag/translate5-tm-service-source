cmake_minimum_required (VERSION 3.10)
project(OpenTM2_PluginManager)

set(PlgMng_Source_Files
        PluginManager.cpp
        PluginManagerImpl.cpp
)

set(CURL_INDLUDE_DIR "${ROOT}/Packages/curl/include/curl")
set(ZLIB_INDLUDE_DIR "${ROOT}/Packages/zlib")
set(MINIZIP_INDLUDE_DIR "${ROOT}/Packages/zlib/contrib/minizip")

set(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../utilities)

include_directories(${CURL_INDLUDE_DIR}
                    ${ZLIB_INDLUDE_DIR}
                    ${MINIZIP_INDLUDE_DIR}
    ${OPENTM2_INCLUDE_DIR}
    ${UTILITIES_DIR}
)

add_library(PluginManager ${PlgMng_Source_Files})

set_target_properties(PluginManager PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(PluginManager libcurl.so libz.so)

install(TARGETS PluginManager DESTINATION lib)
