/*! \brief OTMFUNC.C
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
*/

#include <time.h>
#define INCL_EQF_FOLDER                // folder functions
#define INCL_EQF_ANALYSIS              // analysis functions
#define INCL_EQF_DLGUTILS              // dialog utilities (for tm.h!)
#define INCL_EQF_TM                    // general Transl. Memory functions
#define DLLIMPORTRESMOD              // resource module handle imported from DLL

#include "EQF.H"
#include "EQFTOPS.H"
#include "tm.h"
#include "EQFFUNCI.H"
#include "EQFSTART.H"                  // for TwbGetCheckProfileData
#include "core/utilities/FilesystemWrapper.h"
#include "core/utilities/FilesystemHelper.h"

#define INCL_EQFMEM_DLGIDAS            // include dialog IDA definitions
#include "EQFSETUP.H"                  // directory names
//#include "EQFRPT.H"
//#include "EQFSERNO.H"

#include "core/document/InitPlugins.h"
#include "core/pluginmanager/PluginManager.h"    // Add for P403138
#include "core/utilities/LanguageFactory.H"
#include "core/utilities/Property.h"
#include "core/utilities/LogWrapper.h"
#include "core/utilities/ThreadingWrapper.h"
#include "core/utilities/EncodingHelper.h"
#include "win_types.h"

//#define SESSIONLOG
//#define DEBUGAPI

#ifdef DEBUGAPI
  FILE *hLog = NULL;
  char szLogFile[512];
  BOOL fLog = FALSE;
  int iLogLevel = 0;
  #define EQFAPI_TRIGGER "EQFAPI.TRG"
  #define EQFAPI_LOGFILE "EQFAPI.LOG"
#endif

#ifdef DEBUGAPI
  #define LOGFORCEWRITE() if ( fLog ) { fclose(hLog); hLog = fopen( szLogFile, "a" ); }
#else
  #define LOGFORCEWRITE()
#endif

#ifdef DEBUGAPI
  #define LOGWRITE1( p1 ) { if ( fLog && hLog ) { fprintf( hLog, p1 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE1( p1 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE2( p1, p2 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE2( p1, p2 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE3( p1, p2, p3 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE3( p1, p2, p3 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE4( p1, p2, p3, p4 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3, p4 ); LOGFORCEWRITE(); }}
#else
  #define LOGWRITE4( p1, p2, p3, p4 )
#endif

#ifdef DEBUGAPI
  #define LOGWRITE5( p1, p2, p3, p4, p5 ) { if ( fLog && hLog ) { fprintf( hLog, p1, p2, p3, p4, p5 ); LOGFORCEWRITE(); } }
#else
  #define LOGWRITE5( p1, p2, p3, p4, p5 )
#endif

#ifdef DEBUGAPI
  #define LOGPARMSTRING( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=\"%s\"\n", name, (value) ? value : "<NULL>" ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMSTRING( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMSTRINGW( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %S=\"%s\"\n", name, (value) ? value : "<NULL>" ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMSTRINGW( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMCHAR( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=\'%c\'\n", name, (value) ? value : ' ' ); LOGFORCEWRITE(); } }
#else
  #define LOGPARMCHAR( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMOPTION( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%8.8lX\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMOPTION( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMUSHORT( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%u\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMUSHORT( name, value )
#endif

#ifdef DEBUGAPI
  #define LOGPARMLONG( name, value ) { if ( fLog && hLog ) { fprintf( hLog, "  %s=%ld\n", name, value ); LOGFORCEWRITE(); }}
#else
  #define LOGPARMLONG( name, value )
#endif


#ifdef DEBUGAPI
  #define LOGCLOSE() { if ( hLog ) fclose( hLog ); hLog = NULLHANDLE; }
#else
  #define LOGCLOSE()
#endif

// OtmMemoryService
// import a Translation Memory
USHORT EqfImportMem
(
  std::shared_ptr<EqfMemory> mem,                // Eqf session handle
  PSZ         pszInFile,               // fully qualified name of input file
  LONG        lOptions,                 // options for Translation Memory import
  PSZ         errorBuff
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  FCTDATA    Data;// = NULL;            // ptr to function data area

  // call TM import
  usRC = MemFuncImportMem( &Data, pszInFile, mem, lOptions );
  strcpy(errorBuff, Data.szError);

  return( usRC );
} /* end of function EqfImportMem */


 USHORT CreateSystemProperties( );

// OtmMemoryService
USHORT EqfStartSession
(
  PHSESSION   phSession                // ptr to callers Eqf session handle variable
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area
  CreateSystemProperties();

  // allocate internal data area
  UtlAlloc( (PVOID *)&pData, 0L, sizeof(FCTDATA), NOMSG );
  if ( pData != NULL )
  {
    pData->lMagicWord = FCTDATA_IDENTIFIER;
    pData->hwndErrMsg = HWND_FUNCIF;
//    FctBuildCheckSum( pData, &pData->lCheckSum );
    pData->fComplete = TRUE;   // no incomplete function yet
  }
  else
  {
    T5LOG(T5ERROR) << "EqfStartSession():: Not enought memory for pData";

    LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  // initialize utilities
  if ( usRC == NO_ERROR )
  {
    UtlSetUShort( QS_RUNMODE, FUNCCALL_RUNMODE );
    UtlSetUShort( QS_PROGRAMID, NONDDEAPI_PROGID );

    if ( !UtlInitUtils( NULLHANDLE ) )
    {
      LOG_AND_SET_RC(usRC, T5INFO, ERROR_READ_SYSTEMPROPERTIES);
    } /* endif */
  } /* endif */

  // get profile data and initialize error handler
  if ( usRC == NO_ERROR )
  {
    BOOL fContinue = TwbGetCheckProfileData( pData->szMsgFile,
                                             pData->szSystemPropPath,
                                             pData->szEqfResFile );

    //  Initialize error handler to allow handling of error messages
    UtlInitError( NULLHANDLE, HWND_FUNCIF, (HWND)&(pData->LastMessage),
                  pData->szMsgFile );

    //if ( !fContinue )
    {
    //  T5LOG(T5ERROR) << "EqfStartSession():: fContinue is false";
    //  usRC = 1;
    } /* endif */
  } /* endif */

  // set directory strings and main drive
  if ( usRC == NO_ERROR )
  {
    PPROPSYSTEM  pPropSys = GetSystemPropPtr();

    if ( pPropSys != NULL )
    {
      UtlSetString( QST_PRIMARYDRIVE, pPropSys->szPrimaryDrive );
      UtlSetString( QST_LANDRIVE,     pPropSys->szLanDrive );
      UtlSetString( QST_PROPDIR,      pPropSys->szPropertyPath );
      UtlSetString( QST_CONTROLDIR,   pPropSys->szCtrlPath );
      UtlSetString( QST_PROGRAMDIR,   pPropSys->szProgramPath );
      UtlSetString( QST_DICDIR,       pPropSys->szDicPath );
      UtlSetString( QST_MEMDIR,       pPropSys->szMemPath );
      UtlSetString( QST_TABLEDIR,     pPropSys->szTablePath );
      UtlSetString( QST_LISTDIR,      pPropSys->szListPath );
      UtlSetString( QST_SOURCEDIR,    pPropSys->szDirSourceDoc );
      UtlSetString( QST_SEGSOURCEDIR, pPropSys->szDirSegSourceDoc );
      UtlSetString( QST_SEGTARGETDIR, pPropSys->szDirSegTargetDoc );
      UtlSetString( QST_TARGETDIR,    pPropSys->szDirTargetDoc );
      UtlSetString( QST_DLLDIR,       pPropSys->szDllPath );
      UtlSetString( QST_MSGDIR,       pPropSys->szMsgPath );
      UtlSetString( QST_EXPORTDIR,    pPropSys->szExportPath );
      UtlSetString( QST_BACKUPDIR,    pPropSys->szBackupPath );
      UtlSetString( QST_IMPORTDIR,    pPropSys->szDirImport );
      UtlSetString( QST_COMMEMDIR,    pPropSys->szDirComMem );
      UtlSetString( QST_COMPROPDIR,   pPropSys->szDirComProp );
      UtlSetString( QST_COMDICTDIR,   pPropSys->szDirComDict );
 
      UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
      UtlSetString( QST_MTLOGPATH,    "MTLOG" );
      UtlSetString( QST_DIRSEGMTDIR, "MT" );
      UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
      UtlSetString( QST_PRTPATH,      pPropSys->szPrtPath );
      if ( pPropSys->szWinPath[0] )
      {
        UtlSetString( QST_WINPATH,    pPropSys->szWinPath );
      }
      else
      {
        UtlSetString( QST_WINPATH,      WINDIR );
      } /* endif */
      if ( pPropSys->szEADataPath[0] )
      {
        UtlSetString( QST_EADATAPATH,   pPropSys->szEADataPath );
      }
      else
      {
        UtlSetString( QST_EADATAPATH,   EADATADIR );
      } /* endif */
      UtlSetString( QST_LOGPATH,           "LOGS" );
      UtlSetString( QST_XLIFFPATH,         "XLIFF" );
      UtlSetString( QST_METADATAPATH,      "METADATA" );
      UtlSetString( QST_JAVAPATH,          "JAVA" );
      UtlSetString( QST_MISCPATH,          "MISC" );

      if ( pPropSys->szPluginPath[0] )
      {
        UtlSetString( QST_PLUGINPATH,   pPropSys->szPluginPath );
      }
      else
      {
        UtlSetString( QST_PLUGINPATH,        "PLUGINS" );
      }
      UtlSetString( QST_ENTITY,            "ENTITY" );
      UtlSetString( QST_REMOVEDDOCDIR,     "REMOVED" );

      UtlSetString( QST_ORGEQFDRIVES, pPropSys->szDriveList );
      UtlSetString( QST_VALIDEQFDRIVES, pPropSys->szDriveList );

      // force a refresh of QST_VALIDEQFDRIVES string
      {
        CHAR  szDriveList[MAX_DRIVELIST];
        UtlGetCheckedEqfDrives( szDriveList );
      }

      // set fuzziness limits
      if ( !pPropSys->lSmallLkupFuzzLevel )  
          pPropSys->lSmallLkupFuzzLevel = 3300;
      UtlSetULong( QL_SMALLLOOKUPFUZZLEVEL,    pPropSys->lSmallLkupFuzzLevel );
      if ( !pPropSys->lMediumLkupFuzzLevel ) 
          pPropSys->lMediumLkupFuzzLevel = 3300;
      UtlSetULong( QL_MEDIUMLOOKUPFUZZLEVEL,   pPropSys->lMediumLkupFuzzLevel );
      if ( !pPropSys->lLargeLkupFuzzLevel )  
          pPropSys->lLargeLkupFuzzLevel = 3300;
      UtlSetULong( QL_LARGELOOKUPFUZZLEVEL,    pPropSys->lLargeLkupFuzzLevel );

      if ( !pPropSys->lSmallFuzzLevel )  
          pPropSys->lSmallFuzzLevel = 3300;
	    UtlSetULong( QL_SMALLFUZZLEVEL,    pPropSys->lSmallFuzzLevel );
	    if ( !pPropSys->lMediumFuzzLevel ) 
          pPropSys->lMediumFuzzLevel = 3300;
	    UtlSetULong( QL_MEDIUMFUZZLEVEL,   pPropSys->lMediumFuzzLevel );
	    if ( !pPropSys->lLargeFuzzLevel )  
          pPropSys->lLargeFuzzLevel = 3300;
      UtlSetULong( QL_LARGEFUZZLEVEL,    pPropSys->lLargeFuzzLevel );

      // set SGML/DITA processing flag
      UtlSetUShort( QS_SGMLDITAPROCESSING, (USHORT)!pPropSys->fNoSgmlDitaProcessing );

      UtlSetUShort( QS_ENTITYPROCESSING, (USHORT)pPropSys->fEntityProcessing );

      UtlSetUShort( QS_MEMIMPMRKUPACTION, (USHORT)pPropSys->usMemImpMrkupAction );

    }
    else
    {
      T5LOG(T5ERROR) << "EqfStartSession()::ERROR_READ_SYSTEMPROPERTIES";
      // access to system properties failed
      LOG_AND_SET_RC(usRC, T5INFO, ERROR_READ_SYSTEMPROPERTIES);
    } /* endif */
  } /* endif */


  // set caller's session handle
  if ( usRC == NO_ERROR )
  {

  T5LOG( T5DEBUG) << "==EQFSTARTSESSION==\n  Starting plugins...\n" ;


  // initialie plugins
  if ( usRC == NO_ERROR )
  {
    std::string strPluginPath = FilesystemHelper::GetOtmDir();

		usRC = InitializePlugins( (PSZ)strPluginPath.c_str() );    // add return value for P402974
    // Add for P403115;
    if (usRC != PluginManager::ePluginExpired)
    {
      PluginManager* thePluginManager = PluginManager::getInstance();
    }
    else
    {
      T5LOG(T5ERROR) << "EqfSessioStart()::usRC = ERROR_PLUGIN_EXPIRED";
      usRC = ERROR_PLUGIN_EXPIRED;
    }
    // Add end
  }

  T5LOG( T5INFO) << "   ...Plugins have been started\n" ;
    {
      char szBuf[10];
      int i = _getpid();
      sprintf( szBuf, "%ld", i );
      T5LOG( T5INFO) << "EqfStartSession: Process ID is " << szBuf;
    }
    *phSession = (LONG)pData;
  }
  else
  {
    *phSession = NULLHANDLE;
  } /* endif */

  T5LOG( T5INFO) << "  RC=" << usRC;

  return( usRC );
} /* end of function EqfStartSession */

// OtmMemoryService
USHORT EqfGetLastError
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  PSZ         pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
  USHORT      usRC = NO_ERROR;         // function return code

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    *pusRC = (USHORT)pData->LastMessage.sErrNumber;
    strncpy( pszMsgBuffer, pData->LastMessage.chMsgText, usBufSize );
    pszMsgBuffer[usBufSize-1] = EOS;

    pData->LastMessage.sErrNumber = 0;
    pData->LastMessage.chMsgText[0] = EOS;
  } /* endif */

  return( usRC );
} /* end of function EqfGetLastError */

// OtmMemoryService
USHORT EqfGetLastErrorW
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  wchar_t    *pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
  USHORT      usRC = NO_ERROR;         // function return code

  // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( usRC == NO_ERROR )
  {
    *pusRC = (USHORT)pData->LastMessage.sErrNumber;
    //MultiByteToWideChar( CP_OEMCP, 0, pData->LastMessage.chMsgText, -1, pszMsgBuffer, usBufSize );

    pData->LastMessage.sErrNumber = 0;
    pData->LastMessage.chMsgText[0] = EOS;
  } /* endif */

  return( usRC );
} /* end of function EqfGetLastErrorW */

// FctValidateSession: check and convert a session handle to a FCTDATA pointer
USHORT FctValidateSession
(
  HSESSION    hSession,                // session handle being validated
  PFCTDATA   *ppData                   // address of caller's FCTDATA pointer
)
{
  // convert handle to pointer
  PFCTDATA    pData = (PFCTDATA)hSession;;                   // pointer to FCTDATA area
  USHORT      usRC = NO_ERROR;         // function return code

  // test magic word and checksum
  if ( pData == NULL )
  {
    T5LOG(T5ERROR) << "FctValidateSession()::ERROR_INVALID_SESSION_HANDLE, pData == NULL";
    LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SESSION_HANDLE);
  }
  else if ( pData->lMagicWord != FCTDATA_IDENTIFIER )
  {
    T5LOG(T5ERROR) << "FctValidateSession()::ERROR_INVALID_SESSION_HANDLE, pData->lMagicWord != FCTDATA_IDENTIFIER ";
    LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SESSION_HANDLE);
  }
  
  if ( usRC == NO_ERROR )
  {
     *ppData = pData;
  } /* endif */
  T5LOG( T5DEBUG) << "FctValidateSession() usRC = " << usRC;
  return( usRC );
} /* end of function FctValidateSession */





// OtmMemoryService
/*! \brief Get the OpenTM2 language name for a ISO language identifier
\param hSession the session handle returned by the EqfStartSession call
\param pszISOLang pointer to ISO language name (e.g. en-US )
\param pszOpenTM2Lang buffer for the OpenTM2 language name
\returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetOpenTM2Lang
(
  HSESSION         hSession,
  char*         pszISOLang,
  char*         pszOpenTM2Lang,
  bool*       pfPrefered
)
{
  PFCTDATA    pData = NULL;            // ptr to function data area
                                       // validate session handle
  USHORT usRC = FctValidateSession( hSession, &pData );

  if ( (usRC == NO_ERROR ) && (pszISOLang == NULL) ) 
  {
    char*  pszParm = "pointer to ISO language id";
    LOG_AND_SET_RC(usRC, T5INFO, DDE_MANDPARAMISSING);
    T5LOG(T5ERROR) <<"EqfGetOpenTM2Lang()::DDE_MANDPARAMISSING, (usRC == NO_ERROR ) && (pszISOLang == NULL), pszParam =" << pszParm;
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    char* pszParm = "buffer for OpenTM2 language name";
    LOG_AND_SET_RC(usRC, T5INFO, DDE_MANDPARAMISSING);
    T5LOG(T5ERROR) << "EqfGetOpenTM2Lang()::DDE_MANDPARAMISSING, (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL), pszParam = " << pszParm;
  } /* endif */

  // use the language factory to process the request
  if ( usRC == NO_ERROR )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getOpenTM2NameFromISO( pszISOLang, pszOpenTM2Lang, pfPrefered );
  } /* endif */

  return( usRC );
}

// OtmMemoryService
/*! \brief Get the ISO language identifier for a OpenTM2 language name
\param hSession the session handle returned by the EqfStartSession call
\param pszOpenTM2Lang pointer to the OpenTM2 language name
\param pszISOLang pointer to a buffer for the ISO language identifier
\returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetIsoLang
(
  HSESSION    hSession,
  PSZ         pszOpenTM2Lang,
  PSZ         pszISOLang
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFCTDATA    pData = NULL;            // ptr to function data area

                                       // validate session handle
  usRC = FctValidateSession( hSession, &pData );

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    PSZ pszParm = "pointer to OpenTM2 language name";
    T5LOG(T5ERROR) <<  ":: DDE_MANDPARAMISSING ::" << pszParm;
    LOG_AND_SET_RC(usRC, T5INFO, DDE_MANDPARAMISSING);
  } /* endif */

  if ( (usRC == NO_ERROR ) && (pszOpenTM2Lang == NULL) ) 
  {
    PSZ pszParm = "buffer for ISO language id";
    T5LOG(T5ERROR) << ":: DDE_MANDPARAMISSING :: " << pszParm;
    LOG_AND_SET_RC(usRC, T5INFO, DDE_MANDPARAMISSING);
  } /* endif */

  // use the language factory to process the request
  if ( usRC == NO_ERROR )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getISOName( pszOpenTM2Lang, pszISOLang );
  } /* endif */

  return( usRC );
}
