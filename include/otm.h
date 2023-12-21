#ifndef _OTM_INCLUDED_
#define _OTM_INCLUDED_

#include <vector>
#include <atomic>
#include <string>
#include <time.h>
#include <sstream>
#include <tuple>
#include "FilesystemHelper.h"
#include "win_types.h"
#include "lowlevelotmdatastructs.h"
  
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

//SHORT  EQFNTMOpen
//(
//  USHORT usOpenFlags,                 // Read Only or Read/Write
//);

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
  std::atomic_int segmentsCount{0};
  std::atomic_int segmentsImported{0};
  std::atomic_int invalidSegments{0};
  std::atomic_int invalidSymbolErrors{0};
  std::atomic_int resSegments {0};
  long filteredSegments{0};
  std::map<int, int> invalidSegmentsRCs;
  std::string importTimestamp;
  std::vector<std::tuple<int,int>> firstInvalidSegmentsSegNums;
  long lReorganizeStartTime;
  std::stringstream importMsg;
  bool fReorganize{0}; // true for reorganize call, false for import
  size_t activeSegment{0};
  
  void reset(){
    firstInvalidSegmentsSegNums.clear();
    firstInvalidSegmentsSegNums.reserve(100);
    importMsg.str("");
    usProgress = 0;
    segmentsCount = -1;
    segmentsImported = -1;
    invalidSegments = -1;
    invalidSymbolErrors = -1;
    resSegments = -1;
    fReorganize = 0;
    lReorganizeStartTime = 0;
    importTimestamp = "not finished";    
    invalidSegmentsRCs.clear();
    activeSegment = 0;
  }
  
  std::string toString(){
    std::string res = "progress = " + std::to_string(usProgress);
    if(fReorganize){
      res += "; segmentsReorganized = ";
    }else{
      res +=  "; segmentsImported = " ;
    }
    res += std::to_string(segmentsImported) + "; invalidSegments = " + std::to_string(invalidSegments) + "; time = " + importTimestamp;
    return res;
  }
};



#endif // _OTM_INCLUDED_
