cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Morph)

set(Morph_Source_Files
       MorphFactory.cpp
)

add_library(Morph ${Morph_Source_Files})
