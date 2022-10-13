#ifndef _OSWRAPPER_H_
#define _OSWRAPPER_H_


#include "win_types.h"
#include "ThreadingWrapper.h"

//typedef unsigned int DWORD;
typedef DWORD LCID;
typedef DWORD LCTYPE;
typedef char* LPSTR;

typedef struct _SECURITY_ATTRIBUTES {   
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
    } SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

enum LCTYPES
    {
    LOCALE_NOUSEROVERRIDE = 0x80000000,
    LOCALE_USE_CP_ACP = 0x40000000,
    LOCALE_RETURN_NUMBER = 0x20000000,

    LOCALE_ILANGUAGE = 0x00000001,
    LOCALE_SLANGUAGE = 0x00000002,
    LOCALE_SENGLANGUAGE = 0x00001001,
    LOCALE_SABBREVLANGNAME = 0x00000003,
    LOCALE_SNATIVELANGNAME = 0x00000004,

    LOCALE_ICOUNTRY = 0x00000005,
    LOCALE_SCOUNTRY = 0x00000006,
    LOCALE_SENGCOUNTRY = 0x00001002,
    LOCALE_SABBREVCTRYNAME = 0x00000007,
    LOCALE_SNATIVECTRYNAME = 0x00000008,
    LOCALE_IGEOID = 0x0000005B,

    LOCALE_IDEFAULTLANGUAGE = 0x00000009,
    LOCALE_IDEFAULTCOUNTRY = 0x0000000A,
    LOCALE_IDEFAULTCODEPAGE = 0x0000000B,
    LOCALE_IDEFAULTANSICODEPAGE = 0x00001004,
    LOCALE_IDEFAULTMACCODEPAGE = 0x00001011,

    LOCALE_SLIST = 0x0000000C,
    LOCALE_IMEASURE = 0x0000000D,

    LOCALE_SDECIMAL = 0x0000000E,
    LOCALE_STHOUSAND = 0x0000000F,
    LOCALE_SGROUPING = 0x00000010,
    LOCALE_IDIGITS = 0x00000011,
    LOCALE_ILZERO = 0x00000012,
    LOCALE_INEGNUMBER = 0x00001010,
    LOCALE_SNATIVEDIGITS = 0x00000013,

    LOCALE_SCURRENCY = 0x00000014,
    LOCALE_SINTLSYMBOL = 0x00000015,
    LOCALE_SMONDECIMALSEP = 0x00000016,
    LOCALE_SMONTHOUSANDSEP = 0x00000017,
    LOCALE_SMONGROUPING = 0x00000018,
    LOCALE_ICURRDIGITS = 0x00000019,
    LOCALE_IINTLCURRDIGITS = 0x0000001A,
    LOCALE_ICURRENCY = 0x0000001B,
    LOCALE_INEGCURR = 0x0000001C,

    LOCALE_SDATE = 0x0000001D,
    LOCALE_STIME = 0x0000001E,
    LOCALE_SSHORTDATE = 0x0000001F,
    LOCALE_SLONGDATE = 0x00000020,
    LOCALE_STIMEFORMAT = 0x00001003,
    LOCALE_IDATE = 0x00000021,
    LOCALE_ILDATE = 0x00000022,
    LOCALE_ITIME = 0x00000023,
    LOCALE_ITIMEMARKPOSN = 0x00001005,
    LOCALE_ICENTURY = 0x00000024,
    LOCALE_ITLZERO = 0x00000025,
    LOCALE_IDAYLZERO = 0x00000026,
    LOCALE_IMONLZERO = 0x00000027,
    LOCALE_S1159 = 0x00000028,
    LOCALE_S2359 = 0x00000029,

    LOCALE_ICALENDARTYPE = 0x00001009,
    LOCALE_IOPTIONALCALENDAR = 0x0000100B,
    LOCALE_IFIRSTDAYOFWEEK = 0x0000100C,
    LOCALE_IFIRSTWEEKOFYEAR = 0x0000100D,

    LOCALE_SDAYNAME1 = 0x0000002A,
    LOCALE_SDAYNAME2 = 0x0000002B,
    LOCALE_SDAYNAME3 = 0x0000002C,
    LOCALE_SDAYNAME4 = 0x0000002D,
    LOCALE_SDAYNAME5 = 0x0000002E,
    LOCALE_SDAYNAME6 = 0x0000002F,
    LOCALE_SDAYNAME7 = 0x00000030,
    LOCALE_SABBREVDAYNAME1 = 0x00000031,
    LOCALE_SABBREVDAYNAME2 = 0x00000032,
    LOCALE_SABBREVDAYNAME3 = 0x00000033,
    LOCALE_SABBREVDAYNAME4 = 0x00000034,
    LOCALE_SABBREVDAYNAME5 = 0x00000035,
    LOCALE_SABBREVDAYNAME6 = 0x00000036,
    LOCALE_SABBREVDAYNAME7 = 0x00000037,
    LOCALE_SMONTHNAME1 = 0x00000038,
    LOCALE_SMONTHNAME2 = 0x00000039,
    LOCALE_SMONTHNAME3 = 0x0000003A,
    LOCALE_SMONTHNAME4 = 0x0000003B,
    LOCALE_SMONTHNAME5 = 0x0000003C,
    LOCALE_SMONTHNAME6 = 0x0000003D,
    LOCALE_SMONTHNAME7 = 0x0000003E,
    LOCALE_SMONTHNAME8 = 0x0000003F,
    LOCALE_SMONTHNAME9 = 0x00000040,
    LOCALE_SMONTHNAME10 = 0x00000041,
    LOCALE_SMONTHNAME11 = 0x00000042,
    LOCALE_SMONTHNAME12 = 0x00000043,
    LOCALE_SMONTHNAME13 = 0x0000100E,
    LOCALE_SABBREVMONTHNAME1 = 0x00000044,
    LOCALE_SABBREVMONTHNAME2 = 0x00000045,
    LOCALE_SABBREVMONTHNAME3 = 0x00000046,
    LOCALE_SABBREVMONTHNAME4 = 0x00000047,
    LOCALE_SABBREVMONTHNAME5 = 0x00000048,
    LOCALE_SABBREVMONTHNAME6 = 0x00000049,
    LOCALE_SABBREVMONTHNAME7 = 0x0000004A,
    LOCALE_SABBREVMONTHNAME8 = 0x0000004B,
    LOCALE_SABBREVMONTHNAME9 = 0x0000004C,
    LOCALE_SABBREVMONTHNAME10 = 0x0000004D,
    LOCALE_SABBREVMONTHNAME11 = 0x0000004E,
    LOCALE_SABBREVMONTHNAME12 = 0x0000004F,
    LOCALE_SABBREVMONTHNAME13 = 0x0000100F,

    LOCALE_SPOSITIVESIGN = 0x00000050,
    LOCALE_SNEGATIVESIGN = 0x00000051,
    LOCALE_IPOSSIGNPOSN = 0x00000052,
    LOCALE_INEGSIGNPOSN = 0x00000053,
    LOCALE_IPOSSYMPRECEDES = 0x00000054,
    LOCALE_IPOSSEPBYSPACE = 0x00000055,
    LOCALE_INEGSYMPRECEDES = 0x00000056,
    LOCALE_INEGSEPBYSPACE = 0x00000057,

    LOCALE_FONTSIGNATURE = 0x00000058,
    LOCALE_SISO639LANGNAME = 0x00000059,
    LOCALE_SISO3166CTRYNAME = 0x0000005A,

    LOCALE_IDEFAULTEBCDICCODEPAGE = 0x00001012,
    LOCALE_IPAPERSIZE = 0x0000100A,
    LOCALE_SENGCURRNAME = 0x00001007,
    LOCALE_SNATIVECURRNAME = 0x00001008,
    LOCALE_SYEARMONTH = 0x00001006,
    LOCALE_SSORTNAME = 0x00001013,
    LOCALE_IDIGITSUBSTITUTION = 0x00001014,
    LOCALE_USER_DEFAULT = 0x0400
};


int GetLocaleInfo(
    LCID   Locale,
    LCTYPE LCType,
    LPSTR  lpLCData,
    int    cchData
);

UINT GetOEMCP();

unsigned long int _ttol(const char* source);

int strupr(char * str);
int _strcmp(const char* a, const char* b);
int _strcmpi(const char* a, const char* b);
//int _stricmp(const char* a, const char* b);

void GetSystemTime(LPSYSTEMTIME lpSystemTime);

BOOL SystemTimeToFileTime(
    const SYSTEMTIME *lpSystemTime,
    LPFILETIME       lpFileTime
);

BOOL FileTimeToSystemTime(
    const FILETIME *lpFileTime,
    LPSYSTEMTIME   lpSystemTime
);

HANDLE OpenMutex(
    DWORD dwDesiredAccess,  // access flag
    BOOL bInheritHandle,    // inherit flag
    LPCTSTR lpName          // pointer to mutex-object name
);

DWORD WaitForSingleObject(
    HANDLE hHandle,
    DWORD  dwMilliseconds
);

HANDLE CreateMutex(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL                  bInitialOwner,
    LPCSTR                lpName
);

BOOL ReleaseMutex(
    HANDLE hMutex
);

BOOL FindClose(
    HANDLE hFindFile
);

/* Begin compatibility fixes required for Win8/VS2012 RTM sal.h */
#ifndef _SAL_L_Source_

// Some annotations aren't officially SAL2 yet.
#if _USE_ATTRIBUTES_FOR_SAL /*IFSTRIP=IGN*/
#define _SAL_L_Source_(Name, args, annotes) _SA_annotes3(SAL_name, #Name, "", "2") _Group_(annotes _SAL_nop_impl_)
#else
#define _SAL_L_Source_(Name, args, annotes) _SA_annotes3(SAL_name, #Name, "", "2") _GrouP_(annotes _SAL_nop_impl_)
#endif

#endif

// NLS APIs allow strings to be specified either by an element count or
// null termination. Unlike _In_reads_or_z_, this is not whichever comes
// first, but based on whether the size is negative or not.
#define _In_NLS_string_(size)     _SAL_L_Source_(_In_NLS_string_, (size),  _When_((size) < 0,  _In_z_)           \
                                _When_((size) >= 0, _In_reads_(size)))


//#ifndef CP_UTF8
    #define CP_UTF8 65001
    #define CP_ACP 0
    #define CP_OEMCP 1
//#endif

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
);

int MultiByteToWideChar(
    UINT                              CodePage,
    DWORD                             dwFlags,
    //_In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
    LPCSTR                             lpMultiByteStr,
    int                               cbMultiByte,
    LPWSTR                            lpWideCharStr,
    int                               cchWideChar
    );





#endif