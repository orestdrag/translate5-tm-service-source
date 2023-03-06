/*! \file
	Description: EQF general services

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_DEV
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_MORPH            // morph return codes
#include <string>
#include <EQF.H>                  // General Translation Manager include file
#include "LogWrapper.h"
#include <time.h>                 // C time related functions


#include "EQFUTPRI.H"                  // private utility header file
#include "EQFUTILS.ID"    // IDs used by dialog utilities

//#define DEF_EVENT_ENUMS
#include "EQFEVENT.H"                  // event logging functions

#include "win_types.h"

//static PEVENTTABLE pEventTable;        // ptr to event logging table
static SHORT  asLogGroups[30] = { -1 };// list of active debug log groups
extern PEVENTTABLE pEventTable;        // ptr to event logging table

//default variable used in error messages (will be changed to "" later on)
#define UTLERR_DEFVAR        "\nERROR: VARIABLE NOT SUPPLIED\n"

INT_PTR CALLBACK UTLYESTOALLDLG( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );

#if defined(UTL_LOGGING)
   #define UTLLOGPATH "\\EQFD\\BIN\\EQFUTL.LOG"
#endif

#define MAXNUMOFDDEMESSAGES  24


#define NO_ERRINFO_TXT       "NO SYSTEM ERROR INFO AVAILABLE"
#define NOMSG_TXT            "NO ERROR MESSAGE AVAILABLE"

/**********************************************************************/
/* Global variables                                                   */
/**********************************************************************/
       ERRDATA ErrData[MAX_TASK];      // data required for error handler
static CHAR chMsgBuffer[MSGBUFFERLENGTH]; // buffer for message text
static CHAR chDDEMsgBuffer[MSGBUFFERLENGTH]; // buffer for DDE messages
static CHAR   szDosError[80];          // buffer for DosErrorMessage
static DDEMSGBUFFER DDEMessages[MAXNUMOFDDEMESSAGES+1] = { { (HWND)0, 0, "" } } ;


CHAR  SYSTEM_ERROR_TXT [30];       // "SYSTEM ERROR"
CHAR  QDAM_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  QLDB_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  PROP_ERROR_TXT [30]  ;       // "EQF ERROR"
CHAR  EQF_ERROR_TXT [30]   ;       // "EQF ERROR"
CHAR  EQF_CMD_ERROR_TXT [30];      // "CMD ERROR"
CHAR  EQF_WARNING_TXT [30]  ;      // "EQF Warning"
CHAR  EQF_QUERY_TXT [30]    ;      // "EQF Query"
CHAR  EQF_INFO_TXT [30]     ;      // "EQF Information"
CHAR  INTERNAL_ERROR_TXT [30];     // "INTERNAL ERROR"


/**********************************************************************/
/* Protoypes for internal functions                                   */
/**********************************************************************/
SHORT  UtlQdamMsgTxt ( SHORT );
static SHORT UtlDosMsgTxt ( SHORT, PSZ, PUSHORT );
static SHORT UtlQldbMsgTxt ( SHORT );
static SHORT UtlPropMsgTxt ( SHORT );


USHORT UtlErrorHwnd
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ *   pParmTable,                 // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwndMsgBoxParent            // window which should be msgbox parent
);

//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlError               Report an error condition
//------------------------------------------------------------------------------
// Description:       Displays error message and handles link to help
//                    function (IPF).
//------------------------------------------------------------------------------
// Function Call:     USHORT UtlError( USHORT  usErrorID,
//                                     USHORT  usType,
//                                     USHORT  usNoOfParms,
//                                     PSZ     *pParmTable,
//                                     ERRTYPE ErrorType );
//------------------------------------------------------------------------------
// Parameters:        - usErrorID is the number of the error message to be
//                      displayed.
//                    - usType specifies the buttons to be contained in the
//                      message box (e.g. MB_OK for OK button, MB_YESNOCANCEL
//                      for YES, NO and CANCEL button).
//                      additional the default button has to be defined here
//                      (e.g. MB_YESNO | MB_DEFBUTTON?  ? will become default,
//                      where ? may be 1 2 or 3. )
//                    - usNoOfParms specifies the numbers of parameters (= size
//                      of parameter table) which are to be replaced in the
//                      message text
//                    - pParmTable is the pointer to pointer table for the
//                      paramter strings
//                    - ErrorType is the type of the error. Valid are:
//
//                      SYSTEM_ERROR   (errors in WINxxx and GPIxxx calls;
//                                      message text is retrieved by
//                                      WinQueryLastError )
//                      DOS_ERROR      (errors in DosXXX calls )
//                      EQF_ERROR      (user errors)
//                      EQF_INFO       (information message)
//                      EQF_WARNING    (warnings)
//                      INTERNAL_ERROR (internal processing errors)
//------------------------------------------------------------------------------
// Prerequesits:      The error handler has to be initialized during startup
//                    using the init call UtlInitError.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       return code of WinMessageBox call indicating the selected
//                            button (e.g. MBID_OK = OK button)
//------------------------------------------------------------------------------
// Function flow:    Call UtlErrorHwnd with a NULL window handle
//------------------------------------------------------------------------------
USHORT UtlErrorHwndW
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ_W   *pParmTableW,               // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwnd,                       // window handle
   BOOL    fUnicode                    // parameter table contains unicode strings
);
USHORT UtlErrorW
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ_W   *pParmTableW,               // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   BOOL    fUnicode                    // parameter table contains unicode strings
)
{
  return UtlErrorHwndW( sErrorNumber,               // number of message
                          usMsgType,                  // type of message
                          usNoOfParms,                // number of message parameters
                          pParmTableW,                // pointer to parameter table
                          ErrorType,                  // type of error
                          NULLHANDLE,                 // window handle
                          fUnicode );                 // parameter table contains unicode strings
}

USHORT UtlErrorHwndW
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ_W   *pParmTableW,               // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwnd,                       // window handle
   BOOL    fUnicode                    // parameter table contains unicode strings
)
{
	USHORT usRc = 0;
	PSZ    pParmTable[5];
	int    i = 0;


	if (!fUnicode)
	{
		for ( i=0; i<usNoOfParms; i++ )
		{
			pParmTable[i] = (PSZ)pParmTableW[i];
		}

		usRc =  UtlErrorHwnd( sErrorNumber,
							usMsgType,
							usNoOfParms,
							pParmTable,
							ErrorType,
							hwnd );
	}
	else
	{
		PSZ pData = NULL;
		UtlAlloc((PVOID *) &pData, 0L, usNoOfParms * 256, NOMSG );
		if (pData || (usNoOfParms == 0))
		{
			for ( i=0; i<usNoOfParms; i++ )
			{
				pParmTable[i] = pData + i*256;
				if (UTF16strlenCHAR(pParmTableW[i]) >= 128)
				{
					CHAR_W  chW = pParmTableW[i][128];
					pParmTableW[i][128] = EOS;
					UtlDirectUnicode2Ansi( pParmTableW[i], pParmTable[i], 0L );
					pParmTableW[i][128] = chW;
				}
				else
				{
					UtlDirectUnicode2Ansi( pParmTableW[i], pParmTable[i], 0L );
				}
			}

			usRc =  UtlErrorHwnd( sErrorNumber,
								usMsgType,
								usNoOfParms,
								pParmTable,
								ErrorType,
								hwnd );
			UtlAlloc((PVOID *) &pData, 0L, 0L, NOMSG );
		}

	}
	return( usRc );
}

USHORT UtlError
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ     *pParmTable,                // pointer to parameter table
   ERRTYPE ErrorType                   // type of error
)
{
   return( UtlErrorHwnd( sErrorNumber,
                         usMsgType,
                         usNoOfParms,
                         pParmTable,
                         ErrorType,
                         NULLHANDLE ) );
}


//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlErrorHwnd           Report an error condition
//------------------------------------------------------------------------------
// Description:       Displays error message and handles link to help
//                    function (IPF).
//------------------------------------------------------------------------------
// Function Call:     USHORT UtlErrorHwnd( USHORT  usErrorID,
//                                         USHORT usType,
//                                         USHORT usNoOfParms,
//                                         PSZ *pParmTable,
//                                         ERRTYPE ErrorType,
//                                         HWND  hwndParent );
//------------------------------------------------------------------------------
// Parameters:        - usErrorID is the number of the error message to be
//                      displayed.
//                    - usType specifies the buttons to be contained in the
//                      message box (e.g. MB_OK for OK button, MB_YESNOCANCEL
//                      for YES, NO and CANCEL button).
//                      additional the default button has to be defined here
//                      (e.g. MB_YESNO | MB_DEFBUTTON?  ? will become default,
//                      where ? may be 1 2 or 3. )
//                    - usNoOfParms specifies the numbers of parameters (= size
//                      of parameter table) which are to be replaced in the
//                      message text
//                    - pParmTable is the pointer to pointer table for the
//                      paramter strings
//                    - ErrorType is the type of the error. Valid are:
//
//                      SYSTEM_ERROR   (errors in WINxxx and GPIxxx calls;
//                                      message text is retrieved by
//                                      WinQueryLastError )
//                      DOS_ERROR      (errors in DosXXX calls )
//                      EQF_ERROR      (user errors)
//                      EQF_INFO       (information message)
//                      EQF_WARNING    (warnings)
//                      EQF_QUERY      (queries)
//                      QDAM_ERROR
//                      QLDB_ERROR
//                      PROP_ERROR
//                      INTERNAL_ERROR (internal processing errors)
//
//                    - hwndParent is the parent window for the error
//                      message box
//------------------------------------------------------------------------------
// Prerequesits:      The error handler has to be initialized during startup
//                    using the init call UtlInitError.
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       return code of WinMessageBox call indicating the selected
//                            button (e.g. MBID_OK = OK button)
//------------------------------------------------------------------------------
// Function flow:     mask given message type to strip off invalid flags
//                    switch error type
//                      case EQF_ERROR:
//                        MB style = MB_ERROR
//                        title    = EQF_ERROR_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_WARNING:
//                        MB style = MB_WARNING
//                        title    = EQF_WARNING_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_INFO:
//                        MB style = MB_INFORMATION
//                        title    = EQF_INFOR_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case EQF_QUERY:
//                        MB style = MB_QUERY
//                        title    = EQF_QUERY_TEXT
//                        call UtlGetMsgTxt to get text of message
//                      case SYSTEM_ERROR:
//                        MB style = MB_ERROR
//                        title    = SYSTEM_ERROR_TEXT
//                        call WinGetErrorInfo to get latest error info
//                         and use the supplied data
//                      case DOS_ERROR:
//                        MB style = MB_ERROR
//                        title    = SYSTEM_ERROR_TEXT
//                        call UtlDosMsgTxt to get message for DOS error
//                        call UtlGetMsgTxt to get text of message
//                      case QDAM_ERROR:
//                        MB style = MB_ERROR
//                        title    = QDAM_ERROR_TEXT
//                        call UtlQdamMsgTxt to get message for QDAM error
//                        call UtlGetMsgTxt to get text of message
//                      case QLDB_ERROR:
//                        MB style = MB_ERROR
//                        title    = QLDB_ERROR_TEXT
//                        call UtlQldbMsgTxt to get message for QLDB error
//                        call UtlGetMsgTxt to get text of message
//                      case PROP_ERROR:
//                        MB style = MB_ERROR
//                        title    = PROP_ERROR_TEXT
//                        call UtlPropMsgTxt to get message for PROP error
//                        call UtlGetMsgTxt to get text of message
//                      case INTERNAL_ERROR:
//                        MB style = MB_ERROR
//                        title    = INTERNAL_ERROR_TEXT
//                        call UtlGetMsgTxt to get internal error message text
//                    endswitch
//                    search parent handle for message box or use supplied
//                     handle
//                    add help flag to message box style if help instance exist
//                    if process has a message queue then
//                      call WinMessageBox to show the error message
//                    else
//                      issue a series of beeps as no message box can be shown
//                    endif
//                    free any error info
//------------------------------------------------------------------------------
USHORT UtlErrorHwnd
(
   SHORT   sErrorNumber,               // number of message
   USHORT  usMsgType,                  // type of message
   USHORT  usNoOfParms,                // number of message parameters
   PSZ *   pParmTable,                 // pointer to parameter table
   ERRTYPE ErrorType,                  // type of error
   HWND    hwndMsgBoxParent            // window which should be msgbox parent
)
{
   T5LOG(T5ERROR) << "UtlErrorHwnd(sErrorNumber="<< sErrorNumber <<", usMsgType="<< usMsgType <<", usNoOfParams="<<
      usNoOfParms << ", pParamTable="<< pParmTable <<", ErrorType="<< ErrorType <<" )"; 
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetMsgTxt           Get message text
//------------------------------------------------------------------------------
// Function call:     UtlGetMsgTxt( SHORT msgNo, PSZ pBuffer,
//                                  USHORT usNoOfParms, PSZ *pParmTable );
//------------------------------------------------------------------------------
// Description:       Get the text for a given message number
//------------------------------------------------------------------------------
// Input parameter:   SHORT    msgNo          message number = ID in MSG file
//                    PSZ      pBuffer        buffer for message text
//                    USHORT   usNoOfParms    number of parameters in table
//                    PSZ      *pParmTable    pointer to parameter table
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      Name of message file must be stored in the ErrData
//                    structure
//------------------------------------------------------------------------------
// Function flow:     if no parameter supplied then
//                      set parameter pointer to VARIABLE_NOT_SUPPLIED text
//                    endif
//                    get message text using DosGetMessage
//                    remove CR/LF from message text, replace \n with LF
//------------------------------------------------------------------------------
VOID UtlGetMsgTxt
(
   SHORT    msgNo,                     // message number = ID in MSG file
   PSZ      pBuffer,                   // buffer for message text
   USHORT   usNoOfParms,               // number of parameters in table
   PSZ      *pParmTable                // pointer to parameter table
)
{
   ULONG  ulLength = 0;                // length of message buffer
   PSZ    pMsgText;                    // pointer for message text processing
   PSZ    pDefVar;                     // pointer for default insert variable

   //--- switch to default insert variable if no variable is supplied ---
   if ( usNoOfParms == 0 )
   {
      usNoOfParms = 1;
      pDefVar = UTLERR_DEFVAR;
      pParmTable = &pDefVar;
   } /* endif */

   DosError(0);
   DosGetMessage( pParmTable, usNoOfParms, pBuffer, sizeof(chMsgBuffer), msgNo,
                  ErrData[UtlGetTask()].chMsgFile, &ulLength );
   DosError(1);
   *(pBuffer+ulLength) = '\0';         // force end of message

   // remove CR/LF, replace C linefeed escape character (backslash-n) with LF
   pMsgText = pBuffer;
   while ( *pMsgText != '\0' )
   {
      switch (*pMsgText)
      {
        case BACKSLASH:
           *pBuffer++ = *pMsgText++;   // move character to target
           // if backslash is followed by 'n' and
           if ( *pMsgText == 'n' && (*(pMsgText+1) == ' ' ||  // blank    /*@N4C*/
                                     *(pMsgText+1) == CR  ||  // car ret  /*@N4A*/
                                     *(pMsgText+1) == LF  ) ) // linefeed /*@N4A*/
           {
              pMsgText++;              // ... skip it and
//            *(pBuffer-1) = LF;       // replace backslash with linefeed
              *(pBuffer-1) = '\n';     // replace backslash with '\n'   6-7-16
           } /* endif */
           break;
        case CR:
           pMsgText++;                 // ignore carriage return
           break;
        case LF:
//         *pBuffer++ = ' ';           // replace linefeed with space
           *pBuffer++ = '\n';          // replace linefeed with '\n'    6-7-16
           pMsgText++;
           break;
        default:
           *pBuffer++ = *pMsgText++;   // copy character to output buffer
           break;
      } /* endswitch */

   } /* endwhile */

   *pBuffer = '\0';                    // terminate string

} /* end of function UtlGetMsgTxt */


//------------------------------------------------------------------------------
// External Function
//------------------------------------------------------------------------------
// Function Name:     UtlInitError           Initialization for UtlError calls
//------------------------------------------------------------------------------
// Description:       Initializes error handler, This function MUST be called
//                    before any call to UtlError is done!
//------------------------------------------------------------------------------
// Function Call:     VOID UtlInitError( HAB     hab,
//                                       HWND    hwndFrame,
//                                       HWND    hwndHelpInstance,
//                                       PSZ     pMsgFile );
//------------------------------------------------------------------------------
// Parameters:        - hab is the process anchor block handle
//                    - hwndFrame is the frame window handle of the main window
//                    - hwndHelpInstance is the window handle of the
//                      help instance. Use NULL if no help has been associated
//                      yet.
//                    - pMsgFile points to the fully qualified name of the
//                      message file
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Store supplied values in global ErrData structure
//------------------------------------------------------------------------------
VOID UtlInitError
(
   HAB  hab,                           // anchor block handle
   HWND hwndFrame,                     // handle of main window frame
   HWND hwndHelpInstance,              // help instance handle
   PSZ  pMsgFile                       // name of message file
)
{
   ERRDATA* errData = &ErrData[UtlGetTask()];
   // check for function mode I/F (in this mode we use the help instance
   // handle as pointer to the last message area
   if ( hwndFrame == HWND_FUNCIF )
   {
     errData->hab              = hab;
     errData->hwndFrame        = hwndFrame;
     errData->hwndHelpInstance = hwndHelpInstance;   
     errData->pLastMessage     = (PDDEMSGBUFFER)hwndHelpInstance;
   }
   else
   {
     errData->hab              = hab;
     errData->hwndFrame        = hwndFrame;
     errData->hwndHelpInstance = hwndHelpInstance;
     errData->pLastMessage     = NULL;
   } /* endif */


      strcpy(SYSTEM_ERROR_TXT   , "OpenTM2");
      strcpy(QDAM_ERROR_TXT     , "OpenTM2");
      strcpy(QLDB_ERROR_TXT     , "OpenTM2");
      strcpy(PROP_ERROR_TXT     , "OpenTM2");
      strcpy(EQF_ERROR_TXT      , "OpenTM2");
      strcpy(EQF_CMD_ERROR_TXT  , "OpenTM2 Batch" );
      strcpy(EQF_WARNING_TXT    , "OpenTM2");
      strcpy(EQF_QUERY_TXT      , "OpenTM2");
      strcpy(EQF_INFO_TXT       , "OpenTM2");
      strcpy(INTERNAL_ERROR_TXT , "OpenTM2");

}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     UtlGetLastError
//------------------------------------------------------------------------------
// Function call:     UtlGetLastError ( PSZ )
//------------------------------------------------------------------------------
// Description:
//------------------------------------------------------------------------------
// Returncode type:
//------------------------------------------------------------------------------
// Function flow:
//
//------------------------------------------------------------------------------
USHORT UtlGetDDEErrorCode( HWND hwndDDE )
{
  USHORT usRC = NO_ERROR;

  // look for given handle in our DDE message table
  if ( hwndDDE == HWND_FUNCIF )
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  }
  else
  {
      PDDEMSGBUFFER pBuf = DDEMessages;
      int i;
      for ( i = 0; i < MAXNUMOFDDEMESSAGES; i++, pBuf++ )
      {
       if ( pBuf->hwndDDE == hwndDDE )
       {
         break;                        // found the handle in our table
       } /* endif */
      } /* endfor */
      if ( i < MAXNUMOFDDEMESSAGES )    // found the handle
      {
        usRC = pBuf->sErrNumber;
      } /* endif */
  } /* endif */
  return( usRC );
}

