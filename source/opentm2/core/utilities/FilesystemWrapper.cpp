#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
#include <unistd.h>
#include <cstring>
#include <unistd.h>

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

int filesystem_open_file(const char* path, FILE*& ptr, const char* mode){
    LogMessage2(WARNING,"TEMPORARY HARDCODED useBuffer= true in filesystem_open_file, fname = ", path);
    bool useBuffer = true;//false;
    if(    (strcasestr(path, ".TMI") )
        || (strcasestr(path, ".TMD") )
        || (strcasestr(path, ".MEM") )
    ){
        LogMessage4(INFO, "filesystem_open_file::Openning data file(with ext. .TMI, .TMD, .MEM => forcing to use filebuffers, fName = ", 
                        path, ", useFilebuffer before = ", intToA(useBuffer));
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

    BOOL FlushFileBuffers( HANDLE hFile){
        return true;
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


    //BOOL DeleteFile(LPCTSTR lpFileName){
    //    return false;
    //}

    bool DeleteFile(char const* sFileName){
        int res = FilesystemHelper::DeleteFile(sFileName);
        return res == 0;
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

    BOOL CloseHandle(HFILE hf ){
        FilesystemHelper::CloseFile(hf);
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

/*
DWORD SetFilePointer(HANDLE fp,LONG LoPart,LONG *HiPart,DWORD OffSet)
{
    DWORD ret = 0 ;

    LogMessage6(INFO, "SetFilePointer for file ", FilesystemHelper::GetFileName((HFILE)fp).c_str(), "; offset is ", LoPart, ", direction is ", OffSet);
    //loff_t res ; //It is also LONGLONG variable.
    LONGLONG res ; //It is also LONGLONG variable. LONG is long :)

    unsigned int whence = 0 ;
    
    if(OffSet == FILE_BEGIN)
        whence = SEEK_SET ;
    if(OffSet == FILE_CURRENT)
        whence = SEEK_CUR ;
    if(OffSet == FILE_END)
        whence = SEEK_END ;

    if(HiPart == NULL)
        *HiPart = 0 ;

    ret = syscall(__NR__llseek, fp, *HiPart, LoPart, &res, whence) ; //syscall2

    int k = errno ;
    if( ret == 0 )
    {
        LARGE_INTEGER li ;
        li.QuadPart = res ; //It will move High & Low Order bits.
        ret = li.LowPart ;

        if(*HiPart != 0) //If High Order is not NULL
        {
            *HiPart = li.HighPart ;
        }
    }else{
        ret = -1 ;

        switch(errno){
            case EBADF : 
                LogMessage2(ERROR, "fd is not an open file descriptor. fname = ", ""
                //FilesystemHelper::GetFileName(fp).c_str()
                ) ;
            break ;
            case EFAULT : 
                 LogMessage2(ERROR,"Problem with copying results to user space, fname =  ", ""
                 //FilesystemHelper::GetFileName(fp).c_str() 
                 ) ;
                break ;
            case EINVAL : 
                 LogMessage2(ERROR,"cw whence is invalid, fname = ", ""
                 //FilesystemHelper::GetFileName(fp).c_str() 
                 ) ;
                break ;
            default:
                 LogMessage2(ERROR," default, fname = ", "" 
                 //FilesystemHelper::GetFileName(fp).c_str() 
                 );
        }

    }

    return ret ;
}
//*/

DWORD SetFilePointer(HFILE fp,LONG LoPart,LONG *HiPart,DWORD OffSet)
{
    LogMessage6(INFO, "SetFilePointer for file ", FilesystemHelper::GetFileName((HFILE)fp).c_str(), "; offset is ", intToA(LoPart), ", direction is ", intToA(OffSet));
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
    LogMessage4(INFO,"MoveFile:: moving file from ", lpExistingFileName, " to ", lpNewFileName);
    return FilesystemHelper::MoveFile(lpExistingFileName, lpNewFileName);
}

/*
    BOOL SetFilePointerEx(
        HANDLE hFile,                    
        LARGE_INTEGER liDistanceToMove,  
        PLARGE_INTEGER lpNewFilePointer, 
        DWORD dwMoveMethod               
        ){
            #if 0
            int res = 0;
            if(liDistanceToMove.QuadPart == 0){
                return true;
            }if(dwMoveMethod == FILE_BEGIN){
                
                //LogMessage(ERROR, "SetFilePointerEx::FILE_BEGIN not implemented");
                res = lseek(*((int*)hFile), largeIntToInt(liDistanceToMove), SEEK_SET);
            }else if(dwMoveMethod == FILE_CURRENT){
                //res = lseek(hFile, largeIntToInt(liDistanceToMove), SEEK_CUR);
                LogMessage(ERROR, "SetFilePointerEx::FILE_CURRENT not implemented");
            }else if(dwMoveMethod == FILE_END){
                //res = lseek(hFile, largeIntToInt(liDistanceToMove), SEEK_END);
                LogMessage(ERROR, "SetFilePointerEx::FILE_END not implemented");
            }else{

                LogMessage(FATAL, "SetFilePointerEx::WRONG dwMoveMethod");
            }
            if(lpNewFilePointer && res >= 0){
                *lpNewFilePointer = intToLargeInt(res);
            }
            return res >=0;
            #else
            LONG a, *b, c;
            int h = GetFileId((HFILE) hd);
            a = (LONG)liDistanceToMove.QuadPart;
            if(lpNewFilePointer){
                c = lpNewFilePointer->QuadPart;
                b = &c;
            }else{
                b = NULL;
            }
            return SetFilePointer( hFile, a, b, dwMoveMethod  );

            if(lpNewFilePointer)
                lpNewFilePointer->QuadPart = *b;
            #endif
        }
//*/

//*
BOOL SetFilePointerEx(
        HFILE hFile,                    
        LARGE_INTEGER liDistanceToMove,  
        PLARGE_INTEGER lpNewFilePointer, 
        DWORD dwMoveMethod               
        ){
            if(liDistanceToMove.QuadPart == 0){
                return true;
            }
            LogMessage2(FATAL, "calling not implemented function SetFilePointerEx for file ", FilesystemHelper::GetFileName(hFile).c_str());
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
//*/

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
            LogMessage6(INFO, "GetFileId(", intToA((long int) ptr),") = ", intToA(fno), ", fname = ", FilesystemHelper::GetFileName(ptr).c_str());
            return fno;
        }


        int SkipBytesFromBeginningInFile(HFILE ptr, int numOfBytes){
            LogMessage4(INFO, "SkipBytesFromBeginningInFile, file = ", FilesystemHelper::GetFileName(ptr).c_str(),
                        "; bytes = ", intToA(numOfBytes));
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


int filesystem_flush_buffers(const char* fname, bool tempFile){
    return FilesystemHelper::WriteBuffToFile(fname, tempFile);
}

int filesystem_flush_buffers_ptr(HFILE file, bool tempFile){
    std::string fName = FilesystemHelper::GetFileName(file);
    return FilesystemHelper::WriteBuffToFile(fName.c_str(), tempFile);
}
