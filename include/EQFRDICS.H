/*! Short description: Include file for remote dictionary handling
	Description:       This file contains defines and structures
					for remote dictionary handling.
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef INCL_EQFRDICS
#define INCL_EQFRDICS

//function prototypes
static USHORT GetDictsAndFillLB ( PSZ, HWND, SHORT * );
MRESULT EXPENTRY DICTINCLUDEDLG( HWND, WINMSG, WPARAM, LPARAM );
USHORT GetServerDicts( PFILE_LIST_IN, PFILE_LIST_OUT, USHORT );
USHORT GetServerDictDrives( PDRIVES_IN, PDRIVES_OUT, USHORT );
USHORT ReceiveDictProp( SERVERNAME, PSZ, USHORT );
VOID   GetDictList ( PFILE_LIST_OUT );
 USHORT DictRcHandling ( USHORT, PSZ, HTM, PSZ );
 USHORT DictRcHandling2 ( USHORT, PSZ, HTM, PSZ, PSZ );
BOOL   DictServerReference ( PSZ );
USHORT CheckPropCompatibility( PPROPDICTIONARY, PSZ, PBOOL );
 PSZ    QDAMErrorString( USHORT, PSZ *);

USHORT GetFilePart( PGETDICTPART_IN, PGETPART_OUT );
 USHORT QDAMGetPart( HTM, PSZ, PGETDICTPART_IN, PGETPART_OUT, USHORT );
BOOL   CheckRemoteDictNames( PSZ, BOOL );
 BOOL   fCompDictProps( PPROPDICTIONARY pProp1, PPROPDICTIONARY pProp2 );

#endif

