#pragma once
#ifndef _LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_
#define _LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_

#include <atomic>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "win_types.h"


#include <folly/io/IOBufQueue.h>

#include <proxygen/httpserver/ResponseHandler.h>

class ChunkBuffer{
  //long m_bytesCollecedInChunk = 0;
  //long m_bytesSend = 0;
  //static constexpr size_t chunkSize_ = 4096;
  //std::vector<unsigned char> m_buff;
  proxygen::ResponseHandler* m_responseHandler = nullptr;

  bool canFit(long size);
  
  void triggerChunkSend();
public:
  void writeToBuff(const void * data, long size);
  void setResponseHandler(proxygen::ResponseHandler* responseHandler){m_responseHandler = responseHandler;}
  bool isActive()const {return m_responseHandler != 0; }

  ChunkBuffer(){//m_buff.reserve(chunkSize_+1);
  }
  ~ChunkBuffer(){
    //T5LOG(T5INFO)<< "called dctor of chunk buffer- sending last chunk";
    //triggerChunkSend();
  }

  void SendResponce(const std::string& memName, const std::string& nextInternalKey);


  folly::IOBufQueue bufQueue;
};

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

#define SEGDATABUFLEN 4096

#define MEM_IMPORT_COMPLETE  7001                // the import is complete
// translation source values for usTranslationFlag in MEMEXPIMPSEG sgtructure
#define TRANSL_SOURCE_MANUAL 0
#define TRANSL_SOURCE_MANCHINE 1
#define TRANSL_SOURCE_GLOBMEMORY 2

  enum COMMAND {
        UNKNOWN_COMMAND = 0,
        LIST_OF_MEMORIES,
        SAVE_ALL_TM_ON_DISK,
        SHUTDOWN,    
        DELETE_MEM,
        EXPORT_MEM_TMX,
        EXPORT_MEM_INTERNAL_FORMAT,
        EXPORT_MEM_INTERNAL_FORMAT_STREAM,
        REORGANIZE_MEM,
        STATUS_MEM,
        RESOURCE_INFO,
        FLUSH_MEM,

        START_COMMANDS_WITH_BODY,
        CREATE_MEM = START_COMMANDS_WITH_BODY, 
        EXPORT_MEM_TMX_STREAM,
        FUZZY,
        CONCORDANCE,
        DELETE_ENTRY,
        GET_ENTRY,
        DELETE_ENTRIES_REORGANIZE,
        UPDATE_ENTRY,
        TAGREPLACEMENTTEST,
        IMPORT_MEM,
        IMPORT_MEM_STREAM,
        IMPORT_LOCAL_MEM,
        CLONE_TM_LOCALY
        //IMPORT_MEM_INTERNAL_FORMAT
    };



constexpr  std::initializer_list<std::pair<const COMMAND, const char*>> CommandToStringsMap {
        { UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
        { LIST_OF_MEMORIES, "LIST_OF_MEMORIES" },
        { SAVE_ALL_TM_ON_DISK, "SAVE_ALL_TM_ON_DISK" },
        { SHUTDOWN, "SHUTDOWN" },
        { DELETE_MEM, "DELETE_MEM" },

        { EXPORT_MEM_TMX, "EXPORT_MEM_TMX" },
        { EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { EXPORT_MEM_TMX_STREAM, "EXPORT_MEM_TMX_STREAM" },
        { EXPORT_MEM_INTERNAL_FORMAT_STREAM, "EXPORT_MEM_INTERNAL_FORMAT_STREAM" },
        { STATUS_MEM, "STATUS_MEM" },
        
        { RESOURCE_INFO, "RESOURCE_INFO" },
        { CREATE_MEM, "CREATE_MEM" },
        { FUZZY, "FUZZY" },
        { CONCORDANCE, "CONCORDANCE" },
        { DELETE_ENTRY, "DELETE_ENTRY" },
        
        { GET_ENTRY, "GET_ENTRY" },
        { DELETE_ENTRIES_REORGANIZE, "DELETE_ENTRIES_REORGANIZE" },
        { UPDATE_ENTRY, "UPDATE_ENTRY" },
        { TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { IMPORT_MEM, "IMPORT_MEM" },
        
        { IMPORT_MEM_STREAM, "IMPORT_MEM_STREAM" },
        { IMPORT_LOCAL_MEM, "IMPORT_LOCAL_MEM" },
        { REORGANIZE_MEM, "REORGANIZE_MEM" },
        { CLONE_TM_LOCALY, "CLONE_MEM"},
        { FLUSH_MEM, "FLUSH_MEM" }
    };


const char* searchInCommandToStringsMap(const COMMAND& key);

enum InclosingTagsBehaviour{
  saveAll = 0, skipAll, skipPaired
};

  /*! \brief States of a memory
  */
  typedef enum 
  {
    WAITING_FOR_LOADING_STATUS,//memory is waiting to be loaded
    OPEN_STATUS,            // memory is available and open
    AVAILABLE_STATUS,       // memory has been imported but is not open yet
    IMPORT_RUNNING_STATUS,  // memory import is running
    IMPORT_FAILED_STATUS,   // memory import failed
    REORGANIZE_RUNNING_STATUS, // memory organize is running
    REORGANIZE_FAILED_STATUS,   // memory organize is failed
    FAILED_TO_OPEN_STATUS, 
    OPENNING_STATUS           //memory is openning in anaother process
  } MEMORY_STATUS;

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

class EqfMemory;

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

#define MAX_LANG_LENGTH         40     // length of the name of a language
typedef struct _TMX_SIGN
{
  TIME_L lTime=0;
  TIME_L creationTime=0;
  BYTE bGlobVersion=0;  
  BYTE bMajorVersion=0;
  BYTE bMinorVersion=0;
  CHAR szName[_TMX_SIGN_SZ_NAME];
  CHAR szSourceLanguage[MAX_LANG_LENGTH];
  CHAR szDescription[MAX_MEM_DESCRIPTION];
  LONG segmentIndex = 0;//highest generated segment id, used for generating new id's
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
  //BOOL   fNeedToWrite;                             // Commit before reuse
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
  ULONG ulAccessCounter;
} LOOKUPENTRY_V3, *PLOOKUPENTRY_V3;


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
  USHORT usId = 0;
} TMX_TABLE_ENTRY, * PTMX_TABLE_ENTRY;
// name table structure (TM version 5 and up)
constexpr int NUM_OF_TMX_TABLE_ENTRIES = (BTREE_REC_SIZE_V3 - sizeof(ULONG)) / sizeof(TMX_TABLE_ENTRY);

typedef struct TMX_TABLE_OLD{
  ULONG  ulMaxEntries = 0;
  TMX_TABLE_ENTRY stTmTableEntry[NUM_OF_TMX_TABLE_ENTRIES];
};
typedef TMX_TABLE_OLD * PTMX_TABLE_OLD;

typedef struct TMX_TABLE
{
  //ULONG  ulAllocSize = 0;
  ULONG  ulMaxEntries = 0;
  //std::vector<TMX_TABLE_ENTRY> stTmTableEntry;//[NUM_OF_TMX_TABLE_ENTRIES];
  std::vector<TMX_TABLE_ENTRY> stTmTableEntry;
  TMX_TABLE(){
    stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    for(int i=0;i<NUM_OF_TMX_TABLE_ENTRIES;i++){
      stTmTableEntry[i].szName[0] = '\0';
      stTmTableEntry[i].usId = 0;
    }
  }
};

typedef TMX_TABLE * PTMX_TABLE;
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


BOOL   UtlAlloc ( void **, long, long, unsigned short );
BOOL   UtlAllocHwnd ( void**, long, long, unsigned short, void * );

#include "Filebuffer.h"
#include <cstring>

typedef enum _SEARCHTYPE
{
  FEXACT,                  // exact match requested
  FSUBSTR,                 // only substring match
  FEQUIV,                  // equivalent match
  FEXACT_EQUIV             // editor: exact match then equivalent
} SEARCHTYPE;




///* Maximum size (entries) of the lookup table (it is a USHORT).
#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x0FFFF
//#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x00FF
/* Initial size (entries) of the lookup table */
#define MIN_NUMBER_OF_LOOKUP_ENTRIES 32



struct BTREEDATA{
  BTREEDATA() {
    memset(this, 0, sizeof(*this));
  }

  USHORT       usFirstNode=0;                    // file pointer of record
  USHORT       usFirstLeaf=0;                    // file pointer of record

  //PFN_QDAMCOMPARE compare = nullptr;             // Comparison function
  USHORT       usNextFreeRecord;                 // Next record to expand to
  //CHAR         chFileName[144];                // Name of B-tree file
  RECPARAM     DataRecList[ MAX_LIST ];          // last used data records
  BOOL         fGuard=0;                         // write every record
  BOOL         fOpen=0;                          // open flag
  BOOL         fCorrupted=0;                     // mark as corrupted
  WCHAR        TempKey[HEADTERM_SIZE];           // pointer to temp. key
  //ULONG        ulTempRecSize = MAXDATASIZE;      // size of temp record area
  BOOL         fTerse=0;                         // tersing requested
  BYTE         chEntryEncode[ ENTRYENCODE_LEN];  // significant characters
  BYTE         bEncodeLen[COLLATE_SIZE];         // encoding table length
  CHAR         chEncodeVal[COLLATE_SIZE];        // encoding table
  BYTE         chDecode[ENTRYDECODE_LEN];        // decoding table
  //BYTE         chCollate[COLLATE_SIZE];          // collating sequence to use
  NTMVITALINFO chCollate;
  BYTE         chCaseMap[COLLATE_SIZE];          // case mapping to be used
  USHORT       usFreeKeyBuffer=0;                // index of buffer to use
  USHORT       usFreeDataBuffer=0;               // first data buffer chain
  USHORT       usFirstDataBuffer=0;              // first data buffer
  BTREEBUFFER_V3  BTreeTempBuffer_V3;            // temporary V3 buffer
  LONG         lTime=0;                          // time of last update/open
  USHORT       usVersion=0;                      // version identification...
  CHAR         chEQF[7];                         // The type of file
  BYTE         bVersion=0;                       // version flag
  USHORT       usOpenFlags=0;                    // settings used for open
  LONG         alUpdCtr[MAX_UPD_CTR];            // list of update counters
  HFILE        fpDummy=nullptr;                  // dummy/lock semaphore file handle
  //USHORT       usNumberOfLookupEntries=0;        // Number of allocated lookup-table-entries
  USHORT       usNumberOfAllocatedBuffers=0;     // Number of allocated buffers
  ULONG        ulReadRecCalls=0;                 // Number of calls to QDAMReadRecord
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

  SHORT        sCurrentIndex=0;                  // current sequence array
  USHORT       usCurrentRecord=0;                // current sequence record
  USHORT       usDictNum=0;                      // index in global structure
  wchar_t      chHeadTerm[HEADTERM_SIZE];        // last active head term
  BOOL         fLock=0;                          // head term is locked
  wchar_t      chLockedTerm[HEADTERM_SIZE];      // locked term if any
  //QDAM part
  //PBTREEGLOB   pBTree;                           // pointer to global struct
  ULONG        ulNum=0;
  //CHAR         chDictName[ MAX_EQF_PATH ];
  BOOL         fDictLock=0;                        // is dictionary locked
  //std::atomic<int> usOpenCount;                  // number of accesses...
  //PBTREE       pIdaList[ MAX_NUM_DICTS ];        // number of instances ...
  //End of qdam part

};

struct BTREE: public BTREEDATA
{   
  //new fields 
  FileBuffer fb;
  std::vector<BYTE>         TempRecord;

  std::vector<LOOKUPENTRY_V3> LookupTable_V3;        // Pointer to lookup-table


  BTREE(){
    AllocateMem();
    /*
    memset(&DataRecList[0], 0, sizeof(DataRecList));
    memset(&TempKey, 0, sizeof(TempKey));
    memset(&chEntryEncode, 0, sizeof(chEntryEncode));
    memset(&bEncodeLen, 0, sizeof(bEncodeLen));

    memset(&chEncodeVal[0], 0, sizeof(chEncodeVal));
    memset(&chDecode, 0, sizeof(chDecode));
    memset(&chCaseMap, 0, sizeof(chCaseMap));
    memset(&chEQF, 0, sizeof(chEQF));

    memset(&alUpdCtr[0], 0, sizeof(alUpdCtr));
    memset(&chHeadTerm, 0, sizeof(chHeadTerm));
    memset(&chLockedTerm, 0, sizeof(chLockedTerm));

    memset(&chCollate, 0, sizeof(chCollate));
    memset(&BTreeTempBuffer_V3, 0, sizeof(BTreeTempBuffer_V3));
    //*/
  }
  int AllocateMem(){
    int rc = 0;
    TempRecord = std::vector<BYTE>(MAXDATASIZE, 0);
    return rc;
  }

  void freeMem(){
    freeLookupTable();
  }

  int initLookupTable();
  int resetLookupTable();
  void freeLookupTable();
  int checkLookupTableAndRealocate(int number);  
  int allocateNewLookupTableBuffer(int number, PLOOKUPENTRY_V3& pLEntry, PBTREEBUFFER_V3&   pBuffer);

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


    ULONG GetNumOfSavedRecords()const;
    
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


    SHORT QDAMDictCreateLocal ( TMX_SIGN*, ULONG, bool keepInRamOnly = false);
                                
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
    SHORT QDAMDictOpenLocal(
        SHORT sNumberOfBuffers,             // number of buffers
        USHORT usOpenFlags                 // Read Only or Read/Write
    );

    SHORT  EQFNTMOpen( USHORT usOpenFlags);                 // Read Only or Read/Write
    SHORT QDAMCheckDict( PSZ    pName);                        // name of dictionary
    
    SHORT  QDAMDictLockDictLocal( BOOL ); 
    //SHORT QDAMDictCloseLocal  ();
    //+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictClose    close the dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictClose( PPBTREE );
//
//+----------------------------------------------------------------------------+
// Description:       Close the file
//
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
SHORT QDAMDictClose();


SHORT QDAMDictSignLocal
(
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
);

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

SHORT EQFNTMClose();

SHORT QDAMNewRecord_V3
(
   PBTREEBUFFER_V3  * ppRecord,
   RECTYPE         recType          // data or key record
);

SHORT QDAMInsertKey_V3
(
   PBTREEBUFFER_V3 pRecord,               // record where key is to be inserted
   PWCHAR     pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
);   

size_t GetFileSize()const;


BOOL QDAMTerseData
(
   PUCHAR  pData,                   // pointer to data
   PULONG  pulLen                  // length of the string
);


 SHORT QDAMLocateKey_V3(
   PBTREEBUFFER_V3, PWCHAR, PSHORT, SEARCHTYPE, PSHORT);

};//BTREE

typedef  BTREE * PBTREE;
typedef BTREE ** PPBTREE;



#define NTMNEXTKEY( pBT )  pBT->chCollate.ulNextKey
#define NTMSTARTKEY( pBT ) pBT->chCollate.ulStartKey





//typedef TMX_CLB* PTMX_CLB;
//prefix for each output structures
typedef struct _TMX_PREFIX_OUT
{
  USHORT usLengthOutput;               //length of complete output structure
  USHORT usTmtXRc;                     //function returncode
} TMX_PREFIX_OUT, * PTMX_PREFIX_OUT, XOUT, * PXOUT;



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


#include <proxygen/httpserver/ResponseHandler.h>

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
  proxygen::ResponseHandler* responseHandler = nullptr;
  std::string fileData;
  //InclosingTagsBehaviour inclosingTagsBehaviour = InclosingTagsBehaviour::saveAll;
} MEMEXPIMPINFO, *PMEMEXPIMPINFO;


class EqfMemory;
class OtmProposal;
// structure to pass/receive segment data

typedef struct _MEMEXPIMPSEG
{
  BOOL             fValid;                       // FALSE = segment data is invalid, count as invalid segment
  LONG             lSegNum;                      // segment number
  LONG             lSegNo;
  CHAR             szFormat[EQF_FORMATLEN];      // format / markup table name
  CHAR             szSourceLang[EQF_LANGLEN];    // segment source language
  CHAR             szTargetLang[EQF_LANGLEN];    // segment target language
  CHAR             szDocument[EQF_DOCNAMELEN];   // document  
  unsigned short   usTranslationFlag;            // numeric value indication the source of the translation
  LONG             lTime;                        // segment update time in C time format
  CHAR             szAuthor[EQF_DOCNAMELEN];      // author of segment
  unsigned short   usLength;                     // length of segment data (incl. string end delimiter)
  WCHAR            szSource[SEGDATABUFLEN+1];    // buffer for segment source
  WCHAR            szTarget[SEGDATABUFLEN+1 ];   // buffer for segment target
  CHAR             szReason[200];                // reason for invalid segment
  WCHAR            szContext[SEGDATABUFLEN+1];    // buffer for segment context
  WCHAR            szAddInfo[SEGDATABUFLEN+1];    // buffer for additional information of segment 

 char szInternalKey[SEGDATABUFLEN];
} MEMEXPIMPSEG, *PMEMEXPIMPSEG;


#include "CXMLWRITER.H"

// size of file read buffer in preprocess step
#define TMX_BUFFER_SIZE 8096

// callback function which is used by ExtMemImportprocess to insert segments into the memory
typedef USHORT (/*APIENTRY*/ *PFN_MEMINSERTSEGMENT)( LONG lMemHandle, PMEMEXPIMPSEG pSegment );
class TMXParseHandler;
//class SAXParser;
struct LOADEDTABLE;

typedef struct _TOKENENTRY     // entry in tokenlist :
{
  // !!!! Attention: below has to match TOKENENTRYSEG definition ....  !!!!
  SHORT     sTokenid;          // Tokenid
  int        iLength;          // Length of data string
  SHORT     sAddInfo;          // additional information from tag table
  CHAR    * pDataString;       // pointer to data string
  USHORT    usOrgId;           // original id
  USHORT    ClassId;           // class id of token
  CHAR_W * pDataStringW;       // pointer to data string  - Unicode
  // !!!! Attention: above has to match TOKENENTRYSEG definition ....  !!!!

} TOKENENTRY, *PTOKENENTRY;
class ImportStatusDetails;


#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>


//+----------------------------------------------------------------------------+
//| Our TMX import export class                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CTMXExportImport
{
  public:
    // constructor/desctructor
	  CTMXExportImport();
	  ~CTMXExportImport();
    // export methods
    USHORT WriteHeader( const char *pszOutFile, PMEMEXPIMPINFO pMemInfo );
    USHORT WriteSegment( PMEMEXPIMPSEG pSegment  );
    USHORT WriteEnd();
    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo, ImportStatusDetails* pImportStatusDetails ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG pMemHandle, ImportStatusDetails*     pImportData  ); 
    USHORT EndImport(); 
    USHORT getLastError( PSZ pszErrorBuffer, int iBufferLength );


    CXmlWriter m_xw;
  protected:
    USHORT WriteTUV( PSZ pszLanguage, PSZ pszMarkup, PSZ_W pszSegmentData );
    USHORT PreProcessInFile( const char *pszInFile, const char *pszOutFile );


    TMXParseHandler *m_handler;          // our SAX handler 
    xercesc::SAXParser* m_parser;
    xercesc::XMLPScanToken m_SaxToken; 
    unsigned int m_iSourceSize;          // size of source file
    TOKENENTRY* m_pTokBuf;               // buffer for TaTagTokenize tokens
    CHAR m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    LOADEDTABLE* m_pLoadedTable;         // pointer to currently loaded markup table
    LOADEDTABLE* m_pLoadedRTFTable;      // pointer to loaded RTF tag table
    CHAR m_szInFile[512];                // buffer for input file
    CHAR m_TempFile[540];                // buffer for temporary file name
    BYTE m_bBuffer[TMX_BUFFER_SIZE+1];
    MEMEXPIMPINFO* m_pMemInfo;
    CHAR_W m_szSegBuffer[MAX_SEGMENT_SIZE+1]; // buffer for the processing of segment data
    int  m_currentTu;                    // export: number of currently processed tu
};


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
 CTMXExportImport*          lExternalExportHandle;  // handle of external memory export functions
 //HMODULE       hmodMemExtExport;                 // handle of external export module/DLL
 MEMEXPIMPINFO* pstMemInfo;                        // buffer for memory information
 MEMEXPIMPSEG*  pstSegment;                        // buffer for segment data

 BOOL         fDataCorrupted;             // TRUE = data has been corrupted during export
 CHAR_W       szBackConvArea[MEM_EXPORT_WORK_AREA]; // Work area for conversion check
 EqfMemory    *pMem;                      // pointer to memory being exported
 OtmProposal  *pProposal;                   // buffer for proposal data
 CHAR_W       szSegmentBuffer[MAX_SEGMENT_SIZE]; // Buffer for segment data
 CHAR_W       szSegmentBuffer2[MAX_SEGMENT_SIZE]; // Buffer for preprocessed segment data
 int          iComplete;                  // completion rate
 CHAR         szPlugin[MAX_LONGFILESPEC]; // name of memory plugin handling the current memory database

 int invalidXmlSegments=0;
 long numOfRequestedSegmentsForExport = 0;
 long segmentsExported = 0;
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
/**********************************************************************/
/**********************************************************************/
/* Error Base IDs                                                     */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
#define  Err_NoError          0        // no error occured

/* Error Base Values */
#define  Err_Eqfstart      1000
#define  Err_Prop          2000        // property handler rc
#define  Err_ObjM          2100        // object manager rc
#define  Err_Nfol          2200        // new folder messages
#define  ERR_BTREE_BASE    5000
#define  ERR_BTREE_END     5999
#define  ERR_MEM_BASE      6000
#define  ERR_MEM_END       6999
#define  ERR_MORPH_BASE    7000
#define  ERR_MORPH_END     7999
#define  ERR_DICT_BASE     8000
#define  ERR_DICT_END      8999
#define  ERR_PRINT_BASE    9000
#define  ERR_PRINT_END     9999
#define  ERR_COUNT_BASE   10000
#define  ERR_COUNT_END    10100
#define  QLDB_ERR_BASE    10500
#define  QLDB_ERR_END     11000

/* General error message IDs */

#define Err_NoDiskSpace        (Err_Eqfstart+01) // too less disk space
#define Err_NoStorage          (Err_Eqfstart+02) // no more storage
#define Err_System             (Err_Eqfstart+03) // unexpected error
#define Err_OpenFile           (Err_Eqfstart+04) // error opening file
#define Err_ReadFile           (Err_Eqfstart+05) // error reading file
#define Err_DeleteFile         (Err_Eqfstart+06) // cannot delete file
#define Err_WriteFile          (Err_Eqfstart+07) // error writing to file
#define Err_HandlerExists      (Err_Eqfstart+08) // handler already installed


/* Object manager error message IDs */

#define ErrObjM_AlreadyReg     (Err_ObjM+ 1) // already registered
#define ErrObjM_CreateEntry    (Err_ObjM+ 2) // cannot create tbl entry (nostor?)
#define ErrObjM_NoHandler      (Err_ObjM+ 3) // no anchor found for class

/* Properties handler error message IDs */

#define ErrProp_AccessDenied   (Err_Prop+ 1) // wrong access to properties
#define ErrProp_AlreadyDefined (Err_Prop+ 2) // symbolic name already defined
#define ErrProp_AlreadyExists  (Err_Prop+ 3) // properties already exist
#define ErrProp_InvalidClass   (Err_Prop+ 4) // invalid properties class
#define ErrProp_InvalidData    (Err_Prop+ 5) // invalid properties data
#define ErrProp_InvalidEntry   (Err_Prop+ 6) // invalid entry index
#define ErrProp_InvalidFile    (Err_Prop+ 7) // invalid file detected
//
#define ErrProp_InvalidName    (Err_Prop+ 9) // symbolic name invalid
#define ErrProp_InvalidObjectName (Err_Prop+10) // object name invalid
#define ErrProp_InvalidType    (Err_Prop+11) // properties type invalid
#define ErrProp_NoEnvironment  (Err_Prop+12) // properties environment not active
#define ErrProp_NoName         (Err_Prop+13) // invalid handles
#define ErrProp_NoSystem       (Err_Prop+14) // system properties not located
#define ErrProp_NotAuthorized  (Err_Prop+15) // properties of class not created
#define ErrProp_NotFound       (Err_Prop+16) // property name not located
#define ErrProp_ObjectBusy     (Err_Prop+17) // objects busy
#define ErrProp_SystemInvalid  (Err_Prop+18) // system properties invalid
#define ErrProp_InvalidAccess  (Err_Prop+19) // access flg invalid
#define ErrProp_MissingBuffer  (Err_Prop+20) // buffer addr missing
#define ErrProp_NotLoaded      (Err_Prop+21) // prop load error
#define ErrProp_InvalidParms   (Err_Prop+22) // invalid parm specified
#define ErrProp_InvalidHandle  (Err_Prop+23) // invalid handle to properties


#define BTREE_BASE          ERR_BTREE_BASE        //
#define BTREE_NO_ROOM       BTREE_BASE+1          // Insufficent memory
#define BTREE_ILLEGAL_FILE  BTREE_BASE+2          // Not an index file
#define BTREE_DUPLICATE_KEY BTREE_BASE+3          // Key is not unique
#define BTREE_NO_BUFFER     BTREE_BASE+4          // No buffer available
#define BTREE_NOT_FOUND     BTREE_BASE+5          // Key not found
#define BTREE_INVALID       BTREE_BASE+6          // Btree pointer invalid
#define BTREE_READ_ERROR    BTREE_BASE+7          // read error on file
#define BTREE_CORRUPTED     BTREE_BASE+8          // binary tree is corrupted
#define BTREE_BUFFER_SMALL  BTREE_BASE+9          // buffer to small for data
#define BTREE_DISK_FULL     BTREE_BASE+10         // disk is full
#define BTREE_USERDATA      BTREE_BASE+11         // user data too large
#define BTREE_EOF_REACHED   BTREE_BASE+12         // eof or start reached
#define BTREE_EMPTY         BTREE_BASE+13         // no entries in dictionary
#define BTREE_WRITE_ERROR   BTREE_BASE+14         // error during write
#define BTREE_OPEN_ERROR    BTREE_BASE+15         // error during open
#define BTREE_CLOSE_ERROR   BTREE_BASE+16         // error during close
#define BTREE_NUMBER_RANGE  BTREE_BASE+17         // number out of range
#define BTREE_INV_SERVER    BTREE_BASE+18         // invalid server name
#define BTREE_READONLY      BTREE_BASE+19         // opened for read/only
#define BTREE_DATA_RANGE    BTREE_BASE+20         // data length out of range
#define BTREE_DICT_LOCKED   BTREE_BASE+21         // dictionary is locked
#define BTREE_ENTRY_LOCKED  BTREE_BASE+22         // entry is locked
#define BTREE_MAX_DICTS     BTREE_BASE+23         // max. number of dicts reached
#define BTREE_NO_EXCLUSIVE  BTREE_BASE+24         // no excl. use possible
#define BTREE_FILE_NOTFOUND BTREE_BASE+25         // file not found
#define BTREE_ACCESS_ERROR  BTREE_BASE+26         // access error
#define BTREE_INVALID_DRIVE           BTREE_BASE+27
#define BTREE_OPEN_FAILED             BTREE_BASE+28
#define BTREE_NETWORK_ACCESS_DENIED   BTREE_BASE+29
#define BTREE_LOCK_ERROR              BTREE_BASE+30  // locking failed
#define BTREE_IN_USE                  BTREE_BASE+31  // data is locked/modified
#define BTREE_INVALIDATED             BTREE_BASE+32  // data has been invalidated
#define BTREE_LOOKUPTABLE_NULL        BTREE_BASE+33  // pointer to lookup table is null
#define BTREE_LOOKUPTABLE_CORRUPTED   BTREE_BASE+34  // lookup table is corrupted
#define BTREE_LOOKUPTABLE_TOO_SMALL   BTREE_BASE+35  // lookup table is too small, file is too big
#define BTREE_NOT_SUPPORTED           BTREE_BASE+36 //
#define SEGMENT_ID_NOT_EQUAL          BTREE_BASE+81 // segment and requested id is not equal


#define MAX_NUM_DICTS      1900        // max. number of dict concurrently open

/**********************************************************************/
/* Return Codes                                                       */
/**********************************************************************/
#define TMERR_BASE ERR_MEM_BASE            /* Base for errors          */

#define TMERR_EOF     TMERR_BASE + 12

#define DISK_FULL     TMERR_BASE + 13   // for Add /Replace / Create
#define DB_FULL       TMERR_BASE + 14   // when number of blocks exceeds 2**16 - 1

#define SEG_NOT_FOUND TMERR_BASE + 20   /* for Delete command          */
#define CLUSTER_EMPTY TMERR_BASE + 21   /* Internally only             */

#define TM_FILE_NOT_FOUND                  TMERR_BASE + 30 // for Open
#define FILE_ALREADY_OPEN                  TMERR_BASE + 31 // for Open
#define CORRUPTION_FLAG_ON                 TMERR_BASE + 32 // for Open
#define FILE_MIGHT_BE_CORRUPTED            TMERR_BASE + 33 // for Open
#define VERSION_MISMATCH                   TMERR_BASE + 34 // for Open
#define CORRUPT_VERSION_MISMATCH           TMERR_BASE + 35 // for Open
#define TM_FILE_SCREWED_UP                 TMERR_BASE + 36 // for Open
#define NOT_A_MEMORY_DATABASE              TMERR_BASE + 37 // for Open
// the following error code is returned if open fails because a TM was
// corrupted and (on confirmation via a message) was then
// successfully organized
#define TM_WAS_CORRUPTED_IS_ORGANIZED      TMERR_BASE + 38 // for Open,
#define TM_PROPERTIES_DIFFERENT            TMERR_BASE + 39 // for Open,
#define SERVER_DRIVE_REMOVED               TMERR_BASE + 40 // for Open, /*@89A*/
#define SERVER_DRIVE_ACTIVE                TMERR_BASE + 41 // for Open, /*@89A*/
#define TM_PROPERTIES_EQUAL                TMERR_BASE + 42 // for Open,/*@1170A*/
#define TM_PROPERTIES_NOT_OPENED           TMERR_BASE + 43 // for Open,/*@1170A*/
#define TMD_SIZE_IS_BIGGER_THAN_ALLOWED           TMERR_BASE + 44 
#define IMPORT_TIMEOUT_REACHED           TMERR_BASE + 45

#define BLOCK_SIZE_TOO_SMALL     TMERR_BASE + 48 /* for Create         */
#define FILE_ALREADY_EXISTS      TMERR_BASE + 49 /* for Create         */

#define NO_SEG_FOUND             TMERR_BASE + 50 /* for Extract        */

#define ILLEGAL_TM_COMMAND       TMERR_BASE + 60 /* for TMT            */

#define NOT_REPLACED_OLD_SEGMENT TMERR_BASE + 70 /* for Replace        */
#define SEGMENT_BUFFER_FULL      TMERR_BASE + 71 /* for Replace   */ /*@1108A*/
                                         /*rc from AddSegToCluster*/ /*@1108A*/
#define SEG_SKIPPED_BAD_MARKUP   TMERR_BASE + 72 /* for Import         */
#define SEG_RESET_BAD_MARKUP     TMERR_BASE + 73 /* for Import         */

#define TM_FUNCTION_FAILED       TMERR_BASE + 80 /* Undefinable Error  */

// Currently not used
//#define TMERR_TM_HANDLER_ALREADY_ACTIVE     TMERR_BASE + 90  // Comm code

#define TMERR_TOO_MANY_OPEN_DATABASES       TMERR_BASE + 91  // Comm code
#define TMERR_TOO_MANY_USERS_CONNECTED      TMERR_BASE + 92  // Comm code
#define TMERR_THREAD_NOT_SPAWNED            TMERR_BASE + 93  // Comm code
#define TMERR_TM_OPENED_EXCLUSIVELY         TMERR_BASE + 94  // Comm code
#define TMERR_TM_OPENED_SHARED              TMERR_BASE + 95  // Comm code

// Currently not used
//#define TMERR_PIPE_EXISTS                   TMERR_BASE + 96  // Comm code

#define TMERR_NO_MORE_MEMORY_AVAILABLE      TMERR_BASE + 97  // Comm code
#define TMERR_THREAD_INIT_FAILED            TMERR_BASE + 98  // Comm code
#define TMERR_TOO_MANY_QUERIES              TMERR_BASE + 99  // Comm code
#define TMERR_PROP_EXIST                    TMERR_BASE + 100 // Comm code
#define TMERR_PROP_WRITE_ERROR              TMERR_BASE + 101 // Comm code
#define TMERR_PROP_NOT_FOUND                TMERR_BASE + 102 // Comm code
#define TMERR_PROP_READ_ERROR               TMERR_BASE + 103 // Comm code
#define TMERR_TOO_MANY_TMS                  TMERR_BASE + 104 // Comm code
#define TMERR_SERVER_NOT_STARTED            TMERR_BASE + 105 // Comm code
#define TMERR_SERVERCODE_NOT_STARTED        TMERR_BASE + 106 // Comm code
#define TMERR_COMMUNICATION_FAILURE         TMERR_BASE + 107 // Comm code

// This error code is sent when somebody stops the server code and open connections
// are still there (it's not guaranteed that this error code makes it to the requester
// since the pipe is closed immediately; if this error code doesn't make it to the
// requester, TMERR_COMMUNICATION_FAILURE will be returned)
#define TMERR_SERVER_ABOUT_TO_EXIT          TMERR_BASE + 108 // Comm code
#define TMERR_TOO_MANY_USERS_ON_SERVER      TMERR_BASE + 109 // Comm code

#define SOURCE_STRING_ERROR                 TMERR_BASE + 150
#define ERROR_ADD_TO_TM                     TMERR_BASE + 151

#define LANG_FILE_NOT_FOUND                 TMERR_BASE + 200 // for Create
#define LANGUAGE_NOT_FOUND                  TMERR_BASE + 201 // for Create
#define LANG_FILE_LINE_TOO_LONG             TMERR_BASE + 202 // for Create
#define LANG_CHAR_APPEARS_TWICE             TMERR_BASE + 203 // for Create
#define LANG_GROUP_NUM_TOO_LARGE            TMERR_BASE + 204 // for Create
#define TAG_FILE_NOT_FOUND                  TMERR_BASE + 205 // for Create
#define WORD_FILE_NOT_FOUND                 TMERR_BASE + 206 // for Create
#define TAG_FILE_TOO_LARGE                  TMERR_BASE + 207 // for Create /*@ZDA*/
#define ERROR_OLD_PROPERTY_FILE             TMERR_BASE + 208
#define ERROR_REMOTE_TM_NOT_SUPPORTED       TMERR_BASE + 209
#define ERROR_VERSION_NOT_SUPPORTED         TMERR_BASE + 210

#define  MEM_PROCESS_OK        6900              // Return code of load is OK
#define  MEM_READ_ERR          MEM_PROCESS_OK+1  // Dos read error
#define  MEM_WRITE_ERR         MEM_PROCESS_OK+2  // Dos write error
#define  MEM_SEG_SYN_ERR       MEM_PROCESS_OK+3  // Segment syntax error
#define  MEM_FILE_SYN_ERR      MEM_PROCESS_OK+4  // File syntax error
#define  MEM_LOAD_OK           MEM_PROCESS_OK+5  // Return code of load dialog is OK
#define  MEM_PROCESS_END       MEM_PROCESS_OK+6  // ID which indicates that a proc is terminated
#define  MEM_DB_ERROR          MEM_PROCESS_OK+7  // Unpredictable memory db error
#define  MEM_DB_CANCEL         MEM_PROCESS_OK+8  // load process was canceled /*@47*/
#define  MEM_DB_CANCEL         MEM_PROCESS_OK+8  // load process was canceled /*@47*/
#define  MEM_DB_EXTRACT_ERROR  MEM_PROCESS_OK+9  // retuned from function MemReadWriteSegment when TmExtract fails /*@47*/
#define  MEM_DB_REPLACE_ERROR  MEM_PROCESS_OK+10 // retuned from function MemReadWriteSegment when TmReplace fails /*@47*/
#define  MEM_PROCESS_REACHED_REQUESTED_LIMIT MEM_PROCESS_END+11


enum statusCodes {
  OK = 200, 
  INTERNAL_SERVER_ERROR = 500, 
  PROCESSING = 102, 
  NOT_FOUND = 404,
  BAD_REQUEST = 400,
  CONFLICT = 409,
  CREATED = 201,
  NOT_ACCEPTABLE = 406

};

#include <proxygen/httpserver/ResponseHandler.h>

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
  proxygen::ResponseHandler* responseHandler = nullptr;
  ulong startingRecordKey = 0;
  ushort startingTargetKey = 0;
  ulong numOfProposalsRequested = 0;
  folly::IOBufQueue* pBufQueue = nullptr;
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
SHORT   GetAllProperties( HPROP, PVOID, PEQFINFO);
SHORT   PutAllProperties( HPROP, PVOID, PEQFINFO);
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


#include <proxygen/httpserver/ResponseHandler.h>

class ImportStatusDetails;


#define  SEGMENT_CLUSTER_LENGTH        4  // Length of segment cluster
#define  SEGMENT_NUMBER_LENGTH         6  // Length of segment number
#define MAX_LONGFILESPEC       256     // max length of long file name + extension
/**********************************************************************/
/* structure which holds info for the task "translation memory import"*/
/**********************************************************************/
typedef struct _DDEMEMIMP
{
  HWND    hwndOwner;                   // handle of instance window
  CHAR    szName[MAX_LONGFILESPEC];    // TM name
  CHAR    szInFile[CCHMAXPATH+1];      // input file name
  CHAR    szCurDir[CCHMAXPATH+1];      // current/active directory
  USHORT usBatchOccurrence[MAXALLBATCH];   // occurency of each spec
  DDERETURN  DDEReturn;                    // return structure
  HWND hwndErrMsg;                         // HWND to be used for error messages
} DDEMEMIMP, *PDDEMEMIMP;



#define MAX_TRNOTE_DESC     40             // length of TRNote Desc. Prefix


typedef int   (*PFN)();

typedef struct _TAGLIST
{
   USHORT       uOffset;              /* offset in TagTable                     */
   USHORT       uNumber;              /* number of tags in list                 */
} TAGLIST;

typedef struct _TAGTABLE           /* Tagtable                                    */
{
  USHORT      uLength;             /* length of data following                    */
  CHAR        chId[30];            /* file id for tagtab   @@                 */
  USHORT      uNumTags;            /* number of tags                              */
  USHORT      usVersion;           // version string
  TAGLIST     stTagIndex[27];      /* offset to tags in variable tag part of table */
  TAGLIST     stAttributeIndex[27];/* offset to attributes in variable attribute */
                                   /* part of table                               */
  TAGLIST     stFixTag;          /* offset to first fixed length TAG           */
  TAGLIST     stVarStartTag;     /* offset to first tag starting with wildcard */
  TAGLIST     stAttribute;       /* offset to first attribute                  */
  USHORT        uTagstart;         /* offset to first entry in Tagstart table    */
  USHORT        uStartpos;         /* offset to first entry in Tagstart position table */
  USHORT        uAttrEndDelim;     /* offset to first attribute end delimiter   */
  USHORT      uAttrLength;       /* attribute length, not used at moment    */
  USHORT        uTagNames;         /* offset to first Tagstring                  */
  CHAR    szSegmentExit[MAX_FILESPEC];    // name of user exit dll
  CHAR    chTrnote1Text[MAX_TRNOTE_DESC]; // Indicates a note of level 1
  CHAR    chTrnote2Text[MAX_TRNOTE_DESC]; // indicates a note of level 2
  CHAR    chStartText[MAX_TRNOTE_DESC];       // tag, may be followed by TRNote
  CHAR    chEndText[MAX_TRNOTE_DESC];            // end tag
  USHORT  uAddInfos;                             // offset to additional tag infos
   // new fields contained in tagtables TAGTABLE_VERSION3 and above
   CHAR        chSingleSubst;          // character to use for single substitution
   CHAR        chMultSubst;            // character to use for multiple substitution
   CHAR        szDescriptiveName[MAX_DESCRIPTION]; // descriptive name for format (displayed in format combobox)
   CHAR        szDescription[MAX_DESCRIPTION]; // format description
   USHORT      usCharacterSet;         // characterset to use for import/export
   BOOL        fUseUnicodeForSegFile;  // TRUE = store segmented files in Unicode format
   ULONG       ulPassword;             // protection password
   ULONG       aulFree[9];             // for future use
   BOOL        fProtected;             // TRUE = markup table is protected
     /*******************************************************************/
      /* The following BOOL fReflow is a 3state BOOL:                    */
      /* 0           = not used, as previously                           */
      /* 1           = Reflow allowed                                    */
      /* 2           = no Reflow, for MRI etc                            */
      /*******************************************************************/
   BOOL        fReflow;                //status of Reflow
   BOOL        afFree[8];              // for future use
   CHAR        szFree[80];             // for future use
} TAGTABLE, *PTAGTABLE;

// structure for tag tree nodes (used in EQFBFastTokenize)
typedef struct _NODE
{
   CHAR_W   chPos;                       // character position in tag
   CHAR_W   chNode;                      // node character
   SHORT  sTokenID;                    // ID of token
   struct _NODE *pNext;                // ptr to next node (NULL = end of chain)
} NODE, *PNODE;



/**********************************************************************/
/* Structures for new tokenize function                               */
/**********************************************************************/
typedef struct _TREENODE
{
   CHAR_W   chPos;                       // character position in tag
   CHAR_W   chNode;                      // node character
   SHORT    sTokenID;                    // ID of token
   PSZ_W    pszEndDel;                   // ptr to tag end delimiter
   USHORT   usLength;                    // length of length-terminated tags
   USHORT   usColPos;                    // colums position or 0
   BOOL     fAttr;                       // tag-has-attributes flag
   BOOL     fUnique;                     // this-path-is-unique flag
                                       // ( if flag is FALSE there are more
                                       // possible paths on the current level)
   BOOL   fNoMatch;                    // no match due to substitution characters
   BOOL   fTransInfo;                  // TRUE = tag may contain translatable info
   struct _TREENODE *pNextLevel;       // ptr to next node level (NULL = end of chain)
   struct _TREENODE *pNextChar;        // ptr to next character on same level
} TREENODE, *PTREENODE;

/**********************************************************************/
/* Structure for allocated node ares                                  */
/**********************************************************************/
typedef struct _NODEAREA
{
  PTREENODE   pFreeNode;               // ptr to next free node
  USHORT      usFreeNodes;             // number of free nodes
  struct _NODEAREA *pNext;             // ptr to next node area or NULL if none
  PTREENODE   pRootNode;               // root node of node tree
  TREENODE    Nodes[1];                // array of nodes
} NODEAREA, *PNODEAREA;


typedef EQF_BOOL (*PFNGETSEGCONTTEXT)( PSZ_W, PSZ_W, PSZ_W, PSZ_W, LONG, ULONG );

// structure for loaded tag tables
struct LOADEDTABLE
{
   CHAR        szName[MAX_FNAME];      // tag table name (w/o path and ext.)
   SHORT       sUseCount;              // number of active table users
   PTAGTABLE   pTagTable;              // pointer to tag table in memory
   PNODE       pTagTree;               // Tag tree of table
   PNODE       pAttrTree;              // attribute tree of table
   PNODEAREA   pNodeArea;              // area for node trees
   PNODEAREA   pAttrNodeArea;          // area for attribute node tree
   HMODULE     hmodUserExit;           // user exit (if any)
   BOOL        fUserExitLoadFailed;    // TRUE = load of user exit failed
   PFN         pfnProtTable;           // creat eprotect table function
   BOOL        fProtTableLoadFailed;   // TRUE = ProtTable function load failed
   // new fields contained in tagtables TAGTABLE_VERSION3 and above
   // these fields are copied from the TAGTABLE structure (if available) or filled
   // with default values (for older tagtables)
   CHAR        chSingleSubst;          // character to use for single substitution
   CHAR        chMultSubst;            // character to use for multiple substitution
   CHAR        szDescriptiveName[MAX_DESCRIPTION]; // descriptive name for format (displayed in format combobox)
   CHAR        szDescription[MAX_DESCRIPTION]; // format description
   USHORT      usCharacterSet;         // characterset to use for import/export
   BOOL        fUseUnicodeForSegFile;  // TRUE = store segmented files in Unicode format
   PSZ_W       pTagNamesW;
   PSZ_W       pAttributesW;
   PVOID       pTagTableW;
   PFN         pfnCompareContext;      // compare context info function
   BOOL        fCompareContextLoadFailed;  // TRUE = CompareContext function load failed
   PFN         pfnProtTableW;          // create protect table function (Unicode)
   PFNGETSEGCONTTEXT         pfnGetSegContext;       // get segment context function (Unicode)
   BOOL        fGetSegContextLoadFailed;  // TRUE = GetSegContext function load failed
   BOOL        fReflow;                   // 0=notspec. 1= Reflow allowed, 2= no Reflow
   PFN         pfnQueryExitInfo;        // query exit info function
   BOOL        fQueryExitInfoLoadFailed;// TRUE = query exit info function load failed
   CHAR_W      chMultSubstW;			// multiple substitution
   CHAR_W      chSingleSubstW;          // single substitution
   CHAR_W      chTrnote1TextW[MAX_TRNOTE_DESC]; // TRNotesText1
   CHAR_W      chTrnote2TextW[MAX_TRNOTE_DESC]; // TRNotesText2
   ULONG       ulTRNote1Len;			// length of TRNotesText1
   ULONG       ulTRNote2Len;			// lenght of TRNotesText2
   // timestamp when tag table was loaded the last time
   ULONG       ulTimeStamp;
};
using PLOADEDTABLE = LOADEDTABLE *;

#include <vector>
#include <proxygen/httpserver/ResponseHandler.h>
#include "opentm2/core/utilities/LogWrapper.h"
#include "win_types.h"

class ChunkBufferOld{
  long m_bytesCollecedInChunk = 0;
  long m_bytesSend = 0;
  static constexpr size_t chunkSize_ = 4096;
  std::vector<unsigned char> m_buff;
  proxygen::ResponseHandler* m_responseHandler = nullptr;

  bool canFit(long size);
  
  void triggerChunkSend();
public:
  void writeToBuff(const void * data, long size);
  void setResponseHandler(proxygen::ResponseHandler* responseHandler){m_responseHandler = responseHandler;}
  bool isActive()const {return m_responseHandler != 0; }

  ChunkBufferOld(){m_buff.reserve(chunkSize_+1);}
  ~ChunkBufferOld(){
    T5LOG(T5INFO)<< "called dctor of chunk buffer- sending last chunk";
    triggerChunkSend();
  }
};




// callback function which is used by ExtMemImportprocess to insert segments into the memory
typedef USHORT (/*APIENTRY*/ *PFN_MEMINSERTSEGMENT)( LONG lMemHandle, PMEMEXPIMPSEG pSegment );

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

//+----------------------------------------------------------------------------+
//| Our TMX import export class                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
class TMXParseHandler;


// size of file read buffer in preprocess step
#define TMX_BUFFER_SIZE 8096

class CTMXExportImportOld
{
  public:
    // constructor/desctructor
	  CTMXExportImportOld();
	  ~CTMXExportImportOld();
    // export methods
    USHORT WriteHeader( const char *pszOutFile, PMEMEXPIMPINFO pMemInfo );
    USHORT WriteSegment( PMEMEXPIMPSEG pSegment  );
    USHORT WriteEnd();
    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo, ImportStatusDetails* pImportStatusDetails ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG pMemHandle, ImportStatusDetails*     pImportData  ); 
    USHORT EndImport(); 
    USHORT getLastError( PSZ pszErrorBuffer, int iBufferLength );

  protected:
    USHORT WriteTUV( PSZ pszLanguage, PSZ pszMarkup, PSZ_W pszSegmentData );
    USHORT PreProcessInFile( const char *pszInFile, const char *pszOutFile );


    CXmlWriter m_xw;
    TMXParseHandler *m_handler;          // our SAX handler 
    xercesc::SAXParser* m_parser;
    xercesc::XMLPScanToken m_SaxToken; 
    unsigned int m_iSourceSize;          // size of source file
    PTOKENENTRY m_pTokBuf;               // buffer for TaTagTokenize tokens
    CHAR m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    PLOADEDTABLE m_pLoadedTable;         // pointer to currently loaded markup table
    PLOADEDTABLE m_pLoadedRTFTable;      // pointer to loaded RTF tag table
    CHAR m_szInFile[512];                // buffer for input file
    CHAR m_TempFile[540];                // buffer for temporary file name
    BYTE m_bBuffer[TMX_BUFFER_SIZE+1];
    PMEMEXPIMPINFO m_pMemInfo;
    CHAR_W m_szSegBuffer[MAX_SEGMENT_SIZE+1]; // buffer for the processing of segment data
    int  m_currentTu;                    // export: number of currently processed tu
};


struct MEM_LOAD_DLG_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// TM name to import to
 CHAR         szShortMemName[MAX_FILESPEC];// TM name to import to
 CHAR         szMemPath[2048];            // TM path + name to import to
 CHAR         szFilePath[2048];           // File to be imported path + name
 HFILE        hFile;                      // Handle of file to be imported
 PMEM_IDA     pIDA;                       // pointer to the memory database IDA
 BOOL         fAscii;                     // TRUE = ASCII else internal format
 CHAR         szName[MAX_FNAME];          // file to be imp. name without ext
 CHAR         szExt[MAX_FEXT];            // file to be imp. ext. with leading dot
 CHAR         szString[2048];             // string buffer
 CHAR         szDummy[_MAX_DIR];          // string buffer
 BOOL         fDisabled;                  // Dialog disable flag
 CONTROLSIDA  ControlsIda;                //ida of controls utility
 HPROP        hPropLast;                  // Last used properties handle
 PMEM_LAST_USED pPropLast;                // Pointer to the last used properties
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMIMP   pDDEMemImp;                 // ptr to batch memory import data
 HWND         hwndNotify;                 // send notification about completion
 OBJNAME      szObjName;                  // name of object to be finished
 USHORT       usImpMode;                  // mode for import
 BOOL         fMerge;                     // TRUE = TM is being merged
 PSZ          pszList;                    // list of selected import files or NULL
 BOOL         fSkipInvalidSegments;       // TRUE = skip invalid segments during import
 ULONG        ulOemCP;                    // CP of system preferences lang.
 ULONG        ulAnsiCP;                   // CP of system preferences lang.
 BOOL         fIgnoreEqualSegments;       // TRUE = ignore segments with identical source and target string
 BOOL         fAdjustTrailingWhitespace;  // TRUE = adjust trailing whitespace of target string
 BOOL         fMTReceiveCounting;         // TRUE = count received words and write counts to folder properties
 OBJNAME      szFolObjName;               // object name of folder (only set with fMTReceiveCounting = TRUE)
 HWND         hwndToCombo;                // handle of "to memory" combo box
 BOOL         fYesToAll;                  // yes-to-all flag for merge confirmation
 BOOL         fImpModeSet;                // TRUE = imp mode has been set by the impot file check logic
 std::shared_ptr<EqfMemory>    mem;                      // pointer to memory being imported
 OtmProposal  *pProposal;                 // buffer for proposal data
 BOOL          fForceNewMatchID;          // create a new match segment ID even when the match already has one
 BOOL          fCreateMatchID;            // create match segment IDs
 CHAR          szMatchIDPrefix[256];      // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];     // prefix to be used for the match segment ID (UTF16 version)

 CHAR         szSegmentID[SEGMENT_CLUSTER_LENGTH +
                          SEGMENT_NUMBER_LENGTH + 2]; // Segment identification

 PTOKENENTRY  pTokenList;                     // Pointer to Token List
 std::string fileData;

 PTOKENENTRY  pTokenEntry;                    // A pointer to token entries
 PTOKENENTRY  pTokenEntryWork;                // A work pointer to token entries

 MEMEXPIMPINFO* pstMemInfo;                        // buffer for memory information

 
 CTMXExportImport TMXImport;             // handle of external memory import functions
 PLOADEDTABLE pFormatTable;                   // Pointer to Format Table

 MEMEXPIMPSEG*  pstSegment;                        // buffer for segment data
 BOOL         fEOF;                           // Indicates end of file
};


class TMCursor
{
  ulong _recordKey;
  ushort _targetKey;
public:


  int SplitProposalKeyIntoRecordAndTarget(char    *pszKey, ulong   *pulKey,  ushort  *pusTargetNum);
  int parseAndSetInternalKey(char* pszKey){
    return SplitProposalKeyIntoRecordAndTarget(pszKey, &_recordKey, &_targetKey);
  }
  
  int reset(){ 
    return setInternalKey(0,0);
  }
  
  int setFirstInternalKey()  { 
    return setInternalKey(7,1); 
  }

  int moveToNextRecord() {
    return setInternalKey(_recordKey+1, 1); 
  }

  int setInternalKey(ulong recordKey, ushort targetKey){
    _recordKey = recordKey; 
    _targetKey = targetKey; 
    return 0;
  }


  bool isValid()const{
    return _recordKey > 0 && _targetKey > 0;
  }

  std::string getStr() const { 
    return std::to_string(_recordKey) + ":" + std::to_string(_targetKey) ; 
  }

  ulong getRecordKey()const { return _recordKey; }
  ushort getTargetKey()const { return _targetKey; }

};


//using MEM_LOAD_DLG_IDA* = MEM_LOAD_DLG_IDA *;
// internal session data area
typedef struct _FCTDATA
{
  std::string fileData;
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
  MEM_LOAD_DLG_IDA       MemLoadIda;            // pointer to analysis input data
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
  

  USHORT MemFuncExportProcess();
  USHORT MemFuncPrepExport
(
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions,                // options for Translation Memory export
  std::shared_ptr<EqfMemory> _mem
);
  std::shared_ptr<EqfMemory> mem;
  proxygen::ResponseHandler* responseHandler = nullptr;
 
  TMCursor startingInternalKey;
  TMCursor nextInternalKey;
  
  ulong numOfProposalsRequested = 0;
  folly::IOBufQueue bufQueue;
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





/*! \brief memory proposal types */
	enum MemProposalType
	{
		UNDEFINED_PROPTYPE,			/*!< proposal type has not been set or is not available */
		MANUAL_PROPTYPE,	  		/*!< proposal was produced by manual translation */
		MACHINE_PROPTYPE,	    	/*!< proposal was produced by automatic (machine) translation */
		GLOBMEMORY_PROPTYPE, 	  /*!< proposal is from global memory  */
    GLOBMEMORYSTAR_PROPTYPE /*!< proposal is from global memory, to be marked with an asterisk  */
	};

/*! \brief match type of memory proposals */
	enum MemProposalMatchType
	{
		UNDEFINED_MATCHTYPE,			/*!< match type has not been set or is not available */
		EXACT_MATCHTYPE,	    		/*!< proposal is an exact match */
		EXACTEXACT_MATCHTYPE,		/*!< proposal is an exact match, document name and segment number are exact also */
		EXACTSAMEDOC_MATCHTYPE,	/*!< proposal is an exact match from the same document */
		FUZZY_MATCHTYPE,	    		/*!< proposal is a fuzzy match */
		REPLACE_MATCHTYPE 	    	/*!< proposal is a replace match */
	};

/*! \brief maximum size of segment data */
#define MEMPROPOSAL_MAXSEGLEN 2048

/*! \brief maximum size of names */
#define MEMPROPOSAL_MAXNAMELEN 256


/*! \brief Structure for sending or receiving memory proposal data
*/
typedef struct _MEMPROPOSAL
{
  /*! \brief ID of this proposal */
	char szId[MEMPROPOSAL_MAXNAMELEN];

  /*! \brief Source string of memory proposal  (UTF-16) */
  wchar_t szSource[MEMPROPOSAL_MAXSEGLEN+1];
	//std::wstring strSource;

	/*! \brief Target string of memory proposal (UTF-16). */
  wchar_t szTarget[MEMPROPOSAL_MAXSEGLEN+1];

	/*! \brief Name of document from which the proposal comes from. */
	char szDocName[MEMPROPOSAL_MAXNAMELEN];

	/*! \brief Short (8.3) name of the document from which the proposal comes from. */
	//std::string strDocShortName;
	char szDocShortName[MEMPROPOSAL_MAXNAMELEN];

	/*! \brief Segment number within the document from which the proposal comes from. */
  long lSegmentId;                  

	/*! \brief source language. */
	char szSourceLanguage[MEMPROPOSAL_MAXNAMELEN];

	/*! \brief target language. */
  char szTargetLanguage[MEMPROPOSAL_MAXNAMELEN];

	/*! \brief origin or type of the proposal. */
  MemProposalType eType;

	/*! \brief match type of the proposal. */
  MemProposalMatchType eMatch;

	/*! \brief Author of the proposal. */
  char szTargetAuthor[MEMPROPOSAL_MAXNAMELEN];

	/*! \brief Update time stamp of the proposal. */
  long    lTargetTime;

	/*! \brief Fuzziness of the proposal. */
  int iFuzziness;    

  /*! \brief Words parsed in fuzzy calculations of the proposal. */
  int iWords = -1;

  /*! \brief Diffs in fuzzy calculations of the proposal. */
  int iDiffs = -1;                 

	/*! \brief Markup table (format) of the proposal. */
  char szMarkup[MEMPROPOSAL_MAXNAMELEN];

  /*! \brief Context information of the proposal */
  //std::wstring strContext;  
  wchar_t szContext[MEMPROPOSAL_MAXSEGLEN+1];

  /*! \brief Additional information of the proposal */
  //std::wstring strAddInfo; 
  wchar_t szAddInfo[MEMPROPOSAL_MAXSEGLEN+1];
  
  /*! \brief is source lang is marked as prefered in languages.xml */
  bool fIsoSourceLangIsPrefered = false;
  /*! \brief is target lang is marked as prefered in languages.xml */
  bool fIsoTargetLangIsPrefered = false;

} MEMPROPOSAL, *PMEMPROPOSAL;


std::wstring convertStrToWstr(std::string& str);
std::string convertWstrToStr(std::wstring& wstr);
wchar_t* wcsupr(wchar_t *str);
char* strupr(char *str);

/*! \brief normalize white space within a string by replacing single or multiple white space occurences to a single blank
  \param pszString pointer to the string being normalized
  \returns 0 in any case
*/
SHORT normalizeWhiteSpace
(
  PSZ_W   pszData
);


// concordance search modes
#define SEARCH_IN_SOURCE_OPT  0x00020000L
#define SEARCH_IN_TARGET_OPT  0x00040000L
#define SEARCH_CASEINSENSITIVE_OPT 0x00080000L
#define SEARCH_WHITESPACETOLERANT_OPT 0x00100000L


#define SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT 0x00200000L
#define SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT 0x00400000L
#define SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT 0x00800000L
#define SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT 0x01000000L

#define SEARCH_FILTERS_LOGICAL_OR 0x02000000L
#define SEARCH_FILTERS_NOT 0x04000000L

typedef long FilterOptions;

class ProposalFilter{
//public:  
//  FilterParam m_param;  


public:

enum FilterField{    
    UNKNOWN_FIELD,
    TIMESTAMP,
    //SEG_ID,

    CHAR_FIELDS_START,
    SRC_LANG = CHAR_FIELDS_START,
    TRG_LANG,
    AUTHOR,
    DOC, 
    //MARKUP,

    WIDE_CHAR_FIELDS_START, 
    SOURCE = WIDE_CHAR_FIELDS_START,
    TARGET,
    CONTEXT,
    ADDINFO
  };

  enum FilterType{
    NONE = 0, UNKNOWN, EXACT, CONTAINS, RANGE
  };

  static int StrToFilterType(const char* str, FilterType &filterType, FilterOptions& options);


private:
  FilterField m_field;
  FilterType m_type;
  long m_timestamp1=0, m_timestamp2=0;
  std::string m_searchString;
  std::wstring m_searchStringW;
  FilterOptions m_searchOptions = 0;

public:

  ProposalFilter(std::string& search, FilterField field, FilterType type, FilterOptions searchOptions): m_searchOptions(searchOptions), 
        m_field(field), m_type(type), m_searchStringW(convertStrToWstr(search)),
        m_searchString(search){ 
          if ( searchOptions & SEARCH_CASEINSENSITIVE_OPT )
          { 
            wcsupr(&m_searchStringW[0]);
            m_searchString = convertWstrToStr(m_searchStringW);
          }
          if ( searchOptions & SEARCH_WHITESPACETOLERANT_OPT )
          { 
            normalizeWhiteSpace( &m_searchStringW[0] );
            m_searchString = convertWstrToStr(m_searchStringW);
          }
        };
  
  ProposalFilter(long t1, long t2, FilterField field = TIMESTAMP): m_timestamp1(t1), m_timestamp2(t2), m_field{field}, m_type(FilterType::RANGE){}

  //ProposalFilter(FilterParam&& param) m_param(param){}

  std::string toString()const;

  bool check(OtmProposal& prop);
};


  /*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date time
    \returns 0 
  */
  int convertTimeToUTC( long lTime, char *pszDateTime );

#endif //_LOW_LEVEL_OTM_DATA_STRUCTS_INCLUDED_
