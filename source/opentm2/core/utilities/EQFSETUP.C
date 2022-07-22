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
#include <EQFSERNO.H>                   // Serial number
#ifdef __linux__
    #include <linux/limits.h>
#endif // __linux__
#ifdef _WINDOWS
  #include <direct.h>
#endif
#include "PropertyWrapper.H"
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
USHORT UpdateFolderProp( PSZ   pszFullFileName, CHAR  chDrive );

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

#ifdef __linux__



int init_properties(){
  if (int rc = properties_init()) {
        LogMessage2(FATAL,"Error in init_properties::Failed to initialize property file", toStr(rc).c_str());
        return rc;
    }

    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");

    properties_add_str(KEY_Vers, version);
    properties_add_str(KEY_SYSLANGUAGE, DEFAULT_SYSTEM_LANGUAGE);
    properties_add_str(KEY_SysProp, SYSTEM_PROPERTIES_NAME);    
    properties_add_str("CurVersion", version);
    return 0;

}


int SetupMAT() {
    properties_turn_off_saving_in_file(); // we don't have path for file to save till the end of this function

    init_properties();

    auto homeDir = filesystem_get_home_dir();
      
    auto otmDir = filesystem_get_otm_dir();    
    std::string otmPath = otmDir +'/' + SYSTEM_PROPERTIES_NAME;
    CreateSystemProperties(otmPath.c_str());

    properties_turn_on_saving_in_file();

    if(!homeDir.empty())
      properties_add_str(KEY_Drive, homeDir.c_str());
    //properties_deinit();
    return 0;
}
#endif // __linux__

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
#ifdef __linux__
USHORT CreateSystemProperties(const char* pszPath)
{
    PPROPSYSTEM  pSysPropsOld = 0;     // buffer for old system properties
    PPROPSYSTEM  pSysProps = 0;        // buffer for new system properties
    PSZ          pszEditor = 0;        // ptr for editor name processing
    PSZ          pszTemp = 0;          // general purpose pointer
    USHORT       usRC = 0;             // function return code

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pSysProps = (PPROPSYSTEM)malloc(sizeof(PROPSYSTEM));
    if ( pSysProps )
    {
        memset(pSysProps, NULC, sizeof(PROPSYSTEM));
    }
    else
    {
        LogMessage(ERROR, "CreateSystemProperties()::ERROR_NOT_ENOUGH_MEMORY");
        usRC = ERROR_NOT_ENOUGH_MEMORY;
        return usRC;
    }

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
    /* Set intial restore size to 4/5 of desktop size and center      */
    /* window inside desktop window                                   */
    /******************************************************************/
 
    pSysProps->Swp.x  = (SHORT) (cxDesktop / 5L / 2L);
    pSysProps->Swp.y  = (SHORT) (cyDesktop / 5L / 2L);
    pSysProps->Swp.cx = (SHORT) (cxDesktop * 4L / 5L);
    pSysProps->Swp.cy = (SHORT) (cyDesktop * 4L / 5L);
    pSysProps->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                         EQF_SWP_SHOW | EQF_SWP_MAXIMIZE;
    pSysProps->SwpDef = pSysProps->Swp;


    /******************************************************************/
    /* Add editor information                                         */
    /******************************************************************/
    pszEditor = EDITOR_PROPERTIES_NAME;
    pszTemp = strchr( pszEditor, '.' );
    strncpy( pSysProps->szDefaultEditor, pszEditor, pszTemp - pszEditor );
    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    
    WritePropFile( pszPath, pSysProps, sizeof( PROPSYSTEM) );
    free( pSysProps );
    
    return (usRC);
}
#endif // __linux__

#ifdef _WIN32
USHORT CreateSystemProperties
(
  CHAR chPrimaryDrive,                 // MAT primary drive
  PSZ  pszSecondaryDrives,             // MAT secondary drives
  CHAR chLanDrive                      // MAT LAN drive
)
{
    PPROPSYSTEM  pSysPropsOld = 0;     // buffer for ol system properties
    PPROPSYSTEM  pSysProps = 0;        // buffer for new system properties
    PSZ          pszEditor = 0;        // ptr for editor name processing
    PSZ          pszTemp = 0;          // general purpose pointer
    USHORT       usRC = 0;             // function return code

	chLanDrive;

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pSysProps    = (PPROPSYSTEM) malloc( sizeof( PROPSYSTEM) );
    if ( pSysProps )
    {
      memset( pSysProps, NULC, sizeof( PROPSYSTEM) );
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      SETPROPHEAD( pSysProps->PropHead, SYSTEM_PROPERTIES_NAME,
                   PROP_CLASS_SYSTEM );
    } /* endif */


    /******************************************************************/
    /* Fill rest of properties with default values                    */
    /******************************************************************/
    if ( !usRC )
    {
      sprintf( pSysProps->szPrimaryDrive, "%c:", chPrimaryDrive );
      sprintf( pSysProps->szLanDrive, "%c:", chPrimaryDrive );
      sprintf( pSysProps->szDriveList, "%c%s", chPrimaryDrive, pszSecondaryDrives );
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
      sprintf( pSysProps->RestartFolderLists, "\x15%c:\\%s\\%s",
               chPrimaryDrive, PATH, DEFAULT_FOLDERLIST_NAME );
      sprintf( pSysProps->FocusObject, "%c:\\%s\\%s",
               chPrimaryDrive, PATH, DEFAULT_FOLDERLIST_NAME );
      sprintf( pSysProps->RestartMemory, "\x15%s", MEMORY_PROPERTIES_NAME );
      sprintf( pSysProps->RestartDicts, "\x15%c:\\%s\\%s",
               chPrimaryDrive, PATH, DICT_PROPERTIES_NAME );
      pSysProps->fUseIELikeListWindows = TRUE;
      strcpy( pSysProps->szPluginPath,      PLUGINDIR );
    } /* endif */

    /******************************************************************/
    /* Set intial restore size to 4/5 of desktop size and center      */
    /* window inside desktop window                                   */
    /******************************************************************/
    if ( !usRC )
    {
      pSysProps->Swp.x  = (SHORT) (cxDesktop / 5L / 2L);
      pSysProps->Swp.y  = (SHORT) (cyDesktop / 5L / 2L);
      pSysProps->Swp.cx = (SHORT) (cxDesktop * 4L / 5L);
      pSysProps->Swp.cy = (SHORT) (cyDesktop * 4L / 5L);
      pSysProps->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                          EQF_SWP_SHOW | EQF_SWP_MAXIMIZE;
      pSysProps->SwpDef = pSysProps->Swp;
    } /* endif */

    /******************************************************************/
    /* Add editor information                                         */
    /******************************************************************/
    if ( !usRC )
    {
      pszEditor = EDITOR_PROPERTIES_NAME;
      pszTemp = strchr( pszEditor, '.' );
      strncpy( pSysProps->szDefaultEditor, pszEditor, pszTemp - pszEditor );
    } /* endif */

    /******************************************************************/
    /* Try to read old system properties                              */
    /******************************************************************/
    if ( !usRC )
    {
      pSysPropsOld = InstReadSysProps();
    } /* endif */

    /******************************************************************/
    /* Save some of the values from the old properties to the new ones*/
    /******************************************************************/
    if ( !usRC && pSysPropsOld )
    {
       strcpy( pSysProps->szServerList, pSysPropsOld->szServerList );

       free( pSysPropsOld );           // free storage used for old properties
    } /* endif */

    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, SYSTEM_PROPERTIES_NAME, pSysProps,
                     sizeof( PROPSYSTEM) );
      free( pSysProps );
    } /* endif */

    return( usRC );
} /* end of function CreateSystemProperties */
#endif //_WIN32

/* $KIT0890 A59 */
//+----------------------------------------------------------------------------+
//|Function name:     UpdateSystemProperties                                   |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     UpdateSystemProperties (CHAR chPrimaryDrive, CHAR chLan) |
//+----------------------------------------------------------------------------+
//|Description:       Update system properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//|                   CHAR   chLanDrive           MAT Lan drive                |
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
USHORT UpdateSystemProperties ( CHAR chPrimaryDrive, CHAR chLanDrive )
{
    PPROPSYSTEM  pSysProps = 0;        // buffer for system properties
    USHORT       usRc = 0;             // return code

	chLanDrive;

    /******************************************************************/
    /* Try to read old system properties                              */
    /******************************************************************/
    pSysProps = InstReadSysProps();

    /********************************************************************/
    /* Now fill properties with new contents for actual version of TM/2 */
    /********************************************************************/
    if ( pSysProps )
    {
      strcpy( pSysProps->szPrtPath,    PRTDIR );
      strcpy( pSysProps->szWinPath,    WINDIR );
      strcpy( pSysProps->szDirComDict, COMDICTDIR );
      sprintf( pSysProps->szLanDrive, "%c", chPrimaryDrive );
    } /* endif */

#ifdef _PTM
    /******************************************************************/
    /* Now activate the quick tour folder and set the focus onto it   */
    /******************************************************************/
    if ( pSysProps )
    {
      if ( strstr( pSysProps->RestartFolders, QUICKTOUR_FOL ) == NULL )
      {
        /* Quick Tour Folder not in Restart List, add it */
        CHAR   szQuickFol[MAX_EQF_PATH];

        sprintf( szQuickFol, "\x15%c:\\%s\\%s",
                 chPrimaryDrive, PATH, QUICKTOUR_FOL );
        strcat( pSysProps->RestartFolders, szQuickFol );
      } /* endif */

      sprintf( pSysProps->FocusObject, "%c:\\%s\\%s",
               chPrimaryDrive, PATH, QUICKTOUR_FOL );
    } /* endif */
#endif

LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 104 /* Write properties and free data area                            */");
#ifdef TEMPORARY_COMMENTED
    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( pSysProps )
    {
      WritePropFile( chPrimaryDrive, SYSTEM_PROPERTIES_NAME, pSysProps,
                     sizeof( PROPSYSTEM) );
      free( pSysProps );
    }
    else
    {
      usRc = 1;
    } /* endif */
#endif //TEMPORARY_COMMENTED

    return ( usRc );
} /* end of function UpdateSystemProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateFolderListProperties                               |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateFolderListProperties( CHAR chPrimaryDrive )        |
//+----------------------------------------------------------------------------+
//|Description:       Create folder list properties and write the data to disk.|
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateFolderListProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPFOLDERLIST pPropFll;          // ptr to folderlist properties
    USHORT       usRC = 0;             // function return code

    /******************************************************************/
    /* Allocate area for new properties                               */
    /******************************************************************/
    pPropFll = (PPROPFOLDERLIST)malloc( sizeof( PROPFOLDERLIST));
    if ( pPropFll )
    {
      memset( pPropFll, NULC, sizeof( *pPropFll) );
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )
    {
      LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 105 SETPROPHEAD( pPropFll->PropHead, DEFAULT_FOLDERLIST_NAME, PROP_CLASS_FOLDERLIST );");
#ifdef TEMPORARY_COMMENTED
      SETPROPHEAD( pPropFll->PropHead, DEFAULT_FOLDERLIST_NAME,
                   PROP_CLASS_FOLDERLIST );
                   #endif
    } /* endif */


    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      pPropFll->Swp.x =  (SHORT) (cxDesktopDiv20 * 1);
      pPropFll->Swp.y  = (SHORT) (cyDesktopDiv20 * 5);
      pPropFll->Swp.cx = (SHORT) (cxDesktopDiv20 * 7L);
      pPropFll->Swp.cy = (SHORT) (cyDesktopDiv20 * 8L);
      pPropFll->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE |
                         EQF_SWP_ACTIVATE | EQF_SWP_SHOW;
#ifdef _PTM
      pPropFll->Swp.fs |= EQF_SWP_MINIMIZE;
#endif
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 106 *pPropFll->szDriveList = pPropFll->PropHead.szPath[0];");
#ifdef TEMPORARY_COMMENTED
      *pPropFll->szDriveList = pPropFll->PropHead.szPath[0];
#endif
    } /* endif */

LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 107 /* Write properties and free data area                            */");
#ifdef TEMPORARY_COMMENTED
    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, DEFAULT_FOLDERLIST_NAME, pPropFll,
                     sizeof( PROPFOLDERLIST) );
      free( pPropFll );
    } /* endif */
#endif //TEMPORARY_COMMENTED

   return( usRC );

} /* end of function CreateFolderListProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateImexProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateImexProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create import/export properties.                         |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateImexProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPIMEX pPropImex;               // pointer to properties
    USHORT       usRC = 0;             // function return code


    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropImex = (PPROPIMEX)malloc( sizeof( PROPIMEX));
    if ( pPropImex )
    {
      memset( pPropImex, NULC, sizeof( *pPropImex));
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */


    /******************************************************************/
    /* Load property heading area                                     */
    /******************************************************************/
    if ( !usRC )
    { 
      LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 108 SETPROPHEAD( pPropImex->PropHead, IMEX_PROPERTIES_NAME, PROP_CLASS_IMEX );");
#ifdef TEMPORARY_COMMENTED
      SETPROPHEAD( pPropImex->PropHead, IMEX_PROPERTIES_NAME, PROP_CLASS_IMEX );
      #endif
    } /* endif */

    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      sprintf( pPropImex->szSavedDlgLoadDrive, "%c:", chPrimaryDrive );
      strcpy(  pPropImex->szSavedDlgLoadPath, "\\");
      strcpy(  pPropImex->szSavedDlgLoadPatternName, "*");
      strcpy(  pPropImex->szSavedDlgLoadPatternExt, ".SCR");
      sprintf( pPropImex->szSavedDlgFExpoTPath, "\\%s\\", TARGETDIR );
      sprintf( pPropImex->szSavedDlgFExpoSPath, "\\%s\\", SOURCEDIR );
      sprintf( pPropImex->szSavedDlgFExpoNPath, "\\%s\\", SNOMATCHDIR );
    } /* endif */

LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 109 /* Write properties and free data area                            */");
#ifdef TEMPORARY_COMMENTED
    /******************************************************************/
    /* Write properties and free data area                            */
    /******************************************************************/
    if ( !usRC )
    {
      WritePropFile( chPrimaryDrive, IMEX_PROPERTIES_NAME, pPropImex,
                     sizeof( PROPIMEX) );
      free( pPropImex );
    } /* endif */
#endif //TEMPORARY_COMMENTED

   return( usRC );

} /* end of function CreateImexProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateDictProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateDictProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create dictionary list properties and write the data to  |
//|                   disk.                                                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateDictProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPDICTLIST pPropDict;           // pointer to dictionary properties
    USHORT       usRC = 0;             // function return code


    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropDict = (PPROPDICTLIST) malloc( sizeof( PROPDICTLIST) );
    if ( pPropDict )
    {
      memset( pPropDict, NULC, sizeof( *pPropDict));
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/


    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      pPropDict->Swp.x =  (SHORT) (cxDesktopDiv20 * 8);
      pPropDict->Swp.y =  (SHORT) (cyDesktopDiv20 * 5);
      pPropDict->Swp.cx = (SHORT) (cxDesktopDiv20 * 7L);
      pPropDict->Swp.cy = (SHORT) (cyDesktopDiv20 * 8L);
      pPropDict->Swp.fs = EQF_SWP_SIZE | EQF_SWP_MOVE | EQF_SWP_ACTIVATE |
                          EQF_SWP_SHOW;
#ifdef _PTM
      pPropDict->Swp.fs |= EQF_SWP_MINIMIZE;
#endif
    } /* endif */
   return( usRC );

} /* end of function CreateDictProperties */

//+----------------------------------------------------------------------------+
//|Function name:     CreateEditProperties                                     |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     CreateEditProperties( CHAR chPrimaryDrive )              |
//+----------------------------------------------------------------------------+
//|Description:       Create editor properties and write the data to disk.     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chPrimaryDrive       MAT primary drive            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate data area for properties;                       |
//|                   fill property heading area;                              |
//|                   fill property data area with default values;             |
//|                   write properties and free data area;                     |
//+----------------------------------------------------------------------------+
USHORT CreateEditProperties
(
  CHAR chPrimaryDrive                  // MAT primary drive
)
{
    PPROPEDIT pPropEdit;               // ptr to editor properties
    PSTEQFGEN  pstEQFGen = NULL;       // pointer to generic editor structure
    USHORT       usRC = 0;             // function return code
    LONG       cyTitleBar,             // height of title bar
               cyMenu,                 // height of action bar
               cyHorzSlider,           // height of horizontal scroll bar
               cyBorder,               // height of horizontal border
               cyProposals,            // height of TM/dict proposals windows
               cyProposalsClient;      // height of client window of proposals
    SHORT      sCy;               // character cell size

    /******************************************************************/
    /* Allocate area for new system properties                        */
    /******************************************************************/
    pPropEdit = (PPROPEDIT)malloc( sizeof( PROPEDIT));
    if ( pPropEdit )
    {
      memset( pPropEdit, NULC, sizeof( *pPropEdit));
      pstEQFGen = &(pPropEdit->stEQFGen);// set pointer to generic structure
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    /******************************************************************/
    /* Fill in property heading area                                  */
    /******************************************************************/
    if ( !usRC )   {    } /* endif */

    /******************************************************************/
    /* Fill properties with default values                            */
    /******************************************************************/
    if ( !usRC )
    {
      /* calculate approx. value for size of TM and dict proposals */
      cyProposals = (cyDesktop - 2L * cyTitleBar - cyMenu) / 4L;
      cyProposalsClient = cyProposals - cyTitleBar - cyHorzSlider - 2L*cyBorder;
      if ( cyProposalsClient % (LONG) sCy )
      {
        /* client area is larger than multiple of characters */
        /* size it to next value                             */
        cyProposalsClient = ((cyProposalsClient / (LONG) sCy) + 1L) * (LONG) sCy;
      } /* endif */

      /* now calculate size of complete window */
      cyProposals = cyProposalsClient + cyTitleBar + cyHorzSlider + 2L*cyBorder;

      strcpy( pPropEdit->szMFEStartProc, "STANDARD" );
      RECTL_XLEFT(pstEQFGen->rclEditorTgt)      = 0L;
      RECTL_XRIGHT(pstEQFGen->rclEditorTgt)     = cxDesktop;
      RECTL_YBOTTOM(pstEQFGen->rclEditorTgt)    = 2L * cyProposals;
#ifndef _WINDOWS
      RECTL_YTOP(pstEQFGen->rclEditorTgt)       = cyDesktop - 2L * cyTitleBar
                                           - cyMenu - 1L;
#else
      RECTL_YTOP(pstEQFGen->rclEditorTgt)       = cyDesktop;
#endif

    } /* endif */

   return( usRC );
} /* end of function CreateEditProperties */
//#endif

//+----------------------------------------------------------------------------+
//|Function name:     SetupCreateDir                                           |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     SetupCreateDir( CHAR chDrive, USHORT usPathID )          |
//+----------------------------------------------------------------------------+
//|Description:       Create the requested directory on the specified drive.   |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive where the new directory|
//|                                               should be created            |
//|                   PSZ    pFolderName          name of the folder           |
//|                   USHORT usPathID             symolic name for the path    |
//|                                               (same values as for          |
//|                                                UtlMakeEqfPath function)    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           SetupCreateDir( 'C', SYSTEM_PATH );                      |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build directory name;;                 |
//|                   use DosMKDir to create directory;                        |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+
USHORT SetupCreateDir
(
   CHAR    chDrive,                    // drive for new directory
   PSZ     pszFolder,                  // name of the folder
   USHORT  usPathID                    // symbolic name for path
)
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  USHORT   usRC = NO_ERROR;            // function return code

  BuildPath( szPath, chDrive, pszFolder, usPathID );
LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 42 SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ); if ( !CreateDirectory( szPath, NULL ) )");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
  SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
  if ( !CreateDirectory( szPath, NULL ) )
  {
    usRC = (USHORT)GetLastError();
  } /* endif */
  SetErrorMode( 0 );
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
  if ( usRC <= 5 )                     // directory exists already
  {
     usRC = 0;
  } /* endif */
  return( usRC );
} /* end of function SetupCreateDir */

//+----------------------------------------------------------------------------+
//|Function name:     DeletePropFile                                           |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     DeletePropFile( CHAR chDrive, PSZ pszName )              |
//+----------------------------------------------------------------------------+
//|Description:       Delete the specified property file.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive of property file       |
//|                   PSZ    pszName              name of property file        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           DeletePropFile( 'C', "EQFSYS.PRP" );                     |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build property path;                   |
//|                   add property name to path;                               |
//|                   use DosDelete to delete the property file;               |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+
USHORT DeletePropFile
(
   CHAR    chDrive,                    // drive of property file
   PSZ     pszName                     // name of property file
)
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  USHORT   usRC;                       // function return code

  BuildPath( szPath, chDrive, NULL, PROPERTY_PATH );
  strcat( szPath, BACKSLASH_STR );
  strcat( szPath, pszName );
LogMessage2(ERROR,__func__, ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 43 usRC = (USHORT)DosDelete( szPath, 0L );");
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
  usRC = (USHORT)DosDelete( szPath, 0L );
#endif TO_BE_REPLACED_WITH_LINUX_CODE
  return( usRC );
} /* end of function DeletePropFile */

//+----------------------------------------------------------------------------+
//|Function name:     WritePropFile                                            |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     WritePropFile( CHAR chDrive, PSZ pszName, PVOID  pProp,  |
//|                                  USHORT usSize )                           |
//+----------------------------------------------------------------------------+
//|Description:       Delete the specified property file.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chDrive              drive of property file       |
//|                   PSZ    pszName              name of property file        |
//|                   PVOID  pProp                ptr to property data         |
//|                   USHORT usSize               size of property data        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0  = function completed successfully                     |
//|                   !0 = error code of called Dos functions                  |
//+----------------------------------------------------------------------------+
//|Samples:           WritePropFile( 'C', "EQFSYS.PRP", pSysProp,              |
//|                                  sizeof(*pSysProp) );                      |
//+----------------------------------------------------------------------------+
//|Function flow:     call BuildPath to build property path;                   |
//|                   add property name to path;                               |
//|                   open the property file;                                  |
//|                   write properties to disk;                                |
//|                   close property file;                                     |
//|                   return return code to caller;                            |
//+----------------------------------------------------------------------------+

#ifdef __linux__
USHORT WritePropFile(const char* szPath, PVOID pProp, USHORT usSize)
{
    USHORT  usRC = NO_ERROR;             // function return code
    USHORT tempRC = FilesystemHelper::CreateFilebuffer( SYSTEM_PROPERTIES_NAME );
    auto pData = FilesystemHelper::GetFilebufferData( SYSTEM_PROPERTIES_NAME );
    if ( pData == NULL )
    {
        LogMessage2(ERROR, "WritePropFile(), filebuffer not created and not found: ", szPath);
        usRC = ERROR_PATH_NOT_FOUND;
    }else{
        PUCHAR pPropData = (PUCHAR) pProp;
        pData->resize(usSize+2);
        std::copy(&pPropData[0], &pPropData[usSize], std::back_inserter(*pData));
    } 

    return( usRC );
}

USHORT ReadPropFile(const char* szPath, PVOID *pProp, USHORT usSize)
{
  USHORT  usRC = NO_ERROR;             // function return code

  auto pData = FilesystemHelper::GetFilebufferData( SYSTEM_PROPERTIES_NAME );
  if ( pData == NULL || pData->size() < usSize)
  {
      LogMessage2(ERROR, "ReadPropFile(), filebuffer not found: ", szPath);
      usRC = ERROR_PATH_NOT_FOUND;
  }else{
      memcpy((PVOID)*pProp,(const PVOID)(*pData)[0], usSize);
  } 
  return ( usRC );

}
#endif //__linux__

//+----------------------------------------------------------------------------+
//|Function name:     BuildPath                                                |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     BuildPath( PSZ pszBuffer, CHAR chEqfDrive, PSZ pszFolder,|
//|                              USHORT usPathID )                             |
//+----------------------------------------------------------------------------+
//|Description:       Build the path to the specified MAT directory.           |
//|                   The function works simular to the function UtlMakeEQFPath|
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR   chEqfDrive           drive for the path name      |
//|                   PSZ    pszFolder            name of a folder or NULL     |
//|                   USHORT usPathID             symbolic path identifier     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PSZ    pszBuffer            buffer is filled with path   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           BuildPath( pszBuffer, 'C', NULL, PROPERTY_PATH )         |
//|                      will fill pszBuffer with "C:\EQF\PROPERTY"            |
//+----------------------------------------------------------------------------+
//|Function flow:     get directory name for given path ID;                    |
//|                   build path name;                                         |
//+----------------------------------------------------------------------------+
VOID BuildPath
(
  PSZ pszBuffer,                       // points to buffer for path name
  CHAR chEqfDrive,                     // drive for the path name
  PSZ pszFolder,                       // name of a folder or NULL
  USHORT usPathID                      // symbolic path identifier
)
{
   PSZ         pszSubDir;             // ptr to subdirectory info

   *pszBuffer = NULC;                 // no path info yet

   /*******************************************************************/
   /* Get pointer to name of requested subdirectory                   */
   /*******************************************************************/
   switch ( usPathID )
   {
      case PROPERTY_PATH         :
         pszSubDir = PROPDIR;
         break;
      case CTRL_PATH             :
         pszSubDir = CTRLDIR;
         break;
      case PROGRAM_PATH          :
         pszSubDir = PROGDIR;
         pszFolder = NULL;
         break;
      case DIC_PATH              :
         pszSubDir = DICTDIR;
         pszFolder = NULL;
         break;
      case MEM_PATH              :
         pszSubDir = MEMDIR;
         pszFolder = NULL;
         break;
      case TABLE_PATH            :
         pszSubDir = TABLEDIR;
         pszFolder = NULL;
         break;
      case LIST_PATH             :
         pszSubDir = LISTDIR;
         pszFolder = NULL;
         break;
      case DLL_PATH              :
         pszSubDir = DLLDIR;
         pszFolder = NULL;
         break;
      case MSG_PATH              :
         pszSubDir = MSGDIR;
         pszFolder = NULL;
         break;
      case PRT_PATH              :
         pszSubDir = PRTDIR;
         pszFolder = NULL;
         break;
      case EXPORT_PATH           :
         pszSubDir = EXPORTDIR;
         pszFolder = NULL;
         break;
      case DOCU_PATH             :
         pszSubDir = DOCUDIR;
         pszFolder = NULL;
         break;
      case SYSTEM_PATH           :
         pszSubDir = NULL;
         break;
      case COMMEM_PATH           :
         pszSubDir = COMMEMDIR;
         pszFolder = NULL;
         break;
      case COMPROP_PATH          :
         pszSubDir = COMPROPDIR;
         pszFolder = NULL;
         break;
      case COMDICT_PATH          :
         pszSubDir = COMDICTDIR;
         pszFolder = NULL;
         break;
       case  DIRSOURCEDOC_PATH:
         pszSubDir = SOURCEDIR;
         break;
       case  DIRSEGSOURCEDOC_PATH:
         pszSubDir = SEGSOURCEDIR;
         break;
       case  DIRSEGTARGETDOC_PATH:
         pszSubDir = SEGTARGETDIR;
         break;
       case  DIRTARGETDOC_PATH:
         pszSubDir = TARGETDIR;
         break;
      default:
         pszSubDir = NULL;
         pszFolder = NULL;
         break;
   } /* endswitch */

   /*******************************************************************/
   /* Build path name                                                 */
   /*******************************************************************/
   if ( pszFolder )
   {
      strcat( pszBuffer, "/" );
      strcat( pszBuffer, pszFolder );
   } /* endif */
   if ( pszSubDir )
   {
      strcat( pszBuffer, "/" );
      strcat( pszBuffer, pszSubDir );
   } /* endif */
} /* end of function BuildPath */



//+----------------------------------------------------------------------------+
//|Function name:     InstReadSysProps                                         |
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstReadSysProps()                                       |
//+----------------------------------------------------------------------------+
//|Description:       Try to read MAT system properties                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PPROPSYSTEM                                              |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL   = no system properties could be read              |
//|                   other  = pointer to loaded system properties             |
//+----------------------------------------------------------------------------+
//|Function flow:     get name of system properties from OS2.INI;              |
//|                   open system property file;                               |
//|                   allocate buffer and load system properties into memory;  |
//|                   close open files and, in case of errors, free allocated; |
//|                   buffers;                                                 |
//+----------------------------------------------------------------------------+
PPROPSYSTEM InstReadSysProps( VOID )
{  
   PPROPSYSTEM pSysProp = nullptr;
   std::vector<UCHAR>* pData = FilesystemHelper::GetFilebufferData( SYSTEM_PROPERTIES_NAME );
   if(pData && pData->size()){
     pSysProp = (PPROPSYSTEM)&(*pData)[0];
   }
   return( pSysProp );
} /* end of function InstReadSysProps */


//+----------------------------------------------------------------------------+
//|Function name:     InstLocateString                                         |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstLocateString( PSZ pszHayStack, PSZ pszNeedle )       |
//+----------------------------------------------------------------------------+
//|Description:       Locate the given string pszNeedle in string pszHayStack  |
//|                   ignoring case and blanks.             .          .       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszHayStack  string being searched through        |
//|                   PSZ    pszNeedle    string being searched for            |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL  = string not found                                 |
//|                   other = position of pszNeedle in pszHayStack             |
//+----------------------------------------------------------------------------+
//|Function flow:     loop through haystack and use InstMatch to check for     |
//|                   needle;                                                  |
//|                   return pointer to found location or NULL if not found;   |
//+----------------------------------------------------------------------------+
PSZ InstLocateString
(
  PSZ    pszHayStack,                  // string being searched through
  PSZ    pszNeedle                     // string being searched for
)
{
   int i, j, k;                        // length and loop variables

   i = strlen(pszHayStack);
   j = strlen(pszNeedle);
   for (k = 0; k < i - j; ++k )
   {
      if ( pszHayStack[k] != ' ' )
      {
         if ( InstMatch( pszHayStack+k, pszNeedle ) != NULL )
         {
            return( pszHayStack + k );
         } /* endif */
      } /* endif */
   } /* endfor */

   return( NULL );
}

//+----------------------------------------------------------------------------+
//|Function name:     InstMatch                                                |
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function call:     InstMatch( PSZ pszS1, PSZ pszS2 )                        |
//+----------------------------------------------------------------------------+
//|Description:       Test if pszS1 starts with string pszS2 ignoring blanks   |
//|                   and case.                             .          .       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ    pszS1        first string                         |
//|                   PSZ    pszS2        second string                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL  = string not found                                 |
//|                   other = position of first character in pszS1 not         |
//|                           belonging to pszS2                               |
//+----------------------------------------------------------------------------+
//|Function flow:     compare only non-space characters and stop at end of     |
//|                   first string;                                            |
//+----------------------------------------------------------------------------+
PSZ InstMatch
(
   PSZ pszS1,
   PSZ pszS2
)
{
  CHAR  c1, c2;
  BOOL  fNotThrough = TRUE;

  while ( fNotThrough )
  {
     c1 = (CHAR) toupper( *pszS1 );
     c2 = (CHAR) toupper( *pszS2 );

     if (c1 == NULC)
     {
        pszS1 = NULL;                  // no match found
        fNotThrough = FALSE;
     }
     else if (c1 == ' ')
     {
        ++pszS1;                       // skip over blanks in string 1
     }
     else if (c2 == NULC)
     {
        fNotThrough = FALSE;           // end of string 2 ==> we're OK
     }
     else if (c1 == c2)
     {
        ++pszS1; ++pszS2;              // skip over equal characters
     }
     else
     {
        pszS1 = NULL;                  // bad match
        fNotThrough = FALSE;
     } /* endif */
  } /* endwhile */
  return( pszS1 );
} /* end of InstMatch */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DeleteDictCacheFiles                                     |
//+----------------------------------------------------------------------------+
//|Function call:     DeleteDictCacheFiles( chPrimaryDrive, usPathId )         |
//+----------------------------------------------------------------------------+
//|Description:       delete morph. dictionary cache files                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   CHAR - chPrimaryDrive : TM/2 drive                       |
//|                   USHORT - usPathId : dictionary path id                   |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+

VOID DeleteDictCacheFiles( CHAR chPrimaryDrive, USHORT usPathId )
{
  CHAR     szPath[MAX_EQF_PATH];       // buffer for path name
  CHAR     szCommand[255];             // buffer for commandline

  BuildPath( szPath, chPrimaryDrive, NULL, usPathId );
  sprintf (szCommand, "%s%s*.STC", szPath, BACKSLASH_STR);
  remove( szCommand );

  sprintf (szCommand, "%s%s*.NLC", szPath, BACKSLASH_STR);
  remove( szCommand );

  sprintf (szCommand, "%s%s*.P?C", szPath, BACKSLASH_STR);
  remove( szCommand );
} /* end of function DeleteDictCacheFiles */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlPropFileExist    Check if a property file exists      |
//+----------------------------------------------------------------------------+
//|Description:       Checks if a file exists.                                 |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlPropFileExist( CHAR chDrive, PSZ pszFileName )   |
//+----------------------------------------------------------------------------+
//|Parameters:        - chDrive is the primare TM/2 drive                      |
//|                   - pszFileName is the property filename                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   file exists                                       |
//|                   FALSE  file does not exist                               |
//+----------------------------------------------------------------------------+
//|Function flow:     call UtlFindFirst to check if file exists                |
//|                   close file search handle                                 |
//|                   return result                                            |
//+----------------------------------------------------------------------------+
BOOL UtlPropFileExist( CHAR chDrive, PSZ pszFile )
{
   CHAR     szPath[MAX_EQF_PATH];      // buffer for path name

   DOSVALUE usCount = 1L;
   HDIR     hDirHandle = HDIR_CREATE;  // DosFind routine handle
   USHORT   usDosRC;                   // return code of Dos... alias Utl...

   BuildPath( szPath, chDrive, NULL, PROPERTY_PATH );
   strcat( szPath, BACKSLASH_STR );
   strcat( szPath, pszFile );


   return( usDosRC == 0 );
} /* endof UtlPropFileExist */

USHORT UpdateFolderProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPFOLDER pFolProp = NULL;         // ptr to folder properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                   // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for folder  properties                           */
      /******************************************************************/
      pFolProp = (PPROPFOLDER) malloc ( sizeof( PROPFOLDER ));
      if ( pFolProp )
      {
        memset( pFolProp, NULC, sizeof( PROPFOLDER));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPFOLDER );
      usRC = UtlRead( hFile, pFolProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
   if ( !usRC )
   {
     LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 122 /* copy EQF drive into properties                                  */");
#ifdef TEMPORARY_COMMENTED
     pFolProp->PropHead.szPath[0] = chDrive;
     #endif
     pFolProp->chDrive = chDrive;
   } /* endif */

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pFolProp, usPropFileSize, &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pFolProp )
    free( pFolProp );           // free storage used for old properties

   return( usRC );
} /* end of function UpdateFolderProp  */

USHORT UpdateDocumentProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPDOCUMENT pDocProp = NULL;         // ptr to dictionary properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for document properties                          */
      /******************************************************************/
      pDocProp = (PPROPDOCUMENT) malloc ( sizeof( PROPDOCUMENT ));
      if ( pDocProp )
      {
        memset( pDocProp, NULC, sizeof( PROPDOCUMENT));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPDOCUMENT );
      usRC = UtlRead( hFile, pDocProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 123 /* copy EQF drive into properties                                  */");
#ifdef TEMPORARY_COMMENTED
   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
      pDocProp->PropHead.szPath[0] = chDrive;
      #endif

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
      usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pDocProp, usPropFileSize, &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */
   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pDocProp )
    free( pDocProp );           // free storage used for old properties

   return( usRC );
} /* end of function UpdateDocumentProp  */

USHORT UpdateDictProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPDICTIONARY  pDictProp = NULL;   // ptr to dictionary properties
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;       // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for dictionary properties                        */
      /******************************************************************/
      pDictProp = (PPROPDICTIONARY) malloc ( sizeof( PROPDICTIONARY ));
      if ( pDictProp)
      {
        memset( pDictProp, NULC, sizeof( PROPDICTIONARY));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPDICTIONARY );
      usRC = UtlRead( hFile, pDictProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
    {
      pDictProp->szDictPath[0] = chDrive;
      pDictProp->szIndexPath[0] = chDrive;
      
     LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 124 /* copy EQF drive into properties                                  */");
#ifdef TEMPORARY_COMMENTED
      pDictProp->PropHead.szPath[0] = chDrive;
      #endif
    }
   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pDictProp, usPropFileSize,
                      &usSizeWritten, FALSE);
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   } /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pDictProp )
    free( pDictProp);           // free storage used for properties

   return( usRC );
} /* end of function UpdateDictProp  */

USHORT UpdateTMProp
(
  PSZ   pszFullFileName,
  CHAR  chDrive
)
{
  USHORT   usRC;
  USHORT   usAction;
  PPROPTRANSLMEM   pTMProp = NULL;     // ptr to folder properties
  PPROP_NTM pNTMProp = NULL;
  HFILE    hFile = 0L;                 // File handle for folder properties file
  USHORT           usBytesRead;        // number of bytes read
  USHORT  usSizeWritten;               // Number of bytes written by DosWrite
  USHORT  usPropFileSize = 0;          // Size of property file
  ULONG   ulFilePos = 0L;              // position of file pointer

  chDrive = (CHAR)toupper (chDrive);                              // change driveletter UPPERCASE

   usRC = UtlOpen( pszFullFileName, &hFile, &usAction,
                   0L,                            // Create file size
                   FILE_NORMAL,
                   FILE_OPEN,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                   0L, FALSE );                          // Reserved
   if ( !usRC )
   {
      /******************************************************************/
      /* Allocate area for TM properties                                */
      /******************************************************************/
      pTMProp = (PPROPTRANSLMEM) malloc ( sizeof( PROPTRANSLMEM  ));
      if ( pTMProp)
      {
        memset( pTMProp, NULC, sizeof( PROPTRANSLMEM));
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Load data from property file                                    */
   /*******************************************************************/
   if ( !usRC )
   {
      usPropFileSize = sizeof( PROPTRANSLMEM );
      usRC = UtlRead( hFile, pTMProp, usPropFileSize,
                      &usBytesRead, FALSE );
   }

   /*******************************************************************/
   /* copy EQF drive into properties                                  */
   /*******************************************************************/
    if ( !usRC )
    {
      /* check whether this is a new TM property file */
      pNTMProp = (PPROP_NTM) pTMProp;
      if ( strcmp( pNTMProp->szNTMMarker, NTM_MARKER) == 0 )
      {
     LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 125 /* copy EQF drive into properties                                  */" );
#ifdef TEMPORARY_COMMENTED
        pNTMProp->stPropHead.szPath[0] = chDrive;
        pNTMProp->szFullMemName[0] = chDrive;
        #endif
      }
      else
      {
        
     LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 126 pTMProp->stPropHead.szPath[0] = chDrive;");
#ifdef TEMPORARY_COMMENTED
        pTMProp->stPropHead.szPath[0] = chDrive;
        #endif
        pTMProp->szPath[0]            = chDrive;
        pTMProp->szMemPath[0]         = chDrive;
        pTMProp->szFullMemName[0]     = chDrive;
        pTMProp->szXTagsListFile[0]   = chDrive;
        pTMProp->szXWordsListFile[0]  = chDrive;
        pTMProp->szLanguageFile[0]    = chDrive;
        pTMProp->szFullFtabPath[0]    = chDrive;
      } /* endif */
   }

   /********************************************************************/
   /* Write property data to disk                                      */
   /********************************************************************/
   if ( !usRC  )
     usRC = UtlChgFilePtr( hFile, 0L, FILE_BEGIN, &ulFilePos, FALSE );
   if ( !usRC  )
   {
     usRC = UtlWrite( hFile, (PVOID)pTMProp, usPropFileSize , &usSizeWritten,
                      FALSE );
     if( !usRC && (usPropFileSize != usSizeWritten) )
     {
       usRC = ERROR_WRITE_FAULT;
     } /* endif */
   }  /* endif */

   /********************************************************************/
   /* Close property file                                              */
   /********************************************************************/
   if ( hFile )
   {
     UtlClose( hFile, FALSE );
   } /* endif */

   /********************************************************************/
   /* Free allocated area                                              */
   /********************************************************************/
   if ( pTMProp )
    free( pTMProp );           // free storage used for properties

   return( usRC );
} /* end of function UpdateTMProp  */
