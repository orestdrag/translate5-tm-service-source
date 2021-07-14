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

#define MAX_READREC_CALLS 1000000L
#define ACCESSBONUSPOINTS 5000L
/**********************************************************************/
/* To check that we are opening a valid B-tree file, there is a       */
/* magic cookie stored at the beginning.  This is as follows          */
/* This value might be changed to include version identifications...  */
/**********************************************************************/
//#define BTREE_VERSION       0   // moved to EQFQDAM.h
//#define BTREE_VERSION2      2   // moved to EQFQDAM.h
//#define BTREE_VERSION3      3   // moved to EQFQDAM.h
#define BTREE_HEADER_VALUE_V0 "EQF "
#define BTREE_HEADER_VALUE_V1 "EQFSOH"
#define BTREE_HEADER_VALUE_V2 "EQFSTX"
#define BTREE_HEADER_VALUE_V3 "EQFETX"


#define NTM_VERSION         1
#define BTREE_HEADER_VALUE_TM1 "NTMSOH"
// BTREE databases of version NTM_VERSION2 support data records with
// a size of more than 32k, the length field at the begin of the
// data record is of type ULONG instead of USHORT
#define NTM_VERSION2       2
#define BTREE_HEADER_VALUE_TM2 "NTMSTX"

#define NTM_VERSION3       3
#define BTREE_HEADER_VALUE_TM3 "NT"

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


///* Maximum size (entries) of the lookup table (it is a USHORT).
#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x0FFF0
/* Initial size (entries) of the lookup table */
#define MIN_NUMBER_OF_LOOKUP_ENTRIES 32

/**********************************************************************/
/* typedef used for all vital information on our new approach...      */
/* NOTE: this structure is limited to COLLATE_SIZE, which is 256      */
/**********************************************************************/
typedef struct _TMVITALINFO
{
  ULONG   ulStartKey;                  // key to start with
  ULONG   ulNextKey;                   // currently active key
} NTMVITALINFO, * PNTMVITALINFO;

#define NTMNEXTKEY( pBT )  ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulNextKey
#define NTMSTARTKEY( pBT ) ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulStartKey

/**********************************************************************/
/* 'Magic word' for record containing locked terms                    */
/**********************************************************************/
static CHAR szLOCKEDTERMSKEY[] = "0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR_W szLOCKEDTERMSKEYW[] = L"0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR szLockRec[MAX_LOCKREC_SIZE]; // buffer for locked terms


/**********************************************************************/
/* default collating sequence as returned by DosGetCollating for a    */
/* ASCII 850 code page..                                              */
/**********************************************************************/
UCHAR chDefCollate[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
    96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
    67,  85,  69,  65,  65,  65,  65,  67,  69,  69,  69,  73,  73,  73,  65,  65,
    69,  65,  65,  79,  79,  79,  85,  85,  89,  79,  85,  79,  36,  79, 158,  36,
    65,  73,  79,  85,  78,  78, 166, 167,  63, 169, 170, 171, 172,  33,  34,  34,
   176, 177, 178, 179, 180,  65,  65,  65, 184, 185, 186, 187, 188,  36,  36, 191,
   192, 193, 194, 195, 196, 197,  65,  65, 200, 201, 202, 203, 204, 205, 206,  36,
    68,  68,  69,  69,  69,  73,  73,  73,  73, 217, 218, 219, 220, 221,  73, 223,
    79,  83,  79,  79,  79,  79, 230, 232, 232,  85,  85,  85,  89,  89, 238, 239,
   240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };


/**********************************************************************/
/* Table for punctuation detection                                    */
/* The following characters are treated as punctuation and will be    */
/* ignored for word recognition:                                      */
/* .-/[]_                                                             */
/*                                                                    */
/* Users: EQFMORPH.C(MorphWordRecognition)                            */
/*                                                                    */
/* Note:  The character space 0x20 may not be included in this table  */
/*        otherwise the word recognition will fail                    */
/**********************************************************************/
static BOOL fIsPunctuation[256] = {
/*        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
/* 0. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 1. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 2. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
/* 3. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 4. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 5. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
/* 6. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 7. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 8. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 9. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* A. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* B. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* C. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* D. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* E. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* F. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct _STENCODEBITS
{
  USHORT usLen;
  USHORT usVal;
} STENCODEBITS;

// defines for retries in case of BTREE_IN_USE conditions
#define MAX_RETRY_COUNT 30
#define MAX_WAIT_TIME   100
/**********************************************************************/
/*  encoding sequence used                                            */
/*  The following should never be changed......                       */
/*  The general structure used is an algorithm based on HUFFMANN      */
/*  but working with fixed distributions.                             */
/*                                                                    */
/* Algorithm:                                                         */
/*   The algorithm is a variable length based algorithm, varying in   */
/* length from 3 bits to 11 bits.                                     */
/* The most frequent 15 characters will be encoded in 3 to 5 bits.    */
/* The bit sequence used are listed in the following table.           */
/* All others are encoded in 11 bits, i.e. '000' plus the character.  */
/*                                                                    */
/* The last three tables are used to allow fast access to determine   */
/* the encode values to allow decomposition.                          */
/**********************************************************************/
//                                       Leng  Value   Bits
static STENCODEBITS stEncodeBits[16] = { {3,0},     // 000
                                         {3,1},     // 001
                                         {3,2},     // 010
                                         {3,3},     // 011
                                         {4,8},     // 1000
                                         {4,9},     // 1001
                                         {4,12},    // 1100
                                         {4,13},    // 1101
                                         {5,20},    // 10100
                                         {5,21},    // 10101
                                         {5,22},    // 10110
                                         {5,23},    // 10111
                                         {5,28},    // 11100
                                         {5,29},    // 11101
                                         {5,30},    // 11110
                                         {5,31}};   // 11111

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMKeyCompare      Generic compare function
//------------------------------------------------------------------------------
// Function call:     QDAMKeyCompare( PBTREE,PCHAR, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       This is the generic compare function used
//                    for comparision
//                    It uses the Maptable provided for lower/upercase mapping
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to tree structure
//                    PCHAR                  first key
//                    PCHAR                  second key
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0   keys are equal
//                    <> keys are unequal
//
//------------------------------------------------------------------------------
// Function flow:     while character available in key1
//                      case map characters and find difference
//                      if different then
//                        go to next character touple
//                      else
//                        return collating difference
//                      endif
//                    wend
//                    get difference for last comparison and return collat.diff
//                    return difference
//------------------------------------------------------------------------------
SHORT QDAMKeyCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2                        // pointer to second key
)
{
  SHORT   sDiff;
  CHAR_W  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PBYTE  pCollate = pBT->chCollate;    // pointer to collating sequence
  PSZ_W  pbKey1 = (PSZ_W) pKey1;
  PSZ_W  pbKey2 = (PSZ_W) pKey2;

  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    while ( (c = *pbKey2) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey2++;
    } /* endwhile */
    while ( (c = *pbKey1) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey1++;
    } /* endwhile */
// @@ TODO: check collating sequence and fIsPunctuation

    sDiff = *(pCollate+(BYTE)c) - *(pCollate + (BYTE)*pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pCollate + (BYTE)c) - *(pCollate + (BYTE)*pbKey2));
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMTerseInit  Initialize Tersing                        |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMTerseInit( PBTREE, PUCHAR );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This call will build the compression table from          |
//|                   passed frequency table                                   |
//|                   A decode table is built as well.                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE           pointer to btree structure              |
//|                   PUCHAR           pointer to frequent character table     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     init encoding scheme                                     |
//|                   use the first 15 characters of the passed byte aray to   |
//|                     build encoding scheme                                  |
//|                   use the first 15 characters to build decoding scheme     |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

VOID QDAMTerseInit
(
   PBTREE  pBTIda,                               // pointer to btree
   PUCHAR  pComp                                 // pointer to comp table
)
{
   USHORT usI;                                  // index
   PBTREEGLOB    pBT = pBTIda->pBTree;

   /*******************************************************************/
   /* init values                                                     */
   /*******************************************************************/
   memset( pBT->chEncodeVal, (BYTE) stEncodeBits[0].usVal, COLLATE_SIZE);
   memset( pBT->bEncodeLen, (BYTE) stEncodeBits[0].usLen, COLLATE_SIZE);
   /*******************************************************************/
   /* use the 15 first characters to build the encoding sequence      */
   /*******************************************************************/
   for ( usI = 0; usI < 15; usI++ )
   {
     pBT->bEncodeLen[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usLen;
     pBT->chEncodeVal[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usVal;
     pBT->chDecode[ stEncodeBits[ usI+1 ].usVal ] = *(pComp+usI);
   } /* endfor */

   return;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMKeyCompare      Generic compare function              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMKeyCompare( PBTREE,PULONG,PULONG);                    |
//+----------------------------------------------------------------------------+
//|Description:       This is the generic compare function used                |
//|                   for comparision                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to tree structure         |
//|                   PULONG                 first key                         |
//|                   PULONG                 second key                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0   keys are equal                                       |
//|                   <> keys are unequal                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     return (SHORT) (*pulKey1 - *pulKey2)                     |
//+----------------------------------------------------------------------------+

SHORT NTMKeyCompare
(
    PVOID  pBTIda,                     // pointer to tree structure
    PVOID  pulKey1,                    // pointer to first key
    PVOID  pulKey2                     // pointer to second key
)
{
  ULONG  ulKey1 = *((PULONG)pulKey1);
  ULONG  ulKey2 = *((PULONG)pulKey2);
  SHORT  sRc;
  pBTIda;                              // avoid compiler warnings

  if (ulKey1 > ulKey2 )
  {
    sRc = 1;
  }
  else if (ulKey1 < ulKey2 )
  {
    sRc = -1;
  }
  else
  {
    sRc = 0;
  } /* endif */
  return sRc;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAllocTempAreas    Allocate Temp Areas
//------------------------------------------------------------------------------
// Function call:     QDAMAllocTempAreas( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Allocate temp areas if necessary for key and data
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_ROOM     not enough memory
//------------------------------------------------------------------------------
// Function flow:     if no temp areas exist
//                      if no temp area for key exists
//                        allocate it
//                      endif
//                      if no temp record exists
//                        allocate it
//                      endif
//                      if not both areas allocated then
//                        set Rc to BTREE_NO_ROOM
//                      endif
//                    endif
//                    return Rc
//------------------------------------------------------------------------------

SHORT QDAMAllocTempAreas
(
   PBTREE  pBTIda
)
{
   SHORT  sRc = 0;                     // return code
   PBTREEGLOB    pBT = pBTIda->pBTree;


   if ( !pBT->pTempKey || !pBT->pTempRecord )
   {
      if ( ! pBT->pTempKey)
      {
         UtlAlloc( (PVOID *)&pBT->pTempKey, 0L, (LONG) HEADTERM_SIZE, NOMSG );
      } /* endif */
      if ( ! pBT->pTempRecord )
      {
         UtlAlloc( (PVOID *)&pBT->pTempRecord, 0L, (LONG) MAXDATASIZE, NOMSG );
         if ( pBT->pTempRecord )
         {
           pBT->ulTempRecSize = MAXDATASIZE;
         } /* endif */
      } /* endif */
      if ( !(pBT->pTempRecord && pBT->pTempKey) )
      {
         sRc = BTREE_NO_ROOM;
      } /* endif */
   } /* endif */

   return ( sRc );
}


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
){
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


SHORT QDAMDictUpdSignLocal
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   ULONG  ulLen                        // length of user data
);



SHORT QDAMReadRecordFromDisk_V3
(
   PBTREE         pBTIda,
   USHORT         usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL           fNewRec              // allow new records flag
)
{
  USHORT    usNumBytesRead = 0;                   // number of bytes read
  PBTREEHEADER pHeader;                          // pointer to header
  LONG     lOffset;                              // file offset to be set
  ULONG    ulNewOffset;                          // new position
  PBTREEBUFFER_V3   pBuffer = NULL;
  SHORT    sRc = 0;                             // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V3 pLEntry = NULL;
#ifdef TEMPORARY_COMMENTED
  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );
#endif

  /********************************************************************/
  /* Allocate space for a new buffer and let pBuffer point to it      */
  /********************************************************************/
  if ( !pBT->LookupTable_V3 || ( usNumber >= pBT->usNumberOfLookupEntries ))
  {
    sRc = BTREE_LOOKUPTABLE_CORRUPTED;
  }
  else
  {
    pLEntry = pBT->LookupTable_V3 + usNumber;

    /* Safety-Check: is the lookup table entry (ptr. to buffer) for the
                     record to read NULL ? */
    if ( pLEntry->pBuffer )
    {
      /* ptr. isn't NULL: this should never occur */
      sRc = BTREE_LOOKUPTABLE_CORRUPTED;
    }
    else
    {
      /* Allocate memory for a buffer */
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, (LONG) BTREE_BUFFER_V3 , NOMSG );
      if ( pLEntry->pBuffer )
      {
        (pBT->usNumberOfAllocatedBuffers)++;
        pBuffer = pLEntry->pBuffer;
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    INC_REAL_READ_COUNT;
    if ( !sRc )
    {
      lOffset = ((LONG) usNumber) * BTREE_REC_SIZE_V3;
      sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
    } /* endif */

    // Read the record in to the buffer space
    // Mark the next buffer for future allocations
    if ( !sRc )
    {
      #ifdef TEMPORARY_COMMENTED
      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, NULL );
      #endif
      sRc = UtlRead( pBT->fp,
                     (PVOID)&pBuffer->contents,
                     BTREE_REC_SIZE_V3, &usNumBytesRead, FALSE);
      
      #ifdef TEMPORARY_COMMENTED
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
      #endif

      /****************************************************************/
      /* The following check added by XQG                             */
      /* (Sometimes UtlRead terminates with sRc = 0 and usNumBytes = 0*/
      /*  if the database is locked and although we are NOT at the    */
      /*  end of the file!)                                           */
      /****************************************************************/
      if ( !sRc && !usNumBytesRead )
      {
        if ( usNumber < pBT->usNextFreeRecord )
        {
          // this is rather an error condition than an end-of-file condition!
          if ( pBT->usOpenFlags & ASD_SHARED )
          {
            sRc = BTREE_IN_USE;
          }
          else
          {
            sRc = BTREE_READ_ERROR;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( !sRc )
    {
       pBuffer->usRecordNumber = usNumber;
//     pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
       *ppReadBuffer = pBuffer;
       if ( !usNumBytesRead )
       {
         /*************************************************************/
         /* we are at the end of the file                             */
         /*************************************************************/
         if ( fNewRec )                // new record mode ???
         {
           memset (&pBuffer->contents, 0, sizeof(BTREERECORD_V3));
           // init this record
           pHeader = &(pBuffer->contents.header);
           pHeader->usOccupied = 0;
           pHeader->usNum = usNumber;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );

            #ifdef TEMPORARY_COMMENTED
           sRc = QDAMWRecordToDisk_V3(pBTIda, pBuffer);
           #endif

           if (! sRc )
           {
             if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
             {
               // this condition should NEVER occur for shared resources
               // as write is only allowed if the database has been
               // previously locked
               #ifdef TEMPORARY_COMMENTED
               ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 2, DB_GROUP, NULL );
               #endif

 #ifdef _DEBUG
               WinMessageBox( HWND_DESKTOP,
                              (HWND)UtlQueryULong( QL_TWBCLIENT ),
                              "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/2\nPlease save info required to reproduce this condition.",
                              "Gotcha!", 9999, MB_ERROR );
 #endif
             } /* endif */
             sRc = QDAMWriteHeader( pBTIda );
           } /* endif */
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
         }
         else
         {
           #ifdef TEMPORARY_COMMENTED
           ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, NULL );
           #endif
#ifdef _DEBUG
           WinMessageBox( HWND_DESKTOP,
                          (HWND)UtlQueryULong( QL_TWBCLIENT ),
                          "EQF9998: Write of new QDAM record w/o fNewRec set!\nLoc=READRECORDFROMDISK/3\n",
                          "Internal Error", 9998, MB_ERROR );
#endif
           sRc = BTREE_READ_ERROR;
         } /* endif */
       } /* endif */
    } /* endif */
  } /* endif */

  // For shared databases, check if database has been changed since last access
  // in case of changes force a BTREE_IN_USE return code thus triggering a retry
  // on one the outer calling levels
  if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
  {
      LONG  lUpdCount;                 // buffer for new value of update counter

     /**********************************************************************/
     /* Get current database update count                                  */
     /**********************************************************************/
     #ifdef TEMPORARY_COMMENTED
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );
     #endif

     if ( !sRc )
     {
        if ( lUpdCount != pBT->alUpdCtr[0] )
        {
           sRc = BTREE_INVALIDATED;
        } /* endif */
     } /* endif */
  } /* endif */

  /********************************************************************/
  /* Free record buffer in case of errors so record will be re-read   */
  /* during retry operations                                          */
  /********************************************************************/
  if ( sRc )
  {
    if ( (pLEntry != NULL) && (pLEntry->pBuffer != NULL) )
    {
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
      (pBT->usNumberOfAllocatedBuffers)--;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    #ifdef TEMPORARY_COMMENTED
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
    #endif
  } /* endif */
  return sRc;
}


SHORT  QDAMReadRecord_V3
(
   PBTREE  pBTIda,
   USHORT  usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL    fNewRec
)
{
  USHORT   i;
  SHORT    sRc = 0;                  // return code
  BOOL     fMemOK = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V3  pLEntry;
  PACCESSCTRTABLEENTRY  pACTEntry;
#ifdef TEMPORARY_COMMENTED
  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );
#endif

  *ppReadBuffer = NULL;
  INC_READ_COUNT;

  /********************************************************************/
  /* If Lookup-Table is too small allocate additional memory for it   */
  /********************************************************************/
  if ( usNumber >= MAX_NUMBER_OF_LOOKUP_ENTRIES )
  {
    /* There is no room for this record number in the lookup table */
    sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
  }
  else if ( !pBT->LookupTable_V3 || !pBT->AccessCtrTable )
  {
    sRc = BTREE_LOOKUPTABLE_NULL;
  }
  else
  {
    if ( usNumber >= pBT->usNumberOfLookupEntries )
    {
      /* The lookup-table entry for the record to read doesn't exist */
      /* Reallocate memory for LookupTable and AccessCounterTable */
      fMemOK = UtlAlloc( (PVOID *)&pBT->LookupTable_V3,
              (LONG) pBT->usNumberOfLookupEntries * sizeof(LOOKUPENTRY_V3),
              (LONG) (usNumber + 10) * sizeof(LOOKUPENTRY_V3), NOMSG );
      if ( fMemOK==TRUE )
      {
        fMemOK = UtlAlloc( (PVOID *)&pBT->AccessCtrTable,
                (LONG) pBT->usNumberOfLookupEntries * sizeof(ACCESSCTRTABLEENTRY),
                (LONG) (usNumber + 10) * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( fMemOK==TRUE )
        {
          pBT->usNumberOfLookupEntries = usNumber + 10;
        }
        else
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /******************************************************************/
    /* If record is in memory set ppReadBuffer                        */
    /* else call QDAMReadRecordFromDisk to read it into memory        */
    /******************************************************************/
    pLEntry = pBT->LookupTable_V3 + usNumber;

    if ( pLEntry->pBuffer )
    {
      /* Safety-Check: is rec.number = number of rec. to read ? */
      if ( (pLEntry->pBuffer)->usRecordNumber == usNumber )
      {
        /* Record is already in memory */
        *ppReadBuffer = pLEntry->pBuffer;
      }
      else
      {
        /* This should never occur ! */
        sRc = BTREE_LOOKUPTABLE_CORRUPTED;
      } /* endif */
    }
    else
    {
      /* Record isn't in memory -> read it from disk */
      sRc =  QDAMReadRecordFromDisk_V3( pBTIda, usNumber, ppReadBuffer, fNewRec );
      pACTEntry=pBT->AccessCtrTable + usNumber;
      pACTEntry->ulAccessCounter = 0L;
    } /* endif */
  } /* endif */

  /****************************************************************************/
  /* If #calls of QDAMReadRecord is greater than MAX_READREC_CALLS:           */
  /*   write unlocked records with access counter < MAX_READREC_CALLS to disk */
  /*   (if fNeedToWrite is set) and free the allocated memory                 */
  /*   set #calls of QDAMReadRecord = 0                                       */
  /* else increment #calls of QDAMReadRecord                                  */
  /****************************************************************************/
  if ( !sRc )
  {
    if ( pBT->ulReadRecCalls >= MAX_READREC_CALLS )
    {
      for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
      {
        pLEntry = pBT->LookupTable_V3 + i;
        pACTEntry = pBT->AccessCtrTable + i;
        if ( pLEntry->pBuffer && !((pLEntry->pBuffer)->fLocked) && (i!=usNumber)
             && (pACTEntry->ulAccessCounter<MAX_READREC_CALLS) )
        {
            /* write buffer and free allocated space */
            if ( (pLEntry->pBuffer)->fNeedToWrite )
            {
              if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
              {
                // this condition should NEVER occur for shared resources
                // as write is only allowed if the database has been
                // previously locked
                #ifdef TEMPORARY_COMMENTED
                ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
                #endif

              #ifdef _DEBUG
                WinMessageBox( HWND_DESKTOP,
                   (HWND)UtlQueryULong( QL_TWBCLIENT ),
                   "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/1\nPlease save info required to reproduce this condition.",
                   "Gotcha!", 9999, MB_ERROR );
              #endif
              } /* endif */
              #ifdef TEMPORARY_COMMENTED
              sRc = QDAMWRecordToDisk_V3(pBTIda, pLEntry->pBuffer);
              #endif
            } /* endif */
            if ( !sRc )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L , NOMSG );
              pBT->usNumberOfAllocatedBuffers--;
            } /* endif */
        } /* endif */

        if ( pACTEntry->ulAccessCounter<MAX_READREC_CALLS )
        {
          pACTEntry->ulAccessCounter = 0L;
        }
        else
        {
          pACTEntry->ulAccessCounter -= MAX_READREC_CALLS;
        } /* endif */
      } /* endfor */
      pBT->ulReadRecCalls = 0L;
    }
    else
    {
      pBT->ulReadRecCalls++;
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /* Increase the access counter for the record just read by  ACCESSBONUSPOINTS */
    pACTEntry=pBT->AccessCtrTable + usNumber;
    pACTEntry->ulAccessCounter += ACCESSBONUSPOINTS;
  } /* endif */

  if ( sRc )
  {
    #ifdef TEMPORARY_COMMENTED
    ERREVENT2( QDAMREADRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
    #endif

  } /* endif */

  return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       add dictionary to our global list of open dicts
//------------------------------------------------------------------------------
// Parameters:        PSZ    pName,           name of dictionary
//                    PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_MAX_DICTS     maximum of dictionaries exceeded
//------------------------------------------------------------------------------
// Function flow:     loop thru global struct to find open slot.
//                    if too many open dicts
//                      set return code to BTREE_MAX_DICTS
//                    else
//                      copy significant data into free slot
//                    endif
//                    return success
//
//------------------------------------------------------------------------------
SHORT
QDAMAddDict
(
  PSZ  pName,
  PBTREE  pBTIda
)
{
  USHORT usI = 1;
  SHORT  sRc = 0;
  #ifdef TEMPORARY_COMMENTED
  PQDAMDICT  pQDict;

  while ( QDAMDict[usI].usOpenCount )
  {
    usI++;
  } /* endwhile */
  /********************************************************************/
  /* if still space is available fill it up                           */
  /********************************************************************/
  if ( usI < MAX_NUM_DICTS )
  {
    pQDict = &(QDAMDict[usI]);
    pQDict->pBTree = pBTIda->pBTree;
    pQDict->usOpenCount = 1;
    pQDict->pIdaList[pQDict->usOpenCount] = pBTIda;
    strcpy(pQDict->chDictName, pName);
    pBTIda->usDictNum = usI;
  }
  else
  {
    sRc = BTREE_MAX_DICTS;
  } /* endif */
  #endif

  return sRc;
} /* end of function QDAMAddDict */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCreateLocal   Create local Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCreateLocal( PSZ, PSZ, SHORT, PCHAR, USHORT,
//                                         PCHAR, PCHAR, PCHAR, PBTREE);
//------------------------------------------------------------------------------
// Description:       Establishes the basic parameters for searching a
//                    user dictionary.
//                    These parameters are stored in the first record of the
//                    index file so that subsequent accesses
//                    know what the index is like.
//
//                    If no server name is given (NULL pointer or EOS) than
//                    it is tried to open a local dictionary.
//                    If no collating sequence is given (NULL pointer) the
//                    default collating sequence is assumed
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map file
//                    PBTREE *         pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     allocate space for BTREE
//                    if not possible
//                      set Rc = BTREE_NO_ROOM
//                    else
//                      open the index file which holds the BTREE
//                      if ok
//                        set initial structure heading
//                        allocate buffer space (for NUMBER_OF_BUFFERS many)
//                        if not possible
//                          set Rc = BTREE_NO_ROOM
//                        if not Rc
//                          call QDAMAllocTempAreas
//                        endif
//                        if not Rc
//                          write header to file
//                          if ok
//                            write 2nd part of first record
//                          endif
//                          if ok
//                            write out an empty root node
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_OPEN_ERROR
//                      endif
//                      if not Rc
//                        return pointer to BTree
//                      else
//                        destroy BTree
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictCreateLocal
(
   PSZ    pName,                       // name of file
   SHORT  sNumberOfKey,                // number of key buffers
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   PCHAR  pTermTable,                  // term encoding sequence
   PCHAR  pCollating,                  // collating sequence
   PCHAR  pCaseMap,                    // case map string
   PBTREE * ppBTIda,                   // pointer to btree structure
   PNTMVITALINFO pNTMVitalInfo         // translation memory
)
{
  PBTREE  pBTIda = *ppBTIda;           // init return value
  SHORT i;
  SHORT sRc = 0;                       // return code
  USHORT  usAction;                    // used in open of file
  PBTREEGLOB pBT;                      // set work pointer to passed pointer
  BOOL   fTransMem = (pNTMVitalInfo != NULL);   // translation memory

  sNumberOfKey;                        // get rid of compiler warnings
    
  /********************************************************************/
  /* allocate global area first ...                                   */
  /********************************************************************/
  if ( ! UtlAlloc( (PVOID *)&pBT, 0L, (LONG) sizeof(BTREEGLOB ), NOMSG ) )
  {
     sRc = BTREE_NO_ROOM;
  }
  else
  {
    pBTIda->pBTree = pBT;          // anchor pointer

    // Try to create the index file
    sRc = UtlOpen( pName, &pBT->fp, &usAction, 0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                   OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, FALSE);
  } /* endif */

  if ( !sRc )
  {
    // remember file name
    strcpy( pBTIda->szFileName, pName );

    // set initial structure heading
    UtlTime( &(pBT->lTime) );                                     // set open time
    pBTIda->sCurrentIndex = 0;
    pBT->usFirstNode=pBT->usFirstLeaf = 1;
    pBTIda->usCurrentRecord = 0;
    pBT->compare = QDAMKeyCompare;
    pBT->usNextFreeRecord = 1;
    pBT->usFreeKeyBuffer = 0;
    pBT->usFreeDataBuffer = 0;
    pBT->usFirstDataBuffer = 0;  // first data buffer
    pBT->fOpen  = TRUE;                          // open flag set
    strcpy(pBT->chFileName, pName);              // copy file name into struct
    pBT->fTransMem = fTransMem;
    pBT->fpDummy = NULLHANDLE;
    if ( pBT->fTransMem )
    {
      pBT->bVersion = BTREE_V2;
      pBT->bRecSizeVersion = BTREE_V3;
      pBT->usBtreeRecSize = BTREE_REC_SIZE_V3;
    }
    else
    {
      pBT->bVersion = BTREE_V2;
      pBT->bRecSizeVersion = BTREE_V2;
      pBT->usBtreeRecSize = BTREE_REC_SIZE_V2;
    } /* endif */

    /******************************************************************/
    /* do settings depending if we are dealing with a dict or a tm..  */
    /******************************************************************/
    if ( !fTransMem )
    {
      pBT->usVersion = BTREE_VERSION3;
      strcpy(pBT->chEQF,BTREE_HEADER_VALUE_V3);
      // use passed compression table if available else use default one
      if ( pTermTable )
      {
         memcpy( pBT->chEntryEncode, pTermTable, ENTRYENCODE_LEN );
         QDAMTerseInit( pBTIda, pBT->chEntryEncode );   // initialise for compression
         pBT->fTerse = TRUE;
      }
      else
      {
         pBT->fTerse = FALSE;
      } /* endif */
      /******************************************************************/
      /* support user defined collating sequence and case mapping       */
      /******************************************************************/
      if ( pCollating )
      {
         memcpy( pBT->chCollate, pCollating, COLLATE_SIZE );
      }
      else
      {
        memcpy( pBT->chCollate, chDefCollate, COLLATE_SIZE );
      } /* endif */

      if ( pCaseMap )
      {
         memcpy( pBT->chCaseMap, pCaseMap, COLLATE_SIZE );
      }
      else
      {
        /****************************************************************/
        /* fill in the characters and use the UtlLower function ...     */
        /****************************************************************/
        PBYTE  pTable;
        UCHAR  chTemp;

        pTable = pBT->chCaseMap;
        for ( i=0;i < COLLATE_SIZE; i++ )
        {
           *pTable++ = (CHAR) i;
        } /* endfor */
        chTemp = pBT->chCaseMap[ COLLATE_SIZE - 1];
        pBT->chCaseMap[ COLLATE_SIZE - 1] = EOS;
        pTable = pBT->chCaseMap;
        pTable++;
        UtlLower( (PSZ)pTable );
        pBT->chCaseMap[ COLLATE_SIZE - 1] = chTemp;
      } /* endif */
    }
    else
    {
      pBT->usVersion = NTM_VERSION2;
      strcpy(pBT->chEQF,BTREE_HEADER_VALUE_TM2);
      if ( pTermTable )
      {
         memcpy( pBT->chEntryEncode, pTermTable, ENTRYENCODE_LEN );
         QDAMTerseInit( pBTIda, pBT->chEntryEncode );   // initialise for compression
         pBT->fTerse = TRUE;
      }
      else
      {
         pBT->fTerse = FALSE;
      } /* endif */
      /******************************************************************/
      /* use the free collating sequence buffer to store our vital info */
      /* Its taken care, that the structure will not jeopardize the     */
      /* header...                                                      */
      /******************************************************************/
      memcpy( pBT->chCollate, pNTMVitalInfo, sizeof(NTMVITALINFO));
      /******************************************************************/
      /* new key compare routine ....                                   */
      /******************************************************************/
      pBT->compare = NTMKeyCompare;
    } /* endif */

    /* Allocate space for LookupTable */
    pBT->usNumberOfLookupEntries = 0;
    if ( pBT->bRecSizeVersion == BTREE_V3 )
    {
      UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V3), NOMSG );

      if ( pBT->LookupTable_V3 )
      {
        /* Allocate space for AccessCtrTable */
        UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( !pBT->AccessCtrTable )
        {
          UtlAlloc( (PVOID *)&pBT->LookupTable_V3, 0L, 0L, NOMSG );
          sRc = BTREE_NO_ROOM;
        }
        else
        {
          pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    }
    else
    {
      UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V2), NOMSG );

      if ( pBT->LookupTable_V2 )
      {
        /* Allocate space for AccessCtrTable */
        UtlAlloc( (PVOID *)&pBT->AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( !pBT->AccessCtrTable )
        {
          UtlAlloc( (PVOID *)&pBT->LookupTable_V2, 0L, 0L, NOMSG );
          sRc = BTREE_NO_ROOM;
        }
        else
        {
          pBT->usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */

    pBT->usNumberOfAllocatedBuffers = 0;

    if ( !sRc )
    {
      sRc =  QDAMAllocTempAreas( pBTIda );
    } /* endif */


    if (! sRc )
    {
      /****************************************************************/
      /* in order to initialize the area of the first record we       */
      /* write an empty record buffer to the file before writing the  */
      /* header                                                       */
      /****************************************************************/
      if ( pBT->bRecSizeVersion == BTREE_V3 )
      {
        USHORT usBytesWritten;
        PBTREEBUFFER_V3 pbuffer;
        UtlAlloc( (PVOID *)&pbuffer, 0L, (LONG) BTREE_BUFFER_V3 , NOMSG );
        if ( ! pbuffer )
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
        else
        {
          UtlWrite( pBT->fp, (PVOID)pbuffer, BTREE_REC_SIZE_V3, &usBytesWritten, FALSE );

          UtlAlloc( (PVOID *)&pbuffer, 0L, 0L , NOMSG );

          sRc = QDAMWriteHeader( pBTIda );
        } /* endif */
      }
      else
      {
        USHORT usBytesWritten;
        PBTREEBUFFER_V2 pbuffer;
        UtlAlloc( (PVOID *)&pbuffer, 0L, (LONG) BTREE_BUFFER_V2 , NOMSG );
        if ( ! pbuffer )
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
        else
        {
          UtlWrite( pBT->fp, (PVOID)pbuffer, BTREE_REC_SIZE_V2, &usBytesWritten, FALSE );

          UtlAlloc( (PVOID *)&pbuffer, 0L, 0L , NOMSG );

          sRc = QDAMWriteHeader( pBTIda );
        } /* endif */
      } /* endif */

      if (! sRc )
      {
        sRc = QDAMDictUpdSignLocal(pBTIda, pUserData, usLen );
      } /* endif */

      if ( !sRc )
      {
        /*******************************************************************/
        /* Write out an empty root node                                    */
        /*******************************************************************/
        if ( pBT->bRecSizeVersion == BTREE_V3 )
        {
          PBTREEBUFFER_V3 pRecord;
          sRc = QDAMReadRecord_V3(pBTIda, pBT->usFirstNode, &pRecord, TRUE );
          if ( !sRc )
          {
            #ifdef TEMPORARY_COMMENTED
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V3(pBTIda,pRecord);
            #endif
          } /* endif */
          if ( !sRc )
          {
            #ifdef TEMPORARY_COMMENTED
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
            #endif
            if (! sRc )
            {
                pBT->usFirstDataBuffer = pBT->usNextFreeRecord;
            } /* endif */
          } /* endif */
        }
        else
        {
          PBTREEBUFFER_V2 pRecord;
          #ifdef TEMPORARY_COMMENTED
          sRc = QDAMReadRecord_V2(pBTIda, pBT->usFirstNode, &pRecord, TRUE );
          #endif
          if ( !sRc )
          {

            #ifdef TEMPORARY_COMMENTED
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V2(pBTIda,pRecord);
            #endif
          } /* endif */
          if ( !sRc )
          {
            #ifdef TEMPORARY_COMMENTED
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
            #endif
            if (! sRc )
            {
                pBT->usFirstDataBuffer = pBT->usNextFreeRecord;
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  }
  else
  {
    switch ( sRc )
    {
      case  ERROR_INVALID_DRIVE:
        sRc = BTREE_INVALID_DRIVE;
        break;
      #ifdef TEMPORARY_COMMENTED
      case  ERROR_OPEN_FAILED :
        sRc = BTREE_OPEN_FAILED;
        break;
        #endif
      case  ERROR_NETWORK_ACCESS_DENIED:
        sRc = BTREE_NETWORK_ACCESS_DENIED;
        break;
      default :
        sRc = BTREE_OPEN_ERROR;
        break;
    } /* endswitch */
  } /* endif */

  if ( !sRc )
  {
     *ppBTIda = pBTIda;                          // return pointer to BTree
     /****************************************************************/
     /* add this dictionary to our list ...                          */
     /****************************************************************/
     sRc = QDAMAddDict( pName, pBTIda );
  }
  else
  {
    #ifdef TEMPORARY_COMMENTED
    // leave the return code from create
    // file will not be destroyed if create failed, since then
    // filename is not set
    QDAMDestroy( pBTIda );
    #endif
  } /* endif */

  if ( sRc )
  {
    #ifdef TEMPORARY_COMMENTED
    ERREVENT2( QDAMDICTCREATELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
    #endif
  } /* endif */


  return( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictInsertLocal   Insert entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictInsertLocal( PBTREE, PCHAR, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Add a key and all associated data.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE          pointer to btree structure
//                    PCHAR           key to be inserted
//                    PCHAR           user data to be associated with the key
//                    USHORT          length of the user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_DATA_RANGE  key to long or too short
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      find the appropriate record and set Rc accordingly
//                    endif
//                    if okay then
//                      add data to buffer and set Rc
//                      if okay then
//                        insert the key and the data and set Rc
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT QDAMDictInsertLocal
(
  PBTREE  pBTIda,           // pointer to binary tree struct
  PCHAR_W pKey,             // pointer to key data
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data in bytes
)
{
   SHORT sRc = 0;                            // return code
   RECPARAM   recData;                       // offset/node of data storage
   RECPARAM   recKey;                        // offset/node of key storage
   USHORT     usKeyLen;                      // length of the key
   BOOL       fLocked = FALSE;               // file-has-been-locked flag
   PBTREEGLOB    pBT = pBTIda->pBTree;

   memset( &recKey, 0, sizeof( recKey ) );
   memset( &recData,  0, sizeof( recData ) );
   #ifdef TEMPRORARY_COMMENTED
   DEBUGEVENT2( QDAMDICTINSERTLOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );
   #endif

   if ( pBT->fCorrupted )
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
   #ifdef TEMPORARY_COMMENTED
   if ( !sRc && QDAMDictLockStatus( pBTIda, pKey ) )
   {
     sRc = BTREE_ENTRY_LOCKED;
   } /* endif */ 
   #endif

   /*******************************************************************/
   /* For shared databases: lock complete file                        */
   /*                                                                 */
   /* Note: this will also update our internal buffers and the        */
   /*       header record. No need to call QDamCheckForUpdates here.  */
   /*******************************************************************/
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     #ifdef TEMPORARY_COMMENTED
     sRc = QDAMPhysLock( pBTIda, TRUE, &fLocked );
     #endif
   } /* endif */

   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pNode = NULL;

      if ( !sRc )
      {
        usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : UTF16strlenBYTE( pKey ));
        if ( (usKeyLen == 0) ||
             ((usKeyLen >= HEADTERM_SIZE * sizeof(CHAR_W))) ||
             (ulLen == 0) ||
             ((pBT->usVersion < NTM_VERSION2) && (ulLen >= MAXDATASIZE)) )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKey, usKeyLen+sizeof(CHAR_W) );   // save current data
          #ifdef TEMPORARY_COMMENTED
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V3( pBTIda, pKey, &pNode );
          #endif
        } /* endif */
      } /* endif */

      if ( !sRc )
      {
        #ifdef TEMPORARY_COMMENTED
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V3( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V3 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );
        #endif
          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
      }
   }
   else
   {
      PBTREEBUFFER_V2  pNode = NULL;

      if ( !sRc )
      {
        usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : UTF16strlenBYTE( pKey ));
        if ( usKeyLen == 0 ||(  (usKeyLen >= HEADTERM_SIZE * sizeof(CHAR_W))) || ulLen == 0 || ulLen >= MAXDATASIZE )
        {
          sRc = BTREE_DATA_RANGE;
        }
        else
        {
          memcpy( (PBYTE)pBTIda->chHeadTerm, (PBYTE)pKey, usKeyLen+sizeof(CHAR_W) );   // save current data
          #ifdef TEMPORARY_COMMENTED
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V2( pBTIda, pKey, &pNode );
          #endif
        } /* endif */
      } /* endif */

      if ( !sRc )
      {
        #ifdef TEMPORARY_COMMENTED
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V2( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V2 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );
        #endif
          /****************************************************************/
          /* change time to indicate modifications on dictionary...       */
          /****************************************************************/
          pBT->lTime ++;
      }
   } /* endif */

   /*******************************************************************/
   /* For shared databases: unlock complete file                      */
   /*                                                                 */
   /* Note: this will also incement the dictionary update counter     */
   /*       if the dictionary has been modified                       */
   /*******************************************************************/
   if ( fLocked )
   {
     #ifdef TEMPORARY_COMMENTED
     sRc = QDAMPhysLock( pBTIda, FALSE, NULL );
     #endif
   } /* endif */

   if ( sRc )
   {
     #ifdef TEMPORARY_COMMENTED
     ERREVENT2( QDAMDICTINSERTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
     #endif
   } /* endif */

   return sRc;
}


#ifdef TEMPORARY_COMMENTED
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
#ifdef TEMPORARY_COMMENTED
   DEBUGEVENT( EQFNTMINSERT_LOC, FUNCENTRY_EVENT, 0 );
#endif
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
      ulKey = *pulKey = (NTMNEXTKEY( pBT ))++;
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
      sRc = QDAMDictInsertLocal( (PBTREE) pBTIda, (PSZ_W) pulKey, pData, ulLen );
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
    #ifdef TEMPORARY_COMMENTED
    ERREVENT( EQFNTMINSERT_LOC, INTFUNCFAILED_EVENT, sRc );
    #endif
  } /* endif */

  return sRc;
} /* end of function EQFNTMInsert */
#endif

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdSignLocal Write User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Writes the second part of the first record (user data)
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    USHORT                 length of user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     if tree is corrupted
//                      set sRC = BTREE_CORRUPTED
//                    else
//                      give 1K at beginning as space (UtlChgFilePtr)
//                    endif
//                    if ok then
//                      check if length of user data is correct
//                      if ok then
//                        write length of UserData to disk
//                        check if disk is full
//                        if ok
//                          write pUserData to the disk
//                          check if disk is full
//                        endif
//                      endif
//                      if ok
//                        fill rest up with zeros
//                        check if disk is full
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictUpdSignLocal
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   ULONG  ulLen                        // length of user data
)
{
  SHORT  sRc=0;                        // return code
  ULONG   ulDataLen;                   // number of bytes to be written
  USHORT  usBytesWritten;              // bytes written to disk
  PCHAR   pchBuffer;                   // pointer to buffer
  LONG    lFilePos;                    // file position to position at
  ULONG   ulNewOffset;                 // new offset
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  } /* endif */
  if ( !sRc && !pBT->fOpen )
  {
    sRc = BTREE_READONLY;
  }
  else
  {
     // let 2K at beginning as space
     lFilePos = (LONG) USERDATA_START;
     sRc = UtlChgFilePtr( pBT->fp, lFilePos, FILE_BEGIN,
                          &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    if ( pBT->bRecSizeVersion == BTREE_V3 )
    {
      ulDataLen = ulLen < BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT)? ulLen : BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT);
    }
    else
    {
      ulDataLen =  ulLen < BTREE_REC_SIZE_V2 - USERDATA_START - sizeof(USHORT)? ulLen : BTREE_REC_SIZE_V2 - USERDATA_START - sizeof(USHORT);
    } /* endif */
    if ( ulDataLen < ulLen )
    {
       sRc = BTREE_USERDATA;
    }
    else
    {
       if ( !pUserData  )
       {
         pUserData = "";
         ulDataLen = 0;
       } /* endif */

       /*************************************************************/
       /*  write length of userdata                                 */
       /*************************************************************/
       {
         USHORT usDataLen = (USHORT)ulDataLen;
         sRc = UtlWrite( pBT->fp, &usDataLen, sizeof(USHORT), &usBytesWritten, FALSE );
         ulDataLen = usDataLen;
       }

       // check if disk is full
       if ( ! sRc )
       {
          if (  usBytesWritten != sizeof(USHORT) )
          {
             sRc = BTREE_DISK_FULL;
          } /* endif */
       }
       else
       {
          sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       } /* endif */
       if ( ! sRc)
       {
         /***********************************************************/
         /* write user data itselft                                 */
         /***********************************************************/
          sRc = UtlWrite( pBT->fp, pUserData, (USHORT)ulDataLen,
                          &usBytesWritten, FALSE );
          // check if disk is full
          if ( ! sRc )
          {
             if (  usBytesWritten != ulDataLen )
             {
                sRc = BTREE_DISK_FULL;
             } /* endif */
          }
          else
          {
             sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
          } /* endif */
       } /* endif */
    } /* endif */
    if ( ! sRc )
    {
       // fill rest up with zeros
       if ( pBT->bRecSizeVersion == BTREE_V3 )
       {
         ulDataLen = BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT) - ulDataLen;
       }
       else
       {
         ulDataLen = BTREE_REC_SIZE_V2 - USERDATA_START - sizeof(USHORT) - ulDataLen;
       } /* endif */
       UtlAlloc( (PVOID *)&pchBuffer, 0L, (LONG) ulDataLen, NOMSG );
       if ( pchBuffer )
       {
          sRc = UtlWrite( pBT->fp, pchBuffer, (USHORT)ulDataLen, &usBytesWritten, FALSE );
          // check if disk is full
          if ( ! sRc )
          {
             if ( usBytesWritten != ulDataLen )
             {
                sRc = BTREE_DISK_FULL;
             } /* endif */
          }
          else
          {
             sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
          } /* endif */
          // free the buffer
          UtlAlloc( (PVOID *)&pchBuffer, 0L, 0L, NOMSG );
       }
       else
       {
          sRc = BTREE_NO_ROOM;
       } /* endif */
    } /* endif */
  } /* endif */

   if ( sRc )
   {
    #ifdef TEMPORARY_COMMENTED
     ERREVENT2( QDAMUPDSIGNLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
     #endif
   } /* endif */

  return sRc;
}


