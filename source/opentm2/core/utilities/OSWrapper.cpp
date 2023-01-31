#include "OSWrapper.h"
#include "LogWrapper.h"
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <wchar.h>
#include <iconv.h>
#include <ctype.h>
#include <string>
#include "EncodingHelper.h"

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
                LogMessage(T5ERROR, "GetLocaleInfo():: LOCALE_USER_DEFAULT :: LCType {", toStr(LCType).c_str(), "} is not implemented");
                break;
            }
        }else{
            LogMessage(T5ERROR, "GetLocaleInfo():: Locale {", toStr(Locale).c_str(), "} is not implemented");
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

int MultiByteToWideChar(
  UINT                              CodePage,
  DWORD                             dwFlags,
  //_In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
  LPCSTR                             lpMultiByteStr,
  int                               cbMultiByte,
  LPWSTR                            lpWideCharStr,
  int                               cchWideChar
){
    mbstate_t state;
    int cnt = 0;
    memset (&state, '\0', sizeof (state));
    //return mbsnrtowcs(lpMultiByteStr, lpWideCharStr, cchWideChar, cbMultiByte, &state );
    if(cbMultiByte<=0){ 
        cbMultiByte = strlen(lpMultiByteStr);        
    }
    if(cbMultiByte > cchWideChar-1) cbMultiByte = cchWideChar-1;
    lpWideCharStr[cbMultiByte] = 0;

    return mbsnrtowcs(lpWideCharStr, (const char**)&lpMultiByteStr, cbMultiByte, cchWideChar, &state );
    //EncodingHelper::convertToUTF16(lpMultiByteStr);
    //return mbtowc(lpWideCharStr, lpMultiByteStr, cchWideChar);
}

int WideCharToMultiByte(
        UINT                               CodePage,
        DWORD                              dwFlags,
        //_In_NLS_string_(cchWideChar)LPCWCH lpWideCharStr,
        LPWSTR                             lpWideCharStr,
        int                                cchWideChar,
        LPSTR                              lpMultiByteStr,
        int                                cbMultiByte,
        //LPCCH
        char*                              lpDefaultChar,
        LPBOOL                             lpUsedDefaultChar
    ){
        //mbstate_t state;
        //int cnt = 0;
        //memset (&state, '\0', sizeof (state));

        if(cchWideChar<=0){ 
            cchWideChar = wcslen(lpWideCharStr);            
        }
        if(cchWideChar > cbMultiByte-1) cchWideChar = cbMultiByte-1;
        lpMultiByteStr[cchWideChar] = 0;
        return wcstombs(lpMultiByteStr, (const wchar_t*) &lpWideCharStr, cchWideChar);
        //return wcsnrtombs(lpMultiByteStr, (const wchar_t**)&lpWideCharStr, cchWideChar, cbMultiByte, &state );       
        
    }

void GetSystemTime(LPSYSTEMTIME lpSystemTime){
    LogMessage(T5FATAL, "Called not implemented function::GetSystemTime");
}

BOOL SystemTimeToFileTime(
    const SYSTEMTIME *lpSystemTime,
    LPFILETIME       lpFileTime
){
    LogMessage(T5FATAL, "Called not implemented function::SystemTimeToFileTime");
}

BOOL FileTimeToSystemTime(
        const FILETIME *lpFileTime,
        LPSYSTEMTIME   lpSystemTime
    ){
        LogMessage(T5FATAL, "Called not implemented function::FileTimeToSystemTime");
    }


HANDLE OpenMutex(
  DWORD dwDesiredAccess,  // access flag
  BOOL bInheritHandle,    // inherit flag
  LPCTSTR lpName          // pointer to mutex-object name
){
    LogMessage(T5FATAL, "Called not implemented function::OpenMutex");
    return NULL;
}


DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
){
    LogMessage(T5FATAL, "Called not implemented function::WaitForSingleObject");
    return 0;
}

HANDLE CreateMutex(
        LPSECURITY_ATTRIBUTES lpMutexAttributes,
        BOOL                  bInitialOwner,
        LPCSTR                lpName
    ){
        LogMessage(T5FATAL, "Called not implemented function::CreateMutex");
        return NULL;
    }

BOOL ReleaseMutex(
  HANDLE hMutex
){
    LogMessage(T5FATAL, "Called not implemented function::ReleaseMutex");
    return true;
}

BOOL FindClose(
  HANDLE hFindFile
){
    LogMessage(T5FATAL, "Called not implemented function::FindClose");
}


