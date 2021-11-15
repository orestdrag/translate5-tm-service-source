/*! \file
	Description:  Generic startup code for MFC, Win32 as well as OS/2

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

//  #define STARTUP2_LOGGING  

  #undef _WINDLL

#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_DAM              // QDAM functions
#define INCL_EQF_SLIDER           // slider utility functions
#include <EQF.H>                  // General Translation Manager include file

#include <time.h>                      // C library functions: time
//#include <tchar.h>
#include "locale.h"

#include "EQFSTART.ID"                 // our PM IDs
#include "EQFSTART.H"                  // our stuff
#include "EQFSTART.MRI"                // for text of emergency messages
#include "EQFDRVEX.H"                  // drives dialog
#include "EQFSETUP.H"                  // directory names
#include "EQFPROGR.H"                  // progress indicator defines
//#include <dde.h>                     // DDE defines
#include "EQFEVENT.H"                  // event logging facility
#include "EQFART.H"                    // ART registry functions
#include "EQFRPT.H"                    // report handler defines
#include "../utilities/PropertyWrapper.H"

// should be removed
#include "EQFMT00.H"

#undef _WPTMIF                         // we don't care about WP I/F
#include "EQFHELP.ID"                  // help resource IDs
#include "EQFHLP1.H"                   // first part of help tables
//#include "EQFMSG.HTB"                          // message help table

#include "EQFUTMDI.H"                // MDI Utilities

#include "win_types.h"
#include "../utilities/LogWrapper.h"

/**********************************************************************/
/* statics...                                                         */
/**********************************************************************/

static HHOOK  MessageFilterHook = NULL;// message filter hook
//static WNDPROC lpfnOldMDIClient = NULL;// ptr to original MDI client proc
LONG FAR PASCAL NewMDIClientProc      (HWND, UINT, WPARAM, LPARAM);
VOID PerformPendingUpdates();


#ifdef TEMPORARY_COMMENTED
       CHAR     szMsgBuf[256];                   // general purpose message buffer
#endif //TEMPORARY_COMMENTED
static CHAR     szEqfResFile[MAX_EQF_PATH];      // name of resource file
static CHAR     EqfSystemPath[MAX_EQF_PATH];     // global system path
static CHAR     EqfSystemPropPath[MAX_EQF_PATH]; // global system properties path
static CHAR     EqfSystemMsgFile[MAX_EQF_PATH];  // global message file
static CHAR     EqfSystemHlpFile[MAX_EQF_PATH];  // global help file
static CHAR     szEqfSysLanguage[MAX_EQF_PATH];  // system language
HWND      hTwb = NULLHANDLE;           // handle of Twb main window
static HWND     hHelp;                 // IPF help hwnd
static BOOL     fReportHandler = TRUE;           // report handler enabled flag
static BOOL     fSaveOnClose = FALSE;            // save not allowed during start
static USHORT   fShutdownInProgress;             // shutdown in progress flag
static BOOL     fNotMiniMized= FALSE;                      // save not allowed when minimized
static BOOL     fInCloseProcessing = FALSE;      // in close processing flag
static BOOL     fCloseCheckDone = FALSE;         // close check has been done flag
static BOOL     fMinimize = FALSE;               // start TWB frame not minimized
static BOOL     fMFCEnabled = FALSE;             // are we dealing with MFC??

static PUSERHANDLER pUserHandler;      // ptr to user handler table
static USHORT       usUserHandler;     // number of user handlers

CHAR szMsgWindow[] = ">wnd_msg";     // identifier for message help window

static HMODULE      hNetApiMod;
static HAB          hAB;
static BOOL         fIELookAndFeel = FALSE;


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TwbGetCheckProfileData
//------------------------------------------------------------------------------
// Function call:     TwbGetCheckProfileData();
//------------------------------------------------------------------------------
// Description:       Get MAT profile data from OS2.INI file and check the
//                    received values.
//------------------------------------------------------------------------------
// Input parameter:   VOID
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   successful
//                    FALSE  profile entries corrupted
//------------------------------------------------------------------------------
// Function flow:     Query profile values
//                    Check values and handle errors
//                    Set system variables
//------------------------------------------------------------------------------
BOOL TwbGetCheckProfileData( PSZ pEqfSystemMsgFile, PSZ pEqfSystemPropPath,
                             PSZ pEqfResFile )
{
    LogMessage7(DEBUG, "TwbGetCheckProfileData( pEqfSystemMsgFile= ", pEqfSystemMsgFile, ", pEqfSystemPropPath= ",  pEqfSystemPropPath,
                             ", pEqfResFile = ", pEqfResFile,")" );
    CHAR   szDrive[MAX_DRIVE];         // buffer for drive list
    CHAR   szLanDrive[MAX_DRIVE];      // buffer for lan drive list
    CHAR   szSysPath[MAX_EQF_PATH];    // buffer for system path
    BOOL   fOK = TRUE;                 // internal OK flag
    CHAR   szOTMPath[MAX_EQF_PATH];

    // Get system drive and system path
    properties_get_str(KEY_OTM_DIR, szOTMPath, MAX_EQF_PATH);
    properties_get_str_or_default( KEY_Drive, szDrive, sizeof( szDrive  ), szOTMPath );
    properties_get_str_or_default( KEY_LanDrive, szLanDrive, sizeof( szLanDrive  ), szOTMPath );
    properties_get_str_or_default( KEY_Path, szSysPath, sizeof( szSysPath), szOTMPath );
    sprintf( EqfSystemPath, "%s",  szSysPath );

    // Get name of system property file

    properties_get_str_or_default( KEY_SysProp, EqfSystemPropPath, sizeof( EqfSystemPropPath ), szOTMPath );

    // Get name of current language
    properties_get_str_or_default( KEY_SYSLANGUAGE, szEqfSysLanguage, sizeof( szEqfSysLanguage ), DEFAULT_SYSTEM_LANGUAGE );

    // Set name of resource, help file and message file for selected language
    fOK = UtlQuerySysLangFile( szEqfSysLanguage, szEqfResFile,
                               EqfSystemHlpFile, EqfSystemMsgFile );


    /******************************************************************/
    /* Check if resources exist                                       */
    /* (The resource file is not checked here, it will be checked     */
    /*  later on while loading the resource into memory)              */
    /******************************************************************/
    #ifdef TEMPORARY_COMMENTED
    if ( fOK )
    {
      fOK = UtlFileExist( EqfSystemHlpFile );
    } /* endif */

    if ( fOK && szDrive[0] && szSysPath[0] && EqfSystemPropPath[0] &&
         //EqfSystemHlpFile[0] && 
         szEqfResFile[0] )
    {
      UtlSetString( QST_MSGFILE, EqfSystemMsgFile );
      UtlSetString( QST_HLPFILE, EqfSystemHlpFile );
      UtlSetString( QST_RESFILE, szEqfResFile );
    }
    else
    {
      LogMessage3(ERROR, "TwbGetCheckProfileData()::WinMessageBox, STR_INSAL_ERROR, STR_INSTALL_ERROR_TITLE, fOk=", intToA(fOK), ". " );
      //WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, STR_INSTALL_ERROR,
      //               STR_INSTALL_ERROR_TITLE,
      //               1, MB_ENTER | MB_ICONEXCLAMATION);

      fOK = FALSE;
    } /* endif */
    #endif

    /******************************************************************/
    /* return strings to caller if requested...                       */
    /******************************************************************/
    if ( fOK )
    {
      if ( pEqfSystemPropPath )
      {
        strcpy( pEqfSystemPropPath, EqfSystemPropPath );
      } /* endif */
      
      #ifdef TEMPORARY_COMMENTED
      if ( pEqfSystemMsgFile )
      {
        strcpy( pEqfSystemMsgFile, EqfSystemMsgFile );
      } /* endif */
      
      if ( pEqfResFile )
      {
        strcpy( pEqfResFile, szEqfResFile );
      } /* endif */
      #endif
    } /* endif */

    return( fOK);

} /* end of function TwbGetCheckProfileData */
