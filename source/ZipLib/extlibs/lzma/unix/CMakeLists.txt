
cmake_minimum_required(VERSION 3.10)
project(ZipLib_extlibs_lzma_unix )

add_subdirectory()

set(ZipLibExtLibsSourceFiles_lzma_unix  
    7zBuf2.c 
    7zCrc.c 
    7zCrcOpt.c 
    7zStream.c 
    Aes.c 
    Alloc.c 
    Bra.c 
    Bra86.c 
    BraIA64.c 
    BwtSort.c 
    CpuArch.c 
    Delta.c 
    HeffEnc.c 
    LzFind.c 
    LzFind.c 
    LzFindMt.c 
    Lzma2Dec.c 
    Lzma2Enc.c 
    LzmaDec.c 
    LzmaEnc.c 
    MtCoder.c 
    Ppmd7.c 
    Ppmd7dec.c 
    Ppmd7Enc.c 
    Ppmd8.c 
    Ppmd8Dec.c 
    Ppmd8Enc.c 
    Sha256.c 
    Sort.c 
    Threads.c 
    Xz.c 
    XzCrc64.c 
    XzDec.c 
    XzEnc.c 
    XzIn.c 
    
)


add_library(ZipLibExtLibsSourceFilesLzmaUnix ${ZipLibExtLibsSourceFiles_lzma_unix})
