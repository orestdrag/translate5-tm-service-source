/*! \brief EQFXAPI.C
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
	Description: EQF_API contains the functions described in the document 'Troja Editor API'                                            |
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
//  #define DLLIMPORTRESMOD         // resource module handle imported from DLL

#include "EQF.H"                  // General Translation Manager include file

#include "EQFB.ID"                // Translation Processor IDs
#include <EQFTPI.H>               // private Translation Processor include file
#include "EQFTAI.H"               // Private include file for Text Analysis

#include <EQFDOC00.H>

#include "EQFFOL.H"
#include "EQFHLOG.H"              // private history log include file
#include "core/utilities/LogWrapper.h"

#define SLIDER_INCREMENT    101                  // number of increments on slider bar

PSTEQFGEN        pstEQFGeneric = NULL;          // pointer to generic struct
HGLOBAL       sel;
HANDLE        hMapObject = NULL;              // file map object for shared memory

static USHORT  EQFAllocate (VOID);
static BOOL    fUnicodeSystem = 0;

INT_PTR CALLBACK EQFSHOWDLG ( HWND, WINMSG, WPARAM, LPARAM );
static VOID RegisterShowWnd( VOID );
BOOL EQFShowWndCreate ( PTBDOCUMENT, HWND, ULONG, USHORT );
USHORT WriteHistLogAndAdjustCountInfo( PSZ pszSegTargetFile, BOOL fAdjustCountInfo );
MRESULT APIENTRY EQFSHOWWP ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2);
MRESULT HandleWMChar ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
#define EQFSHOW_CLASS   "EQFSHOW"
#define EQFSHOW_CLASS_W L"EQFSHOW"

typedef struct   _TextPanel
{
    HWND         hwndParent;
    PSZ_W   *    ppData;
    USHORT       usDefFG;
    USHORT       usDefBG;
    USHORT       usForeground;
    USHORT       usBackground;
    PSZ_W        pCaptionText;
    ULONG        flFrameStyle;
    DOCTYPE      docType;
    TBDOCUMENT   TBDoc;
    BYTE         TokBuf[TOK_BUFFER_SIZE * 2];
    BYTE         BlockMark[ sizeof( EQFBBLOCK )];
    BYTE         InBuf[ IO_BUFFER_SIZE * 2];
    BOOL         fUnicode;    // unicode indication
} TEXTPANEL, * PTEXTPANEL;
static BOOL  FillDisplay ( PTEXTPANEL pTextPanel );
static BOOL PrepareLine ( PTEXTPANEL, PSZ_W, PULONG );

static TBSEGMENT  tbInitSegment = { NULL, 0, QF_PROP0PREFIX, 0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"", NULL};
static TBSEGMENT  tbNewLineSegment = { NULL , 1,QF_XLATED,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"\n", NULL};
static VOID  EQFDispWindow ( PTBDOCUMENT  pTBDoc );
static SHORT sTagging[4];

#define GETNUMBER( pszTarget, usValue ) \
{                                   \
   usValue = 0;                     \
   if ( *pszTarget++ )              \
   {                                \
      while ( isdigit(*pszTarget) ) \
      {                             \
         usValue = (usValue * 10) + (*pszTarget++ - '0'); \
      } /* endwhile */              \
   } /* endif */                    \
}
/*------------------------------------------------------------------------------
* Function prototypes (internal)
*-----------------------------------------------------------------------------*/
USHORT  EQF_PackBuf     (PUCHAR, PSZ *);

/*------------------------------------------------------------------------------
* Globals
*-----------------------------------------------------------------------------*/
USHORT          usEQFErrID   = EQFRC_OK;        // ret.message ID
USHORT          usEQFRcClass = EQF_INFO;        // ret.code class
USHORT          usEQFSysRc   = NO_ERROR;        // system return code
ERRTYPE         ErrorType;                      // error classes
CHAR            szMsgFile[CCHMAXPATH] = "c:\\eqf\\msg\\eqf.msg"; // def. msg file
CHAR_W          szMsgBuf[EQF_MSGBUF_SIZE];      // buffer for message

USHORT __cdecl /*APIENTRY*/ EQFWORDCNTPERSEGW
(
  PSZ_W          pszSeg,               // ptr to Segment
  PSZ            pszLang,              // ptr to Language of Segment
  PSZ            pszFormat,            // ptr to Format
  PULONG         pulResult,            // result to be counted
  PULONG         pulMarkUp              // result for markup
)
{
  USHORT         usRC=NO_ERROR;        // success indicator

  if(*pszSeg && *pszLang && *pszFormat && pulResult && pulMarkUp)
  {
    usRC = NO_ERROR;
  }
  else
  {
    usRC = ERROR_INVALID_DATA;
  }/* end if */

  if ( usRC == NO_ERROR )
  {
    PVOID       pVoidTable = NULL;       // ptr to loaded tag table
    PTOKENENTRY pTokBuf = NULL;
    SHORT       sLangID = -1;


    // allocate required buffers
    if ( usRC == NO_ERROR )
    {
      BOOL fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, (LONG)TOK_BUFFER_SIZE, NOMSG );
      if (!fOK )
      {
        usRC = ERROR_INVALID_DATA;
      } /* endif */
    } /* endif */

    // Load Tag Table
    if ( usRC == NO_ERROR )
    {
      usRC =  TALoadTagTable( pszFormat,
                              (PLOADEDTABLE *) &pVoidTable,
                               FALSE, FALSE );
    } /* endif */

    // Get Language ID
    if ( usRC == NO_ERROR )
    {
      //usRC = MorphGetLanguageID( pszLang, &sLangID );
    } /* end if */

    // do the actual counting
    if ( usRC == NO_ERROR )
    {
      *pulResult = 0L;
      *pulMarkUp = 0L;

      usRC = EQFBWordCntPerSeg(
                       (PLOADEDTABLE)pVoidTable,
                       pTokBuf,
                       pszSeg,
                       sLangID,
                       pulResult, pulMarkUp, 1);
    } /* endif */

    // cleanup
    if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
    if ( pVoidTable) TAFreeTagTable( (PLOADEDTABLE)pVoidTable );
  } /* endif */

  return usRC;

} /* end of function EQFWORDCNTPERSEG */
