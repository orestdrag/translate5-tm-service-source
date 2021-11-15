//+----------------------------------------------------------------------------+
//|EQFMEMLP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Markus Conrad                                                       |
//+----------------------------------------------------------------------------+
//|Description: Functions to load/import a TM                                  |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|T  EQFMemLoadStart     -initialize load process                             |
//|T  EQFMemLoadProcess   -processes load process                              |
//|T  EQFMemLoadEnd       -dl cleanup for load process                         |
//|                                                                            |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|   UtlAlloc                                                                 |
//|   UtlQFileInfo                                                             |
//|   UtlMakeEQFPath                                                           |
//|   UtlLoadFile                                                              |
//|   UtlDispatch                                                              |
//|   UtlError                                                                 |
//|   UtlRead                                                                  |
//|   CloseFile                                                                |
//|   TmClose                                                                  |
//|   TmReplace                                                                |
//|   MemGetAddressOfProcessIDA                                                |
//|   MemDestroyProcess                                                        |
//|   MemCreateProcess                                                         |
//|   MemInitSlider                                                            |
//|   MemRcHandling                                                            |
//|   EQFTagTokenize                                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|   MemLoadStart                                                             |
//|   MemLoadProcess                                                           |
//|   MemLoadReadFile                                                          |
//|   MemLoadPrepSeg                                                           |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "win_types.h"
#include "../utilities/LogWrapper.h"
#include "../utilities/FilesystemHelper.h"
#include "../utilities/EqfPluginWrapper.h"
#include <cwctype>

// define for memory import logging (remove to disable logging)
#ifdef _DEBUG
  #define IMPORT_LOGGING
#endif

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_EDITORAPI        // for EQFWORDCOUNTPERSEGW
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions

#include "../utilities/FilesystemWrapper.h"
#include "../pluginmanager/PluginManager.h"
#include "../pluginmanager/OtmMemoryPlugin.h"
#include "../pluginmanager/OtmMemory.h"
#include "../utilities/EncodingHelper.h"
#include "MemoryFactory.h"

#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include "EQFMEM.ID" // Translation Memory IDs
#include <EQFTMTAG.H>             // Translation Memory SGML tags
#include "OTMFUNC.H"              // public defines for function call interface
#include <time.h>                 // C library for time functions
#include "EQFHLOG.H"              // for word count category limits
#include <EQFQDAM.H>              // Low level TM access functions
#include "EQFFUNCI.H"             // private defines for function call interface
#include "PropertyWrapper.H"

#define MEM_START_IMPORT    USER_TASK + 1
#define MEM_IMPORT_TASK     USER_TASK + 2
#define MEM_END_IMPORT      USER_TASK + 3


//--- declaration of internal functions
static USHORT  MemLoadStart( PVOID *ppIda, HWND );
static USHORT  MemLoadProcess(  PMEM_LOAD_IDA pLIDA );
static USHORT  MemLoadReadFile( PMEM_LOAD_IDA pLIDA );
static VOID    MemReduceCRLF( PSZ_W pString );
static BOOL    NTMLoadPrepSeg( PMEM_LOAD_IDA  pLIDA, PBOOL pfSegmentTooLarge );

// Check if a specific tag is following
BOOL MemLPCheckForTag( PTOKENENTRY pTok, SHORT sID );
BOOL MemLPExtractValue( PTOKENENTRY *ppTok, SHORT sStartID, SHORT sEndID, PSZ_W pszBuffer, int iBufSize, PBOOL pfSegmentTooLarge );

USHORT /*APIENTRY*/ MEMINSERTSEGMENT
( 
  LONG lMemHandle, 
  PMEMEXPIMPSEG pSegment 
);


static void GetElapsedTime( LONG64 *plTime )
{
  
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  *plTime =  now.tv_sec + now.tv_nsec / 1000000000.0;
}

USHORT MemHandleCodePageValue
(
  PMEM_LOAD_IDA pLIDA,                           // ptr to memory load IDA
  PSZ_W       pszCodePage,                       // ptr to specified code page value 
  PBOOL       pfRestartImport                    // caller's restart the import flag 
);


// import logging 
//#ifdef IMPORT_LOGGING
  static LONG lImportStartTime = 0;
  static LONG64 lMemAccessTime = 0;
  static LONG64 lFileReadTime = 0;
  static LONG64 lOtherTime = 0;

static void GetElapsedTime( LONG64 *plTime );

//#endif

USHORT MemLoadAndConvert( PMEM_LOAD_IDA pLIDA, PSZ_W pszBuffer, ULONG usSize, PULONG pusBytesRead );

INT_PTR /*CALLBACK*/ NTMTagLangDlg( HWND,         // Dialog window handle
               WINMSG,       // Message ID
               WPARAM,       // Message parameter 1
               LPARAM );     // Message parameter 2

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemLoadStart                                             |
//+----------------------------------------------------------------------------+
//|Function call:     static USHORT  MemLoadStart( HWND    hWnd,               |
//|                                                MPARAM  mp2   )             |
//+----------------------------------------------------------------------------+
//|Description:       Initialize the memory database import process            |
//+----------------------------------------------------------------------------+
//|Parameters:        hWnd  dialog handle                                      |
//|                   mp2   pointer to MEM_LOAD_DLG_IDA                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE  -no error                                          |
//|                   FALSE -indicates that process should be terminated       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|  get access to load dialog IDA                                             |
//|  get access to main TM IDA                                                 |
//|  allocate storage for TM load IDA and set usRc in dependeny of rc          |
//|                                                                            |
//|  if ( usRc )                                                               |
//|  {                                                                         |
//|     initialize segment ID                                                  |
//|     connect load IDA to main TM IDA                                        |
//|  }                                                                         |
//|                                                                            |
//|  if ( usRc )                                                               |
//|  {                                                                         |
//|     move values from dialog IDA to TM load IDA                             |
//|     get file size of file to be imported                                   |
//|     if ( error getting file size )                                         |
//|     {                                                                      |
//|        set usRc = FALSE                                                    |
//|     }                                                                      |
//|     else                                                                   |
//|     {                                                                      |
//|        create full TM and markup table path                                |
//|        load markup table and set usRc in dependency of rc                  |
//|     }                                                                      |
//|  }                                                                         |
//|  if ( usRc )                                                               |
//|  {                                                                         |
//|     allocate storage for tokenlist and set usRc                            |
//|     if ( usRc )                                                            |
//|        initialize tokenlist                                                |
//|  }                                                                         |
//|  if ( usRc )                                                               |
//|     allocate storage for REP_IN and REP_OUT structure and set usRc         |
//|  if ( usRc )                                                               |
//|  {                                                                         |
//|     call function MemCreateProcess to create load process and              |
//|     get the process handle                                                 |
//|     set usRc in dependency of process handle                               |
//|  }                                                                         |
//|  if ( usRc )                                                               |
//|     create progress(slider) window                                         |
//|  if ( !usRc )                                                              |
//|  {                                                                         |
//|     if ( access to main TM IDA still exists )                              |
//|     {                                                                      |
//|        destroy progress window if exist                                    |
//|        free data areas                                                     |
//|        call MemDestroy process to destroy TM load process                  |
//|        display error messgae                                               |
//|      }                                                                     |
//|   }                                                                        |
//|   return ( process handle )                                                |
//+----------------------------------------------------------------------------+
static USHORT  MemLoadStart( PVOID *ppIda,
                             HWND             hWnd )
{
   BOOL             fOK = TRUE;          // Return code
   USHORT           usDosRc = TRUE;       // Return code from Dos open operation
   PMEM_LOAD_IDA    pLIDA;                // Pointer load data area
   PMEM_LOAD_DLG_IDA  pDialogIDA;         // Pointer to the dialog IDA
   PSZ              pReplAddr[2];         // Pointer to an address list of replacement strings

   //--- Get pointer to dialog IDA
   pDialogIDA = (PMEM_LOAD_DLG_IDA)*ppIda;

   //--- Allocate storage for TM load IDA _MEM_LOAD_IDA. If an error occurred
   //--- set fOK to FALSE and issue appropriate message.
   fOK = UtlAllocHwnd( (PVOID *)&pLIDA, 0L, (LONG)sizeof( MEM_LOAD_IDA ), ERROR_STORAGE, pDialogIDA->hwndErrMsg );
   *ppIda = pLIDA;

   if ( fOK )
   {
      // Set segment ID to 1 in case the first segment ID is in error or
      // no segment ID is present at all.
      // Put also the address of MEM_IDA into MEM_LOAD_IDA
      strcpy(pLIDA->szSegmentID, "1");
      //--- initialize flag that message ERROR_MEM_NOT_REPLACED        /*@47A*/
      //--- will be displayed                                          /*@47A*/
      pLIDA->fDisplayNotReplacedMessage = TRUE;                        /*@47A*/

      pLIDA->pProposal = new(OtmProposal);
   } /* endif */

   // If fOK. Get file and memory database handle as well as their path
   // and names from the dialog IDA and store them in the MEM_LOAD_IDA.
   // Then open the tagtable and get its Size. If an error occurred
   // set fOK to FALSE and issue the appropriate message.
   if ( fOK )
   {
      // Move values from dialog IDA to load IDA
      pLIDA->hFile = pDialogIDA->hFile;
      pLIDA->pMem  = pDialogIDA->pMem;
      strcpy( pLIDA->szMemName, pDialogIDA->szMemName );
      strcpy( pLIDA->szShortMemName, pDialogIDA->szShortMemName );
      pLIDA->fMerge = pDialogIDA->fMerge;
      strcpy( pLIDA->szMemPath, pDialogIDA->szMemPath );
      strcpy( pLIDA->szFilePath, pDialogIDA->szFilePath );

      pLIDA->fBatch     = pDialogIDA->fBatch;
      pLIDA->fSkipInvalidSegments = pDialogIDA->fSkipInvalidSegments;
      pLIDA->hwndErrMsg = pDialogIDA->hwndErrMsg;
      pLIDA->pDDEMemImp = pDialogIDA->pDDEMemImp;

      pLIDA->hwndNotify = pDialogIDA->hwndNotify;
      strcpy( pLIDA->szObjName, pDialogIDA->szObjName );

      pLIDA->fIgnoreEqualSegments = pDialogIDA->fIgnoreEqualSegments;
      pLIDA->fMTReceiveCounting   = pDialogIDA->fMTReceiveCounting;
      strcpy( pLIDA->szFolObjName, pDialogIDA->szFolObjName );

      pLIDA->fAdjustTrailingWhitespace = pDialogIDA->fAdjustTrailingWhitespace;

      pLIDA->usImpMode  = pDialogIDA->usImpMode;
      pLIDA->pszList    = pDialogIDA->pszList;

      pLIDA->fYesToAll  = pDialogIDA->fYesToAll;

      pLIDA->pszActFile = pLIDA->pszList;

      pLIDA->fCreateMatchID = pDialogIDA->fCreateMatchID;
      pLIDA->fForceNewMatchID = pDialogIDA->fForceNewMatchID;
      strcpy( pLIDA->szMatchIDPrefix, pDialogIDA->szMatchIDPrefix );
      wcscpy( pLIDA->szMatchIDPrefixW, pDialogIDA->szMatchIDPrefixW );

      if ( pDialogIDA->ulOemCP )
        pLIDA->ulOemCP = pDialogIDA->ulOemCP;
      else
        pLIDA->ulOemCP = GetLangOEMCP(NULL);

      if ( pDialogIDA->ulAnsiCP )
        pLIDA->ulAnsiCP = pDialogIDA->ulAnsiCP;
      else
        pLIDA->ulAnsiCP = GetLangAnsiCP(NULL);

      // read the file size of the file to be imported and store it in IDA
      usDosRc = UtlGetFileSize( pLIDA->hFile, &(pLIDA->ulTotalSize), FALSE );
      if (usDosRc)
      {
        fOK = FALSE;
      }
      else
      {
        // Create full path to the memory database format table
        properties_get_str( KEY_OTM_DIR, pLIDA->szFullFtabPath, MAX_EQF_PATH );
        strcat(pLIDA->szFullFtabPath, "/TABLE/");
        strcat(pLIDA->szFullFtabPath, MEM_FORMAT_TABLE);

        // Load the format table and allocate storage for it
        fOK = ( TALoadTagTableHwnd( MEM_FORMAT_TABLE, &pLIDA->pFormatTable,
                                     TRUE, TRUE, pLIDA->hwndErrMsg ) == NO_ERROR);
      } /* endif */
   } /* endif */

   // If fOK. Allocate storage for the tokenlist and move address in
   // pTokenList, pTokenEntry and pTokenEntryWork.
   // If an error occurred set fOK to FALSE and issue appropriate message.
   if ( fOK )
   {
      fOK = UtlAllocHwnd( (PVOID *)&pLIDA->pTokenList, 0L,
                       (LONG)(NUMB_OF_TOKENS * sizeof(TOKENENTRY)),
                       ERROR_STORAGE, pLIDA->hwndErrMsg );
      if ( fOK )
      {
         // Initialize pTokenEntry and pTokenEntryWork
         pLIDA->pTokenEntry     = pLIDA->pTokenList;
         pLIDA->pTokenEntryWork = pLIDA->pTokenList;

         // Initialize the first token to ENDOFLIST
         pLIDA->pTokenList->sTokenid = ENDOFLIST;
      } /* endif */
   } /* endif */

   // get the token IDs for the the SGML tags, if this token is not contained in the markup table
   // use the default token values
   if ( fOK )
   {
     PTMWCHAR pRest = NULL;
     USHORT usLastColPos = 0;

     pRest = NULL;
     TATagTokenizeW( MEM_CONTEXT_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sContextEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_CONTROL_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sControlEndTokenID = pLIDA->pTokenList->sTokenid;


     pRest = NULL;
     TATagTokenizeW( NTM_DESCRIPTION_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sDescriptionEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_MEMORYDB_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sMemMemoryDBEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( NTM_MEMORYDB_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sNTMMemoryDBEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_SEGMENT_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sSegmentEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_SOURCE_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sSourceEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_TARGET_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sTargetEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_CONTEXT_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sContextTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_CONTROL_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sControlTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( NTM_DESCRIPTION_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sDescriptionTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_MEMORYDB_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sMemMemoryDBTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( NTM_MEMORYDB_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sNTMMemoryDBTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_SEGMENT_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sSegmentTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_SOURCE_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sSourceTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_TARGET_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sTargetTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_CODEPAGE_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sCodePageTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_CODEPAGE_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sCodePageEndTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_ADDDATA_TOKEN, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sAddInfoTokenID = pLIDA->pTokenList->sTokenid;

     pRest = NULL;
     TATagTokenizeW( MEM_ADDDATA_TOKEN_END, pLIDA->pFormatTable, TRUE, &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);
     pLIDA->sAddInfoEndTokenID = pLIDA->pTokenList->sTokenid;


     // if no codepage token is contained in markup table change token ID to an invalid value
     if ( pLIDA->sCodePageTokenID == TEXT_TOKEN)
     {
       pLIDA->sCodePageTokenID = -999;
     } /* endif */

     pLIDA->pTokenList->sTokenid = ENDOFLIST;             // enable correct start of import process
   } /* endif */

   // special handling for import in TMX format
   if ( fOK && ((pLIDA->usImpMode == MEM_FORMAT_TMX) || (pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS)) )
   {
     // allocate memory and segment data area
     fOK = UtlAlloc( (PVOID *)&pLIDA->pstMemInfo, 0L, sizeof(MEMEXPIMPINFO), ERROR_STORAGE );
     if ( fOK ) fOK = UtlAlloc( (PVOID *)&pLIDA->pstSegment, 0L, sizeof(MEMEXPIMPSEG), ERROR_STORAGE );


     #ifdef TO_BE_REMOVED
     // load external memory import module
     if ( fOK ){

        char libPath[MAX_EQF_PATH];
        properties_get_str(KEY_PLUGIN_DIR, libPath, MAX_EQF_PATH);
        strcat(libPath, "libEqfMemoryPlugin.so");
        fOK = (DosLoadModule( NULL, 0, libPath, &(pLIDA->hmodMemExtImport)) == 0 );
        //fOK = (DosLoadModule( NULL, 0, "OTMTMXIE.DLL", &(pLIDA->hmodMemExtImport)) == 0 );
     }

     // load entry points
     if ( fOK )
     {
       fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTSTART", (PFN*)(&(pLIDA->pfnMemImpStart))) == 0);
       if ( fOK ) fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTPROCESS", 
                                           (PFN*)(&(pLIDA->pfnMemImpProcess))) == 0);
       if ( fOK ) fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTEND", 
                                           (PFN*)(&(pLIDA->pfnMemImpEnd))) == 0);
     } /* endif */

     #endif //TO_BE_REMOVED
     
     // prepare segment logging
     LogMessage(WARNING, "TEMPORARY hardcoded if(false) for other logging system");
     if ( fOK  && false)
     {
       UtlMakeEQFPath( pLIDA->szSegLog, NULC, LOG_PATH, NULL );
       UtlMkDir( pLIDA->szSegLog, 0, FALSE );
       strcat( pLIDA->szSegLog, "\\MEMIMPORT.LOG" );
#ifdef _DEBUG
       pLIDA->hSegLog = fopen( pLIDA->szSegLog, "wb" );
#endif
       if ( pLIDA->hSegLog )
       {
         fwrite( UNICODEFILEPREFIX, 1, 2, pLIDA->hSegLog );
       } /* endif */
     } /* endif */

     // call start entry point
     if ( fOK )
     {
       char szMemSourceLang[MAX_LANG_LENGTH];

       // close input file (if open) as external import process will open the file itself...
       if ( pLIDA->hFile ) 
       {
         UtlClose( pLIDA->hFile, FALSE );
         pLIDA->hFile = NULL; 
         pDialogIDA->hFile = NULL;
       } /* endif */

       memset( pLIDA->pstMemInfo, 0, sizeof(MEMEXPIMPINFO) );
       strcpy( pLIDA->pstMemInfo->szName, pLIDA->szMemName );
       pLIDA->pMem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );
       if ( pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS ) 
       {
         pLIDA->pstMemInfo->fCleanRTF = TRUE;
         pLIDA->pstMemInfo->fTagsInCurlyBracesOnly = TRUE;
       }
       
       pLIDA->pstMemInfo->pFileBuffersMap = GetPFileBufferMap();

       //fOK = (pLIDA->pfnMemImpStart( &(pLIDA->lExternalImportHandle), pLIDA->szFilePath, pLIDA->pstMemInfo ) == 0);
       fOK = (EqfPluginWrapper::MemImportStart( &(pLIDA->lExternalImportHandle) , pLIDA->szFilePath, pLIDA->pstMemInfo ) == 0);
       pLIDA->pMem->getSourceLanguage( szMemSourceLang, sizeof(szMemSourceLang) );

       // check if memory source lanuage matchs source language of imported file
       if ( pLIDA->pstMemInfo->szSourceLang[0] && (strcmp( pLIDA->pstMemInfo->szSourceLang, szMemSourceLang ) != 0) )
       {
         USHORT usMBCode;

         pReplAddr[0] = pLIDA->pstMemInfo->szSourceLang;
         pReplAddr[1] = szMemSourceLang;
         usMBCode = LogMessage3(ERROR, __func__,  "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::",pReplAddr[0] );
         if ( usMBCode == MBID_CANCEL )
         {
           fOK = FALSE;
         } /* endif */
       } /* endif */

       // update memory description with description of imported memory
       if ( fOK )
       {
         pLIDA->pMem->setDescription( pLIDA->pstMemInfo->szDescription );
       } /* endif */
     } /* endif */

     pLIDA->fControlFound = TRUE; // avoid error message at end of import 

     //** GQ TODO: cleanup of resources in case of failures
   } /* endif */


   // special handling for import in XLIFF MT format
   if ( fOK && ((pLIDA->usImpMode == MEM_FORMAT_XLIFF_MT)) )
   {
     // allocate memory and segment data area
     fOK = UtlAlloc( (PVOID *)&pLIDA->pstMemInfo, 0L, sizeof(MEMEXPIMPINFO), ERROR_STORAGE );
     if ( fOK ) fOK = UtlAlloc( (PVOID *)&pLIDA->pstSegment, 0L, sizeof(MEMEXPIMPSEG), ERROR_STORAGE );


       #ifdef TO_BE_REMOVED
     // load external memory import module
     if ( fOK ){
        fOK = (DosLoadModule( NULL, 0, "OTMXLFMT.DLL", &(pLIDA->hmodMemExtImport)) == 0 );
     }
     // load entry points
     if ( fOK )
     {
       fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTSTART", (PFN*)(&(pLIDA->pfnMemImpStart))) == 0);
       if ( fOK ) fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTPROCESS", 
                                           (PFN*)(&(pLIDA->pfnMemImpProcess))) == 0);
       if ( fOK ) fOK = (DosGetProcAddr( pLIDA->hmodMemExtImport, "EXTMEMIMPORTEND", 
                                           (PFN*)(&(pLIDA->pfnMemImpEnd))) == 0);
     } /* endif */
     #endif

     // prepare segment logging
     if ( fOK )
     {
       UtlMakeEQFPath( pLIDA->szSegLog, NULC, LOG_PATH, NULL );
       UtlMkDir( pLIDA->szSegLog, 0, FALSE );
       strcat( pLIDA->szSegLog, "\\MEMIMPORT.LOG" );
#ifdef _DEBUG
       pLIDA->hSegLog = fopen( pLIDA->szSegLog, "wb" );
#endif
       if ( pLIDA->hSegLog )
       {
         fwrite( UNICODEFILEPREFIX, 1, 2, pLIDA->hSegLog );
       } /* endif */
     } /* endif */

     // call start entry point
     if ( fOK )
     {
       char szMemSourceLang[MAX_LANG_LENGTH];

       // close input file (if open) as external import process will open the file itself...
       if ( pLIDA->hFile ) 
       {
         UtlClose( pLIDA->hFile, FALSE );
         pLIDA->hFile = NULL; 
       } /* endif */

       memset( pLIDA->pstMemInfo, 0, sizeof(MEMEXPIMPINFO) );
       strcpy( pLIDA->pstMemInfo->szName, pLIDA->szMemName );
       pLIDA->pMem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );

       //fOK = (pLIDA->pfnMemImpStart( &(pLIDA->lExternalImportHandle), pLIDA->szFilePath, pLIDA->pstMemInfo ) == 0);
       fOK = (EqfPluginWrapper::MemImportStart(  &(pLIDA->lExternalImportHandle) , pLIDA->szFilePath, pLIDA->pstMemInfo ) == 0);
       
       pLIDA->pMem->getSourceLanguage( szMemSourceLang, sizeof(szMemSourceLang) );

       // check if memory source lanuage matchs source language of imported file
       if ( pLIDA->pstMemInfo->szSourceLang[0] && (strcmp( pLIDA->pstMemInfo->szSourceLang, szMemSourceLang ) != 0) )
       {
         USHORT usMBCode;

         pReplAddr[0] = pLIDA->pstMemInfo->szSourceLang;
         pReplAddr[1] = szMemSourceLang;
         usMBCode = LogMessage3(ERROR, __func__,  "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::", pReplAddr[0] );
         if ( usMBCode == MBID_CANCEL )
         {
           fOK = FALSE;
         } /* endif */
       } /* endif */

       // update memory description with description of imported memory
       if ( fOK )
       {
         pLIDA->pMem->setDescription( pLIDA->pstMemInfo->szDescription );
       } /* endif */

     } /* endif */

     pLIDA->fControlFound = TRUE; // avoid error message at end of import 

     //** GQ TODO: cleanup of resources in case of failures
   } /* endif */

   if ( !fOK )
   {
     // call import end function when available
     /*
     if ( pLIDA->pfnMemImpEnd != NULL )
     {
       pLIDA->pfnMemImpEnd( pLIDA->lExternalImportHandle );
     }//*/
     EqfPluginWrapper::MemImportEnd(pLIDA->lExternalImportHandle);

      // Clean the storage allocations if the MemLoadStart function failed.
      if ( pLIDA != NULL )
      {
         //--- Free different storage if allocated
         //--- checking for NULL is not neccessary, because UtlAlloc
         //--- handles this correctly
         if ( pLIDA->pFormatTable )
         {
           TAFreeTagTable( pLIDA->pFormatTable );
           pLIDA->pFormatTable = NULL;
         } /* endif */
         UtlAlloc( (PVOID *) &(pLIDA->pTokenList), 0L, 0L, NOMSG );

         // Issue the error message :"Initialization of import from
         // file %1 into translation memory %2 failed."
         pReplAddr[0] = pLIDA->szFilePath;
         pReplAddr[1] = pLIDA->szMemName;
         LogMessage3(ERROR, __func__,  "::ERROR_MEM_LOAD_INITFAILED::", pReplAddr[0] );

         if ( pLIDA->pProposal != NULL ) free(pLIDA->pProposal);

         UtlAlloc( (PVOID *) &pLIDA, 0L, 0L, NOMSG );
         *ppIda = NULL;

         // Dismiss the slider window if it had been created
//WINAPI
         //EqfRemoveObject( TWBFORCE, hWnd );
      } /* endif */
   } /* endif */

#ifdef IMPORT_LOGGING
  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];
    LONG64 lDummyTime = 0;

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\MEMIMP.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      time( &lImportStartTime );
      fprintf( hfLog, "************ Memory Import Log *********************\n"  );
      fprintf( hfLog, "Memory import started at   : %s", asctime( localtime( &lImportStartTime ) ) );
      fprintf( hfLog, "Memory name                : %s\n", pLIDA->szMemName );
      fclose( hfLog );
    } /* endif */
    GetElapsedTime( &lDummyTime );
  }
#endif
   time( &lImportStartTime );
   LogMessage2(DEBUG,__func__, "************ Memory Import Log *********************"  );
   LogMessage3(DEBUG,__func__, "Memory import started at   : ", asctime( localtime( &lImportStartTime ) ) );
   LogMessage3(DEBUG,__func__, "Memory name                : ", pLIDA->szMemName );


   return ((USHORT)fOK);
} /* end of function MemLoadStart */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemLoadProcess                                           |
//+----------------------------------------------------------------------------+
//|Function call:     static USHORT MemLoadProcess( PMEM_LOAD_IDA  pLIDA  )    |
//+----------------------------------------------------------------------------+
//|Description:       Import a segment to TM                                   |
//|                   read a block of data from a file if needed and add a     |
//|                   segment to the memory database.                          |
//+----------------------------------------------------------------------------+
//|Parameters:        pLIDA -pointer to import IDA                             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  MEM_PROCESS_OK     -no error                                              |
//|  MEM_DB_ERROR       -The TM is in error the load process should be stopped |
//|  MEM_FILE_SYN_ERR   -Nothing can be read from the file because no          |
//|                      no CONTROL token is available                         |
//|  MEM_READ_ERR       -error reading file                                    |
//|  MEM_SEG_SYN_ERR    -A segment has a syntax error and the load process     |
//|                      should be stopped                                     |
//|  MEM_PROCESS_END    -No more segments to add                               |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|   while ( no control token and no end token and no error )                 |
//|   {                                                                        |
//|      do syntax checking and set usRc                                       |
//|   }                                                                        |
//|                                                                            |
//|   if ( usRc is OK )                                                        |
//|   {                                                                        |
//|      switch ( tokenID )                                                    |
//|      {                                                                     |
//|         case ( ENFOFLIST token )                                           |
//|            if ( EOF flag is set )                                          |
//|               set usRc to MEM_PROCESS_END                                  |
//|            else                                                            |
//|              call MemLoadReadFile and set usRc                             |
//|            break;                                                          |
//|         case ( MEMCONTROL token )                                          |
//|            call function MemLoadPrepSeg to prepare input for TmReplace     |
//|            if ( MemLoadPrepSeg works OK )                                  |
//|            {                                                               |
//|               call TmReplace function                                      |
//|               update segment counter and do error handling                 |
//|            }                                                               |
//|            else                                                            |
//|            }                                                               |
//|               if ( end of token list but not EOF )                         |
//|               {                                                            |
//|                  call MemLoadReadFile and set usRc                         |
//|               }                                                            |
//|               else                                                         |
//|               {                                                            |
//|                  do error handling                                         |
//|               }                                                            |
//|            }                                                               |
//|            break;                                                          |
//|      }                                                                     |
//|   }                                                                        |
//|   return usRc                                                              |
//+----------------------------------------------------------------------------+
static USHORT MemLoadProcess( PMEM_LOAD_IDA  pLIDA  ) //pointer to the load IDA
{
   USHORT    usRc = MEM_PROCESS_OK;   // Function return code
   BOOL      fProcess = TRUE;         // process segment flag
   USHORT    usTmRc;                  // Tm return code
   USHORT    usResponse;              // return from UtlError
   PTOKENENTRY   pTEW;                // Equivalent for pLIDA->pTokenEntryWork
   USHORT    usSegmentIDLength;       // Length of segment ID in the extracted file
   PSZ       pReplAddr[2];            // Array of pointers to replacement strings
   PSZ_W     pReplAddrW[2];           // Array of pointers to replacement strings
   BOOL      fSegmentTooLarge = FALSE;// segment buffer overflow flag /*@1108A*/
   BOOL      fRestartImport = FALSE;  // TRUE = restart import as conversion has changed


   if ( (pLIDA->usImpMode == MEM_FORMAT_TMX) ||
        (pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS) ||
        (pLIDA->usImpMode == MEM_FORMAT_XLIFF_MT) )
   {
     pLIDA->lProgress = 0;

     //usRc = pLIDA->pfnMemImpProcess( pLIDA->lExternalImportHandle, MEMINSERTSEGMENT, (LONG)pLIDA, &(pLIDA->lProgress) );
     usRc = EqfPluginWrapper::MemImportProcess( pLIDA->lExternalImportHandle , MEMINSERTSEGMENT, (LONG)pLIDA, &(pLIDA->lProgress));

     // handle any error conditions
     if ( pLIDA->pstMemInfo->fError )  
     {
       PSZ pszMsg = pLIDA->pstMemInfo->szError;
       LogMessage3(ERROR, __func__,  "::ERROR_MEMIMP_ERROR::MEM_SEG_SYN_ERR::", pszMsg );
       usRc = MEM_SEG_SYN_ERR;
     } /* endif */

     // map return code to the ones used by memory export...
     switch ( usRc )
     {
       case 0 :
         usRc = MEM_PROCESS_OK;
         break;
       case MEM_IMPORT_COMPLETE: 
         usRc = MEM_PROCESS_END;
         break;
       case MEM_SEG_SYN_ERR:
         // leave as-is
         break;
       default:
         usRc = MEM_DB_ERROR;
         break;
     } /*endswitch */
   }
   else
   {
    usSegmentIDLength = SEGMENT_CLUSTER_LENGTH + SEGMENT_NUMBER_LENGTH;

    // Search a CONTROL or an ENDOFLIST token
    while ( pLIDA->pTokenEntryWork->sTokenid != pLIDA->sControlTokenID &&
            pLIDA->pTokenEntryWork->sTokenid != ENDOFLIST &&
            usRc == MEM_PROCESS_OK &&
            !fRestartImport )
    {
      pTEW = pLIDA->pTokenEntryWork;

      // If a MemoryDB token is found then indicate that this is a valid file
      if ( pTEW->sTokenid == pLIDA->sMemMemoryDBTokenID ||
            pTEW->sTokenid == pLIDA->sNTMMemoryDBTokenID )
      {
        pLIDA->fControlFound = TRUE;
        if ( pTEW->sTokenid == pLIDA->sNTMMemoryDBTokenID )
        {
          pLIDA->usTmVersion = NEW_TM;
        }
        else
        {
          pLIDA->usTmVersion = OLD_TM;
        } /* endif */
      } /* endif */

      // handle any codepage information
      if ( pTEW->sTokenid == pLIDA->sCodePageTokenID )
      {
        pTEW++;
        if ( pTEW->sTokenid == TEXT_TOKEN )
        {
          CHAR_W chTemp;

          // temporary terminate code page string
          chTemp = pTEW->pDataStringW[pTEW->usLength];
          pTEW->pDataStringW[pTEW->usLength] = EOS;

          // process code page string
          usRc = MemHandleCodePageValue( pLIDA, pTEW->pDataStringW, &fRestartImport );
          if ( usRc == NO_ERROR )
          {
            usRc = MEM_PROCESS_OK;

            if ( fRestartImport )
            {
              // restart import as conversion has been changed
              pLIDA->pTokenList->sTokenid = ENDOFLIST;
              pLIDA->fFirstRead = FALSE;
              pLIDA->ulFilled = 0;
              UtlChgFilePtr( pLIDA->hFile, 0L, FILE_BEGIN, &pLIDA->ulBytesRead, FALSE );
              pLIDA->ulBytesRead = 0;
              pLIDA->fEOF = FALSE;
              pLIDA->pTokenEntryWork = pLIDA->pTokenList;
            } /* endif */
          }
          else
          {
            usRc = MEM_FILE_SYN_ERR;
          } /* endif */

          pTEW->pDataStringW[pTEW->usLength] = chTemp;
        } /* endif */
      } /* endif */

      // handle any description
      if ( pTEW->sTokenid == pLIDA->sDescriptionTokenID )
      {
        pTEW++;
        if ( pTEW->sTokenid == TEXT_TOKEN )
        {
          // copy description to IDA
          {
            LONG   lMaxLen = sizeof(pLIDA->szDescription) - 1;
            PSZ_W  pszSource;
            PSZ    pszTarget, pszTemp;
            CHAR_W chTemp;

            // temporary terminate decscription string
            chTemp = pTEW->pDataStringW[pTEW->usLength];
            pTEW->pDataStringW[pTEW->usLength] = EOS;

            // skip leading whitespace
            pszSource = pTEW->pDataStringW;
            while ( isspace(*pszSource) ) pszSource++;

            // copy description up to end and ignore CR/LFs
            Unicode2ASCII( pszSource, pLIDA->szDescription, pLIDA->ulOemCP );
            pszTemp = pszTarget = pLIDA->szDescription;
            while ( *pszTemp && (lMaxLen > 0) )
            {
              if ( (*pszTemp != CR) && (*pszTemp != LF) )
              {
                *pszTarget++ = *pszTemp++;
              }
              else
              {
                pszTemp++;
              } /* endif */
              lMaxLen--;
            } /* endwhile */
            *pszTarget = EOS;
            pTEW->pDataStringW[pTEW->usLength] = chTemp;
          }

          // update TM description
          if ( !pLIDA->fMerge && pLIDA->szDescription[0] != EOS )
          {
            pLIDA->pMem->setDescription( pLIDA->szDescription );
          } /* endif */

          // send EQFN message to memory objects as the message send by
          // property handler when the properties are updated has
          // the wrong item class
          if ( (pLIDA->hwndErrMsg != HWND_FUNCIF) && (pLIDA->szDescription[0] != EOS) )
          {
            UtlMakeEQFPath( pLIDA->szTempPath, NULC, SYSTEM_PATH, NULL );
            strcat( pLIDA->szTempPath, BACKSLASH_STR );
            strcat( pLIDA->szTempPath, pLIDA->szMemName );
            EqfSend2AllObjects( clsMEMORY, WM_EQFN_PROPERTIESCHANGED,
                                MP1FROMSHORT(PROP_CLASS_MEMORY),
                                MP2FROMP(pLIDA->szTempPath) );
          } /* endif */
        } /* endif */
      } /* endif */

      if (pTEW->sTokenid == pLIDA->sSegmentTokenID)
      {
        pTEW++;
        if ( pTEW->sTokenid != ENDOFLIST )
        {
          // If the token following the segment token is 10 bytes Segment
          // Identification plus 2 bytes CRLF then is the syntax correct
          if (pTEW->sTokenid == TEXT_TOKEN &&
              pTEW->usLength <= usSegmentIDLength + 2 )
          {
              // Set the segment identification szSegmentID
              ULONG ulLen = Unicode2ASCIIBuf( pTEW->pDataStringW, pLIDA->szSegmentID, pTEW->usLength, sizeof(pLIDA->szSegmentID), pLIDA->ulOemCP );
              pLIDA->szSegmentID[ulLen] = EOS;
          }
          else
          {
              // The segment syntax is incorrect
              // Issue the message: Segment ID of segment %1 or following in file %2 is invalid.
              CHAR_W  szFilePath[ MAX_LONGFILESPEC ];
              CHAR_W  b = pTEW->pDataStringW[ pTEW->usLength ];
              pTEW->pDataStringW[ pTEW->usLength ] = EOS;
              pReplAddrW[0] = pTEW->pDataStringW;
              pReplAddrW[1] = ASCII2Unicode(pLIDA->szFilePath, szFilePath, pLIDA->ulOemCP );
              CloseFile( &(pLIDA->hFile));
              LogMessage5(ERROR, __func__, ":: ERROR_INVALID_SEGMENT_ID ::  pDataStringW = ", EncodingHelper::convertToUTF8(pReplAddrW[0]).c_str(), "; filepath: ", pLIDA->szFilePath);

              pTEW->pDataStringW[pTEW->usLength] = b ;
              usRc = MEM_SEG_SYN_ERR;
          } /* endif */
        } /* endif */
      } /* endif */
      if ( !fRestartImport )
      {
        pLIDA->pTokenEntryWork++;
      } /* endif */
    } /* end while */

    if (usRc ==  MEM_PROCESS_OK )
    {
      // If ENDOFLIST read file or terminate if EOF. If CONTROL then
      // start process.
      if ( pLIDA->pTokenEntryWork->sTokenid == ENDOFLIST )
      {
        // If EOF then terminate process successfully otherwise
        // read file and tokenize it.
        if ( pLIDA->fEOF )
        {
          usRc = MEM_PROCESS_END;
        }
        else
        {
          // Read block of data and tokenize it
          usRc = MemLoadReadFile( pLIDA );
        } /* endif */
      }
      else if ( pLIDA->pTokenEntryWork->sTokenid == pLIDA->sControlTokenID ) 
      {
          // A CONTROL token has been found
          pLIDA->fControlFound = TRUE;
          pLIDA->pTokenEntry = pLIDA->pTokenEntryWork;

          pLIDA->ulSequenceNumber++;           // Increase segment sequence number in any case

          // Prepare the data for the TmReplace call
          fProcess = NTMLoadPrepSeg( pLIDA, &fSegmentTooLarge );

          // If usProcess then  issue  the Replace call
          if (fProcess)
          {
            // handle adjust trailing blanks flag
            if ( pLIDA->fAdjustTrailingWhitespace )
            {
              BOOL fSegChanged = FALSE;
              BOOL fOK = TRUE;
              PSZ_W pszNewSeg = NULL;
              PSZ_W pWSList = NULL;

              pLIDA->pProposal->getSource( pLIDA->szSource, MAX_SEGMENT_SIZE );
              pLIDA->pProposal->getTarget( pLIDA->szSegBuffer, MAX_SEGMENT_SIZE );
              fOK = TAAdjustWhiteSpace( pLIDA->szSource, pLIDA->szSegBuffer, &pszNewSeg, FALSE, TRUE, &fSegChanged , pWSList);
              if ( fOK && fSegChanged )
              {
                pLIDA->pProposal->setTarget( pszNewSeg );
                UtlAlloc( (PVOID *)&pszNewSeg, 0L, 0L, NOMSG );
              } /* endif */
            } /* endif */

            // do not replace segment if source and target are identical and fIgnoreEqualSegments is set
            pLIDA->pProposal->getSource( pLIDA->szSource, MAX_SEGMENT_SIZE );
            pLIDA->pProposal->getTarget( pLIDA->szSegBuffer, MAX_SEGMENT_SIZE );
            if ( pLIDA->fIgnoreEqualSegments && (UTF16strcmp( pLIDA->szSource, pLIDA->szSegBuffer ) == 0) )
            {
              // ignore segment, but decrement segment counter as it is increased later on
              usTmRc = 0;
              pLIDA->ulSegmentCounter--;
            }
            else
            {

              GetElapsedTime( &lOtherTime );

              usTmRc = (USHORT)pLIDA->pMem->putProposal( *(pLIDA->pProposal) );

              GetElapsedTime( &lMemAccessTime );

              // count received MT segments and words if requested
              if ( pLIDA->fMTReceiveCounting )
              {
                ULONG ulWords = 0;
                ULONG ulMarkup = 0;
                USHORT usRC = 0;
                char szSourceLang[MAX_LANG_LENGTH];
                char szTagTable[MAX_FNAME];

                pLIDA->pProposal->getSourceLanguage( szSourceLang, sizeof(szSourceLang) );
                pLIDA->pProposal->getMarkup( szTagTable, sizeof(szTagTable) );
                usRC = EQFWORDCNTPERSEGW( pLIDA->szSource,szSourceLang, szTagTable, &ulWords, &ulMarkup );
                if ( ulWords < SIMPLE_SENT_BOUND )
                {
                  pLIDA->ulMTReceivedWords[0] += ulWords;
                  pLIDA->ulMTReceivedSegs[0]  += 1;
                }
                else if ( ulWords < MEDIUM_SENT_BOUND )
                {
                  pLIDA->ulMTReceivedWords[1] += ulWords;
                  pLIDA->ulMTReceivedSegs[1]  += 1;
                }
                else
                {
                  pLIDA->ulMTReceivedWords[2] += ulWords;
                  pLIDA->ulMTReceivedSegs[2]  += 1;
                } /* endif */
              } /* endif */
            } /* endif */

            if ( !usTmRc || 
                 usTmRc == NOT_REPLACED_OLD_SEGMENT ||
                 usTmRc == SEG_SKIPPED_BAD_MARKUP   ||
                 usTmRc == SEG_RESET_BAD_MARKUP     )
            {
              //--- if the TM call was OK
              if ( !usTmRc )
              {
                //--- Increase the segment counter
                pLIDA->ulSegmentCounter++;
              }
              else
              if ( usTmRc == SEG_SKIPPED_BAD_MARKUP )
              {
                //--- Increase the skipped segment counter
                pLIDA->ulInvSegmentCounter++;
              }
              else
              if ( usTmRc == SEG_RESET_BAD_MARKUP )
              {
                //--- Increase the markup table reset segment counter
                pLIDA->ulResetSegmentCounter++;
                usTmRc = 0 ;
              }
              else
              {
                //--- TM call returned NOT_REPLACED_OLD_SEGMENT         /*@47C*/
                //--- increase invalid segment counter                  /*@47C*/
                pLIDA->ulInvSegmentCounter++;

                //--- check if replace message should be displayed      /*@47A*/
                if ( pLIDA->fDisplayNotReplacedMessage )                /*@47A*/
                {                                                       /*@47A*/
                  //--- display replace confirmation message            /*@47A*/
                  OEMTOANSI( pLIDA->szMemName );
                  pReplAddr[0] = pLIDA->szMemName;                      /*@47A*/
                  if ( pLIDA->fBatch )
                  {
                    usResponse = MBID_YES;
                  }
                  else
                  {
                    usResponse = UtlError( ERROR_MEM_NOT_REPLACED_CONFIRM,/*@47C*/
                                          MB_YESNOCANCEL | MB_DEFBUTTON2,/*@47C*/
                                          1, &pReplAddr[0], EQF_QUERY ); /*@47C*/
                  } /* endif */
                  ANSITOOEM( pLIDA->szMemName );
                                                                        /*@47A*/
                  switch ( usResponse )                                 /*@47A*/
                  {                                                     /*@47A*/
                    //--------------------------------------------------/*@47A*/
                    case MBID_YES :                                     /*@47A*/
                      //--- do not display message                      /*@47A*/
                      pLIDA->fDisplayNotReplacedMessage = FALSE;        /*@47A*/
                      break;                                            /*@47A*/
                    //--------------------------------------------------/*@47A*/
                    case MBID_NO  :                                     /*@47A*/
                      //--- display message                             /*@47A*/
                      pLIDA->fDisplayNotReplacedMessage = TRUE;         /*@47A*/
                      break;                                            /*@47A*/
                    //--------------------------------------------------/*@47A*/
                    case MBID_CANCEL :                                  /*@47A*/
                      //--- cancel import process                       /*@47A*/
                      usRc = MEM_DB_CANCEL;                             /*@47A*/
                      break;                                            /*@47A*/
                    //--------------------------------------------------/*@47A*/
                    default :                                           /*@47A*/
                      break;                                            /*@47A*/
                  } /* endswitch */                                     /*@47A*/
                } /* endif */
              } /* endif */

              if ( usRc != MEM_DB_CANCEL )                              /*@47A*/
              {                                                         /*@47A*/
                // Advance pTokenEntry
                pLIDA->pTokenEntry = pLIDA->pTokenEntryWork;
                usRc = MEM_PROCESS_OK;
              } /* endif */                                             /*@47A*/
            }
            else
            {
              if ( (usTmRc == DB_FULL) || (usTmRc == BTREE_LOOKUPTABLE_TOO_SMALL) )
              {
                // show segment number of segment not processed anymore
                PSZ pszParms[2];

                pszParms[0] = pLIDA->szMemName;
                pszParms[1] = pLIDA->szSegmentID;
                LogMessage3(ERROR, __func__,  "::ERROR_MEM_DB_FULL_W_SEG::", *pszParms);
              }
              else
              {
                MemoryFactory *pFactory = MemoryFactory::getInstance();
                pFactory->showLastError( NULL, pLIDA->szMemName, pLIDA->pMem, pLIDA->hwndErrMsg );
              } /* endif */
              usRc = MEM_DB_ERROR;
            } /* endif */
          }
          else 
          // If usProcess = FALSE then check next token. If ENDOFLIST
          // and not fEOF read file otherwise issue a syntax error message
          {
            // If current token ENDOFLIST but fEOF still FALSE then read file
            if ( pLIDA->pTokenEntryWork->sTokenid == ENDOFLIST && !(pLIDA->fEOF) && !fSegmentTooLarge )
            {
              // Read block of data and tokenize it
              usRc = MemLoadReadFile( pLIDA );
            }
            else
            {
              if ( pLIDA->hFile )
              {
                if ( !pLIDA->fSkipInvalidSegments )
                {
                  USHORT usMsg = fSegmentTooLarge ? ERROR_MEM_SEGMENT_TOO_LARGE_IMP : ERROR_INVALID_SEGMENT;
                  pReplAddr[0] = pLIDA->szSegmentID;
                  pReplAddr[1] = pLIDA->szFilePath;
                  usResponse = LogMessage4(ERROR, __func__, ":: usMsg = " ,intToA(usMsg),pReplAddr[0]);
                }
                else
                {
                  usResponse = MBID_YES;
                } /* endif */

                if ( usResponse == MBID_EQF_YESTOALL  )
                {
                  pLIDA->fSkipInvalidSegments = TRUE;
                  usResponse = MBID_YES;
                } /* endif */

                // If reply from message is YES then skip the
                // segment. if CANCEL terminate the load process
                if ( usResponse == MBID_YES )
                {
                  // move pTokenEntry to the next token
                  pLIDA->pTokenEntry++;
                  pLIDA->ulInvSegmentCounter++;
                }
                else
                {
                  // set return code usRC to MEM_SEG_SYN_ERR
                  usRc =  MEM_SEG_SYN_ERR;
                } /* endif */
              }
              else
              {
                // lets finish (return code to be set is a don't care)
                usRc =  MEM_SEG_SYN_ERR;
              } /* endif */
            } /* endif */
          } /* endif */
      } /* endif */
    } /* endif */
   } /* endif */

   return usRc;
} /* end of function MemLoadProcess  */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemLoadReadFile                                          |
//+----------------------------------------------------------------------------+
//|Function call:     static USHORT MemLoadReadFile( PMEM_LOAD_IDA pLIDA )     |
//+----------------------------------------------------------------------------+
//|Description:       Read a block of data from the file to be loaded          |
//|                   into the textbuffer and tokenize it.                     |
//+----------------------------------------------------------------------------+
//|Parameters:        pLIDA -pointer to import IDA                             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  MEM_PROCESS_OK      -no error                                             |
//|  MEM_FILE_SYN_ERR    -if nothing can be read from the file because no      |
//|                       no CONTROL token is available                        |
//|  MEM_READ_ERR        -File read ERROR                                      |
//|  MEM_PROCESS_END     -There is nothing in the file to process              |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|  if ( first read flag is FALSE )                                           |
//|  {                                                                         |
//|      initialize buffer size                                                |
//|       call UtlRead and do error handling                                   |
//|   }                                                                        |
//|   else                                                                     |
//|   {                                                                        |
//|      if ( file have invalis syntax )                                       |
//|         do error handling and set usRc                                     |
//|      if ( usRc is OK )                                                     |
//|         get data and do error handling                                     |
//|      if ( usRc is OK )                                                     |
//|      {                                                                     |
//|          prepare data for function EQFTagTokenize                          |
//|          call function EQFTagTokenize and set usRc                         |
//|      }                                                                     |
//|   }                                                                        |
//|   return usRc                                                              |
//+----------------------------------------------------------------------------+
static USHORT MemLoadReadFile( PMEM_LOAD_IDA pLIDA ) // pointer to the load IDA
{
  USHORT    usRc = MEM_PROCESS_OK;     // Return code to calling function
  USHORT    usDosRc;                   // Read return code
  ULONG     ulBytesRead = 0;           // Number of Bytes read
  ULONG     ulBufferSize;              // Buffersize
  PSZ_W     pRest;                     // Pointer to end of tokenized input
  USHORT    usLastColPos;              // last column position
  ULONG     ulNotProcessed;            // Number of bytes not processed yet
  PSZ       pReplAddr[2];              // Array of pointers to replacement strings
//CHAR      szDosRc[5];                // Dos return code in character format

   // If fFirstRead = FALSE then process the first read otherwise
   // process a normal read with buffer shifting.
   if ( pLIDA->fFirstRead == FALSE )
   {
     // Read a full MEM_TEXT_BUFFER-1 and set fFirstRead to TRUE if the read was OK
     // otherwise set usRc to MEM_READ_ERR.
     // MemLoadAndConvert expects number of CHAR_Ws to be read and also returns number
     // of CHAR_Ws...
     ulBufferSize = MEM_TEXT_BUFFER-1;

     usDosRc = MemLoadAndConvert( pLIDA, pLIDA->szTextBuffer,
                                  ulBufferSize, &ulBytesRead );

     if (usDosRc)
     {
       // Issue error message "File %1 can not be found or read"
       CloseFile( &(pLIDA->hFile) );
       usRc = MEM_READ_ERR;
     } /* endif */

     // Set the flag to indicate that the first read has been performed
     pLIDA->fFirstRead = TRUE;

     // Initialize the number of bytes not processed yet
     ulNotProcessed = 0;
     pLIDA->ulFilled = ulBytesRead/sizeof(CHAR_W) + ulNotProcessed;
   }
   else                           /* process a normal read */
   {
     // If pTokenEntry still points to the beginning of the token list
     // which means to pTokenList then the file has
     // invalid syntax. Issue message "File has invalid syntax"
     // set usRc to MEM_FILE_SYN_ERR
     if (pLIDA->pTokenEntry == pLIDA->pTokenList)
     {
        // Close import file
        // Issue the message "The file %1 contains data which can not be processed"
        pReplAddr[0] = pLIDA->szFilePath;
        pReplAddr[1] = pLIDA->szSegmentID;
        CloseFile( &(pLIDA->hFile));
        LogMessage3(ERROR, __func__,  "::ERROR_MEM_INVALID_SGML::",  *pReplAddr);
        usRc = MEM_FILE_SYN_ERR;
     } /* endif */

     // If usRc == MEM_PROCESS_OK then
     //    -move the rest of the text buffer beginning with pDataString
     //     of pTokenEntry  to the start of the text buffer.
     //    -set pTokenEntry as well as pTokenEntryWork to pTokenList.
     //    -calculate remaining space in buffer.
     //    -fill remaining space of buffer by reading from file
     if ( usRc == MEM_PROCESS_OK )
     {
       // Move rest of text buffer to the beginning of the text buffer
       // ulBufferSize will contain the number of bytes to be shifted and read.
       // and ulNotProcessed represents the number of bytes not processed yet.
       ulBufferSize = pLIDA->pTokenEntry->pDataStringW - pLIDA->szTextBuffer;
       ulNotProcessed = pLIDA->ulFilled - ulBufferSize;
       memmove( (PBYTE)&pLIDA->szTextBuffer[0],
                (PBYTE)&pLIDA->pTokenEntry->pDataStringW[0],
                ulNotProcessed * sizeof(CHAR_W));

       ulBufferSize = MEM_TEXT_BUFFER-1 - ulNotProcessed;

       memset( (PBYTE)&pLIDA->szTextBuffer[ulNotProcessed], 0, ulBufferSize *sizeof(CHAR_W));
       // Set pTokenEntry as well as pTokenEntryWork to pTokenList.
       pLIDA->pTokenEntry = pLIDA->pTokenList;
       pLIDA->pTokenEntryWork = pLIDA->pTokenList;

       // Fill remaining space of buffer by reading from file

       usDosRc = MemLoadAndConvert( pLIDA, &(pLIDA->szTextBuffer[ulNotProcessed]),
                                    ulBufferSize, &ulBytesRead );
       if (usDosRc)
       {
         // Issue error message "File %1 can not be found or read"
         CloseFile( &(pLIDA->hFile) );
         usRc = MEM_READ_ERR;
       } /* endif */
       pLIDA->ulFilled = ulBytesRead/sizeof(CHAR_W) + ulNotProcessed;
     } /* endif */
   } /* endif */

   if ( usRc == MEM_PROCESS_OK )
   {
     // Increase the number of total bytes read -- if ASCII/ANSI, we have to take real
     // file position
     if ( pLIDA->usImpMode == SGMLFORMAT_ASCII || pLIDA->usImpMode == SGMLFORMAT_ANSI)
     {
        UtlChgFilePtr( pLIDA->hFile, 0L, FILE_CURRENT, &pLIDA->ulBytesRead, FALSE );
     }
     else
     {
        pLIDA->ulBytesRead += ulBytesRead;
     }

     // If total bytes read still 0 then end the load process
     // by setting the appropriate return code
     if ( !pLIDA->ulBytesRead )
     {
       usRc = MEM_PROCESS_END;
     } /* endif */

     // Invoke the tag tokenize process only if the text buffer is not zero length
     // because EQFTagTokenize crashes if the buffer starts with '\0'
     if ( pLIDA->szTextBuffer[0] != NULC )
     {
       // Call the tokenize process
       TATagTokenizeW( pLIDA->szTextBuffer, pLIDA->pFormatTable, pLIDA->fEOF,
                      &pRest, &usLastColPos, pLIDA->pTokenList, NUMB_OF_TOKENS);

       if ( pLIDA->pTokenList->sTokenid == pLIDA->sMemMemoryDBTokenID )
       {
         INT_PTR   iDlgRc = 1;
         HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

         /*************************************************************/
         /* This is an old TM format display listbox to get           */
         /* source and target language                                */
         /*************************************************************/
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
         DIALOGBOX( QUERYACTIVEWINDOW(), NTMTagLangDlg, hResMod,
                    ID_NTM_TAGLANG_DLG, pLIDA, iDlgRc );
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
         if ( iDlgRc )
         {
           usRc = MEM_PROCESS_END;
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */
   return usRc;
} /* end of function MemLoadReadFile */

static BOOL
NTMLoadPrepSeg( PMEM_LOAD_IDA  pLIDA, PBOOL pfSegmentTooLarge ) //pointer to the load IDA  area
{
   BOOL          fOk = TRUE;
   PSZ_W         pszWork = NULL;
   PTOKENENTRY   pTEW;
   USHORT        usWork;
//   PSZ           pszString;
   PSZ_W         pszStringW;
   USHORT        usPos;
   CHAR_W        szX15String[ 512 ];        // string array for 0x15 characters
   CHAR_W        chParse = X15;             // character used to parse control record

   /*******************************************************************/
   /* get pointer to work token entry an TMX_PUT structure            */
   /*******************************************************************/
   pTEW   = pLIDA->pTokenEntryWork;

   pLIDA->pProposal->clear();

   /*******************************************************************/
   /* check if next token is a text token                             */
   /* if not stop further processing                                  */
   /*******************************************************************/
   pTEW++;
   if (pTEW->sTokenid != TEXT_TOKEN )
   {
      fOk = FALSE;
   } /* endif */

   // If fOk. Check if the control information do have a prefix and a suffix
   // of CR and LF and check also if the real control data do have a size of
   // sizeof(SEG_CTRL_DATA). If not set fOk to FALSE.
   if (fOk)
   {
     /*****************************************************************/
     /* check if the control data starts ands ends with CRLF          */
     /* if not stop further processing                                */
     /*****************************************************************/
     if ( pTEW->usLength < sizeof( szX15String ))
     {
       memcpy( szX15String, pTEW->pDataStringW, pTEW->usLength * sizeof(CHAR_W));
       szX15String[ pTEW->usLength ] = EOS;
       /*****************************************************************/
       /* get rid of CRLF at the end ...                                */
       /*****************************************************************/
       usPos = pTEW->usLength - 1;
       while ( (usPos > 0) &&
               ((szX15String[usPos] == CR) || (szX15String[usPos] == LF)) )
       {
         szX15String[usPos] = EOS;
         usPos --;
       } /* endwhile */
     }
     else
     {
       fOk = FALSE;
     } /* endif */
     pszWork = szX15String;

     if ( fOk && ( memcmp( pszWork, CRLF_STRINGW, UTF16strlenBYTE(CRLF_STRINGW) ) != 0 ) &&
          ( memcmp( pszWork+(pTEW->usLength-UTF16strlenCHAR(CRLF_STRINGW)), CRLF_STRINGW,
                    UTF16strlenBYTE(CRLF_STRINGW) ) !=0 ) )
     {
       /***************************************************************/
       /* control data "has NO" CRLF at beginning and end             */
       /* stop further processing                                     */
       /***************************************************************/
       fOk = FALSE;
     } /* endif */
   } /* endif */

   if (fOk)
   {
     // skip the first CRLF of the source string
     pszWork += 2;

     // GQ: Check if control string uses X15 as seperator, due to a bug in the
     // conversion to Ansi, there may be exported memory databases which uses a
     // different character
     if ( fOk )
     {
       PSZ_W pszX15 = wcschr( pszWork, X15 );
       if ( pszX15 != NULL )
       {
         chParse = X15;
       }
       else
       {
          PSZ_W pszDelimiter = pszWork;
          while ( iswdigit( *pszDelimiter ) )
          {
            pszDelimiter++;
          } /* endwhile */

          if ( *pszDelimiter == X15 )
          {
            // use X15 a delimiter
            chParse = X15;
          }
          else if ( (*pszDelimiter != EOS) && (pszDelimiter != pszWork) )
          {
            // use given delimiter
            // the code could be combined with the assignment above
            // but it has been splitted to allow to set a breakpoint for
            // this condition
            chParse = *pszDelimiter;
          }
          else
          {
            fOk = FALSE;
          }
       } /* endif */
     } /* endif */

     // get the seg. Nr
     pszStringW = UtlParseCharW( pszWork, NTM_SEGNR_ID, chParse );


     /*****************************************************************/
     /* check if the seg Nr. contains only digits                     */
     /* if not stop further processing                                */
     /*****************************************************************/
     usWork = 0;
     while ( usWork < SEG_NUMB && fOk == TRUE)
     {
        if ( !iswdigit( pszStringW[usWork] ) )
        {
          fOk = FALSE;
        } /* endif */
        usWork++;
     } /* endwhile */


     if (fOk)
     {
       /***************************************************************/
       /* seg. Nr Ok. copy it to put structure                        */
       /***************************************************************/
       pLIDA->pProposal->setSegmentNum( wcstol(pszStringW, 0, 10));
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get the MT flag and copy it to put structure                  */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_MTFLAG_ID, chParse );


     switch (*pszStringW)
     {
       case '0':
         pLIDA->pProposal->setType( OtmProposal::eptManual );
        break;
       case '1':
         pLIDA->pProposal->setType( OtmProposal::eptMachine );
        break;
       case '2':
         pLIDA->pProposal->setType( OtmProposal::eptGlobalMemory );
        break;
       default:
         fOk = FALSE;
        break;
     } /* endswitch */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get time stamp, check if it contains only digits              */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_TIME_ID, chParse );
     usWork = 0;
     while ( usWork < TIME_STAMP && fOk == TRUE)
     {
        if ( !isdigit( (UCHAR) pszStringW[usWork] ) )
        {
          fOk = FALSE;
        } /* endif */
        usWork++;
     } /* endwhile */

     if (fOk)
     {
       /***************************************************************/
       /* time stamp Ok. copy it to put structure                     */
       /***************************************************************/
       pLIDA->pProposal->setUpdateTime( wcstol(pszStringW, 0, 10) );
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get source language and check for max allowed length and EOS  */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_SOURCELANG_ID, chParse );

     if (  UTF16strlenCHAR( pszStringW ) > MAX_LANG_LENGTH || pszStringW[0] == EOS)
     {
       fOk = FALSE;
     }
     else
     {
       char szLang[MAX_LANG_LENGTH];
       Unicode2ASCII( pszStringW, szLang, pLIDA->ulOemCP );
       pLIDA->pProposal->setSourceLanguage( szLang );
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get target language and check for max allowed length          */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_TARGETLANG_ID, chParse );

     if (  UTF16strlenCHAR( pszStringW) > MAX_LANG_LENGTH || pszStringW[0] == EOS)
     {
       fOk = FALSE;
     }
     else
     {
       char szLang[MAX_LANG_LENGTH];
       Unicode2ASCII( pszStringW, szLang, pLIDA->ulOemCP );
       pLIDA->pProposal->setTargetLanguage( szLang );
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get author name                                               */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_AUTHORNAME_ID, chParse );
     if (  UTF16strlenCHAR( pszStringW) > MAX_USERID )
     {
       fOk = FALSE;
     }
     else
     {
       char szAuthor[MAX_USERID];
       Unicode2ASCII( pszStringW, szAuthor, pLIDA->ulOemCP );
       pLIDA->pProposal->setAuthor( szAuthor );
     } /* endif */

   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get tagtable name                                             */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_TAGTABLENAME_ID, chParse );

     if (  UTF16strlenCHAR( pszStringW) > MAX_FNAME || pszStringW[0] == EOS)
     {
       fOk = FALSE;
     }
     else
     {
       char szString[MAX_FNAME];
       Unicode2ASCII( pszStringW, szString, pLIDA->ulOemCP );
       pLIDA->pProposal->setMarkup( szString );
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get file name and check for maximal length                    */
     /* if to long stop further processing                            */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_FILENAME_ID, chParse );

     if (  UTF16strlenCHAR( pszStringW) > MAX_FILESPEC || pszStringW[0] == EOS)
     {
       fOk = FALSE;
     }
     else
     {
       char szString[MAX_FILESPEC];
       Unicode2ASCII( pszStringW, szString, pLIDA->ulOemCP );
       pLIDA->pProposal->setDocShortName( szString );
     } /* endif */
   } /* endif */

   if (fOk)
   {
     /*****************************************************************/
     /* get long name and check for maximal length                    */
     /* if to long stop further processing                            */
     /*****************************************************************/
     pszStringW = UtlParseCharW( pszWork, NTM_LONGNAME_ID, chParse );

     if (  UTF16strlenCHAR( pszStringW) > MAX_LONGFILESPEC )
     {
       fOk = FALSE;
     }
     else
     {
       char szString[MAX_LONGFILESPEC];
       Unicode2ASCII( pszStringW, szString, pLIDA->ulOemCP );
       pLIDA->pProposal->setDocName( szString );
     } /* endif */
   } /* endif */

   // Process any context information
   if ( fOk )
   {
     if ( MemLPCheckForTag( pTEW, pLIDA->sContextTokenID ) )
     {
       fOk = MemLPExtractValue( &pTEW, pLIDA->sContextTokenID, pLIDA->sContextEndTokenID, pLIDA->szSegBuffer, MAX_SEGMENT_SIZE, pfSegmentTooLarge );
       if ( fOk ) pLIDA->pProposal->setContext( pLIDA->szSegBuffer );
     } /* endif */      
   } /* endif */      

   // Process any addinfo information
   if ( fOk )
   {
     if ( MemLPCheckForTag( pTEW, pLIDA->sAddInfoTokenID ) )
     {
       fOk = MemLPExtractValue( &pTEW, pLIDA->sAddInfoTokenID, pLIDA->sAddInfoEndTokenID, pLIDA->szSegBuffer, MAX_SEGMENT_SIZE, pfSegmentTooLarge );

       // do any match segment ID processing
       if ( fOk && pLIDA->fCreateMatchID )
       {
         MADAddMatchSegID( pLIDA->szSegBuffer, pLIDA->szMatchIDPrefixW, pLIDA->ulSequenceNumber, pLIDA->fForceNewMatchID );
       } /* endif */

       if ( fOk ) pLIDA->pProposal->setAddInfo( pLIDA->szSegBuffer );
     }
     else if ( pLIDA->fCreateMatchID ) 
     {
       //swprintf( pLIDA->szSegBuffer, "<MatchSegID ID=\"%s%lu\"/>", pLIDA->szMatchIDPrefixW, pLIDA->ulSequenceNumber );
       pLIDA->pProposal->setAddInfo( pLIDA->szSegBuffer );
     } /* endif */      
   } /* endif */      

   // get segment source
   if ( fOk )
   {
     fOk = MemLPExtractValue( &pTEW, pLIDA->sSourceTokenID, pLIDA->sSourceEndTokenID, pLIDA->szSegBuffer, MAX_SEGMENT_SIZE, pfSegmentTooLarge );
     if ( fOk ) pLIDA->pProposal->setSource( pLIDA->szSegBuffer );
   } /* endif */      

   // get segment target
   if ( fOk )
   {
     fOk = MemLPExtractValue( &pTEW, pLIDA->sTargetTokenID, pLIDA->sTargetEndTokenID, pLIDA->szSegBuffer, MAX_SEGMENT_SIZE, pfSegmentTooLarge );
     if ( fOk ) pLIDA->pProposal->setTarget( pLIDA->szSegBuffer );
   } /* endif */      

   if( fOk)
   {
     if ( pLIDA->szSegBuffer [0] == EOS ) pLIDA->pProposal->setTarget( EMPTY_TAG );
   } /* endif */

   /*******************************************************************/
   /* if we are at the end of a token list and not everything read in */
   /* try to read in a new chunk and try operation again --> recursion*/
   /*******************************************************************/
   if ( !fOk && (pTEW->sTokenid == ENDOFLIST) && !(pLIDA->fEOF) )
   {
     // Read block of data and tokenize it
     if ( MemLoadReadFile( pLIDA ) == MEM_PROCESS_OK )
     {
       fOk = NTMLoadPrepSeg( pLIDA, pfSegmentTooLarge );
     } /* endif */
   }
   else
   {
     /*******************************************************************/
     /* Set pTokenEntryWork to the actual value                         */
     /*******************************************************************/
     pLIDA->pTokenEntryWork = pTEW;
   } /* endif */

   return (fOk);
} /* end of function NTMLoadPrepSeg */

static VOID MemReduceCRLF
(
  PSZ_W      pString                  // string to be processed
)
{
  PSZ_W   pszFrom, pszTo;

  pszFrom = pszTo = pString;

  while ( *pszFrom )
  {
    if ( *pszFrom == CR )
    {
      if ( pszFrom[1] != LF )
      {
        // add CR as LF
        *pszTo++ = LF;
        pszFrom++;
      }
      else
      {
        // ignore CR
        pszFrom++;
      } /* endif */
    }
    else
    {
      // copy as-is
      *pszTo++ = *pszFrom++;
    } /* endif */
  } /* endwhile */
  *pszTo = EOS;
} /* end of function MemReduceCRLF */

USHORT MemFuncImportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  PSZ         pszUnused1,              // not in use, for future enhancements
  PSZ         pszUnused2,              // not in use, for future enhancements
  LONG        lOptions                 // options for Translation Memory import
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // when running in COMPLETE_IN_ONE_CALL_OPT mode we use our own data area to be independent of other API calls running in parallel
  if ( lOptions & COMPLETE_IN_ONE_CALL_OPT )
  {
    PFCTDATA pPrivateData = (PFCTDATA)malloc( sizeof( FCTDATA ) );
    memset( pPrivateData, 0, sizeof( FCTDATA ) );
    pPrivateData->fComplete = TRUE;
    pPrivateData->usProgress = 0;
    usRC = MemFuncPrepImport( pPrivateData, pszMemName, pszInFile, pszTM_ID, pszStoreID, pszUnused1, pszUnused2, lOptions );
    if ( usRC == 0 )
    {
      while ( !pPrivateData->fComplete )
      {
        usRC = MemFuncImportProcess( pPrivateData );
      }
      usRC = pPrivateData->usMemLoadRC;
    }
    free( pPrivateData );
  }
  else
  {
    // prepare a new import or continue current one
    if ( pData->fComplete )              // has last run been completed
    {
      // prepare a new analysis run
      pData->usProgress = 0;
      usRC = MemFuncPrepImport( pData, pszMemName, pszInFile, pszTM_ID, pszStoreID, pszUnused1, pszUnused2, lOptions );
    }
    else
    {
      // continue current analysis process
      usRC = MemFuncImportProcess( pData );
    } /* endif */
  }
  return( usRC );
} /* end of function MemFuncImportMem */

// prepare the function I/F TM import
USHORT MemFuncPrepImport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  PSZ         pszUnused1,              // not in use, for future enhancements
  PSZ         pszUnused2,              // not in use, for future enhancements
  LONG        lOptions                 // options for Translation Memory import
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // error parameter pointer
  PMEM_LOAD_DLG_IDA pLoadIDA = NULL;   // Pointer to the load IDA

  pszUnused1; pszUnused2;
  bool fOkFailHandled = false;
   /*******************************************************************/
   /* Allocate storage for the MEM_LOAD_DLG_IDA. The area will be free*/
   /* in the function  EQFMemloadStart. Only in case of an error      */
   /* it will be freed here.                                          */
   /*******************************************************************/
   fOK = UtlAllocHwnd( (PVOID *)&pLoadIDA, 0L, (LONG)sizeof(MEM_LOAD_DLG_IDA), ERROR_STORAGE, HWND_FUNCIF );
    if ( fOK )
    {  // use def.target lang. from system preferences of TM
          pLoadIDA->ulOemCP = GetLangOEMCP(NULL);
          pLoadIDA->ulAnsiCP = GetLangAnsiCP(NULL);
    }else if(!fOkFailHandled){
      fOkFailHandled = true;
      LogMessage(ERROR,"MemFuncPrepImport:: memory allocation fails");
    }

   // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
       usRC = TMT_MANDCMDLINE;
       LogMessage2(ERROR, __func__,  "::TMT_MANDCMDLINE");
     } /* endif */
   }else if(!fOkFailHandled){ 
      fOkFailHandled = true;
      LogMessage(ERROR,"MemFuncPrepImport:: check if a TM has been specified fails");
   }/* endif */
   // set options for import
   if ( fOK )
   {
     pLoadIDA->fSkipInvalidSegments = ( lOptions & IGNORE_OPT );
     pLoadIDA->fAscii = TRUE;   // TM in external format

     if ( (lOptions & TMX_OPT) && (lOptions & CLEANRTF_OPT) )
     {
       pLoadIDA->usImpMode = MEM_FORMAT_TMX_TRADOS;
     }
     else if ( lOptions & TMX_OPT )
     {
       pLoadIDA->usImpMode = MEM_FORMAT_TMX;
     }
     else if ( lOptions & XLIFF_MT_OPT )
     {
       pLoadIDA->usImpMode = MEM_FORMAT_XLIFF_MT;
     }
     else if ( lOptions & UTF16_OPT )
     {
       pLoadIDA->usImpMode = SGMLFORMAT_UNICODE;
     }
     else if ( lOptions & ANSI_OPT )
     {
       pLoadIDA->usImpMode = SGMLFORMAT_ANSI;
     }
     else
     {
       pLoadIDA->usImpMode = SGMLFORMAT_ASCII;
     } /* endif */

     if ( lOptions & CANCEL_UNKNOWN_MARKUP_OPT ) 
     {
        UtlSetUShort( QS_MEMIMPMRKUPACTION, MEMIMP_MRKUP_ACTION_CANCEL );
     }
     else if ( lOptions & SKIP_UNKNOWN_MARKUP_OPT )
     {
        UtlSetUShort( QS_MEMIMPMRKUPACTION, MEMIMP_MRKUP_ACTION_SKIP );
     }
     else if ( lOptions & GENERIC_UNKNOWN_MARKUP_OPT )
     {
        UtlSetUShort( QS_MEMIMPMRKUPACTION, MEMIMP_MRKUP_ACTION_RESET );
     }

     if ( lOptions & FORCENEWMATCHID_OPT )
     {
       pLoadIDA->fForceNewMatchID = TRUE;
     } /* endif */
   } /* endif */

   // prepare segment match ID prefix
   if ( fOK )
   {
     pLoadIDA->fCreateMatchID = MADPrepareMatchSegIDPrefix( pszTM_ID, pszStoreID, pLoadIDA->szMatchIDPrefix );

     if ( pLoadIDA->fCreateMatchID  ) {
        LogMessage(INFO,"pLoadIDA->fCreateMatchID is true, but converting MultiByteToWideChar is not implemented");
        //MultiByteToWideChar( CP_ACP, 0, pLoadIDA->szMatchIDPrefix, -1, pLoadIDA->szMatchIDPrefixW, sizeof(pLoadIDA->szMatchIDPrefixW)/sizeof(CHAR_W) );
        //EncodingHelper::MultiByteToWideChar(CP_ACP, 0, pLoadIDA->szMatchIDPrefix, -1, pLoadIDA->szMatchIDPrefixW, sizeof(pLoadIDA->szMatchIDPrefixW)/sizeof(CHAR_W) );
     }

   }else if(!fOkFailHandled){
     fOkFailHandled = true;
     LogMessage(ERROR,"MemFuncPrepImport::set options for import fails");
   } /* endif */

   /*******************************************************************/
   /* Check if there is a TM with the given name                      */
   /*******************************************************************/
   if ( fOK )
   {
     strcpy( pLoadIDA->szMemName, pszMemName );
     MemoryFactory *pFactory = MemoryFactory::getInstance();
     if ( !pFactory->exists( NULL, pszMemName ) )
     {
       LogMessage2(ERROR, "Mem file not found:: ", pszMemName);
	     fOK = FALSE;
       pszParm = pszMemName;
       usRC = ERROR_MEMORY_NOTFOUND;
       LogMessage3(ERROR, __func__,  "::ERROR_MEMORY_NOTFOUND::", pszParm);
     } /* endif */
  }else if(!fOkFailHandled){
     fOkFailHandled = true;
     LogMessage(ERROR,"MemFuncPrepImport::prepare segment match ID prefix fails");
  } /* endif */

   /*******************************************************************/
   /* Open the TM                                                     */
   /*******************************************************************/
   if ( fOK )
   {
     int iRC = 0;
     MemoryFactory *pFactory = MemoryFactory::getInstance();
     pLoadIDA->pMem = pFactory->openMemory( NULL, pszMemName, EXCLUSIVE, &iRC );
     if ( iRC != 0 )
     {
	     fOK = FALSE;
       pFactory->showLastError( NULL, pszMemName, pLoadIDA->pMem, (HWND)HWND_FUNCIF );
     } /* end */        
  }else if(!fOkFailHandled){
      fOkFailHandled = true;
      LogMessage(ERROR,"MemFuncPrepImport::Check if there is a TM with the given name fails");
  } /* endif */

   /*******************************************************************/
   /* Check if input file exists                                      */
   /*******************************************************************/
   if ( fOK )
   {
     if ( !UtlFileExist( pszInFile ) )
     {
       pszParm = pszInFile;
       LogMessage3(ERROR, __func__,  "::FILE_NOT_EXISTS::",pszParm);
       fOK = FALSE;
     } /* endif */
  }else if(!fOkFailHandled){
     fOkFailHandled = true;
     LogMessage(ERROR,"MemFuncPrepImport::Open the TM fails");
  } /* endif */

   /*******************************************************************/
   /* Open the input file                                             */
   /*******************************************************************/
   if ( fOK )
   {
     //USHORT      usDosRc;              // Return code from Dos functions
     USHORT      usAction;             // Action for DosOpen

     strcpy( pLoadIDA->szFilePath, pszInFile );
     pLoadIDA->hFile = FilesystemHelper::OpenFile(pszInFile, "r", false);
     /*usDosRc = UtlOpenHwnd ( pLoadIDA->szFilePath, //filename
                       &(pLoadIDA->hFile),    //file handle (out)
                       &usAction,      // action taken (out)
                       0L,             // Create file size
                       FILE_NORMAL,    // Normal attributes
                       FILE_OPEN,      // Open if exists else fail
                       OPEN_SHARE_DENYWRITE,// Deny Write access
                       0L,             // Reserved but handle errors
                       TRUE,           // display error message
                       HWND_FUNCIF );//*/
     if( pLoadIDA->hFile == NULLHANDLE )
     {
        fOK = FALSE;
     }
     //if ( usDosRc )  //--- error from open
     //{
     //   //--- Reset file handle to NULL and set fOk to FALSE
     //   pLoadIDA->hFile = NULLHANDLE;
     //   fOK = FALSE;
     //}/* endif */
   }else if(!fOkFailHandled){
      fOkFailHandled = true;
      LogMessage(ERROR,"MemFuncPrepImport::Check if input file exists fails");
   } /* endif */

   /*******************************************************************/
   /* Anchor IDA in function call data area                           */
   /*******************************************************************/
   if ( fOK )
   {
     pLoadIDA->fBatch     = TRUE;
     pLoadIDA->hwndErrMsg = HWND_FUNCIF;
     pData->pvMemLoadIda = (PVOID)pLoadIDA;
   }else if(!fOkFailHandled){
      fOkFailHandled = true;
      LogMessage(ERROR,"MemFuncPrepImport::Open the input file  fails");
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
      LogMessage(ERROR,"MemFuncPrepImport::fOk in false, making Cleanup");
      if ( pLoadIDA )
      {
         if ( pLoadIDA->pMem )
         {
           MemoryFactory *pFactory = MemoryFactory::getInstance();
           pFactory->closeMemory( pLoadIDA->pMem );
           pLoadIDA->pMem = NULL;
         } /* endif */
         if ( pLoadIDA->hFile )
         {
            UtlClose( pLoadIDA->hFile, FALSE );
         } /* endif */
         UtlAlloc( (PVOID *) &pLoadIDA, 0L, 0L, NOMSG );
      } /* endif */
   } /* endif */

   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->usMemLoadPhase = MEM_START_IMPORT;
     pData->usMemLoadRC    = NO_ERROR;
   }
   else if(!fOkFailHandled)
   {
     fOkFailHandled = true;
     LogMessage(ERROR,"MemFuncPrepImport::fOk in false, making Cleanup");
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */
   return( usRC );

} /* end of function MemFuncPrepImport */

// function call interface TM import
USHORT MemFuncImportProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PMEM_LOAD_DLG_IDA pDialogIDA;        // Pointer to the DLG load IDA
  PMEM_LOAD_IDA     pLoadData;         // Pointer to the load IDA
  USHORT            usPhase;           // current processing phase

  pLoadData = (PMEM_LOAD_IDA)pData->pvMemLoadIda;
  UtlSetUShort( QS_LASTERRORMSGID, 0 );
  usPhase = pData->usMemLoadPhase;

  switch ( usPhase )
  {
    case MEM_START_IMPORT:
       LogMessage2(INFO, __func__, "::MEM_START_IMPORT");
       pData->usProgress = 0;
       pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;

       if ( !MemLoadStart( &(pData->pvMemLoadIda), HWND_FUNCIF ) )
       {
         MemoryFactory *pFactory = MemoryFactory::getInstance();
         pFactory->closeMemory( pDialogIDA->pMem );
         if ( pDialogIDA->hFile ) CloseFile( &(pDialogIDA->hFile));
         pData->usMemLoadRC = UtlQueryUShort( QS_LASTERRORMSGID );
         if ( pData->usMemLoadRC == 0 ) pData->usMemLoadRC = ERROR_MEMIMP_ERROR;
         usPhase = 0;
       }
       else
       {
         usPhase = MEM_IMPORT_TASK;
       } /* endif */
       UtlAlloc( (PVOID *) &pDialogIDA, 0L, 0L, NOMSG );
      break;

    case MEM_IMPORT_TASK:
      {
        
        LogMessage3(INFO, __func__, "::MEM_IMPORT_TASK, progress = " , intToA(pData->usProgress));
        USHORT usRc = MemLoadProcess( pLoadData );
        switch ( usRc )
        {
          case MEM_PROCESS_OK:
            {
              INT64 iComplete = (INT64)pLoadData->ulBytesRead;
              INT64 iTotal = (INT64)pLoadData->ulTotalSize;
              INT64 iRatio = iComplete * (INT64)100 / iTotal;
              pData->usProgress  = (USHORT)iRatio;
            }
            usPhase = MEM_IMPORT_TASK;
            break;
          case MEM_PROCESS_END:
            pData->usProgress = 100;
            pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
            if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
            if ( pLoadData->pMem )
            {
              // organize index file of imported memory
              pLoadData->pMem->rebuildIndex();

              MemoryFactory *pFactory = MemoryFactory::getInstance();
              pFactory->closeMemory( pLoadData->pMem);
              pLoadData->pMem = NULL;
            } /* endif */
            usPhase = MEM_END_IMPORT;
            break;
          case MEM_DB_CANCEL :
            usPhase = MEM_END_IMPORT;
            break;
          default:
            pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
            if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
            usRC = pData->usMemLoadRC = UtlQueryUShort( QS_LASTERRORMSGID );
            usPhase = MEM_END_IMPORT;
            break;
        } /* end switch */
      }
      break;

    case MEM_END_IMPORT:
       if ( pLoadData != NULL )
       {
         if ( ( pLoadData->usImpMode == MEM_FORMAT_TMX ) || 
              ( pLoadData->usImpMode == MEM_FORMAT_TMX_TRADOS ) ||
              ( pLoadData->usImpMode == MEM_FORMAT_XLIFF_MT ) )
         {
           //if ( pLoadData->pfnMemImpEnd )     pLoadData->pfnMemImpEnd( pLoadData->lExternalImportHandle );
           EqfPluginWrapper::MemImportEnd(pLoadData->lExternalImportHandle);

           if ( pLoadData->pstMemInfo )       UtlAlloc( (PVOID *)&pLoadData->pstMemInfo, 0L, 0L, NOMSG );
           if ( pLoadData->pstSegment)        UtlAlloc( (PVOID *)&pLoadData->pstSegment, 0L, 0L, NOMSG );

       #ifdef TO_BE_REMOVED
           if ( pLoadData->hmodMemExtImport ) DosFreeModule( &(pLoadData->hmodMemExtImport) );
           #endif
         } /* endif */

         if ( pLoadData->pMem )
         {
           MemoryFactory *pFactory = MemoryFactory::getInstance();
           pFactory->closeMemory( pLoadData->pMem);
           pLoadData->pMem = NULLHANDLE;
         } /* endif */
         pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
         if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
         if ( pLoadData->pFormatTable )
         {
           TAFreeTagTable( pLoadData->pFormatTable );
           pLoadData->pFormatTable = NULL;
         } /* endif */
              
              LONG lCurTime = 0;  

              time( &lCurTime );
              LogMessage9(TRANSACTION, __func__ , "::Memory import ended at     : ", asctime( localtime( &lCurTime )),
                            "\n\tNumber of segments imported : ", intToA(pLoadData->ulSegmentCounter) ,
                           "\n\tNumber of invalid segments  : ", intToA(pLoadData->ulInvSegmentCounter), 
                           "\n\tNumber of OTMUTF8 segments  : ", intToA(pLoadData->ulResetSegmentCounter) );
            
              if ( lImportStartTime )
              {
                LONG lDiff = lCurTime - lImportStartTime;
                char buff[256];
                sprintf( buff, "Overall import time is %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
                        (lDiff - (lDiff / 3600 * 3600)) / 60,
                        (lDiff - (lDiff / 3600 * 3600)) % 60 );
                LogMessage9(TRANSACTION, __func__ , "Memory update time ",intToA(lMemAccessTime)," [ms]\nFile read time     ",
                    intToA(lFileReadTime), " [ms]\nOther times        ",intToA(lOtherTime), " [ms]\n Overall import time is " , buff);
              }

              #ifdef TEMPORARY_COMMENTED
              if ( lImportStartTime )
              {
                LONG lDiff = lCurTime - lImportStartTime;
                fprintf( hfLog, "Overall import time is %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
                        (lDiff - (lDiff / 3600 * 3600)) / 60,
                        (lDiff - (lDiff / 3600 * 3600)) % 60 );
                fprintf( hfLog, "Memory update time %I64d [ms]\n", lMemAccessTime );
                fprintf( hfLog, "File read time     %I64d [ms]\n", lFileReadTime );
                fprintf( hfLog, "Other times        %I64d [ms]\n", lOtherTime );
              }
              #endif

         UtlAlloc( (PVOID *) &(pLoadData->pTokenList),   0L, 0L, NOMSG );
         if ( pLoadData->pProposal != NULL ) free(pLoadData->pProposal);
         UtlAlloc( (PVOID *) &pLoadData,                 0L, 0L, NOMSG );
       } /* endif */
       usPhase = 0;;
       pData->usProgress = 100;
       break;
 } /* endswitch */

 pData->usMemLoadPhase = usPhase;

 if ( usPhase == 0 )
 {
   pData->fComplete = TRUE;
 } /* endif */

 return( usRC );

} /* end of function MemFuncImportProcess */


// load some data from file and convert it if necessary
// usSize is size of pszBuffer(filled as output!) in number of CHAR_W's

USHORT MemLoadAndConvert( PMEM_LOAD_IDA pLIDA, PSZ_W pszBuffer, ULONG ulSize, PULONG pulBytesRead )
{
  USHORT usDosRc = NO_ERROR;
  ULONG  ulBytesRead = 0;

#ifdef IMPORT_LOGGING
    GetElapsedTime( &lOtherTime );
#endif

  switch ( pLIDA->usImpMode )
  {
    case SGMLFORMAT_ANSI :
      {
        PSZ pConvBuffer = NULL;
        LONG   lBytesLeft = 0L;
		    LONG   lRc = 0;
		    ULONG  ulUnicodeChars = 0;

        if ( UtlAlloc( (PVOID *)&pConvBuffer, 0L, ulSize+1, ERROR_STORAGE ) )
        {
          usDosRc = UtlReadHwnd( pLIDA->hFile, pConvBuffer, ulSize, &ulBytesRead,
                                 TRUE, pLIDA->hwndErrMsg );
          if (ulBytesRead != ulSize )
          {
            pLIDA->fEOF = TRUE;
          } /* endif */

          // ensure that there are no DBCS characters splitted at the end of
          // the data just read into memory
          if ( ulBytesRead && !pLIDA->fEOF )
          {
			      // non DBCS case --- no problem at all
			      ulUnicodeChars = UtlDirectAnsi2UnicodeBuf( pConvBuffer, pszBuffer,
													      ulBytesRead, pLIDA->ulAnsiCP, FALSE, &lRc,
													      &lBytesLeft );
			      usDosRc = (USHORT)lRc;
			      if ( lBytesLeft )
			      {    // undo the last character read...
				        // reposition file pointer
				      ULONG ulCurrentPos = 0;
				      UtlChgFilePtr( pLIDA->hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
				      ulCurrentPos -= lBytesLeft;
				      UtlChgFilePtr( pLIDA->hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);
            } /* endif */
            // GQ: use number of Unicode characters (which may differ from number of chars read in) for processing
            ulBytesRead = ulUnicodeChars;
          }
          else
          {
              // either at end or nothing read in...
              ulUnicodeChars = UtlDirectAnsi2UnicodeBuf( pConvBuffer, pszBuffer,
			  										ulBytesRead, pLIDA->ulAnsiCP, FALSE, &lRc,
														 &lBytesLeft );
              ulBytesRead = ulUnicodeChars;
			        usDosRc = (USHORT)lRc;
          } /* endif */
          ulBytesRead *= sizeof(CHAR_W);

          if ( lRc )
          {
            // find character causing the conversion error and show error message
            UtlFindAndShowConversionError( pConvBuffer, ulBytesRead, pLIDA->ulAnsiCP );
          } /* endif */

          UtlAlloc( (PVOID *)&pConvBuffer, 0L, 0L, NOMSG );
        }
        else
        {
          usDosRc = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
      }
      break;

    case SGMLFORMAT_UNICODE :
      {
        // read data into conversion buffer
        usDosRc = UtlReadHwnd( pLIDA->hFile, pszBuffer, (ulSize * sizeof(CHAR_W)),
                               &ulBytesRead, TRUE, pLIDA->hwndErrMsg );

        // set EOF condition
        if ( (usDosRc == NO_ERROR ) && (ulBytesRead != ulSize * sizeof(CHAR_W) ) )
        {
          pLIDA->fEOF = TRUE;
        } /* endif */

        // if first read skip any unicode text prefix
        if ( (usDosRc == NO_ERROR) && (pLIDA->fFirstRead == FALSE) )
        {
          PSZ pszPrefix = UNICODEFILEPREFIX;
          int iLen = strlen(pszPrefix);
          if ( memcmp( pszBuffer, pszPrefix, iLen ) == 0 )
          {
            // skip prefix ...
            ulBytesRead -= iLen;
            memmove( (PBYTE)pszBuffer, (PBYTE)pszBuffer+iLen, ulBytesRead );
          }
          else
          {
             PSZ pszImportFile = pLIDA->szFilePath;
             usDosRc = UtlError( NO_VALID_UNICODEFORMAT, MB_YESNO, 1, &pszImportFile, EQF_WARNING );

             //usDosRc = (usDosRc == MBID_YES ) ? NO_ERROR : ERROR_INVALID_DATA;  
          } /* endif */
        } /* endif */

      }
      break;

    case SGMLFORMAT_ASCII :
    default:
      {
        PSZ pConvBuffer = NULL;
        if ( UtlAlloc( (PVOID *)&pConvBuffer, 0L, ulSize+3, ERROR_STORAGE ) )
        {
          usDosRc = UtlReadHwnd( pLIDA->hFile, pConvBuffer, (ulSize), &ulBytesRead,
                                 TRUE, pLIDA->hwndErrMsg );
          if (ulBytesRead != ulSize )
          {
            pLIDA->fEOF = TRUE;
          } /* endif */

          // ensure that there are no DBCS characters splitted at the end of
          // the data just read into memory
          if ( ulBytesRead && !pLIDA->fEOF )
          {
             BYTE bTest = (BYTE)pConvBuffer[ulBytesRead-1];
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
             if ( IsDBCSLeadByteEx( pLIDA->ulOemCP, bTest ) )
             {
                 int iTry = 5;
                 ULONG ulUnicodeChars = 0;

                 while (  iTry > 0  && ulBytesRead)
                 {
                     // undo the last character read...
                     // reposition file pointer
                    ULONG ulCurrentPos = 0;
                    UtlChgFilePtr( pLIDA->hFile, 0L, FILE_CURRENT,  &ulCurrentPos, FALSE);
                    ulCurrentPos--;
                    UtlChgFilePtr( pLIDA->hFile, ulCurrentPos, FILE_BEGIN,  &ulCurrentPos, FALSE);

                    // adjust counters
                    ulBytesRead--;
                    // return is number of wide chars written to pszBuffer
                    ulUnicodeChars = ASCII2UnicodeBuf( pConvBuffer, pszBuffer, ulBytesRead, pLIDA->ulOemCP  );
                    if ( ulUnicodeChars && pszBuffer[ulUnicodeChars-1]   == 0)
                    {
                        // try again, we probably found a 2nd DBCSbyte which might be in the range of a 1st byte, too
                        iTry--;
                    }
                    else
                    {
                        // leave loop and set ulBytesRead
                        ulBytesRead = ulUnicodeChars;
                        iTry = 0;
                    } /* endif */
                } /* endwhile */

                if ( iTry == 0 && ulUnicodeChars && pszBuffer[ulUnicodeChars-1]   == 0)
                {
                    // something went totally wrong
                    ulBytesRead = ulUnicodeChars-1;
                }
             }
             else
             {
               LONG  lBytesLeft = 0;
               ULONG ulOutPut = 0;
               LONG lRC = 0;

               // non DBCS case --- no problem at all
 	             ulOutPut = ASCII2UnicodeBufEx( pConvBuffer, pszBuffer, ulBytesRead, pLIDA->ulOemCP,
                                              FALSE, &lRC, &lBytesLeft );
	            // For consistent behaviour with old ASCII2UnicodeBuf function where dwFlags was 0:
	            // Within TM : FileRead expects to get EOS at position ulOutPut
              if ( lRC )
              {
                // find character causing the conversion error and show error message
                UtlFindAndShowConversionError( pConvBuffer, ulBytesRead, pLIDA->ulOemCP );
              } /* endif */

              if (lBytesLeft && (ulOutPut < ulBytesRead))
              {
                *(pszBuffer+ulOutPut) = EOS;
                ulOutPut++;
              }
              ulBytesRead = ulOutPut;
            } /* endif */
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
          }
          else
          {
            // either at end or nothing read in...
            ulBytesRead = ASCII2UnicodeBuf( pConvBuffer, pszBuffer, ulBytesRead, pLIDA->ulOemCP  );
          } /* endif */
          // return is number of wide chars written to pszBuffer
          ulBytesRead *= sizeof(CHAR_W);

          UtlAlloc( (PVOID *)&pConvBuffer, 0L, 0L, NOMSG );
        }
        else
        {
          usDosRc = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
      }
      break;
  } /* endswitch */

  // adjust read bytes and add EOS
  if ( usDosRc == NO_ERROR )
  {
    PBYTE pByte = (PBYTE) pszBuffer;
    *((PSZ_W)(pByte+ulBytesRead))  = EOS;


     *pulBytesRead = ulBytesRead;
  } /* endif */

#ifdef IMPORT_LOGGING
    GetElapsedTime( &lFileReadTime );
#endif


  return( usDosRc );
} /* end of function MemLoadAndConvert */

USHORT MemHandleCodePageValue
(
  PMEM_LOAD_IDA pLIDA,                           // ptr to memory load IDA
  PSZ_W       pszCodePage,                       // ptr to specified code page value 
  PBOOL       pfRestartImport                    // caller's restart the import flag 
)
{
  ULONG       ulCP = 0;
  PSZ_W       pMsgError[2];                // ptr to error array
  USHORT      usRc = 0;
  USHORT      usImpMode = 0;

  *pfRestartImport = FALSE;

  // skip leading whitespace 
  while ( iswspace( *pszCodePage ) ) pszCodePage++;

	if ( !usRc )
	{
	   if ( UTF16strnicmp( pszCodePage, L"ANSI=", 5 ) == 0)
	   {
  		  usImpMode = SGMLFORMAT_ANSI;
		    ulCP = wcstol(pszCodePage + 5, 0, 10);

        if ( usImpMode != pLIDA->usImpMode )
		    {
			    if ( pLIDA->usImpMode == SGMLFORMAT_UNICODE )
		      {
				    usRc = NO_VALID_ASCIIANSIFORMAT;
		        pMsgError[0] = L"SGML ANSI";
            LogMessage3(ERROR, __func__,"::NO_VALID_ASCIIANSIFORMAT::", EncodingHelper::convertToUTF8(pMsgError[0]).c_str());
		      }
		      else // usImpMode == SGMLFORMAT_ASCII
		      { 
            if ( (pLIDA->hwndErrMsg == HWND_FUNCIF) || pLIDA->fBatch  )
			      {
			        usRc = MBID_OK;
			      }
			      else
            {
		          pMsgError[1] = L"SGML ASCII";
				      pMsgError[0] = pszCodePage;
              LogMessage3(ERROR, __func__, ":: WARNING_DIMP_FORMAT_SGML ASCII::",    EncodingHelper::convertToUTF8(pMsgError[0]).c_str() );
              usRc = WARNING_DIMP_FORMAT;
			      } /* endif */

			      if ( usRc == MBID_OK )
			      {
				      *pfRestartImport = TRUE;
				      usRc = NO_ERROR;
			      } /* endif */
  		    } /* endif */
		    } /* endif */

		    if ( !usRc && (ulCP != pLIDA->ulAnsiCP) )
		    {
          if ( !UtlIsValidCP( ulCP ) )
          {
			       usRc = NO_VALID_ASCIIANSICP;
             pMsgError[0] = pszCodePage + 5;
             LogMessage5(ERROR, __func__,"::rc = ", intToA(usRc), ":: ", EncodingHelper::convertToUTF8(pMsgError[0]).c_str());
		      } /* endif */
		    }

		    if ( !usRc )
		    {
			    pLIDA->ulAnsiCP = ADJUSTCP(ulCP);
			    pLIDA->usImpMode = usImpMode;
		    } /* endif */
	   }
	   else if ( UTF16strnicmp( pszCodePage, L"ASCII=", 6 ) == 0)
	   {
  		 usImpMode = SGMLFORMAT_ASCII;
		   ulCP = wcstol(pszCodePage + 6, 0, 10);

 		   if( usImpMode != pLIDA->usImpMode )
		   {
		     if ( pLIDA->usImpMode == SGMLFORMAT_UNICODE )
		     {
				  usRc = NO_VALID_ASCIIANSIFORMAT;
				  pMsgError[0] = L"SGML ASCII";
          LogMessage2(ERROR, __func__,"::NO_VALID_ASCIIANSIFORMAT::SGML ASCII");
		    }
		    else
		    {
          if ( ( pLIDA->hwndErrMsg == HWND_FUNCIF) || pLIDA->fBatch )
			    {
  				  usRc = MBID_OK;
			    }
			    else
			    {
		        pMsgError[1] = L"SGML ANSI";
				    pMsgError[0] = pszCodePage;
            usRc = LogMessage2(ERROR, __func__, ":: WARNING_DIMP_FORMAT::SGML ANSI");
			    } /* endif */

			    if (usRc == MBID_OK)
			    {
  		      *pfRestartImport = TRUE;
			      usRc = NO_ERROR;
			    } /* endif */
		    } /* endif */
	     } /* endif */

       if ( !usRc && (ulCP != pLIDA->ulOemCP) )
		   {
         if ( !UtlIsValidCP( ulCP ) )
         {
			     usRc = NO_VALID_ASCIIANSICP;
           pMsgError[0] = pszCodePage + 6;
           LogMessage5(ERROR, __func__, ":: ",intToA(usRc), "::", EncodingHelper::convertToUTF8(pMsgError[0]).c_str());
		     } /* endif */
  		 } /* endif */

		   if ( !usRc )
		   {
			   pLIDA->ulOemCP = ADJUSTCP(ulCP);
			   pLIDA->usImpMode = usImpMode;
		   } /* endif */
	   }
	   else if ( UTF16strnicmp( pszCodePage, L"UTF16", 5 ) == 0)
	   {
  		 usImpMode = SGMLFORMAT_UNICODE;

		   if ( usImpMode != pLIDA->usImpMode )
		   {
		     usRc = ERROR_DIMP_UTF16WRONG;
		     LogMessage3(ERROR, __func__, "::rc = ERROR_DIMP_UTF16WRONG = ", intToA(usRc) );
		   } /* endif */
	   }
	   else
	   {  // error - this should not occur...
		   usRc = ERROR_MEM_INVALID_SGML;
		   LogMessage2(ERROR, __func__, "::rc =  ERROR_MEM_INVALID_SGML" );
	   } /* endif */
  } /* endif */

  return(usRc);
} /* end of function MemHandleCodePageValue */

//
// MEMINSERTSEGMENT  
//
// call back function used by external memory import code to insert a segment into the memory
// 
USHORT /*APIENTRY*/ MEMINSERTSEGMENT
( 
  LONG lMemHandle, 
  PMEMEXPIMPSEG pSegment 
)
{
  USHORT usRC = 0;

  PMEM_LOAD_IDA pLIDA = (PMEM_LOAD_IDA)lMemHandle;

  pLIDA->ulSequenceNumber++;           // Increase segment sequence number in any case

  // prepare segment data
  if ( pSegment->fValid )
  {
    pLIDA->pProposal->clear();
    switch( pSegment->usTranslationFlag )
    {
      case TRANSL_SOURCE_MANUAL: pLIDA->pProposal->setType( OtmProposal::eptManual ); break;
      case TRANSL_SOURCE_MANCHINE: pLIDA->pProposal->setType( OtmProposal::eptMachine ); break;
      case TRANSL_SOURCE_GLOBMEMORY: pLIDA->pProposal->setType( OtmProposal::eptGlobalMemory ); break;
      default: pLIDA->pProposal->setType( OtmProposal::eptUndefined ); break;
    } /* endswitch */
    pLIDA->pProposal->setUpdateTime( pSegment->lTime );
    pLIDA->pProposal->setAuthor( pSegment->szAuthor );
    char szShortName[MAX_FILESPEC];
    UtlLongToShortName( pSegment->szDocument, szShortName );
    pLIDA->pProposal->setDocShortName( szShortName );
    pLIDA->pProposal->setDocName( pSegment->szDocument );

    pLIDA->pProposal->setSource( pSegment->szSource );
    pLIDA->pProposal->setSourceLanguage( pSegment->szSourceLang );
    pLIDA->pProposal->setMarkup( pSegment->szFormat );
    pLIDA->pProposal->setTarget( pSegment->szTarget );
    pLIDA->pProposal->setTargetLanguage( pSegment->szTargetLang );
    pLIDA->pProposal->setAddInfo( pSegment->szAddInfo );
    pLIDA->pProposal->setSegmentNum( pSegment->lSegNum );

    // insert/replace segment in(to) memory
    usRC = (USHORT)pLIDA->pMem->putProposal( *(pLIDA->pProposal) );

    if ( !usRC )
    {
      pLIDA->ulSegmentCounter++;         // Increase the segment counter
    }
    else
    {
      pLIDA->ulInvSegmentCounter++;      // increase invalid segment counter 
    } /* endif */
  }
  else
  {
    pLIDA->ulInvSegmentCounter++;      // increase invalid segment counter 

    // write segment to log file
    if ( pLIDA->hSegLog != NULL )
    {
      fwprintf( pLIDA->hSegLog, L"Segment %ld not imported\r\n",  pSegment->lSegNum );
      fwprintf( pLIDA->hSegLog, L"Reason         = %S\r\n",  pSegment->szReason );
      fwprintf( pLIDA->hSegLog, L"Document       = %S\r\n",  pSegment->szDocument );
      fwprintf( pLIDA->hSegLog, L"SourceLanguage = %S\r\n",  pSegment->szSourceLang );
      fwprintf( pLIDA->hSegLog, L"TargetLanguage = %S\r\n",  pSegment->szTargetLang );
      fwprintf( pLIDA->hSegLog, L"Markup         = %S\r\n",  pSegment->szFormat );
      fwprintf( pLIDA->hSegLog, L"Source         = \r\n");
      fwprintf( pLIDA->hSegLog, L"%s\r\n", pSegment->szSource );
      fwprintf( pLIDA->hSegLog, L"Target         = \r\n");
      fwprintf( pLIDA->hSegLog, L"%s\r\n\r\n", pSegment->szTarget );
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function MEMINSERTSEGMENT */

// Check if a specific tag is following
BOOL MemLPCheckForTag( PTOKENENTRY pTok, SHORT sID )
{
  int i = 1;
  while ( (i <= 3) && (pTok->sTokenid != sID) && (pTok->sTokenid != ENDOFLIST) )
  {
    pTok++; i++;
  } /* endwhile */
  return( pTok->sTokenid == sID );
}

// extract value between start and end tag
BOOL MemLPExtractValue
( 
  PTOKENENTRY *ppTok, 
  SHORT sStartID, 
  SHORT sEndID, 
  PSZ_W pszBuffer, 
  int iBufSize,                         // size of buffer in number of CHAR_W characters
  PBOOL pfSegmentTooLarge               // set to TRUE if segment data is too large
)
{
  BOOL fOK = TRUE;
  PTOKENENTRY pTok = *ppTok; 

  // Check next 3 tokens for start token, if no one found stop further processing
  if ( fOK )
  {
     int i = 1;
     while ( (i <= 3) && (pTok->sTokenid != sStartID) && (pTok->sTokenid != ENDOFLIST) )
     {
        pTok++;
        i++;
     } /* endwhile */

     fOK = pTok->sTokenid == sStartID;
  } /* endif */

  // get value
  if (fOK)
  {
    pTok++;
    memset( pszBuffer, NULC, iBufSize );
    while ( fOK && (pTok->sTokenid != sEndID) && (pTok->sTokenid != ENDOFLIST) )
    {
      int iCurLen = UTF16strlenCHAR( pszBuffer );
      if ( (iCurLen + pTok->usLength) < iBufSize )
      {
        memcpy( pszBuffer + iCurLen, pTok->pDataStringW, pTok->usLength * sizeof(CHAR_W) );
        pszBuffer[iCurLen + pTok->usLength] = EOS;
        pTok++;
      }
      else
      {
        fOK = FALSE;
        *pfSegmentTooLarge = TRUE;
      } /* endif */
    } /* endwhile */

    // In order to keep CRLF as-is deactivate the following line
    MemReduceCRLF( pszBuffer );
  } /* endif */

  // Check for end token
  if (fOK)
  {
    fOK = pTok->sTokenid == sEndID;
    if ( fOK ) pTok++;
  } /* endif */

  *ppTok = pTok; 

  return( fOK );
}
