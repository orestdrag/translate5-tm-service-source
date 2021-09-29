cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Tools)

add_library(Tools CXMLWRITER.CPP)

set_target_properties(Tools PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(common)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

install(TARGETS Tools DESTINATION lib)
