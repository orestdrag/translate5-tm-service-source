#ifndef _FILESYSTEMWRAPPER_H_
#define _FILESYSTEMWRAPPER_H_

#include "win_types.h"
#include "OSWrapper.h"
extern "C" {
        
char * filesystem_get_otm_dir();

char * filesystem_get_home_dir();

const int 
  GENERIC_READ            = 0x80000000,
  GENERIC_WRITE           = 0x40000000,
  FILE_SHARE_READ         = 0x00000001,
  FILE_SHARE_WRITE        = 0x00000002,
  FILE_SHARE_DELETE       = 0x00000004,
  FILE_FLAG_WRITE_THROUGH = 0x80000000,
  
  CREATE_NEW        = 1,
  CREATE_ALWAYS     = 2,
  OPEN_EXISTING     = 3, 
  OPEN_ALWAYS       = 4,
  TRUNCATE_EXISTING = 5,

  ERROR_SHARING_BUFFER_EXCEEDED = 0x80070024,
  ERROR_LOCK_VIOLATION          = 0x80070021 
;


const HFILE INVALID_HANDLE_VALUE = (HFILE)-1;
    
    typedef struct _OVERLAPPED {
            ULONG_PTR Internal;
            ULONG_PTR InternalHigh;
            union {
                struct {
                DWORD Offset;
                DWORD OffsetHigh;
                } DUMMYSTRUCTNAME;
                PVOID Pointer;
            } DUMMYUNIONNAME;
            HANDLE    hEvent;
    } OVERLAPPED, *LPOVERLAPPED;
    
    
    #define OFS_MAXPATHNAME 250

    typedef struct _OFSTRUCT {
            BYTE cBytes;
            BYTE fFixedDisk;
            WORD nErrCode;
            WORD Reserved1;
            WORD Reserved2;
            CHAR szPathName[OFS_MAXPATHNAME];
     } OFSTRUCT, *LPOFSTRUCT, *POFSTRUCT;


    //HANDLE 
    HFILE CreateFile(  LPCSTR                lpFileName,
                        DWORD                 dwDesiredAccess,
                        DWORD                 dwShareMode,
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                        DWORD                 dwCreationDisposition,
                        DWORD                 dwFlagsAndAttributes,
                        HANDLE                hTemplateFile
    );

    BOOL WriteFile(     //HANDLE       hFile,
                        HFILE        hFile,
                        LPCVOID      lpBuffer,
                        DWORD        nNumberOfBytesToWrite,
                        LPDWORD      lpNumberOfBytesWritten,
                        LPOVERLAPPED lpOverlapped
                        );


    BOOL ReadFile(      //HANDLE       hFile,
                        HFILE        hFile,
                        LPVOID       lpBuffer,
                        DWORD        nNumberOfBytesToRead,
                        LPDWORD      lpNumberOfBytesRead,
                        LPOVERLAPPED lpOverlapped
                        );

    //BOOL DeleteFile(LPCTSTR lpFileName);
    bool DeleteFile(char const* sFileName);

    //BOOL CloseFile(LPCTSTR lpFileName){
    //    return false;
    //}
    BOOL CloseFile(HFILE* lpFileName);

    HFILE OpenFile(     LPCSTR     lpFileName,
                        LPOFSTRUCT lpReOpenBuff,
                        UINT       uStyle
                        );

    DWORD GetLastError();

    int GetFileSize(HFILE file);

/*
    typedef struct _WIN32_FIND_DATA {
        DWORD     dwFileAttributes; 
        FILETIME  ftCreationTime; 
        FILETIME  ftLastAccessTime; 
        FILETIME  ftLastWriteTime; 
        DWORD     nFileSizeHigh; 
        DWORD     nFileSizeLow; 
        DWORD     dwReserved0; 
        DWORD     dwReserved1; 
        TCHAR     cFileName[ MAX_PATH ]; 
        TCHAR     cAlternateFileName[ 14 ]; 
        } WIN32_FIND_DATA, *PWIN32_FIND_DATA;  
//*/

    HFILE FindFirstFile(
         LPCTSTR lpFileName,               
         LPWIN32_FIND_DATA lpFindFileData  
        );

    BOOL FindNextFile(
        HANDLE             hFindFile,
        LPWIN32_FIND_DATA lpFindFileData
        );


    BOOL CloseHandle(HFILE hf );

/*
    BOOL SetFilePointerEx(
        HANDLE hFile,                    
        LARGE_INTEGER liDistanceToMove,  
        PLARGE_INTEGER lpNewFilePointer, 
        DWORD dwMoveMethod               
        );//*/

    BOOL SetFilePointerEx(
        HFILE hFile,                    
        LARGE_INTEGER liDistanceToMove,  
        PLARGE_INTEGER lpNewFilePointer, 
        DWORD dwMoveMethod               
        );

    //DWORD SetFilePointer(HANDLE fp,LONG LoPart,LONG *HiPart,DWORD OffSet);
    DWORD SetFilePointer(HFILE fp,LONG LoPart,LONG *HiPart,DWORD OffSet);

    void CopyFilePathReplaceExt(char* dest, const char* src, const char* new_ext);

    BOOL FlushFileBuffers(
        HANDLE hFile
    );

    int GetFileId(HFILE ptr);

    int SkipBytesFromBeginningInFile(HFILE ptr, int numOfBytes);
}



#endif
