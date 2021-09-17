cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Analysis)

set(Analysis_Source_Files
        EQFTATAG.C
        #EQFANA00.CPP
        #EQFTAG00.C
)

add_library(Analysis ${Analysis_Source_Files})


install(TARGETS Analysis DESTINATION lib)
