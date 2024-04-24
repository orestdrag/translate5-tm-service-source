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
int NTMLongNameTableComp( const void *,  const void * );
int NTMLongNameTableCompCaseIgnore( const void *,  const void * );



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGetIDFromName                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMGetIDFromName( EqfMemory* pTmClb,    //input            |
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
EqfMemory::NTMGetIDFromName(
                  PSZ      pszName,    // input
                  PSZ      pszLongName, // input, long name (only for FILE_KEY)
                  USHORT   usTableType,   //input
                  PUSHORT  pusID       )  //output
{
  return( NTMGetIDFromNameEx( pszName, pszLongName, usTableType,
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
USHORT EqfMemory::NTMGetIDFromNameEx
( 
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
    if ( pszName[0] != EOS )
    {
      //--- capitalize input string
      if(usTableType != FILE_KEY)
      {
        strupr( pszName );
      }

      /******************************************************************/
      /* get pointer to table and table entries in dependency of the    */
      /* table type                                                     */
      /******************************************************************/
      usRc = NTMGetPointersToTable( usTableType,
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
                   (pszLongName[0] != EOS) 
                   //&& (strcmp( pszLongName, pszName ) != 0)
                   ;

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
                                                     pLongNames->stTableEntry,
                                                     pLongNames->ulEntries,
                                                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                     NTMLongNameTableComp );


        // if search fails try again using case insenstive search
        if ( pEntry == NULL )
        {
          pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                       pLongNamesCaseIgnore->stTableEntry,
                                                       pLongNames->ulEntries,
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
              usRc = NTMAddNameToTable( pszName, usTableType, pusID );
            } while ( (usRc == NO_ERROR) && (*pusID < pLongNames->ulEntries) ); /* enddo */                 
          } /* endif */

          // add a new entry to the long name table
          if ( usRc == NO_ERROR )
          {
            // enlarge array if necessary
            if ( pLongNames->ulEntries >= pLongNames->ulTableSize)
            {
              ULONG ulOldSize = (ULONG)sizeof(TMX_LONGNAMETABLE) +
                                (ULONG)(sizeof(TMX_LONGNAME_TABLE_ENTRY) *
                                     pLongNames->ulTableSize);
              ULONG ulNewSize = ulOldSize + (ULONG)
                (sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES);

              if ( UtlAlloc( (PVOID *)&pLongNames,
                             ulOldSize, ulNewSize, NOMSG ) )
              {
                pLongNames->ulTableSize += LONGNAMETABLE_ENTRIES;
              }
              else
              {
                LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
              } /* endif */

              // enlarge case ignore table as well
              if ( usRc == NO_ERROR )
              {
                if ( !UtlAlloc( (PVOID *)&pLongNamesCaseIgnore,
                                ulOldSize, ulNewSize, NOMSG ) )
                {
                  LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
                } /* endif */
              } /* endif */
            } /* endif */

            // enlarge buffer if necessary
            if ( usRc == NO_ERROR )
            {
              if ( (pLongNames->ulBufUsed + ulAddLen ) >=
                    pLongNames->ulBufSize)
              {
                PSZ pszOldBuffer;      // ptr to old buffer area

                ULONG ulNewLen = pLongNames->ulBufUsed +
                                 ulAddLen + 256;
                pszOldBuffer = pLongNames->pszBuffer;
                if ( UtlAlloc( (PVOID *)&(pLongNames->pszBuffer),
                               pLongNames->ulBufSize,
                               ulNewLen, NOMSG ) )
                {
                  // remember new buffer size
                  pLongNames->ulBufSize = ulNewLen;

                  // adjust pointers in our table
                  {
                    ULONG ulI;
                    PTMX_LONGNAME_TABLE_ENTRY pEntry;       // ptr to table entry

                    pEntry = pLongNames->stTableEntry;

                    for ( ulI = 0; ulI < pLongNames->ulEntries; ulI++ )
                    {
                      pEntry->pszLongName = pLongNames->pszBuffer +
                                            (pEntry->pszLongName - pszOldBuffer);
                      pEntry++;
                    } /* endfor */
                  }
                }
                else
                {
                  LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
                } /* endif */
              } /* endif */
            } /* endif */

            // add new entry to buffer
            if ( usRc == NO_ERROR )
            {
              PTMX_LONGNAME_TABLE_ENTRY pEntry;  // ptr to table entry
              PSZ pszTarget;                     // ptr into buffer

              // position to table entry
              pEntry = pLongNames->stTableEntry +
                       pLongNames->ulEntries;

              // position to free area in buffer (overwrite end delimiter!)
              pszTarget = pLongNames->pszBuffer +
                          (pLongNames->ulBufUsed - sizeof(USHORT));

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
              pLongNames->ulEntries++;
              pLongNames->ulBufUsed += ulAddLen;
            } /* endif */

            // sort long name array
            if ( usRc == NO_ERROR )
            {
              qsort( pLongNames->stTableEntry,
                     pLongNames->ulEntries,
                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                     NTMLongNameTableComp );
            } /* endif */

            // copy to case ignore table and sort it
            if ( usRc == NO_ERROR )
            {
              memcpy( pLongNamesCaseIgnore->stTableEntry,
                      pLongNames->stTableEntry,
                      pLongNames->ulEntries *
                      sizeof(TMX_LONGNAME_TABLE_ENTRY) );

              qsort( pLongNamesCaseIgnore->stTableEntry,
                     pLongNames->ulEntries,
                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                     NTMLongNameTableCompCaseIgnore );
            } /* endif */

            // update TM record for long names
            if ( usRc == NO_ERROR )
            {
              DEBUGEVENT2( NTMGETIDFROMNAME_LOC, INFO_EVENT, 2, TM_GROUP, pszLongName );

              usRc = TmBtree.EQFNTMUpdate( LONGNAME_KEY,
                                   (PBYTE)pLongNames->pszBuffer,
                                   pLongNames->ulBufUsed );
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
               usRc = NTMAddNameToTable( pszName, usTableType,
                                         pusID );

               // for new languages update our language group table
               if ( (usRc == NO_ERROR) &&
                    (usTableType == LANG_KEY) )
               {
                 usRc = NTMAddLangGroup( pszName, *pusID );
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
                                                     pLongNames->stTableEntry,
                                                     pLongNames->ulEntries,
                                                     sizeof(TMX_LONGNAME_TABLE_ENTRY),
                                                     NTMLongNameTableComp );

        if ( pEntry != NULL )
        {
          pEntry = (PTMX_LONGNAME_TABLE_ENTRY)bsearch( &SearchEntry,
                                                       pLongNamesCaseIgnore->stTableEntry,
                                                       pLongNames->ulEntries,
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
      LOG_AND_SET_RC(usRc, T5INFO, ERROR_INTERNAL);
    } /* endif */
  } /* endif */

  return usRc;
} /* end of function NTMGetIDFromNameEx */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMGetNameFromID                                         |
//+----------------------------------------------------------------------------+
//|Function call:     NTMGetNameFromID( EqfMemory* pTmClb,     //input           |
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
//|Samples:           usRc = pTmClb->NTMGetNameFromID(                         |
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
EqfMemory::NTMGetNameFromID(  PUSHORT  pusID,         //intput
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
  //if ( pTmClb )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( usTableType,
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
                ulI < pLongNames->ulEntries && !fLongFound;
                ulI++ )
          {
            if ( *pusID == pLongNames->stTableEntry[ulI].usId )
            {
              fLongFound = TRUE;
              strcpy( pszLongName,
                      pLongNames->stTableEntry[ulI].pszLongName );
            } /* endif */
          } /* endfor */

          if(!fLongFound){
            strcpy(pszLongName, pszName);
          }
        } /* endif */


        if ( !fFound )
        {
          /************************************************************/
          /* the ID was not found in the table                        */
          /* set usRc and reset pszName                               */
          /************************************************************/
          LOG_AND_SET_RC(usRc, T5INFO, ID_NOT_FOUND);
          pszName[0] = EOS;
        } /* endif */
      } /* endif */
    } /* endif */
  }
  return usRc;
} /* end of function NTMGetNameFromID */


PSZ EqfMemory::NTMFindNameForID(
                  PUSHORT  pusID,         //intput
                  USHORT   usTableType )  // input

{
  USHORT           usRc = NO_ERROR;
  BOOL             fFound = FALSE;
  PTMX_TABLE_ENTRY pstTMTableEntries = NULL; //ptr to first table entry
  PTMX_TABLE       pstTMTable = NULL;        //ptr to table structure
  PSZ pszFoundName = NULL;

  //--- if input parameters OK
  //if ( pTmClb )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( usTableType,
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
                ulI < pLongNames->ulEntries && !fLongFound;
                ulI++ )
          {
            if ( *pusID == pLongNames->stTableEntry[ulI].usId )
            {
              fLongFound = TRUE;
              pszFoundName = pLongNames->stTableEntry[ulI].pszLongName;
            } /* endif */
          } /* endfor */
        } /* endif */


        if ( !fFound )
        {
          /************************************************************/
          /* the ID was not found in the table                        */
          /* set usRc and reset pszName                               */
          /************************************************************/
          LOG_AND_SET_RC(usRc, T5INFO, ID_NOT_FOUND);
          pszFoundName = NULL;
        } /* endif */
      } /* endif */
    } /* endif */
  }

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
//| NTMGetPointersToTable( EqfMemory*         pTmClb,             //input      |
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
EqfMemory::NTMGetPointersToTable( 
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
      *ppstTMTable = (PTMX_TABLE)&Languages;
      break;
    //-----------------------------------------------------------------------
    case FILE_KEY :
      *ppstTMTable = (PTMX_TABLE)&FileNames;
      break;
    //-----------------------------------------------------------------------
    case AUTHOR_KEY :
      *ppstTMTable = (PTMX_TABLE)&Authors;
      break;
    //-----------------------------------------------------------------------
    case TAGTABLE_KEY :
      *ppstTMTable = (PTMX_TABLE)&TagTables;
      break;
    //-----------------------------------------------------------------------
    case LANGGROUP_KEY :
      *ppstTMTable = (PTMX_TABLE)&LangGroups;
      break;
    //-----------------------------------------------------------------------
    default :
      LOG_AND_SET_RC(usRc, T5INFO, ERROR_INTERNAL);
      *ppstTMTable        = NULL;
      *ppstTMTableEntries = NULL;
      break;
  } /* end switch */

  if ( usRc == NO_ERROR )
  {
    /******************************************************************/
    /* get pointer to table entries                                   */
    /******************************************************************/
    *ppstTMTableEntries = &((*ppstTMTable)->stTmTableEntry[0]);
  } /* endif */

  return usRc;
} /* end of function NTMGetPointersToTable */



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
//|Parameters:        EqfMemory* pTmClb,            pointer to control block     |
//|                   BOOL    fLock               TRUE = Lock, FALSE = Unlock  |
//|                   PBOOL   pfLocked            set to TRUE if TM has been   |
//|                                               locked                       |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
// ----------------------------------------------------------------------------+
USHORT NTMLockTM
(
  EqfMemory*    pTmClb,                  // pointer to control block
  BOOL        fLock,                   // TRUE = Lock, FALSE = Unlock
  PBOOL       pfLocked                 // set to TRUE if TM has been locked
)
{
 USHORT usRc = 0;                      // function return code
 BOOL   fLockedData = FALSE;           // data-file-has-been-locked flag
 BOOL   fLockedIndex = FALSE;          // index-file-has-been-locked flagf

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
   pTmClb->RewriteCompactTable();

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
//|Function name:     NTMCreateLongNameTable                                   |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = NTMCreateLongnameTable( pTmClb );                 |
//+----------------------------------------------------------------------------+
//|Description:       Creates an empty table for long document names.          |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory*   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::NTMCreateLongNameTable()
{
  USHORT      usRC = NO_ERROR;         // function return code

  // allocate initial long name pointer array
  if ( UtlAlloc( (PVOID *)&(pLongNames), 0L, (ULONG)
                 (sizeof(TMX_LONGNAMETABLE) +
                  sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES),
                 NOMSG ) )
  {
    pLongNames->ulTableSize = LONGNAMETABLE_ENTRIES;
    pLongNames->ulEntries   = 0;
  }
  else
  {
    LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  // allocate initial long name pointer array for case ignore search
  if ( UtlAlloc( (PVOID *)&(pLongNamesCaseIgnore), 0L, (ULONG)
                 (sizeof(TMX_LONGNAMETABLE) +
                  sizeof(TMX_LONGNAME_TABLE_ENTRY) * LONGNAMETABLE_ENTRIES),
                 NOMSG ) )
  {
    pLongNamesCaseIgnore->ulTableSize = LONGNAMETABLE_ENTRIES;
    pLongNamesCaseIgnore->ulEntries   = 0;
  }
  else
  {
    LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  // allocate initial long name buffer area
  if ( usRC == NO_ERROR )
  {
    if ( UtlAlloc( (PVOID *)&(pLongNames->pszBuffer), 0L,
                   (ULONG) LONGNAMEBUFFER_SIZE, NOMSG ) )
    {
      pLongNames->ulBufSize = (ULONG)LONGNAMEBUFFER_SIZE;
      pLongNames->ulBufUsed = (ULONG)sizeof(USHORT); // end-of-table delimiter
    }
    else
    {
      LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
    } /* endif */
  } /* endif */

  // cleanup in case of errors
  if ( usRC != NO_ERROR )
  {
    NTMDestroyLongNameTable( );
  } /* endif */

  // return to caller
  return( usRC );
} /* end of function NTMCreateLongNameTable */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMDestroyLongNameTable                                  |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = NTMDestroyLongnameTable();                |
//+----------------------------------------------------------------------------+
//|Description:       Destroys a long name table and frees the memory occupied |
//|                   by it.                                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        EqfMemory*   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::NTMDestroyLongNameTable()
{
  USHORT      usRC = NO_ERROR;         // function return code

  // if there is a long name table ...
  if ( pLongNames != NULL )
  {
    // free any buffer area
    if ( pLongNames->pszBuffer != NULL )
    {
      UtlAlloc( (PVOID *)&(pLongNames->pszBuffer), 0L, 0L, NOMSG );
    } /* endif */

    // free table
    UtlAlloc( (PVOID *)&(pLongNames), 0L, 0L, NOMSG );
    pLongNames = NULL;
  } /* endif */

  if ( pLongNamesCaseIgnore != NULL )
  {
    // free table
    UtlAlloc( (PVOID *)&(pLongNamesCaseIgnore), 0L, 0L, NOMSG );
    pLongNamesCaseIgnore = NULL;
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
//|Parameters:        EqfMemory*   pTmClb           pointer to control block     |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     error return code or NO_ERROR if O.K.         |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::NTMReadLongNameTable(){
  USHORT      usRC = NO_ERROR;         // function return code
  LONG        lLen = 0;               // record length

  // call to obtain exact length of record
  usRC =  TmBtree.EQFNTMGet( LONGNAME_KEY, 0, &lLen );

  if ( usRC == NO_ERROR )
  {
    // allocate buffer area if it is too small
    if ( pLongNames->ulBufSize < lLen )
    {
      if ( UtlAlloc( (PVOID *)&pLongNames->pszBuffer, 0L,
                     lLen, NOMSG ))
      {
        pLongNames->ulBufSize = lLen;
      }
      else
      {
        LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
      } /* endif */
    } /* endif */

    // read long name table from database
    if ( usRC == NO_ERROR )
    {
      usRC = TmBtree.EQFNTMGet( LONGNAME_KEY,
                        (PCHAR)pLongNames->pszBuffer, &lLen );
    } /* endif */


    // unterse data if table data is tersed
    if ( usRC == NO_ERROR )
    {
      PTERSEHEADER pTerseHeader = (PTERSEHEADER)pLongNames->pszBuffer;
      PBYTE     pNewArea = NULL;         // ptr to unterse data area

      if ( pTerseHeader->ulMagicWord == TERSEMAGICWORD )
      {
        // table is tersed...

        // allocate buffer for untersed data
        if ( !UtlAlloc( (PVOID *)&pNewArea, 0L,
                        (LONG)pTerseHeader->usDataSize, NOMSG ) )
        {
          LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
        } /* endif */

        // unterse data
        if ( usRC == NO_ERROR )
        {
          ULONG  ulNewLen = 0;

          memcpy( pNewArea,
                  (PBYTE)pLongNames->pszBuffer + sizeof(TERSEHEADER),
                  lLen - sizeof(TERSEHEADER) );
          T5LOG(T5ERROR) << "::TEMPORARY_COMMENTED in NTMReadLongNameTable, fUtlHuffmanExpand";
#ifdef TEMPORARY_COMMENTED
          if ( !fUtlHuffmanExpand( (PUCHAR)pNewArea, pTerseHeader->usDataSize,
                                &ulNewLen ) )
          {
            LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY); // expand failed most likely to
                                            // memory shortage
          } /* endif */
          #endif
        } /* endif */

        // set table data and cleanup
        if ( usRC == NO_ERROR )
        {
          // set size of buffer area
          pLongNames->ulBufSize = (ULONG)pTerseHeader->usDataSize;

          // free tersed data area
          UtlAlloc( (PVOID *)&pLongNames->pszBuffer, 0L, 0L, NOMSG );

          // anchor untersed data area
          pLongNames->pszBuffer = (PSZ)pNewArea;

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
      pLongNames->ulBufUsed = lLen;

      // count number of entries in long name buffer
      pszTemp = pLongNames->pszBuffer;
      while ( *((PUSHORT)pszTemp) != 0 )
      {
        ulEntries++;
        pszTemp += sizeof(USHORT);    // skip ID
        pszTemp += strlen(pszTemp)+1;// skip long name
      } /* endwhile */

      // enlarge pointer array if necessary
      if ( pLongNames->ulTableSize < ulEntries )
      {
        ULONG ulOldSize = sizeof(TMX_LONGNAMETABLE) +
                           (sizeof(TMX_LONGNAME_TABLE_ENTRY) *
                           pLongNames->ulTableSize);
        ULONG ulNewSize = sizeof(TMX_LONGNAMETABLE) +
                           (sizeof(TMX_LONGNAME_TABLE_ENTRY) * ulEntries);
        if ( UtlAlloc( (PVOID *)&pLongNames,
                       ulOldSize, ulNewSize, NOMSG ) )
        {
          pLongNames->ulTableSize = ulEntries;
        }
        else
        {
          LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
        } /* endif */

        // enlarge pointer array for case ignore search as well
        if ( usRC == NO_ERROR )
        {
          if ( UtlAlloc( (PVOID *)&pLongNamesCaseIgnore,
                         ulOldSize, ulNewSize, NOMSG ) )
          {
            pLongNamesCaseIgnore->ulTableSize = ulEntries;
          }
          else
          {
            LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
          } /* endif */
        } /* endif */
      } /* endif */

      // fill pointer array
      if ( usRC == NO_ERROR )
      {
        PTMX_LONGNAME_TABLE_ENTRY pEntry = pLongNames->stTableEntry;
        pLongNames->ulEntries = 0;
        pszTemp = pLongNames->pszBuffer;
        while ( *((PUSHORT)pszTemp) != 0 )
        {

          // get ID of string
          pEntry->usId = *((PUSHORT)pszTemp);
          pszTemp += sizeof(USHORT);    // skip ID

          // set pointer to long document name
          pEntry->pszLongName = pszTemp;

          // continue with next entry
          pEntry++;
          pLongNames->ulEntries++;
          pszTemp += strlen(pszTemp)+1;// skip long name
        } /* endwhile */
      } /* endif */

      // sort long name array
      if ( usRC == NO_ERROR )
      {
        qsort( pLongNames->stTableEntry,
               pLongNames->ulEntries,
               sizeof(TMX_LONGNAME_TABLE_ENTRY),
               NTMLongNameTableComp );
      } /* endif */

      // make copy of long name array for case ignore search
      if ( usRC == NO_ERROR )
      {
        memcpy( pLongNamesCaseIgnore->stTableEntry,
                pLongNames->stTableEntry,
                pLongNames->ulEntries *
                sizeof(TMX_LONGNAME_TABLE_ENTRY) );

        qsort( pLongNamesCaseIgnore->stTableEntry,
               pLongNames->ulEntries,
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
//|Function call:     NTMAddNameToTable( EqfMemory* pTmClb,    //input           |
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
USHORT EqfMemory::NTMAddNameToTable
(
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
  if ( pszName[0] != EOS )
  {
    /******************************************************************/
    /* get pointer to table and table entries in dependency of the    */
    /* table type                                                     */
    /******************************************************************/
    usRc = NTMGetPointersToTable( usTableType,
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
      #ifdef TEMPORARY_COMMENTED
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
                pstTMTableEntries = pTmClb->Languages.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case FILE_KEY :
                pTmClb->FileNames = *pstTMTable;
                pstTMTableEntries = pTmClb->FileNames.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case AUTHOR_KEY :
                pTmClb->Authors = *pstTMTable;
                pstTMTableEntries = pTmClb->Authors.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case TAGTABLE_KEY :
                pTmClb->TagTables = *pstTMTable;
                pstTMTableEntries = pTmClb->TagTables.stTmTableEntry;
                break;
              //---------------------------------------------------------
              case LANGGROUP_KEY :
                pTmClb->LangGroups = *pstTMTable;
                pstTMTableEntries = pTmClb->LangGroups.stTmTableEntry;
                break;
            } /* end switch */
            //pstTMTable->ulAllocSize = ulReallocSize;
          }
          else
          {
            LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
          } /* endif */
        } /* endif */
        
      } /* endif */
      #endif
      //if( (pstTMTable->ulMaxEntries+1) * sizeof(pstTMTableEntries[0]) + sizeof(ULONG) > TMX_REC_SIZE){
      //  T5LOG(T5ERROR) << ":: cannot increase table size, it reached it's limit ; maxEntries="<< pstTMTable->ulMaxEntries << "; tagTableType=" << usTableType;
        //usRc = ERROR_NOT_ENOUGH_MEMORY;
      //}
      const size_t currentCap = pstTMTable->stTmTableEntry.size();
      if(pstTMTable->ulMaxEntries >= currentCap){
        size_t newCap = currentCap + 10;
        if(newCap >= pstTMTable->stTmTableEntry.max_size()){
          T5LOG(T5FATAL) << "tried to set too big capacity for table " <<  usTableType <<"; newCap = " << newCap;
        }
        //T5LOG(T5ERROR)<<"remove this log later: tableType = " << usTableType <<" changing capacity to "<< newCap; 
        pstTMTable->stTmTableEntry.resize(newCap);
        pstTMTableEntries = &pstTMTable->stTmTableEntry[0];
      }

      if ( usRc == NO_ERROR )
      {
        /*************************************************************/
        /* insert name and id to table and sort table                */
        /*************************************************************/
        int pos = pstTMTable->ulMaxEntries;
        *pusID = ++pstTMTable->ulMaxEntries;
        strncpy( pstTMTableEntries[pos].szName,
                pszName, MAX_LANG_LENGTH-1 );
        pstTMTableEntries[pos].usId = *pusID;
        qsort( pstTMTableEntries,
               pstTMTable->ulMaxEntries,
               sizeof(TMX_TABLE_ENTRY),
               NTMCompNames );

        // update table record in TM QDAM file (if not in read-only mode)
        if ( !(usAccessMode & ASD_READONLY) )
        {
          if ( usTableType != LANGGROUP_KEY )
          {
            int occupSize = (pstTMTable->ulMaxEntries+1) * sizeof(TMX_TABLE_ENTRY);
            PBYTE pOldTable = new BYTE[occupSize];
            TMX_TABLE_OLD* table = (TMX_TABLE_OLD*) pOldTable;
            table->ulMaxEntries = pstTMTable->ulMaxEntries;
            if(table->ulMaxEntries){
              memcpy( &table->stTmTableEntry, 
                &pstTMTable->stTmTableEntry[0], sizeof(TMX_TABLE_ENTRY) * table->ulMaxEntries);
            }
            usRc = TmBtree.EQFNTMUpdate(
                                (ULONG)usTableType,
                                //(PBYTE)pstTMTable,
                                (PBYTE)table,
                                //pstTMTable->ulAllocSize 
                                //BTREE_REC_SIZE_V3
                                occupSize
                                );
            delete[] pOldTable;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else  //--- pTmClb is NULL pointer or pszName is empty
  {
    //--- wrong function paramters
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_INTERNAL);
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



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMLoadNameTable                                         |
//+----------------------------------------------------------------------------+
//|Description:       This function loads a TM name table from the database.   |
//|                   If the table is tersed it is uncompressed.               |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::NTMLoadNameTable
(
  ULONG       ulTableKey,              // key of table record
  PTMX_TABLE  pTMTable,                // ptr to table data pointer
  PLONG       plSize                  // ptr to buffer for size of table data
)
{
  USHORT      usRc = NO_ERROR;         // function return code

  // call to obtain exact length of record
  *plSize = 0;
  usRc =  TmBtree.EQFNTMGet( ulTableKey, 0, plSize );
  
  PBYTE pOldTable = nullptr; 
  // read table data
  if ( usRc == NO_ERROR )
  {
    LONG lLen = *plSize;
    pOldTable = new BYTE[lLen + 1];
    usRc =  TmBtree.EQFNTMGet( ulTableKey, (PCHAR)(pOldTable), &lLen );
  } /* endif */

  // handle tersed name tables
  if ( usRc == NO_ERROR )
  {
    PTERSEHEADER pTerseHeader = (PTERSEHEADER)pOldTable;
    PBYTE     pNewArea = NULL;         // ptr to unterse data area

    if ( pTerseHeader->ulMagicWord == TERSEMAGICWORD )
    {
      // table is tersed...

      // allocate buffer for untersed data
      if ( !UtlAlloc( (PVOID *)&pNewArea, 0L,
                      (LONG)pTerseHeader->usDataSize, NOMSG ) )
      {
        LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
      } /* endif */

      // unterse data
      if ( usRc == NO_ERROR )
      {
        LONG lNewLen = 0;

        memcpy( pNewArea, (PBYTE)pOldTable + sizeof(TERSEHEADER),
                *plSize - sizeof(TERSEHEADER) );
       
        T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 51 in NTMLoadNameTable";
#ifdef TEMPORARY_COMMENTED
        if ( !fUtlHuffmanExpand( (PUCHAR)pNewArea, pTerseHeader->usDataSize,
                              &ulNewLen ) )
        {
          LOG_AND_SET_RC(usRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY); // expand failed most likely to
                                          // memory shortage
        } /* endif */
        #endif
      } /* endif */

      // set table data and cleanup
      if ( usRc == NO_ERROR )
      {
        // set size of name table
        *plSize = pTerseHeader->usDataSize;

        // free tersed data area
        //UtlAlloc( (PVOID *)pTMTable, 0L, 0L, NOMSG );

        // anchor untersed data area
        //*pTMTable = pNewArea;

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
    LONG lRest = *plSize;
    PBYTE pbTemp = (PBYTE)pOldTable;
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

  if(usRc == NO_ERROR){
    pTMTable->ulMaxEntries  = ((PTMX_TABLE_OLD)pOldTable)->ulMaxEntries;
    pTMTable->stTmTableEntry.clear();
    pTMTable->stTmTableEntry.resize(pTMTable->ulMaxEntries+1);
    if(pTMTable->ulMaxEntries){
      memcpy(&pTMTable->stTmTableEntry[0], &((PTMX_TABLE_OLD)pOldTable)->stTmTableEntry, pTMTable->ulMaxEntries * sizeof(TMX_TABLE_ENTRY));
    }
  }

  if(pOldTable){
     delete[] pOldTable;
     pOldTable = nullptr;
  }
  // return to caller
  return( usRc );
} /* end of function NTMLoadNameTable */


USHORT EqfMemory::NTMAddLangGroup
(
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
    usRC = NTMGetIDFromName( szLangGroup, NULL, LANGGROUP_KEY, (PUSHORT)&sGroupID );
  } /* endif */

  // enlarge language-ID-to-group-ID table if necessary
  if ( usRC == NO_ERROR )
  {
    LONG lRequiredSize = (sLangID + 1) * sizeof(SHORT);
    if ( lLangIdToGroupTableSize < lRequiredSize )
    {
      lRequiredSize += (10 * sizeof(SHORT));
      if ( UtlAlloc( (PVOID *)&(psLangIdToGroupTable),
                      lLangIdToGroupTableSize,
                      lRequiredSize, NOMSG ) )
      {
        lLangIdToGroupTableSize = lRequiredSize;
        lLangIdToGroupTableUsed = lRequiredSize;
      }
      else
      {
        LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
      } /* endif */
    } /* endif */
  } /* endif */

  // update language ID to group ID table
  if ( usRC == NO_ERROR )
  {
    psLangIdToGroupTable[sLangID] = sGroupID;
  } /* endif */

  return( usRC );
} /* end of function NTMAddLangGroup */

USHORT EqfMemory::NTMCreateLangGroupTable()
{
  USHORT usRC = NO_ERROR;
  //pTmClb->LangGroups.ulAllocSize = TMX_TABLE_SIZE;

  // allocate language-ID-to-group-ID-table
 
  LONG lSize = Languages.ulMaxEntries > 100L ?  Languages.ulMaxEntries : 100L;
  if(lSize <=0) lSize = 1;
  lSize *= sizeof(SHORT);
  if( UtlAlloc( (PVOID *)&(psLangIdToGroupTable),
                0L, lSize, NOMSG ) )
  {
    lLangIdToGroupTableSize = lSize;
    lLangIdToGroupTableUsed = 0L;
  }
  else
  {
    LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  // get group IDs for all languages and fill map table
  if ( usRC == NO_ERROR )
  {
    int i = 0;
    while ( (usRC == NO_ERROR) &&
            (i < (int)Languages.ulMaxEntries) )
    {
      usRC = NTMAddLangGroup( Languages.stTmTableEntry[i].szName,
                              Languages.stTmTableEntry[i].usId );
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
USHORT EqfMemory::NTMOrganizeIndexFile()
{
  USHORT usRC = NO_ERROR;
  usRC = EQFNTMOrganizeIndex( &(InBtree), usAccessMode, START_KEY );

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
    usLength = usLength + (USHORT)((wcslen( pszContext) + 2) * sizeof(CHAR_W));
  } /* endif */     

  if ( (pszAddInfo != NULL) && (*pszAddInfo != 0) )
  {
    // length = characters in add.info + end delimiter + size field + IF field
    usLength = usLength + (USHORT)((wcslen( pszAddInfo) + 2) * sizeof(CHAR_W));
  } /* endif */     

  if ( usLength != 0 )
  {
    // if data is available we need an area end delimiter
    usLength += sizeof(CHAR_W);
  } /* endif */     

  return( usLength );
}


// check if the data is old context data or new additional data
BOOL NTMIsAddData( PSZ_W pData )
{
  BOOL fAddData = FALSE;
  PUSHORT pDataShort = (PUSHORT) pData;
  if ( (*pDataShort == ADDDATA_ADDINFO_ID) || (*pDataShort == ADDDATA_CONTEXT_ID) || (*pDataShort == ADDDATA_ENDOFDATA_ID) )
  {
    fAddData = TRUE;
  } /* endif */     
  return( fAddData );
} /* end of function NTMIsAddData */ 

// find specified data area in additional data, returns ptr to start of area or NULL
PSZ_W NTMFindData( PSZ_W pData, USHORT usDataID )
{
  PUSHORT pDataShort = (PUSHORT) pData;
  while ( (*pDataShort != 0) && (*pDataShort != ADDDATA_ENDOFDATA_ID)  && (*pDataShort != usDataID) )
  {
    USHORT usLen = pDataShort[1];
    pData += (usLen + 1);
    pDataShort = (PUSHORT) pData;
  } /* endwhile */     

  return( (*pDataShort == usDataID) ? pData : NULL );
} /* end of function NTMFindData */ 

// get length of specific data in the combined data area, returns length of data area in number of CHAR_Ws
USHORT NtmGetAddDataLen( PTMX_TARGET_CLB pCLB, USHORT usDataID )
{
  USHORT usLength = 0;

  if ( pCLB->usAddDataLen != 0 )       // do we have additional data ?
  {
    PSZ_W pData = (PSZ_W)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pData ) )
    {
      pData = NTMFindData( pData, usDataID );

      if ( pData != NULL )
      {
        PUSHORT pDataPShort = (PUSHORT) pData;
        usLength = pDataPShort[1];
      } /* endif */         
    }  
  } /* endif */     

  return( usLength );
} /* end of function NtmGetAddDataLen */

// store/combine additional data in the combined area, returns new size of combined data area or 0 in case of errors
USHORT NtmStoreAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszNewData )
{
  PSZ_W pData = (PSZ_W)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));
  PSZ_W pDataStart = pData;
  //PSZ_W pNewDataStr = nullptr;
  PUSHORT pDataShort = (PUSHORT)pData;
  USHORT usNewLength = (USHORT)(wcslen( pszNewData ));

  if ( usNewLength != 0 ) usNewLength += 1;   

  if ( pCLB->usAddDataLen != 0 )       
  {
    // remove any old data for this ID
    PSZ_W pOldData = NTMFindData( pData, usDataID );
    if ( pOldData  != NULL )
    {
      PUSHORT pOldDataPShort = (PUSHORT) pOldData;
      PSZ_W pEndOfData = (PSZ_W)(((PBYTE)pOldData ) + pCLB->usAddDataLen - 2);
      USHORT usOldLen =  pOldDataPShort [1];
      PSZ_W pSource = pOldData + usOldLen + 2;
      PSZ_W pTarget = pOldData ;

      while ( pSource <= pEndOfData )
      {
        *pTarget++ = *pSource++;
      } /* endwhile */         

      pCLB->usAddDataLen = pCLB->usAddDataLen - ((usOldLen + 2)*sizeof(PSZ_W));
    } /* endif */       

    // add new data to end of data area
    if ( usNewLength != 0 )
    {
      PSZ_W pTarget = (PSZ_W)(((PBYTE)pData) + pCLB->usAddDataLen - 1 );
      PUSHORT pTargetPShort = (PUSHORT)pTarget;
      *pTargetPShort++ = usDataID;
      *pTargetPShort++ = usNewLength;
      pTarget++;
      //pNewDataStr = pTarget;
      pszNewData[usNewLength] = 0;
      //wcsncpy( pTarget, pszNewData, usNewLength);
      memcpy( pTarget, pszNewData, sizeof(pData[0])*(usNewLength) );
      pTarget += usNewLength;
      pTargetPShort = (PUSHORT) pTarget;
      *pTargetPShort++ = ADDDATA_ENDOFDATA_ID;
      *pTargetPShort = 0;
      pCLB->usAddDataLen += ((usNewLength + 1 ) * sizeof(pData[0]));
    } /* endif */       
  }
  else if ( usNewLength != 0 )
  {
    // no additional data yet, copy new data
    *pDataShort++ = usDataID;
    *pDataShort = usNewLength;
    pData++;
    pszNewData[usNewLength] = 0;
    //wcsncpy( pData, pszNewData, usNewLength);
    memcpy( pData, pszNewData, sizeof(pData[0])*(usNewLength) );
    //pNewDataStr = pData;
    pData += usNewLength;
    pDataShort = (PUSHORT) pData;
    *pDataShort++ = ADDDATA_ENDOFDATA_ID;
    *pDataShort = 0;
    pCLB->usAddDataLen = (usNewLength + 2) * sizeof(pData[0]);// addinfo_metadana(len+type - 1 wchar) + (usNewLength = strlen + endDelim(1 wchar)) + endAddinfoDelim(1wchar)
  } /* endif */     

  return( pCLB->usAddDataLen );
} /* end of function NtmStoreAddData */

// retrieve specific data from the combined data area, returns length of retrieved data (incl. string end delimiter)
USHORT NtmGetAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszBuffer, USHORT usBufSize )
{
  USHORT usLength = 0;

  if ( pCLB->usAddDataLen != 0 )       // do we have additional data ?
  {
    PSZ_W pData = (PSZ_W)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pData ) )     // is it additional data?
    {
      pData = NTMFindData( pData, usDataID );

      if ( pData != NULL )
      {
        PUSHORT pDataPUShort = (PUSHORT)pData;
        usLength = pDataPUShort[1];
        pData++;
        if ( usLength < usBufSize )
        {
          pszBuffer[0] = 0;
          //wcsncpy(pszBuffer, pData, usLength+1 );
          memcpy(pszBuffer, pData, usLength*sizeof(pData[0]));
          pszBuffer[usLength+1] = 0;
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
      T5LOG(T5WARNING) <<"Add info was not saved properly";      
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
    PSZ_W pData = (PSZ_W)(((PBYTE)pCLB) + sizeof(TMX_TARGET_CLB));

    if ( NTMIsAddData( pData ) )     // is it additional data?
    {
      pData = NTMFindData( pData, usDataID );

      if ( pData != NULL )
      {
        PUSHORT pDataUShort = (PUSHORT) pData;
        usLength = pDataUShort[1];
        if ( usLength > 0 )
        {
          if ( wcsstr( (pData + 1), pszSearch ) != NULL )
          {
            fFound = TRUE;
          } /* endif */             
        } /* endif */           
      } /* endif */         
    }
    else
    {
       T5LOG(T5WARNING) <<"Add info was not saved properly";
    } /* endif */       
  } /* endif */     

  return( fFound );
} /* end of function NtmFindInAddData */
