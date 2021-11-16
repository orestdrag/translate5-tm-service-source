/*!
 UtlMemory.c - memory functions
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2017, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include <malloc.h>

#include "EQF.H"                  // General Translation Manager include file
#include "LogWrapper.h"
#include "ThreadingWrapper.h"
#define STATIC_OWNER
#include "Utility.h"
#undef STATIC_OWNER

PVOID UtlIntAlloc
(
  ULONG  ulLength,                     // length of area to allocate
  USHORT usMessageNo                   // message number for error calls
);
static
USHORT UtlSubAlloc
(
  PBYTE pSel,
  PBYTE * ppStorage,
  ULONG  ulAllocLength
);
USHORT UtlIntFree
(
  PVOID  pOldStorage,
  ULONG  ulLength
);

BOOL UtlAlloc
(
   PVOID *ppStorage,                   // pointer to allocated memory area
   LONG   lOldLength,                  // old length of storage area
   LONG   lNewLength,                  // length of area to be allocated
   USHORT usMessageNo)                 // message to be displayed when error
{
  BOOL fSuccess = TRUE;
  PVOID pTemp;
  /********************************************************************/
  /* check mode of operation                                          */
  /********************************************************************/
  if ( (lOldLength == 0L) && (lNewLength != 0L) )    // allocate new memory
  {
    {
      /******************************************************************/
      /* allocate stuff                                                 */
      /******************************************************************/
      *ppStorage = UtlIntAlloc( lNewLength, usMessageNo );
      fSuccess =( *ppStorage != NULL );
    } /* endif */
  }
  else                                 // reallocate or free
  {
    if ( lNewLength == 0L )
    {
      if ( *ppStorage )
      {
        fSuccess = (BOOL) !UtlIntFree( *ppStorage, lOldLength );
        if ( fSuccess )
        {
          *ppStorage = NULL;
        } /* endif */
      } /* endif */
    }
    else
    {
      /******************************************************************/
      /* reallocate stuff                                               */
      /******************************************************************/
      pTemp = UtlIntAlloc( lNewLength, usMessageNo );
      if ( pTemp )
      {
        memcpy( pTemp, *ppStorage, get_min(lNewLength,lOldLength) );
        fSuccess = (BOOL) ! UtlIntFree( *ppStorage, lOldLength );
        *ppStorage = pTemp;
      }
      else
      {
        fSuccess = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  return fSuccess;
} /* end of function UtlAlloc */

//+----------------------------------------------------------------------------+
//|External Function                                                           |
//+----------------------------------------------------------------------------+
//|Function Name:     UtlAllocHwnd           Interface for UtlAlloc            |
//+----------------------------------------------------------------------------+
//|Description:       Call UtlAlloc and use given window handle for error      |
//|                   messages.                                                |
//+----------------------------------------------------------------------------+
//|Function Call:     BOOL UtlAllocHwnd( PVOID  *ppMemory                      |
//|                                      LONG   lOldLength,                    |
//|                                      LONG   lNewLength,                    |
//|                                      USHORT usMsgNo,                       |
//|                                      HWND   hwndOwner );                   |
//+----------------------------------------------------------------------------+
BOOL UtlAllocHwnd
(
   PVOID *ppStorage,                   // pointer to allocated memory area
   LONG   lOldLength,                  // old length of storage area
   LONG   lNewLength,                  // length of area to be allocated
   USHORT usMessageNo,                 // message to be displayed when error
   HWND   hwnd )                       // handle for UtlError Hwnd call
{
  BOOL fOK = UtlAlloc( ppStorage, lOldLength, lNewLength, NOMSG );

  hwnd;
  usMessageNo;
  if ( !fOK && (usMessageNo != NOMSG ))
  {
    LogMessage3(ERROR, __func__, "::rc = ", intToA(usMessageNo) );
  } /* endif */
  return( fOK );
} /* end of UtlAllocHwnd */

PVOID UtlIntAlloc
(
  ULONG  ulLength,                     // length of area to allocate
  USHORT usMessageNo                   // message number for error calls
)
{
  PBYTE  pStorage;                     // pointer to allocated storage

    pStorage = (PBYTE) malloc( ulLength + 2 * sizeof(ULONG) );

  /********************************************************************/
  /* preset head and trailer as anchor for kamikaze persons....       */
  /********************************************************************/

  if ( pStorage )
  {
    *((PULONG) pStorage ) = ulLength;
    pStorage = (PBYTE) pStorage + sizeof(ULONG);
    *( (PULONG) ((PBYTE)pStorage + ulLength)) = ulLength;
    memset( pStorage, 0, ulLength );
  }
  else 
  {
    if ( usMessageNo != NOMSG )
    {
      UtlError( usMessageNo, MB_CANCEL, 0, NULL, EQF_ERROR );
    } /* endif */
  } /* endif */
  return( pStorage );
}

/**********************************************************************/
/* do an efficient sub allocation...                                  */
/**********************************************************************/
static
USHORT UtlSubAlloc
(
  PBYTE pSel,
  PBYTE * ppStorage,
  ULONG  ulAllocLength
)
{
  USHORT usRc = 0;
  PSUBALLOC_HDR pSubAlloc = (PSUBALLOC_HDR) pSel;
  PBYTE  pData = pSel + sizeof( SUBALLOC_HDR );
  USHORT usLast;

  *ppStorage = NULL;
  ulAllocLength = 0;

  /********************************************************************/
  /* check if we can find a free slot ...                             */
  /********************************************************************/
  usLast = pSubAlloc->usLastUsed;
  pData += (ULONG)pSubAlloc->usLastUsed * (ULONG)pSubAlloc->usSizeOfBlock;

  while ( *pData && ( usLast < pSubAlloc->usNumOfBlocks - 1 ))
  {
    pData += pSubAlloc->usSizeOfBlock;
    usLast++;
  } /* endwhile */

  /********************************************************************/
  /* check if we find a free slot, otherwise wrap around              */
  /********************************************************************/
  if ( usLast == pSubAlloc->usNumOfBlocks-1 )
  {
    /******************************************************************/
    /* start at beginning ...                                         */
    /******************************************************************/
    usLast = 0;
    pData = pSel + sizeof( SUBALLOC_HDR );
    while ( *pData && ( usLast < pSubAlloc->usLastUsed ))
    {
      pData += pSubAlloc->usSizeOfBlock;
      usLast++;
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* return the found slot or an error condition                      */
  /********************************************************************/
  if ( *pData )
  {
    usRc = 1;
  }
  else
  {
    *pData = 1;
    *ppStorage = pData+1;
    pSubAlloc->usLastUsed = usLast;
  } /* endif */

  return usRc;
}

USHORT UtlIntFree
(
  PVOID  pOldStorage,
  ULONG  ulLength
)
{
  USHORT usRC = 0;
  PBYTE  pStorage = (PBYTE) pOldStorage - sizeof(ULONG);
  ULONG  ulActLength = *((PULONG) pStorage );
  ULONG  ulActLength2 = *((PULONG)(((PBYTE) pOldStorage) + ulActLength));

  ulLength;
  /********************************************************************/
  /* check if someone tried to play KAMIKAZE with our memory,         */
  /* fuck him....                                                     */
  /********************************************************************/
  if ( ulActLength != ulActLength2 )
  {
    /******************************************************************/
    /* display error message - someone killed us...                   */
    /******************************************************************/
    usRC = ERROR_INTERNAL;
    LogMessage5(FATAL, __func__,":: someone tried to play KAMIKAZE with our memory ulActLength != ulActLength2, ulActLength = ", 
      intToA(ulActLength), "; ulActLength2 = ", intToA(ulActLength2));
    //UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
  }
  else
  {
    /******************************************************************/
    /* free the momory                                                */
    /******************************************************************/
     free( pStorage );
  } /* endif */
  return usRC;
} /* end of function UtlIntFree */

/**********************************************************************/
/*  get the correct task id ...                                       */
/**********************************************************************/
USHORT UtlGetTask ( void )
{
  int   usTask;
  USHORT currTask;
  
LogMessage2(ERROR,__func__, ":: TEMPORARY_COMMENTED temcom_id = 150");
#ifdef TEMPORARY_COMMENTED
        __asm__
          (
            "MOV      %ax, %SS;"
            "MOV      (usTask), %ax;"
          );
#endif //TEMPORARY_COMMENTED
  usTask = _getpid();
  for ( currTask = 0; currTask < MAX_TASK ; ++currTask )
  {
    if ( UtiVar[currTask].usTask == usTask )
    {
      LogMessage2(DEVELOP, "UtlGetTask()::Task found, id = ", intToA(usTask));
      break;
    }
    else if ( UtiVar[currTask].usTask == 0 )
    {
      /****************************************************************/
      /* empty slot found                                             */
      /****************************************************************/
      SEGTABLE SegTable[MAX_MEM_TABLES] =  // table of segment tables
          { {  NULL, SEG_TABLE_EMPTY,     16,    16,    0 },
            {  NULL, SEG_TABLE_EMPTY,     64,    64,    0 },
            {  NULL, SEG_TABLE_EMPTY,    128,   128,    0 },
            {  NULL, SEG_TABLE_EMPTY,    256,   256,    0 },
            {  NULL, SEG_TABLE_EMPTY,    512,   512,    0 },
            {  NULL, SEG_TABLE_EMPTY,   1020,  1020,    0 },
            {  NULL, SEG_TABLE_EMPTY,   2040,  2040,    0 },
            {  NULL, SEG_TABLE_EMPTY, 0xFFFF,     1,    0 } };
      UtiVar[currTask].usTask = usTask;
      memcpy(&UtiVar[currTask].SegTable, &SegTable,
             sizeof(SEGTABLE) * MAX_MEM_TABLES );
      LogMessage4(INFO, "UtlGetTask():: allocated segtable, usTask = ",intToA(usTask), ", currTask = ", intToA(currTask));
      break;
    } /* endif */
  } /* endfor */
  return( currTask );
}
