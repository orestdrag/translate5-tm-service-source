cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Workbench)

set(Workbench_Source_Files
        EQFMAIN.C
        EQFPLGMG.CPP)

add_library(Workbench ${Workbench_Source_Files})
