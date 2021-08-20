#include "OSWrapper.h"
#include "LogWrapper.h"
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>

int GetLocaleInfo(
        LCID   Locale,
        LCTYPE LCType,
        LPSTR  lpLCData,
        int    cchData
    ){
        if( Locale == LOCALE_USER_DEFAULT){
            switch (LCType)
            {
            case LOCALE_IDEFAULTCODEPAGE:
                strncpy(lpLCData,"866",cchData);
                return 0;
                break;

            case LOCALE_IDEFAULTANSICODEPAGE:
                strncpy(lpLCData,"1251",cchData);
                return 0;
                break;

            default:
                LogMessage3(ERROR, "GetLocaleInfo():: LOCALE_USER_DEFAULT :: LCType {", intToA(LCType), "} is not implemented");
                break;
            }
        }else{
            LogMessage3(ERROR, "GetLocaleInfo():: Locale {", intToA(Locale), "} is not implemented");
        }    
        return -1;
    }

UINT GetOEMCP(){
    return 0;

}


unsigned long int _ttol(const char* source){
    return atoi(source);
}

/*
 int _strcmp(const char* a, const char* b){
    return strcmp(a,b);
 }
 
 int _strcmpi(const char* a, const char* b){
     return strcmp(a,b);
 }
 
 int _stricmp(const char* a, const char* b){
    return strcmp(a,b);
 }//*/ 


int strupr(char * str){
    int len = strlen(str);
    for(int i=0; i<len; i++){
        str[i] = toupper(str[i]);
    }
    return 0;
}

void GetSystemTime(LPSYSTEMTIME lpSystemTime){
    LogMessage(FATAL, "Called not implemented function::GetSystemTime");
}

BOOL SystemTimeToFileTime(
    const SYSTEMTIME *lpSystemTime,
    LPFILETIME       lpFileTime
){
    LogMessage(FATAL, "Called not implemented function::SystemTimeToFileTime");
}

BOOL FileTimeToSystemTime(
        const FILETIME *lpFileTime,
        LPSYSTEMTIME   lpSystemTime
    ){
        LogMessage(FATAL, "Called not implemented function::FileTimeToSystemTime");
    }


HANDLE OpenMutex(
  DWORD dwDesiredAccess,  // access flag
  BOOL bInheritHandle,    // inherit flag
  LPCTSTR lpName          // pointer to mutex-object name
){
    LogMessage(FATAL, "Called not implemented function::OpenMutex");
    return NULL;
}


DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
){
    LogMessage(FATAL, "Called not implemented function::WaitForSingleObject");
    return 0;
}

HANDLE CreateMutex(
        LPSECURITY_ATTRIBUTES lpMutexAttributes,
        BOOL                  bInitialOwner,
        LPCSTR                lpName
    ){
        LogMessage(FATAL, "Called not implemented function::CreateMutex");
        return NULL;
    }

BOOL ReleaseMutex(
  HANDLE hMutex
){
    LogMessage(FATAL, "Called not implemented function::ReleaseMutex");
    return true;
}

BOOL FindClose(
  HANDLE hFindFile
){
    LogMessage(FATAL, "Called not implemented function::FindClose");
}


