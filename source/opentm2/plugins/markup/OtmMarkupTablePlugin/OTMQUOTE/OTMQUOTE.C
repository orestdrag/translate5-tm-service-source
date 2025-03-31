//+----------------------------------------------------------------------------+
//|OTMQUOTE.C                MAT Tools Parser for Quoted string format files   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2013, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//|Author:        G. Queck (QSoft)                                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:   This module contains all functions necessary to build the    |
//|               MRI parser for the translation processor.                    |
//|               It is invoked via the Text Analysis UserExit function        |
//|               The comment characteristics (etc.) uses the standard 'C'     |
//|               logic supporting ( '//' and '/*' and '*/' )                  |
//|               See EQFBMRI.H for the appropriate defines                    |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|   EQFPRESEG2      - the preparser                                          |
//|   EQFPOSTSEG      - the postparser ( NOP until now )                       |
//|   EQFPREUNSEG     - the preparser during unsegmentation ( NOP until now )  |
//|   EQFPOSTUNSEG    - the postparser during unsegmentation                   |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Changes:                                                                    |
//|  13/02/18   OTMUDQUO.  Track double quote changes.                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tag table defines
#define INCL_EQF_TP               // public translation processor functions       
#include "eqf.h"                  // General Translation Manager include file
#include "eqffol.h"

#include "eqfparse.h"                  // headers used


extern void     GetOTMTablePath( char * basename, char * OTMname ) ;

#define INBUF_SIZE         8192        // size of input buffer

#define SETTINGS_EXTENSION   ".CHR"
#define UNDERSCORE_CHAR_W  L'_'
#define UNDERSCORE_CHAR  '_'

#define LF_W               L'\n'       // a linefeed ...
#define CR_W               L'\r'       // a carriage retu

BOOL  fInit = FALSE;                   // indicator for DBCS initialisation

/**********************************************************************/
/* Structure for flags used during MRI parsing                        */
/**********************************************************************/
typedef struct _MRIPARSFLAGS
{
  BOOL        fSingleQuotes;           // TRUE = handle strings in single quotes
  BOOL        fDoubleQuotes;           // TRUE = handle strings in double quotes
  BOOL        fEscapeChars;            // TRUE = process escape chars (prefixed with a backslash)
} MRIPARSEFLAGS, *PMRIPARSEFLAGS;


// static array with markup table names and associated flags
typedef struct _MRIPARSMARKUPINFO
{
  CHAR        szMarkup[20];            // markup table name
  MRIPARSEFLAGS Flags;                 // associated flags
} MRIPARSMARKUPINFO, *PMRIPARSMARKUPINFO;

static MRIPARSMARKUPINFO MarkupInfo[20] = { { "", { 0, 0, 0 }  } };

//+----------------------------------------------------------------------------+
//| EQFPRESEG2      - presegment parser                                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Invoked from text analysis prior to segmentation                        |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - setup a temporary file name for the segmented file (ext. $$$ )   |
//|         - call ParseQuotedFile                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                -- name of tag table                        |
//|  PSZ pEdit,                    -- name of editor dll                       |
//|  PSZ pProgPath,                -- pointer to program path                  |
//|  PSZ pSource,                  -- pointer to source file name              |
//|  PSZ pTempSource,              -- pointer to source path                   |
//|  PBOOL pfNoSegment             -- no further segmenting ??                 |
//|  HWND  hwndSlider              -- handle of slider window                  |
//|  PBOOL pfKill                  -- ptr to 'kill running process' flag       |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+


//+----------------------------------------------------------------------------+
//| EQFPOSTSEG      - postsegment parser                                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from text analysis after segmentation                           |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    -- name of editor dll                       |
//|  PSZ pProgPath,                -- pointer to program path                  |
//|  PSZ pSegSource,               -- pointer to source seg. file name         |
//|  PSZ pSegTarget,               -- pointer to target seg file               |
//|  PTATAG pTATag                 -- ta tag structure                         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+


EQF_BOOL  /*APIENTRY*/ EQFPOSTSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegSource,               // pointer to source seg. file name
   PSZ pSegTarget,               // pointer to target seg file
   PTATAG_W pTATag,                 // ta tag structure
   HWND     hSlider,
   PEQF_BOOL  pfKill
)
{
   pTagTable; pEdit; pProgPath; pSegSource; pSegTarget; pTATag; hSlider; pfKill;

   return ( TRUE );
}

//+----------------------------------------------------------------------------+
//| EQFPREUNSEG     - preunsegment parser                                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from unsegment utility prior to unsegmentation                  |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    // name of editor dll                       |
//|  PSZ pProgPath,                // pointer to program path                  |
//|  PSZ pSegTarget,               // pointer to target seg file               |
//|  PSZ pTemp,                    // pointer to temp file                     |
//|  PTATAG pTATag,                // ta tag structure                         |
//|  PBOOL  pfNoUnseg              // do no further unsegmentation ??          |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+

EQF_BOOL  /*APIENTRY*/ EQFPREUNSEGW
(
   PSZ pTagTable,
   PSZ pEdit,                    // name of editor dll
   PSZ pProgPath,                // pointer to program path
   PSZ pSegTarget,               // pointer to target seg file
   PSZ pTemp,                    // pointer to temp file
   PTATAG_W pTATag,                // ta tag structure
   PEQF_BOOL  pfNoUnseg,          // do no further unsegmentation ??
   PEQF_BOOL  pfKill
)
{
   pTagTable; pEdit; pProgPath; pSegTarget; pTATag; pTemp;
   pfNoUnseg; pfKill;

   return ( TRUE );
}


//+----------------------------------------------------------------------------+
//| EQFPOSTUNSEG     - postunsegment parser                                    |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    invoked from unsegment utility after unsegmentation                     |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         currently a NOP                                                    |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ pTagTable,                                                            |
//|  PSZ pEdit,                    // name of editor dll                       |
//|  PSZ pProgPath,                // pointer to program path                  |
//|  PSZ pTarget,                  // pointer to target file                   |
//|  PTATAG pTATag,                // ta tag structure                         |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  BOOL   fOK       TRUE           - success                                 |
//|                   FALSE          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+

// helper function counting the number of not escaped quotes in the string
int MriCountQuotes( PSZ_W pszString, CHAR_W chQuote, BOOL fEscapeChars )
{
  int iQuotes = 0;
  BOOL fEscaped = FALSE;
  while ( *pszString != 0 )
  {
    if ( *pszString == chQuote )
    {
      if ( fEscaped )
      {
        // an escaped quote, ignore it
        fEscaped = FALSE;
      }
      else if ( pszString[1] == chQuote )
      {
        // ignore doubled quote
        pszString++;
      }
      else
      {
        iQuotes++;
      }
    }
    else if (fEscapeChars && (*pszString == L'\\') )
    {
      // could be start of an escaped quote
      fEscaped = TRUE;
    }
    else
    {
      fEscaped = FALSE;
    } /* endif */       
    pszString++;
  } /* endwhile */     
  return( iQuotes );
}



/**********************************************************************/
/* Table used by function MriGetSettings to check the settings file.  */
/* The table has to be terminated by an empty entry. The values in    */
/* the value list are seperated by a comma. The function uses the     */
/* index of the specified value in the value list as new setting for  */
/* a specific keyword (e.g. "NO,YES", "NO" sets 0, "YES" sets 1).     */
/**********************************************************************/
typedef struct _SETTINGKEYWORD
{
  SHORT       sID;                     // symbolic identifier for keyword
  CHAR        szKey[20];               // keyword of switch
  SHORT       sLen;                    // length of keyword
  CHAR        szValues[80];            // list of possible values
} SETTINGKEYWORD, *PSETTINGKEYWORD;

enum
{
  DOUBLEQUOTES_ID,
  SINGLEQUOTES_ID,
  ESCAPECHARS_ID
} KEYWORDIDS;

SETTINGKEYWORD SettingsKeywords[] =
{
  { DOUBLEQUOTES_ID, "DOUBLEQUOTES=", 13,  "NO,YES" },
  { SINGLEQUOTES_ID, "SINGLEQUOTES=", 13,  "NO,YES" },
  { ESCAPECHARS_ID,  "ESCAPECHARS=",  12,  "NO,YES" },
  { 0,               "",               0,  "" }
};



//+----------------------------------------------------------------------------+
//| ParseQuotedFile         - parse a MRI file                                        |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Parses an input file and creates a segmented output file.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - setup global data and tag names                                  |
//|         - open input and output file                                       |
//|         - enclose strings of input file in QFF/EQFF tags, enclose          |
//|           all other stuff in QFN/EQFN tags and write data to output file   |
//|         - close input and output file                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ       pszSource             - fully qualified name of source file     |
//|  PSZ       pszTarget             - fully qualified name of target file     |
//|  HWND      hwndSlider            - handle of slider window                 |
//|  PBOOL     pfKill                - pointer to kill flag                    |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
