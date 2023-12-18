//+----------------------------------------------------------------------------+
//|EQFNTEXT.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
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
#include "LogWrapper.h"

static USHORT ExtractRecordV6
(
  EqfMemory*        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  TMX_EXT_OUT_W *  pTmExtOut
);

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXExtract  gets source and targets from tm data file   |
//|                                in sequential order                         |
//+----------------------------------------------------------------------------+
//|Description:       Gets source and targets in sequential order              |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXGet( EqfMemory* pTmClb,        //ptr to ctl block struct  |
//|                         PTMX_EXT_IN pTmExtIn,   //ptr to input struct      |
//|                         PTMX_EXT_OUT pTmExtOut ) //ptr to output struct    |
//+----------------------------------------------------------------------------+
//|Input parameter:   EqfMemory*  pTmClb         control block                   |
//|                   PTMX_EXT_IN pTmExtIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_EXT_OUT pTmExtOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     get tm record                                                          |
//|     go to target record specified in ext in structure                      |
//|     if target record exists                                                |
//|       fill put structure in ext out structure                              |
//|       position at next target                                              |
//|         if target record exists                                            |
//|           fill usNextTarget                                                |
//|           ulTmKey is equal to same key                                     |
//|         else                                                               |
//|           set usNextTarget to 1                                            |
//|           ulTmKey is equal to next key                                     |
//|     else                                                                   |
//|       set usNextTarget to 1                                                |
//|       ulTmKey is equal to next key                                         |
//|                                                                            |
// ----------------------------------------------------------------------------+

USHORT TmtXExtract
(
  EqfMemory* pTmClb,         //ptr to ctl block struct
  PTMX_EXT_IN_W  pTmExtIn, //ptr to input struct
  TMX_EXT_OUT_W * pTmExtOut //ptr to output struct
)
{
  USHORT usRc = NO_ERROR;              //return code
  BOOL fOK;                            //success indicator
  PTMX_RECORD pTmRecord = NULL;        //pointer to tm record
  ULONG  ulRecBufSize = 0;             // current size of record buffer
  ULONG  ulLen = 0;                    //length indicator
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  ULONG ulStartKey;                    //start tm key
  ULONG ulNextKey;                     //last tm key
  BOOL  fSpecialMode = FALSE;          //special mode flag
  ULONG   ulOemCP = 1L;

  //allocate 32K for tm record
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) (2 * TMX_REC_SIZE), NOMSG );
  ulRecBufSize = 2 * TMX_REC_SIZE;
  //allocate pString

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  if ( !usRc )
  {
    SHORT sRetries = MAX_RETRY_COUNT;
    {
      LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);                 // reset return code
      /********************************************************************/
      /* Set special mode flag                                            */
      /********************************************************************/
      if ( !usRc )
      {
        if ( (pTmExtIn->usConvert == MEM_OUTPUT_TAGTABLES) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_AUTHORS)   ||
             (pTmExtIn->usConvert == MEM_OUTPUT_LONGNAMES) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_ALLDOCS)   ||
             (pTmExtIn->usConvert == MEM_OUTPUT_DOCUMENTS) ||
             (pTmExtIn->usConvert == MEM_OUTPUT_LANGUAGES) )
        {
          fSpecialMode = TRUE;
        }
        else
        {
          fSpecialMode = FALSE;
        } /* endif */
      } /* endif */

      /********************************************************************/
      /* Special mode to get list of tag tables, authors, documents or    */
      /* languages                                                        */
      /********************************************************************/
      if ( !usRc && fSpecialMode)
      {
        PTMX_TABLE pTable = NULL;          // table containing the requested info
        ULONG     ulMaxEntries = 0;        // last + 1 entry in table

        /******************************************************************/
        /* Address table containg the requested names                     */
        /******************************************************************/
        switch ( pTmExtIn->usConvert )
        {
          case MEM_OUTPUT_TAGTABLES :
            pTable = &pTmClb->TagTables;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_AUTHORS   :
            pTable = &pTmClb->Authors;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_DOCUMENTS :
            pTable = &pTmClb->FileNames;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_LANGUAGES :
            pTable = &pTmClb->Languages;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          case MEM_OUTPUT_LONGNAMES :
            // different handling for long names, no setting of pTable
            ulMaxEntries = pTmClb->pLongNames->ulEntries;
            break;
          case MEM_OUTPUT_ALLDOCS :
            // different handling for long names, no setting of pTable
            pTable = &pTmClb->FileNames;
            ulMaxEntries = pTable->ulMaxEntries;
            break;
          default :
            LOG_AND_SET_RC(usRc, T5INFO, BTREE_INVALID);
            break;
        } /* endswitch */

        /******************************************************************/
        /* Extract requested names                                        */
        /******************************************************************/
        if ( !usRc )
        {
          PTMX_TABLE_ENTRY pEntry;         // ptr to current entry
          PSZ_W   pszBuffer;               // ptr to next free space in buffer
          LONG    lLen;                    // lenght of current name
          LONG    lRoomLeft;               // space left in current buffer
          int     iteration;               // iteration counter
          PSZ     pszName;                 // ptr to current name

          /****************************************************************/
          /* Initialize our output buffers                                */
          /****************************************************************/
          memset( pTmExtOut->szSource, NULC,
                  sizeof(pTmExtOut->szSource) );
          memset( pTmExtOut->szTarget, NULC,
                  sizeof(pTmExtOut->szTarget) );

          /****************************************************************/
          /* Loop two times: first time to fill szSource, the second      */
          /* time to fill the szTarget buffer                             */
          /****************************************************************/
          iteration = 0;
          do
          {
            // set buffer and lRoomLeft variables
            if ( iteration == 0 )
            {
              // use szSource buffer
              pszBuffer = pTmExtOut->szSource;
              // leave room for NULC and ASCII expansion
              lRoomLeft = (sizeof(pTmExtOut->szSource)/sizeof(CHAR_W))/2 - 1;
            }
            else
            {
              // use szTarget buffer
              pszBuffer = pTmExtOut->szTarget;
              // leave room for NULC and ASCII expansion
              lRoomLeft = (sizeof(pTmExtOut->szTarget)/sizeof(CHAR_W))/2 - 1;
            } /* endif */

            while ( lRoomLeft && (pTmExtIn->usNextTarget < ulMaxEntries) )
            {
              if ( pTmExtIn->usConvert == MEM_OUTPUT_LONGNAMES )
              {
                pszName = pTmClb->pLongNames->
                            stTableEntry[pTmExtIn->usNextTarget].pszLongName;
              }
              else
              {
                pEntry = &pTable->stTmTableEntry[pTmExtIn->usNextTarget];
                pszName = pEntry->szName;
                // check if there is a long name for the current document
                if ( pTmExtIn->usConvert == MEM_OUTPUT_ALLDOCS )
                {
                  ULONG ulEntry = 0;
                  while ( (ulEntry < pTmClb->pLongNames->ulEntries) &&
                          (pEntry->usId != pTmClb->pLongNames->
                                           stTableEntry[ulEntry].usId ) )
                  {
                    ulEntry++;
                  } /* endwhile */
                  if ( ulEntry < pTmClb->pLongNames->ulEntries )
                  {
                    pszName = pTmClb->pLongNames->stTableEntry[ulEntry].pszLongName;
                  } /* endif */
                } /* endif */
              } /* endif */
              lLen = strlen(pszName);

              if ( lRoomLeft > (lLen + 1))
              {
                ASCII2Unicode( pszName, pszBuffer, ulOemCP );
                pszBuffer += lLen;
                *pszBuffer++ = X15;
                lRoomLeft -= lLen + 1;
                pTmExtIn->usNextTarget++;
              }
              else
              {
                lRoomLeft = 0;
              } /* endif */
            } /* endwhile */

            *pszBuffer = NULC;             // terminate current buffer
            iteration++;                   // continue with next buffer
          } while ( (iteration <= 1) && (pTmExtIn->usNextTarget < ulMaxEntries) ); /* enddo */

          /****************************************************************/
          /* Set flags in output structure                                */
          /****************************************************************/
          if ( pTmExtIn->usNextTarget < ulMaxEntries )
          {
            // more names available for extract
            pTmExtOut->usNextTarget = pTmExtIn->usNextTarget;
            pTmExtOut->usTranslationFlag = TRANSLFLAG_MACHINE;
          }
          else
          {
            // all names have been extracted
            pTmExtOut->usNextTarget = 0;
            pTmExtOut->usTranslationFlag = TRANSLFLAG_NORMAL;
          } /* endif */
        } /* endif */
      } /* endif */

      /********************************************************************/
      /* Normal mode of TMExtract                                         */
      /********************************************************************/
      if ( !usRc && !fSpecialMode )
      {
        pTmClb->TmBtree.EQFNTMGetNextNumber(  &ulStartKey, &ulNextKey);
        pTmExtOut->ulMaxEntries = (ulNextKey - ulStartKey);
        /******************************************************************/
        /* return one matching entry (if any available)                   */
        /******************************************************************/
        LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
        while ( (pTmExtIn->ulTmKey < ulNextKey) && (usRc == BTREE_NOT_FOUND) )
        {
          ulLen = ulRecBufSize;
          usRc =  pTmClb->TmBtree.EQFNTMGet(
                            pTmExtIn->ulTmKey,
                            (PCHAR)pTmRecord,
                            &ulLen );

          // re-alloc buffer and try again if buffer overflow occured
          if ( usRc == BTREE_BUFFER_SMALL )
          {
            fOK = UtlAlloc( (PVOID *)&(pTmRecord), ulRecBufSize, ulLen, NOMSG );
            if ( fOK )
            {
              ulRecBufSize = ulLen;

              usRc =  pTmClb->TmBtree.EQFNTMGet(
                                pTmExtIn->ulTmKey,
                                (PCHAR)pTmRecord,
                                &ulLen );
            }
            else
            {
              LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
            } /* endif */
          } /* endif */

          if ( usRc == NO_ERROR )
          {
            usRc = ExtractRecordV6( pTmClb, pTmRecord, pTmExtIn, pTmExtOut );            
          }
          /****************************************************************/
          /* setup new starting point (do this even in the case we are    */
          /* dealing with a corrupted TM )                                */
          /****************************************************************/
          if ( usRc != NO_ERROR )
          {
            if ( (usRc == BTREE_NOT_FOUND) || ( usRc == BTREE_CORRUPTED ) || ((pTmClb->usAccessMode & ASD_ORGANIZE) != 0) )
            {
              pTmExtIn->ulTmKey ++;
              pTmExtIn->usNextTarget = 1;
              pTmExtOut->ulTmKey = pTmExtIn->ulTmKey;
              pTmExtOut->usNextTarget = pTmExtIn->usNextTarget;
              usRc = BTREE_NOT_FOUND;     // reset error condition
            } /* endif */
          } /* endif */
        } /* endwhile */

        /******************************************************************/
        /* set end of file condition ...                                  */
        /******************************************************************/
        if ( (usRc == BTREE_NOT_FOUND) &&  (pTmExtIn->ulTmKey == ulNextKey) )
        {
          //arrived at last tm record
          LOG_AND_SET_RC(usRc, T5INFO, BTREE_EOF_REACHED);
        } /* endif */
      } /* endif */
    }
  } /* endif */

  pTmExtOut->stPrefixOut.usLengthOutput = sizeof( TMX_EXT_OUT_W );
  pTmExtOut->stPrefixOut.usTmtXRc = usRc;

  //release memory
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );

  // in organize mode only: change any error code (except BTREE_EOF_REACHED) to BTREE_CORRUPTED
  if ( (usRc != NO_ERROR ) && (usRc != BTREE_EOF_REACHED) && ((pTmClb->usAccessMode & ASD_ORGANIZE) != 0) )
  {
    LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
  } /* endif */

  return( usRc );
}

static
USHORT ExtractRecordV6
(
  EqfMemory*        pTmClb,         //ptr to ctl block struct
  PTMX_RECORD     pTmRecord,
  PTMX_EXT_IN_W   pTmExtIn,
  TMX_EXT_OUT_W *  pTmExtOut
)
{
  USHORT usRc = 0;
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_TARGET_CLB    pTargetClb;       //ptr to target CLB
  LONG       lLeftClbLen = 0;        // remaining length of CLB area
  PBYTE pByte;                         //position ptr
  LONG lSourceLen = 0;              //length of source string
  USHORT usTarget;                     //nr of target records in tm record
  PSZ_W pSourceString = NULL;          //pointer to source string
  PBYTE pSource;                       //position ptr


  UtlAlloc( (PVOID *) &(pSourceString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  if ( !pSourceString )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */


  if ( !usRc )
  {
    //check if record is empty
    if ( ( RECLEN(pTmRecord) != 0) &&
         (pTmRecord->usFirstTargetRecord < RECLEN(pTmRecord)) )
    {
      LONG   lLeftTgtLen;            // remaining target length
      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/
      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );
      lLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;
      //extract source string
      //position at beginning of source structure in tm record
      pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

      //move pointer to corresponding position
      pSource = (PBYTE)(pTmRecord+1);
      pSource += pTMXSourceRecord->usSource;

      //calculate length of source string
      lSourceLen = (RECLEN(pTMXSourceRecord) -
                            sizeof(TMX_SOURCE_RECORD) );

      //copy source string for fill matchtable
      if ( lSourceLen < MAX_SEGMENT_SIZE * sizeof(WCHAR) )
      {
        lSourceLen = EQFCompress2Unicode( pSourceString, pSource, lSourceLen );
        if ( lSourceLen < MAX_SEGMENT_SIZE && lSourceLen >=0 ) {
////      memcpy( pSourceString, pSource, ulSourceLen );
          pSourceString[lSourceLen] = EOS;
        } 
        else
        {
           LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
        }
      }
      else
      {
        LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
      } /* endif */

      if ( !usRc )
      {
        //find target record specified in ExtIn structure

        //move pointer to first target
        pByte = (PBYTE)(pTmRecord);
        pByte += pTmRecord->usFirstTargetRecord;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
        if ( pTMXTargetRecord->usClb >= RECLEN(pTMXTargetRecord) )
        {
          // target record is corrupted, continue with next TM record
          LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);
        }
        else
        {
          lLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;
          lLeftClbLen -= TARGETCLBLEN(pTargetClb); // subtract size of current CLB
        } /* endif */
        usTarget = 1;           //initialize counter

        //loop until correct target is found
        while ( (usTarget < pTmExtIn->usNextTarget) && (lLeftTgtLen > 0) && !usRc )
        {
          // position to first target CLB
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
          pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
          lLeftClbLen = RECLEN(pTMXTargetRecord) -
                         pTMXTargetRecord->usClb;

          // subtract size of current CLB
          if ( lLeftClbLen >= TARGETCLBLEN(pTargetClb) )
          {
            lLeftClbLen -= TARGETCLBLEN(pTargetClb); 
          }
          else
          {
            // database is corrupted
            lLeftClbLen = 0;
            LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
          } /* endif */ 

          // loop over all target CLBs
          while ( (usTarget < pTmExtIn->usNextTarget) && ( lLeftClbLen > 0 ) )
          {
            usTarget++;
            pTargetClb = NEXTTARGETCLB(pTargetClb);
            lLeftClbLen -= TARGETCLBLEN(pTargetClb);
          } /* endwhile */

          // continue with next target if no match yet
          if ( usTarget < pTmExtIn->usNextTarget )
          {
            // position at next target
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            //move pointer to end of target
            if (lLeftTgtLen >= RECLEN(pTMXTargetRecord))
            {
              lLeftTgtLen -= RECLEN(pTMXTargetRecord);
            }
            else
            {
              lLeftTgtLen = 0;
            } /* endif */
            pByte += RECLEN(pTMXTargetRecord);
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pTargetClb = (PTMX_TARGET_CLB)(pByte + pTMXTargetRecord->usClb);
            lLeftClbLen = RECLEN(pTMXTargetRecord) -
                           pTMXTargetRecord->usClb;
            // subtract size of current CLB
            if ( lLeftClbLen >= TARGETCLBLEN(pTargetClb) )
            {
              lLeftClbLen -= TARGETCLBLEN(pTargetClb); 
            }
            else
            {
              // database is corrupted
              lLeftClbLen = 0;
              LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
            } /* endif */ 
            usTarget++;
          } /* endif */
        } /* endwhile */

        if ( !usRc && (usTarget == pTmExtIn->usNextTarget) )
        {
          //position at start of target record
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //if target record exists
          if ( ( lLeftTgtLen > 0) && ( RECLEN(pTMXTargetRecord) != 0) )
          {
            //fill out the put structure as output of the extract function
            usRc = FillExtStructure( pTmClb, pTMXTargetRecord,
                                     pTargetClb,
                                     pSourceString, &lSourceLen,
                                     pTmExtOut );
            pTmClb->NTMGetNameFromID( &pTMXSourceRecord->usLangId, (USHORT)LANG_KEY,
                        pTmExtOut->szOriginalSourceLanguage, NULL );
            if ( ! usRc )
            {
              //check for another target
              if (lLeftTgtLen >= RECLEN(pTMXTargetRecord))
              {
                lLeftTgtLen -= RECLEN(pTMXTargetRecord);
              }
              else
              {
                lLeftTgtLen = 0;
              } /* endif */
              pByte += RECLEN(pTMXTargetRecord);
              pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

              //if target exists
              if ( ((RECLEN(pTMXTargetRecord) > 0) && (lLeftTgtLen > 0 ) )
                   //       ||
                   // (lLeftClbLen > 0) 
                  )
              {
                //increase target count and leave tm record key number as before
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey;
                pTmExtOut->usNextTarget = pTmExtIn->usNextTarget + 1;
              }
              else
              {
                //no more target so get next tm record and initialize target count
                pTmExtOut->ulTmKey = pTmExtIn->ulTmKey +1;
                pTmExtOut->usNextTarget = 1;
              } /* endif */
            } /* endif */
          }
          else
          {
            //no more target so get next tm record and initialize target count
            LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
          } /* endif */
        }
        else
        {
          //no more target so get next tm record and initialize target count
          LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
        } /* endif */
      } /* endif */
    }
    else
    {
      //if record is empty, get next tm record and initialize target count
      LOG_AND_SET_RC(usRc, T5INFO, BTREE_NOT_FOUND);
    } /* endif */

  }

  // release memory
  UtlAlloc( (PVOID *) &pSourceString, 0L, 0L, NOMSG );


  return usRc;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FillExtStructure fills extract output structure with     |
//|                                    sequential source and target            |
//+----------------------------------------------------------------------------+
//|Description:       Fills the ext structure in sequential order              |
//+----------------------------------------------------------------------------+
//|Function call:  FillExtStructure( EqfMemory* pTmClb, //ptr to control block   |
//|                         PTMX_TARGET_RECORD pTMXTargetRecord,               |
//|                                             //ptr to target record         |
//|                         PSZ pSourceString,  //ptr to source string         |
//|                         PUSHORT pulSourceLen, //source string length       |
//|                         PTMX_EXT pstExt ) //ptr to extract structure       |
//+----------------------------------------------------------------------------+
//|Input parameter:   EqfMemory* pTmClb                                          |
//|                   PTMX_TARGET_RECORD pTMXTargetRecord                      |
//|                   PSZ pSourceString                                        |
//|                   PUSHORT pulSourceLen                                     |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_EXT pstExt                                          |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     position at start of target record                                     |
//|       fill extract stucture                                                |
//|     output extract structure                                               |
// ----------------------------------------------------------------------------+


USHORT FillExtStructure
(
  EqfMemory*    pTmClb,                  //ptr to control block
  PTMX_TARGET_RECORD pTMXTargetRecord, //ptr to tm target
  PTMX_TARGET_CLB    pTargetClb,       // ptr to current target CLB
  PSZ_W pSourceString,                 //ptr to source string
  PLONG plSourceLen,                   //length of source string
  PTMX_EXT_OUT_W pstExt                    //extout ext struct
)
{
  PTMX_TARGET_CLB pTMXTargetClb = NULL;      //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXTagTableRecord = NULL;   //ptr to tag table record
  PBYTE pByte;                               //position pointer
  BOOL fOK;                                  //success indicator
  USHORT usRc = NO_ERROR;                    //return code
  LONG  lTargetLen = 0;                      //length indicator
  PSZ_W pTargetString = NULL;                //pointer to target string

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pTargetString), 0L, (LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    pByte = (PBYTE)pTMXTargetRecord;
    pByte += pTMXTargetRecord->usSourceTagTable;
    pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

    if(*plSourceLen > 0)
    {
      //else copy string
      memcpy( pstExt->szSource, pSourceString, *plSourceLen * sizeof(CHAR_W) );
      pstExt->szSource[*plSourceLen] = EOS;
    } /* endif */

    if ( fOK )
    {
      //position at target string
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTarget;

      //calculate length of target string
      lTargetLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;
      fOK = ( lTargetLen < MAX_SEGMENT_SIZE * sizeof(WCHAR) );
    } /* endif */

    // GQ 2008/06/18 Check if target string data is valid
    if ( fOK )
    {
      BYTE b = *pByte;
      
      if ( (b != 0) && (b != BOCU_COMPRESS) )
      {
        fOK = FALSE; // invalid compression ID
      } /* endif */
    } /* endif */

    if ( fOK )
    {
      //copy target string
        lTargetLen = EQFCompress2Unicode( pTargetString, pByte, lTargetLen );
        if ( lTargetLen < MAX_SEGMENT_SIZE ){
          if( lTargetLen >= 0 )
            pTargetString[lTargetLen] = EOS;
        } else {
           fOK = FALSE; // invalid string length
        }

      //position at target tag record
      pByte = (PBYTE)pTMXTargetRecord;
      pByte += pTMXTargetRecord->usTargetTagTable;
      pTMXTagTableRecord = (PTMX_TAGTABLE_RECORD)pByte;

      //fill in the tag table name
      strcpy(pstExt->szTagTable, "OTMXUXLF");

      
      if(lTargetLen >= 0){
        memcpy( pstExt->szTarget, pTargetString, lTargetLen * sizeof(CHAR_W));
        pstExt->szTarget[lTargetLen] = EOS;
      }
      /* endif */

      //position at target control block
      pTMXTargetClb = pTargetClb;

      // in organize mode preset author and file name field with default
      // values as here the corresponding tables may be corrupted
      if ( pTmClb->usAccessMode & ASD_ORGANIZE )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
        pstExt->szLongName[0] = EOS;
        strcpy( pstExt->szAuthorName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target file name
      pTmClb->NTMGetNameFromID( &pTMXTargetClb->usFileId, (USHORT)FILE_KEY,
                        pstExt->szFileName, pstExt->szLongName );
      //use overflow name if no document name available
      if ( pstExt->szFileName[0] == EOS )
      {
        strcpy( pstExt->szFileName, OVERFLOW_NAME );
      } /* endif */

      //fill in the target author
      pTmClb->NTMGetNameFromID( &pTMXTargetClb->usAuthorId, (USHORT)AUTHOR_KEY,
                        pstExt->szAuthorName, NULL );

      //fill in the target language
      pTmClb->NTMGetNameFromID( &pTMXTargetClb->usLangId, (USHORT)LANG_KEY,
                        pstExt->szTargetLanguage, NULL );
      

      //fill in the segment id
      pstExt->ulSourceSegmentId = pTMXTargetClb->ulSegmId;
      //state whether machine translation
      pstExt->usTranslationFlag = pTMXTargetClb->bTranslationFlag ;
      //fill in target time
      pstExt->lTargetTime = pTMXTargetClb->lTime;

      // fill in any segment context info
      pstExt->szContext[0] = 0;
      pstExt->szAddInfo[0] = 0;
      if ( pTMXTargetClb->usAddDataLen >= MAX_ADD_DATA_LEN )
      {
        // target CLB info seems to be corrupted
        fOK = FALSE;
      }
      else if ( pTMXTargetClb->usAddDataLen )
      {
        NtmGetAddData( pTMXTargetClb, ADDDATA_CONTEXT_ID, pstExt->szContext, sizeof(pstExt->szContext) / sizeof(CHAR_W) );
        NtmGetAddData( pTMXTargetClb, ADDDATA_ADDINFO_ID, pstExt->szAddInfo, sizeof(pstExt->szAddInfo) / sizeof(CHAR_W) );
      } /* endif */
    } /* endif */
    if(fOK){
      LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);
    }else{
      LOG_AND_SET_RC(usRc, T5INFO, BTREE_CORRUPTED);

    }
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pTargetString, 0L, 0L, NOMSG );

  return( usRc );
}
