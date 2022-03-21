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
#include "../pluginmanager/OtmMemoryPlugin.h"
#include "../pluginmanager/OtmMemory.h"
#include "../utilities/LogWrapper.h"
#include "../utilities/EncodingHelper.h"
#include "MemoryFactory.h"
#include "EQFMORPI.H"

// EQF.H is included by otmmemory.h
// #include <eqf.h>                  // General Translation Manager include file EQF:H 

extern "C"
{
#include <OTMFUNC.H>               // header file of OpenTM2 API functions
}

#include "vector"
#include <wctype.h>

#include "win_types.h"
#include "LogWrapper.h"
//#include "EQFFUZZ.H"
// prototypes for helper functions


/* \brief Initialize the Memory Plugin Mapper
*/


__declspec(dllexport)
void InitTMPluginWrapper()
{
  // access plugin manager
	// PluginManager* thePluginManager = PluginManager::getInstance();

}



/*   Function Name:   FillMemoryListBox */
USHORT FillMemoryListBox
(
 PLISTCOMMAREA    pCommArea,
 WPARAM           mp1,           // contains the list box handle
 LPARAM           mp2            // contains the address to the object name
)

{
  BOOL              fOK = TRUE;      // Process return code

  pCommArea;

  // Check whether object name is correct
  if ( strcasecmp((const char *) PVOIDFROMMP2(mp2), MEMORY_ALL) )
  {
     fOK = FALSE;
  } /* endif */

  if ( fOK )
  {
    // Stop list box enabeling
    //ENABLEUPDATEHWND_FALSE( (HWND)mp1 );

    // GQ 2014/05/12: Use MemoryFactory to get the list of memories (old code has been commented out and left as reference in case of problems)

    // old code start
    //if ( fOK )
    //{
    //  SHORT  sItemCount;
    //  SHORT  sItemIndex;

    //  // The window handle seems to be Ok
    //  // Establish pointer to entry in TM list and to work string
    //  sItemCount = QUERYITEMCOUNTHWND( pCommArea->hwndLB );

    //  if ( sItemCount != LIT_ERROR )
    //  {
    //    for ( sItemIndex = 0; sItemIndex < sItemCount; sItemIndex++ )
    //    {
    //      if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
    //                       MP1FROMSHORT( sItemIndex ), NULL ) )
    //      {
    //         // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
    //  //         QUERYITEMTEXTHWND( pCommArea->hwndLB, sItemIndex, pCommArea->szBuffer );
    //  SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItemIndex, (LPARAM)pCommArea->szBuffer );

    //  INSERTITEMHWND( (HWND) mp1, UtlParseX15( pCommArea->szBuffer, 0 ) );
    //        usItemRcCount++;
    //      } /* endif */
    //    } /* endfor */
    //  } /* endif */
    // old code end

      //MemoryFactory *pFactory = MemoryFactory::getInstance();
      //pFactory->listMemories( AddMemToListBox, (PVOID)mp1, FALSE );
      

      // Enable the memory database list box
      //ENABLEUPDATEHWND_TRUE( (HWND)mp1 );
  } /* endif */

  //return  ( QUERYITEMCOUNTHWND( ((HWND)mp1)) );
} /* end of function FillMemoryListBox */


/*
SHORT EQFNTMSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  pBTIda; pUserData; pusLen;

//  	return theMemory->CPP_EQFNTMSign(  pBTIda,  pUserData,  pusLen);
    return( 1 );

}//*/

USHORT NTMOrganizeIndexFile
(
  PTMX_CLB pTmClb               // ptr to control block,
)
{
  pTmClb;
//	return theMemory->CPP_NTMOrganizeIndexFile(  pTmClb);
    return( 1 );

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

USHORT TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  htm; szMemPath; pstPutIn; ulUpdKey; usUpdTarget; usFlags; usMsgHandling;

//	return theMemory->CPP_TmUpdSegW(  htm,  szMemPath,  pstPutIn,  ulUpdKey,  usUpdTarget,  usFlags,  usMsgHandling);
    return( 1 );

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

// prepare the match segment ID prefix using the provided TM_ID and StoreID
BOOL MADPrepareMatchSegIDPrefix( PSZ pszTM_ID, PSZ pszStoreID, PSZ pszMatchID )
{
  BOOL fMatchIDPrepared = FALSE;
  if ( (pszTM_ID != NULL) && (*pszTM_ID != EOS) )
  {
    fMatchIDPrepared = TRUE;

    while( *pszTM_ID != EOS )
    {
      if ( *pszTM_ID == '_' )
      { 
        *pszMatchID++ = '+';
      }
      else if ( (*pszTM_ID == '<') || (*pszTM_ID == '>') )
      { 
        *pszMatchID++ = '-';
      }
      else
      {
        *pszMatchID++ = *pszTM_ID;
      } /* endif */
      pszTM_ID++;
    } /* endwhile */
    *pszMatchID++ = '_';
  } /* endif */

  if ( (pszStoreID != NULL) && (*pszStoreID != EOS) )
  {
    fMatchIDPrepared = TRUE;

    while( *pszStoreID != EOS )
    {
      if ( *pszStoreID == '_' )
      { 
        *pszMatchID++ = '+';
      }
      else if ( (*pszStoreID == '<') || (*pszStoreID == '>') )
      { 
        *pszMatchID++ = '-';
      }
      else
      {
        *pszMatchID++ = *pszStoreID;
      } /* endif */
      pszStoreID++;
    } /* endwhile */
    *pszMatchID++ = '_';
  } /* endif */

  *pszMatchID = EOS;

  return( fMatchIDPrepared );
} /* end of MADPrepareMatchSegIDPrefix */


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


/**********************************************************************/
/* find the differences in two strings (w/o cleanup if consecutives   */
/* delete and inserts)                                                */
/**********************************************************************/
BOOL
EQFBFindDiffEx
(
  PVOID pTable,                        // pointer to loaded tagtable
  PBYTE pInBuf,                        // pointer to input buffer
  PBYTE pTokBuf,                       // pointer to temp token buffer
  PSZ_W pString1,                      // first string passed
  PSZ_W pString2,                      // second string
  SHORT sLanguageId,                   // language id to be used for pString1
  PVOID * ppFuzzyTok,                  // returned token list of pString2
  PVOID * ppFuzzyTgt,                   // returned token list of pString1
  ULONG   ulOemCP
)
{
  BOOL fOK = TRUE;                     // success indicator
  USHORT       usLenStr1 = 0;          // length of string 1 (in num. of tokens)
  USHORT       usLenStr2 = 0;          // length of string 2 (in num. of tokens)
  PFUZZYTOK    pTokenList2 = NULL;     // pointer to token lists
  PFUZZYTOK    pTokenList1 = NULL;     // pointer to token lists
  PFUZZYTOK    pToken;                 // pointer to token lists
  PLOADEDTABLE pTagTable = (PLOADEDTABLE) pTable;

  /******************************************************************/
  /* prepare tokens for String1 and string 2                        */
  /******************************************************************/
  fOK = PrepareTokens( //pDoc,
                       pTagTable,
                       pInBuf,
                       pTokBuf,
                       pString1, sLanguageId, &pTokenList1, ulOemCP );
  if ( fOK )
  {
    fOK = PrepareTokens( // pDoc,
                         pTagTable,
                         pInBuf,
                         pTokBuf,
                         pString2, sLanguageId, &pTokenList2, ulOemCP );
  } /* endif */
  if (fOK )
  {
    /********************************************************************/
    /* get number of tokens in strings                                  */
    /********************************************************************/
    NUMBEROFTOKENS(usLenStr1, pTokenList1);
    NUMBEROFTOKENS(usLenStr2, pTokenList2);

    /******************************************************************/
    /* call LCS and compare tokens with strncmp                       */
    /******************************************************************/
    fOK = EQFBCallLCS(pTokenList1, pTokenList2,
                      usLenStr1, usLenStr2, TRUE);
  } /* endif */
  /********************************************************************/
  /* get number of tokens in strings                                  */
  /********************************************************************/
  if ( fOK )
  {
    /****************************************************************/
    /* adjust length to avoid non-marked spaces between marked tokens*/
    /****************************************************************/
    pToken = pTokenList1;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */

    pToken = pTokenList2;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */
    /****************************************************************/
    /* allocate space for fuzzy tokens ,.,,,                        */
    /* this area MUST be freed by the calling application           */
    /****************************************************************/
    fOK = EQFBMarkModDelIns( pTokenList1, pTokenList2,
                          (PFUZZYTOK *)ppFuzzyTgt, (PFUZZYTOK *)ppFuzzyTok,
                         usLenStr1, usLenStr2);

  } /* endif */
  /********************************************************************/
  /* free any allocated resources                                     */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pTokenList1, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pTokenList2, 0L, 0L, NOMSG );
  return( fOK );
} /* end of function EQFBFindDiffEx */

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
// Function call:     TokenizeSource( PTMX_CLB pClb,  //ptr to ctl block        
//                                PTMX_SENTENCE pSentence, //ptr sent struct    
//                                PSZ pTagTableName )  //name of tag table      
//------------------------------------------------------------------------------
// Input parameter:   PTMX_CLB  pClb           control block                    
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
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion                     // version of TM
)
{
  ULONG ulSrcCP = GetLangOEMCP( pSourceLang);
  return( TokenizeSourceEx( pClb, pSentence, pTagTableName, pSourceLang, usVersion, ulSrcCP ) );
}

USHORT TokenizeSourceEx
(
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion,                    // version of TM
   ULONG  ulSrcCP                       // OEM CP of source language     
)
{
    return( TokenizeSourceEx2( pClb, pSentence, pTagTableName, pSourceLang, usVersion, ulSrcCP, 0 ) );
}

USHORT TokenizeSourceEx2
(
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion,                    // version of TM
   ULONG  ulSrcCP,                      // OEM CP of source language     
   int iMode                            // mode to be passed to create protect table function
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
    PSZ_W  pSrc = pSentence->pInputString;
    PSZ_W  pTgt = pSentence->pInputString;
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
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    LogMessage(DEVELOP, "TokenizeSourceEx2");
    pTagRecord = pSentence->pTagRecord;
    pTermTokens = pSentence->pTermTokens;
    pTagEntry = (PBYTE)pTagRecord;
    auto len = &(pSentence->lTermAlloc) - (long)pSentence;
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
        usRc = ERROR_TA_ACC_TAGTABLE;
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
                 usRc = ERROR_TA_ACC_TAGTABLE;
           } 
        } else
        if ( usAction == MEMIMP_MRKUP_ACTION_SKIP ) {
           usRc = SEG_SKIPPED_BAD_MARKUP ;
        }
      } /* endif */

    } /* endif */

    if ( !usRc )
    {
      // build protect start/stop table for tag recognition
       usRc = TACreateProtectTableWEx( pSentence->pInputString, pTable, 0,
                                   (PTOKENENTRY)pTokenList,
                                   TOK_SIZE, &pStartStop,
                                   pTable->pfnProtTable,
                                   pTable->pfnProtTableW, ulSrcCP, iMode);


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
           usRc = TACreateProtectTableWEx( pSentence->pInputString, pTable, 0,
                                      (PTOKENENTRY)pTokenList,
                                       (USHORT)lNewSize, &pStartStop,
                                       pTable->pfnProtTable,
                                       pTable->pfnProtTableW, ulSrcCP, iMode );
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

              CHAR_W chTemp = pSentence->pInputString[pEntry->usStop+1]; // buffer for character values
              pSentence->pInputString[pEntry->usStop+1] = EOS;

              usRc = NTMMorphTokenizeW( usLangId,
                                       pSentence->pInputString + pEntry->usStart,
                                       &ulListSize, (PVOID *)&pTermList,
                                       MORPH_FLAG_OFFSLIST, usVersion );

              pSentence->pInputString[pEntry->usStop+1] = chTemp;

              if ( usRc == MORPH_OK )
              {
                pTerm = pTermList;
                fOK = CheckForAlloc( pSentence, &pTermTokens, 1 );
                if ( !fOK )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
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
                            usRc = ERROR_NOT_ENOUGH_MEMORY;
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
                  memcpy( pszNormStringPos, pSentence->pInputString + pEntry->usStart, usTokLen * sizeof(CHAR_W));
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
                  } /* endif */
                } /* endif */

                if ( !fOK )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                  ((PTMX_TAGENTRY)pTagEntry)->usOffset = pEntry->usStart;
                  ((PTMX_TAGENTRY)pTagEntry)->usTagLen =
                    (pEntry->usStop - pEntry->usStart + 1);
                  memcpy( &(((PTMX_TAGENTRY)pTagEntry)->bData),
                          pSentence->pInputString + pEntry->usStart,
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
        pTermTokens->usLength = (USHORT)UTF16strlenCHAR( pSentence->pInputString );
        pTermTokens->usHash = 0;
        pTermTokens++;
      } /* endif */

      fOK = CheckForAlloc( pSentence, &pTermTokens, 3 );
      if ( !fOK )
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
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

USHORT MorphAddTermToList2W
(
  PSZ_W    *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // alloc. size of term list in # of w's
  PULONG   pulUsed,                    // used # of w's in term list
  PSZ_W    pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term in # of w's
  USHORT   usOffs,                     // offset of term in # of w's???
  ULONG    ulFlags,                    // flags describing the term
  USHORT   usListType                  // type of list
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  ULONG    ulDataLenBytes = 0;         // length of data to add in # of bytes
  ULONG    ulDataLen = 0;              // length of data to add in w's
  BOOL     fOK  = TRUE;                // internal OK flag

  PBYTE    pList = (PBYTE)*ppList;     // ptr to term list
  PSZ_W    *pAnchor = NULL;            // ptr to anchor for list

  /********************************************************************/
  /* position to last block for large term lists                      */
  /********************************************************************/
  if ( usListType == MORPH_LARGE_ZTERMLIST )
  {
    PSZ_W  pNextList;

    // use caller's list pointer as default anchor
    pAnchor = ppList;

    if ( pList != NULL )
    {
      pNextList = *((PSZ_W *)pList);

      while ( pNextList != NULL )
      {
        pAnchor = (PSZ_W *)pList;
        pList = (PBYTE)pNextList;
        pNextList = *((PSZ_W *)pList);
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get required size for new entry                                  */
  /********************************************************************/
  switch ( usListType )
  {
    case MORPH_OFFSLIST :
      /******************************************************************/
      /* space for offset and length of term plus end of term list      */
      /* indicator which is a zero offset and length value              */
      /******************************************************************/
      ulDataLenBytes = 4 * sizeof(USHORT);
      break;

    case MORPH_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    case MORPH_FLAG_OFFSLIST :
      /******************************************************************/
      /* space for flag, offset and length of term plus end of term list*/
      /* indicator which is a zero flag, offset and length value        */
      /******************************************************************/
      //ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(USHORT));
      ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(wchar_t));
      break;

    case MORPH_FLAG_ZTERMLIST :
      /******************************************************************/
      /* space for flag, term, term delimiter and end of term list      */
      /* indicator which is a empty flag and a term delimiter           */
      /******************************************************************/
      ulDataLenBytes = sizeof(ULONG) + (usLength + 1)*sizeof(CHAR_W)
                      + sizeof(ULONG) + sizeof(CHAR_W);
      break;

    case MORPH_LARGE_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    default :
      fOK = FALSE;
      break;
  } /* endswitch */

  /********************************************************************/
  /* enlarge term list if necessary                                   */
  /********************************************************************/
  ulDataLen = ulDataLenBytes / sizeof(CHAR_W);        // # of w's needed
  if ( fOK && ((*pulUsed + ulDataLen) > *pulSize) )
  {
    LONG lOldLen = 0L;
    LONG max = TERMLIST_INCR > ulDataLen? TERMLIST_INCR : ulDataLen ;
    LONG lNewLen = ((LONG)*pulSize) + max ;

    LONG lMaxLen = (usListType == MORPH_LARGE_ZTERMLIST) ?
                                          MORPH_LARGE_ZTERMLIST_LEN : MAX_ALLOC;
    lNewLen = lNewLen * sizeof(CHAR_W);         //  len in # of bytes
    lOldLen = (*pulSize) * sizeof(CHAR_W);
    if ( lNewLen >= lMaxLen )
    {
      // for large termlists allocate another buffer else set error condition
      if ( usListType == MORPH_LARGE_ZTERMLIST )
      {
        lNewLen = (LONG)sizeof(PSZ_W) +
                   (LONG)sizeof(CHAR_W) * max;
        *pulSize = 0;                  // no size yet
        lOldLen = 0L;
        pAnchor  = (PSZ_W *)pList;       // use current list as anchor
      }
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pList, lOldLen, lNewLen, NOMSG );
      if ( fOK )
      {  // set caller's termlist pointer
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           *pAnchor = (PSZ_W)pList;
           if (*pulSize == 0 )         // new block???
           {
             *pulUsed = sizeof(PSZ_W)/ sizeof(CHAR_W);   // leave room for pointer to next block
           } /* endif */
           *pulSize = lNewLen / sizeof(CHAR_W);
         }
         else
         {
           *pulSize = lNewLen / sizeof(CHAR_W);  //@@+= max( TERMLIST_INCR, ulDataLength );??
           *ppList = (PSZ_W)pList;
         } /* endif */
      }
      else
      {
         usRC = MORPH_NO_MEMORY;

         // free the term list in case of erros
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           pList = (PBYTE)*ppList;
           while ( pList != NULL )
           {
             PSZ_W pNextList = *((PSZ_W *)pList);
             UtlAlloc( (PVOID *)&pList, 0L, 0L, NOMSG );
             pList = (PBYTE)pNextList;
           } /* endwhile */
         }
         else
         {
           UtlAlloc( (PVOID *)ppList, 0L, 0L, NOMSG );
         } /* endif */
         *ppList = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* add term to term list (pList is a PBYTE ptr                      */
  /********************************************************************/
  if ( fOK )
  {
    ULONG  ulByteUsedInList = (*pulUsed) * sizeof(CHAR_W);
    switch ( usListType )
    {
      case MORPH_OFFSLIST :
        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += 1;//sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1;//sizeof(USHORT)/sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_LARGE_ZTERMLIST :
      case MORPH_ZTERMLIST :
        if ( usLength )
        {
          memcpy( (PBYTE)(pList + ulByteUsedInList),
                  (PBYTE) pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ((*pulUsed) * sizeof(CHAR_W)))) = EOS;        /* 10-11-16 */
        *pulUsed += 1;
        ulByteUsedInList += sizeof(CHAR_W);
        break;

      case MORPH_FLAG_OFFSLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += 1;//TEMPORARY_HARDCODED sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1; //sizeof(USHORT) / sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_FLAG_ZTERMLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        if ( usLength )
        {
          memcpy( pList + ulByteUsedInList,
                 (PBYTE)pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ulByteUsedInList)) = EOS;     /* 10-11-16 */
        *pulUsed += 1;
        break;

      default :
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* return usRC to calling function                                  */
  /********************************************************************/
  return( usRC );

} /* end of function MorphAddTermToList2W */

USHORT NTMTokenizeW
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
);

USHORT NTMMorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PULONG   pulBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  ULONG     ulTermBufUsed = 0;         // number of bytes used in term buffer

  sLanguageID;                         // avoid compiler warning
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pulBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pulBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
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
      usRC = NTMTokenizeW( NULL,
                          pszInData,
                          pulBufferSize,
                          ppTermList,
                          usListType,
                          usVersion );
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
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT     usReturn = 0;             // returncode
  ULONG      ulTermBufUsed = 0;        // amount of space used in term buffer
  PSZ_W      pTerm;                    // pointer to current token string
  PSZ_W      pszCurPos;                // current position in input data
  BOOL       fOffsList;                // TRUE = return a offset/length list
  LONG       lFlags;                   // flags for current term/token
  BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
  BOOL       fSkip, fNewSentence,      // token processing flags
             fSingleToken, fEndOfToken;
  CHAR_W     c;
  CHAR_W     d;


  pvLangCB;                            // avoid compiler warnings

  fOffsList  = (usListType == MORPH_OFFSLIST) ||
               (usListType == MORPH_FLAG_OFFSLIST);
  pszCurPos = pszInData;

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

    switch ( c )
    {
      //case '.' :
      //case '!' :
      //case '?' :
      case L'.' :
      case L'!' :
      case L'?' :
        switch ( pszCurPos[1] )
        {
          //case ' ':
          //case '\n':
          //case '\0':
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

      //case '\n' :
      //case ' ' :
      case L'\n' :
      case L' ' :
        fEndOfToken  = TRUE;
        fSkip        = TRUE;
        break;

      //case '$' :
      //case '#' :
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

      //case '/' :
      //case '\\' :
      //case ',' :
      //case ';' :
      case L'/' :
      case L'\\' :
      case L',' :
      case L';' :
        switch ( pszCurPos[1] )
        {
          //case ' ':
          //case '\n':
          //case '\0':
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
        if (// ((d = pszCurPos[1]) ==  ' ') || (d ==  '\n') || ( d ==  '\0' ) 
          //||
          (((d = pszCurPos[1]) == L' ') || (d == L'\n') || ( d == L'\0' )) )
        {
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        } /* endif */
        break;

      //case '(' :
      //case ')' :
      //case '\"' :
      //case '\'':
      case L'(' :
      case L')' :
      case L'\"':
      case 8243 ://″
      case L'\'':
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        break;

      default:
        if ( fAlNum ) //&& !chIsText[c] )
        {
			PUCHAR pTextTable;
			
      UtlQueryCharTable( IS_TEXT_TABLE, &pTextTable );
			
      if(c > 255){
        wchar_t buff[2];
        buff[0] = c;
        buff[1] = 0;
        LogMessage4(ERROR, __func__,"::not supported symbol \'", EncodingHelper::convertToUTF8(buff),"\'");
      }else if ( !pTextTable[c] )
			{
				fAlNum = FALSE;
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
    if ( c != EOS ) pszCurPos++;

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


