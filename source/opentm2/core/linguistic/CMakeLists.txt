cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Linguistic)

set(Ling_Source_Files
        EQFMORPH.CPP
)

add_library(Ling ${Ling_Source_Files})
