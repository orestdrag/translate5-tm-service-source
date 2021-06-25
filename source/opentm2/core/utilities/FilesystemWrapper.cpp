#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include <cstring>

char * filesystem_get_otm_dir() {
    std::string sDir = FilesystemHelper::GetOtmDir();
    char * pdir = (char *)malloc(sDir.size() + 1);
    if (pdir)
        strcpy(pdir, sDir.c_str());
    return pdir;
}

char* filesystem_get_home_dir() {
    std::string sDir = FilesystemHelper::GetHomeDir();
    char *pDir = (char *)malloc(sDir.size() + 1);
    if (pDir)
        strcpy(pDir, sDir.c_str());
    return pDir;
}

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


    BOOL ReadFile(      HFILE        hFile,
                        LPVOID       lpBuffer,
                        DWORD        nNumberOfBytesToRead,
                        LPDWORD      lpNumberOfBytesRead,
                        LPOVERLAPPED lpOverlapped
                        ){
                            int bytesRead = 0;
                            int retCode = FilesystemHelper::ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,bytesRead);
                            *lpNumberOfBytesRead = bytesRead;
                            return retCode == FilesystemHelper::FILEHELPER_NO_ERROR;
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
        return FilesystemHelper::GetLastError();
    }

    int GetFileSize(HFILE file){
        return FilesystemHelper::GetFileSize(file);
    }
//}