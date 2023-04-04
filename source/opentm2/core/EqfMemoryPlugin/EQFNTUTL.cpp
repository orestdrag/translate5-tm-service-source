//+----------------------------------------------------------------------------+
//|EQFNTUTL.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Description:  Translation Memory utility functions                          |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|C NTMGetIDFromName - get an ID from a TM file, author, language or tag table|
//|C NTMGetNameFromID - get a name from a TM file, author,language or tag table|
//|C TmtXInfo                                                                  |
//|C NTMOpenProperties                                                         |
//|C NTMCloseOrganize                                                          |
//|C TmtXCloseOrganize                                                         |
//|C NTMConvertProperties                                                      |
//|C NTMReadWriteSegment                                                       |
//|C TmtXDeleteTM                                                              |
//|C NTMConvertCRLF                                                            |
//|C NTMLockTM                                                                 |
//|C NTMCheckForUpdates                                                        |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|C NTMCompNames - compare two names, called by bsearch and qsort             |
//|C NTMGetPointersToTable - get pointers to table and entries                 |
//|C NTMCheckPropFile                                                          |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <EQF.H>                  // General Translation Manager include file
// #include <time.h>
#include <string.h>

#include <tm.h>               // Private header file of Translation Memory
#include <EQFEVENT.H>             // Event logging
#include "LogWrapper.h"
#include "OSWrapper.h"

//#include "EQFCMPR.H"
/**********************************************************************/
/* prototypes for internal functions                                  */
/**********************************************************************/
int NTMCompNames( const void *, const void * );
USHORT NTMGetPointersToTable( PTMX_CLB, USHORT, PTMX_TABLE *,
                              PTMX_TABLE_ENTRY * );
USHORT NTMCheckPropFile( PSZ, PVOID *);
USHORT NTMAddNameToTable( PTMX_CLB, PSZ, USHORT, PUSHORT );
int NTMLongNameTableComp( const void *,  const void * );
int NTMLongNameTableCompCaseIgnore( const void *,  const void * );



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGetIDFromName                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMGetIDFromName( PTMX_CLB pTmClb,    //input            |
//|                                     PSZ      pszName,     //input          |
//|                                     USHORT   usTableType, //input          |
//|                                     PUSHORT  pusID   )    //output         |
//+----------------------------------------------------------------------------+
//|Description:       This function returns the ID of a name using the         |
//|                   passed tag table type. If name is not found in the       |
//|                   table it will be inserted. The table is sorted by name   |
//|                   in ascending order. If a new name is inserted the        |
//|                   specified table is updated in the TM control block and   |
//|                   in the TM QDAM file.                                     |
//+----------------------------------------------------------------------------+
//|Parameters:        pTmClb    - pointer to TM control block                  |
//|                   pszName     - name of ID to be returned                  |
//|                   usTableType - type of table: LANG_KEY                    |
//|                                                FILE_KEY                    |
//|                                                AUTHOR_KEY                  |
//|                                                TAGTABLE_KEY                |
//|                   pusID       - output: ID of pszName                      |
//|                                         The ID is set to 0 in any          |
//|                                         error case.                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes: NO_ERROR                 - hopefully returned in the most case |
//|             ERROR_INTERNAL           - parameter pTmClb is a NULL pointer  |
//|                                        parameter pszName is empty          |
//|             ERROR_TABLE_FULL         - a new entry must be inserted but    |
//|                                        the tabel is full                   |
//|             ERROR_NOT_ENOUGH_MEMORY  - reallocation of table failed        |
//|             others                   - return codes from QDAM              |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The TM must be opened before the function is called      |
//+----------------------------------------------------------------------------+
//|Side effects:      The table in the TM control block and the table record   |
//|                   in the TM QDAM file is updated when the table changed.   |
//|                   The table changed when the passed name is not found      |
//|                   and inserted into the table.                             |
//+----------------------------------------------------------------------------+
//|Samples:           usRc = NTMGetIDFromName( pTmClb,                         |
//|                                            "English(U.S.)"                 |
//|                                             LANG_KEY,                      |
//|                                             pusID       )                  |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|  overview connection of TM control block and table                         |
//|                                                                            |
//| TM_CLB               TM_TABLE               TM_TABLE_ENTRY                 |
//|+---------------+     +--------------+       +-------+          +-------+   |
//|| ............. | +-> | usAllocSize  | +-->  | szName| ........ | szName|   |
//|+---------------+ |   +--------------+ |     +-------+          +-------+   |
//|| pLanguages    +-+   |usMaxEntries  | |     | usID  | ........ | usID  |   |
//|+---------------+     +--------------+ |      -------+           -------+   |
//|| pAuthors      +-+   |stTmTableEntry+-+                                    |
//|+---------------+ |    --------------+                                      |
//|| pFiles        | |                          TM_TABLE_ENTRY                 |
//|+---------------+ |   +--------------+       +-------+          +-------+   |
//|| pTagtables    |  -> | usAllocSize  | +-->  | szName| ........ | szName|   |
//|+---------------+     +--------------+ |     +-------+          +-------+   |
//|| ............. |     |usMaxEntries  | |     | usID  | ........ | usID  |   |
//| ---------------+     +--------------+ |      -------+           -------+   |
//|                      |stTmTableEntry+-+                                    |
//|                       --------------+                                      |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| initialize function return code  usRc = NO_ERROR                           |
//| initialize ID, that in error case a 0 - ID is returned                     |
//|                                                                            |
//| if ( if input parameters OK )                                              |
//|   capitalize input string                                                  |
//|                                                                            |
//|   get pointer to table and table entries in dependency of the              |
//|   table type and set usRc                                                  |
//|                                                                            |
//|   if ( usRc == NO_ERROR  )                                                 |
//|     search name passed in pszName in passed table usTableType              |
//|                                                                            |
//|    if ( if passed name found in table  )                                   |
//|       get ID of entry and set pusID                                        |
//|     else                                                                   |
//|       Check if space is available in table so that new entry               |
//|       will fit into the table. In error case stop further                  |
//|       processing and return that table is full.                            |
//|                                                                            |
//|       if ( table is full )                                                 |
//|         usRc = ERROR_TABLE_FULL;                                           |
//|       end                                                                  |
//|       if ( usRc == NO_ERROR )                                              |
//|         insert name and id to table and sort tabele                        |
//|         update table record in TM QDAM file                                |
//|       end                                                                  |
//|     end                                                                    |
//|   end                                                                      |
//| else                                                                       |
//|   usRc = ERROR_INTERNAL;                                                   |
//| end                                                                        |
//|                                                                            |
//| return usRc;                                                               |
// ----------------------------------------------------------------------------+
USHORT
NTMGetIDFromName( PTMX_CLB pTmClb,   // input
                  PSZ      pszName,    // input
                  PSZ      pszLongName, // input, long name (only for FILE_KEY)
                  USHORT   usTableType,   //input
                  PUSHORT  pusID       )  //output
{
  return( NTMGetIDFromNameEx( pTmClb, pszName, pszLongName, usTableType,
                              pusID, 0L, NULL ) );

} /* end of function NTMGetIDFromName */

// NTMGetIDFromNameEx
// this is an enhanced version of NTMGetIDFromName
// using the new option NTMGETID_NOUPDATE_OPT the update of the name table can
// suppressed for new names (important for get operations which should
// not change the database)
// the pusAlternativeID is filled with the ID of the name in the long name
// table if no pszLongName has been specified and the pszName is found in 
// the short name table as well
USHORT NTMGetIDFromNameEx
( 
  PTMX_CLB    pTmClb,                  // input, memory control block pointer 
  PSZ         pszName,                 // input, name being looked up
  PSZ         pszLongName,             // input, long name (only for FILE_KEY)
  USHORT      usTableType,             // input, type of table to use
  PUSHORT     pusID,                   // output, ID for name being looked up
  LONG        lOptions,                // input, additional options
  PUSHORT     pusAlternativeID         // output, alternative ID
)
{
  USHORT            usRc = NO_ERROR;          //function return coed
  PTMX_TABLE_ENTRY  pstTMTableEntry = NULL;   //ptr to a table entry for bsearch
  PTMX_TABLE_ENTRY  pstTMTableEntries = NULL; //ptr to first table entry
  PTMX_TABLE        pstTMTable = NULL;        //ptr to table structure
  BOOL        fLongName = FALSE;

  // initialize ID
  if ( lOptions & NTMGETID_NOUPDATE_OPT )
  {
    *pusID = NTMGETID_NOTFOUND_ID;           // use as not-found indicator
  }
  else
  {
    *pusID = 0;
  } /* endif */
  if ( pusAlternativeID )  *pusAlternativeID = NTMGETID_NOTFOUND_ID;

  if ( pszName[0] != EOS)
  {
    //--- if input parameters OK
    if ( pTmClb && pszName[0] != EOS )
    {
      //--- capitalize input string
      strupr( pszName );

      /******************************************************************/
      /* get pointer to table and table entries in dependency of the    */
      /* table type                                                     */
      /******************************************************************/
      usRc = NTMGetPointersToTable( pTmClb,
                                    usTableType,
                                    &pstTMTable,
                                    &pstTMTableEntries );
      /****************************************************************/
      /* If we are looking for documents and a long document name is  */
      /* available ...                                                */
      /****************************************************************/

      // skip any leading backslash (added by bug in ITM code ...)
      if ( pszLongName != NULL )
      {
        if ( *pszLongName == BACKSLASH ) pszLongName++;
      } /* endif */

      fLongName = (usTableType == FILE_KEY) &&
                   (pszLongName != NULL) &&
                   (pszLongName[0] != EOS) &&
                   (strcmp( pszLongName, pszName ) != 0);

      if ( (usRc == NO_ERROR) && fLongName )
      {
        /**************************************************************/
        /* ... check first against our long name table                */
        /**************************************************************/
        PTMX_LONGNAME_TABLE_ENTRY pEntry;        // ptr to entry found
        TMX_LONGNAME_TABLE_ENTRY SearchEntry;    // entry being searched

        // prepare search entry
        SearchEntry.pszLongName = pszLongName;

        // do the actual search
        pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                     pTmClb->pLongNames->stTableEntry,
                                                     pTmClb->pLongNames->ulEntries,
                                                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                     NTMLongNameTableComp );


        // if search fails try again using case insenstive search
        if ( pEntry == NULL )
        {
          pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                       pTmClb->pLongNamesCaseIgnore->stTableEntry,
                                                       pTmClb->pLongNames->ulEntries,
                                                       sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                       NTMLongNameTableCompCaseIgnore );
        } /* endif */

        if ( pEntry != NULL )
        {
          // return ID of found entry
           *pusID = pEntry->usId;
        }
        else if ( !(lOptions & NTMGETID_NOUPDATE_OPT) )
        {
          ULONG   ulNameLen = strlen(pszLongName) + 1;
          ULONG   ulAddLen = ulNameLen + sizeof(USHORT);


          // add a new entry for the given short name
          if ( usRc == NO_ERROR )
          {
            // for some (yet unknown) reasons, there are memories which have fewer entries in
            // the short name table when in the long name table, using the ID generated when
            // adding the short name will lead to incorrect IDs for the long name table.
            // In order to circumvent this we call NTMAddNameToTable for the short name table
            // until the returned ID is larger than or equal to pTmClb->pLongNames->ulEntries
            do
            {
              DEBUGEVENT2( NTMGETIDFROMNAME_LOC, INFO_EVENT, 1, TM_GROUP, pszName );
              usRc = NTMAddNameToTable( pTmClb, pszName, usTableType, pusID );
            } while ( (usRc == NO_ERROR) && (*pusID < pTmClb->pLongNames->ulEntries) ); /* enddo */                 
          } /* endif */

          // add a new entry to the long name table
          if ( usRc == NO_ERROR )
          {
            // enlarge array if necessary
            if ( pTmClb->pLongNames->ulEntries >=
                                                pTmClb->pLongNames->ulTableSize)
            {
              ULONG ulOldSize = (ULONG)sizeof(TMX_LONGNAMETABLE) +
                                (ULONG)(sizeof(TMX_LONGNAME_TABLE_ENTRY) *
                                     pTmClb->pLongNames->ulTableSize);
              ULONG ulNewSize = ulOldSize + (ULONG)
                (sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES);

              if ( UtlAlloc( (PVOID *)&pTmClb->pLongNames,
                             ulOldSize, ulNewSize, NOMSG ) )
              {
                pTmClb->pLongNames->ulTableSize += LONGNAMETABLE_ENTRIES;
              }
              else
              {
                usRc = ERROR_NOT_ENOUGH_MEMORY;
              } /* endif */

              // enlarge case ignore table as well
              if ( usRc == NO_ERROR )
              {
                if ( !UtlAlloc( (PVOID *)&pTmClb->pLongNamesCaseIgnore,
                                ulOldSize, ulNewSize, NOMSG ) )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                } /* endif */
              } /* endif */
            } /* endif */

            // enlarge buffer if necessary
            if ( usRc == NO_ERROR )
            {
              if ( (pTmClb->pLongNames->ulBufUsed + ulAddLen ) >=
                    pTmClb->pLongNames->ulBufSize)
              {
                PSZ pszOldBuffer;      // ptr to old buffer area

                ULONG ulNewLen = pTmClb->pLongNames->ulBufUsed +
                                 ulAddLen + 256;
                pszOldBuffer = pTmClb->pLongNames->pszBuffer;
                if ( UtlAlloc( (PVOID *)&(pTmClb->pLongNames->pszBuffer),
                               pTmClb->pLongNames->ulBufSize,
                               ulNewLen, NOMSG ) )
                {
                  // remember new buffer size
                  pTmClb->pLongNames->ulBufSize = ulNewLen;

                  // adjust pointers in our table
                  {
                    ULONG ulI;
                    PTMX_LONGNAME_TABLE_ENTRY pEntry;       // ptr to table entry

                    pEntry = pTmClb->pLongNames->stTableEntry;

                    for ( ulI = 0; ulI < pTmClb->pLongNames->ulEntries; ulI++ )
                    {
                      pEntry->pszLongName = pTmClb->pLongNames->pszBuffer +
                                            (pEntry->pszLongName - pszOldBuffer);
                      pEntry++;
                    } /* endfor */
                  }
                }
                else
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                } /* endif */
              } /* endif */
            } /* endif */

            // add new entry to buffer
            if ( usRc == NO_ERROR )
            {
              PTMX_LONGNAME_TABLE_ENTRY pEntry;  // ptr to table entry
              PSZ pszTarget;                     // ptr into buffer

              // position to table entry
              pEntry = pTmClb->pLongNames->stTableEntry +
                       pTmClb->pLongNames->ulEntries;

              // position to free area in buffer (overwrite end delimiter!)
              pszTarget = pTmClb->pLongNames->pszBuffer +
                          (pTmClb->pLongNames->ulBufUsed - sizeof(USHORT));

              // add ID to buffer and table
              *((PUSHORT)pszTarget) = *pusID;
              pszTarget += sizeof(USHORT);
              pEntry->usId = *pusID;

              // add long name to buffer and table
              strcpy( pszTarget, pszLongName );
              pEntry->pszLongName = pszTarget;
              pszTarget += ulNameLen;

              // add new end delimiter
              *((PUSHORT)pszTarget) = 0;                   // end delimiter

              // adjust entry count and buffer used size
              pTmClb->pLongNames->ulEntries++;
              pTmClb->pLongNames->ulBufUsed += ulAddLen;
            } /* endif */

            // sort long name array
            if ( usRc == NO_ERROR )
            {
              qsort( pTmClb->pLongNames->stTableEntry,
                     pTmClb->pLongNames->ulEntries,
                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                     NTMLongNameTableComp );
            } /* endif */

            // copy to case ignore table and sort it
            if ( usRc == NO_ERROR )
            {
              memcpy( pTmClb->pLongNamesCaseIgnore->stTableEntry,
                      pTmClb->pLongNames->stTableEntry,
                      pTmClb->pLongNames->ulEntries *
                      sizeof(TMX_LONGNAME_TABLE_ENTRY) );

              qsort( pTmClb->pLongNamesCaseIgnore->stTableEntry,
                     pTmClb->pLongNames->ulEntries,
                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                     NTMLongNameTableCompCaseIgnore );
            } /* endif */

            // update TM record for long names
            if ( usRc == NO_ERROR )
            {
              DEBUGEVENT2( NTMGETIDFROMNAME_LOC, INFO_EVENT, 2, TM_GROUP, pszLongName );

              usRc = pTmClb->InBtree.EQFNTMUpdate( LONGNAME_KEY,
                                   (PBYTE)pTmClb->pLongNames->pszBuffer,
                                   pTmClb->pLongNames->ulBufUsed );
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        if ( usRc == NO_ERROR  )
        {
          /********************************************************************/
          /* search name passed in pszName in passed table usTableType        */
          /********************************************************************/
          pstTMTableEntry = (PTMX_TABLE_ENTRY)bsearch( pszName,
                                                       pstTMTableEntries,
                                                       pstTMTable->ulMaxEntries,
                                                       sizeof(TMX_TABLE_ENTRY),
                                                       NTMCompNames );
           //--- if passed name found in table
           if ( pstTMTableEntry )
           {
             //-- get ID of entry and set output parameter
             *pusID = pstTMTableEntry->usId;
           }
           else 
           {
             // name is not contained in name table
             if ( !(lOptions & NTMGETID_NOUPDATE_OPT) )
             {
               usRc = NTMAddNameToTable( pTmClb, pszName, usTableType,
                                         pusID );

               // for new languages update our language group table
               if ( (usRc == NO_ERROR) &&
                    (usTableType == LANG_KEY) )
               {
                 usRc = NTMAddLangGroup( pTmClb, pszName, *pusID );
               } /* endif */
             } /* endif */
           } /* endif */
        } /* endif */
      } /* endif */

      // file name table only:
      // look for given short name in long name table if not found yet or
      // an alternative file ID is requested
      if ( (usRc == NO_ERROR) && (usTableType == FILE_KEY) && !fLongName &&
           ((*pusID == NTMGETID_NOTFOUND_ID) || (pusAlternativeID != NULL)) )
      {
        PTMX_LONGNAME_TABLE_ENTRY pEntry;        // ptr to entry found
        TMX_LONGNAME_TABLE_ENTRY SearchEntry;    // entry being searched

        // prepare search entry
        SearchEntry.pszLongName = pszName;

        // do the actual search
        pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                     pTmClb->pLongNames->stTableEntry,
                                                     pTmClb->pLongNames->ulEntries,
                                                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                     NTMLongNameTableComp );

        if ( pEntry != NULL )
        {
          pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                       pTmClb->pLongNamesCaseIgnore->stTableEntry,
                                                       pTmClb->pLongNames->ulEntries,
                                                       sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                       NTMLongNameTableCompCaseIgnore );
        } /* endif */

        if ( pEntry != NULL )
        {
          // return ID of found entry
          if ( *pusID == NTMGETID_NOTFOUND_ID ) 
          {
            *pusID = pEntry->usId;
          }
          else if ( pusAlternativeID != NULL )
          {
            *pusAlternativeID = pEntry->usId;
          } /* endif */
        }
      } /* endif */
    }
    else  //--- pTmClb is NULL pointer or pszName is empty
    {
      //--- wrong function paramters
      usRc = ERROR_INTERNAL;
    } /* endif */
  } /* endif */

  return usRc;
} /* end of function NTMGetIDFromNameEx */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGetNameFromID                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMGetNameFromID( PTMX_CLB pTmClb,     //input           |
//|                                     PUSHORT pusID,         //intput        |
//|                                     USHORT  usTableType,   //input         |
//|                                     PSZ     pszName )      //output        |
//+----------------------------------------------------------------------------+
//|Description:       This function returnes the name of a passed ID. To get   |
//|                   the the passed table type will be used. If the passed    |
//|                   ID == 0 then corresponding TM QDAM record will be        |
//|                   initialized and saved to QDAM TM file.                   |
//+----------------------------------------------------------------------------+
//|Parameters:        pTmClb    - pointer to TM control block                  |
//|                   pusID       - ID of name to be returned in pszName       |
//|                                 if 0, the QDAM record usTableType is       |
//|                                 initialized and saved to QDAM              |
//|                   usTableType - type of table: LANG_KEY                    |
//|                                                FILE_KEY                    |
//|                                                AUTHOR_KEY                  |
//|                                                TAGTABLE_KEY                |
//|                   pszName     - returned: name of pusID                    |
//|                                           EOS in error case and when       |
//|                                           pusID is not found               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR         - hopefully returned in the most case   |
//|                   ERROR_INTERNAL   - parameter pTmClb is a NULL            |
//|                   ID_NOT_FOUND     - pusID not found in table              |
//|                   others           - return codes from QDAM                |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The TM must be opened before the function is called      |
//+----------------------------------------------------------------------------+
//|Side effects:      When the id passed in pusID == 0 the the QDAM record     |
//|                   in dependency of th passed usTableType will be           |
//|                   initialized and is saved to QDAM, therefor the record    |
//|                   will be reserved in QDAM TM file.                        |
//+----------------------------------------------------------------------------+
//|Samples:           usRc = NTMGetNameFromID( pTmClb,                         |
//|                                            &usID,                          |
//|                                            LANG_KEY,                       |
//|                                            pszName    )                    |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| initialize function return code  usRc = NO_ERROR                           |
//| initialize function fFound = FALSE                                         |
//|                                                                            |
//|   if ( control block is no NULL pointer )                                  |
//|     if ( passed ID == 0 )                                                  |
//|       initialize table and save it to QDAM                                 |
//|     else                                                                   |
//|       call function NTMGetPointersToTable to get pointer to table and      |
//|       table entries in dependency of the table type                        |
//|                                                                            |
//|       search ID in passed table usTableTye and get name of ID              |
//|                                                                            |
//|       if ( ID not found )                                                  |
//|         set usRc and reset pszName                                         |
//|     end                                                                    |
//|   else  //--- pTmClb is NULL pointer                                       |
//|     wrong function paramters set usRc = ERROR_INTERNAL                     |
//|   end                                                                      |
//|                                                                            |
//|   return usRc;                                                             |
// ----------------------------------------------------------------------------+
USHORT
NTMGetNameFromID( PTMX_CLB pTmClb,      //input
                  PUSHORT  pusID,         //intput
                  USHORT   usTableType,   //input
                  PSZ      pszName,       //output
                  PSZ      pszLongName )  //output, long name (only for FILE_KEY)

{
  USHORT           usRc = NO_ERROR;
  BOOL             fFound = FALSE;
  PTMX_TABLE_ENTRY pstTMTableEntries = NULL; //ptr to first table entry
  PTMX_TABLE       pstTMTable = NULL;        //ptr to table structure


  if ( pszLongName != NULL )
  {
    pszLongName[0] = EOS;
  } /* endif */

  //--- if input parameters OK
  if ( pTmClb )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( pTmClb,
                                  usTableType,
                                  &pstTMTable,
                                  &pstTMTableEntries );
    /****************************************************************/
    /* search ID in passed table usTableTye and get name of ID      */
    /****************************************************************/
    if ( usRc == NO_ERROR )
    {
      if ( *pusID == OVERFLOW_ID )
      {
        strcpy( pszName, OVERFLOW_NAME );
        fFound = TRUE;
      }
      else
      {
        ULONG ulI;
        for ( ulI = 0; ulI < pstTMTable->ulMaxEntries && !fFound; ulI++ )
        {
          if ( *pusID == pstTMTableEntries[ulI].usId )
          {
            fFound = TRUE;
            strcpy( pszName, pstTMTableEntries[ulI].szName );
          } /* endif */
        } /* endfor */
        /**************************************************************/
        /* Get any long document name for this ID                     */
        /**************************************************************/
        if ( fFound && (usTableType == FILE_KEY) && (pszLongName != NULL) )
        {
          BOOL fLongFound = FALSE;
          for ( ulI = 0;
                ulI < pTmClb->pLongNames->ulEntries && !fLongFound;
                ulI++ )
          {
            if ( *pusID == pTmClb->pLongNames->stTableEntry[ulI].usId )
            {
              fLongFound = TRUE;
              strcpy( pszLongName,
                      pTmClb->pLongNames->stTableEntry[ulI].pszLongName );
            } /* endif */
          } /* endfor */
        } /* endif */


        if ( !fFound )
        {
          /************************************************************/
          /* the ID was not found in the table                        */
          /* set usRc and reset pszName                               */
          /************************************************************/
          usRc = ID_NOT_FOUND;
          pszName[0] = EOS;
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else  //--- pTmClb is NULL pointer
  {
    //--- wrong function paramters
    usRc = ERROR_INTERNAL;
  } /* endif */

  return usRc;
} /* end of function NTMGetNameFromID */


PSZ NTMFindNameForID( PTMX_CLB pTmClb,      //input
                  PUSHORT  pusID,         //intput
                  USHORT   usTableType )  // input

{
  USHORT           usRc = NO_ERROR;
  BOOL             fFound = FALSE;
  PTMX_TABLE_ENTRY pstTMTableEntries = NULL; //ptr to first table entry
  PTMX_TABLE       pstTMTable = NULL;        //ptr to table structure
  PSZ pszFoundName = NULL;

  //--- if input parameters OK
  if ( pTmClb )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( pTmClb,
                                  usTableType,
                                  &pstTMTable,
                                  &pstTMTableEntries );
    /****************************************************************/
    /* search ID in passed table usTableTye and get name of ID      */
    /****************************************************************/
    if ( usRc == NO_ERROR )
    {
      if ( *pusID == OVERFLOW_ID )
      {
        pszFoundName = OVERFLOW_NAME;
        fFound = TRUE;
      }
      else
      {
        ULONG ulI;
        for ( ulI = 0; ulI < pstTMTable->ulMaxEntries && !fFound; ulI++ )
        {
          if ( *pusID == pstTMTableEntries[ulI].usId )
          {
            fFound = TRUE;
            pszFoundName = pstTMTableEntries[ulI].szName;
          } /* endif */
        } /* endfor */
        /**************************************************************/
        /* Get any long document name for this ID                     */
        /**************************************************************/
        if ( fFound && (usTableType == FILE_KEY) )
        {
          BOOL fLongFound = FALSE;
          for ( ulI = 0;
                ulI < pTmClb->pLongNames->ulEntries && !fLongFound;
                ulI++ )
          {
            if ( *pusID == pTmClb->pLongNames->stTableEntry[ulI].usId )
            {
              fLongFound = TRUE;
              pszFoundName = pTmClb->pLongNames->stTableEntry[ulI].pszLongName;
            } /* endif */
          } /* endfor */
        } /* endif */


        if ( !fFound )
        {
          /************************************************************/
          /* the ID was not found in the table                        */
          /* set usRc and reset pszName                               */
          /************************************************************/
          usRc = ID_NOT_FOUND;
          pszFoundName = NULL;
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else  //--- pTmClb is NULL pointer
  {
    //--- wrong function paramters
    usRc = ERROR_INTERNAL;
  } /* endif */

  return( pszFoundName );
} /* end of function NTMFindNameForID */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCompNames                                             |
//+----------------------------------------------------------------------------+
//|Function call:     NTMCompNames( PTMX_TABLE_ENTRY pstTMTableEntry1, //input |
//|                                 PTMX_TABLE_ENTRY pstTMTableEntry2 )//input |
//+----------------------------------------------------------------------------+
//|Description:       This function is called bt bsearch ans qsort. The names  |
//|                   of the passed table entries are compared and the         |
//|                   compare result is returned.                              |
//|                   pstTMTableEntry1->szName and                             |
//|                   pstTMTableEntry2->szName are compared                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   pstTMTableEntry1 - first table entry                     |
//|                   pstTMTableEntry2 - second table entry                    |
//+----------------------------------------------------------------------------+
//|Returncode type:   int                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       less than 0    : string1 less than string2               |
//|                   0              : string 1 identical string2              |
//|                   greater than 0 : string1 greater string3                 |
//+----------------------------------------------------------------------------+
//|Samples:           bsearch( pszName,                                        |
//|                            pstTMTableEntries,                              |
//|                            pstTMTable->usMaxEntries,                       |
//|                            sizeof(TMX_TABLE_ENTRY),                        |
//|                            NTMCompNames );                                 |
//|                                                                            |
//|                   qsort( pstTMTableEntries,                                |
//|                          pstTMTable->usMaxEntries,                         |
//|                          sizeof(TMX_TABLE_ENTRY),                          |
//|                          NTMCompNames );                                   |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| compare the names of the passed tabele entries using strcmp                |
//| return compare result                                                      |
// ----------------------------------------------------------------------------+
int
NTMCompNames( const void * pstTMTableEntry1,  //input
              const void * pstTMTableEntry2 ) //input
{
  /********************************************************************/
  /* compare the names of the passed tabele entries using strcmp      */
  /********************************************************************/
  return( strcmp( ((PTMX_TABLE_ENTRY) pstTMTableEntry1)->szName,
                  ((PTMX_TABLE_ENTRY) pstTMTableEntry2)->szName ));
} /* end of function NTMCompNames */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGetPointersToTable                                    |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//| NTMGetPointersToTable( PTMX_CLB         pTmClb,             //input      |
//|                        USHORT           usTableType,          //input      |
//|                        PTMX_TABLE       *ppstTMTable,         //output     |
//|                        PTMX_TABLE_ENTRY *ppstTMTableEntries ) //output     |
//+----------------------------------------------------------------------------+
//|Description:       returns pointers to table and table entries in           |
//|                   dependency of passed table type usTableType              |
//+----------------------------------------------------------------------------+
//|Parameters:        pTmClb            - pointer to TM control block        |
//|                   usTableType         - type of table: LANG_KEY            |
//|                                                FILE_KEY                    |
//|                                                AUTHOR_KEY                  |
//|                                                TAGTABLE_KEY                |
//|                   *ppstTMTable        - returned: pointer to table         |
//|                   *ppstTMTableEntries - returned: pointer to table entries |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR         - hopefully returned in the most case   |
//|                   ERROR_INTERNAL   - wrong value in parameter usTableType  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      must only be called by                                   |
//|                   NTMGetIDFromName and                                     |
//|                   NTMGetNameFromID                                         |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| for structure and connections of the tables see graphic in function        |
//| header of function NTMGetIDFromName                                        |
//| usRc = NO_ERROR                                                            |
//|                                                                            |
//| get pointer to table in dependency of passed usTableType                   |
//|                                                                            |
//| if error                                                                   |
//|   set usRc = ERROR_INTERNAL;                                               |
//| end                                                                        |
//|                                                                            |
//| return usRc                                                                |
// ----------------------------------------------------------------------------+
USHORT
NTMGetPointersToTable( PTMX_CLB         pTmClb,             //input
                       USHORT           usTableType,          //input
                       PTMX_TABLE       *ppstTMTable,         //output
                       PTMX_TABLE_ENTRY *ppstTMTableEntries ) //output
{
  USHORT usRc = NO_ERROR;                      //function return code

  /********************************************************************/
  /* get pointer to table in dependency of passed usTableType         */
  /********************************************************************/
  switch ( usTableType )
  {
    //-----------------------------------------------------------------------
    case LANG_KEY :
      *ppstTMTable = (PTMX_TABLE)&pTmClb->Languages;
      break;
    //-----------------------------------------------------------------------
    case FILE_KEY :
      *ppstTMTable = (PTMX_TABLE)&pTmClb->FileNames;
      break;
    //-----------------------------------------------------------------------
    case AUTHOR_KEY :
      *ppstTMTable = (PTMX_TABLE)&pTmClb->Authors;
      break;
    //-----------------------------------------------------------------------
    case TAGTABLE_KEY :
      *ppstTMTable = (PTMX_TABLE)&pTmClb->TagTables;
      break;
    //-----------------------------------------------------------------------
    case LANGGROUP_KEY :
      *ppstTMTable = (PTMX_TABLE)&pTmClb->LangGroups;
      break;
    //-----------------------------------------------------------------------
    default :
      usRc = ERROR_INTERNAL;
      *ppstTMTable        = NULL;
      *ppstTMTableEntries = NULL;
      break;
  } /* end switch */

  if ( usRc == NO_ERROR )
  {
    /******************************************************************/
    /* get pointer to table entries                                   */
    /******************************************************************/
    *ppstTMTableEntries = &((*ppstTMTable)->stTmTableEntry);
  } /* endif */

  return usRc;
} /* end of function NTMGetPointersToTable */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMOpenProperties                                        |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT                                                   |
//|                   NTMOpenProperties( HPROP * phProp,                       |
//|                                      PVOID * ppProp,                       |
//|                                      PSZ     pszPropName,                  |
//|                                      PSZ     pszSysPath,                   |
//|                                      USHORT  usMode,                       |
//|                                      BOOL    fMsg )                        |
//+----------------------------------------------------------------------------+
//|Description:       Open a TM property file of the passed TM name            |
//|                   for read or write access and                             |
//|                   returns handle and pointer to the properties.            |
//+----------------------------------------------------------------------------+
//|Parameters:        phProp      - (out) property handle                      |
//|                   ppProp      - (out) pointer to properties                |
//|                   pszPropName - (in)  property name with ext               |
//|                   pszPropPath - (in)  EQF system path X:\EQF               |
//|                   usMode      - (in)  access mode:  PROP_ACCESS_READ       |
//|                                                     PROP_ACCESS_WRITE      |
//|                   fMsg        - (in)  message handling parameter           |
//|                                         TRUE:  display error message       |
//|                                         FALSE: display no error message    |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR                                                 |
//|                   TM_PROPERTIES_NOT_OPENED                                 |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
NTMOpenProperties( HPROP   * phProp,    //(out) property handle
                   PVOID   * ppProp,    //(out) pointer to properties
                   PSZ     pszPropName, //(in)  property name with ext
                   PSZ     pszSysPath,  //(in)  EQF system path X:\EQF
                   USHORT  usMode,      //(in)  access mode:  PROP_ACCESS_READ
                                        //                    PROP_ACCESS_WRITE
                   BOOL    fMsg )       //(in)  message handling parameter
                                        //        TRUE:  display error message
                                        //        FALSE: display no error message age
{
  USHORT     usRc = NO_ERROR;           //function rc
  EQFINFO    ErrorInfo;                 //error returned from OpenProperties

  /********************************************************************/
  /* call function to open the property file                          */
  /********************************************************************/
  if( (*phProp = OpenProperties( pszPropName,
                                  pszSysPath,
                                  PROP_ACCESS_READ, &ErrorInfo)) == NULL)
  {
    usRc = TM_PROPERTIES_NOT_OPENED;
  } /* endif */

  /********************************************************************/
  /* if TM wass successfully opened and properties should be opened   */
  /* in WRITE mode the call function to set the access mode.          */
  /********************************************************************/
  if ( usRc == NO_ERROR && usMode == PROP_ACCESS_WRITE )
  {
    if( !SetPropAccess( *phProp, PROP_ACCESS_WRITE))
    {
      /****************************************************************/
      /* error set access mode                                        */
      /****************************************************************/
      usRc = TM_PROPERTIES_NOT_OPENED;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get the pointer to the property structure                        */
  /********************************************************************/
  if ( usRc == NO_ERROR )
  {
    if( (*ppProp = MakePropPtrFromHnd( *phProp ))== NULL )
    {
      usRc = TM_PROPERTIES_NOT_OPENED;
    } /* endif */
  } /* endif */

   if ( usRc == NO_ERROR )
   {
     /*****************************************************************/
     /* call function to check if the property are old or new one's   */
     /*****************************************************************/
     usRc = NTMCheckPropFile( pszPropName, ppProp );
   } /* endif */

  /********************************************************************/
  /* if an error accured and message handling should be done,         */
  /* call function to display error message.                          */
  /********************************************************************/
  if ( usRc != NO_ERROR && fMsg )
  {
    if ( usRc == ERROR_OLD_PROPERTY_FILE )
    {
      MemRcHandling( ERROR_OLD_PROPERTY_FILE, pszPropName, NULL, NULL );
    }
    else
    {
      MemRcHandling( TM_PROPERTIES_NOT_OPENED, pszPropName, NULL, NULL );
    } /* endif */
  } /* endif */

  return usRc;
} /* end of function NTMOpenProperties */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCheckPropFile                                         |
//+----------------------------------------------------------------------------+
//|Function call:     USHORT                                                   |
//|                   NTMCheckPropFile( PSZ    pszPropName,                    |
//|                                     PVOID  * ppProp     )                  |
//+----------------------------------------------------------------------------+
//|Description:       Checks if the property file is an old or new one         |
//+----------------------------------------------------------------------------+
//|Parameters:        pszPropName - (in) property name TMTEST.MEM              |
//|                   * ppProp    - (in) pointer to properties                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR                -  new property file             |
//|                   ERROR_OLD_PROPERTY_FILE -  old property file             |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The properties file must be opened and the returned      |
//|                   pointer to the properties must be passed to this         |
//|                   function in ppProp                                       |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
// ----------------------------------------------------------------------------+
USHORT
NTMCheckPropFile( PSZ    pszPropName,
                  PVOID  * ppProp )
{
  USHORT     usRc = NO_ERROR;
  PSZ        pszTemp;
  PPROP_NTM  pTmProp;

  /********************************************************************/
  /* get extension of property filename                               */
  /********************************************************************/
  pszTemp = strchr( pszPropName, DOT );

  if ( pszTemp )
  {
    /******************************************************************/
    /* A dot was found in the passed filename, so ew assume that      */
    /* an extension is also available.                                */
    /******************************************************************/
    //if ( (_strcmpi( pszTemp, EXT_OF_TMPROP ) != 0) || (_strcmpi( pszTemp, LANSHARED_MEM_PROP ) != 0) )
    //if ( (strcmp( pszTemp, EXT_OF_TMPROP ) != 0) 
      //|| (strcmp( pszTemp, LANSHARED_MEM_PROP ) != 0) 
    //  )
    {
      /****************************************************************/
      /* the extension of the passed property file name is the correct*/
      /* extension for TM property files                              */
      /* check if the marker for new TM properties is available       */
      /* in the properties                                            */
      /****************************************************************/
      pTmProp = (PPROP_NTM)*ppProp;
      if ( strncmp( pTmProp->szNTMMarker, NTM_MARKER, sizeof(NTM_MARKER) ) )
      {
        /**************************************************************/
        /* marker for new TM not found in the properties,             */
        /* this is an old TM property file, set usRc                  */
        /**************************************************************/
        usRc = ERROR_OLD_PROPERTY_FILE;
      } /* endif */
    } /* endif */
  } /* endif */

  return usRc;
} /* end of function NTMCheckPropFile */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMLockTM                                                |
//+----------------------------------------------------------------------------+
//|Function call:     NTMLockTM( clb, fLock, pfLocked );                       |
//+----------------------------------------------------------------------------+
//|Description:       Physically lock or unlock the data and the index file    |
//|                   of the given TM                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb,            pointer to control block     |
//|                   BOOL    fLock               TRUE = Lock, FALSE = Unlock  |
//|                   PBOOL   pfLocked            set to TRUE if TM has been   |
//|                                               locked                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
// ----------------------------------------------------------------------------+
USHORT NTMLockTM
(
  PTMX_CLB    pTmClb,                  // pointer to control block
  BOOL        fLock,                   // TRUE = Lock, FALSE = Unlock
  PBOOL       pfLocked                 // set to TRUE if TM has been locked
)
{
 USHORT usRc = 0;                      // function return code
 BOOL   fLockedData = FALSE;           // data-file-has-been-locked flag
 BOOL   fLockedIndex = FALSE;          // index-file-has-been-locked flag

 DEBUGEVENT( NTMLOCKTM_LOC, FUNCENTRY_EVENT, usRc );

 if ( fLock )
 {
   // Lock data file of TM
   usRc = pTmClb->TmBtree.EQFNTMPhysLock( TRUE, &fLockedData );


   // Lock Index file of TM
   if ( !usRc )
   {
         usRc = pTmClb->InBtree.EQFNTMPhysLock( TRUE, &fLockedIndex  );
   } /* endif */

   // Set caller's lock flag
   *pfLocked = fLockedData && fLockedIndex;

   // Unlock data file if lock of index failed
   if ( !fLockedIndex && fLockedData )
   {
     usRc = pTmClb->TmBtree.EQFNTMPhysLock( FALSE, &fLockedData );
   } /* endif */
 }
 else
 {
   // Rewrite compact area if compact area has been changed
   if ( pTmClb->bCompactChanged )
   {
     usRc = pTmClb->InBtree.EQFNTMUpdate( COMPACT_KEY,
                          pTmClb->bCompact, MAX_COMPACT_SIZE-1 );
     if ( !usRc )
     {
       pTmClb->bCompactChanged = FALSE;

       usRc = pTmClb->TmBtree.EQFNTMIncrUpdCounter( COMPACTAREA_UPD_COUNTER,
                             &(pTmClb->alUpdCounter[COMPACTAREA_UPD_COUNTER]) );
     } /* endif */
   } /* endif */

   // Unlock index file of TM
   usRc = pTmClb->InBtree.EQFNTMPhysLock( FALSE, &fLockedIndex );

   // Unlock data file of TM
   usRc =  pTmClb->TmBtree.EQFNTMPhysLock( FALSE, &fLockedData  );
 } /* endif */

 if ( usRc != NO_ERROR )
 {
   ERREVENT( NTMLOCKTM_LOC, ERROR_EVENT, usRc );
 } /* endif */

 return( usRc );
} /* end of function NTMLockTM */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCheckForUpdates                                       |
//+----------------------------------------------------------------------------+
//|Function call:     NTMCheckForUpdateas( pTmClb );                           |
//+----------------------------------------------------------------------------+
//|Description:       Check if the in-memory tables of the TM have been        |
//|                   modified in the database and reload modified tables.     |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB pTmClb,            pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
// ----------------------------------------------------------------------------+
USHORT NTMCheckForUpdates
(
  PTMX_CLB    pTmClb                   // pointer to control block
)
{
 USHORT usRc = 0;                      // function return code
 static LONG   alNewUpdCounter[MAX_UPD_COUNTERS]; // buffer for new update counters

T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 49 usRc = EQFNTMGetUpdCounter( pTmClb->pstTmBtree, alNewUpdCounter, 0, MAX_UPD_COUNTERS );";
#ifdef TEMPORARY_COMMENTED
 // Get new update counter values
 usRc = pTmClb->TmBtree.EQFNTMGetUpdCounter(  alNewUpdCounter, 0, MAX_UPD_COUNTERS );
#endif

 // Check and update compact area
 if ( !usRc )
 {
   if ( alNewUpdCounter[COMPACTAREA_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[COMPACTAREA_UPD_COUNTER] )
   {
     ULONG ulLen =  MAX_COMPACT_SIZE-1;

     usRc =  pTmClb->TmBtree.EQFNTMGet( COMPACT_KEY,
                       (PCHAR)pTmClb->bCompact, &ulLen );
   } /* endif */
 } /* endif */

 // Check authors table
 if ( !usRc )
 {
   if ( alNewUpdCounter[AUTHORS_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[AUTHORS_UPD_COUNTER] )
   {
     ULONG ulLen = 0;

     // Free any existing table
     //if ( pTmClb->Authors ) UtlAlloc( (PVOID *) &pTmClb->pAuthors, 0L, 0L, NOMSG );

     // Get new table
     usRc = NTMLoadNameTable( pTmClb, AUTHOR_KEY, (PBYTE *)&pTmClb->Authors, &ulLen );
   } /* endif */
 } /* endif */

 // Check languages table
 if ( !usRc )
 {
   if ( alNewUpdCounter[LANGUAGES_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[LANGUAGES_UPD_COUNTER] )
   {
     ULONG ulLen = 0;

     // Free any existing table
     //if ( pTmClb->pLanguages ) UtlAlloc( (PVOID *) &pTmClb->pLanguages, 0L, 0L, NOMSG );

     // Get new table
     usRc = NTMLoadNameTable( pTmClb, LANG_KEY, (PBYTE *)&pTmClb->Languages, &ulLen );
   } /* endif */
 } /* endif */

 // Check and update tag tables table
 if ( !usRc )
 {
   if ( alNewUpdCounter[TAGTABLES_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[TAGTABLES_UPD_COUNTER] )
   {
     ULONG ulLen = 0;

     // Free any existing table
     //if ( pTmClb->pTagTables ) UtlAlloc( (PVOID *) &pTmClb->pTagTables, 0L, 0L, NOMSG );

     // Get new table
     usRc = NTMLoadNameTable( pTmClb, TAGTABLE_KEY, (PBYTE *)&pTmClb->TagTables, &ulLen );
   } /* endif */
 } /* endif */

 // Check and update file names table
 if ( !usRc )
 {
   if ( alNewUpdCounter[FILENAMES_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[FILENAMES_UPD_COUNTER] )
   {
     ULONG ulLen = 0;

     // Free any existing table
     //if ( pTmClb->pFileNames ) UtlAlloc( (PVOID *) &pTmClb->pFileNames, 0L, 0L, NOMSG );

     // Get new table
     usRc = NTMLoadNameTable( pTmClb, FILE_KEY, (PBYTE *)&pTmClb->FileNames, &ulLen );
   } /* endif */
 } /* endif */


 // Check and update long file name table
 if ( !usRc )
 {
   if ( alNewUpdCounter[LONGNAMES_UPD_COUNTER] !=
                      pTmClb->alUpdCounter[LONGNAMES_UPD_COUNTER] )
   {
     // Free any existing table
     NTMDestroyLongNameTable( pTmClb );

     // create a new and empty long name table
     usRc = NTMCreateLongNameTable( pTmClb );

     // Get new table
     if ( !usRc )
     {
       T5LOG(T5ERROR) << "TEMPORARY COMMENTED NTMReadLongNameTable"; 
       //usRc = NTMReadLongNameTable( pTmClb );
     } /* endif */
   } /* endif */
 } /* endif */

 // Use new update counters as current update counters
 if ( !usRc )
 {
   memcpy( pTmClb->alUpdCounter, alNewUpdCounter, sizeof(pTmClb->alUpdCounter) );
 } /* endif */

 if ( usRc != NO_ERROR )
 {
   ERREVENT( NTMCHECKFORUPDATES_LOC, ERROR_EVENT, usRc );
 } /* endif */
 return( usRc );
} /* end of function NTMCheckForUpdates */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMCreateLongNameTable                                   |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = NTMCreateLongnameTable( pTmClb );                 |
//+----------------------------------------------------------------------------+
//|Description:       Creates an empty table for long document names.          |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT NTMCreateLongNameTable
(
  PTMX_CLB    pTmClb                   // pointer to control block
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // allocate initial long name pointer array
  if ( UtlAlloc( (PVOID *)&(pTmClb->pLongNames), 0L, (ULONG)
                 (sizeof(TMX_LONGNAMETABLE) +
                  sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES),
                 NOMSG ) )
  {
    pTmClb->pLongNames->ulTableSize = LONGNAMETABLE_ENTRIES;
    pTmClb->pLongNames->ulEntries   = 0;
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // allocate initial long name pointer array for case ignore search
  if ( UtlAlloc( (PVOID *)&(pTmClb->pLongNamesCaseIgnore), 0L, (ULONG)
                 (sizeof(TMX_LONGNAMETABLE) +
                  sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES),
                 NOMSG ) )
  {
    pTmClb->pLongNamesCaseIgnore->ulTableSize = LONGNAMETABLE_ENTRIES;
    pTmClb->pLongNamesCaseIgnore->ulEntries   = 0;
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // allocate initial long name buffer area
  if ( usRC == NO_ERROR )
  {
    if ( UtlAlloc( (PVOID *)&(pTmClb->pLongNames->pszBuffer), 0L,
                   (ULONG) LONGNAMEBUFFER_SIZE, NOMSG ) )
    {
      pTmClb->pLongNames->ulBufSize = (ULONG)LONGNAMEBUFFER_SIZE;
      pTmClb->pLongNames->ulBufUsed = (ULONG)sizeof(USHORT); // end-of-table delimiter
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // cleanup in case of errors
  if ( usRC != NO_ERROR )
  {
    NTMDestroyLongNameTable( pTmClb );
  } /* endif */

  // return to caller
  return( usRC );
} /* end of function NTMCreateLongNameTable */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMDestroyLongNameTable                                  |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = NTMDestroyLongnameTable( pTmClb );                |
//+----------------------------------------------------------------------------+
//|Description:       Destroys a long name table and frees the memory occupied |
//|                   by it.                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT NTMDestroyLongNameTable
(
  PTMX_CLB    pTmClb                   // pointer to control block
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // if there is a long name table ...
  if ( pTmClb->pLongNames != NULL )
  {
    // free any buffer area
    if ( pTmClb->pLongNames->pszBuffer != NULL )
    {
      UtlAlloc( (PVOID *)&(pTmClb->pLongNames->pszBuffer), 0L, 0L, NOMSG );
    } /* endif */

    // free table
    UtlAlloc( (PVOID *)&(pTmClb->pLongNames), 0L, 0L, NOMSG );
    pTmClb->pLongNames = NULL;
  } /* endif */

  if ( pTmClb->pLongNamesCaseIgnore != NULL )
  {
    // free table
    UtlAlloc( (PVOID *)&(pTmClb->pLongNamesCaseIgnore), 0L, 0L, NOMSG );
    pTmClb->pLongNamesCaseIgnore = NULL;
  } /* endif */

  // return to caller
  return( usRC );
} /* end of function NTMDestroyLongNameTable */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMReadLongNameTable                                     |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = NTMReadLongnameTable( pTmClb );                   |
//+----------------------------------------------------------------------------+
//|Description:       Reads the data of a long name table from the database.   |
//+----------------------------------------------------------------------------+
//|Parameters:        PTMX_CLB   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT NTMReadLongNameTable
(
  PTMX_CLB    pTmClb                   // pointer to control block
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  ULONG       ulLen = 0;               // record length

  // call to obtain exact length of record
  ulLen = 0;
  usRC =  pTmClb->TmBtree.EQFNTMGet( LONGNAME_KEY, 0, &ulLen );

  if ( usRC == NO_ERROR )
  {
    // allocate buffer area if it is too small
    if ( pTmClb->pLongNames->ulBufSize < ulLen )
    {
      if ( UtlAlloc( (PVOID *)&pTmClb->pLongNames->pszBuffer, 0L,
                     ulLen, NOMSG ))
      {
        pTmClb->pLongNames->ulBufSize = ulLen;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */

    // read long name table from database
    if ( usRC == NO_ERROR )
    {
      usRC =  pTmClb->TmBtree.EQFNTMGet( LONGNAME_KEY,
                        (PCHAR)pTmClb->pLongNames->pszBuffer, &ulLen );
    } /* endif */


    // unterse data if table data is tersed
    if ( usRC == NO_ERROR )
    {
      PTERSEHEADER pTerseHeader = (PTERSEHEADER)pTmClb->pLongNames->pszBuffer;
      PBYTE     pNewArea = NULL;         // ptr to unterse data area

      if ( pTerseHeader->ulMagicWord == TERSEMAGICWORD )
      {
        // table is tersed...

        // allocate buffer for untersed data
        if ( !UtlAlloc( (PVOID *)&pNewArea, 0L,
                        (LONG)pTerseHeader->usDataSize, NOMSG ) )
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */

        // unterse data
        if ( usRC == NO_ERROR )
        {
          ULONG  ulNewLen = 0;

          memcpy( pNewArea,
                  (PBYTE)pTmClb->pLongNames->pszBuffer + sizeof(TERSEHEADER),
                  ulLen - sizeof(TERSEHEADER) );
          T5LOG(T5ERROR) << "::TEMPORARY_COMMENTED in NTMReadLongNameTable, fUtlHuffmanExpand";
#ifdef TEMPORARY_COMMENTED
          if ( !fUtlHuffmanExpand( (PUCHAR)pNewArea, pTerseHeader->usDataSize,
                                &ulNewLen ) )
          {
            usRC = ERROR_NOT_ENOUGH_MEMORY; // expand failed most likely to
                                            // memory shortage
          } /* endif */
          #endif
        } /* endif */

        // set table data and cleanup
        if ( usRC == NO_ERROR )
        {
          // set size of buffer area
          pTmClb->pLongNames->ulBufSize = (ULONG)pTerseHeader->usDataSize;

          // free tersed data area
          UtlAlloc( (PVOID *)&pTmClb->pLongNames->pszBuffer, 0L, 0L, NOMSG );

          // anchor untersed data area
          pTmClb->pLongNames->pszBuffer = (PSZ)pNewArea;

          // avoid free of untersed data area
          pNewArea = NULL;
        } /* endif */
        if ( pNewArea != NULL ) UtlAlloc( (PVOID *)&pNewArea, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */

    // setup pointer array for long names
    if ( usRC == NO_ERROR )
    {
      ULONG ulEntries = 0;            // number of entries in buffer
      PSZ    pszTemp;                  // ptr for buffer processing

      // remember used space in buffer area
      pTmClb->pLongNames->ulBufUsed = ulLen;

      // count number of entries in long name buffer
      pszTemp = pTmClb->pLongNames->pszBuffer;
      while ( *((PUSHORT)pszTemp) != 0 )
      {
        ulEntries++;
        pszTemp += sizeof(USHORT);    // skip ID
        pszTemp += strlen(pszTemp)+1;// skip long name
      } /* endwhile */

      // enlarge pointer array if necessary
      if ( pTmClb->pLongNames->ulTableSize < ulEntries )
      {
        ULONG ulOldSize = sizeof(TMX_LONGNAMETABLE) +
                           (sizeof(TMX_LONGNAME_TABLE_ENTRY) *
                           pTmClb->pLongNames->ulTableSize);
        ULONG ulNewSize = sizeof(TMX_LONGNAMETABLE) +
                           (sizeof(TMX_LONGNAME_TABLE_ENTRY) * ulEntries);
        if ( UtlAlloc( (PVOID *)&pTmClb->pLongNames,
                       ulOldSize, ulNewSize, NOMSG ) )
        {
          pTmClb->pLongNames->ulTableSize = ulEntries;
        }
        else
        {
          usRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */

        // enlarge pointer array for case ignore search as well
        if ( usRC == NO_ERROR )
        {
          if ( UtlAlloc( (PVOID *)&pTmClb->pLongNamesCaseIgnore,
                         ulOldSize, ulNewSize, NOMSG ) )
          {
            pTmClb->pLongNamesCaseIgnore->ulTableSize = ulEntries;
          }
          else
          {
            usRC = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
      } /* endif */

      // fill pointer array
      if ( usRC == NO_ERROR )
      {
        PTMX_LONGNAME_TABLE_ENTRY pEntry = pTmClb->pLongNames->stTableEntry;
        pTmClb->pLongNames->ulEntries = 0;
        pszTemp = pTmClb->pLongNames->pszBuffer;
        while ( *((PUSHORT)pszTemp) != 0 )
        {

          // get ID of string
          pEntry->usId = *((PUSHORT)pszTemp);
          pszTemp += sizeof(USHORT);    // skip ID

          // set pointer to long document name
          pEntry->pszLongName = pszTemp;

          // continue with next entry
          pEntry++;
          pTmClb->pLongNames->ulEntries++;
          pszTemp += strlen(pszTemp)+1;// skip long name
        } /* endwhile */
      } /* endif */

      // sort long name array
      if ( usRC == NO_ERROR )
      {
        qsort( pTmClb->pLongNames->stTableEntry,
               pTmClb->pLongNames->ulEntries,
               sizeof(TMX_LONGNAME_TABLE_ENTRY),
               NTMLongNameTableComp );
      } /* endif */

      // make copy of long name array for case ignore search
      if ( usRC == NO_ERROR )
      {
        memcpy( pTmClb->pLongNamesCaseIgnore->stTableEntry,
                pTmClb->pLongNames->stTableEntry,
                pTmClb->pLongNames->ulEntries *
                sizeof(TMX_LONGNAME_TABLE_ENTRY) );

        qsort( pTmClb->pLongNamesCaseIgnore->stTableEntry,
               pTmClb->pLongNames->ulEntries,
               sizeof(TMX_LONGNAME_TABLE_ENTRY),
               NTMLongNameTableCompCaseIgnore );
      } /* endif */

    } /* endif */
  } /* endif */

  // return to caller
  return( usRC );
} /* end of function NTMReadLongNameTable */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMAddNameToTable                                        |
//+----------------------------------------------------------------------------+
//|Function call:     NTMAddNameToTable( PTMX_CLB pTmClb,    //input           |
//|                                      PSZ      pszName,     //input         |
//|                                      USHORT   usTableType, //input         |
//|                                      PUSHORT  pusID   )    //output        |
//+----------------------------------------------------------------------------+
//|Description:       This function will add the given name to the specified   |
//|                   table. The table is sorted by name in ascending order.   |
//|                   The specified table is updated in the TM control block   |
//|                   and in the TM QDAM file.                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        pTmClb    - pointer to TM control block                  |
//|                   pszName     - name of ID to be returned                  |
//|                   usTableType - type of table: LANG_KEY                    |
//|                                                FILE_KEY                    |
//|                                                AUTHOR_KEY                  |
//|                                                TAGTABLE_KEY                |
//|                   pusID       - output: ID of pszName                      |
//|                                         The ID is set to 0 in any          |
//|                                         error case.                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes: NO_ERROR                 - hopefully returned in the most case |
//|             ERROR_INTERNAL           - parameter pTmClb is a NULL pointer  |
//|                                        parameter pszName is empty          |
//|             ERROR_TABLE_FULL         - a new entry must be inserted but    |
//|                                        the tabel is full                   |
//|             ERROR_NOT_ENOUGH_MEMORY  - reallocation of table failed        |
//|             others                   - return codes from QDAM              |
// ----------------------------------------------------------------------------+
USHORT NTMAddNameToTable
(
  PTMX_CLB pTmClb,                   // input
  PSZ      pszName,                    // input
  USHORT   usTableType,                //input
  PUSHORT  pusID                       //output
)
{
  USHORT            usRc = NO_ERROR;          //function return coed
  PTMX_TABLE_ENTRY  pstTMTableEntries = NULL; //ptr to first table entry
  PTMX_TABLE        pstTMTable = NULL;        //ptr to table structure
  ULONG             ulNewSize;                //for size calculation
  ULONG             ulReallocSize;            //size for realloc of the table

  /********************************************************************/
  /* initialize ID, that in error case a 0 - ID is returned           */
  /********************************************************************/
  *pusID = 0;

  //--- if input parameters OK
  if ( pTmClb && pszName[0] != EOS )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( pTmClb,
                                  usTableType,
                                  &pstTMTable,
                                  &pstTMTableEntries );

    if ( usRc == NO_ERROR  )
    {
      /*************************************************************/
      /* Check if space is available in table so that new entry    */
      /* will fit into the table.                                  */
      /* Check if the allocated size can hold the new entry.       */
      /* If not reallocate the table and check that the table      */
      /* will not exceed the maximum size of a QDAM record (32K)   */
      /*************************************************************/
      ulNewSize = (ULONG)sizeof(TMX_TABLE) +
                  (pstTMTable->ulMaxEntries * (ULONG)sizeof(TMX_TABLE_ENTRY) );
      if ( ulNewSize >= pstTMTable->ulAllocSize )
      {
        ulReallocSize = (ULONG)pstTMTable->ulAllocSize + (ULONG)TMX_TABLE_SIZE;
        {
          if ( UtlAlloc( (PVOID *) &pstTMTable,
                          (LONG)pstTMTable->ulAllocSize,
                          (LONG)ulReallocSize,
                          NOMSG ) )
          {
            /*****************************************************/
            /* no error from UtlAlloc                            */
            /* reset the pointer in TM_CLB because UtlAlloc      */
            /* reallocates the storage on another place          */
            /*****************************************************/
            switch ( usTableType )
            {
              //---------------------------------------------------------
              case LANG_KEY :
                pTmClb->Languages = *pstTMTable;
                pstTMTableEntries = &pTmClb->Languages.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case FILE_KEY :
                pTmClb->FileNames = *pstTMTable;
                pstTMTableEntries = &pTmClb->FileNames.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case AUTHOR_KEY :
                pTmClb->Authors = *pstTMTable;
                pstTMTableEntries = &pTmClb->Authors.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case TAGTABLE_KEY :
                pTmClb->TagTables = *pstTMTable;
                pstTMTableEntries = &pTmClb->TagTables.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case LANGGROUP_KEY :
                pTmClb->LangGroups = *pstTMTable;
                pstTMTableEntries = &pTmClb->LangGroups.stTmTableEntry;
                break;
            } /* end switch */
            pstTMTable->ulAllocSize = ulReallocSize;
          }
          else
          {
            usRc = ERROR_NOT_ENOUGH_MEMORY;
          } /* endif */
        } /* endif */
      } /* endif */

      if ( usRc == NO_ERROR )
      {
        /*************************************************************/
        /* insert name and id to table and sort table                */
        /*************************************************************/
        *pusID = (USHORT)pstTMTable->ulMaxEntries + 1;
        strncpy( pstTMTableEntries[pstTMTable->ulMaxEntries].szName,
                pszName, MAX_LANG_LENGTH-1 );
        pstTMTableEntries[pstTMTable->ulMaxEntries].usId = *pusID;
        pstTMTable->ulMaxEntries++;
        qsort( pstTMTableEntries,
               pstTMTable->ulMaxEntries,
               sizeof(TMX_TABLE_ENTRY),
               NTMCompNames );

        // update table record in TM QDAM file (if not in read-only mode)
        if ( !(pTmClb->usAccessMode & ASD_READONLY) )
        {
          if ( usTableType != LANGGROUP_KEY )
          {
            usRc = pTmClb->InBtree.EQFNTMUpdate(
                                (ULONG)usTableType,
                                (PBYTE)pstTMTable,
                                pstTMTable->ulAllocSize );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else  //--- pTmClb is NULL pointer or pszName is empty
  {
    //--- wrong function paramters
    usRc = ERROR_INTERNAL;
  } /* endif */

  if ( usRc == ERROR_TABLE_FULL )
  {
    usRc = 0;
    *pusID = OVERFLOW_ID;
  } /* endif */

  return usRc;
} /* end of function NTMAddNameToTable */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMLongNameTableComp                                     |
//+----------------------------------------------------------------------------+
//|Description:       This function is called by bsearch and qsort. The names  |
//|                   of the passed table entries are compared and the         |
//|                   compare result is returned.                              |
// ----------------------------------------------------------------------------+
int NTMLongNameTableComp
(
  const void * pEntry1,  //input
  const void * pEntry2   //input
)
{
  /********************************************************************/
  /* compare the long names of the passed table entries using strcmp  */
  /********************************************************************/
  PSZ lname1 = ((PTMX_LONGNAME_TABLE_ENTRY)pEntry1)->pszLongName;
  PSZ lname2 = ((PTMX_LONGNAME_TABLE_ENTRY)pEntry2)->pszLongName;
  return( strcmp( lname1, lname2 ) );
} /* end of function NTMCompNames */

int NTMLongNameTableCompCaseIgnore
(
  const void * pEntry1,  //input
  const void * pEntry2   //input
)
{
  /********************************************************************/
  /* compare the long names of the passed table entries using strcmp  */
  /********************************************************************/
  //return( _stricmp( ((PTMX_LONGNAME_TABLE_ENTRY)pEntry1)->pszLongName,
  //                 ((PTMX_LONGNAME_TABLE_ENTRY)pEntry2)->pszLongName ));
  return( strcmp( ((PTMX_LONGNAME_TABLE_ENTRY)pEntry1)->pszLongName,
                   ((PTMX_LONGNAME_TABLE_ENTRY)pEntry2)->pszLongName ));
} /* end of function NTMCompNames */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMSaveNameTable                                         |
//+----------------------------------------------------------------------------+
//|Description:       This function saves a TM name table to the database.     |
//|                   If the size of the table exceeds 32k the table is        |
//|                   tersed.                                                  |
// ----------------------------------------------------------------------------+
USHORT NTMSaveNameTable
(
  PTMX_CLB    pTmClb,                  // ptr to TM control block
  ULONG       ulTableKey,              // key of table record
  PBYTE       pTMTable,                // ptr to table data
  ULONG       ulSize                   // size of table data
)
{
  USHORT      usRc = NO_ERROR;         // function return code

  // check which method is to be used for table
  usRc = pTmClb->InBtree.EQFNTMUpdate( ulTableKey, pTMTable, ulSize );

  // return to caller
  return( usRc );
} /* end of function NTMSaveNameTable */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMLoadNameTable                                         |
//+----------------------------------------------------------------------------+
//|Description:       This function loads a TM name table from the database.   |
//|                   If the table is tersed it is uncompressed.               |
// ----------------------------------------------------------------------------+
USHORT NTMLoadNameTable
(
  PTMX_CLB    pTmClb,                  // ptr to TM control block
  ULONG       ulTableKey,              // key of table record
  PBYTE       *ppTMTable,              // ptr to table data pointer
  PULONG      pulSize                  // ptr to buffer for size of table data
)
{
  USHORT      usRc = NO_ERROR;         // function return code

  // call to obtain exact length of record
  *pulSize = 0;
  usRc =  pTmClb->TmBtree.EQFNTMGet( ulTableKey, 0, pulSize );

  // allocate table data area
  if ( usRc == NO_ERROR )
  {
    if ( !UtlAlloc( (PVOID *)ppTMTable, 0L, *pulSize, NOMSG ))
    {
      usRc = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // read table data
  if ( usRc == NO_ERROR )
  {
    ULONG ulLen = *pulSize;
    usRc =  pTmClb->TmBtree.EQFNTMGet( ulTableKey, (PCHAR)(*ppTMTable), &ulLen );
  } /* endif */

  // handle tersed name tables
  if ( usRc == NO_ERROR )
  {
    PTERSEHEADER pTerseHeader = (PTERSEHEADER)*ppTMTable;
    PBYTE     pNewArea = NULL;         // ptr to unterse data area

    if ( pTerseHeader->ulMagicWord == TERSEMAGICWORD )
    {
      // table is tersed...

      // allocate buffer for untersed data
      if ( !UtlAlloc( (PVOID *)&pNewArea, 0L,
                      (LONG)pTerseHeader->usDataSize, NOMSG ) )
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */

      // unterse data
      if ( usRc == NO_ERROR )
      {
        ULONG ulNewLen = 0;

        memcpy( pNewArea, *ppTMTable + sizeof(TERSEHEADER),
                *pulSize - sizeof(TERSEHEADER) );
       
        T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 51 in NTMLoadNameTable";
#ifdef TEMPORARY_COMMENTED
        if ( !fUtlHuffmanExpand( (PUCHAR)pNewArea, pTerseHeader->usDataSize,
                              &ulNewLen ) )
        {
          usRc = ERROR_NOT_ENOUGH_MEMORY; // expand failed most likely to
                                          // memory shortage
        } /* endif */
        #endif
      } /* endif */

      // set table data and cleanup
      if ( usRc == NO_ERROR )
      {
        // set size of name table
        *pulSize = pTerseHeader->usDataSize;

        // free tersed data area
        UtlAlloc( (PVOID *)ppTMTable, 0L, 0L, NOMSG );

        // anchor untersed data area
        *ppTMTable = pNewArea;

        // avoid free of untersed data area
        pNewArea = NULL;
      } /* endif */
      if ( pNewArea != NULL ) UtlAlloc( (PVOID *)&pNewArea, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  // special handling for language name table:
  //   correct word delimiter in "Other Languages" which is
  //   0xA0 under Windows (due to OemToAnsi) but should be 0xFF
  if ( (usRc == NO_ERROR) && (ulTableKey == LANG_KEY) )
  {
    LONG lRest = *pulSize;
    PBYTE pbTemp = *ppTMTable;
    while ( lRest )
    {
      if ( *pbTemp == 0xA0 )
      {
        *pbTemp = 0xFF;
      } /* endif */
      pbTemp++;
      lRest--;
    } /* endwhile */
  } /* endif */

  // return to caller
  return( usRc );
} /* end of function NTMLoadNameTable */


USHORT NTMAddLangGroup
(
  PTMX_CLB    pTmClb,                  // ptr to TM control block
  PSZ         pszLang,                 // ptr to language name
  USHORT      sLangID                  // ID of language in our tables
)
{
  SHORT  sGroupID = 0;
  USHORT usRC = NO_ERROR;              // function return code
  CHAR   szLangGroup[MAX_LANGUAGE_PROPERTIES]; // buffer for language group name

  // get language properties (incl. group name)
  if ( GetLanguageGroup( pszLang, szLangGroup ) )
  {
    if ( szLangGroup[0] == EOS )
    {
      // no language group for language, so use language ID as
      // language group ID but multiply it with -1 to distinguish
      // language group IDs from language IDs
      sGroupID = sLangID * -1;
    } /* endif */
  }
  else
  {
    // no language properties for language, so use language ID as
    // language group ID but multiply it with -1 to distinguish
    // language group IDs from language IDs
      sGroupID = sLangID * -1;
  } /* endif */

  // get ID of group name
  if ( (usRC == NO_ERROR) && (sGroupID == 0) )
  {
    usRC = NTMGetIDFromName( pTmClb, szLangGroup, NULL, LANGGROUP_KEY, (PUSHORT)&sGroupID );
  } /* endif */

  // enlarge language-ID-to-group-ID table if necessary
  if ( usRC == NO_ERROR )
  {
    LONG lRequiredSize = (sLangID + 1) * sizeof(SHORT);
    if ( pTmClb->lLangIdToGroupTableSize < lRequiredSize )
    {
      lRequiredSize += (10 * sizeof(SHORT));
      if ( UtlAlloc( (PVOID *)&(pTmClb->psLangIdToGroupTable),
                      pTmClb->lLangIdToGroupTableSize,
                      lRequiredSize, NOMSG ) )
      {
        pTmClb->lLangIdToGroupTableSize = lRequiredSize;
        pTmClb->lLangIdToGroupTableUsed = lRequiredSize;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  // update language ID to group ID table
  if ( usRC == NO_ERROR )
  {
    pTmClb->psLangIdToGroupTable[sLangID] = sGroupID;
  } /* endif */

  return( usRC );
} /* end of function NTMAddLangGroup */

USHORT NTMCreateLangGroupTable
(
  PTMX_CLB    pTmClb                   // ptr to TM control block
)
{
  USHORT usRC = NO_ERROR;
  pTmClb->LangGroups.ulAllocSize = TMX_TABLE_SIZE;

  // allocate language-ID-to-group-ID-table
 
  LONG lSize = pTmClb->Languages.ulMaxEntries > 100L ?  pTmClb->Languages.ulMaxEntries : 100L;
  lSize *= sizeof(SHORT);
  if( UtlAlloc( (PVOID *)&(pTmClb->psLangIdToGroupTable),
                0L, lSize, NOMSG ) )
  {
    pTmClb->lLangIdToGroupTableSize = lSize;
    pTmClb->lLangIdToGroupTableUsed = 0L;
  }
  else
  {
    usRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // get group IDs for all languages and fill map table
  if ( usRC == NO_ERROR )
  {
    int i = 0;
    while ( (usRC == NO_ERROR) &&
            (i < (int)pTmClb->Languages.ulMaxEntries) )
    {
      PTMX_TABLE_ENTRY pstEntry = &(pTmClb->Languages.stTmTableEntry);
      usRC = NTMAddLangGroup( pTmClb,
                              pstEntry[i].szName,
                              pstEntry[i].usId );
      i++;
    } /* endwhile */
  } /* endif */

  return( usRC );
} /* end of function NTMCreateLangGroupTable */


// 
// function NTMOrganizeIndexFile
//
// compact/organize the index part of a memory
//
USHORT NTMOrganizeIndexFile
(
  PTMX_CLB pTmClb               // ptr to control block,
)
{
  USHORT usRC = NO_ERROR;
  usRC = EQFNTMOrganizeIndex( &(pTmClb->InBtree), pTmClb->usAccessMode, START_KEY );

  return( usRC );
} /* end of function NTMOrganizeIndexFile */


///////////////////////////////////////////////////////////////////////////////////////////////
///   functions for working with the additional data area following the target CLB          ///
///////////////////////////////////////////////////////////////////////////////////////////////

// compute the size of the additional data for the given input 
USHORT NTMComputeAddDataSize( PSZ_W pszContext, PSZ_W pszAddInfo )
{
  USHORT usLength = 0;

  if ( (pszContext != NULL) && (*pszContext != 0) )
  {
    // length = characters in context + end delimiter + size field + IF field
    usLength = usLength + (USHORT)((wcslen( pszContext) + 3) * sizeof(CHAR_W));
  } /* endif */     

  if ( (pszAddInfo != NULL) && (*pszAddInfo != 0) )
  {
    // length = characters in add.info + end delimiter + size field + IF field
    usLength = usLength + (USHORT)((wcslen( pszAddInfo) + 3) * sizeof(CHAR_W));
  } /* endif */     

  if ( usLength != 0 )
  {
    // if data is available we need an area end delimiter
    usLength += sizeof(USHORT);
  } /* endif */     

  return( usLength );
}


// check if the data is old context data or new additional data
BOOL NTMIsAddData( PUSHORT pusData )
{
  BOOL fAddData = FALSE;

  if ( (*pusData == ADDDATA_ADDINFO_ID) || (*pusData == ADDDATA_CONTEXT_ID) || (*pusData == ADDDATA_ENDOFDATA_ID) )
  {
    fAddData = TRUE;
  } /* endif */     
  return( fAddData );
} /* end of function NTMIsAddData */ 

// find specified data area in additional data, returns ptr to start of area or NULL
PUSHORT NTMFindData( PUSHORT pusData, USHORT usDataID )
{
  while ( (*pusData != 0) && (*pusData != ADDDATA_ENDOFDATA_ID)  && (*pusData != usDataID) )
  {
    USHORT usLen = pusData[1];
    pusData += usLen + 2;
  } /* endwhile */     

  return( (*pusData == usDataID) ? pusData : NULL );
} /* end of function NTMFindData */ 

// get length of specific data in the combined data area, returns length of data area in number of CHAR_Ws
USHORT NtmGetAddDataLen( PTMX_TARGET_CLB pCLB, USHORT usDataID )
{
  USHORT usLength = 0;

  if ( pCLB->usAddDataLen != 0 )       // do we have additional data ?
  {
    PUSHORT pusData = (PUSHORT)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pusData ) )
    {
      pusData = NTMFindData( pusData, usDataID );

      if ( pusData != NULL )
      {
        usLength = pusData[1];
      } /* endif */         
    }
    else
    {
      // only old format context data is available
      if ( usDataID == ADDDATA_CONTEXT_ID )
      {
        usLength = (USHORT)(wcslen( (const wchar_t *) pusData ) + 1);
      } /* endif */         
    } /* endif */       
  } /* endif */     

  return( usLength );
} /* end of function NtmGetAddDataLen */

// store/combine additional data in the combined area, returns new size of combined data area or 0 in case of errors
USHORT NtmStoreAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszNewData )
{
  PUSHORT pusData = (PUSHORT)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));
  USHORT usNewLength = (USHORT)(wcslen( pszNewData ));
  if ( usNewLength != 0 ) usNewLength += 1;


  // convert any old style comment
  if ( (pCLB->usAddDataLen != 0) && !NTMIsAddData( pusData ) )
  {
    USHORT usWords = pCLB->usAddDataLen / sizeof(USHORT);

    // make room for ID and length field
    memmove( pusData + 2, pusData, pCLB->usAddDataLen );

    // insert ID and length 
    *pusData = ADDDATA_CONTEXT_ID;
    pusData[1] = pCLB->usAddDataLen;

    // add data area end identifier
    pusData[usWords+2] = ADDDATA_ENDOFDATA_ID;

    // correct usAddDataLen
    pCLB->usAddDataLen += (3 * sizeof(USHORT));
  } /* endif */     

  if ( pCLB->usAddDataLen != 0 )       
  {
    // remove any old data for this ID
    PUSHORT pusOldData = NTMFindData( pusData, usDataID );
    if ( pusOldData  != NULL )
    {
      PUSHORT pusEndOfData = (PUSHORT)(((PBYTE)pusOldData ) + pCLB->usAddDataLen - 2);
      USHORT usOldLen = pusOldData [1];
      PUSHORT pusSource = pusOldData + usOldLen + 2;
      PUSHORT pusTarget = pusOldData ;

      while ( pusSource <= pusEndOfData )
      {
        *pusTarget++ = *pusSource++;
      } /* endwhile */         

      pCLB->usAddDataLen = pCLB->usAddDataLen - ((usOldLen + 2)*sizeof(USHORT));
    } /* endif */       

    // add new data to end of data area
    if ( usNewLength != 0 )
    {
      PUSHORT pusTarget = (PUSHORT)(((PBYTE)pusData) + pCLB->usAddDataLen - 2);
      *pusTarget++ = usDataID;
      *pusTarget++ = usNewLength;
      while ( *pszNewData != 0 ) *pusTarget++ = *pszNewData++;
      *pusTarget++ = 0;
      *pusTarget++ = ADDDATA_ENDOFDATA_ID;
      pCLB->usAddDataLen += ((usNewLength + 2 ) * sizeof(USHORT));
    } /* endif */       
  }
  else if ( usNewLength != 0 )
  {
    // no additional data yet, copy new data
    *pusData++ = usDataID;
    *pusData++ = usNewLength;
    pusData += 2;
    pszNewData[usNewLength] = 0;
    //wcscpy( (PSZ_W)pusData, pszNewData);
    memcpy( pusData, pszNewData, sizeof(wchar_t)*(usNewLength) );
    pusData += usNewLength * 2;
    *pusData = ADDDATA_ENDOFDATA_ID;
    pCLB->usAddDataLen = (usNewLength + 3) * sizeof(wchar_t);
  } /* endif */     

  return( pCLB->usAddDataLen );
} /* end of function NtmStoreAddData */

// retrieve specific data from the combined data area, returns length of retrieved data (incl. string end delimiter)
USHORT NtmGetAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszBuffer, USHORT usBufSize )
{
  USHORT usLength = 0;

  if ( pCLB->usAddDataLen != 0 )       // do we have additional data ?
  {
    PUSHORT pusData = (PUSHORT)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pusData ) )     // is it additional data?
    {
      pusData = NTMFindData( pusData, usDataID );

      if ( pusData != NULL )
      {
        usLength = pusData[1];
        pusData += 4;
        if ( usLength < usBufSize )
        {
          pszBuffer[0] = 0;
          memcpy(pszBuffer, pusData, usLength*sizeof(wchar_t) );
          pszBuffer[usLength] = 0;
          //wcscpy( pszBuffer, (wchar_t *) pusData );
        }
        else
        {
          usLength = 0;
        } /* endif */           
      } /* endif */         
    }
    else
    {
      // only old format context data is available
      if ( usDataID == ADDDATA_CONTEXT_ID )
      {
        usLength = (USHORT)(wcslen( (const wchar_t *) pusData ) + 1);
        if ( usLength < usBufSize )
        {
          memcpy( pszBuffer, pusData, (usLength)*sizeof(wchar_t) );
          //wcscpy( pszBuffer, (const wchar_t *) pusData );
        }
        else
        {
          usLength = 0;
        } /* endif */           
      } /* endif */         
    } /* endif */       
  } /* endif */     

  return( usLength );
} /* end of function NtmGetAddData */


// find a string in a specific data area
BOOL NtmFindInAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszSearch )
{
  USHORT usLength = 0;
  BOOL fFound = FALSE;

  if ( pCLB->usAddDataLen != 0 )       // do we have additional data ?
  {
    PUSHORT pusData = (PUSHORT)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pusData ) )     // is it additional data?
    {
      pusData = NTMFindData( pusData, usDataID );

      if ( pusData != NULL )
      {
        usLength = pusData[1];
        if ( usLength > 0 )
        {
          if ( wcsstr( (const wchar_t *) (pusData + 2), (const wchar_t *) pszSearch ) != NULL )
          {
            fFound = TRUE;
          } /* endif */             
        } /* endif */           
      } /* endif */         
    }
    else
    {
      // only old format context data is available
      if ( usDataID == ADDDATA_CONTEXT_ID )
      {
        usLength = (USHORT)(wcslen( (const wchar_t *) pusData ) + 1);
        if ( usLength != 0 )
        {
          if ( wcsstr( (const wchar_t *) pusData, (const wchar_t *) pszSearch ) != NULL )
          {
            fFound = TRUE;
          } /* endif */             
        } /* endif */           
      } /* endif */         
    } /* endif */       
  } /* endif */     

  return( fFound );
} /* end of function NtmFindInAddData */
