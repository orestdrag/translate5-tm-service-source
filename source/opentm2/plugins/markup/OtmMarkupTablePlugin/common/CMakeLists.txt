cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Markup_OtmPlugins_Common)


set(ROOT "../../../../")
set(OPENTM2_INCLUDE_DIR "${ROOT}/include")
#set(OPENTM2_ROOT ".")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


include_directories(${OPENTM2_ROOT})

set(OpenTM2_Markup_OtmPlugins_Common_Source_Files
        eqfcalls.c
        reseq.c
        unicode.cpp
        usrcalls.c)

add_library(OtmPlugins_Common ${OpenTM2_Markup_OtmPlugins_Common_Source_Files})
