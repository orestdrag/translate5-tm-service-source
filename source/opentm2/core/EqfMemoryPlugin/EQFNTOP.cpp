//+----------------------------------------------------------------------------+
//|EQFNTOP.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>
#include <EQFEVENT.H>             // event logging
#include "LogWrapper.h"


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXOpen     opens   data and index files                |
//+----------------------------------------------------------------------------+
//|Description:       Opens   qdam data and index files                        |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXOpen  ( PTMX_OPEN_IN pTmOpenIn, //input struct          |
//|                            PTMX_OPEN_OUT pTmOpenOut ) //output struct      |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_OPEN_IN pTmOpenIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_OPEN_OUT pTmOpenOut   output structure               |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in open out structure                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| allocate control block     - NTM_CLB                                       |
//| allocate tag table, file name, author and compact area blocks in control   |
//|  block                                                                     |
//| open tm data file                                                          |
//| get signature record contents and store in control block                   |
//| get tag table, author, file name and compact area record from tm data file |
//|  and add to control block                                                  |
//| open index file                                                            |
//|                                                                            |
//| return open out structure                                                  |
// ----------------------------------------------------------------------------+

USHORT TmtXOpen
(
  PTMX_OPEN_IN pTmOpenIn,    //ptr to input struct
  PTMX_OPEN_OUT pTmOpenOut   //ptr to output struct
)
{
  BOOL fOK;                      //success indicator
  PTMX_CLB pTmClb = NULL;        //pointer to control block
  USHORT usRc = NO_ERROR;        //return value
  USHORT usRc1 = NO_ERROR;       //return value
  ULONG  ulLen;                  //length indicator

  DEBUGEVENT( TMTXOPEN_LOC, FUNCENTRY_EVENT, 0 );

  //allocate control block
  fOK = UtlAlloc( (PVOID *) &(pTmClb), 0L, (LONG)sizeof( TMX_CLB ), NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    //allocate table records
    //check access
    if ( pTmOpenIn->stTmOpen.usAccess == READONLYACCESS )
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      pTmClb->usAccessMode = ASD_GUARDED | ASD_READONLY;
    }
    else if ( pTmOpenIn->stTmOpen.usAccess != NONEXCLUSIVE )
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      pTmClb->usAccessMode = ASD_GUARDED | ASD_LOCKED;
    }
    else
    {
      PSZ pszExt = strrchr( pTmOpenIn->stTmOpen.szDataName, DOT );
      pTmClb->usAccessMode = ASD_GUARDED;      
    } /* endif */
    if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
    {
        pTmClb->usAccessMode |= ASD_ORGANIZE;
    } /* endif */
    pTmClb->TmBtree.fb.fileName = pTmOpenIn->stTmOpen.szDataName;
    pTmClb->InBtree.fb.fileName = pTmOpenIn->stTmOpen.szIndexName;
    //call open function for data file
    usRc1 = pTmClb->TmBtree.EQFNTMOpen((USHORT)(pTmClb->usAccessMode | ASD_FORCE_WRITE) );
    if ( (usRc1 == NO_ERROR) || (usRc1 == BTREE_CORRUPTED) )
    {
      //get signature record and add to control block
      USHORT usLen = sizeof( TMX_SIGN );

      usRc = pTmClb->TmBtree.EQFNTMSign((PCHAR) &(pTmClb->stTmSign), &usLen );

      if ( usRc == NO_ERROR )
      {
        if ( pTmClb->stTmSign.bGlobVersion > T5GLOBVERSION  || pTmClb->stTmSign.bMajorVersion > T5MAJVERSION )
        {
          T5LOG(T5ERROR) << "TM was created in newer vertions of t5memory, v0."<<pTmClb->stTmSign.bMajorVersion<<"."<<pTmClb->stTmSign.bMinorVersion;
          usRc = ERROR_VERSION_NOT_SUPPORTED;
        }
        else if (pTmClb->stTmSign.bGlobVersion < T5GLOBVERSION  || pTmClb->stTmSign.bMajorVersion < T5MAJVERSION )
        {
          T5LOG(T5ERROR) << "TM was created in older vertions of t5memory, v"<<pTmClb->stTmSign.bGlobVersion<<"."<<pTmClb->stTmSign.bMajorVersion<<"."<<pTmClb->stTmSign.bMinorVersion;
          usRc = VERSION_MISMATCH;
        } /* endif */
      }
      else if ( pTmClb->usAccessMode & ASD_ORGANIZE )
      {
        // allow to continue even if signature record is corrupted
        usRc = NO_ERROR;
      } /* endif */

      if ( (usRc == NO_ERROR) || (usRc == VERSION_MISMATCH) )
      {
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {          
          ulLen =  MAX_COMPACT_SIZE-1;
          //get compact area and add to control block
          USHORT usTempRc =  pTmClb->TmBtree.EQFNTMGet( COMPACT_KEY, (PCHAR)pTmClb->bCompact, &ulLen );

          // in organize mode allow continue if compact area is corrupted
          if ( (usTempRc != NO_ERROR) && (usTempRc != VERSION_MISMATCH) )
          {
            usTempRc = BTREE_CORRUPTED;
          } /* endif */

          if ( usTempRc == BTREE_CORRUPTED )
          {
            memset( pTmClb->bCompact, 0, sizeof(pTmClb->bCompact) );
          } /* endif */
          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */


        //get languages and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          DEBUGEVENT( TMTXOPEN_LOC, STATE_EVENT, 2 );

          //call to obtain exact length of record
          ulLen = 0;
          USHORT usTempRc = NTMLoadNameTable( pTmClb, LANG_KEY,
                                       (PBYTE *)&pTmClb->Languages, &ulLen );

          if ( usTempRc == BTREE_READ_ERROR ) 
              usTempRc = BTREE_CORRUPTED;
          usTempRc = NTMCreateLangGroupTable( pTmClb );

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get file names and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( pTmClb, FILE_KEY,
                                       (PBYTE *)&pTmClb->FileNames,
                                       &ulLen );

          // in organize mode allow continue if file name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            pTmClb->FileNames.ulAllocSize = TMX_TABLE_SIZE;
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get authors and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( pTmClb, AUTHOR_KEY,
                                       (PBYTE *)&pTmClb->Authors, &ulLen );

          // in organize mode allow continue if author name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {          
            pTmClb->Authors.ulAllocSize = TMX_TABLE_SIZE;            
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get tag tables and add to control block
      if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( pTmClb, TAGTABLE_KEY,
                                       (PBYTE *)&pTmClb->TagTables, &ulLen );


          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            usTempRc = TM_FILE_SCREWED_UP; // cannot continue if no tag tables
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get long document name table
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          // error in memory allocations will force end of open!
          USHORT usTempRc = NTMCreateLongNameTable( pTmClb );
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;

          // now read any long name table from database, if there is
          // none (=older TM before TOP97) write our empty one to the
          // Translation Memory
          if ( usTempRc == NO_ERROR )
          {
            usTempRc = NTMReadLongNameTable( pTmClb );

            switch ( usTempRc)
            {
              case ERROR_NOT_ENOUGH_MEMORY:
                // leave return code as is
                break;
              case NO_ERROR:
                // O.K. no problems at all
                break;
              case BTREE_NOT_FOUND :
              T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 48 usTempRc = NTMWriteLongNameTable( pTmClb );";
#ifdef TEMPORARY_COMMENTED
                // no long name tabel yet,create one ...
                usTempRc = NTMWriteLongNameTable( pTmClb );
                #endif
                break;
              default:
                // read of long name table failed, assume TM is corrupted
                if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
                {
                } /* endif */
                usTempRc = BTREE_CORRUPTED;
                break;
            } /* endswitch */
          } /* endif */

          // just for debugging purposes: check if short and long name table have the same numbe rof entries
          if ( usTempRc == NO_ERROR )
          {
            if ( pTmClb->pLongNames->ulEntries != pTmClb->FileNames.ulMaxEntries )
            {
              // this is strange...
              int iSetBreakPointHere = 1;
            }
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */
      } /* endif */

      //add threshold to control block
      pTmClb->usThreshold = pTmOpenIn->stTmOpen.usThreshold;


      if ( (usRc == NO_ERROR) ||
           (usRc == BTREE_CORRUPTED) ||
           (usRc == VERSION_MISMATCH) )
      {
        //call open function for index file
        USHORT usIndexRc = pTmClb->InBtree.EQFNTMOpen(pTmClb->usAccessMode );
        if ( usIndexRc != NO_ERROR )
        {
          usRc = usIndexRc;
        } /* endif */
      } /* endif */

      if ( usRc == NO_ERROR)
      {
        // no error during open of index, use return code of EQFNTMOPEN for
        // data file
        usRc = usRc1;
      }
      else
      {
        // open of index failed or index is corrupted so leave usRc as-is;
        // data file will be closed in cleanup code below
      } /* endif */
    }
    else
    {
      // error during open of data file, use return code of EQFNTMOPEN for
      // data file
      usRc = usRc1;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Ensure that all required data areas have been loaded and the     */
  /* database files have been opened                                  */
  /********************************************************************/
  if ( (usRc == NO_ERROR) ||
       (usRc == BTREE_CORRUPTED) ||
       (usRc == VERSION_MISMATCH) )
  {
    if ( false //(pTmClb->pLanguages == NULL) ||
         //(pTmClb->pAuthors   == NULL) ||
         //(pTmClb->pTagTables == NULL) ||
         //(pTmClb->pFileNames == NULL) //||
         //(pTmClb.TmBtree == NULL) ||
         //(pTmClb.InBtree == NULL) 
         )
    {
      usRc = TM_FILE_SCREWED_UP;
    } /* endif */
  } /* endif */

  if ( (usRc != NO_ERROR) &&
       (usRc != BTREE_CORRUPTED) &&
       (usRc != VERSION_MISMATCH) )
  {
    EQFNTMClose( &pTmClb->TmBtree );
    EQFNTMClose( &pTmClb->InBtree );

    NTMDestroyLongNameTable( pTmClb );
    UtlAlloc( (PVOID *) &pTmClb, 0L, 0L, NOMSG );
  } /* endif */

  //set out values
  pTmOpenOut->pstTmClb = pTmClb;
  pTmOpenOut->stPrefixOut.usLengthOutput = sizeof( TMX_OPEN_OUT );
  pTmOpenOut->stPrefixOut.usTmtXRc = usRc;

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMTXOPEN_LOC, ERROR_EVENT, usRc );
  } /* endif */

  DEBUGEVENT( TMTXOPEN_LOC, FUNCEXIT_EVENT, 0 );

  return( usRc );
}




USHORT EqfMemory::OpenX()
{
  BOOL fOK;                      //success indicator
  USHORT usRc = NO_ERROR;        //return value
  USHORT usRc1 = NO_ERROR;       //return value
  ULONG  ulLen;                  //length indicator

  {

    //TmBtree.fb.fileName = pTmOpenIn->stTmOpen.szDataName;
    //InBtree.fb.fileName = pTmOpenIn->stTmOpen.szIndexName;
    //call open function for data file
    usRc1 = TmBtree.EQFNTMOpen((USHORT)(usAccessMode | ASD_FORCE_WRITE) );
    if ( (usRc1 == NO_ERROR) || (usRc1 == BTREE_CORRUPTED) )
    {
      //get signature record and add to control block
      USHORT usLen = sizeof( TMX_SIGN );

      usRc = TmBtree.EQFNTMSign((PCHAR) &(stTmSign), &usLen );

      if ( usRc == NO_ERROR )
      {
        if ( stTmSign.bGlobVersion > T5GLOBVERSION  || stTmSign.bMajorVersion > T5MAJVERSION )
        {
          T5LOG(T5ERROR) << "TM was created in newer vertions of t5memory, v0."<<stTmSign.bMajorVersion<<"."<<stTmSign.bMinorVersion;
          usRc = ERROR_VERSION_NOT_SUPPORTED;
        }
        else if (stTmSign.bGlobVersion < T5GLOBVERSION  || stTmSign.bMajorVersion < T5MAJVERSION )
        {
          T5LOG(T5ERROR) << "TM was created in older vertions of t5memory, v"<<stTmSign.bGlobVersion<<"."<<stTmSign.bMajorVersion<<"."<<stTmSign.bMinorVersion;
          usRc = VERSION_MISMATCH;
        } /* endif */
      }
      else if ( usAccessMode & ASD_ORGANIZE )
      {
        // allow to continue even if signature record is corrupted
        usRc = NO_ERROR;
      } /* endif */

      if ( (usRc == NO_ERROR) || (usRc == VERSION_MISMATCH) )
      {
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {          
          ulLen =  MAX_COMPACT_SIZE-1;
          //get compact area and add to control block
          USHORT usTempRc =  TmBtree.EQFNTMGet( COMPACT_KEY, (PCHAR)bCompact, &ulLen );

          // in organize mode allow continue if compact area is corrupted
          if ( (usTempRc != NO_ERROR) && (usTempRc != VERSION_MISMATCH) )
          {
            usTempRc = BTREE_CORRUPTED;
          } /* endif */

          if ( usTempRc == BTREE_CORRUPTED )
          {
            memset( bCompact, 0, sizeof(bCompact) );
          } /* endif */
          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */


        //get languages and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          DEBUGEVENT( TMTXOPEN_LOC, STATE_EVENT, 2 );

          //call to obtain exact length of record
          ulLen = 0;
          USHORT usTempRc = NTMLoadNameTable( this, LANG_KEY,
                                       (PBYTE *)&Languages, &ulLen );

          if ( usTempRc == BTREE_READ_ERROR ) 
              usTempRc = BTREE_CORRUPTED;
          usTempRc = NTMCreateLangGroupTable( this );

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get file names and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( this, FILE_KEY,
                                       (PBYTE *)&FileNames,
                                       &ulLen );

          // in organize mode allow continue if file name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            FileNames.ulAllocSize = TMX_TABLE_SIZE;
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get authors and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( this, AUTHOR_KEY,
                                       (PBYTE *)&Authors, &ulLen );

          // in organize mode allow continue if author name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {          
            Authors.ulAllocSize = TMX_TABLE_SIZE;            
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get tag tables and add to control block
      if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( this, TAGTABLE_KEY,
                                       (PBYTE *)&TagTables, &ulLen );


          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            usTempRc = TM_FILE_SCREWED_UP; // cannot continue if no tag tables
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get long document name table
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          // error in memory allocations will force end of open!
          USHORT usTempRc = NTMCreateLongNameTable( this );
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;

          // now read any long name table from database, if there is
          // none (=older TM before TOP97) write our empty one to the
          // Translation Memory
          if ( usTempRc == NO_ERROR )
          {
            usTempRc = NTMReadLongNameTable( this );

            switch ( usTempRc)
            {
              case ERROR_NOT_ENOUGH_MEMORY:
                // leave return code as is
                break;
              case NO_ERROR:
                // O.K. no problems at all
                break;
              case BTREE_NOT_FOUND :
              T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 48 usTempRc = NTMWriteLongNameTable( this );";
#ifdef TEMPORARY_COMMENTED
                // no long name tabel yet,create one ...
                usTempRc = NTMWriteLongNameTable( this );
                #endif
                break;
              default:
                // read of long name table failed, assume TM is corrupted
                //if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
                {
                } /* endif */
                usTempRc = BTREE_CORRUPTED;
                break;
            } /* endswitch */
          } /* endif */

          // just for debugging purposes: check if short and long name table have the same numbe rof entries
          if ( usTempRc == NO_ERROR )
          {
            if ( pLongNames->ulEntries != FileNames.ulMaxEntries )
            {
              // this is strange...
              int iSetBreakPointHere = 1;
            }
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */
      } /* endif */

      //add threshold to control block
      usThreshold = 33;//pTmOpenIn->stTmOpen.usThreshold;


      if ( (usRc == NO_ERROR) ||
           (usRc == BTREE_CORRUPTED) ||
           (usRc == VERSION_MISMATCH) )
      {
        //call open function for index file
        USHORT usIndexRc = InBtree.EQFNTMOpen(usAccessMode );
        if ( usIndexRc != NO_ERROR )
        {
          usRc = usIndexRc;
        } /* endif */
      } /* endif */

      if ( usRc == NO_ERROR)
      {
        // no error during open of index, use return code of EQFNTMOPEN for
        // data file
        usRc = usRc1;
      }
      else
      {
        // open of index failed or index is corrupted so leave usRc as-is;
        // data file will be closed in cleanup code below
      } /* endif */
    }
    else
    {
      // error during open of data file, use return code of EQFNTMOPEN for
      // data file
      usRc = usRc1;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Ensure that all required data areas have been loaded and the     */
  /* database files have been opened                                  */
  /********************************************************************/
  if ( (usRc == NO_ERROR) ||
       (usRc == BTREE_CORRUPTED) ||
       (usRc == VERSION_MISMATCH) )
  {
    if ( false //(pLanguages == NULL) ||
         //(pAuthors   == NULL) ||
         //(pTagTables == NULL) ||
         //(pFileNames == NULL) //||
         //(this.TmBtree == NULL) ||
         //(this.InBtree == NULL) 
         )
    {
      usRc = TM_FILE_SCREWED_UP;
    } /* endif */
  } /* endif */

  if ( (usRc != NO_ERROR) &&
       (usRc != BTREE_CORRUPTED) &&
       (usRc != VERSION_MISMATCH) )
  {
    EQFNTMClose( &TmBtree );
    EQFNTMClose( &InBtree );

    NTMDestroyLongNameTable( this );
    //UtlAlloc( (PVOID *) &this, 0L, 0L, NOMSG );
  } /* endif */

  //set out values
  //pTmOpenOut->pstTmClb = pTmClb;
  //pTmOpenOut->stPrefixOut.usLengthOutput = sizeof( TMX_OPEN_OUT );
  //pTmOpenOut->stPrefixOut.usTmtXRc = usRc;

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMTXOPEN_LOC, ERROR_EVENT, usRc );
  } /* endif */

  DEBUGEVENT( TMTXOPEN_LOC, FUNCEXIT_EVENT, 0 );

  return( usRc );
}


