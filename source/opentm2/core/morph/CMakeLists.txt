cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Morph)

set(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../utilities)

set(Morph_Source_Files MorphFactory.cpp)

add_library(Morph ${Morph_Source_Files})

target_include_directories(Morph PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${UTILITIES_DIR}
)

install(TARGETS Morph DESTINATION lib)
