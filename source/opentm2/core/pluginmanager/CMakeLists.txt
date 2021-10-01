cmake_minimum_required(VERSION 3.10)
project(OpenTM2_PluginManager)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PlgMng_Source_Files PluginManager.cpp
                        PluginManagerImpl.cpp
)

add_library(PluginManager ${PlgMng_Source_Files})

set_target_properties(PluginManager PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(PluginManager PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${CORE_DIR}/utilities
)

target_link_libraries(PluginManager PRIVATE EqfMemoryPlugin)
