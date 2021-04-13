cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Services)

set(Services_Source_Files
        EQFDDE00.C)

add_library(Services ${Services_Source_Files})
