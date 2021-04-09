cmake_minimum_required (VERSION 3.10)
project(OpenTM2_PluginManager)

set(PlgMng_Source_Files
        PluginManager.cpp
        PluginManagerImpl.cpp)

add_library(PluginManager ${PlgMng_Source_Files})
