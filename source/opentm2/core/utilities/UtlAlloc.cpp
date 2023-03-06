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
    T5LOG(T5ERROR) << "::rc = " << usMessageNo;
  } /* endif */
  return( fOK );
} /* end of UtlAllocHwnd */

PVOID UtlIntAlloc
(
  ULONG  ulLength,                     // length of area to allocate
  USHORT usMessageNo                   // message number for error calls
)
{
  PBYTE pStorage = (PBYTE) malloc( ulLength + 2 * sizeof(ULONG) );

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
    T5LOG(T5FATAL) << "::Cant allocate memory size = " << ulLength;
  } /* endif */
  return( pStorage );
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
    //usRC = ERROR_INTERNAL;
    //T5LOG(T5FATAL) << ":: someone tried to play KAMIKAZE with our memory ulActLength != ulActLength2, ulActLength = "<<
    //  ulActLength << "; ulActLength2 = " << ulActLength2;
    //UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
  }
  /******************************************************************/
  /* free the momory                                                */
  /******************************************************************/
  free( pStorage );
  
  return usRC;
} /* end of function UtlIntFree */

/**********************************************************************/
/*  get the correct task id ...                                       */
/**********************************************************************/
USHORT UtlGetTask ( void )
{
  int   usTask;
  USHORT currTask;
  
  usTask = _getpid();
  for ( currTask = 0; currTask < MAX_TASK ; ++currTask )
  {
    if ( UtiVar[currTask].usTask == usTask )
    {
      T5LOG( T5DEVELOP) << "UtlGetTask()::Task found, id = " << usTask ;
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
      if(VLOG_IS_ON(1))
        T5LOG( T5DEBUG) << "UtlGetTask():: allocated segtable, usTask = " << usTask << ", currTask = " << currTask;
      break;
    } /* endif */
  } /* endfor */
  return( currTask );
}
