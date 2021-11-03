cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Core)

add_subdirectory(pluginmanager)
add_subdirectory(utilities)
add_subdirectory(workbench)

add_subdirectory(analysis)
add_subdirectory(document)
add_subdirectory(tagtable)
add_subdirectory(morph)
add_subdirectory(spell)
add_subdirectory(EqfMemoryPlugin)
