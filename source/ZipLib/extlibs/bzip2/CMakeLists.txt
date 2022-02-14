
cmake_minimum_required(VERSION 3.10)
project(ZipLib_extlibs_bzip2)

set(ZipLibExtLibsSourceFiles_bzip2 blocksort.c 
    bzlib.c 
    bzerror.c
    compress.c
    crctable.c 
    decompress.c 
    huffman.c
    randtable.c  
)


add_library(ZipLibExtLibsBZip2 ${ZipLibExtLibsSourceFiles_bzip2})
