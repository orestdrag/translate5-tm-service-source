cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Memory)

set(OPENTM2_DIR ${ROOT}/source/opentm2)
set(CORE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(Mem_Source_Files MemoryFactory.cpp
                     TMPluginWrapper.cpp
                     EQFMEMLP.CPP
                     EQFMEMEP.CPP
                     EQFMEMCD.CPP
                     EQFMEM00.CPP
                     #EQFMEMIP.CPP
                     #EQFMEMRP.CPP
                     #EQFMEMMP.CPP
                     #EQFTMM.CPP
                     #EQFMEMLD.CPP
                     GenericTagReplace.cpp
                     ${OPENTM2_DIR}/EQFX_API.C
)

add_library(Memory ${Mem_Source_Files})

set_target_properties(Memory PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(Memory PRIVATE ${CORE_DIR}/utilities)
target_link_libraries(Memory PRIVATE Util Tagtable Analysis)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(TARGETS Memory DESTINATION lib)
