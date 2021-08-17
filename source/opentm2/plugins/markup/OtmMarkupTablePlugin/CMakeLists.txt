cmake_minimum_required (VERSION 3.10)
project(OpenTM2_Markup_OtmPlugins)

add_subdirectory(common)



set(OpenTM2_Markup_OtmPlugins_Source_Files
        OtmMarkupTable.cpp
        OtmMarkupTablePlugin.cpp)

add_library(OtmPlugins ${OpenTM2_Markup_OtmPlugins_Source_Files})

