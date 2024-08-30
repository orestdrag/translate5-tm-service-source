//+----------------------------------------------------------------------------+
//|EQFMORPW.C      Word Recognition for morphological functions of TM          |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:    G. Queck    QSoft Quality Software Development GmbH              |
//+----------------------------------------------------------------------------+
//|Description:           API for morphological functions                      |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|  MorphWordRecognition    - Detect words for dictionary lookup              |
//|                                                                            |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_MORPH            // morphologic functions
#include <EQF.H>                  // General Translation Manager include file

#include "OtmDictionaryIF.H"
#include <EQFMORPI.H>                  // private header file of module
#include <EQFCHTBL.H>                  // character tables
#include "LogWrapper.h"

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     MorphWordRecognition   Split a segment into terms        |
//+----------------------------------------------------------------------------+
//|Description:       WordRecoginitions tokenizes a string and stores all terms|
//|                   found in a term list. WordRecognition uses the supplied  |
//|                   dictionary to detect multi-word terms.                   |
//|                   :p.                                                      |
//|                   If the switch fInclNotFoundTerms is FALSE, terms not     |
//|                   contained in the dictionary are not listed in the term   |
//|                   list.                                                    |
//|                   :p.                                                      |
//|                   A term list is series of null-terminated strings         |
//|                   which is terminated by another null value.               |
//|                   See the sample for the layout of a term list.            |
//|                   :p.                                                      |
//|                   If the term list pointer is NULL and the term list size  |
//|                   is zero, the term list is allocated.                     |
//|                   The term list is enlarged if not all terms fit into      |
//|                   the current list.                                        |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = MorphWordRecoginition( SHORT sLanguageID,         |
//|                                                 PSZ pszInData,             |
//|                                                 HUCB hUCB, HDCB hDCB,      |
//|                                                 PUSHORT pusBufferSize,     |
//|                                                 PSZ *ppTermList,           |
//|                                                 BOOL fInclNotFoundTerms,   |
//|                                                 PUSHORT pusOrgRC );        |
//+----------------------------------------------------------------------------+
//|Input parameter:   SHORT    sLanguageID    language ID                      |
//|                   PSZ      pszInData      pointer to input segment         |
//|                   HUCB     hUCB           user control block handle        |
//|                   HDCB     hDCB           dictionary control block handle  |
//|                   BOOL     fInclNotFoundTerms   TRUE = not found terms are |
//|                                             included in the term list      |
//|                                           FALSE = not found terms are      |
//|                                             omitted from the term list     |
//|                   PUSHORT  pusOrgRC       points to buffer for any return  |
//|                                           code returned from called functs.|
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT  pusOrgBufferSize  address of variable containing|
//|                                                size of term list buffer    |
//|                   PSZ      *ppOrgTermList    address of caller's term list |
//|                                                pointer for original terms  |
//|                   PUSHORT  pusDictBufferSize address of variable containing|
//|                                                size of term list buffer    |
//|                   PSZ      *ppDictTermList   address of caller's term list |
//|                                                pointer for dictionary terms|
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       MORPH_OK    function completed successfully              |
//|                   MORPH_ASD_ERROR  an ASD error occured, original ASD      |
//|                               return code is stored at location pointed to |
//|                               by pusOrgRC                                  |
//|                   other       MORPH_ error code, see EQFMORPH.H for a list |
//+----------------------------------------------------------------------------+
//|Prerequesits:      sLanguageID must have been obtained using the            |
//|                   MorphGetLanguageID function.                             |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           USHORT usListSize = 0;     // allocated list size        |
//|                   PSZ    pszList = NULL;     // ptr to list area           |
//|                   SHORT  sLangID;            // language ID                |
//|                                                                            |
//|                   usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );  |
//|                                                                            |
//|                   usRC = MorphWordRecoginition( sLangID,                   |
//|                             "Regular data backup is a prerequisit for      |
//|                                system recovery", &usListSize, &pszList,    |
//|                                NULL, NULL,                                 |
//|                                FALSE );                                    |
//|                                                                            |
//|                   Assuming that the dictionary referenced by hDCB contains |
//|                   the following terms 'data', 'backup', 'data backup',     |
//|                   'system' and 'recovery', a call to MorphWordRecognition  |
//|                   would return the following term list:                    |
//|                                                                            |
//|                     "data backup\0system\0recovery\0\0"                    |
//+----------------------------------------------------------------------------+
//|Function flow:     Check input data                                         |
//|                   Get language control block pointer                       |
//|                   get copy of input segment                                |
//|                   normalize segment by reducing mulitple blanks and CRLF   |
//|                     to single blank                                        |
//|                   tokenize the normalized segment using MorphTokenize      |
//|                   allocate buffer for stem form reduced segment            |
//|                   loop over all terms in segment                           |
//|                      get stem form of term                                 |
//|                      add term's stem form to stem form segment             |
//|                      add delimiting character up to next term to           |
//|                        stem form segment                                   |
//|                   endloop                                                  |
//|                   loop over all terms in segment                           |
//|                      reduce term to stem form                              |
//|                      get all MWTs for term                                 |
//|                      if term found in dictionary                           |
//|                         loop over all MWTs of term                         |
//|                            compare MWT with data in normalized segment     |
//|                            if not matched                                  |
//|                               compare MWT with data in stem form segment   |
//|                            if matched                                      |
//|                               leave loop                                   |
//|                         endloop                                            |
//|                      if MWT found                                          |
//|                         add MWT to term list                               |
//|                         skip all subterms of MWT                           |
//|                      elsif term found or fIncludeNotFoundTerms is set      |
//|                         add term to term list                              |
//|                      end                                                   |
//|                     endloop                                                |
//|                     return function return code                            |
//+----------------------------------------------------------------------------+

BOOL IsDBCSLeadByteEx(
  UINT CodePage,
  BYTE TestChar
);
#define isdbcs1ex(usCP, c)  ((IsDBCSLeadByteEx( usCP, (BYTE) c )) ? DBCS_1ST : SBCS )
#define NO_ERROR 0
