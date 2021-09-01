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
    return mbsnrtowcs(lpWideCharStr, (const char**)&lpMultiByteStr, cbMultiByte, cchWideChar, &state );
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
        mbstate_t state;
        int cnt = 0;
        memset (&state, '\0', sizeof (state));

        return wcsnrtombs(lpMultiByteStr, (const wchar_t**)&lpWideCharStr, cchWideChar, cbMultiByte, &state );
        
        //int ret = wcstombs(lpMultiByteStr, lpWideCharStr, cchWideChar);
        //return ret;
            #ifdef TEMPORARY_COMMENTED
            char   *inptr;  /* Pointer used for input buffer  */
            char   *outptr; /* Pointer used for output buffer */
                                                        /* input buffer */
            iconv_t          cd;     /* conversion descriptor          */
            size_t           inleft = cchWideChar; /* number of bytes left in inbuf  */
            size_t           outleft = cbMultiByte;/* number of bytes left in outbuf */
            int              rc;     /* return code of iconv()         */


            if ((cd = iconv_open("IBM-037", "IBM-1047")) == (iconv_t)(-1)) {
                fprintf(stderr, "Cannot open converter from %s to %s\n",
                                                    "IBM-1047", "IBM-037");
                exit(8);
            }
            inptr = inbuf;
            outptr = (char*)outbuf;

            rc = iconv(cd, &inptr, &inleft, &outptr, &outleft);
            if (rc == -1) {
                fprintf(stderr, "Error in converting characters\n");
                exit(8);
            }
            iconv_close(cd);
            #endif
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


