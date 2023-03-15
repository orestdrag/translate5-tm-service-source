#ifndef _IDA_H_
#define _IDA_H_

#include "EQF.H"

#define  MAX_HANDLER_NAME        40
//ida with dialog control information
typedef struct _CONTROLSIDA
{
 //path strings
 CHAR         szDrive[MAX_DRIVE];          //drive letter with :
 CHAR         szPath[MAX_LONGPATH];        //drive and directory/ies
 CHAR         szName[MAX_LONGPATH];        //filename without ext
 CHAR         szSelectedName[MAX_LONGPATH];//newly specified dictionary name
 CHAR         szExt[MAX_LONGPATH];         //file extension with leading dot
 CHAR         szPatternName[MAX_LONGPATH]; //active pattern name
 CHAR         szPathContent[MAX_LONGPATH]; //content of path entry field
 CHAR         szString[MAX_LONGPATH];      //string buffer
 CHAR         szDummy[MAX_EQF_PATH];       //string buffer
 CHAR         szDriveList[MAX_DRIVELIST];  //Drive list
 CHAR         szHandler[MAX_HANDLER_NAME]; //handler name
 //saved last used values
 CHAR         chSavedDrive;                //saved drive letter
 USHORT       usSavedFormat;               //format
 CHAR         szSavedPath[MAX_LONGPATH];   //saved path
 //boolean value if import or export dialog
 BOOL         fImport;                     //TRUE if import dialog
 //boolean value if wm_command ok button was pressed
 BOOL         fCommand;                    //TRUE if wm_command msg
 //boolean value if listbox is a multi-selection or single selection listbox
 BOOL         fMultiSelectionLB;           //TRUE if listbox is multi-selection
 //dialog control ids needed for both import and export
 USHORT       idCurrentDirectoryEF;        //id of current directory entry field
 USHORT       idPathEF;                    //id of path entry field
 USHORT       idDirLB;                     //id of directories listbox
 USHORT       idFilesLB;                   //id of files listbox
 USHORT       idInternalRB;                //id of internal radio button
 USHORT       idExternalRB;                //id of external radio button
 USHORT       idDriveBTN;                  //id of drive btn
 USHORT       idControlsGB;                //id of controls group box
 USHORT       idOkPB;                      //id of btn that triggers process
 //dialog control ids needed for export
 USHORT       idExportTEXT;                //id of static with filename for export
 //dialog control ids needed for import
 USHORT       idToLB;                            //id of to drop-down combo listbox
 CHAR         szDefPattern[MAX_LONGFILESPEC];  // default pattern to use
 BOOL         fLongFileNames;              //TRUE = allow long file names
} CONTROLSIDA, * PCONTROLSIDA;


#endif //_IDA_H_