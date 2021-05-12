cmake_minimum_required (VERSION 3.10)
project(OpenTM2_EqfMemoryPlugin)

set(OPENTM2_INCLUDE_DIR "${ROOT}/include")

set(EqfMemoryPlugin_Source_Files
    EqfMemory.cpp
    EqfMemoryPlugin.cpp
)
add_library(EqfMemoryPlugin ${EqfMemoryPlugin_Source_Files})
