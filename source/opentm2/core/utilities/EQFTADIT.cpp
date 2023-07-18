#ifdef TO_BE_REMOVED
//+----------------------------------------------------------------------------+
//| EQFTADIT.CPP                                                            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//| 	Copyright (C) 1990-2012, International Business Machines                 |
//| 	Corporation and others. All rights reserved                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: SGML-DITA special processing functions                        |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

//#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// the Win32 Xerces build requires the default structure packing...
//#pragma pack( push )
//#pragma pack(8)

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

//#pragma pack( pop )


XERCES_CPP_NAMESPACE_USE


//#pragma pack( push, TM2StructPacking, 1 )

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include "EQF.H"                  // General .H for EQF
#include "EQFMEMIE.H"

//#pragma pack( pop, TM2StructPacking )


#endif

