#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"

  //HANDLE
  HFILE CreateFile(  LPCSTR                lpFileName,
                        DWORD                 dwDesiredAccess,
                        DWORD                 dwShareMode,
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                        DWORD                 dwCreationDisposition,
                        DWORD                 dwFlagsAndAttributes,
                        HANDLE                hTemplateFile
    ){
        HFILE pFile = NULL;
        //CREATE_NEW - should create new file, it exists-returns error
        if(dwCreationDisposition == CREATE_NEW){
            pFile = FilesystemHelper::CreateFile(lpFileName, "wb");
        }
        //CREATE_ALWAYS  - should create new file. if exists- rewrites, 
        //drops attributes and merge dwFlagsAndAttributes and FILE_ATTRIBUTE_ARCHIVE but don't use SECURITY_ATTRIBUTES.
        else if(dwCreationDisposition == CREATE_ALWAYS){
            pFile = FilesystemHelper::CreateFile(lpFileName, "w+b");
        }
        //OPEN_EXISTING - opens file. if not exists->error
        else if(dwCreationDisposition == OPEN_EXISTING){
            pFile = FilesystemHelper::CreateFile(lpFileName, "rb");
        }
        //OPEN_ALWAYS - opens file if exists. in not-> creates, like with CREATE_NEW.
        else if (dwCreationDisposition == OPEN_ALWAYS){
            pFile = FilesystemHelper::CreateFile(lpFileName, "w");
        } 
        //TRUNCATE_EXISTING - opens existing file and cuts it, so size would be 0
        // calling process should open file with GENERIC_WRITE rights,
        // returns error if file not existing
        else if(dwCreationDisposition == TRUNCATE_EXISTING){
            pFile = FilesystemHelper::CreateFile(lpFileName, "w+b");
        }else
        {
            //error
            pFile = NULL;
        }
        //modes could be r,w,a,r+,w+,a+ 
        return pFile;
    }

    BOOL WriteFile(     //HANDLE       hFile,
                        HFILE        hFile,
                        LPCVOID      lpBuffer,
                        DWORD        nNumberOfBytesToWrite,
                        LPDWORD      lpNumberOfBytesWritten,
                        LPOVERLAPPED lpOverlapped
                        ){
                            *lpNumberOfBytesWritten = FilesystemHelper::WriteToFile(hFile, lpBuffer, nNumberOfBytesToWrite);
                            return (*lpNumberOfBytesWritten > 0);
                        }

    //BOOL DeleteFile(LPCTSTR lpFileName){
    //    return false;
    //}

    bool DeleteFile(char const* sFileName){
        int res = FilesystemHelper::DeleteFile(sFileName);
    }

    //BOOL CloseFile(LPCTSTR lpFileName){
    //    return false;
    //}
    BOOL CloseFile(HFILE* lpFileName){
        if(lpFileName){
            return FilesystemHelper::CloseFile(*lpFileName) == FilesystemHelper::FILEHELPER_NO_ERROR;
        }else{
            return false;
        }
    }

    HFILE OpenFile(     LPCSTR     lpFileName,
                        LPOFSTRUCT lpReOpenBuff,
                        UINT       uStyle
                        ){
                            return NULL;
                        }


    DWORD GetLastError(){
        return 0;
    }
//}