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
   char buff[255];
   sprintf(buff, "UtlErrorHwnd(sErrorNumber=%d, usMsgType=%d, usNoOfParams=%d, pParamTable=%s, ErrorType=%d )", sErrorNumber, usMsgType,
                  usNoOfParms, pParmTable, ErrorType);
   LogMessage(ERROR,buff);
LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 63");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
   unsigned int  usMsgboxStyle;               // style for WinMessageBox call
   unsigned int  usMsgboxRet = 0;             // msg box return value
   PSZ    pMsgboxTitle;                // title text of message box
   PSZ    pErrorText;                  // pointer to system error text
   PSZ    pDosError;                   // pointer to DOS return code string
   PSZ    pQdamError[2];               // pointer to QDAM return code string
   CHAR   chNum[10];                   // itoa string buffer for message number
   PSZ    pQldbError;                  // pointer to QLDB return code string
   PSZ    pPropError;                  // pointer to PROP return code string
   HWND   hwndFrame;                   // handle of troja main window
   HWND    hwndParent;                 // parent window for message box call
   CHAR   szError[80];                 // buffer for Messages
   usMsgType &= ( 0x0F | MB_DEFBUTTON1 | MB_DEFBUTTON2 | MB_DEFBUTTON3 );

   switch ( ErrorType )
   {
      case EQF_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case CMD_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_CMD_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case CMD_INFO :
         //usMsgboxStyle = MB_INFORMATION;
         pMsgboxTitle = EQF_CMD_ERROR_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_WARNING:
         //usMsgboxStyle = MB_WARNING;
         pMsgboxTitle = EQF_WARNING_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_INFO:
         //usMsgboxStyle = MB_INFORMATION;
         pMsgboxTitle = EQF_INFO_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case EQF_QUERY:
         //usMsgboxStyle = MB_QUERY;
         pMsgboxTitle = EQF_QUERY_TXT;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, usNoOfParms, pParmTable );
         break;

      case SYSTEM_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = SYSTEM_ERROR_TXT;
         pErrorText = NO_ERRINFO_TXT;
         strncpy( chMsgBuffer, pErrorText, sizeof(chMsgBuffer) );
         break;

      case DOS_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = SYSTEM_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szDosError, *pParmTable, sizeof(szDosError) );
            szDosError[sizeof(szDosError)-1] = EOS;
         }
         else
         {
            szDosError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlDosMsgTxt ( sErrorNumber, szDosError,
                                       &usMsgType );
         pDosError = szDosError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pDosError );
//         strcpy(chMsgBuffer, "Ebbi's special ");
         break;

      case QDAM_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = QDAM_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         pQdamError[0] = szError;
     pQdamError[1] = strncpy( chNum, std::to_string(sErrorNumber).c_str(), 10 );
     sErrorNumber = UtlQdamMsgTxt ( sErrorNumber );
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 2, &pQdamError[0] );
         break;

      case QLDB_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = QLDB_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlQldbMsgTxt ( sErrorNumber );
         pQldbError = szError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pQldbError );
         break;

      case PROP_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = PROP_ERROR_TXT;
         if ( usNoOfParms && pParmTable && *pParmTable )
         {
            strncpy( szError, *pParmTable, sizeof(szError) );
            szError[sizeof(szError)-1] = EOS;
         }
         else
         {
            szError[0] = EOS;
         } /* endif */
         sErrorNumber = UtlPropMsgTxt ( sErrorNumber ) ;
         pPropError = szError;
         UtlGetMsgTxt ( sErrorNumber, chMsgBuffer, 1, &pPropError );
         break;

      case SHOW_ERROR :
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = EQF_ERROR_TXT;
         strcpy( chMsgBuffer, *pParmTable );
         break;

      case INTERNAL_ERROR :
      default :                        // unknown error type ==> internal error!
         //usMsgboxStyle = MB_ERROR;
         pMsgboxTitle = INTERNAL_ERROR_TXT;
         UtlGetMsgTxt( ERROR_INTERNAL, chMsgBuffer, usNoOfParms, pParmTable );
         break;

   } /* endswitch */

   switch (usMsgboxStyle)
   {
     case MB_ERROR:
        ERREVENT2( UTLERROR_LOC, MSGBOX_EVENT, sErrorNumber, GENERAL_GROUP, "" );
        break;
     default:
        INFOEVENT2( UTLERROR_LOC, MSGBOX_EVENT, sErrorNumber, GENERAL_GROUP, "" );
        break;
   } /* endswitch */

   usMsgboxStyle |= MB_DEFBUTTON1 | MB_MOVEABLE | usMsgType;
   usMsgboxStyle |= MB_APPLMODAL;

   // GQ 2015/10/13 Set parent handle of message box to HWND_FUNCIF when running in API mode and no parent handle has been specififed
   if ( !hwndMsgBoxParent && (UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE ) )
   {
     hwndMsgBoxParent = HWND_FUNCIF;
   }
  
   hwndParent = hwndMsgBoxParent;      // use supplied window handle

   if ( !hwndParent )
   {
      //hwndParent     = QUERYACTIVEWINDOW();
      hwndFrame      = ErrData[UtlGetTask()].hwndFrame;
      if ( (GETPARENT( hwndParent ) == hwndFrame) ||
            (GETOWNER( hwndParent ) == hwndFrame) )
      {
         // ok, this window belongs to Troja
      }
      else
      {
         // focus seems to be on another application
         hwndParent     = hwndFrame;
      } /* endif */
   } /* endif */

   usMsgboxStyle |= ( ErrData[UtlGetTask()].hwndHelpInstance ) ? MB_HELP : 0;

   /*******************************************************************/
   /* Set last used message number not for ERROR_CMD_ERROR to avoid   */
   /* overwrite of actual error code                                  */
   /*******************************************************************/
   if ( sErrorNumber != ERROR_CMD_ERROR )
   {
     UtlSetUShort( QS_LASTERRORMSGID, sErrorNumber );
   } /* endif */
   UtlSetUShort( QS_CURMSGID, (USHORT)sErrorNumber );

   if ( hwndMsgBoxParent == HWND_FUNCIF )
   {
     PDDEMSGBUFFER pMsgBuffer = ErrData[UtlGetTask()].pLastMessage;
     if ( pMsgBuffer != NULL )
     {
       pMsgBuffer->sErrNumber  = sErrorNumber;
       strcpy( pMsgBuffer->chMsgText, chMsgBuffer );
     } /* endif */
   }
   else
   {
	   // display message box
	   if ( (usMsgboxStyle & 0x000F) == MB_EQF_YESTOALL )
	   {
		 YESTOALL_IDA IDA;
		 HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

		 IDA.pszTitle = pMsgboxTitle;
		 IDA.pszText  = chMsgBuffer;
		 IDA.sID      = sErrorNumber;
		 IDA.usStyle  = (USHORT)usMsgboxStyle;

		 DIALOGBOX( hwndParent, UTLYESTOALLDLG, hResMod,
					ID_UTL_YESTOALL_DLG,
					&IDA, usMsgboxRet );
	   }
	   else
	   { 
		   //   No RightStyle for message boxes
		   //     usMsgboxStyle |= MB_RTLREADING | MB_RIGHT;
		   usMsgboxRet = WinMessageBox ( HWND_DESKTOP,
									     hwndParent,
									     chMsgBuffer,
									     pMsgboxTitle,
									     sErrorNumber,
									     usMsgboxStyle );
	   } /* endif */
   }
   UtlSetUShort( QS_CURMSGID, 0 );

   return ( (USHORT)usMsgboxRet );
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
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
   // check for function mode I/F (in this mode we use the help instance
   // handle as pointer to the last message area
   if ( hwndFrame == HWND_FUNCIF )
   {
     ErrData[UtlGetTask()].hab              = hab;
     ErrData[UtlGetTask()].hwndFrame        = hwndFrame;
     ErrData[UtlGetTask()].hwndHelpInstance = hwndHelpInstance;
     strcpy( ErrData[UtlGetTask()].chMsgFile, pMsgFile );
     ErrData[UtlGetTask()].pLastMessage     = (PDDEMSGBUFFER)hwndHelpInstance;
   }
   else
   {
     ErrData[UtlGetTask()].hab              = hab;
     ErrData[UtlGetTask()].hwndFrame        = hwndFrame;
     ErrData[UtlGetTask()].hwndHelpInstance = hwndHelpInstance;
     strcpy( ErrData[UtlGetTask()].chMsgFile, pMsgFile );
     ErrData[UtlGetTask()].pLastMessage     = NULL;
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

   if ( ErrData[UtlGetTask()].hMsgFile )
   {

   LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 137");
#ifdef TEMPORARY_COMMENTED
     UtlClose( ErrData[UtlGetTask()].hMsgFile, FALSE );
     ErrData[UtlGetTask()].hMsgFile = 0;
   #endif
   } /* endif */
   {
     USHORT  usDummy;
     UtlOpen(pMsgFile, &ErrData[UtlGetTask()].hMsgFile , &usDummy,
              0L,                            // Create file size
              FILE_NORMAL,                   // Normal attribute
              OPEN_ACTION_OPEN_IF_EXISTS,    // Open if exists
              OPEN_SHARE_DENYWRITE,          // Deny Write access
              0L,                            // Reserved
              FALSE );                           // no error handling
   }
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

