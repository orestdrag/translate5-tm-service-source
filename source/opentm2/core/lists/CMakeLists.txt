cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Lists)

set(Lists_Source_Files EQFLIST.C)

add_library(Lists ${Lists_Source_Files})

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(TARGETS Lists DESTINATION lib)
