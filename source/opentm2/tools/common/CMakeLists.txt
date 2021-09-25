cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Common)

add_library(Common InitPlugins.cpp)

install(TARGETS Common DESTINATION lib)
