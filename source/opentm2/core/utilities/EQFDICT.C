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
#include "core/utilities/LogWrapper.h"

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

/**********************************************************************/
/* Defines the header of the event list either as a structure or as   */
/* a enumeration                                                      */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTLISTSTART( type, var ) \
     EVENTSTRING var[] = {
#else
   #define EVENTLISTSTART( type, var) typedef enum type {
#endif

#ifdef STRING_EVENT_IDS
   #define EVENTLISTEND( type, dummy ) { #dummy, 32000 } };
#else
   #define EVENTLISTEND( type, dummy ) dummy = 32000} type;
#endif


/**********************************************************************/
/* Macro to either define the event ID as a numeric value or to       */
/* use the event ID as a string (used by EVENTLOG program).           */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTDEF( id, num ) { #id, num },
#else
   #define EVENTDEF( id, num ) id = num,
#endif


/**********************************************************************/
/* List of event locations (only use macro EVENTDEF for entries in        */
/* this list)                                                         */
/**********************************************************************/
EVENTLISTSTART( _EVENTLOC, EventLocations )
EVENTDEF( UTLERROR_LOC,              1 )
EVENTDEF( TWBSTART_LOC,              2 )
EVENTDEF( TWBSHUTDOWN_LOC,           3 )
EVENTDEF( QDAMUPDATELOCKREC_LOC,     4 )
EVENTDEF( QDAMGETUPDCOUNTER_LOC,     5 )
EVENTDEF( QDAMINCRUPDCOUNTER_LOC,    6 )
EVENTDEF( QDAMCHECKFORUPDATES_LOC,   7 )
EVENTDEF( QDAMPHYSLOCK_LOC,          8 )
EVENTDEF( QDAMRECORDTODISK_LOC,      9 )
EVENTDEF( QDAMDICTCREATELOCAL_LOC,   10 )
EVENTDEF( QDAMREADRECORD_LOC,        12 )
EVENTDEF( QDAMWRITEHEADER_LOC,       13 )
EVENTDEF( QDAMDICTOPENLOCAL_LOC,     14 )
EVENTDEF( QDAMDICTCLOSELOCAL_LOC,    15 )
EVENTDEF( QDAMDICTEXACTLOCAL_LOC,    16 )
EVENTDEF( QDAMDICTSUBSTRLOCAL_LOC,   17 )
EVENTDEF( QDAMDICTEQUIVLOCAL_LOC,    18 )
EVENTDEF( QDAMDICTFIRSTLOCAL_LOC,    19 )
EVENTDEF( QDAMDICTNEXTLOCAL_LOC,     20 )
EVENTDEF( QDAMDICTPREVLOCAL_LOC,     21 )
EVENTDEF( QDAMDICTCURRENTLOCAL_LOC,  22 )
EVENTDEF( QDAMNEXT_LOC,              23 )
EVENTDEF( QDAMFIRST_LOC,             24 )
EVENTDEF( QDAMGETSZKEYPARAM_LOC,     25 )
EVENTDEF( QDAMGETSZDATA_LOC,         26 )
EVENTDEF( QDAMLOCATEKEY_LOC,         27 )
EVENTDEF( QDAMVALIDATEINDEX_LOC,     28 )
EVENTDEF( NLPBACKWARDLIST_LOC,       29  )
EVENTDEF( NLPFORWARDDLIST_LOC,       30 )
EVENTDEF( NLPTERMLIST_LOC,           31 )
EVENTDEF( NLPLOCKENTRY_LOC,          32 )
EVENTDEF( NLPDELENTRYASD_LOC,        33 )
EVENTDEF( NLPUPDENTRYASD_LOC,        34 )
EVENTDEF( NLPINSENTRYASD_LOC,        35 )
EVENTDEF( NLPPRVTERMASD_LOC,         36 )
EVENTDEF( NLPNXTTERMASD_LOC,         37 )
EVENTDEF( NLPFNDNUMBERASD_LOC,       38 )
EVENTDEF( NLPRETENTRYASD_LOC,        39 )
EVENTDEF( NLPFNDMATCHASD_LOC,        40 )
EVENTDEF( NLPFNDEQUIVASD_LOC,        41 )
EVENTDEF( NLPFNDBEGINASD_LOC,        42 )
EVENTDEF( QDAMOPENASD_LOC,           43 )
EVENTDEF( QDAMWRITERECORD_LOC,       44 )
EVENTDEF( QDAMNEWRECORD_LOC,         45 )
EVENTDEF( QDAMFREERECORD_LOC,        46 )
EVENTDEF( QDAMUPDSIGNLOCAL_LOC,      47 )
EVENTDEF( QDAMDICTFLUSHLOCAL_LOC,    48 )
EVENTDEF( QDAMSPLITNODE_LOC,         49 )
EVENTDEF( QDAMINSERTKEY_LOC,         50 )
EVENTDEF( QDAMDICTINSERTLOCAL_LOC,   51 )
EVENTDEF( QDAMCHANGEKEY_LOC,         52 )
EVENTDEF( QDAMADDTOBUFFER_LOC,       53 )
EVENTDEF( QDAMDELETEDATAFROMBUFFER_LOC, 54 )
EVENTDEF( QDAMDICTUPDATELOCAL_LOC,   55 )
EVENTDEF( QDAMDICTCOPYLOCAL_LOC,     56 )
EVENTDEF( QDAMREDUCENODE_LOC,        57 )
EVENTDEF( QDAMDICTDELETELOCAL_LOC,   58 )
EVENTDEF( QDAMREADRECORDFROMDISK_LOC,59 )
EVENTDEF( EQFNTMUPDATE_LOC,          60 )
EVENTDEF( EQFNTMINSERT_LOC,          61 )
EVENTDEF( EQFNTMGET_LOC,             62 )
EVENTDEF( EQFNTMPHYSLOCK_LOC,        63 )
EVENTDEF( TMTXREPLACE_LOC,           64 )
EVENTDEF( TOKENIZESOURCE_LOC,        65 )
EVENTDEF( TOKENIZETARGET_LOC,        66 )
EVENTDEF( ADDTOTM_LOC,               67 )
EVENTDEF( FILLCLB_LOC,               68 )
EVENTDEF( DETERMINETMRECORD_LOC,     69 )
EVENTDEF( UPDATETMRECORD_LOC,        70 )
EVENTDEF( COMPAREPUTDATA_LOC,        71 )
EVENTDEF( ADDTMTARGET_LOC,           72 )
EVENTDEF( REPLACETMTARGET_LOC,       73 )
EVENTDEF( TMTXGET_LOC,               74 )
EVENTDEF( GETEXACTMATCH_LOC,         75 )
EVENTDEF( EXACTTEST_LOC,             76 )
EVENTDEF( FILLMATCHTABLE_LOC,        77 )
EVENTDEF( GETFUZZYMATCH_LOC,         78 )
EVENTDEF( FUZZYTEST_LOC,             79 )
EVENTDEF( FILLMATCHENTRY_LOC,        80 )
EVENTDEF( NTMREADWRITESEGMENT_LOC,   81 )
EVENTDEF( NTMLOCKTM_LOC,             82 )
EVENTDEF( NTMCHECKFORUPDATES_LOC,    83 )
EVENTDEF( TMOPEN_LOC,                84 )
EVENTDEF( TMCLOSE_LOC,               85 )
EVENTDEF( TMREPLACE_LOC,             86 )
EVENTDEF( TMGET_LOC,                 87 )
EVENTDEF( TMTXOPEN_LOC,              88 )
EVENTDEF( EQFNTMOPEN_LOC,            89 )
EVENTDEF( EQFNTMCLOSE_LOC,           90 )
EVENTDEF( EQFNTMSIGN_LOC,            91 )
EVENTDEF( UTILSEXIT_LOC,             92 )
EVENTDEF( QDAMLOCSUBSTR_LOC,         93 )
EVENTDEF( QDAMGETSZKEY_LOC,          94 )
EVENTDEF( QDAMCHECKCHECKSUM_LOC,     95 )
EVENTDEF( QDAMFINDRECORD_LOC,        96 )
EVENTDEF( QDAMSETRECDATA_LOC,        97 )
EVENTDEF( EQFDA_LOC,                 98 )
EVENTDEF( DICTCLEANUP_LOC,           99 )
EVENTDEF( EQFDACLOSE_LOC,           100 )
EVENTDEF( ASDOPEN_LOC,              101 )
EVENTDEF( ASDCLOSE_LOC,             102 )
EVENTDEF( ASDTRANSLATE_LOC,         103 )
EVENTDEF( TMUPDSEG_LOC,             104 )
EVENTDEF( DOCEXPCOMMAND_LOC,        105 )
EVENTDEF( DOCUMENTUNLOAD_LOC,       106 )
EVENTDEF( UNLOADDOCUMENT_LOC,       107 )
EVENTDEF( EQFUNSEGREVMARK_LOC,      108 )
EVENTDEF( UNSEGMENT_LOC,            109 )
EVENTDEF( ASDLISTINDEX_LOC,         110 )
EVENTDEF( NTMGETIDFROMNAME_LOC,     111 )
EVENTDEF( TWBEXCEPTION_LOC,         112 )
EVENTLISTEND( TEVENTLOC, DUMMY_LOC )


/**********************************************************************/
/* Group/Area of events                                               */
/**********************************************************************/
EVENTLISTSTART( _EVENTGROUP, EventGroups )
EVENTDEF( NONE_GROUP,           0 )
EVENTDEF( TM_GROUP,             1 )
EVENTDEF( DICT_GROUP,           2 )
EVENTDEF( ANALYSIS_GROUP,       3 )
EVENTDEF( MORPH_GROUP,          4 )
EVENTDEF( DOC_GROUP,            5 )
EVENTDEF( GENERAL_GROUP,        6 )
EVENTDEF( DB_GROUP,             7 )
EVENTLISTEND( TEVENTGROUP, DUMMY_GROUP )

#define  MINFREEKEYS        3
// minimum keys to be in a record - otherwise try to reduce the node
#define  MIN_KEY            MINFREEKEYS
// lock a record for fixing it in the memory
#define  BTREELOCKRECORD(x)  ((x)->fLocked=TRUE)
// test if a record is locked
#define  BTREETESTLOCKRECORD(x)  ((x)->fLocked)
// unlock a (previously locked) record
#define  BTREEUNLOCKRECORD(x)  ((x)->fLocked=FALSE)

/**********************************************************************/
/* List of event classes                                              */
/**********************************************************************/
EVENTLISTSTART( _EVENTCLASS, EventClasses )
EVENTDEF( NA_EVENT,             1 )
EVENTDEF( FUNCENTRY_EVENT,      2 )
EVENTDEF( REFRESH_EVENT,        5 )
EVENTDEF( INTFUNCFAILED_EVENT,  6 )
EVENTDEF( MSGBOX_EVENT,         7 )
EVENTDEF( INVOPERATION_EVENT,   8 )
EVENTDEF( READ_EVENT,           9 )
EVENTDEF( WAIT_EVENT,           10 )
EVENTDEF( STATE_EVENT,          11 )
EVENTDEF( FUNCEXIT_EVENT,       12 )
EVENTDEF( CHECKSUM_EVENT,       13 )
EVENTLISTEND( TEVENTCLASS, DUMMY_CLASS )

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

/* Access Macros */
#define TYPE(x)          ((x)->contents.header.chType)
#define LEAF_NODE        0x00
#define INNER_NODE       0x01
#define ROOT_NODE        0x02
#define DATA_NODE        0x04
#define DATA_NEXTNODE    0x08
#define DATA_KEYNODE     0x10

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


/**********************************************************************/
/* Event types                                                        */
/**********************************************************************/
#ifndef STRING_EVENT_IDS
typedef enum
{
  ERROR_EVENT,                         // event is caused by an error condition
  INFO_EVENT,                          // informational events (e.g. status events)
  EQFDEBUG_EVENT                       // only logged when debug version is build
} EVENTTYPE;
#endif

/**********************************************************************/
/* Event macros                                                       */
/**********************************************************************/
BOOL UtlEvent2
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC,                      // the event return code or error code
  SHORT sGroup,                        // the event group
  PSZ   pszAddData                     // additional data for event (string)
);
#ifndef STRING_EVENT_IDS
  #ifndef NO_ERREVENTS
    #define ERREVENT( id, class, rc ) \
      UtlEvent( ERROR_EVENT, id, class, rc )
  #else
    #define ERREVENT( id, class, rc )
  #endif

  #ifndef NO_ERREVENTS
    #define ERREVENT2( id, class, rc, gr, str ) \
      UtlEvent2( ERROR_EVENT, id, class, rc, gr, str )
  #else
    #define ERREVENT2( id, class, rc, gr, str )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT( id, class, rc ) \
      UtlEvent( INFO_EVENT, id, class, rc )
  #else
    #define INFOEVENT( id, class, rc )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT2( id, class, rc, gr, str ) \
      UtlEvent2( INFO_EVENT, id, class, rc, gr, str )
  #else
    #define INFOEVENT2( id, class, rc, gr, str )
  #endif

  #ifndef _DEBUG
    #define NO_DEBUGEVENTS
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT( id, class, rc ) \
      UtlEvent( EQFDEBUG_EVENT, id, class, rc )
  #else
    #define DEBUGEVENT( id, class, rc )
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT2( id, class, rc, gr, str ) \
      UtlEvent2( EQFDEBUG_EVENT, id, class, rc, gr, str )
  #else
    #define DEBUGEVENT2( id, class, rc, gr, str )
  #endif
#endif


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

  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );


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
    ERREVENT2( QDAMWRITEHEADER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCEXIT_EVENT, 0, DB_GROUP, NULL );


  return sRc;
}


SHORT QDAMDictUpdSignLocal
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   ULONG  ulLen                        // length of user data
);


// Convert Dos... return codes to BTREE return codes
SHORT QDAMDosRC2BtreeRC
(
  SHORT sDosRC,                        // Dos return code
  SHORT sDefaultRC,                    // RC for default case
  USHORT usOpenFlags                   // open flags of database
)
{

   SHORT sRc = 0;                           // converted return code
  LogMessage(FATAL, "called commented out function QDAMDosRC2BtreeRC");
  #ifdef TEMPORARY_COMMENTED
  switch ( sDosRC )
  {
     case  NO_ERROR:
       sRc = NO_ERROR;
      break;
    case  ERROR_INVALID_DRIVE:
      sRc = BTREE_INVALID_DRIVE;
      break;
    case  ERROR_OPEN_FAILED :
      sRc = BTREE_OPEN_FAILED;
      break;
    case  ERROR_NETWORK_ACCESS_DENIED:
    case  ERROR_VC_DISCONNECTED:
      sRc = BTREE_NETWORK_ACCESS_DENIED;
      break;
    case ERROR_ACCESS_DENIED:
      if ( usOpenFlags & ASD_SHARED )
      {
        // map ACCESS_DENIED to IN_USE for shared databases as
        // a locked local file returns ERROR_ACCESS_DENIED rather than
        // ERROR_SHARING_VIOLATION
        sRc = BTREE_IN_USE;
      }
      else
      {
        sRc = BTREE_ACCESS_ERROR;
      } /* endif */
      break;
    case ERROR_DRIVE_LOCKED:
    case ERROR_INVALID_ACCESS:
      sRc = BTREE_ACCESS_ERROR;
      break;
    case ERROR_FILE_NOT_FOUND:
      sRc = BTREE_FILE_NOTFOUND;
      break;
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
      sRc = BTREE_IN_USE;
      break;
    default :
      sRc = sDefaultRC;
      break;
  } /* endswitch */
  #endif

  return( sRc );
} /* end of function QDAMDosRC2BtreeRC  */


SHORT QDAMWRecordToDisk_V3
(
   PBTREE     pBTIda,                     // pointer to btree structure
   PBTREEBUFFER_V3  pBuffer               // pointer to buffer to write
)
{
  SHORT sRc = 0;                          // return code
  LogMessage(FATAL, "called commented out function QDAMWRecordToDisk_V3");
  #ifdef TEMPORARY_COMMENTED
  LONG  lOffset;                          // offset in file
  ULONG ulNewOffset;                      // new file pointer position
  USHORT usBytesWritten;                  // number of bytes written
  PBTREEGLOB  pBT = pBTIda->pBTree;

  INC_REAL_WRITE_COUNT;

  DEBUGEVENT2( QDAMRECORDTODISK_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

//QDAMCheckCheckSum( pBuffer, QDAMRECORDTODISK_LOC );

  if ( !sRc )
  {
    lOffset = RECORDNUM(pBuffer) * (long)BTREE_REC_SIZE_V3;

    sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &pBuffer->contents, BTREE_REC_SIZE_V3, &usBytesWritten, FALSE );

    // check if disk is full
    if ( ! sRc )
    {
       if ( usBytesWritten == BTREE_REC_SIZE_V3 )
       {
          pBuffer->fNeedToWrite = FALSE;
       }
       else
       {
          sRc = BTREE_DISK_FULL;
          pBT->fCorrupted = TRUE;                     // indicate corruption
       } /* endif */
    }
    else
    {
       sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       pBT->fCorrupted = TRUE;                     // indicate corruption
    } /* endif */
  }
  else
  {
    sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    pBT->fCorrupted = TRUE;                        // indicate corruption
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMRECORDTODISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  #endif
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetUpdCounter       Get database update counter
//------------------------------------------------------------------------------
// Function call:     QDAMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );
//------------------------------------------------------------------------------
// Description:       Get one or more of the the database update counters
//                    from the dummy/locked terms file
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PLONG                  ptr to buffer for update counter
//                    SHORT                  index of requested update counter
//                    SHORT                  number of counters requested
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_READ_RROR   read error from disk
//
//------------------------------------------------------------------------------
// Function flow:     read update counter from dummy file
//------------------------------------------------------------------------------
 SHORT QDAMGetUpdCounter
(
   PBTREE     pBTIda,                   // pointer to btree structure
   PLONG      plUpdCount,               // ptr to buffer for update counter
   SHORT      sIndex,                   // index of requested update counter
   SHORT      sNumCounters              // number of counters requested
)
{
  SHORT  sRc=0;
  
  LogMessage(FATAL, "called commented out function QDAMGetUpdCounter");
  #ifdef TEMPORARY_COMMENTED

  USHORT usNumBytes;                   // number of bytes written or read
  ULONG  ulNewOffset;                  // new offset in file
  PBTREEGLOB  pBT = pBTIda->pBTree;
  SHORT      sRetries;                 // number of retries

  sRetries = 0; // MAX_RETRY_COUNT;
  do
  {
     // Position to requested update counter
    sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                         FILE_BEGIN, &ulNewOffset, FALSE);
    sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    // Read requested update counter(s)
    if ( !sRc )
    {
      memset( plUpdCount, 0, sizeof(LONG)*sNumCounters );

      sRc = UtlRead( pBT->fpDummy, (PVOID)plUpdCount,
                     (USHORT)(sizeof(LONG) * sNumCounters),
                     &usNumBytes, FALSE);
      sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     if ( sRc == BTREE_IN_USE )
     {
  //   UtlWait( MAX_WAIT_TIME );
       sRetries--;
     } /* endif */

  // if ( sRetries != MAX_RETRY_COUNT )
  // {
  //   DEBUGEVENT2( QDAMGETUPDCOUNTER_LOC, WAIT_EVENT, (MAX_RETRY_COUNT - sRetries), DB_GROUP, NULL );
  // } /* endif */
  } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );

  if ( sRc )
  {
    ERREVENT2( QDAMGETUPDCOUNTER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  #endif
  return sRc;
}

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
  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

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
      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, NULL );
      sRc = UtlRead( pBT->fp,
                     (PVOID)&pBuffer->contents,
                     BTREE_REC_SIZE_V3, &usNumBytesRead, FALSE);
      
      if (sRc ) 
        sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );


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

           sRc = QDAMWRecordToDisk_V3(pBTIda, pBuffer);

           if (! sRc )
           {
             if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
             {
               // this condition should NEVER occur for shared resources
               // as write is only allowed if the database has been
               // previously locked
               ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 2, DB_GROUP, NULL );

              LogMessage(ERROR, "Gotcha! Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/2\nPlease save info required to reproduce this condition.");

             } /* endif */
             sRc = QDAMWriteHeader( pBTIda );
           } /* endif */
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
         }
         else
         {
           ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, NULL );

          LogMessage(ERROR, "EQF9998: Write of new QDAM record w/o fNewRec set!\nLoc=READRECORDFROMDISK/3\nInternal Error");

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
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );

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
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
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
  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

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
                ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
                LogMessage(ERROR, "Gotcha!Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/1\nPlease save info required to reproduce this condition.");

              } /* endif */
              sRc = QDAMWRecordToDisk_V3(pBTIda, pLEntry->pBuffer);
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
    ERREVENT2( QDAMREADRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
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
  LogMessage(FATAL, "called commented out function QDAMAddDict");
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
// Function name:     QDAMDestroy     Destroy dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDestroy( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Destroy the file if something went wrong during the
//                    create
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//
//------------------------------------------------------------------------------
// Function flow:     if ptr to BTree does not exist
//                      set RC = BTREE_INVALID
//                    else
//                      if filename exists
//                        call QDAMDictClose
//                        delete the file 'filename'
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDestroy
(
   PBTREE  pBTIda                   // pointer to generic structure
)
{
   SHORT sRc = 0;                   // return code

   LogMessage(FATAL, "called commented out function QDAMDestroy");
  #ifdef TEMPORARY_COMMENTED
   CHAR  chName[ MAX_EQF_PATH ];    // file name
   PBTREEGLOB  pBT = pBTIda->pBTree;

   if ( !pBT )
   {
     sRc = BTREE_INVALID;
   }
   else if (* (pBT->chFileName) )
   {
     //
     // QDAMDictClose frees up all of the memory associated with a B-tree,
     // we need to save the filename as we'll be deleteing the file by name
     //
     strcpy(chName, pBT->chFileName);
     sRc = QDAMDictClose( &pBTIda );
     if ( chName[0] )
     {
        UtlDelete( chName, 0L, FALSE);
     }
   }
  #endif
   return ( sRc );
}


SHORT QDAMWriteRecord_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pBuffer
)
{
   SHORT  sRc = 0;                               // return code

   LogMessage(FATAL, "called commented out function QDAMWriteRecord_V3");
  #ifdef TEMPORARY_COMMENTED
   PBTREEGLOB  pBT = pBTIda->pBTree;

   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

   INC_WRITE_COUNT;

   /********************************************************************/
   /* write header with corruption flag -- if not yet done             */
   /********************************************************************/
   if ( pBT->fWriteHeaderPending )
   {
     sRc = QDAMWriteHeader( pBTIda );
     pBT->fWriteHeaderPending = FALSE;
   } /* endif */

   //  if guard flag is on write it to disk
   //  else set only the flag
   if ( !sRc )
   {
     if ( !pBT->fGuard )
     {
        pBuffer->fNeedToWrite = TRUE;
     }
     else
     {
        sRc = QDAMWRecordToDisk_V3(pBTIda, pBuffer);
     } /* endif */
   } /* endif */


   // For shared databases only: set internal update flag, the
   // update counter will be written once the file lock is removed
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     pBT->fUpdated = TRUE;;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMWRITERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCEXIT_EVENT, sRc, DB_GROUP, NULL );
   
   #endif
   
   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecordFromDisk   Read record from disk
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecordFromDisk( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record from disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to read
//                    USHORT             number of buffer to read
//                    PPBTREERECORD      The buffer read
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most
//                    performance critical routines in the module.
//
//------------------------------------------------------------------------------
// Function flow:     allocate memory for a new buffer and set pBuffer
//                    if still okay
//                      position in file and set sRc;
//                      if still okay then
//                        read buffer from disk, set sRc;
//                      endif
//                      if still okay then
//                        if new record then
//                          init header information
//                        endif
//                        set pointer to next buffer to be looked at
//                      else
//                        set BTREE_READ_ERROR
//                      endif
//------------------------------------------------------------------------------
SHORT QDAMReadRecordFromDisk_V2
(
   PBTREE         pBTIda,
   USHORT         usNumber,
   PBTREEBUFFER_V2 * ppReadBuffer,
   BOOL           fNewRec              // allow new records flag
)
{
  USHORT    usNumBytesRead = 0;                   // number of bytes read
  PBTREEHEADER pHeader;                          // pointer to header
  LONG     lOffset;                              // file offset to be set
  ULONG    ulNewOffset;                          // new position
  PBTREEBUFFER_V2   pBuffer = NULL;
  SHORT    sRc = 0;                             // return code
  
  LogMessage(FATAL, "called commented out function QDAMReadRecordFromDisk_V2");
  #ifdef TEMPORARY_COMMENTED
  PBTREEGLOB  pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V2 pLEntry = NULL;

  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

  /********************************************************************/
  /* Allocate space for a new buffer and let pBuffer point to it      */
  /********************************************************************/
  if ( !pBT->LookupTable_V2 || ( usNumber >= pBT->usNumberOfLookupEntries ))
  {
    sRc = BTREE_LOOKUPTABLE_CORRUPTED;
  }
  else
  {
    pLEntry = pBT->LookupTable_V2 + usNumber;

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
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, (LONG) BTREE_BUFFER_V2 , NOMSG );
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
      lOffset = ((LONG) usNumber) * BTREE_REC_SIZE_V2;
      sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
    } /* endif */

    // Read the record in to the buffer space
    // Mark the next buffer for future allocations
    if ( !sRc )
    {
      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, NULL );
      sRc = UtlRead( pBT->fp,
                     (PVOID)&pBuffer->contents,
                     BTREE_REC_SIZE_V2, &usNumBytesRead, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

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
           memset (&pBuffer->contents, 0, sizeof(BTREERECORD_V2));
           // init this record
           pHeader = &(pBuffer->contents.header);
           pHeader->usOccupied = 0;
           pHeader->usNum = usNumber;
           pHeader->usFilled = sizeof(BTREEHEADER );
           pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );

           sRc = QDAMWRecordToDisk_V2(pBTIda, pBuffer);
           if (! sRc )
           {
             if ( (pBT->usOpenFlags & ASD_SHARED) && !pBTIda->fPhysLock )
             {
               // this condition should NEVER occur for shared resources
               // as write is only allowed if the database has been
               // previously locked
               ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 2, DB_GROUP, NULL );
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
           ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, NULL );
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
     sRc = QDAMGetUpdCounter( pBTIda, &lUpdCount, 0, 1 );

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
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  #endif
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWRecordToDisk    Write record to disk
//------------------------------------------------------------------------------
// Function call:     QDAMWRecordToDisk( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to flush
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return value (sRc )
//                    position in file to record to be written
//                    if okay so far then
//                      write record to disk and set sRC
//                      if okay then
//                        check if disk was full, i.e.
//                        if written bytes equal BTREE_REC_SIZE then
//                          reset fNeedToWrite flag;
//                          init buffer
//                        else
//                          set return code BTREE_DISK_FULL;
//                          set fCorrupted flag
//                        endif
//                      else
//                        set BTREE_WRITE_ERROR;
//                        set fCorrupted flag
//                      endif
//                    else
//                      set BTREE_WRITE_ERROR;
//                      set fCorrupted flag
//                    endif
//                    return sRc;
//------------------------------------------------------------------------------
SHORT QDAMWRecordToDisk_V2
(
   PBTREE     pBTIda,                     // pointer to btree structure
   PBTREEBUFFER_V2  pBuffer               // pointer to buffer to write
)
{
  SHORT sRc = 0;                          // return code
  LogMessage(FATAL, "called commented out function QDAMWRecordToDisk_V2");
  #ifdef TEMPORARY_COMMENTED
  LONG  lOffset;                          // offset in file
  ULONG ulNewOffset;                      // new file pointer position
  USHORT usBytesWritten;                  // number of bytes written
  PBTREEGLOB  pBT = pBTIda->pBTree;

  INC_REAL_WRITE_COUNT;

  DEBUGEVENT2( QDAMRECORDTODISK_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

//QDAMCheckCheckSum( pBuffer, QDAMRECORDTODISK_LOC );

  if ( !sRc )
  {
    lOffset = RECORDNUM(pBuffer) * (long)BTREE_REC_SIZE_V2;

    sRc = UtlChgFilePtr( pBT->fp, lOffset, FILE_BEGIN, &ulNewOffset, FALSE);
  } /* endif */

  if ( ! sRc )
  {
    sRc = UtlWrite( pBT->fp, (PVOID) &pBuffer->contents, BTREE_REC_SIZE_V2, &usBytesWritten, FALSE );

    // check if disk is full
    if ( ! sRc )
    {
       if ( usBytesWritten == BTREE_REC_SIZE_V2 )
       {
          pBuffer->fNeedToWrite = FALSE;
       }
       else
       {
          sRc = BTREE_DISK_FULL;
          pBT->fCorrupted = TRUE;                     // indicate corruption
       } /* endif */
    }
    else
    {
       sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
       pBT->fCorrupted = TRUE;                     // indicate corruption
    } /* endif */
  }
  else
  {
    sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    pBT->fCorrupted = TRUE;                        // indicate corruption
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMRECORDTODISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */
  #endif

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecord   Read record
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecord( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record either from cache or from disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to read
//                    USHORT             number of buffer to read
//                    PPBTREERECORD      The buffer read
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_LOOKUPTABLE_TOO_SMALL file is too big for l.table
//                    BTREE_LOOKUPTABLE_NULL      ptr to lookup table is NULL
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most performance
//                    critical routines in the module.
//
//------------------------------------------------------------------------------
// Concept:
//
//   AccessCounter  Record       LookupTable       Memory
//       Table      Number
//     +-------+                  +-------+
//         0           0             NULL
//     +-------+                  +-------+       +--------------------------+
//        700          1             ptr  +------>  Buffer contains Record 1
//     +-------+                  +-------+       +--------------------------+
//         0           2             NULL
//     +-------+                  +-------+
//         .                          .
//         .                          .
//     +-------+                  +-------+       +--------------------------+
//        200          i             ptr  +------>  Buffer contains Record i
//     +-------+                  +-------+       +--------------------------+
//         .                          .
//         .                          .
//     +-------+  pBT->usNumberOf +-------+
//         0       LookupEntries     NULL
//     +-------+                  +-------+
//
//  The lookup table is a dynamically growing array of ptrs to BTREEBUFFER.
//  If the entry for record i is NULL record i isn't buffered in memory.
//  If the rec.num. of the record to read is > pBT->usNumberOfLookupEntries
//  lookup and access counter table will be resized.
//
//  The access counter table is a dynamically growing array of unsigned long.
//  On every read access of record i the entry for record i is increased by
//  ACCESSBONUSPOINTS.
//
//  From time to time (every MAX_READREC_CALLS calls to QDAMReadRecord)
//  unlocked records which aren't read very often ( accesscounter <
//  MAX_READREC_CALLS ) will be written to disk (if fNeedToWrite is set) and
//  the buffer that contains the record will be freed.
//
//------------------------------------------------------------------------------
// Function flow:
//
//     initialize return code (sRc) and pointer to record
//
//     if rec.num. >= MAX_NUMBER_OF_LOOKUP_ENTRIES (l.table would exceed 64kB)
//        or ptr. to lookup- or access-counter-table is NULL
//         set appropriate return code
//     else
//        if rec.num >= number of lookup table entries
//           resize lookup and access counter table
//           and set appropriate return code
//
//     if no error occured
//        if record is already in memory
//          set pointer to it
//        else
//          read record from disk (QDAMReadRecordFromDisk)
//            and set appropriate return code
//          initialize access counter (set it to 0)
//        endif
//
//     if no error occured
//        if #calls to QDAMReadRecord >= MAX_READREC_CALLS
//           write unlocked records with access counter < MAX_READREC_CALLS
//           to disk (if fNeedToWrite is set) and free allocated buffer
//           set pBT->ulReadRecCalls to 0
//        else
//           increment pBT->ulReadRecCalls
//
//     if no error occured
//        increase access counter of the record read by ACCESSBONUSPOINTS
//
//     return sRc
//------------------------------------------------------------------------------
SHORT  QDAMReadRecord_V2
(
   PBTREE  pBTIda,
   USHORT  usNumber,
   PBTREEBUFFER_V2 * ppReadBuffer,
   BOOL    fNewRec
)
{
  USHORT   i;
  SHORT    sRc = 0;                  // return code
  BOOL     fMemOK = FALSE;
  PBTREEGLOB    pBT = pBTIda->pBTree;
  PLOOKUPENTRY_V2  pLEntry;
  PACCESSCTRTABLEENTRY  pACTEntry;

  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, NULL );

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
  else if ( !pBT->LookupTable_V2 || !pBT->AccessCtrTable )
  {
    sRc = BTREE_LOOKUPTABLE_NULL;
  }
  else
  {
    if ( usNumber >= pBT->usNumberOfLookupEntries )
    {
      /* The lookup-table entry for the record to read doesn't exist */
      /* Reallocate memory for LookupTable and AccessCounterTable */
      fMemOK = UtlAlloc( (PVOID *)&pBT->LookupTable_V2,
              (LONG) pBT->usNumberOfLookupEntries * sizeof(LOOKUPENTRY_V2),
              (LONG) (usNumber + 10) * sizeof(LOOKUPENTRY_V2), NOMSG );
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
    pLEntry = pBT->LookupTable_V2 + usNumber;

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
      sRc =  QDAMReadRecordFromDisk_V2( pBTIda, usNumber, ppReadBuffer, fNewRec );
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
        pLEntry = pBT->LookupTable_V2 + i;
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
                ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 1, DB_GROUP, NULL );
              #ifdef _DEBUG
                WinMessageBox( HWND_DESKTOP,
                   (HWND)UtlQueryULong( QL_TWBCLIENT ),
                   "Insecure write to dictionary/TM detected!\nLoc=READRECORDFROMDISK/1\nPlease save info required to reproduce this condition.",
                   "Gotcha!", 9999, MB_ERROR );
              #endif
              } /* endif */
              sRc = QDAMWRecordToDisk_V2(pBTIda, pLEntry->pBuffer);
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
    ERREVENT2( QDAMREADRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */

  return ( sRc );
}

SHORT  QDAMReadRecord_V3
(
   PBTREE  pBTIda,
   USHORT  usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL    fNewRec
);



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAllocKeyRecords   Allocate space for key records
//------------------------------------------------------------------------------
// Function call:     QDAMAllocKeyRecords( PBTREEBUFFER, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Allocate a chunk of records for keys
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    USHORT                 number of records to be allocated
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
// Function flow:     allocate the number of requested records and
//                    put them into a linked list
//
//------------------------------------------------------------------------------
SHORT QDAMAllocKeyRecords
(
   PBTREE pBTIda,
   USHORT usNum
)
{
   SHORT  sRc = 0;
   PBTREEHEADER  pHeader;
   PBTREEGLOB    pBT = pBTIda->pBTree;
LogMessage(FATAL, "called commented out function QDAMAllocKeyRecords");
  #ifdef TEMPORARY_COMMENTED
   if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord;
      while ( usNum-- && !sRc )
      {
          pBT->usNextFreeRecord++;
          sRc = QDAMReadRecord_V3( pBTIda, pBT->usNextFreeRecord, &pRecord, TRUE  );

          if ( !sRc )
          {
            // add free record to the free list
            NEXT( pRecord ) = pBT->usFreeKeyBuffer;
            PREV( pRecord ) = 0;
            pBT->usFreeKeyBuffer = RECORDNUM( pRecord );

            // init this record
            pHeader = &pRecord->contents.header;
            pHeader->usNum = pBT->usFreeKeyBuffer;
            pHeader->usOccupied = 0;
            pHeader->usFilled = sizeof(BTREEHEADER );
            pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

            /*************************************************************/
            /* force write                                               */
            /*************************************************************/
            sRc = QDAMWRecordToDisk_V3( pBTIda, pRecord);
          } /* endif */
      } /* endwhile */
   }
   else
   {
      PBTREEBUFFER_V2  pRecord;
      while ( usNum-- && !sRc )
      {
          pBT->usNextFreeRecord++;
          sRc = QDAMReadRecord_V2( pBTIda, pBT->usNextFreeRecord, &pRecord, TRUE  );

          if ( !sRc )
          {
            // add free record to the free list
            NEXT( pRecord ) = pBT->usFreeKeyBuffer;
            PREV( pRecord ) = 0;
            pBT->usFreeKeyBuffer = RECORDNUM( pRecord );

            // init this record
            pHeader = &pRecord->contents.header;
            pHeader->usNum = pBT->usFreeKeyBuffer;
            pHeader->usOccupied = 0;
            pHeader->usFilled = sizeof(BTREEHEADER );
            pHeader->usLastFilled = BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER );

            /*************************************************************/
            /* force write                                               */
            /*************************************************************/
            sRc = QDAMWRecordToDisk_V2( pBTIda, pRecord);
          } /* endif */
      } /* endwhile */
   } /* endif */
    #endif
   return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteRecord  Write record
//------------------------------------------------------------------------------
// Function call:     QDAMWriteRecord( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record either in cash ( mark it )
//                    or to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to write
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return code
//                    write header with open flag set if not yet done
//                    if guard flag is on then
//                      write it to disk and set sRc
//                    else
//                      set only flag
//                    endif
//                    return sRc
//------------------------------------------------------------------------------
SHORT QDAMWriteRecord_V2
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V2 pBuffer
)
{
   SHORT  sRc = 0;                               // return code
   PBTREEGLOB  pBT = pBTIda->pBTree;
  LogMessage(FATAL, "called commented out function QDAMWriteRecord_V2");
  #ifdef TEMPORARY_COMMENTED
   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, NULL );

   INC_WRITE_COUNT;

   /********************************************************************/
   /* write header with corruption flag -- if not yet done             */
   /********************************************************************/
   if ( pBT->fWriteHeaderPending )
   {
     sRc = QDAMWriteHeader( pBTIda );
     pBT->fWriteHeaderPending = FALSE;
   } /* endif */

   //  if guard flag is on write it to disk
   //  else set only the flag
   if ( !sRc )
   {
     if ( !pBT->fGuard )
     {
        pBuffer->fNeedToWrite = TRUE;
     }
     else
     {
        sRc = QDAMWRecordToDisk_V2(pBTIda, pBuffer);
     } /* endif */
   } /* endif */


   // For shared databases only: set internal update flag, the
   // update counter will be written once the file lock is removed
   if ( !sRc && (pBT->usOpenFlags & ASD_SHARED) )
   {
     pBT->fUpdated = TRUE;;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMWRITERECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCEXIT_EVENT, sRc, DB_GROUP, NULL );
   #endif
   
   return sRc;
}


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
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V3(pBTIda,pRecord);
          } /* endif */
          if ( !sRc )
          {
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
            if (! sRc )
            {
                pBT->usFirstDataBuffer = pBT->usNextFreeRecord;
            } /* endif */
          } /* endif */
        }
        else
        {
          PBTREEBUFFER_V2 pRecord;
          sRc = QDAMReadRecord_V2(pBTIda, pBT->usFirstNode, &pRecord, TRUE );
          if ( !sRc )
          {
            TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;
            sRc = QDAMWriteRecord_V2(pBTIda,pRecord);
          } /* endif */
          if ( !sRc )
          {
            sRc = QDAMAllocKeyRecords( pBTIda, 1 );
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
    // leave the return code from create
    // file will not be destroyed if create failed, since then
    // filename is not set
    QDAMDestroy( pBTIda );
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMDICTCREATELOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
  } /* endif */


  return( sRc );
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockStatus
//------------------------------------------------------------------------------
// Function call:     fLocked = QDAMDictLockStatus( pBTIda, pHeadTerm );
//------------------------------------------------------------------------------
// Description:       This call returns the status of the current entry
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIDa   pointer to instance data
//                    PSZ     pHeadTerm  pointer to head term
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   entry is locked
//                    FALSE  entry is free for update
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of all open instances of our dict..
//                    run thru list and check if this headterm is locked ..
//------------------------------------------------------------------------------
BOOL
QDAMDictLockStatus
(
  PBTREE  pBTIda,
  PSZ_W   pKey
)
{
  BOOL     fLock = FALSE;
  /******************************************************************/
  /* check the subject entry is not locked somewhere else...        */
  /******************************************************************/
  LogMessage(FATAL, "called commented out function QDAMDictLockStatus");
  #ifdef TEMPORARY_COMMENTED

  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;
  pQDict = &(QDAMDict[pBTIda->usDictNum]);
  ppBTemp = &(pQDict->pIdaList[1]);

  while ( *ppBTemp && !fLock)
  {
    if ( (*ppBTemp)->fLock && ! UTF16stricmp( pKey, (*ppBTemp)->chLockedTerm) )
    {
      /****************************************************************/
      /* set fLock only if it's not me who want to update ....        */
      /****************************************************************/
      if ( pBTIda != *ppBTemp )
      {
        fLock = TRUE;
      }
      else
      {
        ppBTemp++;
      } /* endif */
    }
    else
    {
      ppBTemp++;
    } /* endif */
  } /* endwhile */

  return( fLock );
  #endif 
  return false;
} /* end of function QDAMDictLockStatus */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMPhysLock           Physical lock file
//------------------------------------------------------------------------------
// Function call:     QDAMPhysLock( PBTREE, BOOL, PBOOL );
//
//------------------------------------------------------------------------------
// Description:       lock or unlock the database
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The database to be locked
//                    BOOL               TRUE = LOCK, FALSE = Unlock
//                    PBOOL              ptr to locked flag (set to TRUE if
//                                       locking was successful
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
SHORT QDAMPhysLock
(
   PBTREE         pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
)
{
  LogMessage(FATAL, "called commented out function QDAMPhysLock");
  #ifdef TEMPORARY_COMMENTED
  SHORT    sRc = 0;                    // return code
  PBTREEGLOB  pBT = pBTIda->pBTree;
  FILELOCK  Lock;                      // range to be locked or unlocked

  if ( pfLocked != NULL ) *pfLocked = FALSE;  // set initial value
  Lock.lOffset = 0L;                   // start at begin of file
  Lock.lRange  = 0x7FFFFFFF;

  if ( fLock )
  {
    if ( !pBTIda->fPhysLock )
    {
       SHORT sRetries = MAX_RETRY_COUNT;

       // lock update counter file
       do
       {
         sRc = UtlFileLocks( pBT->fpDummy, (PFILELOCK)NULL, &Lock, FALSE );
         if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

         if ( sRc == BTREE_IN_USE )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( (sRc == BTREE_IN_USE) && (sRetries > 0));

       // lock database file (here we do not use a retry loop as the database
       // file should not be locked if the update counter file was not locked)
       if ( sRc == NO_ERROR )
       {
         sRc = UtlFileLocks( pBT->fp, (PFILELOCK)NULL, &Lock, FALSE );
         if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

         // unlock update counter file in case of errors
         if ( sRc != NO_ERROR )
         {
           UtlFileLocks( pBT->fpDummy, &Lock, (PFILELOCK)NULL, FALSE );
         } /* endif */
       } /* endif */

       if ( sRc == NO_ERROR )
       {
         pBTIda->fPhysLock = TRUE;
         if ( pfLocked != NULL ) *pfLocked = TRUE;

         // ensure we work with up-to-date data
         sRc = QDAMCheckForUpdates( pBTIda );
       } /* endif */
    } /* endif */
  }
  else
  {
     // Set update counter in case of database updates
    if ( pBT->fUpdated )
    {
      sRc = QDAMDictFlushLocal( pBTIda );
      if ( sRc == NO_ERROR )
      {
        sRc = QDAMWriteHeader( pBTIda );
      } /* endif */
      QDAMIncrUpdCounter( pBTIda, 0, NULL );
      pBT->fUpdated = FALSE;
    } /* endif */

    // unlock database file
    sRc = UtlFileLocks( pBT->fp, &Lock, (PFILELOCK)NULL, FALSE );

    // unlock update counter file
    if ( sRc == NO_ERROR )
    {
      UtlFileLocks( pBT->fpDummy, &Lock, (PFILELOCK)NULL, FALSE );
      pBTIda->fPhysLock = FALSE;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMPHYSLOCK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
//    sRc = BTREE_IN_USE;
  } /* endif */

  return( sRc ) ;
  #endif 
  return 0;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdStatus
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdStatus( pBTIda );
//------------------------------------------------------------------------------
// Description:       invalidates the status of the current record
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIda   pointer to instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of instances
//                    run through list and invalidate the current entry for
//                      all instances ...
//                    Our instance data will be updated after this call, so
//                      it's okay to get rid of it, too.
//------------------------------------------------------------------------------
VOID
QDAMDictUpdStatus
(
  PBTREE  pBTIda
)
{

  LogMessage(FATAL, "called commented out function QDAMDictUpdStatus");
  #ifdef TEMPORARY_COMMENTED
  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;
  /********************************************************************/
  /* update, i.e. invalidate the sCurrent record and offset           */
  /********************************************************************/
  pQDict = &(QDAMDict[pBTIda->usDictNum]);
  ppBTemp = &(pQDict->pIdaList[1]);

  while ( *ppBTemp )
  {
    (*ppBTemp)->sCurrentIndex = RESET_VALUE;
    (*ppBTemp)->usCurrentRecord = 0;
    ppBTemp++;
  } /* endwhile */
  #endif 
} /* end of function QDAMDictUpdStatus */


SHORT QDAMFindRecord_V3
(
    PBTREE   pBTIda,
    PCHAR_W  pKey,
    PBTREEBUFFER_V3 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  LogMessage(FATAL, "called commented out function QDAMFindRecord_V3");
  #ifdef TEMPORARY_COMMENTED
  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V3( pBTIda, pBT->usFirstNode, ppRecord, FALSE  );
  while ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V3( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endwhile */
  #endif 
  return( sRc );
}


SHORT  QDAMAddToBuffer_V3
(
   PBTREE  pBTIda,                     // pointer to btree struct
   PBYTE   pData,                      // pointer to data
   ULONG   ulDataLen,                  // length of data to be filled in
   PRECPARAM  precReturn               // pointer to return code
)
{
   SHORT sRc = 0;                      // success indicator

   LogMessage(FATAL, "called commented out function QDAMAddToBuffer_V3");
  #ifdef TEMPORARY_COMMENTED
   USHORT  usFilled;                   // number of bytes filled in record
   USHORT  usMaxRec= 0;                // most filled record
   RECPARAM recStart;                  // return code where data are stored
   PRECPARAM pRecParam;                // parameter to stored list
   PRECPARAM pRecTemp ;                // parameter to stored list
   BOOL  fFit = FALSE;                 // indicator if record found
   USHORT  i;                          // index
   PBTREEHEADER  pHeader;              // record header
   PBTREEBUFFER_V3 pRecord = NULL;        // buffer record
   PBTREEBUFFER_V3 pTempRecord;           // buffer record
   CHAR     chNodeType;                // type of node
   BOOL     fTerse = FALSE;            // data are not tersed
   USHORT   usLastPos;
   ULONG    ulFullDataLen;             // length of record
   PUSHORT  pusOffset;
   PCHAR    pRecData;                  // pointer to record data
   ULONG    ulFitLen;                  // currently used length
  PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT   usLenFieldSize;            // size of length field

   memset(&recStart, 0, sizeof(recStart));
   /*******************************************************************/
   /* Enlarge pTempRecord area if it is not large enough to contain   */
   /* the data for this record                                        */
   /*******************************************************************/
#ifdef _DEBUG
   {
     CHAR szTemp[8];
     sprintf( szTemp, "%ld", ulDataLen );
     INFOEVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 4711, DB_GROUP, szTemp );
   }
#endif
   if ( (ulDataLen + sizeof(ULONG)) > pBT->ulTempRecSize )
   {
     if ( UtlAlloc( (PVOID *)&(pBT->pTempRecord), pBT->ulTempRecSize, ulDataLen + sizeof(ULONG), NOMSG ) )
     {
       pBT->ulTempRecSize = ulDataLen + sizeof(ULONG);
     }
     else
     {
       sRc = BTREE_NO_ROOM;
     } /* endif */
   } /* endif */

  if ( pBT->usVersion >= NTM_VERSION2 )
  {
    usLenFieldSize = sizeof(ULONG);
  }
  else
  {
    usLenFieldSize = sizeof(USHORT);
  } /* endif */

   pRecParam = pBT->DataRecList;
   /*******************************************************************/
   /* determine type of compression....                               */
   /*******************************************************************/
   switch ( pBT->fTerse )
   {
     case  BTREE_TERSE_HUFFMAN :
       fTerse = QDAMTerseData( pBTIda, pData, &ulDataLen, pBT->pTempRecord );
       break;
     default :
       fTerse = FALSE;
       break;
   } /* endswitch */

   chNodeType = DATA_NODE;

   // data should be in pBT->pTempRecord for further processing
   // copy them if not yet done.....
   if ( !fTerse )
   {
      memcpy( pBT->pTempRecord, pData, ulDataLen );
   } /* endif */

   if ( !sRc && (ulDataLen <= BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER)) )
   {
      pRecTemp = pRecParam;
      i = 0;
      //  find slot in list
      usFilled = (USHORT)(BTREE_REC_SIZE_V3 - ulDataLen - 2 * usLenFieldSize);
      while ( !fFit && i < MAX_LIST )
      {
         if ( pRecTemp->usOffset == 0 || pRecTemp->usOffset > usFilled )   // will not fit
         {
            pRecTemp++;
            i++;
         }
         else                                     // data will fit
         {
            fFit = TRUE;
         } /* endif */
      } /* endwhile */


      if ( fFit )
      {
         // store position where data will be stored
         recStart.usNum    = pRecTemp->usNum;

         // get record, copy data and write them
         sRc = QDAMReadRecord_V3( pBTIda, recStart.usNum, &pRecord, FALSE  );
         if ( !sRc )
         {
            BTREELOCKRECORD( pRecord );
            if ( TYPE( pRecord ) != chNodeType )
            {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
            }
            else
            {
               // fill in key data
               usLastPos = pRecord->contents.header.usLastFilled;
               usLastPos = usLastPos - (USHORT)(ulDataLen + usLenFieldSize);
               pData = pRecord->contents.uchData + usLastPos;
               // insert reference
               if ( pBT->usVersion >= NTM_VERSION2 )
               {
                 *(PULONG) pData = ulDataLen;
               }
               else
               {
                 *(PUSHORT) pData = (USHORT)ulDataLen;
               }
               memcpy( pData+usLenFieldSize, pBT->pTempRecord, ulDataLen );

               // set address of key data at next free offset
               pusOffset = (PUSHORT) pRecord->contents.uchData;
               *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

               OCCUPIED(pRecord) += 1;
               pRecord->contents.header.usLastFilled = usLastPos;
               pRecord->contents.header.usFilled = pRecord->contents.header.usFilled + (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

               // mark if tersed or not
               if ( fTerse )
               {
                 if ( pBT->usVersion >= NTM_VERSION2 )
                 {
                   *(PULONG) pData |= QDAM_TERSE_FLAGL;
                 }
                 else
                 {
                   *(PUSHORT) pData |= QDAM_TERSE_FLAG;
                 }
               } /* endif */
//             pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

               sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
               // update list entry
               if ( !sRc )
               {
                  pRecTemp->usOffset = pRecord->contents.header.usFilled;
               } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
         } /* endif */
      } /* endif */
   } /* endif */

   //  either record is too large or no space left in list of records

   if ( !sRc && !fFit )
   {
      pRecTemp = pRecParam;
      i = 0;
      // scan list and find empty slot
      while ( (i < MAX_LIST) && !fFit )
      {
         if ( pRecTemp->usNum == 0 )     // empty slot found
         {
            fFit = TRUE;
         }
         else
         {
            pRecTemp++;                  // point to next slot
            i++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* if not found remove slot which is most filled                */
      /****************************************************************/
      if ( !fFit )
      {
         pRecTemp = pRecParam;
         usFilled = 0;
         for ( i=0 ; i < MAX_LIST ;i++ )
         {
            if ( pRecTemp->usOffset > usFilled)
            {
               usMaxRec = i;
               usFilled = pRecTemp->usOffset;
            } /* endif */
            pRecTemp++;                // next record in list
         } /* endfor */
         // set pRecTemp to the selected slot
         pRecTemp = pRecParam + usMaxRec;
      } /* endif */

      // get record and store start position
      memset( pRecTemp, 0 , sizeof(RECPARAM));

      sRc = QDAMNewRecord_V3( pBTIda, &pRecord, DATAREC );
      // fill it up to length or record size
      if ( ! sRc )
      {
         TYPE( pRecord ) = chNodeType;     // indicate that it is a data node
         BTREELOCKRECORD( pRecord );
         pRecData = (PCHAR)(pBT->pTempRecord + ulDataLen);
         ulFullDataLen = ulDataLen;         // store original data len
         while ( !sRc && ulDataLen + pRecord->contents.header.usFilled  >= (BTREE_REC_SIZE_V3 - (usLenFieldSize + sizeof(SHORT))))
         {
           /***********************************************************/
           /* get a new record, anchor it and fill data from end      */
           /* new record will be predecessor of already allocated rec.*/
           /***********************************************************/
           sRc = QDAMNewRecord_V3( pBTIda, &pTempRecord, DATAREC );
           if ( !sRc )
           {
             TYPE( pTempRecord ) = chNodeType;   // data node
             NEXT( pTempRecord ) = RECORDNUM( pRecord );
             PREV( pRecord ) = RECORDNUM( pTempRecord );
             TYPE( pRecord ) = DATA_NEXTNODE;              // data node

             // fill in key data
             /*********************************************************/
             /* adjust sizes of relevant data                         */
             /*********************************************************/
             usLastPos = pRecord->contents.header.usLastFilled;
             {
               ULONG ulMax = (ULONG)(BTREE_REC_SIZE_V3 - pRecord->contents.header.usFilled );
               ulFitLen = min( ulDataLen, ulMax );
               ulFitLen -= (ULONG)(usLenFieldSize + sizeof(USHORT));
             }
             usLastPos = usLastPos - (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
             pData = pRecord->contents.uchData + usLastPos;
             pRecData -= ulFitLen;
             ulDataLen -= ulFitLen;
             /*********************************************************/
             /* store length and copy data of part contained in record*/
             /*********************************************************/
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData = ulFitLen;
             }
             else
             {
               *(PUSHORT) pData = (USHORT)ulFitLen;
             }
             memcpy( pData+usLenFieldSize, pRecData,  ulFitLen );
             // set address of key data at next free offset
             pusOffset = (PUSHORT) pRecord->contents.uchData;
             *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

             OCCUPIED(pRecord) += 1;
             pRecord->contents.header.usLastFilled = usLastPos;
             pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                           (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

             sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
             BTREEUNLOCKRECORD( pRecord );

             /*********************************************************/
             /* toggle for adressing purposes                         */
             /* now: pRecord will be the free one for later filling   */
             /*********************************************************/
             pRecord = pTempRecord;
             BTREELOCKRECORD( pRecord );
           } /* endif */
         } /* endwhile */

         /*************************************************************/
         /* write out rest (this is the normal way in most cases      */
         /*************************************************************/
         if ( !sRc )
         {
           pHeader = &(pRecord->contents.header);
           pHeader->usParent = sizeof(BTREEHEADER );

           // store position where data will be stored
           recStart.usNum    = pHeader->usNum;

           // fill in key data
           usLastPos = pRecord->contents.header.usLastFilled;

           usLastPos = usLastPos - (USHORT)(ulDataLen + (ULONG)usLenFieldSize);
           pData = pRecord->contents.uchData + usLastPos;
           pRecData -= ulDataLen;

           // insert reference
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             *(PULONG) pData = ulFullDataLen;
           }
           else
           {
             *(PUSHORT) pData = (USHORT)ulFullDataLen;
           }
           memcpy( pData+usLenFieldSize, pRecData, ulDataLen );

           // set address of key data at next free offset
           pusOffset = (PUSHORT) pRecord->contents.uchData;
           *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

           OCCUPIED(pRecord) += 1;
           pRecord->contents.header.usLastFilled = usLastPos;
           pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                    (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

           // mark if tersed or not
           if ( fTerse )
           {
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData |= QDAM_TERSE_FLAGL;
             }
             else
             {
               *(PUSHORT) pData |= QDAM_TERSE_FLAG;
             }
           } /* endif */


           // write record
  //       pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V3(pBTIda, pRecord);
           BTREEUNLOCKRECORD( pRecord );         // free the locking
           // update list entry
           if ( !sRc )
           {
              pRecTemp->usOffset = pHeader->usFilled;
              pRecTemp->usNum    = pHeader->usNum;
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   //  return record parameters
   if ( !sRc )
   {
      precReturn->usNum = recStart.usNum - pBT->usFirstDataBuffer;
      precReturn->usOffset = OCCUPIED( pRecord ) - 1;
   }
   else
   {
      memset(&recStart, 0, sizeof(RECPARAM));
      precReturn = &recStart;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMADDTOBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
#endif
   return sRc;
}




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMInsertKey       Insert a key
//------------------------------------------------------------------------------
// Function call:     QDAMInsertKey( PBTREE, PBTREEBUFFER, PCHAR,
//                                    RECPARAM, RECPARAM );
//------------------------------------------------------------------------------
// Description:       Add a key and offset to a record. If there is no room,
//                    the record is split in half and the key is add to the
//                    appropriate half. The key being inserted must be unique.
//------------------------------------------------------------------------------
// Parameters:        PBTREE             pointer to btree structure
//                    PBTREEBUFFER       record where to insert the key
//                    PCHAR              key to be inserted
//                    RECPARAM           position/offset of the key string
//                    RECPARAM           pos.   /offset of data (if data node)
//                                               else position of child record
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if key could be inserted
//                    FALSE  else
//
//------------------------------------------------------------------------------
// Side effects:      Splitting a record will affect the parent node(s) of a
//                    record.
//------------------------------------------------------------------------------
// Function flow:     lock record
//                    check if key is already there
//                    if key found
//                      set RC=BTREE_DUPLICATE_KEY
//                    else
//                      get next free position in record
//                      if node is full
//                        split node before inserting
//                      endif
//                    endif
//                    find proper location in record to insert key
//                    set new current positioon
//                    fill in key data
//                    insert reference, set data information
//                    set address of key data at correct offset
//                    write record
//                    if this is first key in record
//                      adjust above pointers
//                    endif
//                    unlock record
//
//------------------------------------------------------------------------------
SHORT QDAMInsertKey_V3
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V3 pRecord,               // record where key is to be inserted
   PCHAR_W      pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
)
{
  SHORT i = 0;
  PBTREEBUFFER_V3 pTempRec;
  PCHAR_W   pCompKey = NULL;           // key to be compared with
  PCHAR_W   pOldKey;                   // old key at first position
  PCHAR_W   pNewKey;                   // new key at first position
  BOOL fFound = FALSE;
  SHORT  sKeyFound;                    // key found
  SHORT  sNearKey;                     // key found
  SHORT  sRc = 0;                      // return code

  LogMessage(FATAL, "called commented out function QDAMInsertKey_V3");
  #ifdef TEMPORARY_COMMENTED
  USHORT usLastPos;
  USHORT usKeyLen = 0;                 // length of key
  USHORT usDataRec = 0;                // length of key record
  PBYTE  pData;
  PUSHORT  pusOffset = NULL;
  USHORT usCurrentRecord;              // current record
  SHORT  sCurrentIndex;                // current index
  PBTREEGLOB    pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

  recKey;                              // get rid of compiler warnings
  //  check if key is already there -- duplicates will not be supported
  sRc = QDAMLocateKey_V3( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
  BTREELOCKRECORD( pRecord );
  fRecLocked = TRUE;
  if ( !sRc )
  {
     if ( sKeyFound != -1)
     {
       sRc = BTREE_DUPLICATE_KEY;
     }
     else
     {
       i = (SHORT) OCCUPIED(pRecord);
       usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : (UTF16strlenBYTE(pKey) + sizeof(CHAR_W)));

       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAM);
       }
       else
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAMOLD);
       } /* endif */
       /* If node is full. Split the node before inserting */
       if ( usDataRec+sizeof(USHORT)+pRecord->contents.header.usFilled > BTREE_REC_SIZE_V3 )
       {
         BTREEUNLOCKRECORD( pRecord );
         fRecLocked = FALSE;
         sRc = QDAMSplitNode_V3( pBTIda, &pRecord, pKey );
         if ( !sRc )
         {
           // SplitNode may have passed a new record back
           BTREELOCKRECORD( pRecord );
           fRecLocked = TRUE;

           /* if we split an inner node, the parent of the key we are */
           /* inserting must be changed                               */
           if ( !IS_LEAF( pRecord ) )
           {
             sRc = QDAMReadRecord_V3(pBTIda, recData.usNum, &pTempRec, FALSE );
             if ( ! sRc )
             {
               PARENT( pTempRec ) = RECORDNUM( pRecord );
//             pTempRec->ulCheckSum = QDAMComputeCheckSum( pTempRec );
               sRc = QDAMWriteRecord_V3( pBTIda, pTempRec );
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
  } /* endif */


  if ( !sRc )
  {
    i = (SHORT) OCCUPIED(pRecord);               // number of keys in record
    // Insert key at the proper location
    pusOffset = (PUSHORT) pRecord->contents.uchData;
    while ( i > 0 && !fFound )
    {
       pCompKey = QDAMGetszKey_V3( pRecord, (SHORT)(i-1), pBT->usVersion );
       if ( pCompKey )
       {
          if ( (*(pBT->compare))(pBTIda, pKey, pCompKey)  < 0 )
          {
             *(pusOffset+i) = *(pusOffset+i-1);
             i--;
          }
          else
          {
             fFound = TRUE;
          } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */
    } /* endwhile */

    // set new current position
    pBTIda->sCurrentIndex = i;
    pBTIda->usCurrentRecord = RECORDNUM( pRecord );
  } /* endif */

  if ( !sRc )
  {
      CHAR chHeadTerm[128];
      // fill in key data
      usLastPos = pRecord->contents.header.usLastFilled;
      usLastPos = usLastPos - usDataRec;
      pData = pRecord->contents.uchData + usLastPos;
      // insert reference
      if ( !pBT->fTransMem && pBT->usVersion == BTREE_VERSION)
      {
         Unicode2ASCII( pKey, chHeadTerm, 0L );
         usKeyLen = (USHORT)(strlen(chHeadTerm)+1);
         pKey = (PSZ_W)&chHeadTerm[0];
      }
      *(PUSHORT) pData = usKeyLen;
      {
        PBYTE pTarget;
        if ( pBT->usVersion >= NTM_VERSION2 )
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAM);
        }
        else
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAMOLD);
        } /* endif */

        memcpy(pTarget, (PBYTE) pKey, usKeyLen );

      }
      // set data information
      if ( pBT->usVersion >= NTM_VERSION2 )
      {
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recData, sizeof(RECPARAM));
      }
      else
      {
        RECPARAMOLD recDataOld;
        recDataOld.usOffset = recData.usOffset;
        recDataOld.usNum    = recData.usNum;
        recDataOld.sLen     = (SHORT)recData.ulLen;
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recDataOld, sizeof(RECPARAMOLD));
      } /* endif */

      // set address of key data at offset i
      *(pusOffset+i) = usLastPos;

      OCCUPIED(pRecord) += 1;
      pRecord->contents.header.usLastFilled = usLastPos;
      pRecord->contents.header.usFilled += usDataRec + sizeof(USHORT);
//    pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

      sRc = QDAMWriteRecord_V3(pBTIda,pRecord);
  } /* endif */

  // If this is the first key in the record, we must adjust pointers above
  if ( !sRc )
  {
    if ((i == 0) && (!IS_ROOT(pRecord)))
    {
       pOldKey = QDAMGetszKey_V3( pRecord, 1, pBT->usVersion );
       pNewKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
       if ( pOldKey && pNewKey )
       {
         if ( pCompKey && PARENT(pRecord) )
         {
            /**********************************************************/
            /* save old current record since ChangeKey will update it */
            /* (implicit call to QDAMInsertKey) and restore it ...   */
            /**********************************************************/
            usCurrentRecord = pBTIda->usCurrentRecord;
            sCurrentIndex   = pBTIda->sCurrentIndex;
            sRc = QDAMChangeKey_V3( pBTIda, PARENT(pRecord), pOldKey, pNewKey);
            pBTIda->usCurrentRecord = usCurrentRecord;
            pBTIda->sCurrentIndex   = sCurrentIndex;

         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
       } /* endif */
    } /* endif */
  } /* endif */

  if ( pRecord && fRecLocked )
  {
    BTREEUNLOCKRECORD(pRecord);
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMINSERTKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
  #endif
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindRecord      Find Record
//------------------------------------------------------------------------------
// Function call:     QDAMFindRecord( PBTREE, PCHAR, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the leaf node that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
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
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------
SHORT QDAMFindRecord_V2
(
    PBTREE   pBTIda,
    PCHAR_W  pKey,
    PBTREEBUFFER_V2 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc = 0;                           // return code
  PBTREEGLOB    pBT = pBTIda->pBTree;

  LogMessage(FATAL, "called commented out function QDAMFindRecord_V2");
  #ifdef TEMPORARY_COMMENTED
  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V2( pBTIda, pBT->usFirstNode, ppRecord, FALSE  );
  while ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V2( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V2( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V2( pBTIda, recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endwhile */
  #endif
  return( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddToBuffer       Add Data to buffer
//------------------------------------------------------------------------------
// Function call:     QDAMAddToBuffer( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Add the passed data to the string buffer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be inserted
//                    PCHAR                  buffer for user data
//                    PRECPARAM              contains position of data stored
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
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if terse flag is on
//                      terse the data
//                    endif
//                    get list for currently used record type
//                    check if sufficient space is available to
//                       fit into one of these records
//                    if it fits
//                      read record
//                      fill in key data, insert reference
//                      set address of key data
//                      mark if tersed or not
//                      write record update list entry
//                    endif
//                    if record is too large or no space left in list of recs
//                       scan list and find empty slot
//                       if not found
//                         remove slot which is most filled
//                       endif
//                       get record and store start position
//                       fill it up to length or record size
//                       update list entry
//                    endif
//                    return record parameters
//
//------------------------------------------------------------------------------
SHORT  QDAMAddToBuffer_V2
(
   PBTREE  pBTIda,                     // pointer to btree struct
   PBYTE   pData,                      // pointer to data
   ULONG   ulDataLen,                  // length of data to be filled in
   PRECPARAM  precReturn               // pointer to return code
)
{
   SHORT sRc = 0;                      // success indicator

   LogMessage(FATAL, "called commented out function QDAMAddToBuffer_V2");
  #ifdef TEMPORARY_COMMENTED
   USHORT  usFilled;                   // number of bytes filled in record
   USHORT  usMaxRec= 0;                // most filled record
   RECPARAM recStart;                  // return code where data are stored
   PRECPARAM pRecParam;                // parameter to stored list
   PRECPARAM pRecTemp ;                // parameter to stored list
   BOOL  fFit = FALSE;                 // indicator if record found
   USHORT  i;                          // index
   PBTREEHEADER  pHeader;              // record header
   PBTREEBUFFER_V2 pRecord = NULL;        // buffer record
   PBTREEBUFFER_V2 pTempRecord;           // buffer record
   CHAR     chNodeType;                // type of node
   BOOL     fTerse = FALSE;            // data are not tersed
   USHORT   usLastPos;
   ULONG    ulFullDataLen;             // length of record
   PUSHORT  pusOffset;
   PCHAR    pRecData;                  // pointer to record data
   ULONG    ulFitLen;                  // currently used length
  PBTREEGLOB    pBT = pBTIda->pBTree;
   USHORT   usLenFieldSize;            // size of length field

   memset(&recStart, 0, sizeof(recStart));
   /*******************************************************************/
   /* Enlarge pTempRecord area if it is not large enough to contain   */
   /* the data for this record                                        */
   /*******************************************************************/
#ifdef _DEBUG
   {
     CHAR szTemp[8];
     sprintf( szTemp, "%ld", ulDataLen );
     INFOEVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 4711, DB_GROUP, szTemp );
   }
#endif
   if ( (ulDataLen + sizeof(ULONG)) > pBT->ulTempRecSize )
   {
     if ( UtlAlloc( (PVOID *)&(pBT->pTempRecord), pBT->ulTempRecSize, ulDataLen + sizeof(ULONG), NOMSG ) )
     {
       pBT->ulTempRecSize = ulDataLen + sizeof(ULONG);
     }
     else
     {
       sRc = BTREE_NO_ROOM;
     } /* endif */
   } /* endif */

  if ( pBT->usVersion >= NTM_VERSION2 )
  {
    usLenFieldSize = sizeof(ULONG);
  }
  else
  {
    usLenFieldSize = sizeof(USHORT);
  } /* endif */

   pRecParam = pBT->DataRecList;
   /*******************************************************************/
   /* determine type of compression....                               */
   /*******************************************************************/
   switch ( pBT->fTerse )
   {
     case  BTREE_TERSE_HUFFMAN :
       fTerse = QDAMTerseData( pBTIda, pData, &ulDataLen, pBT->pTempRecord );
       break;
     default :
       fTerse = FALSE;
       break;
   } /* endswitch */

   chNodeType = DATA_NODE;

   // data should be in pBT->pTempRecord for further processing
   // copy them if not yet done.....
   if ( !fTerse )
   {
      memcpy( pBT->pTempRecord, pData, ulDataLen );
   } /* endif */

   if ( !sRc && (ulDataLen <= BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER)) )
   {
      pRecTemp = pRecParam;
      i = 0;
      //  find slot in list
      usFilled = (USHORT)(BTREE_REC_SIZE_V2 - ulDataLen - 2 * usLenFieldSize);
      while ( !fFit && i < MAX_LIST )
      {
         if ( pRecTemp->usOffset == 0 || pRecTemp->usOffset > usFilled )   // will not fit
         {
            pRecTemp++;
            i++;
         }
         else                                     // data will fit
         {
            fFit = TRUE;
         } /* endif */
      } /* endwhile */


      if ( fFit )
      {
         // store position where data will be stored
         recStart.usNum    = pRecTemp->usNum;

         // get record, copy data and write them
         sRc = QDAMReadRecord_V2( pBTIda, recStart.usNum, &pRecord, FALSE  );
         if ( !sRc )
         {
            BTREELOCKRECORD( pRecord );
            if ( TYPE( pRecord ) != chNodeType )
            {
               sRc = BTREE_CORRUPTED;
               ERREVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
            }
            else
            {
               // fill in key data
               usLastPos = pRecord->contents.header.usLastFilled;
               usLastPos = usLastPos - (USHORT)(ulDataLen + usLenFieldSize);
               pData = pRecord->contents.uchData + usLastPos;
               // insert reference
               if ( pBT->usVersion >= NTM_VERSION2 )
               {
                 *(PULONG) pData = ulDataLen;
               }
               else
               {
                 *(PUSHORT) pData = (USHORT)ulDataLen;
               }
               memcpy( pData+usLenFieldSize, pBT->pTempRecord, ulDataLen );

               // set address of key data at next free offset
               pusOffset = (PUSHORT) pRecord->contents.uchData;
               *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

               OCCUPIED(pRecord) += 1;
               pRecord->contents.header.usLastFilled = usLastPos;
               pRecord->contents.header.usFilled = pRecord->contents.header.usFilled + (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

               // mark if tersed or not
               if ( fTerse )
               {
                 if ( pBT->usVersion >= NTM_VERSION2 )
                 {
                   *(PULONG) pData |= QDAM_TERSE_FLAGL;
                 }
                 else
                 {
                   *(PUSHORT) pData |= QDAM_TERSE_FLAG;
                 }
               } /* endif */
//             pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

               sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
               // update list entry
               if ( !sRc )
               {
                  pRecTemp->usOffset = pRecord->contents.header.usFilled;
               } /* endif */
            } /* endif */
            BTREEUNLOCKRECORD( pRecord );
         } /* endif */
      } /* endif */
   } /* endif */

   //  either record is too large or no space left in list of records

   if ( !sRc && !fFit )
   {
      pRecTemp = pRecParam;
      i = 0;
      // scan list and find empty slot
      while ( (i < MAX_LIST) && !fFit )
      {
         if ( pRecTemp->usNum == 0 )     // empty slot found
         {
            fFit = TRUE;
         }
         else
         {
            pRecTemp++;                  // point to next slot
            i++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* if not found remove slot which is most filled                */
      /****************************************************************/
      if ( !fFit )
      {
         pRecTemp = pRecParam;
         usFilled = 0;
         for ( i=0 ; i < MAX_LIST ;i++ )
         {
            if ( pRecTemp->usOffset > usFilled)
            {
               usMaxRec = i;
               usFilled = pRecTemp->usOffset;
            } /* endif */
            pRecTemp++;                // next record in list
         } /* endfor */
         // set pRecTemp to the selected slot
         pRecTemp = pRecParam + usMaxRec;
      } /* endif */

      // get record and store start position
      memset( pRecTemp, 0 , sizeof(RECPARAM));

      sRc = QDAMNewRecord_V2( pBTIda, &pRecord, DATAREC );
      // fill it up to length or record size
      if ( ! sRc )
      {
         TYPE( pRecord ) = chNodeType;     // indicate that it is a data node
         BTREELOCKRECORD( pRecord );
         pRecData = (PCHAR)(pBT->pTempRecord + ulDataLen);
         ulFullDataLen = ulDataLen;         // store original data len
         while ( !sRc && ulDataLen + pRecord->contents.header.usFilled  >= (BTREE_REC_SIZE_V2 - (usLenFieldSize + sizeof(SHORT))))
         {
           /***********************************************************/
           /* get a new record, anchor it and fill data from end      */
           /* new record will be predecessor of already allocated rec.*/
           /***********************************************************/
           sRc = QDAMNewRecord_V2( pBTIda, &pTempRecord, DATAREC );
           if ( !sRc )
           {
             TYPE( pTempRecord ) = chNodeType;   // data node
             NEXT( pTempRecord ) = RECORDNUM( pRecord );
             PREV( pRecord ) = RECORDNUM( pTempRecord );
             TYPE( pRecord ) = DATA_NEXTNODE;              // data node

             // fill in key data
             /*********************************************************/
             /* adjust sizes of relevant data                         */
             /*********************************************************/
             usLastPos = pRecord->contents.header.usLastFilled;
             {
               ULONG ulMax = (ULONG)(BTREE_REC_SIZE_V2 - pRecord->contents.header.usFilled );
               ulFitLen = min( ulDataLen, ulMax );
               ulFitLen -= (ULONG)(usLenFieldSize + sizeof(USHORT));
             }
             usLastPos = usLastPos - (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
             pData = pRecord->contents.uchData + usLastPos;
             pRecData -= ulFitLen;
             ulDataLen -= ulFitLen;
             /*********************************************************/
             /* store length and copy data of part contained in record*/
             /*********************************************************/
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData = ulFitLen;
             }
             else
             {
               *(PUSHORT) pData = (USHORT)ulFitLen;
             }
             memcpy( pData+usLenFieldSize, pRecData,  ulFitLen );
             // set address of key data at next free offset
             pusOffset = (PUSHORT) pRecord->contents.uchData;
             *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

             OCCUPIED(pRecord) += 1;
             pRecord->contents.header.usLastFilled = usLastPos;
             pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                           (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

             sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
             BTREEUNLOCKRECORD( pRecord );

             /*********************************************************/
             /* toggle for adressing purposes                         */
             /* now: pRecord will be the free one for later filling   */
             /*********************************************************/
             pRecord = pTempRecord;
             BTREELOCKRECORD( pRecord );
           } /* endif */
         } /* endwhile */

         /*************************************************************/
         /* write out rest (this is the normal way in most cases      */
         /*************************************************************/
         if ( !sRc )
         {
           pHeader = &(pRecord->contents.header);
           pHeader->usParent = sizeof(BTREEHEADER );

           // store position where data will be stored
           recStart.usNum    = pHeader->usNum;

           // fill in key data
           usLastPos = pRecord->contents.header.usLastFilled;

           usLastPos = usLastPos - (USHORT)(ulDataLen + (ULONG)usLenFieldSize);
           pData = pRecord->contents.uchData + usLastPos;
           pRecData -= ulDataLen;

           // insert reference
           if ( pBT->usVersion >= NTM_VERSION2 )
           {
             *(PULONG) pData = ulFullDataLen;
           }
           else
           {
             *(PUSHORT) pData = (USHORT)ulFullDataLen;
           }
           memcpy( pData+usLenFieldSize, pRecData, ulDataLen );

           // set address of key data at next free offset
           pusOffset = (PUSHORT) pRecord->contents.uchData;
           *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

           OCCUPIED(pRecord) += 1;
           pRecord->contents.header.usLastFilled = usLastPos;
           pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                    (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

           // mark if tersed or not
           if ( fTerse )
           {
             if ( pBT->usVersion >= NTM_VERSION2 )
             {
               *(PULONG) pData |= QDAM_TERSE_FLAGL;
             }
             else
             {
               *(PUSHORT) pData |= QDAM_TERSE_FLAG;
             }
           } /* endif */


           // write record
  //       pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
           sRc = QDAMWriteRecord_V2(pBTIda, pRecord);
           BTREEUNLOCKRECORD( pRecord );         // free the locking
           // update list entry
           if ( !sRc )
           {
              pRecTemp->usOffset = pHeader->usFilled;
              pRecTemp->usNum    = pHeader->usNum;
           } /* endif */
         } /* endif */
      } /* endif */
   } /* endif */

   //  return record parameters
   if ( !sRc )
   {
      precReturn->usNum = recStart.usNum - pBT->usFirstDataBuffer;
      precReturn->usOffset = OCCUPIED( pRecord ) - 1;
   }
   else
   {
      memset(&recStart, 0, sizeof(RECPARAM));
      precReturn = &recStart;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMADDTOBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
    #endif
   return sRc;
}

SHORT QDAMInsertKey_V2
(
   PBTREE       pBTIda,
   PBTREEBUFFER_V2 pRecord,               // record where key is to be inserted
   PCHAR_W      pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
)
{
  SHORT i = 0;
  PBTREEBUFFER_V2 pTempRec;
  PCHAR_W   pCompKey = NULL;           // key to be compared with
  PCHAR_W   pOldKey;                   // old key at first position
  PCHAR_W   pNewKey;                   // new key at first position
  BOOL fFound = FALSE;
  SHORT  sKeyFound;                    // key found
  SHORT  sNearKey;                     // key found
  SHORT  sRc = 0;                      // return code

  LogMessage(FATAL, "called commented out function QDAMInsertKey_V2");
  #ifdef TEMPORARY_COMMENTED
  USHORT usLastPos;
  USHORT usKeyLen = 0;                 // length of key
  USHORT usDataRec = 0;                // length of key record
  PBYTE  pData;
  PUSHORT  pusOffset = NULL;
  USHORT usCurrentRecord;              // current record
  SHORT  sCurrentIndex;                // current index
  PBTREEGLOB    pBT = pBTIda->pBTree;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

  recKey;                              // get rid of compiler warnings
  //  check if key is already there -- duplicates will not be supported
  sRc = QDAMLocateKey_V2( pBTIda, pRecord, pKey, &sKeyFound, FEXACT, &sNearKey, FEXACT );
  BTREELOCKRECORD( pRecord );
  fRecLocked = TRUE;
  if ( !sRc )
  {
     if ( sKeyFound != -1)
     {
       sRc = BTREE_DUPLICATE_KEY;
     }
     else
     {
       i = (SHORT) OCCUPIED(pRecord);
       usKeyLen = (USHORT)((pBT->fTransMem) ? sizeof(ULONG) : (UTF16strlenBYTE(pKey) + sizeof(CHAR_W)));

       if ( pBT->usVersion >= NTM_VERSION2 )
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAM);
       }
       else
       {
         usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAMOLD);
       } /* endif */
       /* If node is full. Split the node before inserting */
       if ( usDataRec+sizeof(USHORT)+pRecord->contents.header.usFilled > BTREE_REC_SIZE_V2 )
       {
         BTREEUNLOCKRECORD( pRecord );
         fRecLocked = FALSE;
         sRc = QDAMSplitNode_V2( pBTIda, &pRecord, pKey );
         if ( !sRc )
         {
           // SplitNode may have passed a new record back
           BTREELOCKRECORD( pRecord );
           fRecLocked = TRUE;

           /* if we split an inner node, the parent of the key we are */
           /* inserting must be changed                               */
           if ( !IS_LEAF( pRecord ) )
           {
             sRc = QDAMReadRecord_V2(pBTIda, recData.usNum, &pTempRec, FALSE );
             if ( ! sRc )
             {
               PARENT( pTempRec ) = RECORDNUM( pRecord );
//             pTempRec->ulCheckSum = QDAMComputeCheckSum( pTempRec );
               sRc = QDAMWriteRecord_V2( pBTIda, pTempRec );
             } /* endif */
           } /* endif */
         } /* endif */
       } /* endif */
     } /* endif */
  } /* endif */


  if ( !sRc )
  {
    i = (SHORT) OCCUPIED(pRecord);               // number of keys in record
    // Insert key at the proper location
    pusOffset = (PUSHORT) pRecord->contents.uchData;
    while ( i > 0 && !fFound )
    {
       pCompKey = QDAMGetszKey_V2( pRecord, (SHORT)(i-1), pBT->usVersion );
       if ( pCompKey )
       {
          if ( (*(pBT->compare))(pBTIda, pKey, pCompKey)  < 0 )
          {
             *(pusOffset+i) = *(pusOffset+i-1);
             i--;
          }
          else
          {
             fFound = TRUE;
          } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 1, DB_GROUP, NULL );
       } /* endif */
    } /* endwhile */

    // set new current position
    pBTIda->sCurrentIndex = i;
    pBTIda->usCurrentRecord = RECORDNUM( pRecord );
  } /* endif */

  if ( !sRc )
  {
      CHAR chHeadTerm[128];
      // fill in key data
      usLastPos = pRecord->contents.header.usLastFilled;
      usLastPos = usLastPos - usDataRec;
      pData = pRecord->contents.uchData + usLastPos;
      // insert reference
      if ( !pBT->fTransMem && pBT->usVersion == BTREE_VERSION)
      {
         Unicode2ASCII( pKey, chHeadTerm, 0L );
         usKeyLen = (USHORT)(strlen(chHeadTerm)+1);
         pKey = (PSZ_W)&chHeadTerm[0];
      }
      *(PUSHORT) pData = usKeyLen;
      {
        PBYTE pTarget;
        if ( pBT->usVersion >= NTM_VERSION2 )
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAM);
        }
        else
        {
          pTarget = pData + sizeof(USHORT) + sizeof(RECPARAMOLD);
        } /* endif */

        memcpy(pTarget, (PBYTE) pKey, usKeyLen );

      }
      // set data information
      if ( pBT->usVersion >= NTM_VERSION2 )
      {
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recData, sizeof(RECPARAM));
      }
      else
      {
        RECPARAMOLD recDataOld;
        recDataOld.usOffset = recData.usOffset;
        recDataOld.usNum    = recData.usNum;
        recDataOld.sLen     = (SHORT)recData.ulLen;
        memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recDataOld, sizeof(RECPARAMOLD));
      } /* endif */

      // set address of key data at offset i
      *(pusOffset+i) = usLastPos;

      OCCUPIED(pRecord) += 1;
      pRecord->contents.header.usLastFilled = usLastPos;
      pRecord->contents.header.usFilled += usDataRec + sizeof(USHORT);
//    pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

      sRc = QDAMWriteRecord_V2(pBTIda,pRecord);
  } /* endif */

  // If this is the first key in the record, we must adjust pointers above
  if ( !sRc )
  {
    if ((i == 0) && (!IS_ROOT(pRecord)))
    {
       pOldKey = QDAMGetszKey_V2( pRecord, 1, pBT->usVersion );
       pNewKey = QDAMGetszKey_V2( pRecord, 0, pBT->usVersion );
       if ( pOldKey && pNewKey )
       {
         if ( pCompKey && PARENT(pRecord) )
         {
            /**********************************************************/
            /* save old current record since ChangeKey will update it */
            /* (implicit call to QDAMInsertKey) and restore it ...   */
            /**********************************************************/
            usCurrentRecord = pBTIda->usCurrentRecord;
            sCurrentIndex   = pBTIda->sCurrentIndex;
            sRc = QDAMChangeKey_V2( pBTIda, PARENT(pRecord), pOldKey, pNewKey);
            pBTIda->usCurrentRecord = usCurrentRecord;
            pBTIda->sCurrentIndex   = sCurrentIndex;

         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 10, DB_GROUP, NULL );
       } /* endif */
    } /* endif */
  } /* endif */

  if ( pRecord && fRecLocked )
  {
    BTREEUNLOCKRECORD(pRecord);
  } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMINSERTKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */
  #endif
  return sRc;
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
   DEBUGEVENT2( QDAMDICTINSERTLOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, NULL );


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
          
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V3( pBTIda, pKey, &pNode );

        } /* endif */
      } /* endif */

      if ( !sRc )
      {
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V3( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V3 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );
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
          
          QDAMDictUpdStatus ( pBTIda );
          sRc = QDAMFindRecord_V2( pBTIda, pKey, &pNode );
        } /* endif */
      } /* endif */

      if ( !sRc )
      {
          BTREELOCKRECORD( pNode );
          sRc = QDAMAddToBuffer_V2( pBTIda, pData, ulLen, &recData );
          if ( !sRc )
          {
            recData.ulLen = ulLen;
            sRc = QDAMInsertKey_V2 (pBTIda, pNode, pKey, recKey, recData);
          } /* endif */

          BTREEUNLOCKRECORD( pNode );
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
     sRc = QDAMPhysLock( pBTIda, FALSE, NULL );
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTINSERTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

   return sRc;
}


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
     ERREVENT2( QDAMUPDSIGNLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, NULL );
   } /* endif */

  return sRc;
}


