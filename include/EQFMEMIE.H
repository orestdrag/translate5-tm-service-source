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

//#include "EQFOSWIN.H"

// return codes

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

//
// EXTMEMIMPORTGETLASTERROR
//
// Get the description of the last error (This function has to be called before calling EXTMEMIMPORTEND)
 
USHORT /**/ EXTMEMIMPORTGETLASTERROR
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PSZ              pszErrorBuffer,          // pointer to buffer for the error text
  int              iBufferLength            // sizeof buffer in number of characters
);

// function pointer typedefs for the functions above
typedef USHORT (/**/ *PFN_EXTMEMEXPORTSTART)( PLONG plHandle, PSZ pszOutFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (/**/ *PFN_EXTMEMEXPORTPROCESS)( LONG lHandle, PMEMEXPIMPSEG pSeg);
typedef USHORT (/**/ *PFN_EXTMEMEXPORTEND)( LONG lHandle );
typedef USHORT (/**/ *PFN_EXTMEMIMPORTSTART)( PLONG plHandle, PSZ pszInFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (/**/ *PFN_EXTMEMIMPORTPROCESS)( LONG lHandle, PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLONG lProgress );
typedef USHORT (/**/ *PFN_EXTMEMIMPORTEND)(LONG lHandle );

USHORT TMXTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (TMX) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PSZ              pszMarkup,           // markup to use for segments
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs,       // number of skipped segments
  PSZ              pszErrorMsgBuffer,   // buffer for error message texts,
  int              iErrorMsgBufferSize  // size of buffer for error message texts
);

USHORT EXPTOTMX
( 
  PSZ              pszInMem,            // fully qualified name of input (EXP) memory
  PSZ              pszOutMem,           // fully qualified name of output (TMX) memory
  PSZ              pszInMode,           // input mode "ANSI"/"ASCII"/"UTF16" or a codepage number
  LONG             lOutMode,            // output mode UTF16/UTF8
  PLONG            plSegs,              // number of written (=valid) segments
  PSZ              pszMsgBuffer,        // buffer for error messages
  PLONG            plInvalidSegs       // number of skipped segments
);


USHORT XLFTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (XLIFF) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs        // number of skipped segments
);



#endif
