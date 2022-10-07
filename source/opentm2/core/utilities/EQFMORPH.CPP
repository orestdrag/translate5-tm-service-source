/*! \file
	Description: API for morphological functions

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define MORPH_BUILD_COMP_STEMS         // use compound sep. for stem forms
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_MORPH            // morphologic functions
#include <EQF.H>                  // General Translation Manager include file

#include <EQFMORPI.H>                  // private header file of module
#include <EQFCHTBL.H>                  // character tables
#include "OTMFUNC.H"            // function call interface public defines
#include "EQFFUNCI.H"           // function call interface private defines

#include <ctype.h>
#ifndef INCL_EQF_MORPH
#define INCL_EQF_MORPH            // morphologic functions
#endif

#include "../morph/SpellFactory.h"
#include "../morph/MorphFactory.h"
//#include "../utilities/LogWriter.h"
#include "../utilities/LogWrapper.h"
#include "../utilities/EncodingHelper.h"
#include "../utilities/ThreadingWrapper.h"

#ifdef TEMPORARY_COMMENTED
#include "cxmlwriter.h"
#endif //TEMPORARY_COMMENTED

#define MAX_TERMSIZE        256
#define REPLY_AREA_SIZE   40000        // size of reply area
#define LEMMA_LIST_SIZE    8096        // size of lemma list buffer
#define MAX_DICT_NAME       256

// activate the next define to measure the time in MorphTokenizeW calls
//#define MEASURE_TOKENIZE_TIME


/**********************************************************************/
/* Control block for this language exit                               */
/**********************************************************************/
typedef struct _LANGCB
{
	OtmSpell* m_SpellInstance;
	OtmMorph* m_MorphInstance;
	CHAR     szLanguage[MAX_LONGFILESPEC]; // name of language
	CHAR     szSpellPlugin[MAX_LONGFILESPEC]; // name of spell plugin
	CHAR     szMorphPlugin[MAX_LONGFILESPEC]; // name of morph plugin
	CHAR     szBuffer[CCHMAXPATH];      // general use buffer
	CHAR     szLemmaList[LEMMA_LIST_SIZE];  // buffer for lemma lists
  BOOL     fNoSpellSupport;           // TRUE = no spell support for this language available 
  BOOL     fNoStemInMorph;            // TRUE = stem form reduction is not supported in morph plugin
  BOOL     fNoStemInSpell;            // TRUE = stem form reduction is not supported in spell plugin
} LANGCB, *PLANGCB;

// activate this define to do logging
  #define ACTIVATE_LOGGING
  #define LOG_TOKENIZE
  #define LOG_TOKENIZEW
  //#define LOG_JVMDLL_PRELOAD

#ifdef ACTIVATE_LOGGING
  CHAR szMLogFile[MAX_EQF_PATH] = "";
#endif

USHORT TokenizeW
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszInData,                  // IN : ptr to data being tokenized
 PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list MORPH_ZTERMLIST,
                                     //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                     //    or MORPH_FLAG_ZTERMLIST
 );
void MorphCheckSpellInstance( PLANGCB pLangCB );
USHORT MorphWStrings2TermList( vector<std::wstring> &vStrings, PVOID *ppTermList, PULONG pulTermListSize );


/**********************************************************************/
/**********************************************************************/
/* Global variables                                                   */
/**********************************************************************/
/**********************************************************************/
//static char chDummy1[1024] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
//static PLCB IDtoLCB[256] = {      // ID to language control block table
//PLCB IDtoLCB[256] = {      // ID to language control block table
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
//};
//static char chDummy2[1024] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
PLCB* ppIdToLCB = 0;
BOOL bLCBTableAllocated = FALSE;

// array with names of languages with specific types

typedef struct _MORPHLANGTYPE
{
  CHAR        szLanguage[MAX_LANG_LENGTH];// language name
  SHORT       sType;                      // language type
} MORPHLANGTYPE, *PMORPHLANGTYPE;

static MORPHLANGTYPE LangTypes[] =
{
  { "Chinese(simpl.)", MORPH_DBCS_LANGTYPE },
  { "Chinese(trad.)",  MORPH_DBCS_LANGTYPE },
  { "Japanese",        MORPH_DBCS_LANGTYPE },
  { "Korean",          MORPH_DBCS_LANGTYPE },
  { "Hebrew",          MORPH_BIDI_H_LANGTYPE },
  { "Arabic",          MORPH_BIDI_A_LANGTYPE },
  { "Thai",            MORPH_THAI_LANGTYPE },
  { "",                0 }                   // end of array indicator
};


#if defined(OS2_20)
  #define max( a, b ) ( a > b ) ? (a) : (b)
  #define min( a, b ) ( a < b ) ? (a) : (b)
#endif

#if defined(MEASURETIME)
  #define INITTIME( pLCB )  pLCB->ulLastTime = pLCB->pInfoSeg->msecs
  #define GETTIME( pLCB, ulTime )                       \
  {                                                     \
     ulTime += pLCB->pInfoSeg->msecs - pLCB->ulLastTime;\
     pLCB->ulLastTime = pLCB->pInfoSeg->msecs;          \
  }
#endif

/**********************************************************************/
/* indicator for DBCS initialisation                                  */
/**********************************************************************/
BOOL  fInit = FALSE;

//------------------------------------------------------------------------------
// Function name:     MorphInit
//------------------------------------------------------------------------------
// Description:       Initialization of the language exit.
//------------------------------------------------------------------------------
// Function call:     MorphInit( PVOID *ppvLangCB, PSZ pszMorphDict, PSZ pszSpellPlugin, PSZ pszMorphPlugin,
//                          USHORT usCodePage, USHORT usLAngCode );
//------------------------------------------------------------------------------
// Input parameter:   PSZ        szMorphDict  name of morphologic dictionary
//                    PSZ        pszSpellPlugin name of the spell check plugin
//                    PSZ        pszMorphPlugin name of the morph plugin
//                    USHORT     usCodePage   code page to be used
//                    USHORT     usLangCode   language code (= lang identifier)
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      control block is allocated,
//                    morphologic service is begun,
//                    morphologic dictionary is activated
//------------------------------------------------------------------------------
// Function flow:     allocate language control block
//                    fill language control block
//                    begin Nlp service
//                    set Nlp code page
//                    activate morphologic dictionary
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphInit
(
 PVOID      *ppvLangCB,               // OUT: ptr to buffer for language CB ptr
 PSZ        pszLanguage,              // IN : name of language
 PSZ        pszSpellPlugin,           // IN : name of the spell plugin
 PSZ        pszMorphPlugin,           // IN : name of the morph plug-in
 USHORT     usCodePage,               // IN : code page to be used
 USHORT     usLangCode                // IN : language code (= lang identifier)
 )
{
	PLANGCB    pLangCB;                  // ptr to allocated language CB
	USHORT     usRC = MORPH_OK;          // function return code

  usLangCode; usCodePage; 
	/********************************************************************/
	/* Allocate language control block                                  */
	/********************************************************************/
	if ( !UtlAlloc( (PVOID *)&pLangCB, 0L, (LONG)sizeof(LANGCB), NOMSG ) )
	{
		usRC = MORPH_NO_MEMORY;
	} /* endif */

	if ( usRC == MORPH_OK )
	{
		*ppvLangCB = (PVOID)pLangCB;

    strcpy( pLangCB->szLanguage, pszLanguage );
    strcpy( pLangCB->szSpellPlugin, pszSpellPlugin );
    strcpy( pLangCB->szMorphPlugin, pszMorphPlugin );

    // delay activation of the speller instance until spell functionality is requried
    // .. nothing to do here for the spell instance...

    // activate the morphologic functions
		MorphFactory* tMorphFactoryInstance = MorphFactory::getInstance();
		if (NULL == tMorphFactoryInstance)
		{
			usRC = MORPH_NO_MEMORY;
		}
		pLangCB->m_MorphInstance = tMorphFactoryInstance->getMorph( pszLanguage, pszMorphPlugin );
		if (NULL == pLangCB->m_MorphInstance)
		{
      LogMessage2(WARNING,__func__, ":: pLangCB->m_MorphInstance == NULL");
			usRC = MORPH_NO_MEMORY;
		}

		if (MORPH_OK != usRC)
		{
			UtlAlloc( (PVOID *)&pLangCB, 0L, 0L, NOMSG );
		}
	} /* endif */

	return( usRC );
} /* end of function INIT */

USHORT TokenizeW
(
 PVOID    pvLangCB,                   // IN : ptr to language control block
 PSZ_W    pszInData,                  // IN : ptr to data being tokenized
 PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                      //    containing size of term list buffer
 PVOID    *ppTermList,                // IN/OUT: address of term list pointer
 USHORT   usListType                  // IN: type of term list MORPH_ZTERMLIST,
                                     //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                     //    or MORPH_FLAG_ZTERMLIST
 )
{
	USHORT     usReturn = 0;             // return code
	ULONG      ulTermBufUsed = 0;        // amount of space used in term buffer
	BOOL       fOffsList;                // TRUE = return a offset/length list

	fOffsList  = (usListType == MORPH_OFFSLIST) ||
		(usListType == MORPH_FLAG_OFFSLIST);

	/********************************************************************/
	/* Work on input data                                               */
	/********************************************************************/

  PLANGCB tPLangCB = (PLANGCB)pvLangCB;
  if (NULL == tPLangCB)
  {
    return MORPH_INV_PARMS;
  }
  OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
  if (NULL == tMorphInstance)
  {
    return MORPH_NOT_FOUND;
  }

#ifdef LOG_TOKENIZEW
        if(CheckLogLevel(DEVELOP)){
          auto str = EncodingHelper::convertToUTF8(pszInData);
          LogMessage4(DEVELOP, __func__,  "::Input string is >>>",str.c_str() , "<<<");
        }
#endif

  // split text block into segments (sentences)
  OtmMorph::TERMLIST vSentenceList;
  int tRet = tMorphInstance->tokenizeBySentence( pszInData, vSentenceList );
  if (0 != tRet)
  {
    return MORPH_FUNC_FAILED;
  }

#ifdef LOG_TOKENIZEW
  if(CheckLogLevel(DEVELOP)){
    LogMessage2(DEVELOP, __func__, "---------------results from tokenizeBySentence----------------" );
    for (size_t j = 0; j < vSentenceList.size(); j++)
    {
      PSZ_W pszStartOfSegment = pszInData + vSentenceList[j].iStartOffset;
      wchar_t chTemp = pszStartOfSegment[vSentenceList[j].iLength];
      pszStartOfSegment[vSentenceList[j].iLength] = 0;
      auto str = EncodingHelper::convertToUTF8(pszStartOfSegment);
      LogMessage6(DEVELOP, __func__, "::Segment ",toStr(j).c_str()," >>>",str.c_str(),"<<<" );
      pszStartOfSegment[vSentenceList[j].iLength] = chTemp;
    }
    LogMessage2(DEVELOP, __func__, "--------------------------------------------------------------" );
  }
#endif


  for (size_t j = 0; j < vSentenceList.size(); j++)
  {
    if (!usReturn)
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
        (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, L" ", 1, 0, TF_NEWSENTENCE, usListType );
      } /* endif */
    } /* endif */

    // split segment/sentences into words
    OtmMorph::TERMLIST vTermList;
    PSZ_W pszStartOfSegment = pszInData + vSentenceList[j].iStartOffset;
    wchar_t chTemp = pszStartOfSegment[vSentenceList[j].iLength];
    pszStartOfSegment[vSentenceList[j].iLength] = 0;
    int tRet = tMorphInstance->tokenizeByTerm( pszStartOfSegment, vTermList );
#ifdef LOG_TOKENIZEW
    if(CheckLogLevel(DEVELOP)){
      auto str = EncodingHelper::convertToUTF8(pszStartOfSegment);
      LogMessage4(DEVELOP, __func__, "::---------tokenizeByTerm of string >>>",str.c_str() ,"<<<---------" );
      for( size_t i = 0; i < vTermList.size(); i++ )
      {
        std::string strFlags = "";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NOLOOKUP )    strFlags += "TERMTYPE_NOLOOKUP ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NUMBER )      strFlags += "TERMTYPE_NUMBER ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_ABBR )        strFlags += "TERMTYPE_ABBR ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_ALLCAPS )     strFlags += "TERMTYPE_ALLCAPS ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_NEWSENTENCE ) strFlags += "TERMTYPE_NEWSENTENCE ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_PREFIX )      strFlags += "TERMTYPE_PREFIX ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_COMPLEX )     strFlags += "TERMTYPE_COMPLEX ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_INITCAP )     strFlags += "TERMTYPE_INITCAP ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_WHITESPACE )  strFlags += "TERMTYPE_WHITESPACE ";
        if ( vTermList[i].iTermType & OtmMorph::TERMTYPE_PUNCTUATION ) strFlags += "TERMTYPE_PUNCTUATION ";
        wchar_t chTemp2 = pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength];
        pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength] = 0;
        str = EncodingHelper::convertToUTF8(pszStartOfSegment + vTermList[i].iStartOffset);
        std::string msg = std::string(__func__) + "::Term " +toStr(i) + " Start=" + toStr(vTermList[i].iStartOffset) + ", Length=" + toStr(vTermList[i].iLength)
          + " Term=\"" + str + "\", Flags=" + strFlags;
        LogMessage(DEVELOP, msg.c_str());//LOG_DEVELOP_MSG << msg;
      
        pszStartOfSegment[vTermList[i].iStartOffset+vTermList[i].iLength] = chTemp2;
      }
      LogMessage2(DEVELOP, __func__, "::--------------------------------------------------------------" );
    }
#endif
    pszStartOfSegment[vSentenceList[j].iLength] = chTemp;


    if (0 != tRet)
    {
      return MORPH_FUNC_FAILED;
    }
    else
    { 
      // convert returned term list into morph term list
      for( size_t i = 0; i < vTermList.size(); i++ )
      {
        auto iStartOffs = vTermList[i].iStartOffset;
        auto iLen = vTermList[i].iLength;
        
        if ( (vTermList[i].iTermType & OtmMorph::TERMTYPE_WHITESPACE) != 0 )
        {
          // ignore whitespace tokens
        }
        else if ( iLen == 0 ) 
        {
          // ignore empty tokens
        }
        else
        {
          USHORT usPos = (USHORT)(vSentenceList[j].iStartOffset + iStartOffs);
          usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, 
                                            pszStartOfSegment + iStartOffs, 
                                            (USHORT)iLen, usPos, vTermList[i].iTermType, usListType );
        }
      }
    }
  }

	/*****************************************************************/
	/* terminate the term list                                       */
	/*****************************************************************/
	if ( !usReturn )
	{
		usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList, pulTermListSize, &ulTermBufUsed, L" ", 0, 0, 0L, usListType );
	} /* endif */

	return (usReturn);

} /* end of function TOKENIZEW */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     Terminate
//------------------------------------------------------------------------------
// Description:       Terminate the language exit. All dictionaries are closed
//                    and any allocated storage is freed.
//------------------------------------------------------------------------------
// Function call:     Terminate( PVOID pvLangCB );
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      all dictionaries are close and all memory is freed
//------------------------------------------------------------------------------
// Function flow:     deactivate main morphologic dictionary
//                    if addenda dictionary is active
//                      deactivate addenda dictionary
//                    endif
//                    end Nlp service
//                    free language control block area
//                    return function return code
//------------------------------------------------------------------------------
USHORT Terminate
(
 PVOID pvLangCB                       // IN : ptr to buffer for language CB ptr
 )
{
	PLANGCB    pLangCB;                  // ptr to allocated language CB
	USHORT     usRC = MORPH_OK;          // function return code

	pLangCB = (PLANGCB)pvLangCB;

	/********************************************************************/
	/* free memory for NLP services control block  and language CB      */
	/********************************************************************/
	UtlAlloc( (PVOID *) &pLangCB, 0L, 0L, NOMSG );

  LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED in Terminate::  SpellFactory::close();MorphFactory::close();");
#ifdef TEMPORARY_COMMENTED 
	SpellFactory::close();
	MorphFactory::close();
  #endif

	return( usRC );

} /* end of function Terminate */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     BuildDict
//------------------------------------------------------------------------------
// Description:       Returns all the word of the dictionary according to the dictionary type
//------------------------------------------------------------------------------
// Function call:     BuildDict
//                           (          
//                              PVOID pvLangCB, 
//                              USHORT usDictType, 
//                              PUSHORT pusTermListSize, 
//                              PVOID *ppTermList, 
//                              USHORT usListType
//                            ) 
//------------------------------------------------------------------------------
// Input parameter:   PVOID    pvLangCB        ptr to language control block
//                    PSZ      usDictType      the dictionary type
//                    PUSHORT  pusTermListSize address of term list size var
//                    PVOID    *ppTermList     address of term list pointer
//                    USHORT   usListTyp       only MORPH_ZTERMLIST is valid
//------------------------------------------------------------------------------
// Output parameter:  PVOID      *ppvLangCB   to buffer for language CB ptr
//------------------------------------------------------------------------------
// Return code type:   USHORT
//------------------------------------------------------------------------------
// Return codes:       see include file EQFMORPH.H for list of return codes
//------------------------------------------------------------------------------
// Prerequisites:      pvLangCB must have been created using the INIT function
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     call writeTermFromFile to restore all the words of the specified dictionary
//                    return function return code
//------------------------------------------------------------------------------

USHORT BuildDict
( 
 PVOID pvLangCB, 
 USHORT usDictType, 
 USHORT usTermListSize, 
 PVOID pTermList, 
 USHORT usListType
 ) 
{
	USHORT usRC = MORPH_OK;

  usListType; usTermListSize; 

	if (ADDENDA_DICT != usDictType && ABBREV_DICT != usDictType)
	{
		return MORPH_NOT_FOUND;
	}

	if (NULL == pTermList)
	{
		return MORPH_OK;
	}

	PLANGCB tPLangCB = (PLANGCB)pvLangCB;
	if (NULL == tPLangCB)
	{
		return MORPH_INV_PARMS;
	}

  if ( ABBREV_DICT == usDictType )
  {
    OtmMorph* tMorphInstance = tPLangCB->m_MorphInstance;
	  if (NULL == tMorphInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

    tMorphInstance->clearAbbreviations();

    // be caution here sizeof(PSZ_W)/sizeof(wchar_t)
	  wchar_t* tPTerm = (wchar_t*) pTermList + sizeof(PSZ_W)/sizeof(wchar_t);
	  int tLen = wcslen(tPTerm);

      if(pTermList==NULL)
          tLen = 0;

	  while (tLen > 0)
	  {
		  tMorphInstance->addAbbreviation(tPTerm);
		  tPTerm += tLen;
		  tPTerm++;
		  tLen = wcslen(tPTerm);
	  }

    tMorphInstance->saveAbbreviations();
  }
  else
  {
	  OtmSpell* tSpellInstance = tPLangCB->m_SpellInstance;
	  if (NULL == tSpellInstance)
	  {
		  return MORPH_NOT_FOUND;
	  }

	  vector<wstring> tOldTermList;
	  tSpellInstance->listTerms(tOldTermList, usDictType);
	  for (size_t i = 0; i < tOldTermList.size(); i++)
	  {
		  tSpellInstance->deleteTerm(tOldTermList[i].c_str(), usDictType);
	  }
      // be caution here sizeof(PSZ_W)/sizeof(wchar_t)
	  wchar_t* tPTerm = (wchar_t*) pTermList + sizeof(PSZ_W)/sizeof(wchar_t);
	  int tLen = wcslen(tPTerm);

      if(pTermList==NULL)
          tLen = 0;

	  while (tLen > 0)
	  {
		  tSpellInstance->addTerm(tPTerm, usDictType);
		  tPTerm += tLen;
		  tPTerm++;
		  tLen = wcslen(tPTerm);
	  }


	  if( true != tSpellInstance->writeTermToFile(usDictType))
		  usRC = MORPH_FUNC_FAILED;
  }

	return usRC;
} /* end of function BUILDDICT */

//------------------------------------------------------------------------------
//   Public functions (= module entry points)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphTokenize       Split a segment into terms
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all terms found in a
//                    term list.
//                    :p.
//                    Depending on the type of list requested
//                    a term list is either a series of null-terminated strings
//                    which is terminated by another null value or a list
//                    of offset and length values terminated by a null entry.
//                    See the sample for the layout of a term list.
//                    :p.
//                    If the term list pointer is NULL and the term list size
//                    is zero, the term list is allocated.
//                    The term list is enlarged if not all terms fit into
//                    the current list.
//------------------------------------------------------------------------------
// Function call:     usRC = MorphTokenize( SHORT sLanguageID, PSZ pszInData,
//                                          PUSHORT pusBufferSize,
//                                          PVOID *ppTermList,
//                                          USHORT usListType);
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID
//                    PSZ      pszInData      pointer to input segment
//                    USHORT   usListType     type of term list
//                                              MORPH_ZTERMLIST or
//                                              MORPH_OFFSLIST
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing
//                                               size of term list buffer
//                    PVOID    *ppTermList    address of caller's term list
//                                               pointer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the
//                    MorphGetLanguageID function.
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size
//                    PSZ    pszList = NULL;     // ptr to list area
//                    SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//                    usRC = MorphTokenize( sLangID, "Mary had a little lamb",
//                                          &usListSize, &pszList,
//                                          MORPH_ZTERMLIST );
//
//                    after this call the area pointed to by pszList would
//                    contain the following term list:
//
//                    "Mary\0had\0a\0little\0lamb\0\0"
//------------------------------------------------------------------------------
// Function flow:     check input data
//                    get language control block pointer
//                    call tokenize function of language exit
//                    return function return code
//------------------------------------------------------------------------------
USHORT MorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                 // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   ULONG    ulOemCP                    // CP of language of sLangID!!
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  PLCB      pLCB = NULL;                      // pointer to language control block
  ULONG     ulTermBufUsed = 0;         // number of bytes used in term buffer
  ULONG     ulBufferSize = *pusBufferSize;
#ifdef MEASURE_TOKENIZE_TIME
   static int iMeasureCount = 0;
   static _int64 iTotalTime = 0;
   LARGE_INTEGER iStartTime;
   LARGE_INTEGER iEndTime;
#endif


#ifdef MEASURE_TOKENIZE_TIME
  QueryPerformanceCounter( &iStartTime );
#endif

  ulOemCP; 
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

  /********************************************************************/
  /* Get language control block pointer                               */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphGetLCB( sLanguageID, &pLCB );
  } /* endif */

  /********************************************************************/
  /* Check if refresh is required                                     */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    usRC = MorphCheck4Refresh( pLCB );
  } /* endif */

  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
#ifdef LOG_TOKENIZE
        if(CheckLogLevel(DEVELOP)){
          auto str = EncodingHelper::convertToUTF8(pszInData);
          LogMessage4(DEVELOP, __func__, "::Tokenizing \"",str.c_str(),"\""  );
        }
#endif

      usRC = TokenizeW( pLCB->pvLangCB, pszInData, &ulBufferSize, ppTermList, usListType );

#ifdef LOG_TOKENIZE
      if ( usRC == MORPH_OK && CheckLogLevel(DEVELOP) )
      {
        if ( usListType == MORPH_FLAG_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen;
          LONG lFlag;
          do
          {
            lFlag  = *(PLONG)psData;
            psData += 2;
            sLen   = *psData++;
            sStart = *psData++;
            if(CheckLogLevel(DEVELOP)){
              LogMessage7(DEVELOP, __func__, "::  Start=",toStr(sStart).c_str()," Len=",toStr(sLen).c_str(), " Flag=", std::to_string(lFlag).c_str() );
            }
          } while ( (sStart != 0) || (sLen != 0) || (lFlag != 0) );
        }
        else if ( usListType == MORPH_OFFSLIST )
        {
          PSHORT psData = (PSHORT)*ppTermList;
          SHORT sStart, sLen;
            do
            {
              sLen   = *psData++;
              sStart = *psData++;
              if(CheckLogLevel(DEVELOP)){
                LogMessage5(DEVELOP, __func__, "::Start=",toStr(sStart).c_str(), ", Len=", toStr(sLen).c_str());
              }
            } while ( (sStart != 0) || (sLen != 0) );
        } /* endif */
      }
      else
      {
        LogMessage3(DEVELOP, __func__, ":: TokenizeW failed with rc=", toStr(usRC).c_str() );
      } /* endif */
      
#endif

    }
    else
    {
      usRC = MorphAddTermToList2W( (PSZ_W *)ppTermList, &ulBufferSize, &ulTermBufUsed, L" ", 0, 0, 0L, usListType );
    } /* endif */
  } /* endif */

  *pusBufferSize = (USHORT)ulBufferSize;

#ifdef MEASURE_TOKENIZE_TIME
  QueryPerformanceCounter( &iEndTime );

  iTotalTime += (iEndTime.QuadPart - iStartTime.QuadPart);
  iMeasureCount++;
  if ( iMeasureCount == 100000 )
  {
    char szMsg[60];
    sprintf( szMsg, "Time for %ld MorphTokenizeWCalls is %I64d ticks", iMeasureCount, iTotalTime );
    MessageBox( HWND_DESKTOP, szMsg, "Info", MB_OK );
  }
#endif

  return( usRC );

} /* endof MorphTokenizeW */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     MorphGetLanguageID           Get ID for a language
//------------------------------------------------------------------------------
// Description:       Get the unique ID for the given the language. The ID is
//                    required by subsequent Morph... calls.
//------------------------------------------------------------------------------
// Function call:     MorphGetLanguageID( PSZ pszLanguage, PSHORT psLanguageID)
//------------------------------------------------------------------------------
// Input parameter:   PSZ     pszLanguage         name of language (name used
//                                                in the PROPERTY.LNG file to
//                                                identiy the language)
//------------------------------------------------------------------------------
// Output parameter:  PSHORT  psLanguageID        address of variable receiving
//                                                the language ID
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      The dictioaries for the language are activated and the
//                    stem form caches are read from disk if this is the first
//                    call for the given language.
//------------------------------------------------------------------------------
// Samples:           SHORT  sLangID;            // language ID
//
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );
//
//------------------------------------------------------------------------------
// Function flow:     search language in language table
//                    if not found
//                      allocate language control block
//                      get values for language form PROPERTY.LNG file
//                      fill language control block
//                      create hash tables
//                      read stem form cache from disk
//                      read no-lookup hash table from disk
//                      read POS cache from disk
//                      read POS no-lookup hash table from disk
//                      load language exit DLL
//                      get procedure addresses from language exit
//                      call INIT entry point of language exit
//                      add language to list of active languages
//                      set caller's language ID
//                      cleanup in case of errors
//                      return function return code
//------------------------------------------------------------------------------
USHORT MorphGetLanguageID
(
  PSZ     pszLanguage,                 // name of language
  PSHORT  psLanguageID                 // address of variable receiving the
                                       //    language ID
)
{
  USHORT   usRC = MORPH_OK;            // function returncode
  USHORT   usLangCode;                 // language code from language properties
  SHORT    sID = -1;                   // evaluated language ID
  SHORT    sI;                         // general loop index
  PLCB     pLCB = NULL;                // pointer to new language control block
  PLCB     pLCBTemp;
  PLCB*    ppLCBTemp;
  USHORT   usTid;

    /******************************************************************/
    /* get Task-ID                                                    */
    /* use stack segment as differentiator to define what application */
    /* is calling this time..                                         */
    /******************************************************************/
    
  usTid = _getpid();
   

  /********************************************************************/
  /* Allocate LCB table if necessary                                  */
  /********************************************************************/
  if (bLCBTableAllocated == FALSE)
  {
    if ( !UtlAlloc( (PVOID *)&ppIdToLCB, 0L, (LONG) (sizeof(PLCB) * 256), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
	}
	else
	{
	  bLCBTableAllocated = TRUE;
	  memset(ppIdToLCB, 0, sizeof(PLCB) * 256);
    } /* endif */
  }
  
  /********************************************************************/
  /* Search language in currently active languages                    */
  /********************************************************************/
  ppLCBTemp = ppIdToLCB;
  for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ )
  {
    pLCBTemp = *ppLCBTemp;
    if ( ( pLCBTemp->tidThread == usTid ) &&
           (strcasecmp( pszLanguage, pLCBTemp->szLanguage ) == 0) )
    {
      sID = sI;                        // remember ID of language
      break;                           // leave for loop
    } /* endif */
  } /* endfor */


  /********************************************************************/
  /* Allocate language control block                                  */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    if ( !UtlAlloc( (PVOID *)&pLCB, 0L, (LONG)sizeof(LCB), NOMSG ) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Get language properties                                          */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0) )
  {
    strcpy( pLCB->szBuffer, pszLanguage );
    strcpy(pLCB->szLanguage, pszLanguage);
    strcpy(pLCB->szMorphDict, pszLanguage);
    strcpy(pLCB->szAddendaDict, pszLanguage);
    pLCB->usCodePage = 850;
    pLCB->fCompSep = FALSE;

    SpellFactory* tSpellFactoryInstance = SpellFactory::getInstance();
    if (NULL == tSpellFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    MorphFactory* tMorphFactoryInstance =  MorphFactory::getInstance();
    if (NULL == tMorphFactoryInstance)
    {
      usRC = MORPH_NO_MEMORY;
    }
    if (MORPH_OK == usRC && tMorphFactoryInstance->isSupported(pszLanguage))
    {
    }
    else
    {
      usRC = MORPH_OK; // GQ: disabled temporarely the result of isSupported, correct code is: usRC = MORPH_NO_LANG_PROPS;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Fill language control block                                      */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    pLCB->tidThread = usTid;
  } /* endif */
  
  /********************************************************************/
  /* Activate language exit                                           */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    usRC = MorphInit( &pLCB->pvLangCB,
                             pLCB->szMorphDict,
                             pLCB->szSpellPlugin,
                             pLCB->szMorphPlugin,
                             pLCB->usCodePage,
                             usLangCode );
    if ( usRC != MORPH_OK )
    {
       pLCB->pvLangCB = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Activate stem form caches                                        */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    /******************************************************************/
    /* Create tree for caching                                        */
    /******************************************************************/
    pLCB->HashCache       = MorphHashCreate( 2000 );
    pLCB->HashNoLookup    = MorphHashCreate( 2000 );

    if ( (pLCB->HashCache == NULL) || (pLCB->HashNoLookup == NULL) )
    {
      usRC = MORPH_NO_MEMORY;
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Add language to active languages                                 */
  /********************************************************************/
  if ( (usRC == MORPH_OK) && (sID < 0))
  {
    ppLCBTemp = ppIdToLCB;
    for ( sI = 0; sI < 256, *ppLCBTemp != 0; sI++, ppLCBTemp++ );

    if ( sI < 256 )
    {
      *ppLCBTemp = pLCB;
      sID = sI;
    }
    else
    {
      usRC = MORPH_TOO_MUCH_LANGUAGES;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set caller's language ID                                         */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    *psLanguageID = sID;
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( usRC != MORPH_OK )
  {
    if ( pLCB )
    {
      if ( pLCB->pvLangCB )
      {
        Terminate( pLCB->pvLangCB );
      } /* endif */

      if ( pLCB->pCompList )
      {
        UtlAlloc( (PVOID *)&(pLCB->pCompList), 0L, 0L, NOMSG );
      } /* endif */

      if ( pLCB->HashCache )        MorphHashDestroy( pLCB->HashCache );
      if ( pLCB->HashNoLookup )     MorphHashDestroy( pLCB->HashNoLookup );

      UtlAlloc( (PVOID *)&pLCB, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return function return code to caller                            */
  /********************************************************************/
  return( usRC );

} /* end of function MorphGetLanguageID */

//------------------------------------------------------------------------------
//   General private functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphGetLCB         Get LCB pointer for a language ID
//------------------------------------------------------------------------------
// Description:       Check a given language ID and return the pointer to the
//                    language control block identified by the language ID
//------------------------------------------------------------------------------
// Function call:     MorphGetLCB( SHORT sLanguageID, PLCB *ppLCB )
//------------------------------------------------------------------------------
// Input parameter:   SHORT     sLanguageID   language identifier
//------------------------------------------------------------------------------
// Output parameter:  PLCB      *ppLCB        address of LCB pointr
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully
//                    other       MORPH_ error code, see EQFMORPH.H for a list
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           usRC = MorphGetLCB( sLangID, &pLCB );
//------------------------------------------------------------------------------
// Function flow:     if language ID is in [0..255] and language pointer is set
//                      set callers pointer to language control block
//                    else
//                      set return code to MORPH_INV_LANG_ID
//                    endif
//                    return return code to calling function
//------------------------------------------------------------------------------
USHORT MorphGetLCB
(
  SHORT     sLanguageID,               // language identifier
  PLCB      *ppLCB                     // address of LCB pointr
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  PLCB*    ppLCBTemp = ppIdToLCB + sLanguageID;

  if ( (sLanguageID >= 0)      &&
       (sLanguageID <= 255)    &&
       (*ppLCBTemp != 0) )
  {
    usRC = MORPH_OK;
    *ppLCB = *ppLCBTemp;
  }
  else
  {
    usRC = MORPH_INV_LANG_ID;
  } /* endif */

  return( usRC );

} /* end of function MorphGetLCB */


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
      //ulDataLenBytes = 4 * sizeof(CHAR_W);
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
      ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(CHAR_W));
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
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1;
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
        *pulUsed += 1;//sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1;//sizeof(USHORT) / sizeof(CHAR_W);
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

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashCreate     Create the hash
//------------------------------------------------------------------------------
// Description:       Creates a hash table for the morphologic functions.
//                    The supplied parameter is the size of the hash table
//                    in number of entries.
//------------------------------------------------------------------------------
// Function call:     MorphHashCreate( USHORT usMaxElements );
//------------------------------------------------------------------------------
// Input parameter:   USHORT   usMaxElements       size of hash table
//------------------------------------------------------------------------------
// Returncode type:   PMORPHHASH
//------------------------------------------------------------------------------
// Returncodes:       NULP             in case of errors
//                                     (error has been reported already)
//                    other            pointer to morph cache
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHASH   pHash;
//
//                    pHash = MorphHashCreate( 500 );
//------------------------------------------------------------------------------
// Function flow:     allocate hash control area
//                    initialize hash control area
//                    allocate table for hash elements
//                    add all elements to free chain
//                    initialize recently-used chain
//                    create low level hash
//                    initialize string pool
//                    cleanup in case of errors
//                    return morph cache pointer or NULL
//------------------------------------------------------------------------------
PMORPHHASH MorphHashCreate
(
  ULONG        ulMaxElements               // size of hash table
)
{
  BOOL         fOK = TRUE;             // internal O.K. flag
  PMORPHHASH   pMorphHash = NULL;      // ptr to hash control area
  PHASHENTRY   pElement;               // pointer to hash elements
  ULONG        ulI;                    // general loop index

  /********************************************************************/
  /* Allocate hash control area                                       */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pMorphHash, 0L, (LONG)sizeof(MORPHHASH), ERROR_STORAGE );
  } /* endif */

  /********************************************************************/
  /* Initialize hash control area                                     */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulMaxElements = min( ulMaxElements,
                                     (MAX_SEG_SIZE / sizeof(HASHENTRY)) );
    pMorphHash->ulElements  = 0;
  } /* endif */

  /********************************************************************/
  /* Allocate table for hash elements                                 */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, pMorphHash->ulMaxElements * sizeof(HASHENTRY), ERROR_STORAGE );
  } /* endif */

  /********************************************************************/
  /* Add all elements to free chain                                   */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulFirstFree = END_OF_ELEMENTS;
    pElement = pMorphHash->pElements;
    for ( ulI = 0; ulI < pMorphHash->ulMaxElements; ulI++ )
    {
      pElement->ulKey  = ulI;
      pElement->ulNext = pMorphHash->ulFirstFree;
      pMorphHash->ulFirstFree = ulI;
      pElement++;
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* Initialize recently-used chain                                   */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->ulFirstUsed = END_OF_ELEMENTS;
    pMorphHash->ulLastUsed  = END_OF_ELEMENTS;
  } /* endif */

  /********************************************************************/
  /* Create low level hash                                            */
  /********************************************************************/
  if ( fOK  )
  {
    pMorphHash->pHash = (PHASH)HashCreate( sizeof(HASHENTRY),
                                    pMorphHash->ulMaxElements * 2,
                                    (PFN_HASHVALUE)MorphHashKeyValue,
                                    (PFN_HASHCOMP)MorphHashCompare,
                                    (PFN_HASHFREE)MorphHashFree,
                                    (PVOID)pMorphHash );
    fOK = pMorphHash->pHash != NULL;
  } /* endif */

  /********************************************************************/
  /* Initialize string pool                                           */
  /********************************************************************/
  if ( fOK )
  {
    pMorphHash->StringPool = PoolCreate( INITIAL_POOL_SIZE );
    fOK = pMorphHash->StringPool != NULL;
  } /* endif */

  /********************************************************************/
  /* Cleanup in case of errors                                        */
  /********************************************************************/
  if ( !fOK )
  {
    if ( pMorphHash )
    {
      if ( pMorphHash->pElements )
      {
        UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, 0L, NOMSG );
      } /* endif */
      if ( pMorphHash->pHash )
      {
        HashDestroy( pMorphHash->pHash );
      } /* endif */
      if ( pMorphHash->StringPool )
      {
        PoolDestroy( pMorphHash->StringPool );
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return pointer to morph cash or NULL in case of erros            */
  /********************************************************************/
  return( ( fOK ) ? pMorphHash : NULL );
} /* end of function MorphHashCreate */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashDestroy
//------------------------------------------------------------------------------
// Description:       Destroy a hash created using MorphHashCreate
//------------------------------------------------------------------------------
// Function call:     MorphHashDestroy( PMORPHHASH pHash );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH   pHash   pointer to hash control block
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      the hash table is destroyed and all memory freed
//------------------------------------------------------------------------------
// Samples:           MorphHashDestroy( pHash );
//------------------------------------------------------------------------------
// Function flow:     destroy hash table
//                    destroy string pool
//                    free element table
//                    free morph hash control area
//------------------------------------------------------------------------------
BOOL MorphHashDestroy
(
  PMORPHHASH    pMorphHash             // pointer to morph hash structure
)
{
  /********************************************************************/
  /* Destroy hash table                                               */
  /********************************************************************/
  if ( pMorphHash->pHash )
  {
    HashDestroy( pMorphHash->pHash );
  } /* endif */

  /********************************************************************/
  /* Destroy string pool                                              */
  /********************************************************************/
  if ( pMorphHash->StringPool )
  {
    PoolDestroy( pMorphHash->StringPool );
  } /* endif */

  /********************************************************************/
  /* Free element table                                               */
  /********************************************************************/
  if ( pMorphHash->pElements )
  {
    UtlAlloc( (PVOID *)&pMorphHash->pElements, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Free morph hash control area                                     */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pMorphHash, 0L, 0L, NOMSG );

  return( TRUE );
} /* end of function MorphHashDestroy */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashAdd        Add an entry to the morph hash
//------------------------------------------------------------------------------
// Description:       Adds a term and it's lemma list to the morph hash table.
//                    The term and the lemma list are stored in the string
//                    pool of the morph hash.
//------------------------------------------------------------------------------
// Function call:     MorphHashAdd( PMORPHHASH pMorphHash, PSZ pszTerm,
//                                  USHORT ulLemmaListLen, PSZ pszLemmaList );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszTerm        name of term being added
//                    USHORT      ulLemmaListLen length of lemma list in bytes
//                    PSZ         pszLemmaList   lemma list for term or NULL
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHHASH pHash;
//
//                    pHash = MorphHashCreate( 500 );
//                    MorphHashAdd( pHash, "terms", 5, "term" );
//------------------------------------------------------------------------------
// Function flow:     check if term is already in cash
//                    if all elements of hash are in use then
//                      delete oldest element in cash
//                    endif
//                    add term and term data to string pool
//                    add new element to low level hash
//                    update recently used chain
//------------------------------------------------------------------------------
BOOL MorphHashAdd
(
  PMORPHHASH   pMorphHash,             // pointer to our hash control strucuture
  PSZ_W        pszTerm,                // name of term being added
  ULONG        ulLemmaListLen,         // length of lemma list in bytes
  PSZ_W        pszLemmaList            // lemma list for term or NULL
)
{
  HASHENTRY    HashEntry;              // buffer for new hash entry
  PHASHENTRY   pHashEntry;             // pointer to found/created hash entry
  ULONG        ulEntry;                // number (index) of new entry

  /********************************************************************/
  /* Check if term is already in cash                                 */
  /********************************************************************/
  HashEntry.pszTerm = pszTerm;
  pHashEntry = (PHASHENTRY) HashSearch( pMorphHash->pHash, &HashEntry );

  /********************************************************************/
  /* Remove oldest entry from cache if all cache elements are in use  */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    if ( pMorphHash->ulElements == pMorphHash->ulMaxElements  )
    {
      /****************************************************************/
      /* Cache is full: remove oldest element from cache              */
      /****************************************************************/
      MorphHashDelete( pMorphHash, pMorphHash->pElements + pMorphHash->ulFirstUsed );
      pHashEntry = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add term and lemma list to string pool                           */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    HashEntry.pszTerm = PoolAddStringW( pMorphHash->StringPool, pszTerm );
    HashEntry.ulListLength = ulLemmaListLen;
    if ( pszLemmaList )
    {
      HashEntry.pLemmaList = (PSZ_W) PoolAddData( pMorphHash->StringPool,
                                         (ulLemmaListLen * sizeof(CHAR_W)),
                                          pszLemmaList );
    }
    else
    {
      HashEntry.pLemmaList = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add new element to low level hash                                */
  /********************************************************************/
  if ( !pHashEntry )                   // if not in cache yet ...
  {
    /******************************************************************/
    /* Get entry from free chain                                      */
    /******************************************************************/
    ulEntry = pMorphHash->ulFirstFree;
    pHashEntry = pMorphHash->pElements + ulEntry;
    pMorphHash->ulFirstFree = pHashEntry->ulNext;
    pMorphHash->ulElements++;

    /******************************************************************/
    /* Fill-in element data                                           */
    /******************************************************************/
    pHashEntry->pszTerm      = HashEntry.pszTerm;
    pHashEntry->ulListLength = HashEntry.ulListLength;
    pHashEntry->pLemmaList   = HashEntry.pLemmaList;

    /******************************************************************/
    /* add element to end of recently used chain                      */
    /******************************************************************/
    pHashEntry->ulNext = END_OF_ELEMENTS;
    pHashEntry->ulPrev = pMorphHash->ulLastUsed;
    if ( pMorphHash->ulLastUsed != END_OF_ELEMENTS )
    {
      pMorphHash->pElements[pMorphHash->ulLastUsed].ulNext = pHashEntry->ulKey;
    } /* endif */
    pMorphHash->ulLastUsed = ulEntry;
    if ( pMorphHash->ulFirstUsed == END_OF_ELEMENTS )
    {
      pMorphHash->ulFirstUsed = ulEntry;
    } /* endif */

    /******************************************************************/
    /* Add element to low level hash                                  */
    /******************************************************************/
    HashAdd( pMorphHash->pHash, pHashEntry );
  } /* endif */

  return( TRUE );
} /* end of function MorphHashAdd */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashDelete     Delete an entry in the morph hash
//------------------------------------------------------------------------------
// Description:       Delete an entry from the hash table. The term and
//                    lemma list stay in the string pool.
//------------------------------------------------------------------------------
// Function call:     MorphHashDelete( PMORPHHASH pMorphHash, PSZ pszTerm );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszTerm        name of term being added
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       in any case
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Samples:           PMORPHHASH pHash;
//
//                    pHash = MorphHashCreate( 500 );
//                    MorphHashAdd( pHash, "terms", 5, "term" );
//                    MorphHashDelete( pHash, "terms" );
//------------------------------------------------------------------------------
// Function flow:     call low level hash delete function
//------------------------------------------------------------------------------
BOOL MorphHashDelete
(
  PMORPHHASH   pMorphHash,             // pointer to our hash control strucuture
  PHASHENTRY   pEntry                  // pointer to the entry being sdeleted
)
{

  // delete the hash entry, the callback function MorphHassFree will be called (which adjusts the recently used chain and the free entries chain)
  if ( HashDelete( pMorphHash->pHash, pEntry ) )
  {
    return( TRUE ); // nothing to do any more, MorphHashFree has done the house keeping
  } /* endif */

  // this part of the function is called when HashDelete failed to delete the entry,
  // we will have to remove the entry from the MorphHash table ...
  MorphHashFree( pEntry, (PVOID)pMorphHash );

  return( TRUE );
} /* end of function MorphHashDelete */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashRead      Build a hash table from a disk file
//------------------------------------------------------------------------------
// Description:       Fills a hash table from a disk file produced by
//                    MorphHashWrite.
//
//                    The entries read from disk are added to the hash table in
//                    the same order as they appear in the disk file.
//
//                    Incomplete entries or corrupted entries are ignored.
//------------------------------------------------------------------------------
// Function call:     MorphHashRead( PMORPHHASH pMorphHash, PSZ pszFileName );
//------------------------------------------------------------------------------
// Input parameter:   PMORPHHASH  pMorphHash     pointer to hash control block
//                    PSZ         pszFileName    name of input file
//                    BOOL        fHasLemmaList  has-a-lemma-list flag
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE       hash table successful read from disk
//                    FALSE      read of hash table failed
//------------------------------------------------------------------------------
// Prerequesits:      pHash must have been created using MorphHashCreate
//                    pszFileName must be the name of a file created using
//                       MorphHashWrite
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     allocate string buffer
//                    open the input file
//                    while no error condition raised
//                      read record length
//                      read record
//                      get pointer to lemma list and length of lemma list
//                      add record data to hash table
//                    endwhile
//                    close input file and free allocated memory
//------------------------------------------------------------------------------
BOOL MorphHashRead
(
  PMORPHHASH    pMorphHash,            // pointer to morph hash structure
  PSZ           pszFileName,           // name of input file
  BOOL          fHasLemmaList          // has-a-lemma-list flag
)
{
  PBUFCB       pBufCB = NULL;          // control block for buffered output
  BOOL         fOK;                    // internal O.K. flag
  USHORT       usDosRC;                // return code of DosXXX calls
  PSZ_W        pStringBuffer;          // pointer to buffer for strings
  ULONG        ulBytesRead;            // # of bytes read from file
  USHORT       usRecLen;               // length of disk record
  ULONG        ulLemmaListLength = 0;      // length of lemma list in bytes
  PSZ_W        pszLemmaList = NULL;    // ptr to lemma list

  /********************************************************************/
  /* Allocate string buffer                                           */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pStringBuffer, 0L,
                          (LONG)BUFFER_SIZE *sizeof(CHAR_W), ERROR_STORAGE );

  /********************************************************************/
  /* Open the input file                                              */
  /********************************************************************/
  if ( fOK )
  {
     usDosRC = UtlBufOpen( &pBufCB, pszFileName, BUFFER_SIZE,
                           FILE_OPEN, FALSE );
     fOK = ( usDosRC == MORPH_OK );
  } /* endif */

  /********************************************************************/
  /* Read all elements in disk file and add the elements to the hash  */
  /* table                                                            */
  /********************************************************************/
  while ( fOK )
  {
    /******************************************************************/
    /* Read record length                                             */
    /******************************************************************/
    usDosRC = UtlBufRead( pBufCB,
                          (PSZ)&usRecLen,
                          sizeof(USHORT),
                          &ulBytesRead,
                          FALSE );
    if ( (usDosRC != MORPH_OK) ||
         (ulBytesRead != sizeof(USHORT)) ||
         (usRecLen > BUFFER_SIZE) )
    {
      fOK = FALSE;
    } /* endif */

    /******************************************************************/
    /* Read record                                                    */
    /******************************************************************/
    if ( fOK )
    {
      usDosRC = UtlBufRead( pBufCB,
                            (PSZ)pStringBuffer,
                            usRecLen,
                            &ulBytesRead,
                            FALSE );
      fOK = ( usDosRC == NO_ERROR ) && ( ulBytesRead == (ULONG)usRecLen );
    } /* endif */

    /******************************************************************/
    /* Get pointer to lemma list and length of lemma list             */
    /******************************************************************/
    if ( fOK )
    {
      pszLemmaList = pStringBuffer + UTF16strlenCHAR(pStringBuffer) + 1;
      ulLemmaListLength = *((PUSHORT)pszLemmaList);
      pszLemmaList++;                  // skip lemma list length field (sizeof(USHORT))

      if ( !ulLemmaListLength || !fHasLemmaList )
      {
        pszLemmaList      = NULL;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Add record data to hash table                                  */
    /******************************************************************/
    if ( fOK )
    {
      fOK = MorphHashAdd( pMorphHash, pStringBuffer,
                          ulLemmaListLength, pszLemmaList );
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /* Close input file and free allocated memory                       */
  /********************************************************************/
  if ( pBufCB )         UtlBufClose( pBufCB, FALSE );
  if ( pStringBuffer )  UtlAlloc( (PVOID *)&pStringBuffer, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function MorphHashRead */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashCompare    Compare two hash entries
//------------------------------------------------------------------------------
// Description:       Compare the terms of two hashentries and return the
//                    result.
//                    This function is used by the low level hash functions to
//                    compare two has entries.
//------------------------------------------------------------------------------
// Function call:     MorphHashCompare( PHASHENTRY pEntry1, PHASHENTRY pEntry2)
//------------------------------------------------------------------------------
// Input parameter:   PHASHENTRY  pEntry1     pointer to first hash entry
//                    PHASHENTRY  pEntry2     pointer to second hash entry
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       -1   if term in pEntry1 is smaller than term in pEntry2
//                     0   if term in pEntry1 is equal to term in pEntry2
//                     1   if term in pEntry1 is greater than term in pEntry2
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     return result of strcmp function
//------------------------------------------------------------------------------
SHORT MorphHashCompare
(
  PHASHENTRY pEntry1,
  PHASHENTRY pEntry2,
  PVOID      pUserPtr
)
{
  pUserPtr;                            // avoid compiler warning

  return (SHORT)(UTF16strcmp( pEntry1->pszTerm, pEntry2->pszTerm ));
} /* end of function MorphHashCompare */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashFree       Free a hash entry
//------------------------------------------------------------------------------
// Description:       Is called by the low level hash functions to free
//                    a hash element.
//                    The hash elements have been allocated in one memory
//                    block, so individual elements can't be freed. Freed
//                    elements are chained to the free element list instead.
//------------------------------------------------------------------------------
// Function call:     MorphHashFree( PHASHENTRY pEntry );
//------------------------------------------------------------------------------
// Input parameter:   PHASHENTRY pEntry     pointer to entry to be freed
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       TRUE    in any case
//------------------------------------------------------------------------------
// Prerequesits:      none
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     get pointer to morph hash control block
//                    remove deleted element from recently used chain
//                    add element to free list
//                    return TRUE
//------------------------------------------------------------------------------
SHORT MorphHashFree( PHASHENTRY pEntry, PVOID pUserPtr )
{
  PMORPHHASH    pMorphHash;            // pointer to morph hash structure

  /********************************************************************/
  /* Get pointer to morph hash control structure                      */
  /********************************************************************/
  pMorphHash = (PMORPHHASH)pUserPtr;

  /********************************************************************/
  /* Remove element from recently used chain                          */
  /********************************************************************/
  if ( pEntry->ulPrev != END_OF_ELEMENTS )
  {
    pMorphHash->pElements[pEntry->ulPrev].ulNext = pEntry->ulNext;
  }
  else
  {
    pMorphHash->ulFirstUsed = pEntry->ulNext;
  } /* endif */
  if ( pEntry->ulNext != END_OF_ELEMENTS )
  {
    pMorphHash->pElements[pEntry->ulNext].ulPrev = pEntry->ulPrev;
  }
  else
  {
    pMorphHash->ulLastUsed = pEntry->ulPrev;
  } /* endif */

  /********************************************************************/
  /* Add element to free list                                         */
  /********************************************************************/
  pEntry->ulNext = pMorphHash->ulFirstFree;
  pMorphHash->ulFirstFree = pEntry->ulKey;
  pMorphHash->ulElements--;

  return( TRUE );
} /* end of function MorphHashFree */



/* Two random permutations of the numbers 0..255                             */
static BYTE random1[256] = {
149, 184,  28, 173, 213,  98,  14,  87, 157,  78,  31,  62, 226, 165, 120, 143,
251, 233, 227, 122, 146,  20, 106,  92, 197, 148, 102, 241,  29, 218,  69, 228,
153,  70, 249, 204, 214, 219, 234, 195, 152, 217,  65, 254,   8,  73, 119, 230,
206, 244, 246, 248, 216,  39, 128,  55,  75, 123, 189, 155,  38, 232,  99, 116,
118, 104, 160, 190, 215,  58,  54, 242,  72,  57,   7, 202, 192,  12,  96,  43,
 95, 177, 193,  51, 127, 247,  19,  40,  34, 147, 212,  42, 203, 239, 112,  93,
208, 201, 109,  90, 176,  59, 223,  94, 133, 111, 130,   5,  67, 222,  22, 209,
131, 169,  15, 174,  79,   6,  49,  46,   3, 142,  63, 107, 154, 182, 224, 186,
168,  60, 181, 125, 207,  52, 166,  45,  41, 188, 164, 240, 185, 161, 200,  97,
255, 250,   2,  89, 158,  30,  53,  21,  44, 235,  23, 187, 198, 205,  88, 121,
162,  11,  25, 170,  50,   4, 211, 243,   0,  37,  24, 196, 144, 129,  91,  86,
 76,  26,  18, 220, 237, 210, 194, 136, 156, 100,  27, 229, 137, 101, 236, 191,
221, 159,  35, 126,  68,  71,  10, 183, 179, 110, 150,  36, 151,  85,  82, 114,
  9,  81, 124, 172,  66,   1,  83,  77, 252, 105, 145,  13,  64, 238, 163, 140,
180, 113, 178, 108,  80,  56, 225, 134, 245,  32, 103,  74,  33,  61, 199, 117,
253, 231,  48,  47,  16, 141, 115, 167, 138,  84, 135, 139,  17, 132, 175, 171
};

static BYTE random2[256] = {
247,  72, 239, 215,  34, 179,  66,  63, 229, 107, 127, 226,  96, 105, 110, 197,
 27, 241, 251,   7, 146,  44, 230, 238, 153,  86, 104, 155, 163, 130,  55, 195,
240, 149,  73,  85, 196,   8,  98, 253, 208, 212, 102,  53, 165, 124, 128,  70,
174, 111, 151,  10, 114, 125, 220, 211,  93, 106,  54,  30, 156, 250, 122,  41,
252, 170, 232, 100,  28,   6,  81, 200, 168,  91, 249,  90,  84, 219, 244,  19,
193,  38,  18,  32,  79,  71, 227,  83,  47, 213,  75,  15,  22,  21,  45, 236,
 43, 180,  76, 248,  24, 234, 205,  35,  77, 184, 235, 202,  57,  69,  29, 203,
109, 167,  89, 135, 190, 123, 141, 217, 164,  94, 255,  65,  92, 150,   3, 152,
192, 112, 218,  39,  61,  74, 214, 204, 148,  62, 103, 198, 187, 172, 166,   4,
 16,  88,  46, 224,  56,  87,  36,  99,  95, 126, 185, 201,  50,  20, 160,  67,
242, 245, 133, 101, 158, 145, 154,  78, 136, 132,  49, 225,  80, 188,  48, 237,
 40, 228,  11, 143, 231, 221,  26, 194,   0, 243, 129, 171, 108, 161,  12, 209,
 25, 182, 137,  33, 189,  97, 254,  68, 162, 176, 113,  58, 147, 246,  31,  51,
121,  23, 186,  13, 178,  64,  14,  37, 216, 142,   1, 223, 183,   5,  59,  42,
119,  17, 144, 199,  52, 131, 157, 134, 207, 117, 120,   9, 222, 233, 210, 191,
159, 116, 181, 169, 206, 175,  60, 140, 139, 173, 118, 177, 115, 138,   2,  82
};

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphHashKeyValue   Get hash value for an entry
//------------------------------------------------------------------------------
// Description:       Computes the hash value of an entry by 'OR'ing the
//                    bytes which make up the term of the entry. The bytes
//                    are converted using two random tables.
//                    The returned value will be in the range from
//                    0 to 65535.
//------------------------------------------------------------------------------
// Function call:     MorphHashKeyValue( usSize, pEntry );
//------------------------------------------------------------------------------
// Input parameter:   USHORT     usSize       size of hash table
//                    PHASHENTRY pEntry       point to hash entry
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       any          hash value for entry
//------------------------------------------------------------------------------
// Side effects:      none
//------------------------------------------------------------------------------
// Function flow:     while not end of term
//                      exor table1 value of character to hash1 value
//                      exor table2 value of character to hash2 value
//                      go to next character
//                    endwhile
//                    return ushort from hash1 and hash2 value
//------------------------------------------------------------------------------
USHORT MorphHashKeyValue
(
  ULONG     ulSize,                   // size of hash table
  PHASHENTRY pEntry                    // point to hash entry
)
{
  PSZ_W  pbTerm = pEntry->pszTerm;
  USHORT hash1 = 0;
  USHORT hash2 = 0;

  ulSize;                              // avoid compiler warning

  while ( *pbTerm )
  {
    // GQ 2018/05/09: The following code is not Unicode enabled, index into  random tables had to be truncated to 256 (BYTE value) 
    hash1 = random1[(BYTE)(hash1 ^ *pbTerm) ]; 
    hash2 = random2[(BYTE)(hash2 ^ *pbTerm++) ];
  }
  return (( hash1 << 8) | hash2);
} /* end of function MorphHashKeyValue */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MorphCheck4Refresh  Check if refresh is required
//------------------------------------------------------------------------------
// Description:       If the "dirty" flag is set for an addenda or abbreviation
//                    dictionary, the dictionary is refreshed and the
//                    dirty flag is cleared.
//------------------------------------------------------------------------------
// Function call:     MorphRefreshDicts( pLCB );
//------------------------------------------------------------------------------
// Input parameter:   PLCB       pLCB         pointer to current control block
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
USHORT MorphCheck4Refresh
(
  PLCB       pLCB                      // pointer to current control block
)
{
  USHORT      usRC = MORPH_OK;         // function return code

  if ( pLCB->fAddendaDirty )
  {
    usRC = BuildDict( pLCB->pvLangCB, ADDENDA_DICT, 0, NULL, 0 );
    pLCB->fAddendaDirty = FALSE;
  } /* endif */

  if ( (usRC == MORPH_OK) && pLCB->fAbbrevDirty )
  {
    usRC = BuildDict( pLCB->pvLangCB, ABBREV_DICT, 0, NULL, 0 );
    pLCB->fAbbrevDirty = FALSE;
  } /* endif */

  return( usRC );
} /* end of function MorphCheck4Refresh */

