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


//+----------------------------------------------------------------------------+
//|                                                                            |
//| functions performing the external memory export/import                     |
//|                                                                            |
//+----------------------------------------------------------------------------+

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

#include "tm.h"
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
