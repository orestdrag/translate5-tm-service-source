cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Tagtable)

set(TAGTABLE_SOURCE_FILES
    EQFTOKEN.C
    #EQFTATAG.C
    MarkupPluginMapper.cpp
)

add_library(Tagtable ${TAGTABLE_SOURCE_FILES})

set_target_properties(Tagtable PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Tagtable PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities
)

install(TARGETS Tagtable DESTINATION lib)
