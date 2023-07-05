//+----------------------------------------------------------------------------+
//| EQFMEMIE.H                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Memory Export/Import Interface                                |
//|                                                                            |
//+----------------------------------------------------------------------------+

#ifndef EQFMEMIE_H_INCLUDED
#define EQFMEMIE_H_INCLUDED

#include "lowlevelotmdatastructs.h"

class ImportStatusDetails;


//#pragma pack( push, segmentpacking, 1 )
//#pragma pack( pop, segmentpacking )

// callback function which is used by ExtMemImportprocess to insert segments into the memory
typedef USHORT (/*APIENTRY*/ *PFN_MEMINSERTSEGMENT)( LONG lMemHandle, PMEMEXPIMPSEG pSegment );

//+----------------------------------------------------------------------------+
//|                                                                            |
//| functions performing the external memory export/import                     |
//|                                                                            |
//+----------------------------------------------------------------------------+

//
// EXTMEMEXPORTSTART
//
// This function is called once at the begin of the memory export
USHORT /**/ EXTMEMEXPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszOutFile,         // fully qualified file name for the exported memory
  PMEMEXPIMPINFO   pMemInfo            // pointer to structure containing general information for the exported memory
);

//
// EXTMEMEXPORTPROCESS
//
// This function is called for each segment being exported
USHORT /**/ EXTMEMEXPORTPROCESS
( 
  LONG             lHandle,                 // export/import handle as set by ExtMemExportStart function
  PMEMEXPIMPSEG    pSegment                 // pointer to structure containing the segment data
);

//
// EXTMEMEXPORTEND
//
// This function is called once at the end of the memory export
USHORT /**/ EXTMEMEXPORTEND
( 
  LONG             lHandle                  // export/import handle as set by ExtMemExportStart function
);

//
// EXTMEMIMPORTSTART
//
// This function is called once at the begin of the memory import
USHORT /**/ EXTMEMIMPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszInFile,          // fully qualified file name for the memory being imported
  PMEMEXPIMPINFO   pMemInfo,           // pointer to structure receiving general information for the imported memory
  ImportStatusDetails*     pImportData = nullptr
);

//
// EXTMEMIMPORTPROCESS
//
// This function is called until it returns an error condition or signals the end of the import
//
USHORT /**/ EXTMEMIMPORTPROCESS
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PFN_MEMINSERTSEGMENT pfnInsertSegment,    // callback function to insert segments into the memory
  LONG             lMemHandle,              // memory handle (has to be passed to pfnInsertSegment function)
  ImportStatusDetails*     pImportData
);

//
// EXTMEMIMPORTEND
//
// This function is called once at the end of the memory import
USHORT /**/ EXTMEMIMPORTEND
( 
  LONG             lHandle                  // Import/import handle as set by ExtMemImportStart function
);

#endif
