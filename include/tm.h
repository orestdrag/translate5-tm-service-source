#ifndef _tm_h_included_
#define _tm_h_included_

#include <vector>
#include <atomic>
#include <string>
#include <time.h>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "win_types.h"
#include "requestdata.h"
#include "otm.h"
#include "Proposal.h"

#define INCL_EQF_TAGTABLE         // tag table and format functions
#define INCL_EQF_TP
#define INCL_EQF_TM
#define INCL_EQF_DAM
#include "EQF.H"

#include "FilesystemHelper.h"
#include "LogWrapper.h"

#include "../source/opentm2/core/pluginmanager/OtmPlugin.h"
#include "../source/opentm2/core/utilities/Stopwatch.hpp"

#define MEM_START_ORGANIZE  USER_TASK + 1
#define MEM_ORGANIZE_TASK   USER_TASK + 2
#define MEM_END_ORGANIZE    USER_TASK + 3



#define KEY_DIR_SIZE         4096  // key directory size
#define TM_PREFIX_SIZE          8  // length of prefix bytes in TMT db
#define MAX_SEC_LENGTH       30        // max length of secondary key
#define MAX_LINE_LENGTH      80        // max length of each line in lang file
#define FN_LENGTH            13        // length of input filename
#define PRIM_KEY_LENGTH       4        // number of characters in primary key
#define DATA_IN_SIZE       3900        // buffer size for input
#define DATA_OUT_SIZE      3900        // buffer size for output
#define CODEPAGE_SIZE       256        // size of codepage of the language
#define SEG_MARKER_LENGTH     3        // length of segment marker
#define MAX_TGT_LENGTH     2047        // max length of each target.
#define MAX_MATCH_TAB_ENTRIES 5        // number of entries in match table
#define CREATE_BUFFER_SIZE   40000     // buffer size for create_in
#define MAX_TM_LIST_NUMBER    500      // max. number of TMs that can be listed
                                       // by the TMC_GET_SERVER_TM_LIST command
#define GETPART_BUFFER_SIZE 16384      // read a 16 KB block at a time
#define MEM_PROP_SIZE    2048          // Global size of all memory database properties
#define	_MAX_DIR	256

typedef CHAR  SHORT_FN  [FN_LENGTH];
typedef CHAR  BUFFERIN  [DATA_IN_SIZE];
typedef UCHAR BUFFEROUT  [DATA_OUT_SIZE],
              ACHPRIMKEY [PRIM_KEY_LENGTH],
              SZSECKEY   [MAX_SEC_LENGTH + 1];

typedef CHAR  LANG_LINE  [MAX_LINE_LENGTH + 1];
typedef CHAR  LONG_FN    [MAX_LONGFILESPEC];



struct TMX_EXT_OUT_W
{
  TMX_PREFIX_OUT stPrefixOut;      //prefix of output buffer

  CHAR_W    szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR_W    szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szOriginalSourceLanguage[MAX_LANG_LENGTH]; //language name of the source
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_FILESPEC];          //author name of target
  USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  TIME_L    lTargetTime;                       //time stamp of target
  CHAR_W    szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W    szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  ULONG ulRecKey;
  USHORT usTargetKey;  

  ULONG ulTmKey;                   //tm record key
  USHORT usNextTarget;             //which target record to address next
  ULONG ulMaxEntries;              //number of entries in tm data file
};
using PTMX_EXT_OUT_W = TMX_EXT_OUT_W *;



//#include "../pluginmanager/PluginManager.h"

#include "EQF.H"

// ************ Translation memory flags ********************************
#define far

#define  MEM_OUTPUT_ASIS          0   // Do not convert translation memory output
#define  MEM_OUTPUT_CRLF          1   // Convert translation memory output to CRLF
#define  MEM_OUTPUT_LF            2    // Convert translation memory output to LF

// defines for special names mode of TMExtract (only valid with TMExtract!)
#define  MEM_OUTPUT_TAGTABLES     3   // return list of tag tables of TM
#define  MEM_OUTPUT_AUTHORS       4   // return list of authors of TM
#define  MEM_OUTPUT_DOCUMENTS     5   // return list of documents of TM
#define  MEM_OUTPUT_LANGUAGES     6   // return list of languages of TM
#define  MEM_OUTPUT_LONGNAMES     7   // return list of document long names of TM
#define  MEM_OUTPUT_ALLDOCS       8   // return list of all documensts (long
                                      // names and short names for docs w/o long name)

#define EXCLUSIVE               0
#define NONEXCLUSIVE            1
#define EXCLUSIVE_FOR_GET_PART  2
#define FOR_ORGANIZE            3
#define READONLYACCESS          4

// defines used in TmOpen and MemCreateDlg
#define  TM_LOCAL                  0     // TM resides local
#define  TM_REMOTE                 1     // TM resides remote on a server
#define  TM_LOCALREMOTE            2     // TM may be local or remote
#define  TM_SHARED                 3     // TM resides on a shared drive

//  Match value constants: the similarity level classes   (lSimLevel)
#define BASE_SIMILAR          0L
#define BASE_EXACT_DATE       200L
#define BASE_EXACT_IND        300L
#define BASE_EXACT_SEG_NUM    400L

#define MAX_SIMILAR_VAL       100L
#define MAX_EXACT_DATE_VAL    (BASE_EXACT_IND - 1L)
#define MAX_EXACT_IND_VAL     (BASE_EXACT_SEG_NUM - 1L)
#define MAX_EXACT_SEG_NUM_VAL (500L - 1L)

#define EXTENT_SIMILAR        (MAX_SIMILAR_VAL - BASE_SIMILAR)
#define EXTENT_EXACT_DATE     (MAX_EXACT_DATE_VAL - BASE_EXACT_DATE)
#define EXTENT_EXACT_IND      (MAX_EXACT_IND_VAL - BASE_EXACT_IND)
#define EXTENT_EXACT_SEG_NUM  (MAX_EXACT_SEG_NUM_VAL - BASE_EXACT_SEG_NUM)

// translation flag values
#define TRANSLFLAG_NORMAL      0
#define TRANSLFLAG_MACHINE     1
#define TRANSLFLAG_GLOBMEM     2
#define TRANSLFLAG_GLOBMEMSTAR 3


/*----------------------------------------------------------------------------*\
  System wide threshold values
\*----------------------------------------------------------------------------*/
#define LENGTH_THR         50L  // Length threshold value
#define MAX_LENGTH_THR    100L  // maximal Length threshold value
#define INIT_MATCH_THR     59L  // Initial match threshold value
#define SHORTER_MATCH_THR  50L  // match threshold value for shorter
                                // segments, defined thru LENGTH_SHORTER_VALUE
#define WORDS_MATCH_THR 0L      // Not used yet

//--- Used in GET_IN structure to specify kind of matches.
//--- This defines should be bitwise 'ored' with the number of required matches.
//--- GET_EXACT means if at least one exact match is found return only exact
//--- matches. GET_EXACT_AND_FUZZY means return exact and fuzzy matches.
//--- Example pGetIn->usNumMatchesReq = 3 | GET_EXACT_AND_FUZZY
//--- If only the number of required matches is passed, GET_EXACT is used.
//--- If you want to retrieve translation memory hits with exact context only,
//--- use GET_EXACT_CONTEXT.
//--- If Generic replace should be disabled, you have to or with GET_NO_GENERICREPLACE
#define GET_MOREPROP_INDIC     0x00000200
#define GET_ALL_EXACT_MATCHES  0x00000400
#define GET_RESPECTCRLF        0x00000800
#define GET_IGNORE_PATH        0x00001000
#define GET_NO_GENERICREPLACE  0x00002000
#define GET_EXACT_AND_CONTEXT  0x00004000
#define GET_EXACT_AND_FUZZY    0x00008000
#define GET_ALWAYS_WITH_TAGS   0x00010000
#define GET_IGNORE_COMMENT     0x00020000
#define GET_EXACT              0x00040000

#define EQUAL_EQUAL              100
#define TAGS_EQUAL               98
#define TAGS_UNEQUAL             95

#define NON_EXCLUSIVE       1
#define EXCLUSIVE           0

#define LANG_KEY            1L
#define FILE_KEY            2L
#define AUTHOR_KEY          3L
#define TAGTABLE_KEY        4L
//#define RESERVED_KEY        5L
#define LONGNAME_KEY        5L
#define COMPACT_KEY         6L
#define FIRST_KEY           7L
// Note: the following key is NOT used as the key of a QDAM record
//       it is only a symbolic value used in the Name-to-ID functions
#define LANGGROUP_KEY       8L


// used in GET_OUT structure
#define GET_MORE_EXACTS_AVAIL      0x8000
#define GET_ADDITIONAL_FUZZY_AVAIL 0x4000



/* The segment marker */
#define  SEGMARKER       "###"


/**********************************************************************/
/* indicatos for old or new TM                                        */
/**********************************************************************/
#define OLD_TM      0
#define NEW_TM      1

/**********************************************************************/
/* indicator for organize                                             */
/* TM_CONVERT    - convert an ols TM to an new one                    */
/* TM_ORGANIZE   - organize a new TM                                  */
/**********************************************************************/
#define TM_CONVERT   0
#define TM_ORGANIZE  1


/**********************************************************************/
/* default threshold value for get                                    */
// GQ: beginning with TP6.0.1 the real fuzziness of a proposal (including
//     inline tagging is used. A new threshold has been added to be checked
//     against the new computed fuzziness, the old value is used
//     for the triple threshold checking
/**********************************************************************/
#define TM_DEFAULT_THRESHOLD     33   // prior: 40
#define TM_FUZZINESS_THRESHOLD   10


#include "win_types.h"



typedef BYTE ABGROUP [CODEPAGE_SIZE];
typedef ABGROUP * PABGROUP;

typedef struct _TM_ADDRSS { /* addr */
   USHORT usEntryInDir,      /* >= 0, entry# of the cluster in KeyDir  */
                             /* <= 4095                                */
          usBlockNumber,     /* >  0, indicates block number in TM     */
                             /* 0  used for chaining purposes as null  */
          usDispBlockPtr;    /* size(block header) <= BLOCK_SIZE       */
                             /* indicates location within a block      */
} TM_ADDRESS, * PTM_ADDRESS ;

typedef struct _SEGMENT { /* seg */
   UCHAR   achSegMarker[SEG_MARKER_LENGTH] ;
                              /* marker to beginning of segment        */
   USHORT  usLenSegment,      /* total length of segment               */
           usDispIndustry,    /* industry codes location in bufData    */
           usLenIndustry,     /* length of indus. codes list(BYTES)    */
           usDispSource,      /* source location relative to bufData   */
           usLenSource,       /* source length in bytes                */
           usDispTarget,      /* target location relative to bufData   */
           usLenTarget,       /* target length in bytes                */
           usDispReserved,    /* Reserved area  location in bufData    */
           usLenReserved,     /* length of reserved area               */
           usDispSecKey,      /* secondary key  location in bufData    */
           usLenSecKey,       /* length of secondary key               */
           usDispContext,     /* Context area  location in bufData     */
           usLenContext;      /* length of context area                */
   SHORT_FN szFileName;       /* name of source file (short name)      */
   LONG_FN  szLongName;       /* name of source file (long name or EOS)*/
   USHORT  usSegNumber;       /* segment number in file                */
   EQF_BOOL fLogicalDel,      /* set when logically deleted segment    */
           fAscii;            /*                                       */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   TIME_L  tStamp;            /* time taken from c function            */
   BUFFERIN bufData;          /* fields with variable lengths          */
} SEGMENT, * PSEGMENT;

typedef struct _OLDSEGMENT { /* seg */
   UCHAR   achSegMarker[SEG_MARKER_LENGTH] ;
                              /* marker to beginning of segment        */
   USHORT  usLenSegment,      /* total length of segment               */
           usDispIndustry,    /* industry codes location in bufData    */
           usLenIndustry,     /* length of indus. codes list(BYTES)    */
           usDispSource,      /* source location relative to bufData   */
           usLenSource,       /* source length in bytes                */
           usDispTarget,      /* target location relative to bufData   */
           usLenTarget,       /* target length in bytes                */
           usDispReserved,    /* Reserved area  location in bufData    */
           usLenReserved,     /* length of reserved area               */
           usDispSecKey,      /* secondary key  location in bufData    */
           usLenSecKey;       /* length of secondary key               */
   SHORT_FN szFileName;       /* name of source file (short name)      */
   USHORT  usSegNumber;       /* segment number in file                */
   EQF_BOOL fLogicalDel,      /* set when logically deleted segment    */
           fAscii;            /*                                       */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   TIME_L  tStamp;            /* time taken from c function            */
   BUFFERIN bufData;          /* fields with variable lengths          */
} OLDSEGMENT, * POLDSEGMENT;

typedef struct _MATCH { /* mtch */
   TM_ADDRESS  addr;        /* address of the matching segment         */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   USHORT   usNumExactBytes;/* no. of exact bytes in the               */
                            /*     input and matched sentence          */
   TIME_L   tStamp;         /* time stamp of segment                   */
   LONG     lSimLevel,      /* <= 100 indicates similar                */
                            /* >= 200 indicate exact                   */
                            /* >= 300 indicate exact+same ind code     */
                            /* >= 400 indicate exact+same filename     */
            lLengthTest,    /* value of lengths test                   */
            lInitMatchTest, /* value of initials test                  */
            lWordsTest;     /* value of words test                     */
                     /* last 3 values are meaningless for exact matches*/
   UCHAR     szTarget[MAX_TGT_LENGTH + 1]; /* The target               */
} MATCH , * PMATCH;

/*
    More details about the match ranking method.

    The similarity level has a crude primary ranking as follows:

    400 - 499: If there is an exact match and the filename is the same.

    300 - 399: If there is an exact match, the filename is not the same,
                  but there is at least one industry code in common to
                  database segment and input segment.

    200 - 299: If there is an exact match but no filename or industry code
                  in common.

    0 - 100:   Similar match but no exact match.

    Secondary ranking within the ranges above is as follows:

    In the 400 - 499 range, the actual value is determined by proximity
        of database sentence number (within the text file from which it was
        taken), to the ordinal number of the input sentence within the input
        file.
    In the 300 - 399 range, the actual value is determined by number of equal
        industry codes vs the total number of industry codes that appear in
        both sentences.
    In the 200 - 299 range, the actual value is determined by the proximity
        of dates of the two segments.

    In the case of similar match the value is that of the init test. (See
        explanation to lInitMatchThr below.)

    The exact formulae by which the secondary ranking is evaluated is
        described both in the LLD document and in code comments.
                                                                       */

typedef struct  _PREFIX_IN {
   USHORT  usLenIn,           /* total length of input buffer          */
           idCommand ;        /* command id, previously defined.       */
// } PREFIX_IN, * PPREFIX_IN, IN, * PIN;
} PREFIX_IN, * PPREFIX_IN, * PIN;

typedef struct  _PREFIX_OUT {
   USHORT  usLenOut;           /* total length of output buffer        */
   BOOL    fDiskFull,          /* for Add /Replace / Create when number*/
   fDBfull;                    /* of blocks is a cluster exceeds       */
                               /* 2**16 - 1. Filled by FormatMore      */
   USHORT  rcTmt;              /* returned by TMT function             */

//} PREFIX_OUT, * PPREFIX_OUT, OUT, * POUT;
} PREFIX_OUT, * PPREFIX_OUT, * POUT;

typedef struct  _UPDATE_IN {
   PREFIX_IN  prefin;          /* prefix of each command               */
   SEGMENT    segIn;           /* the rest of this type is a SEGMENT   */
} UPDATE_IN, * PUPDATE_IN;

typedef UPDATE_IN  ADD_IN,  DEL_IN,  REP_IN;
typedef PUPDATE_IN PADD_IN, PDEL_IN, PREP_IN;

typedef struct  _ADD_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   TM_ADDRESS  addr;        /* address of added segment                */
} ADD_OUT, * PADD_OUT;

typedef struct  _DEL_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   TM_ADDRESS  addr;        /* address of deleted segment              */
} DEL_OUT , * PDEL_OUT ;

typedef struct  _REP_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   SERVERNAME   szServer;   /* server name                             */
   TM_ADDRESS  addrDel,     /* address to delete                       */
               addrAdd;     /* address to add                          */
} REP_OUT , * PREP_OUT;

typedef struct  _CLOSE_IN  {
   PREFIX_IN  prefin ;      /* prefix of each command                  */
} CLOSE_IN, * PCLOSE_IN;

typedef struct  _CLOSE_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
} CLOSE_OUT, * PCLOSE_OUT;

typedef struct  _EXT_IN {
   PREFIX_IN  prefin;       /* prefix of each command                  */
   TM_ADDRESS addr;         /* address of extracted segment            */
   USHORT     usConvert;    // Indicates how the output should be converted
} EXT_IN  , * PEXT_IN ;

typedef struct  _EXT_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   SERVERNAME   szServer;   /* server name                             */
   TM_ADDRESS  addr,        /* address of current segment              */
               addrNext;    /* address of next segment                 */
   SEGMENT     segOut;      /* fields with variable lengths            */
} EXT_OUT , * PEXT_OUT;

typedef struct _GET_IN {
   PREFIX_IN prefin;        /* prefix of each command                  */
   SHORT_FN  szFileName;    /* name of source file (short name)        */
   LONG_FN   szLongName;    /* name of source file (long name or EOS)  */
   USHORT    usConvert;    // Indicates how the output should be converted
   USHORT    usSegNumber,   /*                                         */
             usNumMatchesReq,/* number of matches required             */
             usDispIndustry,/* industry codes location in bufData      */
             usLenIndustry, /* length of indus. codes list(BYTES)      */
             usDispSource,  /* source location relative to bufData     */
             usLenSource;   /* source length in bytes                  */
   LONG      lLengthThr,    /* needed to filter out sentences          */
                            /* with different lengths                  */
             lInitMatchThr, /* needed to filter out sentences          */
                            /* with non matching initials              */
             lWordsMatchThr;/* needed to filter out sentences          */
                            /* with non matching words                 */
   BOOL      fAscii;        /*                                         */
   BUFFERIN  bufData;       /* fields with variable lengths            */
} GET_IN, * PGET_IN;

/**********************************************************************/
/* structure passed in case of rename a file ...                      */
/**********************************************************************/
typedef struct _RENFILE_IN
{
   PREFIX_IN    prefin;                /* prefix of each command             */
   SERVERNAME  szServer;                // server name
   CHAR    szOldFile[ MAX_EQF_PATH ];   // old file name
   CHAR    szNewFile[ MAX_EQF_PATH ];   // new file name
   CHAR         szUserId [MAX_USERID];  // userId logged on to requester
} RENFILE_IN, * PRENFILE_IN;

typedef struct _RENFILE_OUT
{
   PREFIX_OUT   prefout;                           /* prefix of each command */
} RENFILE_OUT, * PRENFILE_OUT;

/*  More details about lLengthThr and lInitMatchThr:

    lLengthThr:

    In the length test we compute a length value which measures how close
    are the lengths of the current sentence taken from the database versus
    the input sentence.
    The length value is computed by the formula:

    lLengthValue = ( EXTENT_SIMILAR * min( usLenInput, usLenCurrent ) ) /
                     max( usLenInput, usLenCurrent );

    where EXTENT_SIMILAR is the constant 100 for our purposes.

    The length is measured in number of words rather than characters.
    If LengthValue is 100 it means that both sentences have the same number of
    words. If one sentence has 10 words while the other has 5,
    lLengthValue = 100 * 5/10 which is about 50. In general LengthValue will
    be about the difference in percents. (10 and 5 differ by 50%).

    lLengthThr (the Length Threshold value) is the minimum value allowed in
    the length test. If LengthValue <= lLengthThr the length test fails.
    lLengthThread is increased in dependeny of the number of words of the
    input segment. For more detais see function LengthCorrectThresholds and
    CheckSimilar in file EQFTMRTV.C


    lMatchInitThr:

    lMatchInitThr is used to check whether the initials of words in both
    sentences is about the same. To that end we compute the value:

      lMatchValue = (EXTENT_SIMILAR * usSameInitilas) /
                                           max(usLenInput, usLenCurrent);

    where usSameInitials denotes the number of initials that are common
    to both sentences. lMatchValue is 100 if all the initials of one
    sentence are contained in the other.

    Assuming that both sentences have about the same number of words
    (since they passed the length test), the MatchValue denotes
    their similarity of initials in precents. If they have 20 words each
    and 15 initials are the same then MatchValue = 75.

    InitMatchThr is the minimal value allowed.
    An lActMatchThr is increased in dependency of the number of words in the
    input segment. For more detais see function LengthCorrectThresholds and
    CheckSimilar in file EQFTMRTV.C

    if MatchValue <= lActMatchThr the match value test fails

 */

typedef struct  _GET_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */

   USHORT usNumMatchesFound,/* number of matches found                 */
          usNumMatchesValid,/* number of matches valid                 */
          ausSortedMatches[MAX_MATCH_TAB_ENTRIES];
   MATCH  amtchBest[MAX_MATCH_TAB_ENTRIES]; /* Matches array           */
} GET_OUT, * PGET_OUT;

typedef struct  _OPEN_IN {
   PREFIX_IN  prefin;       /* prefix of each command                  */
   SERVERNAME szServer;     /* servername                              */
   CHAR       szUserId [MAX_USERID]; /* userId logged on to requester  */
   FILENAME  szTmFileName;  /* name of Tm database to open             */
   BOOL       fExclusive;   /* indicate the mode for opening the file  */
   BOOL       fOpenGetPart; /* indicate if Open was due to a get part  */
} OPEN_IN , * POPEN_IN;


typedef struct  _OPEN_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   HTM         htm;         /* handle to the TM                        */
} OPEN_OUT , * POPEN_OUT;

typedef struct _BLOCK_HEADER { /* bh */
   USHORT usBlockNum,       /* >= 1, points to previous block          */
          usPrevBlock,      /* >= 1, points to previous block          */
          usNextBlock,      /* >  1, points to next block.             */
                            /* 0 indicates no chaining                 */
          usFirstAvailByte; /* >= size of block header,                */
                            /* <= block size                           */
                            /* points to first available byte in block */
                            /* If block is full it sets to BlockSize   */
} BLOCK_HEADER , * PBLOCK_HEADER;

typedef struct _CREATE_TMH { /* ctmh */
   FILENAME  szTmFileName;  /* Tm database name                        */
   USHORT    usBlockSize;
   LANGUAGE  szSourceLang,  /* source language                         */
             szTargetLang;  /* target language                         */
   EQF_BOOL  fDbcs;         /* double byte char support 0/1            */
} CREATE_TMH;

typedef struct _CREATE_IN { /* crei */
   PREFIX_IN  prefin;       /* prefix of each command                  */
   SERVERNAME szServer;
   CREATE_TMH ctmh;         /* parameters that copied into Tm header   */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;
   USHORT  usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */
   CHAR    bufData[CREATE_BUFFER_SIZE];
   CHAR    szUserId [MAX_USERID]; /* userId logged on to requester     */
} CREATE_IN , * PCREATE_IN;

typedef OPEN_OUT CREATE_OUT, *PCREATE_OUT;

typedef CLOSE_IN   INFO_IN, * PINFO_IN;

typedef struct _INFO_OUT  { /* infi */
   PREFIX_OUT prefout;      /* prefix of each command                  */
   CREATE_TMH ctmh;         /* parameters that copied into Tm header   */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;
   USHORT  usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */
   CHAR    bufData[CREATE_BUFFER_SIZE];
} INFO_OUT , * PINFO_OUT;

// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_SERVER_DRIVES command which lists all
// disk drives on a server (which are available for the EQF TM Server program)
// The first Drive returned is the system drive (where directory \COMMPROP
// is located - this directory can only exist on the system drive, not on
// any of the secondary drives)
// ----------------------------------------------------------------------------

typedef struct _DRIVES_IN {              /* indr */
   PREFIX_IN    prefin;                  /* prefix of each command */
   SERVERNAME   szServer;                /* which server ?         */
   CHAR         szUserId [MAX_USERID];   /* userId logged on to requester */
} DRIVES_IN, * PDRIVES_IN;

typedef struct _DRIVE_INFO {      /* drin */
   CHAR         cDriveLetter;     /* drive letter of disk drive              */
   ULONG        ulFreeSpace;      /* number of bytes left on this disk drive */
} DRIVE_INFO, * PDRIVE_INFO;

typedef struct _DRIVES_OUT {                    /* outdr */
   PREFIX_OUT   prefout;                        /* prefix of each command    */
   USHORT       usValidDrives;                  /* how much drives have been */
                                                /* found on the server ?     */
   DRIVE_INFO   adrinDrives [MAX_DRIVELIST];    /* info for each valid       */
                                                /* server drive              */
} DRIVES_OUT, * PDRIVES_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_SERVER_TM_LIST command which lists all
// translation memories that have been found on the selected server; for each
// TM its full path name and its current file size is returned
// ----------------------------------------------------------------------------
typedef struct _FILE_LIST_IN {         /* intl                          !!! CHM */
   PREFIX_IN    prefin;                /* prefix of each command        */
   SERVERNAME   szServer;              /* which server ?                */
   CHAR         szUserId [MAX_USERID]; /* userId logged on to requester */
} FILE_LIST_IN, * PFILE_LIST_IN; // !!! CHM

typedef struct _FILE_INFO {                      /* tmin !!! CHM */
   CHAR         szPathFileName [MAX_EQF_PATH];   /* full filename of file    */
   ULONG        ulFileSize;                      /* filesize in bytes        */
} FILE_INFO, * PFILE_INFO; // !!! CHM


typedef struct _FILE_LIST_OUT {                    /* outtl !!! CHM */
   PREFIX_OUT   prefout;                           /* prefix of each command */
   USHORT       usValidFiles;                      /* valid entries in array */
   FILE_INFO    aflinFileList [MAX_TM_LIST_NUMBER]; /* info for avail. files */
} FILE_LIST_OUT, * PFILE_LIST_OUT; // !!! CHM


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_PART_OF_TM_FILE command that transfers
// a selected part of the physical TM file to the Troja requester; input
// structure contains file offset and number of bytes to transfer within this
// block; the returned value of bytes read indicates whether the end of file
// has been reached (ulBytesRead < ulBytesToRead OR ulBytesRead = 0); the
// maximum size to transfer at a time is GETPART_BUFFER_SIZE
// ----------------------------------------------------------------------------


typedef struct _GETPART_IN {      /* ingp */
   PREFIX_IN    prefin;           /* prefix of each command                  */
   ULONG        ulFilePos;        /* file position of first byte to transfer */
   ULONG        ulBytesToRead;    /* number of bytes to transfer             */
} GETPART_IN, * PGETPART_IN;

typedef struct _GETPART_OUT {          /* outgp */
   PREFIX_OUT   prefout;               /* prefix of each command             */
   ULONG        ulBytesRead;           /* number of bytes actually read      */
   ULONG        ulNextFilePos;         /* file position of next byte to read */
   UCHAR        aucOutBuffer [GETPART_BUFFER_SIZE]; /* output buffer        */
} GETPART_OUT, * PGETPART_OUT;

typedef struct _GETDICTPART_IN {
   PREFIX_IN    prefin;           /* prefix of each command                  */
   SERVERNAME   szServer;         /* which server ?                          */
   PVOID        pBTree ;          // ptr to dict file handles
   ULONG        ulFilePos;        /* file position of first byte to transfer */
   ULONG        ulBytesToRead;    /* number of bytes to transfer             */
} GETDICTPART_IN, * PGETDICTPART_IN;


#define MAX_DICT_DESCR   40            // max length of a dictionary description
// Structure of EQF dictionary properties
typedef struct _PROPDICTIONARY
{
   //--- common property part ---
   PROPHEAD  PropHead;                      // header of properties
   //--- general dictionary information ---
   CHAR      szDescription[MAX_DICT_DESCR]; // dictionary description
   CHAR      szSourceLang[MAX_LANG_LENGTH]; // dictionary source language
                                            // dictionary target language(s)
   CHAR      szTargetLang[MAX_TGT_LANG][MAX_LANG_LENGTH];
   EQF_BOOL  fCopyRight;                    // dictionary-is-copyrighted flag
   CHAR      szDictPath[MAX_EQF_PATH];      // fully qualified dict file name
   CHAR      szIndexPath[MAX_EQF_PATH];     // fully qualified index file name
   //--- dictionary profile ---
   USHORT    usLength;                      // number of user profile entries
   USHORT    usUserNameCount;               // number of user created dict names
   COLFONT   ColFontDictEntry;              // colour/font for dict entry
   COLFONT   ColFontEntryVal;               // colour/font for dict entry value
   PROFENTRY ProfEntry[MAX_PROF_ENTRIES];   // user profile entries
   EQF_BOOL  fProtected;                    // dictionary-is-protected flag
   ULONG     ulPassWord;
   CHAR      szServer[MAX_SERVER_NAME];     // Server Name of TM or \0 if TM is local
   CHAR      szUserid[MAX_USERID];          // LAN Userid of TM: if local '\0'
   CHAR      chRemPrimDrive;                // LAN primary drive
   USHORT    usLocation;                    // location of dictionary
   CHAR      szLongName[MAX_LONGFILESPEC];  // dictionary long (descriptive) name
   CHAR      szLongDesc[MAX_LONG_DESCRIPTION];  // dictionary long (descriptive) name
   USHORT    usVersion;                     // Version of dictionary - used for
                                            // comparison during organize if copyrighted
   //--- reserved space ---
   CHAR      chReserve[3402];               // reserve space / filler
} PROPDICTIONARY, *PPROPDICTIONARY;


// ----------------------------------------------------------------------------
// structures needed for the TMC_PUT_TM_PROPERTIES command which is used to
// copy a property file for a remote TM to the server (during TM creation
// process); the contents of the property file are read into the input buffer
// which is up to MEM_PROP_SIZE bytes long; the actual length of the
// property file has to be passed in the ulPropLength field; the field
// szTMPathFileName contains the name of the TM file on the server (including
// the drive and path) which is then used to create a filename for the property
// file (same drive and path but new extension (will be .PRP)
// ----------------------------------------------------------------------------
typedef struct _PUTPROP_IN {                   /* inpp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   ULONG        ulPropLength;                  /* length of property file       */
   UCHAR        aucInBuffer [MEM_PROP_SIZE];   /* buffer for file data          */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
} PUTPROP_IN, * PPUTPROP_IN;

// !!! CHM Start
typedef struct _PUTDICTPROP_IN {               /* inpp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   ULONG        ulPropLength;                  /* length of property file       */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
   PROPDICTIONARY DictProp;                    /* dictionary property buffer    */
} PUTDICTPROP_IN, * PPUTDICTPROP_IN;

/* define union to allow combined processing in server code */
typedef union _PUTPROPCOMBINEDIN
{
   PUTPROP_IN     TmPropIn;
   PUTDICTPROP_IN DictPropIn;
} PUTPROPCOMBINED_IN, * PPUTPROPCOMBINED_IN;
// !!! CHM End

typedef struct _PUTPROP_OUT {                      /* outpp                     */
   PREFIX_OUT   prefout;                           /* prefix of each command    */
} PUTPROP_OUT, * PPUTPROP_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_TM_PROPERTIES command which is used to
// copy a property file for a remote TM from the server to the Troja requester
// (during 'Include Remote TM' process); the contents of the property file are
// put into the output buffer which is up to MEM_PROP_SIZE bytes long; the
// actual length of the property file will be passed in the ulPropLength field;
// the field szTMPathFileName must contain the full path name of the TM file
// on the server and will be used to create the filename of the belonging
// property file which is stored on the same disk in the same path with the
// extension .PRP
// ----------------------------------------------------------------------------
typedef struct _GETPROP_IN {                   /* ingp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
} GETPROP_IN, * PGETPROP_IN;

typedef struct _GETPROP_OUT {                     /* outgp                     */
   PREFIX_OUT   prefout;                          /* prefix of each command    */
   ULONG        ulPropLength;                     /* length of property file   */
   UCHAR        aucOutBuffer [MEM_PROP_SIZE];     /* buffer for file data      */
} GETPROP_OUT, * PGETPROP_OUT;

// !!! CHM Start
typedef struct _GETDICTPROP_OUT {                 /* outgp                     */
   PREFIX_OUT   prefout;                          /* prefix of each command    */
   ULONG        ulPropLength;                     /* length of property file   */
   PROPDICTIONARY DictProp;                       /* dictionary property buffer*/
} GETDICTPROP_OUT, * PGETDICTPROP_OUT;

/* define union to allow combined processing in server code */


// ----------------------------------------------------------------------------
// structures needed for the TMC_END_ORGANIZE command which is used to end a
// running 'Organize' process; this command is sent to the original TM instead
// of the TM_CLOSE command and handles the closing, deletion and renaming of
// the original and temporary TM file; the temporary file has to be closed
// previously; the two filenames must contain the full specifications of the
// original TM file to be deleted and the temporary file to be renamed to the
// original TM filename;
// ----------------------------------------------------------------------------
typedef struct _ENDORG_IN {            /* ineo */
   PREFIX_IN    prefin;                /* prefix of each command             */
   CHAR         szOrgTM [MAX_EQF_PATH];/* full filename of original TM file  */
   CHAR         szTmpTM [MAX_EQF_PATH];/* full filename of temporary TM file */
} ENDORG_IN, * PENDORG_IN;

typedef struct _ENDORG_OUT {                       /* outeo */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} ENDORG_OUT, * PENDORG_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_DELETE_TM command which is used to physical
// delete a TM file and the belonging property file on the server;
// before this command can be submitted, the file has to opened exclusively;
// the supplied filename must contain the full drive and path information;
// ----------------------------------------------------------------------------
typedef struct _DELTM_IN {                       /* indt */
   PREFIX_IN    prefin;                          /* prefix of each command   */
   CHAR         szTMPathFileName [MAX_EQF_PATH]; /* full filename of TM file */
   CHAR         szPropPathFileName[MAX_EQF_PATH]; /* full filename of Prop file */
} DELTM_IN, * PDELTM_IN;

typedef struct _DELTM_OUT {                        /* outdt */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} DELTM_OUT, * PDELTM_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_DELETE_FILE command which is used to physical
// delete a file on the server; this command is only used during startup of the
// TM organization process to cleanup eventually existing temporary files (from
// a previous organize process that died during execution);
// the supplied filename must contain the full drive and path information;
// ----------------------------------------------------------------------------
typedef struct _DELFILE_IN {               /* indf                           */
   PREFIX_IN    prefin;                    /* prefix of each command         */
   SERVERNAME   szServer;                  /* which server ?                 */
   CHAR         szFileName [MAX_EQF_PATH]; /* full filename of file to delete*/
   CHAR         szUserId [MAX_USERID];     /* userId logged on to requester  */
} DELFILE_IN, * PDELFILE_IN;

typedef struct _DELFILE_OUT {                      /* outdf                  */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} DELFILE_OUT, * PDELFILE_OUT;

typedef struct  _CLOSEHANDLER_IN
{
   PREFIX_IN   prefin;
   USHORT      hth;
} CLOSEHANDLER_IN, * PCLOSEHANDLER_IN;

typedef struct  _EXIT_IN
{
   PREFIX_IN   prefin;
} EXIT_IN, * PEXIT_IN;

typedef struct  _EXIT_OUT
{
   PREFIX_OUT  prefout;
} EXIT_OUT, * PEXIT_OUT;

// ----------------------------------------------------------------------------
// data structure used for the TMC_QUERY_FILE_INFO which returns the file info
// structure of the specified file returned by the DosFindFirst call on the
// server
// ----------------------------------------------------------------------------

typedef struct _FILEINFO_IN {                      /* infi */
   PREFIX_IN    prefin;                            /* prefix of each command */
   CHAR         szFileName [MAX_EQF_PATH]; /* full filename of file to query */
   SERVERNAME   szServer;                          /* which server ?         */
   CHAR         szUserId [MAX_USERID]; /* userId logged on to requester !!!! CHM */
} FILEINFO_IN, * PFILEINFO_IN;

typedef struct _FILEINFO_OUT {                  /* outfi */
   PREFIX_OUT   prefout;                        /* prefix of each command    */
   FILEFINDBUF  stFile;                         /* info about file           */
} FILEINFO_OUT, * PFILEINFO_OUT;


typedef struct _TM_HEADER { /* tmh */
/* fixed part */
   UCHAR    achTmPrefix[TM_PREFIX_SIZE];
                            /* should be initiated to EQFTMT$$ 8       */
   USHORT  usTmHeaderSize,  /* actual length of tm header              */
           usDbVersion;
   LONG    ldispFirstBlock; /* disp of first block from                */
                            /* beginning of TM                         */
   CREATE_TMH ctmh;         /* parameters provided by the create       */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;

/* updated part */
   EQF_BOOL    fCorruption;
   USHORT  usNumTMBlocks,    /* number of blocks in the TM             */
           usNumFreeBlocks,  /* # of free, pre-formatted blocks        */
           usFirstAvailBlock,/* points to first available block        */
           ausKeyDirectory[KEY_DIR_SIZE],   /* key directory           */

           usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */

} TM_HEADER, * PTM_HEADER, ** PPTM_HEADER;

#define TAG_LEN             35
#define MAX_NAME            8
#define MAX_RANDOM          20
//#define MAX_VOTES           20 //     30
#define MAX_VOTES           30           // change 27.2.2000
#define ABS_VOTES           400
#define MAX_MATCHES         15
#define TOK_SIZE            4000
#define TMX_REC_SIZE        32760
#define TMX_TABLE_SIZE      512
#define MAX_INDEX_LEN       8150 //  2048




  // name table structure (TM Version 1 - 4)
//  typedef struct _TMX_VER1_TABLE
//  {
//    USHORT usAllocSize;
//    USHORT usMaxEntries;
//    TMX_TABLE_ENTRY stTmTableEntry;
//  } TMX_VER1_TABLE, * PTMX_VER1_TABLE;



/**********************************************************************/
/* Defines and structures for the long document name support          */
/**********************************************************************/

// initial size of long name buffer area
#define LONGNAMEBUFFER_SIZE 8096

// initial number of entries in long document name pointer array
// and increment for table enlargements
#define LONGNAMETABLE_ENTRIES 32




//complete entry id tm data file
typedef struct _TMX_RECORD
{
  LONG   lRecordLen;
  USHORT  usSourceRecord;
  USHORT  usFirstTargetRecord;
} TMX_RECORD, * PTMX_RECORD;

//structure of the source segment
typedef struct _TMX_SOURCE_RECORD
{
  LONG   lRecordLen;
  USHORT  usSource;
  USHORT usLangId;
  void reset(){
    memset(this, 0, sizeof(*this));
  }
} TMX_SOURCE_RECORD, * PTMX_SOURCE_RECORD;

//structure of the target segment
typedef struct _TMX_TARGET_RECORD
{
  LONG   lRecordLen = 0;
  //USHORT  usSourceTagTable;
  //USHORT  usTargetTagTable;
  USHORT  usTarget = 0;
  USHORT  usClb = 0;
  void reset(){
    memset(this, 0, sizeof(*this));
  }
} TMX_TARGET_RECORD, * PTMX_TARGET_RECORD;


//control block structure in target record
typedef struct _TMX_TARGET_CLB
{ 
  TIME_L  lTime;
  TIME_L  lUpdateTime;
  ULONG   ulSegmId;
  USHORT  usLangId;
  USHORT  usFileId;
  USHORT  usAuthorId;
  USHORT  usAddDataLen;  // new for Major_version6: Length of following context and additional info data
  BYTE    bMultiple;
  BYTE    bTranslationFlag;
} TMX_TARGET_CLB, * PTMX_TARGET_CLB;

// helper macros for working with TMX_TARGET_CLBs
#define TARGETCLBLEN( pClb ) (sizeof(TMX_TARGET_CLB) + pClb->usAddDataLen)

#define NEXTTARGETCLB( pClb ) ((PTMX_TARGET_CLB)(((PBYTE)pClb) + TARGETCLBLEN(pClb)))

#define PCONTEXTFROMCLB( pClb ) ((PSZ_W)(((PBYTE)pClb)+sizeof(TMX_TARGET_CLB)))

// max size of additional data area
// (currently 2 * MAX_SEGMENT_SIZE for context and additional info
// plus size of three identifierrs (USHORT) and two size fields (USHORT) )
#define MAX_ADD_DATA_LEN ((2 * MAX_SEGMENT_SIZE)*sizeof(WCHAR) + (5 * sizeof(USHORT)))

// identifier for additional info data in additional data area
#define ADDDATA_ADDINFO_ID 1

// identifier for context data in additional data area
#define ADDDATA_CONTEXT_ID 2

// identifier for the end of the additional data area
#define ADDDATA_ENDOFDATA_ID 9

// compute the size of the additional data for the given input
USHORT NTMComputeAddDataSize( PSZ_W pszContext, PSZ_W pszAddInfo );

// get length of specific data in the combined data area, returns length of data area
USHORT NtmGetAddDataLen( PTMX_TARGET_CLB pCLB, USHORT usDataID );

// store/combine additional data in the combined area, returns new size of combined data area or 0 in case of errors
USHORT NtmStoreAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszAddData );

// retrieve specific data from the combined data area, returns length of retrieved data (incl. string end delimiter)
USHORT NtmGetAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszBuffer, USHORT usBufSize );

// find a string in a specific data area
BOOL NtmFindInAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszSearch );

//tag table structure for both source and target
typedef struct _TMX_TAGTABLE_RECORD
{
  LONG   lRecordLen;
  USHORT  usTagTableId;
  USHORT  usFirstTagEntry;
} TMX_TAGTABLE_RECORD, * PTMX_TAGTABLE_RECORD;

typedef struct _TMX_OLD_TAGTABLE_RECORD
{
  USHORT  usRecordLen;
  USHORT  usTagTableId;
  USHORT  usFirstTagEntry;
} TMX_OLD_TAGTABLE_RECORD, * PTMX_OLD_TAGTABLE_RECORD;

//individual tag entry in tag table record
typedef struct _TMX_TAGENTRY
{
  USHORT  usOffset;
  USHORT  usTagLen;
  BYTE    bData;
} TMX_TAGENTRY, * PTMX_TAGENTRY;

// macros to access certain TM records fields
  #define RECLEN(pRec) pRec->lRecordLen


#define NTMVOTES(l)     ((BYTE)(((ULONG)(l) >> 24) & 0xFF))
#define NTMKEY(l)       ((ULONG)(l) & 0xFFFFFF)
#define NTMINDEX(b,l)   ((ULONG)(((l) & 0xFFFFFF) | (((ULONG)(b)) << 24 )))

typedef ULONG  TMX_INDEX_ENTRY, *PTMX_INDEX_ENTRY;

typedef struct _TMX_INDEX_RECORD
{
  USHORT  usRecordLen;
  TMX_INDEX_ENTRY  stIndexEntry;
} TMX_INDEX_RECORD, * PTMX_INDEX_RECORD;

typedef struct _TMX_TERM_TOKEN
{
   USHORT usOffset;
   USHORT usLength;
   USHORT usHash;
} TMX_TERM_TOKEN, *PTMX_TERM_TOKEN;

typedef struct _TMX_MATCHENTRY
{
  ULONG ulKey;
  USHORT usMaxVotes;
  USHORT usMatchVotes;
  BYTE cCount;
} TMX_MATCHENTRY, * PTMX_MATCHENTRY;

//#pragma pack(show)      
//#pragma pack(push, 8)
//#pragma pack(8)

#include<string>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>


class ImportStatusDetails;


// callback function which is used by ExtMemImportprocess to insert segments into the memory
typedef USHORT (/*APIENTRY*/ *PFN_MEMINSERTSEGMENT)( LONG lMemHandle, PMEMEXPIMPSEG pSegment );
// IDs of TMX elelements
typedef enum
{
  UNKNOWN_ELEMENT =-1,
  TMX_ELEMENT     = 1,
  PROP_ELEMENT    = 2,
  HEADER_ELEMENT  = 3,
  TU_ELEMENT      = 4,
  TUV_ELEMENT     = 5,
  BODY_ELEMENT    = 6,
  SEG_ELEMENT     = 7,
  SEGMENT_TAGS    = 8,

  BEGIN_INLINE_TAGS = 10,
  //pair tags
  BEGIN_PAIR_TAGS = 10,
  BPT_ELEMENT     = 10, 
  EPT_ELEMENT     = 11,
  G_ELEMENT       = 12,
  HI_ELEMENT      = 13,
  SUB_ELEMENT     = 14,
  BX_ELEMENT      = 15,
  EX_ELEMENT      = 16,
  END_PAIR_TAGS   = 16,

  //standalone tags
  BEGIN_STANDALONE_TAGS = 20,
  PH_ELEMENT            = 20, 
  X_ELEMENT             = 21,
  IT_ELEMENT            = 22,
  UT_ELEMENT            = 23,
  T5_N_ELEMENT          = 24,
  END_STANDALONE_TAGS   = 25,

  END_INLINE_TAGS       = 29,

  TMX_SENTENCE_ELEMENT  = 30,
  INVCHAR_ELEMENT       = 31
} ELEMENTID;

  
enum ACTIVE_SEGMENT{
  SOURCE_SEGMENT  = 0,
  TARGET_SEGMENT  = 1, 
  REQUEST_SEGMENT = 2 //from fuzzy search request
};


typedef ACTIVE_SEGMENT TagLocation;

struct TagInfo
{
  TagLocation tagLocation = SOURCE_SEGMENT;
  //bool fPairedTag;                    // is tag is paired tag (bpt, ept, )
  bool fPairedTagClosed = false;        // false for bpt/ept tag - waiting for matching ept/bpt tag 
  bool fTagAlreadyUsedInTarget = false; // we save tags only from source segment and then try to match\bind them in target

  int generated_i = 0;                 // for pair tags - generated identifier to find matching tag. the same as in original_i if it's not binded to other tag in segment
  int generated_x = 0;                 // id of tag. should match original_x, if it's not occupied by other tags
  ELEMENTID generated_tagType = UNKNOWN_ELEMENT;           // replaced tagType, could be PH_ELEMENT, BPT_ELEMENT, EPT_ELEMENT  

  int original_i = 0;        // original paired tags i
  int original_x = 0;        // original id of tag
  ELEMENTID original_tagType;  // original tagType
  //if targetTag -> matching tag from source tags
  //if sourceTag -> matching tag from target tags
  //TagInfo* matchingTag;
  //int matchingTagIndex = -1;

  bool fDeleted = false;
  
  //t5n
  std::string t5n_key;
  std::string t5n_value;
};


class TagReplacer{
  public:
  
  std::vector<TagInfo> sourceTagList;
  std::vector<TagInfo> targetTagList;
  std::vector<TagInfo> requestTagList;
  ACTIVE_SEGMENT activeSegment = SOURCE_SEGMENT;
  int      iHighestPTI  = 0;               // increments with each opening pair tags
  int      iHighestPTId = 500;             // increments with pair tag
  int      iHighestPHId = 100;             // increments with ph tag
  bool fFuzzyRequest = false;              // if re are dealing with import or fuzzy request
  bool fReplaceNumberProtectionTagsWithHashes = false;//
  bool fSkipTags = false;

  //to track id and i attributes in request and then generate new values for tags in srt and trg that is not matching
  int iHighestRequestsOriginalI  = 0;
  int iHighestRequestsOriginalId = 0;


  //idAttr==0 -> look for id in attributes
  //iAttr==0 ->i attribute not used 
  //tag should be only "ph", "bpt", or "ept"
  TagInfo GenerateReplacingTag(ELEMENTID tagType, xercesc::AttributeList* attributes, bool saveTagToTagList = true);
  std::wstring PrintTag(TagInfo& tag);
  void reset();
  std::string static LogTag(TagInfo& tag);
  std::wstring PrintReplaceTagWithKey(TagInfo& tag);

  TagReplacer(){
    sourceTagList.reserve(50);
    targetTagList.reserve(50);
    requestTagList.reserve(50);
    reset();  
  }  
};

// PROP types
typedef enum { TMLANGUAGE_PROP, TMMARKUP_PROP, TMDOCNAME_PROP, TMAUTHOR_PROP, MACHINEFLAG_PROP, SEG_PROP, TMDESCRIPTION_PROP, TMNOTE_PROP, TMNOTESTYLE_PROP,
  TRANSLATIONFLAG_PROP, TMTMMATCHTYPE_PROP, TMMTSERVICE_PROP, TMMTMETRICNAME_PROP, TMMTMETRICVALUE_PROP, TMPEEDITDISTANCECHARS_PROP, TMPEEDITDISTANCEWORDS_PROP,  
  TMMTFIELDS_PROP, TMWORDS_PROP, TMMATCHSEGID_PROP, UNKNOWN_PROP } TMXPROPID, PROPID;

// stack elements
typedef struct _TMXELEMENT
{
  ELEMENTID ID;                        // ID of element 
  BOOL      fInlineTagging;            // TRUE = we are processing inline tagging
  BOOL      fInsideTagging;            // TRUE = we are currently inside inline tagging
  TMXPROPID PropID;                    // ID of prop element (only used for props)
  CHAR      szDataType[50];            // data type of current element
  CHAR      szTMXLanguage[50];         // TMX language of element
  CHAR      szTMLanguage[50];          // TM language of element
  CHAR      szTMMarkup[50];            // TM markup of element
  LONG      lSegNum;                   // TM segment number
} TMXELEMENT, *PTMXELEMENT;

//
// class for our SAX handler
//
class TMXParseHandler : public xercesc::HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  TMXParseHandler(InclosingTagsBehaviour InclosingTagsBehaviour = InclosingTagsBehaviour::saveAll); 
  virtual ~TMXParseHandler();

  // setter functions for import info
  void SetMemInfo( PMEMEXPIMPINFO m_pMemInfo ); 
  void SetImportData(ImportStatusDetails * _pImportDetails) { pImportDetails = _pImportDetails;}
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, //LOADEDTABLE* pTable,
     PTOKENENTRY pTokBuf, int iTokBufSize ); 
  void SetSourceLanguage( char *pszSourceLang );

  // getter functions 
  void GetDescription( char *pszDescription, int iBufSize );
  void GetSourceLanguage( char *pszSourceLang, int iBufSize );
  BOOL IsHeaderDone( void );
  BOOL ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );
  std::wstring GetParsedData() const;
  std::wstring GetParsedDataWithReplacedNpTags()const;
  std::wstring GetParsedNormalizedData()const;
  int iStopImportWRc = 0;

  bool fReplaceWithTagsWithoutAttributes;
  size_t getInvalidCharacterErrorCount() const { return _invalidCharacterErrorCount; }

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, xercesc::AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);
  //void ignorableWhitespace(const XMLCh* const chars, const unsigned int length);
  //void resetDocument();


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const xercesc::SAXParseException& exc);
  void error(const xercesc::SAXParseException& exc );
  void fatalError(const xercesc::SAXParseException& exc);
  void fatalInternalError(const xercesc::SAXException& exc);
  //void resetErrors();

  
  TagReplacer tagReplacer;
  BOOL fInitialized = false;
  BOOL fCreateNormalizedStr = false;

  USHORT insertSegUsRC{0};

  void setDocumentLocator(const xercesc::Locator* const locator) override;

private:
  const  xercesc::Locator* m_locator = nullptr;
  ImportStatusDetails* pImportDetails = nullptr;
  ELEMENTID GetElementID( PSZ pszName );
  void Push( PTMXELEMENT pElement );
  void Pop( PTMXELEMENT pElement );
  BOOL GetValue( PSZ pszString, int iLen, int *piResult );
  BOOL TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage );
  USHORT RemoveRTFTags( PSZ_W pszString, BOOL fTagsInCurlyBracesOnly );

  // date conversion help functions
  BOOL IsLeapYear( const int iYear );
  int GetDaysOfMonth( const int iMonth, const int iYear );
  int GetDaysOfYear( const int iYear );
  int GetYearDay( const int iDay, const int iMonth, const int iYear );

  // mem import interface data
  PMEMEXPIMPINFO m_pMemInfo;
  PFN_MEMINSERTSEGMENT pfnInsertSegment;
  LONG  lMemHandle;

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fWithTagging;                   // TRUE = add tagging to data
  BOOL fWithTMXTags;                   // TRUE = segment contains TMX tags
  BOOL fTMXTagStarted;                 // TRUE = TMX inline tag started
  BOOL fHeaderDone;                    // TRUE = header has been processed
  BOOL fError;                         // TRUE = parsing ended with error
  
  // segment data
  ULONG ulSegNo;                       // segmet number
  LONG  lTime;                         // segment date/time
  USHORT usTranslationFlag;            // type of translation flag
  int   iNumOfTu;
  size_t _invalidCharacterErrorCount{0};
  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR_W   szReplacedNpData[DATABUFFERSIZE];
    CHAR_W   szNormalizedData[DATABUFFERSIZE];
    CHAR_W   szPropW[DATABUFFERSIZE]; // buffer for collected prop values
    CHAR     szLang[50];              // buffer for language
    CHAR     szDocument[EQF_DOCNAMELEN];// buffer for document name

    CHAR     szAuthor[EQF_DOCNAMELEN];// buffer for author name
    CHAR     szChangeId[EQF_DOCNAMELEN];// buffer for author/changeId name
    CHAR     szCreationId[EQF_DOCNAMELEN];// buffer for author/creationId name

    MEMEXPIMPSEG SegmentData;         // buffer for segment data
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    CHAR     szMemSourceIsoLang[50];
    CHAR     szErrorMessage[1024];    // buffer for error message text
    CHAR_W   szNote[MAX_SEGMENT_SIZE];// buffer for note text
    CHAR_W   szNoteStyle[100];        // buffer for note style
    CHAR_W   szMTMetrics[MAX_SEGMENT_SIZE]; // buffer for MT metrics data
    ULONG    ulWords;                       // number of words in the segment text
    CHAR_W   szMatchSegID[MAX_SEGMENT_SIZE];// buffer for match segment ID
    
    //for inclosing tags skipping. If pFirstBptTag is nullptr->segments don't start from bpt tag-> inclosing tags skipping should be ignored
    int        inclosingTagsAtTheStartOfSegment = 0;
    int        inclosingTagsAtTheEndOfSegment   = 0;
    
    InclosingTagsBehaviour inclosingTagsBehaviour = saveAll;
    
    bool        fUseMajorLanguage;
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  // TUV data area
  typedef struct _TMXTUV
  {
    CHAR_W   szText[DATABUFFERSIZE];  // buffer for TUV text
    CHAR     szLang[50];              // buffer for TUV language (converted to Tmgr language name) or original language in case of fInvalidLang
    CHAR     szIsoLang[10];
    BOOL     fInvalidChars;           // TRUE = contains invalid characters 
    BOOL     fInlineTags;             // TRUE = contains inline tagging
    BOOL     fInvalidLang;            // TRUE = the language of the TUV is invalid
  } TMXTUV, *PTMXTUV;

  PTMXTUV pTuvArray;
  int iCurTuv;                        // current TUV index
  int iTuvArraySize;                  // current size of TUV array

  // element stack
  int iStackSize;
  int iCurElement;
  TMXELEMENT CurElement;
  PTMXELEMENT pStack;

  bool            fSawErrors;
  BOOL      fInvalidChars;             // TRUE = current TUV contains invalid characters 
  BOOL      fInlineTags;               // TRUE = current TUV contains inline tagging

  // data for remove tag function
  PTOKENENTRY pTokBuf;
  int iTokBufSize;
  //ULONG ulCP;

  void fillSegmentInfo( PTMXTUV pSource, PTMXTUV pTarget, PMEMEXPIMPSEG pSegment );

};


class StringVariants{
protected:
  std::wstring original; // full original str
  std::wstring originalTarget;
  bool fSuccess = false; 

  
  xercesc::SAXParser parser;
  TMXParseHandler handler;
  xercesc::XMLPScanToken saxToken;
public:
  StringVariants(std::wstring&& w_src, std::wstring&& w_trg){
    original = std::move(w_src);
    originalTarget = std::move(w_trg);
  }
  bool isParsed() const{ return fSuccess; }
  std::wstring& getOriginalSrcStr(){ return original; }
  std::wstring& getOriginalTrgStr(){ return original; }
  wchar_t* getOriginalStrC() { return &original[0]; }
  wchar_t* getOriginalTrgStrC() {return &originalTarget[0];}
};

class StringTagVariants: public StringVariants{
protected:
  std::wstring norm; // without any tags
  std::wstring npReplaced; // with generic(generated) tags but np tags is replaced with their keys(r attr)
  std::wstring genericTags; // generated tags so there could be only ph or bpt/ept with id and r attributes that would map input str to common tag format

  std::wstring genericTarget; 
  std::wstring normTarget;

  void initParser();
  void parseSrc();
  void parseTrg();
  void logResults();
  std::string src;
  std::string trg;

public:
  StringTagVariants(std::wstring&& w_src): StringTagVariants(std::move(w_src), L""){}
  StringTagVariants(std::wstring&& w_src, std::wstring&& w_trg): StringVariants(std::move(w_src), std::move(w_trg)){
    initParser();
    parseSrc();
    if(fSuccess){
      parseTrg();
      logResults();
    }
  }

  bool hasTarget() const { return !genericTarget.empty(); }
  bool replaceTags(std::wstring&& w_request_input);

  std::wstring& getNormStr(){ return norm; }
  std::wstring& getNpReplacedStr(){ return npReplaced; }
  std::wstring& getGenericTagsString() { return genericTags; }
  std::wstring& getNormTargetStr(){return normTarget; }
  
  wchar_t* getNormStrC(){return &norm[0];}
  wchar_t* getNpReplStrC(){return &npReplaced[0];}
  wchar_t* getGenericTagStrC() { return &genericTags[0]; }
  wchar_t* getGenericTargetStrC() {return &genericTarget[0];}
  wchar_t* getNormalizedTargetStrC() { return &normTarget[0]; }
};

class RequestTagReplacer: public StringVariants{
  std::wstring replacedTagsSrc, replacedTagsTrg, genericTagsReq, originalReq;
  void parseAndReplaceTags();
  public:

  RequestTagReplacer(std::wstring&& w_src, std::wstring&& w_trg, std::wstring&& w_req): StringVariants(std::move(w_src), std::move(w_trg)){
      originalReq = std::move(w_req);
      parseAndReplaceTags();
  };

  wchar_t* getSrcWithTagsFromRequestC(){ return &replacedTagsSrc[0];}
  wchar_t* getTrgWithTagsFromRequestC(){ return &replacedTagsTrg[0];}
  wchar_t* getReqGenericTagStrC(){return &genericTagsReq[0]; }
  wchar_t* getOrigReqC(){return &originalReq[0];}

  std::wstring& getSrcWithTagsFromRequest(){ return replacedTagsSrc;}
  std::wstring& getTrgWithTagsFromRequest(){ return replacedTagsTrg;}
  std::wstring& getReqGenericTagStr(){return genericTagsReq; }
  std::wstring& getOrigReq(){return originalReq;}
};

struct TMX_SENTENCE
{
  //std::unique_ptr<StringTagVariants>  pStrings = nullptr;
  StringTagVariants* pStrings = nullptr;

  PSZ_W pAddString = nullptr;
  //USHORT  usNormLen = 0;
  PTMX_TERM_TOKEN pTermTokens = nullptr;
  LONG lTermAlloc = 0;
  PTMX_TAGTABLE_RECORD pTagRecord = nullptr;
  LONG lTagAlloc = 0;
  PTMX_TAGENTRY pTagEntryList = nullptr;
  USHORT  usActVote = 0;
  PULONG  pulVotes = nullptr;
  //std::unique_ptr<StringTagVariants>  pPropString = nullptr;
  StringTagVariants * pPropString = nullptr;

  PTMX_TERM_TOKEN pPropTermTokens = nullptr;     // buffer for Termtokens

  //ADDINFO
  CHAR      szSourceLanguage[MAX_LANG_LENGTH]; //language name of source
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_FILESPEC];          //author name of target
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  TIME_L    lTime;                             //time stamp
  CHAR_W    szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W    szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  BOOL      fMarkupChanged;                    // Markup does not exist, changed to OTMUTF8 during import
  //end ADDINFO


  TMX_SENTENCE(std::wstring&& w_src, std::wstring &&w_trg){
    if(w_trg.empty()){
      pStrings = new StringTagVariants(std::move(w_src)); //;std::make_unique<StringTagVariants> (std::move(w_src));
    }else{
      pStrings =  new StringTagVariants(std::move(w_src), std::move(w_trg)); // std::make_unique<StringTagVariants> (std::move(w_src), std::move(w_trg));
    }
    bool fOK = true;
    if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pulVotes), 0L, (LONG)(ABS_VOTES * sizeof(ULONG)), NOMSG );
    if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pTagRecord), 0L, (LONG)(2*TOK_SIZE), NOMSG);
    if ( fOK ) fOK = UtlAlloc( (PVOID *) &(pTermTokens), 0L, (LONG)TOK_SIZE, NOMSG );

    if ( fOK )
    {
      lTermAlloc = (LONG)TOK_SIZE;
      lTagAlloc = (LONG)(2*TOK_SIZE);
    } /* endif */
    if(!fOK){
      throw ;//new Exception();
    }
    //pStrings = input;
    //usNormLen = pStrings->getNormStr().length();    
  }

  TMX_SENTENCE(wchar_t* w_src): TMX_SENTENCE(std::move(w_src), L""){}

  ~TMX_SENTENCE(){
    if(pStrings){
      delete pStrings;
      pStrings = nullptr;
    }
    if(pPropString){
      delete pPropString;
      pPropString = nullptr;
    }
    //if(pPropString) pPropString.reset();
    //if(pStrings) pStrings.reset();
    UtlAlloc( (PVOID *) &pTermTokens, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pTagRecord, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pulVotes, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pAddString, 0L, 0L, NOMSG );
    UtlAlloc( (PVOID *) &pTermTokens, 0L, 0L, NOMSG ); 
  }

};
using PTMX_SENTENCE = TMX_SENTENCE *;


//#pragma pack(pop)
//#pragma pack()


//#pragma pack(pop)
//#pragma pack()

typedef struct _TMX_REPLTAGPAIR
{
  PBYTE     pSrcTok;           // token for paired toks
  PBYTE     pPropTok;
  BOOL      fUsed;             //TRUE = Tag pair has been used in replacement already
} TMX_REPLTAGPAIR, * PTMX_REPLTAGPAIR;

typedef struct _TMX_SUBSTPROP
{
  CHAR_W szSource    [ MAX_SEGMENT_SIZE ]; // source segment
  CHAR_W szPropSource[ MAX_SEGMENT_SIZE ]; // source proposal
  CHAR_W szPropTarget[ MAX_SEGMENT_SIZE ]; // translation of proposal
  char szSourceLanguage[MAX_LANG_LENGTH];// source language
  char szSourceTagTable[MAX_FNAME];      // source tagtable
  char szTargetLanguage[MAX_LANG_LENGTH];// target language
  char szPropTagTable[MAX_FNAME];        // tagtable for proposal
  PBYTE                pTokSource;       // token buffer for source tokens
  PBYTE                pTokPropSource;   // token buffer for proposal tokens
  PBYTE                pTokPropTarget;   // token buffer for proposal target ..
  //PTMX_TAGTABLE_RECORD pTagsSource;      // tag table record for source  ..
  //PTMX_TAGTABLE_RECORD pTagsPropSource;  // tag table record for prop source
  //PTMX_TAGTABLE_RECORD pTagsPropTarget;  // tag table record for prop target
  CHAR_W chBuffer[ MAX_SEGMENT_SIZE * 2 ]; // generic token buffer
  USHORT usTokenSource;                  // number of source tokens
  USHORT usTokenPropSource;              // number of prop source tokens
  USHORT usTokenPropTarget;              // number of prop target tokens
  ULONG  ulRandom[MAX_RANDOM];                   // random sequence number
  PTMX_REPLTAGPAIR pTagPairs;                    // tags to be replaced by each other
  PTMX_REPLTAGPAIR pDelTagPairs;                 // tags to be deleted in prop
} TMX_SUBSTPROP, * PTMX_SUBSTPROP;



//=======================================================================
typedef struct _TMX_PREFIX_IN
{
  USHORT usLengthInput;                //length of complete input structure
  USHORT usTmCommand;                  //TM command id
} TMX_PREFIX_IN, * PTMX_PREFIX_IN, XIN, * PXIN;

typedef struct _TMX_PUT_W
{
  CHAR_W    szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR_W    szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szSourceLanguage[MAX_LANG_LENGTH]; //language name of source
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_LANG_LENGTH];          //author name of target
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  TIME_L    lTime;                             //time stamp
  CHAR_W    szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W    szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  BOOL      fMarkupChanged;                    // Markup does not exist, changed to OTMUTF8 during import
} TMX_PUT_W, * PTMX_PUT_W;



typedef struct _TMX_GET_W
{
  CHAR_W      szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR        szTagTable[MAX_FNAME];             //tag table name of source
  CHAR        szSourceLanguage[MAX_LANG_LENGTH]; //language of source
  CHAR        szFileName[MAX_FILESPEC];          //file name the source comes from
  LONG_FN     szLongName;                        // name of source file (long name or EOS)
  ULONG       ulSegmentId;                       //segment number from analysis
  CHAR        szAuthorName[MAX_LANG_LENGTH];          //author name
  CHAR        szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  USHORT      usRequestedMatches;                //number of requested matches
  USHORT      usMatchThreshold;                  //threshold for match level
  USHORT      usConvert;                //how the output should be converted
  ULONG       ulParm;                   //for future use, xmp. GET_ONLY_MT_MATCHES, GET_ONLY_EXACT_MATCHES...)
  CHAR_W      szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W      szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  ULONG       ulSrcOemCP;
  ULONG       ulTgtOemCP;
  PVOID       pvReplacementList;                 // ptr to a SGML-DITA replacement list or NULL
  PVOID       pvGMOptList;                       // ptr to a global memory option list or NULL
  bool fSourceLangIsPrefered;
  bool fTargetLangIsPrefered;
} TMX_GET_W, * PTMX_GET_W;


//=======================================================================
// structure TMX_GET_OUT

typedef struct _TMX_MATCH_TABLE_W
{
  CHAR_W  szSource[MAX_SEGMENT_SIZE];   //source sentence with tags
  CHAR    szFileName[MAX_FILESPEC];     //file name where the source comes from.
  LONG_FN szLongName;                   // name of source file (long name or EOS)
  ULONG   ulSegmentId;                  //segment number from analysis
  CHAR_W  szTarget[MAX_SEGMENT_SIZE];   //target sentence with tags
  CHAR    szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  CHAR    szOriginalSrcLanguage[MAX_LANG_LENGTH]; //language of src of translation
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR    szTargetAuthor[OTMPROPOSAL_MAXNAMELEN];   //author name of target
  TIME_L  lTargetTime;                  //time stamp of target
  USHORT  usMatchLevel;                 //similarity of the source
  int iWords;
  int iDiffs;
  USHORT  usOverlaps;                   //temp field - nr of overlapping triples
  CHAR    szTagTable[MAX_FNAME];        //tag table name of source
  ULONG   ulKey;                        // key of match
  USHORT  usTargetNum;                  // number of target
  USHORT  usDBIndex;                    // number of memory in current hierarchy
  CHAR_W  szContext[MAX_SEGMENT_SIZE];  // segment context info
  CHAR_W  szAddInfo[MAX_SEGMENT_SIZE];  // additional segment information
  USHORT  usContextRanking;             // context ranking from user exit context processing
  USHORT  usMatchInfo;                  // for future use: special info for match
} TMX_MATCH_TABLE_W, * PTMX_MATCH_TABLE_W;

using TMX_GET_OUT_W = struct _TMX_GET_OUT_W : public TMX_EXT_OUT_W
{
  //TMX_PREFIX_OUT    stPrefixOut;               //prefix of Output buffer
  USHORT            usNumMatchesFound;         //number of matches found
  TMX_MATCH_TABLE_W stMatchTable[MAX_MATCHES]; //match structure
  USHORT fsAvailFlags;                         // additional flags (more exact/fuzzy matches avail)
};
using PTMX_GET_OUT_W = TMX_GET_OUT_W *;






//=======================================================================
/**********************************************************************/
/* Attention: for special names mode of TMExtract, the TMX_EXT_IN     */
/*            structure is used in a different way:                   */
/*            usConvert contains the ID of the names to extract:      */
/*              MEM_OUTPUT_TAGTABLES  return list of tag tables of TM */
/*              MEM_OUTPUT_AUTHORS    return list of authors of TM    */
/*              MEM_OUTPUT_DOCUMENTS  return list of documents of TM  */
/*              MEM_OUTPUT_LANGUAGES  return list of languages of TM  */
/*            usNextTarget contains 0 for the first call or the       */
/*            index returned in the usNextTarget field of the         */
/*            TMX_EXT_OUT structure for subsequent calls if more      */
/*            names are available than fit into the output buffers.   */
/*            The remining fields are unused.                         */
/**********************************************************************/
//structure TMX_EXT_IN
typedef struct _TMX_EXT_IN
{
  ULONG ulTmKey;                 //tm get to get
  USHORT usConvert;              //how the output is to appear
  USHORT usNextTarget;           //which target record to address next
} TMX_EXT_IN, * PTMX_EXT_IN;
typedef TMX_EXT_IN TMX_EXT_IN_W, *PTMX_EXT_IN_W;

//=======================================================================
/**********************************************************************/
/* Attention: for special names mode of TMExtract, the TMX_EXT_OUT    */
/*            structure is used in a different way:                   */
/*            szSource and szTarget contain a null terminated list    */
/*            of names (NULL-terminated strings followed by another   */
/*            NULL).                                                  */
/*            fMachineTranslation is TRUE if more names are available */
/*            usNextTarget gives the index of the next name if more   */
/*            names are available than fit into the output buffers    */
/*            The remaining fields are unused.                        */
/**********************************************************************/
// structure TMX_EXT_OUT

class EqfMemoryPlugin;
class Dummy{};


#include <mutex>
class EqfMemory //: public TMX_CLB
/*! \brief This class implements the standard translation memory (EQF) for OpenTM2.
*/

{
  
public:
  std::mutex tmMutex;
  bool fOpen = false;
  BTREE TmBtree;
  BTREE InBtree;
  TMX_TABLE Languages;
  TMX_TABLE FileNames;
  TMX_TABLE Authors;
  TMX_TABLE TagTables;
  USHORT usAccessMode = 0;
  USHORT usThreshold = 0;
  TMX_SIGN stTmSign;
  BYTE     bCompact[MAX_COMPACT_SIZE-1];
  BYTE     bCompactChanged = 0;
  LONG     alUpdCounter[MAX_UPD_COUNTERS];
  PTMX_LONGNAME_TABLE pLongNames = nullptr;
  TMX_TABLE LangGroups;              //  table containing language group names
  PSHORT     psLangIdToGroupTable = nullptr; // language ID to group ID table
  LONG       lLangIdToGroupTableSize = 0; // size of table (alloc size)
  LONG       lLangIdToGroupTableUsed = 0; // size of table (bytes in use)
  PVOID      pTagTable = nullptr;         // tag table loaded for segment markup (TBLOADEDTABLE)

  // copy of long name table sorted ignoring the case of the file names
  // Note: only the stTableEntry array is filled in this area, for all other
  //       information use the entries in the pLongNames structure
  PTMX_LONGNAME_TABLE pLongNamesCaseIgnore = nullptr;

  // fields for work area pointers of various subfunctions which are allocated
  // only once for performance reasons
  PVOID      pvTempMatchList = nullptr;        // matchlist of FillMatchEntry function
  PVOID      pvIndexRecord = nullptr;          // index record area of FillMatchEntry function
  PVOID      pvTmRecord = nullptr;             // buffer for memory record used by GetFuzzy and GetExact
  ULONG      ulRecBufSize = 0;                 // current size of pvTMRecord;

  // fields for time measurements and logging
  BOOL       fTimeLogging = 0;           // TRUE = Time logging is active
  LONG64     lAllocTime = 0;             // time for memory allocation
  LONG64     lTokenizeTime = 0;          // time for tokenization
  LONG64     lGetExactTime = 0;          // time for GetExact
  LONG64     lOtherTime = 0;             // time for other activities
  LONG64     lGetFuzzyTime = 0;          // time for GetFuzzy
  LONG64     lFuzzyOtherTime = 0;        // other time spent in GetFuzzy
  LONG64     lFuzzyTestTime = 0;         // FuzzyTest time spent in GetFuzzy
  LONG64     lFuzzyGetTime = 0;          // NTMGet time spent in GetFuzzy
  LONG64     lFuzzyFillMatchEntry = 0;   // FillMatchEntry time spent in GetFuzzy
  LONG64     lFillMatchAllocTime = 0;    // FillMatchEntry: allocation time
  LONG64     lFillMatchOtherTime = 0;    // FillMatchEntry: other times
  LONG64     lFillMatchReadTime = 0;     // FillMatchEntry: read index DB time
  LONG64     lFillMatchFillTime = 0;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchCleanupTime = 0;  // FillMatchEntry: cleanup match list time
  LONG64     lFillMatchFill1Time = 0;    // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill2Time = 0;    // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill3Time = 0;    // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill4Time = 0;    // FillMatchEntry: fill match list time

  //std::shared_ptr<EqfMemory> readOnlyPtr;
  //std::shared_ptr<EqfMemory> writePtr;
  std::shared_ptr<int> readOnlyCnt;
  std::shared_ptr<int> writeCnt;

/*! \brief OtmMemory related return codes

*/
  static const int ERROR_INTERNALKEY_MISSING = 1001;

/*! \brief Constructors
*/
	EqfMemory()  {
    TmBtree.AllocateMem();
    InBtree.AllocateMem();
    //readOnlyPtr = std::make_shared<EqfMemory>(*this);
    //writePtr(std::make_shared<EqfMemory>(*this));
    readOnlyCnt = std::make_shared<int>(0);
    writeCnt = std::make_shared<int>(0);
    /*
    Languages.ulMaxEntries = 0;
    Languages.stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    FileNames.ulMaxEntries = 0;
    FileNames.stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    Authors.ulMaxEntries = 0;
    Authors.stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    TagTables.ulMaxEntries = 0;
    TagTables.stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    LangGroups.ulMaxEntries = 0;
    LangGroups.stTmTableEntry.resize(NUM_OF_TMX_TABLE_ENTRIES);
    //*/

    pvGlobalMemoryOptions = NULL;
    int rc = NTMCreateLongNameTable();
    if(rc) T5LOG(T5ERROR) << "NTMCreateLongNameTable returned non-zero, but " << rc; 
  };

	EqfMemory( HTM htm, char *pszName );

/*! \brief Destructor
*/
	~EqfMemory();

  USHORT OpenX();
/*! \brief Info structure for an open translation memory
*/
  //typedef struct _OPENEDMEMORY
  //{
  //char szName[260];               // name of the memory
  time_t tLastAccessTime = 0;       // last time memory has been used
  long lHandle = 0;                 // memory handle     
  MEMORY_STATUS eStatus;          // status of the memory
  MEMORY_STATUS eImportStatus;    // status of the current/last memory import
  //std::atomic<double> dImportProcess; 
  //ushort * pusImportPersent = nullptr;
  ImportStatusDetails* importDetails = nullptr;
  std::string strError;           // pointer to an error message (only when eStatus is IMPORT_FAILED_STATUS)
 
  PPROP_NTM memoryProperties;
    /*! \brief structure for memory information */
  //  typedef struct _MEMORYINFO
  //  {
      
  //  } MEMORYINFO, *PMEMORYINFO;
//} OPENEDMEMORY ;

/*! \brief Error code definition
*/
  static const int ERROR_MEMORYOBJECTISNULL   = 8003;
  static const int ERROR_BUFFERTOOSMALL       = 8004;
  static const int ERROR_INVALIDOBJNAME       = 8005;
  static const int ERROR_MEMORYEXISTS         = 8006;
  static const int INFO_ENDREACHED            = 8008;
  static const int ERROR_ENTRYISCORRUPTED     = 8009;


  /*! \brief Flags for the update of proposals */

  static const int UPDATE_MARKUP   = 0x01;           // update markup/tag table
  static const int UPDATE_MTFLAG   = 0x02;           // update machine translation flag
  static const int UPDATE_TARGLANG = 0x04;           // update target language
  static const int UPDATE_DATE     = 0x08;           // update proposal update time

  ushort TmtXDelSegm
(
  OtmProposal& pTmDelIn,  //ptr to input struct
  TMX_EXT_OUT_W* pTmDelOut //ptr to output struct
);

USHORT TmtXDelSegmByKey
(
  OtmProposal& TmDelIn,  //ptr to input struct
  TMX_EXT_OUT_W* pTmDelOut //ptr to output struct
);

USHORT FindTargetAndDelete( PTMX_RECORD pTmRecord,
                            OtmProposal&  TmDel,
                            TMX_EXT_OUT_W * pTmExtOut,
                            PULONG pulKey );

USHORT FindTargetByKeyAndDelete(
                            PTMX_RECORD pTmRecord,
                            OtmProposal&  pTmDel,
                            PTMX_SENTENCE pSentence,
                            TMX_EXT_OUT_W * pTmExtOut,
                            PULONG pulKey );


USHORT TokenizeSource( PTMX_SENTENCE, PSZ, PSZ );

USHORT NTMGetIDFromName( PSZ, PSZ, USHORT, PUSHORT );
USHORT NTMGetIDFromNameEx
(
  PSZ         pszName,                 // input, name being looked up
  PSZ         pszLongName,             // input, long name (only for FILE_KEY)
  USHORT      usTableType,             // input, type of table to use
  PUSHORT     pusID,                   // output, ID for name being looked up
  LONG        lOptions,
  PUSHORT     pusAlternativeID         // output, alternative ID
);
USHORT NTMGetNameFromID( PUSHORT, USHORT, PSZ, PSZ );
USHORT NTMGetPointersToTable(  USHORT, PTMX_TABLE *,
                              PTMX_TABLE_ENTRY * );
USHORT NTMAddNameToTable(  PSZ, USHORT, PUSHORT );
PSZ NTMFindNameForID( 
                  PUSHORT  pusID,         //intput
                  USHORT   usTableType );

USHORT FillClb( PTMX_TARGET_CLB, OtmProposal& );
USHORT ComparePutData
(
  PTMX_RECORD *ppTmRecord,             // ptr to ptr of tm record data buffer
  PULONG      pulRecBufSize,           // current size of record buffer
  OtmProposal&  TmProposal,                  // pointer to get in data
  PULONG      pulKey                   // tm key
);
USHORT AddTmTarget(
  OtmProposal& TmProposal,       //pointer to get in data
  PTMX_RECORD *ppTmRecord,          //pointer to tm record data pointer
  PULONG pulRecBufSize,             //ptr to current size of TM record buffer
  PULONG pulKey );                  //tm key

/*! \brief Store the supplied proposal in the memory
    When the proposal aready exists it will be overwritten with the supplied data

    \param pProposal pointer to a OtmProposal object

  	\returns 0 or error code in case of errors
*/
  int putProposal
  (
    OtmProposal &Proposal
  ); 

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns handle for usage with GetNextProposal or 0 in case of errors
*/
  int getFirstProposal
  (
    OtmProposal &Proposal
  ); 

  /*! \brief Get the the first proposal from the memory and prepare sequential access
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns handle for usage with GetNextProposal or 0 in case of errors
*/
  int getFirstProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ); 

  //fuzzy search
  USHORT TmtXGet
  (
    PTMX_GET_W pTmGetIn,    //ptr to input struct
    PTMX_GET_OUT_W pTmGetOut   //ptr to output struct
  );

  // TM segment update prototypes
  USHORT TmtXUpdSeg
  (
    OtmProposal* pTmPutIn,    // ptr to put input data
    PTMX_EXT_OUT_W pTmPutOut,   //ptr to output struct
    USHORT      usFlags      // flags controlling the updated fields
  );

  USHORT TmtXExtract
  (
    PTMX_EXT_IN_W  pTmExtIn, //ptr to input struct
    TMX_EXT_OUT_W * pTmExtOut //ptr to output struct
  );

  //tm extract prototypes
  USHORT FillExtStructure( PTMX_TARGET_RECORD,
                         PTMX_TARGET_CLB,
                         PSZ_W, PLONG, PTMX_EXT_OUT_W );
/*! \brief Get the next proposal from the memory 
    \param lHandle the hande returned by GetFirstProposal
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
  	\returns 0 or error code in case of errors
*/
  int getNextProposal
  (
    OtmProposal &Proposal
  ); 

  int RewriteCompactTable();
  
/*! \brief Get the next proposal from the memory (with progress info) 
    \param lHandle the hande returned by GetFirstProposal
    \param Proposal reference to a OtmProposal object which will be filled with the proposal data
    \param piProgress pointer to buffer for progress indicator, this indicator goes from 0 up to 100
  	\returns 0 or error code in case of errors
*/
  int getNextProposal
  (
    OtmProposal &Proposal,
    int *piProgress
  ); 

    /*! \brief Get the current sequential access key (the key for the next proposal in the memory) 
    \param pszKeyBuffer pointer to the buffer to store the sequential access key
    \param iKeyBufferSize size of the key buffer in number of characters
  	\returns 0 or error code in case of errors
  */
  int getSequentialAccessKey
  (
    char *pszKeyBuffer,
    int  iKeyBufferSize
  ); 


  USHORT AddToTm( OtmProposal&, PULONG );
  USHORT UpdateTmIndex( PTMX_SENTENCE, ULONG );

  void importDone(int iRC, char *pszError );
  void reorganizeDone(int iRC, char *pszError );
    
  /*! \brief Set the current sequential access key to resume the sequential access at the given position
    \param pszKey a sequential access key previously returned by getSequentialAccessKey
  	\returns 0 or error code in case of errors
  */
  int setSequentialAccessKey
  (
    char *pszKey
  ); 

  size_t GetRAMSize()const;
  size_t GetExpectedRAMSize()const;

  USHORT ExtractRecordV6
  (
    PTMX_RECORD     pTmRecord,
    PTMX_EXT_IN_W   pTmExtIn,
    TMX_EXT_OUT_W *  pTmExtOut
  );

/*! \brief Get the the proposal having the supplied key (InternalKey from the OtmProposal)
    \param pszKey internal key of the proposal
    \param Proposal buffer for the returned proposal data
  	\returns 0 or error code in case of errors
*/
  int getProposal
  (
    char *pszKey,
    OtmProposal &Proposal
  ); 


  USHORT TmtXReplace
  (
    OtmProposal& TmProposal,  //ptr to input struct
    PTMX_EXT_OUT_W pTmPutOut //ptr to output struct
  );

USHORT NTMLoadNameTable
(
  ULONG       ulTableKey,              // key of table record
  PTMX_TABLE  pTMTable,              // ptr to table data pointer
  PULONG      pulSize                  // ptr to buffer for size of table data
);

/*! \brief Get a list of memory proposals matching the given search key

    This method uses the search data contained in the search key to find one or more
    matching proposals in the memory. At least the szSource and the szTargetLang members of the
    search key have to be filled by the caller.
    The caller provides a list of OtmProposals which will be filled with the data of the matching 
    proposals. The number of requested proposals is determined by the number
    of proposals in the list.

    \param SearchKey proposal containing search string and meta data
    \param FoundProposals refernce to vector with OtmProposal objects
    \param ulOptions options for the lookup

  	\returns 0 or error code in case of errors
*/
  int SearchProposal
  (
    OtmProposal &SearchKey,
    std::vector<OtmProposal> &FoundProposals,
    unsigned long ulOptions
  ); 

  USHORT UpdateTmRecord( OtmProposal& );
  
  USHORT UpdateTmRecordByInternalKey
  (
    OtmProposal&
  );
  
  int getNumOfMarkupNames();

/*! \brief Get markup name at position n [n = 0.. GetNumOfMarkupNames()-1]
    \param iPos position of markup table name
    \param pszBuffer pointer to a buffer for the markup table name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  int getMarkupName
  (
    int iPos,
    char *pszBuffer,
    int iSize
  );


/*! \brief Get language at position n [n = 0.. GetNumOfLanguages()-1]
    \param iPos position of language
    \param pszBuffer pointer to a buffer for the document name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to buffer
*/
  int getLanguage
  (
    int iPos,
    char *pszBuffer,
    int iSize
  );



/*! \brief Get source language of the memory
    \param pszBuffer pointer to a buffer for the source language name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getSourceLanguage
  (
    char *pszBuffer,
    int iSize
  );

/*! \brief Get the name of the memory
    \param pszBuffer pointer to a buffer for the name
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getName
  (
    char *pszBuffer,
    int iSize
  );

  /*! \brief Get the name of the memory
    \param strName reference of a string receiving the memory name
*/
  int getName
  (
    std::string &strName
  );


/*! \brief Get description of the memory
    \param pszBuffer pointer to a buffer for the description
    \param iSize size of buffer in number of characters
  	\returns number of characters copied to the buffer
*/
  int getDescription
  (
    char *pszBuffer,
    int iSize
  );

  /*! \brief Set the description of the memory
    \param pszBuffer pointer to the description text
  */
  void setDescription
  (
    char *pszBuffer
  );

/*! \brief Get plugin responsible for this memory
  	\returns pointer to memory plugin object
*/
  void *getPlugin();

  
/*! \brief Get overall file size of this memory
  	\returns size of memory in number of bytes
*/
  unsigned long getFileSize();

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
	int getLastError
  (
    std::string &strError
  ); 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
	int getLastError
  (
    char *pszError,
    int iBufSize
  ); 
  
/*! \brief Set or clear the pointer to a loaded global memory option file

    This method sets a pointer to a loaded global memory option file.
    When set the option file will be used to decide how global memory proposals will be processed.

    \param pvGlobalMemoryOptions pointer to a loaded global memory option file or NULL to clear the current option file pointer

  	\returns 0 or error code in case of errors
*/
  int setGlobalMemoryOptions
  (
    void *pvGlobalMemoryOptions
  ); 


//private:

  //HTM htm;                                       // old fashioned memory handle for this memory
  //TMX_EXT_IN_W  TmExtIn;                       // ptr to extract input struct
  //TMX_EXT_OUT_W TmExtOut;                      // ptr to extract output struct
  //TMX_PUT_W  TmPutIn;                       // ptr to TMX_PUT_W structure
  //TMX_EXT_OUT_W TmPutOut;                      // ptr to TMX_EXT_OUT_W structure
  //TMX_GET_W  TmGetIn;                       // ptr to TMX_PUT_W structure
  //TMX_GET_OUT_W TmGetOut;                      // ptr to TMX_EXT_OUT_W structure
  ULONG ulNextKey = 0;                       // next TM key for GetFirstProposal/GetNextProposal
  USHORT usNextTarget = 0;                   // next TM target for GetFirstProposal/GetNextProposal
  //char szName[MAX_LONGFILESPEC];                 // memory name
	std::string strLastError;
	int iLastError = 0;
  void *pvGlobalMemoryOptions = nullptr;                   // pointert to global memory options to be used for global memory proposals

public:
  char szPlugin[256];                          // name of the plugin controlling this memory
  char szName[256];                            // name of the memory
  char szDescription[256];                     // description of the memory
  char szFullPath[256];                        // full path to memory file(s) (if applicable only)
  char szFullDataFilePath[256];                // full path to memory file(s) (if applicable only)
  char szFullIndexFilePath[256];               // full path to memory file(s) (if applicable only)
  char szSourceLanguage[MAX_LANG_LENGTH+1];    // memory source language
  char szOwner[256];                           // ID of the memory owner
  char szDescrMemoryType[256];                 // descriptive name of the memory type
  unsigned long ulSize = 0;                        // size of the memory  
  BOOL fEnabled = 0;                               // memory-is-enabled flag  


  // functions dealing with long document tables
  USHORT NTMCreateLongNameTable();
  USHORT NTMReadLongNameTable();
  USHORT NTMWriteLongNameTable();
  USHORT NTMDestroyLongNameTable();
  USHORT NTMCreateLangGroupTable();
  USHORT NTMAddLangGroup( PSZ pszLang, USHORT sLangID );
  USHORT NTMOrganizeIndexFile();
//private:
/*! \brief Fill OtmProposal from TMX_GET_OUT_W structure
    \param ulKey key of record containing the proposal
    \param usTargetNum number of target within record 
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int SetProposalKey
(
  ULONG   ulKey,
  USHORT  usTargetNum,
  OtmProposal *pProposal
);

/*! \brief Split an internal key into record number and target number
    \param Proposal reference to the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int SplitProposalKeyIntoRecordAndTarget
(
  OtmProposal &Proposal,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
);

/*! \brief Split an internal key into record number and target number
    \param pszKey pointer to the internal key of the OtmProposal 
    \param pulKey pointer to record number buffer
    \param pusTargetNum pointer to buffer for number of target within record 
  	\returns 0 or error code in case of errors
*/
int SplitProposalKeyIntoRecordAndTarget
(
  char    *pszKey,
  ULONG   *pulKey,
  USHORT  *pusTargetNum
);


/*! \brief Fill OtmProposal from TMX_GET_OUT_W structure
    \param pExtOut pointer to the TMX_GET_OUT_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int ExtOutToOtmProposal
(
  TMX_EXT_OUT_W* pExtOut,
  OtmProposal &Proposal
);

/*! \brief Fill OtmProposal from TMX_MATCH_TABLE_W structure
    \param pMatch pointer to the TMX_MATCH_TABLE_W structure
    \param Proposal reference to the OtmProposal being filled
  	\returns 0 or error code in case of errors
*/
int MatchToOtmProposal
(
  PTMX_MATCH_TABLE_W pMatch,
  OtmProposal *pProposal
);


/*! \brief Fill TMX_GET_W structure with OtmProposal data
    \param Proposal reference to the OtmProposal containing the data
    \param pGetIn pointer to the TMX_GET_W structure
  	\returns 0 or error code in case of errors
*/
int OtmProposalToGetIn
(
  OtmProposal &Proposal,
  PTMX_GET_W pGetIn
);

int OtmProposalToPutIn
(
  OtmProposal &Proposal,
  PTMX_PUT_W pPutIn
);

/*! \brief Handle a return code from the memory functions and create 
    the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name
  	\returns original or modified error return code
*/
int handleError( int iRC, char *pszMemName, char *pszMarkup = "OTMXUXLF" );


/// newTM code 
public: 
  EqfMemory(const std::string& tmName);
  bool IsLoadedInRAM();
  int ReloadFromDisk();
  int UnloadFromRAM();
  int FlushFilebuffers();

private:   
};


OtmProposal::eProposalType FlagToProposalType( USHORT usTranslationFlag );
BYTE ProposalTypeToFlag(OtmProposal::eProposalType t);

USHORT  TmGetServerDrives( PDRIVES_IN  pDrivesIn,      // Pointer to DRIVES_IN struct
                           PDRIVES_OUT pDrivesOut,     // Pointer to DRIVES_OUT struct
                           USHORT      usMsgHandling );// Message handling parameter
USHORT  TmGetServerDrivesHwnd( PDRIVES_IN, PDRIVES_OUT, USHORT, HWND );
USHORT  TmGetServerTMs( PFILE_LIST_IN  pTmListIn,      // Pointer to FILE_LIST_IN struct !!! CHM
                        PFILE_LIST_OUT pTmListOut,     // Pointer to FILE_LIST_OUT struct !!! CHM
                        USHORT       usMsgHandling );  // Message handling parameter
USHORT  TmGetTMPart( HTM          htm,                 // Memory database handle
                     PSZ          pszMemPat,           // Full translation memory path
                     PGETPART_IN  pGetPartIn,          // Pointer to GETPART_IN struct
                     PGETPART_OUT pGetPartOut,         // Pointer to GETPART_OUT struct
                     USHORT       usMsgHandling );     // Message handling parameter
USHORT  TmSendTMProp( SERVERNAME szServer,             // Servername
                      PSZ        pszMemName,           // Translation memory name
                      USHORT     usMsgHandling );      // Message handling parameter
USHORT  TmReceiveTMProp( SERVERNAME szServer,          // Servername
                         PSZ        pszMemName,        // Translation memory name
                         USHORT     usMsgHandling );       // Message handling parameter

//use this define as usMsgHandling for special handling of greyedout TMs
#define DELETE_GREYEDOUT 2

USHORT  TmDeleteFile( SERVERNAME szServer,             // Servername
                      PSZ        pszFilePath,          // Full file path
                      USHORT     usMsgHandling );      // Message handling parameter
USHORT TmGetServerFileInfo( SERVERNAME   szServer,        //Servername
                            PSZ          pszFilePath,     //Full file path
                            PFILEFINDBUF pstFile,        // File info
                            USHORT       usMsgHandling ); // Message handling parameter
USHORT TmGetServerFileInfoHwnd( SERVERNAME, PSZ, PFILEFINDBUF, USHORT, HWND );
USHORT TmCompareLocalRemoteProperties( SERVERNAME, PSZ, USHORT );
USHORT MemRcHandling( USHORT,  PSZ, HTM *, PSZ );
USHORT MemRcHandlingHwnd( USHORT,  PSZ, HTM *, PSZ, HWND );

/**********************************************************************/
/* MemReadWriteSegment                                                */
/**********************************************************************/

/**********************************************************************/
/* NTMReadWriteSegment                                                */
/**********************************************************************/
// message handling flags for NTMReadWriteSegment :
//   TMRWS_NOMSG:    show no error message at all
//   TMRWS_ALLMSG:   show all error messages
//   TMRWS_WRITEMSG: show only error messages for write segment (for TM organize!)
#define TMRWS_NOMSG    0
#define TMRWS_ALLMSG   1
#define TMRWS_WRITEMSG 2

USHORT NTMReadWriteSegmentW( HTM,          //Handle of Input TM
                             PSZ,          //Full path to Input TM
                             HTM,          //Handle of Output TM
                             PSZ,          //Full path to Output TM
                             PTMX_EXT_IN_W,//Pointer to the EXTRACT_IN structure
                             TMX_EXT_OUT_W *, //Pointer to the EXTRACT_OUT structure
                             PTMX_PUT_W,  //Pointer to the REPLACE_OUT structure
                             PTMX_EXT_OUT_W, //Pointer to the REPLACE_OUT structure
                             ULONG *,      //Pointer to a segment counter
                             ULONG *,      //Pointer to the invalid segment counter
                             PSZ,          //pointer to TM source language
                             USHORT );     //message handling flags
USHORT NTMReadWriteSegmentHwndW( HTM,      //Handle of Input TM
                             PSZ,          //Full path to Input TM
                             HTM,          //Handle of Output TM
                             PSZ,          //Full path to Output TM
                             PTMX_EXT_IN_W,  //Pointer to the EXTRACT_IN structure
                             TMX_EXT_OUT_W *, //Pointer to the EXTRACT_OUT structure
                             PTMX_PUT_W,  //Pointer to the REPLACE_OUT structure
                             PTMX_EXT_OUT_W, //Pointer to the REPLACE_OUT structure
                             ULONG *,      //Pointer to a segment counter
                             ULONG *,      //Pointer to the invalid segment counter
                             PSZ,          //pointer to TM source language
                             USHORT,       //message handling flags
                             HWND );       //handle for error messages parent



BOOL fMemIsAvail( PSZ, PSZ, PSZ );      // memory is available and accessible
USHORT MemFillCBNames( HWND, PSZ, PSZ );
USHORT MemFillTableLB( HWND   hListBox, USHORT usBoxType, PSZ    pszLastUsed );

/**********************************************************************/
/* TmOpen                                                             */
/**********************************************************************/
USHORT TmOpen( PSZ, HTM *, USHORT, USHORT,    //(in)  location:    TM_LOCAL
                   //                   TM_REMOTE
                   //                   TM_LOCAL_REMOTE
        USHORT,    //(in)  message handling parameter
                   //      TRUE:  display error message
                   //      FALSE: display no error message
        HWND   );  //(in)  window handle for error messages

/**********************************************************************/
/* TmClose                                                            */
/**********************************************************************/
USHORT
TmClose( HTM,        //(in) TM handle returned from open
         PSZ,        //(in) full TM name x:\eqf\mem\mem.mem
         USHORT,     //(in) message handling parameter
                     //     TRUE:  display error message
                     //     FALSE: display no error message
         HWND );     //(in) window handle for error messages
/**********************************************************************/
/* TmReplace                                                          */
/**********************************************************************/
USHORT
TmReplace( HTM, PSZ, PTMX_PUT_W, PTMX_EXT_OUT_W, USHORT, HWND );

/**********************************************************************/
/* TmGet                                                              */
/**********************************************************************/
USHORT
TmGetW(EqfMemory*,               //(in)  TM handle
       PSZ,               //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_W,     //(in)  pointer to get input structure
       PTMX_GET_OUT_W,    //(out) pointer to get output structure
       USHORT );          //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message

/**********************************************************************/
/* TmExtract                                                          */
/**********************************************************************/
USHORT
TmExtractW( HTM, PSZ, PTMX_EXT_IN_W, TMX_EXT_OUT_W *, USHORT );

USHORT
TmExtractHwndW( HTM, PSZ, PTMX_EXT_IN_W, TMX_EXT_OUT_W *, USHORT, HWND );

/**********************************************************************/
/* information levels for TmInfo function call                        */
/* TM_INFO_SIGNATURE - returns the signature record of a TM           */
/**********************************************************************/
#define TMINFO_SIGNATURE 0

/**********************************************************************/
/* TmDeleteTM                                                         */
/**********************************************************************/
USHORT TmDeleteTM( PSZ, USHORT, HWND, PUSHORT );

/**********************************************************************/
/* TmDeleteSegment                                                    */
/**********************************************************************/

USHORT
TmDeleteSegmentW( HTM, PSZ, PTMX_PUT_W, PTMX_EXT_OUT_W, USHORT );



/**********************************************************************/
/* NTMConvertCRLF                                                     */
/**********************************************************************/
VOID
NTMConvertCRLF( PSZ,
                PSZ,
                USHORT );
VOID NTMConvertCRLFW( PSZ_W, PSZ_W, USHORT );


/**********************************************************************/
/* NTMOpenProperties                                                  */
/**********************************************************************/
USHORT
NTMOpenProperties( HPROP *,
                   PVOID *,
                   PSZ,
                   PSZ,
                   USHORT,
                   BOOL );




// Update specific parts of segment

// flags controlling the updated segment parts
#define  TMUPDSEG_MARKUP       0x01    // update markup/tag table
#define  TMUPDSEG_MTFLAG       0x02    // update MT flag
#define  TMUPDSEG_TARGLANG     0x04    // update target language
#define  TMUPDSEG_DATE         0x08    // update segment time


USHORT TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
);

USHORT TmUpdSegHwndW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
);



// structure for MT_TMMERGE pointer
typedef struct _MT_TMMERGE
{
  CHAR        chMemory[MAX_EQF_PATH];            // TM name to be merged into
  CHAR        chSGMLFile[MAX_EQF_PATH];          // external memory
  OBJNAME     szObjName;                         // document object name
  HWND        hwndNotify;                        // parent to be notified
  CHAR        szTargetLang[MAX_LANG_LENGTH];
  OBJNAME     szFolObjName;                      // object name of folder
} MT_TMMERGE, *PMT_TMMERGE;

BOOL MTTMMergeStart
(
  PLISTCOMMAREA pCommArea,
  PMT_TMMERGE pTMMerge
);


// return codes of function MemConvertMem

// memory converted successfully
#define MEM_CONVERTMEM_SUCCESS  0

// memory conversion not required, memory is in new format already
#define MEM_CONVERTMEM_ALREADYNEWFORMAT 7001


// do a cleanup of temporary memories
void TMCleanupTempMem
(
  PSZ         pszPrefix                                     // ptr to memory prefix
);

// create a temporary memory
USHORT TMCreateTempMem
(
  PSZ         pszPrefix,                                    // short prefix to be used for memory name (should start with a dollar sign)
  PSZ         pszMemPath,                                   // ptr to buffer for memory path
  HTM         *pHtm,                                        // ptr to buffer for memory handle
  HTM         htm,                                          // htm of similar memory
  PSZ         pszSourceLang,                                // memory source language
  HWND        hwnd                                          // window handle for error messages
);

// delete a temporary memory
void TMDeleteTempMem
(
  PSZ         pszMemPath                                    // ptr to memory path
);

BOOL TMFuzzynessEx
(
  PSZ pszMarkup,                       // markup table name
  PSZ_W pszSource,                     // original string
  PSZ_W pszMatch,                      // found match
  SHORT sLanguageId,                   // language id to be used
  PUSHORT     pusFuzzy,                 // fuzzyness
  ULONG       ulOemCP,
  PUSHORT     pusWords,                // number of words in segment
  PUSHORT     pusDiffs                 // number of diffs in segment
);


USHORT NTMMorphTokenizeW( SHORT, PSZ_W, PULONG, PVOID*, USHORT);

BOOL NTMTagSubst                     // generic inline tagging for TM
(
  PTMX_SUBSTPROP pSubstProp,
  ULONG          ulSrcOemCP,
  ULONG          ulTgtOemCP
);

BOOL NTMDocMatch( PSZ pszShort1, PSZ pszLong1, PSZ pszShort2, PSZ pszLong2 );


void NTMFreeSubstProp( PTMX_SUBSTPROP pSubstProp );

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! tm.h      Internal header file for Translation Memory functions
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/
//#ifndef _EQFTMI_H_
//#define _EQFTMI_H_
/**********************************************************************/
/* This include file requires EQFDDE.H!!!                             */
/**********************************************************************/
#ifndef EQFDDE_INCLUDED
  #include "EQFDDE.H"
#endif

// include external memory export/import interface
#include "EQFMEMIE.H"


#ifndef FIELDOFFSET
  #define FIELDOFFSET(type, field)    (LOWORD(&(((type *)0)->field)))
#endif

// file extension used for shared memory (LAN based) property files in property directory
#define LANSHARED_MEM_PROP ".SLM"


/*---------------------------------------------------------------------*\
                 #Define
\*---------------------------------------------------------------------*/

// memory export and import formats
#define MEM_SGMLFORMAT_ANSI     1
#define MEM_SGMLFORMAT_ASCII    2
#define MEM_SGMLFORMAT_UNICODE  3
#define MEM_FORMAT_TMX          4
// the same define value is used for the import as TMX (Trados) and for export as TMX (UTF-8)
#define MEM_FORMAT_TMX_UTF8     5
#define MEM_FORMAT_TMX_TRADOS   5

#define MEM_FORMAT_TMX_NOCRLF   6
#define MEM_FORMAT_TMX_UTF8_NOCRLF 7
#define MEM_FORMAT_XLIFF_MT     8


// filter for mem SGML formats (same sequence as memory export and import formats required)
//#define MEM_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML Unicode\0*.*\0\0\0"
#define MEM_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML UTF-16\0*.*\0TMX\0*.*\0TMX (Trados)\0*.*\0XLIFF (MT)\0*.*\0\0\0"

// filter for mem SGML formats (same sequence as memory export and import formats required)
//#define MEM_FORMAT_FILTERS_EXP "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML Unicode\0*.*\0\0\0"
#define MEM_FORMAT_FILTERS_EXP "SGML ANSI (*.EXP)\0*.EXP\0SGML ASCII (*.EXP)\0*.EXP\0SGML UTF-16 (*.EXP)\0*.EXP\0TMX (UTF-16) (*.TMX)\0*.TMX\0TMX (UTF-8) (*.TMX)\0*.TMX\0TMX (UTF-16) (remove CRLF) (*.TMX)\0*.TMX\0TMX (UTF-8) (remove CRLF) (*.TMX)\0*.TMX\0\0\0"

// defines for retries in case of BTREE_IN_USE conditions
#define MAX_RETRY_COUNT 30
#define MAX_WAIT_TIME   100

#define TMT_CODE_VERSION    5            // Code version
#define TM_PREFIX          "EQFTMT$$"     // TM DB identifier
#define HUNDRED           100          //
// ID to be used if a table overflow occurs
#define OVERFLOW_ID       32000
// name to be used if a table overflow occurs
#define OVERFLOW_NAME     "$$$$$$$$"

#define LAST_CLUSTER_NUMBER  4095

//--- size of the message buffer for the Error message               /*@1108A*/
//--- ERROR_MEM_SEGMENT_TOO_LARGE_CON                                /*@1108A*/
#define MSG_BUFFER 35                                                /*@1108A*/


#define MIN_SPOOL_SIZE      20480  // 20k bytes for preformatted TM blocks
#define SECTOR_SIZE          4096  // Disk sector. Fixed now, dynamic later
#define BLOCK_HEADER_SIZE       8  // size of block header
#define MAX_TM_HEADER_SIZE  47600  // max size of TM header
#define MAX_BLOCK_SIZE       4096  // Maximum block size
#define MAX_TEXT_TAB_SIZE (MAX_TAGS_TAB_ENTRIES * sizeof(TOKENENTRY))
#define MAX_WORDS_TAB_SIZE (MAX_WORDS_TAB_ENTRIES * sizeof(TOKENENTRY))

#define MAX_TM_WORK_AREA  ( MAX_TGT_LENGTH + 300 )
                               /* Work areas to convert LF into CRFL   */
                               /* and the other way round              */

// The following define should not exceed 65535 Bytes
#define MAX_TMT_GLOBALS_SIZE (sizeof(TMT_GLOBALS) +\
                             MAX_TM_HEADER_SIZE +\
                             MAX_BLOCK_SIZE+\
                             MAX_TEXT_TAB_SIZE+\
                             MAX_WORDS_TAB_SIZE)


#define MAX_WORDS_TAB_ENTRIES  200 /* Number of entries in WordsTable  */
#define MAX_TAGS_TAB_ENTRIES   100 /* number of entries in TextTable   */
#define KEY_DIR_ENTRIES_NUM   4096 /* number of entries key directory  */

#define TOP_BLOCK_NUMBER (USHORT)65535
                               /* 2**16 - 1, the max number of a block */
#define MAX_BLOCKS_NUM   (USHORT)65535
                               /* 2**16 - 1, maximal number of blocks  */
#define GROUP_NUM           8  /* number of distribution groups        */

#define  TAG_STARTING_VALUE 0  /* the lowest attribute to tags in text */
                               /* table                                */
#ifndef  TM_ADM
#define MAX_SERVER_NAME    15  /* length of server name                */
#endif

/* The two defines below should not be used. Displacement can be       */
/* computed in runtime from the structure.                             */
#define SEG_LENGTH_DISP     5  /* disp of length field in a SEGMENT    */
#define DEL_FLAG_DISP       6  /* disp of del flag from segment start  */

// data compress flag
#define   BOCU_COMPRESS  0x01


/* define for write Tm Header function                                 */
#define WF_ALL            0x0000  /* Write the whole header............*/
#define WF_CORRUPT        0x0001
#define WF_TMBLOCKS       0x0002
#define WF_FREEBLOCKS     0x0004
#define WF_FIRSTAVAIL     0x0008
#define WF_UPDATE         0x4000  /* Write all but the key directory...*/
#define WF_KEYDIR         0x8000  /* Write the key directory...........*/


/*---------------------------------------------------------------------*\
  match type definitions
\*---------------------------------------------------------------------*/
#define USE_CRLF       1
#define ALIGN_CRLF     2
#define IGNORE_CRLF    3

/* return codes for the Tmt Commands                                   */
#define OK                  NO_ERROR    /* for Tmt commands            */



/*----------------------------------------------------------------------------*\
  System wide lengthes for correction of the thresholds in dependency
  of the length of the input segment
\*----------------------------------------------------------------------------*/
//@@@                           // segments, defined thru LENGTH_SHORTER_VALUE
#define LENGTH_SHORTER_VALUE   2      // Length for shorter segment
//@@@                           // segments, defined thru LENGTH_SHORTER_VALUE
#define LENGTH_SHORT_VALUE     6      // Length for short segment
#define LENGTH_MEDIUM_VALUE   10      // Length for medium segment
#define LENGTH_LONG_VALUE     20      // Length for long segment

/*----------------------------------------------------------------------------*\
  System wide percentages for correction of the MATCH threshold
\*----------------------------------------------------------------------------*/
#define MATCH_PERCENTAGE_SHORT_VALUE    1L     // percentage for short segment
#define MATCH_PERCENTAGE_MEDIUM_VALUE  10L     // percentage for medium segment
#define MATCH_PERCENTAGE_LONG_VALUE    20L     // percentage for long segment
#define MATCH_PERCENTAGE_LONGER_VALUE  40L     // percentage for longer segment

/*----------------------------------------------------------------------------*\
  System wide percentages for correction of the LENGTH threshold
\*----------------------------------------------------------------------------*/
#define LENGTH_PERCENTAGE_SHORT_VALUE    1L     // percentage for short segment
#define LENGTH_PERCENTAGE_MEDIUM_VALUE  10L     // percentage for medium segment
#define LENGTH_PERCENTAGE_LONG_VALUE    20L     // percentage for long segment
#define LENGTH_PERCENTAGE_LONGER_VALUE  30L     // percentage for longer segment

/*---------------------------------------------------------------------*\
                            Macro Definition
\*---------------------------------------------------------------------*/
#define FPTR(type, field) ((ULONG)(FIELDOFFSET(type, field)))

/**********************************************************************/
/* macro to build index name from fully qualified data name           */
/**********************************************************************/
#define INDEXNAMEFROMMEMPATH( mem, ind ) \
  { \
  Utlstrccpy( ind, mem, DOT ); \
  if ( strcmp( strrchr( mem, DOT ), EXT_OF_SHARED_MEM ) == 0 ) \
    strcat( ind, EXT_OF_SHARED_MEMINDEX ); \
  else \
    strcat( ind, EXT_OF_TMINDEX ); \
  }


#include "EQFTAG.H"
/*---------------------------------------------------------------------*\
                 Typedefs Definition
+-----------------------------------------------------------------------+
  Variable prefixes:
  USHORT id, num, len, disp
  ULONG  ldisp
  CHAR   buf[]
\*---------------------------------------------------------------------*/


typedef struct _TM_HEADER_UPDATE { /* tmhu */
   BOOL    fCorruption ;
   USHORT  usNumTMBlocks,    /* number of blocks in the TM             */
           usNumFreeBlocks,  /* # of free, pre-formatted blocks        */
           usFirstAvailBlock;/* points to first available block        */
} TM_HEADER_UPDATE;

typedef union  _UNIONIN
{
   PREFIX_IN         in;
   ADD_IN            ain;
   DEL_IN            din;
   REP_IN            rin;
   EXT_IN            ein;
   GET_IN            gin;
   CLOSE_IN          clin;
   OPEN_IN           oin;
   CREATE_IN         crin;
   INFO_IN           infoin;
   CLOSEHANDLER_IN   chin;
   EXIT_IN           xin;
   DRIVES_IN         drivesin;
   FILE_LIST_IN      filelistin; // !!! CHM
   GETPART_IN        getpartin;
   PUTPROP_IN        putpropin;
   PUTDICTPROP_IN    PutDictPropIn; // !!! CHM added
   PUTPROPCOMBINED_IN PutPropCombinedIn; // !!! CHM added
   GETPROP_IN        getpropin;
   ENDORG_IN         endorgin;
   DELTM_IN          deltmin;
   DELFILE_IN        delfilein;
   FILEINFO_IN       fileinfoin;
   RENFILE_IN        renfilein;
} UNIONIN, * PUNIONIN; /* uin */

typedef union  _UNIONOUT
{
   PREFIX_OUT  out;
   ADD_OUT     aout;
   DEL_OUT     dout;
   REP_OUT     rout;
   EXT_OUT     eout;
   GET_OUT     gout;
   CLOSE_OUT   clout;
   OPEN_OUT    oout;
   CREATE_OUT  crout;
   INFO_OUT    infoout;
   EXIT_OUT    xout;
   DRIVES_OUT    drivesout;
   FILE_LIST_OUT filelistout; // !!! CHM
   GETPART_OUT   getpartout;
   PUTPROP_OUT   putpropout;
   GETPROP_OUT   getpropout;
   ENDORG_OUT    endorgout;
   DELTM_OUT     deltmout;
   DELFILE_OUT   delfileout;
   FILEINFO_OUT  fileinfoout;
   RENFILE_OUT   renfileout;
} UNIONOUT, * PUNIONOUT; /* uout */

/*
 +---------------------------------------------------------------------------+
    Name:         fTmComInit
    Purpose:      Initialise the TM Communication code (U code)
    Parameters:   1.VOID   - nothing
    Returns:      BOOL - TRUE -> initialization completed successfully
                         FALSE -> initialization ended with an error
    Comments:     This procedure initalizes the TM Communication code (U
                  code). At the moment this only resets the top pointer of
                  the TM Handle information list.
    Samples:      fRc = fTmComInit ();
 +---------------------------------------------------------------------------+
*/
BOOL fTmComInit (VOID);



typedef struct _TMT_GLOBALS { /* tmtg */
 /* this structure keeps data and pointers to variables that exist from*/
 /* the point that the TM was created or opened, until close command   */
 /* BlockImage, TextTable, WordsTable and TmHeader are allocated       */
 /* dynamically during Create/Open  and are freed at Close time.       */
  HFILE    hfTM;                  /* handle to TM db (from Dos call)   */
  PUCHAR   pchBlockImage;         /* pointer to block image            */
  PTOKENENTRY pteTextTable,       /* pointer to TextTable              */
              pteWordsTable,      /* pointer to WordsTable             */
              pteFirstSigWord,    /* pointer to first significant word */
              pteSecondSigWord;   /* pointer to second significant word*/
  PPREFIX_OUT pPrefixOut;         /* pointer to the current PrefixOut  */
                                  /* to be used by FormatMore          */
  SEGMENT     seg;                /* placed here instead of allocating */
                                  /* this space on stack (point 276)   */
  MATCH       mtch;               /* placed here instead of allocating */
                                  /* this space on stack (point 276)   */
  PCHAR       pWorkArea1;         /* Pointer to work area 1            */
  PCHAR       pWorkArea2;         /* Pointer to work area 2            */
  PTM_HEADER  pTmHeader;          /* pointer to Tm header structure    */
  LONG        lActLengthThr;      /* length corrected length threshold values*/
  LONG        lActMatchThr;       /* length corrected match threshold values */
  PLOADEDTABLE pLoadedTagTable;   /* pointer to loaded tagtable        */
  PVOID        pstLoadedTagTable; /* pointer to loaded tagtable, used for */
                                  /* function TATagTokenize           */
} TMT_GLOBALS, * PTMT_GLOBALS, ** PPTMT_GLOBALS;

// ***************** Work Constants global **************************
#define  INIT_NUMB_OF_ENTRIES     5   // Initial number of entries in a table or List
#define  SEG_NUMB                 6   // Segment number in char.
#define  ASCII_FLAG               1   // Ascii flag converted to char.
#define  MACH_TRANS               1   // Machine trans.flag converted to char.
#define  TIME_STAMP              16   // Time integer converted to character
#define  IND_CODES               20   // External industry codes space to hold a maximum of 6 codes
#define  ASCII_IND_CODES_LENGTH   3   // Length of Industry codes in external format
#define  NUMB_OF_IND_CODES        6   // Number of Industry codes in external format
#define  MEM_NAME                 8   // Length of memory database name without extension
#define  DRIVE_NAME              12   // Special drive name
#define  PTR_MEMHANDLER_IDA       0   // Relative position in the extra bytes for handler
#define  PTR_MEM_IDA              0   // Relative position in the extra bytes for instance
#define  PTR_CRT_IDA              0   // Relative position in the extra bytes for create dialog
#define  PTR_DLG_IDA              0   // Relative position in the extra bytes for dialogs
#define  TEMP                    40   // Size of temporaty work area
                                      // of WinRegisterClass call.
#define  TEXT_100_CHAR               100  // string length for temporary strings
//#define  NUMB_OF_TOKENS             1000  // Number of tokens in the token list
#define  NUMB_OF_TOKENS             8000  // Number of tokens in the toklist:RJ increase nec.
                                          // if 65520 bytes are read-in at once during MemImport
#define  MEM_PROP_SIZE              2048  // Global size of all memory database properties
#define  MEM_BLOCK_SIZE             1024  // Translation Memory block size
#define  SEGMENT_CLUSTER_LENGTH        4  // Length of segment cluster
#define  SEGMENT_NUMBER_LENGTH         6  // Length of segment number
#define  PROCESS_NUMB_OF_MESSAGES     10  // Number of messages to be process till next message is issued
#define  MEM_DBCS                      0  // 0 = No support,    1 = DBCS support
#define  MEM_LOAD_PATTERN_NAME        "*" // Default names to be shown for load dialog
#define  MEM_LOAD_PATTERN_EXT        ".*" // Default extension to be shown for load dialog
#define  EQF_IDENTIFICATION         "EQF" // EQF identification
#define  CLBCOL_TITLE_STRING        "                   " // Placeholder for of CLBCOL titles


// *****************  Memory database load **************************
//#define  MEM_CONTEXT_TOKEN_END     0     // </Context>
//#define  MEM_CONTROL_TOKEN_END     1     // </Control> UNUSED!
//#define  NTM_DESCRIPTION_TOKEN_END 2     // </Description>
//#define  MEM_MEMORYDB_TOKEN_END    3     // </MemoryDb>
//#define  NTM_MEMORYDB_TOKEN_END    4     // </NTMMemoryDb>
//#define  MEM_SEGMENT_TOKEN_END     5     // </Segment>
//#define  MEM_SOURCE_TOKEN_END      6     // </Source>
//#define  MEM_TARGET_TOKEN_END      7     // </Target>
//#define  MEM_CONTEXT_TOKEN         8     // <Context>
//#define  MEM_CONTROL_TOKEN         9     // <Control>
//#define  NTM_DESCRIPTION_TOKEN    10     // <Description>
//#define  MEM_MEMORYDB_TOKEN       11     // <MemoryDb>
//#define  NTM_MEMORYDB_TOKEN       12     // <NTMMemoryDb>
//#define  MEM_SEGMENT_TOKEN        13     // <Segment>
//#define  MEM_SOURCE_TOKEN         14     // <Source>
//#define  MEM_TARGET_TOKEN         15     // <Target>

// strings to create the token IDs for the tags dynamically using TATagTokenize
#define  MEM_CONTEXT_TOKEN_END     L"</Context>"
#define  MEM_CONTROL_TOKEN_END     L"</Control>"
#define  NTM_DESCRIPTION_TOKEN_END L"</Description>"
#define  MEM_MEMORYDB_TOKEN_END    L"</MemoryDb>"
#define  NTM_MEMORYDB_TOKEN_END    L"</NTMMemoryDb>"
#define  MEM_SEGMENT_TOKEN_END     L"</Segment>"
#define  MEM_SOURCE_TOKEN_END      L"</Source>"
#define  MEM_TARGET_TOKEN_END      L"</Target>"
#define  MEM_CONTEXT_TOKEN         L"<Context>"
#define  MEM_CONTROL_TOKEN         L"<Control>"
#define  NTM_DESCRIPTION_TOKEN     L"<Description>"
#define  MEM_MEMORYDB_TOKEN        L"<MemoryDb>"
#define  NTM_MEMORYDB_TOKEN        L"<NTMMemoryDb>"
#define  MEM_SEGMENT_TOKEN         L"<Segment>"
#define  MEM_SOURCE_TOKEN          L"<Source>"
#define  MEM_TARGET_TOKEN          L"<Target>"
#define  MEM_CODEPAGE_TOKEN        L"<CodePage>"
#define  MEM_CODEPAGE_TOKEN_END    L"</CodePage>"
#define  MEM_ADDDATA_TOKEN_END     L"</AddData>"
#define  MEM_ADDDATA_TOKEN         L"<AddData>"




#define  TM_IMPORT_OK              0     // Translation memory import OK
#define  TM_IMPORT_FAILED          1     // Translation memory import failed
#define  TM_IMPORT_FORCED          2     // Translation memory import was forced


//used for input from create folder dialog
typedef enum
{
  MEM_CREATE_NAME_IND,
  MEM_CREATE_MARKUP_IND,
  MEM_CREATE_SOURCELANG_IND,
  MEM_CREATE_TARGETLANG_IND
} MEM_CREATEINDEX;

#define  MEM_TEXT_BUFFER         65520     // Length of the text buffer


// ************ Definitions for TM list box display *********************
typedef enum
{
  MEM_OBJECT_IND,
  MEM_NAME_IND,
  MEM_DESCRIPTION_IND,
  MEM_SIZE_IND,
  MEM_DRIVE_IND,
  MEM_SERVER_IND,
  MEM_OWNER_IND,
  MEM_SOURCELNG_IND,
  MEM_TARGETLNG_IND,
  MEM_MARKUP_IND
} MEM_ITEMINDX;

/**********************************************************************/
/* string IDs to parse the CONTROL string of the TM exported format   */
/* using UtlParseX15                                                  */
/**********************************************************************/
//#define NTM_SEGNR_ID          0   //seg nr from analysis
//#define NTM_MTFLAG_ID         1   //machine translation flag
//#define NTM_TIME_ID           2   //time stamp
//#define NTM_SOURCELNG_ID      3   //source language
//#define NTM_TARGETLNG_ID      4   //target language
//#define NTM_AUTHORNAME_ID     5   //author name
//#define NTM_TAGTABLENAME_ID   6   //tag table name
//#define NTM_FILENAME_ID       7        //file name

typedef enum
{
  NTM_SEGNR_ID,
  NTM_MTFLAG_ID,
  NTM_TIME_ID,
  NTM_SOURCELANG_ID,
  NTM_TARGETLANG_ID,
  NTM_AUTHORNAME_ID,
  NTM_TAGTABLENAME_ID,
  NTM_FILENAME_ID,
  NTM_LONGNAME_ID
} NTM_IMPORTINDEX;

#define START_KEY             0xFFFFFF

#define EXT_OF_RENAMED_TMDATA  ".TRD"
#define EXT_OF_RENAMED_TMINDEX ".TRI"
#define EXT_OF_TEMP_TMPROP     ".TMR"
#define EXT_OF_RENAMED_TMPROP  ".RMR"
#define EXT_OF_TEMP_TMDATA     ".TTD"
#define EXT_OF_TEMP_TMINDEX    ".TTI"
#define EXT_OF_TMPROP          EXT_OF_MEM

#include <vector>
class ProposalFilter;

// ************ Type definitions ****************************************

typedef struct _SEG_CTRL_DATA
{
 CHAR     chSegmentNumber[SEG_NUMB];  /* Segment number converted to character*/
 CHAR     chAsciiFlag[ASCII_FLAG];    /* Ascii flag converted to char.        */
 CHAR     chMachineTrans[MACH_TRANS]; /* Machine trans.flag converted to char.*/
 CHAR     chTimeStamp[TIME_STAMP];    /* Time integer converted to character  */
 CHAR     chIndustryCodes[IND_CODES]; /* Industry codes                       */
 CHAR     chFileName[MAX_FILESPEC-1]; /* Segment origin. from that file name  */
}SEG_CTRL_DATA, * PSEG_CTRL_DATA;




//#ifdef _OTMMEMORY_H_
typedef struct _MEM_LOAD_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];    // Memory database name
 CHAR         szShortMemName[MAX_FILESPEC];   // Memory database name
 CHAR         szMemPath[2048];                // Full memory database name + path
 CHAR         szFilePath[2048];               // Full file name + path
 BOOL         fControlFound;                  // Indicator whether a CONTROL token was found
 std::shared_ptr<EqfMemory> mem;                          // pointer to memory object
 OtmProposal  *pProposal;                     // buffer for memory proposal data
 CHAR_W       szSegBuffer[MAX_SEGMENT_SIZE+1];// buffer for segment data
 CHAR_W       szSource[MAX_SEGMENT_SIZE+1];   // buffer for segment source data
 HFILE        hFile;                          // Handle of file to be loaded
 HWND         hProgressWindow;                // Handle of progress indicator window
 ULONG        ulProgressPos;                  // position of progress indicator
 ULONG        ulBytesRead;                    // bytes already read from the import file
 ULONG        ulTotalSize;                    // total size of import file
 BOOL         fFirstRead;                     // Read Flag
 PLOADEDTABLE pFormatTable;                   // Pointer to Format Table
 PTOKENENTRY  pTokenList;                     // Pointer to Token List
 BOOL         fEOF;                           // Indicates end of file
 PTOKENENTRY  pTokenEntry;                    // A pointer to token entries
 PTOKENENTRY  pTokenEntryWork;                // A work pointer to token entries
 //ImportStatusDetails* pImportDetails = nullptr;
 //ULONG        ulSegmentCounter;               // Segment counter of progress window
 //ULONG        ulInvSegmentCounter;            // Invalid Segment counter
 //ULONG        ulProgress;
 //ULONG        ulInvSymbolsErrors;
 ULONG        ulResetSegmentCounter;          // Segments using generic markup when not valid
 CHAR         szSegmentID[SEGMENT_CLUSTER_LENGTH +
                          SEGMENT_NUMBER_LENGTH + 2]; // Segment identification
 CHAR         szFullFtabPath[MAX_EQF_PATH];   // Full path to tag tables
 CHAR_W       szTextBuffer[MEM_TEXT_BUFFER];  // Text buffer to keep the file
 PMEM_IDA     pIDA;                           // Address to the memory database IDA
 BOOL         fDisplayNotReplacedMessage;     // message flag if segment not /*@47A*/
                                              // not replaced message should /*@47A*/
                                              // be displayed or not         /*@47A*/
 CHAR         szTargetLang[MAX_LANG_LENGTH];
 CHAR         szTagTable[MAX_FNAME];
 BOOL         fTagLangDlgCanceled;
 USHORT       usTmVersion;
 BOOL         fBatch;                         // TRUE = we are in batch mode
 HWND         hwndErrMsg;                     // parent handle for error messages
 PDDEMEMIMP   pDDEMemImp;                     // ptr to batch memory import data
 HWND         hwndNotify;                 // send notification about completion
 OBJNAME      szObjName;                  // name of object to be finished
 CHAR         szDescription[MAX_MEM_DESCRIPTION];   //description of memory database
 CHAR         szTempPath[MAX_PATH144];        // buffer for path names
 USHORT       usImpMode;                  // mode for import
 BOOL         fMerge;                     // TRUE = TM is being merged
 PSZ          pszList;                    // list of imported files or NULL
 PSZ          pszActFile;                 // ptr to active file or NULL
 ULONG        ulFilled;                   // filled during read (in num CHAR_Ws)
 BOOL         fSkipInvalidSegments;       // TRUE = skip invalid segments w/o notice
 ULONG        ulOemCP;                    // CP of system preferences lang.
 ULONG        ulAnsiCP;                   // CP of system preferences lang.
 BOOL         fIgnoreEqualSegments;       // TRUE = ignore segments with identical source and target string
 BOOL         fAdjustTrailingWhitespace;  // TRUE = adjust trailing whitespace of target string

 // dynamically evaluated token IDs
 SHORT sContextEndTokenID;
 SHORT sControlEndTokenID;
 SHORT sDescriptionEndTokenID;
 SHORT sMemMemoryDBEndTokenID;
 SHORT sNTMMemoryDBEndTokenID;
 SHORT sSegmentEndTokenID;
 SHORT sSourceEndTokenID;
 SHORT sTargetEndTokenID;
 SHORT sContextTokenID;
 SHORT sControlTokenID;
 SHORT sDescriptionTokenID;
 SHORT sMemMemoryDBTokenID;
 SHORT sNTMMemoryDBTokenID;
 SHORT sSegmentTokenID;
 SHORT sSourceTokenID;
 SHORT sTargetTokenID;
 SHORT sCodePageTokenID;
 SHORT sCodePageEndTokenID;
 SHORT sAddInfoEndTokenID;
 SHORT sAddInfoTokenID;

 // fields for external memory import methods
 //HMODULE       hmodMemExtImport;                 // handle of external import module/DLL
 PMEMEXPIMPINFO pstMemInfo;                        // buffer for memory information
 PMEMEXPIMPSEG  pstSegment;                        // buffer for segment data

 LONG          lExternalImportHandle;            // handle of external memory import functions

 BOOL         fMTReceiveCounting;         // TRUE = count received words and write counts to folder properties
 OBJNAME      szFolObjName;               // object name of folder (only set with fMTReceiveCounting = TRUE)
 ULONG        ulMTReceivedWords[3];       // received words per category
 ULONG        ulMTReceivedSegs[3];        // received segments per category
 LONG         lProgress;                  // progress indicator of external import methods
 BOOL         fYesToAll;                  // yes-to-all flag for merge confirmation
 BOOL          fForceNewMatchID;          // create a new match segment ID even when the match already has one
 BOOL          fCreateMatchID;            // create match segment IDs
 CHAR          szMatchIDPrefix[256];      // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];     // prefix to be used for the match segment ID (UTF16 version)
 CHAR_W        szMatchSegIdInfo[512];     // buffer for the complete match segment ID entry
 ULONG         ulSequenceNumber;          // segment sequence number within current external memory file
 //ULONG         ulActiveSegment;
}MEM_LOAD_IDA, * PMEM_LOAD_IDA;


/*! \brief search a string in a proposal
  \param pProposal pointer to the proposal 
  \param pszSearch pointer to the search string (when fIngoreCase is being used, this strign has to be in uppercase)
  \param lSearchOptions combination of search options
  \returns TRUE if the proposal contains the searched string otherwise FALSE is returned
*/
BOOL searchInProposal
( 
  OtmProposal *pProposal,
  PSZ_W pszSearch,
  LONG lSearchOptions
);


/*! \brief search a string in a proposal
  \param pProposal pointer to the proposal 
  \param pszSearch pointer to the search string (when fIngoreCase is being used, this strign has to be in uppercase)
  \param lSearchOptions combination of search options
  \returns TRUE if the proposal contains the searched string otherwise FALSE is returned
*/
BOOL searchExtendedInProposal
( 
  OtmProposal *pProposal,
  std::vector<ProposalFilter>& filters,
  LONG lSearchOptions
);




/*! \brief check if search string matches current data
  \param pData pointer to current position in data area
  \param pSearch pointer to search string
  \returns 0 if search string matches data
*/
SHORT compareString
(
  PSZ_W   pData,
  PSZ_W   pSearch
);

/*! \brief find the given string in the provided data
  \param pszData pointer to the data being searched
  \param pszSearch pointer to the search string
  \returns TRUE if the data contains the searched string otherwise FALSE is returned
*/
BOOL findString
( 
  PSZ_W pszData,
  PSZ_W pszSearch
);

typedef struct _MEM_MERGE_IDA
{
 CHAR          szPathMergeMem[MAX_LONGPATH];  // Full path and name of TM to be merged
 CHAR          szPathMergeProp[MAX_LONGPATH]; // Full path and name of Properties to be imported
 CHAR          szDriveMergeMem[MAX_DRIVE];    // Drive letter of TM to be merged w o : ( d )
 CHAR          szNameMergeMem[MAX_LONGFILESPEC];// Name of TM to be merged w o ext ( heller )
 CHAR          szShortNameMergeMem[MAX_FILESPEC];// Short name of TM being merged
 CHAR          szDirMergeMem[MAX_LONGPATH];   // Path of directories fo TM to be merged ( \eqf\import\ )
 CHAR          szExtMergeMem[MAX_FEXT];       // Ext of TM to be merged ( MIP or MEM or ? )
 EqfMemory     *pMergeMem;                     // TM handle of TM to be merged
 CHAR          szInvokingHandler[40];         // ??? 40 ???? to be in Gerds stuff. Name of handler which invoked the process
 CHAR          szSystemPath[MAX_EQF_PATH];    // Path to the EQF system
 CHAR          szTemp[MAX_EQF_PATH];          // Temporary path area
 CHAR          szPropName[MAX_FILESPEC];      // TM property name ( xxxx.MEM )
 CHAR          szPathMem[MAX_LONGPATH];       // Full name and Path to the TM ( c:\EQF\MEM\xxxx.MEM )
 CHAR          szPathProp[MAX_LONGPATH];      // Full name and Path to the Properties
 CHAR          szDriveMem[MAX_DRIVE];         // Drive letter of TM w o : ( h )
 CHAR          szNameMem[MAX_LONGFILESPEC];   // Long name of TM w o ext ( gallus )
 CHAR          szShortNameMem[MAX_FILESPEC];  // Short name of TM w o ext ( gallus )
 EqfMemory     *pOutputMem;                   // TM handle
 BOOL          fPropExist;                    // Existence of properties 0=No 1=Yes
 BOOL          fPropCreated;                  // Tm properties have been created
 BOOL          fMsg;                          // A message has been issued already
 BOOL          fImport;                       // If set the merge is started via an import
 USHORT        usImportRc;                    // Import function return code
 ULONG         ulSegmentCounter;              // Number of segments merged
 ULONG         ulInvSegmentCounter;           // Invalid Segment counter
 EXT_IN        stOldExtIn;                    // input for TmOldExtract
 EXT_OUT       stOldExtOut;                   // output for TmOldExtract
 TMX_EXT_IN_W  stExtIn;                       // TMX_EXTRACT_IN structure
 TMX_EXT_OUT_W stExtOut;                      // TMX_EXTRACT_OUT structure
 TMX_PUT_W  stPutIn;                       // TMX_PUT_IN structure
 TMX_EXT_OUT_W stPutOut;                      // TMX_PUT_OUT structure
 TIME_L        tStamp;                        // Time stamp of merge start time
 HWND          hwndMemLb;                     // Handle to the TM listbox
 HWND          hProgressWindow;               // Handle of progress indicator window
 ULONG         ulProgressPos;                 // position of progress indicator
 CHAR          szServer[MAX_LONGPATH];        // Server Name of TM or \0 if TM is local
 BOOL          fPropReceived;                 // Property file got via Receive ?
 BOOL          fOrganizeInvoked;              // TRUE if Organize is invoked
 BOOL          fImportAnyway;                 // FALSE if import of TM with
                                              // different IDs is canceled
 CHAR          szSourceMemSourceLang[MAX_LANG_LENGTH];
 CHAR          szSourceMemTargetLang[MAX_LANG_LENGTH];
 CHAR          szSourceMemMarkup[MAX_FNAME];                         /*@1276A*/
 CHAR          szTargetMemSourceLang[MAX_LANG_LENGTH];
 CHAR          szTargetMemTargetLang[MAX_LANG_LENGTH];
 CHAR          szTargetMemMarkup[MAX_FNAME];                         /*@1276A*/
 BOOL          fDisplayMsg;             // message flag if segment not /*@47A*/
                                        // not replaced message should /*@47A*/
                                        // be displayed or not         /*@47A*/
 BOOL          fOldPropFile;                  // used for folder import
 CHAR          szTagTable[MAX_FILESPEC];      // tag table of org TM
 CHAR          szPathMergeIndex[MAX_LONGPATH];// Full path and name of index to be merged
 CHAR          szPathIndex[MAX_LONGPATH]; // name and path to index C:\EQF\MEM\xxxx.TMI )
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMEXP   pDDEMemExp;                 // ptr to batch memory export data
 CHAR         szLongName[MAX_LONGFILESPEC]; // buffer for long TM names
 USHORT       usTask;                      // current taskl to do (batch mode)
 OtmProposal  *pProposal;                   // buffer for proposal data
 BOOL         fFirstExtract;                // TRUE = this ist the first extract call
 int          iComplete;                    // process completion rate
}MEM_MERGE_IDA, * PMEM_MERGE_IDA;

/**********************************************************************/
/* Dialog IDA for TM property dialog                                  */
/**********************************************************************/
typedef struct _MEM_PROP_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// Memory database name without extension
 EqfMemory    *pMem;                       // Handle of memory database
 //HPROP        hPropMem;                   // Memory database property handle
 //PPROP_NTM    pPropMem;                   // pointer to TM properties
 CHAR         szPropName[MAX_FILESPEC];   // buffer for property name
 CHAR         szPropPath[MAX_EQF_PATH];   // buffer for property path
 CHAR         szTempPath[MAX_EQF_PATH];   // buffer for path names
}MEM_PROP_IDA, * PMEM_PROP_IDA;


BOOL EqfMemPropsToHtml( HWND hwndParent, PMEM_IDA  pIDA, EqfMemory *pMem );

VOID    EQFMemImportTrojaEnd( PMEM_MERGE_IDA );
USHORT  CloseMergeTmAndTm( PMEM_MERGE_IDA, BOOL );                   /*@1139A*/
USHORT MemFuncMergeTM
(
  PMEM_MERGE_IDA    pMIDA           // Pointer to the merge IDA
);

//#endif

typedef struct _MEMORY_HANDLER_DATA
{
 IDA_HEAD  stIdaHead;                    // Standard Ida head
}MEMORY_HANDLER_DATA, * PMEMORY_HANDLER_DATA;


/**********************************************************************/
/* TMX_ENDORG_IN, TMX_ENDORG_OUT used by NTMCloseOrganize             */
/* Structures needed for the TMC_END_ORGANIZE command which is used   */
/* to end an 'Organize' process.                                      */
/* This command is sent to the original TM instead                    */
/* of the TM_CLOSE command and handles the closing, deletion and      */
/* renaming of the original and temporary TM file and index.          */
/* The temporary file has to be closed previously.                    */
/**********************************************************************/



/**********************************************************************/
/* The following structures for dialog IDAs are defined only          */
/* if INCL_EQFMEM_DLGIDAS has been defined                            */
/**********************************************************************/
#if defined(INCL_EQFMEM_DLGIDAS)

typedef struct _MEM_INCL_DLG_IDA
{
 BOOL         fInitErrorOccurred;         // Important init error occurred
}MEM_INCL_DLG_IDA, * PMEM_INCL_DLG_IDA;

#endif


typedef struct _SLIDER_DATA
{
   CHAR szLine [5] [TEXT_100_CHAR+MAX_PATH144]; // 5 text lines of 100 characters
                                             // + max path length
} SLIDER_DATA, * PSLIDER_DATA;



/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  OpenTmFile (PTMT_GLOBALS ptmtg,      /*.The TM Globals Area.............*/
              PSZ          pszFileName,/*.The TM Full File Name...........*/
              USHORT       idCommand), /* TMC_CREATE / TMC_OPEN...........*/

  ReadTmHeader (PTMT_GLOBALS ptmtg),  /* Tmt Globals area ................*/

  AllocTmtGlobals (USHORT usTmtGlobalsSize, /* The allocateion size.......*/
                   PPTMT_GLOBALS pptmtg),   /* The TM Globals Area........*/

  SetTmtWorkPointers (PTMT_GLOBALS ptmtg);  /* The TM Globals Area........*/

/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
VOID
GetFirstSegAddress
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PSZ          pszSource,         // pointer to source segment
  PTM_ADDRESS  pAddr              // pointer to address
);

VOID
CleanSource
(
   PTMT_GLOBALS ptmtg,            // Pointer to Tmt globals
   PSZ          pszSource         // Pointer to string to be tokenized
);

VOID
CalcPrimaryKey
(
  PTMT_GLOBALS ptmtg,             // Pointer to Globals structure
  PUCHAR       pchPrimaryKey      // Pointer to primary key
);

VOID
CalcSecondaryKey
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PSZ          pszSortedSecKey    // Ptr to sorted secondary key
);

VOID
Get4Chars
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  SHORT        sTokenId,          // WORD/USELESS/NOISE/TEXT/TAG
  PTOKENENTRY  pteFirst,          // Ptr to Text or Word Table
  PUCHAR       pchPrimaryKey      // 4 chars for primary key
);

VOID
WordsTokenize
(
  PTMT_GLOBALS  ptmtg             // Pointer to Tmt globals
);

VOID
UselessFiltering
(
  PTOKENENTRY  pteWord            // Pointer to word in WordsTable
);

USHORT
Tmt
(
  HTM       htm,                  // Pointer to TmtGlobals
  PIN       pIn,                  // Pointer to input buffer
  POUT      pOut                  // Pointer to output buffer
);


/**********************************************************************/
/* TmtX                                                               */
/**********************************************************************/
USHORT
TmtX ( HTM       htm,                  // Pointer to TmtGlobals
       PXIN       pIn,                 // Pointer to input buffer
       PXOUT      pOut );              // Pointer to output buffer


USHORT
CalcEntryInKeyDir
(
  PTM_HEADER  ptmh,               // Pointer to Tm Header
  PUCHAR      pchPrimaryKey       // Primary Key
);

USHORT
WriteTmhToDisk
(
  PTMT_GLOBALS ptmtg,             // Pointer to globals
  USHORT       fsWrite            // Write control flags word
);

USHORT
WriteToDisk
(
  PTMT_GLOBALS ptmtg,             // TMT globals
  ULONG        ldispPtr,          // New file pointer location
  PVOID        pvWrite,           // Pointer to write buffer
  USHORT       numWrite           // Number of bytes to write
);

USHORT
FormatMore
(
  PTMT_GLOBALS ptmtg              // Pointer to Tmt globals
);

USHORT
ReadStringFromDisk
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PCHAR        bufRead,           // Read buffer
  USHORT       lenString,         // String length
  PTM_ADDRESS  paddr              // Pointer to string buffer
);

USHORT
ReadSegmentFromDisk
(
  PTMT_GLOBALS ptmtg,             // pointer to Tmt globals
  PTM_ADDRESS  pAddr,             // Pointer to an address
  PSEGMENT     pSegment,          // Pointer to seg. buffer
  PBOOL        pfFirstSeg,        // Pointer to 1st seg. flag
  PBOOL        pfLastSeg          // Pointer to last seg. flag
);

USHORT
ReadBlock
(
  PTMT_GLOBALS ptmtg,                // Pointer to Tmt globals
  PTM_ADDRESS  paddr,                // Pointer to an address
  BOOL         fAddressInParameter   // Address in pAddr
);

INT
CharCompare
(
  const void * arg1,                       // First comparand
  const void * arg2                        // Second comparand
);

BOOL
fStrcmpCRLF
(
  PSZ     pszStr1,           // pointer to first string to be compared
  PSZ     pszStr2,           // pointer to second string to be compared
  PUSHORT pusEqualChars,     // (return value) pointer to USHORT where the
                             // number of equal characters (first string)
  USHORT  fCompareType);     // flag indicating whether CRLF and LF are ignored
                             // ALIGN_CRLF -> CRLF and LF are equal   equal
                             // USE_CRLF   -> exact matches required s
                             // IGNORE_CRL -> characters are equal


/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  TmtExtract (PTMT_GLOBALS    ptmtg,           /* pointer to Tmt globals....*/
              PEXT_IN         pExtIn,          /* pointer to input buffer...*/
              PEXT_OUT        pExtOut);        /* pointer to output buffer..*/

USHORT
  TmtGet (PTMT_GLOBALS     ptmtg,              /* pointer to Tmt globals....*/
          PGET_IN          pGetIn,             /* pointer to input buffer...*/
          PGET_OUT         pGetOut),           /* pointer to output buffer..*/

  TmtGetTMPart( PTMT_GLOBALS  ptmtg,           /* pointer to Tmt globals....*/
                PGETPART_IN   pGetPartIn,      /* pointer to input buffer...*/
                PGETPART_OUT  pGetPartOut),    /* pointer to output buffer..*/

  GetSegByAddr (PTMT_GLOBALS     ptmtg,        /* pointer to TmtGlobals.....*/
                PTM_ADDRESS      pAddr,        /* pointer to input address..*/
                PTM_ADDRESS      pAddrNext,    /* pointer to next address...*/
                PSEGMENT         pseg),        /* pointer to segment........*/


  TmtInfo     (PTMT_GLOBALS     ptmtg,         /* pointer to Tmt globals....*/
               PINFO_OUT        pInfoOut);     /* pointer to output buffer..*/


LONG
  CheckSimilar (PTMT_GLOBALS ptmtg,          /* pointer to Tmt globals.....*/
                PSEGMENT     pCurrentSegment,/* pointer to current segmnt .*/
                PTOKENENTRY  pTokenEntry,    /* points to 1'st & 2'nd word */
                PGET_IN      pGetIn,         /* pointer to input buffer....*/
                PSZ          pszInputKey,    /* pointer to sortedkey of....*/
                                             /* input segment..............*/
                PMATCH       pmtch),         /* pointer to stMatch.........*/

  CheckExact (PGET_IN      pGetIn,           /* pointer to input segment....*/
              PSEGMENT     pseg,             /* pointer to current seg......*/
              BOOL         fSimilarMod,      /* Similar= TRUE, Exact=FALSE..*/
              PMATCH       pmtch),           /* pointer to stMatch..........*/

  CalcExactIndustry (PGET_IN   pGetIn,       /* pointer to input segment....*/
                     PSEGMENT  pseg);        /* pointer to current seg......*/

VOID
  RankNewMatch (USHORT     numMatchesReq,    // number of matches required
                USHORT     usConvert,        // Conversion indicator
                PMATCH     pmtchCurrent,     // pointer to stMatch
                PSEGMENT   pseg,             // pointer to current seg
                PGET_OUT   pGetOut);         // pointer to GET_OUT struct

BOOL
  CheckWord   (PTOKENENTRY  pteFirst,       /*Point to first word ..........*/
               PTOKENENTRY  pteSecond) ;    /*Point to second word .........*/

VOID
LengthCorrectThresholds( PTMT_GLOBALS ptmtg,      //pointer to Tmt globals
                         USHORT       usLenInput, //number of words in inp. seg.
                         PGET_IN      pGetIn );            //pointer to input buffer

USHORT
  TmtAdd (PTMT_GLOBALS pTmtGlobals,        /* Pointer to Tmt Globals........*/
          PADD_IN      pAddIn,             /* Pointer to input buffer.......*/
          PADD_OUT     pAddOut),           /* Pointer to output buffer......*/

  TmtDelete (PTMT_GLOBALS ptmtg,               /* Pointer to Tmt Globals....*/
             PDEL_IN      pDeleteIn,           /* Pointer to input buffer...*/
             PDEL_OUT     pDeleteOut),         /* Pointer to output buffer..*/

  TmtReplace (PTMT_GLOBALS ptmtg,              /* Pointer to Tmt Globals....*/
              PREP_IN      pReplaceIn,         /* Pointer to input buffer...*/
              PREP_OUT     pReplaceOut),       /* Pointer to output buffer..*/

  AddSegToCluster (PTMT_GLOBALS  ptmtg,         /* Pointer to globals area..*/
                   PTM_ADDRESS   pAddr,         /* Ptr to initial address...*/
                   PSEGMENT      pseg),         /* Pointer to a segment.....*/

  FindFreshAddrInCluster (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals.*/
                          PTM_ADDRESS  pAddr,     /* Pointer to address.....*/
                          BOOL fReadFirstBlock),  /* Read block flag........*/

  TakeBlockFromSpool (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals.....*/
                      USHORT rcPrevious,      /* RC of the previous call....*/
                      PTM_ADDRESS  pAddr),    /* Pointer to an address......*/

  WriteStringToDisk (PTMT_GLOBALS ptmtg,      /* Pointer to Tmt globals.....*/
                     PCHAR   bufWrite,        /* Pointer to the string......*/
                     USHORT  lenString,       /* String length..............*/
                     PTM_ADDRESS  pAddr),     /* Pointer to string address..*/

  WriteBlock (PTMT_GLOBALS ptmtg),            /* Pointer to Tmt globals.....*/

  FindMatchSegInCluster (PTMT_GLOBALS ptmtg,        /* Ptr to Tmt globals...*/
                         PTM_ADDRESS  pAddr,        /* Pointer to address...*/
                         PBOOL        pfLastSeg,    /* Last segment flag....*/
                         PSEGMENT     psegIn,       /* Ptr to input buffer..*/
                         PSEGMENT     pseg),        /* Pointer to segment...*/

  DeleteSegment (PTMT_GLOBALS ptmtg,       /* Pointer to Tmt globals........*/
                PTM_ADDRESS   pAddr,       /* Address of segment to delete..*/
                BOOL          fLastSeg,    /* Last segment in cluster flag..*/
                PSEGMENT      pseg);       /* Pointer to the segment........*/

BOOL
  CheckMatchForDelete (PSEGMENT psegIn,    /* pointer to input buffer.......*/
                       PSEGMENT pseg);     /* currently, output buffer......*/

USHORT TmtDeleteTM( HTM        hMem,                 // TM handle
                    PDELTM_IN  pDelTmIn,             // Input structure
                    PDELTM_OUT pDelTmOut );          // Output structure


USHORT TmtCloseOrganize( HTM         hMem,           // TM handle
                         PENDORG_IN  pEndOrgIn,      // Input structure
                         PENDORG_OUT pEndOrgOut );   // Output structure



VOID   TmtDeleteFile( PDELFILE_IN  pDelFileIn,          // Input structure
                     PDELFILE_OUT pDelFileOut );       // Output structure

VOID   GetFileInfo( PFILEINFO_IN pFileInfoIn,        // Input structure
                    PFILEINFO_OUT pFileInfoOut );     // Output structure

VOID   RenameFile( PRENFILE_IN  pRenFileIn,          // Input structure
                   PRENFILE_OUT pRenFileOut );       // Output structure

BOOL fCheckFileClosed ( PSZ );         //--- pointer to file name


/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  TmtOpen (POPEN_IN   pOpenIn,           /* Pointer to input structure....*/
           POPEN_OUT  pOpenOut),         /* Pointer to output structure...*/

  TmtCreate (PCREATE_IN   pCreateIn,     /* Pointer to input structure....*/
             PCREATE_OUT  pCreateOut),   /* Pointer to output structure...*/

  TmtClose (PTMT_GLOBALS ptmtg,       /* Pointer to Globals structure.....*/
            PCLOSE_OUT   pCloseOut);  /* Pointer to output buffer.........*/

INT_PTR /*CALLBACK*/ MEMCREATEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MEMLOADDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MEMEXPORTDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MEMMERGEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MEMPROPDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MemCreateName( HWND, USHORT, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ UTLSERVERLISTDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ UTLSERVERLISTNAMEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ MEMINCLUDEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR /*CALLBACK*/ SERVWAITDLG( HWND, WINMSG, WPARAM, LPARAM );
USHORT  MemCreateProcess( PMEM_IDA, PSZ, USHORT );
VOID    MemDestroyProcess( PMEM_IDA, USHORT * );
USHORT  MemGetAddressOfProcessIDA( PMEM_IDA, WPARAM, USHORT *, PVOID * );
VOID    MemRcHandlingErrorUndefined( USHORT, PSZ );
VOID    MemRcHandlingErrorUndefinedHwnd( USHORT, PSZ, HWND );
USHORT  EQFMemOrganizeStart( PPROCESSCOMMAREA );
VOID    EQFMemOrganizeProcess( PPROCESSCOMMAREA );
VOID    EQFMemOrganizeEnd( PPROCESSCOMMAREA );
VOID    EQFMemLoadStart( PPROCESSCOMMAREA, HWND );
VOID    EQFMemLoadProcess( PPROCESSCOMMAREA, HWND );
VOID    EQFMemLoadEnd( PPROCESSCOMMAREA, HWND, LPARAM );
USHORT  EQFMemExportStart( PPROCESSCOMMAREA, HWND );
USHORT  EQFMemExportProcess( PPROCESSCOMMAREA, HWND );
USHORT  EQFMemExportEnd( PPROCESSCOMMAREA, HWND, LPARAM );
VOID    EQFMemMergeStart( PPROCESSCOMMAREA, HWND );
VOID    EQFMemMergeProcess( PPROCESSCOMMAREA, HWND );
VOID    EQFMemMergeEnd( PPROCESSCOMMAREA, HWND, LPARAM );
USHORT  EQFMemImportTrojaStart( HWND, LPARAM, HWND );
USHORT  MemInitSlider( HWND, USHORT, PSZ, USHORT, PSZ, PHWND, USHORT, USHORT);
USHORT  ReadABGrouping( PSZ, PSZ, ABGROUP );
MRESULT MemOrganizeCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
MRESULT MemExportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
MRESULT MemMergeCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
MRESULT MemImportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );



// ************* Memory macro definitions *******************************

//=======================================================================
//prototypes and definitions for TM utilities
#define SIZE_32K 32768
#define ERROR_TABLE_FULL 1111
#define ID_NOT_FOUND     2222


// adjust pointger for a new location
#define ADJUSTPTR( new, old, offsptr ) \
  ((PBYTE)new + ((PBYTE)offsptr - (PBYTE)old))

// do not update name tables when name is not contained in the table
#define NTMGETID_NOUPDATE_OPT 0x00000001

// returned ID if a name is not found in the table
#define NTMGETID_NOTFOUND_ID 0xFFFF


USHORT TmtXGet( EqfMemory*, PTMX_GET_W, PTMX_GET_OUT_W );

//tm put prototypes
VOID HashSentence( PTMX_SENTENCE );
USHORT HashTupel( PBYTE, USHORT, USHORT );
USHORT HashTupelW( PSZ_W, USHORT );
static VOID BuildVotes( PTMX_SENTENCE );
static VOID Vote( PTMX_TERM_TOKEN, PTMX_SENTENCE, USHORT );
USHORT CheckCompactArea( PTMX_SENTENCE, EqfMemory* );

USHORT TokenizeTarget( StringTagVariants*, PTMX_TAGTABLE_RECORD*, PLONG, PSZ, EqfMemory* );

VOID FillTmRecord( PTMX_SENTENCE,
                   PTMX_RECORD, PTMX_TARGET_CLB, USHORT );

USHORT DetermineTmRecord( EqfMemory*, PTMX_SENTENCE, PULONG );

VOID FillTargetRecord( PTMX_SENTENCE, //PTMX_TAGTABLE_RECORD,
                       PSZ_W, USHORT, PTMX_TARGET_RECORD *, PTMX_TARGET_CLB );

VOID DeleteOldestRecord( PTMX_RECORD, PULONG );

//tm get prototypes
USHORT GetExactMatch( EqfMemory*, PTMX_SENTENCE, PTMX_GET_W, PTMX_MATCH_TABLE_W,
                      PUSHORT, PTMX_GET_OUT_W );
USHORT ExactTest( EqfMemory*, PTMX_RECORD, PTMX_GET_W, PTMX_SENTENCE,
                  PTMX_MATCH_TABLE_W, PUSHORT, ULONG );

BOOL AddTagsToString( PSZ, PULONG, PTMX_TAGTABLE_RECORD, PSZ );
BOOL AddTagsToStringW( PSZ_W, PLONG, PTMX_TAGTABLE_RECORD, PSZ_W );

INT CompCount( const void *, const void * );
INT CompCountVotes( const void *, const void * );
VOID CleanupTempMatch( PTMX_MATCHENTRY, PTMX_MATCHENTRY *, PUSHORT, PUSHORT );
USHORT FillMatchEntry( EqfMemory*, PTMX_SENTENCE, PTMX_MATCHENTRY, PUSHORT );
USHORT FuzzyTest( EqfMemory*, PTMX_RECORD, PTMX_GET_W, PTMX_MATCH_TABLE_W, PUSHORT,
                  PUSHORT, PUSHORT, PUSHORT, PTMX_SENTENCE, ULONG );
USHORT GetFuzzyMatch( EqfMemory*, PTMX_SENTENCE, PTMX_GET_W, PTMX_MATCH_TABLE_W, PUSHORT );


//tm delete segment prototypes
USHORT NTMCheckForUpdates( EqfMemory* );
USHORT NTMLockTM( EqfMemory*, BOOL, PBOOL );
USHORT MemBatchTMCreate( HWND hwnd, PDDEMEMCRT pMemCrt );
USHORT MemBatchTMExport( HWND hwnd, PDDEMEMEXP pMemExp );
USHORT MemBatchTMImport( HWND hwnd, PDDEMEMIMP pMemImp );
USHORT MemBatchTMDelete( HWND hwnd, PDDEMEMDEL pMemDel );

// definitions for terse of TM name tables
// max size of in-memory name table
#define SIZE_64K 0xFF00L

// magic word for recognition of tersed name tables
// Note: this ULONG value has to ensure that it will never been used
//       as a name so it is prefixed and suffixed by a 0x00
#define TERSEMAGICWORD 0x0019FF00L

// structure of header for tersed name tables
typedef struct _TERSEHEADER
{
  USHORT      usAllocSize;             // alloc size in untersed tables
  USHORT      usMaxEntries;            // max entries in untersed tables
  ULONG       ulMagicWord;             // magic word for tersed name
  USHORT      usDataSize;              // size of data area when expanded
  USHORT      usCompression;           // type of compression used for data
                                       // (values as defined in EQFCMPR.H)
} TERSEHEADER, *PTERSEHEADER;

BOOL TMDelTargetClb
(
  PTMX_RECORD        pTmRecord,        // ptr to TM record
  PTMX_TARGET_RECORD pTargetRecord,    // ptr to target record within TM record
  PTMX_TARGET_CLB    pTargetClb        // ptr to target control record
);

ULONG EQFUnicode2Compress( PBYTE pTarget, PSZ_W pInput, ULONG usLenChar );
ULONG EQFCompress2Unicode( PSZ_W pOutput, PBYTE pTarget, ULONG usLenComp );

#include "EncodingHelper.h"


/*
class FilterParam{

public:

   FilterParam(std::string& search, FilterField field, FilterType type): m_field(field), m_type(type), 
        m_searchString(search), m_searchStringW(convertStrToWstr(search)){};
  
  FilterParam(long t1, long t2, FilterField field = TIMESTAMP): m_timestamp1(t1), m_timestamp2(t2), m_field{field}{}

private:
  FilterField m_field;
  FilterType m_type;

  long m_timestamp1=0, m_timestamp2=0;
};//*/



/*
class StringFilterParam{
  std::string m_searchString;
  std::wstring m_searchStringW;

public:
  StringFilterParam(std::string& search, ProposalFilter::FilterType type): FilterParam(type, field), {}
}//*/

typedef struct _MEM_ORGANIZE_IDA
{
 std::shared_ptr<int> memRef;
 TMX_PUT_W  stPutIn;                        // input for TmReplace
 TMX_EXT_OUT_W stPutOut;                       // The REPLACE_OUT structure
 TMX_EXT_IN_W  stExtIn;                        // input for TmExtract
 TMX_EXT_OUT_W stExtOut;                       // output for TmExtract
 CHAR          szPathOrganizeMem[2048];        // Full path and name of TM to be organized
 CHAR          szDrive[MAX_DRIVE];             // Drive letter without colum
 CHAR          szMemName[MAX_LONGFILESPEC];    // Translation memory name 
 CHAR          szTempMemName[MAX_LONGFILESPEC];// name of temporary translation memory 
 CHAR          szPluginName[MAX_LONGFILESPEC]; // name of plugin used for the memory
 CHAR          szPathTempMem[2048];            // Full path of temporary transl. mem.
 OtmProposal   *pProposal;                     // buffer for memory proposal
 std::shared_ptr<EqfMemory>     pMem;                           // Handle of transl. memory
 std::shared_ptr<EqfMemory>     pMemTemp;                       // Handle of tmporary transl. memory
 BOOL          fMsg;                           // A message has been issued already
 //ULONG         ulSegmentCounter;               // Number of segments organized
 //ULONG         ulInvSegmentCounter;            // Invalid Segment counter
 TIME_L        tStamp;                         // Time stamp of organize start time
 HWND          hProgressWindow;                // Handle of progress indicator window
 ULONG         ulProgressPos;                  // position of progress indicator
 CHAR          szPathTempIndex[MAX_EQF_PATH];  //full temporary index name

 LANGUAGE      szSourceLanguage;               // source language of org TM
 LANGUAGE      szTargetLanguage;               // target language of org TM
 CHAR          szTagTable[MAX_FILESPEC];       // tag table of org TM

 CHAR          szPropertyName[MAX_FILESPEC];   // property name with ext.
 CHAR          szTempPropertyName[MAX_FILESPEC]; // property name temp TM with ext

 CHAR          szOrgProp[MAX_EQF_PATH];        // full name of original property file
 CHAR          szTmpProp[MAX_EQF_PATH];        // full name of temporary property file

 CHAR          szEqfPath[MAX_EQF_PATH];        // system path D:\EQF
 USHORT        usOrgType;                      // type ORGANIZE, CONVERT
 BOOL          fBatch;                 // TRUE if organizing in batch mode
 HWND          hwndErrMsg;             // handle of window to be used for error msgs
 PDDEMEMORG    pDDEMemOrg;             // ptr to batch memory organize data
 USHORT        usRC;                   // return code / error code
 LONG          NextTask;               // next task in non-DDE batch mode
 PSZ           pszNameList;            // pointer to list of TMs being organized
 PSZ           pszActiveName;          // points to current name in pszNameList
 CHAR          szBuffer[2048];         // general purpose buffer
 BOOL          fFirstGet;              // TRUE = this is the first get access
 std::vector<ProposalFilter> m_reorganizeFilters;
}MEM_ORGANIZE_IDA, * PMEM_ORGANIZE_IDA;

USHORT  TmCloseOrganize( PMEM_ORGANIZE_IDA, USHORT );

USHORT NTMCloseOrganize ( PMEM_ORGANIZE_IDA, USHORT            );

USHORT NTMConvertProperties( PPROPTRANSLMEM, PMEM_ORGANIZE_IDA );


/*! \brief constant defining the maximum number of opened memories
*/
#define OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES 40
//5MB reserved for service
#define MEMORY_RESERVED_FOR_SERVICE 5000000


class CreateMemRequestData;
class EqfMemoryPlugin : public OtmPlugin
/*! \brief This class implements the standard translation memory plugin (EQF) for OpenTM2.
*/

{
  //static EqfMemoryPlugin* _instance;
public:
/*! \brief Constructor
*/
	EqfMemoryPlugin();
/*! \brief Destructor
*/
	~EqfMemoryPlugin();

  static EqfMemoryPlugin* GetInstance();

  	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

  
/*! \enum eRegRc
	Possible return values of EqfMemory and EqfMemoryPlugin methods
*/
	enum eRc
	{
		eSuccess = 0,			        /*!< method completed successfully */
		eUnknownPlugin,		        /*!< the specified memory plugin is not available */
//		eInvalidName,		          /*!< plugin-name is invalid */
//		eAlreadyRegistered,	      /*!< plugin with same name was already registered before */
//		eInvalidRequest,	        /*!< method may only be called from within registerPlugins call */
    eMemoryNotFound,          /*!< the specified memory does not exist or is not controlled by this memory plugin*/
		eUnknown,		            	/*!< plugin with that name was not registered before */
		eNotSupported,		        /*!< method is not supported by this plugin */
		eBufferTooSmall,          /*!< the provided buffer is too small */
    eNotSupportedMemoryType,
    eNotEnoughMemory,         /*!< not enough system memory to process the request */
    eRepeat                   /*!< repeat calling this method until processing has been completed*/
	};


// options for the importFromMemoryFiles method
static const int IMPORTFROMMEMFILES_COMPLETEINONECALL_OPT = 1;  // complete the import in one call, do not divide the processing into smaller steps


/*! \brief Returns the name of the plugin
*/
	const char* getName();
/*! \brief Returns a short plugin-Description
*/
	const char* getShortDescription();
/*! \brief Returns a verbose plugin-Description
*/
	const char* getLongDescription();
/*! \brief Returns the version of the plugin
*/
	const char* getVersion();
/*! \brief Returns the name of the plugin-supplier
*/
	const char* getSupplier();

  EqfMemory* openMemoryNew(const std::string& memName);

/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
  int closeMemory(
	  EqfMemory *pMemory			 
  );

/*! \brief set description of a memory
  \param pszName name of the memory
  \param pszDesc description information
	\returns 0 if successful or error return code
*/
  int setDescription(
    const char* pszName,
    const char* pszDesc);

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
	int getLastError
  (
    std::string &strError
  ); 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
	int getLastError
  (
    char *pszError,
    int iBufSize
  ); 
  

/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	bool stopPlugin( bool fForce = false );

/*! \brief Handle a return code from the memory functions and create the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name or NULL if not available
    \param pszMemPath fully qualified memory path name or NULL if not available
    \param strLastError reference to string object receiving the message text
    \param iLastError reference to a integer variable receiving the error code
  	\returns original or modified error return code
*/
static int handleError( int iRC, char *pszMemName, char *pszMarkup, char *pszMemPath, std::string &strLastError, int &iLastError );
    
	int iLastError;
	std::string strLastError;  
  std::vector< std::shared_ptr<EqfMemory> > m_MemInfoVector;

private:

 std::shared_ptr<EqfMemory>  findMemory( const char *pszName );
  int findMemoryIndex(const char *pszName);

	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
	std::string descrType;
  char szBuffer[4000];                         // general purpose buffer area
  char szSupportedDrives[27]; // list of supported drives

};

class TMManager{

  public:
  
  std::atomic_bool fWriteRequestsAllowed{0};
  std::atomic_bool fServiceIsRunning{1};
    /*! \brief Pointer to the list of opened memories
    */
    //std::vector<EqfMemory> vMemoryList;
  typedef std::map <std::string, std::shared_ptr<EqfMemory>> TMMap;
  TMMap tms;

  std::mutex mutex_requestTM;
  std::mutex mutex_access_tms;

  enum TMManagerCodes{
    TMM_NO_ERROR = 0,

    TMM_TMD_NOT_FOUND = 16,
    TMM_TMI_NOT_FOUND = 32,
    TMM_TM_NOT_FOUND = TMM_TMD_NOT_FOUND + TMM_TMI_NOT_FOUND,


  };

  bool IsMemoryLoaded(const std::string& strMemName);
  bool IsMemoryLoadedUnsafe(const std::string& strMemName);
  int TMExistsOnDisk(const std::string& tmName, bool logErrorIfNotExists = true);

  int AddMem(const std::shared_ptr<EqfMemory> NewMem);
  int OpenTM(const std::string& strMemName);
  int CloseTM(const std::string& strMemName);
  int CloseTMUnsafe(const std::string& strMemName);
  int DeleteTM(const std::string& strMemName, std::string& outputMsg);

  int RenameTM(const std::string& oldMemName, const std::string& newMemName, std::string& outputMsg);

  int MoveTM(
    const std::string& oldMemoryName,
    const std::string& newMemoryName,
    std::string &strError
  );
  
  std::shared_ptr<EqfMemory> requestServicePointer(const std::string& strMemName, COMMAND command);
  std::shared_ptr<EqfMemory> requestReadOnlyTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack);
  std::shared_ptr<EqfMemory> requestWriteTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack);

  std::shared_ptr<EqfMemory> CreateNewEmptyTM(const std::string& strMemName, const std::string& strSrcLang, 
      const std::string& strMemDescription, int& _rc_, bool keepInRamOnly = false);


    /*! \brief OpenTM2 API session handle
    */
    LONG hSession = 0;
    /*! \brief close a memory and remove it from the open list
      \param iIndex index of memory in the open list
      \returns 0 
    */
    
    /*! \brief find a memory in our list of active memories
        \param pszMemory name of the memory
        \returns index in the memory table if successful, -1 if memory is not contained in the list
    */
    int findMemoryInList( const std::string& memName );

    /*! \brief close any memories which haven't been used for a long time
        \returns 0
    */
    size_t CleanupMemoryList(size_t memoryRequested);

    /*! \brief calcuate total amount of RAM occupied by opened memory files
        \returns 0
    */
    size_t CalculateOccupiedRAM();

    std::shared_ptr<EqfMemory>  findOpenedMemory( const std::string& memName);

    int GetMemImportInProcessCount();
    
    /*! \brief Close all open memories
    \returns http return code0 if successful or an error code in case of failures
    */
    int closeAll();

///MEMORY FACTORY REGION
    /*! \brief   This class provides factory methods for EqfMemory objects 

*/

//public:


/*! \brief Error code definition
*/
  static const int ERROR_PLUGINNOTAVAILABLE   = 1002;
  static const int ERROR_MEMORYOBJECTISNULL   = 1003;
  static const int ERROR_BUFFERTOOSMALL       = 1004;
  static const int ERROR_INVALIDOBJNAME       = 1005;
  static const int ERROR_MISSINGPARAMETER     = 1006;

/*! \brief Options for proposal sorting
*/
  static const LONG LATESTPROPOSAL_FIRST = 0x00000001;

  /*! \brief This static method returns a pointer to the TMManager object.
	The first call of the method creates the TMManager instance.
*/
  static TMManager* GetInstance();

  TMManager(){
    T5LOG( T5DEBUG) << "::Ctor of TMManager";
    pluginList = NULL;
    pHandleToMemoryList = new std::vector<EqfMemory *>;
    refreshPluginList();    
  }

/* \brief Close a memory 
   Close the memory object and free all memory related resources.
   The memory object is not valid anymore.
   \param pMemory pointer to the memory object being closes
   \returns 0 when successful or a error return code
*/
int closeMemory
(
  EqfMemory *pMemory
);

/*! \brief Show error message for the last error
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory causing the problem
  memory object name (pluginname + colon + memoryname)
  \param pMemory pointer to existing memory object or NULL if not available
  \param hwndErrMsg handle of parent window message box
*/
void showLastError(
  char *pszPluginName,
  char *pszMemoryName,
  EqfMemory *pMemory,
  HWND hwndErrMsg
);

/*! \brief   get error message for the last error
  \param   pMemory pointer to existing memory object or NULL if not available
  \param   iLastError the last error number
  \param   strError the error string returned with
  \returns the last error string
*/
std::string& getLastError(
    EqfMemory *pMemory,
    int& iLastError,
    std::string& strError);

/*! \brief Copy best matches from one proposal vector into another
  and sort the proposals
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
*/
void copyBestMatches(
  std::vector<OtmProposal *> &SourceProposals,
  std::vector<OtmProposal *> &TargetProposals,
  int iMaxProposals
);

/*! \brief Copy best matches from one proposal vector into another
  and sort the proposals
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \param iMTDisplayFactor factor for the placement of machine matches within the table
  \param fExactAndFuzzies switch to control the handling of fuzzy matches when exact matches exist, TRUE = keep fuzzy matches even when exact matches exist
  \param lOptions options for the sorting of the proposals
*/
void copyBestMatches(
  std::vector<OtmProposal *> &SourceProposals,
  std::vector<OtmProposal *> &TargetProposals,
  int iMaxProposals, 
  int iMTDisplayFactor,
  BOOL fExactAndFuzzies,
  LONG lOptions = 0
);

/*! \brief Insert proposal into proposal vector at the correct position and
  remove a proposal with lesser relevance when iMaxPropoals have already been filled
  \param NewProposal pointer to proposal being inserted
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \param fLastEntry true = this is the last entry in the table
  \param iMTDisplayFactor factor for the placement of machine matches within the table
  \param lOptions options for the sorting of the proposals
*/
void insertProposalData(
  OtmProposal *newProposal,
  std::vector<OtmProposal *> &Proposals,
  int iMaxProposals,
  BOOL fLastEntry, 
  int iMTDisplayFactor = -1,
  LONG lOptions = 0
);

/*! \brief Get the object name for the memory
  \param pMemory pointer to the memory object
  \param pszObjName pointer to a buffer for the object name
  \param iBufSize size of the object name buffer
  \returns 0 when successful or the error code
*/
int getObjectName( EqfMemory *pMemory, char *pszObjName, int iBufSize );

/*! \brief Get the plugin name and the memory name from a memory object name
  \param pszObjName pointer to the memory object name
  \param pszPluginName pointer to the buffer for the plugin name or 
    NULL if no plugin name is requested
  \param iPluginBufSize size of the buffer for the plugin name
  \param pszluginName pointer to the buffer for the plugin name or
    NULL if no memory name is requested
  \param iNameBufSize size of the buffer for the memory name
  \returns 0 when successful or the error code
*/
int splitObjName( char *pszObjName, char *pszPluginName, int iPluginBufSize, char *pszMemoryName, int iNameBufSize  );

 /*! \brief Get the sort order key for a memory match
  \param Proposal reference to a proposal for which the sort key is evaluated
  \param iMTDisplayFactor the machine translation display factor, -1 to ignore the factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
 int getProposalSortKey(  OtmProposal &Proposal, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions = 0 );

 /*! \brief Get the sort order key for a memory match
  \param Proposal reference to a proposal for which the sort key is evaluated
  \returns the proposal sort key
*/
int getProposalSortKey(  OtmProposal &Proposal );

 /*! \brief Get the sort order key for a memory match
  \param MatchType match type of the match
  \param ProposalType type of the proposal
  \param iFuzziness fuzziness of the proposal
  \param iMTDisplayFactor the machine translation display factor, -1 to use the system MT display factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int getProposalSortKey(  OtmProposal::eMatchType MatchType, OtmProposal::eProposalType ProposalType, int iFuzzyness, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions = 0 );

  /*! \brief Replace a memory with the data from another memory
    This method bevaves like deleting the replace memory and renaming the
    replaceWith memory to the name of the replace memory without the overhead of the
    actual delete and rename operations
    \param pszPluginName name of plugin of the memory
    \param pszReplace name of the memory being replaced
    \param pszReplaceWith name of the memory replacing the pszReplace memory
	  returns 0 if successful or error return code
  */
 int ReplaceMemory
  (
    const std::string& strReplace,
    const std::string& strReplaceWith, 
    std::string& outputMsg
  );


private:
  
  /* \brief Get memory plugin with the given plugin name
   \param pszPlugin plugin-name
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *getPlugin
(
  const char *pszPluginName
);

/* \brief Refresh internal list of memory plugins 
*/
void refreshPluginList();

/*! \brief search option flags
*/
#define SEARCH_SOURCE  1
#define SEARCH_TARGET  2
#define SEARCH_CASEINSENSITIVE 4
#define SEARCH_WHITESPACETOLERANT 8


/*! \brief Pointer to the list of installed memory plugins
*/
 std::vector<EqfMemoryPlugin *> *pluginList;

 /*! \brief error code of last error occured
*/
 int iLastError;

 /*! \brief error text for last error occured
*/
 std::string strLastError;

/*! \brief buffer for memory object names
*/
 char szMemObjName[512];

/*! \brief buffer for name of default memory plugin
*/
 char szDefaultMemoryPlugin[512];

/*! \brief buffer for name of default shared memory plugin
*/
 char szDefaultSharedMemoryPlugin[512];

/*! \brief array containing the memory objects referred to by a handle
*/
std::vector<EqfMemory *> *pHandleToMemoryList;

/*! \brief Buffer for segment text
*/
  CHAR_W m_szSegmentText[MAX_SEGMENT_SIZE+1];

  /*! \brief Buffer for search string
  */
  CHAR_W m_szSearchString[MAX_SEGMENT_SIZE + 1];


//end of MEMORY FACTORY region
};


/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyOtmProposalToMemProposal( OtmProposal *pOtmProposal, PMEMPROPOSAL pProposal  );

/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyMemProposalToOtmProposal( PMEMPROPOSAL pProposal, OtmProposal *pOtmProposal );

////OTMFUNC region
/**
* \file tm.h
*
* \brief External header file for OpenTM2 API functions
*
*	Copyright Notice:
*
*	Copyright (C) 1990-2017, International Business Machines
*	Corporation and others. All rights reserved
**/

#ifndef _OTMFUNC_H_INCLUDED
#define _OTMFUNC_H_INCLUDED

#include "win_types.h"
#include "EQF.H"

// include EQFPAPI.H for segment definitions
#ifndef _EQFPAPI_H_INCLUDED
  #include "EQFPAPI.H"
#endif 


typedef LONG HSESSION;
typedef HSESSION *PHSESSION;

typedef int BOOL;

typedef struct _REPORTTYPE
{
  PSZ  pszReport;
  LONG lRepType;
  PSZ  pszDescription;
} REPORTTYPE, *PREPORTTYPE;


typedef struct _REPORTSETTINGS
{
  PSZ  pszCountType;
  BOOL bShow;
  BOOL bSummary;
  PSZ  pszRepLayout;
  BOOL bShrink;
  PSZ  pszStatisticType;
  BOOL bExProposal;
} REPORTSETTINGS, *PREPORTSETTINGS;


typedef struct _FACTSHEET
{
  float lComplexity;
  float lPayFactor;
} FACTSHEET,*PFACTSHEET;


typedef struct _FINALFACTORS
{
  LONG  lUnit;
  float lCurrFactor;
  PSZ   pszLocalCurrency;
} FINALFACTORS,*PFINALFACTORS;


typedef struct _EXTFOLPROP
{
  CHAR  chDrive;
  CHAR  szTargetLang[MAX_LANG_LENGTH];
  CHAR  szRWMemory[MAX_LONGFILESPEC];
  CHAR  szROMemTbl[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];
  CHAR  szDicTbl[ NUM_OF_FOLDER_DICS][MAX_FILESPEC];
} EXTFOLPROP, *PEXTFOLPROP;
//
// option flags used by the function call interface
//

#define NO_ERROR 0

// location flags for TMs and dictionaries
#define LOCAL_OPT         0x00000001L
#define SHARED_OPT        0x00000002L

// analysis flags 
#define TMMATCH_OPT            0x00000010L
#define ADDTOMEM_OPT           0x00000020L
#define AUTOSUBST_OPT          0x00000040L
#define UNTRANSLATED_OPT       0x00000080L
#define AUTOLAST_OPT           0x00000100L
#define AUTOJOIN_OPT           0x00000200L
#define AUTOCONTEXT_OPT        0x00000400L
#define REDUNDCOUNT_OPT        0x00000800L
#define IGNOREPATH_OPT         0x00001000L
#define ADJUSTLEADWS_OPT       0x00002000L
#define ADJUSTTRAILWS_OPT      0x00004000L
#define RESPECTCRLF_OPT        0x00008000L
#define NOBLANKATSEGEND_OPT    0x00010000L
#define NOSUBSTIFIDENTICAL_OPT 0x00020000L
#define SENDTOMT_OPT           0x00080000L
#define STOPATFIRSTEXACT_OPT   0x00100000L
// ignore memory proposals which are flagged with a comment

#define IGNORECOMMENTED_OPT    0x00200000L

/** @brief protect text inside XMP sections, MsgNum sections, Meta sections, screen sections, and code block sections */
#define PROTECTXMPSCREEN_OPT   0x00040000L

/** @brief protect text inside XMP sections */
#define PROTECTXMP_OPT         0x00400000L
/** @brief protect text inside MsgNum sections */
#define PROTECTMSGNUM_OPT      0x00800000L
/** @brief protect text inside Meta sections */
#define PROTECTMETA_OPT        0x01000000L
/** @brief protect text inside screen sections */
#define PROTECTSCREEN_OPT      0x02000000L
/** @brief protect text inside CodeBlock sections */
#define PROTECTCODEBLOCK_OPT   0x04000000L


// folder export and folder import flags
#define WITHDICT_OPT           0x00001000L
#define WITHMEM_OPT            0x00002000L
#define DELETE_OPT             0x00004000L
#define WITHREADONLYMEM_OPT    0x00008000L
#define WITHDOCMEM_OPT         0x00010000L
#define MASTERFOLDER_OPT       0x00020000L
#define XLIFF_OPT              0x00040000L
#define NOMARKUP_UPDATE_OPT    0x00080000L
#define WO_REDUNDANCY_DATA_OPT 0x00100000L

//
// document import specific options
//

// do not search in sub-directories for specified document names
#define NOSUBDIRSEARCH_OPT  0x00001000L

//
// flags for document export and wordcount
//

// Count replace matches separately option (can only be used in conjunction with TMMATCH_OPT)
#define SEPERATEREPLMATCH_OPT  0x02000000L

// create word count without duplicates (R009560)
#define DUPMEMMATCH_OPT   0x01000000L

// create word count without duplicates (R009560)
#define DUPLICATE_OPT     0x00010000L

// create source word count / export source document
#define SOURCE_OPT        0x00020000L

// create translated/untranslated word count / export target document
#define TARGET_OPT        0x00040000L

// export SNOMATCH document
#define SNOMATCH_OPT      0x00080000L

// count fuzzy matches (only EqfCountWords)
#define FUZZYMATCH_OPT         0x00080000L

//

// flags for dictionary import merge options
// (IGNORE_OPT also used in EqfImportMem to skip invalid segments)
//
#define IGNORE_OPT        0x00000020L
#define REPLACE_OPT       0x00000040L
#define COMBINE_OPT       0x00000080L

// flags for dictionary import/export in DXT UTF8 format
#define DXT_UTF8_OPT     0x00200000L

// external/internal flags
#define EXTERNAL_OPT      0x01000000L
#define INTERNAL_OPT      0x02000000L

// flags for build archive TM function EqfArchiveTM
#define USEASFOLDERTM_OPT   0x20000000L
#define SOURCESOURCEMEM_OPT 0x40000000L
#define SETMFLAG_OPT        0x80000000L

// general flags
#define OVERWRITE_OPT     0x10000000L

// characterset flags for dictionary/memory import and export
#define ASCII_OPT        0x00020000L
#define ANSI_OPT         0x00040000L
#define UTF16_OPT        0x00080000L

// import in TMX format (UTF-16 and UTF-8)
#define TMX_OPT          0x00100000L

// export in TMX format
#define TMX_UTF16_OPT    0x00100000L
#define TMX_UTF8_OPT     0x00200000L

// clean/remove RTF tags
#define CLEANRTF_OPT     0x00400000L

// restrict RTF tag removal to tags enclosed in curly braces only
#define INCURLYBRACE_OPT   0x00800000L

// remove linebreaks in text of translation units
#define TMX_NOCRLF_OPT     0x01000000L

// import in XLIFF machine translation (MT) format
#define XLIFF_MT_OPT       0x02000000L

// how to handle unknown markup table when importing memory
#define CANCEL_UNKNOWN_MARKUP_OPT      0x10000000L
#define SKIP_UNKNOWN_MARKUP_OPT        0x20000000L
#define GENERIC_UNKNOWN_MARKUP_OPT     0x40000000L

// option for EqfImportMemEx only: create a new segment match ID even if the match has one already
#define FORCENEWMATCHID_OPT 0x00000004L

// flags for document export in validation format

// export documents in validation format (cannot be used together with the other export options)
// export validation document in XML format
#define VALFORMAT_XML_OPT    0x00200000L  
// export validation document in HML format
#define VALFORMAT_HTML_OPT   0x00400000L  
//export validation document in DOC format
#define VALFORMAT_DOC_OPT    0x00800000L  
//export validation document in ODT format
#define VALFORMAT_ODT_OPT    0x00100000L  
//export validation document in DOCX format
#define VALFORMAT_DOCX_OPT    0x08000000L  

// flags which can be used together with the export options above

// combine documents into a single file 
#define VALFORMAT_COMBINE_OPT  0x01000000L  
// include protected segments in the validation document
#define VALFORMAT_PROTSEGS_OPT 0x02000000L  

// export documents as plain XML
#define PLAINXML_OPT           0x04000000L

// relative path document export options
#define WITHRELATIVEPATH_OPT      0x20000000L
#define WITHOUTRELATIVEPATH_OPT   0x40000000L

// document export/import in internal format
#define OPENTM2FORMAT_OPT 0x80000000L

// include TVT tracking ID document export option
#define WITHTRACKID_OPT 0x08000000L

//
// output type options for EqfCountWords and EqfCreateCountReport
//
#define TEXT_OUTPUT_OPT   0x00100000L  
#define XML_OUTPUT_OPT    0x00200000L  
#define HTML_OUTPUT_OPT   0x00400000L  

// report IDs for function EqfCreateCountReport
#define HISTORY_REP             1
#define COUNTING_REP            2
#define CALCULATING_REP         3
#define PREANALYSIS_REP         4
#define REDUNDANCY_REP          5
#define REDUNDANCYSEGMENT_REP   6

// report types for function EqfCreateCountReport

// types for HISTORY_REPORT
#define BRIEF_SORTBYDATE_REPTYPE  1
#define BRIEF_SORTBYDOC_REPTYPE   2
#define DETAIL_REPTYPE            3

// types for COUNTING_REPORT
#define WITHTOTALS_REPTYPE        4
#define WITHOUTTOTALS_REPTYPE     5

// types for CALCULATING_REPORT, PREANALYSIS_REPORT, REDUNDANCY_REPORT,
//           and REDUNDANCYSEGMENT_REPORT
#define BASE_REPTYPE                    6
#define BASE_SUMMARY_REPTYPE            7
#define BASE_SUMMARY_FACTSHEET_REPTYPE  8
#define SUMMARY_FACTSHEET_REPTYPE       9
#define FACTSHEET_REPTYPE              10


//options for EqfCreateITM lType
#define NOANA_TYP           0x1
#define NOTM_TYP            0x2
#define PREPARE_TYP         0x4

//options for EqfChangeMFlag lAction
#define CLEAR_MMOPT         0
#define SET_MMOPT           1

// options for non-dde interface (calculating report)
#define BASE_TYP              1
#define FACT_TYP              2
#define SUM_TYP               4
#define BRIEF_SORTBYDATE_TYP  8
#define BRIEF_SORTBYDOC_TYP  16
#define DETAIL_TYP           32


#define PLAUS_OPT   1             // (Plausibilty check)
#define LOST_OPT    2             // (Lost Data: Force new shipment)
#define LIST_OPT    4             // (List of Documents)

// option for fuzzy segment search to mark the differences in the output file
#define MARKDIFFERENCES_OPT 0x08000000L

// fuzzy segment search modes
#define UPTOSELECTEDCLASS_MODE 0
#define SELECTEDCLASSANDHIGHER_MODE 1
#define ONLYSELECTEDCLASS_MODE 2


// return when all activitiy has been completed (instead of iterative processing)
#define COMPLETE_IN_ONE_CALL_OPT 0x01000000

// non-DDE function interface specific return codes

// Info: return code for functions not implemented yet
#define NOTIMPLEMENTED_RC               10001

// Error: passed session handle is invalid
#define ERROR_INVALID_SESSION_HANDLE    10002

// Notification: current function has not been completed yet
//               re-call function until a non-CONTINUE_RC
//               return code is retuned
#define CONTINUE_RC                     10003

// Error: Wrong or conflicting options specified
#define WRONG_OPTIONS_RC                10004

// Error: different function has been called although
//        current task has not been completed yet
#define LASTTASK_INCOMPLETE_RC          10005

// Error: Segment has been joined to a previous one and is not
//        part of the visible document. It cannot be used
//        in function EqfGetSourceLine
#define SEGMENTISJOINED_RC               10006

// Error: Given segment number is invalid
//        (probably not contained in document)
#define INVALIDSEGMENT_RC                10007

// Error: Given handle of loaded file is invalid
#define INVALIDFILEHANDLE_RC             10008

// Error: There is no segment matching the given position
#define NOMATCHINGSEGMENT_RC             10009

// Info: End if data reached
#define ENDREACHED_RC                    10010

// Info: Time out occurend (eceeded given search time)
#define TIMEOUT_RC                    10011

#define VAL_COMBINE_OPT               0x00000001L 
#define VAL_PRESERVE_LINKS_OPT        0x00000002L 
#define VAL_MAN_EXACTMATCH_OPT        0x00000004L
#define VAL_REMOVE_INLINE_OPT         0x00000008L
#define VAL_TRANSTEXT_ONLY_OPT        0x00000010L
#define VAL_INCLUDE_COUNT_OPT         0x00000020L
#define VAL_INCLUDE_MATCH_OPT         0x00000040L
#define VAL_MISMATCHES_ONLY_OPT       0x00000080L

#define VAL_AUTOSUBST_OPT             0x00000100L
#define VAL_MOD_AUTOSUBST_OPT         0x00000200L
#define VAL_EXACT_OPT                 0x00000400L
#define VAL_MOD_EXACT_OPT             0x00000800L
#define VAL_GLOBAL_MEM_OPT            0x00001000L
#define VAL_MACHINE_OPT               0x00002000L
#define VAL_FUZZY_OPT                 0x00004000L
#define VAL_NEW_OPT                   0x00008000L
#define VAL_NOT_TRANS_OPT             0x00010000L
#define VAL_PROTECTED_OPT             0x00020000L
#define VAL_REPLACE_OPT               0x00040000L

#define VAL_VALIDATION_OPT            0x01000000L
#define VAL_PROOFREAD_OPT             0x02000000L


class IMPORTMEMORYDATA;
/*! \brief Import a Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemName name of the Translation Memory
  \param pszInFile fully qualified name of the input file
  \param lOptions options for Translation Memory import
         one of the format options 
         - TMX_OPT import in TMX format
         - XLIFF_MT_OPT import in XLIFF format
         - UTF16_OPT import in the EXP format (Unicode UTF-16 encoded)
         - ANSI_OPT import in the EXP format (ANSI encoded)
         - ASCII_OPT import in the EXP format (ASCII encoded)
         .
         and one of the markup table handling functions
         - CANCEL_UNKNOWN_MARKUP_OPT stop import when an unknown markup is detected
         - SKIP_UNKNOWN_MARKUP_OPT skip segments with unknown markup
         - GENERIC_UNKNOWN_MARKUP_OPT use the default markup table for segments with unknown markup
         .
         additional options which can be used
         - CLEANRTF_OPT to remove RTF inline tags (only in combination with TMX_OPT)
         - IGNORE_OPT ignore invalid segments and continue the import 
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
USHORT EqfImportMem
(
  IMPORTMEMORYDATA* pData,
  //std::shared_ptr<EqfMemory>    hSession,                // mand: Eqf session handle
  //PSZ         pszInFile,               // mand: fully qualified name of input file
  LONG        lOptions                 // opt: options for Translation Memory import
                                       // @Import Mode: TMX_OPT{CLEANRTF_OPT}, XLIFF_MT_OPT,UTF16_OPT,ANSI_OPT,ASCII_OPT(default)
                                       // @Markup Table Handling: CANCEL_UNKNOWN_MARKUP_OPT(default), SKIP_UNKNOWN_MARKUP_OPT, GENERIC_UNKNOWN_MARKUP_OPT
									                     // @Other: {CLEANRTF_OPT,IGNORE_OPT}
                                       
);


/*! \brief Organize a Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemname name of the Translation Memory
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
USHORT EqfOrganizeMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  std::shared_ptr<EqfMemory>  pMem,     // mand: name of Translation Memory
  long& reorgSegCount, 
  long& invSegCount
);

/*! \brief Start a OpenTM2 API call session
    \param phSession pointer to a buffer receiving the session handle
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
USHORT EqfStartSession
(
  PHSESSION   phSession                // mand: ptr to callers Eqf session handle variable
);


/*! \brief Get information about the last occured error
    \param hSession the session handle returned by the EqfStartSession call
    \param pusRC pointer to the buffer for last return code
    \param pszMsgBuffer pointer to a buffer receiving the error message text 
    \param usBufSize size of message buffer in number of bytes
    \returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetLastError
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  PSZ         pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
);
/*! \brief Get information about the last occured error (in UTF16 encoding)
    \param hSession the session handle returned by the EqfStartSession call
    \param pusRC pointer to the buffer for last return code
    \param pszMsgBuffer pointer to a buffer receiving the error message text (
    \param usBufSize size of message buffer in number of characters
    \returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetLastErrorW
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  wchar_t    *pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
);

/*! \brief Structure containing the segment or memory match information
     This structure is used by the EqfGetMatchLevel API for the segment from the
     document and for the memory proposal
*/
//#pragma pack( push, TM2StructPacking, 1 )
typedef struct _EQFSEGINFO
{
  WCHAR           szSource[EQF_SEGLEN];          // segment source text (UTF-16 encoded)
  WCHAR           szTarget[EQF_SEGLEN];          // segment target text (UTF-16 encoded) (empty for document segment)
  LONG            lSegNumber;                    // segment number
  CHAR            szDocument[MAX_LONGFILESPEC];  // name of document
  CHAR            szSourceLanguage[MAX_LANG_LENGTH];   // source language of segment
  CHAR            szTargetLanguage[MAX_LANG_LENGTH];   // target language of segment
  CHAR            szMarkup[MAX_FILESPEC];        // markup of segment
} EQFSEGINFO, *PEQFSEGINFO;
//#pragma pack( pop, TM2StructPacking )

//
// options for EqfGetMatchLevel API
//

// suppress generic inline tag replacement
#define NO_GENERIC_INLINETAG_REPL_OPT    0x0001

// use generic inline tag replacement function
#define USE_GENERIC_INLINETAG_REPL_OPT   0x0002

/*! \brief State/type of matches used by EqfGetMatchLevel API
*/
typedef enum _EQFMATCHSTATE
{
  REPLACE_MATCHSTATE,                            // replace match
  FUZZYREPLACE_MATCHSTATE,                       // fuzzy replace match
  FUZZY_MATCHSTATE,                              // fuzzy match 
  NONE_MATCHSTATE,                               // no match
  EXACT_MATCHSTATE,                              // exact match 
  EXACTEXACT_MATCHSTATE                          // exact-exact match (match from same document and same segment)      
} EQFMATCHSTATE, *PEQFMATCHSTATE;

// options fpr EqfCleanMatch API call

// create internal memory
#define CLEANMEM_INTERNAL_MEMORY_OPT  0x00000001 

// create external memory
#define CLEANMEM_EXTERNAL_MEMORY_OPT  0x00000002

// return when all activitiy has been completed (instead of iterative processing)
#define CLEANMEM_COMPLETE_IN_ONE_CALL_OPT 0x01000000
#define COMPLETE_IN_ONE_CALL_OPT 0x01000000


// activate logging (will append log info to the log file \EQF\LOGS\CLEANMEMORY.LOG)
#define CLEANMEM_LOGGING_OPT          0x00010000

// if specified only the best match will be written to the output memory
// if not specified the best 3 fuzzy matches will be written to the output memory
#define CLEANMEM_BESTMATCH_OPT        0x00020000

// merge into existing internal memory
#define CLEANMEM_MERGE_OPT            0x20000000

// keep duplicate exact matches and fuzzy matches in the memory
#define CLEANMEM_KEEP_DUPS_OPT       0x40000000

// rename modes
#define RENAME_FOLDER        1
#define RENAME_MEMORY        2
#define RENAME_DICTIONARY    4

// options
#define ADJUSTREFERENCES_OPT  0x04000000

// constants for API call EqfGetShortName

#define FOLDER_OBJ    1
#define MEMORY_OBJ    2
#define DICT_OBJ      3
#define DOCUMENT_OBJ  4 


/*! \brief Search segments having fuzzy memory proposals
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName name of the folder containing the searched documents
  \param pszDocuments list of documents being searched or NULL to search all documents of the folder
  \param pszOutputFile fully qualified of the file receiving the list of segments having fuzzy proposals
  \param iSearchMode search mode, one of the values UPTOSELECTEDCLASS_MODE, SELECTEDCLASSANDHIGHER_MODE, and ONLYSELECTEDCLASS_MODE
  \param iClass search class, a value from 0 to 6
  \param lOptions options for searching fuzzy segments
         - OVERWRITE_OPT overwrite any existing output file
         - MARKDIFFERENCES_OPT mark the differences between segment source and fuzzy proposal in teh output
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/

/*! \brief Lookup a segment in the memory
  \param hSession the session handle returned by the EqfStartSession call
  \param lHandle handle of a previously opened memory
  \param pSearchKey pointer to a MemProposal structure containing the searched criteria
  \param *piNumOfProposals pointer to the number of requested memory proposals, will be changed on return to the number of proposals found
  \param pProposals pointer to a array of MemProposal structures receiving the search results
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT EqfQueryMem
(
  HSESSION    hSession,    
  EqfMemory*        lHandle,          
  PMEMPROPOSAL pSearchKey, 
  int         *piNumOfProposals,
  PMEMPROPOSAL pProposals, 
  LONG        lOptions     
);


/*! \brief Update a segment in the memory
  \param hSession the session handle returned by the EqfStartSession call
  \param lHandle handle of a previously opened memory
  \param pNewProposal pointer to an MemProposal structure containing the segment data
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT EqfUpdateMem
(
  HSESSION    hSession,
  EqfMemory*   lHandle, 
  PMEMPROPOSAL pNewProposal,
  LONG        lOptions
);


/*! \brief Get the OpenTM2 language name for a ISO language identifier
\param hSession the session handle returned by the EqfStartSession call
\param pszISOLang pointer to ISO language name (e.g. en-US )
\param pszOpenTM2Lang buffer for the OpenTM2 language name
\returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetOpenTM2Lang
(
  HSESSION    hSession,
  PSZ         pszISOLang,
  PSZ         pszOpenTM2Lang, 
  bool*       pfPrefered = nullptr
);


/*! \brief Get the ISO language identifier for a OpenTM2 language name
\param hSession the session handle returned by the EqfStartSession call
\param pszOpenTM2Lang pointer to the OpenTM2 language name
\param pszISOLang pointer to a buffer for the ISO language identifier
\returns 0 if successful or an error code in case of failures
*/
USHORT EqfGetIsoLang
(
  HSESSION    hSession,
  PSZ         pszOpenTM2Lang,
  PSZ         pszISOLang
);

#endif


///end of OTMFUNC region

#endif //_tm_h_included_