#include "FilesystemWrapper.h"

  //HANDLE
  HFILE CreateFile(  LPCSTR                lpFileName,
                        DWORD                 dwDesiredAccess,
                        DWORD                 dwShareMode,
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                        DWORD                 dwCreationDisposition,
                        DWORD                 dwFlagsAndAttributes,
                        HANDLE                hTemplateFile
    ){
        return 0;
    }

    BOOL WriteFile(     //HANDLE       hFile,
                        HFILE        hFile,
                        LPCVOID      lpBuffer,
                        DWORD        nNumberOfBytesToWrite,
                        LPDWORD      lpNumberOfBytesWritten,
                        LPOVERLAPPED lpOverlapped
                        ){
                            return false;
                        }

    //BOOL DeleteFile(LPCTSTR lpFileName){
    //    return false;
    //}

    bool DeleteFile(char const* sFileName){
        return false;
    }

    //BOOL CloseFile(LPCTSTR lpFileName){
    //    return false;
    //}
    BOOL CloseFile(HFILE* lpFileName){
        return false;
    }

    HFILE OpenFile(     LPCSTR     lpFileName,
                        LPOFSTRUCT lpReOpenBuff,
                        UINT       uStyle
                        ){
                            return -1;
                        }


    DWORD GetLastError(){
        return 0;
    }
//}