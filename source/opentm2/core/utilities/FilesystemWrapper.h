#ifndef _FILESYSTEMWRAPPER_H_
#define _FILESYSTEMWRAPPER_H_

#include "win_types.h"

//extern "C" {
    
    typedef struct _SECURITY_ATTRIBUTES {
            DWORD  nLength;
            LPVOID lpSecurityDescriptor;
            BOOL   bInheritHandle;
    } SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
    
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


    HANDLE CreateFile(  LPCSTR                lpFileName,
                        DWORD                 dwDesiredAccess,
                        DWORD                 dwShareMode,
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                        DWORD                 dwCreationDisposition,
                        DWORD                 dwFlagsAndAttributes,
                        HANDLE                hTemplateFile
    );

    BOOL WriteFile(     HANDLE       hFile,
                        LPCVOID      lpBuffer,
                        DWORD        nNumberOfBytesToWrite,
                        LPDWORD      lpNumberOfBytesWritten,
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
//}



#endif
