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
  pRIDA->pMemTemp = TMManager::GetInstance()->CreateNewEmptyTM(pRIDA->szTempMemName, pRIDA->szSourceLanguage, "", iRC, true);
  
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
    //if(pRIDA->pMem->importDetails == nullptr){
    //  pRIDA->pMem->importDetails = new ImportStatusDetails;
    //}
    //pRIDA->pMem->importDetails->reset();
    //pRIDA->pMem->importDetails->fReorganize = true;
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
    if(use_count != 2)//1)
    {
      T5LOG(T5WARNING) << ":: use_count for tm for reorganize is not 3, but " << use_count;
    }
    //TMManager::GetInstance()->CloseTM(strMemName);
    //pRIDA->pMem = nullptr;

    // replace original memory with the organized one
    usURc = TMManager::GetInstance()->ReplaceMemory( strMemName, strTempMemName, outputMessage );

 } /* endif */

 return usURc;
} /* end of function NTMCloseOrganize */


// ================ Handle the message WM_EQF_MEMORGANIZE_PROCESS =====================

bool IsValidXml(std::wstring&& sentence);
VOID EQFMemOrganizeProcess
(
  PPROCESSCOMMAREA  pCommArea
)
{
  USHORT            usRc = TRUE;    // Return code to control a process
  USHORT            usDosRc = 0;    // Dos Returncode
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
    auto ll = T5Logger::GetInstance()->suppressLogging(); 
    wchar_t seg_buff[OTMPROPOSAL_MAXSEGLEN+1];
    pRIDA->pProposal->getSource(seg_buff, OTMPROPOSAL_MAXSEGLEN);

    bool fValidXml =  IsValidXml( seg_buff);

    if(fValidXml){
      pRIDA->pProposal->getTarget(seg_buff, OTMPROPOSAL_MAXSEGLEN);
      fValidXml =  IsValidXml( seg_buff);        
      T5Logger::GetInstance()->desuppressLogging(ll);
      if(!fValidXml){
        T5LOG(T5ERROR) << "skipping tu with invalid target segment: "<< *pRIDA->pProposal;
      } 
    }else{
      T5Logger::GetInstance()->desuppressLogging(ll);
      T5LOG(T5ERROR) << "skipping tu with invalid source segment: "<< *pRIDA->pProposal;
    }
    int sourceLen = pRIDA->pProposal->getSourceLen(),
      targetLen = pRIDA->pProposal->getTargetLen();

    if (!fValidXml || (sourceLen == 0) || (sourceLen >= OTMPROPOSAL_MAXSEGLEN) || (targetLen == 0) || (targetLen>=OTMPROPOSAL_MAXSEGLEN) ||
      (pRIDA->szTargetLanguage[0] == EOS) || (pRIDA->szTagTable[0] == EOS) )
    {
      // ignore invalid proposal
      pRIDA->pMem->importDetails->invalidSegments++;
      pRIDA->pMem->importDetails->firstInvalidSegmentsSegNums.push_back(std::make_tuple(pRIDA->pProposal->getSegmentNum(), -8));
    }
    else
    {
      // write proposal to output memory
      bool fFilterPassed = true;
      for(auto& filter: pRIDA->m_reorganizeFilters){
        if(filter.check(*pRIDA->pProposal)){
          fFilterPassed = false;
          break;
        }
      }
      if(fFilterPassed == false){
        pRIDA->pMem->importDetails->filteredSegments++;
      }else{
        pRIDA->pProposal->pInputSentence = new TMX_SENTENCE(pRIDA->pProposal->szSource, pRIDA->pProposal->szTarget);
        iRC = pRIDA->pMemTemp->putProposal( *(pRIDA->pProposal) );
        if ( iRC != 0 )
        {
          T5LOG(T5ERROR) << "segment in reorganize was skipped! segment: "<< *pRIDA->pProposal;
          pRIDA->pMem->importDetails->invalidSegments++;
          pRIDA->pMem->importDetails->firstInvalidSegmentsSegNums.push_back(std::make_tuple(pRIDA->pProposal->getSegmentNum(), iRC));
          pRIDA->pMem->importDetails->invalidSegmentsRCs[iRC] ++;
        }
        else
        {
          pRIDA->pMem->importDetails->segmentsImported++;
        } /* endif */             
      }
    } /* endif */
  }
  else if ( iRC == EqfMemory::ERROR_ENTRYISCORRUPTED )
  {
    pCommArea->usComplete = (USHORT)iProgress;
    pRIDA->pMem->importDetails->invalidSegments++;    
    pRIDA->pMem->importDetails->firstInvalidSegmentsSegNums.push_back(std::make_tuple(pRIDA->pProposal->getSegmentNum(), -9));    
    T5LOG(T5ERROR) << "Skipping proposal iRC == EqfMemory::ERROR_ENTRYISCORRUPTED ; proposal :\n" << *pRIDA->pProposal;
  }
  else if ( iRC == EqfMemory::INFO_ENDREACHED )
  {
    pRIDA->pMemTemp->NTMOrganizeIndexFile();
    T5LOG(T5WARNING) << "reorganize is done, segCount = " << pRIDA->pMem->importDetails->segmentsImported << "; invSegCount = " << pRIDA->pMem->importDetails->invalidSegments;
    int use_count = pRIDA->pMemTemp.use_count();
    if(use_count != 1){
      T5LOG(T5WARNING) << ":: use_count for temporary tm is not 1, but " << use_count;
    }
    //pRIDA->pMemTemp->TmBtree.fb.Flush();
    //pRIDA->pMemTemp->InBtree.fb.Flush();  
    pRIDA->pMem->TmBtree.fb.data = std::move(pRIDA->pMemTemp->TmBtree.fb.data);
    pRIDA->pMem->InBtree.fb.data = std::move(pRIDA->pMemTemp->InBtree.fb.data);

    //pRIDA->pMemTemp = nullptr;
    //TMManager::GetInstance()->closeMemory( pRIDA->pMemTemp.get() );

    if (usRc)
    {
      //T5LOG(T5FATAL) <<"UNIMPLEMENTED FUNCTION";
      //usDosRc = NTMCloseOrganize( pRIDA, FALSE );

      // set the progress indicator to 100 percent
      if ( !pRIDA->fBatch )
      {
        pCommArea->usComplete = 100;
      } /* endif */

      if ( usDosRc == NO_ERROR )
      {
        // everything in the termination process was ok
        //pRIDA->pMem = NULL;

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
    
    pRIDA->pMem->importDetails->usProgress = pCommArea->usComplete + 1;
    // -----------------------------------------------------
    // Issue message WM_EQF_MEMORGANIZE_END
    pRIDA->NextTask = MEM_END_ORGANIZE;
  }
  else
  {
    // Issue a message "Translation memory organize abnormally terminated."
    T5LOG(T5ERROR) << "Issue a message \"Translation memory organize abnormally terminated.\" for " << pRIDA->pMem->szName ;
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

  LONG lCurTime = 0;  
  time( &lCurTime );
                
  if ( pRIDA->pMem->importDetails->lReorganizeStartTime )
  {
    LONG lDiff = lCurTime - pRIDA->pMem->importDetails->lReorganizeStartTime;
    char buff[256];
    sprintf( buff, "Overall reorganize time is      : %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
            (lDiff - (lDiff / 3600 * 3600)) / 60,
            (lDiff - (lDiff / 3600 * 3600)) % 60 );
            
    pRIDA->pMem->importDetails->importTimestamp = buff;
  }
  //pRIDA->pMem->importDetails->usProgress = 100;
} /* end of function EQFMemOrganizeEnd */


