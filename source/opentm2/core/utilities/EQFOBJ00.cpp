/*!
   EQFOBJ00.C - EQF Object Manager
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#include "EQF.H"                  // General .H for EQF
#include "EQFOBJ00.H"             // Object manager defines
#include "EQFSETUP.H"
#include "PropertyWrapper.H"
#include "win_types.h"
#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
static HWND hObjMan[MAX_TASK] = {NULLHANDLE};
POBJM_IDA     pObjBatchIda = NULL;     // Points to IDA when in function call mode


////////////////////////////////////////////////////////////////////
// defines for symbol table in shared memory area in nonDDE mode  //
////////////////////////////////////////////////////////////////////

// name of shared memory table with locked objects
#define EQFLOCKOBJ_SHMEM     "/SHAREMEM/OTM/OTMLOCKOBJ"

// element of locked objects
typedef struct _FUNCIF_LOCK_ENTRY
{
  CHAR szLockObj[MAX_EQF_PATH];        // name of locked object
  DWORD dwProcessID;                   // ID of process locking the object
} FUNCIF_LOCK_ENTRY, *PFUNCIF_LOCK_ENTRY;

// structure of shared memory area
typedef struct _FUNCIF_LOCK_TABLE
{
  BOOL fUpdateInProgress;              // TRUE = table is currently updated
  int  iReadAccess;                    // number of processes currently reading table
  int  iEntries;                       // number of entries in table
  int  iFiller1;                       // currently not used
  int  iFiller2;                       // currently not used
  int  iFiller3;                       // currently not used
  int  iFiller4;                       // currently not used
  int  iFiller5;                       // currently not used
  int  iFiller6;                       // currently not used
  int  iFiller7;                       // currently not used
  int  iFiller8;                       // currently not used
  CHAR chFiller9[128];                 // currently not used
  FUNCIF_LOCK_ENTRY aLock[SYMBOLENTRIES]; // table of currently locked ojects
} FUNCIF_LOCK_TABLE, *PFUNCIF_LOCK_TABLE;

// handle of shared memory object
static HANDLE hSharedMem = NULLHANDLE;

// utility functions for lock table handling
HANDLE ObjLockTable_CreateOrOpenTable();
PFUNCIF_LOCK_TABLE ObjLockTable_AccessTable( HANDLE );
void ObjLockTable_ReleaseTable( PFUNCIF_LOCK_TABLE );
BOOL ObjLockTable_SetUpdateFlag( PFUNCIF_LOCK_TABLE );
BOOL ObjLockTable_ClearUpdateFlag( PFUNCIF_LOCK_TABLE );
PFUNCIF_LOCK_ENTRY ObjLockTable_Search( PFUNCIF_LOCK_TABLE, PSZ, BOOL );
BOOL ObjLockTable_Delete( PFUNCIF_LOCK_TABLE, PSZ, DWORD );
BOOL ObjLockTable_Add( PFUNCIF_LOCK_TABLE, PSZ, DWORD );
BOOL ObjLockTable_WaitWhenUpdated( PFUNCIF_LOCK_TABLE pLockTable );

HWND EqfQueryObjectManager( VOID)
{
    return( hObjMan[UtlGetTask()]);
}
USHORT Send2AllHandlers( WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    POBJM_IDA     pIda;                // Points to instance data area
    USHORT        usResult;             //

    if (pIda != NULL  )                                                   /*@02A*/
    {                                                                     /*@02A*/
      usResult = SendAll( pIda->pHndlrTbl, clsANY, msg, mp1, mp2);
    }                                                                     /*@02A*/
    else                                                                  /*@02A*/
    {                                                                     /*@02A*/
      usResult = FALSE;                                                   /*@02A*/
    } /* endif */                                                         /*@02A*/
    return( usResult);                                                    /*@02A*/
}

USHORT Send2AllObjects( USHORT cls, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    POBJM_IDA     pIda;                // Points to instance data area
T5LOG(T5ERROR) <<  ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 28 pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
    pIda = ACCESSWNDIDA( hObjMan[UtlGetTask()], POBJM_IDA);
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
    return( SendAll( pIda->pObjTbl, (CLASSES) cls, msg, mp1, mp2));
}

/*+--------------------------------------------------------------------------+
     Send a message to all active EQF objects
     -> multiple tables not yet supported
  +--------------------------------------------------------------------------+
*/
USHORT SendAll( POBJTBL pt, CLASSES objClass, WINMSG message,
                WPARAM mp1, LPARAM mp2)
{
    register int i;
    POBJENTRY pe;
    USHORT x=0;

//  If not controlled or indicated from outside this proc it is strongly
//  required to loop through the table from bottom to top !! to meet the
//  last registered objects before sending a msg to the "elder" objects.
//  This is required e.g. for message WM_EQF_TERMINATE which might send
//  messages to their quasi parents (i.e. parent objects).
//  There is no other way at the present moment to control the sequence
//  of objects which will be sent the message.

//  wnd proc returns NULL if ok otherwise !NULL
    return( x);
}

MRESULT EqfSend2Handler( PSZ psz, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  return( false );
}


// Function PropHandlerInitForBatch
// Initialize the property handler for non-windows environments;
// i.e. perform WM_CREATE handling to allocate our IDA
BOOL ObjHandlerInitForBatch( void )
{
  POBJM_IDA     pIda;                // Points to instance data area
  int us;

  hObjMan[UtlGetTask()] = NULLHANDLE;      // we are not ready yet
  if( !UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof( *pIda), ERROR_STORAGE ) )
    return( FALSE);
  memset( pIda, 0, sizeof( *pIda));
  pObjBatchIda = pIda;
  strcpy( pIda->IdaHead.szObjName, OBJECTMANAGER );
  pIda->IdaHead.pszObjName = pIda->IdaHead.szObjName;

  us = sizeof( OBJTBL) + HANDLERENTRIES * sizeof( OBJENTRY);
  UtlAlloc( (PVOID *)&pIda->pHndlrTbl, 0L, (LONG)us, ERROR_STORAGE );
  memset( pIda->pHndlrTbl, 0, us);
  pIda->pHndlrTbl->usUsed = 0;
  pIda->pHndlrTbl->usMax  = HANDLERENTRIES;
  pIda->pHndlrTbl->pObjEntry =
     (POBJENTRY) ( (PBYTE)pIda->pHndlrTbl+ sizeof( OBJTBL) );

  us = sizeof( OBJTBL) + OBJECTENTRIES * sizeof( OBJENTRY);
  UtlAlloc( (PVOID *)&pIda->pObjTbl, 0L, (LONG)us, ERROR_STORAGE );
  memset( pIda->pObjTbl, 0, us);
  pIda->pObjTbl->usUsed = 0;
  pIda->pObjTbl->usMax  = OBJECTENTRIES;
  pIda->pObjTbl->pObjEntry =
     (POBJENTRY)( (PBYTE)pIda->pObjTbl+ sizeof( OBJTBL) );
  pIda->hAtomTbl = WinCreateAtomTable( 0, 0 );

  //instead of a symbol table we use a lock table in shared memory
  hSharedMem = ObjLockTable_CreateOrOpenTable();

  return( TRUE );
} /* end of function ObjHandlerInitForBatch */

SHORT ObjQuerySymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        PFUNCIF_LOCK_ENTRY pEntry;
        pEntry = ObjLockTable_Search(pLockTable,pszSymbol,TRUE);
        if ( !pEntry ) sRC = -1;
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    }
  }
  else
  {
T5LOG(T5ERROR) <<  ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 31 sRC = (SHORT) WinSendMsg( EqfQueryObjectManager(), WM_EQF_QUERYSYMBOL, NULL, MP2FROMP(pszSymbol) );";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
    sRC = (SHORT) WinSendMsg( EqfQueryObjectManager(),
                      WM_EQF_QUERYSYMBOL, NULL, MP2FROMP(pszSymbol) );
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
  } /* endif */
  return( sRC );
} /* endif ObjQuerySymbol */

SHORT ObjSetSymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    } /* endif */
  }

  return( sRC );
} /* endif ObjSetSymbol */

SHORT ObjRemoveSymbol( PSZ pszSymbol )
{
  SHORT sRC = 0;

  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    if ( hSharedMem)
    {
      PFUNCIF_LOCK_TABLE pLockTable = ObjLockTable_AccessTable( hSharedMem);
      if ( pLockTable )
      {
        if ( !ObjLockTable_Delete( pLockTable, pszSymbol, 0 ) )
        {
          sRC = -1;
        } /* endif */
        ObjLockTable_ReleaseTable(pLockTable);
      } /* endif */
    } /* endif */
  }
  return( sRC );
} /* endif ObjRemoveSymbol */

//////////////////////////////////////////////////////////////////
/// Lock Table handling                                        ///
//////////////////////////////////////////////////////////////////

HANDLE ObjLockTable_CreateOrOpenTable()
{
  return( nullptr );
} /* end of function ObjLockTable_CreateOrOpenTable */

PFUNCIF_LOCK_TABLE ObjLockTable_AccessTable( HANDLE hSharedMem )
{
  return( nullptr );
} /* end of function ObjLockTable_AccessTable */

void ObjLockTable_ReleaseTable( PFUNCIF_LOCK_TABLE pTable )
{
} /* end of function ObjLockTable_AccessTable */

BOOL ObjLockTable_SetUpdateFlag( PFUNCIF_LOCK_TABLE pLockTable )
{
  BOOL  fTimeOut = FALSE;

  fTimeOut = ObjLockTable_WaitWhenUpdated( pLockTable );
  if ( !fTimeOut )
  {
    pLockTable->fUpdateInProgress = TRUE;
  } /* endif */
  return( !fTimeOut );
} /* end of function ObjLockTable_SetUpdateFlag */

BOOL ObjLockTable_ClearUpdateFlag( PFUNCIF_LOCK_TABLE pLockTable )
{
  pLockTable->fUpdateInProgress = FALSE;
  return( TRUE );
} /* end of function ObjLockTable_ClearUpdateFlag */

PFUNCIF_LOCK_ENTRY ObjLockTable_Search
(
  PFUNCIF_LOCK_TABLE pLockTable,
  PSZ                pszObject,
  BOOL               fUpdateCheck      // true = check update flag
)
{
  PFUNCIF_LOCK_ENTRY pEntry = pLockTable->aLock;
  PFUNCIF_LOCK_ENTRY pFound = NULL;
  BOOL  fTimeOut = FALSE;
  BOOL fRestart = FALSE;

  // wait until any pending update has been completed
  do
  {
    int  iEntries = 0;
    fRestart = FALSE;

    // wait until any pending update has been completed
    if ( fUpdateCheck )
    {
      fTimeOut = ObjLockTable_WaitWhenUpdated( pLockTable );
    } /* endif */

    if ( !fTimeOut && pLockTable->iEntries )
    {
      pEntry = pLockTable->aLock;
      iEntries = pLockTable->iEntries;

      do
      {
        if ( fUpdateCheck && pLockTable->fUpdateInProgress )
        {
          fRestart = TRUE;
        }
        else if ( strcasecmp( pEntry->szLockObj, pszObject ) == 0 )
        {
          pFound = pEntry;
        }
        else
        {
          pEntry++;
          iEntries--;
        } /* endif */
      }
      while ( !fRestart && iEntries && (pFound == NULL) );
    } /* endif */
  } while ( !fTimeOut && fRestart );

  return( pFound );
} /* end of function ObjLockTable_Search */

// delete a specific entry or all entries belonging to the given process
// either an object or a process ID has to be specified
BOOL ObjLockTable_Delete
(
  PFUNCIF_LOCK_TABLE pLockTable,
  PSZ   pszObject,                     // object being removed or NULL
  DWORD dwProcessID                    // processID if no object has been specified
)
{
  BOOL fOK = TRUE;

  if ( ObjLockTable_SetUpdateFlag(pLockTable) )
  {
    if ( pszObject )
    {
      // delete given object
      PFUNCIF_LOCK_ENTRY pEntry = ObjLockTable_Search( pLockTable, pszObject, FALSE );
      if ( pEntry )
      {
        int iEntry = pEntry - pLockTable->aLock;
        int iEntriesToMove = pLockTable->iEntries - iEntry - 1;
        if ( iEntriesToMove )
        {
          memmove( pEntry, pEntry+1, iEntriesToMove * sizeof(FUNCIF_LOCK_ENTRY) );
        } /* endif */
        pLockTable->iEntries--;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    }
    else
    {
      // delete all object from given process
      int i = 0;
      PFUNCIF_LOCK_ENTRY pSource, pTarget;

      pSource = pTarget = pLockTable->aLock;
      while ( i < pLockTable->iEntries )
      {
        if ( pSource->dwProcessID == dwProcessID )
        {
          // remove element
          pLockTable->iEntries--;
        }
        else
        {
          // copy element to current position
          memcpy( pTarget, pSource, sizeof(FUNCIF_LOCK_ENTRY) );
          pTarget++;
        } /* endif */
        i++;
        pSource++;
      } /* endwhile */
    } /* endif */
    ObjLockTable_ClearUpdateFlag(pLockTable);
  } /* endif */
  return( fOK );
} /* endif */

// wait until a pending update has been completed or a timeout occured
BOOL ObjLockTable_WaitWhenUpdated( PFUNCIF_LOCK_TABLE pLockTable )
{
  BOOL fTimeOut = FALSE;
  int  iNumOfRetries = 50;
  while ( pLockTable->fUpdateInProgress && !fTimeOut )
  {
    //Sleep( 20 );
    iNumOfRetries--;
    fTimeOut = (iNumOfRetries == 0 );
  }
  return( fTimeOut );
} /* end of function ObjLockTable_Search */



USHORT CreateMemFile
(
  PSZ         pszName,             // ptr to object long name
  POBJLONGTOSHORTSTATE pObjState      // ptr to buffer for returned object state
)
{
  CHAR      szFullMemPath[MAX_EQF_PATH];   // object path
  // local variables
  PSZ         pszOrgName = pszName; // original start of long name
  USHORT      usRC = NO_ERROR;         // function return code
  OBJLONGTOSHORTSTATE  ObjState = OBJ_IS_NEW;  // local copy of caller's object state buffer

  // ignore any leading blanks
  while ( *pszOrgName == ' ' ) pszOrgName++;

  // setup search path and path of full object name depending on object type
  if ( usRC == NO_ERROR )
  {
        properties_get_str(KEY_MEM_DIR, szFullMemPath, MAX_EQF_PATH);
        strcat( szFullMemPath, BACKSLASH_STR );
        strcat( szFullMemPath, pszOrgName );
        strcat( szFullMemPath, ".MEM" );
  }

  // look for objects having the same short name
  if ( usRC == NO_ERROR )
  {
    BOOL   fOK;
    HANDLE hMutexSem = NULL;

    //GETMUTEX(hMutexSem);
    if(FilesystemHelper::FindFiles(szFullMemPath).empty()){
      ObjState = OBJ_IS_NEW;
    }else{
      ObjState = OBJ_EXISTS_ALREADY;
    }
  }
  // find a unique name if document is not contained in the folder
  if ( (usRC == NO_ERROR) && (ObjState == OBJ_IS_NEW)  )
  {
    HANDLE hMutexSem = NULL;
      
    PPROP_NTM pProp = NULL;
    if ( UtlAlloc( (PVOID *)&pProp, 0L, sizeof(PROP_NTM ), NOMSG ) )
    {
      strcpy( pProp->stPropHead.szName, UtlGetFnameFromPath(szFullMemPath));
      strcat( pProp->stPropHead.szName, "$_RESERVED");
      UtlWriteFile( szFullMemPath, sizeof(PROP_NTM ), (PVOID)pProp, FALSE );
      //UtlWriteFile( szFullMemPath, strlen(szFullMemPath), (PVOID)szFullMemPath, FALSE );
      //WritePropFile(szFullMemPath, (PVOID)pProp, sizeof(PROP_NTM ));
      
      UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
    } /* endif */
    // release Mutex
    //RELEASEMUTEX(hMutexSem);
   
  } /* endif */

  // return to caller
  if ( pObjState != NULL ) 
      *pObjState = ObjState;

  return( usRC );
}

