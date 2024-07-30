/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "../pluginmanager/PluginManager.h"
#include "LogWrapper.h"
#include "tm.h"
#include <cstring>
#include "EQF.H"

/*! \brief Data class for the transport of memory proposals
 *
 * 
 */

/*! \brief Prototypes of helper functions */
//int CopyToBufferW( const std::wstring &strSource, wchar_t *pszBuffer, int iBufSize );
//int CopyToBuffer( const std::string &strSource, char *pszBuffer, int iBufSize );
int CopyToBufferW(const wchar_t *pszSource, wchar_t *pszBuffer, int iBufSize );
int CopyToBuffer(const char *pszSource, char *pszBuffer, int iBufSize );

/*! \brief Constructors */
OtmProposal::OtmProposal() 
{
  this->clear();
  fFilled = 0;
};

/*! \brief Destructor */
OtmProposal::~OtmProposal() 
{   
if(pInputSentence) delete pInputSentence;
};

/* operations */

void OtmProposal::clear()//bool clearKeys)
{ 
  if(this->pInputSentence) delete pInputSentence;
  TMCursor currentKey, nextKey;
  //if(false == clearKeys){  }
  memset( this, 0, sizeof(OtmProposal) );
  eType = OtmProposal::eptUndefined;
  eMatch = OtmProposal::emtUndefined;
}

OtmProposal::eProposalType getMemProposalType( char *pszType )
{
  if ( strcasecmp( pszType, "GlobalMemory" ) == 0 )
  {
    return( OtmProposal::eptGlobalMemory );
  }
  else if ( strcasecmp( pszType, "GlobalMemoryStar" ) == 0 )
  {
    return( OtmProposal::eptGlobalMemoryStar );
  }
  else if ( strcasecmp( pszType, "MachineTranslation" ) == 0 )
  {
    return( OtmProposal::eptMachine );
  }
  else if ( strcasecmp( pszType, "Manual" ) == 0 )
  {
    return( OtmProposal::eptManual );
  } /* endif */
  return( OtmProposal::eptUndefined );
}

/* setters and getters */

/* \brief get the internal key of the proposal
   The internal key is the memory internal identifier of the proposal
   \param pszBuffer Pointer to buffer receiving the proposal key
   \param iBufSize Size of the buffer in number of characters
	 \returns Number of characters copied to pszBuffer including the terminating null character
*/
int OtmProposal::getInternalKey( char *pszBuffer, int iBufSize )
{
  return( CopyToBuffer( getInternalKey().c_str(), pszBuffer, iBufSize ) );
}


/* \brief get length of proposal source text 
  	\returns Number of characters in proposal source text
  */
int OtmProposal::getSourceLen()
{  
  return( wcslen( szSource ) );
}

/* \brief get proposal source text 
    \param pszBuffer Pointer to buffer receiving the proposal source text
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getSource( wchar_t *pszBuffer, int iBufSize )
{  
  return( CopyToBufferW( szSource, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal source text
    \param pszBuffer Pointer to buffer containing the proposal source text
  */
void OtmProposal::setSource( wchar_t *pszBuffer )
{
  size_t len = wcslen(pszBuffer);
  if(len > OTMPROPOSAL_MAXSEGLEN){
    T5LOG(T5ERROR) << "OtmProposal::setSource::Segment had been longer than 2048 bytes and would be skipped. Origina len =  " << len;
    len = 1; 
    pszBuffer[len] = L'\0';
  }
  wcsncpy( szSource, pszBuffer, len );
  fFilled = 1;
}
  	
/* \brief get length of proposal target text 
  	\returns Number of characters in proposal target text
  */
int OtmProposal::getTargetLen()
{  
  return( wcslen( szTarget ) );
}

/* \brief get proposal target text   
    \param pszBuffer Pointer to buffer receiving the proposal target text
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getTarget( wchar_t *pszBuffer, int iBufSize )
{  
  return( CopyToBufferW( szTarget, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal target text
    \param pszBuffer Pointer to buffer containing the proposal target text
  */
void OtmProposal::setTarget( wchar_t *pszBuffer )
{ 
  size_t len = wcslen(pszBuffer);

  if(len > OTMPROPOSAL_MAXSEGLEN){
    T5LOG(T5ERROR) << "OtmProposal::setTarget::Segment had been longer than 2048 bytes and would be skipped. Origina len = " << len;
    len = 1; 
    pszBuffer[len] = L'\0';
  }
  wcsncpy( szTarget, pszBuffer, len );
  fFilled = 1;
}
  	
/* \brief get proposal ID
    \param pszBuffer Pointer to buffer receiving the proposal ID
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getID( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szId, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal ID 
    \param pszBuffer Pointer to buffer containing the proposal ID
  */
void OtmProposal::setID( char *pszBuffer )
{  
  strncpy( szId, pszBuffer, sizeof(szId)-1 );
  fFilled = 1;
}
  	

/* \brief get proposal document name

  \param pszBuffer Pointer to buffer receiving the document name
  \param iBufSize Size of the buffer in number of characters
  \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getDocName( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szDocName, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal document short name
    \param pszBuffer Pointer to buffer containing the document name
  */
void OtmProposal::setDocName( char *pszBuffer )
{  
  strncpy( szDocName, pszBuffer, sizeof(szDocName)-1 );
  fFilled = 1;
}
  	



/* \brief get proposal segment number
  \returns proposal segment number
  */
long OtmProposal::getSegmentId()
{  
  return( lSegmentId );
}
  	
/* \brief set the proposal segment number
    \param lSegmentId new segment number of proposal
  */
void OtmProposal::setSegmentId( long lSegmentIdIn )
{  
  lSegmentId = lSegmentIdIn;
  fFilled = 1;
}

  	
/* \brief get proposal source language
    \param pszBuffer Pointer to buffer receiving the proposal source language
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getSourceLanguage( char *pszBuffer, int iBufSize )
{
  return( CopyToBuffer( szSourceLanguage, pszBuffer, iBufSize ) );
}

int OtmProposal::getOriginalSourceLanguage( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szOriginalSourceLanguage, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal source language
    \param pszBuffer Pointer to buffer containing the proposal source language
  */
void OtmProposal::setSourceLanguage( char *pszBuffer )
{  
  strncpy( szSourceLanguage, pszBuffer, sizeof(szSourceLanguage)-1 );
  fFilled = 1;
}
  
void OtmProposal::setOriginalSourceLanguage( char *pszBuffer )
{  
  strncpy( szOriginalSourceLanguage, pszBuffer, sizeof(szOriginalSourceLanguage)-1 );
  fFilled = 1;
}

/*
void OtmProposal::setKey(ULONG ulKey)
{
   
  ulKey = ulKey;
}

void OtmProposal::setTargetNum(USHORT usTargetNum)
{
   
  usTargetNum = usTargetNum;
}

ULONG OtmProposal::getKey()
{
  if ( this->pvProposalData == NULL ) return 0 ; 
  return ulKey ;
}

USHORT OtmProposal::getTargetNum()
{
  if ( this->pvProposalData == NULL ) return 0 ; 
  return usTargetNum ;
}//*/


/* \brief get proposal target language
    \param pszBuffer Pointer to buffer receiving the proposal target language
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getTargetLanguage( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szTargetLanguage, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal target language
    \param pszBuffer Pointer to buffer containing the proposal target language
  */
void OtmProposal::setTargetLanguage( char *pszBuffer )
{   
  strncpy( szTargetLanguage, pszBuffer, sizeof(szTargetLanguage)-1 );
  fFilled = 1;
}


/* \brief get proposal type
    \returns proposal type
  */
OtmProposal::eProposalType OtmProposal::getType()
{
  return( eType );
}
  	
/* \brief set the proposal type
    \param eType new type of the proposal
  */
void OtmProposal::setType( eProposalType eTypeIn )
{
  eType = eTypeIn;
  fFilled = 1;
}

/* \brief get match type
    \returns proposal type
  */
OtmProposal::eMatchType OtmProposal::getMatchType()
{
  return( eMatch );
}
  	
/* \brief set the match type
    \param eType new type of the proposal
  */
void OtmProposal::setMatchType( OtmProposal::eMatchType eMatchTypeIn )
{  
  eMatch = eMatchTypeIn;
  fFilled = 1;
}


/* \brief get name of proposal author
    \param pszBuffer Pointer to buffer receiving the name of the proposal author
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getAuthor( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szTargetAuthor, pszBuffer, iBufSize ) );
}
  	
/* \brief set the name of the proposal author
    \param pszBuffer Pointer to buffer containing the name of the proposal author
  */
void OtmProposal::setAuthor( char *pszBuffer )
{  
  strncpy( szTargetAuthor, pszBuffer, sizeof(szTargetAuthor)-1 );
  fFilled = 1;
}


/* \brief get proposal time stamp
    \returns proposal segment number
  */
long OtmProposal::getUpdateTime()
{  
  return( lTargetTime );
}
  	
/* \brief set the proposal time stamp
    \param lTime new time stamp of proposal
  */
void OtmProposal::setUpdateTime( long lTime )
{  
  lTargetTime = lTime;
  fFilled = 1;
}

/* \brief get proposal fuzziness
    \returns proposal fuzziness
  */
int OtmProposal::getFuzziness()
{  
  return( iFuzziness );
}

  	
/* \brief set the proposal fuzziness
    \param lFuzzinessTime new fuzziness of proposal
  */
void OtmProposal::setFuzziness( long iFuzzinessIn )
{
  iFuzziness = iFuzzinessIn;
  fFilled = 1;
}


 /* \brief get proposal diffs count from fuzzy calculations
     \returns proposal diffs count from fuzzy calculations
   */
  int OtmProposal::getDiffs(){
    return( iDiffs );
  }

  /* \brief set the proposal diffs count from fuzzy calculations
     \param iDiffs new diffs count from fuzzy calculations of proposal
   */
  void OtmProposal::setDiffs( long iDiffs ){
    iDiffs = iDiffs;
  }

    /* \brief get proposal words count during fuzzy calculations
     \returns proposal fuzziness
   */
  int OtmProposal::getWords(){
    return( iWords );
  }

  /* \brief set the proposal words count during fuzzy calculations
     \param iWords new words count during fuzzy calculations of proposal
   */
  void OtmProposal::setWords( long iWords ){      
    iWords = iWords;
  }

/* \brief get markup table name (format)
    \param pszBuffer Pointer to buffer receiving the name of the markup table name
    \param iBufSize Size of the buffer in number of characters
    \returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getMarkup( char *pszBuffer, int iBufSize )
{  
  return( CopyToBuffer( szMarkup, pszBuffer, iBufSize ) );
}
  	
/* \brief set markup table name (format)
    \param pszBuffer Pointer to buffer containing the markup table name
  */
void OtmProposal::setMarkup( char *pszBuffer )
{  
  strncpy( szMarkup, pszBuffer, sizeof(szMarkup)-1 );
  fFilled = 1;
}

#include <iostream>
#include <iomanip>
#include <sstream>
#include "EncodingHelper.h"

#define ALIGN std::setfill(' ') << std::setw(20) <<
std::ostream & operator<<( std::ostream & o, OtmProposal & proposal ){
  
  o << "|-----------------------------------------------------------------------------|\n";
  wchar_t wbuff [OTMPROPOSAL_MAXSEGLEN];
  char buff [OTMPROPOSAL_MAXSEGLEN];
  wbuff[0] = L'\0';
  buff[0] = '\0';  

  proposal.getSource(wbuff, OTMPROPOSAL_MAXSEGLEN);
  std::string src = EncodingHelper::convertToUTF8(wbuff);
  o << ALIGN "src: \"" << src << "\"\n";
  wbuff[0] = L'\0';
  
  proposal.getSourceLanguage(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "srcLang: \"" << buff << "\"\n";
  buff[0] = '\0';
  
  proposal.getTarget(wbuff, OTMPROPOSAL_MAXSEGLEN);
  std::string trg = EncodingHelper::convertToUTF8(wbuff);
  o <<ALIGN "trg: \"" << trg << "\"\n";
  wbuff[0] = L'\0';
  
  proposal.getTargetLanguage(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "trgLang: \"" << buff << "\"\n";
  buff[0] = '\0';
  
  proposal.getMarkup(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "markup: \"" << buff << "\"\n";
  buff[0] = '\0';
  
  proposal.getAuthor(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "author: \"" << buff << "\"\n";
  buff[0] = '\0';
  
  proposal.getAddInfo(wbuff, OTMPROPOSAL_MAXSEGLEN);
  std::string addInfo = EncodingHelper::convertToUTF8(wbuff);
  o <<ALIGN "add info: \"" << addInfo << "\"\n";
  wbuff[0] = L'\0';

  proposal.getContext(wbuff, OTMPROPOSAL_MAXSEGLEN);
  std::string contex = EncodingHelper::convertToUTF8(wbuff);
  o <<ALIGN "context: \"" << contex << "\"\n";
  wbuff[0] = L'\0';

  proposal.getDocName(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "doc name: \"" << buff << "\"\n";
  buff[0] = L'\0';

  //proposal.getDocShortName(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "short doc name: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  proposal.getID(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "id: \"" << buff << "\"\n";
  buff[0] = L'\0';


  proposal.getInternalKey(buff, OTMPROPOSAL_MAXSEGLEN);
  o <<ALIGN "internal key: \"" << buff << "\"\n";
  buff[0] = L'\0';

  long segNum = proposal.getSegmentId();
  o <<ALIGN "segment num: \"" << segNum << "\"\n";

  long memoryIndex = proposal.getMemoryIndex();
  o <<ALIGN "memory index: \"" << memoryIndex << "\"\n";
  buff[0] = L'\0';

  long updateTime = proposal.getUpdateTime();
  o <<ALIGN "update time: \"" << updateTime << "\"\n";
  buff[0] = L'\0';

  o << "|-----------------------------------------------------------------------------|\n\n";
  //proposal.getWords(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "words: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  //proposal.getType(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "type: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  //proposal.getMatchType(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "match type: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  //proposal.getFuzziness(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "fuzziness: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  //proposal.getContextRanking(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "context ranking: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  //proposal.getDiffs(buff, OTMPROPOSAL_MAXSEGLEN);
  //o <<ALIGN "diffs: \"" << buff << "\"\n";
  //buff[0] = L'\0';

  return o;
}

/* \brief get length of proposal context 
  	\returns Number of characters in proposal context
  */
int OtmProposal::getContextLen()
{
  return( wcslen( szContext ) );
}

/* \brief get proposal Context 
    \param pszBuffer Pointer to buffer receiving the proposal context
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getContext( wchar_t *pszBuffer, int iBufSize )
{
  return( CopyToBufferW( szContext, pszBuffer, iBufSize ) );
}
  	
/* \brief set the proposal context
    \param pszBuffer Pointer to buffer containing the proposal context
  */
void OtmProposal::setContext( wchar_t *pszBuffer )
{
  wcsncpy( szContext, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  fFilled = 1;
}

/* \brief get proposal context ranking
  	\returns proposal context ranking
  */
int OtmProposal::getContextRanking()
{
  return( iContextRanking );
}
  	
/* \brief set the proposal context ranking
     \param iContextRanking context ranking for the proposal (0..100)
  */
void OtmProposal::setContextRanking( int iContextRankingIn )
{
  iContextRanking = iContextRankingIn;
  fFilled = 1;
}

/* \brief get length of proposal AddInfo text 
  	\returns Number of characters in proposal AddInfo text
  */
int OtmProposal::getAddInfoLen()
{
  return( wcslen( szAddInfo ) );
}

/* \brief get additional info
    \param pszBuffer Pointer to buffer receiving the additional info
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
  */
int OtmProposal::getAddInfo( wchar_t *pszBuffer, int iBufSize )
{
  return( CopyToBufferW( szAddInfo, pszBuffer, iBufSize ) );
}


/* \brief set the proposal additional information
    \param pszBuffer Pointer to buffer containing the additional info
  */
void OtmProposal::setAddInfo( wchar_t *pszBuffer )
{ 
  wcsncpy( szAddInfo, pszBuffer, OTMPROPOSAL_MAXSEGLEN );
  fFilled = 1;
}


/* \brief set the proposal memory index
    \param iIndex new value for the memory index
  */
void OtmProposal::setMemoryIndex( int iIndex )
{
  iMemoryIndex = iIndex;
}

/* \brief get memory index
  	\returns memory index of this proposal
  */
int OtmProposal::getMemoryIndex()
{
  return( iMemoryIndex );
}


/* \brief set the replacement list
    \param pList new value for the replacement list
  */
void OtmProposal::setReplacementList( long pList )
{
  pvReplacementList = pList;
}

/* \brief get replacement list
  	\returns pointer to replacement list
  */
long OtmProposal::getReplacementList()
{
  return( (long)pvReplacementList );
}


/* \brief check if proposal match type is one of the exact match types
  */
bool OtmProposal::isExactMatch()
{
  return( (eType == eptManual) && ((eMatch == emtExact) || (eMatch == emtExactSameDoc) || (eMatch == emtExactExact)) );
}

/* \brief check if proposal is empty (i.e. has not been used)
  */
bool OtmProposal::isEmpty()
{
  return( !fFilled );
}


bool OtmProposal::isSourceLangIsPrefered(){
  return( fIsoSourceLangIsPrefered );
}

void OtmProposal::setIsSourceLangIsPrefered(bool fPref){
  fIsoSourceLangIsPrefered = fPref;
}


/* \brief check if source and target of proposal is equal
  */
bool OtmProposal::isSourceAndTargetEqual()
{ 
  return( wcscmp( szSource, szTarget ) == 0 );
}

/* \brief check if target strings of two proposals are identical
   \param otherProposal pointer to second proposal 
  */
bool OtmProposal::isSameTarget( OtmProposal *otherProposal )
{
  return( wcscmp( szTarget, otherProposal->szTarget ) == 0 );
}

/*! \brief Clear the data of proposals stored in a vector
  \param Proposals reference to a vector containing the proposals
*/
void OtmProposal::clearAllProposals(
  std::vector<OtmProposal> &Proposals
)
{
  for ( int i = 0; i < (int)Proposals.size(); i++ )
  {
    Proposals[i].clear();
  } /* endfor */     
}

/*! \brief Get the number of filled proposals in a proposal list
  \param Proposals reference to a vector containing the proposals
*/
int OtmProposal::getNumOfProposals(
  std::vector<OtmProposal> &Proposals
)
{
  int iFilled = 0;
  for ( int i = 0; i < (int)Proposals.size(); i++ )
  {
    if ( !Proposals[i].isEmpty() ) iFilled++;
  } /* endfor */  
  return( iFilled );
}




  /* \brief assignment operator to copy the datafields from one
    OtmProposal to another one
     \param copyme reference to OtmProposal being copied
   */
OtmProposal &OtmProposal::operator=( const OtmProposal &copyme )
{
  if (this != &copyme ) 
  {
    OtmProposal* pSource = (OtmProposal*)&copyme;
    OtmProposal* pTarget = (OtmProposal*)this;

    if ( (pSource != NULL) && (pTarget != NULL) && (pSource != pTarget) )
    {
      memcpy( pTarget, pSource, sizeof(OtmProposal) );
    }
  }
  return *this; 
}

/*! \brief Copies a string to the user supplied buffer area*/
int CopyToBufferW(const wchar_t *pszSource, wchar_t *pszBuffer, int iBufSize )
{
  int iCopied = wcslen( pszSource );
  if ( iCopied >= iBufSize ) iCopied = iBufSize - 1;
  //memcpy( pszBuffer, pszSource, iCopied * sizeof(wchar_t) );
  wcsncpy(pszBuffer, pszSource, iCopied);
  pszBuffer[iCopied] = 0;
  return( iCopied );
}

int CopyToBuffer(const char *pszSource, char *pszBuffer, int iBufSize )
{
  int iCopied = strlen( pszSource );
  if ( iCopied >= iBufSize ) iCopied = iBufSize - 1;
  memcpy( pszBuffer, pszSource, iCopied );
  pszBuffer[iCopied] = 0;
  return( iCopied );
}
 
