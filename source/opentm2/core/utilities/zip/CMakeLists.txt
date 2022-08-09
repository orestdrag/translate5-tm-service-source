cmake_minimum_required(VERSION 3.10)

project(zip
  LANGUAGES C
  VERSION "0.2.3")
#project(OpenTM2_Util_zip)
#add_library(Util_zip zip.c)


set(SRC miniz.h zip.h zip.c)

add_library(${PROJECT_NAME} ${SRC})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#set_target_properties(Util PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(${PROJECT_NAME} PRIVATE miniz)