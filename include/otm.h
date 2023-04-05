#ifndef _OTM_INCLUDED_
#define _OTM_INCLUDED_

#include <vector>
#include <atomic>
#include <string>
#include <time.h>
#include <sstream>
#include "FilesystemHelper.h"
#include "win_types.h"

#define ENTRYENCODE_LEN    15          // number of significant characters
#define MAX_LIST           20          // number of recently used records
#define COLLATE_SIZE      256          // size of the collating sequence
#define ENTRYDECODE_LEN    32          // length of decoding array
#define MAX_UPD_CTR        10          // max number of update counters

#define MAX_LONGPATH           260     // max length of fully qualified path
typedef char * PSZ;

typedef LONG  TIME_L;                  // new typedef to avoid conflicts with Lotus

typedef struct _RECPARAM
{
    USHORT  usNum;           // record number
    USHORT  usOffset;        // record offset
    ULONG   ulLen;           // record length
}  RECPARAM, * PRECPARAM;

/**********************************************************************/
/* typedef used for all vital information on our new approach...      */
/* NOTE: this structure is limited to COLLATE_SIZE, which is 256      */
/**********************************************************************/
typedef struct _TMVITALINFO
{
  ULONG   ulStartKey;                  // key to start with
  ULONG   ulNextKey;                   // currently active key
} NTMVITALINFO, * PNTMVITALINFO;



#define MAX_MEM_DESCRIPTION     41     // length of a memory description field
#define _TMX_SIGN_SZ_NAME 128
//#define _TMX_SIGN_SZ_NAME MAX_FILESPEC

#define MAX_COMPACT_SIZE    3217   // 256

#define MAX_LANG_LENGTH         20     // length of the name of a language
typedef struct _TMX_SIGN
{
  TIME_L lTime=0;
  BYTE bGlobVersion=0;  
  BYTE bMajorVersion=0;
  BYTE bMinorVersion=0;
  CHAR szName[_TMX_SIGN_SZ_NAME];
  CHAR szSourceLanguage[MAX_LANG_LENGTH];
  CHAR szDescription[MAX_MEM_DESCRIPTION];

} TMX_SIGN, * PTMX_SIGN;



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

  bool  fOpen;                                // open flag/corruption flag
  USHORT    usFirstNode;                          // first node record
  USHORT    usFirstLeaf;                          // first leaf record
  RECPARAM  DataRecList[ MAX_LIST ];          // last used data records
  bool  fTerse;                               // tersing requested
  BYTE      chCollate[COLLATE_SIZE];              // collating sequence to use
  BYTE      chCaseMap[COLLATE_SIZE];              // case mapping to be used
  BYTE      chEntryEncode[ ENTRYENCODE_LEN ];     // significant characters
  USHORT    usFreeKeyBuffer;                      // index of buffer to use
  USHORT    usFreeDataBuffer;                     // first data buffer chain
  USHORT    usFirstDataBuffer;                    // first data buffer
  ULONG     ulUpdCount;                           // last update counter
  USHORT    usNextFreeRecord;                     // Next record to expand to
} BTREEHEADRECORD, PBTREEHEADRECORD ;


typedef enum _RECTYPE
{
  DATAREC,                 // record contains data
  KEYREC,                  // record contains keys
  ROOTREC                  // record contains root key data
} RECTYPE;

/*****************************************************************************/
/* BTREERECORD  is the format of each of the blocks on the disk.             */
/*  This structure should be BTREE_REC_SIZE long                             */
/*****************************************************************************/

#define BTREE_REC_SIZE_V3 (16384)          // record size V3
#define BTREE_BUFFER_V3   (BTREE_REC_SIZE_V3 + 10*sizeof(USHORT)) // buffer size

#define FREE_SIZE_V3  (BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER))
typedef struct _BTREERECORD_V3
{
  BTREEHEADER  header;                             // 16 bytes header
  UCHAR        uchData[ FREE_SIZE_V3 ] ;              // free size to be used
} BTREERECORD_V3, *PBTREERECORD_V3, **PPBTREERECORD_V3;

/*****************************************************************************/
/* BTREEBUFFER  is the format of the buffers when read in to the buffer      */
/* cache.  It maps a record number and its properties to its contents        */
/*****************************************************************************/
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


#define HEADTERM_SIZE     256          // size of the head term


/*****************************************************************************/
/* BTree is the memory resident structure describing the current state of    */
/* the B tree.  It contains the house keeping information for all of the     */
/* functions                                                                 */
/*****************************************************************************/

typedef SHORT _PFN_QDAMCOMPARE( PVOID, PVOID, PVOID );
typedef _PFN_QDAMCOMPARE *PFN_QDAMCOMPARE;

typedef struct _LOOKUPENTRY_V3
{
  PBTREEBUFFER_V3 pBuffer; // Pointer to BTREEBUFFER
} LOOKUPENTRY_V3, *PLOOKUPENTRY_V3;



typedef struct _ACCESSCOUNTERTABLEENTRY
{
  ULONG ulAccessCounter;
} ACCESSCTRTABLEENTRY, * PACCESSCTRTABLEENTRY;


typedef LHANDLE HTM;




#define MAX_SERVER_NAME         15     // max. length of a server name
#define MAX_USERID              15     // max. length of LAN user ID
#define MAXDATASIZE    0x8000          // maximum data size allowed

typedef struct _BTREEIDA
{   
  FileBuffer fb;
  //{From BTREEGLOB

   USHORT       usFirstNode=0;                    // file pointer of record
   USHORT       usFirstLeaf=0;                    // file pointer of record

   PBTREEINDEX_V3  pIndexBuffer_V3 = nullptr;     // Pointer to index records
   USHORT       usIndexBuffer=0;                  // number of index buffers
   PFN_QDAMCOMPARE compare = nullptr;             // Comparison function
   USHORT       usNextFreeRecord;                 // Next record to expand to
   //CHAR         chFileName[144];                // Name of B-tree file
   RECPARAM     DataRecList[ MAX_LIST ];          // last used data records
   BOOL         fGuard=0;                         // write every record
   BOOL         fOpen=0;                          // open flag
   BOOL         fCorrupted=0;                     // mark as corrupted
   WCHAR        TempKey[HEADTERM_SIZE];           // pointer to temp. key
   BYTE         TempRecord[MAXDATASIZE];          // pointer to temp record
   ULONG        ulTempRecSize = MAXDATASIZE;      // size of temp record area
   BOOL         fTerse=0;                         // tersing requested
   BYTE         chEntryEncode[ ENTRYENCODE_LEN];  // significant characters
   BYTE         bEncodeLen[COLLATE_SIZE];         // encoding table length
   CHAR         chEncodeVal[COLLATE_SIZE];        // encoding table
   BYTE         chDecode[ENTRYDECODE_LEN];        // decoding table
   BYTE         chCollate[COLLATE_SIZE];          // collating sequence to use
   BYTE         chCaseMap[COLLATE_SIZE];          // case mapping to be used
   USHORT       usFreeKeyBuffer=0;                // index of buffer to use
   USHORT       usFreeDataBuffer=0;               // first data buffer chain
   USHORT       usFirstDataBuffer=0;              // first data buffer
   BTREEBUFFER_V3  BTreeTempBuffer_V3;            // temporary V3 buffer
   BOOL         fWriteHeaderPending=0;            // write of header pending
   LONG         lTime=0;                          // time of last update/open
   USHORT       usVersion=0;                      // version identification...
   CHAR         chEQF[7];                         // The type of file
   BYTE         bVersion=0;                       // version flag
   USHORT       usOpenFlags=0;                    // settings used for open
   LONG         alUpdCtr[MAX_UPD_CTR];            // list of update counters
   HFILE        fpDummy=nullptr;                  // dummy/lock semaphore file handle
   BOOL         fUpdated=0;                       // database-has-been-modified flag
   USHORT       usNumberOfLookupEntries=0;        // Number of allocated lookup-table-entries
   USHORT       usNumberOfAllocatedBuffers=0;     // Number of allocated buffers
   ULONG        ulReadRecCalls=0;                 // Number of calls to QDAMReadRecord
   PLOOKUPENTRY_V3 LookupTable_V3=nullptr;        // Pointer to lookup-table
   PACCESSCTRTABLEENTRY AccessCtrTable=nullptr;   // Pointer to access-counter-table
   BYTE         bRecSizeVersion=0;                  // record size version flag
  //} //end of PBTREEGLOB


  // the following define moved from eqfqdami.h since it must be known in
  // eqfdorg.c
  /**********************************************************************/
  /* To check that we are opening a valid B-tree file, there is a       */
  /* magic cookie stored at the beginning.  This is as follows          */
  /* This value might be changed to include version identifications...  */
  /**********************************************************************/
  #define BTREE_VERSION3      3



  HTM          htm=nullptr;                      // handle in remote case
  SHORT        sCurrentIndex=0;                  // current sequence array
  USHORT       usCurrentRecord=0;                // current sequence record
  USHORT       usDictNum=0;                      // index in global structure
  wchar_t      chHeadTerm[HEADTERM_SIZE];        // last active head term
  BOOL         fLock=0;                          // head term is locked
  wchar_t      chLockedTerm[HEADTERM_SIZE];      // locked term if any


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMUpdSign  Write User Data
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Writes the second part of the first record (user data)
//                    This is done using the original QDAMDictUpdSignLocal
//                    function
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    USHORT                 length of user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+
// NOTE:              This function could be implemented as MACRO too, but
//                    for consistency reasons, the little overhead was used...
//+----------------------------------------------------------------------------+



//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMSign      Read signature record
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );
//+----------------------------------------------------------------------------+
// Description:       Gets the second part of the first record ( user data )
//                    This is done using the original QDAMDictSignLocal func.
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//
//+----------------------------------------------------------------------------+
// Side effects:      return signature record even if dictionary is corrupted
//+----------------------------------------------------------------------------+
// NOTE:              This function could be implemented as MACRO too, but
//                    for consistency reasons, the little overhead was used...
//+----------------------------------------------------------------------------+

SHORT EQFNTMSign
(
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
);

//SHORT
//EQFNTMInsert
//(
//  PULONG  pulKey,           // pointer to key
//  PBYTE   pData,            // pointer to user data
//  ULONG   ulLen             // length of user data
//);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMInsert
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMInsert( pBTIda, &ulKey, pData, usLen );
//+----------------------------------------------------------------------------+
// Description:       insert a new key (ULONG) with data
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE  pBTIda,      pointer to binary tree struct
//                    PULONG  pulKey,      pointer to key
//                    PBYTE   pData,       pointer to user data
//                    ULONG   ulLen        length of user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range
//                    BTREE_READONLY       file is opened read only - no write
//                    BTREE_CORRUPTED      file is corrupted
//                    errors returned by QDAMDictInsertLocal
//                    0                    success indicator
//+----------------------------------------------------------------------------+
SHORT
EQFNTMInsert
(
  PULONG  pulKey,           // pointer to key
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMUpdate
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMUpdate( pBTIda,  ulKey, pData, usLen );
//+----------------------------------------------------------------------------+
// Description:       update the data of an already inserted key
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE  pBTIda,      pointer to binary tree struct
//                    ULONG   ulKey,      key value
//                    PBYTE   pData,       pointer to user data
//                    ULONG   ulLen        length of user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range
//                    BTREE_READONLY       file is opened read only - no write
//                    BTREE_CORRUPTED      file is corrupted
//                    errors returned by QDAMDictInsertLocal
//                    0                    success indicator
//+----------------------------------------------------------------------------+
SHORT
EQFNTMUpdate
(
  ULONG   ulKey,            // key value
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGet
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMGet( pBTIda, ulKey, chData, &usLen );
//+----------------------------------------------------------------------------+
// Description:       get the data string for the passed key
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBTIda,       pointer to btree struct
//                    ULONG  ulKey,        key to be searched for
//                    PCHAR  pchBuffer,    space for user data
//                    PULONG  pulLength    in/out length of returned user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       same as for QDAMDictExactLocal...
//+----------------------------------------------------------------------------+
SHORT
EQFNTMGet
(
   ULONG  ulKey,                       // key to be searched for
   PCHAR  pchBuffer,                   // space for user data
   PULONG  pulLength                   // in/out length of returned user data
);



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGetMaxNumber
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMGetNextNumber( pBTIda, &ulKey, &ulNextFree );
//+----------------------------------------------------------------------------+
// Description:       get the start key and the next free key ...
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBTIda,       pointer to btree struct
//                    PULONG pulStartKey   first key
//                    PULONG pulNextKey    next key to be assigned
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0     always
//+----------------------------------------------------------------------------+
// Function flow:     access data from internal structure
//+----------------------------------------------------------------------------+
SHORT
EQFNTMGetNextNumber
(
   PULONG pulStartKey,                 // return start key number
   PULONG pulNextKey                   // return next key data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMPhysLock
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMPhysLock( pBTIda );
//+----------------------------------------------------------------------------+
// Description:       Physicall lock or unlock database.
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE             The database to be locked
//                    BOOL               TRUE = LOCK, FALSE = Unlock
//                    PBOOL              ptr to locked flag (set to TRUE if
//                                       locking was successful
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
SHORT EQFNTMPhysLock
(
   BOOL           fLock,
   PBOOL          pfLocked
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     QDAMIncrUpdCounter      Inrement database update counter
//+----------------------------------------------------------------------------+
// Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )
//
//+----------------------------------------------------------------------------+
// Description:       Update one of the update counter field in the dummy
//                    /locked terms file
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    SHORT                  index of counter field
//                                                                       PLONG                                                                  ptr to buffer for new counte value
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//+----------------------------------------------------------------------------+
// Function flow:     read update counter from dummy file
//                    increment update counter
//                    position ptr to begin of file
//                    write update counter to disk
//+----------------------------------------------------------------------------+
SHORT EQFNTMIncrUpdCounter
(
   SHORT      sIndex,                  // index of update counter
   PLONG                         plNewValue                                               // ptr to buffer for new counte value
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGetUpdCounter       Get database update counter
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );
//+----------------------------------------------------------------------------+
// Description:       Get one or more of the the database update counters
//                    from the dummy/locked terms file
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PLONG                  ptr to buffer for update counter
//                    SHORT                  index of requested update counter
//                    SHORT                  number of counters requested
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
SHORT EQFNTMGetUpdCounter
(
   PLONG      plUpdCount,               // ptr to buffer for update counter
   SHORT      sIndex,                   // index of requested update counter
   SHORT      sNumCounters              // number of counters requested
);

  
   VOID  QDAMUpdateList_V3( PBTREEBUFFER_V3 );
   
 VOID  QDAMFreeFromList_V3(PRECPARAM ,PBTREEBUFFER_V3 );
 SHORT QDAMFreeRecord_V3( PBTREEBUFFER_V3 pRecord, RECTYPE  recType/* data or key record*/);

 ULONG QDAMGetrecDataLen_V3 ( PBTREEBUFFER_V3, SHORT );
 SHORT QDAMDeleteDataFromBuffer_V3( RECPARAM recParam);
 SHORT QDAMDictUpdateLocal ( PWCHAR, PBYTE, ULONG );
 
 SHORT QDAMDictExactLocal  ( PWCHAR, PBYTE, PULONG, USHORT );
 SHORT EQFNTMCreate
(
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   ULONG  ulStartKey                  // first key to start automatic insert...
);


 SHORT QDAMDictCreateLocal ( TMX_SIGN*, ULONG );
                             
 SHORT QDAMDictInsertLocal ( PWCHAR, PBYTE, ULONG );
 BOOL   QDAMDictLockStatus ( PWCHAR );
 VOID   QDAMDictUpdStatus ();
 SHORT QDAMFindRecord_V3( PWCHAR, PBTREEBUFFER_V3 * );


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

SHORT  QDAMReadRecord_V3
(
   USHORT  usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL    fNewRec
);


 SHORT QDAMAddToBuffer_V3( PBYTE, ULONG, PRECPARAM);
 SHORT QDAMWriteHeader ();

SHORT QDAMDictUpdSignLocal
(
   PTMX_SIGN  pSign                   // pointer to user data
);

 SHORT QDAMReadRecordFromDisk_V3( USHORT, PBTREEBUFFER_V3 *, BOOL );
 
 SHORT QDAMWRecordToDisk_V3( PBTREEBUFFER_V3 );
 SHORT QDAMWriteRecord_V3( PBTREEBUFFER_V3  pBuffer);
 SHORT QDAMAllocKeyRecords(   USHORT usNum);

 } BTREE, * PBTREE, ** PPBTREE, BTREEGLOB, * PBTREEGLOB, ** PPBTREEGLOB ;


 
//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMClose  close the TM file
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMClose( PPBTREE );
//+----------------------------------------------------------------------------+
// Description:       Close the file
//+----------------------------------------------------------------------------+
// Parameters:        PPBTREE                pointer to btree structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//+----------------------------------------------------------------------------+

SHORT EQFNTMClose(PBTREE);

  
/**********************************************************************/
/* NTM prototypes                                                     */
/**********************************************************************/
#define NTMREQUESTNEWKEY 0xFFFFFFFF
//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMCreate
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMCreate( pTMName, pUserData, usLen, ulStart, &pNTM);
//+----------------------------------------------------------------------------+
// Description:       This function will create the appropriate Transl.Memory
//                    file.
//+----------------------------------------------------------------------------+
// Parameters:        PSZ  pTMName,         name of file to be created
//                    PCHAR  pUserData,     user data
//                    USHORT usLen,         length of user data
//                    ULONG  ulStartKey,    first key to start automatic insert
//                    PBTREE * ppBTIda      pointer to structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//+----------------------------------------------------------------------------+
SHORT
EQFNTMCreate
(
   PSZ  pTMName,                       // name of file to be created
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   ULONG  ulStartKey,                  // first key to start automatic insert...
   PBTREE  * ppBTIda                    // pointer to structure
);


//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMOpen     Open a Translation Memory file
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMOpen( PSZ, USHORT, PPBTREE );
//+----------------------------------------------------------------------------+
// Description:       Open a file locally for processing
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    USHORT           open flags
//                    PPBTREE          pointer to btree structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_ILLEGAL_FILE not a valid dictionary
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+

SHORT  EQFNTMOpen
(
  PSZ   pName,                        // name of the file
  USHORT usOpenFlags,                 // Read Only or Read/Write
  BTREE  * ppBTIda                    // pointer to BTREE structure
);

// function to organize index file
USHORT EQFNTMOrganizeIndex
(
   PBTREE,
   USHORT         usOpenFlags,         // open flags to be used for index file
   ULONG          ulStartKey           // first key to start automatic insert...
);

// Values for update counter indices
#define RESERVED_UPD_COUNTER     0
#define COMPACTAREA_UPD_COUNTER  1
#define AUTHORS_UPD_COUNTER      2
#define FILENAMES_UPD_COUNTER    3
#define LANGUAGES_UPD_COUNTER    4
#define TAGTABLES_UPD_COUNTER    5
#define LONGNAMES_UPD_COUNTER    6
#define MAX_UPD_COUNTERS        20



//table entry structure
typedef struct _TMX_TABLE_ENTRY
{
  CHAR   szName[MAX_LANG_LENGTH];
  USHORT usId;
} TMX_TABLE_ENTRY, * PTMX_TABLE_ENTRY;
// name table structure (TM version 5 and up)
typedef struct _TMX_TABLE
{
  ULONG  ulAllocSize;
  ULONG  ulMaxEntries;
  TMX_TABLE_ENTRY stTmTableEntry;
} TMX_TABLE, * PTMX_TABLE;

//signature structure




typedef char *PSZ;

// table entry structure for long document name table
typedef struct _TMX_LONGNAME_TABLE_ENTRY
{
  PSZ    pszLongName;                  // ptr to long name in buffer area
  USHORT usId;                         // ID for long name
} TMX_LONGNAME_TABLE_ENTRY, * PTMX_LONGNAME_TABLE_ENTRY;


//table structure for long document table
typedef struct _TMX_LONGNAME_TABLE
{
  PSZ    pszBuffer;                              // buffer for names and IDs
  ULONG  ulBufUsed;                              // number of bytes used in buffer
  ULONG  ulBufSize;                              // size of buffer in bytes
  ULONG  ulTableSize;                            // table size (# entries)
  ULONG  ulEntries;                              // number of entries in table
  TMX_LONGNAME_TABLE_ENTRY stTableEntry[1];      // dyn. array of table entries
} TMX_LONGNAMETABLE, * PTMX_LONGNAME_TABLE;



//TM control block structure
struct TMX_CLB
{
  BTREE TmBtree;
  BTREE InBtree;
  TMX_TABLE Languages;
  TMX_TABLE FileNames;
  TMX_TABLE Authors;
  TMX_TABLE TagTables;
  USHORT usAccessMode;
  USHORT usThreshold;
  TMX_SIGN stTmSign;
  BYTE     bCompact[MAX_COMPACT_SIZE-1];
  BYTE     bCompactChanged;
  LONG     alUpdCounter[MAX_UPD_COUNTERS];
  PTMX_LONGNAME_TABLE pLongNames;
  TMX_TABLE LangGroups;              //  table containing language group names
  PSHORT     psLangIdToGroupTable;     // language ID to group ID table
  LONG       lLangIdToGroupTableSize; // size of table (alloc size)
  LONG       lLangIdToGroupTableUsed; // size of table (bytes in use)
  PVOID      pTagTable;               // tag table loaded for segment markup (TBLOADEDTABLE)

  // copy of long name table sorted ignoring the case of the file names
  // Note: only the stTableEntry array is filled in this area, for all other
  //       information use the entries in the pLongNames structure
  PTMX_LONGNAME_TABLE pLongNamesCaseIgnore;

  // fields for work area pointers of various subfunctions which are allocated
  // only once for performance reasons
  PVOID      pvTempMatchList;        // matchlist of FillMatchEntry function
  PVOID      pvIndexRecord;          // index record area of FillMatchEntry function
  PVOID      pvTmRecord;             // buffer for memory record used by GetFuzzy and GetExact
  ULONG      ulRecBufSize;           // current size of pvTMRecord;

  // fields for time measurements and logging
  BOOL       fTimeLogging;           // TRUE = Time logging is active
  LONG64     lAllocTime;             // time for memory allocation
  LONG64     lTokenizeTime;          // time for tokenization
  LONG64     lGetExactTime;          // time for GetExact
  LONG64     lOtherTime;             // time for other activities
  LONG64     lGetFuzzyTime;          // time for GetFuzzy
  LONG64     lFuzzyOtherTime;        // other time spent in GetFuzzy
  LONG64     lFuzzyTestTime;         // FuzzyTest time spent in GetFuzzy
  LONG64     lFuzzyGetTime;          // NTMGet time spent in GetFuzzy
  LONG64     lFuzzyFillMatchEntry;   // FillMatchEntry time spent in GetFuzzy
  LONG64     lFillMatchAllocTime;    // FillMatchEntry: allocation time
  LONG64     lFillMatchOtherTime;    // FillMatchEntry: other times
  LONG64     lFillMatchReadTime;     // FillMatchEntry: read index DB time
  LONG64     lFillMatchFillTime;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchCleanupTime;  // FillMatchEntry: cleanup match list time
  LONG64     lFillMatchFill1Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill2Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill3Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill4Time;     // FillMatchEntry: fill match list time
  
  
};

typedef TMX_CLB* PTMX_CLB;

//prefix for each output structures
typedef struct _TMX_PREFIX_OUT
{
  USHORT usLengthOutput;               //length of complete output structure
  USHORT usTmtXRc;                     //function returncode
} TMX_PREFIX_OUT, * PTMX_PREFIX_OUT, XOUT, * PXOUT;

typedef struct _TMX_CREATE_OUT
{
  TMX_PREFIX_OUT  stPrefixOut;
  PTMX_CLB pstTmClb;
} TMX_CREATE_OUT, * PTMX_CREATE_OUT;


typedef struct _TMX_OPEN_OUT
{
  TMX_PREFIX_OUT stPrefixOut;
  PTMX_CLB pstTmClb;
} TMX_OPEN_OUT, * PTMX_OPEN_OUT;


struct ImportStatusDetails{
  std::atomic_short usProgress{0};
  std::atomic_int segmentsImported{-1};
  std::atomic_int invalidSegments{-1};
  std::atomic_int invalidSymbolErrors{-1};
  std::string importTimestamp;
  std::stringstream importMsg;
  
  void reset(){
    importMsg.str("");
    usProgress = 0;
    segmentsImported = -1;
    invalidSegments = -1;
    invalidSymbolErrors = -1;
    importTimestamp = "not finished";    
  }
  std::string toString(){
    std::string res;
    res = "progress = " + std::to_string(usProgress) + "; segmentsImported = " + std::to_string(segmentsImported) + "; invalidSegments = " + std::to_string(invalidSegments) + "; time = " + importTimestamp;
    return res;
  }
};


  /*! \brief States of a memory
  */
  typedef enum 
  {
    OPEN_STATUS,            // memory is available and open
    AVAILABLE_STATUS,       // memory has been imported but is not open yet
    IMPORT_RUNNING_STATUS,  // memory import is running
    IMPORT_FAILED_STATUS    // memory import failed
  } MEMORY_STATUS;

//#define _PROPHEAD_SZNAME 13
//#define _PROPHEAD_SZNAME MAX_FILESPEC

#define _PROPHEAD_SZNAME 144
//#define _PROPHEAD_SZPATH MAX_PATH144

// Structure of properties heading area
typedef struct _PROPHEAD
{
   USHORT usClass;              // Class name ID of properties
   char chType;                 // Indicator to type of properties
   //char dummy;                  // T=Template, I=Instance, N=New
   char szName[_PROPHEAD_SZNAME];    // Path to properties
} PROPHEAD, *PPROPHEAD;


/**********************************************************************/
/* Marker for NTM in new TM properties                                */
/**********************************************************************/
#define NTM_MARKER "#@@NTM01@@#"

/**********************************************************************/
/* TM property structure                                              */
/**********************************************************************/
typedef struct _PROP_NTM
{
  PROPHEAD stPropHead;
  TMX_SIGN stTMSignature;
  CHAR     szNTMMarker[sizeof(NTM_MARKER)];
  USHORT   usThreshold;

} PROP_NTM, *PPROP_NTM;


class NewTM{
  public:
  NewTM();
  NewTM(const std::string& tmName);


  BTREE TmBtree;
  BTREE InBtree;
  TMX_TABLE Languages;
  TMX_TABLE FileNames;
  TMX_TABLE Authors;
  TMX_TABLE TagTables;
  USHORT usAccessMode;
  USHORT usThreshold;
  TMX_SIGN stTmSign;
  BYTE     bCompact[MAX_COMPACT_SIZE-1];
  BYTE     bCompactChanged;
  LONG     alUpdCounter[MAX_UPD_COUNTERS];
  PTMX_LONGNAME_TABLE pLongNames = nullptr;
  TMX_TABLE pLangGroups;              //  table containing language group names
  PSHORT     psLangIdToGroupTable;     // language ID to group ID table
  LONG       lLangIdToGroupTableSize; // size of table (alloc size)
  LONG       lLangIdToGroupTableUsed; // size of table (bytes in use)
  PVOID      pTagTable;               // tag table loaded for segment markup (TBLOADEDTABLE)

  // copy of long name table sorted ignoring the case of the file names
  // Note: only the stTableEntry array is filled in this area, for all other
  //       information use the entries in the pLongNames structure
  PTMX_LONGNAME_TABLE pLongNamesCaseIgnore;

  // fields for work area pointers of various subfunctions which are allocated
  // only once for performance reasons
  PVOID      pvTempMatchList= nullptr;;        // matchlist of FillMatchEntry function
  PVOID      pvIndexRecord = nullptr;;          // index record area of FillMatchEntry function
  PVOID      pvTmRecord = nullptr;;             // buffer for memory record used by GetFuzzy and GetExact
  ULONG      ulRecBufSize;           // current size of pvTMRecord;

  // fields for time measurements and logging
  BOOL       fTimeLogging;           // TRUE = Time logging is active
  LONG64     lAllocTime;             // time for memory allocation
  LONG64     lTokenizeTime;          // time for tokenization
  LONG64     lGetExactTime;          // time for GetExact
  LONG64     lOtherTime;             // time for other activities
  LONG64     lGetFuzzyTime;          // time for GetFuzzy
  LONG64     lFuzzyOtherTime;        // other time spent in GetFuzzy
  LONG64     lFuzzyTestTime;         // FuzzyTest time spent in GetFuzzy
  LONG64     lFuzzyGetTime;          // NTMGet time spent in GetFuzzy
  LONG64     lFuzzyFillMatchEntry;   // FillMatchEntry time spent in GetFuzzy
  LONG64     lFillMatchAllocTime;    // FillMatchEntry: allocation time
  LONG64     lFillMatchOtherTime;    // FillMatchEntry: other times
  LONG64     lFillMatchReadTime;     // FillMatchEntry: read index DB time
  LONG64     lFillMatchFillTime;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchCleanupTime;  // FillMatchEntry: cleanup match list time
  LONG64     lFillMatchFill1Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill2Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill3Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill4Time;     // FillMatchEntry: fill match list time
};


#define MAX_EQF_PATH           255     // max length of a file in an EQF dir

// message buffer to keep info about last message
#define MSGBUFFERLENGTH   2048         // length of error message buffer
typedef struct _DDEMSGBUFFER
{
  HWND  hwndDDE;                       // handle of DDE window
  SHORT sErrNumber;                    // error number / return code
  CHAR  chMsgText[MSGBUFFERLENGTH];    // associated message text
} DDEMSGBUFFER, *PDDEMSGBUFFER;


typedef CHAR  OBJNAME[MAX_EQF_PATH];
typedef OBJNAME *POBJNAME;

// internal session data area
typedef struct _FCTDATA
{
  ULONG       lMagicWord;              // FUNCDATA area identifier
  LONG        lCheckSum;               // checksum of FUNCDATA area
  SHORT       sLastFunction;           // last function performed

  // area for session data
  std::atomic<bool>        fComplete;               // current process is complete flag
  CHAR        szEqfResFile[MAX_EQF_PATH];// name of resource file
  CHAR        szSystemPropPath[MAX_EQF_PATH]; // system properties path
  CHAR        szMsgFile[MAX_EQF_PATH]; // message file for error messages
  CHAR        szError[256]; // text buffer for error messages
  DDEMSGBUFFER LastMessage;            // buffer for last error message
  HWND        hwndErrMsg;              // handle for error mesages (HWND_FUNCIF)

  // area used by mem import
  PVOID       pvMemLoadIda;            // pointer to analysis input data
  USHORT      usMemLoadPhase;          // current/next phase of TM import
  USHORT      usMemLoadRC;             // return code

  // area used by mem organize
  PVOID       pvMemOrganizeCommArea;   // pointer to organize CommArea

  // area used by mem export
  PVOID       pvMemExportCommArea;

  // general use buffer area
  OBJNAME     szObjName;               // buffer for object names

  // current progress of some nonDDE functions
  //USHORT      usProgress;
  USHORT      usExportProgress;

  ImportStatusDetails*     pImportData;


  USHORT MemFuncExportProcess();
  USHORT MemFuncPrepExport
(
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
);
} FCTDATA, *PFCTDATA;

#endif // _OTM_INCLUDED_