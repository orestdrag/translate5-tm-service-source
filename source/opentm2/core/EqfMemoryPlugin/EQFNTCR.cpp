//+----------------------------------------------------------------------------+
//|EQFNTCR.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   G.Dreckschmidt                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  New Translation Memory                                        |
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
// $Revision: 1.2 $ ----------- 11 Jan 2005
// GQ: - implicitely open created files in ASD_LOCKED mode
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
// $Revision: 1.2 $ ----------- 18 Sep 2001
// -- RJ: Cleanup
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2000
// - added handling for language group table
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - added support for large name tables
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFNTCR.CV_   1.2   13 May 1997 10:24:26   BUILD  $
 *
 * $Log:   J:\DATA\EQFNTCR.CV_  $
 *
 *    Rev 1.2   13 May 1997 10:24:26   BUILD
 * - handling for splitted usVersion field of signature record
 *
 *    Rev 1.1   24 Feb 1997 11:49:36   BUILD
 * - added support for long document names
 *
 *    Rev 1.0   09 Jan 1996 09:20:18   BUILD
 * Initial revision.
*/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>
#include "FilesystemWrapper.h"
#include "LogWrapper.h"
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXCreate   creates data and index files                |
//+----------------------------------------------------------------------------+
//|Description:       Creates qdam data and index files                        |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXCreate( PTMX_CREATE_IN pTmCreateIn, //input struct      |
//|                            PTMX_CREATE_OUT pTmCreateOut ) //output struct  |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_CREATE_IN pTmCreateIn     input structure             |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_CREATE_OUT pTmCreateOut   output structure           |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in the create out structure           |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| allocate TM control block structure - TM_CLB                               |
//|                                                                            |
//|   fill TM_SIGN structure as follows:                                       |
//|     szName - name of data file from TM_CREATE with extension               |
//|     szServer - server from TM_CREATE                                       |
//|     szUserid - user id from TM_CREATE                                      |
//|     szTmCreateTime - get current time with Time function                   |
//|     szSourceLanguage - source language from TM_CREATE                      |
//|     usVersion - use define set in eqftmi.h                                 |
//|     szDescription - description from TM_CREATE                             |
//|                                                                            |
//|   create tm data file                                                      |
//|   if successful                                                            |
//|     initialize the author, tag table, file name and compact area           |
//|     records in the tm data file                                            |
//|                                                                            |
//|   create tm index file                                                     |
//|   return the out structure                                                 |
// ----------------------------------------------------------------------------+

USHORT TmtXCreate
(
  PTMX_CREATE_IN pTmCreateIn,    //ptr to input struct
  PTMX_CREATE_OUT pTmCreateOut   //ptr to output struct
)
{
  BOOL       fOK;                      //success indicator
  EqfMemory*   pTmClb = NULL;            //pointer to control block
  PSZ        pszName;                  //pointer to string
  ULONG      ulKey;                    //qdam data file keys
  USHORT     usRc = NO_ERROR;          //return value

  //allocate control block
  fOK = UtlAlloc( (PVOID *) &pTmClb, 0L, (LONG)sizeof( EqfMemory ), NOMSG );

  if ( fOK )
  {
    usRc = pTmClb->NTMCreateLongNameTable();
    
    fOK = (usRc == NO_ERROR );
  } /* endif */
  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    //build name and extension of tm data file
    pszName = UtlGetFnameFromPath( pTmCreateIn->stTmCreate.szDataName );

    //fill signature record structure
    strcpy( pTmClb->stTmSign.szName, pszName );
    UtlTime( &(pTmClb->stTmSign.lTime) );
    strcpy( pTmClb->stTmSign.szSourceLanguage,
            pTmCreateIn->stTmCreate.szSourceLanguage );

    //TODO - replace version with current t5memory version
    pTmClb->stTmSign.bGlobVersion = T5GLOBVERSION;
    pTmClb->stTmSign.bMajorVersion = T5MAJVERSION;
    pTmClb->stTmSign.bMinorVersion = T5MINVERSION;
    strcpy( pTmClb->stTmSign.szDescription,
            pTmCreateIn->stTmCreate.szDescription );

    const char* pFullName = pTmCreateIn->stTmCreate.szDataName;
    //call create function for data file
    pTmClb->usAccessMode = ASD_LOCKED;         // new TMs are always in exclusive access...

    usRc = pTmClb->TmBtree.EQFNTMCreate((PCHAR) &(pTmClb->stTmSign), sizeof(TMX_SIGN), FIRST_KEY);
  
    if ( usRc == NO_ERROR )
    {
      //insert initialized record to tm data file
      ulKey = AUTHOR_KEY;
      usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                 (PBYTE)&pTmClb->Authors, TMX_TABLE_SIZE );

      if ( usRc == NO_ERROR )
      {
        ulKey = FILE_KEY;
        usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                    (PBYTE)&pTmClb->FileNames, TMX_TABLE_SIZE );     
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        ulKey = TAGTABLE_KEY;
        usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                    (PBYTE)&pTmClb->TagTables, TMX_TABLE_SIZE );
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        ulKey = LANG_KEY;
        usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                 (PBYTE)&pTmClb->Languages, TMX_TABLE_SIZE );
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        int size = sizeof( MAX_COMPACT_SIZE-1 );//OLD, probably bug
        size = MAX_COMPACT_SIZE-1 ;
        //initialize and insert compact area record
        memset( pTmClb->bCompact, 0, size );

        ulKey = COMPACT_KEY;
        usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                             pTmClb->bCompact, size);  
 
      } /* endif */

      // add long document table record
      if ( usRc == NO_ERROR )
      {
         ulKey = LONGNAME_KEY;
        // write long document name buffer area to the database
        usRc = pTmClb->TmBtree.EQFNTMInsert(&ulKey,
                            (PBYTE)pTmClb->pLongNames->pszBuffer,
                            pTmClb->pLongNames->ulBufUsed );        
  
      } /* endif */

      // create language group table
      if ( usRc == NO_ERROR )
      {
        usRc = pTmClb->NTMCreateLangGroupTable();
        
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        //modify name fields in signature record structure as ext different
        pszName = UtlGetFnameFromPath( pTmCreateIn->stTmCreate.szIndexName );
        //fill signature record structure
        strcpy( pTmClb->stTmSign.szName, pszName );

        //HERE .TMI file is created
        usRc = pTmClb->InBtree.EQFNTMCreate((PCHAR) &(pTmClb->stTmSign), sizeof( TMX_SIGN ), START_KEY);
                             
      } /* endif */

      if(usRc == NO_ERROR){        
        filesystem_flush_buffers(pFullName);  
        filesystem_flush_buffers(pTmCreateIn->stTmCreate.szIndexName);
      }/* endif */

    } /* endif */

    if ( usRc )
    {
      //something went wrong during create or insert so delete data file
      UtlDelete( pTmCreateIn->stTmCreate.szDataName, 0L, FALSE );
    } /* endif */
  } /* endif */

  if ( usRc )
  {
    //free allocated memory
    pTmClb->NTMDestroyLongNameTable();
    UtlAlloc( (PVOID *) &pTmClb, 0L, 0L, NOMSG );
  } /* endif */

  //set return values
  pTmCreateOut->pstTmClb = pTmClb;
  pTmCreateOut->stPrefixOut.usLengthOutput = sizeof( TMX_CREATE_OUT );
  pTmCreateOut->stPrefixOut.usTmtXRc = usRc;

  return( usRc );
}

BOOL AllocTable( PTMX_TABLE *ppTable )
{
  BOOL       fOK;    //success indicator

  //fOK = UtlAlloc( (PVOID *) ppTable, 0L, (LONG)( TMX_TABLE_SIZE ), NOMSG );
  //if ( fOK )
  //{
  //  (*ppTable)->ulAllocSize = TMX_TABLE_SIZE;
  //} /* endif */
  //return( fOK );
}

