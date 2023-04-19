#ifndef _OTM_INCLUDED_
#define _OTM_INCLUDED_

#include <vector>
#include <atomic>
#include <string>
#include <time.h>
#include <sstream>
#include "FilesystemHelper.h"
#include "win_types.h"
#include "lowlevelotmdatastructs.h"

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


class JSONFactory;

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

#endif // _OTM_INCLUDED_