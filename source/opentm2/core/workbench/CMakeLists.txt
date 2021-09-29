cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Workbench)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(Workbench_Source_Files EQFMAIN.C
                           #EQFPLGMG.CPP
                           #EQFLNG00.C
                           #EQFFLL00.CPP
                           #EQFFOLLI.C
)

add_library(Workbench ${Workbench_Source_Files})

target_include_directories(Workbench PRIVATE ${CORE_DIR}/utilities)
target_link_libraries(Workbench PRIVATE Util)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(TARGETS Workbench DESTINATION lib)
