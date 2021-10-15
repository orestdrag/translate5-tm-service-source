cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Morph)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(Morph_Source_Files MorphFactory.cpp
        OtmMorphICU.cpp 
        OtmMorphICUPlugin.cpp 
        )

add_library(Morph ${Morph_Source_Files})

target_include_directories(Morph PRIVATE
    ${OPENTM2_INCLUDE_DIR}
    ${CORE_DIR}/utilities
    ${CORE_DIR}/pluginmanager
)
find_package(ICU 66.0 COMPONENTS uc i18n REQUIRED)

target_link_libraries(Morph PRIVATE ICU::uc ICU::i18n )