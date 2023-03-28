//------------------------------------------------------------------------------
//  EQFMEM00.C - EQF Memory List Handler
//------------------------------------------------------------------------------
//  Copyright Notice:
//
//      Copyright (C) 1990-2016, International Business Machines
//      Corporation and others. All rights reserved


#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM

#include "EQFFUNCI.H"
#include "../pluginmanager/PluginManager.h"
#include "LogWrapper.h"
#include "tm.h"
#include "MemoryUtil.h"

// EQF.H is included by otmmemory.h
// #include <EQF.H>                  // General Translation Manager include file EQF:H 

#include <EQFDDE.H>                  // batch mode definitions
#define INCL_EQFMEM_DLGIDAS
#include "EQFMEM.ID"                 // Translation Memory IDs
#include "EQFCOLW.ID"                // column width IDs

BOOL EQFTMMaintain( PSZ );


// new function to replace CREATE_PATH task message
BOOL MemCreatePath( PSZ pszString )
{
  USHORT          usRc;

 POPENEDMEMORY pInfo = new( OPENEDMEMORY );
  TMManager *pFactory = TMManager::GetInstance();

  usRc = (USHORT)-1;
  if(pInfo != NULL && pFactory!= NULL)
     usRc = (USHORT)pFactory->getMemoryInfo( NULL, pszString, pInfo );

  if (usRc == 0)
  {
	  strcpy( pszString,pInfo->szName);
  }
  else
  {
     *pszString = EOS;
  }
  if(pInfo != NULL)
      delete pInfo;

  return (usRc==0);
} /* end of function MemCreatePath */

//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemFuncDeleteMem
//------------------------------------------------------------------------------
//Description:       Deletes a Translation Memory in function I/F mode
//------------------------------------------------------------------------------
//Input parameter:  PSZ pszMemName  ptr to name of memory
//------------------------------------------------------------------------------
//Returncode type:   USHORT              error code or 0 if success
//------------------------------------------------------------------------------

USHORT MemFuncDeleteMem( PSZ pszMemName )
{
  BOOL            fOK = TRUE;         // return value
  USHORT          usRC = NO_ERROR;    // function return code

  // check if a TM has been specified
  if ( (pszMemName == NULL) || (*pszMemName == EOS) )
  {
    usRC = ERROR_MEMORY_NOTFOUND;
    T5LOG( T5DEBUG) << "Error in MemFuncDeleteMem::(pszMemName == NULL) || (*pszMemName == EOS) ";
  } /* endif */

 OPENEDMEMORY Info;
  TMManager *pFactory = TMManager::GetInstance();
  // check if there is a TM with the given name
  if ( usRC == NO_ERROR )
  {
      // check if memory existed
      usRC = (USHORT)pFactory->getMemoryInfo( NULL, pszMemName, &Info );
      if (usRC != 0)
      {
        usRC = ERROR_MEMORY_NOTFOUND;              
        T5LOG( T5DEBUG) << "Error in MemFuncDeleteMem::check if memory existed error:: memName =  " << pszMemName << " not found; usRC = " << usRC;
      }
  }

  // Delete the TM
  if ( usRC == NO_ERROR )
  {
    usRC = pFactory->deleteMemory( NULL,pszMemName);
  }

  return( usRC );
} /* end of function MemFuncDeleteMem  */

/*
USHORT MemFuncPrepOrganize
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
);
// Prepare the organize of a TM in function call mode
USHORT MemFuncOrganizeProcess
(
  PFCTDATA    pData                    // function I/F session data
);


// Organize a TM in function call mode
USHORT MemFuncOrganizeMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new organize run or continue current one
  // call TM organize
  if ( usRC == NO_ERROR )
  {
    pData->sLastFunction = FCT_EQFORGANIZEMEM;
    if ( pData->fComplete )              // has last run been completed
    {
      // prepare a new analysis run
      usRC = MemFuncPrepOrganize( pData, pszMemName );
    }
    else
    {
      // continue current organize process
      usRC = MemFuncOrganizeProcess( pData );
    } 
  }
  return( usRC );
} /* end of function MemFuncOrganizeMem */



#define MEM_START_ORGANIZE  USER_TASK + 1
#define MEM_ORGANIZE_TASK   USER_TASK + 2
#define MEM_END_ORGANIZE    USER_TASK + 3


// ================ Close the Tm to be organized and close the temporary Tm ============

 USHORT CloseTmAndTempTm
 (
    PMEM_ORGANIZE_IDA  pRIDA                  // Pointer to the organize instance area
 )
   // The function returns TRUE if everything OK else FALSE
 {
   USHORT    usTmRc1 = FALSE;        // Tm return code
   USHORT    usTmRc2 = FALSE;        // Tm return code
   USHORT    usRc = TRUE;            // Function return code
   TMManager *pFactory = TMManager::GetInstance();

   if ( pRIDA->pMem != NULL )
   {
     // Close the translation memory to be organized
     usTmRc1 = (USHORT)pFactory->closeMemory( pRIDA->pMem );
     pRIDA->pMem = NULL;
   } /* endif */

   if ( pRIDA->pMemTemp != NULL )
   {
     // Close the temporary translation memory
     usTmRc2 = (USHORT)pFactory->closeMemory( pRIDA->pMemTemp );
     pRIDA->pMemTemp = NULL;
   } /* endif */

   if ( usTmRc1 || usTmRc2 )
   {
     usRc = FALSE;
   } /* endif */

 return usRc;
 } /* end of function CloseTmAndTempTm  */

//#define EqfRemoveObject( flg, hwnd)  (BOOL)WinSendMsg( EqfQueryObjectManager(),\
//                                                WM_EQF_REMOVEOBJECT,           \
//                                                MP1FROMSHORT( flg ),           \
//                                                MP2FROMHWND(hwnd) )

bool EqfRemoveObject(auto flg, auto hwnd){
  LOG_UNIMPLEMENTED_FUNCTION; 
  return true;
}

// ================ Handle the message WM_EQF_MEMORGANIZE_START =======================

time_t lOrganizeStartTime;

USHORT EQFMemOrganizeStart
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd
)
  // The function initializes the TM organize. It returns TRUE if
  // the initialization was OK else it returns FASLE.
{
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the dialog IDA
    USHORT            usRc = TRUE;    // Return code
    PSZ               pReplAddr[4];   // Pointer to an address list of replacement strings
    TMManager *pFactory = TMManager::GetInstance();

    // Address the organize IDA
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;
    pRIDA->fFirstGet = TRUE;
    pRIDA->pProposal = new OtmProposal();

#ifdef ORGANIZE_LOGGING
  {
    if ( T5Logger::GetInstance()->CheckLogLevel(T5INFO) )
    {
      time( &lOrganizeStartTime );
      T5LOG(T5INFO) <<  "************ Memory Organize Log *********************\n"  
        <<  "\nMemory organize started at : " 
        //<< asctime( localtime( &lOrganizeStartTime ) 
        << "\nMemory name                :  "<< pRIDA->szMemName ;
    } /* endif */
  }
#endif

    // --------------------------------------------------------------------
    // Create the Output translation memory
    if ( usRc )
    {
      int iRC = 0;
     OPENEDMEMORY MemInfo;
      OtmPlugin *pAnyPlugin = (OtmPlugin *)pRIDA->pMem->getPlugin();
      if ( pAnyPlugin->getType() == OtmPlugin::eTranslationMemoryType )
      {
        OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)pAnyPlugin;
        pRIDA->pMem->getSourceLanguage( pRIDA->szSourceLanguage, sizeof(pRIDA->szSourceLanguage) );
        pRIDA->pMem->getDescription( pRIDA->szBuffer, sizeof(pRIDA->szBuffer) );
        pPlugin->getMemoryInfo( pRIDA->szMemName, &MemInfo );
      }
      
      strcpy( pRIDA->szPluginName, pAnyPlugin->getName() );
      strcpy( pRIDA->szTempMemName, "$Org-" );
      strcat( pRIDA->szTempMemName, pRIDA->szMemName );

      // delete any memory left over from a previous organize call
      {
        auto ll = T5Logger::GetInstance()->suppressLogging();
        pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );
        T5Logger::GetInstance()->desuppressLogging(ll);
      }
      if ( MemInfo.szFullPath[0] != '\0' )
      {
        pRIDA->pMemTemp = pFactory->createMemory( pRIDA->szPluginName,
             pRIDA->szTempMemName,  pRIDA->szBuffer,  pRIDA->szSourceLanguage, MemInfo.szFullPath[0], NULL, true, &iRC );
      }
      else
      {
        pRIDA->pMemTemp = pFactory->createMemory( pRIDA->szPluginName, pRIDA->szTempMemName, pRIDA->szBuffer, 
                  pRIDA->szSourceLanguage, '\0', NULL, true, &iRC );
      }

      if ( (iRC != 0) || (pRIDA->pMemTemp == NULL ) )
      {
        usRc = FALSE;
        pRIDA->fMsg = FALSE;
        pFactory->showLastError( pRIDA->szPluginName, pRIDA->szTempMemName, NULL, pRIDA->hwndErrMsg );
      } /* endif */
    } /* endif */

    // --------------------------------------------------------------------
    // The initialization of translation memory organize failed
    // Close translation memories. Clean the storage allocations.
    if ( !usRc )
    {
      // Issue an error message.
      if ( pRIDA != NULL )
      {
        // Close the Translation memory which should be organized.
        if ( pRIDA->pMemTemp != NULL ) 
        {
          pFactory->closeMemory( pRIDA->pMemTemp );
          pRIDA->pMemTemp = NULL;
          pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );
        } /* endif */

        // Issue the error message :"Initialization of organize for
        // translation memory %1 failed". Issue the message only if
        // no other significant message has been issued already.
        /*
        if ( !pRIDA->fMsg )
        {
          LOG_TEMPORARY_COMMENTED_W_INFO ("OEMTOANSI"); //OEMTOANSI ( pRIDA->szMemName );
          pReplAddr[0] = pRIDA->szMemName;
          UtlErrorHwnd( ERROR_MEM_ORGANIZE_INITFAILED, MB_CANCEL, 1,
                        &pReplAddr[0], EQF_ERROR, pRIDA->hwndErrMsg );
          LOG_TEMPORARY_COMMENTED_W_INFO ("ANSITOOEM"); //ANSITOOEM ( pRIDA->szMemName );
        } /* endif */

        if ( pRIDA->fBatch )
        {
          pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
          if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
          {
            pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
          } /* endif */
        } /* endif */

        // Dismiss the slider window if it had been created
        /*if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          EqfRemoveObject( TWBFORCE, hWnd );
        } /* endif */
      } /* endif */
    }
    // --------------------------------------------------------------------
    // Move process-ID into mp1 and issue a message WM_EQF_MEMORGANIZE_PROCESS
    else if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
    {
      pRIDA->NextTask = MEM_ORGANIZE_TASK;
    }
    else
    {
       T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";  //WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
      //            MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL);
    } /* endif */
    return usRc;
  } /* end of function EQFMemOrganizeStart */



USHORT
NTMCloseOrganize ( PMEM_ORGANIZE_IDA pRIDA,           //pointer to organize IDA
                   USHORT            usMsgHandling )  //message handling flag
{
  USHORT  usURc = NO_ERROR ;   //function returncode
  TMManager *pFactory = TMManager::GetInstance();

  usMsgHandling;

  /* close the original TM                                          */
  if ( pRIDA->pMem != NULL )
  {
    int iRC = 0;

    pFactory->closeMemory( pRIDA->pMem );
    pRIDA->pMem = NULL;

    // replace original memory with the organized one
    iRC = pFactory->replaceMemory( pRIDA->szPluginName, pRIDA->szMemName, pRIDA->szTempMemName );

 } /* endif */

 return usURc;
} /* end of function NTMCloseOrganize */


// ================ Handle the message WM_EQF_MEMORGANIZE_PROCESS =====================

VOID EQFMemOrganizeProcess
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd
)
{
    USHORT            usRc = TRUE;    // Return code to control a process
    USHORT            usDosRc;        // Dos Returncode
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the organize IDA
    PSZ               pReplAddr[3];   // Arrey of pointers to replacement strings

    // Get the address of the process IDA by means of the process handle
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

    // If usRc. Call the memory database load function
    // and handle return codes
    if ( usRc )
    {
      int iProgress = 0;
      int iRC = 0;

      if ( pRIDA->fFirstGet )
      {
        iRC = pRIDA->pMem->getFirstProposal( *(pRIDA->pProposal), &iProgress );
        pRIDA->fFirstGet = FALSE;
      }
      else
      {
        iRC = pRIDA->pMem->getNextProposal( *(pRIDA->pProposal), &iProgress );
      } /* endif */         

      if ( iRC == NO_ERROR )
      {
        pCommArea->usComplete = (USHORT)iProgress;

        // do some consistency checking
        pRIDA->pProposal->getMarkup( pRIDA->szTagTable, sizeof(pRIDA->szTagTable) );
        pRIDA->pProposal->getTargetLanguage( pRIDA->szTargetLanguage, sizeof(pRIDA->szTargetLanguage) );
        if ( (pRIDA->pProposal->getSourceLen() == 0) || (pRIDA->pProposal->getTargetLen() == 0) ||
             (pRIDA->szTargetLanguage[0] == EOS) || (pRIDA->szTagTable[0] == EOS) )
        {
          // ignore invalid proposal
          pRIDA->ulInvSegmentCounter++;
        }
        else
        {
          // write proposal to output memory
          iRC = pRIDA->pMemTemp->putProposal( *(pRIDA->pProposal) );
          if ( iRC != 0 )
          {
            pRIDA->ulInvSegmentCounter++;
          }
          else
          {
            pRIDA->ulSegmentCounter++;
          } /* endif */             
        } /* endif */

        // Post the next WM_EQF_MEMORGANIZE_PROCESS message but check
        // first whether an other message is in the message queue
        // and if it is dispatch it.
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          UtlDispatch();
           T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";  //WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL );
        } /* endif */
      }
      else if ( iRC == OtmMemory::ERROR_ENTRYISCORRUPTED )
      {
        pCommArea->usComplete = (USHORT)iProgress;
        pRIDA->ulInvSegmentCounter++;
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          UtlDispatch();
           T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";  //WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL );
        } /* endif */
      }
      else if ( iRC == OtmMemory::INFO_ENDREACHED )
      {
        TMManager *pFactory = TMManager::GetInstance();

        pRIDA->pMemTemp->rebuildIndex();

        pFactory->closeMemory( pRIDA->pMemTemp );

        pRIDA->pMemTemp = NULL;

        if (usRc)
        {
          //T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";
          usDosRc = NTMCloseOrganize( pRIDA, FALSE );

          // set the progress indicator to 100 percent
          if ( !pRIDA->fBatch )
          {
            pCommArea->usComplete = 100;
            //WinSendMsg( hWnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT( 100 ), NULL );
          } /* endif */

          if ( usDosRc == NO_ERROR )
          {
            // everything in the termination process was ok
            pRIDA->pMem = NULL;

            // end message only in GUI single organize mode
            if ( !pRIDA->fBatch && !pRIDA->pszNameList)
            {
              CHAR szNumber1[10];    // buffer for character string
              CHAR szNumber2[10];    // buffer for character string

              LOG_TEMPORARY_COMMENTED_W_INFO ("OEMTOANSI"); //OEMTOANSI ( pRIDA->szMemName );
              pReplAddr[0] = pRIDA->szMemName;
              //pReplAddr[1] = ltoa( pRIDA->ulSegmentCounter, szNumber1, 10 );
              //pReplAddr[2] = ltoa( pRIDA->ulInvSegmentCounter, szNumber2, 10 );
              UtlError( MESSAGE_MEM_ORGANIZE_COMPLETED, MB_OK, /*3*/1, &pReplAddr[0], EQF_INFO );
              LOG_TEMPORARY_COMMENTED_W_INFO ("ANSITOOEM"); //ANSITOOEM ( pRIDA->szMemName );
            } /* endif */
          }
          else
          {
            usRc = FALSE;
          } /* endif */
        } /* endif */

        if (!usRc)
        {
          // something in the termination process failed
          pReplAddr[0] = pRIDA->szMemName;
          UtlErrorHwnd( ERROR_MEM_ORGANIZE_TERMFAILED, MB_CANCEL, 1, &pReplAddr[0], EQF_ERROR, pRIDA->hwndErrMsg );
          if ( pRIDA->fBatch )
          {
            pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
            if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
            {
              pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
            } /* endif */
          } /* endif */
        } /* endif */

        // -----------------------------------------------------
        // Issue message WM_EQF_MEMORGANIZE_END
        if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
        {
          pRIDA->NextTask = MEM_END_ORGANIZE;
        }
        else
        {
           T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";  //WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_ORGANIZE ), NULL );
        } /* endif */
      }
      else
      {
        // Issue a message "Translation memory organize abnormally terminated."
        if ( !pRIDA->fBatch )
        {
          pReplAddr[0] = pRIDA->szMemName;
          UtlError( ERROR_MEM_ORGANIZE_TERMINATED, MB_CANCEL, 1,
                    &pReplAddr[0], EQF_ERROR );
        }
        else
        {
          pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
          if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
          {
            pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
          } /* endif */
        } /* endif */

        // Close the input translation memory and temporary translation memory
        CloseTmAndTempTm( pRIDA );

          // Delete the temporary translation memory
        TMManager *pFactory = TMManager::GetInstance();
        pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );

        // Issue message WM_EQF_MEMORGANIZE_END
        if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
        {
          pRIDA->NextTask = MEM_END_ORGANIZE;
        }
        else
        {
           T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";  //WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_ORGANIZE ), NULL );
        } /* endif */
      } /* endif */
    } /* endif */
} /* end of function EQFMemOrganizeProcess */


// ================ Handle the message WM_EQF_MEMORGANIZE_END =========================

VOID EQFMemOrganizeEnd
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd,
  LPARAM            mp2,
  BOOL              fTerminate
)
{
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the data for the organize process
    PSZ               pReplAddr[2];   // Arrey of pointers to replacement strings

    // Get the address of the process IDA by means of the process handle
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

    // Check if the termination was due to a
    // CLOSE message. Mp2 is in that case not zero.
    if (SHORT1FROMMP2(mp2) != 0 )
    {
      // -----------------------------------------------------
      // Issue a message "Termination of memory database load"
      // "was forced and is not completed"
      if ( !pRIDA->fBatch )
      {
        LOG_TEMPORARY_COMMENTED_W_INFO ("OEMTOANSI"); //OEMTOANSI ( pRIDA->szMemName );
        pReplAddr[0] = pRIDA->szMemName;
        UtlError( ERROR_MEM_ORGANIZE_TERM_FORCED, MB_CANCEL, 1,
                &pReplAddr[0], EQF_WARNING );
        LOG_TEMPORARY_COMMENTED_W_INFO ("ANSITOOEM"); //ANSITOOEM ( pRIDA->szMemName );
      } /* endif */
      // -----------------------------------------------------
      // Close the input translation memory and temporary translation memory
      CloseTmAndTempTm( pRIDA );

      // Delete the temporary translation memory
      TMManager *pFactory = TMManager::GetInstance();
      pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );
    }
    else
    {
      // Refresh the memory database list box
      sprintf( pCommArea->szBuffer, "%s:%s", pRIDA->szPluginName, pRIDA->szMemName );

      if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
      {
        //T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION"; 
        //ObjBroadcast( WM_EQFN_PROPERTIESCHANGED, PROP_CLASS_MEMORY, pCommArea->szBuffer );
      }
      else
      {
        EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pCommArea->szBuffer ));
      } /* endif */
    } /* endif */

    if ( fTerminate )
    {
      if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
      {
        EqfRemoveObject( TWBFORCE, hWnd );
      } /* endif */
    } /* endif */

#ifdef ORGANIZE_LOGGING
  {
    if ( T5Logger::GetInstance()->CheckLogLevel(T5INFO) )
    {
      LONG lCurTime = 0;  

      time( &lCurTime );
      T5LOG(T5INFO) << "Memory organize ended at   : " << asctime( localtime( &lCurTime ) ) ;
      if ( lOrganizeStartTime )
      {
        LONG lDiff = lCurTime - lOrganizeStartTime;
        T5LOG(T5INFO) << "Overall organize time is "<<lDiff / 3600<<":" 
              << (lDiff - (lDiff / 3600 * 3600)) / 60 <<":" << (lDiff - (lDiff / 3600 * 3600)) % 60  <<"\n";         
                
      }
    } /* endif */
  }
#endif


} /* end of function EQFMemOrganizeEnd */




// Prepare the organize of a TM in function call mode
USHORT MemFuncPrepOrganize
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PPROCESSCOMMAREA pCommArea = NULL;   // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA = NULL;      // pointer to instance area
  BOOL        fOK = TRUE;              // internal O.K. flag
  PSZ         pszParm;                 // error parameter pointer
  CHAR        szMemPath[MAX_EQF_PATH];
  OtmMemory *pMem = NULL;       // TM handle
  SHORT       sRC;                     // value used for symbol checking..
  TMManager *pFactory = TMManager::GetInstance();

   // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
       UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                     EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // check if there is a TM with the given name
   if ( fOK )
   {
     int iRC = 0;

     pMem = pFactory->openMemory( NULL, pszMemName, FOR_ORGANIZE, &iRC );
     if ( iRC != 0)
     {
       pszParm = pszMemName;
       UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     // prepare TM organize process
     if ( fOK )
     {
       // allocate storage for the organize process communication area
       fOK = UtlAllocHwnd( (PVOID *)&pCommArea, 0L,
                           (LONG)sizeof(PROCESSCOMMAREA),
                           ERROR_STORAGE, HWND_FUNCIF );

       // allocate storage for the MEM_ORGANIZE_IDA.
       if ( fOK )
       {
         fOK = UtlAllocHwnd( (PVOID *)&pRIDA, 0L,
                             (LONG)sizeof(MEM_ORGANIZE_IDA),
                             ERROR_STORAGE, HWND_FUNCIF );
       } /* endif */

       if ( fOK )
       {
         // Fill  IDA with necessary values
         pCommArea->pUserIDA = pRIDA;
         pRIDA->pMem = pMem;
         pRIDA->usRC = NO_ERROR;
         strcpy( pRIDA->szMemName, pszMemName );
         pRIDA->fBatch = TRUE;
         pRIDA->hwndErrMsg = HWND_FUNCIF;
         pRIDA->NextTask = MEM_START_ORGANIZE;

         if ( !fOK )
         {
           // free IDA, otherwise it will be freed in organize process
           UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   // enable organize process if OK
   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->sLastFunction = FCT_EQFORGANIZEMEM;
     pData->pvMemOrganizeCommArea = pCommArea;
     pCommArea->pUserIDA = pRIDA;
    } /* endif */

   // cleanup in case of errors
   if ( !fOK )
   {
     usRC = UtlGetDDEErrorCode( HWND_FUNCIF );
     if ( pMem != NULL )
     {
       pFactory->closeMemory( pMem );
     } /* endif */
     pData->fComplete = TRUE;
   } /* endif */

  return( usRC );
} /* end of function MemFuncPrepOrganize */

// Prepare the organize of a TM in function call mode
USHORT MemFuncOrganizeProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PPROCESSCOMMAREA pCommArea = (PPROCESSCOMMAREA)pData->pvMemOrganizeCommArea;          // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;             // pointer to instance area

  switch ( pRIDA->NextTask )
  {
    case MEM_START_ORGANIZE:
      EQFMemOrganizeStart( pCommArea, HWND_FUNCIF );
      usRC = pRIDA->usRC;
      break;

    case MEM_ORGANIZE_TASK:
      EQFMemOrganizeProcess( pCommArea, HWND_FUNCIF );
      usRC = pRIDA->usRC;
      break;

    case MEM_END_ORGANIZE:
      pData->fComplete = TRUE;
      EQFMemOrganizeEnd( pCommArea, HWND_FUNCIF, 0L, TRUE );
      if ( pRIDA->pszNameList ) UtlAlloc( (PVOID *)&pRIDA->pszNameList, 0L, 0L, NOMSG );
      //REMOVESYMBOL( pCommArea->szObjName );
      usRC = pRIDA->usRC;
      if ( pCommArea ) UtlAlloc( (PVOID *)&pCommArea, 0L, 0L, NOMSG );
      if ( pRIDA )     UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
      break;
  } /* endswitch */

  return( usRC );
} /* end of function MemFuncOrganizeProcess */
