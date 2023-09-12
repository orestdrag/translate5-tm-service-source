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
#include "../utilities/EncodingHelper.h"

#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include <tm.h>               // Private header file of Translation Memory
//#include "EQFMEM.ID" // Translation Memory IDs
#include <EQFTMTAG.H>             // Translation Memory SGML tags
#include <time.h>                 // C library for time functions
#include "EQFHLOG.H"              // for word count category limits
#include "otm.h" 
#include "EQFFUNCI.H"             // private defines for function call interface
#include "Property.h"

#define MEM_START_IMPORT    USER_TASK + 1
#define MEM_IMPORT_TASK     USER_TASK + 2
#define MEM_END_IMPORT      USER_TASK + 3


//--- declaration of internal functions
static USHORT  MemLoadStart( PVOID *ppIda, HWND, ImportStatusDetails*     pImportData);
static USHORT  MemLoadProcess(  PMEM_LOAD_IDA pLIDA );


USHORT /*APIENTRY*/ MEMINSERTSEGMENT
( 
  LONG lMemHandle, 
  PMEMEXPIMPSEG pSegment 
);


static void GetElapsedTime( long long *plTime )
{
  if(plTime) *plTime = 0;
  //struct timespec now;
  //clock_gettime(CLOCK_MONOTONIC, &now);
  //*plTime =  now.tv_sec + now.tv_nsec / 1000000000.0;
}

USHORT MemHandleCodePageValue
(
  PMEM_LOAD_IDA pLIDA,                           // ptr to memory load IDA
  PSZ_W       pszCodePage,                       // ptr to specified code page value 
  PBOOL       pfRestartImport                    // caller's restart the import flag 
);


// import logging 
  //static LONG lImportStartTime = 0;
  static LONG64 lMemAccessTime = 0;
  static LONG64 lFileReadTime = 0;
  static LONG64 lOtherTime = 0;

//#endif

USHORT MemLoadAndConvert( PMEM_LOAD_IDA pLIDA, PSZ_W pszBuffer, ULONG usSize, PULONG pusBytesRead );

INT_PTR /*CALLBACK*/ NTMTagLangDlg( HWND,         // Dialog window handle
               WINMSG,       // Message ID
               WPARAM,       // Message parameter 1
               LPARAM );     // Message parameter 2


#include "lowlevelotmdatastructs.h"

typedef struct _MEM_LOAD_DLG_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// TM name to import to
 CHAR         szShortMemName[MAX_FILESPEC];// TM name to import to
 CHAR         szMemPath[2048];            // TM path + name to import to
 CHAR         szFilePath[2048];           // File to be imported path + name
 HFILE        hFile;                      // Handle of file to be imported
 PMEM_IDA     pIDA;                       // pointer to the memory database IDA
 BOOL         fAscii;                     // TRUE = ASCII else internal format
 CHAR         szName[MAX_FNAME];          // file to be imp. name without ext
 CHAR         szExt[MAX_FEXT];            // file to be imp. ext. with leading dot
 CHAR         szString[2048];             // string buffer
 CHAR         szDummy[_MAX_DIR];          // string buffer
 BOOL         fDisabled;                  // Dialog disable flag
 CONTROLSIDA  ControlsIda;                //ida of controls utility
 HPROP        hPropLast;                  // Last used properties handle
 PMEM_LAST_USED pPropLast;                // Pointer to the last used properties
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMIMP   pDDEMemImp;                 // ptr to batch memory import data
 HWND         hwndNotify;                 // send notification about completion
 OBJNAME      szObjName;                  // name of object to be finished
 USHORT       usImpMode;                  // mode for import
 BOOL         fMerge;                     // TRUE = TM is being merged
 PSZ          pszList;                    // list of selected import files or NULL
 BOOL         fSkipInvalidSegments;       // TRUE = skip invalid segments during import
 ULONG        ulOemCP;                    // CP of system preferences lang.
 ULONG        ulAnsiCP;                   // CP of system preferences lang.
 BOOL         fIgnoreEqualSegments;       // TRUE = ignore segments with identical source and target string
 BOOL         fAdjustTrailingWhitespace;  // TRUE = adjust trailing whitespace of target string
 BOOL         fMTReceiveCounting;         // TRUE = count received words and write counts to folder properties
 OBJNAME      szFolObjName;               // object name of folder (only set with fMTReceiveCounting = TRUE)
 HWND         hwndToCombo;                // handle of "to memory" combo box
 BOOL         fYesToAll;                  // yes-to-all flag for merge confirmation
 BOOL         fImpModeSet;                // TRUE = imp mode has been set by the impot file check logic
 std::shared_ptr<EqfMemory>    mem;                      // pointer to memory being imported
 OtmProposal  *pProposal;                 // buffer for proposal data
 BOOL          fForceNewMatchID;          // create a new match segment ID even when the match already has one
 BOOL          fCreateMatchID;            // create match segment IDs
 CHAR          szMatchIDPrefix[256];      // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];     // prefix to be used for the match segment ID (UTF16 version)
}MEM_LOAD_DLG_IDA, * PMEM_LOAD_DLG_IDA;


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
                             HWND             hWnd,
                             ImportStatusDetails*     pImportData )
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
   if(pDialogIDA->mem == nullptr){
    fOK = false;
   }
   
   if ( fOK )
   {
      // Move values from dialog IDA to load IDA
      pLIDA->hFile = pDialogIDA->hFile;
      pLIDA->mem  = pDialogIDA->mem;
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

      pLIDA->ulAnsiCP = pLIDA->ulOemCP = 1;

      // read the file size of the file to be imported and store it in IDA
      usDosRc = UtlGetFileSize( pLIDA->hFile, &(pLIDA->ulTotalSize), FALSE );
      if (usDosRc)
      {
        fOK = FALSE;
      }
      else
      {
        // Create full path to the memory database format table
        Properties::GetInstance()->get_value( KEY_OTM_DIR, pLIDA->szFullFtabPath, MAX_EQF_PATH );
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
     PWCHAR pRest = NULL;
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
       pLIDA->mem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );
       if ( pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS ) 
       {
         pLIDA->pstMemInfo->fCleanRTF = TRUE;
         pLIDA->pstMemInfo->fTagsInCurlyBracesOnly = TRUE;
       }
       
       fOK = (EqfPluginWrapper::MemImportStart( &(pLIDA->lExternalImportHandle) , pLIDA->szFilePath, pLIDA->pstMemInfo,  pImportData ) == 0);
       pLIDA->mem->getSourceLanguage( szMemSourceLang, sizeof(szMemSourceLang) );

       // check if memory source lanuage matchs source language of imported file
       if ( pLIDA->pstMemInfo->szSourceLang[0] && (strcmp( pLIDA->pstMemInfo->szSourceLang, szMemSourceLang ) != 0) )
       {
         pReplAddr[0] = pLIDA->pstMemInfo->szSourceLang;
         pReplAddr[1] = szMemSourceLang;
         T5LOG(T5INFO)  <<  "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::" << pReplAddr[0] <<  "; szMemSourceLang = " <<  pReplAddr[1];

          //fOK = FALSE;
       } /* endif */

       // update memory description with description of imported memory
       if ( fOK )
       {
         pLIDA->mem->setDescription( pLIDA->pstMemInfo->szDescription );
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
       pLIDA->mem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );

       //fOK = (pLIDA->pfnMemImpStart( &(pLIDA->lExternalImportHandle), pLIDA->szFilePath, pLIDA->pstMemInfo ) == 0);
       fOK = (EqfPluginWrapper::MemImportStart(  &(pLIDA->lExternalImportHandle) , pLIDA->szFilePath, pLIDA->pstMemInfo, pImportData ) == 0);
       
       pLIDA->mem->getSourceLanguage( szMemSourceLang, sizeof(szMemSourceLang) );

       // check if memory source lanuage matchs source language of imported file
       if ( pLIDA->pstMemInfo->szSourceLang[0] && (strcmp( pLIDA->pstMemInfo->szSourceLang, szMemSourceLang ) != 0) )
       {
         USHORT usMBCode;

         pReplAddr[0] = pLIDA->pstMemInfo->szSourceLang;
         pReplAddr[1] = szMemSourceLang;
         //usMBCode = T5LOG(T5ERROR) <<   "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::", pReplAddr[0] );
         T5LOG(T5INFO)  <<  "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::", pReplAddr[0] ;
         //if ( usMBCode == MBID_CANCEL )
         //{
         //  fOK = FALSE;
         //} /* endif */
       } /* endif */

       // update memory description with description of imported memory
       if ( fOK )
       {
         pLIDA->mem->setDescription( pLIDA->pstMemInfo->szDescription );
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
         T5LOG(T5ERROR) <<  "::ERROR_MEM_LOAD_INITFAILED::" << pReplAddr[0] ;

         if ( pLIDA->pProposal != NULL ) delete(pLIDA->pProposal);

         UtlAlloc( (PVOID *) &pLIDA, 0L, 0L, NOMSG );
         *ppIda = NULL;
 
         // Dismiss the slider window if it had been created
//WINAPI
         //EqfRemoveObject( TWBFORCE, hWnd );
      } /* endif */
   }else{
    time( &pLIDA->mem->importDetails->lReorganizeStartTime );
    T5LOG(T5DEBUG)  << "************ Memory Import Log *********************\n Memory import started at   : "<<
          asctime( localtime( &pLIDA->mem->importDetails->lReorganizeStartTime ) )<< "\nMemory name                : "  << pLIDA->szMemName ;
   } /* endif */   

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
static USHORT MemLoadProcess( PMEM_LOAD_IDA  pLIDA, ImportStatusDetails*     pImportData  ) //pointer to the load IDA
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
     
     usRc = EqfPluginWrapper::MemImportProcess( pLIDA->lExternalImportHandle , MEMINSERTSEGMENT, (LONG)pLIDA, pImportData);

     // handle any error conditions
     if ( pLIDA->pstMemInfo->fError )  
     {
       PSZ pszMsg = pLIDA->pstMemInfo->szError;
       T5LOG(T5ERROR) <<  "::ERROR_MEMIMP_ERROR::MEM_SEG_SYN_ERR::" <<  pszMsg ;
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
   else{
    LOG_UNIMPLEMENTED_FUNCTION;
   }
   return usRc;
} /* end of function MemLoadProcess  */


USHORT MemFuncImportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszInFile,               // fully qualified name of input file
  std::shared_ptr<EqfMemory> mem,             
  LONG        lOptions                 // options for Translation Memory import
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // when running in COMPLETE_IN_ONE_CALL_OPT mode we use our own data area to be independent of other API calls running in parallel
  FCTDATA PrivateData;
  memset( &PrivateData, 0, sizeof( FCTDATA ) );
  PrivateData.fComplete = TRUE;
  PrivateData.mem = mem;
  usRC = MemFuncPrepImport( &PrivateData, pszInFile, lOptions );
  if ( usRC == 0 )
  {
    while ( !PrivateData.fComplete )
    {
      usRC = MemFuncImportProcess( &PrivateData );
    }
    usRC = PrivateData.usMemLoadRC;
  }
  
  if(usRC){
    strcpy(pData->szError, PrivateData.szError);
  }
  return( usRC );
} /* end of function MemFuncImportMem */

// prepare the function I/F TM import
USHORT MemFuncPrepImport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszInFile,               // fully qualified name of input file
  LONG        lOptions                 // options for Translation Memory import
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // error parameter pointer
  PMEM_LOAD_DLG_IDA pLoadIDA = NULL;   // Pointer to the load IDA

  bool fOkFailHandled = false;
   /*******************************************************************/
   /* Allocate storage for the MEM_LOAD_DLG_IDA. The area will be free*/
   /* in the function  EQFMemloadStart. Only in case of an error      */
   /* it will be freed here.                                          */
   /*******************************************************************/
   fOK = UtlAllocHwnd( (PVOID *)&pLoadIDA, 0L, (LONG)sizeof(MEM_LOAD_DLG_IDA), ERROR_STORAGE, HWND_FUNCIF );
    if ( fOK )
    {  // use def.target lang. from system preferences of TM
          pLoadIDA->ulOemCP = 1;
          pLoadIDA->ulAnsiCP = 1;
    }else if(!fOkFailHandled){
      fOkFailHandled = true;
      T5LOG(T5ERROR) << "MemFuncPrepImport:: memory allocation fails";
    }

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
   if ( fOK ){
     pLoadIDA->szMatchIDPrefix[0] = '\0';
   }else if(!fOkFailHandled){
     fOkFailHandled = true;
     T5LOG(T5ERROR) << "MemFuncPrepImport::set options for import fails";
   } /* endif */

   /*******************************************************************/
   /* Check if there is a TM with the given name                      */
   /*******************************************************************/
   if ( fOK )
   {
     strcpy( pLoadIDA->szMemName, pData->mem.get()->szName );
     //TMManager *pFactory = TMManager::GetInstance();
  //   if ( !pFactory->exists( NULL, pszMemName ) )
  //   {
  //     T5LOG(T5ERROR) << "Mem file not found:: " << pszMemName;
	//     fOK = FALSE;
  //     pszParm = pszMemName;
  //     usRC = ERROR_MEMORY_NOTFOUND;
  //     T5LOG(T5ERROR) <<  "::ERROR_MEMORY_NOTFOUND::" << pszParm;
  //   } /* endif */
  }else if(!fOkFailHandled){
     fOkFailHandled = true;
     T5LOG(T5ERROR) << "MemFuncPrepImport::prepare segment match ID prefix fails";
  } /* endif */

   /*******************************************************************/
   /* Open the TM                                                     */
   /*******************************************************************/
   //
   //if ( fOK )
   //{
   //  int iRC = 0;
   //  TMManager *pFactory = TMManager::GetInstance();
   //  pLoadIDA->pMem = pFactory->openMemory( NULL, pszMemName, EXCLUSIVE, &iRC );
   //  if ( iRC != 0 || pLoadIDA->pMem == nullptr )
   //  {
	 //    fOK = FALSE;
   //    pFactory->showLastError( NULL, pszMemName, pLoadIDA->pMem, (HWND)HWND_FUNCIF );
   //  } /* end */        
  //}else if(!fOkFailHandled){
  //    fOkFailHandled = true;
  //    T5LOG(T5ERROR) << "MemFuncPrepImport::Check if there is a TM with the given name fails";
  //} /* endif */

   /*******************************************************************/
   /* Check if input file exists                                      */
   /*******************************************************************/
   if ( fOK )
   {
     if ( !UtlFileExist( pszInFile ) )
     {
       pszParm = pszInFile;
       T5LOG(T5ERROR) <<  "::FILE_NOT_EXISTS::" << pszParm;
       fOK = FALSE;
     } /* endif */
  }else if(!fOkFailHandled){
     fOkFailHandled = true;
     T5LOG(T5ERROR) << "MemFuncPrepImport::Open the TM fails";
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
      T5LOG(T5ERROR) << "MemFuncPrepImport::Check if input file exists fails";
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
      T5LOG(T5ERROR) << "MemFuncPrepImport::Open the input file  fails" ;
   } /* endif */

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
      T5LOG(T5ERROR) << "MemFuncPrepImport::fOk in false, making Cleanup";
      if ( pLoadIDA )
      {
         //if ( pLoadIDA->pMem )
         //{
         //  TMManager *pFactory = TMManager::GetInstance();
         //  pFactory->closeMemory( pLoadIDA->pMem );
         //  pLoadIDA->pMem = NULL;
         //} /* endif */
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
     pLoadIDA->mem = pData->mem;
   }
   else if(!fOkFailHandled)
   {
     fOkFailHandled = true;
     T5LOG(T5ERROR) << "MemFuncPrepImport::fOk in false, making Cleanup";
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
       T5LOG( T5INFO) << "::MEM_START_IMPORT";
       pData->mem->importDetails->usProgress = 0;
       pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;

       if ( !MemLoadStart( &(pData->pvMemLoadIda), HWND_FUNCIF, pData->mem->importDetails ) )
       {
         //TMManager *pFactory = TMManager::GetInstance();
         //pFactory->closeMemory( pDialogIDA->mem );
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
        
        T5LOG( T5INFO) << "::MEM_IMPORT_TASK, progress = " << pData->mem->importDetails->usProgress;
        USHORT usRc = MemLoadProcess( pLoadData, pData->mem->importDetails );
        switch ( usRc )
        {
          case MEM_PROCESS_OK:            
            usPhase = MEM_IMPORT_TASK;
            break;
          case MEM_PROCESS_END:
             //pData->pImportData->usProgress = 100;
            pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
            if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
            if ( pLoadData->mem )
            {
              // organize index file of imported memory
              pLoadData->mem->NTMOrganizeIndexFile();

              //TMManager *pFactory = TMManager::GetInstance();
              //pFactory->closeMemory( pLoadData->mem);
              //pLoadData->mem = NULL;
            } /* endif */
            usPhase = MEM_END_IMPORT;
            break;
          case MEM_DB_CANCEL :
            usPhase = MEM_END_IMPORT;
            break;
          default:
            pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
            if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
            pData->usMemLoadRC = usRC = usRc? usRc : UtlQueryUShort( QS_LASTERRORMSGID );
            strncpy(pData->szError, pLoadData->pstMemInfo->szError, sizeof(pData->szError));
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
         } /* endif */

         if ( pLoadData->mem )
         {
           //TMManager *pFactory = TMManager::GetInstance();
           //pFactory->closeMemory( pLoadData->mem);
           //pLoadData->mem = NULLHANDLE;
         } /* endif */
         pDialogIDA = (PMEM_LOAD_DLG_IDA)pData->pvMemLoadIda;
         if ( pDialogIDA->hFile ) CloseFile( &(pLoadData->hFile));
         if ( pLoadData->pFormatTable )
         {
           TAFreeTagTable( pLoadData->pFormatTable );
           pLoadData->pFormatTable = NULL;
         } /* endif */
              
        {
          LONG lCurTime = 0;  
          time( &lCurTime );
                  
          if ( pData->mem->importDetails->lReorganizeStartTime )
          {
            LONG lDiff = lCurTime - pData->mem->importDetails->lReorganizeStartTime;
            char buff[256];
            sprintf( buff, "Overall import time is      : %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
                    (lDiff - (lDiff / 3600 * 3600)) / 60,
                    (lDiff - (lDiff / 3600 * 3600)) % 60 );
                    
            pData->mem->importDetails->importTimestamp = buff;
          }
          
          if(VLOG_IS_ON(1)){
            std::string logMsg = "::Memory import ended at     : " + std::string(asctime( localtime( &lCurTime ))) +
                                  "\tNumber of segments imported : " + toStr(pData->mem->importDetails->segmentsImported)+
                                "\n\tNumber of invalid segments  : " + toStr(pData->mem->importDetails->invalidSegments)+ 
                                "\n\tNumber of OTMUTF8 segments  : " + toStr(pData->mem->importDetails->resSegments) + "\n\t" + pData->mem->importDetails->importTimestamp;
            T5LOG( T5TRANSACTION) << logMsg;
          }
        }

         UtlAlloc( (PVOID *) &(pLoadData->pTokenList),   0L, 0L, NOMSG );
         if ( pLoadData->pProposal != NULL ) delete(pLoadData->pProposal);
         UtlAlloc( (PVOID *) &pLoadData,                 0L, 0L, NOMSG );
       } /* endif */
       usPhase = 0;;
       pData->mem->importDetails->usProgress = 100;
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
T5LOG(T5ERROR) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = x if ( IsDBCSLeadByteEx( pLIDA->ulOemCP, bTest ) )";
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
            T5LOG(T5ERROR) << "::NO_VALID_ASCIIANSIFORMAT::" << EncodingHelper::convertToUTF8(pMsgError[0]);
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
              T5LOG(T5ERROR) << ":: WARNING_DIMP_FORMAT_SGML ASCII::"<<    EncodingHelper::convertToUTF8(pMsgError[0]);
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
             T5LOG(T5ERROR) << "::rc = " << usRc << ":: " << EncodingHelper::convertToUTF8(pMsgError[0]);
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
          T5LOG(T5ERROR) << "::NO_VALID_ASCIIANSIFORMAT::SGML ASCII";
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
            T5LOG(T5ERROR) <<  ":: WARNING_DIMP_FORMAT::SGML ANSI";
            //usRc = T5LOG(T5ERROR) <<  ":: WARNING_DIMP_FORMAT::SGML ANSI";
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
           T5LOG(T5ERROR) <<  ":: " << usRc << "::" << EncodingHelper::convertToUTF8(pMsgError[0]);
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
		     T5LOG(T5ERROR) << "::rc = ERROR_DIMP_UTF16WRONG = " << usRc;
		   } /* endif */
	   }
	   else
	   {  // error - this should not occur...
		   usRc = ERROR_MEM_INVALID_SGML;
		   T5LOG(T5ERROR) <<  "::rc =  ERROR_MEM_INVALID_SGML" ;
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

  pLIDA->mem->importDetails->segmentsCount++;           // Increase segment sequence number in any case

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
    usRC = (USHORT)pLIDA->mem->putProposal( *(pLIDA->pProposal) );

    if ( !usRC )
    {
      pLIDA->mem->importDetails->segmentsImported++;         // Increase the segment counter
    }
    else
    {
      pLIDA->mem->importDetails->invalidSegmentsRCs[usRC] ++;
      pLIDA->mem->importDetails->invalidSegments++;      // increase invalid segment counter 
      //if( pLIDA->mem->importDetails->invalidSegments < 100){
         pLIDA->mem->importDetails->firstInvalidSegmentsSegNums.push_back(pSegment->lSegNum);
      //}
    } /* endif */
  }
  else
  {
    pLIDA->mem->importDetails->invalidSegmentsRCs[-1]++;
    pLIDA->mem->importDetails->invalidSegments++;      // increase invalid segment counter 
    
    //pRIDA->pMem->importDetails->invalidSegmentsRCs[iRC] ++;
    #ifdef IMPORT_LOGGING
    // write segment to log file
    if ( T5Logger::GetInstance()->CheckLogLevel(T5INFO) )
    {
      T5LOG(T5DEBUG) <<"Segment "<<pSegment->lSegNum <<" not imported\r\n";
      T5LOG(T5DEBUG) <<"Reason         = \r\n",  pSegment->szReason ;
      T5LOG(T5DEBUG) <<"Document       = \r\n",  pSegment->szDocument ;
      T5LOG(T5DEBUG) <<"SourceLanguage = \r\n",  pSegment->szSourceLang ;
      T5LOG(T5DEBUG) <<"TargetLanguage = \r\n",  pSegment->szTargetLang ;
      T5LOG(T5DEBUG) <<"Markup         = \r\n",  pSegment->szFormat ;
      T5LOG(T5DEBUG) <<"Source         = \r\n";
      T5LOG(T5DEBUG) <<"\r\n", pSegment->szSource ;
      T5LOG(T5DEBUG) <<"Target         = \r\n";
      T5LOG(T5DEBUG) <<"\r\n\r\n", pSegment->szTarget ;
    } /* endif */
    #endif
  } /* endif */

  return( usRC );
} /* end of function MEMINSERTSEGMENT */

