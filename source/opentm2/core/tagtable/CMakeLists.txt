cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Tagtable)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(TAGTABLE_SOURCE_FILES
    EQFTOKEN.C
    MarkupPluginMapper.cpp
)

set_source_files_properties(${TAGTABLE_SOURCE_FILES} PROPERTIES LANGUAGE CXX )

add_library(Tagtable ${TAGTABLE_SOURCE_FILES})

set_target_properties(Tagtable PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Tagtable PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${CORE_DIR}/utilities
)
