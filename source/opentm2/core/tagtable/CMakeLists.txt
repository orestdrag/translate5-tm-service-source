cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Tagtable)

set(Tagtable_Source_Files
        #EQFTOKEN.C
        MarkupPluginMapper.cpp)

add_library(Tagtable ${Tagtable_Source_Files})
