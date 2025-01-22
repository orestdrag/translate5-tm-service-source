#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
#include <unistd.h>
#include <cstring>
#include <unistd.h>

std::string filesystem_get_otm_dir() {
    return FilesystemHelper::GetOtmDir();
}

std::string filesystem_get_home_dir() {
    return FilesystemHelper::GetHomeDir();
}

int filesystem_open_file(const char* path, FILE*& ptr, const char* mode){
    T5LOG( T5WARNING) <<"TEMPORARY HARDCODED useBuffer= true in filesystem_open_file, fname = " << path;
    bool useBuffer = true;//false;
    if(    (strcasestr(path, ".TMI") )
        || (strcasestr(path, ".TMD") )
        //|| (strcasestr(path, ".MEM") )
    ){
        T5LOG( T5INFO) << "filesystem_open_file::Openning data file(with ext. .TMI, .TMD, .MEM => forcing to use filebuffers, fName = "<< 
                        path<< ", useFilebuffer before = " <<useBuffer;
        useBuffer = true;
    }
    ptr = FilesystemHelper::OpenFile(path, mode, useBuffer);
    return ptr == NULL;
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
                            int retCode = FilesystemHelper::ReadFileBuff(hFile,lpBuffer,nNumberOfBytesToRead,bytesRead, -1);
                            *lpNumberOfBytesRead = bytesRead;
                            return retCode == FilesystemHelper::FILEHELPER_NO_ERROR;
                        }

    BOOL WriteFile(     //HANDLE       hFile,
                        HFILE        hFile,
                        LPCVOID      lpBuffer,
                        int        nNumberOfBytesToWrite,
                        int&      riNumberOfBytesWritten,
                        LPOVERLAPPED lpOverlapped
                        ){                            
                            //*lpNumberOfBytesWritten = 
                            FilesystemHelper::WriteToFileBuff(hFile, lpBuffer, nNumberOfBytesToWrite, riNumberOfBytesWritten, -1);
                            
                            return (riNumberOfBytesWritten > 0);
                        }
    BOOL WriteFile(     //HANDLE       hFile,
                        HFILE        hFile,
                        LPCVOID      lpBuffer,
                        DWORD        nNumberOfBytesToWrite,
                        LPDWORD      lpNumberOfBytesWritten,
                        LPOVERLAPPED lpOverlapped
                        ){                            
                            int written = 0; 
                            FilesystemHelper::WriteToFileBuff(hFile, lpBuffer, nNumberOfBytesToWrite, written, -1);
                            *lpNumberOfBytesWritten = written;
                            
                            return (*lpNumberOfBytesWritten > 0);
                        }

    bool DeleteFile(char const* sFileName){
        int res = FilesystemHelper::DeleteFile(sFileName);
        return res == 0;
    }

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

    bool CreateDir(const char* path){
        return FilesystemHelper::CreateDir(path);

    }

    HFILE FindFirstFile(
         LPCTSTR lpFileName,               
         LPWIN32_FIND_DATA lpFindFileData  
        ){
            return FilesystemHelper::FindFirstFile(lpFileName);
        }

    BOOL FindNextFile(
        HANDLE             hFindFile,
        LPWIN32_FIND_DATA lpFindFileData
        ){
            FilesystemHelper::FindNextFile();
            return false;
        }


LARGE_INTEGER intToLargeInt(int i) {
    LARGE_INTEGER li;
    li.QuadPart = i;
    return li;
}

int largeIntToInt(LARGE_INTEGER li) {
    return (int)li.QuadPart;
}

#ifndef __NR__llseek
#define __NR__llseek            140
#endif


DWORD SetFilePointer(HFILE fp,LONG LoPart,LONG *HiPart,DWORD OffSet)
{
    T5LOG( T5INFO) << "SetFilePointer for file " << FilesystemHelper::GetFileName((HFILE)fp) << "; offset is " << LoPart << 
            ", direction is " << OffSet;
    LONG hipart;
    int ret = FilesystemHelper::SetFileCursor(fp, LoPart, hipart, OffSet);
    if(HiPart)
        *HiPart = hipart;
    return ret;
}

BOOL MoveFile(
  LPCTSTR lpExistingFileName, 
  LPCTSTR lpNewFileName       
){
    T5LOG( T5INFO) <<"MoveFile:: moving file from " << lpExistingFileName << " to " << lpNewFileName;
    return FilesystemHelper::MoveFile(lpExistingFileName, lpNewFileName);
}


BOOL SetFilePointerEx(
        HFILE hFile,                    
        LARGE_INTEGER liDistanceToMove,  
        PLARGE_INTEGER lpNewFilePointer, 
        DWORD dwMoveMethod               
        ){
            if(liDistanceToMove.QuadPart == 0){
                return true;
            }
            T5LOG(T5FATAL) << "calling not implemented function SetFilePointerEx for file " << FilesystemHelper::GetFileName(hFile);
            //int h = GetFileId( hFile);
            //PVOID ph = &h;
            LONG a, *b, c;
            a = (LONG)liDistanceToMove.QuadPart;
            if(lpNewFilePointer){
                c = lpNewFilePointer->QuadPart;
                b = &c;
            }else{
                b = NULL;
            }
            SetFilePointer( hFile, a, b, dwMoveMethod  );

            if(lpNewFilePointer)
                lpNewFilePointer->QuadPart = *b;
            return false;
        }


        void CopyFilePathReplaceExt(char* dest, const char* src, const char* new_ext){
            std::string str(src);
            int pos = str.rfind('.');
            if(pos>0)
                str = str.substr(0, pos);
            str+= new_ext;

            strcpy(dest, str.c_str());
        }

        int GetFileId(HFILE ptr){
            int fno = fileno(ptr);
            //sprintf(proclnk, "/proc/self/fd/%d", fno);
            T5LOG( T5INFO) << "GetFileId(" << (long int) ptr << ") = " << fno << ", fname = " << FilesystemHelper::GetFileName(ptr);
            return fno;
        }


        int SkipBytesFromBeginningInFile(HFILE ptr, int numOfBytes){
            T5LOG( T5INFO) << "SkipBytesFromBeginningInFile, file = " << FilesystemHelper::GetFileName(ptr) <<
                        "; bytes = " << numOfBytes;
            int err = 0;
            if(ptr == NULL){
                return -1;
            }

            if(numOfBytes < 0){
                return -1;
            }

            LONG readed = 0;
            if(!FilesystemHelper::SetFileCursor(ptr, numOfBytes, readed, FILE_BEGIN)){
                readed = numOfBytes;
            }
            
            FilesystemHelper::SetOffsetInFilebuffer(ptr,numOfBytes);

            return readed == 0 && numOfBytes != 0;
        }

        int TruncateFileForBytes(HFILE ptr, int numOfBytes){
            return FilesystemHelper::TruncateFileForBytes(ptr, numOfBytes);
        }
//}

