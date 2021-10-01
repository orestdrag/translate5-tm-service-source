cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Spell)

add_library(Spell SpellFactory.cpp)
target_include_directories(Spell PRIVATE ${OPENTM2_INCLUDE_DIR})
