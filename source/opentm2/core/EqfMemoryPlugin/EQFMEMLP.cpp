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
static USHORT  MemLoadStart( MEM_LOAD_DLG_IDA* pIda, HWND, ImportStatusDetails*     pImportData);
static USHORT  MemLoadProcess(  MEM_LOAD_DLG_IDA* pLIDA );


USHORT /*APIENTRY*/ MEMINSERTSEGMENT
( 
  LONG lMemHandle, 
  PMEMEXPIMPSEG pSegment 
);


void GetElapsedTime( long long *plTime )
{
  if(plTime) *plTime = 0;
  //struct timespec now;
  //clock_gettime(CLOCK_MONOTONIC, &now);
  //*plTime =  now.tv_sec + now.tv_nsec / 1000000000.0;
}



// import logging 
  //static LONG lImportStartTime = 0;
  static LONG64 lMemAccessTime = 0;
  static LONG64 lFileReadTime = 0;
  static LONG64 lOtherTime = 0;

//#endif


INT_PTR /*CALLBACK*/ NTMTagLangDlg( HWND,         // Dialog window handle
               WINMSG,       // Message ID
               WPARAM,       // Message parameter 1
               LPARAM );     // Message parameter 2


#include "lowlevelotmdatastructs.h"



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
static USHORT  MemLoadStart( MEM_LOAD_DLG_IDA* pLIDA,
                             HWND             hWnd,
                             ImportStatusDetails*     pImportData )
{
   BOOL             fOK = TRUE;          // Return code
   USHORT           usDosRc = 0;       // Return code from Dos open operation
   PSZ              pReplAddr[2];         // Pointer to an address list of replacement strings
   //--- Get pointer to dialog IDA

    // Set segment ID to 1 in case the first segment ID is in error or
    // no segment ID is present at all.
    // Put also the address of MEM_IDA into MEM_LOAD_IDA
    strcpy(pLIDA->szSegmentID, "1");

    pLIDA->pProposal = new(OtmProposal);

   // If fOK. Get file and memory database handle as well as their path
   // and names from the dialog IDA and store them in the MEM_LOAD_IDA.
   // Then open the tagtable and get its Size. If an error occurred
   // set fOK to FALSE and issue the appropriate message.
   if(pLIDA->mem == nullptr){
    fOK = false;
   }
   
   if ( fOK )
   {
      // Move values from dialog IDA to load IDA

      pLIDA->ulAnsiCP = pLIDA->ulOemCP = 1;

      // read the file size of the file to be imported and store it in IDA
      if(pLIDA->hFile){
        //usDosRc = UtlGetFileSize( pLIDA->hFile, &(pLIDA->ulTotalSize), FALSE );
      }else if(!pLIDA->fileData.empty()){
        //pLIDA->ulTotalSize = pLIDA->fileData.size();
      }else{
        T5LOG(T5ERROR) << "stream and file pointer of tmx file both are equal to null";
        fOK = FALSE;
      }

      if (usDosRc)
      {
        fOK = FALSE;
      }
      else
      {
        /*
        // Create full path to the memory database format table
        Properties::GetInstance()->get_value( KEY_OTM_DIR, pLIDA->szFullFtabPath, MAX_EQF_PATH );
        strcat(pLIDA->szFullFtabPath, "/TABLE/");
        strcat(pLIDA->szFullFtabPath, MEM_FORMAT_TABLE);

        // Load the format table and allocate storage for it
        fOK = ( TALoadTagTableHwnd( MEM_FORMAT_TABLE, &pLIDA->pFormatTable,
                                     TRUE, TRUE, pLIDA->hwndErrMsg ) == NO_ERROR);//*/
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
       } /* endif */

       memset( pLIDA->pstMemInfo, 0, sizeof(MEMEXPIMPINFO) );
       strcpy( pLIDA->pstMemInfo->szName, pLIDA->szMemName );
       pLIDA->mem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );
       if ( pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS ) 
       {
         pLIDA->pstMemInfo->fCleanRTF = TRUE;
         pLIDA->pstMemInfo->fTagsInCurlyBracesOnly = TRUE;
       }
       
       memset(&(pLIDA->TMXImport), 0 ,sizeof(pLIDA->TMXImport));
       fOK = (pLIDA->TMXImport.StartImport(pLIDA->szFilePath, pLIDA->pstMemInfo,  pImportData ) == 0);
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

       memset( pLIDA->pstMemInfo, 0, sizeof(MEMEXPIMPINFO) );
       strcpy( pLIDA->pstMemInfo->szName, pLIDA->szMemName );
       pLIDA->mem->getSourceLanguage( pLIDA->pstMemInfo->szSourceLang, sizeof(pLIDA->pstMemInfo->szSourceLang) );

       fOK = (pLIDA->TMXImport.StartImport(pLIDA->szFilePath, pLIDA->pstMemInfo, pImportData ) == 0);
       
       pLIDA->mem->getSourceLanguage( szMemSourceLang, sizeof(szMemSourceLang) );

       // check if memory source lanuage matchs source language of imported file
       if ( pLIDA->pstMemInfo->szSourceLang[0] && (strcmp( pLIDA->pstMemInfo->szSourceLang, szMemSourceLang ) != 0) )
       {
         USHORT usMBCode;

         pReplAddr[0] = pLIDA->pstMemInfo->szSourceLang;
         pReplAddr[1] = szMemSourceLang;
         T5LOG(T5INFO)  <<  "::ERROR_MEM_DIFFERENT_SOURCE_LANG_IMPORT::", pReplAddr[0] ;
         
       } /* endif */

       // update memory description with description of imported memory
       if ( fOK )
       {
         pLIDA->mem->setDescription( pLIDA->pstMemInfo->szDescription );
       } /* endif */

     } /* endif */

     //** GQ TODO: cleanup of resources in case of failures
   } /* endif */

   if ( !fOK )
   {
     // call import end function when available
     pLIDA->TMXImport.EndImport();

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
      } /* endif */
   }else{
    time( &pLIDA->mem->importDetails->lImportStartTime );
    T5LOG(T5DEBUG)  << "************ Memory Import Log ********************* Memory import started at   : "<<
          asctime( localtime( &pLIDA->mem->importDetails->lImportStartTime ) )<< "\tMemory name                : "  << pLIDA->szMemName ;
   } /* endif */   

   return ((USHORT)fOK);
} /* end of function MemLoadStart */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MemLoadProcess                                           |
//+----------------------------------------------------------------------------+
//|Function call:     static USHORT MemLoadProcess( MEM_LOAD_DLG_IDA*  pLIDA  )    |
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
static USHORT MemLoadProcess( MEM_LOAD_DLG_IDA*  pLIDA, ImportStatusDetails*     pImportData  ) //pointer to the load IDA
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
     //pLIDA->lProgress = 0;
     
     usRc = pLIDA->TMXImport.ImportNext( MEMINSERTSEGMENT, (LONG)pLIDA, pImportData);

     // handle any error conditions
     if ( pLIDA->pstMemInfo->fError )  
     {
       PSZ pszMsg = pLIDA->pstMemInfo->szError;
       T5LOG(T5ERROR) <<  "::ERROR_MEMIMP_ERROR::MEM_SEG_SYN_ERR::" <<  pszMsg ;
       LOG_AND_SET_RC(usRc, T5INFO, MEM_SEG_SYN_ERR);
     } /* endif */

     // map return code to the ones used by memory export...
     switch ( usRc )
     {
       case 0 :
         LOG_AND_SET_RC(usRc, T5DEVELOP, MEM_PROCESS_OK);
         break;
       case MEM_IMPORT_COMPLETE: 
         LOG_AND_SET_RC(usRc, T5INFO, MEM_PROCESS_END);
         break;
       case MEM_SEG_SYN_ERR:
         // leave as-is
         break;
       default:
         LOG_AND_SET_RC(usRc, T5INFO, MEM_DB_ERROR);
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
  //memset( &PrivateData, 0, sizeof( FCTDATA ) );
  PrivateData.fComplete = TRUE;
  PrivateData.mem = mem;
  
  if(!pData->fileData.empty())
    PrivateData.fileData = std::move(pData->fileData);
  
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
  MEM_LOAD_DLG_IDA* pLoadIDA = &pData->MemLoadIda;   // Pointer to the load IDA

  bool fOkFailHandled = false;
   /*******************************************************************/
   /* Allocate storage for the MEM_LOAD_DLG_IDA. The area will be free*/
   /* in the function  EQFMemloadStart. Only in case of an error      */
   /* it will be freed here.                                          */
   /*******************************************************************/
   memset(pLoadIDA, 0, sizeof(*pLoadIDA));
   //fOK = UtlAllocHwnd( (PVOID *)&pLoadIDA, 0L, (LONG)sizeof(MEM_LOAD_DLG_IDA), ERROR_STORAGE, HWND_FUNCIF );
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
   if ( fOK  )
   {
     if ( pData->fileData.empty() && !UtlFileExist( pszInFile ) )
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
    if(pData->fileData.empty()){
      strcpy( pLoadIDA->szFilePath, pszInFile );
      pLoadIDA->hFile = FilesystemHelper::OpenFile(pszInFile, "r", false);
      if( pLoadIDA->hFile == NULLHANDLE )
      {
        fOK = FALSE;
      }
    }else{
      pLoadIDA->hFile = nullptr;
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
         if ( pLoadIDA->hFile && pLoadIDA->hFile )
         {
            UtlClose( pLoadIDA->hFile, FALSE );
         } /* endif */
         //UtlAlloc( (PVOID *) &pLoadIDA, 0L, 0L, NOMSG );
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
  MEM_LOAD_DLG_IDA* pLIDA =  &pData->MemLoadIda;        // Pointer to the DLG load IDA
  USHORT            usPhase;           // current processing phase
  
  UtlSetUShort( QS_LASTERRORMSGID, 0 );
  usPhase = pData->usMemLoadPhase;

  switch ( usPhase )
  {
    case MEM_START_IMPORT:
       T5LOG( T5INFO) << "::MEM_START_IMPORT";
       pData->mem->importDetails->usProgress = 0;
       
       if(!pData->fileData.empty()){
        pLIDA->fileData = std::move(pData->fileData);
       }

       if ( !MemLoadStart( &(pData->MemLoadIda), HWND_FUNCIF, pData->mem->importDetails ) )
       {
         //TMManager *pFactory = TMManager::GetInstance();
         //pFactory->closeMemory( pLIDA->mem );
         if ( pLIDA->hFile ) CloseFile( &(pLIDA->hFile));
         pData->usMemLoadRC = UtlQueryUShort( QS_LASTERRORMSGID );
         if ( pData->usMemLoadRC == 0 ) pData->usMemLoadRC = ERROR_MEMIMP_ERROR;
         usPhase = 0;
       }
       else
       {
         usPhase = MEM_IMPORT_TASK;
       } /* endif */
       //UtlAlloc( (PVOID *) &pLIDA, 0L, 0L, NOMSG );
      break;

    case MEM_IMPORT_TASK:
      {
        
        T5LOG( T5DEVELOP) << "::MEM_IMPORT_TASK, progress = " << pData->mem->importDetails->usProgress;
        USHORT usRc = MemLoadProcess( pLIDA, pData->mem->importDetails );
        switch ( usRc )
        {
          case MEM_PROCESS_OK:     
            usPhase = MEM_IMPORT_TASK;
            break;
          case MEM_PROCESS_END:
            if(!pData->mem->importDetails->importRc){
              pData->mem->importDetails->usProgress = 100;
            }
            if ( pLIDA->hFile ) CloseFile( &(pLIDA->hFile));
            if ( pLIDA->mem )
            {
              // organize index file of imported memory
              pLIDA->mem->NTMOrganizeIndexFile();
            } /* endif */
            usPhase = MEM_END_IMPORT;
            break;
          case MEM_DB_CANCEL :
            usPhase = MEM_END_IMPORT;
            break;
          default:
            pLIDA = (MEM_LOAD_DLG_IDA*)&pData->MemLoadIda;
            if ( pLIDA->hFile ) CloseFile( &(pLIDA->hFile));
            pData->usMemLoadRC = usRC = usRc? usRc : UtlQueryUShort( QS_LASTERRORMSGID );
            strncpy(pData->szError, pLIDA->pstMemInfo->szError, sizeof(pData->szError));
            usPhase = MEM_END_IMPORT;
            break;
        } /* end switch */
      }
      break;

    case MEM_END_IMPORT:
       if ( pLIDA != NULL )
       {
         if ( ( pLIDA->usImpMode == MEM_FORMAT_TMX ) || 
              ( pLIDA->usImpMode == MEM_FORMAT_TMX_TRADOS ) ||
              ( pLIDA->usImpMode == MEM_FORMAT_XLIFF_MT ) )
         {
           pLIDA->TMXImport.WriteEnd();

           if ( pLIDA->pstMemInfo )       UtlAlloc( (PVOID *)&pLIDA->pstMemInfo, 0L, 0L, NOMSG );
           if ( pLIDA->pstSegment)        UtlAlloc( (PVOID *)&pLIDA->pstSegment, 0L, 0L, NOMSG );
         } /* endif */

         
         if ( pLIDA->hFile ) CloseFile( &(pLIDA->hFile));
         if ( pLIDA->pFormatTable )
         {
           TAFreeTagTable( pLIDA->pFormatTable );
           pLIDA->pFormatTable = NULL;
         } /* endif */
              
        {
          LONG lCurTime = 0;  
          time( &lCurTime );
                  
          if ( pData->mem->importDetails->lImportStartTime )
          {
            LONG lDiff = lCurTime - pData->mem->importDetails->lImportStartTime;
            char buff[256];
            sprintf( buff, "Overall import time is      : %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
                    (lDiff - (lDiff / 3600 * 3600)) / 60,
                    (lDiff - (lDiff / 3600 * 3600)) % 60 );
                    
            pData->mem->importDetails->importTimestamp = buff;
          }
          
          if(VLOG_IS_ON(1)){
            std::string logMsg = "::Memory import ended at     : " + std::string(asctime( localtime( &lCurTime ))) +
                                  "\tNumber of segments imported : " + toStr(pData->mem->importDetails->segmentsImported)+
                                "\tNumber of invalid segments  : " + toStr(pData->mem->importDetails->invalidSegments)+ 
                                "\tNumber of OTMUTF8 segments  : " + toStr(pData->mem->importDetails->resSegments) + "\t" + pData->mem->importDetails->importTimestamp;
            T5LOG( T5TRANSACTION) << logMsg;
          }
        }

         UtlAlloc( (PVOID *) &(pLIDA->pTokenList),   0L, 0L, NOMSG );
         if ( pLIDA->pProposal != NULL ) delete(pLIDA->pProposal);
       } /* endif */
       usPhase = 0;;
       if(!pData->mem->importDetails->importRc){
          pData->mem->importDetails->usProgress = 100;
        }
       break;
 } /* endswitch */

 pData->usMemLoadPhase = usPhase;

 if ( usPhase == 0 )
 {
   pData->fComplete = TRUE;
 } /* endif */

 return( usRC );

} /* end of function MemFuncImportProcess */



#include <xercesc/sax/SAXParseException.hpp>
DECLARE_int32(maxBadSegmentsIdsSaved);
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

  MEM_LOAD_DLG_IDA* pLIDA = (MEM_LOAD_DLG_IDA*)lMemHandle;

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
    pLIDA->pProposal->setDocName( pSegment->szDocument );

    pLIDA->pProposal->setSource( pSegment->szSource );
    pLIDA->pProposal->setSourceLanguage( pSegment->szSourceLang );
    pLIDA->pProposal->setMarkup( pSegment->szFormat );
    pLIDA->pProposal->setTarget( pSegment->szTarget );
    pLIDA->pProposal->setTargetLanguage( pSegment->szTargetLang );
    pLIDA->pProposal->setAddInfo( pSegment->szAddInfo );
    pLIDA->pProposal->setContext( pSegment->szContext );
    pLIDA->pProposal->setSegmentId( pSegment->lSegNum );

    pLIDA->pProposal->pInputSentence = new TMX_SENTENCE(pSegment->szSource, pSegment->szTarget);
    
    if(pLIDA->pProposal->pInputSentence->wasParsedSuccessfully()){
      pLIDA->pProposal->setSegmentId(0);// set to 0 to generate new idd
      // insert/replace segment in(to) memory
      usRC = (USHORT)pLIDA->mem->putProposal( *(pLIDA->pProposal) );
    }else{
      T5LOG(T5ERROR) << "TMX_SENTENSE was not parsed properly,  src = \'"<< EncodingHelper::convertToUTF8(pSegment->szSource) << "\'; target = \'" << EncodingHelper::convertToUTF8(pSegment->szTarget) << "\'; segNum = " << pSegment->lSegNum;
      usRC = -2;
    }

    if ( !usRC )
    {
      pLIDA->mem->importDetails->segmentsImported++;         // Increase the segment counter
    }
    else
    {
      pLIDA->mem->importDetails->invalidSegmentsRCs[usRC] ++;
      pLIDA->mem->importDetails->invalidSegments++;      // increase invalid segment counter 
      //if( pLIDA->mem->importDetails->invalidSegments < 100){
      if(pLIDA->mem->importDetails->invalidSegments < FLAGS_maxBadSegmentsIdsSaved){
         pLIDA->mem->importDetails->firstInvalidSegmentsSegNums.push_back(std::make_tuple(pSegment->lSegNo, usRC) );
      }
      if(usRC == BTREE_LOOKUPTABLE_CORRUPTED || usRC == BTREE_LOOKUPTABLE_TOO_SMALL || usRC == TMD_SIZE_IS_BIGGER_THAN_ALLOWED)
      {    
        pLIDA->mem->importDetails->importRc = usRC;
        std::string msg = "TM is reached it's size limit, please create another one and import segments there, rc = " 
          + toStr(usRC) + "; aciveSegment = " + toStr(pSegment->lSegNo) + "\n\nSegment " + toStr(pSegment->lSegNo) + " not imported\r\n" 
          +"\nReason         = " +  pSegment->szReason 
          +"\nDocument       = " +  pSegment->szDocument 
          +"\nSourceLanguage = " +  pSegment->szSourceLang 
          +"\nTargetLanguage = " +  pSegment->szTargetLang 
          +"\nMarkup         = " +  pSegment->szFormat 
          +"\nSource         = " +  EncodingHelper::convertToUTF8(pSegment->szSource) 
          +"\nTarget         = " +  EncodingHelper::convertToUTF8(pSegment->szTarget) ;
        throw xercesc::SAXException(msg.c_str());        
      }
    } /* endif */
  }
  else
  {
    pLIDA->mem->importDetails->invalidSegmentsRCs[-1]++;
    pLIDA->mem->importDetails->invalidSegments++;      // increase invalid segment counter 
    
    #ifdef IMPORT_LOGGING
    // write segment to log file
    if ( T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) )
    {
      T5LOG(T5DEBUG) <<"Segment "<<pSegment->lSegNo <<" not imported";
      T5LOG(T5DEBUG) <<"; Reason         = " <<  pSegment->szReason ;
      T5LOG(T5DEBUG) <<"; Document       = " <<  pSegment->szDocument ;
      T5LOG(T5DEBUG) <<"; SourceLanguage = " <<  pSegment->szSourceLang ;
      T5LOG(T5DEBUG) <<"; TargetLanguage = " <<  pSegment->szTargetLang ;
      T5LOG(T5DEBUG) <<"; Markup         = " <<  pSegment->szFormat ;
      T5LOG(T5DEBUG) <<"; Source         = ";
      T5LOG(T5DEBUG) <<"" << pSegment->szSource ;
      T5LOG(T5DEBUG) <<"; Target         = ";
      T5LOG(T5DEBUG) <<"" << pSegment->szTarget ;
    } /* endif */
    #endif
  } /* endif */

  return( usRC );
} /* end of function MEMINSERTSEGMENT */

