//+----------------------------------------------------------------------------+
//|  EQFMEMCD.C - TM create dialog and TM properties dialog                    |
//+----------------------------------------------------------------------------+
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Author                                   :   Markus Conrad                |
//|  Code for creation of remote TMs added by :   Stefan Doersam               |
//+----------------------------------------------------------------------------+
//|  Description  : This program provides the end user dialog                  |
//|                 and the function to create a translation memory            |
//|                 and the new TM property dialog                             |
//+----------------------------------------------------------------------------+
//|  Entry Points : MEMCREATEDLG( HWND, USHORT, MPARAM, MPARAM )               |
//|                 MEMPROPDLG( HWND, USHORT, MPARAM, MPARAM )                 |
//+----------------------------------------------------------------------------+
//|  Externals    :                                                            |
//+----------------------------------------------------------------------------+
//|  Internals    : MemCreateCommand                                           |
//|                 MemCreateControl                                           |
//|                 UpdateLastUsedProp                                         |
//|                 PrepTmCreate                                               |
//|                 HandleOtherControlMsgs                                     |
//|                 SetupDriveIcons                                            |
//|                 DeleteDriveIcons                                           |
//|                 DisplayDriveIcons                                          |
//|                 DisplayServerDriveIcons                                    |
//|                 ProcessWM_COMMAND                                          |
//|                 InitMemCreateDlg                                           |
//|                 SelectLocalRB                                              |
//|                 iCompare                                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities

#include "../pluginmanager/PluginManager.h"
#include "tm.h"
#include "EQFMORPH.H"
#include "LogWrapper.h"

#undef _WPTMIF                         // we don't care about WP I/F
//#include "eqfhelp.id"                  // help resource IDs
//#include "eqfhlp1.h"                   // first part of help tables
//#include "eqfmsg.htb"                          // message help table


#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFMEM.ID" // Translation Memory IDs
// #include <TIME.H>                 // C time functions
#include <EQFQDAM.H>            // Low level TM access functions

// the name of the default memory plugin
#define DEFAULT_MEMORY_PLUGIN "EqfMemoryPlugin" 

/**********************************************************************/
/* IDA structure for TM Create Dialog                                 */
/**********************************************************************/
typedef struct _TMX_CREATE_IDA
{
  PSZ    pszParms;                         // ptr to input parameter, will receive new memory name on exit
  CHAR   szMemName[MAX_LONGFILESPEC];      // TM name as entered by user
  CHAR   szSourceLang[MAX_LANG_LENGTH];    //language name
  CHAR   szPlugin[MAX_LONGFILESPEC];       // name of selected plugin
  std::vector<OtmMemoryPlugin *> vMemoryPlugins; // list of available memory plugins
  CHAR   szUserid[MAX_USERID];             //LAN Userid of TM: if local '\0'
  OtmPlugin *pSelectedPlugin;              // pointer to selected plugin
  CHAR   szDriveList[MAX_DRIVELIST];       // list of drives to be displayed 
  CHAR   szLastDriveList[MAX_DRIVELIST];   // list of currently displayed drives

  CHAR   szMemPath[MAX_EQF_PATH];          //D:\EQF\MEM\, TM path
  CHAR   szMemShortName[MAX_LONGFILESPEC]; // TM short name
//CHAR   szMemPathMemName[MAX_EQF_PATH];   //D:\EQF\MEM\TESTTM concat of szMemPath,szMemName
  CHAR   szFullMemName[MAX_EQF_PATH];      //D:\EQF\MEM\TESTTM.TMD
  CHAR   szMemPropPath[MAX_EQF_PATH];      //D:\EQF
  CHAR   szMemPropName[MAX_FILESPEC];      //TESTTM.MEM, TM property name
  CHAR   szServer[MAX_SERVER_NAME];        //server Name of TM or \0 if TM is local
  CHAR   szMemDesc[MAX_MEM_DESCRIPTION];   //description of memory database
  CHAR   szTemp[MAX_PATH144];              //temp work string
  CHAR   szTemp2[MAX_PATH144];             //temp work string
  USHORT usCreate;                         //if FALSE the TM create dialog failed
  USHORT usLocation;                       //TM_LOCAL,TM_REMOTE, TM_LOCALREMOTE,
} TMX_CREATE_IDA, * PTMX_CREATE_IDA;

// create a new Translation Memory using the function I/F
USHORT MemFuncCreateMem
(
const char*         pszMemName,              // name of new Translation Memory
const char*         pszDescription,          // description for new Translation Memory or NULL
const char*         pszSourceLanguage,       // Translation Memory source language
LONG                lOptions                 // type of new Translation Memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  char*       pszParm;                 // pointer for error parameters
  char        szEmpty[2];
  TMManager *pFactory = TMManager::GetInstance();

  szEmpty[0] = EOS;
  {
    T5LOG(T5DEBUG) << "MemFuncCreateMem( pszMemName = "<<pszMemName <<", pszDescription = "<<pszDescription<<", pszSourceLanguage = "<<pszSourceLanguage<<" )";
  }
  // check required parameters
  if ( usRC == NO_ERROR )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      usRC = TMT_MANDCMDLINE;
      T5LOG(T5ERROR) "::MemFuncCreateMem()::TMT_MANDCMDLINE";
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    if ( (pszSourceLanguage == NULL) || (*pszSourceLanguage == EOS) )
    {
      T5LOG(T5ERROR) << "::MemFuncCreateMem()::ERROR_NO_SOURCELANG";
      usRC = ERROR_NO_SOURCELANG;
    } /* endif */
  } /* endif */

  // Check memory name syntax
  if ( usRC == NO_ERROR )
  { 
    if ( !UtlCheckLongName( (PSZ) pszMemName ))
    {
      pszParm = (PSZ) pszMemName;
      T5LOG(T5ERROR) <<   "::ERROR_INV_LONGNAME::" << pszParm;
      usRC = ERROR_MEM_NAME_INVALID;
    } /* endif */
  } /* endif */

  // check if TM exists already
  if ( usRC == NO_ERROR )
  {
    int logLevel = T5Logger::GetInstance()->suppressLogging();
    if ( pFactory->exists( NULL, pszMemName ) )
    {
      T5Logger::GetInstance()->desuppressLogging(logLevel);
      PSZ pszParam = (PSZ)pszMemName;
      T5LOG(T5ERROR) <<  "::ERROR_MEM_NAME_EXISTS:: TM with this name already exists: " << pszParam;
      usRC = ERROR_MEM_NAME_EXISTS;
    }
    T5Logger::GetInstance()->desuppressLogging(logLevel);
  } /* endif */


  // check if source language is valid
  if ( usRC == NO_ERROR )
  {
    SHORT sID = 0;    
    if ( MorphGetLanguageID( (PSZ)pszSourceLanguage, &sID ) != MORPH_OK )
    {
      usRC = ERROR_PROPERTY_LANG_DATA;
      //T5LOG( T5ERROR) <<"  ERROR_PROPERTY_LANG_DATA, MB_CANCEL, 1,  &((PSZ)pszSourceLanguage) "";
    } /* endif */
  } /* endif */


  // Check specified options
  if ( usRC == NO_ERROR)
  {
    if ( lOptions == 0L )
    {
      lOptions = LOCAL_OPT;
    }
    else if ( lOptions == LOCAL_OPT )
    {
      // O.K. option is valid
    }
    else
    {
      T5LOG(T5FATAL) <<   "MemFuncCreateMem supports only local_opt, rc = WRONG_OPTIONS_RC";
      usRC = WRONG_OPTIONS_RC;
    } /* endif */
  } /* endif */


  // create memory database
  BOOL fOK = (usRC==NO_ERROR);
  OtmMemory     *pMemory  = NULL; 

  if (fOK)
  {
    int iRC;
    char szPlugin[256];

    strcpy( szPlugin, pFactory->getDefaultMemoryPlugin() );    

    T5LOG( T5DEBUG) << "MemFuncCreateMem():: create the memory" ;    
    pMemory = pFactory->createMemory( szPlugin, pszMemName, ( pszDescription == NULL ) ? szEmpty : pszDescription, pszSourceLanguage,NULL, false, &iRC );
    if ( pMemory == NULL )
    {
      T5LOG(T5ERROR) << "MemFuncCreateMem()::pMemory == NULL" ;
      fOK = FALSE;
    } 
  } 

  // get memory object name
  if ( fOK )
  {
    char szObjName[MAX_LONGFILESPEC];
    pFactory->getObjectName( pMemory, szObjName, sizeof(szObjName) );
  } 

  //--- Tm is created. Close it. 
  if ( pMemory != NULL )
  {
    T5LOG( T5INFO ) << "MemFuncCreateMem()::Tm is created. Close it";
    pFactory->closeMemory( pMemory );
    pMemory = NULL;
  }
  
  if(usRC == 0){
    usRC = (!fOK);
  }
  T5LOG( T5INFO) << "MemFuncCreateMem() done, usRC = " << usRC;
  return( usRC );
} /* end of function MemFuncCreateMem */

