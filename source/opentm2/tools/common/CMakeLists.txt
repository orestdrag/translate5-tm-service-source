cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Common)

set(OPENTM2_INCLUDE_DIR "${ROOT}/include")

set(Common_Source_Files InitPlugins.cpp)
add_library(Common ${Common_Source_Files})
