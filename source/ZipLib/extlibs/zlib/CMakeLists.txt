
cmake_minimum_required(VERSION 3.10)
project(ZipLib_extlibs_zlib)



set(ZipLibExtLibsSourceFiles_zlib  
    adler32.c 
    compress.c 
    crc32.c 
    deflate.c 
    infback.c
    inffast.c 
    inflate.c 
    inftrees.c 
    trees.c 
    uncompr.c 
    zutil.c 
    
)


add_library(ZipLibExtLibsZlib ${ZipLibExtLibsSourceFiles_zlib})
