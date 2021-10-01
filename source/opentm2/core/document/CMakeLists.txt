cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Document)

set(Document_Source_Files #EQFDOC00.C
                          DocumentPluginMapper.cpp
)

add_library(Document ${Document_Source_Files})
