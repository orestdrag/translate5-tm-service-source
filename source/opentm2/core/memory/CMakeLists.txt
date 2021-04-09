cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Memory)

set(Mem_Source_Files
        MemoryFactory.cpp
        EQFMEMLP.CPP
        EQFMEMEP.CPP
        EQFMEMCD.CPP
        EQFMEM00.CPP)

add_library(Memory ${Mem_Source_Files})
target_link_libraries(Memory Util)
