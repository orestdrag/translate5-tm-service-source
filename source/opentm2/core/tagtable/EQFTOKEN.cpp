//	Copyright Notice:
//
//	Copyright (C) 1990-2014, International Business Machines
//	Corporation and others. All rights reserved

/*! \file

	
	This function identifies all tags in the passed  text block and builds
	a token list for the text block. The tags are identified using the
	Tagtable provided by the calling function.
	The token list area has to be allocated by the calling function.
	A token in the token list consists of:

		-  Token Identifier (which is a Tag Id or TEXT or ENDOFLIST)
		-  pointer to start of token in the text block
		-  length of token

	The token list is terminated by the ENDOFLIST token.

	If the calling function passes an incomplete text block, i.e. more
	text will follow in subsequent calls, the current text block will
	be tokenized up to the last complete token, and a pointer to the
	rest of input, which has not been tokenized will be returned.

	If the calling function does not provide an area large enough to
	build the token list for the complete text block in, EQFTagTokenize
	will stop tokenizing when the token list area is full and return
	a pointer to the rest of input that has not been tokenized.

	The following parameters have to be provided by the calling function:

	- pTextBuffer  (PSZ)
	  Pointer to the text buffer that shall be tokenized.
	  The text must be null-terminated.

	- pTagTable    (TAGTABLE *)
	  Pointer to the start of the tag table to be used for tokenizing.

	- fAll         (BOOL)
	  Flag which indicates whether a complete text block was passed or
	  more text will follow.
	  fAll = TRUE:  the text is complete and is to be tokenized up to the
					very end
	  fAll = FALSE: the text is not complete and more text blocks will follow
					in subsequent calls, thus there might be an incomplete
					token at the end.

	- ppRest       (PSZ *)
	  Pointer to the rest of the text buffer. It indicates up to where the
	  text has been tokenized by EQFTagTokenize. If the text block has been
	  completely tokenized, *ppRest points to the last character in the text
	  buffer.
	  Cases where a text block will not be completely tokenized are either
	  if fAll is FALSE and thus there may be an incomplete token at the end
	  of the text or if the memory allocated for the tokenlist is not large
	  enough to hold all tokens of the text block.

	- pusLastColPos (USHORT *)
	  Pointer to the column position where the text block starts. When
	  EQFTagTokenize is called for the first time for a text the column
	  position has to be initialized with 0 (* pusLastColPos = 0).
	  EQFTagTokenize will return the column position of the last character
	  up to where the text block has been tokenized. If subsequent text
	  blocks are passed to EQFTagTokenize by the calling function the
	  previously returned column position has to be provided.

	- pTokenlist   (TOKENENTRY *)
	  Pointer to the memory allocated for the tokenlist by the calling
	  function

	- uMaxNumTokens (USHORT)
	  The maximum number of Tokenlist entries that fit in the memory
	  allocated by the calling function

*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#include <EQF.H>                  // General Translation Manager include file

#include "EQFTAI.H"               // Private include file for Text Analysis

#include "EQFTAG00.H"             

#include "win_types.h"
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include "LogWrapper.h"

#define DBLQUOTE   '\"'                     // double quote character (")

// size of buffer (in number of tokens) for quoted string tokenization in TACreateProtectTableW
#define MAX_ATTR_TOKENS 512

//--- Function prototypes for internal C functions                                */
static BOOL Tagsearch (PSZ, ULONG, USHORT, BOOL, PTAG, CHAR *, TAGTABLE *, SHORT *);
static BOOL Attributesearch(PSZ, ULONG, PATTRIBUTE, CHAR * ,TAGTABLE *, SHORT *);
static BOOL Skipwhitespace(PSZ *, USHORT *);
static BOOL WildCardSearch (PSZ, PSZ );
static PSZ SkipToTextEnd( PSZ  );
static PSZ SkipTextString ( PSZ, USHORT * );
#define MAX_CHINPUTW 40000
static CHAR_W chInputW[ MAX_CHINPUTW ];

#define TAG_END_CHAR       "."        // tag end character
#define TAG_END_CHARW      L"."
PNODE  TAFastCreateNodeTree( PTBTAGENTRY pTags, ULONG ulNoOfTags );
PNODE  TAFastCreateTagTree( PLOADEDTABLE pLoadedTable, PTAGTABLE pTagTable );
PNODE  TAFastCreateAttrTree( PLOADEDTABLE pLoadedTable, PTAGTABLE pTagTable );
BOOL   TAFastAddNode ( PNODE *, PNODE, PULONG, PULONG );
int    TATastTagCompare( const void *, const void * );

VOID TASetStartStopType(PSTARTSTOP pCurrent, USHORT usStart, USHORT usStop, USHORT usType );
USHORT TAFindQuote ( PTOKENENTRY pTok, USHORT usStartSearch, PCHAR_W pchQuote );
USHORT  TAProtectedPartsInQuotedText
(
  PTOKENENTRY   pTok,
  int           iAttrStartOffset,
  PSTARTSTOP    * pStartStop,
  PSTARTSTOP    * pCurrent,
  PTOKENENTRY     pAttrTokBuffer,
  PLOADEDTABLE    pTagTable,
  PULONG          pulTableAlloc,
  PULONG          pulTableUsed,
  PUSHORT         pusColPos,
  CHAR_W          chQuote,
  LONG            lAttrSize,
  BOOL            fSpecialMode
);

USHORT TAGotoNextStartStopEntry
(
	PSTARTSTOP  * ppStartStop,
	PSTARTSTOP  * ppCurrent,
	PULONG    pulTableUsed,
	PULONG    pulTableAlloc
);
static void
 TATagAddWSpace
   (
	   PSZ_W            pszSegment,
	   PTOKENENTRY  	pTokBuffer,
	   SHORT            sNumTags
   );


/**********************************************************************/
/* add pragmas to force load into different text segments             */
/**********************************************************************/
//#pragma alloc_text(EQFTOKEN1, EQFTagTokenize)
//
//#pragma alloc_text(EQFTOKEN2, TAFastTokenize)
//
//
//#pragma alloc_text(EQFTOKEN3, TACreateTagTree, TACreateAttrTree,          \
//                              TACreateNodeTree)
//
//#pragma alloc_text(EQFTOKEN4, TATagTokenize, TAMatchTag,                  \
//                              TACreateProtectTable, TAPrepProtectTable,   \
//                              TAEndProtectTable)

#define TA_IS_TAG(a,b)  ( (a < b) && (a >=0) )

#define TA_IS_ATTRIBUTE(a,b)  (a >= b)


//------------------------------------------------------------------------------
//   New TagTokenize function TATagTokenize
//   and related defines, macros and functions
//
//   The new approach for tag recognition reduces the time for tag recognition
//   by minimizing the number of compare operation required to check for tags.
//
//   The tag table is converted to a node tree there each node represents a
//   character of the tag table and the links between the nodes are the
//   possible paths for valid tags.
//
//   An example:  Assume a tag table containing the following tags:
//
//                    :TAG1.
//                    :TAG2.
//                    :NONE.
//
//                The node tree for this trivial tag table looks like this:
//
//                                        :
//                                    +---�---+
//                                    N       T
//
//                                    O       A
//
//                                    N       G
//                                         +--�--+
//                                    E    1     2
//
//                                    .    .     .
//
//
//                The scan for tags always starts at the nodes in the root
//                level of the tree. If the character in text does not match
//                the root character(s), the character in the text is no tag
//                start.
//
//                If a match occurs the next character in the text is
//                compared with the nodes linked to the matched root character.
//                If again a matching character is found, the check continues
//                on the next level of the node tree until a mismatch occurs
//                or the end of a branch has been reached (in this case a
//                tag has been recognized).
//
//                This approache ensures that only a minimal number of
//                characters has to be compared for tag recognition.
//
//------------------------------------------------------------------------------

#define RETRYSTACKSIZE      50         // retry stack size in # of elements
#define MSSTACKSIZE         10         // mulitple substitution stack size
                                       //    in # of elements

//------------------------------------------------------------------------------
//   Table to compare characters in node trees
//   the index value for the table is:
//
//   0 for multiple substitution characters
//   1 for single substitution characters
//   2 for end of string delimiters (EOS)
//   3 for column based tag characters
//   4 for all other characters
//
//   the first index is the index for the current tag character
//   the second index is the index for current node character
//
//   the table contains -1 if the character is smaller
//                       1 if the character is greater
//                       0 if the characters should be compared
//------------------------------------------------------------------------------
// fix for KBT0022: treat '?' chars as equal
SHORT asCharCompare[5][5] =
{ {  1,  1, -1,  1,  1 },
  { -1,  0, -1,  1,  1 },
  {  1,  1,  1,  1,  1 },
  { -1, -1, -1,  0, -1 },
  { -1, -1, -1,  1,  0 } };



//------------------------------------------------------------------------------
//   Update column position macro: increment column position, set to
//   zero if line end is reached
//------------------------------------------------------------------------------
#define UPDATECOLPOS( chChar, usColPos )    \
  if ( ( chChar == CR) || (chChar == LF))   \
  {                                         \
    usColPos = 0;                           \
  }                                         \
  else                                      \
  {                                         \
    usColPos++;                             \
  } /* endif */

//------------------------------------------------------------------------------
//   Table for uppercase conversion
//   the index into the table is the character being uppercased, the value
//   at the table position is the uppercased character
//------------------------------------------------------------------------------
BYTE chLookup[256] =
   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
     0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
     0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
     0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
     0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
     0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
     0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
     0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
     0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
     0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
     0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
     0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

USHORT chLookupW[256] =
   { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
     0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
     0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
     0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
     0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
     0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
     0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
     0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
     0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
     0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
     0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
     0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
     0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
     0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
     0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
     0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
     0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
     0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
     0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
     0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
     0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
     0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };


//------------------------------------------------------------------------------
//   Table for whitespace recognition
//   the index into the table is the character being tested, the value
//   at the table position is 0 if the character is no whitespace character or
//   1 if the character is a whitespace character
//------------------------------------------------------------------------------
CHAR fWhiteSpace[256] =
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TATagTokenize
//------------------------------------------------------------------------------
// Function call:     TATagTokenize( PSZ pszInput, PLOADEDTABLE pVoidTable,
//                                   BOOL  fComplete, PSZ * &pRest,
//                                   PUSHORT &usColPos, PTOKENENTRY pTokBuf,
//                                   USHORT usTokens );
//------------------------------------------------------------------------------
// Description:       Mainline for tag tokenization based on tag tables.
//------------------------------------------------------------------------------
// Parameters:        PSZ          pszInput    pointer to input data
//                    PLOADEDTABLE pVoidTable  pointer to loaded tag table
//                    BOOL         fComplete   TRUE = no more buffers to follow
//                    PSZ          *ppRest     pointer to not processed data in
//                                             buffer
//                    PUSHORT      pusColPos   column position
//                    PTOKENENTRY  pTokBuf     buffer for created tokens
//                    USHORT       usTokens    max entries in token buffer
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//                         TRUE  = succesful tokenized
//                         FALSE = something went wrong (main reason for this
//                                 are memory allocation problems)
//------------------------------------------------------------------------------
// Prerequesits:      Tag table pointed to by pVoidTable must have been loaded
//                    using TALoadtagTable function.
//------------------------------------------------------------------------------
// Function flow:     Initialize tokenize status area
//                    Create tag trees if not done yet
//                    Loop while there is data and token buffer is not full
//                       match next tag
//                       if a tag was found then
//                          Build token for data up to start of recognized tag
//                          Add tag token
//                          if tag may have attributes then
//                          do
//                             match attribute
//                          while attributes are found and token buffer is
//                            not full
//                       else
//                         if data is complete then
//                            add remaining data as TEXT token
//                         else
//                            process remaining data the next time ...
//                         endif
//                       endif
//                    endloop
//                    if not processed completely or not last part of file
//                    set pRest pointer to begin of last tag or (if no tags
//                    are contained in data) to last linebreak.
//------------------------------------------------------------------------------
BOOL TATagTokenizeW
(
   PTMWCHAR       pszInput,            // pointer to input data
   PLOADEDTABLE pVoidTable,            // pointer to loaded tag table
   BOOL      fComplete,                // TRUE = no more buffers to follow
   PTMWCHAR      *ppRest,              // pointer to not processed data in buffer
   PUSHORT   pusColPos,                // column position
   PTOKENENTRY pTokBuf,                // buffer for created tokens
   USHORT      usTokens                // max entries in token buffer
)
{
   PTMWCHAR       pszData;             // pointer for input data processing
   PTMWCHAR       pszDataStart;        // pointer for end of data block
   PTOKENENTRY pToken;                 // pointer to current token
   PTMWCHAR       pszEnd;              // pointer to end of data
   BOOL        fFound = FALSE;         // flag used in setting pRest
   PLOADEDTABLE pTable = (PLOADEDTABLE)pVoidTable;   // pointer to loaded tag table
   TOKSTATUS   TokStatus;              // tokenize status area
   TOKENENTRY  NewToken;               // buffer for new tokens
   MSSTACK     MSStack[10];
   PTAG        pTag;                   // pointer to first tag in tagtable
   PTOKENENTRY pLastTag = NULL;        // pointer to last tag token
   USHORT      usTagColPos = 0;        // column position of last tag
   USHORT      usCurColPos;            // current column position
   BOOL        fProcessingAttributes;  // TRUE = attributes are processed
   PATTRIBUTE  pAttr;                  // ptr to attribute array of tag table
   PBYTE       pByte;                  // helper pointer
   SHORT       sTags;                  // number of tags in tag table
   BOOL        fOK = TRUE;             // function return code
   PTAGADDINFO pTagAddInfo;            // additional taginfo (tokenid, classid)

   // address tag and attribute array
   pByte = (PBYTE)pTable->pTagTable;
   pTag = (PTAG) (pByte + pTable->pTagTable->stFixTag.uOffset);
   pAttr = (PATTRIBUTE) ( pByte + pTable->pTagTable->stAttribute.uOffset);
   sTags = (SHORT)pTable->pTagTable->uNumTags;

   if ( pTable->pTagTable->usVersion >= ADDINFO_VERSION )
   {
     pTagAddInfo = (PTAGADDINFO) (pByte + pTable->pTagTable->uAddInfos);
   }
   else
   {
     pTagAddInfo = NULL;
   } /* endif */


   /*******************************************************************/
   /* Initialize tokenize status area                                 */
   /*******************************************************************/
   memset( &TokStatus, 0, sizeof(TokStatus) );
   TokStatus.usColPos = *pusColPos;

   TokStatus.chSingleSubst = pTable->chSingleSubstW;
   TokStatus.chMultSubst   = pTable->chMultSubstW;
   TokStatus.usMSStackSize = 10;
   TokStatus.pMSStack      = MSStack;
   TokStatus.pTable        = pTable;

   /*******************************************************************/
   /* Create tag trees if not done yet                                */
   /*******************************************************************/
   if ( pTable->pNodeArea == NULL )
   {
     // additional parameter needed for pTagNamesW
     pTable->pNodeArea = TACreateTagTree( pTable,
                                          pTable->pTagTable,
                                          TokStatus.chMultSubst,
                                          TokStatus.chSingleSubst,
                                          TRUE );
     fOK = ( pTable->pNodeArea != NULL );
   } /* endif */

   if ( fOK && (pTable->pAttrNodeArea == NULL) )
   {
     pTable->pAttrNodeArea = TACreateAttrTree( pTable,
                                               pTable->pTagTable,
                                               TokStatus.chMultSubst,
                                               TokStatus.chSingleSubst,
                                               TRUE );
     fOK = ( pTable->pAttrNodeArea != NULL );
   } /* endif */

   pTag = (PTAG)((PBYTE)pTable->pTagTable + pTable->pTagTable->stFixTag.uOffset );

   pszData = pszInput;                 // start at begin of input buffer
   pToken  = pTokBuf;                  // start at begin of token buffer
   pToken->sTokenid = TEXT_TOKEN;      // assume text data
   pToken->pDataStringW = pszData;      // remember start of data
   usTokens--;                         // reserve space for end of list token
   fProcessingAttributes = FALSE;      // we are waiting for tags ...
   memset(&NewToken, 0, sizeof(NewToken));
   while( fOK && *pszData && usTokens )// while not end of data and free tokens
   {
     USHORT      usProcessFlag;        // processing flag        /* @KWT0010A */
     
//   the following statement has been disabled to fix PTM KAT0330
//   fProcessingAttributes = FALSE;    // we are waiting for tags ...
     pszDataStart = pszData;
     usCurColPos  = TokStatus.usColPos;
     usProcessFlag = (fComplete ) ?                              /* @KWT0010A */
                      ( MATCH_SKIP_DATA|MATCH_DATA_COMPLETE) : MATCH_SKIP_DATA;
     fFound = TAMatchTag( &TokStatus,
                           &pszData,
                           pTable->pNodeArea->pRootNode,
                           &NewToken,
                           usProcessFlag);                       /* @KWT0010M */
     if ( fFound )
     {
       fProcessingAttributes = FALSE;

       /*************************************************************/
       /* Build token for data up to start of recognized tag        */
       /*************************************************************/
       if ( pszDataStart != NewToken.pDataStringW )
       {
         pToken->sTokenid = TEXT_TOKEN;      // text data
         pToken->pDataStringW = pszDataStart; // set start of data
         pToken->usLength = (USHORT)(NewToken.pDataStringW - pszDataStart);
         pToken->sAddInfo = 0;
         pToken->ClassId = CLS_DEFAULT;
         pToken->usOrgId = 0;
         usTokens--;
         pToken++;
       } /* endif */

       /*************************************************************/
       /* Add tag token                                             */
       /*************************************************************/
       if ( usTokens )
       {
         // special handling for tags which may be followed by attributes:
         // check if tag end character is a blank, when it is one look-ahead
         // for first non-blank character, when this non-blank character is
         // a valid end character for this tag include all blanks in tag
         if ( (NewToken.sTokenid >= 0) && pTag[NewToken.sTokenid].fAttr && (pTag[NewToken.sTokenid].uEndDelimOffs != 0) )
         {
           PSZ_W pszTest = NewToken.pDataStringW + (NewToken.usLength - 1);

           // only for blank end delimiters...
           if ( *pszTest == L' ')
           {
             PSZ_W pszEndDelim = pTable->pTagNamesW + pTag[NewToken.sTokenid].uEndDelimOffs;

             // find first non-blank character
             while ( *pszTest == L' ' )
             {
               pszTest++;
             } /*endwhile */

             // check if this character is a valid end-of-tag character
             if ( *pszTest != 0 )
             {
               while ( (*pszEndDelim != 0)  && (*pszEndDelim != *pszTest) )
               {
                 pszEndDelim++;
               } /*endwhile */

               // adjust tag length and scan position when valid end character found
               if ( *pszEndDelim != 0 )
               {
                 pszData = pszTest + 1;
                 NewToken.usLength = (USHORT)(pszData - NewToken.pDataStringW);
               } /* endif */
             } /* endif */
           } /* endif */
         } /* endif */

         // add tag token to token list
         pLastTag    = pToken;
         usTagColPos = TokStatus.usStartColPos;
         memcpy( pToken, &NewToken, sizeof(TOKENENTRY) );
         pToken->sAddInfo = pTag[NewToken.sTokenid].BitFlags.AddInfo;
         if ( pTagAddInfo )
         {
           pToken->ClassId = pTagAddInfo[NewToken.sTokenid].ClassId;
           pToken->usOrgId = pTagAddInfo[NewToken.sTokenid].usFixedTagId;
         } /* endif */
         pToken++;
         usTokens--;
       } /* endif */

       /**********************************************************/
       /* Process attributes if tag has any                      */
       /**********************************************************/
       if ( (NewToken.sTokenid >= 0) && pTag[NewToken.sTokenid].fAttr )
       {
         fProcessingAttributes = TRUE;      // attributes may follow ...
         do
         {
           /***********************************************************/
           /* If attributes are not delimited by whitespace           */
           /* characters they will be ignored (this makes sense only  */
           /* for bookmaster tags (e.g. :DL. COMPACT) but had to be   */
           /* added to be compatible to the old EQFTagTokenize        */
           /* function)                                               */
           /***********************************************************/
           /*********************************************************/
           /* Check the last character of the tag                   */
           /*********************************************************/
           if ( !fWhiteSpace[NewToken.pDataStringW[NewToken.usLength-1]] )
           {
             fFound = FALSE;
             fProcessingAttributes = FALSE;  // no more attributes
           }
           else
           {
             fFound = TRUE;
           } /* endif */

           /***********************************************************/
           /* Check for attributes                                    */
           /***********************************************************/
           if ( fFound )
           {
             usCurColPos  = TokStatus.usColPos;
             fFound = TAMatchTag( &TokStatus,
                                   &pszData,
                                   pTable->pAttrNodeArea->pRootNode,
                                   &NewToken,
                                   MATCH_SKIP_WHITESPACE |
                                   MATCH_QUOTED_STRINGS );
             // the following code has been disabled to fix KAT0330
             // if no attribute has been found the pszData pointer still
             // points to the begin of the maybe attribute ...
             // if ( !fFound && *pszData )
             // {
             //   /*******************************************************/
             //   /* Attribute mismatch, but not end of text block       */
             //   /*******************************************************/
             //   fProcessingAttributes = FALSE;  // no more attributes
             // } /* endif */
           } /* endif */

           if ( fFound && usTokens )
           {
             memcpy( pToken, &NewToken, sizeof(TOKENENTRY) );
             pToken->sAddInfo = pAttr[NewToken.sTokenid-sTags].BitFlags.AddInfo;

             if ( pTagAddInfo )
             {
               pToken->ClassId = pTagAddInfo[NewToken.sTokenid].ClassId;
               pToken->usOrgId = pTagAddInfo[NewToken.sTokenid].usFixedTagId;
             } /* endif */
             pToken++;
             usTokens--;
           } /* endif */
         } while ( fFound && usTokens ); /* enddo */
       } /* endif */
     }
     else
     {
       /*************************************************************/
       /* No tag in remaining data found                            */
       /*************************************************************/
       if ( fComplete )
       {
         /***********************************************************/
         /* Add remaining data as TEXT token                        */
         /***********************************************************/
         pToken->sTokenid = TEXT_TOKEN;      // text data
         pToken->pDataStringW = pszDataStart; // set start of data
         pToken->usLength = (USHORT)UTF16strlenCHAR(pszDataStart);
         pszData = pszDataStart + pToken->usLength;
         usTokens--;
         pToken++;
       }
       else
       {
         /*************************************************************/
         /* Process remaining data the next time ...                  */
         /*************************************************************/
         pszData = pszDataStart + UTF16strlenCHAR(pszDataStart);
       } /* endif */
     } /* endif */
   } /* endwhile */

   // if not processed completely or not last part of file
   // set pRest pointer to begin of last tag or (if no tags
   // are contained in data) to last linebreak.
   if ( !fOK )
   {
     // nothing was processed due to errors (memory allocation?)
     *ppRest = pszInput;
   }
   else if ( !usTokens ||              // end of token table reached
             !fComplete )              // or not last block of data  ???
   {
     // at least two tokens in buffer ....
     if ( (pToken != pTokBuf) && ( pToken != (pTokBuf+1)) )   // if tokens in buffer
     {
        /**************************************************************/
        /* There are tokens in the buffer, let's see how we have to   */
        /*  deal with them ...                                        */
        /**************************************************************/

        // go back to the last tag to allow tag attribute and whitespace
        // processing in the next tokenization block (unless this tag
        // is the first token in the token buffer or the remaining data
        // is longer than twice the max segment size )
        int iRemainLength = UTF16strlenCHAR(pToken[-1].pDataStringW + pToken[-1].usLength);
        if ( ( (pLastTag != pTokBuf) &&                    // more than one tag in buffer
               (iRemainLength < (2*MAX_SEGMENT_SIZE)) ) || // rest is smaller than 2 segments
             (usTokens == 0) )                             // token buffer overflow
        {
          pToken = pLastTag;
          TokStatus.usColPos = usTagColPos;
          *ppRest = pToken->pDataStringW;
        }
        else
        {
          // continue right behind the last recognized token
          *ppRest = pToken[-1].pDataStringW + pToken[-1].usLength;
          if ( (*pszData == EOS) && (*ppRest >= pszData) )
          {
            // no more data to follow, segment has been processed completely
            *ppRest = NULL;
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* The token buffer is empty, no tags were recognized ...     */
        /* In addition we came here if only one tag is recognized...  */
        /*                                                            */
        /* Go back to the last LF and add that data as a TEXT token   */
        /**************************************************************/
        PSZ_W pszAreaStart = NULL;
        pszEnd = pszData;
        pszData--;
        if ( pToken == pTokBuf )
        { 
           // no token in buffer
           pszAreaStart = pszInput;     // set start of data
        }
        else
        {
           // one token in buffer
           pszAreaStart = pToken[-1].pDataStringW + pToken[-1].usLength;
        } /* endif */            

        while ( pszData > pszAreaStart )
        {
          if ( *pszData == LF )
          {
             break;                  // leave while loop
          }
          else
          {
             pszData--;              // test previous character
          } /* endif */
        } /* endwhile */

        if ( pszData == pszInput )    // no linefeed found ???
        {
            *ppRest = pszEnd - 1;      // set pRest to end of data CHECK in DEBUG
        }
        else
        {
            *ppRest = pszData;         // set pRest to last linefeed
        } /* endif */
        pToken->sTokenid    = TEXT_TOKEN;   // text data
        pToken->pDataStringW = pszAreaStart;     // set start of data
        pToken->usLength    = (USHORT)(*ppRest - pszAreaStart);
        
        usTokens--;
        pToken++;
      } /* endif */
   }
   else
   {
      *ppRest = NULL;
   } /* endif */

   // add end-of-list token
   if ( fOK )
   {
     memset( pToken, 0, sizeof(TOKENENTRY) );
     pToken->usLength = 0;
     pToken->sTokenid = ENDOFLIST;

     *pusColPos = TokStatus.usColPos;
   } /* endif */

   /*******************************************************************/
   /* cleanup                                                         */
   /*******************************************************************/
   if ( TokStatus.pRetryStack != NULL )
   {
     UtlAlloc( (PVOID *)&TokStatus.pRetryStack, 0L, 0L, NOMSG );
   } /* endif */

   return( fOK );
} /* end of function TATagTokenize */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TAMatchTag
//------------------------------------------------------------------------------
// Function call:     TAMatchTag( PSZ *ppszData, PTREENODE pNode,
//                                PTOKENENTRY pToken, USHORT usFlag );
//------------------------------------------------------------------------------
// Description:       Look for a tag match in the input data.
//------------------------------------------------------------------------------
// Parameters:        PTOKSTATUS  pTokStatus  ptr to tokenize status area
//                    PSZ         *ppszData   pointer to current position
//                    PTREENODE   pNode       root node of node tree
//                    PTOKENENTRY pToken      ptr to token buffer for results
//                    USHORT      usFlag      flags for processing
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     a tag has been matched
//                    FALSE    no tag found in input data
//------------------------------------------------------------------------------
BOOL TAMatchTag
(
  PTOKSTATUS  pTokStatus,              // ptr to tokenize status area
  PSZ_W         *ppszData,               // pointer to current position in data
  PTREENODE   pNode,                   // root node of node tree for matching
  PTOKENENTRY pToken,                  // ptr to token buffer for results
  USHORT      usFlags                  // flags for processing
)
{
  PSZ_W         pszData;                 // ptr to current position
  PSZ_W         pszStartPos;             // ptr to data start position
  PSZ_W         pszTag;                  // ptr to start of attribute
  PSZ_W         pszEndDel;               // ptr for end delimiter processing
  CHAR_W        chTest;                  // character being tested
  USHORT      usMSElement = 0;         // number of current MS stack element
  USHORT      usRetryStackElement = 0; // number of current MS stack element
  BOOL        fQuote;                  // flag for quoted strings processing
  PTREENODE   pFirstNode;              // start of node tree
  ULONG       ulTagLen;                // length of tag
  USHORT      usColPos;                // current column position
  USHORT      usTagCol;                // tag start column position
  CHAR_W      chTemp;
  enum        PROCESS_STATES           // states during processing:
  {
    IN_ERROR_STATE,                    // an error occured
    TAG_MATCH_STATE,                   // text matches tag
    TAG_NOMATCH_STATE,                 // text does not match tag
    WORKING_STATE                      // during tag matching
  } ProcState = WORKING_STATE;

  /********************************************************************/
  /* Start at supplied data pointer                                   */
  /********************************************************************/
  pszData    = *ppszData;
  pFirstNode = pNode;
  usColPos   = pTokStatus->usColPos;

  /********************************************************************/
  /* Ignore any whitespace (whitespace table returns TRUE for         */
  /* LF, CR, TAB and BLANK)                                           */
  /********************************************************************/
  if ( usFlags & MATCH_SKIP_WHITESPACE )
  {
    CHAR_W c;
    while ( ((c = *pszData) <= 0x0ff) && fWhiteSpace[c] )
    {
      UPDATECOLPOS( *pszData, usColPos );
      pszData++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* Check for tag                                                    */
  /********************************************************************/
  pszStartPos = pszData;
  pszTag   = pszData;                  // remember start of tag
  usTagCol = usColPos;

  do
  {
    UPDATECOLPOS( *pszData, usColPos );
    chTest = ((chTest = *pszData++) <= 0x0ff ) ? chLookupW[chTest] : chTest;

    /****************************************************************/
    /* Look for character on current level of node list             */
    /****************************************************************/
    while ( pNode &&
            ((pNode->chNode != chTest) ||
             (pNode->usColPos && (pNode->usColPos != usColPos))) &&
            (pNode->chNode != pTokStatus->chMultSubst) &&
            (pNode->chNode != pTokStatus->chSingleSubst) &&
            (pNode->chNode != EOS) )
    {
       pNode = pNode->pNextChar;
    } /* endwhile */

    if ( pNode )
    {
      /****************************************************************/
      /* Check if the selected path is unique; if not add current     */
      /* position to retry stack to allow a retry at this position    */
      /* (but only if current position is not on stack already!)      */
      /****************************************************************/
      if ( !pNode->fUnique && pNode->pNextChar )
      {
        /**************************************************************/
        /* Check if current position is already on retry stack        */
        /**************************************************************/
        if ( (usRetryStackElement == 0) ||
             (pTokStatus->pRetryStack[usRetryStackElement-1].pNode !=
                         pNode->pNextChar ) )
        {
          /**************************************************************/
          /* Enlarge stack if stack limits have been reached            */
          /**************************************************************/
          if ( usRetryStackElement >= pTokStatus->usRetryStackSize )
          {
            if ( UtlAlloc( (PVOID *)&pTokStatus->pRetryStack,
                      (LONG)(pTokStatus->usRetryStackSize * sizeof(RETRYSTACK)),
                      (LONG)((pTokStatus->usRetryStackSize + RETRYSTACKSIZE ) *
                                                            sizeof(RETRYSTACK)),
                      (USHORT)(( pTokStatus->fMsg ) ? ERROR_STORAGE : NOMSG) ) )
            {
              pTokStatus->usRetryStackSize += RETRYSTACKSIZE;

            }
            else
            {
              ProcState = IN_ERROR_STATE;
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* push retry position on retry stack                         */
          /**************************************************************/
          if ( ProcState != IN_ERROR_STATE )
          {
            pTokStatus->pRetryStack[usRetryStackElement].pNode = pNode->pNextChar;
            pTokStatus->pRetryStack[usRetryStackElement].pszData = pszData - 1;
            pTokStatus->pRetryStack[usRetryStackElement].pszTag = pszTag;
            usRetryStackElement++;
          } /* endif */
        } /* endif */
      } /* endif */

      /**************************************************************/
      /* Character has been located on curent level of node tree    */
      /**************************************************************/
      if ( pNode->chNode == pTokStatus->chMultSubst )
      {
        /**************************************************************/
        /* Handle strings enclosed in quotes                          */
        /**************************************************************/
        if ( ((usFlags & MATCH_QUOTED_STRINGS) || pNode->fTransInfo)
                          &&
             ((chTest == QUOTE) || (chTest == DBLQUOTE)) )
        {
          CHAR_W chQuote = chTest;

          fQuote = TRUE;
          while (  fQuote && ((chTest = ((chTest = *pszData) <= 0x0ff ) ? chLookupW[chTest] : chTest) != 0))
          //while (chTest = chLookupW[(UCHAR)*pszData]) && fQuote )
          {
            if ( chTest == chQuote )
            {
              /********************************************/
              /* if two consecutive QUOTES we are dealing */
              /* with inline quote else we are done..     */
              /********************************************/
              chTemp = ((chTemp = *(pszData+1)) <= 0x0ff ) ? chLookupW[chTemp] : chTemp;
              if ( chTemp != chQuote )
              //if ( chLookupW[(UCHAR)*(pszData+1)] != chQuote )
              {
                fQuote = FALSE;
              }
              else
              {
                pszData += 2;
                usColPos += 2;
              } /* endif */
            }
            else
            {
              UPDATECOLPOS( *pszData, usColPos );
              pszData++;
            } /* endif */
          } /* endwhile */

          if ( chTest != EOS )
          {
            UPDATECOLPOS( *pszData, usColPos );
            pszData++;                   // point to next active charact.
            UPDATECOLPOS( *pszData, usColPos );
            chTest = ((chTest = *pszData++) <= 0x0ff ) ? chLookupW[chTest] : chTest;
            //chTest = chLookupW[(UCHAR)*pszData++]; // and use this character for testing
          }
          else
          {
            pszData--;                // closing quote not contained in data
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Check for tag end delimiter or for tag length              */
        /**************************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;  //@@ DEBUG here!
          if ( ulTagLen == pNode->usLength )
          {
            /********************************************************/
            /* End of length-terminated tag has been reached        */
            /********************************************************/
            if ( pNode->pNextLevel->chNode == EOS )
            {
              ProcState = TAG_MATCH_STATE;
              pToken->sTokenid    = pNode->sTokenID;
              pToken->pDataStringW = pszTag;
              pszData--;
              usColPos--;
              pToken->usLength    = (USHORT)(pszData - pszTag);
            }
            else
            {
              /********************************************************/
              /* end of tag but not end of pattern string ...         */
              /********************************************************/
              ProcState = TAG_NOMATCH_STATE;
              pszData--;
              usColPos--;
            } /* endif */
          }
          else if ( ulTagLen > pNode->usLength )
          {
            /********************************************************/
            /* Tag will not match as length has been                */
            /* exceeded                                             */
            /********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Check for tag end when pattern string ends               */
          /************************************************************/
          if ( pNode->pNextLevel->chNode == EOS )
          {
            pszEndDel = pNode->pszEndDel;
            while ( *pszEndDel )
            {
              if ( (CHAR_W)*pszEndDel == chTest )
              {
                break;
              }
              else
              {
                pszEndDel++;
              } /* endif */
            } /* endwhile */
            if ( *pszEndDel )
            {
              ProcState           = TAG_MATCH_STATE;
              pToken->sTokenid    = pNode->sTokenID;
              pToken->pDataStringW = pszTag;
              pToken->usLength    = (USHORT)(pszData - pszTag);
            } /* endif */
          } /* endif */
        } /* endif */

        if (ProcState == WORKING_STATE)
        {
          if ( (pNode->pNextLevel->chNode == chTest) )
          {
            /************************************************/
            /* Push current node to multiple substitution   */
            /* stack and follow the node path               */
            /************************************************/
            if ( usMSElement <  pTokStatus->usMSStackSize )
            {
              pTokStatus->pMSStack[usMSElement].pNode = pNode;
              usMSElement++;
            } /* endif */

            pNode = pNode->pNextLevel;
            pszData--;
            usColPos--;
          }
          else
          {
            /**********************************************************/
            /* check all further multiple substitution tags if they   */
            /* may be a matching starting point...                    */
            /**********************************************************/
            PTREENODE   pTempNode  = pNode;
            while ( pTempNode->pNextChar )
            {
              pTempNode = pTempNode->pNextChar;
              if ( pTempNode->pNextLevel &&
                   (pTempNode->pNextLevel->chNode == chTest) )
              {
                /************************************************/
                /* Push current node to multiple substitution   */
                /* stack and follow the node path               */
                /************************************************/
                if ( usMSElement <  pTokStatus->usMSStackSize )
                {
                  pTokStatus->pMSStack[usMSElement].pNode = pNode;
                  usMSElement++;
                } /* endif */

                pNode = pTempNode->pNextLevel;
                pszData--;
                usColPos--;
                break;
              } /* endif */
            } /* endwhile */
          } /* endif */
        } /* endif */
      }
      else if ( pNode->chNode == EOS )
      {
        /**************************************************************/
        /* Pattern string has ended, check if current tag has ended   */
        /* also                                                       */
        /**************************************************************/

        /************************************************/
        /* Check for tag end delimiter or for tag length*/
        /************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;
          if ( ulTagLen == pNode->usLength )
          {
            /********************************************************/
            /* End of length terminated tag has been reached        */
            /********************************************************/
            ProcState = TAG_MATCH_STATE;
            pToken->sTokenid    = pNode->sTokenID;
            pToken->pDataStringW = pszTag;
            pszData--;
            usColPos--;
            pToken->usLength    = (USHORT)(pszData - pszTag);
          }
          else
          {
            /********************************************************/
            /* Attribute will not match as length does not match    */
            /********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        }
        else
        {
          pszEndDel = pNode->pszEndDel;
          while ( *pszEndDel )
          {
            if ( (CHAR_W)*pszEndDel == chTest )
            {
              break;                   // valid end delimiter, leave loop
            }
            else
            {
              pszEndDel++;
            } /* endif */
          } /* endwhile */

          /************************************************/
          /* If found, fill-in attribute data in token    */
          /* otherwise check redo stack, redo or end                  */
          /* loop depending on stack                                  */
          /************************************************/
          if ( *pszEndDel )
          {
            ProcState = TAG_MATCH_STATE;
            pToken->sTokenid    = pNode->sTokenID;
            pToken->pDataStringW = pszTag;
            pToken->usLength    = (USHORT)(pszData - pszTag);
          }
          else
          {
            /****************************************************/
            /* Mismatch of currently tested character:          */
            /*   If multiple-substitution-stack is active       */
            /*      reset node position to last element on stack*/
            /*   else                                           */
            /*      leave attribute recognition loop            */
            /****************************************************/
            if ( usMSElement != 0 )                              /* @KWT0034A */
            {                                                    /* @KWT0034A */
              usMSElement--;                                     /* @KWT0034A */
              pNode = pTokStatus->pMSStack[usMSElement].pNode;   /* @KWT0034A */
              pszData--;                                         /* @KWT0034A */
            }                                                    /* @KWT0034A */
            else                                                 /* @KWT0034A */
            {                                                    /* @KWT0034A */
              /************************************************/ /* @KWT0034M */
              /* Leave loop                                   */ /* @KWT0034M */
              /************************************************/ /* @KWT0034M */
              ProcState = TAG_NOMATCH_STATE;                     /* @KWT0034M */
              pszData--;                                         /* @KWT0034M */
              usColPos--;                                        /* @KWT0034M */
            } /* endif */                                        /* @KWT0034A */
          } /* endif */
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* Check if tag end delimiter has been reached or tag length */
        /* has been exceeded.                                         */
        /**************************************************************/
        if ( pNode->usLength )
        {
          ulTagLen = (pszData - pszTag) - 1;
          if ( ulTagLen >= pNode->usLength )
          {
            /**********************************************************/
            /* Tag is not at it's end but tag length has been         */
            /* exceeded                                               */
            /**********************************************************/
            ProcState = TAG_NOMATCH_STATE;
            pszData--;
            usColPos--;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /* Follow current path if no mismatch yet                     */
        /**************************************************************/
        if ( ProcState != TAG_NOMATCH_STATE )
        {
          if ( pNode->pNextLevel )
          {
            /************************************************************/
            /* The are more levels to follow ...                        */
            /************************************************************/
            pNode = pNode->pNextLevel;
          }
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************/
      /* Mismatch of currently tested character:          */
      /*   If multiple-substitution-stack is active       */
      /*      reset node position to last element on stack*/
      /*   else                                           */
      /*      leave attribute recognition loop            */
      /****************************************************/
      if ( usMSElement != 0 )
      {
        usMSElement--;
        pNode = pTokStatus->pMSStack[usMSElement].pNode;
        pszData--;
      }
      else
      {
        /**************************************************/
        /* Leave loop                                      */
        /**************************************************/
        ProcState = TAG_NOMATCH_STATE;
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* Attribute mismatch detected, check the retry stack leave       */
    /* loop if stack is empty else pop stack entry and retry          */
    /******************************************************************/
    if ( ProcState == TAG_NOMATCH_STATE )
    {
      if ( usRetryStackElement )
      {
        usRetryStackElement--;
        pNode   = pTokStatus->pRetryStack[usRetryStackElement].pNode;
        pszData = pTokStatus->pRetryStack[usRetryStackElement].pszData;
        pszTag  = pTokStatus->pRetryStack[usRetryStackElement].pszTag;

        usMSElement = 0;               // reset multiiple-substitution-stack

        ProcState = WORKING_STATE;

        if ( pszData == pszStartPos )
        {
          chTest = 0x20;               // dummy value (not equal to EOS!)
        }
        else
        {
          chTest = ((chTest = *(pszData-1)) <= 0x0ff ) ? chLookupW[chTest] : chTest;
          //chTest = chLookupW[(UCHAR)(pszData[-1])];// reset to old value
        } /* endif */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* continue search if we are in WORKING_STATE and already at end  */
    /* of passed data, but the passed data are a complete record.     */
    /******************************************************************/
    if ( (!chTest) && (usFlags & MATCH_DATA_COMPLETE) &&         /* @KWT0010A */
         (ProcState == WORKING_STATE)  )                         /* @KWT0010A */
    {                                                            /* @KWT0010A */
      /************************************************/         /* @KWT0010A */
      /* reset state to no tag match - dirty work of  */         /* @KWT0010A */
      /* resetting will be done below ....            */         /* @KWT0010A */
      /************************************************/         /* @KWT0010A */
      ProcState  = TAG_NOMATCH_STATE;                            /* @KWT0010A */
      chTest     = *pszTag;    /* reset to try again  */         /* @KWT0010A */
    } /* endif */                                                /* @KWT0010A */
    /******************************************************************/
    /* Continue search if we are in TAG_NOMATCH_STATE and             */
    /* MATCH_SKIP_DATA has been set                                   */
    /******************************************************************/
    if ( (ProcState == TAG_NOMATCH_STATE) && (usFlags & MATCH_SKIP_DATA) )
    {
      ProcState  = WORKING_STATE;      // reset state to "Working"
      usColPos   = usTagCol;           // restore column position
      UPDATECOLPOS( *pszTag, usColPos );

      /****************************************************************/
      /* Check for DBCS character and avoid restart of search for     */
      /* tags on second DBCS characters                               */
      /****************************************************************/
     // if ( (_dbcs_cp == DBCS_CP) && (isdbcs1(*pszTag) == DBCS_1ST) )
     // {
     //   // skip second DBCS character ...
     //   pszTag++;                        // skip DBCS 2nd character
     // } /* endif */
      pszTag++;                        // restart at next character
      usTagCol   = usColPos;
      pszData    = pszTag;             // set data pointer
      pNode      = pFirstNode;         // restart at begin of node tree
    } /* endif */
  } while( (ProcState == WORKING_STATE) && (chTest != EOS) );

  if ( ProcState == TAG_MATCH_STATE )
  {
    /******************************************************************/
    /* Avoid split of CRLF combination at end of tags                 */
    /******************************************************************/
    if ( (pToken->pDataStringW[pToken->usLength-1] == CR ) &&
         (*pszData == LF) )
    {
      pszData++;
      usColPos = 0;
      pToken->usLength++;
    } /* endif */

    /******************************************************************/
    /* Update caller's data pointer and column position               */
    /******************************************************************/
    *ppszData = pszData;
    pTokStatus->usColPos = usColPos;
    pTokStatus->usStartColPos = usTagCol;

  } /* endif */

  return( (ProcState == TAG_MATCH_STATE) );
} /* end of MatchTag */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateTagTree
//------------------------------------------------------------------------------
// Function call:     TACreateTagTree( PTAGTABLE pTagTable, CHAR chMultSubst,
//                                     CHAR chSingleSubst, BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Create a tag tree required for TATagTokenize
//------------------------------------------------------------------------------
// Parameters:        PTAGTABLE  pTagTable     ptr to a loaded tag table
//                    CHAR       chMultSubst   muliple substitution character
//                    CHAR       chSingleSubst single substitution character
//                    BOOL       fMsg          show error messages flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of tag tree failed
//                    other       ptr to created tag tree
//------------------------------------------------------------------------------
PNODEAREA TACreateTagTree
(
  PLOADEDTABLE pLoadedTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
)
{
   PTAG        pTag;                   // pointer to structure of active tag
   PSZ_W       pTagNamesW;              // pointer to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI, usJ;               // general loop index
   PTBTAGENTRY pExtractedTags = NULL;  // ptr to tags extracted from tag table
   PNODEAREA   pNodeArea = NULL;       // area containing node tree
   USHORT      usEntry = 0;            // index into extracted tags array

   // address tag table lists
   pByte = (PBYTE) pTagTable;
   pTag = (PTAG) ( pByte + pTagTable->stFixTag.uOffset);
   //pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);
   pTagNamesW = pLoadedTable->pTagNamesW;
   ulNoOfTags = pTagTable->stFixTag.uNumber;
   for ( usI = 0; usI < 27; usI++ )
   {
      ulNoOfTags += pTagTable->stTagIndex[usI].uNumber;
   } /* endfor */
   ulNoOfTags += pTagTable->stVarStartTag.uNumber;

   // allocate tag entry array
   fOK = UtlAlloc( (PVOID *)&pExtractedTags, 0L,
                   (LONG) (ulNoOfTags * sizeof(TBTAGENTRY)),
                   ERROR_STORAGE );

   // extract tags from tag table
   if ( fOK )
   {
      for ( usI = 0; usI < pTagTable->stFixTag.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag    = pTagNamesW + pTag->uTagnameOffs;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pTag->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = pTag->usPosition;
         pExtractedTags[usEntry].usLength  = pTag->usLength;
         pExtractedTags[usEntry].fAttr     = pTag->fAttr;
         pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
         pExtractedTags[usEntry].sID       = usEntry;
         usEntry++;
         pTag++;
      } /* endfor */

      for ( usJ = 0; usJ < 27; usJ++ )
      {
         pTag = (PTAG) ( pByte + pTagTable->stTagIndex[usJ].uOffset);
         for ( usI = 0; usI < pTagTable->stTagIndex[usJ].uNumber; usI++ )
         {
            pExtractedTags[usEntry].pszTag   = pTagNamesW + pTag->uTagnameOffs;
            pExtractedTags[usEntry].pszEndDel= pTagNamesW + pTag->uEndDelimOffs;
            pExtractedTags[usEntry].usColPos = pTag->usPosition;
            pExtractedTags[usEntry].usLength = pTag->usLength;
            pExtractedTags[usEntry].fAttr    = pTag->fAttr;
            pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
            pExtractedTags[usEntry].sID      = usEntry;
            pTag++;
            usEntry++;
         } /* endfor */
      } /* endfor */

      pTag = (PTAG) ( pByte + pTagTable->stVarStartTag.uOffset);
      for ( usI = 0; usI < pTagTable->stVarStartTag.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag    = pTagNamesW + pTag->uTagnameOffs;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pTag->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = pTag->usPosition;
         pExtractedTags[usEntry].usLength  = pTag->usLength;
         pExtractedTags[usEntry].fAttr     = pTag->fAttr;
         pExtractedTags[usEntry].fTranslInfo = pTag->BitFlags.fTranslate;
         pExtractedTags[usEntry].sID       = usEntry;
         pTag++;
         usEntry++;
      } /* endfor */

      pNodeArea = TACreateNodeTree( pExtractedTags, ulNoOfTags,
                                     chMultSubst, chSingleSubst,
                                     fMsg );
   } /* endif */


   if ( pExtractedTags ) UtlAlloc( (PVOID *)&pExtractedTags, 0L, 0L, NOMSG );

   return ( pNodeArea );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateAttrTree
//------------------------------------------------------------------------------
// Function call:     TACreateAttrTree( PTAGTABLE pTagTable, CHAR chMultSubst,
//                                     CHAR chSingleSubst, BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Create an attribute tree required for TATagTokenize
//------------------------------------------------------------------------------
// Parameters:        PTAGTABLE pTagTable      ptr to a loaded tag table
//                    CHAR       chMultSubst   muliple substitution character
//                    CHAR       chSingleSubst single substitution character
//                    BOOL       fMsg          show error messages flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of attribute tree failed
//                    other       ptr to created attribute tree
//------------------------------------------------------------------------------
PNODEAREA TACreateAttrTree
(
  PLOADEDTABLE pLoadedTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
)
{
   PATTRIBUTE  pAttr;                  // ptr to structure of active attribute
   PSZ_W       pTagNamesW;              // ptr to start of tagnames
   PBYTE       pByte;                  // helper pointer
   ULONG       ulNoOfTags;             // number of tags to process
   BOOL        fOK = TRUE;             // internal OK flag
   USHORT      usI, usJ;               // general loop index
   USHORT      usEntry;                // current entry in pExtractedTags
   PTBTAGENTRY pExtractedTags = NULL;  // ptr to tags extracted from tag table
   SHORT       sID;                    // IDs of attributes
   ULONG       ulLength;               // length of attribute array in bytes
   PNODEAREA   pNodeArea = NULL;       // area containing node tree

   // address tag table lists
   pByte = (PBYTE) pTagTable;
//   pTagNames = (PSZ)( pByte +  pTagTable-> uTagNames);

   pTagNamesW = pLoadedTable->pTagNamesW;
   ulNoOfTags = pTagTable->stAttribute.uNumber;
   for ( usI = 0; usI < 27; usI++ )
   {
      ulNoOfTags += pTagTable->stAttributeIndex[usI].uNumber;
   } /* endfor */
   sID = pTagTable->uNumTags;

   // allocate attribute entry array
   ulLength = ulNoOfTags * sizeof(TBTAGENTRY);
   fOK = UtlAlloc( (PVOID *)&pExtractedTags, 0L,
                   (LONG) get_max( ulLength, MIN_ALLOC ),
                   ERROR_STORAGE);

   // extract fixed attributes from tag table
   usEntry = 0;                      // start with first entry in table
   sID = pTagTable->uNumTags;          // set ID of first attribute
   pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttribute.uOffset);
   if ( fOK )
   {
      for ( usI = 0; usI < pTagTable->stAttribute.uNumber; usI++ )
      {
         pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
         pExtractedTags[usEntry].sID    = sID++;
         pExtractedTags[usEntry].pszEndDel = pTagNamesW + pAttr->uEndDelimOffs;
         pExtractedTags[usEntry].usColPos  = 0;
         pExtractedTags[usEntry].usLength  = pAttr->usLength;
         pExtractedTags[usEntry].fAttr     = FALSE;
         pAttr++;
         usEntry++;
      } /* endfor */
   } /* endif */

   // extract variable attributes from tag table
   if ( fOK )
   {
      for ( usJ = 0; usJ < 27; usJ++ )
      {
         pAttr = (PATTRIBUTE) ( pByte + pTagTable->stAttributeIndex[usJ].uOffset);
         for ( usI = 0; usI < pTagTable->stAttributeIndex[usJ].uNumber; usI++ )
         {
            pExtractedTags[usEntry].pszTag = pAttr->uStringOffs + pTagNamesW;
            pExtractedTags[usEntry].sID    = sID++;
            pExtractedTags[usEntry].pszEndDel = pTagNamesW + pAttr->uEndDelimOffs;
            pExtractedTags[usEntry].usColPos  = 0;
            pExtractedTags[usEntry].usLength  = pAttr->usLength;
            pExtractedTags[usEntry].fAttr     = FALSE;
            pAttr++;
            usEntry++;
         } /* endfor */
      } /* endfor */
      pNodeArea = TACreateNodeTree( pExtractedTags, ulNoOfTags,
                                     chMultSubst, chSingleSubst,
                                     fMsg );
   } /* endif */


   if ( fOK )
   {
      UtlAlloc( (PVOID *)&pExtractedTags, 0L, 0L, NOMSG );
   } /* endif */

   return ( pNodeArea );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TACreateNodeTree
//------------------------------------------------------------------------------
// Function call:     TACreateNodeTree( PTBTAGENTRY pTags, USHORT ulNoOfTags,
//                                      CHAR chMultSubst, CHAR chSingleSubst,
//                                      BOOL fMsg );
//------------------------------------------------------------------------------
// Description:       Creates a node tree for a given list of tags or
//                    attributes.
//------------------------------------------------------------------------------
// Parameters:        PTBTAGENTRY pTags         ptr to list of tags
//                    USHORT      ulNoOfTags    number of tags in list
//                    CHAR        chMultSubst   muliple substitution character
//                    CHAR        chSingleSubst single substitution character
//                    BOOL        fMsg          do error message handling flag
//------------------------------------------------------------------------------
// Returncode type:   PNODEAREA
//------------------------------------------------------------------------------
// Returncodes:       NULL        create of node tree failed
//                    other       ptr to created node tree
//------------------------------------------------------------------------------
PNODEAREA TACreateNodeTree
(
  PTBTAGENTRY pTags,                   // ptr to list of tags
  ULONG       ulNoOfTags,              // number of tags in list
  CHAR_W        chMultSubst,             // muliple substitution character
  CHAR_W        chSingleSubst,           // single substitution character
  BOOL        fMsg                     // do error message handling flag
)
{
   BOOL        fOK = TRUE;             // internal OK flag
   CHAR_W      chCurChar;              // currently processed character
   PTREENODE   pNode;                  // currently processed node
   PTREENODE   pTempNode;              // temporary node pointer
   PTREENODE   pLastNode;              // last node processed
   PTREENODE   pPrevNode;              // previous node of current node
   PTREENODE   pNextNode = NULL;       // next node of current node
   PTREENODE   pFoundNode;             // found node
   PTREENODE   pFirstNodeOnLevel;      // first node on current level
   USHORT      usCurPos;               // currently tested character position
   USHORT      usCharPos;              // position of character to test
   PSZ_W         pszTag;               // pointer into current tag
   PNODEAREA   pNewArea = NULL;        // ptr to newly allocated node area
   PNODEAREA   pNodeArea = NULL;       // ptr to current node area
   PNODEAREA   pRootArea = NULL;       // ptr to root node area
   ULONG       ulAreaSize = 0;
   USHORT      usTag;                  // number (=ID) of current tag
   USHORT      usCurCharIndex;         // index of current character
   USHORT      usNodeCharIndex;        // index of current node character
   SHORT       sCompare;               // result of character comparism
   BOOL        fInsert;                // insert-a-new-node flag


   pNewArea = NULL;
   pNodeArea = NULL;
   usTag = 0;

  /********************************************************************/
  /* Allocate area for node tree (initial size = 3 times number of    */
  /* tags)                                                            */
  /********************************************************************/
  if ( fOK )
  {
    ulAreaSize = get_min( (LONG)(ulNoOfTags * 3 * sizeof(TREENODE)) +
                             sizeof(NODEAREA), MAX_ALLOC );
    fOK = UtlAlloc( (PVOID *)&pNodeArea, 0L, ulAreaSize,
                            (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
    if ( fOK )
    {
      pRootArea = pNodeArea;
      pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                               sizeof(TREENODE);
      pNodeArea->pFreeNode   = &(pNodeArea->Nodes[0]);
      pRootArea->pRootNode   = NULL;
    }
    else
    {
      pRootArea = NULL;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Loop over all tags and add tags to node tree                     */
  /********************************************************************/
  while ( fOK && ulNoOfTags )
  {
    /******************************************************************/
    /* Add current tag to node tree                                   */
    /******************************************************************/
    pszTag     = pTags->pszTag;
    usCharPos  = 0;
    pNode      = pRootArea->pRootNode;
    pLastNode  = NULL;

    do
    {
      chCurChar = *pszTag++;
      usCurPos  = usCharPos++;

      if ( pNode == NULL )
      {
        /**************************************************************/
        /* No nodes on this level yet, create new node on this level  */
        /**************************************************************/

        /************************************************************/
        /* Get a free node                                          */
        /************************************************************/
        if ( pNodeArea->usFreeNodes )
        {
          pNode = pNodeArea->pFreeNode++;
          pNodeArea->usFreeNodes--;
        }
        else
        {
          pNewArea = NULL;
          fOK = UtlAlloc( (PVOID *)&pNewArea, 0L, ulAreaSize,
                          (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
          if ( fOK )
          {
            pNodeArea->pNext = pNewArea;
            pNodeArea = pNewArea;
            pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                                     sizeof(TREENODE);
            pNodeArea->pFreeNode   = &pNodeArea->Nodes[0];
            pNode = pNodeArea->pFreeNode++;
            pNodeArea->usFreeNodes--;
          } /* endif */
        } /* endif */

        /************************************************************/
        /* fill-in node data                                        */
        /************************************************************/
        pNode->chNode     = chCurChar;
        pNode->chPos      = (CHAR_W)usCurPos;
        pNode->pNextLevel = NULL;
        pNode->pNextChar  = NULL;
        pNode->pszEndDel  = pTags->pszEndDel;
        pNode->usLength   = pTags->usLength;
        pNode->usColPos   = ( usCurPos == 0 ) ? pTags->usColPos : 0;
        pNode->fAttr      = pTags->fAttr;
        pNode->fTransInfo = pTags->fTranslInfo;
        pNode->sTokenID   = pTags->sID;
        pNode->fUnique    = TRUE;

        /************************************************************/
        /* Anchor node                                              */
        /************************************************************/
        if ( pRootArea->pRootNode == NULL )
        {
          pRootArea->pRootNode = pNode;
        } /* endif */

        if ( pLastNode )
        {
          pLastNode->pNextLevel = pNode;
        } /* endif */

        /************************************************************/
        /* Use current node as last node for next operation         */
        /************************************************************/
        pLastNode = pNode;
        pNode     = NULL;
      }
      else
      {
        /**************************************************************/
        /* Get compare table index for current character              */
        /**************************************************************/
        if ( chCurChar == chMultSubst )
        {
          usCurCharIndex = 0;
        }
        else if ( chCurChar == chSingleSubst )
        {
          usCurCharIndex = 1;
        }
        else if ( chCurChar == EOS )
        {
          usCurCharIndex = 2;
        }
        else if ( (pTags->usColPos != 0) && (usCurPos == 0) )
        {
          usCurCharIndex = 3;
        }
        else
        {
          usCurCharIndex = 4;
        } /* endif */

        /**************************************************************/
        /*                                                            */
        /**************************************************************/
        pFirstNodeOnLevel = pNode;
        pPrevNode  = NULL;             // no previous node yet
        pFoundNode = NULL;             // no matching node found yet
        fInsert    = FALSE;            // nothing to insert yet
        while ( !fInsert && !pFoundNode )
        {
          /************************************************************/
          /* Compare tag character with character of current node     */
          /* order of characters is:                                  */
          /* 1. alpha with column position                            */
          /* 2. alpha                                                 */
          /* 3. single substitution character                         */
          /* 4. multiple substitution character                       */
          /* 5. end of string delimiter (EOS)                         */
          /*                                                          */
          /* The following table is used to do the comparism:         */
          /*                                                          */
          /*               Node Character                             */
          /*                %     ?    EOS   Pos   other              */
          /*         ----�-----�-----�-----�-----�-------+            */
          /*            %   >     >     <     >      >                */
          /*         ----�-----�-----�-----�-----�-------+            */
          /* current    ?   <     >     <     >      >                */
          /*  char  -----�-----�-----�-----�-----�-------+            */
          /*          EOS   >     >     >     >      >                */
          /*        -----�-----�-----�-----�-----�-------+            */
          /*          Pos   <     <     <     =      <                */
          /*        -----�-----�-----�-----�-----�-------+            */
          /*        Other   <     <     <     >      =                */
          /*        -----�-----�-----�-----�-----�-------+            */
          /************************************************************/


          /**************************************************************/
          /* Get compare table index for node character                 */
          /**************************************************************/
          if ( pNode->chNode == chMultSubst )
          {
            usNodeCharIndex = 0;
          }
          else if ( pNode->chNode == chSingleSubst )
          {
            usNodeCharIndex = 1;
          }
          else if ( pNode->chNode == EOS )
          {
            usNodeCharIndex = 2;
          }
          else if ( pNode->usColPos != 0 )
          {
            usNodeCharIndex = 3;
          }
          else
          {
            usNodeCharIndex = 4;
          } /* endif */

          sCompare = asCharCompare[ usCurCharIndex ][ usNodeCharIndex ];
          if ( sCompare == 0 )
          {
            if ( chCurChar > pNode->chNode )
            {
              sCompare = 1;
            }
            else if ( chCurChar < pNode->chNode )
            {
              sCompare = -1;
            }
            else if ( (pTags->usColPos != pNode->usColPos) && (usCurPos == 0) )
            {
              sCompare = 1;
            } /* endif */
          } /* endif */

          if ( sCompare > 0 )
          {
            if ( pNode->pNextChar )
            {
              /********************************************************/
              /* Continue search with next node on this level         */
              /********************************************************/
              pPrevNode = pNode;
              pNode     = pNode->pNextChar;
            }
            else
            {
              /********************************************************/
              /* No nodes to follow on this level, add new node       */
              /* behind last node of this level                       */
              /********************************************************/
              pPrevNode = pNode;
              pNextNode = NULL;
              fInsert   = TRUE;
            } /* endif */
          }
          else if ( sCompare == 0 )
          {
            /**********************************************************/
            /* Follow the given path ...                              */
            /**********************************************************/
            pFoundNode = pNode;
            pLastNode  = pFoundNode;
            pNode      = pFoundNode->pNextLevel;
          }
          else
          {
            /**********************************************************/
            /* Insert new node right before current node              */
            /**********************************************************/
            pNextNode = pNode;
            fInsert   = TRUE;
          } /* endif */
        } /* endwhile */

        /****************************************************************/
        /* Follow found path or add new node on this level              */
        /****************************************************************/
        if ( fInsert )
        {
          /**************************************************************/
          /* Get a free node                                            */
          /**************************************************************/
          if ( pNodeArea->usFreeNodes )
          {
            pNode = pNodeArea->pFreeNode++;
            pNodeArea->usFreeNodes--;
          }
          else
          {
            pNewArea = NULL;
            fOK = UtlAlloc( (PVOID *)&pNewArea, 0L, ulAreaSize,
                            (USHORT)(( fMsg ) ? ERROR_STORAGE : NOMSG) );
            if ( fOK )
            {
              pNodeArea->pNext = pNewArea;
              pNodeArea = pNewArea;
              pNodeArea->usFreeNodes = ((USHORT)ulAreaSize - sizeof(NODEAREA)) /
                                       sizeof(TREENODE);
              pNodeArea->pFreeNode   = &pNodeArea->Nodes[0];
              pNode = pNodeArea->pFreeNode++;
              pNodeArea->usFreeNodes--;
            } /* endif */
          } /* endif */

          /**************************************************************/
          /* fill-in node data                                          */
          /**************************************************************/
          pNode->chNode     = chCurChar;
          pNode->chPos      = (CHAR_W)usCurPos;
          pNode->pNextLevel = NULL;
          pNode->pNextChar  = pNextNode;
          pNode->pszEndDel  = pTags->pszEndDel;
          pNode->usLength   = pTags->usLength;
          pNode->usColPos   = ( usCurPos == 0 ) ? pTags->usColPos : 0;
          pNode->fAttr      = pTags->fAttr;
          pNode->fTransInfo = pTags->fTranslInfo;
          pNode->sTokenID   = pTags->sID;
          pNode->fUnique    = TRUE;

          /**************************************************************/
          /* Anchor node                                                */
          /**************************************************************/
          if ( pPrevNode )
          {
            pPrevNode->pNextChar = pNode;
          }
          else if ( pLastNode )
          {
            pLastNode->pNextLevel = pNode;
          }
          else
          {
            pRootArea->pRootNode = pNode;
          } /* endif */
          if ( pPrevNode == NULL )
          {
            pFirstNodeOnLevel = pNode;
          } /* endif */

          /************************************************************/
          /* Update unique flags of nodes on this level               */
          /*                                                          */
          /* Nodes are not unique if either there are nodes with      */
          /* multiple substitution, single substitution or EOS        */
          /* characters or if the same character is more than once in */
          /* the node list of this level                              */
          /************************************************************/
          pTempNode = pFirstNodeOnLevel;
          while ( pTempNode )
          {
            /**********************************************************/
            /* Check remaining nodes following the current node       */
            /**********************************************************/
            pNextNode = pTempNode->pNextChar;
            while ( pNextNode && pTempNode->fUnique )
            {
              if ( (pNextNode->chNode == EOS)           ||
                   (pNextNode->chNode == chSingleSubst) ||
                   (pNextNode->chNode == chMultSubst)   ||
                   (pNextNode->chNode == pTempNode->chNode) )
              {
                pTempNode->fUnique = FALSE;
              }
              else
              {
                pNextNode = pNextNode->pNextChar;
              } /* endif */
            } /* endwhile */
            pTempNode = pTempNode->pNextChar;
          } /* endwhile */

          /**************************************************************/
          /* Use current node as last node for next operation           */
          /**************************************************************/
          pLastNode = pNode;
          pNode     = NULL;
        } /* endif */
      } /* endif */
    } while ( chCurChar ); /* enddo */

    /******************************************************************/
    /* Continue with next tag                                         */
    /******************************************************************/
    pTags++;
    ulNoOfTags--;
    usTag++;
  } /* endwhile */

  // free allocated memory in case of errors
  if ( !fOK && pRootArea )
  {
    do
    {
      pNodeArea = pRootArea;
      pRootArea = pNodeArea->pNext;
      UtlAlloc( (PVOID *)&pNodeArea, 0L, 0L, NOMSG );
    } while ( pRootArea ); /* enddo */
  } /* endif */

  return( pRootArea );
}


// Create SO/SI Protect table
USHORT TASoSiProtectTable
(
  PSZ_W              pszSegment,         // ptr to text of segment being processed
  PSTARTSTOP       *ppStartStop        // ptr to caller's start/stop table ptr
)
{
  USHORT           usRC = NO_ERROR;    // function return code
  PSTARTSTOP       pStartStop = *ppStartStop;  // ptr to new start/stop table
  PSTARTSTOP       pCurrent = NULL;    // ptr for start/stop table processing
  PSTARTSTOP       pAct, pActNew;      // ptr for start/stop table processing
  PSZ_W              pText = pszSegment; // ptr to segment
  USHORT           usCountSOSI = 0;
  CHAR_W           c;

  /********************************************************************/
  /* check if something todo                                          */
  /********************************************************************/
  while ( (c = *pText++ ) != NULC )
  {
    if ((c == SO ) || (c== SI) )
    {
      // a text string containing an SO/SI is split at that character into
      // two additional units, the SO/SI and the rest of the text string
      usCountSOSI += 2;
    } /* endif */
  } /* endwhile */



  if ( usCountSOSI )
  {
    pAct = pStartStop;
    while ( pAct->usStop )
    {
      pAct++;
      usCountSOSI++;
    } /* endwhile */
    usCountSOSI++;

    LONG size = (usCountSOSI * sizeof(STARTSTOP)) > MIN_ALLOC ? (usCountSOSI * sizeof(STARTSTOP)) : MIN_ALLOC;
    if ( !UtlAlloc( (PVOID *)&pCurrent, 0L, size,  NOMSG ))
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */


    if ( !usRC )
    {
      pAct = pStartStop;
      pActNew = pCurrent;
      while ( pAct->usStop )
      {
        memcpy( pActNew, pAct, sizeof(STARTSTOP));
        if ( pAct->usType != PROTECTED_CHAR )
        {
          CHAR_W c, b;
          SHORT sEnd;
          USHORT usType;
          pText = pszSegment + pAct->usStart;

          b = pszSegment[pAct->usStop];
          pszSegment[pAct->usStop] = EOS;

          while( (c = *pText++) != NULC)
          {
            switch (c)
            {
              case SO:
              case SI:
                sEnd  = pActNew->usStop = (USHORT)(pText - pszSegment- 2);
                usType = pActNew->usType;
                // insert new element for SO/SI as PROTECTED_CHAR
                if ( sEnd > pActNew->usStart )
        {
                  pActNew++;
        }
                sEnd++;
                pActNew->usStart = pActNew->usStop = sEnd;
                pActNew->usType  = PROTECTED_CHAR;
                // insert start of normal text again with old status
        sEnd++;
        if ( sEnd < pAct->usStop )
        {
                  pActNew++;
                  pActNew->usStart = sEnd; //+1;
                  pActNew->usStop  = pAct->usStop;
                  pActNew->usType  = usType;
        }
                break;
            }
          }

          pszSegment[pAct->usStop] = b;
        }
        pActNew++; pAct++;
      } /* endwhile */
      // Add end element ..
      memcpy( pActNew, pAct, sizeof(STARTSTOP));

      UtlAlloc( (PVOID *) ppStartStop, 0L, 0L, NOMSG );
      *ppStartStop = pCurrent;
    } /* endif */
  } /* endif */

  return (usRC);
}


typedef EQF_BOOL (// /*APIENTRY*/ 
*PFNSTARTSTOPEXIT) ( PSZ, PSTARTSTOP *);
typedef EQF_BOOL (// /*APIENTRY*/ 
*PFNSTARTSTOPEXITW) ( PSZ_W, PSTARTSTOP *);

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TACreateProtectTable
//------------------------------------------------------------------------------
// Function call:     TACreateProtectTable
//------------------------------------------------------------------------------
// Description:       Creates a table with start/stop info for protected/
//                    unprotected data in input string.
//------------------------------------------------------------------------------
// Parameters:        PSZ          pszSegmen  ptr to text of segment
//                    PVOID        pVoidTabl  ptr to tag table
//                    USHORT       usColPos   colpos of first char in segment
//                    PTOKENENTRY  pTokBuffe  buffer used temporarly for tokens
//                    USHORT       usTokBuff  size of token buffer in bytes
//                    PSTARTSTOP   *ppStartS  ptr to caller's start/stop table
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       NO_ERROR              if function completed successfully
// Returncodes:       ERROR_INVALID_DATA    if one of the input parameter is in
//                                          error
//                    ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
//------------------------------------------------------------------------------
USHORT TACreateProtectTableW
(
  PSZ_W            pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP                 // ASCII cp fitting to Segmenttext
)
{
  return( TACreateProtectTableWEx( pszSegment, pVoidTable, usColPos, pTokBuffer, usTokBufferSize,
          ppStartStop, pvUserExit, pvUserExitW, ulCP, 0 ) );
}

int memicmp(const void *buf1, const void *buf2, size_t count){
  return memcmp(buf1, buf2, count);
}

USHORT TACreateProtectTableWEx
(
  PSZ_W            pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP,                // ASCII cp fitting to Segmenttext
  int              iMode                // mode for function
)
{
  USHORT           usRC = NO_ERROR;    // function return code
  PSTARTSTOP       pStartStop = NULL;  // ptr to new start/stop table
  PSTARTSTOP       pCurrent;           // ptr for start/stop table processing
  PSTARTSTOP       pAct;               // ptr for start/stop table processing
  BOOL             fTag;               // we are in tag processing mode
  ULONG            ulTableUsed = 0;
  ULONG            ulTableAlloc = 0;
  PLOADEDTABLE     pTagTable;          // ptr to loaded tag table
  PTOKENENTRY      pTok;               // ptr for token list processing
  PSZ_W            pRest;              // ptr to not tokenized data
  PTAG             pTag = NULL;        // pointer to tags in tagtable
  PATTRIBUTE       pAttribute = NULL;  // pointer to attributes in tagtable
  PBYTE            pByte;              // help pointer for tag table addressing
  SHORT            sNumTags = 0;       // number of tags in tag table
  PSZ_W            pszTemp;            // temporary text pointer
  USHORT           usI;                // loop counter
  CHAR_W           chQuote = NULC;     // buffer for starting quote
  PFNSTARTSTOPEXIT pfnUserExit;        // type corrected user exit pointer
  PFNSTARTSTOPEXITW pfnUserExitW;
  BOOL             fTRNoteFound = 0;   // true if TRNOTE found in curr segment
  BOOL             fTRTag= FALSE;      // true if inside of a trnote
  CHAR             chAsciiSeg[MAX_SEGMENT_SIZE];
  PTOKENENTRY      pAttrTokBuffer = NULL;  // used temp.for translatable attributetokens
  LONG             lAttrSize = 0L;
  PTOKENENTRY      pNextTok;             // ptr for next token in token list

  /********************************************************************/
  /* Check input parameters                                           */
  /********************************************************************/
  pTagTable = (PLOADEDTABLE) pVoidTable;

  if ( (pszSegment == NULL)    ||
       (pTagTable  == NULL)    ||
       (pTokBuffer == NULL)    ||
       (usTokBufferSize == 0)  ||
       (ppStartStop == NULL) )
  {
    usRC = ERROR_INVALID_DATA;
  } /* endif */

  /********************************************************************/
  /* Use user exit for start/stop table if there is one               */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    if (pvUserExitW )
    {
      pfnUserExitW = (PFNSTARTSTOPEXITW) pvUserExitW;
      if ( !pfnUserExitW( pszSegment, &pStartStop ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    }
    else  if ( false && pvUserExit )
    {
      /****************************************************************/
      /* Let user exit create the start/stop table                    */
      /****************************************************************/
      pfnUserExit = (PFNSTARTSTOPEXIT)pvUserExit;
      Unicode2ASCII( pszSegment, &chAsciiSeg[0], ulCP);
      if ( !pfnUserExit( &chAsciiSeg[0], &pStartStop ) )
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    }
    else
    {
      LogMessage( T5DEBUG, "TEMPORARY_HARDCODED in TACreateProtectTableWEx if ( false &sa& pvUserExit )");
      /******************************************************************/
      /* Create start/stop table using tokenizer                        */
      /******************************************************************/

      /********************************************************************/
      /* Set tag table variables                                          */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        sNumTags   = (SHORT)pTagTable->pTagTable->uNumTags;
        pByte      = (PBYTE)pTagTable->pTagTable;
        pTag       = OFFSETTOPOINTER(PTAG, pTagTable->pTagTable->stFixTag.uOffset );
        pAttribute = OFFSETTOPOINTER(PATTRIBUTE,
                                     pTagTable->pTagTable->stAttribute.uOffset );
      } /* endif */

      /********************************************************************/
      /* Tokenize input segment                                           */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pTagTable = (PLOADEDTABLE)pVoidTable;
        pRest     = NULL;
        TATagTokenizeW( pszSegment,
                       pTagTable,
                       TRUE,
                       &pRest,
                       &usColPos,
                       pTokBuffer,
                       (USHORT)(usTokBufferSize / sizeof(TOKENENTRY)) );
        usRC = ( pRest == NULL ) ? NO_ERROR : EQFRS_AREA_TOO_SMALL;
      } /* endif */

      /********************************************************************/
      /* Count number of tokens in list to get start/stop list size       */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pTok        = pTokBuffer;

        while ( pTok->sTokenid != ENDOFLIST )
        {
          /****************************************************************/
          /* Check for attributes and tags with translatable info         */
          /****************************************************************/
          if ( ( (pTok->sTokenid >= sNumTags) &&
                 (pAttribute[pTok->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
               ( (pTok->sTokenid >= 0)       &&
                 (pTok->sTokenid < sNumTags) &&
                 (pTag[pTok->sTokenid].BitFlags.fTranslate) ) )
          {
            /************************************************************/
            /* for any quote in the token text leave room for one more  */
            /* start/stop entry                                         */
            /************************************************************/
            pszTemp = pTok->pDataStringW;
            for ( usI = 0; usI < pTok->usLength; usI++, pszTemp++ )
            {
              if ( (*pszTemp == QUOTE) || (*pszTemp == DBLQUOTE) )
              {
                ulTableAlloc++;           // leave room for one more entry
				// P016238: leave room for 4 more entries assuming 2 inline tags
				// per each translatable part of an attribute
                ulTableAlloc = ulTableAlloc + 4;
              } /* endif */
            } /* endfor */
          } /* endif */
          ulTableAlloc++;                   // leave room for token
          pTok++;
        } /* endwhile */
        ulTableAlloc++;                     // leave room for table end delimiter
      } /* endif */

      // assure that ulTableAlloc * sizeof(STARTSTOP) is at least MIN_ALLOC
      if (ulTableAlloc < 4 )
      {
		    ulTableAlloc = 4;
      }

      /********************************************************************/
      /* Allocate start/stop list                                         */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        if ( !UtlAlloc( (PVOID *)&pStartStop, 0L,
                       (LONG) (ulTableAlloc * sizeof(STARTSTOP)),
                       NOMSG ))
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
        ulTableUsed = 0;
      } /* endif */

      TATagAddWSpace(pszSegment, pTokBuffer, sNumTags);

      /********************************************************************/
      /* Fill start/stop table                                            */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pCurrent = pStartStop;
        pTok     = pTokBuffer;
        while ( (pTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR))
        {
          /****************************************************************/
          /* Check for attributes or tags with translatable info          */
          /****************************************************************/
          if ( ( (pTok->sTokenid >= sNumTags) &&
                 (pAttribute[pTok->sTokenid - sNumTags].BitFlags.fTranslate) ) ||
               ( (pTok->sTokenid >= 0)       &&
                 (pTok->sTokenid < sNumTags) &&
                 (pTag[pTok->sTokenid].BitFlags.fTranslate) ) )
          {
            /**************************************************************/
            /* Process data up to first quote or up to end of token       */
            /* Complete protected start/stop entry in front of          */
            /* quoted string                                            */
            /**************************************************************/
            pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);
            pCurrent->usStop = TAFindQuote( pTok, (USHORT) (pCurrent->usStart-1), &chQuote);
            pCurrent->usType  =
                (pTok->sTokenid >= sNumTags) ?  PROTECTED_CHAR : TAGPROT_CHAR;

            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
                                               &ulTableUsed, &ulTableAlloc);

            /**************************************************************/
            /* Process quoted strings if any                              */
            /**************************************************************/
            while ( pTok->usLength && (usRC == NO_ERROR) )
			{ // P016238: check whether quoted text contains protected parts!
			  if (!pAttrTokBuffer)
			  {
				lAttrSize = MAX_ATTR_TOKENS * sizeof(TOKENENTRY);
				if (!UtlAlloc((PVOID *) &pAttrTokBuffer, 0L, lAttrSize, NOMSG) )
				{
				   usRC = ERROR_NOT_ENOUGH_MEMORY;
				} /* endif */
			  } /* endif */
			  if ( usRC == NO_ERROR)
			  {
				usRC = TAProtectedPartsInQuotedText( pTok,
										  pTok->pDataStringW - pszSegment,
										  &pStartStop,
										  &pCurrent,
										  pAttrTokBuffer,
										  pTagTable,
										  &ulTableAlloc,
										  &ulTableUsed,
										  &usColPos,
										  chQuote,
										  lAttrSize / sizeof(TOKENENTRY),
                      ((iMode & CREATEPROTTABLE_MARKATTR) != 0) );
			  }

			  /************************************************************/
			  /* Add entry for protected data up to next quote or up to   */
			  /* end of token                                             */
			  /************************************************************/
			  if ( pTok->usLength && (usRC == NO_ERROR))
			  {
				pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);
				pCurrent->usType  = PROTECTED_CHAR;

				pTok->pDataStringW++;          // assure quote is in pCurrent
				pTok->usLength--;
				pCurrent->usStop = TAFindQuote( pTok, (USHORT) (pCurrent->usStart), &chQuote);

				usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
				                                 &ulTableUsed, &ulTableAlloc);

			  } /* endif */
            } /* endwhile */
          }
          else
          {
            pCurrent->usStart = (USHORT)(pTok->pDataStringW - pszSegment);

            if ( pTok->sTokenid == TEXT_TOKEN  )
            {
              pCurrent->usType = UNPROTECTED_CHAR;
            }
            else if (pTok->sTokenid >= sNumTags )
            {
              // do not include translatable variables in protected text when working in CREATEPROTECTTABLE_NOTRANSLVAR mode
              if ( (pTok->ClassId == CLS_TRANSLVAR) && (iMode & CREATEPROTTABLE_NOTRANSLVAR) )
              {
                pCurrent->usType = UNPROTECTED_CHAR;
              }
              else
              {
                pCurrent->usType = PROTECTED_CHAR;
              } /* endif */                 
            }
            else
            {
              pCurrent->usType = TAGPROT_CHAR;
              if (pTag[pTok->sTokenid].BitFlags.fTRNote  )
              {
                if (fTRNoteFound )
                {
                  /****************************************************/
                  /* this approach requires that a TRNote is within   */
                  /* one segment; also nesting of notes is not allowed*/
                  /****************************************************/
                  pCurrent->usType = TRNOTE_CHAR;
                }
                else
                {
                  if (memicmp((PBYTE)&pTagTable->chTrnote1TextW[0],
                              (PBYTE)(pTok->pDataStringW+pTok->usLength),
                              pTagTable->ulTRNote1Len) == 0 )
                  {
                    pCurrent->usType = TRNOTE_CHAR;
                    fTRNoteFound = TRUE;
                  }
                  else if (memicmp((PBYTE)&pTagTable->chTrnote2TextW[0],
                                   (PBYTE)(pTok->pDataStringW+pTok->usLength),
                                   pTagTable->ulTRNote2Len) == 0 )
                  {
                    pCurrent->usType = TRNOTE_CHAR;
                    fTRNoteFound = TRUE;
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
            pCurrent->usStop  = pCurrent->usStart + pTok->usLength - 1;
            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
		                                     &ulTableUsed, &ulTableAlloc);

          } /* endif */

          pTok++;
        } /* endwhile */

       if (usRC == NO_ERROR)
       {
          TASetStartStopType(pCurrent, 0,0,0 );     //terminate start/stoptabl.

	        usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
	                                      &ulTableUsed, &ulTableAlloc);

       }
      } /* endif */


      /********************************************************************/
      /* now loop true list again and join tags with their attributes     */
      /* this is necessary to avoid splitting of tags and attributes      */
      /* during the insert operation in the editor...                     */
      /********************************************************************/
      if ( usRC == NO_ERROR )
      {
        pAct = pCurrent = pStartStop;
        pTok = pTokBuffer;
        pNextTok = pTok + 1;
        if ( (pAct->usType == TAGPROT_CHAR) && TA_IS_TAG(pTok->sTokenid, sNumTags))
        {
          pAct->usType = PROTECTED_CHAR;
          fTag = TRUE;
        }
        else
        {
          fTag = FALSE;
        } /* endif */
        if ( pCurrent->usType )
        {
          pCurrent++;
          if (pNextTok && pNextTok->pDataStringW &&
               (pCurrent->usStart >=(USHORT)(pNextTok->pDataStringW - pszSegment)))
          {
            pTok ++;
            pNextTok++;
          }

          while ( pCurrent->usType )
          {
            switch ( pCurrent->usType )
            {
              case  TAGPROT_CHAR:
                if ( (pCurrent->usStart == (USHORT)(pTok->pDataStringW-pszSegment))
                      && TA_IS_TAG(pTok->sTokenid, sNumTags))
				{
				    fTag = TRUE;
                }
                pAct++;
                pAct->usStart = pCurrent->usStart;
                pAct->usStop  = pCurrent->usStop;
                pAct->usType = PROTECTED_CHAR;
                break;
              case  PROTECTED_CHAR:
                if ( fTag  && (pCurrent->usType == pAct->usType))
                {
                  /************************************************************/
                  /* combine tag and attribute...                             */
                  /************************************************************/
                  pAct->usStop = pCurrent->usStop;
                }
                else
                {
                  pAct++;
                  pAct->usStart = pCurrent->usStart;
                  pAct->usStop  = pCurrent->usStop;
                  pAct->usType  = pCurrent->usType;
                } /* endif */
                break;
              default :
                pAct++;
                pAct->usStart = pCurrent->usStart;
                pAct->usStop  = pCurrent->usStop;
                pAct->usType  = pCurrent->usType;
                if ( ((pCurrent->usStart == (USHORT)(pTok->pDataStringW-pszSegment))
				      && (pTok->sTokenid == TEXT_TOKEN)) ||
				      (pCurrent->usType == TRNOTE_CHAR))
                {
                    fTag = FALSE;
                }
                // P016804: if text-token is from quoted text, do not reset!!
                //fTag = FALSE;
                break;
            } /* endswitch */
            /******************************************************************/
            /* point to next one...                                           */
            /******************************************************************/
            pCurrent++;
            if (pNextTok && pNextTok->pDataStringW &&
                (pCurrent->usStart >=(USHORT)(pNextTok->pDataStringW - pszSegment)))
            {
              pTok ++;
              pNextTok++;
            }
          } /* endwhile */
          /********************************************************************/
          /* add stopping mode....                                            */
          /********************************************************************/
          pAct++;
          pAct->usStart = 0;
          pAct->usStop  = 0;
          pAct->usType  = 0;
        } /* endif */

        /********************************************************************/
        /* now loop thru list and combine the tokens of a translators note  */
        /* into one start-stop token                                        */
        /********************************************************************/
        if (fTRNoteFound )
        {
          pAct = pCurrent = pStartStop;
          if ( pAct->usType == TRNOTE_CHAR )
          {
            fTRTag = TRUE;
          }
          else
          {
            fTRTag = FALSE;
          } /* endif */
          if ( pCurrent->usType )
          {
            pCurrent++;

            while ( pCurrent->usType )
            {
              switch ( pCurrent->usType )
              {
                case TRNOTE_CHAR :      // end of TRNOTE of begin of next TRNOTE
                  if (fTRTag )
                  {
                    pAct->usStop = pCurrent->usStop;
                  }
                  else
                  {
                    pAct++;
                    pAct->usStart = pCurrent->usStart;
                    pAct->usStop  = pCurrent->usStop;
                    pAct->usType  = pCurrent->usType;
                  } /* endif */
                  fTRTag = !fTRTag;
                  break;
                default:
                  if ( fTRTag )
                  {
                    /************************************************************/
                    /* combine tag and attribute...                             */
                    /************************************************************/
                    pAct->usStop = pCurrent->usStop;
                  }
                  else
                  {
                    pAct++;
                    pAct->usStart = pCurrent->usStart;
                    pAct->usStop  = pCurrent->usStop;
                    pAct->usType  = pCurrent->usType;
                  } /* endif */
                  break;
              } /* endswitch */
              /******************************************************************/
              /* point to next one...                                           */
              /******************************************************************/
              pCurrent++;
            } /* endwhile */
            /********************************************************************/
            /* add stopping mode....                                            */
            /********************************************************************/
            pAct++;
            pAct->usStart = 0;
            pAct->usStop  = 0;
            pAct->usType  = 0;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( pAttrTokBuffer )
  {
     UtlAlloc( (PVOID *)&pAttrTokBuffer, 0L, 0L, NOMSG );
  } /* endif */

  /********************************************************************/
  /* Return start/stop list to caller or cleanup                      */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    *ppStartStop = pStartStop;
    if (IsDBCS_CP(ulCP))
    {
      usRC =  TASoSiProtectTable( pszSegment, ppStartStop);
    }
  }
  else
  {
    if ( pStartStop )
    {
      UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
    } /* endif */
    *ppStartStop = NULL;
  } /* endif */

  return( usRC );
}

VOID TASetStartStopType
(
  PSTARTSTOP       pCurrent,        // ptr to caller's current start/stop entry
  USHORT           usStart,
  USHORT           usStop,
  USHORT           usType
)
{
  pCurrent->usStart = usStart;
  pCurrent->usStop = usStop;
  pCurrent->usType = usType;
  return;
}

USHORT TAFindQuote
(
    PTOKENENTRY pTok,
    USHORT      usStartSearch,
    PCHAR_W     pchQuote
)
{
  USHORT  usStop = usStartSearch;
  while ( pTok->usLength &&
          (pTok->pDataStringW[0] != QUOTE) &&
          (pTok->pDataStringW[0] != DBLQUOTE) )
  {
    pTok->pDataStringW++;
    pTok->usLength--;
    usStop++;
  } /* endwhile */

  if ( pTok->usLength )
  {
    *pchQuote = pTok->pDataStringW[0];
    pTok->pDataStringW++;
    pTok->usLength--;
    usStop++;
  } /* endif */
  return(usStop);
}

USHORT TAGotoNextStartStopEntry
(
	PSTARTSTOP  * ppStartStop,
	PSTARTSTOP  * ppCurrent,
	PULONG    pulTableUsed,
	PULONG    pulTableAlloc
)
{
	PSTARTSTOP pCurrent    = *ppCurrent;
	ULONG      ulTableUsed = * pulTableUsed;
	ULONG      ulTableAlloc = *pulTableAlloc;
	USHORT     usRC        = NO_ERROR;    // function return code

    pCurrent ++;
    ulTableUsed ++;
    // for tests only is pCurrent after add as above?
    { PSTARTSTOP pStartStop = *ppStartStop;
       pCurrent = pStartStop + ulTableUsed;
    }
	if ( ulTableAlloc <= ulTableUsed + 1)
	{
    	 LONG  lOldLen;                       // old length
         LONG  lNewLen;                       // new length
    	PSTARTSTOP pNewStartStop = *ppStartStop;

    	lOldLen = ulTableAlloc * sizeof(STARTSTOP);
    	ulTableAlloc = ulTableAlloc + 20;
    	lNewLen = ulTableAlloc * sizeof(STARTSTOP);

		// realloc nec
        if ( !UtlAlloc( (PVOID *)&pNewStartStop, lOldLen,
                                  lNewLen, NOMSG ))
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
        else
        {
			// if realloc ok, set pointers
			pCurrent = pNewStartStop + ulTableUsed;
            *ppStartStop = pNewStartStop;
            *pulTableAlloc = ulTableAlloc;
	    }
    }
    if ( usRC == NO_ERROR)
    {
      // set return values
      *ppCurrent = pCurrent;
      *pulTableUsed = ulTableUsed;
    }
	return ( usRC );
}


USHORT  TAProtectedPartsInQuotedText
(
  PTOKENENTRY   pTok,
  int           iAttrStartOffset,
  PSTARTSTOP    * ppStartStop,
  PSTARTSTOP    * ppCurrent,
  PTOKENENTRY     pAttrTokBuffer,
  PLOADEDTABLE    pTagTable,
  PULONG          pulTableAlloc,
  PULONG          pulTableUsed,
  PUSHORT         pusColPos,
  CHAR_W          chQuote,
  LONG            lAttrSize,
  BOOL            fSpecialMode
)
{
    USHORT           usRC = NO_ERROR;
    CHAR_W           chTempBuf[MAX_SEGMENT_SIZE];
    PTOKENENTRY      pAttrTok;
    PSZ_W            pAttrStart;
    USHORT           usAttrLength = 0;
    PSZ_W            pRest;              // ptr to not tokenized data
    PSZ_W            pTemp;
    PSTARTSTOP       pCurrent = *ppCurrent;
    PSTARTSTOP       pStartStop = *ppStartStop;


      memset (&chTempBuf[0], 0, sizeof(CHAR_W) * (MAX_SEGMENT_SIZE));

      UTF16strcat(chTempBuf, pTok->pDataStringW);
      pAttrStart = &chTempBuf[0];
      pTemp = &chTempBuf[0];
      while ( pTemp && (*pTemp != chQuote) )
      {
        pTemp++;
        usAttrLength ++;
      } /* endwhile */

      if (*pTemp == chQuote )
      {
          *pTemp = EOS;    //investigate only text inside quotes!
      }

      if ( usRC == NO_ERROR )
      {
        TATagTokenizeW( chTempBuf,
                        pTagTable,
                        TRUE,
                        &pRest,
                        pusColPos,
                        pAttrTokBuffer,
                        (USHORT)lAttrSize );
        usRC = ( pRest == NULL ) ? NO_ERROR : EQFRS_AREA_TOO_SMALL;
      } /* endif */

      if ( usRC == NO_ERROR )
      {
          pAttrTok        = pAttrTokBuffer;
          while ( (pAttrTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR))
          {
            pCurrent->usStart = (USHORT)(pAttrTok->pDataStringW - pAttrStart
                                + iAttrStartOffset);
            pCurrent->usType = (pAttrTok->sTokenid == TEXT_TOKEN) ?
                                UNPROTECTED_CHAR : PROTECTED_CHAR;
            if ( fSpecialMode )
            {
              pCurrent->usType |= 0x8000;
            } /* endif */
            pCurrent->usStop = pCurrent->usStart + pAttrTok->usLength - 1;

            usRC = TAGotoNextStartStopEntry( &pStartStop, &pCurrent,
	                                      pulTableUsed, pulTableAlloc);

            pAttrTok++;
          } /* endwhile */
      } /* endif */

      /**********************************************************/
      /* Add data up to first quote                             */
      /**********************************************************/
      while ( pTok->usLength && (pTok->pDataStringW[0] != chQuote) )
      {
        pTok->pDataStringW++;
        pTok->usLength--;
        usAttrLength --;
      } /* endwhile */
      // CHECK: does this loop run as far as pTemp in chTempBuf?
      if (usAttrLength != 0)
      {
          // this should not happen!!
          usAttrLength = 0;
      }

      *ppCurrent = pCurrent;
      *ppStartStop = pStartStop;
    return (usRC);
}



USHORT TACreateProtectTable
(
  PSZ              pszSegment,        // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  ULONG            ulCP
)
{
  USHORT usRC = 0;
     PSZ_W        pszInputW = &chInputW[0];

   ASCII2Unicode( pszSegment, pszInputW, ulCP );

   usRC = TACreateProtectTableW( pszInputW, pVoidTable, usColPos, pTokBuffer, usTokBufferSize,
                                ppStartStop, pvUserExit, NULL, ulCP );

   // attention:
   // for DBCS code pages the returned offsets and lengths do not match the actual
   // position in the source data and have to be re-adjusted

  return usRC;
}

USHORT TAPrepProtectTable
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit    // address of ptr to TOC  func
)
{
  return( TALoadEditUserExit( pVoidTable, "", phModule, ppfnUserExit, ppfnCheckSegExit,
                              ppfnWYSIWYGExit, ppfnTocGotoExit, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL ) );

} /* end of function TAPrepProtectTable */

USHORT TALoadEditUserExit
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit,   // address of ptr to TOC  func
  PFN              *ppfnGetSegContext, // address of ptr to EQFGETSEGCONTEXT function
  PFN              *ppfnUpdateContext, // address of ptr to EQFUPDATECONTEXT function
  PFN              *ppfnFormatContext, // address of ptr to EQFFORMATCONTEXT function
  PFN              *ppfnCompareContext, // address of ptr to EQFCOMPARECONTEXT function
  PFN              *ppfnUserExitW,     // unicode user exit to create start-stop-tbl
  PFN              *ppfnCheckSegExitW,  // unicode user exit to check segment
  PFN              *ppfnCheckSegExExitW,// unicode user exit to check segment Ex
  PFN              *ppfnCheckSegType   // user exit to check segment type
)
{
  PTAGTABLE        pTagTable;          // pointer to tag table
  USHORT           usRC = NO_ERROR;    // function return code

  /********************************************************************/
  /* initialize user's data fields                                    */
  /********************************************************************/
  *phModule = NULLHANDLE;
  if ( ppfnUserExit ) *ppfnUserExit = NULL;
  if ( ppfnCheckSegExit ) *ppfnCheckSegExit = NULL;
  if ( ppfnWYSIWYGExit ) *ppfnWYSIWYGExit = NULL;
  if ( ppfnTocGotoExit ) *ppfnTocGotoExit = NULL;
  if ( ppfnGetSegContext ) *ppfnGetSegContext  = NULL;
  if ( ppfnUpdateContext ) *ppfnUpdateContext  = NULL;
  if ( ppfnFormatContext ) *ppfnFormatContext  = NULL;
  if ( ppfnCompareContext ) *ppfnCompareContext  = NULL;
  if ( ppfnUserExitW ) *ppfnUserExitW = NULL;
  if ( ppfnCheckSegExitW ) *ppfnCheckSegExitW = NULL;
  if ( ppfnCheckSegExExitW ) *ppfnCheckSegExExitW = NULL;
  if ( ppfnCheckSegType ) *ppfnCheckSegType = NULL;


  /********************************************************************/
  /* Address tag table                                                */
  /********************************************************************/
  pTagTable = ((PLOADEDTABLE)pVoidTable)->pTagTable;

  /********************************************************************/
  /* Load user exit if tag table has one                              */
  /********************************************************************/
  if ( pTagTable->szSegmentExit[0] != EOS )
  {
    CHAR  szExit[MAX_LONGFILESPEC];        // buffer for library name
    CHAR  *ptr ;

//  strcpy( szExit, pTagTable->szSegmentExit );
//  strcat( szExit, EXT_OF_DLL );
//  if ( MUGetUserExitFileName( pTagTable->szSegmentExit, NULL, szExit, sizeof(szExit) ) )
    if ( MUGetUserExitFileName( pszTableName, NULL, szExit, sizeof(szExit) ) )
    {
      ptr = strrchr( szExit, '\\' ) ;
      if ( ptr ) 
         strcpy( ++ptr, pTagTable->szSegmentExit ) ;

      usRC = DosLoadModule( NULL, 0, szExit, phModule );
     
      if ( usRC == NO_ERROR )
      {
        if ( ppfnUserExit )
        {
     
          usRC = DosGetProcAddr( *phModule, EQFPROTTABLE_EXIT, (PFN*)ppfnUserExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnUserExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load SegmentCheckExit function                        */
        /****************************************************************/
        if ( ppfnCheckSegExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEG_EXIT,
                                  (PFN*)ppfnCheckSegExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnCheckSegExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load ShowTransl  Exit function                        */
        /****************************************************************/
        if ( ppfnWYSIWYGExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFSHOWTRANS_EXIT,
                                (PFN*)ppfnWYSIWYGExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnWYSIWYGExit = NULL;
          } /* endif */
        }
        /****************************************************************/
        /* try to load CheckSegType  Exit function                        */
        /****************************************************************/
        if ( ppfnCheckSegType )
        {
          *ppfnCheckSegType = NULL;
          DosGetProcAddr( *phModule, EQFCHECKSEGTYPE_EXIT, (PFN*)ppfnCheckSegType);
        }
        /****************************************************************/
        /* try to load TOC user    Exit function                        */
        /****************************************************************/
        if ( ppfnTocGotoExit )
        {
          usRC = DosGetProcAddr( *phModule, EQFTOCGOTO_EXIT,
                                  (PFN*)ppfnTocGotoExit);
          if ( usRC != NO_ERROR )
          {
            *ppfnTocGotoExit = NULL;
          } /* endif */
        }
        // try to load context related user exit functions
        if ( ppfnGetSegContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFGETSEGCONTEXT_EXIT, (PFN*)ppfnGetSegContext );
          if ( usRC != NO_ERROR ) *ppfnGetSegContext = NULL;
        }
        if ( ppfnUpdateContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFUPDATECONTEXT_EXIT, (PFN*)ppfnUpdateContext );
          if ( usRC != NO_ERROR ) *ppfnUpdateContext = NULL;
        }
        if ( ppfnCompareContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFCOMPARECONTEXT_EXIT, (PFN*)ppfnCompareContext );
          if ( usRC != NO_ERROR ) *ppfnCompareContext = NULL;
        }
        if ( ppfnFormatContext )
        {
          usRC = DosGetProcAddr( *phModule, EQFFORMATCONTEXT_EXIT, (PFN*)ppfnFormatContext );
          if ( usRC != NO_ERROR ) *ppfnFormatContext = NULL;
        }
        if ( ppfnUserExitW )
        {
           usRC = DosGetProcAddr( *phModule, EQFPROTTABLEW_EXIT, (PFN*)ppfnUserExitW);
           if ( usRC != NO_ERROR )  *ppfnUserExitW = NULL;
        }
        if ( ppfnCheckSegExitW )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEGW_EXIT, (PFN*)ppfnCheckSegExitW);
          if ( usRC != NO_ERROR )     *ppfnCheckSegExitW = NULL;
        }
        if ( ppfnCheckSegExExitW )
        {
          usRC = DosGetProcAddr( *phModule, EQFCHECKSEGEXW_EXIT, (PFN*)ppfnCheckSegExExitW);
          if ( usRC != NO_ERROR )     *ppfnCheckSegExExitW = NULL;
        }
     
      } /* endif */
    }
  } /* endif */

  return( NO_ERROR );
} /* end of function TAPrepProtectTable */


USHORT TAEndProtectTable
(
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit ,     // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit,    // address of ptr to TOC func
  PFN              *ppfnUserExitW,      // unicode UserExit to make StartStoptable
  PFN              *ppfnCheckSegExitW   // unicode CheckSegExit
)
{
  TAFreeEditUserExit( phModule );

  if ( ppfnUserExit )    *ppfnUserExit = NULL;
  if ( ppfnCheckSegExit) *ppfnCheckSegExit = NULL;
  if ( ppfnWYSIWYGExit ) *ppfnWYSIWYGExit = NULL;
  if ( ppfnTocGotoExit ) *ppfnTocGotoExit = NULL;
  if ( ppfnUserExitW )    *ppfnUserExitW = NULL;
  if ( ppfnCheckSegExitW) *ppfnCheckSegExitW = NULL;

  return( NO_ERROR );
} /* end of function TAEndProtectTable */

USHORT TAFreeEditUserExit
(
  HMODULE      *phModule
)
{
  if ( *phModule )
  {
      //DosFreeModule( *phModule );
      DosFreeModule( phModule );
      *phModule = NULLHANDLE;
  } /* endif */
  return( NO_ERROR );
} /* end of function TAFreeEditUserExit */

/*
bool iswspace(wchar_t ch){
  return std::iswspace(ch);
}
//*/

// P016804:
 static void
 TATagAddWSpace
   (
	   PSZ_W                pszSegment,
	   PTOKENENTRY  	    pTokBuffer,
	   SHORT                sNumTags
   )
   {
     PTOKENENTRY      pTok;
     USHORT           usRC = NO_ERROR;
     PTOKENENTRY      pNextTok;
     USHORT           usStart = 0;
     USHORT           usNextStart = 0;
     PSZ_W            pTmp = NULL;
     USHORT           usI = 0;
     USHORT           usNewLen = 0;

        pTok     = pTokBuffer;
        pNextTok = pTok + 1;
        while ( (pTok->sTokenid != ENDOFLIST) && (usRC == NO_ERROR) && (pNextTok->sTokenid != ENDOFLIST))
        {
		  if (TA_IS_TAG(pTok->sTokenid, sNumTags) || TA_IS_ATTRIBUTE(pTok->sTokenid, sNumTags))
		  {
			  if (TA_IS_ATTRIBUTE(pNextTok->sTokenid, sNumTags))
			  { // if there are whitespace between pTok and pNextTok, add them to pTok
				 usStart = (USHORT)(pTok->pDataStringW - pszSegment);
				 usNextStart = (USHORT)(pNextTok->pDataStringW - pszSegment);
				 usI = usStart + pTok->usLength ;
				 pTmp = pTok->pDataStringW + pTok->usLength;
				 usNewLen = pTok->usLength;
				 while ( usI < usNextStart && iswspace(*pTmp))
				 {
					 usI++;
					 pTmp++;
					 usNewLen ++;
				 }
				 if (usI == usNextStart)
				 { // add the whitespaces to the length of pTok!
					pTok->usLength = usNewLen;
				 }
			  }
		   } /* endif */
           pTok ++;
		   pNextTok++;
        } /* endwhile */
        return;
	}

