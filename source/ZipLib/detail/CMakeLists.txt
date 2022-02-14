
cmake_minimum_required(VERSION 3.10)
project(ZipLib_detail)


set(ZipLibDetailSourceFiles EndOfCentralDirectoryBlock.cpp 
                      ZipCentralDirectoryFileHeader.cpp 
                      ZipGenericExtraField.cpp
                      ZipLocalFileHeader.cpp 
)


add_library(ZipLibDetail ${ZipLibDetailSourceFiles})
