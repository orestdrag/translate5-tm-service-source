/*! EQFQDAMI.H
	Description: internal header file for QDAM
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/
#ifndef _EQFQDAMI_H_
#define _EQFQDAMI_H_

#include <atomic>

#define MIN_SIZE          128          // minimum free space requested in entry
#define MAXWASTESIZE      512          // max size  to be wasted due to updates
#define MAX_LOCKREC_SIZE 4000          // max size for locked terms record
#define USERDATA_START   2046          // start of user data
#define INDEX_BUFFERS      20          // index buffers
#define NUMBER_OF_BUFFERS  20          // 20 // number of buffers to be used

#define RESET_VALUE       -3

#define RETRY_COUNT        5           // number of retries for BTREE_IN_USE condition

// max time [ms] to wait for resources currently in use
#define MAX_WAIT_TIME     100

// max number of retries for resources currently in use
#define MAX_RETRY_COUNT    30

/**********************************************************************/
/* defines for the tersing to be used....                             */
/**********************************************************************/
#define BTREE_TERSE_HUFFMAN  1
#define BTREE_TERSE_LZSSHMAN 2
#define  QDAM_TERSE_FLAG   0x8000
#define  QDAM_TERSE_FLAGL   0x80000000L
/**********************************************************************/
/* To check that we are opening a valid B-tree file, there is a       */
/* magic cookie stored at the beginning.  This is as follows          */
/* This value might be changed to include version identifications...  */
/**********************************************************************/

// BTREE databases of version NTM_VERSION2 support data records with
// a size of more than 32k, the length field at the begin of the
// data record is of type ULONG instead of USHORT

#define NTM_VERSION3       3
#define BTREE_HEADER_VALUE_TM3 "NTM"

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




typedef struct _STENCODEBITS
{
  USHORT usLen;
  USHORT usVal;
} STENCODEBITS;




/* Every MAX_READREC_CALLS calls to QDAMReadRecord unlocked records with
   access counter < MAX_READREC_CALLS will be written to disk and memory
   will be freed.
   Every read access of a record will increase the access counter of that
   record by ACCESSBONUSPOINTS */
//#ifndef _WINDOWS
  #define MAX_READREC_CALLS 1000000L
  #define ACCESSBONUSPOINTS 5000L
//#else
//  /* To avoid swapping under Windows the values are very low */
//  #define MAX_READREC_CALLS 2000L
//  #define ACCESSBONUSPOINTS 100L
//#endif




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





/* Access Macros */
#define TYPE(x)          ((x)->contents.header.chType)
#define LEAF_NODE        0x00
#define INNER_NODE       0x01
#define ROOT_NODE        0x02
#define DATA_NODE        0x04
#define DATA_NEXTNODE    0x08
#define DATA_KEYNODE     0x10

#define IS_LEAF(x)          ((TYPE(x) & INNER_NODE) == 0)
#define IS_ROOT(x)          ((TYPE(x) & ROOT_NODE) != 0)

#define PREV(x)             ((x)->contents.header.usPrevious)
#define NEXT(x)             ((x)->contents.header.usNext)
#define PARENT(x)           ((x)->contents.header.usParent)
#define RECORDNUM(x)        ((x)->usRecordNumber)
#define OCCUPIED(x)         ((x)->contents.header.usOccupied)
#define FILLEDUP(x)         ((x)->contents.header.usFilled)


#define  MINFREEKEYS        3
// minimum keys to be in a record - otherwise try to reduce the node
#define  MIN_KEY            MINFREEKEYS
// lock a record for fixing it in the memory
#define  BTREELOCKRECORD(x)  ((x)->fLocked=TRUE)
// test if a record is locked
#define  BTREETESTLOCKRECORD(x)  ((x)->fLocked)
// unlock a (previously locked) record
#define  BTREEUNLOCKRECORD(x)  ((x)->fLocked=FALSE)


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictOpen   Open Dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictOpen( PSZ, PSZ, SHORT, BOOL, PBTREE * );
//
//+----------------------------------------------------------------------------+
// Description:       Open a file for processing
//
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of bytes per record
//                    BOOL             TRUE  read/write FALSE  read/only
//                    PPBTREE          pointer to btree structure
//
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
SHORT  QDAMDictOpen
(
  PSZ   pName,                        // name of the file
  PSZ   pServer,                      // name of the server
  SHORT sNumberOfBuffers,             // number of buffers
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PBTREE * ppBT                       // pointer to BTREE structure
);

typedef struct _DICTOPEN
{
  HTM    htm;                         // pointer to BTREE structure
  SERVERNAME szServer;                // servername
  USHORT usName;                      // offset of name of file in chData
  SHORT  sNumberOfBuffers;            // number of buffers
  USHORT usOpenFlags;                 // Read Only or Read/Write
  BYTE   chVarData;                   // data pointer
} DICTOPEN, * PDICTOPEN;

///////////////////////////////////////////////////////////////////////////////
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCreate      Create Dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCreate( PSZ, PSZ, SHORT, PCHAR, USHORT, PCHAR,
//                                    PCHAR, PCHAR, PPBTREE);
//+----------------------------------------------------------------------------+
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
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map structure
//                    PBTREE *         pointer to btree structure
//
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
SHORT QDAMDictCreate
(
   PSZ    pName,                       // name of file
   PSZ    pServer,                     // name of the server
   SHORT  sNumberOfBuffers,            // number of buffers
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   PCHAR  pTermTable,                  // term encoding table
   PCHAR  pCollate,                    // pointer to collating sequence
   PCHAR  pCaseMap,                    // pointer to casemap sequence
   PBTREE * ppBT                       // pointer to btree structure
);
typedef struct _DICTCREATE
{
   HTM    htm;                         // pointer to BTREE structure
   SERVERNAME szServer;                // servername
   USHORT usName;                      // name of the file
   SHORT  sNumberOfBuffers;            // number of buffers
   USHORT usUserData;                  // user data
   USHORT usLen;                       // length of user data
   USHORT usTermTable;                 // term encoding table
   USHORT usCollate;                   // offset to collating sequence
   USHORT usCaseMap;                   // offset to casemap sequence
   BYTE   chVarData;                   // data pointer
} DICTCREATE, *PDICTCREATE;



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictSign    Read User Data
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSign( PBTREE, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Gets the second part of the first record ( user data )
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//
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
SHORT QDAMDictSign
(
   PBTREE pBT,                         // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
);

typedef struct _DICTSIGN
{
   USHORT usUserData;                  // pointer to user data
   USHORT usLen;                       // length of user data
   BYTE   chVarData;                   // data pointer
} DICTSIGN, * PDICTSIGN;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictInsert     Insert entry
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictInsert( PBTREE, PCHAR, PCHAR, USHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Add a key and all associated data.
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE          pointer to btree structure
//                    PCHAR           key to be inserted
//                    PCHAR           user data to be associated with the key
//                    USHORT          length of the user data
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
//
//+----------------------------------------------------------------------------+
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictInsert
(
  PBTREE  pBT,               // pointer to binary tree struct
  PWCHAR   pKey,              // pointer to key data
  PBYTE   pData,             // pointer to user data
  ULONG   ulLen              // length of user data
);

typedef struct _DICTINSERT
{
  USHORT  usKey;             // pointer to key data
  USHORT  usData;            // pointer to user data
  ULONG   ulLen;             // length of user data
  BYTE    chVarData;         // data pointer
} DICTINS, * PDICTINS;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictExact      Find Exact match
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictExact( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:        Find an exact match for the passed key
//
//+----------------------------------------------------------------------------+
// Parameters:         PBTREE               pointer to btree structure
//                     PCHAR                key to be inserted
//                     PCHAR                buffer for user data
//                     PUSHORT              on input length of buffer
//                                          on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//+----------------------------------------------------------------------------+
SHORT QDAMDictExact
(
   PBTREE   pBT,                         // pointer to btree struct
   PWCHAR  pKey,                        // key to be searched for
   PBYTE    pchBuffer,                   // space for user data
   PULONG   pulLength,                   // in/out length of returned user data
   USHORT   usSeachSubType  
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictSubStr   Find Key starting with stubstring
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSubStr(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//+----------------------------------------------------------------------------+
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    If no error happened set this location as new
//                    current position
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictSubStr
(
   PBTREE pBT,                         // pointer to btree struct
   PWCHAR  pKey,                        // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                  // in/out length of returned key data
   PBYTE  pchUserData,                 // space for user data
   PULONG pulUserLen                   // in/out length of returned user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictEquiv      Find equivalent match
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictEquiv(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//+----------------------------------------------------------------------------+
// Description:       Find the first key which is equivalent to
//                    the passed key
//                    If no error happened set this location as
//                    new current position
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE               pointer to btree structure
//                    PCHAR                key to be looked for
//                    PCHAR                buffer for the key
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//                    PCHAR                buffer for the user data
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictEquiv
(
   PBTREE pBT,                         // pointer to btree struct
   PWCHAR  pKey,                        // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                   // in/out length of returned key data
   PBYTE  pchUserData,                 // space for user data
   PULONG pulUserLen                   // in/out length of returned user data
);

typedef struct _DICTFIND
{
   USHORT usKey;                       // offset to key
   USHORT usBuffer;                    // space for key data
   ULONG  ulLength;                    // in/out length of returned user data
   USHORT usUserBuffer;                // space for user data
   ULONG  ulUserLen;                   // in/out length of returned user data
   BYTE   chVarData;                   // data pointer
} DICTFIND, * PDICTFIND;


typedef struct _DICTNUMENTRIES
{
   ULONG ulNum;                     // in/out length of returned user data
} DICTNUM, * PDICTNUM;


typedef struct _DICTLOCK
{
   BOOL      fLock;                    // lock/unlock dictionary
   USHORT    usKey;                    // pointer to key
   BYTE      chVarData;                // data pointer
} DICTLOCK, * PDICTLOCK;


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictLockEntry  lock/unlock the specified entry
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictLockEntry(PBTREE, PSZ, BOOL );
//+----------------------------------------------------------------------------+
// Description:       Locks/Unlocks the specified dictionary entry
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBT          pointer to btree structure
//                    PSZ    pEntry          dictionary entry
//                    BOOL   fLock           lock/unlock dictionary entry
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+
SHORT QDAMDictLockEntry
(
   PBTREE pBT,
   PSZ_W    pEntry,
   BOOL   fLock
);
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictFirst   Get the first entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictFirst( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the first entry and pass back the
//                    associated information into the user provided
//                    buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictFirst
(
   PBTREE     pBT,
   PWCHAR      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictNext   Get the next entry back -
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictNext( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the next entry (by collating sequence) and
//                    pass back the associated information into
//                    the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictNext
(
   PBTREE     pBT,
   PWCHAR    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictPrev     Get the prev entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictPrev( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the previous entry (by collating sequence)
//                    and pass back the associated
//                    information into the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictPrev
(
   PBTREE     pBT,
   PWCHAR      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCurrent   Get current entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCurrent( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT )
//
//+----------------------------------------------------------------------------+
// Description:       Locate the current entry  and pass back the
//                    associatedinformation into the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//                    BTREE_EOF_REACHED eof or start reached
//                    BTREE_CORRUPTED   dictionary corrupted
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictCurrent
(
   PBTREE     pBT,
   PBYTE      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCopy    Copy Entries
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCopy( PBTREE, PBTREE );
//
//+----------------------------------------------------------------------------+
// Description:       Insert the current entry from the source dictionary
//                    into the target dictionary and point to the next
//                    entry in the source dictionary
//                    Source and Target have to be both either local or remote
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE       pointer to btree struct of source dict
//                    PBTREE       pointer to btree struct of target dict
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictCopy
(
   PBTREE pBTSource,                // pointer of source tree structure
   PBTREE pBTTarget                 // pointer of target tree structure
);

typedef struct _DICTDELETE
{
   USHORT    usKey;                    // pointer to key
   BYTE      chVarData;                // data pointer
} DICTDEL, * PDICTDEL;



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictNumber  Position at passed entry number
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictNumber( PBTREE, ULONG, PUCHAR, PUSHORT, PUCHAR,
//                                    PUSHORT );
//+----------------------------------------------------------------------------+
// Description:       Try to position at the passed entry number
//                    and fill the provided area with the data.
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    ULONG                  term number
//                    PUCHAR                 pointer to space for term
//                    PUSHORT                length of key
//                    PUCHAR                 pointer to data
//                    PUSHORT                length of data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictNumber
(
    PBTREE   pBT,                    // pointer to generic structure
    ULONG    ulTermNum,              // term number
    PWCHAR   pKey,                   // pointer to space for term
    PULONG   pulKeyLen,              // length of key
    PBYTE    pData,                  // pointer to data
    PULONG   pulLen                  // length of data
);

/**********************************************************************/
/* structure used for end of remote organize                          */
/**********************************************************************/
typedef struct _DICTORG
{
  CHAR   szDictName[MAX_EQF_PATH];     // dictionary asd name
  CHAR   chPrimDrive;                  // remote primary drive
  USHORT usCloseRc;                 // organize/Close successful??
} DICTORG, * PDICTORG;


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictUpdTimeLocal
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdTimeLocal( pBTree, &lTime );
//+----------------------------------------------------------------------------+
// Description:       gets the update time
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE    pBT   pointer to control block
//                    PLONG     plTime   time of last open
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       sRc = LX_RC_OK_ASD -
//+----------------------------------------------------------------------------+
// Function flow:     retrieve the update/open time
//                    return
//+----------------------------------------------------------------------------+
 SHORT QDAMDictUpdTimeLocal ( PBTREE, PLONG );
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictUpdTime
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdTime     ( pBTree, &lTime );
//+----------------------------------------------------------------------------+
// Description:       gets the update time
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE    pBT   pointer to control block
//                    PLONG     plTime   time of last open
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       sRc = LX_RC_OK_ASD -
//+----------------------------------------------------------------------------+
// Function flow:     retrieve the update/open time
//                    return
//+----------------------------------------------------------------------------+
 SHORT QDAMDictUpdTime( PBTREE, PLONG );
/**********************************************************************/
/* structure used for DictUpdTime                                     */
/**********************************************************************/
typedef struct _DICTTIME
{
  LONG   lTime;                     // time of last update or open
} DICTTIME, *PDICTTIME;


/**********************************************************************/
/*  define some items for measuring performance                       */
/**********************************************************************/
#if defined(MEASURE)
    USHORT usASI;
    USHORT usASD;
    ULONG ulStart, ulEnd;
    ULONG ulBegin, ulTerseEnd;
    ULONG ulBeg;
    ULONG ulBuffer;
    ULONG ulSeek;
    ULONG ulUnTerseEnd;
    ULONG ulUpdate;
    ULONG ulFind;
    ULONG ulComp;
    ULONG ulBegComp;
    ULONG ulBegRead;
    ULONG ulBegRealRead;
    ULONG ulBegString;
    ULONG ulString;
    ULONG ulRealReadASD;
    ULONG ulRealReadASI;
    ULONG ulRead;
    ULONG ulGet;
    ULONG ulAdd;
    SEL selGlobalSeg, selLocalSeg;
    GINFOSEG far * pGlobInfoSeg;

    USHORT  usEntry;
    USHORT  usEntryLength;
    USHORT  usNumIndexBuffer[2];
  #if defined(DISTRIB)
    ULONG  ulChar[256];
  #endif
#endif


 SHORT QDAMDictEquivLocal  ( PBTREE, PWCHAR, PBYTE, PULONG,  PBYTE,PULONG );

 SHORT QDAMDictPrevLocal   ( PBTREE, PWCHAR, PULONG,  PBYTE, PULONG  );

 SHORT QDAMDictCurrentLocal( PBTREE, PBYTE, PULONG,  PBYTE, PULONG  );

 SHORT QDAMDictDeleteLocal ( PBTREE, PWCHAR );
 SHORT QDAMDictFlushLocal  ( PBTREE );
 SHORT QDAMDictCopyLocal   ( PBTREE, PBTREE );
 
 SHORT QDAMDictNumEntriesLocal ( PBTREE, PULONG );

 SHORT QDAMDictFirstLocal ( PBTREE, PWCHAR, PULONG, PBYTE, PULONG );


 SHORT QDAMInsertKey_V3( PBTREE, PBTREEBUFFER_V3, PWCHAR, RECPARAM, RECPARAM );
 SHORT QDAMFindParent_V3(PBTREE, PBTREEBUFFER_V3, PUSHORT );


 SHORT QDAMNewRecord_V3( PBTREE, PBTREEBUFFER_V3 *, RECTYPE );
 SHORT QDAMDeleteDataFromBuffer ( PBTREE, RECPARAM );


 SHORT QDAMGetszKeyParam_V3( PBTREE, RECPARAM, PWCHAR, PULONG );
 SHORT QDAMGetszData_V3 ( PBTREE, RECPARAM,  PBYTE, PULONG, CHAR );

 BOOL  QDAMDeleteKey_V3( PBTREE, PBTREEBUFFER_V3, PCHAR);
 SHORT QDAMDestroy ( PBTREE );
 SHORT QDAMKeyCompare ( PVOID, PVOID, PVOID );
 
 SHORT QDAMKeyCompareNonUnicode ( PVOID, PVOID, PVOID );
 SHORT QDAMFirst_V3( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMNext_V3( PBTREE, PRECPARAM,  PRECPARAM,PRECPARAM );
 SHORT QDAMPrev_V3( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMHeaderFirst ( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMHeaderNext ( PBTREE, PRECPARAM,  PRECPARAM,PRECPARAM );
 VOID  QDAMTerseInit ( PBTREE, PUCHAR );
 SHORT QDAMUnTerseData ( PBTREE, PUCHAR, ULONG, PULONG );
 SHORT QDAMValidateIndex_V3( PBTREE,  PBTREEBUFFER_V3 * );

 PSZ_W     QDAMGetszKey_V3 ( PBTREEBUFFER_V3,  USHORT, USHORT );
 RECPARAM  QDAMGetrecData_V3 ( PBTREEBUFFER_V3, SHORT, USHORT );
 VOID      QDAMSetrecData_V3 ( PBTREEBUFFER_V3, SHORT, RECPARAM, USHORT );
 RECPARAM  QDAMGetrecKey_V3 ( PBTREEBUFFER_V3, SHORT );
 VOID  QDAMReArrangeKRec_V3 ( PBTREE, PBTREEBUFFER_V3 );
 VOID  QDAMCopyKeyTo_V3( PBTREEBUFFER_V3, SHORT, PBTREEBUFFER_V3, SHORT, USHORT );
 VOID  QDAMCopyDataTo_V3( PBTREEBUFFER_V3, SHORT, PBTREEBUFFER_V3, SHORT, USHORT );
 SHORT QDAMLastEntry_V3( PBTREE, PBTREEBUFFER_V3 * );
 BOOL  QDAMFetchFromIndexList_V3( PBTREE, PBTREEBUFFER_V3, USHORT );
 VOID  QDAMAddToIndexList_V3( PBTREE, PBTREEBUFFER_V3 );


 SHORT  QDAMAllocKeyRecords ( PBTREE, USHORT );

 USHORT QDAMRemoveDict( PBTREE );
 SHORT  QDAMDictLockEntryLocal( PBTREE, PUSHORT, BOOL );
 USHORT QDAMMoveProperties ( PPROPDICTIONARY , PUSHORT );
 PBTREE QDAMFindInstance ( PBTREE, PUSHORT );

 SHORT QDAMDictCloseOrganize ( PPBTREE, PUSHORT, CHAR, USHORT );

 USHORT RenameOrganize( PUSHORT pszDictPath, CHAR chPrimDrive, USHORT usCloseRc );

 
 USHORT DamBTreeRc ( SHORT sRc );

 /*********************************************************************/
 /* our own Nlp layer...                                              */
 /*********************************************************************/
#define DAMLINK _loadds far pascal


void DAMLINK NlpLockEntryW ( USHORT, PSZ_W, BOOL, PUSHORT );

void DAMLINK NlpLockDict ( USHORT, BOOL, PUSHORT );

void DAMLINK NlpRetBTree ( USHORT, PVOID * , PLONG );

void DAMLINK NlpCloseOrganize( USHORT, PSZ, CHAR, PUSHORT, USHORT );

void DAMLINK NlpDictUpdTime( USHORT, PLONG, PUSHORT );

void DAMLINK NlpTermListW ( USHORT, USHORT, PSZ_W, USHORT, USHORT,
                                      PSZ_W, USHORT, PUSHORT );


void DAMLINK
  NlpBegAsd( USHORT,                 /* in  - max. files allowed  */
             USHORT,                 /* in  - max. indexes / base */
             USHORT,                 /* in  - max. associations   */
             USHORT,                 /* in  - max. files / assoc. */
             PUSHORT,                /* out - user handle         */
             PUSHORT);               /* out - return code         */


void DAMLINK
  NlpEndAsd( USHORT,                 /* in  - user handle         */
             PUSHORT);               /* out - return code         */

void DAMLINK
  NlpOpenAsd(PUCHAR,                /* in  - name of dictionary  */
             USHORT,                /* in  - number of pages     */
             USHORT,                /* in  - immediate writing?  */
             USHORT,                /* in  - user handle         */
             PUSHORT,               /* out - dictionary handles  */
             PUSHORT,               /* out - length of user area */
             PUSHORT);              /* out - return code         */


void DAMLINK
  NlpBuildAsd(PUCHAR,               /* in  - dictionary name     */
              USHORT,               /* in  - dictionary type     */
              USHORT,               /* in  - number of pages     */
              USHORT,               /* in  - immediate writing?  */
              USHORT,               /* in  - language            */
              USHORT,               /* in  - codepage            */
              USHORT,               /* in  - feature mask        */
              PUCHAR,               /* in  - primary collate seq.*/
              PUCHAR,               /* in  - secondary coll. seq.*/
              PUCHAR,               /* in  - edit command string */
              PUCHAR,               /* in  - term encoding table */
              PUCHAR,               /* in  - entry encoding table*/
              PUSHORT,              /* in  - length of user data */
              PUCHAR,               /* in  - user defined data   */
              PUCHAR,               /* in  - index name list     */
              USHORT,               /* in  - user handle         */
              PUSHORT,              /* out - dictionary handle   */
              PUSHORT);             /* out - return code         */


void DAMLINK
  NlpCloseAsd(USHORT,               /* in  - dictionary handle   */
              USHORT,               /* in  - user handle         */
              PUSHORT);             /* out - return code         */


void DAMLINK
  NlpAssocAsd(PUSHORT,              /* in  - dictionary handles  */
              USHORT,               /* in  - user handle         */
              PUSHORT,              /* out - association handles */
              PUSHORT);             /* out - return code         */

void DAMLINK
  NlpRetSignature(USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user handle         */
                  PUSHORT,          /* out - dictionary type     */
                  PUSHORT,          /* out - feature mask        */
                  PUSHORT,          /* out - codepage            */
                  PUCHAR,           /* out - primary collate seq.*/
                  PUCHAR,           /* out - secondary coll. seq.*/
                  PUCHAR,           /* out - edit command string */
                  PUCHAR,           /* out - term encoding table */
                  PUCHAR,           /* out - entry encoding tbl. */
                  PUCHAR,           /* out - dictionary names    */
                  PUSHORT,          /* out - length of user-data */
                  PUCHAR,           /* out - user-defined data   */
                  PUSHORT);         /* out - return code         */

void DAMLINK
  NlpUpdSignature(USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user handle         */
                  USHORT,           /* in  - protection mode     */
                  PUCHAR,           /* in  - dictionary names    */
                  USHORT,           /* in  - length of user data */
                  PUCHAR,           /* in  - user-defined data   */
                  PUSHORT);         /* out - return code         */


void DAMLINK
  NlpRenumberAsd(USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user area handle    */
                 PUSHORT);          /* out - return code         */

void DAMLINK
  NlpCompTermsAsd(PUCHAR,           /* in  - 1st compare term    */
                  PUCHAR,           /* in  - 2nd compare term    */
                  USHORT,           /* in  - type of compare     */
                  USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user area handle    */
                  PUSHORT,          /* out - result of compare   */
                  PUSHORT);         /* out - return code         */


void DAMLINK
  NlpFndBeginAsdW(PSZ_W,            /* in  - desired substring   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W ,            /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length   */
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT);          /* out - return code         */


void DAMLINK
  NlpFndEquivAsdW(PSZ_W,            /* in  - desired term        */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W,            /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length   */
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT);          /* out - return code         */


void DAMLINK
  NlpFndMatchAsdW(PSZ_W,            /* in  - desired term        */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PBYTE,             /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length in bytes*/
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT,           /* out - return code         */
                 USHORT);           /* in  - search subtype      */


void DAMLINK
  NlpFndNumberAsdW(ULONG,            /* in  - desired term number */
                  USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - relocation flag     */
                  USHORT,           /* in  - user handle         */
                  PSZ_W ,           /* out - matching term found */
                  PULONG,           /* out - entry data length   */
                  PUSHORT,          /* out - dictionary of match */
                  PUSHORT);         /* out - return code         */


void DAMLINK
  NlpNxtTermAsdW(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PSZ_W ,             /* out - matching term found */
                PULONG,             /* out - term number         */
                PULONG,             /* out - entry data length   */
                PUSHORT,            /* out - dictionary of match */
                PUSHORT);           /* out - return code         */

void DAMLINK
NlpNxtTermAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pMatch,
   PULONG pulNum,
   PULONG pulLen,
   PUSHORT pusDict,
   PUSHORT pusRc
);             // needed for organize of old dicts


void DAMLINK
  NlpPrvTermAsdW(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PSZ_W ,             /* out - matching term found */
                PULONG,             /* out - term number         */
                PULONG,             /* out - entry data length   */
                PUSHORT,            /* out - dictionary of match */
                PUSHORT);           /* out - return code         */


void DAMLINK
  NlpInsEntryAsdW(PSZ_W,            /* in  - term for new entry  */
                 PBYTE,            /* in  - data for term entry */
                 ULONG,             /* in  - entry data length   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PULONG,            /* out - inserted term's #   */
                 PUSHORT);          /* out - return code         */


void DAMLINK
  NlpUpdEntryAsdW(PSZ_W ,            /* in  - term to update      */
                 PBYTE ,            /* in  - data for term entry */
                 ULONG,             /* in  - entry data length in bytes   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PULONG,            /* out - updated term number */
                 PUSHORT);          /* out - return code         */


void DAMLINK
  NlpDelEntryAsdW(PSZ_W,            /* in  - term to be deleted  */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PUSHORT);          /* out - return code         */


void DAMLINK
  NlpRetEntryAsdW(USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W ,            /* out - term for this entry */
                 PULONG,            /* out - term number         */
                 PBYTE ,            /* out - data for term entry */
                 PULONG,            /* out - entry data length in bytes*/
                 PUSHORT,           /* out - dictionary of term  */
                 PUSHORT);          /* out - return code         */


void DAMLINK
NlpRetEntryAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pTerm,
   PULONG pulNum,
   PUCHAR pData,
   PULONG pulDataLen,
   PUSHORT pusDict,
   PUSHORT pusRc
);
void DAMLINK
  NlpRetSpellAsd(PUCHAR,            /* in  - input term          */
                 USHORT,            /* in  - dictionary handle   */
                 PUCHAR,            /* in  - term editing rules  */
                 USHORT,            /* in  - verify words?       */
                 USHORT,            /* in  - user handle         */
                 PUSHORT,           /* out - # of output terms   */
                 PUCHAR,            /* out - output term list    */
                 PULONG,            /* out - length of list      */
                 PUSHORT);          /* out - return code         */

void DAMLINK
  NlpRetListAsd(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - requested # terms   */
                USHORT,             /* in  - user handle         */
                PUSHORT,            /* out - output # terms      */
                PUCHAR,             /* out - output term list    */
                PULONG,             /* out - length of list      */
                PUSHORT);           /* out - return code         */


void DAMLINK
  NlpResynchAsd(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PUSHORT);           /* out - return code         */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     NTMKeyCompare      Generic compare function
//+----------------------------------------------------------------------------+
// Function call:     NTMKeyCompare( PBTREE,PULONG,PULONG);
//+----------------------------------------------------------------------------+
// Description:       This is the generic compare function used
//                    for comparision
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to tree structure
//                    PULONG                 first key
//                    PULONG                 second key
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0   keys are equal
//                    <> keys are unequal
//+----------------------------------------------------------------------------+

SHORT NTMKeyCompare
(
    PVOID  pBT,                     // pointer to tree structure
    PVOID  pulKey1,                    // pointer to first key
    PVOID  pulKey2                     // pointer to second key
);

/**********************************************************************/
/* validate the passed BTree pointer                                  */
/**********************************************************************/
#define CHECKPBTREE(pBTIda, sRc)                  \
     sRc = pBTIda ? 0 : BTREE_INVALID; 

  #define LENGTHOFDATA( pBT, pData ) *((PULONG)(pData)) 

  #define STARTOFDATA( pBT, pData ) (PBYTE)pData + sizeof(ULONG) 

  #define SETDATALENGTH( pBT, pData, ulLen )   *((PULONG)(pData)) = ulLen; 


SHORT QDAMPhysLock
(
   PBTREE         pBT,
   BOOL           fLock,
   PBOOL          pfLocked
);


SHORT QDAMIncrUpdCounter
(
   PBTREE     pBT,                  // pointer to btree structure
   SHORT      sIndex,                  // index of requested update counter
   PLONG      plNewValue               // ptr to buffer for new counte value
);



SHORT QDAMUpdateLockRec
(
   PBTREE     pBT,                  // pointer to btree structure
   PSZ_W      pszTerm,                 // term being added or removed
   BOOL       fAdd                     // TRUE = Add term to lock record
                                       // FALSE = remove term from lock record
);

SHORT QDAMDosRC2BtreeRC
(
  SHORT sDosRC,                        // Dos return code
  SHORT sDefaultRC,                    // RC for default case
  USHORT usOpenFlags                   // open flags of database
);


USHORT QDamWildCardList
(
  USHORT,             // in  - user handle
  USHORT,             // in  - dictionary handle
  PSZ_W,                // in - term to start with
  PSZ_W,                // in - search pattern/compound(s)
  USHORT,             // in - number of terms to be looked up
  BOOL,               // in - compound search flag
  PSZ_W,             // in - buffer for term list
  ULONG               // in - size of buffer
);

SHORT QDAMDictNextWildLocal
(
   PBTREE     pBT,
   PSZ_W      pPattern,                // search pattern or compound
   BOOL       fCompound,               // compound search flag
   PBYTE      pKeyData,                // pointer to space for key data
   PULONG     pulKeyLen                // length of space for key data
);

BOOL QDAMMatchCompound
(
  PSZ_W  pKey,
  PSZ_W  pCompound
);

BOOL QDAMMatchWild
(
  PUCHAR pKey,
  PUCHAR pPattern
);

ULONG QDAMComputeCheckSum_V3 (PBTREEBUFFER_V3 pRecord );
BOOL QDAMCheckCheckSum_V3( PBTREEBUFFER_V3 pRecord, SHORT sLocation );

// get nect dictionary of an association list

USHORT DAMGetNextDict
(
                 USHORT             /* in  - dictionary handle   */
);


PBTREE DamGetBTreeFromDamRec(USHORT usIndex);


#endif //_EQFQDAMI_H_
