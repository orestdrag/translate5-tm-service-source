//+----------------------------------------------------------------------------+
//|EQFNTGET.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFTPI.H>               // Private header file of Standard Editor
#include <EQFMORPI.H>
#include <EQFEVENT.H>             // event logging
#include <OTMGLOBMEM.H>         // Global Memory defines
#include "OtmMarkup.h"
#include "MarkupPluginMapper.H"
#include <map>
#include "../utilities/LogWrapper.h"
#include "../utilities/Property.h"
#include "../utilities/EncodingHelper.h"

// import logging 
//#ifdef MEASURETIME

static LARGE_INTEGER liLast = { 0 };
static LARGE_INTEGER liFrequency = { 0 };
static int QueryPerformanceFrequency(LARGE_INTEGER* li){
  //int res = clock_gettime(CLOCK_REALTIME, li->QuadPart);
  return 0;
}

//#endif

#define ACTIVATE_NTMGenericDelete 


#ifdef MEASURETIME
  void LogWritePerfTime( FILE *hLog, PLARGE_INTEGER pliStart, PLARGE_INTEGER pliEnd, char *pszText );

  void LogWritePerfTime( FILE *hLog, PLARGE_INTEGER pliStart, PLARGE_INTEGER pliEnd, char *pszText ) 
  {
    LARGE_INTEGER liFrequency;

    QueryPerformanceFrequency( &liFrequency );

    if ( liFrequency.QuadPart != 0 )
    {
      LONGLONG ldwDiff = pliEnd->QuadPart - pliStart->QuadPart;
      DWORD dwTime = (DWORD)((ldwDiff * 1000) / liFrequency.QuadPart);
 
      if ( *pszText )
      {
        fprintf( hLog, "Time for %-20s : %ld ms (%I64d ticks)\n", pszText, dwTime, ldwDiff );
      }
      else
      {
        fprintf( hLog, "Total time                    : %ld ms (%I64d ticks)\n", dwTime, ldwDiff );
      } /* endif */
    }
  }
#endif

void NTMLogSegData( PSZ_W pszForm, PSZ_W pszSegData );



USHORT FillMatchTable( EqfMemory*, PSZ_W, PLONG, PTMX_TARGET_RECORD,
                PTMX_TARGET_CLB,
                PTMX_MATCH_TABLE_W, PUSHORT, BOOL, PUSHORT, PUSHORT, PUSHORT,
                ULONG, USHORT, PTMX_GET_W, PTMX_TAGTABLE_RECORD, USHORT, USHORT, int, int, USHORT);

/**********************************************************************/
/* Prototypes for Static functions...                                 */
/**********************************************************************/
//For R012645 begin
static std::map<std::string,std::string> Name2FamilyMap;
static std::string& GetAndCacheFamilyName
(
  char*  pszMarkupName
);

//For R012645 end

static VOID
MakeHashValueW
(
  PULONG    pulRandom,                 // array of random numbers for hashing
  USHORT    usMaxRandom,               // maximum random numbers
  PSZ_W     pData,                     // ptr to data to be hashed
  PULONG    pulHashVal                 // resulting hash value
);


static
BOOL NTMPrepareTokens
(
  PTMX_SUBSTPROP       pSubstProp,
  PSZ_W                pData,
  PFUZZYTOK           *ppTokData,
  PUSHORT              pusTokens,
  PTMX_TAGTABLE_RECORD pTagRecord,
  SHORT                sLangID,
  ULONG                ulOemCP,
  PLOADEDTABLE         pTagTable
);

static BOOL
NTMFuzzyReplace
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
);

#ifdef ACTIVATE_NTMGenericDelete 
static BOOL
NTMGenericDelete
(
  PTMX_SUBSTPROP pSubstProp,
  PSZ_W     pSource,                   // source string
  PSZ_W     pProp,                     // proposal string
  PSZ_W     pTrans,                    // translation string
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList               // list of tokens to be replaced
);
#endif

static BOOL NTMCheckTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplPropSrc,              // list of same tokens in source and prop
  PREPLLIST pReplaceList,               // list of tokens to be replaced
  BOOL      fRespectLFs
);


static PTMX_REPLTAGPAIR
NTMFindTagPair
(
  PFUZZYTOK         pTempTok,
  PSZ_W             pTempTokData,
  PTMX_SUBSTPROP    pSubstProp,
  PTMX_REPLTAGPAIR  pCurTagPair,
  BOOL              fRespectLFs
);


static BOOL
NTMReplaceTags
(
  PTMX_SUBSTPROP pSubstProp,
  BOOL           fRespectLFs
);

static BOOL
NTMCopyTokData
(
   PSZ_W      pTempData,
   SHORT      sLen,
   PSZ_W *    ppNewData,
   PLONG      plNewLen
);


static PFUZZYTOK
NTMSplitAndAddTokens ( PTMX_SUBSTPROP,
                       PFUZZYTOK, PUSHORT, USHORT, SHORT,
                       USHORT, PSZ_W, SHORT, ULONG, PLOADEDTABLE );

USHORT NTMCompareContext
(
  EqfMemory* pTmClb,                     // ptr to ctl block struct
  PSZ         pszMarkup,               // ptr to name markup used for segment
  PSZ_W       pszContext1,             // context of first segment
  PSZ_W       pszContext2              // context of second segment
);


//USHORT FillMatchEntryEx
//( 
//  EqfMemory* pTmClb,
//  PTMX_SENTENCE pSentence,
//  PTMX_MATCHENTRY pMatchEntry,
//  PUSHORT pusMatchThreshold,
//  PTMX_INDEX_RECORD pIndexRecord 
//);


//typedef USHORT (APIENTRY *PFNCOMPCONTEXT)( PSZ_W, PSZ_W, PUSHORT );
typedef USHORT ( *PFNCOMPCONTEXT)( PSZ_W, PSZ_W, PUSHORT );

VOID SetAvailFlags( PTMX_GET_OUT_W pTmGetOut, USHORT usMatchesFound)
{
  // store indication if more matches are available
  if ( usMatchesFound > 1 )
  {
     USHORT   i;                                     // index value
     PTMX_MATCH_TABLE_W pstMT = &(pTmGetOut->stMatchTable[0]);

       // indicate if other exact matches are available
     pTmGetOut->fsAvailFlags |=
                   (pTmGetOut->stMatchTable[1].usMatchLevel >= EQUAL_EQUAL) ? GET_MORE_EXACTS_AVAIL : 0;
     // indicate if fuzzy matches are available
     for ( i=1; i < usMatchesFound ;i++ )
     {
       if ((pstMT+i)->usMatchLevel < EQUAL_EQUAL )
       {
         pTmGetOut->fsAvailFlags |= GET_ADDITIONAL_FUZZY_AVAIL;
         break;
       } /* endif */
     } /* endfor */
  }
}

// get match type 
OtmProposal::eMatchType GetMatchTypeFromMatchLevel( USHORT usMatchLevel )
{
  if ( usMatchLevel == 101 ) return( OtmProposal::emtExactSameDoc );; 
  if ( usMatchLevel == 100 ) return( OtmProposal::emtExact );
  if ( usMatchLevel == 102 ) return( OtmProposal::emtExactExact );
  return ( OtmProposal::emtFuzzy );
}

static OtmProposal::eProposalType GetProposalTypeFromFlag( USHORT usTranslationFlag )
{
  if ( usTranslationFlag == TRANSLFLAG_MACHINE ) return ( OtmProposal::eptMachine );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEM ) return ( OtmProposal::eptGlobalMemory );
  if ( usTranslationFlag == TRANSLFLAG_GLOBMEMSTAR ) return ( OtmProposal::eptGlobalMemoryStar );
  if ( usTranslationFlag == TRANSLFLAG_NORMAL ) return ( OtmProposal::eptManual );
  return ( OtmProposal::eptUndefined );
}


void NTMFreeSubstProp( PTMX_SUBSTPROP pSubstProp );
void NTMMakeTagTablename( PSZ pszMarkup, PSZ pszTagTableName );

BOOL NTMAlignTags( PFUZZYTOK pTokSource, PFUZZYTOK pTokTarget, PREPLLIST *ppReplaceList );

static BOOL NTMCheckAndDeleteTagPairs
(
  PTMX_SUBSTPROP pSubstProp,
  PREPLLIST pReplaceSourceList,         // list of same tokens in source and prop
  PREPLLIST pReplaceList,              // toklist to be replaced in prop src+tgt
  BOOL      fRespectLFs
);


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXGet      gets data to the tm database                |
//+----------------------------------------------------------------------------+
//|Description:       Gets data from tm data file - exact matches and fuzzies  |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXGet( EqfMemory* pTmClb,        //ptr to ctl block struct  |
//|                         PTMX_GET_IN pTmGetIn,   //ptr to input struct      |
//|                         PTMX_GET_OUT pTmGetOut ) //ptr to output struct    |
//+----------------------------------------------------------------------------+
//|Input parameter:   EqfMemory*  pTmClb         control block                   |
//|                   PTMX_GET_IN pTmGetIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_GET_OUT pTmGetOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     get id of source language, call                                        |
//|       NTMGetIdFromName, returns pointer to id                              |
//|                                                                            |
//|     using parameters from the Tm_Get structure passed                      |
//|     call TokenizeSegment with szTagTable, szSource, SourceLangId,          |
//|       TagTable record, pNormString                                         |
//|                                                                            |
//|     call HashSentence with pNormString                                     |
//|                                                                            |
//|     work through                                                           |
//|       checking if on in compact area in TM control block                   |
//|       if all hashes on                                                     |
//|         if only exact matches requested                                    |
//|           assume exact match so apply exact match test                     |
//|           if no matches found                                              |
//|             aplly the fuzzy match test                                     |
//|         else                                                               |
//|           first apply the exact match test and then the fuzzy match test   |
//|       else                                                                 |
//|         assume fuzzy match so apply fuzzy match test                       |
//|                                                                            |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::TmtXGet
(
  PTMX_GET_IN_W pTmGetIn,    //ptr to input struct
  PTMX_GET_OUT_W pTmGetOut   //ptr to output struct
)
{
  TMX_SENTENCE Sentence(std::make_shared<StringTagVariants>(pTmGetIn->stTmGet.szSource));      // ptr to sentence structure
  USHORT usRc = NO_ERROR;              // return code
  USHORT usOverlaps = 0;               // compact area triple hits
  CHAR szString[MAX_EQF_PATH];         // character string
  szString[0] = '\0';

  ULONG ulStrippedParm = pTmGetIn->stTmGet.ulParm &
    ~(GET_RESPECTCRLF | GET_IGNORE_PATH | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS | GET_IGNORE_COMMENT);

#ifdef MEASURETIME
  LONG64 lDummy = 0;
  fTimeLogging = TRUE;
  GetElapsedTime( &lDummy );
#endif


  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){    
      auto str_source = EncodingHelper::convertToUTF8(pTmGetIn->stTmGet.szSource );
      T5LOG(T5DEBUG) << "== Lookup in memory  ==" << stTmSign.szName <<" ==\nLookupSource = >>>" <<str_source << "<<<" ;
      auto str = EncodingHelper::convertToUTF8(pTmGetIn->stTmGet.szSource);
      T5LOG(T5INFO) << "-------------- Looking up:\n" <<  str << "\n" ;
  }

#ifdef MEASURETIME
  GetElapsedTime( &(lOtherTime) );
#endif


#ifdef MEASURETIME
  GetElapsedTime( &(lAllocTime) );
#endif

  if ( !usRc )
  {
    //build tag table path
    Properties::GetInstance()->get_value(KEY_OTM_DIR, szString, sizeof(szString));
    strcat( szString, "/TABLE/");
    strcat( szString, pTmGetIn->stTmGet.szTagTable ); 
    strcat( szString, EXT_OF_FORMAT );

    //remember start of norm string
    //pSentence->pNormStringStart = pSentence->pNormString;
    
   
    //tokenize source segment, resuting in normalized string and
    //tag table record
    usRc = TokenizeSource( this, &Sentence, szString, pTmGetIn->stTmGet.szSourceLanguage);

    // set the tag table ID in the tag record (this can't be done in TokenizeSource anymore)
    if ( usRc == NO_ERROR )
    {
      usRc = NTMGetIDFromName( this, pTmGetIn->stTmGet.szTagTable, NULL, (USHORT)TAGTABLE_KEY, &Sentence.pTagRecord->usTagTableId );
    }
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(lTokenizeTime) );
#endif

  if ( !usRc )
  {
    SHORT sRetries = MAX_RETRY_COUNT;
    {
      usRc = NO_ERROR;                 // reset return code

      if ( !usRc )
      {
          //Sentence.pNormString = Sentence.pNormStringStart;
          HashSentence( &Sentence );

#ifdef MEASURETIME
  GetElapsedTime( &(lOtherTime) );
#endif

          usOverlaps = CheckCompactArea( &Sentence, this );
          T5LOG( T5INFO) << "TmtXGet: Checked compact area, usOverlaps=" << usOverlaps;
          if ( usOverlaps == Sentence.usActVote ) //all hash triples found
          {
#ifdef MEASURETIME
            GetElapsedTime( &(lOtherTime) );
#endif
            if ( ulStrippedParm & GET_EXACT ) 
            {
              T5LOG( T5INFO) << "TmtXGet: Calling GetExactMatch" ;
              //get exact matches only
              usRc = GetExactMatch( this, &Sentence, &pTmGetIn->stTmGet,
                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound, pTmGetOut );

#ifdef MEASURETIME
              GetElapsedTime( &(lGetExactTime) );
#endif
              //if usNumMatchesFound is zero then try for fuzzies
              if ( (pTmGetOut->usNumMatchesFound == 0) && (usRc == NO_ERROR) )
              {                
                T5LOG( T5INFO) << "TmtXGet: No exact matches found, trying GetFuzzyMatch" ;
                usRc = GetFuzzyMatch( this, &Sentence, &pTmGetIn->stTmGet,

                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );
                T5LOG( T5INFO) << "TmtXGet: GetFuzzyMatch returned " << pTmGetOut->usNumMatchesFound << " matches" ;
              } /* endif */
#ifdef MEASURETIME
              GetElapsedTime( &(lGetFuzzyTime) );
#endif
            }
            else
            {
                T5LOG( T5INFO) << "TmtXGet: Calling GetFuzzyMatch" ;
                
                usRc = GetFuzzyMatch( this, &Sentence, &pTmGetIn->stTmGet,

                        pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );
                T5LOG( T5INFO) <<  "TmtXGet: GetFuzzyMatch returned " << pTmGetOut->usNumMatchesFound  << " matches" ;

#ifdef MEASURETIME
                GetElapsedTime( &(lGetFuzzyTime) );
#endif
            } /* endif */
          }
          else
          {
            BOOL fCheckForFuzzyMatches = FALSE;

            // check if the overlap is good enough...


            // GQ 2016/04/05: new approach: (tentative fix for P403177)
            //   sentences with up to 8 (and incl) votes: always check fuzzyness
            //   sentences from 9 up to 12 votes: check fuzziness when 20% of votes are matching
            //   sentences with 12 or more votes: check fuzziness when pTmGetIn->stTmGet.usMatchThreshold (=33%) of votes are matching

            // old code:
            //  if (( usOverlaps * 100 >= pSentence->usActVote * pTmGetIn->stTmGet.usMatchThreshold ) || (pSentence->usActVote <= 8))

            // GQ 2016/05/12: disabled new approach as it causes a 12% decrease in the overall analysis performance 
            if ( Sentence.usActVote <= 8 )
            {
              fCheckForFuzzyMatches = TRUE;
            }
            //else if ( pSentence->usActVote < 18 )
            //{
            //  fCheckForFuzzyMatches = (usOverlaps * 100) >= (pSentence->usActVote * 20);
            //}
            else 
            {
              fCheckForFuzzyMatches = (usOverlaps * 100) >= (Sentence.usActVote * pTmGetIn->stTmGet.usMatchThreshold);
            } /* endif */

            if ( fCheckForFuzzyMatches )
            {
              T5LOG(T5INFO) << "TmtXGet: usOverlaps= "<<usOverlaps<<
                  " ,usActVote=" << Sentence.usActVote << ", usMatchThreshold=" << pTmGetIn->stTmGet.usMatchThreshold;

              //not all triples on in compact area so fuzzy match
#ifdef MEASURETIME
            GetElapsedTime( &(lOtherTime) );
#endif
              T5LOG( T5INFO) << "TmtXGet: Calling GetFuzzyMatch" ;
              usRc = GetFuzzyMatch( this, &Sentence, &pTmGetIn->stTmGet, pTmGetOut->stMatchTable, &pTmGetOut->usNumMatchesFound );

              T5LOG( T5INFO) << "TmtXGet: GetFuzzyMatch returned "<<pTmGetOut->usNumMatchesFound<<" matches" ;
              
#ifdef MEASURETIME
              GetElapsedTime( &(lGetFuzzyTime) );
#endif
            } /* endif */
          } /* endif */
      } /* endif */
    }
  } /* endif */

  // store indication if more matches are available
  if ( pTmGetOut->usNumMatchesFound > 1 )
  {
     SetAvailFlags( pTmGetOut, pTmGetOut->usNumMatchesFound);
  }

  // if we have more than one exact match check if we have an exact match where the source
  // matchs exactly the source segment data
  if ( !usRc && (pTmGetOut->usNumMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT) )
  {
     PTMX_MATCH_TABLE_W pstMatchTable = &(pTmGetOut->stMatchTable[0]);
     int iNumExactSourceMatches = 0;   // number of matches having exact the same source
     int i;                            // index values
     int iIndexOfExactMatch = 0;       // index of exact match

     for ( i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
     {
       PTMX_MATCH_TABLE_W pstActMatch = &(pTmGetOut->stMatchTable[i]);
       if ( (wcscmp( pstActMatch->szSource, pTmGetIn->stTmGet.szSource ) == 0) && (pTmGetOut->stMatchTable[i].usTranslationFlag == TRANSLFLAG_NORMAL) )
       {
         iNumExactSourceMatches++;
         iIndexOfExactMatch = i;
       } /* endif */
     } /* endfor */

     // if we have exactly one exact match which exact matches the source segment we can discard
     // the other exact matches 
     if ( iNumExactSourceMatches == 1 )
     {
       for ( i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
       {
         if ( i != iIndexOfExactMatch )
         {
           // remove this match
           ULONG ulLen = ( MAX_MATCHES - i - 1 ) * sizeof(TMX_MATCH_TABLE_W);
           memmove( pstMatchTable+i, pstMatchTable+1+i, ulLen );
           pTmGetOut->usNumMatchesFound--;
           if ( i < iIndexOfExactMatch )
           {
             iIndexOfExactMatch--;  // adjust index of our exact match
           } /* endif */
           i--;                    // reset i because we deleted the i-th one
         } /* endif */
       } /* endfor */
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* check if we have more than one equal tgt match in our proposals  */
  /********************************************************************/
  if ( !usRc && (pTmGetOut->usNumMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT ))
  {
     PTMX_MATCH_TABLE_W pstMatchTable = &(pTmGetOut->stMatchTable[0]);
     PTMX_MATCH_TABLE_W pstActMatch;
     USHORT   i, j;                     // index values
     INT wsDiff;
     
     for ( j = 0 ; j < pTmGetOut->usNumMatchesFound - 1  ;j++ )
     {
       pstActMatch = &(pTmGetOut->stMatchTable[j]);
       for ( i=j+1; i< pTmGetOut->usNumMatchesFound ; i++)
       {
         //if (UtlCompIgnWhiteSpaceW(pstActMatch->szTarget, (pstMatchTable+i)->szTarget, 0, &wsDiff) == 0L && wsDiff == 0)
         if(wcscmp(pstActMatch->szTarget, (pstMatchTable+i)->szTarget) == 0)
         {
           /*************************************************************/
           /* delete this match...                                      */
           /*************************************************************/
           ULONG ulLen = ( MAX_MATCHES - i - 1 ) * sizeof(TMX_MATCH_TABLE_W);
           memmove( pstMatchTable+i, pstMatchTable+1+i, ulLen );
           pTmGetOut->usNumMatchesFound--;
           i--;                    // reset i because we deleted the i-th one
         } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endif */

  /********************************************************************/
  /* restrict number of matches found to the maximum number requested */
  /* by user                                                          */
  /********************************************************************/  
  pTmGetOut->usNumMatchesFound =  pTmGetOut->usNumMatchesFound < pTmGetIn->stTmGet.usRequestedMatches ?
               pTmGetOut->usNumMatchesFound : pTmGetIn->stTmGet.usRequestedMatches;

  { 
    int iTemp = sizeof( TMX_GET_OUT_W);
    pTmGetOut->stPrefixOut.usLengthOutput = (USHORT)(iTemp);
  }

  pTmGetOut->stPrefixOut.usTmtXRc = usRc;

  if ( usRc ) ERREVENT2( TMTXGET_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );

  // release access to any tag table loaded for segment markup
  // (may have been loaded for context handling)
  if ( pTagTable )
  {
    TAFreeTagTable( (PLOADEDTABLE)(pTagTable) );
    pTagTable = NULL;
  } /* endif */

#ifdef MEASURETIME
            GetElapsedTime( &(lOtherTime) );
#endif


  T5LOG( T5INFO) <<"TmtXGet::Lookup complete, found "<<pTmGetOut->usNumMatchesFound<<" matches" ;

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP))
  {
    for ( int i = 0; i < pTmGetOut->usNumMatchesFound; i++ )
    {
      PTMX_MATCH_TABLE_W pMatch = &(pTmGetOut->stMatchTable[i]);
      T5LOG( T5DEVELOP) << "TmtXGet:: Match "<< i << " , MatchLevel = "<< pMatch->usMatchLevel<< 
          " , MFlag = " << pMatch->usTranslationFlag << " ,  segment = " << pMatch->ulSegmentId;
      if(pMatch->szLongName[0] != '0') {
        T5LOG( T5DEVELOP)<<"TmtXGet:: origin = "<<pMatch->szFileName;
      }else{
        T5LOG( T5DEVELOP)<<"TmtXGet:: origin = "<<pMatch->szLongName;
      }

      auto strSource = EncodingHelper::convertToUTF8(pMatch->szSource);
      auto strTarget = EncodingHelper::convertToUTF8(pMatch->szTarget);
      T5LOG( T5DEVELOP) << "TmtXGet::\n  Source = >>>"<< strSource << "<<<\n  Target = >>>" <<  strTarget << "<<< ";
    } /* endfor */
  } /* endif */  

  return( usRc );
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetExactMatch                                            |
//+----------------------------------------------------------------------------+
//|Description:       determines the degree of exactness of a match            |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory* pTmClb                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT plMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT plMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               determine which tm record to extract                         |
//|               loop through the tm records                                  |
//|                 applying the exact match criteria                          |
// ----------------------------------------------------------------------------+

USHORT GetExactMatch
(
  EqfMemory* pTmClb,                     //ptr to ctl block struct
  PTMX_SENTENCE pSentence,             //ptr to sentence structure
  PTMX_GET_W pGetIn,                   //ptr to data in get in structure
  PTMX_MATCH_TABLE_W pstMatchTable,    //get out output structure
  PUSHORT pusMatchesFound,             //number of matches found
  PTMX_GET_OUT_W pTmGetOut             // pointer to output struct
)
{
  BOOL fOK = TRUE;                   //success indicator
  PULONG pulSids = NULL;             //ptr to sentence ids
  PULONG pulSidStart = NULL;         //ptr to sentence ids
  USHORT usRc = NO_ERROR;            //return code
  ULONG ulLen;                       //length indicator
  ULONG ulKey;                       //tm record key
  PTMX_RECORD pTmRecord = NULL;      //pointer to tm record
  USHORT   usMatchEntries = 0;         //nr of found matches
  ULONG  ulRecBufSize = 0;             // current size of record buffer

  ULONG ulStrippedParm = pGetIn->ulParm &
    ~(GET_RESPECTCRLF | GET_IGNORE_PATH | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS | GET_IGNORE_COMMENT);

  //allocate 32K for tm record
  if ( pTmClb->pvTmRecord )
  {
    pTmRecord = (PTMX_RECORD)pTmClb->pvTmRecord; 
    ulRecBufSize = pTmClb->ulRecBufSize;
//    memset( pTmRecord, 0, ulRecBufSize );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvTmRecord = pTmRecord;
    if ( fOK ) pTmClb->ulRecBufSize = ulRecBufSize = TMX_REC_SIZE;
  } /* endif */


  //allocate for sentence ids
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pulSids), 0L,
                             (LONG)((MAX_INDEX_LEN + 5) * sizeof(ULONG)),
                             NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    pulSidStart = pulSids;
    usRc = DetermineTmRecord( pTmClb, pSentence, pulSids );
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (*pulSids) && (usRc == NO_ERROR) )
      {
        ulKey = *pulSids;
        ulLen = TMX_REC_SIZE;
        T5LOG( T5INFO) << "GetExactMatch: EQFNTMGET of record" <<ulKey;
        usRc =  pTmClb->TmBtree.EQFNTMGet( ulKey, (PCHAR)pTmRecord, &ulLen );

        if ( usRc == BTREE_BUFFER_SMALL)
        {
          fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
          if ( fOK )
          {
            pTmClb->ulRecBufSize = ulRecBufSize = ulLen;
            pTmClb->pvTmRecord = pTmRecord;
            memset( pTmRecord, 0, ulLen );

            usRc =  pTmClb->TmBtree.EQFNTMGet( ulKey, (PCHAR)pTmRecord,
                              &ulLen );
          }
          else
          {
            LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
          } /* endif */
        } /* endif */
        if ( usRc == NO_ERROR )
        {
          //compare tm record data with data passed in the get in structure
          T5LOG( T5INFO) << "GetExactMatch: ExactTest of record " << ulKey;
          usRc = ExactTest( pTmClb, pTmRecord, pGetIn, pSentence,
                            pstMatchTable, &usMatchEntries, ulKey );
          T5LOG( T5INFO) << "GetExactMatch: ExactTest found " << usMatchEntries << " matching entries" ;
          
          if ( !usRc )
          {
            //nr of matches found
            (*pusMatchesFound) = usMatchEntries;

            //get next tm record
            pulSids++;
          } /* endif */
        } /* endif */
      } /* endwhile */
      /****************************************************************/
      /* check if we are only looking for an exact match and we really*/
      /* found more than one...                                       */
    /* store the availability of additional flags for later use     */
      /****************************************************************/
      SetAvailFlags( pTmGetOut, *pusMatchesFound);

      if ( !usRc && (*pusMatchesFound > 1 ) && (ulStrippedParm & GET_EXACT) )
      {
         /*************************************************************/
         /* there is one exact exact match available -  forget about  */
         /* any 'normal' matches                                      */
         /*************************************************************/
         if ( pstMatchTable->usMatchLevel >= EQUAL_EQUAL )
         {
           if ( pstMatchTable->usMatchLevel == (EQUAL_EQUAL + 2) )
           {
             /*********************************************************/
             /* we have an exact exact ...                            */
             /*********************************************************/
             *pusMatchesFound = 1;
           }
           else
           {
             /*********************************************************/
             /* we have two or more exact one's.                      */
             /*********************************************************/
             USHORT   i;                             // index value
             for ( i=1; i< *pusMatchesFound ;i++ )
             {
               if ((pstMatchTable+i)->usMatchLevel < EQUAL_EQUAL)
               {
                 *pusMatchesFound = i;
                 break;
               } /* endif */
             } /* endfor */
           } /* endif */
         } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( GETEXACTMATCH_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  return( usRc );
}


// helper function for searching text strings
PSZ_W GlobMemFind( PSZ_W pszText, PSZ_W pszFind )
{
  PSZ_W pszSearchPos = pszText;
  PSZ_W pszResult = NULL;
  int iFindLen = wcslen( pszFind );
  int iRemaining = wcslen( pszText );

  while ( (iFindLen <= iRemaining) && (pszResult == NULL ) )
  {
    if ( *pszSearchPos == *pszFind )
    {
      if ( wcsnicmp( pszSearchPos, pszFind, iFindLen ) == 0 )
      {
        pszResult = pszSearchPos;
      } /* endif */         
    } /* endif */

    if ( pszResult == NULL )
    {
      pszSearchPos++;
      iRemaining--;
    } /* endif */       
  } /* endwhile */     

  return( pszResult );
}

// get the flag for a global memory proposal based on the project-ID
GMMEMOPT GlobMemGetFlagForProposal( PVOID pCTIDList, PSZ_W pszAddData )
{
  BOOL fFound = FALSE;
  PGMOPTIONFILE pList = (PGMOPTIONFILE)pCTIDList;
  CHAR_W szProjectID[PROJECTID_SIZE+1];
  CHAR_W chICU = 0;
  GMMEMOPT optReturn = GM_HFLAG_OPT;
  const PSZ_W pszGlobMemKeyword = L"<GlobMem>";
  const PSZ_W pszGlobMemEndKeyword = L"</GlobMem>";
  const PSZ_W pszHamsterKeyword = L"<Hamster>";
  const PSZ_W pszHamsterEndKeyword = L"</Hamster>";
  const PSZ_W pszCTIDKeyword = L"<CTID>";
  const PSZ_W pszCTIDEndKeyword = L"</CTID>";

  // search CTID entry in additional data
  PSZ_W pszGlobMemGroup = GlobMemFind( pszAddData, pszGlobMemKeyword );
  if ( pszGlobMemGroup != NULL )
  { 
    pszGlobMemGroup += wcslen(pszGlobMemKeyword);
    PSZ_W pszGlobMemGroupEnd = GlobMemFind( pszGlobMemGroup, pszGlobMemEndKeyword );
    if ( pszGlobMemGroupEnd != NULL )
    {
      // copy global memory group to our buffer for further processing
      wcsncpy( pList->szBufferW, pszGlobMemGroup, pszGlobMemGroupEnd - pszGlobMemGroup ); 
       
      // search our CTID group
      PSZ_W pszCTID = GlobMemFind( pList->szBufferW, pszCTIDKeyword );
      if ( pszCTID != NULL )
      {
        pszCTID += wcslen(pszCTIDKeyword);
        PSZ_W pszCTIDEnd = GlobMemFind( pList->szBufferW, pszCTIDEndKeyword );
        if ( pszCTID != NULL )
        {
          // extract projectID and ICU
          *pszCTIDEnd = 0;
          if ( wcslen( pszCTID ) >= 11 )
          {
            fFound = TRUE;
            memset( szProjectID, 0, sizeof(szProjectID) );
            wcsncpy( szProjectID, pszCTID, PROJECTID_SIZE ); 
            chICU = pszCTID[7];
          } /* endif */             
        } /* endif */         
      } /* endif */         
    } /* endif */       
  } /* endif */     

  // search CTID entry in additional data using old name (Hamster)
  if ( !fFound ) 
  {
    PSZ_W pszGlobMemGroup = GlobMemFind( pszAddData, pszHamsterKeyword );
    if ( pszGlobMemGroup != NULL )
    { 
      pszGlobMemGroup += wcslen(pszHamsterKeyword);
      PSZ_W pszGlobMemGroupEnd = GlobMemFind( pszGlobMemGroup, pszHamsterEndKeyword );
      if ( pszGlobMemGroupEnd != NULL )
      {
        // copy global memory group to our buffer for further processing
        wcsncpy( pList->szBufferW, pszGlobMemGroup, pszGlobMemGroupEnd - pszGlobMemGroup ); 
       
        // search our CTID group
        PSZ_W pszCTID = GlobMemFind( pList->szBufferW, pszCTIDKeyword );
        if ( pszCTID != NULL )
        {
          pszCTID += wcslen(pszCTIDKeyword);
          PSZ_W pszCTIDEnd = GlobMemFind( pList->szBufferW, pszCTIDEndKeyword );
          if ( pszCTID != NULL )
          {
            // extract projectID and ICU
            *pszCTIDEnd = 0;
            if ( wcslen( pszCTID ) >= 11 )
            {
              fFound = TRUE;
              memset( szProjectID, 0, sizeof(szProjectID) );
              wcsncpy( szProjectID, pszCTID, PROJECTID_SIZE ); 
              chICU = pszCTID[7];
            } /* endif */             
          } /* endif */         
        } /* endif */         
      } /* endif */       
    } /* endif */     
  }

  // search entry for found CTID
  if ( fFound )
  {
    fFound = FALSE;
    LONG lEntry = 0;
    while ( !fFound && (lEntry < pList->lNumOfEntries) )
    {
      if ( (pList->Options[lEntry].chICU == chICU) && (wcsicmp( pList->Options[lEntry].szProjectID, szProjectID ) == 0 ) )
      {
        fFound = TRUE;
      }
      else
      {
        lEntry++;
      } /* endif */       
    } /* endwhile */  

    if ( fFound )
    {
      optReturn = pList->Options[lEntry].Option;
    } /* endif */       
  } /* endif */     

  return( optReturn );
}

#include <tm.h>

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ExactTest                                                |
//+----------------------------------------------------------------------------+
//|Description:       Fills the match table structure as required by the       |
//|                   getout structure                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory* pTmClb                                          |
//|                   PTMX_RECORD pTmRecord                                    |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               if source strings are equal                                  |
//|               loop through the target records in the tm record             |
//|                 if target languages are equal                              |
//|                   if source tag table records are equal                    |
//|                     equal match so fill match table                        |
//|                   else                                                     |
//|                     if source tag ids are equal                            |
//|                       equal fuzzy match with tags so fill match table      |
//|                     else                                                   |
//|                       equal fuzzy match with no tags so fill match table   |
// ----------------------------------------------------------------------------+
USHORT ExactTest
(
  EqfMemory* pTmClb,                  //ptr to ctl block struct
  PTMX_RECORD pTmRecord,            //pointer to tm record data
  PTMX_GET_W pGetIn,                //pointer to get in data
  PTMX_SENTENCE pSentence,          //pointer to sentence structure
  PTMX_MATCH_TABLE_W pstMatchTable, //get out structure to be filled
  PUSHORT pusMatchEntries,          //nr of entries in match entry structure
  ULONG   ulKeyNum                  // number of key record
)
{
  BOOL fOK;                            //success indicator
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_TARGET_CLB    pTMXTargetClb;    //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
  ULONG ulLen;                        //length indicator
  PSZ_W pString = NULL;                  //pointer to character string
  USHORT usRc = NO_ERROR;              //returned value from function
  USHORT usMatchLevel;                 //pointer to ushort value
  BOOL   fStringEqual = FALSE;         // indicator for string equal
  BOOL   fEqualFound = FALSE;          // tagging equal ??
  USHORT usEqual = 0;
  PSZ_W pContextBuffer = NULL;        //pointer to buffer for context processing
  USHORT usTargetTranslationFlag = (USHORT)-1;    // translation flag of target CLB (processed)
  PSZ pszDocName = NULL;             // document name to check for equal document names in GET_IGNORE_PATH mode

  //allocate pString and buffer for context processing (when necessary)
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  if ( fOK )
  {
    if ( pGetIn->szContext[0] || (pGetIn->pvGMOptList != NULL) )
    {
      fOK = UtlAlloc( (PVOID *) &(pContextBuffer), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
    } /* endif */       
  
  } /* endif */     

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

    //move pointer to corresponding position
    pByte = (PBYTE)(pTmRecord+1);
    pByte += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulLen = (RECLEN(pTMXSourceRecord) - sizeof(TMX_SOURCE_RECORD));

    //copy source string for later compare function
    ulLen = EQFCompress2Unicode( pString, pByte, ulLen );
    auto normalizedTmStr = std::make_unique<StringTagVariants>(pString);
    //compare source strings
    fStringEqual = UtlCompIgnWhiteSpaceW((PSZ_W)normalizedTmStr->getNormStr().c_str(), pSentence->pStrings->getNormStrC(), 0) == 0L;

    if ( fStringEqual )
    {
      USHORT  usTgtNum = 0;            // init target number
      LONG    lLeftTgtLen;            // remaining target length
      USHORT  usGetLang;               // id of target language
      USHORT  usGetFile;               // id of target file
      USHORT  usAlternateGetFile;      // alternate ID of target file

      USHORT  usOldMatches = *pusMatchEntries;


      //get id of target language in the getin structure
      if (NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage,
                              NULL,
                              (USHORT)LANG_KEY, &usGetLang,
                              0, NULL ))
      {
        usGetLang = 1;
      } /* endif */

      //get file name id of file name in the getin structure
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szFileName,
                               pGetIn->szLongName,
                               (USHORT)FILE_KEY, &usGetFile,
                               0,
                               &usAlternateGetFile ))
      {
        usGetFile = 1;
      } /* endif */

      if ( pGetIn->ulParm & GET_IGNORE_PATH )
      {
        // get pointer to document name without path 
        if ( pGetIn->szLongName[0] != EOS )
        {
          pszDocName = UtlGetFnameFromPath( pGetIn->szLongName );
          if ( pszDocName == NULL )
          {
            pszDocName = pGetIn->szLongName;
          } /* endif */             
        }
        else
        {
          pszDocName = pGetIn->szFileName;
        } /* endif */           
      } /* endif */           


      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/

      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );

      lLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      //position at first target record
      pByte = (PBYTE)(pTmRecord+1);
      pByte += RECLEN(pTMXSourceRecord);
      pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //source strings are identical so loop through target records
      while ( (usRc == NO_ERROR) && //(ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0)) )
                              ( lLeftTgtLen >= RECLEN(pTMXTargetRecord) ) && ( RECLEN(pTMXTargetRecord) > 0))
      {
        BOOL fTestCLB = TRUE;
        BOOL fMatchingDocName = FALSE;
        USHORT usContextRanking = 0;       // context ranking of this match


        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        usTgtNum++;            //  target number

        //assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );

        lLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //next check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

        //compare target language group IDs
        if ( pTmClb->psLangIdToGroupTable[pTMXTargetClb->usLangId] != pTmClb->psLangIdToGroupTable[usGetLang] )
        {
          fTestCLB = FALSE;
        } 

        // check for comments when requested
        if ( pGetIn->ulParm & GET_IGNORE_COMMENT )
        {
          if ( NtmFindInAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, L"<Note" ) )
          {
            T5LOG( T5INFO) << "::CLB with comment skipped" ;
            fTestCLB = FALSE;
          } 
        } 

        // compare target table IDs
        if ( fTestCLB )
        {
          //position at source tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at source tag table
          pByte += pTMXTargetRecord->usSourceTagTable;
          pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          //compare tag table records
          fStringEqual = true; // tag table should be always equal in t5memory
          //fStringEqual = memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
          //                       RECLEN(pTMXSourceTagTable) ) == 0;
          //if ( !fStringEqual )
          {
            /**********************************************************/
            /* if tagging record is unequal than we have some         */
            /* (slight) differences                                   */
            /* i.e. we will create a fully qualified string and try   */
            /* another compare...                                     */
            /**********************************************************/

            if ( fOK )
            {
              LONG lLenTmp = ulLen;       // len of pString in # of w's
              if ( !fOK )
              {
                LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
              }
              else
              {
                fStringEqual = FALSE;
                auto genericTagsTmSeg = std::make_unique<StringTagVariants>(pString);
                fStringEqual = (UtlCompIgnWhiteSpaceW(
                                              pSentence->pStrings->getNpReplStrC(), 
                                              genericTagsTmSeg->getNpReplStrC(),
                                              0 ) == 0 );
              } /* endif */
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            if (fStringEqual)
            {
              fEqualFound = TRUE;
              usEqual = EQUAL_EQUAL;
            }
            else
            {
              usEqual = EQUAL_EQUAL - 3;
            } /* endif */

            //extract data from tm record
            //target with tags(TRUE)
            //compare file name and segment id in control block
            //position at control block
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pByte += pTMXTargetRecord->usClb;
            pTMXTargetClb = (PTMX_TARGET_CLB)pByte;
            usTargetTranslationFlag = pTMXTargetClb->bTranslationFlag;

            // loop over CLBs and look for best matching entry
            {
              LONG lLeftClbLen;      // left CLB entries in CLB list
              PTMX_TARGET_CLB pClb;    // pointer for CLB list processing
              #define SEG_DOC_AND_CONTEXT_MATCH   8
              #define DOC_AND_CONTEXT_MATCH   7
              #define CONTEXT_MATCH           6
              #define SAME_SEG_AND_DOC_MATCH  5
              #define SAME_DOC_MATCH          4
              #define MULT_DOC_MATCH          3
              #define NORMAL_MATCH            2
              #define IGNORE_MATCH            1
              SHORT sCurMatch = 0;

              // loop over all target CLBs
              pClb = pTMXTargetClb;
              lLeftClbLen = RECLEN(pTMXTargetRecord) -
                             pTMXTargetRecord->usClb;
              while ( ( lLeftClbLen > 0 ) && (sCurMatch < SAME_SEG_AND_DOC_MATCH) )
              {
                T5LOG( T5DEBUG) << ":: lLeftClbLen = " << lLeftClbLen << "; sCurMatch = " << sCurMatch;
                USHORT usTranslationFlag = pClb->bTranslationFlag;
                USHORT usCurContextRanking = 0;       // context ranking of this match
                BOOL fIgnoreProposal = FALSE;

                // apply global memory option file on global memory proposals
                if ( pClb->bTranslationFlag == TRANSLFLAG_GLOBMEM )
                {
                  if ( (pGetIn->pvGMOptList != NULL) && pClb->usAddDataLen  )
                  {
                    USHORT usAddDataLen = NtmGetAddData( pClb, ADDDATA_ADDINFO_ID, pContextBuffer, MAX_SEGMENT_SIZE );   
                    if ( usAddDataLen )
                    {
                      GMMEMOPT GobMemOpt = GlobMemGetFlagForProposal( pGetIn->pvGMOptList, pContextBuffer );
                      switch ( GobMemOpt )
                      {
                        case GM_SUBSTITUTE_OPT: usTranslationFlag = TRANSLFLAG_NORMAL; break;
                        case GM_HFLAG_OPT     : usTranslationFlag = TRANSLFLAG_GLOBMEM; break;
                        case GM_HFLAGSTAR_OPT : usTranslationFlag = TRANSLFLAG_GLOBMEMSTAR; break;
                        case GM_EXCLUDE_OPT   : fIgnoreProposal = TRUE; break;
                      } /* endswitch */   
                    } /* endif */                   
                  } /* endif */                 
                  if ( pClb == pTMXTargetClb )
                  {
                    usTargetTranslationFlag = usTranslationFlag;
                  } /* endif */                       
                } /* endif */                 

                // check context strings (if any)
                if ( 
                  (!fIgnoreProposal) 
                    && pGetIn->szContext[0] 
                    && pClb->usAddDataLen )
                {
                  USHORT usContextLen = NtmGetAddData( pClb, ADDDATA_CONTEXT_ID, pContextBuffer, MAX_SEGMENT_SIZE );   
                  if ( usContextLen != 0 )
                  {
                    usCurContextRanking = NTMCompareContext( pTmClb, pGetIn->szTagTable, pGetIn->szContext, pContextBuffer );
                  } /* endif */                     
                } /* endif */

                // check for matching document names
                if ( pGetIn->ulParm & GET_IGNORE_PATH )
                {
                  // we have to compare the real document names rather than comparing the document name IDs
                  PSZ pszCLBDocName = NTMFindNameForID( pTmClb, &(pClb->usFileId), (USHORT)FILE_KEY ); 
                  if ( pszCLBDocName != NULL )
                  {
                    PSZ pszName = UtlGetFnameFromPath( pszCLBDocName );
                    if ( pszName == NULL )
                    {
                      pszName = pszCLBDocName;
                    } /* endif */             
                    fMatchingDocName = stricmp( pszName, pszDocName ) == 0;
                  }
                  else
                  {
                    // could not access the document name, we have to compare the document name IDs
                    fMatchingDocName = ((pClb->usFileId == usGetFile) || (pClb->usFileId == usAlternateGetFile));
                  } /* endif */                     
                }
                else
                {
                  // we can compare the document name IDs
                  fMatchingDocName = ((pClb->usFileId == usGetFile) || (pClb->usFileId == usAlternateGetFile));
                } /* endif */


                if ( fIgnoreProposal )
                {
                  if ( sCurMatch == 0 )
                  {
                    sCurMatch = IGNORE_MATCH;
                  } /* endif */                     
                }
                else if ( usCurContextRanking == 100  )
                {
                  if ( fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1)) )
                  {
                    if ( sCurMatch < SEG_DOC_AND_CONTEXT_MATCH )
                    {
                      sCurMatch = SEG_DOC_AND_CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                  }
                  else if ( fMatchingDocName )
                  {
                    if ( sCurMatch < DOC_AND_CONTEXT_MATCH )
                    {
                      sCurMatch = DOC_AND_CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                    else if ( sCurMatch == DOC_AND_CONTEXT_MATCH )
                    {
                      // we have already a match of this type so check if context ranking
                      if ( usCurContextRanking > usContextRanking )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } 
                      // use time info to ensure that latest match is used
                      else if ( usCurContextRanking == usContextRanking )
                      {
                        // GQ 2015-04-10 New approach: If we have an exact-exact match use this one, otherwise use timestamp for the comparism
                        BOOL fExactExactNewCLB = fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1));
                        BOOL fExactExactExistingCLB = ((pTMXTargetClb->usFileId == usGetFile) || (pTMXTargetClb->usFileId == usAlternateGetFile)) && 
                                                       (pTMXTargetClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pTMXTargetClb->ulSegmId <= (pGetIn->ulSegmentId + 1));
                        if ( fExactExactNewCLB && !fExactExactExistingCLB )
                        {
                          // use exact-exact CLB for match
                          pTMXTargetClb = pClb;   
                          usTargetTranslationFlag = usTranslationFlag;
                          usContextRanking = usCurContextRanking;
                        }
                        else if ( (fExactExactNewCLB == fExactExactExistingCLB) && (pClb->lTime > pTMXTargetClb->lTime) )
                        {
                          // use newer target CLB for match
                          pTMXTargetClb = pClb;   
                          usTargetTranslationFlag = usTranslationFlag;
                          usContextRanking = usCurContextRanking;
                        }
                      } /* endif */
                    } /* endif */
                  }
                  else
                  {
                    if ( sCurMatch < CONTEXT_MATCH )
                    {
                      sCurMatch = CONTEXT_MATCH;
                      pTMXTargetClb = pClb;   // use this target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    }
                    else if ( sCurMatch == CONTEXT_MATCH )
                    {
                      // we have already a match of this type so check if context ranking
                      if ( usCurContextRanking > usContextRanking )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } 
                      // use time info to ensure that latest match is used
                      else if ( (usCurContextRanking == usContextRanking) && (pClb->lTime > pTMXTargetClb->lTime) )
                      {
                        pTMXTargetClb = pClb;   // use newer target CLB for match
                        usTargetTranslationFlag = usTranslationFlag;
                        usContextRanking = usCurContextRanking;
                      } /* endif */
                    } /* endif */
                  } /* endif */                     
                }
                else if ( fMatchingDocName && (pClb->ulSegmId >= (pGetIn->ulSegmentId - 1)) && (pClb->ulSegmId <= (pGetIn->ulSegmentId + 1)) )
                {
                  // same segment from same document available
                  sCurMatch = SAME_SEG_AND_DOC_MATCH;
                  pTMXTargetClb = pClb;          // use this target CLB for match
                  usContextRanking = usCurContextRanking;
                  usTargetTranslationFlag = usTranslationFlag;
                }
                else if ( fMatchingDocName )
                {
                  // segment from same document available
                  if ( sCurMatch < SAME_DOC_MATCH )
                  {
                    sCurMatch = SAME_DOC_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  }
                  else if ( sCurMatch == SAME_DOC_MATCH )
                  {
                    // we have already a match of this type so
                    // use time info to ensure that latest match is used
                    if ( pClb->lTime > pTMXTargetClb->lTime )
                    {
                      pTMXTargetClb = pClb;   // use newer target CLB for match
                      usTargetTranslationFlag = usTranslationFlag;
                      usContextRanking = usCurContextRanking;
                    } /* endif */
                  } /* endif */
                }
                else if ( pClb->bMultiple )
                {
                  // multiple target segment available
                  if ( sCurMatch < MULT_DOC_MATCH )
                  {
                    // no better match yet
                    sCurMatch = MULT_DOC_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  } /* endif */
                } 
                else if ( usTranslationFlag == TRANSLFLAG_NORMAL )
                {
                  // a 'normal' memory match is available
                  if ( sCurMatch < NORMAL_MATCH )
                  {
                    // no better match yet
                    sCurMatch = NORMAL_MATCH;
                    pTMXTargetClb = pClb;   // use this target CLB for match
                    usTargetTranslationFlag = usTranslationFlag;
                    usContextRanking = usCurContextRanking;
                  } /* endif */
                } /* endif */

                // continue with next target CLB
                if ( sCurMatch < SAME_SEG_AND_DOC_MATCH )
                {
                  lLeftClbLen -= TARGETCLBLEN(pClb);
                  if (lLeftClbLen > 0)
                  {
                    usTgtNum++;
                    pClb = NEXTTARGETCLB(pClb);
                  }
                } /* endif */
              } /* endwhile */


              {
                BOOL fNormalMatch = (usTargetTranslationFlag == TRANSLFLAG_NORMAL) || 
                                    (usTargetTranslationFlag == TRANSLFLAG_GLOBMEM) || 
                                    (usTargetTranslationFlag == TRANSLFLAG_GLOBMEMSTAR);
                switch ( sCurMatch )
                {
                  case IGNORE_MATCH :
                    usMatchLevel = 0;
                    break;
                  case SAME_SEG_AND_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                    break;
                  case SEG_DOC_AND_CONTEXT_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1; // exact-exact match with matching context
                    break;
                  case DOC_AND_CONTEXT_MATCH :
                    if ( usContextRanking == 100 )
                    {
                      // GQ 2015/05/09: treat 100% context matches as normal exact matches
                      // usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                      usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    }
                    else
                    {
                      usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    } /* endif */                       
                    break;
                  case CONTEXT_MATCH :
                    if ( usContextRanking == 100 )
                    {
                      // GQ 2015/05/09: treat 100% context matches as normal exact context matches
                      // usMatchLevel = fNormalMatch ? usEqual+2 : usEqual-1;
                      // GQ 2016/10/24: treat 100% context matches as normal exact matches
                      usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    }
                    else
                    {
                      usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    } /* endif */                       
                    break;
                  case SAME_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    break;
                  case MULT_DOC_MATCH :
                    usMatchLevel = fNormalMatch ? usEqual+1 : usEqual-1;
                    break;
                  default :
                    usMatchLevel = fNormalMatch ? usEqual : usEqual-1;
                    break;
                } /* endswitch */
              }
            }
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

            /**********************************************************/
            /* store only exact matches...                            */
            /**********************************************************/
            if ( usMatchLevel >= 100 )
            {
              LONG lSourceLen = ulLen;

              // use CLB translation flag is ustargetTranslationFlag is not set
              if ( usTargetTranslationFlag == (USHORT)-1 ) usTargetTranslationFlag = pTMXTargetClb->bTranslationFlag;
              FillMatchTable( pTmClb, pString, &lSourceLen, pTMXTargetRecord,
                              pTMXTargetClb,
                              pstMatchTable, &usMatchLevel, TRUE,
                              pusMatchEntries, &pSentence->usActVote,
                              &pSentence->usActVote,
                              ulKeyNum, usTgtNum,
                              pGetIn,  pSentence->pTagRecord, usTargetTranslationFlag, usContextRanking, -1, -1, pTMXSourceRecord->usLangId );
            }
            else
            {
              /********************************************************/
              /* don't treat  MT matches as exact ones..              */
              /********************************************************/
            } /* endif */
          } /* endif */
        } /* endif */

        //position at next target
        pByte = pStartTarget;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        //move pointer to end of target
        pByte += RECLEN(pTMXTargetRecord);
        //remember the end/beginning of record
        pStartTarget = pByte;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      }  /* endwhile */

      /****************************************************************/
      /* check if we found an exact match -- if not we only use best  */
      /* fuzzy                                                        */
      /****************************************************************/
      if ( !fEqualFound )
      {
        /**************************************************************/
        /* use only the best one...                                   */
        /**************************************************************/
        if (*pusMatchEntries > usOldMatches )
        {
          *pusMatchEntries = usOldMatches + 1;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  if ( pContextBuffer ) UtlAlloc( (PVOID *)&pContextBuffer, 0L, 0L, NOMSG );

  return( usRc );
}

#if defined(SGMLDITA_LOGGING) || defined(INLINE_TAG_REPL_LOGGING) || defined(MATCHLIST_LOG)
  void NTMMarkCRLF( PSZ_W pszSource, PSZ_W pszTarget )
  {
    while ( *pszSource )
    {
      if ( *pszSource == L'\n' )
      {
        wcscpy( pszTarget, L"<lf>" );
        pszTarget += 4;
      }
      else if ( *pszSource == L'\r' )
      {
        wcscpy( pszTarget, L"<cr>" );
        pszTarget += 4;
      }
      else 
      {
        *pszTarget++ = *pszSource;
      } /* endif */
      pszSource++;
    } /*endwhile */
    *pszTarget = 0;
  }


  void NTMLogSegData( PSZ_W pszForm, PSZ_W pszSegData )
  {
    static CHAR_W szSegBuf[4096];
    static CHAR_W szLineBuf[4096];
    int iLen;

    NTMMarkCRLF( pszSegData, szSegBuf );
    //iLen = swprintf( szLineBuf, pszForm, szSegBuf );
    T5LOG( T5INFO) << "; szLineBuf = " << szLineBuf << "; pszForm" <<  pszForm <<"; pszForm = " << szSegBuf ;
    //fwrite( szLineBuf, 2, iLen, hfLog ); 
  }

// removed superfluos whitespace at the beggining or end of the proposal
void NTMRemoveAdditionalWhitepace
(
  PSZ_W         pszSource, 
  PSZ_W         pszProposal 
)
{
  int           iLen;

  // only remove leading whitespace if source has none
  if ( (*pszSource != L'\n') && (*pszSource != L'\r') && (*pszSource != L' ') )
  {
    PSZ_W pszProp = pszProposal;
    PSZ_W pszTemp = pszProposal;

    while ( (*pszTemp == L'\n') || (*pszTemp == L'\r') || (*pszTemp == L' ') )
    {
      pszTemp++;
    } /*endwhile */

    if ( pszTemp != pszProp )
    {
      while ( *pszTemp )
      {
        *pszProp++ = *pszTemp++;
      } /*endwhile */
      *pszProp = 0;
    } /* endif */
  } /* endif */

  // check trailing whitespace
  iLen = wcslen( pszSource );
  if ( iLen )
  {
    iLen--;
    if  ( (pszSource[iLen] != L'\n') && (pszSource[iLen] != L'\r') && (pszSource[iLen] != L' ') )
    {
      iLen = wcslen( pszProposal );
      while ( iLen &&  ( (pszProposal[iLen-1] == L'\n') || (pszProposal[iLen-1] == L'\r') || (pszProposal[iLen-1] == L' ') ) )
      {
        iLen--;
        pszProposal[iLen] = 0;
      } /*endwhile */
    } /* endif */
  } /* endif */
} /* end of function NTMRemoveAdditionalWhitepace */

#endif 



BOOL TMFuzzynessEx
(
  PSZ pszMarkup,                       // markup table name
  PSZ_W pszSource,                     // original string
  PSZ_W pszMatch,                      // found match
  SHORT sLanguageId,                   // language id to be used
  PUSHORT     pusFuzzy,                // fuzzyness
  ULONG       ulOemCP,
  PUSHORT     pusWords,                // number of words in segment
  PUSHORT     pusDiffs                 // number of diffs in segment
)
{
  BOOL  fOK;
  PFUZZYTOK    pFuzzyTgt = NULL;
  PFUZZYTOK    pFuzzyTok = NULL;       // returned token list
  PSZ          pInBuf = NULL;          // buffer for EQFBFindDiffEx function
  PSZ          pTokBuf = NULL;         // buffer for EQFBFindDiffEx function
  PLOADEDTABLE pTable = NULL;          // ptr to loaded tag table


  // fast exit if one or both strings are empty...
  if ( (*pszSource == EOS) || (*pszMatch == EOS) )
  {
    if ( (*pszSource == EOS) && (*pszMatch == EOS) )
    {
      *pusFuzzy = 100;
      *pusDiffs = 0;
      *pusWords = 0;
    }
    else
    {
      *pusFuzzy = 0;
      *pusDiffs = 0;
      *pusWords = 0;
    } /* endif */
    return( TRUE );
  } /*   endif */

  // allocate required buffers
  fOK = UtlAlloc((PVOID *)&pInBuf, 0L, 64000L, NOMSG );
  if ( fOK ) fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, 64000 /*(LONG)TOK_BUFFER_SIZE*/, NOMSG );

  // load tag table
  if ( fOK )
  {
    fOK = (TALoadTagTableExHwnd( pszMarkup, &pTable, FALSE,
                                 TALOADUSEREXIT | TALOADPROTTABLEFUNC,
                                 FALSE, NULLHANDLE ) == NO_ERROR);
  } /* endif */

  // call function to evaluate the differences
  if ( fOK )
  {
    fOK = EQFBFindDiffEx( pTable, (PBYTE)pInBuf, (PBYTE)pTokBuf, pszSource,
                          pszMatch, sLanguageId, (PVOID *)&pFuzzyTok,
                          (PVOID *)&pFuzzyTgt, ulOemCP );
  } /* endif */

  if ( fOK )
  {
    EQFBCountDiff( pFuzzyTok, pusWords, pusDiffs );
  } /* endif */

  // free allocated buffers and lists
  if ( pFuzzyTgt ) UtlAlloc( (PVOID *)&pFuzzyTgt, 0L, 0L, NOMSG );
  if ( pFuzzyTok ) UtlAlloc( (PVOID *)&pFuzzyTok, 0L, 0L, NOMSG );
  if ( pInBuf )    UtlAlloc( (PVOID *)&pInBuf, 0L, 0L, NOMSG );
  if ( pTokBuf )   UtlAlloc( (PVOID *)&pTokBuf, 0L, 0L, NOMSG );
  if ( pTable )    TAFreeTagTable( pTable );

  if(*pusWords > *pusDiffs){
    *pusFuzzy = (*pusWords != 0) ? ((*pusWords - *pusDiffs) * 100 / *pusWords) : 100;
    T5LOG( T5INFO) <<"TMFuzzynessEx::\n Fuzzy = (Words != 0) ? ((Words - Diff) * 100 / Words) : 100;\n Words = " 
          << *pusWords<< ";\n Diff  = " <<*pusDiffs <<";\n Fuzzy = " <<*pusFuzzy<< ";\n";
  }else{
    *pusFuzzy = 0;
    T5LOG( T5INFO) <<"TMFuzzynessEx::\n Fuzzy: Words <= Diff:\n Words = " 
            <<*pusWords<< ";\n Diff  = "<<*pusDiffs<<";\n Fuzzy = "<<*pusFuzzy<< ";\n";
  }
  return fOK;
} /* end of function TMFuzzyness */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillMatchTable                                           |
//+----------------------------------------------------------------------------+
//|Description:       Fills the match table structure as required by the       |
//|                   getout structure                                         |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory* pTmClb                                          |
//|                   PSZ pSourceString                                        |
//|                   PUSHORT pusSourceLen                                     |
//|                   PTMX_TARGET_RECORD pTMXTargetRecord                      |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchLevel                                    |
//|                   BOOL fTag                                                |
//|                   PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchEntries                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|               determine the correct position in match table                |
//|               fill in all TMX_MATCH_TABLE field needed in the pGetOut      |
//|                structure                                                   |
//|               output number of entries in match table                      |
// ----------------------------------------------------------------------------+

USHORT FillMatchTable( EqfMemory* pTmClb,         //ptr to ctl block struct
                   PSZ_W pSourceString,         //pointer to normalized source string
                   PLONG plSourceLen,        //length of source string
                   PTMX_TARGET_RECORD pTMXTargetRecord, //pointer to tm target
                   PTMX_TARGET_CLB pTMXTargetClb,    //ptr to target control block
                   PTMX_MATCH_TABLE_W pstMatchTable, //getout match table struct
                   PUSHORT pusMatchLevel,       //how good is match
                   BOOL    fTag,                //target with/without subst. tags
                   PUSHORT pusMatchEntries,     //nr of entries in match table
                   PUSHORT pusMaxVotes,         //nr of tm source string votes
                   PUSHORT pusOverlaps,         //nr of overlapping triples
                   ULONG   ulKeyNum,            // key number
                   USHORT  usTgtNum,            // target number
                   PTMX_GET_W pGetIn,           // input structure
                   PTMX_TAGTABLE_RECORD pTagRecord,
                   USHORT usTranslationFlag,
                   USHORT usContextRanking,
                   int iWords, int iDiffs, USHORT usSrcLangId )
{
  PSZ    pSrcFileName = pGetIn->szFileName;  // source file name
  PSZ    pSrcLongName = pGetIn->szLongName;  // long source file name
  PBYTE  pByte;                              //position pointer
  BOOL   fOK;                                //success indicator
  USHORT usRc = NO_ERROR;                    //return code
  LONG  lTargetLen;                          //length indicator
  LONG  ulLen;                               //length indicator
  PSZ_W pString = NULL;                      //pointer to temp string
  USHORT usCurrentEntry;                     //counter
  BOOL fFound = FALSE;                       //indicates if correct pos found
  PTMX_SUBSTPROP pSubstProp = NULL;              // tag substitution array
  BOOL           fSubstAll = FALSE;
  PTMX_MATCH_TABLE_W pstMatchTableStart = pstMatchTable; //point to start of match table
  ULONG          ulSrcOemCP = 0L;
  ULONG          ulTgtOemCP = 0L;
  TMManager *pFactory = TMManager::GetInstance();
  SHORT sLangID = 0;

  // force fTag flag if proposal with inline tagging is requested
  if ( pGetIn->ulParm & GET_ALWAYS_WITH_TAGS )
  {
    fTag = TRUE;
  } /* endif */

  /********************************************************************/
  /* if tagtables are different, call substitution function           */
  /********************************************************************/
  if ( !(pGetIn->ulParm & GET_NO_GENERICREPLACE) )
  {
    fOK = UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG);

    if ( !fOK )
    {
      LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
    }
    else
    {
      /****************************************************************/
      /* add current tagging to source and translation of proposal    */
      /****************************************************************/
      ulSrcOemCP = 1;
      strcpy( pSubstProp->szSourceTagTable, pGetIn->szTagTable );
      strcpy( pSubstProp->szSourceLanguage, pGetIn->szSourceLanguage);
      strcpy( pSubstProp->szTargetLanguage, pGetIn->szTargetLanguage );
      UTF16strcpy( pSubstProp->szSource, pGetIn->szSource );
      pSubstProp->pTagsSource = pTagRecord;
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usSourceTagTable;

      MorphGetLanguageID( pGetIn->szSourceLanguage, &sLangID );

      pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pByte;
      //usRc = (AddTagsToStringW( pSourceString,
      //                        plSourceLen,      // in # of w's
      //                       (PTMX_TAGTABLE_RECORD)pByte,
      //                       pSubstProp->szPropSource )) ? usRc : BTREE_CORRUPTED;
        //instead of lines above we now save not normalize string to btree
        wcsncpy(pSubstProp->szPropSource, pSourceString, *plSourceLen);
      if ( usRc == NO_ERROR )
      {
        PSZ_W  pTarget;

        //position at target string
        pByte = (PBYTE)pTMXTargetRecord;
        pByte += pTMXTargetRecord->usTarget;

        //calculate length of target string
        lTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

        if ( UtlAlloc( (PVOID *) &pTarget, 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG))
        {
          lTargetLen = EQFCompress2Unicode(pTarget, pByte, lTargetLen);
          //position at target tag record
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTargetTagTable;

          pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pByte;

          //usRc = (AddTagsToStringW( pTarget,
          //                          &lTargetLen,     // in # of w's
          //                          (PTMX_TAGTABLE_RECORD)pByte,
          //                          pSubstProp->szPropTarget )) ? usRc : BTREE_CORRUPTED;      
         //instead of lines above we now save not normalize string to btree
            wcsncpy(pSubstProp->szPropTarget, pTarget, lTargetLen);
          UtlAlloc( (PVOID *) &pTarget, 0L, 0L, NOMSG );
        }
        else
        {
          LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
        }
        if ( usRc == NO_ERROR )
        {
          //fill in the markup table
          PBYTE p = ((PBYTE)pTMXTargetRecord)+pTMXTargetRecord->usTargetTagTable;
          NTMGetNameFromID( pTmClb,
                            &(((PTMX_TAGTABLE_RECORD)p)->usTagTableId),
                            (USHORT)TAGTABLE_KEY,
                            pSubstProp->szPropTagTable, NULL );
          ulTgtOemCP = 1;

          //fill in the target language
          NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId,
                            (USHORT)LANG_KEY,
                            pSubstProp->szTargetLanguage, NULL );
          //NTMGetNameFromID( pTmClb, &usSrcLangId,
          //                    (USHORT)LANG_KEY,
          //                    pSubstProp->szOriginalSrcLanguage, NULL );
        } /* endif */

      } /* endif */
    } /* endif */
  }
  else
  {
  } /* endif */

  /********************************************************************/
  /* if src of prop is empty ( i.e. only inline tagging), add tagging */
  /********************************************************************/
  if (*plSourceLen <= 0 )
  {
    fTag = TRUE;
  } /* endif */

  if ( usRc == NO_ERROR )
  {

    //locate correct position in matchtable array
    usCurrentEntry = 0;
    while ( !fFound && usCurrentEntry < MAX_MATCHES )
    {
      BOOL fEndOfTable = (usCurrentEntry + 1) >= MAX_MATCHES;

      int iMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTable->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTable->usTranslationFlag ), (int)pstMatchTable->usMatchLevel , -1, 
                          pstMatchTable->usContextRanking, fEndOfTable );
      int iNewMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( *pusMatchLevel ), GetProposalTypeFromFlag( usTranslationFlag ), 
                             (int)*pusMatchLevel, -1, usContextRanking, fEndOfTable );

      if ( iMatchSortKey  > iNewMatchSortKey  )
      {
        pstMatchTable++;
        usCurrentEntry++;
      }
      else if ( iMatchSortKey == iNewMatchSortKey )
      {
        CHAR szTgtFileName[ MAX_FILESPEC ];                  // target file name
        static CHAR szTgtLongName[ MAX_LONGFILESPEC ];       // long target file name

        //fill in the target file name
        NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId, (USHORT)FILE_KEY, szTgtFileName, szTgtLongName );

        /****************************************************************/
        /* as the matching levels are the same determine the most recent*/
        /* one, but have in mind that proposals from the same file have */
        /* higher priority                                              */
        /****************************************************************/
        
        if ( NTMDocMatch( szTgtFileName, szTgtLongName, pSrcFileName, pSrcLongName ) )
        {
          /**************************************************************/
          /* if compared match is from same file, check the update time */
          /* because now we deal with two matches from same source doc. */
          /**************************************************************/
          if ( (pstMatchTable->lTargetTime > pTMXTargetClb->lTime) &&
               NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName,
                            pSrcFileName, pSrcLongName ) )
          {
            pstMatchTable++;
            usCurrentEntry++;
          }
          else
          {
            //found the right position
              ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
              memmove( pstMatchTable+1, pstMatchTable, ulLen );
              memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
              fFound = TRUE;
          } /* endif */
        }
        else
        {
          /**************************************************************/
          /* if compared match is from other file, check update time    */
          /* otherwise put in our match, it's coming from same source   */
          /* document                                                   */
          /**************************************************************/
          if ( NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName, pSrcFileName, pSrcLongName ) )
          {
            // match from match table is from same document, so skip to next position
            pstMatchTable++;
            usCurrentEntry++;
          }
          else if ( pstMatchTable->lTargetTime > pTMXTargetClb->lTime )
          {
            // match from match table is newer
            pstMatchTable++;
            usCurrentEntry++;
          }
          else
          {
            //found the right position
              ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
              memmove( pstMatchTable+1, pstMatchTable, ulLen );
              memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
              fFound = TRUE;
          } /* endif */
        } /* endif */

      }
      else
      {
        //found the right position
        ULONG ulLen = ( MAX_MATCHES - usCurrentEntry - 1 ) * sizeof(TMX_MATCH_TABLE_W);
        memmove( pstMatchTable+1, pstMatchTable, ulLen );
        memset( pstMatchTable, 0, sizeof(TMX_MATCH_TABLE_W) );
        fFound = TRUE;
      } /* endif */
    } /* endwhile */

    if ( fFound )
    {
      //allocate pString
      fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

      if ( !fOK )
      {
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
      }
      else
      {
        pByte = (PBYTE)pTMXTargetRecord;
        pByte += pTMXTargetRecord->usSourceTagTable;
        if (fSubstAll )
        {
          /**************************************************************/
          /* do a substitution of the tags...                           */
          /**************************************************************/
          *plSourceLen = UTF16strlenCHAR( pSubstProp->szPropSource );
          if( *plSourceLen ) {
            memcpy( pstMatchTable->szSource, pSubstProp->szPropSource,
                  *plSourceLen * sizeof(CHAR_W));
          }
        }
        else
        {
          if (false // segments after 0.4.52 would be saved as is 
              && fTag 
          )
          {
            /************************************************************/
            /* add tags to source string                                */
            /************************************************************/
            fOK = AddTagsToStringW( pSourceString, plSourceLen,
                                   (PTMX_TAGTABLE_RECORD)pByte,
                                   pstMatchTable->szSource );
            if ( !fOK )
            {
              LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
            } /* endif */
          }
          else
          {
            //else copy normalized propsource string
            if(*plSourceLen > 0 ){
              memcpy( pstMatchTable->szSource, pSourceString, *plSourceLen * sizeof(CHAR_W));
            }
          } /* endif */
        } /* endif */

        if ( usRc == NO_ERROR )
        {
          //position at target string
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTarget;

          //calculate length of target string
          lTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

          //copy target string for later compare function
          lTargetLen = EQFCompress2Unicode( pString, pByte, lTargetLen );
          // now ulTargetLen is # of w's
          //position at target tag record
          pByte = (PBYTE)pTMXTargetRecord;
          pByte += pTMXTargetRecord->usTargetTagTable;

          if (fSubstAll )
          {
            /**************************************************************/
            /* do a substitution of the replaced target string and        */
            /* adjust match level                                         */
            /**************************************************************/
            lTargetLen = UTF16strlenCHAR( pSubstProp->szPropTarget );
            memcpy( pstMatchTable->szTarget, pSubstProp->szPropTarget,
                    lTargetLen * sizeof(CHAR_W));

            ulLen = UTF16strlenCHAR(pSubstProp->szSource);
            *plSourceLen = UTF16strlenCHAR(pSubstProp->szPropSource);
            if (*plSourceLen > ulLen )
            {
              ulLen = *plSourceLen;
            } /* endif */

            {
              BOOL fEqual;
              USHORT usFuzzy = 0, usWords = 0, usDiffs = 0;
              BOOL fFuzzynessOK;

/*Q!*/        fFuzzynessOK = TMFuzzynessEx( pGetIn->szTagTable,
                                          pSubstProp->szPropSource, pSubstProp->szSource,
                                          sLangID, &usFuzzy, ulSrcOemCP, &usWords, &usDiffs );

              if ( pGetIn->ulParm & GET_RESPECTCRLF )
              {
                fEqual = (UtlCompIgnSpaceW( pSubstProp->szPropSource,
                                           pSubstProp->szSource, 0 ) == 0);
              }
              else
              {
                fEqual = (usFuzzy == 100);
//                  (UtlCompIgnWhiteSpace( pSubstProp->szPropSource,
//                                                pSubstProp->szSource,
//                                                ulLen) == 0);

              } /* endif */

              // GQ 2004/08/04: do not set match level to 100 for machine translations!!
              // RJ 2004/08/16: assure that *pusMatchLevel is filled independently
              // of bMT flag set or not (P020111)
              if ( fEqual && (usFuzzy == 100) /*&& !pTMXTargetClb->bMT */)
              {
                *pusMatchLevel = (usTranslationFlag  == TRANSLFLAG_MACHINE) ? 99 : 100;
              } /* endif */
#ifdef SGMLDITA_LOGGING    
            T5LOG(T5INFO) << "new match level is "<< *pusMatchLevel <<", fuzziness is " << usFuzzy ;
#endif
            }
          }
          else
          {
            if ( false && // segments after 0.4.52 would be saved as is 
             fTag 
            )
            {
              //add tags to target string if flag set to true
              fOK = AddTagsToStringW( pString, &lTargetLen,
                                     (PTMX_TAGTABLE_RECORD)pByte,
                                     pstMatchTable->szTarget );
              if ( !fOK )
              {
                LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
              } /* endif */
            }
            else
            {
              //else copy normalized target string
              memcpy( pstMatchTable->szTarget, pString, lTargetLen * sizeof(CHAR_W));
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            //position at target control block
            // should already be done!!!
  //          pByte = (PBYTE)pTMXTargetRecord;
  //          pByte += pTMXTargetRecord->usClb;
  //          pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

            //fill in the target file name
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usFileId,
                              (USHORT)FILE_KEY,
                              pstMatchTable->szFileName,
                              pstMatchTable->szLongName );
            //fill in the target author - don't care about author name....
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usAuthorId,
                              (USHORT)AUTHOR_KEY,
                              pstMatchTable->szTargetAuthor, NULL );

            //fill in the markup table
            {
              PBYTE p = ((PBYTE)pTMXTargetRecord)+pTMXTargetRecord->usTargetTagTable;
              NTMGetNameFromID( pTmClb,
                                &(((PTMX_TAGTABLE_RECORD)p)->usTagTableId),
                                (USHORT)TAGTABLE_KEY,
                                pstMatchTable->szTagTable, NULL );
            }

            //fill in the target and original src language
            NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId,
                              (USHORT)LANG_KEY,
                              pstMatchTable->szTargetLanguage, NULL );

            NTMGetNameFromID( pTmClb, &usSrcLangId,
                              (USHORT)LANG_KEY,
                              pstMatchTable->szOriginalSrcLanguage, NULL );
            //fill in the segment id
            pstMatchTable->ulSegmentId = pTMXTargetClb->ulSegmId;
            //state whether machine translation
            pstMatchTable->usTranslationFlag = usTranslationFlag ;
            //fill in target time
            pstMatchTable->lTargetTime = pTMXTargetClb->lTime;
            //fill in match percentage
            pstMatchTable->usMatchLevel = *pusMatchLevel;
            pstMatchTable->iWords = iWords;
            pstMatchTable->iDiffs = iDiffs;
            //fill in nr of overlapping triples
            pstMatchTable->usOverlaps = *pusOverlaps;
            pstMatchTable->usContextRanking = usContextRanking;


            pstMatchTable->szContext[0] = 0;
            pstMatchTable->szAddInfo[0] = 0;
            if ( pTMXTargetClb->usAddDataLen )
            {
              NtmGetAddData( pTMXTargetClb, ADDDATA_CONTEXT_ID, pstMatchTable->szContext, sizeof(pstMatchTable->szContext) / sizeof(CHAR_W) );
              NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pstMatchTable->szAddInfo, sizeof(pstMatchTable->szAddInfo) / sizeof(CHAR_W) );
            } /* endif */

            /****************************************************************/
            /* fill in key and target number ...                            */
            /****************************************************************/
            pstMatchTable->ulKey = ulKeyNum;
            pstMatchTable->usTargetNum = usTgtNum;


            // check if after generic inline substitution the quality of the match
            // is still at the right place in the match table.
            while ( (pstMatchTableStart < pstMatchTable) )
            {
              PTMX_MATCH_TABLE_W pstMatchTableTemp = pstMatchTable-1;
              BOOL fEndOfTable = FALSE;

              
              int iMatchSortKey = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTable->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTable->usTranslationFlag ), 
                  (int)pstMatchTable->usMatchLevel , -1, pstMatchTable->usContextRanking, fEndOfTable );
              int iMatchSortKeyTemp = pFactory->getProposalSortKey( GetMatchTypeFromMatchLevel( pstMatchTableTemp->usMatchLevel ), GetProposalTypeFromFlag( pstMatchTableTemp->usTranslationFlag ), 
                  (int)pstMatchTableTemp->usMatchLevel, -1, pstMatchTableTemp->usContextRanking, fEndOfTable );

              if ((iMatchSortKeyTemp <  iMatchSortKey) ||
                  ((iMatchSortKeyTemp == iMatchSortKey) &&
                   (pstMatchTableTemp->lTargetTime  <   pstMatchTable->lTargetTime) &&
                    NTMDocMatch( pstMatchTable->szFileName, pstMatchTable->szLongName,
                                 pstMatchTableTemp->szFileName, pstMatchTableTemp->szLongName)) )
              {
                // interchange entries pstMatchTable moved on position pstMatchTableTemp
                TMX_MATCH_TABLE_W stMatchEntry;
                memcpy( &stMatchEntry, pstMatchTableTemp, sizeof( stMatchEntry ));
                memcpy( pstMatchTableTemp, pstMatchTable, sizeof( stMatchEntry ));
                memcpy( pstMatchTable, &stMatchEntry,     sizeof( stMatchEntry ));
                pstMatchTable = pstMatchTableTemp;
              }
              else
              {
                // leave loop
                pstMatchTableStart = pstMatchTable;
              }
            }

            //update match entry structure counter
            if ( *pusMatchEntries < MAX_MATCHES )
            {
              (*pusMatchEntries)++;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */

      //release memory
      UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    ERREVENT2( FILLMATCHTABLE_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  /********************************************************************/
  /* free any allocated resource                                      */
  /********************************************************************/
  NTMFreeSubstProp( pSubstProp );

  return( usRc );
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddTagsToString                                          |
//+----------------------------------------------------------------------------+
//|Description:       Add tags to correct position of the normalized string    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pInString                                            |
//|                   PUSHORT ousInStringLen                                   |
//|                   PTMX_TAGTABLE_RECORD pTMXTagTable                        |
//|                   PSZ pOutString                                           |
//+----------------------------------------------------------------------------+
//|Output parameter:  PSZ pOutString                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     add tags at the given offsets and output string with tags|
// ----------------------------------------------------------------------------+

BOOL AddTagsToStringW( PSZ_W pInString,            //character string
                  PLONG plInStringLen,       //length of pInString in # of w's
                  PTMX_TAGTABLE_RECORD pTMXTagTable,  //ptr to tag table record
                  PSZ_W pOutString )                    //output string
{
  PBYTE pByte;                         //record position pointer
  USHORT usStringPos, j;               //positioning counters
  PTMX_TAGENTRY pTagEntry = NULL;      //ptr to tag entries in tag table record
  ULONG  ulTagLen = 0;                 //length of all tags added to string
  ULONG  ulEnd = 0;                    //end length for for loop
  USHORT usTagRecordPos = 0;           //current length processed in tag record
  BOOL fNoTagsLeft = FALSE;            //indication of existence of tags
  ULONG  ulInputStringLen = *plInStringLen;
  PSZ_W  pOutStart = pOutString;       // anchar start pointers
  PSZ_W  pInStart  = pInString;        //
  BOOL   fOK = TRUE;
  //position at beginning of target record
  pByte = (PBYTE)pTMXTagTable;
  pByte += pTMXTagTable->usFirstTagEntry;

  fOK = (*plInStringLen < MAX_SEGMENT_SIZE );

  if ( fOK && ( RECLEN(pTMXTagTable) > sizeof(TMX_TAGTABLE_RECORD)) )
  {
    //there is tag info so go through all tag entries and add tags to outstring
    pTagEntry = (PTMX_TAGENTRY)pByte;

    //start of for loop
    usStringPos = 0;
    //initial positioning st beginning of first tag entry
    usTagRecordPos = sizeof(TMX_TAGTABLE_RECORD);
    //initial stop criterium for end of for loop
    ulEnd = pTagEntry->usOffset;

    while ( fOK && (*pInString || !fNoTagsLeft) )
    {
      if ( ulEnd <=  ulInputStringLen )
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          *pOutString = *pInString;
        } /* endfor */
      }
      else
      {
        for ( j = usStringPos; j < ulEnd; j++, pOutString++, pInString++ )
        {
          if ( j < ulInputStringLen )
          {
            *pOutString = *pInString;
          }
          else
          {
            *pOutString = ' ';
            --pInString;     // we've gone 1 too far..
          } /* endif */

          if ( pOutString >=
                  pOutStart + MAX_SEGMENT_SIZE + usStringPos - ulEnd )
          {
            *pOutStart = EOS;
            fOK = FALSE;
            break;
          } /* endif */
        } /* endfor */
      } /* endif */

      if ( fOK && (j == ulEnd) && (!fNoTagsLeft) )
      {
        /**************************************************************/
        /* Add check that tag length does not jeopardize our          */
        /* output string...                                           */
        /**************************************************************/
        if ( (pOutString+pTagEntry->usTagLen)  < (pOutStart+MAX_SEGMENT_SIZE)
              && (pTagEntry->usTagLen < MAX_SEGMENT_SIZE)    )
        {
          USHORT usTagLenCHAR = pTagEntry->usTagLen;
          memcpy( pOutString, &pTagEntry->bData, usTagLenCHAR * sizeof(CHAR_W));
          pOutString += usTagLenCHAR;
          ulInputStringLen += usTagLenCHAR;
          //reset for loop start
          usStringPos = j + usTagLenCHAR;
          //update tag table record position counter
          usTagRecordPos += sizeof(TMX_TAGENTRY) + usTagLenCHAR * sizeof(CHAR_W);
          //remember length of tags added to string
          ulTagLen += pTagEntry->usTagLen;

          //move to next tag entry only if there are still tags to add to string
          if ( usTagRecordPos < RECLEN(pTMXTagTable) )
          {
            USHORT usOldOffs = pTagEntry->usOffset;
            pByte += sizeof(TMX_TAGENTRY) + pTagEntry->usTagLen * sizeof(CHAR_W);
            pTagEntry = (PTMX_TAGENTRY)pByte;
            //new end criterium for for loop
            if ( usOldOffs == pTagEntry->usOffset  )
            {
              // something went wrong: two tag entries for the same offset
              fOK = FALSE;
              memcpy( pOutStart, pInStart, *plInStringLen * sizeof(CHAR_W));
              fNoTagsLeft = TRUE;
              pInString = pInStart + *plInStringLen;
              ulEnd = *plInStringLen +  ulTagLen;
            }
            else if ( pTagEntry->usOffset < ulEnd  )
            {
              // tag data seems to be corrupted, ignore the tags
              fOK = FALSE;
              memcpy( pOutStart, pInStart, *plInStringLen * sizeof(CHAR_W));
              fNoTagsLeft = TRUE;
              pInString = pInStart + *plInStringLen;
              ulEnd = *plInStringLen +  ulTagLen;
            }
            else
            {
              ulEnd = pTagEntry->usOffset;
            } /* endif */
          }
          else
          {
            fNoTagsLeft = TRUE;
            //new end criterium for for loop, in this case end of string
            ulEnd = *plInStringLen +  ulTagLen;
          } /* endif */
        }
        else
        {
          // something got corrupted -- ignore the tags ...
          fOK = FALSE;
          memcpy( pOutStart, pInStart, *plInStringLen * sizeof(CHAR_W) );
          fNoTagsLeft = TRUE;
          pInString = pInStart + *plInStringLen;
          ulEnd = *plInStringLen +  ulTagLen;
        } /* endif */
      } /* endif */

      if ( pOutString >= pOutStart + MAX_SEGMENT_SIZE )
      {
        *pOutStart = EOS;
        fOK = FALSE;
        break;
      } /* endif */
    } /* endwhile */

    if ( *pOutStart )
    {
      // Add the end of string
      *pOutString = EOS;
    }
  }
  else
  {
    //there are no tags so copy character string
    if ( *plInStringLen < MAX_SEGMENT_SIZE )
    {
      memcpy( pOutString, pInString, *plInStringLen * sizeof(CHAR_W));
      pOutString[*plInStringLen] = EOS;
    }
    else
    {
      fOK = FALSE;
      *pOutStart = EOS;
    } /* endif */
  } /* endif */

  return fOK;

}


USHORT GetFuzzyMatch
(
  EqfMemory* pTmClb,                     //ptr to ctl block struct
  PTMX_SENTENCE pSentence,             //ptr to sentence structure
  PTMX_GET_W pGetIn,                   //ptr to data in get in structure
  PTMX_MATCH_TABLE_W pstMatchTable,    //get out output structure
  PUSHORT pusMatchesFound              //number of matches found
)
{
  BOOL fOK = TRUE;                     //success indicator
  USHORT usRc = NO_ERROR;              //return code
  ULONG  ulLen;                        //length indicator
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record
  PTMX_MATCHENTRY pMatchEntry = NULL;  //pointer to match entry structure
  PTMX_MATCHENTRY pMatchStart = NULL;  //pointer to match entry structure
  USHORT usMatchEntries = 0;           //nr of matches found
  USHORT usOverlaps = 0;               //nr of overlapping triples
  ULONG  ulRecBufSize = 0L;            // current size of record buffer

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

  //allocate 32K for tm record if not done yet
  if ( pTmClb->pvTmRecord )
  {
    pTmRecord = (PTMX_RECORD)pTmClb->pvTmRecord; 
    ulRecBufSize = pTmClb->ulRecBufSize;
//    memset( pTmRecord, 0, ulRecBufSize );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvTmRecord = pTmRecord;
    pTmClb->ulRecBufSize = ulRecBufSize = TMX_REC_SIZE;
  } /* endif */

  //allocate for match entry
  if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pMatchEntry), 0L, (LONG)(ABS_VOTES * sizeof(TMX_MATCHENTRY)), NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    pMatchStart = pMatchEntry;

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif
    //fill pMatchEntry with the tm keys with the highest frequency
    usRc = FillMatchEntry( pTmClb, pSentence, pMatchEntry, &pGetIn->usMatchThreshold );
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyFillMatchEntry) );
#endif
    if ( usRc == NO_ERROR )
    {
      //get tm record(s)
      while ( (pMatchEntry->ulKey) && (usRc == NO_ERROR)) // &&
                                                          //(*pusMatchesFound <= pGetIn->usRequestedMatches) )
      {
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif
        ulLen = ulRecBufSize;

        T5LOG( T5INFO) << "GetFuzzyMatch: EQFNTMGET of record " << pMatchEntry->ulKey;

        usRc =  pTmClb->TmBtree.EQFNTMGet( pMatchEntry->ulKey, (PCHAR)pTmRecord, &ulLen ); 

        if ( usRc == BTREE_BUFFER_SMALL)
        {
          fOK = UtlAlloc( (PVOID *)&pTmRecord, ulRecBufSize, ulLen, NOMSG );
          if ( fOK )
          {
            pTmClb->ulRecBufSize = ulRecBufSize = ulLen;
            pTmClb->pvTmRecord = pTmRecord;
            memset( pTmRecord, 0, ulLen );
            usRc =  pTmClb->TmBtree.EQFNTMGet( pMatchEntry->ulKey, (PCHAR)pTmRecord, &ulLen );
          }
          else
          {
            LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
          } /* endif */
        } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyGetTime) );
#endif
        if ( usRc == NO_ERROR )
        {
          T5LOG( T5INFO) << "GetFuzzyMatch: MaxVotes=" << pMatchEntry->usMaxVotes << ", ActVote=" << pSentence->usActVote << " EQFNTMGET of record ";

          //compare tm record data with data passed in the get in structure
          usOverlaps = std::min( pMatchEntry->usMaxVotes, pSentence->usActVote );
          usRc = FuzzyTest( pTmClb, pTmRecord, pGetIn, pstMatchTable,
                            &usMatchEntries, &usOverlaps,
                            &pMatchEntry->usMaxVotes, &pSentence->usActVote,
                            pSentence, pMatchEntry->ulKey );
          if ( !usRc )
          {
            //nr of matches found
            (*pusMatchesFound) = usMatchEntries;

            //get next tm record
            pMatchEntry++;
          } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyTestTime) );
#endif

        } /* endif */
      } /* endwhile */

      // limit matches to number of matches requested
      if (*pusMatchesFound > pGetIn->usRequestedMatches)
      {
        // special handling for machine matches: ensure that at least one machine match is left in the truncated list
        if ( (pstMatchTable[(*pusMatchesFound)-1].usTranslationFlag == TRANSLFLAG_MACHINE) && 
             (pstMatchTable[pGetIn->usRequestedMatches-1].usMatchLevel < 100))
        {
          memcpy( pstMatchTable + (pGetIn->usRequestedMatches-1), pstMatchTable + (*pusMatchesFound - 1),  sizeof(TMX_MATCH_TABLE_W) );
        } /* endif */           

        memset( &pstMatchTable[pGetIn->usRequestedMatches], 0, sizeof( TMX_MATCH_TABLE_W ));
        *pusMatchesFound = pGetIn->usRequestedMatches;
      }

    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pMatchStart, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( GETFUZZYMATCH_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFuzzyOtherTime) );
#endif

  return( usRc );
}




std::wstring removeTagsFromString(std::wstring input){

  bool fTagOpened = false;
  wchar_t* pIn  = &input[0];
  wchar_t* pOutStart = pIn;
  wchar_t* pOut = pOutStart;

  while( *pIn != L'\0'){
    switch (*pIn)
    {
    case L'<':
      fTagOpened = true;
      break;
    case L'>':
      fTagOpened = false;
      break;
    
    default:
      if(fTagOpened == false){
        *pOut = *pIn;
        pOut++;
      }
      break;
    }
    
    pIn++;
  }

  *pOut = L'\0';
  return std::wstring(pOutStart);
}




//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FuzzyTest                                                |
//+----------------------------------------------------------------------------+
//|Description:       The fuzzy match algorithm to determine the degree of     |
//|                   fuzziness.                                               |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory* pTmClb                                          |
//|                   PTMX_RECORD pTmRecord                                    |
//|                   PTMX_GET pGetIn                                          |
//|                   PTMX_MATCH_TABLE pstMatchTable                           |
//|                   PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT pusMatchesFound                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     position at start of first target record in tm record    |
//|                   if target language                                       |
//|                     if tag table id                                        |
//|                       fill match table and add strings with tags           |
//|                     else                                                   |
//|                       fill match table and don't add tags                  |
//|                   else                                                     |
//|                     try next target record                                 |
// ----------------------------------------------------------------------------+

USHORT FuzzyTest ( EqfMemory* pTmClb,           //ptr to control block
                   PTMX_RECORD pTmRecord,     //ptr to tm record
                   PTMX_GET_W pGetIn,         //ptr to data in get in structure
                   PTMX_MATCH_TABLE_W pstMatchTable, //get out output structure
                   PUSHORT pusMatchesFound,   //number of matches found
                   PUSHORT pusOverlaps,       //number of triple overlaps
                   PUSHORT pusTmMaxVotes,     //nr of tm source triple
                   PUSHORT pusGetMaxVotes,    //nr of get source triples
                   PTMX_SENTENCE pSentence,   //ptr to sentence structure
                   ULONG   ulKeyNum )         // record number
{
  PBYTE pByte;                         //position ptr
  PBYTE pSource;                       //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL;     //ptr to target record
  PTMX_TAGTABLE_RECORD pTMXTargetTagTable = NULL; //ptr to target tag record
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_CLB pTMXTargetClb = NULL;       //ptr to target control block
  PSZ_W pString = NULL;                //ptr to source string
  BOOL fOK = TRUE;                     //success indicator
  ULONG ulSourceLen = 0;              //length of normalized source string
  USHORT usRc = NO_ERROR;              //return code
  USHORT usFuzzy = 0, usWords = 0, usDiffs = 0;  //fuzzy match value
  BOOL   fStringEqual;                 // strings are equal ???
  BOOL   fNormStringEqual;             // normalized strings are equal ???
  BOOL   fRespectCRLFStringEqual = 0L;

  SHORT sLangID = MorphGetLanguageID( pGetIn->szSourceLanguage, &sLangID );

  if (!pGetIn->ulSrcOemCP)
  {
    pGetIn->ulSrcOemCP = 1;
  }

  T5LOG( T5INFO) << "FuzzyTest for record " <<ulKeyNum;

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
  
  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);
    if(pGetIn->fSourceLangIsPrefered == false){
      char recordSrcLang[MAX_LANG_LENGTH];
      NTMGetNameFromID( pTmClb, &pTMXSourceRecord->usLangId,
                              (USHORT)LANG_KEY,
                              recordSrcLang, NULL );
      if(strcasecmp(recordSrcLang, pGetIn->szSourceLanguage)){
        T5LOG( T5WARNING) <<":: source langs is different in record(" << recordSrcLang << ") and request (" << pGetIn->szSourceLanguage << ")";
        return SOURCE_LANG_DIFFERENT;
      }
    }
    
    //move pointer to corresponding position
    pSource = (PBYTE)(pTmRecord+1);
    pSource += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulSourceLen = (RECLEN(pTMXSourceRecord) -
                           sizeof(TMX_SOURCE_RECORD));

    //copy source string for fill matchtable
    ulSourceLen = EQFCompress2Unicode( pString, pSource, ulSourceLen );
    pSentence->pPropString = std::make_unique<StringTagVariants>(pString);

    if(T5Logger::GetInstance()->CheckLogLevel(T5INFO)){    
      auto str = EncodingHelper::convertToUTF8(pString);
      T5LOG( T5INFO) << "::FuzzyTest: \n<SOURCE>\r\n" << str << "\r\n</SOURCE>\r\n" ;
    }

    if (pGetIn->ulParm & GET_RESPECTCRLF )   // if-else nec for P018279
    {
	    fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pSentence->pPropString->getNormStrC(), pSentence->pStrings->getNormStrC(), 0 )== 0L);

	    fStringEqual = (UtlCompIgnWhiteSpaceW( pSentence->pPropString->getNormStrC(), pSentence->pStrings->getNormStrC(), 0 ) == 0L);
	    if (fStringEqual && !fRespectCRLFStringEqual)
	    {  // there is a LF difference!
            fStringEqual = fRespectCRLFStringEqual;
	    }
    }
    else
    {  //compare source strings
       fStringEqual = (  UtlCompIgnWhiteSpaceW(pSentence->pPropString->getNormStrC(), pSentence->pStrings->getNormStrC(), 0 ) == 0L 
                      || UtlCompIgnWhiteSpaceW(pSentence->pPropString->getGenericTagStrC(), pSentence->pStrings->getGenericTagStrC(),0) == 0L  ) ;
    } /* endif*/

    T5LOG( T5INFO) << "FuzzyTest: After String compare, fStringEqual = " <<fStringEqual;

    /******************************************************************/
    /* if strings equal - apply equal test first ...                  */
    /* if nothing found during equal test, try with fuzzy...          */
    /******************************************************************/
    fNormStringEqual = fStringEqual;
    if ( fStringEqual )
    {
      //string are equal so apply equal test
      USHORT  usNumMatches = *pusMatchesFound;
      usRc = ExactTest( pTmClb, pTmRecord, pGetIn, pSentence, pstMatchTable, pusMatchesFound, ulKeyNum );

      fStringEqual = ( *pusMatchesFound > usNumMatches );
      T5LOG(T5INFO) << "FuzzyTest: After ExactTest, fStringEqual = " <<fStringEqual;
    } /* endif */

    if ( !fStringEqual && !usRc )
    {
      USHORT  usTgtNum = 0;            // first target number
      ULONG   ulLeftTgtLen;            // target length record...
      USHORT  usTagId;                 // tag table id
      USHORT  usTargetId;              // target language id
      BOOL    fTagTableEqual;          // TRUE if TagTable of prop == current
      USHORT  usTagThreshold;          // set dependent of fTagTableEqual
      LONG    lTempSrcLen;            // temp. source length

      //get id of target tag table in the get structure
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTagTable,
                             NULL,
                             (USHORT)TAGTABLE_KEY, &usTagId,
                             NTMGETID_NOUPDATE_OPT, NULL ))
      {
        usTagId = 1;   // set default...
      } /* endif */
        //get id of target language in the get structure

      // we have to update the memory language table to keep the language group table up-to-date...
//      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage, NULL, (USHORT)LANG_KEY, &usTargetId, NTMGETID_NOUPDATE_OPT, NULL ))
      if ( NTMGetIDFromNameEx( pTmClb, pGetIn->szTargetLanguage, NULL, (USHORT)LANG_KEY, &usTargetId, 0, NULL ))
      {
        usTargetId = 1;  // set default..
      } /* endif */


      //strings are not equal, assume fuzzy
      //position at the first target record
      pByte = (PBYTE)pTmRecord;
      pByte += pTmRecord->usFirstTargetRecord;
      pTMXTargetRecord = (PTMX_TARGET_RECORD)pByte;
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //loop through target records
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      T5LOG( T5INFO) << "FuzzyTest: Checking targets, ulLeftTgtLen = "<<ulLeftTgtLen;
      while ( ( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) ) && (ulLeftTgtLen > 0) && (RECLEN(pTMXTargetRecord) != 0) )
      {
        BOOL fTestCLB = TRUE;
        USHORT usModifiedTranslationFlag = 0;

        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        usTgtNum ++;            // we are dealing with next target

        //assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );

        ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pTMXTargetClb = (PTMX_TARGET_CLB)pByte;

        //compare target language group IDs
        if(pGetIn->fTargetLangIsPrefered){
          if ( pTmClb->psLangIdToGroupTable[pTMXTargetClb->usLangId] != pTmClb->psLangIdToGroupTable[usTargetId] )
          {
            T5LOG( T5INFO) <<"FuzzyTest: Wrong target language!" ;
            fTestCLB = FALSE;
          } 
        } else {
          char recordTrgLang[MAX_LANG_LENGTH];
          NTMGetNameFromID( pTmClb, &pTMXTargetClb->usLangId,
                                  (USHORT)LANG_KEY,
                                  recordTrgLang, NULL );
          if(strcasecmp(recordTrgLang, pGetIn->szTargetLanguage)){
            T5LOG( T5WARNING) <<":: target langs is different in record(" << recordTrgLang << ") and request (" << pGetIn->szTargetLanguage<< ")";
            //return SOURCE_LANG_DIFFERENT;
            fTestCLB = FALSE;
          }
        }
        

        // check for comments when requested
        if ( pGetIn->ulParm & GET_IGNORE_COMMENT )
        {
          if ( NtmFindInAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, L"<Note" ) )
          {
            T5LOG( T5INFO) << "FuzzyTest: CLB with comment skipped" ;
            fTestCLB = FALSE;
          } 
        } 

        // preprocessing for global memory processing
        usModifiedTranslationFlag = pTMXTargetClb->bTranslationFlag;
        if ( fTestCLB && (pTMXTargetClb->bTranslationFlag == TRANSLFLAG_GLOBMEM) && (pGetIn->pvGMOptList != NULL) && pTMXTargetClb->usAddDataLen  )
        {
          USHORT usAddDataLen = 0;
          if ( !pSentence->pAddString ) UtlAlloc( (PVOID *) &pSentence->pAddString, 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );
          usAddDataLen = NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pSentence->pAddString, MAX_SEGMENT_SIZE );   
          if ( usAddDataLen )
          {
            GMMEMOPT GobMemOpt = GlobMemGetFlagForProposal( pGetIn->pvGMOptList, pSentence->pAddString );
            switch ( GobMemOpt )
            {
              case GM_SUBSTITUTE_OPT: usModifiedTranslationFlag  = TRANSLFLAG_NORMAL; break;
              case GM_HFLAG_OPT     : usModifiedTranslationFlag  = TRANSLFLAG_GLOBMEM; break;
              case GM_HFLAGSTAR_OPT : usModifiedTranslationFlag  = TRANSLFLAG_GLOBMEMSTAR; break;
              case GM_EXCLUDE_OPT   : fTestCLB = FALSE;; break;
            } /* endswitch */   
          } /* endif */                   
        } /* endif */                 

        T5LOG( T5INFO) << "FuzzyTest: fTestCLB="<<( fTestCLB ? "Yes" : "NO") ;

        // compare target table IDs
        if ( fTestCLB )
        {
          //compare target tag table ids
          //position at target tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at target tag table
          pByte += pTMXTargetRecord->usTargetTagTable;
          pTMXTargetTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          // GQ: only compare tag tables if segment and proposal have inline tagging
          if ( (RECLEN(pTMXTargetTagTable) > sizeof(TMX_TAGTABLE_RECORD)) ||  (RECLEN(pSentence->pTagRecord) > sizeof(TMX_TAGTABLE_RECORD)) )
          {
            fTagTableEqual =(pTMXTargetTagTable->usTagTableId == usTagId);
            // For R012645 begin
            if(!fTagTableEqual)
            {
              std::string familyNameIn = GetAndCacheFamilyName(pGetIn->szTagTable);

              if( !familyNameIn.empty() )
              {
                // get markup name firstly, also with cache
                char szMarkup[MAX_FNAME];
                memset(szMarkup,0,sizeof(szMarkup));
                NTMGetNameFromID( pTmClb, &(pTMXTargetTagTable->usTagTableId), (USHORT)TAGTABLE_KEY,szMarkup, NULL );
                  
                std::string familyNameTgt = GetAndCacheFamilyName(szMarkup);
                if( !familyNameTgt.empty() && familyNameIn==familyNameTgt)
                  fTagTableEqual = TRUE;
              } 
            }
            // For R012645 end
          }
          else
          {
            fTagTableEqual = TRUE;
          } /* endif */

          /**********************************************************/
          /* TAGS_EQUAL == base 98, TAGS_UNEQUAL == base 95         */
          /**********************************************************/
          usTagThreshold = (fTagTableEqual) ? TAGS_EQUAL : TAGS_UNEQUAL;

          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          {
            BOOL  fFuzzynessOK;

// as the strings doe not contain the inline tagging, we have
            // to add the inline tags first
            // if the tag tables are different this is omitted as the
            // inline tags are suppressed in the proposal in such a case


            if ( fOK )
            {
              LONG  lLenTmp = ulSourceLen;
              PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
              //position at source tag table record
              pByte = pStartTarget;
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
              pByte += pTMXTargetRecord->usSourceTagTable;
              pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;
              if ( !fOK )
              {
                LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
              } /* endif */              
            } /* endif */

            //TODO: remove punctuation here
            fFuzzynessOK = TMFuzzynessEx( pGetIn->szTagTable,
                                        pSentence->pStrings->getNormStrC(),
                                        pSentence->pPropString->getNormStrC(),
                                        sLangID, &usFuzzy,
                                        pGetIn->ulSrcOemCP, &usWords, &usDiffs);

            /**********************************************************/
            /* additional comparison if normalized strings are equal  */
            /**********************************************************/
//            if ( usFuzzy >= 100 )
            if ( fNormStringEqual )
            {
              PTMX_TAGTABLE_RECORD pTMXSourceTagTable; //ptr to source tag info
              BOOL fStringEqual = FALSE;
              //compare source tag table records
              //position at source tag table record
              pByte = pStartTarget;
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

              //position at source tag table
              pByte += pTMXTargetRecord->usSourceTagTable;
              pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

              //compare tag table records
              //fStringEqual = memcmp( pTMXSourceTagTable, pSentence->pTagRecord,
              //                       RECLEN(pTMXSourceTagTable) ) == 0;
              //if ( !fStringEqual )
              //{
                /**********************************************************/
                /* if tagging record is unequal than we have some         */
                /* (slight) differences                                   */
                /* i.e. we will create a fully qualified string and try   */
                /* another compare...                                     */
                /**********************************************************/
                //allocate pString

                if ( fOK )
                {
                  //calculate length of source string
                  LONG  lLenTmp = ulSourceLen;
                  
                  if ( !fOK )
                  {
                    LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
                  }
                  else
                  {
                    fStringEqual = FALSE;
                    
                    fStringEqual = (UtlCompIgnWhiteSpaceW(pSentence->pPropString->getGenericTagStrC(),
                                                  pSentence->pStrings->getGenericTagStrC(),
                                                  0 ) == 0 );
                    if (!fStringEqual && (usFuzzy >= 100) )
                    {
                       // compare with respect to protected/unprotected parts
                      // We can assume all Text tokens to be equal ( usFuzzy = 100),
                      // but we do not know about tag tokens
                      BOOL fTempStringEqual = FALSE;
                      BOOL fOK = FALSE;
                      fOK = NTMCompareBetweenTokens(
                            pSentence->pPropString->getGenericTagStrC(),
                            pSentence->pStrings->getGenericTagStrC(),
                            pGetIn->szTagTable,
                            sLangID,
                            pGetIn->ulSrcOemCP, &fTempStringEqual);
                        if (fOK)
                        {
                          fStringEqual = fTempStringEqual;
                        }
                    }
                  } /* endif */
                } /* endif */
              //} /* endif */
              /********************************************************/
              /* ensure that we are dealing with a fuzzy match        */
              /********************************************************/
              if ( !fStringEqual )
              {
                if ( usFuzzy > 3 )
                {
                  usFuzzy -= 3;
                }
                else
                {
                  usFuzzy = 0;
                } /* endif */

                usFuzzy = std::min( (USHORT)99, usFuzzy );
              } /* endif */

            } /* endif */
          }

          //fill get output structure
          T5LOG( T5INFO) << "FuzzyTest: usFuzzy="<<usFuzzy<<", fTagTableEqual="<<fTagTableEqual;
          if ( (usModifiedTranslationFlag == TRANSLFLAG_MACHINE) && (usFuzzy < 100) )
          {
            // ignore machine fuzzy matches
          }
          else if ( usFuzzy > TM_FUZZINESS_THRESHOLD )
          {
            /********************************************************/
            /* give MT flag a little less fuzziness                 */
            /********************************************************/
            if ( usModifiedTranslationFlag == TRANSLFLAG_MACHINE )
            {
              if ( usFuzzy > 1 )
              {
                usFuzzy -= 1;
              }
              else
              {
                usFuzzy = 0;
              } /* endif */
            } /* endif */
            if (usFuzzy == 100 && (pGetIn->ulParm & GET_RESPECTCRLF) && !fRespectCRLFStringEqual )
        		{ // P018279!
    			   usFuzzy -= 1;
		        }
            lTempSrcLen = ulSourceLen;
            FillMatchTable( pTmClb, pString, &lTempSrcLen, pTMXTargetRecord,
                            pTMXTargetClb,
                            pstMatchTable, &usFuzzy, fTagTableEqual,
                            pusMatchesFound, pusTmMaxVotes, pusOverlaps,
                            ulKeyNum, usTgtNum,
                            pGetIn, pSentence->pTagRecord, usModifiedTranslationFlag, 0, usWords, usDiffs, pTMXSourceRecord->usLangId );
          } /* endif */
        } /* endif */
        //position at next target
        pByte = pStartTarget;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        //move pointer to end of target
        pByte += RECLEN(pTMXTargetRecord);
        //remember the beginning of next record
        pStartTarget = pByte;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      }/* endwhile */
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );

  if ( usRc )
  {
    ERREVENT2( FUZZYTEST_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  return( usRc );
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillMatchEntry                                           |
//+----------------------------------------------------------------------------+
//|Description:       Determine the sentence keys for all triple hashes        |
//|                   and their frequencies and add to match table.            |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory* pTmClb                                          |
//|                   PTMX_SENTENCE pSentence                                  |
//|                   PTMX_MATCHENTRY pMatchEntry                              |
//|                   PUSHORT pusMatchThreshold                                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     for all triple hashes built for the sentence segment     |
//|                     get the index record                                   |
//|                     loop through all index entries adding the sentence     |
//|                     keys and updating their respective frequency counts    |
// ----------------------------------------------------------------------------+
USHORT FillMatchEntry
( 
  EqfMemory* pTmClb,
  PTMX_SENTENCE pSentence,
  PTMX_MATCHENTRY pMatchEntry,
  PUSHORT pusMatchThreshold
)
{
  PTMX_INDEX_RECORD pIndexRecord = NULL;      //pointer to index record
  BOOL fOK = TRUE;                            //success indicator
  PTMX_MATCHENTRY pTempMatch = NULL;          //ptr to working match entry structure
  PTMX_MATCHENTRY pTempStart = NULL;          //ptr to working match entry structure
  USHORT usRc = NO_ERROR;                     //return code
  PTMX_INDEX_ENTRY pIndexEntry = NULL;        //pointer to index entry
  PULONG pulVotes;                   // pointer to votes
  ULONG  ulLen;                      // length paramter
  USHORT i, j;                       // index in for loop
  USHORT usMaxEntries;               // nr of index entries in index record
  ULONG  lMatchEntries = 0;             // nr of entries in pTempMatch
  ULONG ulKey;                       // index key
  USHORT usMaxSentences = MAX_INDEX_LEN * 4;
  LONG   lMatchListSize = (LONG)((usMaxSentences+1) * sizeof(TMX_MATCHENTRY));


#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lOtherTime) );
#endif

  //allocate index record buffer if not done yet
  if ( pTmClb->pvIndexRecord )
  {
    pIndexRecord = (PTMX_INDEX_RECORD)pTmClb->pvIndexRecord;
    memset( pIndexRecord, 0, TMX_REC_SIZE );
  }
  else
  {
    fOK = UtlAlloc( (PVOID *) &(pIndexRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );
    pTmClb->pvIndexRecord = pIndexRecord;
  } /* endif */

  // allocate match list array if not done yet
  if ( fOK )
  {
    if ( pTmClb->pvTempMatchList )
    {
      pTempMatch = (PTMX_MATCHENTRY)pTmClb->pvTempMatchList;
      memset( pTempMatch , 0, lMatchListSize );
    }
    else
    {
      fOK = UtlAlloc( (PVOID *) &(pTempMatch), 0L, lMatchListSize, NOMSG );
      pTmClb->pvTempMatchList = pTempMatch;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pTempStart = pTempMatch;
  }
  else
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchAllocTime) );
#endif
  if ( fOK )
  {
    pulVotes = pSentence->pulVotes;
    lMatchEntries = 0;

    for ( i = 0; i < pSentence->usActVote; i++, pulVotes++ )
    {
      ulKey = (*pulVotes) & START_KEY;
      ulLen = TMX_REC_SIZE;
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchOtherTime) );
#endif
      usRc = pTmClb->InBtree.EQFNTMGet(  ulKey,  (PCHAR)pIndexRecord, &ulLen ); 

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchReadTime) );
#endif
      if ( usRc == NO_ERROR )
      {
        //calculate number of index entries in index record
        T5LOG(T5INFO) <<"Processing index record "<<ulKey;

        ulLen = pIndexRecord->usRecordLen;
        usMaxEntries = (USHORT)((ulLen - sizeof(USHORT)) / sizeof(TMX_INDEX_ENTRY));

        pIndexEntry = &pIndexRecord->stIndexEntry;
        pTempMatch = pTempStart;

        //end criteria are all sentence ids in index key or only one
        //sentence id left in pulSids
        for ( j = 0; (j < usMaxEntries) && (lMatchEntries < usMaxSentences); j++, pIndexEntry++ )
        {
          ULONG ulIndexKey = NTMKEY(*pIndexEntry);

          // reset to begin of matching table
          pTempMatch = pTempStart;
          //before adding sentence id check if already in pulsids as the
          //respective tm record need only be checked once
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill1Time) );
#endif
          
          // GQ: do a binary search for the key ( bsearch() cannot be used as we need the insert position) and
          //     a linear search is far too slow 
          {
            int high = lMatchEntries;
            int low = -1;
            int probe;

            while (high - low > 1)
            {
              probe = (high + low) / 2;
              if (pTempStart[probe].ulKey > ulIndexKey)
                high = probe;
              else
                low = probe;
            } /* endwhile */

            if (low == -1 )
            {
              pTempMatch = pTempStart;
            }
            else
            {
              pTempMatch = pTempStart + low;
              if ( pTempMatch->ulKey < ulIndexKey ) pTempMatch++;
            } /* endif */
          }

          // below is the old search code
          //while ( (ulIndexKey > pTempMatch->ulKey) && pTempMatch->ulKey )
          //{
          //  pTempMatch++;
          //} /* endwhile */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill2Time) );
#endif
          if ( ulIndexKey == pTempMatch->ulKey )
          {
            pTempMatch->cCount++;
          }
          else if ( !pTempMatch->ulKey )
          {
            //add new match entry
            pTempMatch->ulKey = NTMKEY(*pIndexEntry);
            pTempMatch->usMaxVotes = NTMVOTES(*pIndexEntry);
            pTempMatch->cCount = 1;
            pTempMatch->usMatchVotes = pSentence->usActVote;
            lMatchEntries++;
          }
          else
          {
            //insert in the middle of match entry structure
            if ( lMatchEntries )
            {
/*Fix*/       memmove( pTempMatch+1, pTempMatch, (lMatchEntries - (pTempMatch - pTempStart) ) * sizeof(TMX_MATCHENTRY) );
            } /* enidf */
            pTempMatch->ulKey = ulIndexKey;
            pTempMatch->usMaxVotes = NTMVOTES(*pIndexEntry);
            pTempMatch->cCount = 1;
            lMatchEntries++;
          } /* endif */
        } /* endfor */

        /**************************************************************/
        /* make clean up  if we are too near at the limit ..          */
        /**************************************************************/
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill3Time) );
#endif
        if (lMatchEntries >= 2 * MAX_INDEX_LEN )
        {
          PTMX_MATCHENTRY pTemp1Start;

          T5LOG( T5INFO) <<"Cleaning up..." ;

          fOK = UtlAlloc( (PVOID *) &(pTemp1Start), 0L,
                         (LONG)((usMaxSentences+1)* sizeof(TMX_MATCHENTRY)), NOMSG );
          if ( fOK )
          {
            USHORT  k = 0;
            PTMX_MATCHENTRY pTemp1Match;
            pTemp1Match = pTemp1Start;
            pTempMatch = pTempStart;

            for (k=0; k<lMatchEntries ; k++ )
            {
              if ( pTempMatch->cCount > 1 )
              {
                *pTemp1Match++ = *pTempMatch++;
              }
              else
              {
                pTempMatch++;
              } /* endif */
            } /* endfor */
/*Fix*/     memset( pTemp1Match, 0, sizeof(TMX_MATCHENTRY) );

            /**********************************************************/
            /* copy area back into orignal, adjust num entries and    */
            /* free allocation                                        */
            /**********************************************************/
            memcpy( pTempStart, pTemp1Start, (usMaxSentences+1)* sizeof(TMX_MATCHENTRY));
/*Fix*/     lMatchEntries = pTemp1Match - pTemp1Start;
            UtlAlloc( (PVOID *) &pTemp1Start, 0L, 0L, NOMSG );
          } /* endif */
        }
        else
        {
        } /* endif */
#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFill4Time) );
#endif

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchFillTime) );
#endif

      } /* endif */
    } /* endfor */

    //position at start of structure
    pTempMatch = pTempStart;

    //sort the sentence keys by frequency discard those below the given
    //threshold
    CleanupTempMatch( pTempMatch, &pMatchEntry, &pSentence->usActVote, pusMatchThreshold );
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchCleanupTime) );
#endif

  //if the entry does not exist in the index file it is not to be regarded as
  //an error
  if ( usRc == BTREE_NOT_FOUND )
  {
    LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);
  } /* endif */

#ifdef MEASURETIME
  GetElapsedTime( &(pTmClb->lFillMatchAllocTime) );
#endif
  if ( usRc )
  {
    ERREVENT2( FILLMATCHENTRY_LOC, ERROR_EVENT, usRc, TM_GROUP, "" );
  } /* endif */

  T5LOG( T5INFO) << "FillMatchEntry: lMatchEntries=" <<lMatchEntries<<", rc=" <<usRc;

  return( usRc );
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CleanupTempMatch                                         |
//+----------------------------------------------------------------------------+
//|Description:       Removes all entries below calculated threshold and       |
//|                   sort sentence keys by frequency.                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_MATCHENTRY pTempMatch                               |
//|                   PTMX_MATCHENTRY *ppMatchEntry                            |
//|                   PUSHORT pusActVote                                       |
//|                   PUSHORT pusMatchThreshold                                |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_MATCHENTRY *ppMatchEntry                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//+----------------------------------------------------------------------------+
//|Function flow:     ignore sentence keys that fall below calculated threshold|
//|                   and sort by frequency.                                   |
// ----------------------------------------------------------------------------+

VOID CleanupTempMatch( PTMX_MATCHENTRY pTempMatch,
                       PTMX_MATCHENTRY *ppMatchEntry,
                       PUSHORT pusActVote,
                       PUSHORT pusMatchThreshold )
{
  PTMX_MATCHENTRY pTempPos = NULL;            //pointers tp pTempMatch
  PTMX_MATCHENTRY pActualPos = NULL;          //pointers to pTempMatch
  PTMX_MATCHENTRY pMatchEntry = NULL;         //pointers to pMatchEntey
  PTMX_MATCHENTRY pMatchEntryStart = NULL;    //pointers to pMatchEntey
  USHORT usThreshold;                  //clipping factor
  USHORT usEntries;                    //nr of entries in pTempMatch
  USHORT i;                            //counter
  USHORT usEntriesInList;              //nr of entries in pTempMatch
  USHORT usActVote = *pusActVote;

  pMatchEntry = *ppMatchEntry;
  pMatchEntryStart = pMatchEntry;

  // reduce act vote if MAX_VOTES is reached (the cCount value can never reach MAX_VOTES)
  if ( usActVote >= (MAX_VOTES - 3) )
  {
    usActVote = MAX_VOTES - 3;
  } /* endif */

  //calculate threshold based on number of triple hashes found for the
  //sentence

  usThreshold =  ((usActVote) * (*pusMatchThreshold)) / 100;

  T5LOG( T5INFO) << "::Threshhold computation, usThreshold=" << usThreshold
        <<", *pusActVote="<<*pusActVote<<", *pusMatchThreshold=" <<*pusMatchThreshold;

  // if threshold is no restriction, i.e. sentence too short
  // only allow for exact match
  //if ((usThreshold == 0 ) || (*pusActVote <= 3))
  //  usThreshold = *pusActVote;

  //eliminate all entries in pTempMatch that fall below usThreshold
  //move down the list with pTempPos and insert at pActualPos if the
  //threshold criterium holds
  pActualPos = pTempMatch;
  pTempPos = pTempMatch;
  usEntries = 0;
  T5LOG( T5INFO) << "CleanupTempMatch, temp matches before cleaning up";
  while ( pTempPos->cCount )
  {
    // reduce temp act vote if MAX_VOTES is reached (the cCount value can never reach MAX_VOTES)
    USHORT usTempMaxVotes = pTempPos->usMaxVotes;

    if ( usTempMaxVotes >= (MAX_VOTES - 3) )  
    {
      usTempMaxVotes = MAX_VOTES - 3;
    } /* endif */

    T5LOG( T5INFO) << "ulKey=" << pTempPos->ulKey << " usMaxVotes=" << pTempPos->usMaxVotes << " cCount=" << pTempPos->cCount;

    /******************************************************************/
    /* check that threshold criteria is fulfilled for BOTH sources    */
    /******************************************************************/
    if ( ((USHORT)pTempPos->cCount >= usThreshold ) &&
         ((usTempMaxVotes * (*pusMatchThreshold)) <= (USHORT)pTempPos->cCount * 100 ) &&
         (pTempPos->usMaxVotes != 0) )
    {
      memcpy( pActualPos, pTempPos, sizeof(TMX_MATCHENTRY) );
      pActualPos++;
      pTempPos++;
      usEntries++;
    }
    else
    {
      pTempPos++;
    } /* endif */
  } /* endwhile */

  //sort the entries above the threshold in pTempMatch so that the entries
  //with the highest frequency are at the top
  usEntriesInList = usEntries;
  qsort( pTempMatch, usEntries, sizeof(TMX_MATCHENTRY), CompCount );
  T5LOG( T5INFO) << "CleanupTempMatch: Ranked entries after sort are:" ;

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG))
  {
    PTMX_MATCHENTRY pTest = pTempMatch;
    for ( i = 0; i < usEntries; i++, pTest++ )
    {
      T5LOG( T5DEBUG) << "ulKey=" << pTest->ulKey << " usMaxVotes=" << pTest->usMaxVotes << " cCount=" <<pTest->cCount ;
    } /* endfor */
  }

  // limit number to really ones of interest ( the MAX_MATCHES best )
  usEntries = std::min( usEntries, (USHORT)std::min(MAX_MATCHES, (int)(ABS_VOTES/2-1)) );

  //fill pMatchEntry with the highest ranked entries
  pTempPos = pTempMatch;
  T5LOG( T5INFO) << "CleanupTempMatch: Highest ranked entries returned are:" ;
  for ( i = 0; i < usEntries; i++, pMatchEntry++, pTempPos++ )
  {
    memcpy( pMatchEntry, pTempPos, sizeof(TMX_MATCHENTRY ));
    T5LOG(T5INFO) <<  "::ulKey=" << (pTempPos->ulKey) << " usMaxVotes=" << (pTempPos->usMaxVotes)<<" cCount="<< ( pTempPos->cCount );
  } /* endfor */


  // add highest matches by using a different metric
  qsort( pTempMatch, usEntriesInList, sizeof(TMX_MATCHENTRY), CompCountVotes );
  T5LOG( T5INFO) << "CleanupTempMatch: Ranked entries after sort are:" ;

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG))
  {
    PTMX_MATCHENTRY pTest = pTempMatch;
    for ( i = 0; i < usEntries; i++, pTest++ )
    {
      T5LOG( T5DEBUG) << "ulKey="<<(pTest->ulKey)<<" usMaxVotes="<<(pTest->usMaxVotes)<<" cCount="<<(pTest->cCount);
    } /* endfor */
  }

  // copy new possible match entry into list
  pTempPos = pTempMatch;
  for (i = 0; i < usEntries; i++, pTempPos++)
  {
    pMatchEntry = pMatchEntryStart;

    while ( memcmp( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY) ) != 0 && pMatchEntry->cCount )
    {
      pMatchEntry++;
    }
    if (pMatchEntry->cCount == 0)
    {
      memcpy( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY ));
    }
  }

  if ( usEntriesInList > usEntries )
  {
    // check if the following matches have same quality... if so
  // we have to use them too
    PTMX_MATCHENTRY pTest = pTempPos-1;
    while ( (pTempPos->usMaxVotes == pTest->usMaxVotes)  &&
        (pTempPos->cCount == pTest->cCount) &&
      (usEntries < std::min((USHORT)(ABS_VOTES/2-1),usEntriesInList)))
    {
        pMatchEntry = pMatchEntryStart;

        while ( memcmp( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY) ) != 0 && pMatchEntry->cCount )
        {
            pMatchEntry++;
        }
        if (pMatchEntry->cCount == 0)
        {
            memcpy( pMatchEntry, pTempPos, sizeof( TMX_MATCHENTRY ));
        }
        // point to next entry
      pTempPos++;
      usEntries++;
    }

  }
  *ppMatchEntry = pMatchEntryStart;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CompCount                                                |
//+----------------------------------------------------------------------------+
//|Description:       Compare the passed frequencies.                          |
//|                   This function is called by qsort.                        |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_MATCHENTRY pEntry1   pointer to first frequency     |
//|                   PTMX_MATCHENTRY pEntry2   pointer to second frequency    |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       >0   Entry1 > Entry2                                     |
//|                   =0   Entry1 = Entry2                                     |
//|                   <0   Entry1 < Entry2                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     compare the frequencies in matchentry and return the     |
//|                   comparison value                                         |
// ----------------------------------------------------------------------------+
// Note: This comparison does not fit too good under some circumstances.
//       But to be upward compatible it is still in use.
//       A new comparison function is added which adds another metric for sorting.
//       This new metric looks a lot more intuitive.
//       The result of both sort orders are merged, hence we are only getting better
//       results.
int
CompCount
(
  const void * pvEntry1,  // pointer to first frequency
  const void * pvEntry2   // pointer to second frequency
)
{
//  SHORT sRc;                           // return code
  PTMX_MATCHENTRY pEntry1 = (PTMX_MATCHENTRY)pvEntry1;
  PTMX_MATCHENTRY pEntry2 = (PTMX_MATCHENTRY)pvEntry2;

  SHORT  sCount1 = (SHORT) pEntry1->cCount ;
  SHORT  sCount2 = (SHORT) pEntry2->cCount ;

  if ( (pEntry1->usMaxVotes != 0) && (pEntry2->usMaxVotes != 0) )
  {
    sCount1 = (sCount1 << 8) / (SHORT)pEntry1->usMaxVotes;
    sCount2 = (sCount2 << 8) / (SHORT)pEntry2->usMaxVotes;
  } /* endif */     

  return ( sCount2 - sCount1 );

} /* end of function CompCount */

// Sort the possible candidates according to their maxVotes
int
CompCountVotes
(
  const void * pEntry1,  // pointer to first frequency
  const void * pEntry2   // pointer to second frequency
)
{
  int iResult;
  iResult = abs(((PTMX_MATCHENTRY)pEntry1)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry1)->usMatchVotes) -
               abs(((PTMX_MATCHENTRY)pEntry2)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry2)->usMatchVotes);
  if (iResult == 0)
  {
    iResult =  ((PTMX_MATCHENTRY)pEntry2)->usMaxVotes - ((PTMX_MATCHENTRY)pEntry1)->usMaxVotes;
  }
  if (iResult == 0)
  {
    iResult = ((PTMX_MATCHENTRY)pEntry2)->cCount - ((PTMX_MATCHENTRY)pEntry1)->cCount;
  }
  return iResult;

} /* end of function CompCount */



// function to compare the context strings of two segments
USHORT NTMCompareContext
(
  EqfMemory* pTmClb,                     // ptr to ctl block struct
  PSZ         pszMarkup,               // ptr to name markup used for segment
  PSZ_W       pszContext1,             // context of first segment
  PSZ_W       pszContext2              // context of second segment
)
{
  USHORT usRanking = 1;              // default = weak context match

  if ( pszContext1 && *pszContext1 &&
       pszContext2 && *pszContext2 )
  {
    // load tag table if not done yet
    if ( pTmClb->pTagTable == NULL )
    {
      (TALoadTagTableExHwnd( pszMarkup, (PLOADEDTABLE *)&(pTmClb->pTagTable),
                                        FALSE,
                                        TALOADUSEREXIT | TALOADCOMPCONTEXTFUNC,
                                        FALSE, NULLHANDLE ) == NO_ERROR);
    } /* endif */

    // call compare function of user exit if available
    if ( pTmClb->pTagTable != NULL )
    {
      PLOADEDTABLE pTable = (PLOADEDTABLE)pTmClb->pTagTable;
      PFNCOMPCONTEXT pfnCompContext = (PFNCOMPCONTEXT)pTable->pfnCompareContext;

      if ( pfnCompContext )
      {
        pfnCompContext( pszContext1, pszContext2, &usRanking );
      }
      else
      {
        if ( wcsicmp( pszContext1, pszContext2 ) == 0 )
        {
          usRanking = 100;
        }
      } /* endif */
    }
    else
    {
      if ( wcsicmp( pszContext1, pszContext2 ) == 0 )
      {
        usRanking = 100;
      }
    } /* endif */
  } /* endif */

  return( usRanking );
} /* end of function NTMCompareContext */

typedef struct _NTMGETMATCHLEVELDATA
{
  CHAR             szSegmentTagTable[MAX_EQF_PATH];        // tag table name for segment markup
  CHAR             szProposalTagTable[MAX_EQF_PATH];       // tag table name for proposal markup
  CHAR_W           szSegment[8096];                        // buffer for segment data
  BYTE             bTokenBuffer[40000];                    // buffer for tokenizer in replace match detection
                                                           // this buffer has to hold token entries with a size of 18 bytes each
                                                           // theoratically a segmet could contain 2047 tokens....
  BYTE             bBuffer[40000];                         // general purpose buffer
  CHAR             szLang1[MAX_LANG_LENGTH];               // language buffer 1
  CHAR             szLang2[MAX_LANG_LENGTH];               // language buffer 2
} NTMGETMATCHLEVELDATA, *PNTMGETMATCHLEVELDATA;

// static data of NTMGetMatchLevelData
static char szLastSourceLang[MAX_LANG_LENGTH] = "";
static char szLastTargetLang[MAX_LANG_LENGTH] = "";
static ULONG ulLastSrcOemCP = 0;
static ULONG ulLastTgtOemCP = 0;               

// If existed, return it, otherwise it will determine markup family name then cache it
std::string& GetAndCacheFamilyName(char* pszMarkupName)
{
  if( Name2FamilyMap.find(pszMarkupName) == Name2FamilyMap.end() )
  {
    OtmMarkup *pMarkup = GetMarkupObject( pszMarkupName, NULL);
    if( pMarkup != NULL )
      Name2FamilyMap[pszMarkupName] = pMarkup->getFamilyName();
    else
      Name2FamilyMap[pszMarkupName] = "";
  }
  return Name2FamilyMap[pszMarkupName];
}

