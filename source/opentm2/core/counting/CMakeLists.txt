cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Counting)

set(Counting_Source_Files
        EQFRPT.C)

add_library(Counting ${Counting_Source_Files})

install(TARGETS Counting DESTINATION lib)
