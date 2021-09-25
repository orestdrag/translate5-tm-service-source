cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Memory)

set(OPENTM2_ROOT "../..")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(Mem_Source_Files
        MemoryFactory.cpp
        TMPluginWrapper.cpp
        EQFMEMLP.CPP
        EQFMEMEP.CPP
        EQFMEMCD.CPP
        EQFMEM00.CPP
       # EQFMEMIP.CPP
       # EQFMEMRP.CPP
       # EQFMEMMP.CPP
       # EQFTMM.CPP
       # EQFMEMLD.CPP
        ${OPENTM2_ROOT}/EQFX_API.C
)


add_library(Memory ${Mem_Source_Files})

target_include_directories(Memory PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities
)

target_link_libraries(Memory Util Tagtable Analysis)

install(TARGETS Memory DESTINATION lib)
