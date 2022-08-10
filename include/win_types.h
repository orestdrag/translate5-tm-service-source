/*
    Windows Data Types
    https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types
*/

#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#ifndef WIN_TYPES_H
#define WIN_TYPES_H

#include <cstdio>
#include <strings.h>

#define WINAPI /*__stdcall*/
#define APIENTRY WINAPI
#define EXPENTRY WINAPI
#define CALLBACK /*__stdcall*/
#define CONST const
#define VOID void
#define far 
#define FAR 
#define NEAR 
#define PASCAL 
#define pascal
#define _loadds

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



//CHANGE BASIC TYPE OF STRINGS HERE
typedef char TM_CHAR;
typedef TM_CHAR * PTM_CHAR;

//typedef unsigned short TMWCHAR;
typedef wchar_t TMWCHAR;
typedef TMWCHAR * PTMWCHAR;
typedef TMWCHAR WCHAR;


typedef char CHAR;
typedef char CCHAR;

typedef CHAR *LPSTR;
typedef CONST CHAR *LPCSTR;

#ifdef UNICODE
    typedef WCHAR TCHAR;
    typedef WCHAR TBYTE;
    typedef LPCWSTR PCTSTR;
    typedef LPCWSTR LPCTSTR; 
    typedef LPWSTR LPTSTR;
    typedef LPWSTR PTSTR;
#else
    typedef char TCHAR;
    typedef unsigned char TBYTE;
    typedef LPCSTR PCTSTR;
    typedef LPCSTR LPCTSTR;
    typedef LPSTR LPTSTR;
    typedef LPSTR PTSTR;
#endif

typedef unsigned short WORD;
typedef WORD ATOM;
typedef WORD LANGID;

typedef unsigned char BYTE;
typedef BYTE BOOLEAN;
typedef BYTE far *LPBYTE;

typedef int BOOL;
typedef BOOL far *LPBOOL;

typedef unsigned long DWORD;
typedef DWORD COLORREF;
typedef DWORD LCID;
typedef DWORD LCTYPE;
typedef DWORD LGRPID;
typedef DWORD *LPCOLORREF;

typedef unsigned int DWORD32;
typedef unsigned long long DWORD64;

typedef int INT;
typedef long long INT_PTR;
typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;
typedef signed long long INT64;

typedef int *PINT;
typedef INT_PTR *PINT_PTR;
typedef INT8 *PINT8;
typedef INT16 *PINT16;
typedef INT32 *PINT32;
typedef INT64 *PINT64;

typedef unsigned int UINT;
typedef unsigned int UINT_PTR;
typedef UINT_PTR *PUINT_PTR;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

typedef int *LPINT;
typedef UINT *PUINT;
typedef UINT8 *PUINT8;
typedef UINT16 *PUINT16;
typedef UINT32 *PUINT32;
typedef UINT64 *PUINT64;
typedef UINT_PTR WPARAM;

typedef long LONG;
typedef signed int LONG32;
typedef long long LONG64;

typedef LONG HRESULT;

typedef long long LONGLONG;
typedef long long LONG_PTR;
typedef LONG_PTR LPARAM;
typedef long *LPLONG;
typedef LONG *PLONG;
typedef LONGLONG *PLONGLONG;
typedef LONG_PTR *PLONG_PTR;
typedef LONG32 *PLONG32;
typedef LONG64 *PLONG64;

typedef unsigned long long ULONG64;
typedef unsigned long long ULONGLONG;
typedef unsigned long long ULONG_PTR;

typedef unsigned long long DWORDLONG;
typedef ULONG_PTR DWORD_PTR;

typedef float FLOAT;

typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE LHANDLE;
typedef HANDLE HACCEL;
typedef HANDLE HBITMAP;
typedef HANDLE HBRUSH;
typedef HANDLE HCOLORSPACE;
typedef HANDLE HCONV;
typedef HANDLE HCONVLIST;
typedef HANDLE HDC;
typedef HANDLE HDDEDATA;
typedef HANDLE HDESK;
typedef HANDLE HDROP;
typedef HANDLE HDWP;
typedef HANDLE HENHMETAFILE;
typedef HANDLE HFONT;
typedef HANDLE HGDIOBJ;
typedef HANDLE HGLOBAL;
typedef HANDLE HHOOK;
typedef HANDLE HICON;
typedef HANDLE HINSTANCE;
typedef HANDLE HKEY;
typedef HANDLE HKL;
typedef HANDLE HLOCAL;
typedef HANDLE HMENU;
typedef HANDLE HMETAFILE;
typedef HANDLE HPALETTE;
typedef HANDLE HPEN;
typedef HANDLE HRGN;
typedef HANDLE HRSRC;
typedef HANDLE HSZ;
typedef HANDLE WINSTA;
typedef HANDLE HWND;

typedef HINSTANCE HMODULE;

//typedef int HFILE;
typedef FILE *HFILE;

typedef int HALF_PTR;

typedef HICON HCURSOR;

typedef CONST void *LPCVOID;
typedef CONST WCHAR *LPCWSTR;
typedef DWORD *LPDWORD;
typedef HANDLE *LPHANDLE;

typedef void *LPVOID;
typedef WORD *LPWORD;
typedef WCHAR *LPWSTR;
typedef LONG_PTR LRESULT;
typedef BOOL *PBOOL;
typedef BOOLEAN *PBOOLEAN;
typedef BYTE *PBYTE;
typedef CHAR *PCHAR;
typedef CONST CHAR *PCSTR;

typedef CONST WCHAR *PCWSTR;
typedef DWORD *PDWORD;
typedef DWORDLONG *PDWORDLONG;
typedef DWORD_PTR *PDWORD_PTR;
typedef DWORD32 *PDWORD32;
typedef DWORD64 *PDWORD64;
typedef FLOAT *PFLOAT;

typedef HALF_PTR *PHALF_PTR;

typedef HANDLE *PHANDLE;
typedef HKEY *PHKEY;
typedef PDWORD PLCID;
typedef short SHORT;
typedef SHORT *PSHORT;
typedef ULONG_PTR SIZE_T;
typedef LONG_PTR SSIZE_T;
typedef SIZE_T *PSIZE_T;
typedef SSIZE_T *PSSIZE_T;
typedef CHAR *PSTR;
typedef TBYTE *PTBYTE;
typedef TCHAR *PTCHAR;

typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;

typedef unsigned int UHALF_PTR;
typedef UHALF_PTR *PUHALF_PTR;

typedef unsigned long ULONG;
//typedef unsigned int ULONG;
typedef ULONG *PULONG;
typedef ULONGLONG *PULONGLONG;
typedef ULONG_PTR *PULONG_PTR;
typedef unsigned int ULONG32;
typedef ULONG32 *PULONG32;
typedef ULONG64 *PULONG64;
typedef unsigned short USHORT;
//typedef unsigned int USHORT;
typedef USHORT *PUSHORT;
typedef WCHAR *PWCHAR;
typedef WORD *PWORD;
typedef WCHAR *PWSTR;
typedef unsigned long long QWORD;
typedef HANDLE SC_HANDLE;
typedef LPVOID SC_LOCK;
typedef HANDLE SERVICE_STATUS_HANDLE;

/*
typedef struct _UNICODE_STRING {
    USHORT  Length;
    USHORT  MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
*/
typedef LONGLONG USN;

#define __declspec(dllexport) /* __attribute__((visibility("default"))) */

/* https://docs.microsoft.com/en-us/previous-versions/ms915433(v=msdn.10) */

#define LF_FACESIZE 32

typedef struct tagLOGFONT { 
    LONG lfHeight; 
    LONG lfWidth; 
    LONG lfEscapement; 
    LONG lfOrientation; 
    LONG lfWeight; 
    BYTE lfItalic; 
    BYTE lfUnderline; 
    BYTE lfStrikeOut; 
    BYTE lfCharSet; 
    BYTE lfOutPrecision ; 
    BYTE lfClipPrecision ; 
    BYTE lfQuality ; 
    BYTE lfPitchAndFamily ; 
    TCHAR lfFaceName[LF_FACESIZE] ; 
} LOGFONT, *PLOGFONT;

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
    DWORD  lPrivate;
} MSG, *PMSG, *NPMSG, *LPMSG;

typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct _SIZE {
  LONG cx;
  LONG cy;
} SIZEL, *PSIZEL, *LPSIZEL;

typedef struct _POINTL {
  LONG x;
  LONG y;
} POINTL, *PPOINTL;

typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef struct _RECTL {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECTL, *PRECTL, *LPRECTL;

typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

#define MAX_PATH 260

typedef struct _WIN32_FIND_DATAA {
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  CHAR     cFileName[MAX_PATH];
  CHAR     cAlternateFileName[14];
  DWORD    dwFileType;
  DWORD    dwCreatorType;
  WORD     wFinderFlags;
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

typedef HANDLE HGLOBAL;
typedef DWORD LCID;

typedef struct _charformat2 {
    UINT  cbSize;
    DWORD  dwMask;
    DWORD  dwEffects;
    LONG  yHeight;
    LONG  yOffset;
    COLORREF  crTextColor;
    BYTE  bCharSet;
    BYTE  bPitchAndFamily;
    TCHAR  szFaceName[LF_FACESIZE];
    WORD  wWeight;
    SHORT  sSpacing;
    COLORREF  crBackColor;
    LCID  lcid;
    DWORD  dwReserved;
    SHORT  sStyle;
    WORD  wKerning;
    BYTE  bUnderlineType;
    BYTE  bAnimation;
    BYTE  bRevAuthor;
    BYTE  bReserved1;	
} CHARFORMAT2;

typedef struct _charformat2w {
  UINT     cbSize;
  DWORD    dwMask;
  DWORD    dwEffects;
  LONG     yHeight;
  LONG     yOffset;
  COLORREF crTextColor;
  BYTE     bCharSet;
  BYTE     bPitchAndFamily;
  WCHAR    szFaceName[LF_FACESIZE];
  WORD     wWeight;
  SHORT    sSpacing;
  COLORREF crBackColor;
  LCID     lcid;
  union {
    //DWORD dwReserved;
    DWORD dwCookie;
  };
  DWORD    dwReserved;
  SHORT    sStyle;
  WORD     wKerning;
  BYTE     bUnderlineType;
  BYTE     bAnimation;
  BYTE     bRevAuthor;
  BYTE     bUnderlineColor;
} CHARFORMAT2W;

/* https://docs.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamea */

#define OFN_NOCHANGEDIR        0x00000008
#define OFN_PATHMUSTEXIST      0x00000800
#define OFN_NOTESTFILECREATE   0x00010000
#define OFN_EXPLORER           0x00080000
#define OFN_NODEREFERENCELINKS 0x00100000
#define OFN_LONGNAMES          0x00200000
#define OFN_ENABLESIZING       0x00800000


/* http://www.jasinskionline.com/WindowsApi/ref/o/openfilename.html */

typedef struct tagOFN {
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCSTR        lpstrFilter;
    LPSTR         lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPSTR         lpstrFile;
    DWORD         nMaxFile;
    LPSTR         lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCSTR        lpstrInitialDir;
    LPCSTR        lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCSTR        lpstrDefExt;
    LPARAM        lCustData;
    //LPOFNHOOKPROC lpfnHook;
//WINAPI https://docs.microsoft.com/en-us/windows/win32/api/commdlg/nc-commdlg-lpofnhookproc
    LPCSTR        lpTemplateName;
} OPENFILENAME;

/* https://docs.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamea */
typedef struct tagOFNA {
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCSTR        lpstrFilter;
    LPSTR         lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPSTR         lpstrFile;
    DWORD         nMaxFile;
    LPSTR         lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCSTR        lpstrInitialDir;
    LPCSTR        lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCSTR        lpstrDefExt;
    LPARAM        lCustData;
    //LPOFNHOOKPROC lpfnHook; //WINAPI https://docs.microsoft.com/en-us/windows/win32/api/commdlg/nc-commdlg-lpofnhookproc
    LPCSTR        lpTemplateName;
    //LPEDITMENU    lpEditInfo;
    LPCSTR        lpstrPrompt;
    void          *pvReserved;
    DWORD         dwReserved;
    DWORD         FlagsEx;
} OPENFILENAMEA, *LPOPENFILENAMEA;

/* https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmap */

typedef struct tagBITMAP {
    LONG   bmType;
    LONG   bmWidth;
    LONG   bmHeight;
    LONG   bmWidthBytes;
    WORD   bmPlanes;
    WORD   bmBitsPixel;
    LPVOID bmBits;
} BITMAP, *PBITMAP, *NPBITMAP, *LPBITMAP;

/* https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-large_integer-r1 */

typedef union _LARGE_INTEGER {
    DWORD LowPart;
    LONG  HighPart;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

/* https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-systemtime */

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

/* https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32 */

typedef struct tagPROCESSENTRY32 {
    DWORD     dwSize;
    DWORD     cntUsage;
    DWORD     th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD     th32ModuleID;
    DWORD     cntThreads;
    DWORD     th32ParentProcessID;
    LONG      pcPriClassBase;
    DWORD     dwFlags;
    CHAR      szExeFile[MAX_PATH];
} PROCESSENTRY32;

/* https://www.aldeid.com/wiki/OSVERSIONINFOEX  */

typedef struct _OSVERSIONINFOEX {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    TCHAR szCSDVersion[128];
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} OSVERSIONINFOEX, *POSVERSIONINFOEX, *LPOSVERSIONINFOEX;

/* https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointer */

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

/* https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox */

#define MB_DEFBUTTON1 0x00000000L
#define MB_DEFBUTTON2 0x00000100L
#define MB_DEFBUTTON3 0x00000200L
#define MB_DEFBUTTON4 0x00000300L

/* https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles */

#define WS_TABSTOP      0x00010000L
#define WS_GROUP        0x00020000L
#define WS_THICKFRAME   0x00040000L
#define WS_MAXIMIZEBOX  0x00010000L
#define WS_MINIMIZEBOX  0x00020000L
#define WS_SYSMENU      0x00080000L
#define WS_CAPTION      0x00C00000L
#define WS_VISIBLE      0x10000000L

#define get_max(a, b) (a > b) ? (a) : (b)
#define get_min(a, b) (a > b) ? (b) : (a)

/* https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/18d8fbe8-a967-4f1c-ae50-99ca8e491d2d */

#define ERROR_INVALID_PARAMETER     0x00000057
#define ERROR_DISK_FULL             0x00000070
#define ERROR_INVALID_ACCESS        0x0000000C
#define ERROR_READ_FAULT            0x0000001E
#define ERROR_INVALID_ACCESS        0x0000000C
#define ERROR_NO_MORE_FILES         0x00000012
#define ERROR_FILE_NOT_FOUND        0x00000002
#define ERROR_INVALID_DATA          0x0000000D
#define ERROR_INVALID_NAME          0x0000007B
#define ERROR_MOD_NOT_FOUND         0x0000007E
#define ERROR_MR_MID_NOT_FOUND      0x0000013D
#define ERROR_NOT_READY             0x00000015
#define ERROR_WRITE_FAULT           0x0000001D
#define ERROR_NET_WRITE_FAULT       0x00000058
#define ERROR_INVALID_DRIVE         0x0000000F
#define ERROR_BAD_NETPATH           0x00000035
#define ERROR_BAD_NET_NAME          0x00000043
#define ERROR_DEV_NOT_EXIST         0x00000037
#define ERROR_SHARING_VIOLATION     0x00000020 
#define ERROR_ACCESS_DENIED         0x00000005
#define ERROR_NETWORK_ACCESS_DENIED 0x00000041
#define ERROR_WRITE_PROTECT         0x00000013
#define ERROR_INVALID_PASSWORD      0x00000056
#define ERROR_DISK_CHANGE           0x0000006B
#define ERROR_PATH_NOT_FOUND        0x00000003
#define ERROR_DIRECTORY             0x0000010B
#define ERROR_FILE_EXISTS           0x00000050
#define ERROR_SECTOR_NOT_FOUND      0x0000001B

/* https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics */

#define SM_CXSCREEN 0

/* https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/splitpath-wsplitpath?view=msvc-160 */

#define _MAX_DRIVE 0
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256





int _wcsicmp(const PTMWCHAR p1, const PTMWCHAR p2);

int _stricmp(const char* p1, const char* p2);

int wcsicmp(const PTMWCHAR p1, const PTMWCHAR p2);

int stricmp(const char* p1, const char* p2);

int _wcsnicmp(const PTMWCHAR p1, const PTMWCHAR p2, const int sz);

int wcsnicmp(const PTMWCHAR p1, const PTMWCHAR p2, const int sz);

int _strnicmp(const char* p1, const char* p2, const int sz);

int strnicmp(const char* p1, const char* p2, const int sz);

long _wtol(const wchar_t *str);


/*
wchar_t *wcscpy(
   wchar_t *strDestination,
   const wchar_t *strSource
);
int wcslen(
   const wchar_t *strSource
);//*/

#endif
