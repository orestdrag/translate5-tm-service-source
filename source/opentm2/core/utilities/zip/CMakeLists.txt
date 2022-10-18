cmake_minimum_required(VERSION 3.10)

project(zip
  LANGUAGES C
  VERSION "0.2.3")

set(SRC miniz.h zip.h zip.c)

add_library(${PROJECT_NAME} ${SRC})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
