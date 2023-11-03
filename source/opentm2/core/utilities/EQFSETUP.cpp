//+----------------------------------------------------------------------------+
//|EQFSETUP.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:    Setup utilities for MAT and MAT TM server.                  |
//|                These utilities are used by EQF*INST.                       |
//|                Define INSTCOM to compile for MAT TM server                 |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|   SetupMAT         - Setup MAT environment (OS2.INI+directories+properties)|
//|   SetupDemo        - Setup EQF\EXPORT path                                 |
//|   BuildPath        - Build path to MAT directories                         |
//|   InstReadSysProps - Read system property file into meory                  |
//|   InstLocateString - Locate a given string                                 |
//|   InstMatch        - Test if pszS1 starts with string pszS2                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|   CreateSystemProperties     - create system properties                    |
//|   CreateFolderListProperties - create folderlist properties                |
//|   CreateImexProperties       - create document import/export properties    |
//|   CreateDictProperties       - create dictionary list properties           |
//|   CreateEditProperties       - create editor properties                    |
//|   SetupCreateDir             - create a MAT directory                      |
//|   DeletePropFile             - delete a property file                      |
//|   WritePropFile              - write properties to a file                  |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
#define STANDARDEDIT

/**********************************************************************/
/* Include files                                                      */
/**********************************************************************/
#define INCL_VIO
#define INCL_AVIO
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM
#include <EQF.H>                  // General Translation Manager include file
#include "EQFSETUP.H"                   // MAT setup functions
//#include <EQFSERNO.H>                   // Serial number
#ifdef __linux__
    #include <linux/limits.h>
#endif // __linux__
#ifdef _WINDOWS
  #include <direct.h>
#endif
#include "Property.h"
#include "FilesystemHelper.h"
#include "FilesystemWrapper.h"
#include "LogWrapper.h"

/**********************************************************************/
/* Global Variables                                                   */
/**********************************************************************/
static CHAR  chTempPath[CCHMAXPATH+1];// buffer for path names
static LONG  cyDesktop;               // desktop width in pels
static LONG  cxDesktop;               // desktop height in pels
static LONG  cyDesktopDiv20;          // desktop width / 20
static LONG  cxDesktopDiv20;          // desktop height / 20

CHAR    szLineBuf[MAX_STR_LEN];        // line buffer for CONFIG.SYS lines
PSZ     pConfLine[1000];               // table holding lines of CONFIG.SYS
BOOL    fConfChanged;                  // CONGIG.SYS-needs-update flag
CHAR    chBootDrive[4];                // boot drive (to find CONFIG.SYS)

PPROPSYSTEM InstReadSysProps( VOID );

USHORT UpdateTMProp( PSZ   pszFullFileName, CHAR  chDrive );
USHORT UpdateDictProp( PSZ   pszFullFileName, CHAR  chDrive );
USHORT UpdateDocumentProp( PSZ   pszFullFileName, CHAR  chDrive );

#ifdef _WINDOWS
  #define DosOpen(a,b,c,d,e,f,g,h)    UtlOpen(a,b,c,d,e,f,g,h,FALSE)
  #define DosFindFirst(a,b,c,d,e,f,g) UtlFindFirst(a,b,c,d,e,f,g,FALSE)
  #define DosFindClose( a )           UtlFindClose( a, FALSE )
  #define DosQFileInfo( a, b, c, d )  UtlQFileInfo( a, b, c, d, FALSE )
#endif

//+----------------------------------------------------------------------------+
//|Function name:     SetupMat                                                 |
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     SetupMAT( HAB hab, CHAR chPrimaryDrive,                  |
//|                             PSZ pszSecondaryDrives, CHAR chLanDrive        |
//+----------------------------------------------------------------------------+
//|Description:       Create EQF environment: create required directories,     |
//|                   create property files and add entries to OS2 profile     |
//|                   OS2.INI.                                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   HAB  hab                main process anchor block handle |
//|                   CHAR chPrimaryDrive     primary MAT drive                |
//|                   PSZ  pszSecondaryDrives list of secondary MAT drives     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Side effects:      Directories are created, property files are created and  |
//|                   OS2.INI is updated.                                      |
//+----------------------------------------------------------------------------+
//|Function flow:     delete old entries of MAT from OS2.INI;                  |
//|                   add MAT entries to OS2.INI;                              |
//|                   create all necessary directories on primary drive;       |
//|                   while there are secondary drives;                        |
//|                      create all necessary directories on secondary drive;  |
//|                   endif;                                                   |
//|                   delete old property files;                               |
//|                   create new property files;                               |
//+----------------------------------------------------------------------------+



//+----------------------------------------------------------------------------+
//|Function name:     CreateSystemProperties                                   |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateSystemProperties( CHAR chPrimaryDrive,             |
//|                                           PSZ  pszSecondaryDrives,         |
//|                                           CHAR chLanDrive )                |
//+----------------------------------------------------------------------------+
//|Description:       Create system properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//|                   PSZ    pszSecondaryDrives   MAT secondary drives         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   add editor information;                                  |
//|                   if old properties exist;                                 |
//|                      copy selected data from old properties;               |
//|                   endif;                                                   |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
PPROPSYSTEM GetSystemPropInstance(){
  static PROPSYSTEM _instance;
  return &_instance;
}

USHORT CreateSystemProperties()
{
    PPROPSYSTEM  pSysProps = 0;        // buffer for new system properties
    PSZ          pszEditor = 0;        // ptr for editor name processing
    PSZ          pszTemp = 0;          // general purpose pointer
    USHORT       usRC = 0;             // function return code

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pSysProps = GetSystemPropInstance();
    memset(pSysProps, NULC, sizeof(PROPSYSTEM));
    

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    
    strcpy(pSysProps->PropHead.szName, SYSTEM_PROPERTIES_NAME);
    
    pSysProps->PropHead.usClass = PROP_CLASS_SYSTEM;
    pSysProps->PropHead.chType  = PROP_TYPE_INSTANCE;
    
    /******************************************************************/
    /* Fill rest of properties with default values                    */
    /******************************************************************/
    strcpy( pSysProps->szPropertyPath,    PROPDIR );
    strcpy( pSysProps->szProgramPath,     WINDIR );
    strcpy( pSysProps->szDicPath,         DICTDIR );
    strcpy( pSysProps->szMemPath,         MEMDIR );
    strcpy( pSysProps->szTablePath,       TABLEDIR );
    strcpy( pSysProps->szCtrlPath,        CTRLDIR );
    strcpy( pSysProps->szDllPath,         DLLDIR );
    strcpy( pSysProps->szListPath,        LISTDIR );
    strcpy( pSysProps->szMsgPath,         MSGDIR );
    strcpy( pSysProps->szPrtPath,         PRTDIR );
    strcpy( pSysProps->szExportPath,      EXPORTDIR );
    strcpy( pSysProps->szBackupPath,      BACKUPDIR );
    strcpy( pSysProps->szDirSourceDoc,    SOURCEDIR );
    strcpy( pSysProps->szDirSegSourceDoc, SEGSOURCEDIR );
    strcpy( pSysProps->szDirTargetDoc,    TARGETDIR );
    strcpy( pSysProps->szDirSegTargetDoc, SEGTARGETDIR );
    strcpy( pSysProps->szDirImport,       IMPORTDIR );
    strcpy( pSysProps->szDirComMem,       COMMEMDIR );
    strcpy( pSysProps->szDirComDict,      COMDICTDIR );
    strcpy( pSysProps->szDirComProp,      COMPROPDIR );
    strcpy( pSysProps->szWinPath,         WINDIR );
    sprintf( pSysProps->RestartFolderLists, "%s/%s", PATH, DEFAULT_FOLDERLIST_NAME );
    sprintf( pSysProps->FocusObject, "%s/%s", PATH, DEFAULT_FOLDERLIST_NAME );
    sprintf( pSysProps->RestartMemory, "%s", MEMORY_PROPERTIES_NAME );
    sprintf( pSysProps->RestartDicts, "%s/%s", PATH, DICT_PROPERTIES_NAME );
    //pSysProps->fUseIELikeListWindows = TRUE;
    strcpy( pSysProps->szPluginPath,      PLUGINDIR );
  


    /******************************************************************/
    /* Add editor information                                         */
    /******************************************************************/
    pszEditor = EDITOR_PROPERTIES_NAME;
    pszTemp = strchr( pszEditor, '.' );
    strncpy( pSysProps->szDefaultEditor, pszEditor, pszTemp - pszEditor );

    return (usRC);
}


