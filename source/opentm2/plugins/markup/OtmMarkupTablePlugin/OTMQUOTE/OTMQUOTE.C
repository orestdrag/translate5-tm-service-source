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

static USHORT MriGetSettings( PSZ pszTagTable, PMRIPARSEFLAGS pFlags );

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

// check segment content before it is saved

EQF_BOOL  /*APIENTRY*/ EQFCHECKSEGEXW
(
   PSZ_W pszPrevSrc,                     // previous source segment
   PSZ_W pszSrc,                         // current source segment
   PSZ_W pszTgt,                         // current translation
   PEQF_BOOL pfChanged,                  // segment changed
   LONG  lInfo,                          // info handle to use with EQFGETPREVSEG(W),EQFGETNEXTSEG(W)
   ULONG ulSegNum,                       // segment number to use with EQFGETPREVSEG(W),EQFGETNEXTSEG(W)
   EQF_BOOL fMsg                         // message display requested
)
{
  CHAR_W chQuote = L' ';
  int iSourceQuotes, iTargetQuotes;
  EQF_BOOL  fContinue = TRUE;
  MRIPARSEFLAGS Flags; 

  // adress TBDOCUMENT for access to markup table name
  PTBDOCUMENT pDoc = (PTBDOCUMENT)lInfo;
  PLOADEDTABLE pTable = (PLOADEDTABLE)pDoc->pDocTagTable;

  pszPrevSrc; pfChanged; ulSegNum;


  MriGetSettings( pTable->szName, &Flags );

  chQuote = Flags.fSingleQuotes ? QUOTE_DELIMITER_W : STRING_DELIMITER_W;

  // count not escaped delimiters in source and target
  iSourceQuotes = MriCountQuotes( pszSrc, chQuote, Flags.fEscapeChars );
  iTargetQuotes = MriCountQuotes( pszTgt, chQuote, Flags.fEscapeChars );
  if ( iSourceQuotes != iTargetQuotes )
  {
    if ( fMsg )
    {
      USHORT usMBCode = UtlError( EQFQUOT_QUOTE_MISMATCH, MB_YESNO | MB_DEFBUTTON2, 0, NULL, EQF_QUERY );
      if ( usMBCode == MBID_NO )
      {
        fContinue = FALSE;
      } /* endif */
    }
    else
    {
      fContinue = FALSE;
    } /* endif */       
  } /* endif */     

  return( fContinue );
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


/**********************************************************************/
/*  MriGetSettings                                                    */
/*                                                                    */
/* Get any settings file (name of settings file is name of tag table  */
/* with an extension of .CHR) and set the flags in the passed         */
/* flags structure                                                    */
/**********************************************************************/
static USHORT MriGetSettings
(
  PSZ         pszTagTable,             // name of tag table
  PMRIPARSEFLAGS pFlags                // reference to MRI parser flag structure
)
{
  // private data area
  typedef struct _GETSETTINGSDATA
  {
    CHAR chInBuf[8096];                // input buffer
    CHAR szLine[1024];                 // buffer for current line
    CHAR szSettingsFile[MAX_EQF_PATH]; // path name of settings file
  } GETSETTINGSDATA, *PGETSETTINGSDATA;

  PGETSETTINGSDATA pData = NULL;       // pointer to private data
  HFILE hfSettings = NULLHANDLE;       // file handle of settings file
  BOOL        fOK = TRUE;              // internal O.K. flag
  LONG        lBytesToRead = 0L;       // number of bytes to read from file
  ULONG       ulBytesInBuffer = 0;     // number of bytes in buffer
  PMRIPARSMARKUPINFO pMarkupInfo;

  // initialize caller's flags
  memset( pFlags, 0, sizeof(MRIPARSEFLAGS) );

  // check if info is already contained in our static buffer
  pMarkupInfo = MarkupInfo;
  while ( (pMarkupInfo->szMarkup[0] != EOS) && (_stricmp( pMarkupInfo->szMarkup, pszTagTable ) != 0) ) pMarkupInfo++;
  if ( pMarkupInfo->szMarkup[0] != EOS )
  {
    memcpy( pFlags, &(pMarkupInfo->Flags), sizeof(MRIPARSEFLAGS) );
    return( NO_ERROR );  
  } /* endif */


  // allocate our private data structure
  fOK = UtlAlloc( (PVOID *)&pData, 0L, sizeof(GETSETTINGSDATA), NOMSG );

  // setup name of settings file and open settings file
  if ( fOK )
  {
    USHORT usActionTaken;              // action performed by file open

    UtlMakeEQFPath( pData->szSettingsFile, NULC, TABLE_PATH, NULL );
    GetOTMTablePath( pData->szSettingsFile, pData->szLine ) ;
    strcpy( pData->szSettingsFile, pData->szLine ) ;
    Utlstrccpy( pData->szSettingsFile + strlen(pData->szSettingsFile),
                pszTagTable, DOT );
    strcat( pData->szSettingsFile, SETTINGS_EXTENSION );
    fOK = UtlOpen( pData->szSettingsFile, &hfSettings, &usActionTaken, 0L,
             FILE_NORMAL, FILE_OPEN,
             OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
             0L, FALSE ) == NO_ERROR;
  } /* endif */

  // get size of input file
  if ( fOK )
  {
    FILESTATUS  stStatus;              // File status information

    fOK = UtlQFileInfo( hfSettings,  1, (PBYTE)&stStatus,
                        sizeof(FILESTATUS),  FALSE ) == NO_ERROR;
    lBytesToRead = stStatus.cbFile;
  } /* endif */

  // read and process until done
  while( fOK && ((lBytesToRead != 0L)|| (ulBytesInBuffer != 0)) )
  {
    PSETTINGKEYWORD pKeyWord = NULL;          // ptr for keyword processing

    // fill input buffer
    if ( lBytesToRead && (ulBytesInBuffer < sizeof(pData->chInBuf)) )
    {
      USHORT usBytesRead;              // number of bytes read from file
      ULONG  ulBytesToRead;

      ulBytesToRead = min( ((LONG)sizeof(pData->chInBuf)- ulBytesInBuffer),
                                    (ULONG)lBytesToRead );
      // do not upgrade to UtlReadL, since  downward compatibility would be lost!
      // RJ - 030325 - GQ agreed
      fOK = UtlRead( hfSettings, pData->chInBuf + ulBytesInBuffer,
                     (USHORT)ulBytesToRead,
                     &usBytesRead, FALSE ) == NO_ERROR;
      ulBytesInBuffer += usBytesRead;
      lBytesToRead -= usBytesRead;
    } /* endif */

    // extract next line from input buffer
    if ( fOK )
    {
      USHORT usPos = 0;

      // copy bytes to line buffer until EOL detected
      while ( (usPos < ulBytesInBuffer) &&
              (pData->chInBuf[usPos] != CR) &&
              (pData->chInBuf[usPos] != LF ) )
      {
        pData->szLine[usPos] = pData->chInBuf[usPos];
        usPos++;
      } /* endwhile */

      // terminate line buffer and skip EOL characters
      pData->szLine[usPos] = EOS;
      while ( (usPos < ulBytesInBuffer) &&
              ( (pData->chInBuf[usPos] == CR) ||
                (pData->chInBuf[usPos] == LF ) ) )
      {
        usPos++;
      } /* endwhile */

      // adjust data in input buffer
      memmove( pData->chInBuf, pData->chInBuf + usPos, ulBytesInBuffer - usPos );
      ulBytesInBuffer -= usPos;
    } /* endif */

    // check for settings keywords
    if ( fOK )
    {
      // lookup settings keyword in keyword table
      pKeyWord = SettingsKeywords;
      while ( (pKeyWord->szKey[0] != EOS) &&
              (strnicmp( pKeyWord->szKey, pData->szLine, pKeyWord->sLen ) != 0) )
      {
        pKeyWord++;
      }
    } /* endif */

    // check supplied value if keyword found and do any settings
    if ( pKeyWord->szKey[0] != EOS )
    {
      PSZ pszTemp;
      PSZ pszSpecifiedValue = pData->szLine + pKeyWord->sLen;
      PSZ pszValue;
      SHORT sValueIndex = 0;
      BOOL fDone = FALSE;

      // truncate value at first non-alphanumeric character
      pszTemp = pszSpecifiedValue;
      while ( isalnum(*pszTemp) ) pszTemp++;
      *pszTemp = EOS;

      // compare specified value with possible values
      pszValue = pKeyWord->szValues;

      while ( (*pszValue != EOS) && !fDone )
      {
        CHAR chTemp;                   // buffer for value end character

        // isolate current value of possible value list
        pszTemp = pszValue;
        while ( (*pszTemp != ',') && (*pszTemp != EOS) ) pszTemp++;
        chTemp = *pszTemp;
        *pszTemp = EOS;

        // compare against specified value
        if ( stricmp( pszValue, pszSpecifiedValue ) == 0 )
        {
          fDone = TRUE;
        } /* endif */

        // continue with next value (if any)
        *pszTemp = chTemp;
        if ( !fDone && (chTemp != EOS) )
        {
          pszValue = pszTemp + 1;
          sValueIndex += 1;
        }
        else
        {
          pszValue = pszTemp;
        } /* endif */
      } /* endwhile */

      // set flags if valid
      if ( fDone )
      {
        switch ( pKeyWord->sID )
        {
          case DOUBLEQUOTES_ID :
           pFlags->fDoubleQuotes = sValueIndex;
           break;
          case SINGLEQUOTES_ID :
           pFlags->fSingleQuotes = sValueIndex;
           break;
          case ESCAPECHARS_ID :
           pFlags->fEscapeChars = sValueIndex;
           break;
          default:
           break;
        } /* endswitch */
      } /* endif */
    } /* endif */
  } /* endwhile */

  // update static markup table info array 
  if ( !pFlags->fDoubleQuotes && !pFlags->fSingleQuotes )
  {
    pFlags->fDoubleQuotes = TRUE;
  } /* end */     
  strcpy( pMarkupInfo->szMarkup, pszTagTable );
  memcpy( &(pMarkupInfo->Flags), pFlags, sizeof(MRIPARSEFLAGS) );
  pMarkupInfo[1].szMarkup[0] = EOS;


  // cleanup
  if ( hfSettings ) UtlClose( hfSettings, FALSE );
  if ( pData )      UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

  // return to caller
  return( NO_ERROR );
} /* end of function MriGetSettings */



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
