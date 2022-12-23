//+----------------------------------------------------------------------------+
//|EQFNTMDB.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  TranslationMemory Layer for QDAM functions                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.3 $ ----------- 13 Jan 2005
// GQ: - removed compiler warnings
// 
// 
// $Revision: 1.2 $ ----------- 11 Jan 2005
// GQ: - added function EQFNTMOrganizeIndex to compact the index part of a memory
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.3 $ ----------- 31 Oct 2001
//
//
// $Revision: 1.2 $ ----------- 22 Oct 2001
// -- RJ: get rid of compiler warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFNTMDB.CV_   1.4   09 Nov 1998 09:39:38   BUILD  $
 *
 * $Log:   K:\DATA\EQFNTMDB.CV_  $
 *
 *    Rev 1.4   09 Nov 1998 09:39:38   BUILD
 * - continued work on new TM approach
 *
 *    Rev 1.3   29 Sep 1998 07:31:34   BUILD
 * - adapted to ULONG length functions in QDAM
 *
 *    Rev 1.2   20 Dec 1996 09:57:14   BUILD
 * -- KAT0254: correct plausibility check for key (use '>' instead of '>=')
 *
 *    Rev 1.1   30 Oct 1996 19:38:14   BUILD
 * - do not write event log for BTREE_NOT_FOUND condition in EQFNTMGet
 *
 *    Rev 1.0   09 Jan 1996 09:16:48   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD
#define INCL_EQF_DAM
#include "EQF.H"
#include "EQFQDAMI.H"
#include "EQFEVENT.H"                  // event logging facility
#include "LogWrapper.h"
#include "FilesystemWrapper.h"
#include "PropertyWrapper.H"

UCHAR  ucbEncodeTbl[30]
        =  { 00,  06,
             97,  32, 101,  65, 116, 110, 105, 115, 114,  99, 111,
             14, 100, 108, 117, 104,  98, 103,  71, 102, 109, 112,  10,
             02,  03,  04,  05 };


//#define TEMPORARY_HARDCODED_SEPARATE_KEYS

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMCreate                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMCreate( pTMName, pUserData, usLen, ulStart, &pNTM);|
//+----------------------------------------------------------------------------+
//|Description:       This function will create the appropriate Transl.Memory  |
//|                   file.                                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pTMName,         name of file to be created         |
//|                   PCHAR  pUserData,     user data                          |
//|                   USHORT usLen,         length of user data                |
//|                   ULONG  ulStartKey,    first key to start automatic insert|
//|                   PBTREE * ppBTIda      pointer to structure               |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_NO_ROOM     memory shortage                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_OPEN_ERROR  dictionary already exists              |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//+----------------------------------------------------------------------------+
//|Function flow:     use standard DictCreateLocal with some modified parts..  |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMCreate
(
   PSZ  pTMName,                       // name of file to be created
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   ULONG  ulStartKey,                  // first key to start automatic insert...
   PVOID  * ppBTIda                    // pointer to structure
)
{
   SHORT sRc = 0;                      // return code
   PBTREE  pBTIda;
   NTMVITALINFO NtmVitalInfo;          // structure to contain vital info for TM

   NtmVitalInfo.ulStartKey = NtmVitalInfo.ulNextKey = ulStartKey;


  UtlAlloc( (PVOID *)&pBTIda, 0L, (LONG) sizeof(BTREE), NOMSG );
  if ( !pBTIda )
  {
    sRc = BTREE_NO_ROOM;
  }
  else
  {
    *ppBTIda = pBTIda;                     // set pointer to base structure

    /******************************************************************/
    /* move @@@@ line one below and you will get compression active   */
    /******************************************************************/
    sRc = QDAMDictCreateLocal( pTMName, 20, pUserData, usLen,
//@@@@                        ucbEncodeTbl,
                              NULL,
                              NULL, NULL, (PPBTREE) ppBTIda,
                              &NtmVitalInfo );
    if ( sRc == BTREE_OPEN_ERROR )
    {
       UtlAlloc( (PVOID *)&(pBTIda->pBTree), 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)ppBTIda, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( sRc );
} /* end of function EQFNTMCreate */

SHORT
QDAMCheckDict
(
  PSZ    pName,                        // name of dictionary
  PBTREE pBTIda                        // pointer to ida
);


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMOpen     Open a Translation Memory file            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMOpen( PSZ, USHORT, PPBTREE );                      |
//+----------------------------------------------------------------------------+
//|Description:       Open a file locally for processing                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ              name of the index file                  |
//|                   USHORT           open flags                              |
//|                   PPBTREE          pointer to btree structure              |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_NO_ROOM     memory shortage                        |
//|                   BTREE_OPEN_ERROR  dictionary already exists              |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_ILLEGAL_FILE not a valid dictionary                |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//+----------------------------------------------------------------------------+
//|Function flow:     Open the file read/write or readonly                     |
//|                   if ok                                                    |
//|                     read in header                                         |
//|                     if ok                                                  |
//|                       check for validity of the header                     |
//|                       if ok                                                |
//|                         fill tree structure                                |
//|                         call UtlQFileInfo to get the next free record      |
//|                         if ok                                              |
//|                           allocate buffer space                            |
//|                           if alloc fails                                   |
//|                             set Rc = BTREE_NO_ROOM                         |
//|                           endif                                            |
//|                         endif                                              |
//|                       endif                                                |
//|                     else                                                   |
//|                       set Rc = BTREE_READ_ERROR                            |
//|                     endif                                                  |
//|                     if error                                               |
//|                       close dictionary                                     |
//|                     endif                                                  |
//|                     if read/write open was done                            |
//|                       set open flag and write it into file                 |
//|                     endif                                                  |
//|                     get corruption flag and set rc if nec.                 |
//|                   else                                                     |
//|                    set Rc = BTREE_OPEN_ERROR                               |
//|                   endif                                                    |
//|                   return Rc                                                |
// ----------------------------------------------------------------------------+

SHORT  EQFNTMOpen
(
  PSZ   pName,                        // name of the file
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PVOID  * ppBTIda                    // pointer to BTREE structure
)
{
   PBTREE    pBTIda;                   // pointer to BTRee structure
   SHORT     sRc = 0;                  // return code

   DEBUGEVENT( EQFNTMOPEN_LOC, FUNCENTRY_EVENT, 0 );
  

   if ( ! UtlAlloc( (PVOID *)&pBTIda, 0L , (LONG) sizeof( BTREE ), NOMSG )  )
   {
      sRc = BTREE_NO_ROOM;
   }
   else
   {
     /*****************************************************************/
     /* check if same dictionary is already open else return index    */
     /* of next free slot...                                          */
     /*****************************************************************/
     sRc = QDAMCheckDict( pName, pBTIda );
     if ( !sRc )
     {
       /***************************************************************/
       /* check if dictionary is locked                               */
       /***************************************************************/
       if ( ! pBTIda->usDictNum )
       {
         SHORT  RetryCount;                  // retry counter for in-use condition
         RetryCount = MAX_RETRY_COUNT;
         do
         {
           sRc = QDAMDictOpenLocal( pName, 20, usOpenFlags, &pBTIda );
           if ( sRc == BTREE_IN_USE )
           {
             RetryCount--;
             UtlWait( MAX_WAIT_TIME );
             if ( RetryCount > 0 )
             {
               /*******************************************************/
               /* re-allocate PBTIDA (has been de-allocated within    */
               /* QDAMDictOpenLocal due to error code)                */
               /*******************************************************/
               if ( ! UtlAlloc( (PVOID *)&pBTIda, 0L , (LONG) sizeof( BTREE ), NOMSG )  )
               {
                  sRc = BTREE_NO_ROOM;
               } /* endif */
             } /* endif */
           } /* endif */
         } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
                   (RetryCount > 0) ); /* enddo */
       }
       else
       {
          sRc = ( usOpenFlags & ASD_LOCKED ) ? BTREE_DICT_LOCKED : sRc;
       } /* endif */
     } /* endif */
   } /* endif */

   *ppBTIda = pBTIda;                               // set base pointer

   if ( sRc != NO_ERROR )
   {
     ERREVENT( EQFNTMOPEN_LOC, ERROR_EVENT, sRc );
   } /* endif */

   DEBUGEVENT( EQFNTMOPEN_LOC, FUNCEXIT_EVENT, 0 );

   return ( sRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMClose  close the TM file                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMClose( PPBTREE );                                  |
//+----------------------------------------------------------------------------+
//|Description:       Close the file                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PPBTREE                pointer to btree structure        |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_INVALID     incorrect pointer                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//|                   BTREE_CLOSE_ERROR error closing dictionary               |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictCloseLocal routine ...                      |
// ----------------------------------------------------------------------------+

SHORT EQFNTMClose
(
  PVOID * ppBTIda
)
{
  SHORT sRc;

  DEBUGEVENT( EQFNTMCLOSE_LOC, FUNCENTRY_EVENT, 0 );

  if ( *ppBTIda )
  {
    sRc = QDAMDictCloseLocal( (PBTREE) *ppBTIda );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( !sRc )
  {
    UtlAlloc( ppBTIda, 0L, 0L, NOMSG );
  } /* endif */

  if ( sRc != NO_ERROR )
  {
    ERREVENT( EQFNTMCLOSE_LOC, ERROR_EVENT, sRc );
  } /* endif */

  DEBUGEVENT( EQFNTMCLOSE_LOC, FUNCEXIT_EVENT, 0 );

  return sRc;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMUpdSign  Write User Data                           |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       Writes the second part of the first record (user data)   |
//|                   This is done using the original QDAMDictUpdSignLocal     |
//|                   function                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PCHAR                  pointer to user data              |
//|                   USHORT                 length of user data               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_INVALID     pointer invalid                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//+----------------------------------------------------------------------------+
//|NOTE:              This function could be implemented as MACRO too, but     |
//|                   for consistency reasons, the little overhead was used... |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictUpdSignLocal ...                            |
// ----------------------------------------------------------------------------+

SHORT EQFNTMUpdSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   USHORT usLen                        // length of user data
)
{
  return( QDAMDictUpdSignLocal( (PBTREE) pBTIda, pUserData, usLen ) );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMSign      Read signature record                    |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );             |
//+----------------------------------------------------------------------------+
//|Description:       Gets the second part of the first record ( user data )   |
//|                   This is done using the original QDAMDictSignLocal func.  |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PCHAR                  pointer to user data              |
//|                   PUSHORT                length of user data area (input)  |
//|                                          filled length (output)            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_INVALID     pointer invalid                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_NO_BUFFER   no buffer free                         |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      return signature record even if dictionary is corrupted  |
//+----------------------------------------------------------------------------+
//|NOTE:              This function could be implemented as MACRO too, but     |
//|                   for consistency reasons, the little overhead was used... |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictSignLocal ....                              |
// ----------------------------------------------------------------------------+

SHORT EQFNTMSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  SHORT sRc;                           // function return code
  SHORT RetryCount;                    // retry counter for in-use condition

  RetryCount = MAX_RETRY_COUNT;
  do
  {
    sRc = QDAMDictSignLocal( (PBTREE) pBTIda, pUserData, pusLen );
    if ( sRc == BTREE_IN_USE )
    {
      RetryCount--;
      UtlWait( MAX_WAIT_TIME );
    } /* endif */
  } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
            (RetryCount > 0) ); /* enddo */

  if ( sRc != NO_ERROR )
  {
    ERREVENT( EQFNTMSIGN_LOC, ERROR_EVENT, sRc );
  } /* endif */


  return( sRc );

}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMInsert                                             |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMInsert( pBTIda, &ulKey, pData, usLen );      |
//+----------------------------------------------------------------------------+
//|Description:       insert a new key (ULONG) with data                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE  pBTIda,      pointer to binary tree struct       |
//|                   PULONG  pulKey,      pointer to key                      |
//|                   PBYTE   pData,       pointer to user data                |
//|                   USHORT  usLen        length of user data                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range  |
//|                   BTREE_READONLY       file is opened read only - no write |
//|                   BTREE_CORRUPTED      file is corrupted                   |
//|                   errors returned by QDAMDictInsertLocal                   |
//|                   0                    success indicator                   |
//+----------------------------------------------------------------------------+
//|Function flow:     check for corruption and open in correct mode            |
//|                   if okay                                                  |
//|                     either check passed key or assign new key              |
//|                                    and update the header                   |
//|                   endif                                                    |
//|                   if okay                                                  |
//|                     call QDAMDictInsertLocal                               |
//|                   endif                                                    |
//|                   return success indicator..                               |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMInsert
(
  PVOID   pBTIda,           // pointer to binary tree struct
  PULONG  pulKey,           // pointer to key
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
)
{
   SHORT         sRc = 0;   // return code
   PBTREEGLOB    pBT = NULL;

   DEBUGEVENT( EQFNTMINSERT_LOC, FUNCENTRY_EVENT, 0 );

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   if ( !pBTIda )
   {
     sRc = BTREE_INVALID;
   }
   else
   {
     pBT = ((PBTREE)pBTIda)->pBTree;
   } /* endif */

  /********************************************************************/
  /* do initial security checking...                                  */
  /********************************************************************/
   if ( !sRc && pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */
   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */
  /********************************************************************/
  /* if user wants that we find an appropriate key, we have to do so..*/
  /********************************************************************/
  if ( !sRc )
  {
    if ( *pulKey == NTMREQUESTNEWKEY )
    {
      /******************************************************************/
      /* find next free key and anchor new value in file ...            */
      /******************************************************************/
      ULONG  ulKey;
      PNTMVITALINFO pvi = (PNTMVITALINFO)(&pBT->chCollate[0]);
      //ulKey = *pulKey = ++(pvi->ulNextKey);
      ulKey = *pulKey = (pvi->ulNextKey)++;
      if ( ulKey > 0xFFFFFF )
      {
        sRc = BTREE_NUMBER_RANGE;
      }
      else
      {
        /**************************************************************/
        /* force update of header (only from time to time to avoid    */
        /* too much performance degration)...                         */
        /**************************************************************/
        if ( (ulKey & 0x020) || (pBT->usOpenFlags & ASD_SHARED) )
        {
          sRc = QDAMWriteHeader( (PBTREE)pBTIda );
          pBT->fUpdated = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* check if key is in valid range ...                           */
      /****************************************************************/
      if ( *pulKey > NTMSTARTKEY( pBT ) )
      {
        sRc = BTREE_NUMBER_RANGE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* call QDAMDictInsert to do the dirty work of inserting entry...   */
  /********************************************************************/
  if ( !sRc )
  {
    SHORT  RetryCount;                  // retry counter for in-use condition
    RetryCount = MAX_RETRY_COUNT;
    do
    {
      sRc = QDAMDictInsertLocal( (PBTREE) pBTIda, (PTMWCHAR) pulKey, pData, ulLen );
      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMINSERT_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */

  return sRc;
} /* end of function EQFNTMInsert */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMUpdateList - Update In core List
//------------------------------------------------------------------------------
// Function call:     QDAMUpdateList( PBTREE, PBTREERECORD )
//
//------------------------------------------------------------------------------
// Description:       update the list of filled records in cache
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree
//                    PBTREERECORD       The buffer to be updated
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     set record pointer to DataRecList
//                    while !fFound and i < MAX_LIST
//                      if record found then
//                        set fFound
//                        set offset to new filled mark
//                      else
//                        increase index i and point to next record
//                      endif
//                    endwhile
//                    if not found yet then
//                      check for a free slot where we can insert our record
//                      - reset record pointer to start (DataRecList )
//                      - while !fFound and i<MAX_LIST
//                          if record number != 0  then
//                            increase index and record pointer
//                          else
//                            set fFound
//                            fill in the new data
//                          endif
//                        endwhile
//                    endif
//                    if not found yet then
//                      check for the slot which is filled most
//                      - reset record pointer to start
//                      - loop thru list of stored records and find record
//                        which is filled most
//                      - set pRecTemp to the selected slot
//                      - store new values for offset and record number
//                    endif
//                    return sRc
//------------------------------------------------------------------------------
VOID  QDAMUpdateList_V3
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V3 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;
  USHORT   usFilled;                      // length of slot filled
  USHORT   usMaxRec = 0;                  // record filled at most
  PRECPARAM  pRecTemp;                    // record descr. structure
  PBTREEGLOB  pBT = pBTIda->pBTree;

  usNumber = pRecord->contents.header.usNum;

  pRecTemp = pBT->DataRecList;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data will fit
     {
        fFound = TRUE;
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
     } /* endif */
  } /* endwhile */

  //  if not found yet check for a free slot now where we can insert our record
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     i = 0;
     while ( !fFound && i < MAX_LIST )
     {
        if ( pRecTemp->usNum != 0 )
        {
           pRecTemp++;
           i++;
        }
        else
        {
           fFound = TRUE;
           // fill in the new data
           pRecTemp->usOffset = pRecord->contents.header.usFilled;
           pRecTemp->usNum = pRecord->contents.header.usNum;
        } /* endif */
     } /* endwhile */
  } /* endif */

  //  if not found yet check for the slot which is filled most
  if ( !fFound )
  {
     pRecTemp = pBT->DataRecList;
     usFilled = 0;
     for ( i=0 ; i < MAX_LIST ;i++ )
     {
        if ( pRecTemp->usOffset > usFilled)
        {
           usMaxRec = i;
           usFilled = pRecTemp->usOffset;
        } /* endif */
        pRecTemp++;                    // next record in list
     } /* endfor */
     // set pRecTemp to the selected slot
     pRecTemp = pBT->DataRecList + usMaxRec;

     if ( usFilled > pRecord->contents.header.usFilled )
     {
        pRecTemp->usOffset = pRecord->contents.header.usFilled;
        pRecTemp->usNum = pRecord->contents.header.usNum;
     } /* endif */
  } /* endif */

  return;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFreeFromList    Free Record from List in cache
//------------------------------------------------------------------------------
// Function call:     QDAMFreeFromList( PRECPARAM, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       delete the record from the list maintained in cache
//
//------------------------------------------------------------------------------
// Parameters:        PRECPARAM          pointer to record parameter
//                    PBTREERECORD       The buffer to be updated
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     while !fFound and i < MAX_LIST
//                      if record found
//                        set fFound
//                        reset offset and number in record structure
//                      else
//                        increase index i and point to next record
//                      endif
//                    endwhile
//------------------------------------------------------------------------------
VOID  QDAMFreeFromList_V3
(
   PRECPARAM  pRecTemp,
   PBTREEBUFFER_V3 pRecord
)
{
  USHORT   i;                             // index
  USHORT   usNumber;                      // number of record
  BOOL     fFound = FALSE;

  usNumber = pRecord->contents.header.usNum;

  i = 0;
  //  find slot in list
  while ( !fFound && i < MAX_LIST )
  {
     if ( pRecTemp->usNum != usNumber )
     {
        pRecTemp++;
        i++;
     }
     else                                     // data record found
     {
        fFound = TRUE;
        pRecTemp->usNum = 0;
        pRecTemp->usOffset = 0;
     } /* endif */
  } /* endwhile */

  return ;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFreeRecord     Add record to free list
//------------------------------------------------------------------------------
// Function call:     QDAMFreeRecord( PBTREE, PBTREERECORD, RECTYPE );
//
//------------------------------------------------------------------------------
// Description:       add the requested record to the free list
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             B-tree
//                    PBTREERECORD       record buffer
//                    RECTYPE            data or key record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//------------------------------------------------------------------------------
// Function flow:     if pRecord is valid
//                      reinit contents ot pRecord
//                      if it is a data record
//                        add deleted record to the free list of data recs
//                        take it from the intermal data list if avail.
//                      else
//                        add deleted record to the free list of key recs
//                      endif
//                      init the header of the free record
//                      write the record (QDAMWriteRecord)
//                    else
//                      set return code = BTREE_INVALID
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFreeRecord_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pRecord,
   RECTYPE      recType                // data or key record
)
{
  SHORT  sRc;                          // return code
  PBTREEHEADER  pHeader;               // pointer to header
  USHORT        usRecNum;              // record number
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pRecord )
  {

     memset( pRecord->contents.uchData, 0, sizeof(pRecord->contents.uchData) );
     if ( recType == DATAREC)           // it is a data record
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeDataBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeDataBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeDataBuffer;
       // get it from the internal data list if available
       QDAMFreeFromList_V3(pBT->DataRecList ,pRecord);
     }
     else
     {
       // add deleted record to the free list
       NEXT( pRecord ) = pBT->usFreeKeyBuffer;
       PREV( pRecord ) = 0;
       pBT->usFreeKeyBuffer = RECORDNUM( pRecord );
       usRecNum = pBT->usFreeKeyBuffer;

     } /* endif */
     // init this record
     pHeader = &pRecord->contents.header;
     pHeader->usNum = usRecNum;
     pHeader->usOccupied = 0;
     pHeader->usWasteSize = 0;
     pHeader->usFilled = sizeof(BTREEHEADER );
     pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
     sRc = QDAMWriteRecord_V3( pBTIda, pRecord);
  }
  else
  {
    sRc = BTREE_INVALID;
  }  /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFREERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecDataLen  get data length
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecDataLen( PBTREEBUFFER, SHORT )
//
//------------------------------------------------------------------------------
// Description:       Get data length record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   ULONG
//------------------------------------------------------------------------------
// Returncodes:       length of data
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to data description
//                    return length indication
//
//------------------------------------------------------------------------------
ULONG QDAMGetrecDataLen_V3
(
   PBTREE   pBTIda,
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid                            // key number
)
{
   PCHAR   pData = NULL;
   PUSHORT  pusOffset;
   PBTREEGLOB  pBT = pBTIda->pBTree;
   ULONG    ulLength;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                                      // point to key
   /*******************************************************************/
   /* pusOffset should only be in the allowed range                   */
   /*******************************************************************/
   pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   
   ulLength = *(PULONG)pData;        // get length
   ulLength &= ~QDAM_TERSE_FLAGL;    // get rid off any terse flag
   
   return( ulLength );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCopyDataTo   Copy Data into new record
//------------------------------------------------------------------------------
// Function call:     QDAMCopyDataTo( PBTREEBUFFER, SHORT, PBTREEBUFFER,SHORT);
//
//------------------------------------------------------------------------------
// Description:       Copy data to new record for reorganizing
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    SHORT                  data to be used
//                    PBTREEBUFFER           record pointer to copy into
//                    SHORT                  data to be used
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     copy the data from the passed position to the new one
//                    return
//
//------------------------------------------------------------------------------
VOID QDAMCopyDataTo_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         i,
   PBTREEBUFFER_V3  pNew,
   SHORT         j,
   USHORT        usVersion             // version of BTREE
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data
   USHORT   usLenFieldSize;            // size of data length field

   if ( usVersion < NTM_VERSION2 )
   {
     LogMessage3(FATAL,__func__, ":: btree version is not supported, version = ", toStr(usVersion).c_str());
     BTREE_NOT_SUPPORTED;
   }
   
   usLenFieldSize = sizeof(ULONG);

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
       if(usLastPos < usLen && CheckLogLevel(DEBUG)){
        LogMessage4(ERROR, "QDAMCopyDataTo_V3::DEBUG 1 Assetrion fails : usLastPos >= usLen, usLastPos = ", toStr(usLastPos).c_str(), ", usLen = ", toStr(usLen).c_str() );
      }
      ULONG ulLen = *(PULONG) pOldData;
      if ( ulLen & QDAM_TERSE_FLAGL)
      {
        ulLen &= ~QDAM_TERSE_FLAGL;
      } /* endif */
      
      usLen = (USHORT)ulLen;

      usLen = usLen + usLenFieldSize;       // add size of length indication

      if(usLastPos < usLen && CheckLogLevel(DEBUG)){
        LogMessage4(FATAL, "QDAMCopyDataTo_V3::DEBUG 2 Assetrion fails : usLastPos >= usLen, usLastPos = ", toStr(usLastPos).c_str(), ", usLen = ", toStr(usLen).c_str() );
      }
      //assert( (usLastPos >= usLen) );

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDeleteDataFromBuffer    Delete Data From Buffer
//------------------------------------------------------------------------------
// Function call:     QDAMDeleteDataFromBuffer( PBTREE, RECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Delete stored data (either key or data)
//
//                    Data are stored the following way
//                     USHORT      length of data
//                     USHORT      record number where associated key is stored
//                     USHORT      offset where key starts
//                     PCHAR       data following here in length of usDataLen
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    RECPARAM               position/offset of the key string
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Side effects:       if size of holes > MAXWASTESIZE reallocate the data
//
//------------------------------------------------------------------------------
// Function flow:     get record number to read
//                    read in requested record
//                    if okay then
//                      lock record, get offset and data length
//                      if data length > record size then
//                        get next record number
//                        as long as record number set and rc okay do
//                          read in record, set rc
//                        endwhile
//                      endif
//                      set waste size to record length
//                      mark it as deleted
//                      prepare new record
//                    endif
//                    if okay so far then
//                      write new rearranged record
//                    endif
//                    unlock record
//                    return return code
//------------------------------------------------------------------------------
SHORT QDAMDeleteDataFromBuffer_V3
(
  PBTREE  pBTIda,
  RECPARAM recParam
)
{
   SHORT sRc = 0;
   PUSHORT  pusOffset;                           // offset pointer
   SHORT   i;                                    // index
   PBTREEBUFFER_V3  pRecord;
   PBTREEBUFFER_V3  pNew;                           // temporary record
   PBTREEHEADER  pHeader;                        // pointer to header
   ULONG         ulDataLen;                      // length of data
   USHORT        usNum;                          // number of record
   PBTREEGLOB    pBT = pBTIda->pBTree;

   // read in record
   recParam.usNum = recParam.usNum + pBT->usFirstDataBuffer;      // adjust for offset
   sRc = QDAMReadRecord_V3( pBTIda, recParam.usNum, &pRecord, FALSE  );
   if ( ! sRc )
   {
      BTREELOCKRECORD( pRecord );              // lock eecord.
      // set offset in index table to 0;
      pusOffset = (PUSHORT) pRecord->contents.uchData;

      ulDataLen = QDAMGetrecDataLen_V3( pBTIda, pRecord, recParam.usOffset );

      /****************************************************************/
      /* if more than 4k than free the next records                   */
      /* They have the following characteristic:                      */
      /*    they all contain nothing else than this record            */
      /*    they all are of type DATA_NEXTNODE                        */
      /*    they all are chained                                      */
      /****************************************************************/
      if ( ulDataLen >= BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER) )
      {
        usNum = NEXT( pRecord );
        while ( !sRc && usNum )
        {
          sRc = QDAMReadRecord_V3( pBTIda, usNum, &pNew, FALSE  );
          if ( !sRc )
          {
            if ( TYPE(pNew) != DATA_NEXTNODE )
            {
              sRc = BTREE_CORRUPTED;
              ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, STATE_EVENT, sRc, DB_GROUP, "" );
            }
            else
            {
              usNum = NEXT( pNew );
              sRc = QDAMFreeRecord_V3( pBTIda, pNew, DATAREC );
              ulDataLen -= (BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER) );
            } /* endif */
          } /* endif */
        } /* endwhile */
        /**************************************************************/
        /* remove the chaining information - they are not chained any */
        /* more                                                       */
        /**************************************************************/
        NEXT( pRecord ) = 0;

      } /* endif */
      pRecord->contents.header.usWasteSize = pRecord->contents.header.usWasteSize + (USHORT)
                           QDAMGetrecDataLen_V3( pBTIda, pRecord, recParam.usOffset );

      *(pusOffset + recParam.usOffset ) = 0;

      /******************************************************************/
      /* if nothing left in record free it - else adjust size ...KAT009 */
      /******************************************************************/
      if ( OCCUPIED(pRecord) == 1 )
      {
        sRc = QDAMFreeRecord_V3( pBTIda, pRecord, DATAREC );
      }
      else
      {
        /**************************************************************/
        /* do garbage collection only if not second part of a record..*/
        /* Optimal solution would be:                                 */
        /*  - do a garbage collection in any case                     */
        /*    but you have to adjust the length of the stored data    */
        /*    to take care of parts stored in other records ...       */
        /**************************************************************/
        if ( !NEXT(pRecord) && (pRecord->contents.header.usWasteSize > MAXWASTESIZE) )
        {
           // get temp record
           pNew = &pBT->BTreeTempBuffer_V3;
           pHeader = &pNew->contents.header;
           pHeader->usOccupied = 0;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

           // for all data values move them into new record
           i = 0;
           while ( i < (SHORT) OCCUPIED( pRecord ))
           {
              QDAMCopyDataTo_V3( pRecord, i, pNew, i, pBT->usVersion );
              i++;
           } /* endwhile */
           // copy record data back to avoid misplacement in file
           memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V3 );
           pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
           pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;
           pRecord->contents.header.usWasteSize = 0;       // no space wasted
           QDAMUpdateList_V3( pBTIda, pRecord );
        } /* endif */

        if ( !sRc )
        {
//         pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
        } /* endif */
      } /* endif */

      BTREEUNLOCKRECORD( pRecord );              // unlock previous record.
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDELETEDATAFROMBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

   return ( sRc );

}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMSetrecData    Set Data Record Pointer
//------------------------------------------------------------------------------
// Function call:     QDAMSetrecData( PBTREEBUFFER, SHORT, RECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Set data record pointer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//                    RECPARAM               record pointer
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to offset
//                    copy passed record structure at this place
//                    return
//------------------------------------------------------------------------------
VOID QDAMSetrecData_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid,                           // key number
   RECPARAM      recData,                        // data pointer
   USHORT        usVersion                       // version of database
)
{
   PCHAR   pData = NULL;
   PCHAR   pEndOfRec;
   PUSHORT  pusOffset;

   // get max pointer value
   pEndOfRec = (PCHAR)&(pRecord->contents) + BTREE_REC_SIZE_V3;


   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   if ( (PCHAR)pusOffset > pEndOfRec )
   {
     // offset pointer is out of range
     pData = NULL;
     ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, "" );
   }
   else
   {
     pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
     pData += sizeof(USHORT );                    // get pointer to datarec

     if ( pData > pEndOfRec )
     {
       // data pointer is out of range
       pData = NULL;
       LogMessage2(FATAL,__func__,":: pData > 16kBt, it's out of range of record");
       ERREVENT2( QDAMSETRECDATA_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, "" );
     }
     else
     {      
       memcpy( (PRECPARAM) pData, &recData, sizeof(RECPARAM ) );       
     } /* endif */

     // re-compute record checksum
//   pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
   } /* endif */

   return ;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecKey     Get key record pointer
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecKey( PBTREEBUFFER, SHORT );
//
//------------------------------------------------------------------------------
// Description:       Get data for key record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   RECPARAM
//------------------------------------------------------------------------------
// Returncodes:       return filled data record
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to key  description
//                    copy key     description structure for return
//
//
//------------------------------------------------------------------------------
RECPARAM  QDAMGetrecKey_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid                            // key number
)
{
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key

   recData.usNum  = pRecord->usRecordNumber;
   recData.usOffset  = *pusOffset;
   recData.ulLen = *(PUSHORT) (pRecord->contents.uchData +*pusOffset);
   return ( recData );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdateLocal   Update Entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdateLocal( PBTREE, PCHAR, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Update existing data
//
//                    Data are stored the following way:
//                     USHORT    length of data
//                     USHORT    record number where associated key is stored
//                     USHORT    offset where key starts
//                     PCHAR     data following here in length of usDataLen
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key string
//                    PCHAR                  user data
//                    USHORT                 user data length
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   data not found
//
//------------------------------------------------------------------------------
// Function flow:     Locate the Leaf node that contains the appropriate
//                      key
//                    find the key
//                    if found then
//                      get data and key value associated with key
//                      set new data value
//                      mark old value as deleted
//                    endif
//                    return sRc
//
//------------------------------------------------------------------------------
SHORT QDAMDictUpdateLocal
(
  PBTREE  pBTIda,                        //
  PCHAR_W pKey,                          //  key string
  PBYTE   pUserData,                     //  user data
  ULONG   ulLen                          //  user data length
)
{
   SHORT sRc = 0;                        // return code
   SHORT   i ;
   SHORT    sNearKey;                   // nearest key found
   RECPARAM      recData;               // pointer to data value
   RECPARAM      recOldData;            // pointer to old data value
   RECPARAM      recOldKey;             // pointer to old key value
   BOOL          fLocked = FALSE;       // file-has-been-locked flag
   PBTREEGLOB    pBT = NULL;            // pointer to btree

   DEBUGEVENT2( QDAMDICTUPDATELOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, "" );

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   CHECKPBTREE( pBTIda, sRc );

   if ( !sRc )
   {
     pBT = pBTIda->pBTree;
   } /* endif */

   if ( !sRc && pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */

   /*******************************************************************/
   /* check if entry is locked ....                                   */
   /*******************************************************************/
   if ( !sRc && QDAMDictLockStatus( pBTIda, pKey ) )
   {
     sRc = BTREE_ENTRY_LOCKED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: lock complete file                        */
   /*                                                                 */
   /* Note: this will also update our internal buffers and the        */
   /*       header record. No need to call QDamCheckForUpdates here.  */
   /*******************************************************************/
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     sRc = QDAMPhysLock( pBTIda, TRUE, &fLocked );
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord = NULL;        // pointer to record
      if ( !sRc )
      {
        if ( ulLen == 0 )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          if ( pBT->fTransMem )
          {
            memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG));   // save data
          }
          else
          {
            UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
          } /* endif */
          QDAMDictUpdStatus( pBTIda );
          sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
        } /* endif */
      } /* endif */

      // Locate the Leaf node that contains the appropriate key
      if ( !sRc )
      {
        //  find the key
        sRc = QDAMLocateKey_V3( pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey ) ;
        if ( !sRc )
        {
            if ( i != -1)
            {
              BTREELOCKRECORD( pRecord );
              // set new current position
              pBTIda->sCurrentIndex = i;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
              // get data value associated with key
              recOldKey = QDAMGetrecKey_V3( pRecord,i );
              recOldData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
              //  set new data value
              if ( recOldKey.usNum && recOldData.usNum )
              {
                  sRc = QDAMAddToBuffer_V3( pBTIda, pUserData, ulLen, &recData );
                  if ( !sRc )
                  {
                    if(ulLen > TMX_REC_SIZE && CheckLogLevel(INFO)){
                      LogMessage3(ERROR, __func__, ":: tried to set bigget ulLen than rec size, ulLen = ", toStr(ulLen).c_str());
                    }
                    recData.ulLen = ulLen;
                    QDAMSetrecData_V3( pRecord, i, recData, pBT->usVersion );
                    sRc = QDAMWriteRecord_V3( pBTIda, pRecord );
                  } /* endif */
                /****************************************************************/
                /* change time to indicate modifications on dictionary...       */
                /****************************************************************/
                pBT->lTime ++;
              }
              else
              {
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMDICTUPDATELOCAL_LOC, STATE_EVENT, 1, DB_GROUP, "" );
              } /* endif */

              //  mark old value as deleted
              if ( !sRc )
              {
                  sRc = QDAMDeleteDataFromBuffer_V3( pBTIda, recOldData );
              } /* endif */
              BTREEUNLOCKRECORD( pRecord );
            }
            else
            {
              sRc = BTREE_NOT_FOUND;
              // set new current position
              pBTIda->sCurrentIndex = sNearKey;
              pBTIda->usCurrentRecord = RECORDNUM( pRecord );
            } /* endif */
        } /* endif */
    } /* endif */
   }
   else
   {
     LogMessage2(FATAL, __func__, "::TEMPORARY_COMMENTED::QDAMDictUpdateLocal:: BTREE_V2 is not supported ");
     throw;
     sRc = BTREE_NOT_SUPPORTED;
   } /* endif */

   /*******************************************************************/
   /* For shared databases: unlock complete file                      */
   /*                                                                 */
   /* Note: this will also incement the dictionary update counter     */
   /*       if the dictionary has been modified                       */
   /*******************************************************************/
   if ( fLocked )
   {
     SHORT sRc1 = QDAMPhysLock( pBTIda, FALSE, NULL );
     sRc = (sRc) ? sRc : sRc1;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTUPDATELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

   return sRc;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMUpdate                                             |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMUpdate( pBTIda,  ulKey, pData, usLen );      |
//+----------------------------------------------------------------------------+
//|Description:       update the data of an already inserted key               |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE  pBTIda,      pointer to binary tree struct       |
//|                   ULONG   ulKey,      key value                            |
//|                   PBYTE   pData,       pointer to user data                |
//|                   USHORT  usLen        length of user data                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range  |
//|                   BTREE_READONLY       file is opened read only - no write |
//|                   BTREE_CORRUPTED      file is corrupted                   |
//|                   errors returned by QDAMDictInsertLocal                   |
//|                   0                    success indicator                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictUpdateLocal                                 |
//|                   return success indicator..                               |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMUpdate
(
  PVOID   pBTIda,           // pointer to binary tree struct
  ULONG   ulKey,            // key value
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
)
{
  SHORT  sRc = 0;           // success indicator

  DEBUGEVENT( EQFNTMUPDATE_LOC, FUNCENTRY_EVENT, 0 );

  if ( pBTIda )
  {
    SHORT  RetryCount;                  // retry counter for in-use condition
    RetryCount = MAX_RETRY_COUNT;
    do
    {
      sRc = QDAMDictUpdateLocal( (PBTREE)pBTIda, (PSZ_W) &ulKey, pData, ulLen );
      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMUPDATE_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */


  return sRc;
} /* end of function EQFNTMUpdate */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictExactLocal   Find Exact match
//------------------------------------------------------------------------------
// Function call:     QDAMDictExactLocal( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:        Find an exact match for the passed key
//
//------------------------------------------------------------------------------
// Parameters:         PBTREE               pointer to btree structure
//                     PCHAR                key to be inserted
//                     PCHAR                buffer for user data
//                     PUSHORT              on input length of buffer
//                                          on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted, then
//                      set Rc = BTREE_CORRUPTED
//                    else
//                      locate the leaf node that contains appropriate key
//                      set Rc correspondingly
//                    endif
//                    if okay so far then
//                      locate the key with option set to FEXACT;  set Rc
//                      if key found then
//                        set current position to the found key
//                        pass back either length only or already data,
//                         depending on user request.
//                        endif
//                      else
//                        set Rc to BTREE_NOT_FOUND
//                        set current position to the nearest key
//                      endif
//                    endif
//                    return Rc
//
//
//------------------------------------------------------------------------------

SHORT QDAMDictExactLocal
(
   PBTREE pBTIda,                      // pointer to btree struct
   PTMWCHAR pKey,                       // key to be searched for
   PBYTE  pchBuffer,                   // space for user data
   PULONG pulLength,                   // in/out length of returned user data
   USHORT usSearchSubType              // special hyphenation lookup flag
)
{
  SHORT    i;
  SHORT    sNearKey;                   // nearest key position
  SHORT    sRc  = 0;                   // return code
  RECPARAM     recData;                // point to data structure
  PBTREEGLOB    pBT = pBTIda->pBTree;

  if ( pBT->fCorrupted )
  {
    LogMessage2(FATAL, __func__,":: BTREE_CORRUPTED");
     sRc = BTREE_CORRUPTED;
  }else if(pBT->bRecSizeVersion != BTREE_V3){
    LogMessage2(FATAL, __func__,":: not supported btree version");
    sRc = BTREE_NOT_SUPPORTED;

  } else
  {    
       PBTREEBUFFER_V3 pRecord = NULL;
       SHORT sRetries = MAX_RETRY_COUNT;
       do
       {
          /*******************************************************************/
          /* For shared databases: discard all in-memory pages if database   */
          /* has been changed since last access                              */
          /*******************************************************************/
         if ( (!sRc || (sRc == BTREE_IN_USE)) && (pBT->usOpenFlags & ASD_SHARED) )
         {
            sRc = QDAMCheckForUpdates( pBTIda );
          } /* endif */

          if ( !sRc )
          {
             /* Locate the Leaf node that contains the appropriate key */
             sRc = QDAMFindRecord_V3( pBTIda, pKey, &pRecord );
          } /* endif */

          if ( !sRc )
          {
            sRc = QDAMLocateKey_V3(pBTIda, pRecord, pKey, &i, FEXACT, &sNearKey);
            if ( !sRc )
            {
               if ( i != -1 )
               {
                  // set new current position
                 pBTIda->sCurrentIndex = i;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );

                 if ( pBT->fTransMem )
                 {
                   memcpy( pBTIda->chHeadTerm, pKey, sizeof(ULONG) );  // save data
                 }
                 else
                 {
                   UTF16strcpy( pBTIda->chHeadTerm, pKey );          // save current data
                 } /* endif */
                 recData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
                 if ( *pulLength == 0 || ! pchBuffer )
                 {
                    *pulLength = recData.ulLen;
                 }
                 else if ( *pulLength < recData.ulLen )
                 {
                    if(CheckLogLevel(DEBUG)){
                      LogMessage5(ERROR, __func__, "::BTREE_BUFFER_SMALL, pulLength = ", toStr(*pulLength).c_str(), "; recData.ulLen = ", toStr(recData.ulLen).c_str());
                    }
                    *pulLength = recData.ulLen;
                    sRc = BTREE_BUFFER_SMALL;
                 }
                 else
                 {
                    sRc = QDAMGetszData_V3( pBTIda, recData, pchBuffer, pulLength, DATA_NODE );
                 } /* endif */
               }
               else
               {
                 sRc = BTREE_NOT_FOUND;
                  // set new current position
                 pBTIda->sCurrentIndex = sNearKey;
                 pBTIda->usCurrentRecord = RECORDNUM( pRecord );
                 *pulLength = 0;            // init returned length
               } /* endif */
            } /* endif */
          } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) && (sRetries > 0));    
   } /* endif */


  if ( sRc && (sRc != BTREE_NOT_FOUND))
  {
    ERREVENT2( QDAMDICTEXACTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return ( sRc );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMGet                                                |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMGet( pBTIda, ulKey, chData, &usLen );        |
//+----------------------------------------------------------------------------+
//|Description:       get the data string for the passed key                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE pBTIda,       pointer to btree struct             |
//|                   ULONG  ulKey,        key to be searched for              |
//|                   PCHAR  pchBuffer,    space for user data                 |
//|                   PUSHORT pusLength    in/out length of returned user data |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       same as for QDAMDictExactLocal...                        |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictExactLocal to retrieve the data             |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMGet
(
   PVOID  pBTIda,                      // pointer to btree struct
   ULONG  ulKey,                       // key to be searched for
   PCHAR  pchBuffer,                   // space for user data
   PULONG pulLength                    // in/out length of returned user data
)
{
  SHORT sRc;                           // return code
  DEBUGEVENT( EQFNTMGET_LOC, FUNCENTRY_EVENT, 0 );

  CHECKPBTREE( ((PBTREE)pBTIda), sRc );
  if ( !sRc )
  {
    SHORT  RetryCount = MAX_RETRY_COUNT;                  // retry counter for in-use condition
    ULONG ulLength;
    do
    {
      /******************************************************************/
      /* disable corruption flag to allow get of data in case memory    */
      /* is corrupted                                                   */
      /******************************************************************/
      PBTREEGLOB    pBT = ((PBTREE)pBTIda)->pBTree;
      BOOL          fCorrupted = pBT->fCorrupted;

      ulLength = *pulLength;
      pBT->fCorrupted = FALSE;

      sRc = QDAMDictExactLocal( (PBTREE) pBTIda,(PTMWCHAR) &ulKey, (PBYTE)pchBuffer, &ulLength, FEXACT );

      pBT->fCorrupted = fCorrupted;

      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
    *pulLength = ulLength;
  } /* endif */

  if ( sRc && (sRc != BTREE_NOT_FOUND) )
  {
    ERREVENT( EQFNTMGET_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */

  return sRc;
} /* end of function EQFNTMGet */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMGetMaxNumber                                       |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMGetNextNumber( pBTIda, &ulKey, &ulNextFree );|
//+----------------------------------------------------------------------------+
//|Description:       get the start key and the next free key ...              |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE pBTIda,       pointer to btree struct             |
//|                   PULONG pulStartKey   first key                           |
//|                   PULONG pulNextKey    next key to be assigned             |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0     always                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     access data from internal structure                      |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMGetNextNumber
(
   PVOID  pBTIda,                      // pointer to btree struct
   PULONG pulStartKey,                 // return start key number
   PULONG pulNextKey                   // return next key data
)
{
  NTMVITALINFO ntmVitalInfo;

  memcpy( &ntmVitalInfo, ((PBTREE)pBTIda)->pBTree->chCollate,
          sizeof(NTMVITALINFO));
  *pulStartKey = ntmVitalInfo.ulStartKey;
  *pulNextKey  = ntmVitalInfo.ulNextKey;

  return 0;
} /* end of function EQFNTMGetNextNumber */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMPhysLock                                           |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMPhysLock( pBTIda );                          |
//+----------------------------------------------------------------------------+
//|Description:       Physicall lock or unlock database.                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE             The database to be locked             |
//|                   BOOL               TRUE = LOCK, FALSE = Unlock           |
//|                   PBOOL              ptr to locked flag (set to TRUE if    |
//|                                      locking was successful                |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
// ----------------------------------------------------------------------------+
SHORT EQFNTMPhysLock
(
   PVOID          pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
)
{
  SHORT       sRc = 0;                 // function return code

  DEBUGEVENT( EQFNTMPHYSLOCK_LOC, FUNCENTRY_EVENT, 0 );

  if ( pBTIda )
  {
    sRc = QDAMPhysLock( (PBTREE)pBTIda, fLock, pfLocked );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMPHYSLOCK_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */


  return sRc;
} /* end of function EQFNTMPhysLock */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMIncrUpdCounter      Inrement database update counter |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       Update one of the update counter field in the dummy      |
//|                   /locked terms file                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   SHORT                  index of counter field            |
//|                                                                      PLONG                                                                  ptr to buffer for new counte value|
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     read update counter from dummy file                      |
//|                   increment update counter                                 |
//|                   position ptr to begin of file                            |
//|                   write update counter to disk                             |
// ----------------------------------------------------------------------------+
SHORT EQFNTMIncrUpdCounter
(
   PVOID      pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of update counter
   PLONG                         plNewValue                                               // ptr to buffer for new counte value|
)
{
  SHORT       sRc = 0;                 // function return code

  if ( pBTIda )
  {
    sRc = QDAMIncrUpdCounter( (PBTREE)pBTIda, sIndex, plNewValue );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  return sRc;
}

