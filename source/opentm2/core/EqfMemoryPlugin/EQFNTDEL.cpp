//+----------------------------------------------------------------------------+
//|EQFNTDEL.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
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
#include "./LogWrapper.h"
#include "./Property.h"
#include "./EncodingHelper.h"


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXDelSegm   deletes target translation in tm record    |
//+----------------------------------------------------------------------------+
//|Description:       Deletes target translation in tm record if untranslate   |
//|                   segment is initiated in translation environment          |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXDelSegm( EqfMemory* pTmClb,    //ptr to ctl block struct  |
//|                            PTMX_PUT_IN pTmDelIn,  //ptr to input struct    |
//|                            PTMX_PUT_OUT pTmDelOut ) //ptr to output struct |
//+----------------------------------------------------------------------------+
//|Input parameter:   EqfMemory*  pTmClb         control block                   |
//|                   PTMX_PUT_IN pTmDelIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter:  PTMX_PUT_OUT pTmDelOut   output structure                |
//+----------------------------------------------------------------------------+
//|Returncode type:  USHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:  Identical to return code in out structure                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|     tokenize source string in del in structure                             |
//|     build hashes and triple hashes                                         |
//|     check if hashes on in compact area                                     |
//|     if all hashes are on                                                   |
//|      find tm keys for found hashes                                         |
//|      loop through the tm keys                                              |
//|        get tm record                                                       |
//|        find and delete target record readjusting tm record length          |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::TmtXDelSegm
(
  OtmProposal& TmDelIn,  //ptr to input struct
  TMX_EXT_OUT_W* pTmDelOut //ptr to output struct
)
{
  //TMX_SENTENCE Sentence(std::make_unique<StringTagVariants>(pTmDelIn->stTmPut.szSource));    // ptr to sentence structure
  ULONG ulKey;                         // tm record key
  BOOL fOK;                            // success indicator
  USHORT usRc = NO_ERROR;              // return code
  USHORT usMatchesFound = 0;           // compact area hits
  ULONG  ulLen = 0;                    // length indication
  PTMX_RECORD pTmRecord = NULL;        // pointer to tm record
  CHAR szString[MAX_EQF_PATH];         // character string
  PULONG pulSids = NULL;               // ptr to sentence ids
  PULONG pulSidStart = NULL;           // ptr to sentence ids
  ULONG ulRecBufSize = TMX_REC_SIZE;   // current size of record buffer

  //allocate pSentence
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pulSids), 0L, (LONG)(MAX_INDEX_LEN * sizeof(ULONG)),
                    NOMSG );
    if ( fOK )
      pulSidStart = pulSids;
  } /* endif */

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  if ( !usRc )
  {
    //build tag table path
    Properties::GetInstance()->get_value(KEY_OTM_DIR, szString, MAX_EQF_PATH);
    strcat (szString, "/TABLE/");
    strcat( szString, TmDelIn.szMarkup );
    strcat( szString, EXT_OF_FORMAT );

    //remember start of norm string - why?
    //Sentence.pNormStringStart = Sentence.pStrings->getNormStrC();


    //tokenize source segment, resuting in normalized string and tag table record
    usRc = TokenizeSource( TmDelIn.pInputSentence, szString,
                           TmDelIn.szSourceLanguage);

    // set the tag table ID in the tag record (this can't be done in TokenizeSource anymore)
    TmDelIn.pInputSentence->pTagRecord->usTagTableId = 1;
  }


  // update TM databse
  if ( !usRc )
  {
    //set pNormString to beginning of string
    //Sentence.pNormString = Sentence.pStrings->getNormStrC();
    HashSentence( TmDelIn.pInputSentence );
  } /* endif */


  // update TM databse
  if ( !usRc )
  {
    usMatchesFound = CheckCompactArea( TmDelIn.pInputSentence, this );
    if ( usMatchesFound == TmDelIn.pInputSentence->usActVote ) //all hash triples found
    {
      usRc = DetermineTmRecord( this, TmDelIn.pInputSentence, pulSids );
      if ( !usRc )
      {
        while ( *pulSids )
        {
          ulKey = *pulSids;
          ulLen = TMX_REC_SIZE;
          usRc =  TmBtree.EQFNTMGet(
                            ulKey,  //tm record key
                            (PCHAR)pTmRecord,   //pointer to tm record data
                            &ulLen );  //length
          // re-alloc buffer and try again if buffer overflow occured
          if ( usRc == BTREE_BUFFER_SMALL )
          {
            fOK = UtlAlloc( (PVOID *)&(pTmRecord), ulRecBufSize, ulLen, NOMSG );
            if ( fOK )
            {
              ulRecBufSize = ulLen;

              usRc =  TmBtree.EQFNTMGet(
                                ulKey,
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
            //find target record and delete, if the target record was the
            //only target in the tm record, delete the entire record
            usRc = FindTargetAndDelete( pTmRecord, TmDelIn, pTmDelOut, &ulKey );
            if ( usRc == SEG_NOT_FOUND )
            {
              //get next tm record
              pulSids++;
            }
            else
            {
              //target record was found and deleted or an error occured
              //so don't try other sids
              *pulSids = 0;
            } /* endif */
          } /* endif */
        } /* endwhile */
      } /* endif */
    }
    else
    {
      LOG_AND_SET_RC(usRc, T5INFO, SEG_NOT_FOUND);
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &pulSidStart, 0L, 0L, NOMSG );

  pTmDelOut->stPrefixOut.usLengthOutput = sizeof( TMX_EXT_OUT_W );
  pTmDelOut->stPrefixOut.usTmtXRc = usRc;
  return( usRc );
}


USHORT EqfMemory::TmtXDelSegmByKey
(
  OtmProposal& TmDelIn,  //ptr to input struct
  TMX_EXT_OUT_W * pTmDelOut //ptr to output struct
)
{
  PTMX_SENTENCE pSentence = NULL;    // ptr to sentence structure
  ULONG ulKey=TmDelIn.recordKey;                         // tm record key
  BOOL fOK;                            // success indicator
  USHORT usRc = NO_ERROR;              // return code
  USHORT usMatchesFound = 0;           // compact area hits
  ULONG  ulLen = 0;                    // length indication
  PTMX_RECORD pTmRecord = NULL;        // pointer to tm record
  CHAR szString[MAX_EQF_PATH];         // character string
  //PULONG pulSids = NULL;               // ptr to sentence ids
  //PULONG pulSidStart = NULL;           // ptr to sentence ids
  ULONG ulRecBufSize = TMX_REC_SIZE;   // current size of record buffer

  //allocate pSentence
  fOK = UtlAlloc( (PVOID *) &(pTmRecord), 0L, (LONG) TMX_REC_SIZE, NOMSG );

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  // update TM databse
  if ( !usRc )
  {
    ulLen = TMX_REC_SIZE;
    usRc =  TmBtree.EQFNTMGet(
                      ulKey,  //tm record key
                      (PCHAR)pTmRecord,   //pointer to tm record data
                      &ulLen );  //length
    // re-alloc buffer and try again if buffer overflow occured
    if ( usRc == BTREE_BUFFER_SMALL )
    {
      fOK = UtlAlloc( (PVOID *)&(pTmRecord), ulRecBufSize, ulLen, NOMSG );
      if ( fOK )
      {
        ulRecBufSize = ulLen;

        usRc =  TmBtree.EQFNTMGet(
                          ulKey,
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
      
      //find target record and delete, if the target record was the
      //only target in the tm record, delete the entire record
      usRc = FindTargetByKeyAndDelete( pTmRecord,
                          TmDelIn, pSentence, pTmDelOut, &ulKey );
    } /* endif */
  } /* endif */

  //release memory
  
  UtlAlloc( (PVOID *) &pTmRecord, 0L, 0L, NOMSG );

  pTmDelOut->stPrefixOut.usLengthOutput = sizeof( TMX_EXT_OUT_W );
  pTmDelOut->stPrefixOut.usTmtXRc = usRc;
  return( usRc );
}
//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindTargetAndDelete delete target record if it matches   |
//|                                       input data                           |
//+----------------------------------------------------------------------------+
//|Description:       Deletes a target record - function triggered in          |
//|                   translation environment when untranslate segment is      |
//|                   done                                                     |
//+----------------------------------------------------------------------------+
//|Function call:  FindTargetAnddelete( EqfMemory* pTmClb, //ptr to ctl block    |
//|                            PTMX_RECORD pTmRecord, //ptr to tm record       |
//|                            PTMX_PUT_IN pTmDelIn->stTmPut, //input structure|
//|                            PTMX_SENTENCE pSentence, //ptr to sent structure|
//|                            PULONG pulKey ) //tm key                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   EqfMemory*  pTmClb         control block                   |
//|                   PTMX_RECORD pTmRecord    tm record                       |
//|                   PTMX_PUT_IN pTmDelIn     input structure                 |
//|                   PTMX_SENTENCE pSentence  sentence structure              |
//|                   PULONG pulKey            tm key                          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: NO_ERROR                                                       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|    if source strings equal                                                 |
//|     loop through all target records in tm record                           |
//|       if target langs equal                                                |
//|         if source string tag table records are equal                       |
//|           if target strings are equal                                      |
//|             if target tag table records are equal                          |
//|               if segment id, file name and mt flag are equal               |
//|                 if multiple flag not on                                    |
//|                   delete target record and readjust total length of tm     |
//|                   record                                                   |
//|                   leave target record loop                                 |
//|                 else                                                       |
//|                   don't delete                                             |
//|                   leave target record loop                                 |
//|               else                                                         |
//|                 next target record                                         |
//|             else                                                           |
//|               next target record                                           |
//|           else                                                             |
//|             next target record                                             |
//|         else                                                               |
//|           next target record                                               |
//|       else                                                                 |
//|         next target record                                                 |
//|    else                                                                    |
//|      get next sentence key                                                 |
// ----------------------------------------------------------------------------+
USHORT EqfMemory::FindTargetAndDelete(
                            PTMX_RECORD pTmRecord,
                            OtmProposal&  TmDel,
                            TMX_EXT_OUT_W * pTmExtOut,
                            PULONG pulKey )
{
  BOOL fOK = FALSE;                    //success indicator
  BOOL fStop = FALSE;                  //indicates whether to leave loop or not
  PBYTE pByte;                         //position ptr
  PBYTE pStartTarget;                  //position ptr
  PTMX_SOURCE_RECORD pTMXSourceRecord = NULL; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord = NULL; //ptr to target record
  PTMX_TARGET_CLB    pClb = NULL;    //ptr to target control block
  PTMX_TAGTABLE_RECORD pTMXSourceTagTable = NULL; //ptr to source tag info
  PTMX_TAGTABLE_RECORD pTMXTargetTagTable = NULL; //ptr to tag info
  PTMX_TAGTABLE_RECORD pTagRecord = NULL;  //ptr to tag info
  LONG lTagAlloc;                      //allocate length
  ULONG ulLen = 0;                    //length indicator
  LONG lSrcLen = 0;
  PSZ_W  pString = NULL;               //pointer to character string
  USHORT usId = 0;                     //returned id from function
  USHORT usRc = NO_ERROR;              //returned value from function
  USHORT usTarget = 0;           //initialize counter

  //allocate pString
  fOK = UtlAlloc( (PVOID *) &(pString), 0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), NOMSG );

  //allocate 4k for pTagRecord
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &(pTagRecord), 0L, (LONG) TOK_SIZE, NOMSG );
    if ( fOK )
     lTagAlloc = (LONG)TOK_SIZE;
  }

  if ( !fOK )
  {
    LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
  }
  else
  {
    //position at beginning of source structure in tm record
    pTMXSourceRecord = (PTMX_SOURCE_RECORD)(pTmRecord+1);

    //move pointer to source string
    pByte = (PBYTE)(pTmRecord+1);
    pByte += pTMXSourceRecord->usSource;

    //calculate length of source string
    ulLen = (RECLEN(pTMXSourceRecord) - sizeof( TMX_SOURCE_RECORD ));

    //clear pString for later use
    memset( pString, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W) );

    //copy source string for later compare function
//    memcpy( pString, pByte, ulLen );
    ulLen = EQFCompress2Unicode( pString, pByte, ulLen );
    lSrcLen = ulLen;

    //compare source strings
    //if ( !UTF16strcmp( pString, pSentence->pNormString ) )
    if ( !UTF16strcmp( pString, TmDel.pInputSentence->pStrings->getGenericTagStrC() ) )
    {
      ULONG   ulLeftTgtLen;            // remaining target length
      /****************************************************************/
      /* get length of target block to work with                      */
      /****************************************************************/
      assert( RECLEN(pTmRecord) >= pTmRecord->usFirstTargetRecord );
      ulLeftTgtLen = RECLEN(pTmRecord) - pTmRecord->usFirstTargetRecord;

      //source strings equal
      //position at first target record
      pByte = (PBYTE)(pTmRecord+1);
      pByte += RECLEN(pTMXSourceRecord);
      pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      pStartTarget = (PBYTE)pTMXTargetRecord;

      //source strings are identical so loop through target records
      while ( ulLeftTgtLen && ( RECLEN(pTMXTargetRecord) != 0) && !fStop )
      {
        usTarget++;
        /**************************************************************/
        /* update left target length                                  */
        /**************************************************************/
        assert( ulLeftTgtLen >= RECLEN(pTMXTargetRecord) );
        ulLeftTgtLen -= RECLEN(pTMXTargetRecord);

        //next check the target language
        //position at target control block
        pByte += pTMXTargetRecord->usClb;
        pClb = (PTMX_TARGET_CLB)pByte;

        //get id of target language in the put structure
        usRc = NTMGetIDFromName( TmDel.szTargetLanguage,
                                 NULL,
                                 (USHORT)LANG_KEY, &usId );
        //compare target language ids
        if ( (pClb->usLangId == usId) && !usRc )
        {
          //compare source tag table records
          //position at source tag table record
          pByte = pStartTarget;
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //position at source tag table
          pByte += pTMXTargetRecord->usSourceTagTable;
          pTMXSourceTagTable = (PTMX_TAGTABLE_RECORD)pByte;

          //compare tag table records
          if(//UtlCompIgnWhiteSpaceW(pSentence->pInputString, (wchar_t*)pStringWNormalizedTags.c_str(), pStringWNormalizedTags.size() ) == 0
          true) // we use the same tag tables in t5?
          {
            //source tag tables are identical
            pByte = pStartTarget;
            pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
            pByte += pTMXTargetRecord->usTarget;

            //calculate length of target string
            ulLen = pTMXTargetRecord->usClb - pTMXTargetRecord->usTarget;

            //clear pString for later use
            memset( pString, 0, MAX_SEGMENT_SIZE * sizeof(CHAR_W));

            //copy target string for later compare function
//            memcpy( pString, pByte, ulLen );
            ulLen = EQFCompress2Unicode( pString, pByte, ulLen );

            //tokenize target string in del structure
            //usRc = TokenizeTarget( pTmDel->szTarget, pNormString, &pTagRecord, &lTagAlloc, pTmDel->szTagTable, &usNormLen, this );

            if ( !usRc )
            {
              //compare target strings
              //if ( !UTF16strcmp( pString, pNormString ) )
              if ( !UTF16strcmp( pString, TmDel.pInputSentence->pStrings->getGenericTargetStrC() ) )
              {
                //target strings are equal so compare target tag records
                //position at target tag table record
                pByte = pStartTarget;
                pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
                pByte += pTMXTargetRecord->usTargetTagTable;
                pTMXTargetTagTable = (PTMX_TAGTABLE_RECORD)pByte;

                //compare tag table records
                if (true 
                //||   //we use the same tag tables
                //     !memcmp( pTMXTargetTagTable, pTagRecord,
                //              RECLEN(pTMXTargetTagTable) ) 
                )
                {
                  //identical target tag table as in del structure so
                  //check segment id and file name in control block
                  pByte = pStartTarget;
                  pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

                  //position at target control block
                  pByte += pTMXTargetRecord->usClb;
                  pClb = (PTMX_TARGET_CLB)pByte;

                  //get id of filename in the put structure
                  usRc = NTMGetIDFromName( TmDel.szDocName,
                                           TmDel.szDocName,
                                           (USHORT)FILE_KEY, &usId );
                  if ( !usRc )
                  {
                    ULONG ulLeftClbLen = RECLEN(pTMXTargetRecord) -
                                         pTMXTargetRecord->usClb;
                    while ( ulLeftClbLen && !fStop )
                    {
                      if ( 
                           //(pClb->usFileId == usId) &&
                           //(pClb->ulSegmId == pTmDel->ulSourceSegmentId) &&
                           //(pClb->bTranslationFlag == (BYTE)pTmDel->usTranslationFlag)
                           //||
                           true 
                           )
                      {
                        //if segment id and filename are equal then delete
                        //target CLB and any empty target record

                        //check that multiple flag isn't on
                        //if on leave while loop as though delete was carried out
                        if ( !pClb->bMultiple || (BOOL)TmDel.lTargetTime  || true)
                        {
                          //fill out the put structure as output of the extract function
                          usRc = FillExtStructure( this, pTMXTargetRecord,
                                                  pClb,
                                                  TmDel.pInputSentence->pStrings->getGenericTargetStrC(), &lSrcLen,
                                                  pTmExtOut );
                          NTMGetNameFromID( &pTMXSourceRecord->usLangId, (USHORT)LANG_KEY,
                                      pTmExtOut->szOriginalSourceLanguage, NULL );
                          pTmExtOut->ulRecKey = TmDel.recordKey = *pulKey;
                          pTmExtOut->usTargetKey = TmDel.targetKey = usTarget;

                          TMDelTargetClb( pTmRecord, pTMXTargetRecord, pClb );
                          
                          //add updated tm record to database
                          /**********************************************/
                          /* we usually should delete the record here   */
                          /* (if no translation is available for it)    */
                          /* BUT: since usually an untranslate is follow*/
                          /*  by a new translation, we will not remove  */
                          /* the key (only get rid of any target data)  */
                          /**********************************************/
                          usRc = TmBtree.EQFNTMUpdate(
                                               *pulKey,
                                               (PBYTE)pTmRecord,
                                               RECLEN(pTmRecord) );
                        } /* endif */

                        //leave while loop
                        fStop = TRUE;
                      }
                      else
                      {
                        // try next target CLB
                        ulLeftClbLen -= TARGETCLBLEN(pClb);
                        pClb = NEXTTARGETCLB(pClb);
                      } /* endif */
                    } /* endwhile */
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        //position at next target
        pByte = pStartTarget;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
        //move pointer to end of target
        pByte += RECLEN(pTMXTargetRecord);
        //remember the end/beginning of record
        pStartTarget = pByte;
        pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);
      } /* endwhile */
    } /* endif */

    if ( !fStop )
    {
      LOG_AND_SET_RC(usRc, T5INFO, SEG_NOT_FOUND);
    } /* endif */
  } /* endif */

  //release memory
  UtlAlloc( (PVOID *) &pString, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *) &(pTagRecord), 0L, 0L, NOMSG );

  return( usRc );
}

USHORT EqfMemory::FindTargetByKeyAndDelete(
                            PTMX_RECORD pTmRecord,
                            OtmProposal&  TmDel,
                            PTMX_SENTENCE pSentence,
                            TMX_EXT_OUT_W * pTmExtOut,
                            PULONG pulKey )
{
  USHORT usRc = 0;
  PTMX_SOURCE_RECORD pTMXSourceRecord; //ptr to source record
  PTMX_TARGET_RECORD pTMXTargetRecord; //ptr to target record
  PTMX_TARGET_CLB    pTargetClb;       //ptr to target CLB
  LONG       lLeftClbLen = 0;        // remaining length of CLB area
  PBYTE pByte;                         //position ptr
  LONG lSourceLen = 0;                 //length of source string
  USHORT usTarget = 0;                 //nr of target records in tm record
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

//NEW CODE
     //loop until correct target is found
        while ( (usTarget < TmDel.targetKey) && (lLeftTgtLen > 0) && !usRc )
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
          while ( (usTarget < TmDel.targetKey) && ( lLeftClbLen > 0 ) )
          {
            usTarget++;
            pTargetClb = NEXTTARGETCLB(pTargetClb);
            lLeftClbLen -= TARGETCLBLEN(pTargetClb);
          } /* endwhile */

          // continue with next target if no match yet
          if ( usTarget < TmDel.targetKey )
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

        if ( !usRc && (usTarget == TmDel.targetKey) )
        {
          //position at start of target record
          pTMXTargetRecord = (PTMX_TARGET_RECORD)(pByte);

          //if target record exists
          if ( ( lLeftTgtLen > 0) && ( RECLEN(pTMXTargetRecord) != 0) )
          {
            //check that multiple flag isn't on
            //if on leave while loop as though delete was carried out
            
            //fill out the put structure as output of the extract function
            usRc = FillExtStructure( this, pTMXTargetRecord,
                                     pTargetClb,
                                     pSourceString, &lSourceLen,
                                     pTmExtOut );
            NTMGetNameFromID( &pTMXSourceRecord->usLangId, (USHORT)LANG_KEY,
                        pTmExtOut->szOriginalSourceLanguage, NULL );
            pTmExtOut->ulRecKey = TmDel.recordKey = *pulKey;
            pTmExtOut->usTargetKey = TmDel.targetKey = usTarget;
            
            //if ( !pClb->bMultiple || (BOOL)TmDel.lTargetTime )
            {
              TMDelTargetClb( pTmRecord, pTMXTargetRecord, pTargetClb );

              //add updated tm record to database
              /**********************************************/
              /* we usually should delete the record here   */
              /* (if no translation is available for it)    */
              /* BUT: since usually an untranslate is follow*/
              /*  by a new translation, we will not remove  */
              /* the key (only get rid of any target data)  */
              /**********************************************/
              usRc = TmBtree.EQFNTMUpdate(
                                    *pulKey,
                                    (PBYTE)pTmRecord,
                                    RECLEN(pTmRecord) );
            } /* endif */
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
      }
    } 

  } /* endif */

  //release memory

  return( usRc );
}

