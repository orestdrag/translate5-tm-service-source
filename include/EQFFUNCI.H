/*! EQFFUNCI.H      internal header file for non-DDE batch functions
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
*/
#pragma once 

#define FCTDATA_IDENTIFIER 0x1998A04FL
#define FCTSEGFILE_IDENTIFIER 0x2002F3ABL

// name delimiter for name lists (e.g. list of document names)
#define NAME_DELIMITER            ','

#include "win_types.h"
#include "EQF.H"
#include "tm.h"
#include <atomic>
#include <string>
#include "../source/opentm2/core/utilities/LogWrapper.h"
// the identifier for our functions
typedef enum _FCTFUNCTIONS
{
  FCT_NONE,
  FCT_EQFCREATEFOLDER,
  FCT_EQFEXPORTFOLDER,
  FCT_EQFCOUNTWORDS,
  FCT_EQFIMPORTDICT,
  FCT_EQFEXPORTDICT,
  FCT_EQFIMPORTDOC,
  FCT_EQFEXPORTDOC,
  FCT_EQFEXPORTDOCVAL,
  FCT_EQFIMPORTMEM,
  FCT_EQFEXPORTMEM,
  FCT_EQFORGANIZEMEM,
  FCT_EQFCREATEMEM,
  FCT_EQFANALYZEDOC,
  FCT_EQFDELETEMEM,
  FCT_EQFIMPORTFOLDER,
  FCT_EQFDELETEFOLDER,
  FCT_EQFARCHIVETM,
  FCT_EQFDELETEDOC,
  FCT_EQFCLEANMEMORY,
  FCT_EQFRENAME,
  FCT_EQFPROCESSNOMATCH,
  FCT_EQFEXPORTSEGS,
  FCT_EQFCREATEMARKUP,
  FCT_EQFIMPORTMEMEX,
  FCT_EQFFILTERNOMATCHFILE,
  FCT_EQFADDMATCHSEGID
} FCTFUNCTIONS, *PFCTFUNCTIONS;

// processing step identifiers
typedef enum _FCTPHASES
{
  FCTPHASE_INIT,
  FCTPHASE_NEXT,
  FCTPHASE_PROCESS,
  FCTPHASE_CLEANUP,
  FCTPHASE_DONE,
  FCTPHASE_ABORT
} FCTPHASES, *PFCTPHASES;


// internal segmented file control area
typedef struct _FCTSEGFILE
{
  ULONG       lMagicWord;              // FUNCDATA area identifier
  HPARSER     hParser;                 // parser handle
  HPARSSEGFILE hParsSegFile;           // handle of loaded segmented file
  PVOID       pNext;                   // ptr to next FCTSEGFILE area
  HSESSION    hSession;                // session handle
} FCTSEGFILE, *PFCTSEGFILE;



// Internal functions

// FctValidateSession: check and convert a session handle to a FCTDATA pointer
USHORT FctValidateSession
(
  HSESSION    hSession,                // session handle being validated
  PFCTDATA   *ppData                   // address of to caller's FCTDATA pointer
);

USHORT FctBuildCheckSum
(
  PFCTDATA    pData,                   // FCTDATA area
  PLONG       plCheckSum               // ptr to checksum variable
);

// Function to enable Automatic Refresh of TM List Windows after Non-DDE call
void SetSharingFlag(ULONG ulRefreshFlag);

// Function call I/F specific functions in other modules
USHORT TAFuncAnalyzeDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PSZ         pszMTOutputOptions,      // MT output options
  LONG        lOptions                 // options for analysis
);
USHORT TAFuncAnalysisProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
);
USHORT TAFuncPrepAnalysis
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PSZ         pszMTOutputOptions,      // MT output options
  LONG        lOptions                 // options for analysis
);

USHORT MemFuncAddMatchSegID
(
  PFCTDATA    pMatchSegIDData,         // function I/F session data
  PSZ         pszMemName,              // Translation Memory being processed
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  LONG        lOptions                 // options for function
);

USHORT MemFuncImportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  PSZ         pszUnused1,              // not in use, for future enhancements
  PSZ         pszUnused2,              // not in use, for future enhancements
  LONG        lOptions,                 // options for Translation Memory import
  ImportStatusDetails*     pImportData
);


USHORT MemFuncPrepImport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszTM_ID,                // translation memory ID or NULL if not used
  PSZ         pszStoreID,              // ID for the origin of the translation memory or NULL if not used
  PSZ         pszUnused1,              // not in use, for future enhancements
  PSZ         pszUnused2,              // not in use, for future enhancements
  LONG        lOptions                 // options for Translation Memory import
);


USHORT MemFuncImportProcess
(
  PFCTDATA    pData,                    // function I/F session data
  ImportStatusDetails*     pImportData

);

USHORT FolFuncCreateFolder
(
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // optional export conversion
  PSZ         pszReadOnlyMems          // list of readonly TMs or NULL
);

USHORT FolFuncCreateControlledFolder
(
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // optional export conversion
  PSZ         pszReadOnlyMems,         // list of readonly TMs or NULL
  PSZ         pszPassword,             // password
  PSZ         pszProjCoordName,        // name of the project coordinator
  PSZ         pszProjCoordMail,        // project coordinator's mail
  PSZ         pszTranslatorName,       // name of the translator
  PSZ         pszTranslatorMail,       // translator's mail
  PSZ         pszProductName,          // Name of the product
  PSZ         pszProductFamily,        // Product Famiily
  PSZ         pszSimilarProduct,       // Similar Product Family
  PSZ         pszProductDict,          // Product subject area dictionary
  PSZ         pszProductMem,           // Product subject area memory
  PSZ         pszPreviousVersion,      // Previous version of the product
  PSZ         pszVersion,              // Version of the Product
  PSZ         pszShipmentNumber        // Shipment number
);

USHORT FolFuncChangeFolProps
(
  PSZ         pszFolder,               // name of folder
  CHAR        chTargetDrive,           // traget drive
  PSZ         pszTargetLanguage,       // target language or NULL
  PSZ         pszMemName,              // folder Translation Memory or NULL
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszROMemories,           // list of read-only search memories or NULL
  PSZ         pszDescription,          // folder description or NULL
  PSZ         pszProfile,              // calculation profile name or NULL
  PSZ         pszShipment              // shipment number or NULL
);

USHORT FolFuncGetFolProps
(
  PSZ         pszFolderName,           // name of the folder
  PEXTFOLPROP pExtFolProps             // structure to fill in folder properties
);

USHORT FolFuncGetFolPropEx
(
  PSZ         pszFolderName,           // mand: name of the folder to get the property value from
  PSZ         pszKey,                  // mand: name of the requested value
                                       //@: DRIVE,TARGETLANGUAGE,SOURCELANGUAGE,MEMORY,DICTIONARIES,ROMEMORIES,DESCRIPTION,PROFILE,SHIPMENT
  PSZ         pszBuffer,               // mand: buffer for the returned value
  int         iBufSize                 // mand: size of the buffer
);


USHORT FolFuncCreateSubFolder
(
  PSZ         pszParentFolder,         // name of parent (sub)folder
  PSZ         pszSubFolderName,        // subfolders name
  PSZ         pszMemName,              // subfolders Translation Memory or NULL
  PSZ         pszMarkup,               // name of Markup used for subfolder
  PSZ         pszSourceLanguage,       // Source Language used for subfolder
  PSZ         pszTargetLanguage,       // Target Language used for subfolder
  PSZ         pszEditor,
  PSZ         pszConversion,           // Conversion used for subfolder
  PSZ         pszTranslator,           // Name of translator
  PSZ         pszTranslatorMail        // Mail of translator
);

USHORT TAFuncArchTM
(
  PFCTDATA    pData,
  PSZ         pszFolderName,
  CHAR        chToDrive,
  PSZ         pszDocuments,
  PSZ         pszMemName,
  LONG        lOptions
);

USHORT CntFuncCountWords
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  LONG        lOptions,                // options for word count
  PSZ         pszOutFile               // name of output file
);

USHORT FolFuncExportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszTargetPath,           // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,          // export description or NULL
  PSZ         pszExportAs,
  PSZ         pszMemoryExportAs        // export folder memory with this name
);

USHORT DicFuncImportDict
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszDictName,             // name of dictionary
  PSZ         pszPassword,             // password of dictionary
  LONG        lOptions                 // dictionary import options
);

// export a dictionary
USHORT DicFuncExportDict
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
);

// import a document
USHORT DocFuncImportDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder receiving the documents
  PSZ         pszFiles,                // list of input files (documents)
  PSZ         pszMemname,              // document Translation Memory or NULL
  PSZ         pszMarkup,               // document markup or NULL
  PSZ         pszEditor,               // document editor or NULL
  PSZ         pszSourceLanguage,       // document source language or NULL
  PSZ         pszTargetLanguage,       // document target language or NULL
  PSZ         pszAlias,                // alias for document name or NULL
  PSZ         pszStartPath,            // optional start path
  PSZ         pszConversion,           // optional export conversion
  LONG        lOptions                 // document import options or 0L
);

// export a document
USHORT DocFuncExportDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszFiles,                // list of documents with path information
  PSZ         pszStartPath,            // optional start path
  LONG        lOptions                 // options for document export
);

USHORT DocFuncExportDocVal
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,   
  PSZ         pszFiles,     
  PSZ         pszStartPath,
  LONG        lFormat,                          
  LONG        lOptions,
  LONG        lMatchTypes,
  LONG        lType,
  PSZ         pszTranslator,
  PSZ         pszPM
);

USHORT MemFuncExportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
);

// create a new Translation Memory
USHORT MemFuncCreateMem
(
  const char*         pszMemname,              // name of new Translation Memory
  const char*         pszDescription,          // description for new Translation Memory or NULL
  const char*         pszSourceLanguage,       // Translation Memory source language
  LONG                lOptions                 // type of new Translation Memory
                                               // LOCAL_TM_OPT is the default
);

// delete a Translation Memory
USHORT MemFuncDeleteMem
(
  PSZ         pszMemName               // Translation Memory being deleted
);

USHORT MemFuncClearMTFlag( PSZ pszInMemory, PSZ pszOutMemory );

// organize a Translation Memory
//USHORT MemFuncOrganizeMem
//(
//  PFCTDATA    pData,                   // function I/F session data
//  PSZ         pszMemName               // Translation Memory being deleted
//);

USHORT FolFuncImportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  PSZ         pszNewMemNames,          // opt: comma separated list of new memory names, the first entry is used to rename the folder memory, the remaining names are used to rename the search memories
  LONG        lOptions                 // folder import options
);

USHORT FolFuncDeleteFolder
(
  PSZ         pszFolderName            // name of folder being deleted
);

USHORT DocFuncDeleteDoc
(
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments             // list of documents being deleted
);

USHORT MemFuncCreateITM
(
    PSZ  pszMemName,
    PSZ  pszFilePairs,
    PSZ  pszMarkup,
    PSZ  pszSGMLMemFile,
    PSZ  pszSourceLanguage,
    PSZ  pszTargetLanguage,
    PSZ  pszSourceStartPath,
    PSZ  pszTargetStartPath,
    LONG lType
);


USHORT TAFuncArchiveTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for analysis
);
USHORT TAFuncArchiveTMProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
);
USHORT TAFuncPrepArchTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  CHAR        chToDrive,               // drive letter
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for analysis
);

USHORT MemFuncCleanMemory
(
  PFCTDATA         pData,              // function I/F session data
  PSZ              pszFolder,          // name of folder containing the translatable material
  PSZ              pszInMemory,        // fully qualified name of external input memory (encoding: UTF-16)
  PSZ              pszOutMemory,       // name of internal output memory or fully qualified name of external output memory
  LONG             lOptions            // options for processing
);

USHORT MemFuncProcessNomatch
(
  PFCTDATA    pFuncData,              // function I/F session data
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszUnused,               // additional parameter for future enhancements - currently no in use
  LONG        lOptions                 // options for processing
);

USHORT FolFuncDeleteMTLog
(
  PSZ         pszFolderName            // name of the folder
);

USHORT MarkupFuncCreateMarkup
(
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Markup Table create
);

USHORT FolFuncOpenDoc
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszDocument,             // name of document
  ULONG       ulSegNum,                // segment number of segment to be activated
  ULONG       ulLine,                  // line to be activated (ulSegNum has to be 0)
  PSZ_W       pszSearch,               // search string 
  PSZ         pszTrackID               // TVT track ID to be activated
);

USHORT FolFuncRemoveDocs
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszListFile              // name of list file 
);

USHORT FolFuncRestoreDocs
(
  PSZ         pszFolderName            // name of the folder
);

USHORT FolFuncAddCTIDList
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszCTIDListFile          // fully qualified name of the CTID list file
);

unsigned short MorphFuncCheckSpelling
(
  char *pszLanguage,                   // name of the language being used for the spell checking
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation
  long  lOptions
);

unsigned short MorphFuncReduceToStemForm
(
  char *pszLanguage,                   // name of the language being used for the stem form reduction
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long  lOptions
);

// function removed irrelevant segments from an external memory
USHORT FilterNoMatchFile
(
  PFCTDATA         pData,                   // function I/F session data
  char             *pszInNoMatchXML,        // IN: input nomatch file (XML format)
  char             *pszGlobMemOptionFile,   // IN: global memory option file
  char             *pszMemory,              // IN: Memory for filtering
  char             *pszOutNoMatchXML,       // OUT: filtered nomatch file (XML format)  
  char             *pszOutNoMatchEXP,       // OUT: filtered nomatch file (EXP format)  
  char             *pszWordCountReport,     // OUT: Word count report
  LONG             lOptions                 // options for processing
);

// delete a dictionary
USHORT DicFuncDeleteDict
(
  PSZ         pszMemName               // Translation Memory being deleted
);

// change the machine translation flags of memory proposals
USHORT MemFuncChangeMFlag
(
    PSZ  pszMemName,
    LONG lAction
);

USHORT InternalExportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszTargetPath,           // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,          // export description or NULL
  PSZ         pszExportAs,              // Export as - optional new name for exported folder
  PSZ         pszMemoryExportAs         // Export folder memory with this new name or NULL
);

/*! \brief Create a new folder
\param hSession the session handle returned by the EqfStartSession call
\returns 0 if successful or an error code in case of failures
*/
USHORT InternalImportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  PSZ         pszNewFolderName,        // new name for the folder
  PSZ         pszNewMemNames,          // opt: comma separated list of new memory names, the first entry is used to rename the folder memory, the remaining names are used to rename the search memories
  LONG        lOptions                 // folder import options
);


// fuzzy semgnet search function
USHORT FolFuncFuzzySearch
(
  PSZ              pszFolderName,      // name of folder
  PSZ              pszDocuments,       // list of documents or NULL 
  PSZ              pszOutputFile,      // fully qualified name of output file
  int              iSearchMode,        // search mode
  int              iClass,             // searched class
  LONG             lOptions            // processing options
);
