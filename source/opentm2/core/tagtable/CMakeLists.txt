cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Tagtable)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(TAGTABLE_SOURCE_FILES
    EQFTOKEN.C
    #EQFTATAG.C
    MarkupPluginMapper.cpp
)

add_library(Tagtable ${TAGTABLE_SOURCE_FILES})

set_target_properties(Tagtable PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Tagtable PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${CORE_DIR}/utilities
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(TARGETS Tagtable DESTINATION lib)
