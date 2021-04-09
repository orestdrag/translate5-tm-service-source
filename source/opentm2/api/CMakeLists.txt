cmake_minimum_required (VERSION 3.10)
project(OpenTM2_API)

set(API_Source_Files OTMFUNC.C)

add_library(OTMFUNC ${API_Source_Files})

target_link_libraries(OTMFUNC
                        Memory
                        PluginManager
                        Util
                        Workbench
                        Common)
