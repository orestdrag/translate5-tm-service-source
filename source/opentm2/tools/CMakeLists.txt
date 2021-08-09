cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Tools)

set(OPENTM2_INCLUDE_DIR "${ROOT}/include")

set(Tools_Source_Files
    CXMLWRITER.CPP
)



#include_directories(${XERCESC_INCLUDE_DIR})

add_subdirectory(common)

add_library(Tools ${Tools_Source_Files})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")