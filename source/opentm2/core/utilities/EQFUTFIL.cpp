// #define LOG_NETACCESS
//+----------------------------------------------------------------------------+
//|  EQFUTFIL.C - EQF general services                                         |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2017, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_DEV
#include <EQF.H>                  // General Translation Manager include file
#include <time.h>                 // C time related functions


#ifdef TEMPORARY_COMMENTED
#include "zip.h"
#include "unzip.h"
#endif //TEMPORARY_COMMENTED

#include "EQFUTPRI.H"                  // private utility header file
#include "tm.h"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
#include "ZipHelper.h"
static USHORT usLastDosRc;             // buffer for last DOS return code

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlQCurDisk            Query current disk drive          |
//+----------------------------------------------------------------------------+
//|Description:       Similar to DosQCurDisk. Returns drive letter of current  |
//|                   disk                                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   BYTE                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       drive letter of current disk                             |
//+----------------------------------------------------------------------------+
//|Samples:           curletter = UtlQCurDisk();                               |
//|                   if A:, B: and C: are available and C: is the current     |
//|                   drive then curletter is loaded with 'C'                  |
//+----------------------------------------------------------------------------+
//|Function flow:     get current drive using DosQCurDisk                      |
//|                   return to drive letter converted return code             |
//+----------------------------------------------------------------------------+

BYTE UtlQCurDisk()
{
    static CHAR szCurDir[MAX_PATH+100];

    DosError(0);
T5LOG(T5ERROR) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 65 if ( GetCurrentDirectory( sizeof(szCurDir), szCurDir ) == 0 ) { szCurDir[0] = EOS; }";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
    if ( GetCurrentDirectory( sizeof(szCurDir), szCurDir ) == 0 )
    {
      szCurDir[0] = EOS;
    } /* endif */
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
    DosError(1);

    return (BYTE)szCurDir[0];
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetDriveList        Get list of drive letters         |
//+----------------------------------------------------------------------------+
//|Description:       Load string with drive letters of available drives       |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT UtlGetDriveList( BYTE *szList)                    |
//+----------------------------------------------------------------------------+
//|Parameters:        1. BYTE * - array of bytes to contain drive letters      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - index of letter of current drive within list           |
//+----------------------------------------------------------------------------+
//|Samples:           curixletter = UtlDriveList( str);                        |
//|                   if A:, B: and C: are available and C: is the current     |
//|                   drive then str is loaded "ABC" and curixletter is 2      |
//+----------------------------------------------------------------------------+
//|Function flow:     get number of floppy drives usinf DosDevConfig           |
//|                   get drive bitmap using DosQCurDisk                       |
//|                   loop through bitmap and generate drive letters for all   |
//|                    existing drives                                         |
//|                   return index of current drive                            |
//+----------------------------------------------------------------------------+

USHORT UtlGetDriveList( BYTE *szList)
{
  LOG_UNIMPLEMENTED_FUNCTION;
  return( 0 );   // set index relative to 0
}

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetLANDriveList        Get list of LAN drive letters  |
//+----------------------------------------------------------------------------+
//|Description:       Load string with drive letters of LAN drives or LAN      |
//|                   capable drives                                           |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT UtlGetLANDriveList( BYTE *szList)                 |
//+----------------------------------------------------------------------------+
//|Parameters:        1. BYTE * - array of bytes to contain drive letters      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       - index of letter of current drive within list           |
//+----------------------------------------------------------------------------+

USHORT UtlGetLANDriveList( PBYTE pszList )
{
  
  LOG_UNIMPLEMENTED_FUNCTION;
  return( 0 );
} /* end of function UtlGetLANDriveList */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLoadFile            Load a file into memory           |
//+----------------------------------------------------------------------------+
//|Description:       Load a file into memory                                  |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlLoadFile(PSZ    pszFilename,                     |
//|                                    PVOID  *ppLoadedFile,                   |
//|                                    USHORT *pulBytesRead,                   |
//|                                    BOOL   fContinue,                       |
//|                                    BOOL   fMsg );                          |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFilename contains the full path to the file to      |
//|                     be loaded.                                             |
//|                   - ppLoadedFile is the pointer to a pointer which         |
//|                     addresses the memory area the file will be loaded into.|
//|                     If this value is not NULL the utility assumes          |
//|                     the storage has been allocated already and it will     |
//|                     load the file at that point. If *ppLoadedFile is       |
//|                     NULL at entry storage will be allocated and            |
//|                     *ppLoadedFile will be set accordingly.                 |
//|                   - pulBytesRead will point to a USHORT containing         |
//|                     the size of the file which has been loaded.            |
//|                   - fContinue if set will prompt the user with a message   |
//|                     if the load failed. The user may then continue or      |
//|                     cancel. If fContinue is set the flag fMsg is set       |
//|                     automatically by the program.                          |
//|                   - fMsg if set will show error messages.                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   if OK. If the load failed but fContinue           |
//|                          is set and the user comitted to continue then TRUE|
//|                          will be returned.                                 |
//|                   FALSE  in case of errors                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     open the file                                            |
//|                   if ok then                                               |
//|                     get size of file                                       |
//|                     if ok and no area has been allocated for the file then |
//|                       allocate buffer for file                             |
//|                     endif                                                  |
//|                   endif                                                    |
//|                   if ok then                                               |
//|                     read the input file into the buffer                    |
//|                   endif                                                    |
//|                   if file has been opened then                             |
//|                     close the file                                         |
//|                   endif                                                    |
//|                   if not ok and memory has been allocated then             |
//|                     free allocated memory                                  |
//|                   endif                                                    |
//|                   if not ok and message handling has been requested then   |
//|                     issue error message                                    |
//|                   endif                                                    |
//|                   return ok flag                                           |
//+----------------------------------------------------------------------------+
BOOL UtlLoadFile(PSZ    pszFilename,         // name of file to be loaded
                   PVOID  *ppLoadedFile,       // return pointer to loaded file
                   USHORT *pusBytesRead,       // length of loaded file
                   BOOL   fContinue,           // continue in case of error. If this flag
                                               // is set fMsg must be set too.
                   BOOL   fMsg )                  // Display message in case of error
{
	ULONG  ulBytesRead = *pusBytesRead;

    BOOL fOK = UtlLoadFileHwnd( pszFilename, ppLoadedFile, &ulBytesRead, fContinue,
                             fMsg, NULLHANDLE ) ;
    *pusBytesRead = (USHORT)ulBytesRead;

    return(fOK);
}

BOOL UtlLoadFileL(PSZ    pszFilename,         // name of file to be loaded
                  PVOID  *ppLoadedFile,       // return pointer to loaded file
                  ULONG *pulBytesRead,       // length of loaded file
                  BOOL   fContinue,           // continue in case of error. If this flag
                                              // is set fMsg must be set too.
                  BOOL   fMsg )                  // Display message in case of error
{
   return( UtlLoadFileHwnd( pszFilename, ppLoadedFile, pulBytesRead, fContinue,
                            fMsg, NULLHANDLE ) );
}

BOOL UtlLoadFileHwnd
(
   PSZ    pszFilename,                 // name of file to be loaded
   PVOID  *ppLoadedFile,               // return pointer to loaded file
   ULONG  *pulBytesRead,               // length of loaded file
   BOOL   fContinue,                   // continue in case of error. If this flag
                                       // is set fMsg must be set too.
   BOOL   fMsg,                        // Display message in case of error
   HWND   hwnd                         // parent handle for error message box
)
{
    BOOL             fOK = TRUE;           // Procssing status flag
    USHORT           usDosRc = TRUE;       // Return code from Dos operations
    HFILE            hInputfile = NULLHANDLE; // File handle for input file
    USHORT           usAction;             // input file action
    USHORT           usMsgButton;          // buttons to be set on msgbox
    USHORT           usMessage;            // Message number
    BOOL             fStorage = FALSE;     // If storage was allocated then TRUE
    ULONG            ulSize = 0;

    *pulBytesRead = 0;                     // init length of file
    if (fContinue)
    {
      fMsg = TRUE;
    } /* endif */

    if ( fMsg == NOMSG )
    {
      fMsg = FALSE;
    } /* endif */

    usDosRc = UtlOpen(pszFilename, &hInputfile , &usAction,
                      0L,                            // Create file size
                      FILE_NORMAL,                   // Normal attribute
                      OPEN_ACTION_OPEN_IF_EXISTS,    // Open if exists
                      OPEN_SHARE_DENYWRITE,          // Deny Write access
                      0L,                            // Reserved
                      FALSE );                       // no error handling

    fOK = ( usDosRc == 0 );

    // If fOK. Get the size of the input data and allocate storage to load the file.
    // If an error occurred set usRc to FALSE and set the appropriate message code.
    if ( fOK )
    {
      // Get file size
      usDosRc = UtlGetFileSize( hInputfile, &ulSize, FALSE );

      // If previous call OK allocate storage if required
      if ( !usDosRc && (ulSize > 0L) )
      {
        if ( *ppLoadedFile == NULL )
        {
          usMessage = ( fMsg ) ? ERROR_STORAGE : NOMSG;
          fOK = UtlAllocHwnd( (PVOID *)ppLoadedFile,  // Allocate storage for input data
                          0L,
                          get_max( (LONG) MIN_ALLOC, ulSize ),
                          usMessage, hwnd );
          fStorage = TRUE;
        } /* endif */
      }
      else
      {
        T5LOG(T5WARNING) <<"UtlLoadFileHwnd(path=" << pszFilename <<"):: usDosRc: " << usDosRc << ", ulSize: " << ulSize << "-> fOK=false now";
        fOK = FALSE;
      } /* endif */
    } /* endif */

    // If fOK. Load the input data in storage and close it if load was OK
    if (fOK)
    {
      usDosRc = UtlReadL( hInputfile , *ppLoadedFile,
                         ulSize, pulBytesRead, FALSE );

      fOK = (usDosRc == 0 );

      // Check if the entire file was read
      if ( ulSize != *pulBytesRead )
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */

    if ( hInputfile )
    {
      UtlCloseHwnd( hInputfile, fMsg, hwnd );          // Close Input file if needed
    } /* endif */

    // Free storage allocation if allocated and something went wrong
    if ( !fOK && fStorage )
    {
      UtlAlloc( (PVOID *)ppLoadedFile, 0L, 0L, NOMSG );
    } /* endif */

    if ( !fOK )                       // display error message if not ok
    {                                         // set msg btn depending on Contin.
       usMsgButton = (USHORT)(( fContinue ) ? MB_OKCANCEL : MB_CANCEL);
       // Message text:   Table or file %1 could not be accessed.
       //usDosRc = 
       T5LOG(T5ERROR) <<  "::ERROR_TA_ACC_FILE:: fName = " << pszFilename;

       fOK = fContinue && ( usDosRc == MBID_OK );
    } /* endif */
    return ( fOK );
 } /* end of function UtlLoadFile */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlMkMultDir           Make dir with all intermediate dir|
//+----------------------------------------------------------------------------+
//|Description:       Creates all directories specified in the input path      |
//|                   if required. The function uses UtlMkDir for all          |
//|                   directories contained in the specified path.             |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlMkMultDir( PSZ pszPath, BOOL fMsg )            |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszPath is the fully qualified path without a file     |
//|                     name at the end (e.g. C:\EQF\IMPORT\PROPERTY).         |
//|                   - fMsg if set will show error messages.                  |
//+----------------------------------------------------------------------------+
//|Samples:           usRC =  UtlMkMultDir( "C:\EQF\FOLDER\SOURCE", TRUE )     |
//|                   will create the directories C:\EQF, C:\EQF\FOLDER and    |
//|                   C:\EQF\FOLDER\SOURCE.                                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       Return code of UtlMkDir function. (It is no error if     |
//|                   one or more of the directories do exist).                |
//+----------------------------------------------------------------------------+
//|Function flow:     get position of backslash in path                        |
//|                   while backslash position found                           |
//|                     if next character is EOS then                          |
//|                       leave loop                                           |
//|                     else                                                   |
//|                       get position of next backslash in path               |
//|                       truncate path at found location                      |
//|                       make directory (don't care if directory exists)      |
//|                       append rest of path                                  |
//|                     endif                                                  |
//|                   endwhile                                                 |
//|                   return retcode of called functions                       |
//+----------------------------------------------------------------------------+
USHORT UtlMkMultDir( PSZ pszPath, BOOL fMsg )
{
  return ( UtlMkMultDirHwnd( pszPath, fMsg, NULLHANDLE ) );
}

USHORT UtlMkMultDirHwnd( PSZ pszPath, BOOL fMsg, HWND hwnd )
{
   PSZ   pszPathEnd;                   // ptr to current end of path
   USHORT usRC = 0;                    // function return value

   pszPathEnd = strchr( pszPath, BACKSLASH );
   while ( pszPathEnd && !usRC )
   {
      if ( *(pszPathEnd+1) == EOS )    // at terminating backslash ...
      {
         pszPathEnd = NULL;            // ... force end of loop
      }
      else                             // else create directory
      {
         pszPathEnd = strchr( pszPathEnd+1, BACKSLASH );
         if ( pszPathEnd )
         {
            *pszPathEnd = EOS;
         } /* endif */

         usRC = UtlMkDirHwnd( pszPath, 0L, fMsg, hwnd );

         if ( pszPathEnd )
         {
            *pszPathEnd = BACKSLASH;
         } /* endif */
      } /* endif */
   } /* endwhile */
   return( usRC );
} /* UtlMkMultDir */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlFileExist           Check if a file exists            |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlFileExist( PSZ pszFileName )                     |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFileName is the fully qualified file name           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if file exists                |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlFileExist( PSZ pszFile )
{
    return !FilesystemHelper::FindFiles(pszFile).empty();
} /* endof UtlFileExist */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlDirExist           Check if a directory exists        |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlDirExist( PSZ pszDirName )                       |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszDirName is the fully qualified direcorty name       |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   directory exists                                  |
//|                   FALSE  directory does not exist                          |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if directory exists           |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlDirExist( PSZ pszDir )
{
   DWORD dwAttrib;

   if (strlen(pszDir) <= 3)
   {
     // pure drive e.g. "A:\", "E:"
     BYTE    driveletter[30];

     UtlGetDriveList(driveletter);
     if (strchr((PSZ)driveletter, toupper(pszDir[0])) && strlen(pszDir) > 0)
     {
       return TRUE;
     }
   }

   //dwAttrib = GetFileAttributes(pszDir);

   //return( dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) );
} /* endof UtlDirExist */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlWriteFile           Write a file to disk              |
//+----------------------------------------------------------------------------+
//|Description:       Writes a file (counterpart to UtlLoadFile)               |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlWriteFile( PSZ    pszFile,                     |
//|                                        USHORT usDataLength,                |
//|                                        PVOID  pData,                       |
//|                                        BOOL   fMsg );                      |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFile is the fully qualified file name               |
//|                   - usDataLength is the length of the data to write        |
//|                   - pData points to the data to write to the file          |
//|                   - if fMsg is TRUE error handling is done in the function |
//+----------------------------------------------------------------------------+
//|Samples:           UtlWriteFile( "TEST.DATA", 10, "test data" );            |
//|                     will create/rewrite a file named "TEST.DAT" which will |
//|                     contain the string "test data"                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of called Dosxxxx functions in               |
//|                   UtlWriteFileHwnd                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     calls UtlWriteFileHwnd with NULL window handle           |
//+----------------------------------------------------------------------------+

USHORT UtlWriteFile
(
  PSZ       pszFile,            // name of file to write
  USHORT    usDataLength,       // length of data to write to the file
  PVOID     pData,              // pointer to data being written to file
  BOOL      fMsg                // TRUE = do message handling
)
{
   ULONG  ulDataLength = usDataLength;

   return( UtlWriteFileHwnd( pszFile,
                             ulDataLength,
                             pData,
                             fMsg,
                             NULLHANDLE ) );;

}

USHORT UtlWriteFileL
(
  PSZ       pszFile,            // name of file to write
  ULONG     ulDataLength,       // length of data to write to the file
  PVOID     pData,              // pointer to data being written to file
  BOOL      fMsg                // TRUE = do message handling
)
{
   return( UtlWriteFileHwnd( pszFile,
                             ulDataLength,
                             pData,
                             fMsg,
                             NULLHANDLE ) );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlWriteFileHwnd       Write a file to disk              |
//+----------------------------------------------------------------------------+
//|Description:       Writes a file (counterpart to UtlLoadFile)               |
//+----------------------------------------------------------------------------+
//|Function Call:     USHORT UtlWriteFileHwnd( PSZ    pszFile,                 |
//|                                            USHORT usDataLength,            |
//|                                            PVOID  pData,                   |
//|                                            BOOL   fMsg,                    |
//|                                            HWND   hwndParent );            |
//+----------------------------------------------------------------------------+
//|Parameters:        - pszFile is the fully qualified file name               |
//|                   - usDataLength is the length of the data to write        |
//|                   - pData points to the data to write to the file          |
//|                   - if fMsg is TRUE error handling is done in the function |
//|                   - hwndParent is the parent window                        |
//+----------------------------------------------------------------------------+
//|Samples:           UtlWriteFile( "TEST.DATA", 10, "test data" );            |
//|                     will create/rewrite a file named "TEST.DAT" which will |
//|                     contain the string "test data"                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of called Dosxxxx functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     open the file for output                                 |
//|                   if ok write given data to file                           |
//|                   if file is open close file                               |
//|                   return any error codes to caller                         |
//+----------------------------------------------------------------------------+
USHORT UtlWriteFileHwnd                                                  /*@86C*/
(
  PSZ       pszFile,                   // name of file to write
  ULONG     ulDataLength,              // length of data to write to the file
  PVOID     pData,                     // pointer to data being written to file
  BOOL      fMsg,                      // TRUE = do message handling
  HWND      hwndParent
)
{
    USHORT           usDosRC;              // Return code from Dos operations
    HFILE            hOutFile = NULLHANDLE;// File handle for input file
    USHORT           usAction;             // file action performed by DosOpen
    ULONG            ulBytesWritten;       // number of bytes written to file
//    PSZ              pszErrParm;          // ptr to error parameter

    /******************************************************************/
    /* Open the file                                                  */
    /******************************************************************/
    usDosRC = UtlOpenHwnd( pszFile, &hOutFile, &usAction,
                           0L,
                           FILE_NORMAL,
                           FILE_CREATE | FILE_TRUNCATE,
                           OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
                           0L,
                           fMsg,
                           hwndParent );

    /******************************************************************/
    /* if ok, write given data to file                                */
    /******************************************************************/
    if ( !usDosRC )
    {
      usDosRC = UtlWriteHwnd( hOutFile,
                              pData,
                              ulDataLength,
                              &ulBytesWritten,
                              fMsg,
                              hwndParent );
//      if ( !usDosRC && ( usBytesWritten != usDataLength ) )      /*KIT1274*/
//      {                                                          /*KIT1274*/
//        usDosRC = ERROR_DISK_FULL;                               /*KIT1274*/
//      } /* endif */                                              /*KIT1274*/
    } /* endif */

    /******************************************************************/
    /* Close the file                                                 */
    /******************************************************************/
    if ( hOutFile )
    {
      UtlCloseHwnd( hOutFile,
                    fMsg,
                    hwndParent );
      /******************************************************************/
      /* Delete file if no Bytes written                                */
      /******************************************************************/
      if ( usDosRC )
      {
           UtlDeleteHwnd( pszFile,
                          0L,
                          FALSE,
                          hwndParent );
//           if ( fMsg )                                           /*KIT1274*/
//           {                                                     /*KIT1274*/
//             pszErrParm = pszFile;                               /*KIT1274*/
//             T5LOG( T5ERROR) <<   ERROR_EQF_DISK_FULL, MB_CANCEL, 1,    /*KIT1274*/
//                           &pszErrParm, EQF_ERROR,               /*KIT1274*/
//                           hwndParent );                         /*KIT1274*/
//           } /* endif */                                         /*KIT1274*/
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Return error codes to calling function                         */
    /******************************************************************/
    return ( usDosRC );

} /* UtlWriteFile */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlIsEqfDrive          Check if a drive is a MAT drive   |
//+----------------------------------------------------------------------------+
//|Description:       Check if a given drive is a) in the EQF drive list       |
//|                   and b) that all required directories exist on this drive |
//+----------------------------------------------------------------------------+
//|Parameters:        1. CHAR   - letter of drive beeing checked               |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE ==> function completed successfully                 |
//|                   FALSE ==> function could not complete due to             |
//|                             serious errors                                 |
//+----------------------------------------------------------------------------+
//|Samples:           if ( UtlIsSecondaryDrive ( 'E' ) )                       |
//|                   ...                                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     set isEqfDrive flag to TRUE                              |
//|                   if drive is not in EQF drive list reset isEqfDrive flag  |
//|                   if isEqfDrive then                                       |
//|                     reset isEqfDrive flag if drive letter is invalid       |
//|                   endif                                                    |
//|                   if isEqfDrive then                                       |
//|                     reset isEqfDrive flag if EQF directories are missing   |
//|                   endif                                                    |
//|                   return isEqfDrive flag                                   |
//+----------------------------------------------------------------------------+
BOOL UtlIsEqfDrive
(
   CHAR chDrive                        // drive letter being checked
)
{
   BOOL fIsEqfDrive = TRUE;            // function return value
   union
   {
      CHAR  szDrives[MAX_DRIVELIST];   // buffer for drive letters
      CHAR  szPath[MAX_EQF_PATH+1];    // buffer for path names
   } Buffers;

   //
   // get string containing all known EQF drives
   //
   UtlQueryString( QST_ORGEQFDRIVES, Buffers.szDrives,
                   sizeof(Buffers.szDrives) );

   //
   // check if given drive is contained in EQF drive list
   //
   if ( !strchr( Buffers.szDrives, chDrive ) )
   {
      fIsEqfDrive = FALSE;
   } /* endif */

   //
   // check if given drive is a valid drive for this system
   //
   if ( fIsEqfDrive )
   {
      UtlGetDriveList( (PBYTE)Buffers.szDrives );     // get list of attached drives
      if ( !strchr( Buffers.szDrives, chDrive ) )
      {
         fIsEqfDrive = FALSE;
      } /* endif */
   } /* endif */

   //
   // check if required directories exist on drive
   //
   if ( fIsEqfDrive )
   {
      UtlMakeEQFPath( Buffers.szPath, chDrive, MEM_PATH, NULL );
      if ( !UtlDirExist( Buffers.szPath ) )
      {
         fIsEqfDrive = FALSE;
      }
      else
      {
         UtlMakeEQFPath( Buffers.szPath, chDrive, DIC_PATH, NULL );
         if ( !UtlDirExist( Buffers.szPath ) )
         {
            fIsEqfDrive = FALSE;
         }
         else
         {
            UtlMakeEQFPath( Buffers.szPath, chDrive, TABLE_PATH, NULL );
            if ( !UtlDirExist( Buffers.szPath ) )
            {
               fIsEqfDrive = FALSE;
            }
            else
            {
               UtlMakeEQFPath( Buffers.szPath, chDrive, LIST_PATH, NULL );
               if ( !UtlDirExist( Buffers.szPath ) )
               {
                  fIsEqfDrive = FALSE;
               }
               else
               {
                  // everything seems to be ok with the given drive
               } /* endif */
            } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   return( fIsEqfDrive );
} /* end of UtlIsEqfDrive */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlGetCheckedEqfDrives Get list of checked MAT drives    |
//+----------------------------------------------------------------------------+
//|Description:       Returns a list of EQF drives and checks each             |
//|                   of the drives if it is still valid                       |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ    - ptr to buffer for drive letter string        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE ==> function completed successfully                 |
//|                   FALSE ==> function could not complete due to             |
//|                             serious errors                                 |
//+----------------------------------------------------------------------------+
//|Samples:           UtlGetCheckedEqfDrives( pszDrives );                     |
//|                   would place "DE" in pszDrives if "DET" where EQF drives, |
//|                   but T: did not have the EQF directories anymore.         |
//+----------------------------------------------------------------------------+
//|Function flow:     get list of original EQF drives                          |
//|                   for all drives in list do                                |
//|                     if drive is an EQF drive (call to UtlIsEqfDrive) then  |
//|                       add drive to checked EQF drive list                  |
//|                     endif                                                  |
//|                   endfor                                                   |
//|                   replace QST_VALIDDRIVES string with checked EQF drive    |
//|                    list                                                    |
//|                   copy checked EQF drives to supplied buffer               |
//|                   return ok flag                                           |
//+----------------------------------------------------------------------------+
BOOL UtlGetCheckedEqfDrives
(
   PSZ pszDrives                       // ptr to buffer for EQF drives
)
{
   BOOL fOK = TRUE;                    // function return code
   CHAR  szOrgDrives[MAX_DRIVELIST];   // buffer for original EQF drive letters
   PSZ   pszOrgDrive = szOrgDrives;    // pointer for drive string processing
   CHAR  szValidDrives[MAX_DRIVELIST]; // buffer for valid EQF drive letters
   PSZ   pszValidDrive = szValidDrives;// pointer for drive string processing

   /*******************************************************************/
   /* Get list of MAT drives (source: system properties)              */
   /*******************************************************************/
   UtlQueryString( QST_ORGEQFDRIVES, szOrgDrives, sizeof(szOrgDrives) );

   /*******************************************************************/
   /* Check each of the drives in the drive list                      */
   /*******************************************************************/
   while ( *pszOrgDrive)
   {
      if ( UtlIsEqfDrive( *pszOrgDrive ) )
      {
         *pszValidDrive++ = *pszOrgDrive;        // add to valid drives
      } /* endif */
      pszOrgDrive++;                             // test next drive
   } /* endwhile */
   *pszValidDrive = EOS;                                   // terminate valid drives list

   /*******************************************************************/
   /* Set list of checked MAT drives                                  */
   /*******************************************************************/
   UtlReplString( QST_VALIDEQFDRIVES, szValidDrives );
   if ( pszDrives )
   {
      strcpy( pszDrives, szValidDrives );
   } /* endif */

   return( fOK );
} /* end of UtlGetCheckedEqfDrives */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufOpen                                               |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufOpen( PBUFCB *ppBufCB, PSZ pszFile,                |
//|                               USHORT usBufSize, BOOL fMsg )                |
//+----------------------------------------------------------------------------+
//|Description:       Opens a file for buffered output                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   *ppBufCB   address of buffered output control   |
//|                                       block pointer                        |
//|                   USHORT   usBufSize  size of input/output buffer          |
//|                   USHORT   usReadWrite read/write flag                     |
//|                            FILE_OPEN   for read access                     |
//|                            FILE_CREATE for write access                    |
//|                            FILE_APPEND for write/append access             |
//|                   BOOL     fMsg  TRUE if error messages should be          |
//|                                  displayed, otherwise FALSE                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0       function completed successfully                  |
//|                   ERROR_NOT_ENOUGH_MEMORY  allocation of memory failed     |
//|                   other   return codes of Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = UtlBufOpen( &pCB, "TEST.LST", 2048, FILE_CREATE ) |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate control block / output buffer                   |
//|                   open the file                                            |
//|                   initialize control block variables                       |
//+----------------------------------------------------------------------------+
USHORT UtlBufOpen
(
  PBUFCB *ppBufCB,
  PSZ pszFile,
  ULONG ulBufSize,
  ULONG ulReadWrite,
  BOOL   fMsg
)
{
  return( UtlBufOpenHwnd( ppBufCB, pszFile, ulBufSize, ulReadWrite, fMsg,
                          NULLHANDLE ) );
}

USHORT UtlBufOpenHwnd
(
  PBUFCB *ppBufCB,
  PSZ pszFile,
  ULONG ulBufSize,
  ULONG ulReadWrite,
  BOOL   fMsg,
  HWND   hwnd
)
{
  USHORT       usOpenAction;           // action performed by UtlOpen
  USHORT       usRC = 0;               // return code of function
  USHORT       usOpenFlag;             // File open flag
  USHORT       usOpenMode;             // File open mode
  PBUFCB       pBufCB;                 // buffered input/output control block
  ULONG        ulFilePos;              // new file position

  /********************************************************************/
  /* Allocate control block / input output buffer                     */
  /********************************************************************/
  UtlAllocHwnd( (PVOID *)&pBufCB, 0L, (sizeof(BUFCB) + ulBufSize ),
                 ERROR_STORAGE, hwnd );
  if ( !pBufCB )
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  /********************************************************************/
  /* Open the output file                                             */
  /********************************************************************/
  if ( !usRC )
  {
     switch (ulReadWrite)
     {
      case FILE_CREATE:
        {
        usOpenFlag = FILE_TRUNCATE | FILE_CREATE ;
        usOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
        }
        break;
      case  FILE_APPEND:
        {
        usOpenFlag = FILE_OPEN;
        usOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE;
        }
        break;
      default:
        {
        usOpenFlag = FILE_TRUNCATE | FILE_OPEN;
        usOpenMode = OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE;
        }
        break;
     } /* endswitch */

     usRC = UtlOpenHwnd( pszFile,
                         &pBufCB->hFile,
                     &usOpenAction, 0L,
                     FILE_NORMAL,
                     usOpenFlag,
                     usOpenMode,
                     0L,
                     fMsg, hwnd );
  } /* endif */
  if ( !usRC && (ulReadWrite == FILE_APPEND) )
  {
    usRC = UtlChgFilePtrHwnd( pBufCB->hFile, 0L, FILE_END, &ulFilePos, fMsg, hwnd );
  } /* endif */


  /********************************************************************/
  /* Initialize control block variables                               */
  /********************************************************************/
  if ( !usRC )
  {
     pBufCB->ulUsed = 0;
     pBufCB->ulSize = ulBufSize;
     pBufCB->ulProcessed  = 0;
     strcpy( pBufCB->szFileName, pszFile );
     if ( ulReadWrite != FILE_CREATE )
     {
       usRC = UtlGetFileSizeHwnd( pBufCB->hFile,
                                  &(pBufCB->ulRemaining),
                                  fMsg, hwnd );
       pBufCB->fWrite = ( ulReadWrite == FILE_APPEND );
     }
     else
     {
       pBufCB->fWrite = TRUE;
     } /* endif */
     *ppBufCB = pBufCB;
  } /* endif */

  return( usRC );
} /* end of function UtlBufOpen */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufClose            Close a buffered output file      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufClose( PBUFCB pBufCB, BOOL fMsg )                  |
//+----------------------------------------------------------------------------+
//|Description:       Closes a file which has been opened for buffered output  |
//|                   using UtlBufOpen.                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   pBufCB     pointer to buffered output CB        |
//|                   BOOL     fMsg       TRUE = show error messages           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      function completed successfully                   |
//|                   other  error codes from Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pBufCB must have been created using UtlBufOpen           |
//+----------------------------------------------------------------------------+
//|Function flow:     write pending data to output file                        |
//|                   close output file                                        |
//|                   free memory of buffered output control block             |
//+----------------------------------------------------------------------------+
USHORT UtlBufClose
(
  PBUFCB pBufCB,                       // pointer to buffered output CB
  BOOL   fMsg                          // TRUE for message processing
)
{
  return( UtlBufCloseHwnd( pBufCB, fMsg, NULLHANDLE ) );
}

USHORT UtlBufCloseHwnd
(
  PBUFCB pBufCB,                       // pointer to buffered output CB
  BOOL   fMsg,                         // TRUE for message processing
  HWND   hwnd                          // handle for error messages
)
{
  USHORT      usRC = 0;               // return code of function
  ULONG       ulBytesWritten;         // number of bytes written to file
  PSZ         pszErrParm;             // ptr to error parameter

  /********************************************************************/
  /* Write pending data to output file                                */
  /********************************************************************/
  if ( pBufCB->fWrite && pBufCB->ulUsed )
  {
    usRC = UtlWriteHwnd( pBufCB->hFile,
                     (PVOID)pBufCB->Buffer,
                     pBufCB->ulUsed,
                     &ulBytesWritten,
                     fMsg, hwnd );
    if ( !usRC && (pBufCB->ulUsed != ulBytesWritten) )
    {
      usRC = ERROR_DISK_FULL;
      if ( fMsg )
      {
        pszErrParm = pBufCB->szFileName,
        T5LOG(T5ERROR) <<   "; rc = " << usRC << "; " << pszErrParm ;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Close the file in any case                                       */
  /********************************************************************/
  UtlCloseHwnd( pBufCB->hFile, fMsg, hwnd );

  /********************************************************************/
  /* Free control block                                               */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pBufCB, 0L, 0L, NOMSG );

  return( usRC );
} /* end of function UtlBufClose */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlBufRead             Read data buffered from file      |
//+----------------------------------------------------------------------------+
//|Function call:     UtlBufRead( PBUFCB pBufCB, PSZ pszData, BOOL fMsg )      |
//+----------------------------------------------------------------------------+
//|Description:       Reads data buffered from file.                           |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBUFCB   pBufCB     pointer to buffered input/output CB  |
//|                   PSZ      pszData    ptr to buffer for data               |
//|                   USHORT   ulLength   length of data to read               |
//|                   PUSHORT  pulBytesRead bytes actually read from file      |
//|                   BOOL     fMsg  TRUE = show error messages                |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0      function completed successfully                   |
//|                   other  error codes from Dosxxxx calls                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pBufCB must have been created using UtlBufOpen           |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = UtlBufRead( pCB, pBuffer, ulLength, &usRead );    |
//+----------------------------------------------------------------------------+
//|Function flow:     TBD                                                      |
//+----------------------------------------------------------------------------+
USHORT UtlBufRead
(
  PBUFCB   pBufCB,                     // pointer to buffered input/output CB
  PSZ      pszData,                    // ptr to buffer for data
  ULONG    ulLength,                   // length of data to read
  PULONG   pulBytesRead,               // bytes actually read from file
  BOOL     fMsg                        // TRUE = show error messages
)
{
   return( UtlBufReadHwnd( pBufCB, pszData, ulLength, pulBytesRead, fMsg,
                           NULLHANDLE ) );
}

USHORT UtlBufReadHwnd
(
  PBUFCB   pBufCB,                     // pointer to buffered input/output CB
  PSZ      pszData,                    // ptr to buffer for data
  ULONG    ulLength,                   // length of data to read
  PULONG   pulBytesRead,               // bytes actually read from file
  BOOL     fMsg,                       // TRUE = show error messages
  HWND     hwnd                        // handle for error messages
)
{
   USHORT      usRC = 0;               // return code returned to caller
   ULONG       ulBytesToRead;          // number of bytes to read from disk
   ULONG       ulBytesRead;            // number of bytes actual read from disk
   ULONG       ulBytesToCopy;          // number of bytes to be copied
   ULONG       ulBytesCopied = 0;      // number of bytes copied to data buffer

   if ( !pBufCB->fWrite )
   {
     /*****************************************************************/
     /* loop until all bytes are read or end-of-file is reached       */
     /*****************************************************************/
     while ( (usRC == NO_ERROR) &&
             (ulBytesCopied < ulLength) &&
             (pBufCB->ulRemaining || pBufCB->ulUsed) )
     {
       /***************************************************************/
       /* copy data from input buffer to data area                    */
       /***************************************************************/
       if ( pBufCB->ulUsed )
       {
         /*************************************************************/
         /* Get length of data area which can be copied to target     */
         /* buffer                                                    */
         /*************************************************************/
         ulBytesToCopy   = get_min( pBufCB->ulUsed, (ulLength - ulBytesCopied) );

         /*************************************************************/
         /* copy data and adjust variables                            */
         /*************************************************************/
         memcpy( pszData + ulBytesCopied,
                 pBufCB->Buffer + pBufCB->ulProcessed,
                 ulBytesToCopy );
         ulBytesCopied   += ulBytesToCopy;
         pBufCB->ulUsed  -= ulBytesToCopy;
         pBufCB->ulProcessed += ulBytesToCopy;
       } /* endif */

       /***************************************************************/
       /* read into buffer if buffer is empty                         */
       /***************************************************************/
       if ( !pBufCB->ulUsed && pBufCB->ulRemaining )
       {
         ulBytesToRead = get_min( pBufCB->ulSize, pBufCB->ulRemaining );
         usRC = UtlReadHwnd( pBufCB->hFile,
                         pBufCB->Buffer,
                         ulBytesToRead,
                         &ulBytesRead,
                         fMsg, hwnd );
         if ( !usRC && (ulBytesToRead != ulBytesRead) )
         {
           usRC = ERROR_READ_FAULT;
         } /* endif */
         if ( usRC == NO_ERROR )
         {
           pBufCB->ulRemaining -= ulBytesRead;
           pBufCB->ulUsed       = ulBytesRead;
           pBufCB->ulProcessed  = 0;
         } /* endif */
       } /* endif */
     } /* endwhile */
   }
   else
   {
     usRC = ERROR_INVALID_ACCESS;
   } /* endif */

   /*******************************************************************/
   /* set caller's number-of-bytes-read variable                      */
   /*******************************************************************/
   *pulBytesRead = ulBytesCopied;

   return( usRC );
} /* end of function UtlBufRead */


BOOL IsDBCSLeadByteEx(
  UINT CodePage,
  BYTE TestChar
){
  T5LOG( T5WARNING) << "called IsDBCSLeadByteEx, not sure if implementation fit";
  if(TestChar & 128){
    return true;
  }
  return false;
}
#define isdbcs1ex(usCP, c)  ((IsDBCSLeadByteEx( usCP, (BYTE) c )) ? DBCS_1ST : SBCS )



//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlLongToShortName   Convert long file name to short name|
//+----------------------------------------------------------------------------+
//|Description:       Convert a long file name to a short file name by         |
//|                   stripping all invalid characters until the end of the    |
//|                   long name is detected or 8 characters have been          |
//|                   found. There is no check done if a file with this name   |
//|                   already exists.                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        1. PSZ - ptr to long file name                           |
//|                   2. PSZ - ptr to buffer for short name                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0        always                                          |
//+----------------------------------------------------------------------------+
USHORT UtlLongToShortName( PSZ pszLongName, PSZ pszShortName )
{
  SHORT i = 0;                         // number of characters in short name

  // skip any path information in long file name
  {
    PSZ pszTemp = strrchr( pszLongName, BACKSLASH );
    if ( pszTemp != NULL )
    {
      pszLongName = pszTemp + 1;
    } /* endif */
  }
  strcpy(pszShortName, pszLongName);
  // return to caller
  return( 0 );
}



/*! \brief add a list of files to a ZIP package
  \param pszFileList list of comma separated, fully qualified file names to be added to the package
  \param pszPackage fully qualified name of the ZIP package being created
  \returns 0 in any case
*/

int UtlZipFiles( const char *pszFileList, const char * pszPackage  )
{
  char szCurFile[MAX_PATH];

  ZIP* pZip = ZipHelper::ZipOpen( pszPackage , 'w' );

  while( *pszFileList != EOS )
  {
    // get current file name
    Utlstrccpy( szCurFile, (PSZ)pszFileList, ',' );
    // add it to theZIP package
    ZipHelper::ZipAdd( pZip, szCurFile );

    // continue with next file
    pszFileList += strlen(szCurFile);
    if ( *pszFileList == ',' ) pszFileList++;
  } /* endwhile */
  
  ZipHelper::ZipClose( pZip );
  return( 0 );
}


