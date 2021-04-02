/*
    Windows Data Types
    https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types
*/

#ifndef WIN_TYPES_H
#define WIN_TYPES_H

#define WINAPI /*__stdcall*/
#define APIENTRY WINAPI
#define CALLBACK /*__stdcall*/
#define CONST const
#define VOID void
#define far 
#define FAR 
#define NEAR 
#define PASCAL 

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef wchar_t WCHAR;

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

typedef int HFILE;

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
typedef ULONG *PULONG;
typedef ULONGLONG *PULONGLONG;
typedef ULONG_PTR *PULONG_PTR;
typedef unsigned int ULONG32;
typedef ULONG32 *PULONG32;
typedef ULONG64 *PULONG64;
typedef unsigned short USHORT;
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

typedef struct _WIN32_FIND_DATA { 
  DWORD dwFileAttributes; 
  FILETIME ftCreationTime; 
  FILETIME ftLastAccessTime; 
  FILETIME ftLastWriteTime; 
  DWORD nFileSizeHigh; 
  DWORD nFileSizeLow; 
  DWORD dwOID; 
  TCHAR cFileName[MAX_PATH]; 
} WIN32_FIND_DATA,*PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;  

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
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

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

#define WS_TABSTOP 0x00010000L
#define WS_GROUP   0x00020000L
#define WS_VISIBLE 0x10000000L

#define get_max(a, b) (a > b) ? (a) : (b)
#define get_min(a, b) (a > b) ? (b) : (a)

#endif
