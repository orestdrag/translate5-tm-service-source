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

// EQF.H is included by otmmemory.h
// #include <EQF.H>                  // General Translation Manager include file EQF:H 

#include <EQFDDE.H>                  // batch mode definitions
#define INCL_EQFMEM_DLGIDAS
//#include "EQFMEM.ID"                 // Translation Memory IDs
//#include "EQFCOLW.ID"                // column width IDs

BOOL EQFTMMaintain( PSZ );


// new function to replace CREATE_PATH task message
BOOL MemCreatePath( PSZ pszString )
{
  USHORT          usRc;

 std::shared_ptr<EqfMemory>  pInfo = std::make_shared<EqfMemory> ( EqfMemory() );
  TMManager *pFactory = TMManager::GetInstance();

  usRc = (USHORT)-1;
  if(pInfo.get() != NULL && pFactory!= NULL)
     usRc = (USHORT)pFactory->getMemoryInfo( NULL, pszString, pInfo );

  if (usRc == 0)
  {
	  strcpy( pszString,pInfo->szName);
  }
  else
  {
     *pszString = EOS;
  }
  //if(pInfo.get() != NULL)
  //    delete pInfo;

  return (usRc==0);
} /* end of function MemCreatePath */




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
     usTmRc1 = (USHORT)pFactory->closeMemory( pRIDA->pMem.get() );
     pRIDA->pMem = NULL;
   } /* endif */

   if ( pRIDA->pMemTemp != NULL )
   {
     // Close the temporary translation memory
     usTmRc2 = (USHORT)pFactory->closeMemory( pRIDA->pMemTemp.get() );
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
  PPROCESSCOMMAREA  pCommArea
)
  // The function initializes the TM organize. It returns TRUE if
  // the initialization was OK else it returns FASLE.
{
  PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the dialog IDA
  PSZ               pReplAddr[4];   // Pointer to an address list of replacement strings

  // Address the organize IDA
  pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;
  pRIDA->fFirstGet = TRUE;
  pRIDA->pProposal = new OtmProposal();
  
  // --------------------------------------------------------------------
  // Create the Output translation memory

  int iRC = 0;
  pRIDA->pMem->getSourceLanguage( pRIDA->szSourceLanguage, sizeof(pRIDA->szSourceLanguage) );
  pRIDA->pMem->getDescription( pRIDA->szBuffer, sizeof(pRIDA->szBuffer) );
  
  strcpy( pRIDA->szTempMemName, "$Org-" );
  strcat( pRIDA->szTempMemName, pRIDA->szMemName );

  // delete any memory left over from a previous organize call
  std::string outputMsg;
  if(!TMManager::GetInstance()->TMExistsOnDisk(pRIDA->szTempMemName)){
    auto ll = T5Logger::GetInstance()->suppressLogging();
    std::string strMemName(pRIDA->szTempMemName);
    TMManager::GetInstance()->DeleteTM(strMemName, outputMsg);
    outputMsg = "";
    T5Logger::GetInstance()->desuppressLogging(ll);
  }
  pRIDA->pMemTemp = TMManager::GetInstance()->CreateNewEmptyTM(pRIDA->szTempMemName, pRIDA->szSourceLanguage, "", iRC);
  
  if ( (iRC != 0) || (pRIDA->pMemTemp == NULL ) )
  {
    //usRc = FALSE;
    pRIDA->fMsg = FALSE;
    TMManager::GetInstance()->showLastError( pRIDA->szPluginName, pRIDA->szTempMemName, NULL, pRIDA->hwndErrMsg );
  } /* endif */

  // --------------------------------------------------------------------
  // The initialization of translation memory organize failed
  // Close translation memories. Clean the storage allocations.
  if ( iRC )
  {
    // Issue an error message.
    if ( pRIDA != NULL )
    {
      // Close the Translation memory which should be organized.
      if ( pRIDA->pMemTemp != NULL ) 
      {
        TMManager::GetInstance()->closeMemory( pRIDA->pMemTemp.get() );
        pRIDA->pMemTemp = NULL;
        
        std::string strMsg;
        TMManager::GetInstance()->DeleteTM( pRIDA->szTempMemName, strMsg );
      } /* endif */

      // Issue the error message :"Initialization of organize for
      // translation memory %1 failed". Issue the message only if
      // no other significant message has been issued already.
      /*
      if ( !pRIDA->fMsg )
      {
        pReplAddr[0] = pRIDA->szMemName;
        UtlErrorHwnd( ERROR_MEM_ORGANIZE_INITFAILED, MB_CANCEL, 1,
                      &pReplAddr[0], EQF_ERROR, pRIDA->hwndErrMsg );
      } /* endif */

      if ( pRIDA->fBatch )
      {
        pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
        } /* endif */
      } /* endif */

    } /* endif */
  }
  // --------------------------------------------------------------------
  // Move process-ID into mp1 and issue a message WM_EQF_MEMORGANIZE_PROCESS
  else
  {
    pRIDA->NextTask = MEM_ORGANIZE_TASK;
  } 
  return iRC;
} /* end of function EQFMemOrganizeStart */



USHORT
NTMCloseOrganize ( PMEM_ORGANIZE_IDA pRIDA,           //pointer to organize IDA
                   USHORT            usMsgHandling )  //message handling flag
{
  USHORT  usURc = NO_ERROR ;   //function returncode
  std::string outputMessage, strMemName(pRIDA->szMemName), strTempMemName(pRIDA->szTempMemName);

  /* close the original TM                                          */
  if ( pRIDA->pMem != NULL )
  {
    int use_count = pRIDA->pMem.use_count();
    if(use_count != 3)//1)
    {
      T5LOG(T5WARNING) << ":: use_count for tm for reorganize is not 1, but " << use_count;
    }
    //TMManager::GetInstance()->CloseTM(strMemName);
    pRIDA->pMem = nullptr;

    // replace original memory with the organized one
    usURc = TMManager::GetInstance()->ReplaceMemory( strTempMemName, strMemName, outputMessage );

 } /* endif */

 return usURc;
} /* end of function NTMCloseOrganize */


// ================ Handle the message WM_EQF_MEMORGANIZE_PROCESS =====================

VOID EQFMemOrganizeProcess
(
  PPROCESSCOMMAREA  pCommArea
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

  int iProgress = 0;
  int iRC = 0;

  if ( pRIDA->fFirstGet )
  {
    pRIDA->ulInvSegmentCounter = 0;
    pRIDA->ulSegmentCounter = 0;
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
  }
  else if ( iRC == EqfMemory::ERROR_ENTRYISCORRUPTED )
  {
    pCommArea->usComplete = (USHORT)iProgress;
    pRIDA->ulInvSegmentCounter++;
  }
  else if ( iRC == EqfMemory::INFO_ENDREACHED )
  {
    pRIDA->pMemTemp->NTMOrganizeIndexFile();
    T5LOG(T5WARNING) << "reorganize is done, segCount = " << pRIDA->ulSegmentCounter << "; invSegCount = " << pRIDA->ulInvSegmentCounter;
    int use_count = pRIDA->pMemTemp.use_count();
    if(use_count != 1){
      T5LOG(T5WARNING) << ":: use_count for temporary tm is not 1, but " << use_count;
    }
    //pRIDA->pMemTemp->TmBtree.fb.Flush();
    //pRIDA->pMemTemp->InBtree.fb.Flush();  
    pRIDA->pMemTemp = nullptr;
    //TMManager::GetInstance()->closeMemory( pRIDA->pMemTemp.get() );

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

          pReplAddr[0] = pRIDA->szMemName;
          //pReplAddr[1] = ltoa( pRIDA->ulSegmentCounter, szNumber1, 10 );
          //pReplAddr[2] = ltoa( pRIDA->ulInvSegmentCounter, szNumber2, 10 );
          UtlError( MESSAGE_MEM_ORGANIZE_COMPLETED, MB_OK, /*3*/1, &pReplAddr[0], EQF_INFO );
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
      } /* endif */
    } /* endif */

    // -----------------------------------------------------
    // Issue message WM_EQF_MEMORGANIZE_END
    pRIDA->NextTask = MEM_END_ORGANIZE;
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
    } /* endif */

    // Close the input translation memory and temporary translation memory
    CloseTmAndTempTm( pRIDA );

    // Delete the temporary translation memory    
    std::string strMsg;
    TMManager::GetInstance()->DeleteTM( pRIDA->szTempMemName, strMsg );

    // Issue message WM_EQF_MEMORGANIZE_END
    pRIDA->NextTask = MEM_END_ORGANIZE;    
  } /* endif */
} /* end of function EQFMemOrganizeProcess */


// ================ Handle the message WM_EQF_MEMORGANIZE_END =========================

VOID EQFMemOrganizeEnd
(
  PPROCESSCOMMAREA  pCommArea
)
{
  PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the data for the organize process
  PSZ               pReplAddr[2];   // Arrey of pointers to replacement strings

  // Get the address of the process IDA by means of the process handle
  pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

  // Check if the termination was due to a
  // CLOSE message. Mp2 is in that case not zero.
  
  // Refresh the memory database list box
  sprintf( pCommArea->szBuffer, "%s:%s", pRIDA->szPluginName, pRIDA->szMemName );

  if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
  {
    EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pCommArea->szBuffer ));
    EqfRemoveObject( TWBFORCE, HWND_FUNCIF );
  } /* endif */
} /* end of function EQFMemOrganizeEnd */


