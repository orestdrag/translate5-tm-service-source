/*!
   EQFPRO00.C - EQF Property Handler
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_EDITORAPI        // editor API
#include "EQF.H"                  // General .H for EQF
#include "EQFPRO00.H"             // Property Handler defines
#include "OTMFUNC.H"

#include <pthread.h>
#include "LogWrapper.h"
#include "PropertyWrapper.H"
#include "FilesystemHelper.h"
#include "EQFSETUP.H"
// activate the following define for property failure logging
//#define PROPLOGGING

// IDA pointer for batch mode
PPROP_IDA pPropBatchIda = NULL;
HWND      hwndPropertyHandler = NULL;

/*!
     Create a property handle
*/
PPROPHND MakePropHnd( PPROP_IDA pIda)
{
    int i;

    PPROPHND  ptr1, ptr2;              // Temp. ptr to handles
    if( !pIda->TopFreeHnd){
      i = sizeof( PLUG) + (PROPHWND_ENTRIES * sizeof( PROPHND));
      UtlAlloc( (PVOID *)&ptr1, 0L, (LONG)i, ERROR_STORAGE );
      if (!ptr1) {
        *pIda->pErrorInfo = Err_NoStorage;
        return( (PPROPHND) NULP);
      }
      memset( ptr1, NULC, i);
      UtlPlugIn( &ptr1->Plug, (PPLUG) NULP, pIda->ChainBlkHnd);
      if( !pIda->ChainBlkHnd)
        pIda->ChainBlkHnd = &ptr1->Plug;
      ptr1 = (PPROPHND)( (char *)ptr1 + sizeof( PLUG));
      pIda->TopFreeHnd = ptr1;
      for( ptr2= (PPROPHND) NULP,i= PROPHWND_ENTRIES-1; i; i--){
        UtlPlugIn( &ptr1->Plug, &ptr2->Plug,(PPLUG) NULP);
        ptr2 = ptr1;
        ptr1++;
      }
    }
    ptr1 = (PPROPHND)pIda->TopFreeHnd->Plug.Fw;
    ptr2 = (PPROPHND)UtlPlugOut( &pIda->TopFreeHnd->Plug);
    pIda->TopFreeHnd = ptr1;
    UtlPlugIn( &ptr2->Plug, (PPLUG) NULP, &pIda->TopUsedHnd->Plug);
    pIda->TopUsedHnd = ptr2;
//  pIda->TopUsedHnd->pHndID = pIda;   // for verification purposes
    return( ptr2);
}

/*!
     Release a property handle
*/
SHORT FreePropHnd( PPROP_IDA pIda, PPROPHND hprop)
{
    if( !hprop->Plug.Bw)               // is it the top most one ?
      pIda->TopUsedHnd = (PPROPHND)hprop->Plug.Fw;  // ..yes, set new anchor
    UtlPlugOut( &hprop->Plug);
    memset( hprop, NULC, sizeof( *hprop));
    UtlPlugIn( &hprop->Plug, (PPLUG) NULP, &pIda->TopFreeHnd->Plug);
    pIda->TopFreeHnd = hprop;          // set new anchor to free elements
    return( 0);
}


/*!
     Search for a property handle
*/
PPROPHND FindPropHnd( PPROPHND ptop, PPROPHND hprop)
{
//  nothing to be done in this release of EQF because hprop is the
//  requested pointer
//! check whether hprop is a valid handle
//  if( hprop->pHndID != testid)
//    return( NULP);
    return( hprop);
}


/*!
     Load the properties file
*/
PPROPCNTL LoadPropFile( PPROP_IDA pIda, PSZ pszName, PSZ pszPath, USHORT usAcc)
{
    PPROPCNTL     pcntl= (PPROPCNTL) NULP;          // Points to properties cntl buffer
    PROPHEAD      prophead;            // Properties heading area
    CHAR          fname[ MAX_EQF_PATH];// Temporary filename
    USHORT        sizeprop = 0;            // Size of properties area
    USHORT        size, sizread;       //
    HFILE         hf=NULLHANDLE;      // pointer to variable for file handle        */
    USHORT        usAction, usrc;
    BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

    *pIda->pErrorInfo = 0L;
    pcntl = FindPropCntl( pIda->TopCntl, pszName, pszPath);
    if( pcntl)
      if( (usAcc & PROP_ACCESS_WRITE) && (pcntl->lFlgs & PROP_ACCESS_WRITE)){
        *pIda->pErrorInfo = ErrProp_AccessDenied;
        return( (PPROPCNTL) NULP);
      } else {
        pcntl->usUser++;
        return( pcntl);
      }
    //MakePropPath( fname, "", pszPath, pszName, ""); // no drive, no .ext
    sprintf(fname, "%s%s", pszPath, pszName);
    do {
      usrc = UtlOpen( fname, &hf, &usAction, 0L,
                      FILE_NORMAL, FILE_OPEN,
                      OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, 0);

      if( usrc){
        *pIda->pErrorInfo = Err_OpenFile;
        break;
      }
      usrc = UtlRead( hf, &prophead, sizeof( prophead), &sizread, 0);
      if( usrc || (sizread != sizeof( prophead))){
        LogMessage4(ERROR, "LoadPropFile, couldn't read file: ", fname, ", bytes read: ", toStr(sizread).c_str());
        *pIda->pErrorInfo = Err_ReadFile;
        break;
      }
      
      if( (size = sizeprop = GetPropSize( prophead.usClass)) == 0)
      {
        *pIda->pErrorInfo = ErrProp_InvalidClass;
        break;
      }
      UtlAlloc( (PVOID *)&pcntl, 0L, (LONG)(size + sizeof( *pcntl)), ERROR_STORAGE );
      if( !pcntl){
        *pIda->pErrorInfo = Err_NoStorage;
        break;
      }
      size -= sizeof( prophead);       // sub bytes alread read
      memset( pcntl, NULC, sizeof( *pcntl));
      pcntl->pHead  = (PPROPHEAD)(pcntl+1);
      *pcntl->pHead = prophead;
      usrc = UtlRead( hf, pcntl->pHead+1, size, &sizread, 0);
      if( usrc || (sizread != size))
      {
        LogMessage6(ERROR,"UtlRead:: usrc = ",toStr(usrc).c_str(), "; sizread = ", toStr(sizread).c_str(), ", size = ", toStr(size).c_str());
        // for folder property files it is O.K. to read less than
        // size bytes
        if ( (prophead.usClass == PROP_CLASS_FOLDER) &&
             (sizread >= 2000) )
        {
          // continue ...
        }
        else if ( (prophead.usClass == PROP_CLASS_DOCUMENT) &&
             (sizread >= 2000) )
        {
          // may be a document property file created using a defect
          // OS/2 version which omitted the filler at the end of the
          // property structure
          // so continue ...
        }
        else if ( (prophead.usClass == PROP_CLASS_FOLDERLIST) &&
             (sizread >= 2000) )
        {
          // smaller property files from old versions are allowed...
        }
        else
        {
          LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED :: Err_ReadFile // files created in linux don't have that filler to fit 2k");
          // files created in linux don't have that filler to fit 2k
#ifdef TEMPORARY_COMMENTED
          *pIda->pErrorInfo = Err_ReadFile;
          #endif
          break;
        } /* endif */
      }
    } while( fTrueFalse /*TRUE & FALSE*/);
    if( hf)
      UtlClose( hf, 0);
    if( *pIda->pErrorInfo){
      UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
      return( (PPROPCNTL) NULP);
    }
//  initialize control block
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, (LONG) sizeof(OBJNAME), ERROR_STORAGE );
    strcpy( pcntl->pszFname, fname );
    pcntl->usFsize  = sizeprop;
    pcntl->usUser   = 1;
    pcntl->lFlgs    = usAcc & (PROP_ACCESS_READ | PROP_ACCESS_WRITE);
    UtlPlugIn( &pcntl->Plug, &pIda->LastCntl->Plug, (PPLUG) NULP);
    if( !pIda->TopCntl)
      pIda->TopCntl = pcntl;
    if( !pIda->LastCntl)
      pIda->LastCntl = pcntl;
    return( pcntl);
}


/*!
     Read in system properties
*/
PPROPSYSTEM GetSystemPropInstance();
USHORT GetSysProp( PPROP_IDA pIda)
{
    PPROPHND      hprop;               //
    PPROPCNTL     pcntl = NULL;        // Points to properties cntl buffer
    USHORT        size, sizread;       // Size of properties area
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc = 0;
    BOOL          error=TRUE;
    BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

    if( (hprop = MakePropHnd( pIda)) == NULL)
      return( TRUE );
    memset( hprop, NULC, sizeof( *hprop));

    do {
      LogMessage2(DEBUG, "GetSysProp::path = ", pIda->IdaHead.pszObjName);
      
      size = sizeof( PROPSYSTEM);
      auto pData = GetSystemPropInstance();    
      
      UtlAlloc( (PVOID *)&pcntl, 0L, (LONG)(size + sizeof( *pcntl)), ERROR_STORAGE );      
      if( !pcntl){ 
          break;
      }

      memset( pcntl, NULC, sizeof( *pcntl));
      pcntl->pHead = (PPROPHEAD)(pcntl+1);
      if( !pcntl){ 
          break;
      }
      memcpy(pcntl->pHead, pData, size);
    
      error = FALSE;
    } while( fTrueFalse /*TRUE & FALSE*/);

    if( error)
    {
      if ( pcntl )
      {
         UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
      } /* endif */
      return( TRUE );
    }
//  initialize control block
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, (LONG) sizeof(OBJNAME), ERROR_STORAGE );
    strcpy( pcntl->pszFname, pIda->IdaHead.pszObjName );
    pcntl->usFsize  = sizeof( PROPSYSTEM);
    pcntl->usUser = 1;
    pcntl->lFlgs  = PROP_ACCESS_READ;
    UtlPlugIn( &pcntl->Plug, &pIda->LastCntl->Plug, (PPLUG) NULP);
    if( !pIda->TopCntl)
      pIda->TopCntl = pcntl;
    if( !pIda->LastCntl)
      pIda->LastCntl = pcntl;

    hprop->pCntl = pcntl;
    hprop->lFlgs = PROP_ACCESS_READ;
    hprop->pCntl->usUser = 1;
    pIda->hSystem = hprop;             // anchor handle to system properties

    return( 0 );
}


/*!
     PutItAway
*/
USHORT PutItAway( PPROPCNTL pcntl)
{
    USHORT        sizwrite;            // number of bytes written
    HFILE         hf=NULLHANDLE;       // pointer to variable for file handle        */
    USHORT        usAction, usrc;

    // always reset updated flag even if save fails, otherwise
    // the properties cannot be closed
    pcntl->lFlgs &= ~PROP_STATUS_UPDATED;

    if ( (usrc = UtlOpen( pcntl->pszFname, &hf, &usAction, 0L,
                    FILE_NORMAL, FILE_OPEN,
                    OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE,
                    0L, 0)) != NO_ERROR )
    {
	  return( usrc);
	}

    usrc = UtlWrite( hf, pcntl->pHead, pcntl->usFsize, &sizwrite, 0);

    if( !usrc && (sizwrite != pcntl->usFsize))
      usrc = Err_WriteFile;
    UtlClose( hf, 0);
    return( usrc);
}


/*!
     Drop properties file from memory
*/
VOID DropPropFile( PPROP_IDA pIda, PPROPCNTL pcntl)
{
    UtlAlloc( (PVOID *)&pcntl->pszFname, 0L, 0L, NOMSG );
    if( pcntl->Plug.Fw == NULP)
      pIda->LastCntl = (PPROPCNTL)pcntl->Plug.Bw;
    if( pcntl->Plug.Bw == NULP)
      pIda->TopCntl = (PPROPCNTL)pcntl->Plug.Fw;
    UtlPlugOut( &pcntl->Plug);
    UtlAlloc( (PVOID *)&pcntl, 0L, 0L, NOMSG );
}

/*!
     Find Properties Control block
*/
PPROPCNTL FindPropCntl( PPROPCNTL ptop, PSZ pszName, PSZ pszPath)
{
    for(; ptop; ptop=(PPROPCNTL)ptop->Plug.Fw){
      if( !strcmp( ptop->pHead->szName, pszName)
       //&&!strcmp( ptop->pHead->szPath, pszPath)
       )
        return( ptop);
    }
    return( (PPROPCNTL) NULP);
}

/*!
     Load Properties Message area
*/
SHORT LoadPropMsg( PPROPMSGPARM pm, PPROPHND hprop, PSZ name, PSZ path, \
                    USHORT flg, PEQFINFO pErrorInfo, BYTE *buffer)
{
    pm->hObject = hprop;
    pm->pBuffer = buffer;
    pm->fFlg    = flg;
    pm->pErrorInfo = pErrorInfo;
    if( name || path)
      if( !path){
        strcpy( (PSZ)(pm->tmpName), name);
        if( ( pm->pszName = UtlSplitFnameFromPath((PSZ) pm->tmpName)) == NULL)
          return( TRUE );
        pm->pszPath = (PSZ)(pm->tmpName);
      } else {
        pm->pszName = name;
        pm->pszPath = path;
      }
    else {
        pm->pszName = (PSZ) NULP;
        pm->pszPath = (PSZ) NULP;
    }
    return( 0);              // no checks included now
}

/*!
     Notify All
*/
VOID NotifyAll( HPROP hprop)
{
    PPROPHEAD ph;
    char name[ MAX_EQF_PATH];

    if ( UtlQueryUShort( QS_RUNMODE ) != FUNCCALL_RUNMODE )
    {
      ph = (PPROPHEAD)(((PPROPHND)hprop)->pCntl->pHead);
      LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 98 strcat( strcat( strcpy( name, ph->szPath), \"\\\"), ph->szName);");
#ifdef TEMPORARY_COMMENTED
      strcat( strcat( strcpy( name, ph->szPath), "\\"), ph->szName);
      #endif
      EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                           MP1FROMSHORT( ph->usClass ),
                           MP2FROMP(name) );
    } /* endif */
}

/*!
    Open Properties

*/
HPROP OpenProperties( PSZ pszObjName, PSZ pszPath, USHORT usAccess,
                               PEQFINFO pErrorInfo)
{
   HPROP     hprop=NULL;
   PPROPMSGPARM pmsg = NULL;

   UtlAlloc( (PVOID *)&pmsg, 0L, (LONG)sizeof(PROPMSGPARM), ERROR_STORAGE );
   if ( !pmsg )
   {
      *pErrorInfo = Err_NoStorage;
      return( NULL );
   } /* endif */

   if( !pszObjName)
   {
     *pErrorInfo = ErrProp_InvalidObjectName;
   }
   else if( usAccess & ~PROP_ACCESS_READWRITE)
   {
     *pErrorInfo = ErrProp_InvalidAccess;
   }
   else if( LoadPropMsg( pmsg, NULL, pszObjName, pszPath, usAccess,
                         pErrorInfo, NULL ) )
   {
     *pErrorInfo = ErrProp_InvalidParms;
   }
   else
   {
     if(CheckLogLevel(DEBUG)) LogMessage1(WARNING,"if(true) hardcoded in OpenProperties");
     if (true ||  UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPHND      hProp;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        //GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = pmsg->pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return

        if( (hProp = MakePropHnd( pIda)) != NULL)
        {
          hProp->pCntl = LoadPropFile( pIda, pmsg->pszName, pmsg->pszPath, pmsg->fFlg);
          if( !hProp->pCntl )
          {
            FreePropHnd( pIda, hProp );
            hProp = NULL;
          } /* endif */
        } /* endif */

        if ( hProp != NULL )
        {
          hProp->lFlgs |= pmsg->fFlg;
          hprop = (HPROP)hProp;
        } /* endif */

        // release Mutex
        //RELEASEMUTEX(hMutexSem);
     }
     else
     {
       LogMessage2(ERROR,__func__, "::TEMPORARY_COMMENTED call EqfCallPropertyHandler ");
#ifdef TEMPORARY_COMMENTED
     hprop = (HPROP)EqfCallPropertyHandler( WM_EQF_OPENPROPERTIES,
                                            MP1FROMSHORT(0),
                                            MP2FROMP(pmsg) );
#endif
     } /* endif */
   } /* endif */
   UtlAlloc( (PVOID *)&pmsg, 0L, 0L, NOMSG );

   return( hprop);
}


/*!
    Close Properties
*/
SHORT CloseProperties(
   HPROP     hObject,                 // Handle to object properties *input
   USHORT    fClose,                  // Flags for closeing          *input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = TRUE;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127

   do {
     if( !hObject)
     {
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }

     if( LoadPropMsg( &PropMsg, (PPROPHND) hObject, NULL, NULL, fClose,
                      pErrorInfo, NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     
     if(CheckLogLevel(DEBUG))  LogMessage1(WARNING, "if(true) hardcoded in CloseProperties");
     if (true || UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hProp = NULL;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        //GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hProp=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) PropMsg.hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;        // indicate error
        }
        else
        {
          pcntl = hProp->pCntl;
          if( pcntl->lFlgs & PROP_STATUS_UPDATED)
            if( PropMsg.fFlg & PROP_FILE)
            {
              if( !(hProp->lFlgs & PROP_ACCESS_WRITE))
              {
                *pIda->pErrorInfo = ErrProp_AccessDenied;
                rc =  -2;        // indicate error
              }
              else if( (*pIda->pErrorInfo = PutItAway( pcntl)) != 0 )
              {
                rc = -3;        // indicate error
              }
            }
          if ( rc == TRUE )
          {
            if( --pcntl->usUser < 1)
              DropPropFile( pIda, pcntl);
            else
              if( hProp->lFlgs & PROP_ACCESS_WRITE)
                pcntl->lFlgs &= ~PROP_ACCESS_WRITE;
            FreePropHnd( pIda, hProp);
            rc = *pIda->pErrorInfo ? -4 : 0;
          } /* endif */
        } /* endif */

        // release Mutex
        //RELEASEMUTEX(hMutexSem);
     }
     else
     {
       LogMessage1(WARNING,"TEMPORARY_COMMENTED in Close Properties, SHORT1FROMMR");
#if TEMPORARY_COMMENTED
       rc = SHORT1FROMMR(EqfCallPropertyHandler( WM_EQF_CLOSEPROPERTIES,
                                                 MP1FROMSHORT(0),
                                                 MP2FROMP(&PropMsg) ) );
#endif
     } /* endif */

   } while( fTrueFalse /*TRUE & FALSE*/);

   return( rc);
}


/*!
    Save Properties
*/
SHORT SaveProperties(
   HPROP     hObject,                 // Handle to object properties *input
   PEQFINFO  pErrorInfo               // Error indicator           * output
)
{
   PROPMSGPARM PropMsg;                // buffer for message structure

   SHORT rc = 0;
   BOOL fTrueFalse = TRUE&FALSE;    // to avoid compile-w C4127
   do {
     if( !hObject)
     {
       *pErrorInfo = ErrProp_InvalidHandle;
       break;
     }

     if( LoadPropMsg( &PropMsg, (PPROPHND) hObject, NULL, NULL, 0,
                      pErrorInfo, NULL))
     {
       *pErrorInfo = ErrProp_InvalidParms;
       break;
     }
     if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
     {
        PPROP_IDA     pIda;           // Points to instance area
        PPROPCNTL     pcntl;
        PPROPHND      hprop;
        HANDLE hMutexSem = NULL;

        // keep other process from doing property related stuff..
        //GETMUTEX(hMutexSem);

        pIda = pPropBatchIda;
        pIda->pErrorInfo = PropMsg.pErrorInfo;
        *pIda->pErrorInfo = 0L;      // assume a good return
        if( (hprop=FindPropHnd( pIda->TopUsedHnd, (PPROPHND) PropMsg.hObject)) == NULL)
        {
          *pIda->pErrorInfo = ErrProp_InvalidHandle;
          rc = -1;
        }
        else
        {
          pcntl = hprop->pCntl;
          if( pcntl->lFlgs & PROP_STATUS_UPDATED)
          {
            if( !(hprop->lFlgs & PROP_ACCESS_WRITE))
            {
              *pIda->pErrorInfo = ErrProp_AccessDenied;
              rc = -2;
            }
            else
            {
              *pIda->pErrorInfo = PutItAway( pcntl);
            } /* endif */
          }
          if ( !rc ) rc = *pIda->pErrorInfo ? -3 : 0;
        } /* endif */

        // release Mutex
       //RELEASEMUTEX(hMutexSem);
     }
     else
     {
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 100 rc = SHORT1FROMMR( EqfCallPropertyHandler( WM_EQF_SAVEPROPERTIES, MP1FROMSHORT(0), MP2FROMP(&PropMsg) ) );");
#ifdef TEMPORARY_COMMENTED
       rc = SHORT1FROMMR( EqfCallPropertyHandler( WM_EQF_SAVEPROPERTIES,
                                           MP1FROMSHORT(0),
                                           MP2FROMP(&PropMsg) ) );
#endif
     } /* endif */
 } while( fTrueFalse /*TRUE & FALSE*/);

   return( rc);
}


/*!
    Miscellaneous properties functions
*/
PVOID MakePropPtrFromHnd( HPROP hprop)
{
     return( hprop ? (PVOID)(((PPROPHND)hprop)->pCntl->pHead) : NULP);
}
PVOID MakePropPtrFromHwnd( HWND hObject)
{
   //PIDA_HEAD pIda = ACCESSWNDIDA( hObject, PIDA_HEAD );
   // return( pIda ? MakePropPtrFromHnd( pIda->hProp) : NULL);
}

PPROPSYSTEM GetSystemPropPtr( VOID )
{
    HPROP hSysProp;
    //if(CheckLogLevel(DEBUG))  LogMessage1(WARNING, "if(true) hardcoded in GetSystemPropPtr");
    if (true || UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
    {
      PPROP_IDA     pIda;           // Points to instance area
      pIda = pPropBatchIda;
      hSysProp = pIda->hSystem;

    }
    else
    {
      hSysProp = EqfQuerySystemPropHnd();
      
      LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 101 assert( hSysProp != NULL );");
#ifdef TEMPORARY_COMMENTED
      assert( hSysProp != NULL );
      #endif
    } /* endif */
    return( (PPROPSYSTEM)( MakePropPtrFromHnd( hSysProp )));
 }


BOOL SetPropAccess( HPROP hprop, USHORT flgs)
{
    if( flgs & PROP_ACCESS_WRITE){
      if( ((PPROPHND)hprop)->lFlgs & (ULONG)PROP_ACCESS_WRITE)
       return( FALSE);
      if( ((PPROPHND)hprop)->pCntl->lFlgs & (ULONG)PROP_ACCESS_WRITE)
       return( FALSE);
      ((PPROPHND)hprop)->pCntl->lFlgs |= (ULONG)(PROP_ACCESS_WRITE | \
                                                 PROP_STATUS_UPDATED);
      ((PPROPHND)hprop)->lFlgs |= (ULONG)PROP_ACCESS_WRITE;
    }
    return( TRUE);
}
VOID ResetPropAccess( HPROP hprop, USHORT flgs)
{
    ((PPROPHND)hprop)->pCntl->lFlgs &= (ULONG)~flgs;
    ((PPROPHND)hprop)->lFlgs &= (ULONG)~flgs;
    if( flgs & PROP_ACCESS_WRITE) 
        NotifyAll( hprop);
}

/*!
     GetPropSize - return size of properties given by its class
*/
SHORT GetPropSize( USHORT usClass)
{
   USHORT usSize;                      // size of properties

   switch ( usClass )
   {
      case PROP_CLASS_SYSTEM :
         usSize = sizeof( PROPSYSTEM );
         break;
      case PROP_CLASS_FOLDERLIST :
         usSize = sizeof( PROPFOLDERLIST );
         break;
      case PROP_CLASS_FOLDER :
         usSize = sizeof( PROPFOLDER );
         break;
      case PROP_CLASS_DOCUMENT :
         usSize = sizeof( PROPDOCUMENT );
         break;
      case PROP_CLASS_IMEX :
         usSize = sizeof( PROPIMEX );
         break;
      case PROP_CLASS_EDITOR :
         usSize = sizeof( PROPEDIT );
         break;
      case PROP_CLASS_DICTIONARY:
         usSize = sizeof( PROPDICTIONARY );
         break;
      case PROP_CLASS_DICTLIST:
         usSize = sizeof( PROPDICTLIST );
         break;
      case PROP_CLASS_TAGTABLE:
         usSize = sizeof( PROPTAGTABLE );
         break;
      case PROP_CLASS_LIST:
         usSize = sizeof( PROPLIST );
         break;
      case PROP_CLASS_MEMORY :
      case PROP_CLASS_MEMORYDB :
      case PROP_CLASS_MEMORY_LASTUSED :
        if(CheckLogLevel(DEBUG)) LogMessage1(WARNING, "if(true) hardcoded in GetPropSize::PROP_CLASS_MEMORY");
         if (true || UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
         {
           usSize = 2048; // MEM_PROP_SIZE;
         }
         else
         {
           usSize = (USHORT) EqfSend2Handler( MEMORYHANDLER, WM_EQF_QUERYPROPSIZE, MP1FROMSHORT(usClass), 0L);
         } /* endif */
         break;
      case PROP_CLASS_TQMLIST :
         usSize = (USHORT) EqfSend2Handler( TQMLISTHANDLER, WM_EQF_QUERYPROPSIZE, MP1FROMSHORT(usClass), 0L);
         break;
      default :
         usSize = 0;
         break;
   } /* endswitch */
    return( usSize );
}

/*!
     MakePropPath
*/
PSZ MakePropPath( PSZ pbuf, PSZ pd, PSZ pp, PSZ pn, PSZ pe)
{
    PPROPSYSTEM pprop;
    PPROPHND    hprop;
    CHAR        tmppath[ MAX_EQF_PATH];
    *pbuf = NULC;
    if( (hprop = (PPROPHND) EqfQuerySystemPropHnd())== NULL)
      return( pbuf);
    pprop = (PPROPSYSTEM) MakePropPtrFromHnd( hprop);
    sprintf( tmppath, "%s/%s", pp, pprop->szPropertyPath );
    //_makepath( pbuf, pd, tmppath, pn, pe);
    return( pbuf);
}


// Function PropHandlerInitForBatch
// Initialize the property handler for non-windows environments;
// i.e. perform WM_CREATE handling to allocate our IDA
BOOL PropHandlerInitForBatch( void )
{
  int size;
  BOOL fOK = TRUE;

  if( !UtlAlloc( (PVOID *)&pPropBatchIda, 0L, (LONG)sizeof( *pPropBatchIda), ERROR_STORAGE ) )
    return( FALSE );      // do not create the window
  memset( pPropBatchIda, NULC, sizeof( *pPropBatchIda));
  size = sizeof(pPropBatchIda->IdaHead.szObjName);
  pPropBatchIda->IdaHead.pszObjName = pPropBatchIda->IdaHead.szObjName;
  //properties_get_str(KEY_OTM_DIR, pPropBatchIda->IdaHead.pszObjName, size);
  //strcat(pPropBatchIda->IdaHead.pszObjName, "/EQFSYSW.PRP");
  strcpy(pPropBatchIda->IdaHead.pszObjName, SYSTEM_PROPERTIES_NAME);

  pthread_mutex_t mutex;
  { // keep other process from doing property related stuff..
	  //pthread_mutex_lock(&mutex);
    if( GetSysProp( pPropBatchIda))
      fOK = FALSE;
    //pthread_mutex_unlock(&mutex);
   }

 return( fOK );
} /* end of function PropHandlerInitForBatch */


#define ACCESSWNDIDA( hwnd, type ) \
    (type)GetWindowLong( hwnd, GWL_USERDATA /*0*/ )

HPROP EqfQuerySystemPropHnd( void )
{
  HPROP hprop;

  LogMessage1(WARNING, "hardcoded if(true) in EqfQuerySystemPropHnd");
  if (true || UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE )
  {
    PPROP_IDA pIda = pPropBatchIda;
    if(pIda){
      hprop = pIda->hSystem;
    }else{
      hprop = NULL;
      LogMessage1(ERROR,"pId and pPropBatchIda are NULL");
    }
  }
  else
  {
//    hprop = (HPROP)WinSendMsg( EqfQueryHandler( PROPERTYHANDLER),
//                                        WM_EQF_QUERYSYSTEMPROPHND,
//                                        NULL, NULL);
      if (hwndPropertyHandler)
      {
      	//PPROP_IDA pIda = ACCESSWNDIDA( hwndPropertyHandler, PPROP_IDA );
      	//hprop = (HPROP)pIda->hSystem;
	    }
	    else
	    {
		    hprop = NULL;
	    }
  } /* endif */
  return( hprop );
} /* end of function EqfQuerySystemPropHnd */
