/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#define INCL_EQF_ASD     
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>

#include <EQFEVENT.H>             // event logging

#include <wctype.h>
#include "../utilities/LogWrapper.h"
#include "../utilities/Property.h"
#include "../utilities/EncodingHelper.h"
#include "../utilities/FilesystemHelper.h"

//static data
//distribution criteria for building tuples
static USHORT usTLookUp[24]= { 0,1,2, 0,1,3, 0,2,3,
                               0,0,1, 0,0,2, 0,1,1, 0,2,2, 0,0,0  };
typedef enum
 {
   REC_CLB,
   REC_SRCTAGTABLE,
   REC_TGTTAGTABLE,
   REC_NEXTTARGET,
   REC_TGTSTRING,
   REC_SRCSTRING,
   REC_FIRSTTARGET
 } RECORD_POSITION;

static
PBYTE NTRecPos(PBYTE pStart, int iType);

USHORT NTMAdjustAddDataInTgtCLB
	(
		PTMX_RECORD		   *ppTmRecordStart,
		PULONG     		   pulRecBufSize,
	  OtmProposal& 		   TmProposal,
		PTMX_TARGET_CLB    *ppClb,
		PTMX_RECORD        *ppCurTmRecord,
		PTMX_TARGET_RECORD	 *ppTMXTargetRecord,
	  PLONG  		 pulLeftClbLen,
 		PBOOL 			 pfUpdate);

USHORT TMLoopAndDelTargetClb
	(
		PTMX_RECORD         pTmRecord,
		OtmProposal& 	  TmProposal,
    USHORT              usPutLang,
    USHORT              usPutFile,
    USHORT              usPutAuthor,
    PBOOL               fNewerTargetExists,
    PINT                pTargetKey
	);
/**********************************************************************/
/* activate this define to get a faster memory                        */
/**********************************************************************/
#define NTMFAST_TOKENIZE 1

#ifdef NTMFAST_TOKENIZE

  /********************************************************************/
  /* get the substitute for the isAlNum function                      */
  /********************************************************************/
//  extern CHAR chIsText[];
  /**********************************************************************/
  /* activate optimization                                              */
  /**********************************************************************/
//  #pragma optimize("let", on )


// check for valid left length: ulLeft must be bigger or equal than ulRight
#define NTASSERTLEN(ulLeft, ulRight, iEvent)\
	if ( (ULONG)ulLeft < (ULONG)ulRight )                               \
	{                                                                   \
	  ERREVENT2( ADDTOTM_LOC, ERROR_EVENT, iEvent, TM_GROUP, "" );      \
	  UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );   \
	  abort();                                                          \
	}

bool isAddDataIsTheSame(OtmProposal& TmProposal,TMX_TARGET_CLB& Clb)
{ 
  wchar_t szAddInfo[OTMPROPOSAL_MAXSEGLEN+1];

  USHORT usAddDataLen = NTMComputeAddDataSize( TmProposal.szContext, TmProposal.szAddInfo );
  if(Clb.usAddDataLen != usAddDataLen)
    return false;

  if (Clb.usAddDataLen)
  {
    NtmGetAddData( &Clb, ADDDATA_CONTEXT_ID, szAddInfo, sizeof(szAddInfo) / sizeof(CHAR_W) );
    if(wcscmp(szAddInfo,TmProposal.szContext)) 
      return false;

    NtmGetAddData( &Clb, ADDDATA_ADDINFO_ID, szAddInfo, sizeof(szAddInfo) / sizeof(CHAR_W) );
    if(wcscmp(szAddInfo,TmProposal.szAddInfo)) 
      return false;
  } 
  return true;  
}

static
BOOL CheckForAlloc
(
   PTMX_SENTENCE      pSentence,
   PTMX_TERM_TOKEN  * ppTermTokens,
   USHORT             usEntries
);



//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TmtXReplace      adds data to the tm database             
//------------------------------------------------------------------------------
// Description:       Adds new data to database or modifies existing data       
//------------------------------------------------------------------------------
// Function call:  TmtXReplace( EqfMemory* pTmClb,    //ptr to ctl block struct   
//                             PTMX_PUT_IN pTmPutIn,  //ptr to input struct     
//                             PTMX_PUT_OUT pTmPutOut ) //ptr to output struct  
//------------------------------------------------------------------------------
// Input parameter:   EqfMemory*  pTmClb         control block                    
//                    PTMX_PUT_IN pTmPutIn     input structure                  
//------------------------------------------------------------------------------
// Output parameter:  PTMX_PUT_OUT pTmPutOut   output structure                 
//------------------------------------------------------------------------------
// Returncode type:                                                             
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:                                                                
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      get id of source language, call                                         
//        NTMGetIdFromName, returns pointer to id                               
//                                                                              
//      using parameters from the Tm_Put structure passed                       
//      call TokenizeSegment with szTagTable, szSource, SourceLangId,           
//        TagTable record, pNormString                                          
//                                                                              
//      call HashSentence with pNormString                                      
//                                                                              
//      work through ulong list of tuple hashes in sentence structure           
//        checking if on in compact area in TM control block                    
//      if all on                                                               
//        UpdateInTM( pstTmPut, pstSentence, pTmClb, pTagTable )                
//      else                                                                    
//        fill source record of tm_record (source string and tagtable struct)   
//        work through ulong hash list in pstSentence switching on              
//        respective bits  in compact area in pstTmClb                          
//        AddToTM( pszTmPut, pstTagTable, pstTmClb )                            
//        UpdateTmIndex( pstTmClb, ulHashList )                                 
//------------------------------------------------------------------------------
USHORT EqfMemory::TmtXReplace
(
  OtmProposal& TmProposal,  //ptr to input struct
  PTMX_EXT_OUT_W pTmPutOut //ptr to output struct
)
{
  ULONG      ulNewKey = 0;             // sid of newly added tm record
  BOOL       fOK = TRUE;               // success indicator
  USHORT     usRc = NO_ERROR;          // return code
  USHORT     usMatchesFound;           // compact area hits
  CHAR       szString[MAX_EQF_PATH];   // character string
  szString[0] = 0;
  BOOL        fLocked = FALSE;         // TM-database-has-been-locked flag
  BOOL         fUpdateOfIndexFailed = FALSE; // TRUE = update of index failed
  if(TmProposal.lSegmentId > 0)
  {
    if(TmProposal.lSegmentId >= stTmSign.segmentIndex - 1){
      stTmSign.segmentIndex = TmProposal.lSegmentId;
    }
    //keep id that was send
  }else{ // generate new id
    TmProposal.setSegmentId( ++stTmSign.segmentIndex);
  }

  if ( !usRc )
  {
    //build tag table path
    Properties::GetInstance()->get_value(KEY_OTM_DIR, szString, sizeof(szString));
    strcat( szString, "/TABLE/");
    strcat( szString, TmProposal.szMarkup );
    strcat( szString, EXT_OF_FORMAT );

    //tokenize source segment, resulting in norm. string and tag table record
    usRc = TokenizeSource( TmProposal.pInputSentence, szString,
                           TmProposal.szSourceLanguage);             
    
    if ( strstr( szString, "OTMUTF8" ) ) {
       strcpy( TmProposal.szMarkup, "OTMUTF8" );
       TmProposal.fMarkupChanged = TRUE ;
    }
  } /* endif */

  if ( !usRc )
  {
    HashSentence( TmProposal.pInputSentence );
    TmProposal.pInputSentence->pTagRecord[0].usTagTableId = 1;
    
    if(usRc){
      T5LOG( T5WARNING) <<  ":: NTMGetIDFromName( tagtable ) returned " << usRc;
    }
    
    
  } /* endif */

  if ( !usRc )
  {
    usMatchesFound = CheckCompactArea( TmProposal.pInputSentence, this );
    if ( usMatchesFound == TmProposal.pInputSentence->usActVote ) //all hash triples found
    {
      //update entry in tm database
      usRc = UpdateTmRecord(TmProposal);

      if(usRc == NO_ERROR)
      {
        //ulNewKey = TmProposal.recordKey;
        ulNewKey = TmProposal.currentInternalKey.getRecordKey();
      }
      //if no tm record fitted for update assume new and add to tm
      else if ( usRc == ERROR_ADD_TO_TM )
      {
        usRc = AddToTm(TmProposal, &ulNewKey );
        //update index
        if ( !usRc )
        {
          usRc = UpdateTmIndex( TmProposal.pInputSentence, ulNewKey );

          if ( usRc ) fUpdateOfIndexFailed = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      //add new tm record to tm database
      usRc = AddToTm(TmProposal, &ulNewKey );

      //update index
      if ( !usRc )
      {
        usRc = UpdateTmIndex( TmProposal.pInputSentence, ulNewKey );

        if ( usRc ) fUpdateOfIndexFailed = TRUE;
      } /* endif */
    } /* endif */
  } /* endif */

  if(!usRc){
    pTmPutOut->ulTmKey = TmProposal.currentInternalKey.getRecordKey();
    pTmPutOut->usTargetKey = TmProposal.currentInternalKey.getTargetKey();
    RewriteCompactTable();
    
    usRc = TmBtree.QDAMDictUpdSignLocal(&stTmSign);
  }

  pTmPutOut->stPrefixOut.usLengthOutput = sizeof( TMX_EXT_OUT_W );
  pTmPutOut->stPrefixOut.usTmtXRc = usRc;

  // special handling if update of index failed with BTREE_LOOKUPTABLE_TOO_SMALL:
  // delete the data record to ensure that any organize
  // of the TM can still be done without running into the
  // problem again
  if ( fUpdateOfIndexFailed && (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) )
  {
    // try to delete the segment from the memory

    PTMX_EXT_OUT_W pstDelOut = NULL;
    UtlAlloc( (PVOID *)&pstDelOut, 0L, sizeof (TMX_EXT_OUT_W), NOMSG );

    if ( pstDelOut )
    {
      TmtXDelSegm( TmProposal, pstDelOut );

      UtlAlloc( (PVOID *)&pstDelOut, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     HashSentence    Build and store hash value for terms      
//------------------------------------------------------------------------------
// Description:       Builds hash values for terms                              
//------------------------------------------------------------------------------
// Function call:     HashSentence( PTMX_SENTENCE pSentence ) //ptr sent struct 
//------------------------------------------------------------------------------
// Input parameter:   PTMX_SENTENCE pSentence     sentence structure            
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      build hash for each word in pNormString                                 
//        use adapted function in prototype                                     
//        HashTupel with pWord and length of word, returns filled out token     
//          record with offset, length and hash                                 
//                                                                              
//      build tuples out of these hash codes                                    
//        use function in prototype                                             
//        BuildVotes with token record of all words, return actual number of    
//         hash votes and ulong list of all the tuple hashes in sentence record 
//        (don't create all tuple combinations for a all tuples - rather work   
//          through all words and keep to MAX_VOTES)                            
//                                                                              
//------------------------------------------------------------------------------
DECLARE_bool(log_hashes_in_hash_sentence);
DECLARE_bool(add_tokens_to_fuzzy);
VOID HashSentence
(
  PTMX_SENTENCE pSentence         // pointer to sentence structure
)
{
  PSZ_W  pNormOffset;               // pointer to start of normalized string
  USHORT usCount = 0;               // counter


  //while ( pTermTokens->usLength )
  for(auto& token: pSentence->pTermTokens)
  {
    pNormOffset = pSentence->pStrings->getNormStrC() + token.usOffset;
    token.usHash = HashTupelW( pNormOffset, token.usLength );

    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP) || FLAGS_log_hashes_in_hash_sentence || FLAGS_add_tokens_to_fuzzy){
      auto str = EncodingHelper::convertToUTF8(pNormOffset).substr(0, token.usLength);
      pSentence->tokens.push_back(str);
      //if(FLAGS_log_hashes_in_hash_sentence){
        T5LOG( T5DEBUG) <<"HashSentence:: pNormOffset = \"" << str << "\"; len = " << token.usLength <<"; hash = " << token.usHash;
      //}
    }
    //max nr of hashes built
    usCount++;
    //pTermTokens++;

  } /* endwhile */

  /********************************************************************/
  /* if usCount < 4 add single tokens to allow matching simple        */
  /* sentences                                                        */
  /********************************************************************/
  if ( usCount < 5 )
  {
    PTMX_TERM_TOKEN pTermTokens = pSentence->pTermTokens.data();
    for (USHORT i=0; i < usCount; i++ )
    {
      //pSentence->pulVotes[pSentence->usActVote] = pTermTokens[i].usHash;
      pSentence->pulVotes.push_back(pTermTokens[i].usHash);
      pSentence->usActVote++;
      T5LOG(T5TRANSACTION) << "pSentence->pulVotes.size = " << pSentence->pulVotes.size() <<"; actVote = " << pSentence->usActVote;
      
    } /* endfor */
  } /* endif */

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
    auto str = EncodingHelper::convertToUTF8(pSentence->pStrings->getOriginalStrC());
    T5LOG( T5DEVELOP) <<"HashSentence:: inputString =\"" << str << "\"; count = " << usCount;
  }
  
  //build tuples of the term hashes
  if ( usCount >= 3 )
  {
    T5LOG( T5DEVELOP) <<"HashSentence:: Building Votes";
    BuildVotes( pSentence );
  } /* endif */
}



//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     HashTupel                                                 
//------------------------------------------------------------------------------
// Description:       hash the passed tupel into a hash number                  
//------------------------------------------------------------------------------
// Parameters:        PBYTE  pToken,         passed token                       
//                    USHORT usLen           length of tupel                    
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       hash value                                                
//------------------------------------------------------------------------------
// Function flow:     build hash value of passed token (be careful with DBCS).  
//------------------------------------------------------------------------------

// HashtTupelW for memory version 7.1 and above
USHORT HashTupelW
(
  PSZ_W  pToken,                       // passed token
  USHORT usLen                        // length of tupel
)
{
  USHORT usHash = 0;                   // hash value
  wchar_t c;                           // active character

  while ( usLen )
  {
    c = towlower( *pToken++ );
    if ( (c >= 'a') && (c <= 'z') )
    {
      usHash = (usHash * 131) + c-'a';
    }
    else
    {
      usHash = (usHash * 131) + c;
    } /* endif */
    usLen--;
  } /* endwhile */

  //ensure that usHash isn't zero
  if ( usHash == 0 )
  {
    usHash = 1;
  } /* endif */
  return (usHash);
} /* end of function HashTupel */


//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     BuildVotes                                                
//------------------------------------------------------------------------------
// Description:       build a histogram of possible matching sentences/phrases  
//                    and sort it in descending order                           
//------------------------------------------------------------------------------
// Parameters:        PTMX_SENTENCE pstSentence  pointer to sentence structure  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if not enough tokens (at least 3) we have to simulate     
//                    a creation of at least one triple.                        
//                    Call vote routine as long as there are tokens available.  
//                    Comments indicate which triples are build for a sentence  
//                    with 7 words A B C D E F G                                
//                    PREREQ: it is ensured that sentence has at least 3 tokens 
//------------------------------------------------------------------------------
VOID
BuildVotes
(
  PTMX_SENTENCE pSentence              // pointer to sentence structure
)
{
  PTMX_TERM_TOKEN  pTermTokens;        // pointer to active token
  USHORT           usIndex = 0;
  PTMX_TERM_TOKEN  pLastTerm;

  //run through list of tokens and build tuples
  pTermTokens = pSentence->pTermTokens.data();

  while ( pTermTokens->usLength && (pSentence->usActVote < ABS_VOTES) )
  {
    Vote( pTermTokens, pSentence, 0 );         // ABC, BCD, CDE, DEF, EFG
    pTermTokens++;
  } /* endwhile */
  pLastTerm = pTermTokens;
  pLastTerm --;

  pTermTokens = pSentence->pTermTokens.data();

  while ( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote( pTermTokens, pSentence, 1 );         // ABD, BCE, CDF, DEG
    pTermTokens++;
  } /* endwhile */
  pTermTokens = pSentence->pTermTokens.data();

  while ( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote( pTermTokens, pSentence, 2 );         // ACD, BDE, CEF, DFG
    pTermTokens++;
  } /* endwhile */

  pTermTokens = pSentence->pTermTokens.data();
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 4 );         // 1 * AAC
  } /* endif */


  pTermTokens  = pLastTerm - 2;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 6 );         // 1 * EGG
  } /* endif */
  /********************************************************************/
  /* use 1st token as triple                                          */
  /********************************************************************/
  pTermTokens = pSentence->pTermTokens.data();
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 7 );         // 1 * AAA
  } /* endif */

  /********************************************************************/
  /* use last token as triple to ensure even distribution of token    */
  /********************************************************************/
  pTermTokens = pLastTerm;
  if( pTermTokens->usLength && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 7 );         // 1 * GGG
  } /* endif */

  usIndex = 0;
  pTermTokens = pSentence->pTermTokens.data();
  while ((usIndex < 3) && pTermTokens->usLength
         && (pSentence->usActVote < MAX_VOTES) )
  {
    Vote (pTermTokens, pSentence, 3 );           // 3 * AAB
    if ((pLastTerm-1)->usLength
        && (pSentence->usActVote < MAX_VOTES ) )
    {
      Vote (pLastTerm-1, pSentence, 5 );         // 3 * FGG
    } /* endif */
    usIndex++;
  } /* endwhile */

} /* end of function BuildVotes */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     Vote                                                      
//------------------------------------------------------------------------------
// Description:       build the triples (3 out of 4)                            
//------------------------------------------------------------------------------
// Parameters:        PTMX_SENTENCE pstSentence ptr to sentence control struct  
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     build all possible 3 tupels out of a sequence of 4 consec 
//                    utive tokens. (base is the FLASH algorithm).              
//------------------------------------------------------------------------------
VOID
Vote
(
  PTMX_TERM_TOKEN pTermTokens,             //ptr to term tokens
  PTMX_SENTENCE pSentence,                 //pointer to sentence structure
  USHORT usTuple                           //tuple grouping
)
{
  ULONG  ulVote;                       //actual vote
  USHORT i;                            //index values
  BOOL   fGo = FALSE;

  // create all possible 3 tupels out of a sequence of 4 consecutive tokens
  //for dealing correctly with the tail...
  if ( usTuple == 0 )
  {
    fGo = (pTermTokens+2)->usLength;
  }
  else if ((usTuple == 2) || (usTuple == 1) )
  {
    fGo = (pTermTokens+3)->usLength;
  }
  else
  {
    fGo = TRUE;
  } /* endif */

  if ( fGo )
  {
    ulVote = 0;
    for ( i = 0; i < 3; i++ )
    {
      ulVote = (ulVote*131) + pTermTokens[usTLookUp[i+3*usTuple]].usHash;
    } /* endfor */
    
    pSentence->pulVotes.push_back(ulVote);

    //pSentence->pulVotes[pSentence->usActVote] = ulVote;
    pSentence->usActVote++;
    
    //#ifdef DEBUG
    T5LOG(T5TRANSACTION) << "pulVodet.size()=" << pSentence->pulVotes.size() << "; actVote = " << pSentence->usActVote;
    //#endif
  } /* endif */
} /* end of function Vote */

//------------------------------------------------------------------------------
// Internal function                                                            
//------------------------------------------------------------------------------
// Function name:     CheckCompactArea                                          
//------------------------------------------------------------------------------
// Description:       calculate what percentage of triples already in tm        
//------------------------------------------------------------------------------
// Parameters:    PTMX_SENTENCE pstSentence pointer to sentence control struct  
//                EqfMemory* pTmClb         pointer to tm control block           
//------------------------------------------------------------------------------
// Returncode type:   USHORT  number of bits in compact area on                 
//------------------------------------------------------------------------------
// Function flow:     calculate bit value of hash triple and check if set in    
//                    compact area in control block                             
//------------------------------------------------------------------------------
USHORT CheckCompactArea
(
  PTMX_SENTENCE pSentence,             // pointer to sentence structure
  EqfMemory*  pTmClb                     // pointer to tm control block
)
{                        
  PULONG pulVotes = pSentence->pulVotes.data(); // pointer to begin of votes                           
  USHORT usMatch = 0;                    // number of matches                           

  for (USHORT i = 0; ( i < pSentence->usActVote) ; i++, pulVotes++ )
  {
    ULONG ulVote = *pulVotes;                                // actual vote
    ulVote = ulVote % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
    
    BYTE bTuple = (BYTE) ulVote;                             // active byte
    BYTE bRest  = bTuple & 0x7;                              // relevant bit of byte

    PBYTE pByte = ((PBYTE)pTmClb->bCompact) + (ulVote >> 3); // byte pointer
    if ( *pByte & (1 << bRest) )
    {
      usMatch++;
    } /* endif */
  } /* endfor */
  return( usMatch );
}

#include <tm.h>
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TokenizeTarget      Split a string into tags and words    
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all tags and terms found.   
//------------------------------------------------------------------------------
// Function call:     TokenizeTarget( PSZ pString,  //ptr to target string      
//                            PSZ * ppNormString, //ptr to norm string          
//                            PTMX_TAGTABLE_RECORD * ppTagRecord, //tag record  
//                            PSZ pTagTableName   // name of tag table          
//------------------------------------------------------------------------------
// Input parameter:   PSZ pString             target string                     
//                    PSZ ptagTableName       name of target tag table          
//------------------------------------------------------------------------------
// Output parameter:      PSZ * ppNormString, //ptr to norm string              
//                        PTMX_TAGTABLE_RECORD * ppTagRecord, //tag record      
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR    function completed successfully               
//                    other       error code                                    
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//     allocate storage                                                         
//     tokenize string with correct tag table                                   
//     process outputed token list                                              
//     if tag                                                                   
//       remember position of tag and add to entry structure                    
//       add length of tag to structure                                         
//       add string to structure                                                
//     else if text                                                             
//       build normalized string                                                
//                                                                              
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     AddToTm             add a new tm record to tm database    
//------------------------------------------------------------------------------
// Description:       Add a new tm record to tm data file.                      
//------------------------------------------------------------------------------
// Function call:     AddToTm( pTmRecord,  // ptr to tm record PTMX_SENTENCE    
//                             pSentence,  // ptr to sentence structure         
//                             pTmClb,     // ptr to control block              
//                             pTmPut )    // ptr to put input structure        
//------------------------------------------------------------------------------
// Input parameter:   PTMX_RECORD pTmRecord                                     
//                    PTMX_SENTENCE pSentence                                   
//                    EqfMemory* pTmClb                                           
//                    PTMX_PUT pTmPut                                           
//                    PULONG pulNewKey                                          
//------------------------------------------------------------------------------
// Output parameter:                                                            
//                                                                              
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR    function completed successfully               
//                    other       error code                                    
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//      call TokenizeSegment with szTagTable, szTarget, usTargetLangId,         
//        pstTagTableRecord, pListOfWords                                       
//                                                                              
//      fill target control block structure with data from TM_PUT               
//      fill rest of target tm_record fields                                    
//                                                                              
//      add tm record to tm data file                                           
//------------------------------------------------------------------------------
USHORT EqfMemory::AddToTm
(
  OtmProposal& TmProposal,         // ptr to sentence structure
  PULONG pulNewKey                    // sid of newly added tm record
)
{
  PTMX_RECORD pTmRecord = NULL;           // ptr to tm record
  PTMX_TARGET_CLB pTargetClb = NULL;      // ptr to target ctl block
  USHORT usRc = NO_ERROR;                 // return code
  BOOL fOK;                               // success indicator
  USHORT usAddDataLen = 0;

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  //allocate target control block record
  if ( fOK )
  {
    usAddDataLen = NTMComputeAddDataSize( TmProposal.szContext, TmProposal.szAddInfo );

    fOK = UtlAlloc( (PVOID *) &pTargetClb, 0L, (LONG)(sizeof(TMX_TARGET_CLB)+usAddDataLen), NOMSG );
  } /* endif */

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {    
    if ( usRc == NO_ERROR )
    {
      usRc = FillClb( pTargetClb, TmProposal );
      if ( usRc == NO_ERROR )
      {
        USHORT usSrcLang = 0; 
        usRc = NTMGetIDFromName( TmProposal.szSourceLanguage, NULL, (USHORT)LANG_KEY, &usSrcLang );
        //fill tm record to add to database
        FillTmRecord ( TmProposal.pInputSentence,    // ptr to sentence struct for source info
                       pTmRecord,    // filled tm record returned
                       pTargetClb, usSrcLang );
        
        //add new tm record to database
        *pulNewKey = NTMREQUESTNEWKEY;
        
        usRc = TmBtree.EQFNTMInsert(//ptr to tm structure
                             pulNewKey,          //to be allocated in funct
                             (PBYTE)pTmRecord,   //pointer to tm record
                             pTmRecord->lRecordLen);     //length
        if(!usRc){
          TmProposal.currentInternalKey.setInternalKey(*pulNewKey, 1);
        }
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &(pTmRecord), 0L, 0L, NOMSG);
  UtlAlloc( (PVOID *) &(pTargetClb), 0L, 0L, NOMSG);

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillTmRecord        fill complete tm record               
//------------------------------------------------------------------------------
// Description:       Fill complete tm record                                   
//------------------------------------------------------------------------------
// Function call:     FillTmRecord( pSentence,                                  
//                                  pTagRecord,                                 
//                                  pNormString,                                
//                                  usNormLen,                                  
//                                  pTmRecord )                                 
//------------------------------------------------------------------------------
// Input parameter: PTMX_SENTENCE  pSentence //ptr to sent struct for source    
//             PTMX_TAGTABLE_RECORD pTagRecord //ptr to target string tag table 
//             PSZ pNormString        // ptr to target normalized string        
//             USHORT usNormLen       // length of target normalized string     
//------------------------------------------------------------------------------
// Output parameter: PTMX_RECORD pTmRecord    // filled tm record returned      
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Returncodes:       NONE                                                      
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//      fill tm record structure                                                
//------------------------------------------------------------------------------
VOID FillTmRecord
(
  PTMX_SENTENCE  pSentence,          // ptr to sentence struct for source info
  //PTMX_TAGTABLE_RECORD pTagRecord,   // ptr to target string tag table
  //PSZ_W pNormString,                 // ptr to target normalized string
  //USHORT usNormLen,                  // length of target normalized string
  PTMX_RECORD pTmRecord,             // filled tm record returned
  PTMX_TARGET_CLB pTargetClb,        // ptr to target control block
  USHORT usSrcLangId
)
{
  PTMX_SOURCE_RECORD pTMXSourceRecord;      //ptr to start of source structure
  PTMX_TARGET_RECORD pTMXTargetRecord;      //ptr to start of target structure
  PBYTE pTarget;                            //ptr to target record
  ULONG ulSrcLen = wcslen(pSentence->pStrings->getGenericTagStrC());
  ULONG ulTrgLen = wcslen(pSentence->pStrings->getGenericTargetStrC());

  //position source structure in tm record
  pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);
  pTMXSourceRecord->reset();
  //source structure offset in tm record
  pTmRecord->usSourceRecord = (USHORT)((PBYTE)pTMXSourceRecord - (PBYTE)pTmRecord);

  //set source string offset and copy source string
  pTMXSourceRecord->usSource = sizeof( TMX_SOURCE_RECORD );
//  memcpy( pTMXSourceRecord+1, pSentence->pNormString, pSentence->usNormLen *sizeof(CHAR_W));
//@@@
  //ulSrcNormLen = EQFUnicode2Compress( (PBYTE)(pTMXSourceRecord+1), pSentence->pNormString, ulSrcNormLen );
  ulSrcLen = EQFUnicode2Compress( (PBYTE)(pTMXSourceRecord+1), pSentence->pStrings->getGenericTagStrC(), ulSrcLen );
  pTMXSourceRecord->usLangId = usSrcLangId;
  //size of source record
  RECLEN(pTMXSourceRecord) = sizeof( TMX_SOURCE_RECORD ) + ulSrcLen;

  //position target structure in tm record
  pTarget = (PBYTE)(pTmRecord+1);
  pTarget += RECLEN(pTMXSourceRecord);

  //first target offset in tm record
  pTmRecord->usFirstTargetRecord = (USHORT)(pTarget - (PBYTE)pTmRecord);

  //position to start of target structure
  pTMXTargetRecord = ((PTMX_TARGET_RECORD)pTarget);
  pTMXTargetRecord->reset();
  pTMXTargetRecord->usTarget = sizeof(*pTMXTargetRecord);
  pTarget += pTMXTargetRecord->usTarget;


  //set source tag table offset
  //pTMXTargetRecord->usSourceTagTable = sizeof(TMX_TARGET_RECORD);

  //position pointer for source tag table
  //pTarget += pTMXTargetRecord->usSourceTagTable;

  //copy source tag table record to correct position
  //memcpy( pTarget, pSentence->pTagRecord, RECLEN(pSentence->pTagRecord) );

  //set target tag table start offset
  //pTMXTargetRecord->usTargetTagTable = (USHORT)(pTMXTargetRecord->usSourceTagTable +
  //                             RECLEN(pSentence->pTagRecord));
  //adjust target pointer for target tag table
  //pTarget += RECLEN(pSentence->pTagRecord);

  //copy target tag table record to correct position
  //memcpy( pTarget, pTagRecord, RECLEN(pTagRecord) );

  //set target string start offset
  //pTMXTargetRecord->usTarget = //(USHORT)(pTMXTargetRecord->usTargetTagTable +
                               //RECLEN(pTagRecord);
  //adjust target pointer for target string
  //pTarget += RECLEN(pTagRecord);

  //copy target string to correct position
//@@  memcpy( pTarget, pNormString, usNormLen );
  { 
    ulTrgLen = EQFUnicode2Compress( pTarget, pSentence->pStrings->getGenericTargetStrC(), ulTrgLen );
  }

  //set target string control block start offset
  pTMXTargetRecord->usClb = pTMXTargetRecord->usTarget + 
                              ulTrgLen;

  //adjust target pointer for control block
  pTarget += ulTrgLen;

  //copy target control block
  memcpy( pTarget, pTargetClb, TARGETCLBLEN(pTargetClb) );

  //size of target record
  RECLEN(pTMXTargetRecord) = sizeof(TMX_TARGET_RECORD) +
                                  //RECLEN(pSentence->pTagRecord) +
                                  //RECLEN(pTagRecord) +
                                  ulTrgLen +
                                  //ulSrcLen +
                                  TARGETCLBLEN(pTargetClb);
  //size of entire tm record
  RECLEN(pTmRecord) = sizeof( TMX_RECORD ) +
                           RECLEN(pTMXSourceRecord) +
                           RECLEN(pTMXTargetRecord);

  //initialize subsequent empty target record
  //position at end of target record
  pTarget +=  TARGETCLBLEN(pTargetClb);
  //position at next target record
  pTMXTargetRecord = (PTMX_TARGET_RECORD)pTarget;

  RECLEN(pTMXTargetRecord) = 0;
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillClb             fill target control block             
//------------------------------------------------------------------------------
// Description:       Fill target control block                                 
//------------------------------------------------------------------------------
// Function call:     FillClb( PTMX_TARGET_CLB * ppTargetClb,//ptr to ctl block 
//                             EqfMemory* pTmClb,  // ptr to tm ctl block         
//                             PTMX_PUT pTmPut ) // ptr to put input struct     
//------------------------------------------------------------------------------
// Input parameter:   PTMX_PUT pTmPut         put input structure               
//                    EqfMemory* pTmClb         tm control block                  
//------------------------------------------------------------------------------
// Output parameter:  PTM_TARGET_CLB * ppClb  //ptr to target control block     
//------------------------------------------------------------------------------
// Returncode type:   BOOL                                                      
//------------------------------------------------------------------------------
// Returncodes:       TRUE        function completed successfully               
//                    FALSE       error                                         
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//      get all necessary ids for strings                                       
//      allocate target control block                                           
//                                                                              
//      fill target control block structure with data from TM_PUT               
//      return ulKey                                                            
//------------------------------------------------------------------------------
USHORT EqfMemory::FillClb
(
  PTMX_TARGET_CLB  pTargetClb,    // ptr to target control block
  OtmProposal& TmProposal                 // ptr to put input structure
)
{
  USHORT  usTrgLang,usSrcLang;
  USHORT  usFile = 0;
  USHORT  usAuthor = 0;                 // ids
  USHORT  usRc = NO_ERROR;             // returned value

  //get id of target language, call
  usRc = NTMGetIDFromName( TmProposal.szTargetLanguage, NULL, (USHORT)LANG_KEY, &usTrgLang );

  //if ( !usRc )
  //{
  //  usRc = NTMGetIDFromName( pTmClb, TmProposal.szSourceLanguage, NULL, (USHORT)LANG_KEY, &usSrcLang );
  //} /* endif */

  //get id of file name, call
  if ( !usRc )
  {
    usRc = NTMGetIDFromName( TmProposal.szDocName, TmProposal.szDocName, (USHORT)FILE_KEY, &usFile );
  } /* endif */

  //get id of target author
  if ( !usRc )
  {
    usRc = NTMGetIDFromName( TmProposal.szTargetAuthor, NULL, (USHORT)AUTHOR_KEY, &usAuthor );
  } /* endif */

  if ( !usRc )
  {
    pTargetClb->usLangId = usTrgLang;
    //pTargetClb->usSrcLangId = usSrcLang;
    pTargetClb->bTranslationFlag = ProposalTypeToFlag(TmProposal.eType);
    //if a time is given take it else use current time
    if ( TmProposal.lTargetTime )
    {
      pTargetClb->lTime = TmProposal.lTargetTime;
    }
    else
    {
      UtlTime( &(pTargetClb->lTime) );
    } /* endif */
    pTargetClb->usFileId = usFile;
    pTargetClb->ulSegmId = TmProposal.getSegmentId();
    pTargetClb->usAuthorId = usAuthor;
    pTargetClb->usAddDataLen = 0;
    if ( TmProposal.szContext[0] ) 
      NtmStoreAddData( pTargetClb, ADDDATA_CONTEXT_ID, TmProposal.szContext );
    if ( TmProposal.szAddInfo[0] ) 
      NtmStoreAddData( pTargetClb, ADDDATA_ADDINFO_ID, TmProposal.szAddInfo );
  } /* endif */


  return (usRc);
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     UpdateTmIndex    check if index exists else add           
//------------------------------------------------------------------------------
// Description:       Checks if triple hash entry in index file exists and if   
//                    add the new sentence id else adds a new hash entry with   
//                    sentence id                                               
//+---------------------------------------------------------------------------- 
// Function call:     UpdateTmIndex( PTMX_SENTENCE  pSentence,//sent struct     
//                                   ULONG  ulSidKey,     //tm record key       
//                                   EqfMemory* pTmClb) //ptr to tm ctl block     
//------------------------------------------------------------------------------
// Input parameter: PTMX_SENTENCE pSentence                                     
//                  ULONG  ulSidKey                                             
//                  EqfMemory* pTmClb                                             
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//------------------------------------------------------------------------------
// Function flow:                                                               
//   get index record for first hash in Sentence structure                      
//     extract all sids                                                         
//     loop through remaining tuple hashes in pSentence as follows              
//       QDAMDictExactLongLocal with next hash key                              
//       extract all sids returned in pData                                     
//       check for matches, remembering matches until only one left or all      
//       hashes tried                                                           
//                                                                              
//     if match(es)                                                             
//       get tm record with remaining sid(s) from QDAM using                    
//       CheckTmRecord( pTmRecord(from QDAM call), pstTmPut, pstSentence,       
//                      pTmClb )                                                
//     else                                                                     
//      fill source record of tm_record (source string and tagtable structure)  
//      call AddToTm                                                            
//------------------------------------------------------------------------------

USHORT EqfMemory::UpdateTmIndex
(
  PTMX_SENTENCE  pSentence,            //pointer to sentence structure
  ULONG  ulSidKey                      //tm record key
)
{
  USHORT   usRc = 0;                   // return code
  ULONG    ulLen;                      // length paramter
  USHORT   usIndexEntries;             // nr of entries in index record
  PULONG   pulVotes = NULL;            // pointer to votes
  USHORT   i;                          // index in for loop
  ULONG    ulKey;                      // index key
  std::vector<TMX_INDEX_RECORD> pIndexRecord;  // pointer to index structure
  BOOL     fOK = FALSE;                // success indicator
  PBYTE    pIndex;                     // position pointer

  //for all votes add the index to the corresponding list
  pulVotes = pSentence->pulVotes.data();

  //allocate 32K for tm index record
  pIndexRecord.resize(1 + TMX_REC_SIZE/sizeof(TMX_INDEX_RECORD));
  fOK = UtlAlloc( (PVOID *) &(pIndexRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    for ( i = 0; i < pSentence->usActVote; i++, pulVotes++ )
    {
      if ( usRc == NO_ERROR )
      {
        ulKey = (*pulVotes) & START_KEY;
        ulLen = TMX_REC_SIZE;
        memset( pIndexRecord.data(), 0, TMX_REC_SIZE );
        usRc = InBtree.EQFNTMGet( ulKey,  //index key
                          (PCHAR)pIndexRecord.data(),   //pointer to index record
                          &ulLen );  //length

        if ( usRc == BTREE_NOT_FOUND )
        {
          //key is not in index file; add a new index entry
          pIndexRecord.data()->usRecordLen = sizeof( TMX_INDEX_RECORD );

          pIndexRecord.data()->stIndexEntry = NTMINDEX(pSentence->usActVote,ulSidKey);

          usRc = InBtree.EQFNTMInsert( &ulKey,
                               (PBYTE)pIndexRecord.data(),  //pointer to index
                               pIndexRecord.data()->usRecordLen );  //length

          // if index DB is full and memory is in exclusive access we try to compact the index file
          if ( (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) && (usAccessMode & ASD_LOCKED) )
          {
             usRc = EQFNTMOrganizeIndex( &(InBtree), usAccessMode, START_KEY );

             if ( usRc == NO_ERROR )
             {
               usRc = InBtree.EQFNTMInsert(&ulKey, (PBYTE)pIndexRecord.data(), pIndexRecord.data()->usRecordLen );
             } /* endif */
          } /* endif */

          if ( !usRc )
          {
            //add the match(tuple) to compact area
            ulKey = *pulVotes % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
            *((PBYTE)bCompact + (ulKey >> 3)) |=
                                  1 << ((BYTE)ulKey & 0x07);
//                                  1 << (USHORT)(ulKey & 0x07);       @01M
            bCompactChanged = TRUE;
          } /* endif */
        }
        else
        {
          if ( usRc == NO_ERROR )
          {
            BOOL fFound = FALSE;

            //key is in index file; update index entry with new sid
            ulLen = pIndexRecord.data()->usRecordLen;

            //calculate number of entries in index record
            usIndexEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));
  
            //// check if SID is already contained in list..
            //{
            //  int i = (int)usIndexEntries;
            //  PULONG pulIndex = (PULONG)&(pIndexRecord.data()->stIndexEntry); 
            //  while ( i )
            //  {
            //    if ( *pulIndex == ulNewSID )
            //    {
            //      fFound = TRUE;
            //      break;
            //    } /* endif */
            //    pulIndex++;
            //    i--;
            //  } /*endwhile */
            //}

            if ( !fFound )
            {
              if ( usIndexEntries >= (MAX_INDEX_LEN -1))
              {
                //position pointer at beginning of index record
                pIndex = (PBYTE)pIndexRecord.data();
                memmove( pIndex, pIndex + sizeof(ULONG), ulLen - sizeof(ULONG) );
                ulLen -= sizeof(ULONG);
                usIndexEntries--;
              }
              //only update index file if index record is not too large
              if ( (usIndexEntries < (MAX_INDEX_LEN - 1)) 
                    && ((ulLen + sizeof( TMX_INDEX_ENTRY )) <= TMX_REC_SIZE) )
              {
                //position pointer at beginning of index record
                pIndex = (PBYTE)pIndexRecord.data();

                //move pointer to end of index record
                pIndex += ulLen;

                // fill in new index entry
                *((PTMX_INDEX_ENTRY) pIndex ) =
                                    NTMINDEX(pSentence->usActVote,ulSidKey);

                //update index record size
                pIndexRecord.data()->usRecordLen = (USHORT)(ulLen + sizeof( TMX_INDEX_ENTRY ));

                usRc = InBtree.EQFNTMUpdate( 
                                    ulKey,
                                    (PBYTE)pIndexRecord.data(),  //pointer to index
                                    pIndexRecord.data()->usRecordLen );  //length
                // if index DB is full and memory is in exclusive access we try to compact the index file
                if ( (usRc == BTREE_LOOKUPTABLE_TOO_SMALL) && (usAccessMode & ASD_LOCKED) )
                {
                  usRc = EQFNTMOrganizeIndex( &(InBtree), usAccessMode, START_KEY );

                  if ( usRc == NO_ERROR )
                  {
                    usRc = InBtree.EQFNTMUpdate(ulKey, (PBYTE)pIndexRecord.data(), pIndexRecord.data()->usRecordLen ); 
                  } /* endif */
                } /* endif */
                if ( !usRc )
                {
                  //add the match(tuple) to compact area
                  ulKey = *pulVotes % ((LONG)(MAX_COMPACT_SIZE-1) * 8);
                  *((PBYTE)bCompact + (ulKey >> 3)) |=
                                        1 << ((BYTE)ulKey & 0x07);
                  bCompactChanged = TRUE;
                } /* endif */
              }
              else
              {
                usIndexEntries = usIndexEntries;

              }  /* endif */
            } /* endif */

          } /* endif */
        } /* endif */
      }
      else
      {
        //error so leave for loop
        i = pSentence->usActVote;
      } /* endif */
    } /* endfor */
  } /* endif */


  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     DetermineTmRecord outputs all possible sids               
//------------------------------------------------------------------------------
// Description:       This function returns a list of all legitimate sentence   
//                    keys                                                      
//+---------------------------------------------------------------------------- 
// Function call:  DetermineTmRecord( EqfMemory* pTmClb,                          
//                                    PTMX_SENTENCE pSentence,                  
//                                    PULONG pulSids )                          
//------------------------------------------------------------------------------
// Input parameter: EqfMemory* pTmClb                                             
//                  PTMX_SENTENCE pSentence                                     
//------------------------------------------------------------------------------
// Output parameter:  PULONG pulSids                                            
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   get index record for first triple                                          
//     get list of all sids in this index record                                
//     loop through all triple hashes built for source string                   
//       and determine valid sids eliminating those from list that no longer    
//       hold                                                                   
//------------------------------------------------------------------------------
#ifdef _DEBUG
  static BOOL  fSidLog = FALSE;
#endif

USHORT DetermineTmRecord
(
  EqfMemory* pTmClb,                   // ptr to tm control block
  PTMX_SENTENCE pSentence,           // ptr to sentence structure
  PULONG pulSids                     // ptr to tm record
)
{
  USHORT usRc = NO_ERROR;            // return code
  ULONG  ulLen = 0;                  // length paramter
  USHORT usSid = 0;                  // number of sentence ids found
  USHORT usPos;                      // position in pulSids
  PULONG pulVotes;                   // pointer to votes
  PULONG pulSidStart;                // po ter to votes
  USHORT i, j;                       // index in for loop
  USHORT usMaxEntries = 0;           // nr of index entries in index record
  ULONG ulKey;                       // index key
  std::vector<BYTE> pIndexRecord; // pointer to index structure
  BOOL fOk = true;                          // success indicator
  PTMX_INDEX_ENTRY pIndexEntry;      // pointer to index entry structure

  pulSidStart = pulSids;

  //for all votes add the index to the corresponding list
  pulVotes = pSentence->pulVotes.data();

  //allocate 32K for tm index record
  pIndexRecord.resize(TMX_REC_SIZE);


  ulKey = (*pulVotes) & START_KEY;
  memset( pIndexRecord.data(), 0, pIndexRecord.size() );
  usRc = pTmClb->InBtree.EQFNTMGet( ulKey,  //index key
                    pIndexRecord);   //pointer to index record

  if ( usRc == NO_ERROR )
  {
    //calculate number of index entries in index record
    ulLen = toIndexRecord(pIndexRecord)->usRecordLen;
    usMaxEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
      std::string msg = __func__ + std::string(":: Number Entries:  ") + toStr(usMaxEntries).c_str() +"; Entries:";
      pIndexEntry = &toIndexRecord(pIndexRecord)->stIndexEntry;
      for (j=0 ; j<usMaxEntries;j++,pIndexEntry++ )
      {
        if (j % 10 == 0)
        {
          msg+="\n";
        } /* endif */
        msg +=  std::to_string(NTMKEY(*pIndexEntry)) + "; " ;
      } /* endfor */
      T5LOG( T5DEBUG) << msg;
    }

    pIndexEntry = &toIndexRecord(pIndexRecord)->stIndexEntry;

    for ( j = 0; j < usMaxEntries; j++, pIndexEntry++ )
    {
      if ( NTMVOTES(*pIndexEntry) == (BYTE) pSentence->usActVote )
      {
        *pulSids = NTMKEY(*pIndexEntry);
        usSid++;
        pulSids++;
      } /* endif */
    } /* endfor */

    if ( (usSid > 1) )
    {
      //there is more than one sentence to check so go through other
      //indices until all have been tried or usSid has been reduced to 1
      pulVotes++;
      for ( i = 0; (i < pSentence->usActVote-1) && (usSid > 1);
            i++, pulVotes++ )
      {
        if ( usRc == NO_ERROR )
        {
          ulKey = (*pulVotes) & START_KEY;
          memset( pIndexRecord.data(), 0, pIndexRecord.size() );
          usRc = pTmClb->InBtree.EQFNTMGet( ulKey,  //index key
                            pIndexRecord);  //pointer to index record

          if ( usRc == NO_ERROR )
          {
            //calculate number of index entries in index record
            ulLen = toIndexRecord(pIndexRecord)->usRecordLen;
            usMaxEntries = (USHORT)((pIndexRecord.size() - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

            pIndexEntry = &toIndexRecord(pIndexRecord)->stIndexEntry;
            pulSids = pulSidStart;
            usPos = 0;

            //end criteria are all sentence ids in index key or only one
            //sentence id left in pulSids
            for ( j = 0; (j < usMaxEntries) && (usSid > 1);
                  j++, pIndexEntry++ )
            {
              if ( NTMVOTES(*pIndexEntry) == (BYTE) pSentence->usActVote )
              {
                //before adding sentence id check if already in pulsids as the
                //respective tm record need only be checked once
                while ( (NTMKEY(*pIndexEntry) > *pulSids) && (usSid > 1) )
                {
                  //remove sid from pulSids and decrease sid counter
                  if ( usSid > usPos )
                  {
                    ulLen = (usSid - usPos) * sizeof(ULONG);
                  }
                  else
                  {
                    ulLen = sizeof(ULONG);
                  } /* endif */
                  memmove( (PBYTE) pulSids, (PBYTE)(pulSids+1), ulLen );
                  usSid--;
                } /* endwhile */

                if ( NTMKEY(*pIndexEntry) == *pulSids )
                {
                  //move on one position in pulSids
                  pulSids++;
                  usPos++;
                } /* endif */
              } /* endif */
            } /* endfor */
          }
          else
          {
            //in case the index record doesn't exist exit function and
            //set return code to 0
            if ( usRc == BTREE_NOT_FOUND )
            {
              LOG_AND_SET_RC(usRc, T5DEVELOP, NO_ERROR);
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endfor */
    } /* endif */
  }
  else
  {
    //in case the index record doesn't exist exit function and set return
    //code to 0
    if ( usRc == BTREE_NOT_FOUND )
    {
      LOG_AND_SET_RC(usRc, T5DEVELOP, NO_ERROR);
    } /* endif */
  } /* endif */


  if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
    std::string msg = __func__ + std::string(":: Matching Sids: ");
    pIndexEntry = &toIndexRecord(pIndexRecord)->stIndexEntry;
    while ( *pulSidStart )
    {
      msg +=  std::to_string(*pulSidStart) + "; ";
      pulSidStart++;
    } 
    T5LOG( T5DEBUG) << msg;
  }

  if ( usRc )
  {
    ERREVENT2( DETERMINETMRECORD_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  return( usRc );
}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     UpdateTmRecord                                            
//------------------------------------------------------------------------------
// Description:       This function either adds a new target, sets the          
//                    multiple flag to true or replaces an existing target      
//                    record                                                    
//+---------------------------------------------------------------------------- 
// Function call:     UpdateTmRecord( EqfMemory* pTmClb,                          
//                                    PTMX_PUT pTmPut,                          
//                                    PTMX_SENTENCE pSentence                   
//------------------------------------------------------------------------------
// Input parameter: EqfMemory*                                                    
//                  PTMX_PUT                                                    
//                  PTMX_SENTENCE                                               
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   determine all valid sentence keys for source string                        
//     for all returned sentence keys                                           
//       get tm record                                                          
//       call function with put criteria                                        
//       if record putted stop processing                                       
//       else try next sentence key                                             
//------------------------------------------------------------------------------

USHORT EqfMemory::UpdateTmRecord
(
  OtmProposal&    TmProposal                //pointer to get in data
)
{
  BOOL   fOK = true;                   //success indicator
  BOOL   fStop = FALSE;                //indication to leave while loop
  PULONG pulSids = NULL;               //ptr to sentence ids
  PULONG pulSidStart = NULL;           //ptr to sentence ids
  USHORT usRc = NO_ERROR;              //return code
  ULONG  ulLen;                        //length indicator
  ULONG ulKey;                         //tm record key
  std::vector<BYTE> TmRecord;        //pointer to tm record
  
  TmRecord.resize(TMX_REC_SIZE);


  //allocate for sentence ids
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pulSids), 0L,
                    (LONG)((MAX_INDEX_LEN + 5) * sizeof(ULONG)),
                    NOMSG );
  } /* endif */

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    pulSidStart = pulSids;
    usRc = DetermineTmRecord( this, TmProposal.pInputSentence, pulSids );
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (*pulSids) && (usRc == NO_ERROR) && !fStop )
      {
        ulKey = *pulSids;
        usRc =  TmBtree.EQFNTMGet( ulKey,  //tm record key
                          TmRecord);   //pointer to tm record data

        if ( usRc == NO_ERROR )
        {
          //compare tm record data with data passed in the get in structure
          usRc = ComparePutData( TmRecord, TmProposal, &ulKey );

          if ( usRc == SOURCE_STRING_ERROR )
          {
            //get next tm record
            pulSids++;
            LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);
          }
          else if ( usRc == NO_ERROR )
          {
            //TmProposal.recordKey = ulKey;
            //new target record was added or an existing one was successfully
            //replaced so don't try other sids
            fStop = TRUE;
          } /* endif */
        } /* endif */
      } /* endwhile */

      if ( !(*pulSids) && !fStop )
      {
        //issue message that tm needs to be organized if pulsid is empty and
        //no get was successful
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_ADD_TO_TM);
      } /* endif */
    }  /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  return( usRc );
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     UpdateTmRecord                                            
//------------------------------------------------------------------------------
// Description:       This function either adds a new target, sets the          
//                    multiple flag to true or replaces an existing target      
//                    record                                                    
//+---------------------------------------------------------------------------- 
// Function call:     UpdateTmRecord( EqfMemory* pTmClb,                          
//                                    PTMX_PUT pTmPut,                          
//                                    PTMX_SENTENCE pSentence                   
//------------------------------------------------------------------------------
// Input parameter: EqfMemory*                                                    
//                  PTMX_PUT                                                    
//                  PTMX_SENTENCE                                               
//------------------------------------------------------------------------------
// Output parameter:                                                            
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   determine all valid sentence keys for source string                        
//     for all returned sentence keys                                           
//       get tm record                                                          
//       call function with put criteria                                        
//       if record putted stop processing                                       
//       else try next sentence key                                             
//------------------------------------------------------------------------------

USHORT EqfMemory::UpdateTmRecordByInternalKey
(
  OtmProposal& TmProposal
)
{
  BOOL   fOK = true;                          //success indicator
  BOOL   fStop = FALSE;                //indication to leave while loop
  PULONG pulSids = NULL;               //ptr to sentence ids
  PULONG pulSidStart = NULL;           //ptr to sentence ids
  USHORT usRc = NO_ERROR;              //return code
  ULONG  ulLen;                        //length indicator
  ULONG ulKey;                         //tm record key
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record

  std::vector<BYTE> TmRecord;         // space for user data
  TmRecord.resize(TMX_REC_SIZE);

  //allocate for sentence ids
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pulSids), 0L,
                    (LONG)((MAX_INDEX_LEN + 5) * sizeof(ULONG)),
                    NOMSG );
  } /* endif */

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    pulSidStart = pulSids;
    usRc = DetermineTmRecord( this, TmProposal.pInputSentence, pulSids );
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (*pulSids) && (usRc == NO_ERROR) && !fStop )
      {
        ulKey = *pulSids;
        memset( &TmRecord, 0, TmRecord.size() );
        usRc =  TmBtree.EQFNTMGet(ulKey,  //tm record key
                          TmRecord);   //pointer to tm record data
        

        if ( usRc == NO_ERROR )
        {
          //compare tm record data with data passed in the get in structure
          usRc = ComparePutData( TmRecord, TmProposal, &ulKey );

          if ( usRc == SOURCE_STRING_ERROR )
          {
            //get next tm record
            pulSids++;
            LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);
          }
          else if ( usRc == NO_ERROR )
          {
            //new target record was added or an existing one was successfully
            //replaced so don't try other sids
            fStop = TRUE;
          } /* endif */
        } /* endif */
      } /* endwhile */

      if ( !(*pulSids) && !fStop )
      {
        //issue message that tm needs to be organized if pulsid is empty and
        //no get was successful
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_ADD_TO_TM);
      } /* endif */
    }  /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  return( usRc );
}

DECLARE_bool(ignore_newer_target_exists_check);
DECLARE_bool(log_memmove_in_compareputdata);
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     ComparePutData   checks the put criteria                  
//------------------------------------------------------------------------------
// Description:       This function either adds a new target, sets the          
//                    multiple flag to true or replaces an existing target      
//                    record                                                    
//+---------------------------------------------------------------------------- 
// Function call:     ComparePutData( EqfMemory* pTmClb,                          
//                                    PTMX_RECORD pTmRecord,                    
//                                    PTMX_PUT pTmPut,                          
//                                    PTMX_SENTENCE pSentence,                  
//                                    PULONG pulKey                             
//------------------------------------------------------------------------------
// Input parameter: EqfMemory* pTmClb                                             
//                  PTMX_RECORD pTmRecord                                       
//                  PTMX_PUT pTmPut                                             
//                  PTMX_SENTENCE pSentence                                     
//                  PULONG pulKey                                               
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:    usRc NO_ERROR                                                
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   if source strings are equal                                                
//     tokenize target                                                          
//     Delete old entry                                                         
//     Loop thru target records                                                 
//        loop over all target CLBs or until fStop                              
//                   if segment+file id found (exact-exact-found!)              
//                      update time field in control block                      
//                      set fUpdate= fStop=TRUE                                 
//                      update context info                                     
//                   if not fStop                                               
//                      goto next CLB                                           
//                endloop                                                       
//                if no matching CLB has been found (if not fStop)              
//                    add new CLB (ids, context, timestamp etc. )               
//        endloop                                                               
//     endloop                                                                  
//     if fupdated, update TM record                                            
//     if !fStop (all target record have been tried & none matches )            
//       add new target record to end of tm record                              
//   else                                                                       
//       source_string_error                                                    
//------------------------------------------------------------------------------
USHORT EqfMemory::ComparePutData
(
  std::vector<BYTE>& ppTmRecord,             // ptr to ptr of tm record data buffer
  OtmProposal&  TmProposal,                  // pointer to get in data
  PULONG      pulKey                   // tm key
)
{
  BOOL fOK = true;                            //success indicator
  BOOL fStop = FALSE;                  //indicates whether to leave loop or not
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL; //ptr to target record
  PTMX_TARGET_CLB    pClb = NULL;    //ptr to target control block
  LONG lLen = 0;                        //length indicator
  USHORT usNormLen = 0;                    //length of normalized string
  //PSZ_W pString = NULL;                  //pointer to character string
  std::vector<wchar_t> pString(MAX_SEGMENT_SIZE);
  USHORT usRc = NO_ERROR;              //returned value from function
  BOOL        fUpdate = FALSE;         // TRUE = record has been updated
  PTMX_RECORD pTmRecord = *ppTmRecord; // pointer to tm record data
  USHORT      usAuthorId;              // ID for author string
  LONG lLeftClbLen;
  int delTargetKey = 0, targetKey = 0;

  //allocate pString
  //fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE*sizeof(CHAR_W), NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    BOOL  fStringEqual;                // indicate string equal
    USHORT  usPutLang;                 // language id of target language
    USHORT  usPutFile;                 // file id of new entry

    //get id of target language in the put structure
    if (NTMGetIDFromName( TmProposal.szTargetLanguage,
                          NULL, (USHORT)LANG_KEY, &usPutLang ))
    {
      usPutLang = 1;
    } /* endif */

    NTMGetIDFromName( TmProposal.szTargetAuthor, 			// get author ID
                      NULL, (USHORT)AUTHOR_KEY, &usAuthorId );

    usRc = NTMGetIDFromName( TmProposal.szDocName,		 // get file id
                             TmProposal.szDocName, (USHORT)FILE_KEY, &usPutFile );
    if ( !usRc )
    {
      //position at beginning of source structure in tm record
      pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);
      pByte = NTRecPos((PBYTE)(pTmRecord+1), REC_SRCSTRING);

      //calculate length of source string
      lLen = (RECLEN(pTMXSourceRecord) - sizeof( TMX_SOURCE_RECORD ));
      //copy and compare source string
      memset( pString.data(), 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));
      lLen = EQFCompress2Unicode( pString, pByte, lLen );
      fStringEqual = ! UTF16strcmp( pString.data(), TmProposal.pInputSentence->pStrings->getGenericTagStrC() );

      if ( fStringEqual )
      {
        BOOL fNewerTargetExists = FALSE;
        LONG   lLeftTgtLen;          // remaining target length

        NTASSERTLEN(RECLEN(pTmRecord), pTmRecord->usFirstTargetRecord, 4712)

        // get length of target block to work with
        //source strings equal - position at first target record
        lLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
        pStartTarget = NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);
        pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;

        fStop = (usRc != 0);
        //RJ: 04/01/22: P018830:
        // loop through all target records
        // delete entry if current segment has already been translated        
        TMLoopAndDelTargetClb(pTmRecord, TmProposal, usPutLang, usPutFile, usAuthorId,  &fNewerTargetExists, &delTargetKey );

        // recalc since record may have changed during delete above!
        lLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
        pStartTarget = NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);
        pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;

        if ( fNewerTargetExists && !FLAGS_ignore_newer_target_exists_check)
        {
          fStop = TRUE;           // do not continue update loop
        } 
        else
        {
          //source strings are identical so loop through target records
          // and add new entry
          while ( fOK && (lLeftTgtLen > 0)  && (lLeftTgtLen <= RECLEN(pTMXTargetRecord)) 
                && (RECLEN(pTMXTargetRecord) > 0) && !fStop )
          { 
            // check for valid target length
            NTASSERTLEN(lLeftTgtLen, RECLEN(pTMXTargetRecord), 4713);
            
            // update target length
            lLeftTgtLen -= RECLEN(pTMXTargetRecord);

            // next check the target language and target tag info
            // position at first target control block and to source tag info
            pClb = (PTMX_TARGET_CLB)NTRecPos((PBYTE)pTMXTargetRecord, REC_CLB);

            // compare target language IDs and source tag record
            if (pClb->usLangId == usPutLang)
            {
              // check if target string and target tag record are identical
              pByte = NTRecPos(pStartTarget, REC_TGTSTRING);
              lLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;
              lLen = EQFCompress2Unicode( pString, pByte, lLen );
              pString.data()[lLen] = EOS;

              //compare target strings and target tag record
              if ( !UTF16strcmp( pString.data(), TmProposal.pInputSentence->pStrings->getGenericTargetStrC()))
              {  //target strings and target tag record are equal
                //position at first control block
                pClb = (PTMX_TARGET_CLB)NTRecPos(pStartTarget, REC_CLB);

                // loop over all target CLBs
                pTMXTargetRecord = (PTMX_TARGET_RECORD)pStartTarget;
                lLeftClbLen = RECLEN(pTMXTargetRecord) - pTMXTargetRecord->usClb;
                while ( lLeftClbLen > 0 && !fStop )
                {
                  targetKey++;
                  if (
                        (pClb->usFileId == usPutFile) &&
                        (pClb->usAuthorId == usAuthorId) &&                        
                        isAddDataIsTheSame(TmProposal, *pClb)                        
                      )
                  {
                    // an identical segment already in record (update
                    // time if newer than existing one
                    pClb->lTime     = TmProposal.lTargetTime;
                    pClb->ulSegmId  = TmProposal.getSegmentId();
                    pClb->usFileId  = usPutFile;
                    pClb->bTranslationFlag       = ProposalTypeToFlag(TmProposal.eType);
                    pClb->usAuthorId = usAuthorId;
                    fUpdate         = TRUE;
                    fStop           = TRUE;

                    // adjust context part of target control block if necessary
                    NTMAdjustAddDataInTgtCLB( ppTmRecord, pulRecBufSize, TmProposal, &pClb, &pTmRecord, &pTMXTargetRecord, &lLeftClbLen, &fUpdate );
                  } /* endif */

                  // continue with next target CLB
                  if ( !fStop )
                  {
                    lLeftClbLen -= TARGETCLBLEN(pClb);
                    pClb = NEXTTARGETCLB(pClb);
                  } /* endif */
                } /* endwhile */

                // add new CLB if no matching CLB has been found
                if ( !fStop )
                {
                  targetKey++;
                  // re-alloc record buffer if too small
                  USHORT usAddDataLen = NTMComputeAddDataSize( TmProposal.szContext, TmProposal.szAddInfo );
                  ULONG ulNewSize = RECLEN(pTmRecord) + sizeof(TMX_TARGET_CLB) + usAddDataLen;
                  if ( ulNewSize > *pulRecBufSize)
                  {
                    fOK = UtlAlloc( (PVOID *)ppTmRecord, *pulRecBufSize, ulNewSize, NOMSG );
                    if ( fOK )
                    {
                      *pulRecBufSize = ulNewSize;
                      pClb = (PTMX_TARGET_CLB) ADJUSTPTR( *ppTmRecord, pTmRecord, pClb );
                      pTMXTargetRecord = (PTMX_TARGET_RECORD) ADJUSTPTR( *ppTmRecord, pTmRecord, pTMXTargetRecord );
                      pTmRecord = *ppTmRecord;
                    }
                    else
                    {
                      LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
                    } /* endif */
                  } /* endif */

                  // make room at pCLB for a new CLB and adjust TM record
                  // length and target record length
                  if ( fOK )
                  {
                    LONG lNewClbLen = sizeof(TMX_TARGET_CLB) + usAddDataLen; 
                    //LONG size =  RECLEN(pTmRecord) - ((PBYTE)pClb - (PBYTE)pTmRecord);
                    LONG size = TARGETCLBLEN(pClb); 
                
                    //try{                    
                    if(size>=0)
                    {
                      size_t occupiedSize = RECLEN(pTmRecord) + sizeof( TMX_SOURCE_RECORD ) + sizeof(TMX_TARGET_RECORD);
                      if(lNewClbLen + occupiedSize >= TMX_REC_SIZE){
                        usRc = BTREE_NODE_IS_FULL;
                        fStop = true;
                        fOK = false;
                      }else{
                        if(FLAGS_log_memmove_in_compareputdata)
                        {
                          T5LOG(T5ERROR) << "memmove size = "<< size << "; lNewClbLen = " << lNewClbLen << ";  RECLEN(pTmRecord) = " 
                            <<  RECLEN(pTmRecord) << ";  RECLEN(pTMXTargetRecord)  = " << RECLEN(pTMXTargetRecord) <<"; occupiedSize = " << occupiedSize
                            << "; ulKey = " << *pulKey ;
                        }
                        memmove( (((PBYTE)pClb) + lNewClbLen), pClb, size);
                        RECLEN(pTmRecord) += lNewClbLen;
                        RECLEN(pTMXTargetRecord) += lNewClbLen;
                      }
                    }
                    else
                    //}catch(...)
                    {                    
                      T5LOG(T5ERROR) << "::memmove size is less or equal to 0, size = " << size << "; lNewClbLen = " << lNewClbLen << "; segment was not saved";//; memmove crash";
                      usRc = ERROR_ADD_TO_TM;
                      fOK = false;
                    }
                  } /* endif */

                  // fill-in new target CLB
                  if ( fOK )
                  {
                    //pClb->bMultiple = FALSE;
                    if ( TmProposal.lTargetTime )
                    {
                      pClb->lTime = TmProposal.lTargetTime;
                    }
                    else
                    {
                      UtlTime( &(pClb->lTime) );
                    } /* endif */
                    pClb->ulSegmId   = TmProposal.getSegmentId();
                    pClb->usFileId   = usPutFile;
                    pClb->bTranslationFlag = ProposalTypeToFlag(TmProposal.eType);
                    pClb->usAuthorId = usAuthorId;
                    pClb->usLangId   = usPutLang;
                    pClb->usAddDataLen = 0;
                    NtmStoreAddData( pClb, ADDDATA_CONTEXT_ID, TmProposal.szContext );
                    NtmStoreAddData( pClb, ADDDATA_ADDINFO_ID, TmProposal.szAddInfo );
                  } /* endif */
                  if(fOK){
                    fStop = TRUE;        // avoid add of a new target record at end of outer loop
                    fUpdate = TRUE;
                  }//else{
                  //  fOK = true;
                  //}
                } /* endif */
              } /* endif */
            } /* endif */

            //position at next target
            if ( !fStop )
            {
			        pStartTarget = NTRecPos(pStartTarget, REC_NEXTTARGET);
			        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pStartTarget);
            } /* endif */
          } /* endwhile */
        } /* endif */

        // update TM record if required
        if ( fUpdate )
        {
          usRc = TmBtree.EQFNTMUpdate( *pulKey, (PBYTE)pTmRecord, RECLEN(pTmRecord) );
        } /* endif */

        if ( fStop )
        {
          TmProposal.currentInternalKey.setInternalKey(*pulKey, targetKey);
          //TmProposal.recordKey = *pulKey;
          //TmProposal.targetKey = targetKey;          
        }else{
          //all target records have been checked but nothing overlapped
          //so add new target record to end of tm record
          usRc = AddTmTarget( TmProposal, ppTmRecord, pulRecBufSize, pulKey );
          pTmRecord = *ppTmRecord;
          
          if(!usRc)
          {
            TmProposal.currentInternalKey.setInternalKey(*pulKey, 1);
            //TmProposal.recordKey = *pulKey;
            //TmProposal.targetKey = 1;
          }
        } /* endif */
      }
      else
      {
        //source strings are not equal so try another sid or if all have been
        //tries add new tm record
        LOG_AND_SET_RC(usRc, T5INFO, SOURCE_STRING_ERROR);
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  //UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  //UtlAlloc( (PVOID *) &pTagRecord, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( COMPAREPUTDATA_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  return( usRc );
}



//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     AddTmTarget       add a new target record to end of tm    
//                                      record                                  
//------------------------------------------------------------------------------
// Description:       Adds a new target to tm record                            
//+---------------------------------------------------------------------------- 
// Function call:     AddTmTarget( EqfMemory* pTmClb,                             
//                                 PTMX_PUT pTmPut,                             
//                                 PTMX_SENTENCE pSentence,                     
//                                 PTMX_RECORD pTmRecord,                       
//                                 PULONG pulKey )                              
//------------------------------------------------------------------------------
// Input parameter: EqfMemory* pTmClb                                             
//                  PTMX_PUT pTmPut                                             
//                  PTMX_SENTENCE pSentence                                     
//                  PTMX_RECORD pTmRecord                                       
//                  PULONG pulKey                                               
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   tokenize target string                                                     
//   fill target control block structure                                        
//   fill target tm record structure add end of tm record                       
//   insert tm record in tm data fill                                           
//------------------------------------------------------------------------------
USHORT EqfMemory::AddTmTarget(
  OtmProposal& TmProposal,       //pointer to get in data
  PTMX_RECORD *ppTmRecord,          //pointer to tm record data pointer
  PULONG pulRecBufSize,             //ptr to current size of TM record buffer
  PULONG pulKey )                   //tm key
{
  PTMX_TARGET_CLB pTargetClb = nullptr ;              // ptr to target ctl block
  PTMX_TARGET_RECORD pTargetRecord = nullptr;  // ptr to target record
  std::vector<BYTE> vTargetClb, vTargetRecord;//for managing memory
  USHORT       usRc = NO_ERROR;           // return code
  BOOL         fOK;                       // success indicator
  PBYTE        pByte;                     // position pointer
  PTMX_RECORD pTmRecord = *ppTmRecord;    //pointer to tm record data
  ULONG       ulAddDataLen = 0;

  //allocate target control block record
  ulAddDataLen = NTMComputeAddDataSize( TmProposal.szContext, TmProposal.szAddInfo );
  vTargetClb.resize(sizeof(TMX_TARGET_CLB)+ulAddDataLen);
  vTargetRecord.resize(TMX_REC_SIZE);
  pTargetClb = (PTMX_TARGET_CLB)vTargetClb.data();
  pTargetRecord = (PTMX_TARGET_RECORD)vTargetRecord.data();


  {
    if ( usRc == NO_ERROR )
    {
      usRc = FillClb( pTargetClb, TmProposal );
      if ( usRc == NO_ERROR )
      {
        //fill target record
        FillTargetRecord( TmProposal.pInputSentence,  //ptr to sentence structure
                          TmProposal.pInputSentence->pStrings->getGenericTargetStrC(),
                          wcslen(TmProposal.pInputSentence->pStrings->getGenericTargetStrC()),
                          &pTargetRecord,        //filled tm record returned
                          pTargetClb );                    //tm target control block

        //check space requirements
        {
          // re-alloc record buffer if too small
          ULONG ulNewSize = RECLEN(pTmRecord) + sizeof(TMX_TARGET_RECORD) +
                            RECLEN(pTargetRecord);
          if ( ulNewSize > *pulRecBufSize)
          {
            fOK = UtlAlloc( (PVOID *)ppTmRecord, *pulRecBufSize, ulNewSize, NOMSG );
            if ( fOK )
            {
              *pulRecBufSize = ulNewSize;
              pTmRecord = *ppTmRecord;
            }
            else
            {
              LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
            } /* endif */
          } /* endif */
        }

        if ( fOK )
        {
          //postion at end on tm record
          pByte = (PBYTE)pTmRecord;
          pByte += RECLEN(pTmRecord);

          //add new target record to end
          memcpy( pByte, pTargetRecord, RECLEN(pTargetRecord) );

          //update overall length of tm record
          RECLEN(pTmRecord) += RECLEN(pTargetRecord);

          //add updated tm record to database
          usRc = TmBtree.EQFNTMUpdate(
                               *pulKey,
                               (PBYTE)pTmRecord,
                               RECLEN(pTmRecord) );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    ERREVENT2( ADDTMTARGET_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  return( usRc );
}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     FillTargetRecord fills target record and adds to end of   
//                    tm record                                                 
//------------------------------------------------------------------------------
// Description:       Fills a tm target record                                  
//+---------------------------------------------------------------------------- 
// Function call:     FillTargetRecord( PTMX_SENTENCE pSentence,                
//                                    PTMX_TAGTABLE_RECORD pTagRecord,          
//                                    PSZ pNormString,                          
//                                    USHORT usNormLen,                         
//                                    PTMX_TARGET_RECORD pTMXTargetRecord,      
//                                    PTMX_TARGET_CLB pTargetClb )              
//------------------------------------------------------------------------------
//   Input parameter:                                                           
//     PTMX_SENTENCE pSentence,           //ptr to sentence structure           
//     PTMX_TAGTABLE_RECORD pTagRecord,   //ptr to target string tag table      
//     PSZ pNormString,                   //ptr to target normalized string     
//     USHORT usNormLen,                  //length of target normalized string  
//     PTMX_TARGET_CLB pTargetClb         //ptr to target control block         
//------------------------------------------------------------------------------
// Output parameter: PTMX_TARGET_RECORD pTMXTargetRecord //ptr to target record 
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:                                                                 
//                                                                              
//------------------------------------------------------------------------------
// Function flow:                                                               
//   fill target tm record structure                                            
//------------------------------------------------------------------------------

VOID FillTargetRecord
(
  PTMX_SENTENCE pSentence,           //ptr to sentence structure
//  PTMX_TAGTABLE_RECORD pTagRecord,   //ptr to target string tag table
  PSZ_W pNormString,                 //ptr to target normalized string
  USHORT usNormLen,                  //length of target normalized string
  PTMX_TARGET_RECORD *ppTMXTargetRecord,  //ptr to target record
  PTMX_TARGET_CLB pTargetClb         //ptr to target control block
)
{
  PBYTE pTarget;                                 //ptr to target record
  PTMX_TARGET_RECORD pTMXTargetRecord;           //ptr to target record

  pTMXTargetRecord = *ppTMXTargetRecord;

  //position to start of target structure
  pTarget = (PBYTE)pTMXTargetRecord;

  //set source tag table offset
  //pTMXTargetRecord->usSourceTagTable = sizeof(TMX_TARGET_RECORD);

  //position pointer for source tag table
  //pTarget += pTMXTargetRecord->usSourceTagTable;

  //copy source tag table record to correct position
  //memcpy( pTarget, pSentence->pTagRecord, RECLEN(pSentence->pTagRecord) );

  //set target tag table start offset
  //pTMXTargetRecord->usTargetTagTable = (USHORT)(pTMXTargetRecord->usSourceTagTable +
  //                                     RECLEN(pSentence->pTagRecord));
  
  //adjust target pointer for target tag table
  //pTarget += RECLEN(pSentence->pTagRecord);

  //copy target tag table record to correct position
  //memcpy( pTarget, pTagRecord, RECLEN(pTagRecord) );

  //set target string start offset
  pTMXTargetRecord->usTarget = sizeof(*pTMXTargetRecord);
  pTarget += pTMXTargetRecord->usTarget;
  //(USHORT)(pTMXTargetRecord->usTargetTagTable + RECLEN(pTagRecord);
  //adjust target pointer for target string
  //pTarget += RECLEN(pTagRecord);

  //copy target string to correct position
  { ULONG ulTempLen = usNormLen;
    ulTempLen = EQFUnicode2Compress( pTarget, pNormString, ulTempLen );
    usNormLen = (USHORT)ulTempLen;
  }
//  memcpy( pTarget, pNormString, usNormLen );

  //set target string control block start offset
  pTMXTargetRecord->usClb = pTMXTargetRecord->usTarget + usNormLen;

  //adjust target pointer for control block
  pTarget += usNormLen;

  //copy target control block
  memcpy( pTarget, pTargetClb, TARGETCLBLEN(pTargetClb) );

  //size of target record
  RECLEN(pTMXTargetRecord) = sizeof(TMX_TARGET_RECORD) +
                                  //RECLEN(pSentence->pTagRecord) +
                                  //RECLEN(pTagRecord) +
                                  usNormLen +
                                  TARGETCLBLEN(pTargetClb);
  *ppTMXTargetRecord = pTMXTargetRecord;
}



/**********************************************************************/
/* the following routines implement a fast tokenize independent on    */
/* language characteristics                                           */
/**********************************************************************/

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TmtXUpdSeg       update segment data                      
//------------------------------------------------------------------------------
// Description:       Updates the segment data of a specific segment            
//------------------------------------------------------------------------------
// Function call:  TmtXUpdSeg( pTmClb, pTmPutIn, ulUpdKey, usUpdTarget, usFlags 
//------------------------------------------------------------------------------
// Input parameter:   EqfMemory*  pTmClb         control block                    
//                    PTMX_PUT_IN pTmPutIn     input structure                  
//                    ULONG       ulUpdKey     SID of record being updated      
//                    USHORT      usUpdTarget  number of target being updated   
//                    USHORT      usFlags      flags controlling the update     
//------------------------------------------------------------------------------
USHORT EqfMemory::TmtXUpdSeg
(
  OtmProposal* pTmPutIn,    // ptr to put input data
  PTMX_EXT_OUT_W pTmPutOut,   //ptr to output struct
  USHORT      usFlags      // flags controlling the updated fields
)
{
  BOOL       fOK;                      // success indicator
  USHORT     usRc = NO_ERROR;          // return code
  BOOL        fLocked = FALSE;         // TM-database-has-been-locked flag
  ULONG  ulLen = 0;                    //length indicator
  PBYTE pByte;                         //position ptr
  USHORT usTarget;                     //nr of target records in tm record
  PTMX_TARGET_RECORD pTMXTargetRecord; // ptr to target record
  PTMX_TARGET_CLB    pTargetClb;       // ptr to target CLB
  ULONG       ulLeftClbLen;            // remaining length of CLB area
  
  std::vector<BYTE> TmRecord;         // space for user data
  TmRecord.resize(TMX_REC_SIZE);

  // get TM record being modified and update the record
  if ( !usRc )
  {
    LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);

    ulLen = TMX_REC_SIZE;
    usRc =  TmBtree.EQFNTMGet( pTmPutIn->currentInternalKey.getRecordKey(), TmRecord );
    PTMX_RECORD pTmRecord = (PTMX_RECORD) TmRecord.data();    

    if ( usRc == NO_ERROR )
    {
      //check if record is empty
      if ( ( RECLEN(pTmRecord) != 0) &&
           (pTmRecord->usFirstTargetRecord < RECLEN(pTmRecord)) )
      {
        ULONG   ulLeftTgtLen;                    // remaining target length
        ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

        if ( !usRc )
        {
          //find target record specified in usUpdTarget
          //move pointer to first target
          pByte = (PBYTE)(pTmRecord);
          pByte += pTmRecord->usFirstTargetRecord;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
          ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          ulLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB
          usTarget = 1;           //initialize counter

          //loop until correct target is found
          while ( (usTarget < pTmPutIn->currentInternalKey.getTargetKey()) //pTmPutIn->targetKey) 
            && ulLeftTgtLen )
          {
            // position to first target CLB
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
            ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                           pTMXTargetRecord->usClb;
            ulLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB

            // loop over all target CLBs
            while ( (usTarget < pTmPutIn->currentInternalKey.getTargetKey()) && ulLeftClbLen )
            {
              usTarget++;
              pTargetClb = NEXTTARGETCLB(pTargetClb);
              ulLeftClbLen -= TARGETCLBLEN(pTargetClb);
            } /* endwhile */

            // continue with next target if no match yet
            if ( usTarget < pTmPutIn->currentInternalKey.getTargetKey() )
            {
              // position at next target
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
              //move pointer to end of target
              if (ulLeftTgtLen >= RECLEN(pTMXTargetRecord))
              {
                ulLeftTgtLen -= RECLEN(pTMXTargetRecord);
              }
              else
              {
                ulLeftTgtLen = 0;
              } /* endif */
              pByte += RECLEN(pTMXTargetRecord);
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
              pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
              usTarget++;
            } /* endif */
          } /* endwhile */

          if ( usTarget == pTmPutIn->currentInternalKey.getTargetKey())//pTmPutIn->targetKey )
          {
            //position at start of target record
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

            //if target record exists
            if ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) )
            {
              // update requested fields

              // change markup/tag table if requested
              if ( //(usFlags & TMUPDSEG_MARKUP) &&
                   (pTmPutIn->szMarkup[0] != EOS) )
              {
                PTMX_TAGTABLE_RECORD pTMXTagTableRecord;
                USHORT usNewId;
                PBYTE pByte;

                // get ID for new tag table
                //usRc = NTMGetIDFromName( pTmPutIn->szMarkup,
                //                         NULL,
                //                         (USHORT)TAGTABLE_KEY,
                //                         &usNewId );
                usNewId = 1;

                // update source tag table record
                //pByte = (PBYTE)pTMXTargetRecord;
                //pByte += pTMXTargetRecord->usSourceTagTable;
                //pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;
                //pTMXTagTableRecord->usTagTableId = 1;//usNewId;

                // update target tag table record
                //pByte = (PBYTE)pTMXTargetRecord;
                //pByte += pTMXTargetRecord->usTargetTagTable;
                //pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;
                //pTMXTagTableRecord->usTagTableId = 1;//usNewId;
              } /* endif */

              // update MT flag if requested
              if ( (usRc == NO_ERROR) 
                // && (usFlags & TMUPDSEG_MTFLAG) 
                )
              {
                // set type of translation flag
                pTargetClb->bTranslationFlag = (BYTE)pTmPutIn->eType;
              } /* endif */

              // update target language if requested
              if ( (usRc == NO_ERROR) 
                  // && (usFlags & TMUPDSEG_TARGLANG) 
                )
              {
                // set target language
                usRc = NTMGetIDFromName( pTmPutIn->szTargetLanguage,
                                         NULL,
                                         (USHORT)LANG_KEY,
                                         &pTargetClb->usLangId );
              } /* endif */

              // update segment date if requested
              if ( (usRc == NO_ERROR) 
                  // && (usFlags & TMUPDSEG_DATE) 
                )
              {
                pTargetClb->lTime = pTmPutIn->lTargetTime;
              } /* endif */


              // rewrite TM record
              if ( usRc == NO_ERROR )
              {
                usRc = TmBtree.EQFNTMUpdate( pTmPutIn->currentInternalKey.getRecordKey(),
                                     pTmRecord, RECLEN(pTmRecord) );
              } /* endif */

              if(usRc == NO_ERROR)
              {
                usRc = RewriteCompactTable();
              }
            }
            else
            {
              // target not found
              LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
            } /* endif */
          }
          else
          {
            // record not found
            LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
          } /* endif */
        } /* endif */
      }
      else
      {
        // record is empty and should not be updated
        LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRc );
} /* end of function TmtXUpdSeg */


/**********************************************************************/
/* TMDelTargetClb                                                     */
/*                                                                    */
/* Delete target CLB and, if there is no more target CLB left, the    */
/* complete target record                                             */
/* Return TRUE if target record has been deleted                      */
/**********************************************************************/
BOOL TMDelTargetClb
(
  PTMX_RECORD        pTmRecord,        // ptr to TM record
  PTMX_TARGET_RECORD pTargetRecord,    // ptr to target record within TM record
  PTMX_TARGET_CLB    pTargetClb        // ptr to target control record
)
{
  BOOL fTargetRemoved = FALSE;
  PBYTE  pTmEnd;
  PBYTE  pEndTarget;
  USHORT  usTargetCLBLen;
  pTmEnd = (PBYTE)pTmRecord;
  pTmEnd += RECLEN(pTmRecord);

  // check if there are more than one CLBs in current target record
  usTargetCLBLen = TARGETCLBLEN(pTargetClb);
  if ( RECLEN(pTargetRecord) <=
       (ULONG)(pTargetRecord->usClb + usTargetCLBLen) )
  {
    // only one CLB in target record ==> delete complete target record

    //remember length of current target to be replaced
    pEndTarget = (PBYTE)pTargetRecord + RECLEN(pTargetRecord);

    //calculate new length of TM record
    RECLEN(pTmRecord) -= RECLEN(pTargetRecord);

    //remove the current target record
    memmove( pTargetRecord, pEndTarget, pTmEnd - pEndTarget );

    // remember that the complete target record has been removed
    fTargetRemoved = TRUE;

    // initialize last target record
    pTmEnd = (PBYTE)pTmRecord;
    pTmEnd += RECLEN(pTmRecord);
    pTargetRecord = (PTMX_TARGET_RECORD)pTmEnd;
    RECLEN(pTargetRecord) = 0;
  }
  else
  {
    // delete only current target CLB

    //calculate new length of tm record
    RECLEN(pTmRecord) -= usTargetCLBLen;
    RECLEN(pTargetRecord) -= usTargetCLBLen;

    //remove the current target CLB
    {
      PBYTE pNextClb = ((PBYTE)pTargetClb) + usTargetCLBLen;
      memmove( pTargetClb, pNextClb, pTmEnd - pNextClb );
    }
  } /* endif */

  return( fTargetRemoved );
} /* end of function TMDelTargetClb */


// Do a primitive on BOCU (see IBM DeveloperWorks) based compression...
static long lBoundary[6] = {   -0x02F41,   -0x041,  -0x01, 0x03f, 0x02f3f, 0x010ffff };
static int  byteCount[6] = {          3,        2,      1,     1,       2,         3 };
static long lOffset[6]   = {  0x010ffff,  0x04040,  0x080, 0x080, 0x0bfc0, 0x0ef0000 };
static long lCompress[6] = {       0x10,    0x03f,  0x07f, 0x0bf,   0x0ee,     0x0ff };



ULONG EQFUnicode2Compress( PBYTE pTarget, PSZ_W pInput, ULONG ulLenChar )
{
  ULONG ulLen = ulLenChar * sizeof(CHAR_W);


  if ( ulLen < 20 )   // don't care about too short strings
  {
    *pTarget++ = 0;
    memcpy( pTarget, pInput, ulLen );
    pTarget[ulLen] = EOS;
  }
  else
  {
    long delta;
    PBYTE pTemp;
    PSZ_W pTempIn = pInput;
    int  oldCodePoint = 0x80;
    int  i, bCount, bCountTemp, cp;


    *pTarget++ = BOCU_COMPRESS;
    pTemp = pTarget;


    for ( ulLen = 0; ulLen < ulLenChar; ulLen++ )
    {
      cp = *pInput++;
      delta = cp - oldCodePoint;
      oldCodePoint = cp;


      for (i=0;  ; ++i)   // will always break
      {
         if ( delta <= lBoundary[i])
         {
           delta += lOffset[i];
           bCountTemp = bCount = byteCount[i];

           while (--bCount >0 )
           {
             pTarget[bCount] = ((BYTE)delta & 0x0ff);
             delta = (delta & ~0xff) >> 8;
           }

           // store lead byte
           *pTarget  = (BYTE)(*pTarget  + (BYTE)delta);
           pTarget += bCountTemp;
           break;
         }
      }
    }
    ulLen = pTarget - pTemp;
//    pTarget[ulLen] = EOS;
    pTemp[ulLen] = EOS;

    // just in case we enlarged the string -- use original one...
    if (ulLen > ulLenChar * sizeof(CHAR_W))
    {
      pTarget = pTemp;
      ulLen = ulLenChar * sizeof(CHAR_W);
      *(pTarget-1) = 0;   // no compression
      memcpy( pTarget, pTempIn, ulLen );
      pTarget[ulLen] = EOS;
    }
  }

  return ulLen+1;
}


LONG EQFCompress2Unicode( std::string& pOutput, PBYTE pTarget, ULONG ulLenComp )
{
    LONG  lLen = ulLenComp-1;
    pOutput.clear();

    if(lLen <= 0 ) 
      return 0;
      
    pOutput.reserve(ulLenComp * 1.5);
    
    BYTE b = *pTarget++;
    if ( b == 0 )  // no compression
    {
        lLen = ulLenComp / sizeof(CHAR_W);
        pOutput.insert(pOutput.begin(), pTarget, pTarget + lLen);
        pOutput[ lLen ] = EOS;
    }
    else if (b == BOCU_COMPRESS)
    {
      //PSZ_W pTemp = pOutput;
      long delta = 0;
      USHORT iLen = 0;
      int  oldCodePoint = 0x80;
      int  i, bCount;

      for ( iLen = 0; iLen < lLen; iLen++ )
      {
        delta = (unsigned char)*pTarget++;

        for (i=0; ; i++ )
        {
          if ( delta <= lCompress[i] )
          {
            bCount = byteCount[i];

            while (--bCount >0 )
            {
              delta = (delta << 8) + (unsigned char)*pTarget++;
              iLen++;
            }

            delta = delta + oldCodePoint - lOffset[i];

            pOutput.insert(pOutput.end(), (unsigned short) delta);
            oldCodePoint = delta;

            break;
          }
        }
      }
      pOutput.push_back('\0');
      lLen = pOutput.size();//(pOutput - pTemp);
    }
    else
    {
        //FilesystemHelper::FlushAllBuffers();
        assert( 0 == 1);
    }
    return lLen;
}
#endif 


// adjust context part of
// target control block if necessary

USHORT NTMAdjustAddDataInTgtCLB
(
	PTMX_RECORD	      *ppTmRecordStart,
	PULONG     		   pulRecBufSize,
	OtmProposal& 		   TmProposal,
	PTMX_TARGET_CLB    *ppClb,
	PTMX_RECORD        *ppCurTmRecord,
	PTMX_TARGET_RECORD *ppTMXTargetRecord,
	PLONG  		   pulLeftClbLen,
	PBOOL 			   pfUpdate)
{
  BOOL    fOK = TRUE;
  USHORT  usOldLen = 0;
  ULONG   ulNewLen = 0;
  USHORT  usRc = NO_ERROR;
  PTMX_RECORD  pCurTmRecord = NULL;
  PTMX_TARGET_RECORD  pTMXTargetRecord;           //ptr to target record
  PTMX_TARGET_CLB     pCurClb;
  PTMX_TARGET_CLB     pNextClb = NULL;

  pTMXTargetRecord = *ppTMXTargetRecord;
  pCurTmRecord     = *ppCurTmRecord;
  pCurClb          = *ppClb;
  usOldLen         = pCurClb->usAddDataLen;

  if ( TmProposal.szContext[0] )
  {
	  ulNewLen = NTMComputeAddDataSize( TmProposal.szContext, TmProposal.szAddInfo );
  } /* endif */

  if ( usOldLen == ulNewLen )
  {
	  // no resizing required, set update flag as data may have been changed
	  if ( ulNewLen )
	  {
		  *pfUpdate = TRUE;
	  } /* endif */
  }
  else if ( usOldLen > ulNewLen )
  {
	  // new length is smaller, reduce CLB size later when the additional data area has been re-filled but
    // remember position of next CLB
	  pNextClb = NEXTTARGETCLB(pCurClb);

  }
  else
  {
	  // new length is larger, re-alloc record and adjust pointers
	  ULONG ulDiff = ulNewLen - usOldLen;

	  // re-alloc record buffer if too small
	  ULONG ulNewSize = RECLEN(pCurTmRecord) + (ULONG)ulDiff;
	  if ( ulNewSize > *pulRecBufSize)
	  {
	    fOK = UtlAlloc( (PVOID *)ppTmRecordStart, *pulRecBufSize,
					    ulNewSize, NOMSG );
	    if ( fOK )
	    {
		    *pulRecBufSize = ulNewSize;
		    pCurClb = (PTMX_TARGET_CLB) ADJUSTPTR( *ppTmRecordStart, pCurTmRecord, pCurClb );
		    // *ppTMRecordStart + (pClb-ppCurTmRecord)
		    pTMXTargetRecord = (PTMX_TARGET_RECORD) ADJUSTPTR( *ppTmRecordStart, pCurTmRecord, pTMXTargetRecord );
		    pCurTmRecord = *ppTmRecordStart;
	    }
	    else
	    {
		    LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
	    } /* endif */
	  } /* endif */

	  // enlarge current control block and adjust TM record
	  // length and target record length
	  if ( fOK )
	  {
	    pNextClb = NEXTTARGETCLB(pCurClb);
	    memmove( ((PBYTE)pNextClb) + ulDiff, pNextClb, RECLEN(pCurTmRecord) - ((PBYTE)pNextClb - (PBYTE)pCurTmRecord) );
	    RECLEN(pCurTmRecord)  += ulDiff;
	    RECLEN(pTMXTargetRecord) += ulDiff;
	    *pulLeftClbLen += ulDiff;
	    *pfUpdate         = TRUE;
	  } /* endif */
  } /* endif */

  // update additional data
  pCurClb->usAddDataLen = 0;
  NtmStoreAddData( pCurClb, ADDDATA_CONTEXT_ID, TmProposal.szContext );
  NtmStoreAddData( pCurClb, ADDDATA_ADDINFO_ID, TmProposal.szAddInfo );

  // reduce size of CLB when the new content is smaller
  if ( usOldLen > ulNewLen )
  {
	  // new length is smaller, reduce the CLB size and adjust following data
	  ULONG ulDiff = usOldLen - ulNewLen;
	  memmove( ((PBYTE)pNextClb) - ulDiff, pNextClb, RECLEN(pCurTmRecord) - ((PBYTE)pNextClb - (PBYTE)pCurTmRecord) );
	  RECLEN(pCurTmRecord)       -= ulDiff;
	  RECLEN(pTMXTargetRecord)   -= ulDiff;
	  *pulLeftClbLen -= ulDiff;
	  *pfUpdate         = TRUE;
  } /* endif */

  *ppTMXTargetRecord = pTMXTargetRecord;
  *ppCurTmRecord = pCurTmRecord;
  *ppClb = pCurClb;
  return (usRc);
} // end of context handling



//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TMLoopAndDelTargetClb                     
//------------------------------------------------------------------------------
// Description:       This function loops thru all target records and deletes   
//                    the current entry, if found                               
//+---------------------------------------------------------------------------- 
// Function call:     TMLoopAndDelTargetClb( PTMX_RECORD pTmRecord              
//                                    PTMX_PUT_W  pTmPut,                       
//                                    PTMX_SENTENCE pSentence,                  
//                                    USHORT       usPutLang ,                  
//                                    USHORT       usPutFile                    
//------------------------------------------------------------------------------
// Input parameter: PTMX_RECORD         pTmRecord,							    
//                   PTMX_PUT_W 			pTmPut,                             
//                   PTMX_SENTENCE       pSentence,                             
//                   USHORT              usPutLang,                             
//                   USHORT              usPutFile                              
//------------------------------------------------------------------------------
// Returncode type: USHORT                                                      
//------------------------------------------------------------------------------
// Returncodes:      NO_ERROR                                                   
//------------------------------------------------------------------------------
// Function flow:                                                               
//     loop through all target records in tm record checking                    
//       if src tagtable and src tags are equal                                 
//                loop over all target CLBs or until fStop                      
//                   if lang + segment+file id found (exact-exact-found!)       
//                      if entry is older                                       
//                         delete it, fDel = TRUE                               
//                      else goon with search in next tgt CLB (control block)   
//                   else                                                       
//                      goon with search in next tgt CLB (control block)        
//                endloop                                                       
//       endif                                                                  
//       if not fDel                                                            
//          position at next target record                                      
//     endloop                                                                  
//------------------------------------------------------------------------------

USHORT TMLoopAndDelTargetClb
(
	PTMX_RECORD         pTmRecord,
	OtmProposal& 		TmProposal,
	USHORT              usPutLang,
  USHORT              usPutFile,
  USHORT              usPutAuthor,
  PBOOL               pfNewerTargetExists,
  PINT                pTargetKey
)
{
  USHORT 				usRc = NO_ERROR;
  PTMX_TARGET_CLB    	pClb = NULL;    //ptr to target control block
  //PTMX_TAGTABLE_RECORD 	pTMXSrcTTable = NULL; //ptr to source tag info
  LONG        			lLeftClbLen;
  LONG        			lLeftTgtLen;
  BOOL         			fTgtRemoved = FALSE;
  PTMX_TARGET_RECORD  	pTMXTgtRec = NULL;
  BOOL         			fDel = FALSE;

  // preset callers flag
  *pfNewerTargetExists = FALSE;

  // loop until either end of record or one occ. found&deleted
  lLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

  //source strings equal, position at first target record
  pTMXTgtRec = (PTMX_TARGET_RECORD)NTRecPos((PBYTE)(pTmRecord+1), REC_FIRSTTARGET);
  if(pTargetKey) *pTargetKey = 0;

  while ( ( lLeftTgtLen <= RECLEN(pTMXTgtRec) )  && (RECLEN(pTMXTgtRec) != 0) && !fDel )
  {
	NTASSERTLEN(lLeftTgtLen, RECLEN(pTMXTgtRec), 4713);
	lLeftTgtLen -= RECLEN(pTMXTgtRec);

	{
		pClb = (PTMX_TARGET_CLB)NTRecPos((PBYTE)pTMXTgtRec, REC_CLB);
		lLeftClbLen = RECLEN(pTMXTgtRec) - pTMXTgtRec->usClb;

		while ( ( lLeftClbLen > 0 ) && !fDel )
		{
      (*pTargetKey)++;
			if ( (pClb->usLangId == usPutLang) &&
           (pClb->usAuthorId == usPutAuthor) &&
			     (pClb->usFileId == usPutFile) &&
           isAddDataIsTheSame(TmProposal, *pClb)
           )
			{  	// remove target CLB and target record (if only 1 CLB)
				// as the segment is putted with a new value        
				if ( (pClb->lTime < TmProposal.lTargetTime) || !TmProposal.lTargetTime )
				{
				  lLeftClbLen -= TARGETCLBLEN(pClb);
				  // loop over all CLBs of this target record and remove
				  // any CLB for the current segment
				  fTgtRemoved = TMDelTargetClb( pTmRecord, pTMXTgtRec, pClb);
				  fDel = TRUE;
				}
				else
				{  // existing match is newer, continue with next one
				  lLeftClbLen -= TARGETCLBLEN(pClb);
          *pfNewerTargetExists = TRUE;
				  pClb = NEXTTARGETCLB(pClb);
				} /* endif */
			}
			else
			{ 	// no match, try next one
			  lLeftClbLen -= TARGETCLBLEN(pClb);
			  pClb = NEXTTARGETCLB(pClb);
			} /* endif */
		} /* endwhile */
	} /* endif */
	//position at next target
	if ( !fDel )
	{
	  pTMXTgtRec = (PTMX_TARGET_RECORD)NTRecPos((PBYTE)pTMXTgtRec, REC_NEXTTARGET);
	} /* endif */
   } /* endwhile */
   return(usRc);
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     NTRecPos                                                  
//------------------------------------------------------------------------------
// Description:       This function positions at the requested position         
//                    in the given record                                       
//+---------------------------------------------------------------------------- 
// Function call:     NTRecPos( pStart, iType)                                  
//------------------------------------------------------------------------------
// Input parameter: PBYTE pStart,                							    
//                   int  iType                                                 
//------------------------------------------------------------------------------
// Returncode type: PBYTE                                                      
//------------------------------------------------------------------------------
// Returncodes:      new position                                               
//------------------------------------------------------------------------------
// Function flow:                                                               
//     switch(Type )                                                            
//       case REC_CLB: position a first TMX_TARGET_CLB                          
//       case REC_SRCTAGTABLE: position at source TMX_TAGTABLE_RECORD           
//       case REC_TGTTAGTABLE: position at target TMX_TAGTABLE_RECORD           
//       case REC_NEXTTARGET : position at next TMX_TARGET_RECORD               
//       case REC_TGTSTRING: position at target string                          
//       case REC_SRCSTRING:  position at source string                         
//       case REC_SRCTAGTABLE: position at sourc TMX_TAGTABLE_RECORD            
//       case REC_FIRSTTARGET: position at begin of first TMX_TARGET_RECORD     
//     endswitch                                                                
//------------------------------------------------------------------------------


static
PBYTE
NTRecPos
(
	PBYTE pStart,
	int iType
)
{
	PBYTE  pNewPosition = NULL;
	PTMX_TARGET_RECORD  pTmpTgtRecord = NULL;

	switch (iType)
	{
	 	case REC_CLB:
            pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
            pNewPosition  = pStart + pTmpTgtRecord->usClb;
	 	break;
	 	case REC_SRCTAGTABLE:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 		pNewPosition  = pStart;// + pTmpTgtRecord->usSourceTagTable;
	 	break;
	 	case REC_TGTTAGTABLE:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 		pNewPosition  = pStart;// + pTmpTgtRecord->usTargetTagTable;
	 	break;
	 	case REC_NEXTTARGET:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 	    pNewPosition = pStart + RECLEN(pTmpTgtRecord);
	 	break;
	 	case REC_TGTSTRING:
	 	    pTmpTgtRecord = (PTMX_TARGET_RECORD)pStart;
	 	    pNewPosition = pStart + pTmpTgtRecord->usTarget;
        break;
        case REC_SRCSTRING:
          {
			PTMX_SOURCE_RECORD pTmpSrcRecord = (PTMX_SOURCE_RECORD)pStart;
            pNewPosition = pStart + pTmpSrcRecord->usSource;
	      }
        break;
        case REC_FIRSTTARGET:
          {
            PTMX_SOURCE_RECORD pTmpSrcRecord = (PTMX_SOURCE_RECORD)pStart;
            pNewPosition = pStart + RECLEN(pTmpSrcRecord);
	      }
          break;
	 	default:
          pNewPosition = NULL;
	 	break;
    } /* endswitch */

    return (pNewPosition);
}
