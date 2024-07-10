/*! \file

Description: Implementation of the abstract OtmMemory class


Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_DAM
//#define INCL_EQF_ANALYSIS         // analysis functions
//#include <eqf.h>                  // General Translation Manager include file
#include "tm.h"
#include "../../core/pluginmanager/PluginManager.h"
#include "../../core/utilities/LogWrapper.h"
#include "../../core/utilities/EncodingHelper.h"
#include "../../core/utilities/Property.h"
#include "../../core/utilities/FilesystemHelper.h"

#include <EQFQDAMI.H>             // Private header file of QDAM 
//#include "EQFRDICS.H"             // remote dictionary functions
#include "LanguageFactory.H"

// activate the folllowing define to activate logging
/*! \brief Prototypes of helper functions */
static int CopyToBuffer( char *pszSource, char *pszBuffer, int iBufSize );

EqfMemory::~EqfMemory()
{
  T5LOG(T5DEBUG) << "Closing memory " << szName;
  UnloadFromRAM();
  //free allocated memory
  NTMDestroyLongNameTable();
  //if ( this->pTmExtIn != NULL )  delete  this->pTmExtIn ;
  //if ( &TmExtOut != NULL ) delete  &TmExtOut ;
  //if ( TmPutIn != NULL )  delete  TmPutIn ;
  //if ( this->pTmPutOut != NULL ) delete  this->pTmPutOut ;
  //if ( this->pTmGetIn != NULL )  delete  this->pTmGetIn ;
  //if ( this->pTmGetOut != NULL ) delete  this->pTmGetOut;
}

/*! \brief Get number of markups used for the proposals in this mmoryProvides a part of the memory in binary format
  	\returns number of markups used by the memory proposals or 0 if no markup information can be provided
*/
int EqfMemory::getNumOfMarkupNames()
{
  return( (int)(TagTables.ulMaxEntries) );  
}

/*! \brief Get markup name at position n [n = 0.. GetNumOfMarkupNames()-1]
    \param iPos position of markup table name
    \param pszBuffer pointer to a buffer for the markup table name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfMemory::getMarkupName
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( (iPos >= 0) && (iPos < (int)(TagTables.ulMaxEntries)) )
  {
    return( CopyToBuffer( TagTables.stTmTableEntry[iPos].szName, pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}

/*! \brief Get source language of the memory
    \param pszBuffer pointer to a buffer for the source language name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfMemory::getSourceLanguage
(
  char *pszBuffer,
  int iSize
)
{
  return( CopyToBuffer( stTmSign.szSourceLanguage, pszBuffer, iSize ) );
}

/*! \brief Get description of the memory
    \param pszBuffer pointer to a buffer for the description
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfMemory::getDescription
(
  char *pszBuffer,
  int iSize
)
{
  return( CopyToBuffer(stTmSign.szDescription, pszBuffer, iSize ) );   
}

/*! \brief Set the description of the memory
  \param pszBuffer pointer to the description text
*/
void EqfMemory::setDescription
(
  char *pszBuffer
)
{
  //if ( )
  {
    BOOL fOK = TRUE;
    PTMX_SIGN pTmSign = new(TMX_SIGN);

    // get current signature record
    USHORT usSignLen = sizeof(TMX_SIGN);
    USHORT usRc = TmBtree.EQFNTMSign((PCHAR)pTmSign, &usSignLen );
    fOK = (usRc == NO_ERROR);

     // update description field
     if ( fOK )
     {
       strncpy( pTmSign->szDescription, pszBuffer, sizeof(pTmSign->szDescription)-1 );
       pTmSign->szDescription[sizeof(pTmSign->szDescription)-1] = EOS;

       //trigger plugin to set description
       //pMemoryPlugin->setDescription(szName,pTmSign->szDescription);
     } /* endif */

     // re-write signature record
     if ( fOK )
     {
       usRc = TmBtree.QDAMDictUpdSignLocal(pTmSign);
       fOK = (usRc == NO_ERROR);
      } /* endif */

      // free any allocated buffer
      delete ( pTmSign );
  } /* endif */     
  return;
}





/*! \brief Get the name of the memory
    \param pszBuffer pointer to a buffer for the name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
int EqfMemory::getName
(
  char *pszBuffer,
  int iSize
)
{
  return( CopyToBuffer( this->szName, pszBuffer, iSize ) );    
}

/*! \brief Get the name of the memory
    \param strName reference of a string receiving the memory name
*/
int EqfMemory::getName
(
  std::string &strName
)
{
  strName = this->szName;
  return( 0 );     
}


/*! \brief Get overall file size of this memory
  	\returns size of memory in number of bytes
*/
unsigned long EqfMemory::getFileSize()
{
  ULONG ulFileSize = 0;         // size of TM files

  //if ( pTmClb != NULL )
  {
    unsigned long ulDataSize  = FilesystemHelper::GetFileSize( TmBtree.fb.file );
    unsigned long ulIndexSize = FilesystemHelper::GetFileSize( InBtree.fb.file );

    ulFileSize = ulDataSize + ulIndexSize;
  }
  return( ulFileSize );
}



/*! \brief Store the supplied proposal in the memory
    When the proposal aready exists it will be overwritten with the supplied data

    \param pProposal pointer to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
int EqfMemory::putProposal
(
  OtmProposal &Proposal
)
{
  int iRC = 0;
  TMX_EXT_OUT_W TmPutOut;
  memset( &TmPutOut, 0, sizeof(TMX_EXT_OUT_W) );

  //OtmProposalToPutIn( Proposal, &TmPutIn );

  if(T5Logger::GetInstance()->CheckLogLevel(T5INFO)){
    std::string source = EncodingHelper::convertToUTF8(Proposal.pInputSentence->pStrings->getOriginalStrC());
    T5LOG( T5INFO) <<"EqfMemory::putProposal, source = " << source;
  }
  /********************************************************************/
  /* fill the TMX_PUT_IN prefix structure                             */
  /* the TMX_PUT_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  iRC = TmtXReplace ( Proposal, &TmPutOut );

  if ( iRC != 0 ){
    T5LOG(T5ERROR) <<  "EqfMemory::putProposal result = " << iRC; 
  }

  //if ( ( iRC == 0 ) &&
       //( TmPutIn.fMarkupChanged ) ) {
  //   iRC = SEG_RESET_BAD_MARKUP ;
  //}

  return( iRC );
}


/*! \brief Get the the first proposal from the memory and prepare sequential access
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  int EqfMemory::getFirstProposal
  (
    OtmProposal &Proposal
  )
  {
    return( this->getFirstProposal( Proposal, NULL ) );
  }

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
  int EqfMemory::getFirstProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  )
  {
    this->resetInternalCursor();

    return( this->getNextProposal( Proposal, piProgress ) );
  }

/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
int EqfMemory::getNextProposal
(
  OtmProposal &Proposal
)
{
 return( this->getNextProposal( Proposal, NULL ) );
}


int EqfMemory::RewriteCompactTable(){
  USHORT usRc = 0;
  if ( bCompactChanged )
   {
     usRc = TmBtree.EQFNTMUpdate( COMPACT_KEY,
                          bCompact, MAX_COMPACT_SIZE-1 );
     if ( !usRc )
     {
       bCompactChanged = FALSE;

       //usRc = TmBtree.EQFNTMIncrUpdCounter( COMPACTAREA_UPD_COUNTER,
       //                      &(alUpdCounter[COMPACTAREA_UPD_COUNTER]) );
     } /* endif */
   } /* endif */

   return usRc;
}


size_t EqfMemory::GetRAMSize()const{
  size_t size = sizeof(*this);
  size += TmBtree.fb.data.capacity();
  size += InBtree.fb.data.capacity();
  return size;
};

size_t EqfMemory::GetExpectedRAMSize()const{
  size_t size = sizeof(*this);
  size += TmBtree.GetFileSize();
  size += InBtree.GetFileSize();
  return size;
};

/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param pProposal pointer to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
int EqfMemory::getNextProposal
(
  OtmProposal &Proposal,
  int *piProgress
)
{
  int iRC = 0;
  TMX_EXT_IN_W TmExtIn;
  TMX_EXT_OUT_W TmExtOut;

  memset( &TmExtIn, 0, sizeof(TMX_EXT_IN_W) );
  memset( &TmExtOut, 0, sizeof(TMX_EXT_OUT_W) );

  Proposal.clear();
  TmExtIn.ulTmKey      = this->ulNextKey;
  TmExtIn.usNextTarget = this->usNextTarget;
  TmExtIn.usConvert    = MEM_OUTPUT_ASIS;

  iRC = (int)TmtXExtract( &TmExtIn,  &TmExtOut);

  if ( (iRC == 0) || (iRC == BTREE_CORRUPTED) )
  {
    if ( (piProgress != NULL) && (TmExtOut.ulMaxEntries != 0) )
    {
      *piProgress =  (int)(( TmExtIn.ulTmKey - FIRST_KEY) * 100) /  (int)TmExtOut.ulMaxEntries;
    } /* endif */     
  } /* endif */       

  if ( iRC == 0 )
  {
    ExtOutToOtmProposal( &TmExtOut, Proposal );

    // set current proposal internal key ,which is used in updateProposal
    Proposal.SetProposalKey(  TmExtIn.ulTmKey, TmExtIn.usNextTarget );
  } /* endif */       

  if ( (iRC == 0) || (iRC == BTREE_CORRUPTED) )
  {
    this->ulNextKey = TmExtOut.ulTmKey;
    this->usNextTarget = TmExtOut.usNextTarget;
  } /* endif */       

  if ( iRC == 0 )
  {
    // nothing to do
  }
  else if ( iRC == BTREE_EOF_REACHED )
  {
    iRC = INFO_ENDREACHED;
  }
  else
  {
    handleError( iRC, this->szName);
    if ( iRC == BTREE_CORRUPTED ) iRC = ERROR_ENTRYISCORRUPTED;
  }

  return( iRC );
}

  /*! \brief Get the current sequential access key (the key for the next proposal in the memory) 
  \param pszKeyBuffer pointer to the buffer to store the sequential access key
  \param iKeyBufferSize size of the key buffer in number of characters
  \returns 0 or error code in case of errors
*/
int EqfMemory::getSequentialAccessKey
(
  char *pszKeyBuffer,
  int  iKeyBufferSize
)
{
  int iRC = 0;
  char szKey[20];

  sprintf( szKey, "%lu:%u", this->ulNextKey, this->usNextTarget );
  if ( strlen(szKey)+1 <= iKeyBufferSize )
  {
    strcpy( pszKeyBuffer, szKey );
  }
  else
  {
    iRC = ERROR_BUFFERTOOSMALL;
  }
  return( iRC );
}

    
/*! \brief Set the current sequential access key to resume the sequential access at the given position
  \param pszKey a sequential access key previously returned by getSequentialAccessKey
  \returns 0 or error code in case of errors
*/
int EqfMemory::setSequentialAccessKey
(
  char *pszKey
)
{
  int iRC = this->SplitProposalKeyIntoRecordAndTarget( pszKey, &(this->ulNextKey), &(this->usNextTarget) );
  return( iRC );
}


/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
int EqfMemory::getProposal
(
  ulong recordKey,
  ushort targetKey,
  OtmProposal &Proposal
)
{
  int iRC = 0;
  TMX_EXT_IN_W TmExtIn;
  TMX_EXT_OUT_W TmExtOut;
  memset( &TmExtIn, 0, sizeof(TMX_EXT_IN_W) );
  memset( &TmExtOut, 0, sizeof(TMX_EXT_OUT_W) );
  Proposal.clear();

  TmExtIn.ulTmKey = recordKey;
  TmExtIn.usNextTarget = targetKey;
  TmExtIn.usConvert    = MEM_OUTPUT_ASIS;

  if ( iRC == 0 )
  {
    iRC = (int)TmtXExtract(  &TmExtIn,  &TmExtOut);
  } /* endif */     

  if ( iRC == 0 )
  {
    ExtOutToOtmProposal( &TmExtOut, Proposal );
    // set current proposal internal key ,which is used in updateProposal
    Proposal.SetProposalKey(  TmExtIn.ulTmKey, TmExtIn.usNextTarget );
    this->ulNextKey = TmExtOut.ulTmKey;
    this->usNextTarget = TmExtOut.usNextTarget;
  } /* endif */      

  if ( iRC != 0 ) handleError( iRC, this->szName );

  return( iRC );
}


/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
int EqfMemory::getProposal
(
  char *pszKey,
  OtmProposal &Proposal
)
{
  int iRC = 0;
  TMX_EXT_IN_W TmExtIn;
  TMX_EXT_OUT_W TmExtOut;
  memset( &TmExtIn, 0, sizeof(TMX_EXT_IN_W) );
  memset( &TmExtOut, 0, sizeof(TMX_EXT_OUT_W) );
  Proposal.clear();

  iRC = this->SplitProposalKeyIntoRecordAndTarget( pszKey, &( TmExtIn.ulTmKey), &( TmExtIn.usNextTarget) );
  TmExtIn.usConvert    = MEM_OUTPUT_ASIS;

  if ( iRC == 0 )
  {
    iRC = (int)TmtXExtract(  &TmExtIn,  &TmExtOut);
  } /* endif */     

  if ( iRC == 0 )
  {
    ExtOutToOtmProposal( &TmExtOut, Proposal );
    // set current proposal internal key ,which is used in updateProposal
    Proposal.SetProposalKey(  TmExtIn.ulTmKey, TmExtIn.usNextTarget );
    this->ulNextKey = TmExtOut.ulTmKey;
    this->usNextTarget = TmExtOut.usNextTarget;
  } /* endif */      

  if ( iRC != 0 ) handleError( iRC, this->szName );

  return( iRC );
}


/*! \brief Get a list of memory proposals matching the given search key

    This method uses the search data contained in the search key to find one or more
    matching proposals in the memory. At least the szSource and the szTargetLang members of the
    search key have to be filled by the caller.
    The caller provides a list of OtmProposals which will be filled with the data of the matching 
    proposals. The number of requested proposals is determined by the number
    of proposals in the list.

    \param SearchKey proposal containing search string and meta data
    \param FoundProposals refernce to vector with OtmProposal objects
    \param ulOptions options for the lookup

  	\returns 0 or error code in case of errors
*/
int EqfMemory::SearchProposal
(
  OtmProposal &SearchKey,
  std::vector<OtmProposal> &FoundProposals,
  unsigned long ulOptions
)
{
  return( 0 );
}




/*! \brief Get plugin responsible for this memory
  	\returns pointer to memory plugin object
*/
void *EqfMemory::getPlugin()
{
  return(EqfMemoryPlugin::GetInstance());
}



/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int EqfMemory::getLastError
(
  std::string &strError
)
{
  strError = this->strLastError;
  return( this->iLastError );
}

  /*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int EqfMemory::getLastError
(
  char *pszError,
  int iBufSize
)
{
  strError = strLastError.c_str();
  return( this->iLastError );
}

/*! \brief Get language at position n [n = 0.. GetNumOfLanguages()-1]
    \param iPos position of language
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
int EqfMemory::getLanguage
(
  int iPos,
  char *pszBuffer,
  int iSize
)
{
  if ( (iPos >= 0) && (iPos < (int)(Languages.ulMaxEntries)) )
  {
    return( CopyToBuffer( Languages.stTmTableEntry[iPos].szName, pszBuffer, iSize ) );
  }
  else
  {
    return( 0 );
  } /* end */     
}



/* private helper functions */

/*! \brief Build OtmProposal key from record number and target number
    \param ulKey key of record containing the proposal
    \param usTargetNum number of target within record 
    \param Proposal reference to the OtmProposal 
  	\returns 0 or error code in case of errors
*/
int OtmProposal::SetProposalKey
(
  ULONG   ulKey,
  USHORT  usTargetNum
)
{  
  recordKey = ulKey;
  targetKey = usTargetNum;
  return( 0 );
}

std::string OtmProposal::getInternalKey()const{
  return std::to_string( recordKey) + ":" + std::to_string(targetKey);
}

/*! \brief Split an internal key into record number and target number
    \param Proposal reference to the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int EqfMemory::SplitProposalKeyIntoRecordAndTarget
(
  OtmProposal &Proposal,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
)
{
  int iRC = 0;
  char szKey[20]= {0};
  Proposal.getInternalKey( szKey, sizeof(szKey) );
  return( this->SplitProposalKeyIntoRecordAndTarget( szKey, pulKey, pusTargetNum ) );
}

/*! \brief Split an internal key into record number and target number
    \param Proposal reference to the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int EqfMemory::SplitProposalKeyIntoRecordAndTarget
(
  char    *pszKey,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
)
{
  int iRC = 0;
  char *pszTarget = strchr( pszKey, ':' );
  if ( pszTarget != NULL )
  {
    *pszTarget = '\0';
    *pulKey = atol( pszKey );
    *pusTargetNum = (USHORT)atoi( pszTarget + 1 );
  }
  else
  {
    iRC = EqfMemory::ERROR_INTERNALKEY_MISSING;
  } /* end */     

  return( iRC );
}




/*! \brief Fill OtmProposal from TMX_GET_OUT_W structure
    \param pExtOut pointer to the TMX_GET_OUT_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int EqfMemory::ExtOutToOtmProposal
(
  TMX_EXT_OUT_W * pExtOut,
  OtmProposal &Proposal
)
{
  int iRC = 0;

  Proposal.clear();
  Proposal.setSource( pExtOut->szSource );
  Proposal.setTarget( pExtOut->szTarget );
  Proposal.setAuthor( pExtOut->szAuthorName );
  Proposal.setMarkup( pExtOut->szTagTable );
  Proposal.setTargetLanguage( pExtOut->szTargetLanguage );
  Proposal.setSourceLanguage( stTmSign.szSourceLanguage);
  Proposal.setAddInfo( pExtOut->szAddInfo );
  Proposal.setContext( pExtOut->szContext );
  Proposal.setDocName( pExtOut->szLongName );
  Proposal.setSegmentId( pExtOut->ulSourceSegmentId );
  Proposal.setType( FlagToProposalType( pExtOut->usTranslationFlag ) );
  Proposal.setUpdateTime( pExtOut->lTargetTime );
  Proposal.setContextRanking( 0 );
  Proposal.setOriginalSourceLanguage( pExtOut->szOriginalSourceLanguage );
  Proposal.recordKey = pExtOut->ulRecKey;
  Proposal.targetKey = pExtOut->usTargetKey;

  return( iRC );
}

/*! \brief Fill OtmProposal from TMX_MATCH_TABLE_W structure
    \param pMatch pointer to the TMX_MATCH_TABLE_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int EqfMemory::MatchToOtmProposal
(
  PTMX_MATCH_TABLE_W pMatch,
  OtmProposal *pProposal
)
{
  int iRC = 0;

  pProposal->clear();
  pProposal->setSource( pMatch->szSource );
  pProposal->setTarget( pMatch->szTarget );
  pProposal->setAuthor( pMatch->szTargetAuthor );
  pProposal->setMarkup( pMatch->szTagTable );
  pProposal->setTargetLanguage( pMatch->szTargetLanguage );
  pProposal->setSourceLanguage( stTmSign.szSourceLanguage);
  pProposal->setAddInfo( pMatch->szAddInfo );
  pProposal->setContext( pMatch->szContext );
  pProposal->setDocName( pMatch->szLongName );
  pProposal->setSegmentId( pMatch->ulSegmentId );
  pProposal->setType( FlagToProposalType( pMatch->usTranslationFlag ) );
  pProposal->setUpdateTime( pMatch->lTargetTime );
  pProposal->SetProposalKey( pMatch->ulKey, pMatch->usTargetNum );
  pProposal->setContextRanking( (int)pMatch->usContextRanking );
  pProposal->setMemoryIndex( pMatch->usDBIndex );
  pProposal->setWords( pMatch->iWords );
  pProposal->setDiffs( pMatch->iDiffs );
  pProposal->setOriginalSourceLanguage( pMatch->szOriginalSrcLanguage );
  pProposal->SetProposalKey(pMatch->ulKey, pMatch->usTargetNum);
  
  switch ( pMatch->usMatchLevel )
  {
    case 102: 
      pProposal->setFuzziness( 100 ); 
      pProposal->setMatchType( OtmProposal::emtExactExact ); 
      break;
    case 101: 
      pProposal->setFuzziness( 100 ); 
      pProposal->setMatchType( OtmProposal::emtExactSameDoc ); 
      break;
    case 100: 
      pProposal->setFuzziness( 100 ); 
      pProposal->setMatchType( OtmProposal::emtExact ); 
      break;
    case  99: 
      pProposal->setFuzziness( (pMatch->usTranslationFlag == TRANSLFLAG_MACHINE) ? 100 : 99 ); 
      pProposal->setMatchType( (pMatch->usTranslationFlag == TRANSLFLAG_MACHINE) ? OtmProposal::emtExact : OtmProposal::emtFuzzy );
      break;
    default: 
      pProposal->setFuzziness( pMatch->usMatchLevel); 
      pProposal->setMatchType( OtmProposal::emtFuzzy ); 
      break;
  } /* end */     

  return( iRC );
}

BYTE ProposalTypeToFlag(OtmProposal::eProposalType t){
  switch ( t )
  {
    case OtmProposal::eptManual: return TRANSLFLAG_NORMAL; break;
    case OtmProposal::eptMachine: return TRANSLFLAG_MACHINE; break;
    case OtmProposal::eptGlobalMemory: return TRANSLFLAG_GLOBMEM; break;
    case OtmProposal::eptGlobalMemoryStar: return TRANSLFLAG_GLOBMEMSTAR; break;
    default: return TRANSLFLAG_NORMAL; break;
  } 
}


/*! \brief Fill TMX_GET_W structure with OtmProposal data
    \param Proposal reference to the OtmProposal containing the data
    \param pGetIn pointer to the TMX_GET_W structure
  	\returns 0 or error code in case of errors
*/
int EqfMemory::OtmProposalToGetIn
(
  OtmProposal &Proposal,
  PTMX_GET_W pGetIn
)
{
  int iRC = 0;

  memset(  pGetIn, 0, sizeof(pGetIn) );
  Proposal.getSource( pGetIn->szSource, sizeof(pGetIn->szSource) );
  Proposal.getAuthor( pGetIn->szAuthorName, sizeof(pGetIn->szAuthorName) );
  Proposal.getMarkup( pGetIn->szTagTable, sizeof(pGetIn->szTagTable)  );
  Proposal.getSourceLanguage( pGetIn->szSourceLanguage, sizeof(pGetIn->szSourceLanguage)  );
  Proposal.getTargetLanguage( pGetIn->szTargetLanguage, sizeof(pGetIn->szTargetLanguage)  );
  Proposal.getAddInfo( pGetIn->szAddInfo, sizeof(pGetIn->szAddInfo)  );
  Proposal.getContext( pGetIn->szContext, sizeof(pGetIn->szContext)  );
  Proposal.getDocName( pGetIn->szLongName, sizeof(pGetIn->szLongName)  );
  Proposal.getDocName( pGetIn->szFileName, sizeof(pGetIn->szFileName)  );
  
  //pGetIn->usMatchThreshold = TM_DEFAULT_THRESHOLD;
  int threshold = TM_DEFAULT_THRESHOLD;
  //Properties::GetInstance()->get_value_or_default(KEY_TRIPLES_THRESHOLD, threshold, threshold);
  pGetIn->usMatchThreshold = threshold;
  pGetIn->ulSegmentId = Proposal.getSegmentId();
  pGetIn->pvReplacementList = (PVOID)Proposal.getReplacementList();
  //pGetIn->fSourceLangIsPrefered = Proposal.isSourceLangIsPrefered();
  pGetIn->fSourceLangIsPrefered = LanguageFactory::getInstance()->isPreferedLang( (pGetIn->szSourceLanguage) );
  pGetIn->fTargetLangIsPrefered = LanguageFactory::getInstance()->isPreferedLang( (pGetIn->szTargetLanguage) );
  return( iRC );
}


int CopyToBuffer( char *pszSource, char *pszBuffer, int iBufSize )
{
  int iCopied = 0;
  iBufSize--;  // leave room for string terminator
  while ( (iCopied < iBufSize) && (*pszSource != '\0') )
  {
    *pszBuffer++ = *pszSource++;
    iCopied++;
  } /* end */     
  *pszBuffer = '\0';
  return( iCopied );
}

OtmProposal::eProposalType FlagToProposalType( USHORT usTranslationFlag )
{
  if ( usTranslationFlag == TRANSLFLAG_NORMAL ) return( OtmProposal::eptManual );
  if ( usTranslationFlag == TRANSLFLAG_MACHINE ) return( OtmProposal::eptMachine );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEM ) return( OtmProposal::eptGlobalMemory );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEMSTAR ) return( OtmProposal::eptGlobalMemoryStar );
  return( OtmProposal::eptUndefined );
}

/*! \brief Handle a return code from the memory functions and create 
    the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name
  	\returns original or modified error return code
*/
int EqfMemory::handleError( int iRC, char *pszMemName, char *pszMarkup )
{
  return( EqfMemoryPlugin::handleError( iRC, pszMemName, pszMarkup, (PSZ)TmBtree.fb.fileName.c_str(), this->strLastError, this->iLastError ) );
}

/*! \brief Set or clear the pointer to a loaded global memory option file

    This method sets a pointer to a loaded global memory option file.
    When set the option file will be used to decide how global memory proposals will be processed.

    \param pvGlobalMemoryOptionsIn pointer to a loaded global memory option file or NULL to clear the current option file pointer

  	\returns 0 or error code in case of errors
*/
int EqfMemory::setGlobalMemoryOptions
(
    void *pvGlobalMemoryOptionsIn
)
{
  this->pvGlobalMemoryOptions = pvGlobalMemoryOptionsIn;
  return( 0 );
}



// NewTM region
EqfMemory::EqfMemory(const std::string& tmName): EqfMemory(){
  TmBtree.fb.fileName = FilesystemHelper::GetTmdPath(tmName);
  InBtree.fb.fileName = FilesystemHelper::GetTmiPath(tmName);
  strcpy(szName, tmName.c_str());
}

int EqfMemory::ReloadFromDisk(){
  int rc = 0;
  TmBtree.resetLookupTable();
  InBtree.resetLookupTable();
  rc = TmBtree.fb.ReadFromFile();
  rc = InBtree.fb.ReadFromFile();
  return rc;
}

int EqfMemory::FlushFilebuffers(){
  TmBtree.fb.Flush();
  InBtree.fb.Flush();
  return 0;
}


int EqfMemory::UnloadFromRAM(){
  //if(TmBtree.fb.file == nullptr){
    //file is in mem
  //  return -1;
  //}
  //if(InBtree.fb.file == nullptr){
    //file is in mem
  //  return -1;
  //}
  //TmBtree.fb.WriteToFile();
  //InBtree.fb.WriteToFile();
  
  //FlushFilebuffers();
  //TmBtree.fb.data.clear();
  //TmBtree.fb.data.shrink_to_fit();

  //InBtree.fb.data.clear();
  //InBtree.fb.data.shrink_to_fit();
  //fOpen = false;
  return 0;
}

//end of NewTM region 
