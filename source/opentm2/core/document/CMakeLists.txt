cmake_minimum_required(VERSION 3.10)
project(OpenTM2_Document)

set(Document_Source_Files EQFMAIN.C
                          DocumentPluginMapper.cpp
                          CXMLWRITER.CPP 
                          InitPlugins.cpp
                          EQFTATAG.C
)

set_source_files_properties(${Document_Source_Files} PROPERTIES LANGUAGE CXX )

add_library(Document ${Document_Source_Files})

set_target_properties(Document PROPERTIES POSITION_INDEPENDENT_CODE ON)
