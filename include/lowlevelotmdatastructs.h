#ifndef _LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_
#define _LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_

#include <atomic>
#include "win_types.h"


#define ENTRYENCODE_LEN    15          // number of significant characters
#define MAX_LIST           20          // number of recently used records
#define COLLATE_SIZE      256          // size of the collating sequence
#define ENTRYDECODE_LEN    32          // length of decoding array
#define MAX_UPD_CTR        10          // max number of update counters

typedef wchar_t CHAR_W;
typedef wchar_t * PCHAR_W;
typedef wchar_t * PSZ_W;

typedef char* PSZ;

// general defines

#define MAX_TGT_LANG     10            // max number of target languages
#define MAX_PROF_ENTRIES 100           // max number of profile entries

#define SEGDATABUFLEN 8096

#define MEM_IMPORT_COMPLETE  7001                // the import is complete
// translation source values for usTranslationFlag in MEMEXPIMPSEG sgtructure
#define TRANSL_SOURCE_MANUAL 0
#define TRANSL_SOURCE_MANCHINE 1
#define TRANSL_SOURCE_GLOBMEMORY 2


/**********************************************************************/
/* Equivalent to OS/2 SWP structure                                   */
/* Note: bFiller1 and bFiller2 have been inserted to enlarge the      */
/*       structure to the size of the OS/2 version (HWND has a        */
/*       size of 4 bytes under OS/2 and a size of 2 bytes under       */
/*       Windows)                                                     */
/**********************************************************************/
typedef struct _SWP { /* swp */
    USHORT  fs;
    SHORT   cy;
    SHORT   cx;
    SHORT   y;
    SHORT   x;
    HWND    hwndInsertBehind;
    HWND    hwnd;
} SWP;
typedef SWP *PSWP;
/**********************************************************************/
/* Type definitions for OS/2 types - the new types are length         */
/* compatible to the TKT1.3 types. Use these defines in all           */
/* property file definitions.                                         */
/**********************************************************************/
typedef UINT   WINMSG;
typedef SWP   EQF_SWP;
typedef PSWP  EQF_PSWP;
  #define NULLHANDLE 0
  #define SWP_FLAG(swp) swp.fs
  #define PSWP_FLAG(pswp) pswp->fs
  #define APIENTRY16 APIENTRY
  #define PSZ16 PSZ
  #define PBOOL16 PBOOL

// max length of segment data (number of characters)
#define EQF_SEGLEN 2048

// max length of a document name
#define EQF_DOCNAMELEN     256

// max length of a document name
#define EQF_AUTHORLEN      15 

#define  TEXT_LINE_LENGTH             40  // Text line length

// max length of a language name  
#define EQF_LANGLEN 20


// max length of a format/markup table name
#define EQF_FORMATLEN 13



typedef ULONG EQFINFO, *PEQFINFO;

/**********************************************************************/
/* constants                                                          */
/**********************************************************************/
/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH      260

//#define PROP_NTM_SZ_FULL_MEM_NAME_SIZE MAX_EQF_PATH
#define PROP_NTM_SZ_FULL_MEM_NAME_SIZE 54


#define MAXALLBATCH  50                     // max no of batches possible

/**********************************************************************/
/* size and length defines                                            */
/**********************************************************************/
#define MAX_FILESPEC           256     // max length of file name + extension
#define MAX_LONGFILESPEC       256     // max length of long file name + extension
#define MAX_PATH144            144     // max length of fully qualified path
#define MAX_FNAME              250     // max length of name part of a file names
#define MAX_FEXT                 5     // max length of a file name extension
#define MAX_FOLDERNAME         250     // max length of a folder name
#define MAX_DRIVELIST           27     // max size of a drive list
#define MAX_VIEW                20     // max. viewable # of columns in a CLB listbox
#define MAX_DEFINEDCOLUMNS      50     // max. definable # of columns in a CLB listbox
#define MAX_DESCRIPTION         41     // length of description fields
#define MAX_LONG_DESCRIPTION    300    // long length of description fields
#define MAX_DICTS               20     // max. number of dictionaries
#define MAX_SERVER_LIST_SIZE    100    // length of a server list in
                                       // system properties
#define MAX_LANGUAGE_PROPERTIES 200    // max. length of language properties
#define MAX_EDIT_LINE_LENGTH    66    // max. length of a line
#define MAX_SEGMENT_SIZE        4096  // maximum segment size
#define MAX_TERM_LEN            255   // maximum length of a single term
#define NUM_OF_DICTS            10    // number of dictionaries per folder
#define MAX_FINDCHANGE_LEN      80    // max length of find/change buffer
#define MAX_MOSTUSEDDOCSPERSEG   5    // max number of docs in most used segment
                                      // area of redundancy log
#define MAX_SEARCH_HIST         5     // max number of search history
#define MAX_FILELIST            8096  // max length of non-dde filelist buffer
#define MAX_DICTPID_VALUES      400   // max. length of dict. PID values



// max length of a memory description 
#define EQF_DESCRIPTIONLEN 41 


/**********************************************************************/
/* Marker for NTM in new TM properties                                */
/**********************************************************************/
#define NTM_MARKER "#@@NTM01@@#"


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


#include "Filebuffer.h"

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


#define  MEM_EXPORT_OUT_BUFFER      8200  // Output buffer for export

#define  MAX_HANDLER_NAME        40

#define MAX_DRIVE              250     // max size of a drive specification

#define MAX_EQF_PATH           255     // max length of a file in an EQF dir

//ida with dialog control information
typedef struct _CONTROLSIDA
{
 //path strings
 CHAR         szDrive[MAX_DRIVE];          //drive letter with :
 CHAR         szPath[MAX_LONGPATH];        //drive and directory/ies
 CHAR         szName[MAX_LONGPATH];        //filename without ext
 CHAR         szSelectedName[MAX_LONGPATH];//newly specified dictionary name
 CHAR         szExt[MAX_LONGPATH];         //file extension with leading dot
 CHAR         szPatternName[MAX_LONGPATH]; //active pattern name
 CHAR         szPathContent[MAX_LONGPATH]; //content of path entry field
 CHAR         szString[MAX_LONGPATH];      //string buffer
 CHAR         szDummy[MAX_EQF_PATH];       //string buffer
 CHAR         szDriveList[MAX_DRIVELIST];  //Drive list
 CHAR         szHandler[MAX_HANDLER_NAME]; //handler name
 //saved last used values
 CHAR         chSavedDrive;                //saved drive letter
 USHORT       usSavedFormat;               //format
 CHAR         szSavedPath[MAX_LONGPATH];   //saved path
 //boolean value if import or export dialog
 BOOL         fImport;                     //TRUE if import dialog
 //boolean value if wm_command ok button was pressed
 BOOL         fCommand;                    //TRUE if wm_command msg
 //boolean value if listbox is a multi-selection or single selection listbox
 BOOL         fMultiSelectionLB;           //TRUE if listbox is multi-selection
 //dialog control ids needed for both import and export
 USHORT       idCurrentDirectoryEF;        //id of current directory entry field
 USHORT       idPathEF;                    //id of path entry field
 USHORT       idDirLB;                     //id of directories listbox
 USHORT       idFilesLB;                   //id of files listbox
 USHORT       idInternalRB;                //id of internal radio button
 USHORT       idExternalRB;                //id of external radio button
 USHORT       idDriveBTN;                  //id of drive btn
 USHORT       idControlsGB;                //id of controls group box
 USHORT       idOkPB;                      //id of btn that triggers process
 //dialog control ids needed for export
 USHORT       idExportTEXT;                //id of static with filename for export
 //dialog control ids needed for import
 USHORT       idToLB;                            //id of to drop-down combo listbox
 CHAR         szDefPattern[MAX_LONGFILESPEC];  // default pattern to use
 BOOL         fLongFileNames;              //TRUE = allow long file names
} CONTROLSIDA, * PCONTROLSIDA;


#define  MEM_EXPORT_WORK_AREA       6000  // Length of export work area

typedef struct _MEM_PROC_HANDLE
{
 CHAR   * pDataArea;                   /* Pointer to process data area        */
 USHORT   usMessage;                   /* Termination message for process     */
}MEM_PROC_HANDLE, * PMEM_PROC_HANDLE;


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


typedef CHAR LANGUAGE[MAX_LANG_LENGTH];

// last used values
typedef struct _MEM_LAST_USED
{
  // Do only insert new values at the end of the stucture !!!!!!!!!!
  PROPHEAD stPropHead;                        // Common header of properties
  CHAR     szFormat[MAX_FILESPEC];            // Create dialog: last used format table
  CHAR     szExclusion[MAX_FILESPEC];         // Create dialog: last used exclusion list
  LANGUAGE szSourceLang;                      // Create dialog: last used source language
  LANGUAGE szTargetLang;                      // Create dialog: last used target language
  CHAR     szDrive[MAX_FILESPEC];             // Create dialog: last used local disk drive
  CHAR     szExpDriveLastUsed[MAX_DRIVE];     // Export dialog: last used drive format: ~x:~
  CHAR     szExpDirLastUsed[_MAX_DIR];        // Export dialog: last used directory format: ~\xxx\yyy\~
  // Structure end in Troja 2.1
  CHAR     szIncludeServer[MAX_SERVER_NAME];  // Include dialog: Last used server ~SSER0545~
  CHAR     szCreateServer[MAX_SERVER_NAME];   // Create dialog: Last used server ~SSER0545~
  BOOL     usCreateLocation;                  // Create dialog: Possible values: TM_LOCAL, TM_REMOTE
  CHAR     szRemoteDrive[MAX_FILESPEC];       // Create dialog: last used remote disk drive
  // Structure end in Troja 3.0
  CHAR     szImpPathLastUsed[MAX_PATH144];    // Import dlg: last used path
                                              // x:\xx
  // Structure end in Troja 3.c
  USHORT   usExpMode;                         // last used export mode
  USHORT   usImpMode;                         // last used import mode
  CHAR     szImpPathLastUsed2[MAX_LONGPATH];  // Import dlg: last used path (long)
}MEM_LAST_USED, * PMEM_LAST_USED;


typedef USHORT EQF_BOOL;
typedef PUSHORT PEQF_BOOL;



typedef CHAR  OBJNAME[MAX_EQF_PATH];
typedef OBJNAME *POBJNAME;


//--- common part of instance areas ---
typedef struct _IDA_HEAD
{
     PSZ          pszObjName;           // Object name ( old format )
     PVOID        hProp;                // Handle to object properties
     HWND         hFrame;               // Frame wnd handle
     EQF_BOOL     fMustNotClose;        // reject WM_TERMINATE etc. if TRUE
     OBJNAME      szObjName;            // Object name ( new format )
} IDA_HEAD, *PIDA_HEAD;

typedef struct _MEM_IDA
{
 IDA_HEAD stIdaHead;                     // Standard Ida head
 HWND     hWndListBox;                   // Handle of memory window listbox
 SHORT    sLastUsedViewList[MAX_VIEW];   // last used view list
 SHORT    sDetailsViewList[MAX_VIEW];    // user's details view list'
 SHORT    sSortList[MAX_VIEW];           // user's sort criteria list'
 HWND     hWnd;                          // Handle of memory window
 CHAR     szTemp[4096];                  // Temporary work area
 CHAR     szTempPath[MAX_EQF_PATH];      // Temporary work area for a path
 CHAR     szMemPath[4096];               // Full path to TM starting an operation
 CHAR     szMemName[MAX_LONGFILESPEC];   // (long) TM name
 CHAR     szMemObjName[2*MAX_LONGFILESPEC];   // memory object name ( pluginname + ":" + (long) TM name)
 CHAR     szPluginName[MAX_LONGFILESPEC]; // name of plugin for this memory
 LONG     lTemp;                         // Temporary long field
 CHAR     szMemFullPath[MAX_EQF_PATH];   // Full path to memory database
 CHAR     szSecondaryDrives[MAX_DRIVELIST]; // Drives of secondary installations
 CHAR   * pWorkPointerLoad;                 // A work pointer for the load proc.
// SHORT    sNumberOfMem;                     // Number of memory databases entries available
// SHORT    sNumberOfMemUsed;                 // Number of memory databases entries used
 PMEM_PROC_HANDLE  pMemProcHandles;         // pointer to List of proc. handle
 SHORT    sNumberOfProc;                    // Number of process handle entries
 SHORT    sNumberOfProcUsed;                // Number of proc. handle entries used
 SHORT    sRunningProcesses;                // Number of concurrently running processes
 CHAR     szText[TEXT_LINE_LENGTH];         // Containing text
// CHAR     szMemTopItem[MAX_FNAME +          // In case of refresh contains top item of listbox
//                       MAX_DESCRIPTION + 5];// the 5 additional char's are for \x15 control characters
// CHAR     szMemSelectedItem[MAX_FNAME +     // In case of refresh contains selected item of listbox
//                       MAX_DESCRIPTION + 5];// the 5 additional char's are for \x15 control characters
 CHAR     szProcessTaskWithObject[MAX_FILESPEC]; // A Task has to be processed with this Object
 CHAR     szNAString[TEXT_LINE_LENGTH];     // String to store the "N/A" string
}MEM_IDA, * PMEM_IDA;




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

typedef struct _DDERETURN
{
  USHORT  usRc;                        // error code
  CHAR chMsgBuf[256];                  // message text
} DDERETURN, *PDDERETURN;

/**********************************************************************/
/* structure which holds info for the task "translation memory export"*/
/**********************************************************************/
typedef struct _DDEMEMEXP
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szOutFile[CCHMAXPATH+1];     // output file name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  BOOL    fOverWrite;                  // TRUE = overwrite existing files
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMEXP, *PDDEMEMEXP;

// structure to pass/receive general memory information
typedef struct _MEMEXPIMPINFO
{
  BOOL    fUTF16;                          // TRUE = export in UTF16 format
  CHAR    szName[256];
  CHAR    szSourceLang[EQF_LANGLEN];
  CHAR    szDescription[EQF_DESCRIPTIONLEN];
  CHAR    szError[1024];                   // buffer for error messages
  BOOL    fError;                          // TRUE = error during parsing
  CHAR    szFormat[EQF_FORMATLEN];         // format / markup table name
  LONG    lOutMode;                        // converter only: memory output mode
  BOOL    fCleanRTF;                       // clean memory from RTF tags
  BOOL    fSourceSource;                   // converter only: convert to source/source memory
  BOOL    fTagsInCurlyBracesOnly;         // special option for fCleanRTF: only remove tags enclosed in curly braces
  BOOL    fNoCRLF;                        // remove CRLFs in segment text
  PSZ     pszMarkupList;                   // pointer to a list of markup tables (zero terminated C strigns followed by another zero)
  BYTE    fUnused[20];                     // room for enhancements
} MEMEXPIMPINFO, *PMEMEXPIMPINFO;


class EqfMemory;
class OtmProposal;
// structure to pass/receive segment data

typedef struct _MEMEXPIMPSEG
{
  BOOL             fValid;                       // FALSE = segment data is invalid, count as invalid segment
  LONG             lSegNum;                      // segment number
  CHAR             szFormat[EQF_FORMATLEN];      // format / markup table name
  CHAR             szSourceLang[EQF_LANGLEN];    // segment source language
  CHAR             szTargetLang[EQF_LANGLEN];    // segment target language
  CHAR             szDocument[EQF_DOCNAMELEN];   // document  
  unsigned short   usTranslationFlag;            // numeric value indication the source of the translation
  LONG             lTime;                        // segment update time in C time format
  CHAR             szAuthor[EQF_AUTHORLEN];      // author of segment
  unsigned short   usLength;                     // length of segment data (incl. string end delimiter)
  WCHAR            szSource[SEGDATABUFLEN+1];    // buffer for segment source
  WCHAR            szTarget[SEGDATABUFLEN+1 ];   // buffer for segment target
  CHAR             szReason[200];                // reason for invalid segment
  WCHAR            szContext[SEGDATABUFLEN+1];    // buffer for segment context
  WCHAR            szAddInfo[SEGDATABUFLEN+1];    // buffer for additional information of segment 
} MEMEXPIMPSEG, *PMEMEXPIMPSEG;


typedef struct _MEM_EXPORT_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// Memory database name without extension
 CHAR         szMemPath[2048];            // Full memory database name + path
 HTM          hMem;                       // Handle of memory database
 HFILE        hFile;                      // Handle of file the export should go to
 PMEM_IDA     pIDA;                       // Address to the memory database IDA
 //HPROP        hPropMem;                   // Memory database property handle
 PPROP_NTM    pPropMem;                   // pointer to TM properties
 //HPROP        hPropLast;                  // Last used property handle
 PMEM_LAST_USED pPropLast;                // Pointer to the last used properties
 BOOL         fAscii;                     // TRUE = ASCII else EQF internal format
 CHAR         szDummy[_MAX_DIR];          // string buffer
 USHORT       usEntryInDirToStop;         // The key where the export should stop
 ULONG        ulTotalBytesWritten;              // Total bytes written to extract file
 CHAR_W       szWorkArea[MEM_EXPORT_WORK_AREA]; // Work area for extract
 //TMX_EXT_IN_W  stExtIn;                         // TMX_EXT_IN
 //TMX_EXT_OUT_W stExtOut;                        // TMX_EXT_OUT
 ULONG        ulSegmentCounter;                 // Segment counter of progress window
 BOOL         fEOF;                             // Indicates end of file
 BOOL         fFirstExtract;                    // First extract call flag
 HWND         hProgressWindow;                  // Handle of progress indicator window
 ULONG        ulProgressPos;                        // position of progress indicator
 CHAR         acWriteBuffer[MEM_EXPORT_OUT_BUFFER]; // Export output buffer
 ULONG        ulBytesInBuffer;                      // No of bytes already in buffer
 CONTROLSIDA  ControlsIda;                      //ida of control utility
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMEXP   pDDEMemExp;                 // ptr to batch memory export data
 USHORT       NextTask;                   // next task for non-DDE batch TM export
 USHORT       usRC;                       // error code in non-DDE batch mode
 USHORT       usExpMode;                  // mode for export
 CHAR         szConvArea[MEM_EXPORT_WORK_AREA]; // Work area for extract
 PSZ           pszNameList;            // pointer to list of TMs being organized
 PSZ           pszActiveName;          // points to current name in pszNameList
 USHORT        usYesToAllMode;         // yes-to-all mode for replace existing files
 ULONG         ulOemCP;                // ASCII cp of system preferences language
 ULONG         ulAnsiCP;
 // fields for external memory export methods
 LONG          lExternalExportHandle;  // handle of external memory export functions
 //HMODULE       hmodMemExtExport;                 // handle of external export module/DLL
 PMEMEXPIMPINFO pstMemInfo;                        // buffer for memory information
 PMEMEXPIMPSEG  pstSegment;                        // buffer for segment data

 BOOL         fDataCorrupted;             // TRUE = data has been corrupted during export
 CHAR_W       szBackConvArea[MEM_EXPORT_WORK_AREA]; // Work area for conversion check
 EqfMemory    *pMem;                      // pointer to memory being exported
 OtmProposal  *pProposal;                   // buffer for proposal data
 CHAR_W       szSegmentBuffer[MAX_SEGMENT_SIZE]; // Buffer for segment data
 CHAR_W       szSegmentBuffer2[MAX_SEGMENT_SIZE]; // Buffer for preprocessed segment data
 int          iComplete;                  // completion rate
 CHAR         szPlugin[MAX_LONGFILESPEC]; // name of memory plugin handling the current memory database
}
MEM_EXPORT_IDA, * PMEM_EXPORT_IDA;



/**********************************************************************/
/* Defines for the layout of the process window                       */
/**********************************************************************/
typedef enum _PROCWINSTYLE
{
  PROCWIN_TEXTONLY,                    // Process window contains a single
                                       // text line (e.g. folder import );
  PROCWIN_SLIDERONLY,                  // Process window contains a slider
                                       // control (e.g. folder export)
  PROCWIN_TEXTSLIDER,                  // Process window contains a text line
                                       // and a slider control (e.g. tm export)
  PROCWIN_SLIDERENTRY,                 // Process window contains a slider
                                       // control and a groupbox containing a
                                       // text line
                                       // for the current entry (e.g. dict export)
  PROCWIN_TEXTSLIDERENTRY,             // Process window contains a text line
                                       // and a slider control (e.g. tm export)
                                       // and a groupbox containing a text line
                                       // for the current entry (e.g. dict export)
  PROCWIN_TEXTSLIDERLISTBOX,           // Process window contains a text line
                                       // and a slider control (e.g. tm export)
                                       // and a two listboxes (e.g. analysis)
  PROCWIN_BATCH                        // Process window is invisible (for use
                                       // in batch mode processes)
} PROCWINSTYLE;


/**********************************************************************/
/* OS/2 PM types directly mapped to Windows types                     */
/**********************************************************************/
#define HPOINTER    HICON



/**********************************************************************/
/* Structure for the communication between generic process window     */
/* instance procedure and the process instance callback function      */
/**********************************************************************/
typedef struct _PROCESSCOMMAREA
{
  /********************************************************************/
  /* Variables independent of the type of the process window          */
  /********************************************************************/
  CHAR             szObjName[1024];    // R/O: object name of process window
                                       // value is supplied by ProcessInstance
  PROCWINSTYLE     Style;              // R/W: style of the process window
                                       // for a description of the styles see
                                       // PROCWINSTYLE enumeration
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sProcessObjClass;   // R/W: class of process instance window
                                       // e.g. clsFOLDEREXP
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sProcessWindowID;   // R/W: ID to be used for process
                                       // instance window; e.g. ID_FOLEXP_WINDOW
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szTitle[1024];      // R/W: title text of list window
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  HPOINTER         hIcon;              // handle of icon for process window
                                       //   e.g. hiconFOLEXP
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fNoClose;           // R/W: disable-system-menu-close flag
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  BOOL             fDoNotRegisterObject;// R/W: do-not-register-object flag
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
                                       // if this flag is set the generic
                                       // process window procedure will not
                                       // register the process window, is it
                                       // up to the callback function to
                                       // register it
  SWP              swpSizePos;         // R/W: size and position of list window
                                       // MUST be set by callback function
                                       // during WM_CREATE processing
  PVOID            pUserIDA;           // R/W: ptr to IDA of callback function
                                       // use this variable to anchor the IDA
                                       // of the callback function
  BOOL             fUserFlag;          // R/W: user flag
                                       // not used by generic list instance
  SHORT            asMsgsWanted[20];   // R/W: additional messages to be passed
                                       // to the callback function, list nust
                                       // be terminated by a zero value
  CHAR             szBuffer[1024];     // R/W: buffer for temporary values of
                                       // the callback function

  /********************************************************************/
  /* Variables not used for style PROCWIN_SLIDERONLY                  */
  /********************************************************************/
  SHORT            sTextID;            // R/W: ID to be used for text line
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szText[1024];       // R/W: text to be displayed in text
                                       // line of the the process window
  CHAR             szText2[1024];      // R/W: text to be displayed in the
                                       // second text line of the the process window

  /********************************************************************/
  /* Variables not used for style PROCWIN_TEXTONLY                    */
  /********************************************************************/
  SHORT            sSliderID;          // R/W: ID to be used for slider
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  USHORT           usComplete;         // R/W: current completion rate
                                       // value must be in the range
                                       // from 0 to 100

  /********************************************************************/
  /* Variables only used for style PROCWIN_TEXTSLIDERENTRY            */
  /********************************************************************/
  SHORT            sEntryGBID;         // R/W: ID to be used for entry groupbox
                                       // control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szGroupBoxTitle[80];// R/W: text to be displayed as label
                                       // of the groupbox for process windows
                                       // of style PROCWIN_TEXTSLIDERENTRY
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sEntryID;           // R/W: ID to be used for entry text
                                       // line control
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szEntry[80];        // R/W: text to be displayed in the
                                       // entry text line of the the process
                                       // window

  /********************************************************************/
  /* Variables only used for style PROCWIN_TEXTSLIDERLISTBOX          */
  /********************************************************************/
  HWND             hwndLB1;            // R/O: handle of the first listbox for
                                       // process windows of type
                                       // PROCWIN_TEXTSLIDERLISTBOX
  HWND             hwndLB2;            // R/O: handle of the second listbox for
                                       // process windows of type
                                       // PROCWIN_TEXTSLIDERLISTBOX
  SHORT            sLB1TextID;         // R/W: ID to be used for the label
                                       // of the first listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB2TextID;         // R/W: ID to be used for the label
                                       // of the second listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB1ID;             // R/W: ID to be used for the first
                                       // listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  SHORT            sLB2ID;             // R/W: ID to be used for the second
                                       // listbox
                                       // value MUST be supplied by callback
                                       // function during WM_CREATE processing
  CHAR             szLB1Text[80];      // R/W: text to be displayed as label
                                       // for the first listbox
  CHAR             szLB2Text[80];      // R/W: text to be displayed as label
                                       // for the second listbox
} PROCESSCOMMAREA, *PPROCESSCOMMAREA;




/*! \brief Error return codes
  */
#define ERROR_INTERNALFUNCTION_FAILED  14001
#define ERROR_INPUT_PARMS_INVALID      14002
#define ERROR_TOO_MANY_OPEN_MEMORIES   14003
#define ERROR_MEMORY_NOT_FOUND         14004


#define HPROP PVOID
class EqfMemory;


// Structure of EQF system properties
typedef struct _PROPSYSTEM
{
   //-------------------------- property header --------------------------------
   PROPHEAD  PropHead;                      // common property header
   //------------------ drive and path information -----------------------------
   CHAR szPrimaryDrive[MAX_DRIVE];          // primary TWB drive( e.g. "X:\0")
   CHAR szDriveList[MAX_DRIVELIST];         // list of EQF drives
   CHAR szPropertyPath[MAX_FILESPEC];       // directory for properties
   CHAR szProgramPath[MAX_FILESPEC];        // directory for program files
   CHAR szDicPath[MAX_FILESPEC];            // directory for dictionaries
   CHAR szMemPath[MAX_FILESPEC];            // directory for memory data bases
   CHAR szTablePath[MAX_FILESPEC];          // directory for translation tables
   CHAR szListPath[MAX_FILESPEC];           // directory for lists
   CHAR szExportPath[MAX_FILESPEC];         // directory for exported files
   CHAR szBackupPath[MAX_FILESPEC];         // directory for backup files
   CHAR szDirSourceDoc[MAX_FILESPEC];       // directory for source documents
   CHAR szDirSegSourceDoc[MAX_FILESPEC];    // directory for segm. source docs
   CHAR szDirSegTargetDoc[MAX_FILESPEC];    // directory for segm. target docs
   CHAR szDirTargetDoc[MAX_FILESPEC];       // directory for target docs
   CHAR szDirDocWordLists[MAX_FILESPEC];    // directory for document lists
   CHAR szDllPath[MAX_FILESPEC];            // directory for DLLs
   CHAR szMsgPath[MAX_FILESPEC];            // directory for msg and help files
   CHAR szCtrlPath[MAX_FILESPEC];           // directory for control information
   //----------------------- restart information -------------------------------
   CHAR RestartFolderLists[ 128];           // list of folderlists for restart
   CHAR RestartFolders[ 256];               // list of folder for restart
   EQF_SWP  SwpDef;                         // TWB default window size
   EQF_SWP  Swp;                            // TWB window position
   CHAR RestartMemory[ 256];                // list of mem windows for restart
   CHAR RestartDicts[ 256];                 // list for dict windows for restart
   OBJNAME FocusObject;                     // object which had the focus
   CHAR RestartTagTables[ 128];             // list of tag tables for restart
   CHAR szDefaultEditor[MAX_FILESPEC];      // default editor
   //---------------------- TM server code values ------------------------------
   CHAR szServerList[MAX_SERVER_LIST_SIZE]; // list of servers
   //------------------ drive and path information (2) -------------------------
   CHAR szDirComMem[MAX_FILESPEC];          // directory for server TMs
   CHAR szDirComProp[MAX_FILESPEC];         // directory for server TM props
   CHAR szDirImport[MAX_FILESPEC];          // directory for imported files
   //----------------------- restart information (2) ---------------------------
   CHAR RestartLists[ 256];                 // list of list handlers for restart
   CHAR RestartDocs[ 1000];                 // list of documents for restart
   //----------------------- new paths and directories -------------------------
   CHAR szPrtPath[MAX_FILESPEC];            // directory for print formats
   CHAR szDirComDict[MAX_FILESPEC];         // directory for server Dicts
   CHAR szWinPath[MAX_FILESPEC];            // directory for Windows DLLs
   //------------------------- general options ---------------------------------
   EQF_BOOL  fFolImpNoDocExistsQuery;       // FolImp no query for existing docs
   EQF_BOOL  fReserveOption01;              // reserve option
   EQF_BOOL  fReserveOption02;              // reserve option
   EQF_BOOL  fReserveOption03;              // reserve option
   EQF_BOOL  fReserveOption04;              // reserve option
   EQF_BOOL  fReserveOption05;              // reserve option
   EQF_BOOL  fReserveOption06;              // reserve option
   EQF_BOOL  fReserveOption07;              // reserve option
   EQF_BOOL  fReserveOption08;              // reserve option
   EQF_BOOL  fReserveOption09;              // reserve option
   EQF_BOOL  fReserveOption10;              // reserve option
   EQF_BOOL  fReserveOption11;              // reserve option
   EQF_BOOL  fReserveOption12;              // reserve option
   EQF_BOOL  fReserveOption13;              // reserve option
   EQF_BOOL  fReserveOption14;              // reserve option
   EQF_BOOL  fReserveOption15;              // reserve option
   EQF_BOOL  fReserveOption16;              // reserve option
   EQF_BOOL  fReserveOption17;              // reserve option
   EQF_BOOL  fReserveOption18;              // reserve option
   EQF_BOOL  fReserveOption19;              // reserve option
   //---------------------------------------------------------------------------
   CHAR szEADataPath[MAX_FILESPEC];         // directory for EA data
   //----------------------------- LAN drive letter ----------------------------
   CHAR szLanDrive[MAX_DRIVE];    // primary TWB LAN drive( e.g. "X:\0")
   //-------------------- values in system property dialog ---------------------
   EQF_BOOL  fNoShutDownConf;               // no shutdown confirmation flag
   EQF_BOOL  fNoDeleteConf;                 // no delete confirmation flag
   EQF_BOOL  fSaveAlways;                   // always save workbench during shutdown
   EQF_BOOL  fUseSymbolicMarkupNames;       // use symbolic markup names flag
   EQF_BOOL  fNoGenericMarkup;              // suppress generic markup handling
   CHAR      szWebBrowser[MAX_LONGPATH];    // buffer for web browser name/path

   //---------------------------- TQM stuff --------------------------------------
   CHAR      szTQMProjectPath[MAX_EQF_PATH];// name of TQM project directory
   CHAR      szTQMEvalPath[MAX_EQF_PATH];   // name of TQM evalutation profile directory
   CHAR      szTQMArchivePath[MAX_EQF_PATH];// name of TQM archived projects directory
   CHAR      szTQMReportPath[MAX_EQF_PATH]; // name of TQM report directory
   CHAR      szTQMVendorPath[MAX_EQF_PATH]; // name of TQM vendor directory

   //-------------------- more values in system property dialog ---------------------
   EQF_BOOL  fUseIELikeListWindows;         // use IE like folder list

   // currently active fuzziness levels with two desimal places
   // (e.g. 50 is stored as 5000)
   LONG         lSmallFuzzLevel;       // fuzzines level for small segments
   LONG         lMediumFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeFuzzLevel;       // fuzzines level for large segments

   // internal code version number, used to special preprocessing if a new
   // code version is about to be started for the first time (e.g. delete
   // old stem form cache files if format haas been changed)
   LONG         lCodeVersion;          // current EQFD/EQFDLL code version
   CHAR         szSystemPrefLang[MAX_LANG_LENGTH]; // System Preferences LANGUAGE
   ULONG        ulSystemPrefCP;

   CHAR         RestartMTList[80];    // list of MT job lists to restart (only one!)
   LONG         lSmallLkupFuzzLevel;       // fuzzines level for small segments(1LONG=3CHAR)
   LONG         lMediumLkupFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeLkupFuzzLevel;       // fuzzines level for large segments

   SHORT  		sCalcReportWinX;
   SHORT        sCalcReportWinY;
   SHORT        sCalcReportWinCX;
   SHORT        sCalcReportWinCY;
   SHORT        sLeftPaneWidth;           // P021036

   EQF_BOOL     fNoSgmlDitaProcessing;// true = disable preprocessing of IDDOC proposals for DITA documents
   SHORT        sXSLTEngine;          // XSLT engine to be used
   EQF_BOOL     fEntityProcessing;    // true = perform special processing for IDDOC and DITA entities

   CHAR szPluginPath[MAX_FILESPEC];   // directory for plugins

   USHORT       usMemImpMrkupAction;  // Action for unknown markup: 0=Abort, 1=Skip, 2=Use default
   USHORT       usHideToolBars;       // Whether to hide TWB and translation environment tool bars

   //----------------------------- filler --------------------------------------
   CHAR         chReserve[240];       //  reserve space/ fills props to 4K)
} PROPSYSTEM, *PPROPSYSTEM;

/**********************************************************************/
/* Function declarations for the EQF Property Handler                 */
/**********************************************************************/
HPROP   OpenProperties(   PSZ, PSZ, USHORT, PEQFINFO);
HPROP   CreateProperties( PSZ, PSZ, USHORT, PEQFINFO);
HPROP   CreatePropertiesEx( PSZ, PSZ, USHORT, PEQFINFO, BOOL );
SHORT   DeleteProperties( PSZ, PSZ, PEQFINFO);
SHORT   CloseProperties(  HPROP, USHORT, PEQFINFO);
SHORT   GetAllProperties( HPROP, PVOID, PEQFINFO);
SHORT   PutAllProperties( HPROP, PVOID, PEQFINFO);
SHORT   SaveProperties( HPROP, PEQFINFO);
PVOID   MakePropPtrFromHnd( HPROP hObject);
PVOID   MakePropPtrFromHWnd( HWND hObject);
BOOL    SetPropAccess( HPROP hprop, USHORT flgs);
VOID    ResetPropAccess( HPROP hprop, USHORT flgs);
PSZ     MakePropPath( PSZ pbuf, PSZ pd, PSZ pp, PSZ pn, PSZ pe);
SHORT   GetPropSize( USHORT usClass);
PPROPSYSTEM GetSystemPropPtr( VOID );
BOOL    PropHandlerInitForBatch( void );
BOOL    PropHandlerTerminateForBatch( void );
HPROP   EqfQuerySystemPropHnd( void );
USHORT  ReloadSysProp(PPROPSYSTEM);




// message buffer to keep info about last message
#define MSGBUFFERLENGTH   2048         // length of error message buffer
typedef struct _DDEMSGBUFFER
{
  HWND  hwndDDE;                       // handle of DDE window
  SHORT sErrNumber;                    // error number / return code
  CHAR  chMsgText[MSGBUFFERLENGTH];    // associated message text
} DDEMSGBUFFER, *PDDEMSGBUFFER;


class ImportStatusDetails;
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


// Structure of EQF memory properties and IDA of the memcreate dialog
// and required define statements
// This part of the include file is maintained by CON



/* Macros to make an MPARAM from standard types. */
/*                                                                    */
#define MRESULT                    LRESULT
typedef MRESULT (*PFNWP)(HWND, UINT, WPARAM, LPARAM);

typedef struct _PROPMEMORY
{
 PROPHEAD     stPropHead;                     // Common header of properties
 // szName must be the first item after the property header
 // Do only insert new values at the end of the stucture !!!!!!!!!!
 CHAR         szName[MAX_FILESPEC];           // Property name of memory database ~TEST.MEM~
 CHAR         szPath[MAX_EQF_PATH];           // Property path of memory database  ~C:\EQF~
 USHORT       usCreate;                       // Work variable: If FALSE the memory database dialog failed
 PFNWP        wProcName;                      // Work variable: Old window procedure of name field
 CHAR         szMemName[MAX_FILESPEC];        // Memory database name without extension  ~TEST~
 CHAR         szMemPath[MAX_EQF_PATH];        // Work variable: path to the memory databases with any disk drive ~T:\EQF\MEM\~
 CHAR         szFullMemName[MAX_EQF_PATH];    // Full name and path to the memory databases
                                              // with correct drive, name and ext. ~C:\EQF\MEM\TEST.MEM~
 CHAR         szMemDesc[MAX_MEM_DESCRIPTION]; // Description of memory database  ~Test 25 Beschreibung~
 CHAR         szFormat[MAX_FILESPEC];         // Format table name without extension ~GML~
 CHAR         szXTagsListFile[MAX_EQF_PATH];  // Full path + name + ext to format table ~C:\EQF\TABLE\GML.TBL~
 CHAR         szExclusion[MAX_FILESPEC];      // Exclusion list name without extension  ~ENGNOISE~
 CHAR         szXWordsListFile[MAX_EQF_PATH]; // Full path + name + ext to noise list ~C:\EQF\TABLE\ENGNOISE.LST~
 LANGUAGE     szSourceLang;                   // Source language ~English~
 LANGUAGE     szTargetLang;                   // Target language ~German~
 CHAR         szLanguageFile[MAX_EQF_PATH];   // Full path + name + ext to the language file ~C:\EQF\TABLE\SOURCE.LNG~
 CHAR         szFullFtabPath[MAX_EQF_PATH];   // Full path to tag tables and exclusion lists ~C:\EQF\TABLE\~
 CHAR         szTemp[MAX_PATH144];            // Work field to construct a specific path  ~??????????~
 CHAR         szServerDriveList[MAX_DRIVELIST];  // Work variable: List of server drives
 USHORT       usBlockSize;                    // Block size of memory database ~1024 stored in USHORT~
 EQF_BOOL     fDbcs;                          // DBCS indicator TRUE = DBCS, FALSE = SBCS
 CHAR         szPathLastExported[MAX_PATH144];// Full name and path of exported file in ASCII format ~C:\EQFTEST\TEST.EXP~
 CHAR         szEmpty1[4];
 // Structure end in Troja 2.1
 CHAR         szServer[MAX_SERVER_NAME];      // Server Name of TM or \0 if TM is local
 CHAR         szUserid[MAX_USERID];           // LAN Userid of TM: if local '\0'
 TIME_L       tCreate;                        // TM ID which is the creation time stamp
                                              // TM properties of Troja 2.1 or eaelier have
                                              // set the ID value to '\0'
 USHORT       usLocation;                     // Work variable: Possible values: TM_LOCAL,
                                              //  TM_REMOTE, TM_SHARED
 EQF_BOOL     fDeleteDriveIcons;                   // Work variable: Flag for dialog process
 CHAR         szLangProp[MAX_LANGUAGE_PROPERTIES]; // Work variable: Work area to get the language properties
 // Structure end in Troja 3.0
 CHAR         szLANDriveList[MAX_DRIVELIST];  // Work variable: List of LAN drives
} PROPTRANSLMEM, *PPROPTRANSLMEM;


typedef EQF_BOOL (*PFNGETSEGCONTTEXT)( PSZ_W, PSZ_W, PSZ_W, PSZ_W, LONG, ULONG );


#endif //_LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_
