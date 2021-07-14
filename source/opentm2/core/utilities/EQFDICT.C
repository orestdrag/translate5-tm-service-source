/*! \file
	Description: Internal functions used by EQFQDAM.C For details see description in EQFQDAM.C

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions
#include <EQF.H>                  // General Translation Manager include file

//#include <EQFQDMAI.H>             // Private QDAM defines
//#include <eqfcmpr.h>              // defines for compression/expand...
//#include <eqfchtbl.h>             // character tables
#include <time.h>
//#include "eqfevent.h"                  // event logging facility

#if defined(ASDLOGGING)
 extern FILE *hAsdLog;
 #define ASDLOG()                \
    if ( hAsdLog )               \
       fprintf( hAsdLog, "QDAM: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define ASDLOG()
#endif


SHORT QDAMCaseCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2,                       // pointer to second key
    BOOL  fIgnorePunctuation           // ignore punctuation flag
);

#define INC_READ_COUNT
#define INC_CACHED_COUNT
#define INC_CACHED_READ_COUNT
#define INC_WRITE_COUNT
#define INC_REAL_READ_COUNT
#define INC_REAL_WRITE_COUNT


#define BTREE_REC_SIZE_V2  (4096)          // record size V2
#define BTREE_REC_SIZE_V3 (16384)          // record size V3

#define BTREE_BUFFER_V2   (BTREE_REC_SIZE_V2 + 10*sizeof(USHORT)) // buffer size
#define BTREE_BUFFER_V3   (BTREE_REC_SIZE_V3 + 10*sizeof(USHORT)) // buffer size

#define MAX_LIST           20          // number of recently used records
#define MIN_SIZE          128          // minimum free space requested in entry
#define MAXWASTESIZE      512          // max size  to be wasted due to updates
#define MAXDATASIZE    0x8000          // maximum data size allowed
#define MAX_LOCKREC_SIZE 4000          // max size for locked terms record
#define MAX_UPD_CTR        10          // max number of update counters
#define HEADTERM_SIZE     256          // size of the head term
#define COLLATE_SIZE      256          // size of the collating sequence
#define ENTRYENCODE_LEN    15          // number of significant characters
#define ENTRYDECODE_LEN    32          // length of decoding array
#define USERDATA_START   2046          // start of user data
#define INDEX_BUFFERS      20          // index buffers
#define NUMBER_OF_BUFFERS  20          // 20 // number of buffers to be used
#define MAX_NUM_DICTS      99          // max. number of dict concurrently open

// Value for chVersion in older databses (intitial version)
#define BTREE_V0 0x00

// Value for chVersion in second BTREE version
// Note: In this version the usNextFreeRecord field was introduced. As the
//       part of the header record containing this field was not initialized
//       in the older version the contents of the field has to be ignored if
//       chVersion is BTREE_V0.
#define BTREE_V1 0x01

// Value for chVersion in BTREE version 2
// Note: In this version the 32k length limit of data records was eliminated
#define BTREE_V2 0x02

// Value for chVersion in BTREE version 3
// Note: In this version the record size has been changed to 16k
#define BTREE_V3 0x03

 typedef struct _RECPARAM
  {
    USHORT  usNum;           // record number
    USHORT  usOffset;        // record offset
    ULONG   ulLen;           // record length
  }  RECPARAM, * PRECPARAM;

  typedef struct _RECPARAMOLD
  {
    USHORT  usNum;           // record number
    USHORT  usOffset;        // record offset
    SHORT  sLen;             // record length
  }  RECPARAMOLD, * PRECPARAMOLD;

/*****************************************************************************/
/* BTREEHEADRECORD is the record at the start of a B-tree file.  It is used  */
/* to confirm that the file is a valid one; it holds  database wide          */
/* persistent information                                                    */
/*****************************************************************************/
typedef struct _BTREEHEADRECORD
{
  CHAR      chEQF[7];                             // The type of file
//  BYTE      bVersion;                             // version flag
  struct
  {
   unsigned char bVersion     : 4;                // does the attribute contain text to be translated?
   unsigned char free1        : 1;                // unused
   unsigned char free2        : 1;                // unused
   unsigned char free3        : 1;                // unused
   unsigned char f16kRec      : 1;                // TRUE = BTREE uses 16k Recs (V3 record size)
  } Flags;

  EQF_BOOL  fOpen;                                // open flag/corruption flag
  USHORT    usFirstNode;                          // first node record
  USHORT    usFirstLeaf;                          // first leaf record
   RECPARAMOLD  DataRecList[ MAX_LIST ];          // last used data records
  EQF_BOOL  fTerse;                               // tersing requested
  BYTE      chCollate[COLLATE_SIZE];              // collating sequence to use
  BYTE      chCaseMap[COLLATE_SIZE];              // case mapping to be used
  BYTE      chEntryEncode[ ENTRYENCODE_LEN ];     // significant characters
  USHORT    usFreeKeyBuffer;                      // index of buffer to use
  USHORT    usFreeDataBuffer;                     // first data buffer chain
  USHORT    usFirstDataBuffer;                    // first data buffer
  ULONG     ulUpdCount;                           // last update counter
  USHORT    usNextFreeRecord;                     // Next record to expand to
} BTREEHEADRECORD, PBTREEHEADRECORD ;


/*****************************************************************************/
/* Used to determine amount of padding required for the Btree record so      */
/* that the header + keys fix in to a single disk record                     */
/*****************************************************************************/
typedef struct _BTREEHEADER
{
  CHAR    chType;                                 // record type
  USHORT  usNum;                                  // record number
  USHORT  usPrevious;                             // previous leaf node
  USHORT  usNext;                                 // next leaf node
  USHORT  usParent;                               // parent node
  USHORT  usOccupied;                             // # of keys in record
  USHORT  usFilled;                               // number of bytes filled
  USHORT  usLastFilled;                           // ptr. to next free byte
  USHORT  usWasteSize;                            // waste size?
} BTREEHEADER , * PBTREEHEADER;

/*****************************************************************************/
/* BTREERECORD  is the format of each of the blocks on the disk.             */
/*  This structure should be BTREE_REC_SIZE long                             */
/*****************************************************************************/

#define FREE_SIZE_V3  (BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER))
typedef struct _BTREERECORD_V3
{
  BTREEHEADER  header;                             // 16 bytes header
  UCHAR        uchData[ FREE_SIZE_V3 ] ;              // free size to be used
} BTREERECORD_V3, *PBTREERECORD_V3, **PPBTREERECORD_V3;

#define FREE_SIZE_V2  (BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER))
typedef struct _BTREERECORD_V2
{
  BTREEHEADER  header;                             // 16 bytes header
  UCHAR        uchData[ FREE_SIZE_V2 ] ;              // free size to be used
} BTREERECORD_V2, *PBTREERECORD_V2, **PPBTREERECORD_V2;



/*****************************************************************************/
/* BTREEBUFFER  is the format of the buffers when read in to the buffer      */
/* cache.  It maps a record number and its properties to its contents        */
/*****************************************************************************/
typedef struct _BTREEBUFFER_V2
{
  USHORT usRecordNumber;                           // index of rec in buffer
  BOOL   fLocked;                                  // Is the record locked ?
  BOOL   fNeedToWrite;                             // Commit before reuse
  SHORT  sUsed;                                    // buffer used count
  ULONG  ulCheckSum;                               // CheckSum of contents data
  BTREERECORD_V2 contents;                         // data from disk
} BTREEBUFFER_V2, * PBTREEBUFFER_V2;

typedef struct _BTREEINDEX_V2
{
  struct _BTREEINDEX_V2  * pNext;                    // point to next index buffer
  BTREEBUFFER_V2  btreeBuffer;                       // data from disk
} BTREEINDEX_V2, * PBTREEINDEX_V2;

typedef struct _BTREEBUFFER_V3
{
  USHORT usRecordNumber;                           // index of rec in buffer
  BOOL   fLocked;                                  // Is the record locked ?
  BOOL   fNeedToWrite;                             // Commit before reuse
  SHORT  sUsed;                                    // buffer used count
  ULONG  ulCheckSum;                               // CheckSum of contents data
  BTREERECORD_V3 contents;                            // data from disk
} BTREEBUFFER_V3, *PBTREEBUFFER_V3;


typedef struct _BTREEINDEX_V3
{
  struct _BTREEINDEX_V3 * pNext;                    // point to next index buffer
  BTREEBUFFER_V3  btreeBuffer;                       // data from disk
} BTREEINDEX_V3, * PBTREEINDEX_V3;

typedef SHORT _PFN_QDAMCOMPARE( PVOID, PVOID, PVOID );
typedef _PFN_QDAMCOMPARE *PFN_QDAMCOMPARE;


typedef struct _LOOKUPENTRY_V2
{
  PBTREEBUFFER_V2 pBuffer; // Pointer to BTREEBUFFER
} LOOKUPENTRY_V2, * PLOOKUPENTRY_V2;

typedef struct _LOOKUPENTRY_V3
{
  PBTREEBUFFER_V3 pBuffer; // Pointer to BTREEBUFFER
} LOOKUPENTRY_V3, *PLOOKUPENTRY_V3;


typedef struct _ACCESSCOUNTERTABLEENTRY
{
  ULONG ulAccessCounter;
} ACCESSCTRTABLEENTRY, * PACCESSCTRTABLEENTRY;


typedef struct _BTREEGLOB
{
   HFILE        fp;                               // index file handle
   HTM          htm;                              // handle in remote case

   USHORT       usFirstNode;                      // file pointer of record
   USHORT       usFirstLeaf;                      // file pointer of record

   PBTREEINDEX_V2  pIndexBuffer_V2;               // Pointer to index records
   PBTREEINDEX_V3  pIndexBuffer_V3;               // Pointer to index records
   USHORT       usIndexBuffer;                    // number of index buffers
   PFN_QDAMCOMPARE compare;                       // Comparison function
   USHORT       usNextFreeRecord;                 // Next record to expand to
   CHAR         chFileName[144];                  // Name of B-tree file
   RECPARAM     DataRecList[ MAX_LIST ];          // last used data records
   BOOL         fGuard;                           // write every record
   BOOL         fOpen;                            // open flag
   BOOL         fCorrupted;                       // mark as corrupted
   PCHAR_W      pTempKey;                         // pointer to temp. key
   PBYTE        pTempRecord;                      // pointer to temp record
   ULONG        ulTempRecSize;                    // size of temp record area
   BOOL         fTerse;                           // tersing requested
   BYTE         chEntryEncode[ ENTRYENCODE_LEN];  // significant characters
   BYTE         bEncodeLen[COLLATE_SIZE];         // encoding table length
   CHAR         chEncodeVal[COLLATE_SIZE];        // encoding table
   BYTE         chDecode[ENTRYDECODE_LEN];        // decoding table
   BYTE         chCollate[COLLATE_SIZE];          // collating sequence to use
   BYTE         chCaseMap[COLLATE_SIZE];          // case mapping to be used
   USHORT       usFreeKeyBuffer;                  // index of buffer to use
   USHORT       usFreeDataBuffer;                 // first data buffer chain
   USHORT       usFirstDataBuffer;                // first data buffer
   BTREEBUFFER_V2  BTreeTempBuffer_V2;            // temporary 4k buffer
   BTREEBUFFER_V3  BTreeTempBuffer_V3;            // temporary V3 buffer
   BOOL         fWriteHeaderPending;              // write of header pending
   LONG         lTime;                            // time of last update/open
   USHORT       usVersion;                        // version identification...
   CHAR         chEQF[7];                         // The type of file
   BYTE         bVersion;                         // version flag
   BOOL         fTransMem;                        // translation memory???
   USHORT       usOpenFlags;                      // settings used for open
   LONG         alUpdCtr[MAX_UPD_CTR];            // list of update counters
   HFILE        fpDummy;                          // dummy/lock semaphore file handle
   BOOL         fUpdated;                         // database-has-been-modified flag
   USHORT       usNumberOfLookupEntries;    // Number of allocated lookup-table-entries
   USHORT       usNumberOfAllocatedBuffers; // Number of allocated buffers
   ULONG        ulReadRecCalls;             // Number of calls to QDAMReadRecord
   PLOOKUPENTRY_V2 LookupTable_V2;                // Pointer to lookup-table
   PLOOKUPENTRY_V3 LookupTable_V3;                // Pointer to lookup-table
   PACCESSCTRTABLEENTRY AccessCtrTable;     // Pointer to access-counter-table
   USHORT       usBtreeRecSize;                   // size of BTREE records
   BYTE         bRecSizeVersion;                  // record size version flag
} BTREEGLOB, * PBTREEGLOB, ** PPBTREEGLOB ;


typedef enum _DICTCMD
{
  QDAMDICTOPEN,               // dictionary open
  QDAMDICTCREATE,             // dictionary create
  QDAMDICTCLOSE,              // dictionary close
  QDAMDICTSIGN,               // return signature record
  QDAMDICTUPDSIGN,            // update signature record
  QDAMDICTSUBSTR,             // find term containing substring
  QDAMDICTEQUIV,              // find equivalent term
  QDAMDICTEXACT,              // find exact term
  QDAMDICTNEXT,               // return next term
  QDAMDICTPREV,               // return prev term
  QDAMDICTCURR,               // return current term
  QDAMDICTINSERT,             // insert term
  QDAMDICTUPDATE,             // update term
  QDAMDICTDELETE,             // delete term
  QDAMDICTFLUSH,              // resynchronize dict.
  QDAMDICTCOPY,               // copy dict
  QDAMDICTNUMENTRIES,         // find number of terms in dict
  QDAMDICTFIRST,              // position at first entry
  QDAMDICTLOCKDICT,           // lock the dictionary
  QDAMDICTLOCKENTRY,          // lock the entry
  QDAMDICTCLOSEORGANIZE,      // end the organize
  QDAMDICTUPDTIME             // get update status
} DICTCMD;

/**********************************************************************/
/*  structure for Client/Server access                                */
/**********************************************************************/
typedef struct  _QDAMLAN
{
   union PREFINOUT
   {
     PREFIX_IN  prefin;                           // prefix in of each command
     PREFIX_OUT prefout;                          // prefix out of each command
   }          PrefInOut;
   DICTCMD    DictCmd;                            // dictionary command
   struct       _BTREEIDA  *  FRemote;       // pointer to remote BTREE
   USHORT     usOutLen;                           // length of the output struct
   CHAR       szUserId [MAX_USERID];              // logged on userid
} QDAMLAN , * PQDAMLAN;

typedef struct _BTREEIDA
{
   HTM          htm;                              // handle in remote case
   BOOL         fRemote;                          // we are dealing with remote
   SHORT        sCurrentIndex;                    // current sequence array
   USHORT       usCurrentRecord;                  // current sequence record
   PBTREEGLOB   pBTree;                           // pointer to global struct
   USHORT       usDictNum;                        // index in global structure
   CHAR_W       chHeadTerm[HEADTERM_SIZE];        // last active head term
   BOOL         fLock;                            // head term is locked
   CHAR_W       chLockedTerm[HEADTERM_SIZE];      // locked term if any
   CHAR         chServer[ MAX_SERVER_NAME + 1];   // server name
   PQDAMLAN     pQDAMLanIn;                       // pointer to buffer for LAN
   PQDAMLAN     pQDAMLanOut;                      // pointer to buffer for LAN
   struct       _BTREEIDA  *  pBTreeRemote;       // pointer to remote BTREE
   CHAR         szUserId [MAX_USERID];            // logged on userid
   BOOL         fPhysLock;                        // db has been physically locked
   CHAR         szFileName[MAX_LONGPATH];         // fully qualified name of file
 } BTREE, * PBTREE, ** PPBTREE;


static BTREEHEADRECORD header; // Static buffer for database header record

/**********************************************************************/
/* 'Magic word' for record containing locked terms                    */
/**********************************************************************/
static CHAR szLOCKEDTERMSKEY[] = "0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR_W szLOCKEDTERMSKEYW[] = L"0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR szLockRec[MAX_LOCKREC_SIZE]; // buffer for locked terms



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteHeader       Write initial header
//------------------------------------------------------------------------------
// Function call:     QDAMWriteHeader( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Writes the first record of the (index) file so that
//                    subsequent accesses know what the index is like.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     init initial record (describing the disk)
//                    position ptr to begin of file
//                    write record to disk
//                    check if disk is full
//                    if so, set sRC = BTREE_WRITE_ERROR
//
//------------------------------------------------------------------------------
 
SHORT QDAMWriteHeader
(
   PBTREE     pBTIda                   // pointer to btree structure
)
{
  SHORT  sRc=0;
  BTREEHEADRECORD HeadRecord;          // header record
  USHORT usBytesWritten;               // number of bytes written
  ULONG  ulNewOffset;                  // new offset in file
  PBTREEGLOB  pBT = pBTIda->pBTree;

#ifdef TEMPORARY_COMMENTED
  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );
#endif

  // Write the initial record (describing the index) out to disk

  memset( &HeadRecord, '\0', sizeof(BTREEHEADRECORD));
  memcpy(HeadRecord.chEQF,pBT->chEQF, sizeof(pBT->chEQF));
  HeadRecord.usFirstNode = pBT->usFirstNode;
  HeadRecord.usFirstLeaf = pBT->usFirstLeaf;
  HeadRecord.usFreeKeyBuffer = pBT->usFreeKeyBuffer;
  HeadRecord.usFreeDataBuffer = pBT->usFreeDataBuffer;
  HeadRecord.usFirstDataBuffer = pBT->usFirstDataBuffer;  // first data buffer
  HeadRecord.fOpen  = (EQF_BOOL)pBT->fOpen;                    // open flag set
  // DataRecList in header is in old format (RECPARAMOLD),
  // so convert in-memory copy of list (in the RECPARAM format) to
  // the old format
  {
    int i;
    for ( i = 0; i < MAX_LIST; i++ )
    {
      HeadRecord.DataRecList[i].usOffset = pBT->DataRecList[i].usOffset;
      HeadRecord.DataRecList[i].usNum    = pBT->DataRecList[i].usNum;
      HeadRecord.DataRecList[i].sLen     = (SHORT)pBT->DataRecList[i].ulLen;
    } /* endfor */
  }
  HeadRecord.fTerse = (EQF_BOOL) pBT->fTerse;
  memcpy( HeadRecord.chCollate, pBT->chCollate, COLLATE_SIZE );
  memcpy( HeadRecord.chCaseMap, pBT->chCaseMap, COLLATE_SIZE );

  memcpy( HeadRecord.chEntryEncode, pBT->chEntryEncode, ENTRYENCODE_LEN );

  /********************************************************************/
  /* Update usNextFreeRecord field of Header and indicate that the    */
  /* field is valid by assigning BTREE_V1 to the bVersion field.      */
  /********************************************************************/
  HeadRecord.usNextFreeRecord = pBT->usNextFreeRecord;
  HeadRecord.Flags.bVersion         = BTREE_V1;


  if ( pBT->bRecSizeVersion == BTREE_V3 )
  {
    HeadRecord.Flags.f16kRec  = TRUE;
  }
  else
  {
    HeadRecord.Flags.f16kRec  = FALSE;
  } /* endif */

  sRc = UtlChgFilePtr( pBT->fp, 0L, FILE_BEGIN, &ulNewOffset, FALSE);

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &HeadRecord,
                    sizeof(BTREEHEADRECORD), &usBytesWritten, FALSE );
    if ( sRc )
    {
      sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    }
    else
    {
      // check if disk is full
      if ( usBytesWritten != sizeof(BTREEHEADRECORD)  )
      {
        sRc = BTREE_DISK_FULL;
      }
      /****************************************************************/
      /* write through of header record if in GUARD mode or for       */
      /* shared databases                                             */
      /****************************************************************/
      else if ( pBT->fGuard || (pBT->usOpenFlags & ASD_SHARED) )
      {
        if ( UtlBufReset( pBT->fp, FALSE ))
        {
          sRc = BTREE_WRITE_ERROR;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  if ( sRc == NO_ERROR )
  {
    pBT->fUpdated = TRUE;
  } /* endif */

  if ( sRc )
  {
      #ifdef TEMPORARY_COMMENTED
    ERREVENT2( QDAMWRITEHEADER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
    #endif
  } /* endif */
  
  #ifdef TEMPORARY_COMMENTED
  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCEXIT_EVENT, 0, DB_GROUP, NULL );
  #endif

  return sRc;
}