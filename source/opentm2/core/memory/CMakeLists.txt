cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Memory)

set(Mem_Source_Files
        MemoryFactory.cpp
       # TMPluginWrapper.cpp
       # EQFMEMLP.CPP
       # EQFMEMEP.CPP
       # EQFMEMCD.CPP
       # EQFMEM00.CPP
       # EQFMEMIP.CPP
       # EQFMEMRP.CPP
       # EQFMEMMP.CPP
       # EQFTMM.CPP
       # EQFMEMLD.CPP
)

add_library(Memory ${Mem_Source_Files})
target_link_libraries(Memory Util)
