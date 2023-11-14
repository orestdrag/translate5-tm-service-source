/*! \file

 \brief wrapper and helper functions for memory plugin

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_MORPH            // morphologic functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP              // editor functions
#include "../pluginmanager/PluginManager.h"
#include "tm.h"
#include "../utilities/LogWrapper.h"
#include "../utilities/EncodingHelper.h"
#include "EQFMORPI.H"

// EQF.H is included by EqfMemory.h
// #include <eqf.h>                  // General Translation Manager include file EQF:H 


#include "vector"
#include <wctype.h>

#include "win_types.h"
#include "LogWrapper.h"
//#include "EQFFUZZ.H"
// prototypes for helper functions


/* \brief Initialize the Memory Plugin Mapper
*/



void InitTMPluginWrapper()
{
  // access plugin manager
	// PluginManager* thePluginManager = PluginManager::getInstance();

}



/**********************************************************************/
/* macro to calculate the number of tokens in the list ...            */
/* and to adjust ends of tokens                                       */
/**********************************************************************/
#define  NUMBEROFTOKENS(usLenStr, pTokenList)   \
         {                                      \
           PFUZZYTOK pTest = pTokenList;        \
           usLenStr = 0;                        \
           while ( pTest->ulHash )              \
           {                                    \
             usLenStr++;                        \
             pTest++;                           \
           } /* endwhile */                     \
         }






// memory utility functions


PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADCompareKey( PSZ_W pszKey1, PSZ_W pzKey2 );
BOOL  MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 );
BOOL  MADNextAttr( PSZ_W *ppszAttr );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize );
PSZ_W MADSkipName( PSZ_W pszName );
PSZ_W MADSkipWhitespace( PSZ_W pszData );

//
// Delete a key with all its atttributes and data
//
BOOL MADDeleteKey( HADDDATAKEY pKey )
{
  PSZ_W pszCurPos = pKey;
  BOOL fAttrAvailable = FALSE;
  PSZ_W pszTagName = NULL;
  PSZ_W pszTagNameEnd = NULL;

  // check key parameter
  if ( (pszCurPos == NULL) || (*pszCurPos == 0) )
    return( FALSE );

  // skip tag name
  if ( *pszCurPos == L'<' ) pszCurPos++;
  pszTagName = pszCurPos;
  pszTagNameEnd = pszCurPos = MADSkipName( pszCurPos );
  do
  {
    fAttrAvailable = MADNextAttr( &pszCurPos );
  } while ( fAttrAvailable ); /* enddo */

  // find end of key data
  if ( (*pszCurPos == L'/') &&  (*(pszCurPos+1) == L'>') )
  {
    // a selfcontained tag... so remove everything up to end of the tag
    PSZ_W pszSource = pszCurPos + 2;
    PSZ_W pszTarget = pKey;
    while ( *pszSource ) *pszTarget++ = *pszSource++;
    *pszTarget = 0;
    return( TRUE );
  }

  // find ending tag
  // note: this is a very simple approach which only looks for the ending tag and does not interpret the data in any way...
  while ( *pszCurPos != 0 )
  {
    // move to begin of next tag
    while ( (*pszCurPos != 0) && (*pszCurPos != L'<') ) *pszCurPos++;

    // check for end tag start
    if ( (*pszCurPos == L'<') &&  (*(pszCurPos+1) == L'/') )
    {
      // check if we have the correct end tag
      if ( MADCompareKey( pszCurPos + 2, pszTagName ) )
      {
        // end tag found, now skip end tag name and closing pointy brace
        pszCurPos = MADSkipName( pszCurPos + 2 );
        pszCurPos = MADSkipWhitespace( pszCurPos );
        if ( *pszCurPos == L'>' ) pszCurPos++;

        // remove all data between (and including) start tag and end tag
        PSZ_W pszSource = pszCurPos;
        PSZ_W pszTarget = pKey;
        while ( *pszSource ) *pszTarget++ = *pszSource++;
        *pszTarget = 0;

        // removal of tag has been completed
        return( TRUE );
      } /* endif */
    } /* endif */
    pszCurPos++;
  } /* endwhile */

  return( FALSE );
}


// Add a match segment ID to the additional data section
BOOL MADAddMatchSegID( PSZ_W pszAddData, PSZ_W pszMatchIDPrefix, ULONG ulNum, BOOL fForce )
{
  HADDDATAKEY hKey;
  BOOL fMatchIDAdded = FALSE;

  // find any existing match segment ID
  hKey = MADSearchKey( pszAddData, MATCHSEGID_KEY );
  if ( (hKey != NULL) && fForce )
  {
    MADDeleteKey( hKey );
    hKey = NULL;
  } /* endif */

  // add match segment ID if non exists or the existing has been deleted and there is enough room left in the AddInfo buffer
  if ( hKey == NULL )
  {
    int iCurLen = wcslen(pszAddData);
    if ( (wcslen(pszMatchIDPrefix) + iCurLen + 23) < MAX_SEGMENT_SIZE )
    {
      swprintf( pszAddData + iCurLen, wcslen(pszAddData) - iCurLen, L"<MatchSegID ID=\"%s%lu\"/>", pszMatchIDPrefix, ulNum );
      fMatchIDAdded  = TRUE;
    } /* endif */
  } /* endif */
  return( fMatchIDAdded );
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMDocMatch                                              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMDocMatch( pszDoc1ShortName, pszDoc1LongName,          |
//|                                pszDoc2ShortName, pszDoc2LongName );        |
//+----------------------------------------------------------------------------+
//|Description:       Check if the give documents are identical. If the        |
//|                   long document names are available the long names         |
//|                   are compare else the comparism is based on the           |
//|                   short names.                                             |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pszDoc1ShortName        ptr to short name of 1st doc |
//|                   PSZ pszDoc1LongName         ptr to long name of 1st doc  |
//|                   PSZ pszDoc2ShortName        ptr to short name of 2nd doc |
//|                   PSZ pszDoc2LongName         ptr to long name of 2nd doc  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL       TRUE  = the document names match              |
//|                              FALSE = the document names do not match       |
//+----------------------------------------------------------------------------+
BOOL NTMDocMatch
(
  PSZ pszDoc1ShortName,                // ptr to short name of 1st doc
  PSZ pszDoc1LongName,                 // ptr to long name of 1st doc
  PSZ pszDoc2ShortName,                // ptr to short name of 2nd doc
  PSZ pszDoc2LongName                  // ptr to long name of 2nd doc
)
{
  BOOL fMatch = FALSE;                 // function return code

  // check if we can compare the long names
  if ( (pszDoc1LongName != NULL) && (pszDoc1LongName[0] != EOS) &&
       (pszDoc2LongName != NULL) && (pszDoc2LongName[0] != EOS) )
  {
    // base the match on the long names
    fMatch = (strcmp( pszDoc1LongName, pszDoc2LongName ) == 0 );
  }
  else
  {
    // use the short names for the match
    fMatch = (strcmp( pszDoc1ShortName, pszDoc2ShortName ) == 0 );
  } /* endif */
  return( fMatch );
} /* end of function NTMDocMatch */


BOOL
PrepareTokens
(
  PLOADEDTABLE  pTagTable,
  PBYTE         pInBuf,
  PBYTE         pTokBuf,
  PSZ_W         pString,               // pointer to string to be tokenized
  SHORT         sLanguageId,           // language ID
  PFUZZYTOK   * ppTransTokList,         // resulting list of tokens
  ULONG         ulOemCP
);




BOOL EQFBCallLCS
(
  PFUZZYTOK    pTokenList1,             // ptr to token list1
  PFUZZYTOK    pTokenList2,                      // ptr to token list2
  USHORT       usLenStr1,
  USHORT       usLenStr2,
  BOOL         fCompareAll
);

BOOL EQFBMarkModDelIns
(
    PFUZZYTOK   pTokenList1,
    PFUZZYTOK   pTokenList2,
    PFUZZYTOK  * ppFuzzy1,
    PFUZZYTOK  * ppFuzzy2,
    USHORT      usLenStr1,
    USHORT      usLenStr2
);

static
BOOL CheckForAlloc( PTMX_SENTENCE pSentence, PTMX_TERM_TOKEN * ppTermTokens, USHORT usEntries );

static
BOOL CheckForAlloc
(
   PTMX_SENTENCE      pSentence,
   PTMX_TERM_TOKEN  * ppTermTokens,
   USHORT             usEntries
)
{
  LONG     lFilled = 0L;
  USHORT   usNewAlloc = 0;
  BOOL     fOK = TRUE;
  PTMX_TERM_TOKEN  pTermTokens;

  pTermTokens = *ppTermTokens;
  lFilled = ( (PBYTE)pTermTokens - (PBYTE)pSentence->pTermTokens);
  if ( (pSentence->lTermAlloc - lFilled) <= (LONG)(usEntries * sizeof(TMX_TERM_TOKEN)) )
  {
    //remember offset of pTagEntry
    lFilled = pTermTokens - pSentence->pTermTokens;
    if (usEntries == 1)
    {
      usNewAlloc = TOK_SIZE;
    }
    else
    {
      usNewAlloc = usEntries * sizeof(TMX_TERM_TOKEN);
    }
    //allocate another 4k for pTagRecord
    fOK = UtlAlloc( (PVOID *) &pSentence->pTermTokens, pSentence->lTermAlloc,
                    pSentence->lTermAlloc + (LONG)usNewAlloc, NOMSG );
    if ( fOK )
    {
      pSentence->lTermAlloc += (LONG)usNewAlloc;

      //set new position of pTagEntry
      pTermTokens = pSentence->pTermTokens + lFilled;
    } /* endif */
  } /* endif */
  *ppTermTokens= pTermTokens;
  return(fOK);
}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TokenizeSource      Split a segment into tags and words   
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all tags and terms found.   
//------------------------------------------------------------------------------
// Function call:     TokenizeSource( EqfMemory* pClb,  //ptr to ctl block        
//                                PTMX_SENTENCE pSentence, //ptr sent struct    
//                                PSZ pTagTableName )  //name of tag table      
//------------------------------------------------------------------------------
// Input parameter:   EqfMemory*  pClb           control block                    
//                    PTMX_SENTENCE pSentence  sentence structure               
//                    PSZ      pTagTableName  name of tag table                 
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully               
//                    other       MORPH_ error code, see EQFMORPH.H for a list  
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
//       morph tokenize returning list of terms                                 
//       add term info to structure                                             
//                                                                              
//------------------------------------------------------------------------------

USHORT TokenizeSource
(
   EqfMemory* pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang                      // source language
)
{
  PVOID     pTokenList = NULL;         // ptr to token table
  BOOL      fOK;                       // success indicator
  PBYTE     pTagEntry;                 // pointer to tag entries
  PTMX_TERM_TOKEN pTermTokens;         // pointer to term tokens
  PLOADEDTABLE pTable = NULL;          // pointer to tagtable
  PTMX_TAGTABLE_RECORD pTagRecord;     // pointer to record
  USHORT    usLangId;                  // language id
  USHORT    usRc = NO_ERROR;           // returned value
  USHORT    usFilled;                  // counter
  USHORT    usTagEntryLen;             // length indicator
  CHAR      szString[MAX_FNAME];       // name without extension
  USHORT    usStart;                   // position counter
  PSTARTSTOP pStartStop = NULL;        // ptr to start/stop table
  int        iIterations = 0;
  USHORT     usAddEntries = 0;
  PSZ_W  pszNormStringPos = pSentence->pNormString; // current pNpormString output pointer

  /********************************************************************/
  /* normalize \r\n combinations in input string ..                   */
  /********************************************************************/
  {
    PSZ_W  pSrc = pSentence->pInputStringWithNPTagHashes;
    PSZ_W  pTgt = pSentence->pInputStringWithNPTagHashes;
    CHAR_W c;

    while ( (c = *pSrc++) != NULC )
    {
      /****************************************************************/
      /* in case of \r we have to do some checking, else only copy..  */
      /****************************************************************/
      if ( c == '\r' )
      {
        if ( *pSrc == '\n' )
        {
          /************************************************************/
          /* ignore the character                                     */
          /************************************************************/
        }
        else
        {
          *pTgt++ = c;
        } /* endif */
      }
      else
      {
        *pTgt++ = c;
      } /* endif */
    } /* endwhile */
    *pTgt = EOS;
  }

  //allocate 4K pTokenlist for TaTagTokenize
  fOK = UtlAlloc( (PVOID *) &(pTokenList), 0L, (LONG)TOK_SIZE, NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    //LOG_DEVELOP_MSG << "TokenizeSourceEx2";
    pTagRecord = pSentence->pTagRecord;
    pTermTokens = pSentence->pTermTokens;
    pTagEntry = (PBYTE)pTagRecord;
    pTagEntry += sizeof(TMX_TAGTABLE_RECORD);
    
    RECLEN(pTagRecord) = 0;
    pTagRecord->usFirstTagEntry = (USHORT)(pTagEntry - (PBYTE)pTagRecord);

    //get id of tag table, call
    Utlstrccpy( szString, UtlGetFnameFromPath( pTagTableName ), DOT );

    pTagRecord->usTagTableId = 0;

    //get lang id of source lang for morphtokenize call
    usLangId = 0;
    if ( !usRc )
    {
      //load tag table for tokenize function
      usRc = TALoadTagTableExHwnd( szString, &pTable, FALSE,
                                   TALOADUSEREXIT | TALOADPROTTABLEFUNC |
                                   TALOADCOMPCONTEXTFUNC,
                                   FALSE, NULLHANDLE );
      if ( usRc )
      {
        USHORT usAction = UtlQueryUShort( QS_MEMIMPMRKUPACTION);
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_TA_ACC_TAGTABLE);
        if ( usAction == MEMIMP_MRKUP_ACTION_RESET ) {
           CHAR  *ptrMarkup ;
           ptrMarkup = strstr( pTagTableName, szString ) ;
           if ( ptrMarkup ) {
              strcpy( ptrMarkup, "OTMUTF8.TBL" ) ;
              strcpy( szString, "OTMUTF8" ) ;
              usRc = TALoadTagTableExHwnd( szString, &pTable, FALSE,
                                           TALOADUSEREXIT | TALOADPROTTABLEFUNC |
                                           TALOADCOMPCONTEXTFUNC,
                                           FALSE, NULLHANDLE );
              if ( usRc )
                 LOG_AND_SET_RC(usRc, T5INFO, ERROR_TA_ACC_TAGTABLE);
           } 
        } else
        if ( usAction == MEMIMP_MRKUP_ACTION_SKIP ) {
           LOG_AND_SET_RC(usRc, T5INFO, SEG_SKIPPED_BAD_MARKUP);
        }
      } /* endif */

    } /* endif */

    if ( !usRc )
    {
      // build protect start/stop table for tag recognition
       usRc = TACreateProtectTableWEx( pSentence->pInputStringWithNPTagHashes, pTable, 0,
                                   (PTOKENENTRY)pTokenList,
                                   TOK_SIZE, &pStartStop,
                                   pTable->pfnProtTable,
                                   pTable->pfnProtTableW, 1, 0);


      while ((iIterations < 10) && (usRc == EQFRS_AREA_TOO_SMALL))
      {
        // (re)allocate token buffer
        LONG lOldSize = (usAddEntries * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;
        LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;

        if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
        {
          usAddEntries += 128;
          iIterations++;
        }
        else
        {
          iIterations = 10;    // force end of loop
        } /* endif */
        // retry tokenization
        if (iIterations < 10 )
        {
           usRc = TACreateProtectTableWEx( pSentence->pInputStringWithNPTagHashes, pTable, 0,
                                      (PTOKENENTRY)pTokenList,
                                       (USHORT)lNewSize, &pStartStop,
                                       pTable->pfnProtTable,
                                       pTable->pfnProtTableW, 1, 0 );
        } /* endif */

      } /* endwhile */
    } /* endif */

    if ( !usRc )
    {
      USHORT usEntries = 0; // number of entries in start stop table

      PSTARTSTOP pEntry = pStartStop;
      while ( (pEntry->usStart != 0) ||
              (pEntry->usStop != 0)  ||
              (pEntry->usType != 0) )
      {
        switch ( pEntry->usType )
        {
          case UNPROTECTED_CHAR :
            // handle translatable text
            {
              PFLAGOFFSLIST pTerm;                   // pointer to terms list
              PFLAGOFFSLIST pTermList = NULL;        // pointer to offset/length term list
              ULONG     ulListSize = 0;    
              USHORT    usTokLen = pEntry->usStop - pEntry->usStart + 1;

              CHAR_W chTemp = pSentence->pInputStringWithNPTagHashes[pEntry->usStop+1]; // buffer for character values
              pSentence->pInputStringWithNPTagHashes[pEntry->usStop+1] = EOS;

              usRc = NTMMorphTokenizeW( usLangId,
                                       pSentence->pInputStringWithNPTagHashes + pEntry->usStart,
                                       &ulListSize, (PVOID *)&pTermList,
                                       MORPH_FLAG_OFFSLIST );

              pSentence->pInputStringWithNPTagHashes[pEntry->usStop+1] = chTemp;

              if ( usRc == MORPH_OK )
              {
                pTerm = pTermList;
                fOK = CheckForAlloc( pSentence, &pTermTokens, 1 );
                if ( !fOK )
                {
                  LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
                }
                else
                {
                  usStart = pEntry->usStart;
                  if ( pTerm )
                  {
                      while ( pTerm->usLen && !usRc )
                      {
                        if ( !(pTerm->lFlags & TF_NEWSENTENCE) )
                        {                         
                          fOK = CheckForAlloc( pSentence, &pTermTokens, 1 );
                          if ( !fOK )
                          {
                            LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
                          }
                          else
                          {
                            pTermTokens->usOffset = pTerm->usOffs + usStart;
                            pTermTokens->usLength = pTerm->usLen;
                            pTermTokens->usHash = 0;
                            pTermTokens++;
                          }
                        } /* endif */
                        pTerm++;
                      } /* endwhile */
                  } /* endif */
                  memcpy( pszNormStringPos, pSentence->pInputStringWithNPTagHashes + pEntry->usStart, usTokLen * sizeof(CHAR_W));
                  pSentence->usNormLen =  pSentence->usNormLen + usTokLen;
                  pszNormStringPos += usTokLen;
                } /* endif */
              } /* endif */
              UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
            } /* end case UNPROTECTED_CHAR */
            break;
          default :
            // handle not-translatable data
            {
              // if next start/stop-entry is a protected one ...
              if ( ((pEntry+1)->usStart != 0) &&
                   ((pEntry+1)->usType != UNPROTECTED_CHAR) )
              {
                // enlarge next entry and ignore current one
                (pEntry+1)->usStart = pEntry->usStart;
              }
              else
              {
                // add tag data
                usTagEntryLen = sizeof(TMX_TAGENTRY) +
                                (pEntry->usStop - pEntry->usStart + 1) * sizeof(CHAR_W);
                if ( (pSentence->lTagAlloc - (pTagEntry - (PBYTE)pSentence->pTagRecord))
                                                       <= (SHORT)usTagEntryLen )
                {
                  LONG lBytesToAlloc = get_max( ((LONG)TOK_SIZE), ((LONG)usTagEntryLen) );

                  //remember offset of pTagEntry
                  usFilled = (USHORT)(pTagEntry - (PBYTE)pSentence->pTagRecord);

                  //allocate another 4k for pTagRecord
                  fOK = UtlAlloc( (PVOID *) &pSentence->pTagRecord, pSentence->lTagAlloc,
                                  pSentence->lTagAlloc + lBytesToAlloc, NOMSG );
                  if ( fOK )
                  {
                    pSentence->lTagAlloc += lBytesToAlloc;

                    //set new position of pTagEntry
                    pTagEntry = (PBYTE)(pSentence->pTagRecord) + usFilled;
                    pTagRecord = pSentence->pTagRecord;
                  } /* endif */
                } /* endif */

                if ( !fOK )
                {
                  LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
                }
                else
                {
                  ((PTMX_TAGENTRY)pTagEntry)->usOffset = pEntry->usStart;
                  ((PTMX_TAGENTRY)pTagEntry)->usTagLen =
                    (pEntry->usStop - pEntry->usStart + 1);
                  memcpy( &(((PTMX_TAGENTRY)pTagEntry)->bData),
                          pSentence->pInputStringWithNPTagHashes + pEntry->usStart,
                          ((PTMX_TAGENTRY)pTagEntry)->usTagLen * sizeof(CHAR_W));
                  pTagEntry += usTagEntryLen;
                } /* endif */
              } /* endif */
            } /* end default */
            break;
        } /* endswitch */
        pEntry++;
        usEntries++;
      } /* endwhile */

      /********************************************************/
      /* check if we filled at least one term token -- if not */
      /* use a dummy token - just to get an exact match ...   */
      /********************************************************/
      if ( pTermTokens == pSentence->pTermTokens )
      {
        pTermTokens->usOffset = 0;
        pTermTokens->usLength = (USHORT)UTF16strlenCHAR( pSentence->pInputStringWithNPTagHashes );
        pTermTokens->usHash = 0;
        pTermTokens++;
      } /* endif */

      fOK = CheckForAlloc( pSentence, &pTermTokens, 3 );
      if ( !fOK )
      {
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
      }

      //set total tag record length
      RECLEN(pTagRecord) = pTagEntry - (PBYTE)pTagRecord;
    } /* endif */
  } /* endif */

  //release allocated memory
  if ( pStartStop ) UtlAlloc( (PVOID *) &pStartStop, 0L, 0L, NOMSG );
  if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );

  //free tag table / decrement use count
  if ( pTable != NULL )
  {
    TAFreeTagTable( pTable );
  } /* endif */

  return ( usRc );
}


USHORT NTMTokenizeW
(
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
);

USHORT NTMMorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PULONG   pulBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  ULONG     ulTermBufUsed = 0;         // number of bytes used in term buffer

  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pulBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pulBufferSize != 0) ) )
  {
    LOG_AND_SET_RC(usRC, T5INFO, MORPH_INV_PARMS);
  } /* endif */

///********************************************************************/
///* Get language control block pointer  -- not needed                */
///********************************************************************/
//if ( usRC == MORPH_OK )
//{
//  usRC = MorphGetLCB( sLanguageID, &pLCB );
//} /* endif */


  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
      usRC = NTMTokenizeW(pszInData,
                          pulBufferSize,
                          ppTermList,
                          usListType
                          );
    }
    else
    {
      usRC = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                    pulBufferSize,
                                    &ulTermBufUsed,
                                   (PSZ_W)EMPTY_STRING,
                                    0,
                                    0,
                                    0L,
                                    usListType );
    } /* endif */
  } /* endif */

  return( usRC );
} /* endof NTMMorphTokenizeW */


USHORT NTMTokenizeW
(
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
)
{
  USHORT     usReturn = 0;             // returncode
  ULONG      ulTermBufUsed = 0;        // amount of space used in term buffer
  PSZ_W      pTerm;                    // pointer to current token string
  PSZ_W      pszCurPos = pszInData;    // current position in input data
  BOOL       fOffsList;                // TRUE = return a offset/length list
  LONG       lFlags;                   // flags for current term/token
  BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
  BOOL       fSkip, fNewSentence,      // token processing flags
             fSingleToken, fEndOfToken;
  CHAR_W     c, d;

  fOffsList  = (usListType == MORPH_OFFSLIST) ||
               (usListType == MORPH_FLAG_OFFSLIST);

  /********************************************************************/
  /* Always assume start of a new sentence                            */
  /********************************************************************/
  if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
       (usListType == MORPH_FLAG_OFFSLIST) )
  {
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   L" ",
                                   1,
                                   0, // no offset possible
                                   TF_NEWSENTENCE,
                                   usListType );
  } /* endif */

  /********************************************************************/
  /* Initialize processing flags                                      */
  /********************************************************************/
  pTerm    = pszCurPos;
  fAllCaps = TRUE;
  fAlNum   = TRUE;
  fNumber  = TRUE;

  /********************************************************************/
  /* Work on input data                                               */
  /********************************************************************/
  while ( !usReturn && ((c = *pszCurPos)!= NULC) )
  {
    /******************************************************************/
    /* Get type of processing required for current character          */
    /******************************************************************/
    fSkip         = FALSE;
    fNewSentence  = FALSE;
    fSingleToken  = FALSE;
    fEndOfToken   = FALSE;
    int tagLen = 0;

    if( pszCurPos[0] == L'<' ) //probably tag start -> skip tag token 
    {
      wchar_t* s = pszCurPos;
      if(wcsncasecmp(L"<ph" , s, 3) == 0 ||
         wcsncasecmp(L"<bpt", s, 4) == 0 ||
         wcsncasecmp(L"<ept", s, 4) == 0
         ){
          while( *s++ != '\0'){
            if( s[0] == '/' && s[1] == '>' ){
              tagLen = s - pszCurPos + 2;
              fEndOfToken = TRUE;
              break;
            }
          }      
        }
    }
    
    
    switch ( c )
    {
      case L'.' :
      case L'!' :
      case L'?' :
        switch ( pszCurPos[1] )
        {
          case L' ':
          case L'\n':
          case L'\0':
            fNewSentence = TRUE;
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            if ( c != '.'  && c != L'.' )
            {
              fEndOfToken  = TRUE;
              fSingleToken  = TRUE;
            }
            else
            {
              fAllCaps = FALSE;
              fAlNum   = FALSE;
            } /* endif */
            break;
        } /* endswitch */
        break;

      case L'\n' :
      case L' ' :
        fEndOfToken  = TRUE;
        fSkip        = TRUE;
        break;

      case L'$' :
      case L'#' :
        fAllCaps = FALSE;
        fAlNum   = FALSE;
        break;

      //case ':' :
      case L':' :
        
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        
        break;

      case L'/' :
      case L'\\' :
      case L',' :
      case L';' :
        switch ( pszCurPos[1] )
        {
          case L' ':
          case L'\n':
          case L'\0':
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            fAllCaps = FALSE;
            fAlNum   = FALSE;
            fNumber  = FALSE;
            break;
        } /* endswitch */
        break;
      
      case 8216://‘ symbol
      case 8217://’ symbol
      case L'-' :            // dash within word or as 'Gedankenstrich'
        if ((((d = pszCurPos[1]) == L' ') || (d == L'\n') || ( d == L'\0' )) )
        {
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        } /* endif */
        break;

      case L'(' :
      case L')' :
      case L'\"':
      case 8243 ://″
      case L'\'':
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        break;
      case L'<':
        if(tagLen == 0){
          fSingleToken = TRUE;
          fEndOfToken  = TRUE;            
        }
        break;
      default:
        if ( fAlNum ) //&& !chIsText[c] )
        {
          PUCHAR pTextTable;          
          UtlQueryCharTable( IS_TEXT_TABLE, &pTextTable );
          
          if( c > 255 ){
            if( !iswalpha(c) ){
              fAlNum = false;
            }
          }else{
            if ( !pTextTable[c] )
            {
              fAlNum = FALSE;
            }
          }
        } /* endif */
        if ( fNumber && !iswdigit(c) )
        {
          fNumber  = FALSE;
        } /* endif */
        if ( fAllCaps && (!iswalpha(c) || iswlower(c)) )
        {
          fAllCaps = FALSE;
        } /* endif */
        break;
    } /* endswitch */

    /******************************************************************/
    /* Terminate current token and start a new one if requested       */
    /******************************************************************/
    if ( fEndOfToken && (pTerm < pszCurPos) )
    {
      lFlags = 0L;
      if ( fAllCaps ) lFlags |= TF_ALLCAPS;
      if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
      if ( fNumber )  lFlags |= TF_NUMBER;
      usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                     pulTermListSize,
                                     &ulTermBufUsed,
                                     pTerm,
                                     (USHORT)(pszCurPos - pTerm),
                                     (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                     lFlags,
                                     usListType );
      pTerm    = pszCurPos;
      fAllCaps = TRUE;
      fAlNum   = TRUE;
      fNumber  = TRUE;
    } /* endif */

    /******************************************************************/
    /* Handle single character tokens                                 */
    /******************************************************************/
    if ( fSingleToken && !usReturn )
    {
      usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                     pulTermListSize,
                                     &ulTermBufUsed,
                                     pszCurPos,
                                     1,
                                     (USHORT)(fOffsList ? (pszCurPos-pszInData) : 0),
                                     TF_NOLOOKUP,
                                     usListType );
      pTerm    = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Handle start of a new sentence                                 */
    /******************************************************************/
    if ( fNewSentence && !usReturn )
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
           (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                       pulTermListSize,
                                       &ulTermBufUsed,
                                       L" ",
                                       1,
                                       0, // no offset possible
                                       TF_NEWSENTENCE,
                                       usListType );
      } /* endif */
    } /* endif */


    /******************************************************************/
    /* Handle skip flag                                               */
    /******************************************************************/
    if ( fSkip )
    {
      pTerm = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Continue with next character                                   */
    /******************************************************************/
    if (tagLen > 0){
      pszCurPos += tagLen;
      pTerm = pszCurPos;
    }else if ( c != EOS ) pszCurPos++;
  } /* endwhile */

  /********************************************************************/
  /* Process any pending token                                        */
  /********************************************************************/
  if ( !usReturn && (pTerm < pszCurPos) )
  {
    lFlags = 0L;
    if ( fAllCaps ) lFlags |= TF_ALLCAPS;
    if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
    if ( fNumber )  lFlags |= TF_NUMBER;
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   pTerm,
                                   (USHORT)(pszCurPos - pTerm),
                                   (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                   lFlags,
                                   usListType );
  } /* endif */

  /*****************************************************************/
  /* terminate the term list                                       */
  /*****************************************************************/
  if ( !usReturn )
  {
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   (PSZ_W)EMPTY_STRING,
                                   0,
                                   0,
                                   0L,
                                   usListType );
  } /* endif */

  return (usReturn);
} /* end of function TOKENIZE */

