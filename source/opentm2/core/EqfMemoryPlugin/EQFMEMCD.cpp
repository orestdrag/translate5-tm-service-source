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
#include "EqfMemoryPlugin.h"

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
  
  return( -1 );
} /* end of function */

