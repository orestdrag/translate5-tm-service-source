/*! \file
	Description: Internal functions used by EQFQDAM.C For details see description in EQFQDAM.C

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions

#include <atomic>
#include <EQF.H>                  // General Translation Manager include file
#include "LogWrapper.h"
#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include "win_types.h"
#include <malloc.h>
#include "otm.h"

//#include <EQFQDAMI.H>             // Private QDAM defines
//#include <eqfcmpr.h>              // defines for compression/expand...
//#include <eqfchtbl.h>             // character tables
#include <time.h>
//#include "eqfevent.h"                  // event logging facility

#if defined(ASDLOGGING)
 extern FILE *hAsdLog;
 #define ASDLOG()                \
    if ( hAsdLog )               \
       fprintf( hAsdLog, "QDAM: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define ASDLOG()
#endif



#define INC_READ_COUNT
#define INC_CACHED_COUNT
#define INC_CACHED_READ_COUNT
#define INC_WRITE_COUNT
#define INC_REAL_READ_COUNT
#define INC_REAL_WRITE_COUNT

#define BTREE_REC_SIZE_V3 (16384)          // record size V3
#define BTREE_BUFFER_V3   (BTREE_REC_SIZE_V3 + 10*sizeof(USHORT)) // buffer size

#define MIN_SIZE          128          // minimum free space requested in entry
#define MAXWASTESIZE      512          // max size  to be wasted due to updates
#define MAX_LOCKREC_SIZE 4000          // max size for locked terms record
#define USERDATA_START   2046          // start of user data
#define INDEX_BUFFERS      20          // index buffers
#define NUMBER_OF_BUFFERS  20          // 20 // number of buffers to be used


// Value for chVersion in BTREE version 3
// Note: In this version the record size has been changed to 16k
#define BTREE_V3 0x03

#define MAX_READREC_CALLS 1000000L
#define ACCESSBONUSPOINTS 5000L
/**********************************************************************/
/* To check that we are opening a valid B-tree file, there is a       */
/* magic cookie stored at the beginning.  This is as follows          */
/* This value might be changed to include version identifications...  */
/**********************************************************************/

#define NTM_VERSION3       3
#define BTREE_HEADER_VALUE_TM3 "NTM"



#define IS_LEAF(x)          ((TYPE(x) & INNER_NODE) == 0)
#define IS_ROOT(x)          ((TYPE(x) & ROOT_NODE) != 0)

/**********************************************************************/
/* Defines the header of the event list either as a structure or as   */
/* a enumeration                                                      */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTLISTSTART( type, var ) \
     EVENTSTRING var[] = {
#else
   #define EVENTLISTSTART( type, var) typedef enum type {
#endif

#ifdef STRING_EVENT_IDS
   #define EVENTLISTEND( type, dummy ) { #dummy, 32000 } };
#else
   #define EVENTLISTEND( type, dummy ) dummy = 32000} type;
#endif


/**********************************************************************/
/* Macro to either define the event ID as a numeric value or to       */
/* use the event ID as a string (used by EVENTLOG program).           */
/**********************************************************************/
#ifdef STRING_EVENT_IDS
   #define EVENTDEF( id, num ) { #id, num },
#else
   #define EVENTDEF( id, num ) id = num,
#endif




static  USHORT us1BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    2,       // 00000000 00000010
                                    4,       // 00000000 00000100
                                    8,       // 00000000 00001000
                                    16,      // 00000000 00010000
                                    32,      // 00000000 00100000
                                    64,      // 00000000 01000000
                                    128,     // 00000000 10000000
                                    256,     // 00000001 00000000
                                    512,     // 00000010 00000000
                                    1024,    // 00000100 00000000
                                    2048,    // 00001000 00000000
                                    4096,    // 00010000 00000000
                                    8192,    // 00100000 00000000
                                    16384 }; // 01000000 00000000

static  USHORT us2BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    6,       // 00000000 00000110
                                    12,      // 00000000 00001100
                                    24,      // 00000000 00011000
                                    48,      // 00000000 00110000
                                    96,      // 00000000 01100000
                                    192,     // 00000000 11000000
                                    384,     // 00000001 10000000
                                    768,     // 00000011 00000000
                                    1536,    // 00000110 00000000
                                    3072,    // 00001100 00000000
                                    6144,    // 00011000 00000000
                                    12288,   // 00110000 00000000
                                    24576 }; // 01100000 00000000

static  USHORT us3BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    7,       // 00000000 00000111
                                    14,      // 00000000 00001110
                                    28,      // 00000000 00011100
                                    56,      // 00000000 00111000
                                    112,     // 00000000 01110000
                                    224,     // 00000000 11100000
                                    448,     // 00000001 11000000
                                    896,     // 00000011 10000000
                                    1792,    // 00000111 00000000
                                    3584,    // 00001110 00000000
                                    7168,    // 00011100 00000000
                                    14336,   // 00111000 00000000
                                    28672 }; // 01110000 00000000

static  USHORT us8BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    7,       // 00000000 00000111
                                    15,      // 00000000 00001111
                                    31,      // 00000000 00011111
                                    63,      // 00000000 00111111
                                    127,     // 00000000 01111111
                                    255,     // 00000000 11111111
                                    510,     // 00000001 11111110
                                    1020,    // 00000011 11111100
                                    2040,    // 00000111 11111000
                                    4080,    // 00001111 11110000
                                    8160,    // 00011111 11100000
                                    16320,   // 00111111 11000000
                                    32640 }; // 01111111 10000000





/**********************************************************************/
/* List of event locations (only use macro EVENTDEF for entries in        */
/* this list)                                                         */
/**********************************************************************/
EVENTLISTSTART( _EVENTLOC, EventLocations )
EVENTDEF( UTLERROR_LOC,              1 )
EVENTDEF( TWBSTART_LOC,              2 )
EVENTDEF( TWBSHUTDOWN_LOC,           3 )
EVENTDEF( QDAMUPDATELOCKREC_LOC,     4 )
EVENTDEF( QDAMGETUPDCOUNTER_LOC,     5 )
EVENTDEF( QDAMINCRUPDCOUNTER_LOC,    6 )
EVENTDEF( QDAMPHYSLOCK_LOC,          8 )
EVENTDEF( QDAMRECORDTODISK_LOC,      9 )
EVENTDEF( QDAMDICTCREATELOCAL_LOC,   10 )
EVENTDEF( QDAMREADRECORD_LOC,        12 )
EVENTDEF( QDAMWRITEHEADER_LOC,       13 )
EVENTDEF( QDAMDICTOPENLOCAL_LOC,     14 )
EVENTDEF( QDAMDICTCLOSELOCAL_LOC,    15 )
EVENTDEF( QDAMDICTEXACTLOCAL_LOC,    16 )
EVENTDEF( QDAMDICTSUBSTRLOCAL_LOC,   17 )
EVENTDEF( QDAMDICTEQUIVLOCAL_LOC,    18 )
EVENTDEF( QDAMDICTFIRSTLOCAL_LOC,    19 )
EVENTDEF( QDAMDICTNEXTLOCAL_LOC,     20 )
EVENTDEF( QDAMDICTPREVLOCAL_LOC,     21 )
EVENTDEF( QDAMDICTCURRENTLOCAL_LOC,  22 )
EVENTDEF( QDAMNEXT_LOC,              23 )
EVENTDEF( QDAMFIRST_LOC,             24 )
EVENTDEF( QDAMGETSZKEYPARAM_LOC,     25 )
EVENTDEF( QDAMGETSZDATA_LOC,         26 )
EVENTDEF( QDAMLOCATEKEY_LOC,         27 )
EVENTDEF( QDAMVALIDATEINDEX_LOC,     28 )
EVENTDEF( NLPBACKWARDLIST_LOC,       29  )
EVENTDEF( NLPFORWARDDLIST_LOC,       30 )
EVENTDEF( NLPTERMLIST_LOC,           31 )
EVENTDEF( NLPLOCKENTRY_LOC,          32 )
EVENTDEF( NLPDELENTRYASD_LOC,        33 )
EVENTDEF( NLPUPDENTRYASD_LOC,        34 )
EVENTDEF( NLPINSENTRYASD_LOC,        35 )
EVENTDEF( NLPPRVTERMASD_LOC,         36 )
EVENTDEF( NLPNXTTERMASD_LOC,         37 )
EVENTDEF( NLPFNDNUMBERASD_LOC,       38 )
EVENTDEF( NLPRETENTRYASD_LOC,        39 )
EVENTDEF( NLPFNDMATCHASD_LOC,        40 )
EVENTDEF( NLPFNDEQUIVASD_LOC,        41 )
EVENTDEF( NLPFNDBEGINASD_LOC,        42 )
EVENTDEF( QDAMOPENASD_LOC,           43 )
EVENTDEF( QDAMWRITERECORD_LOC,       44 )
EVENTDEF( QDAMNEWRECORD_LOC,         45 )
EVENTDEF( QDAMFREERECORD_LOC,        46 )
EVENTDEF( QDAMUPDSIGNLOCAL_LOC,      47 )
EVENTDEF( QDAMDICTFLUSHLOCAL_LOC,    48 )
EVENTDEF( QDAMSPLITNODE_LOC,         49 )
EVENTDEF( QDAMINSERTKEY_LOC,         50 )
EVENTDEF( QDAMDICTINSERTLOCAL_LOC,   51 )
EVENTDEF( QDAMCHANGEKEY_LOC,         52 )
EVENTDEF( QDAMADDTOBUFFER_LOC,       53 )
EVENTDEF( QDAMDELETEDATAFROMBUFFER_LOC, 54 )
EVENTDEF( QDAMDICTUPDATELOCAL_LOC,   55 )
EVENTDEF( QDAMDICTCOPYLOCAL_LOC,     56 )
EVENTDEF( QDAMREDUCENODE_LOC,        57 )
EVENTDEF( QDAMDICTDELETELOCAL_LOC,   58 )
EVENTDEF( QDAMREADRECORDFROMDISK_LOC,59 )
EVENTDEF( EQFNTMUPDATE_LOC,          60 )
EVENTDEF( EQFNTMINSERT_LOC,          61 )
EVENTDEF( EQFNTMGET_LOC,             62 )
EVENTDEF( EQFNTMPHYSLOCK_LOC,        63 )
EVENTDEF( TMTXREPLACE_LOC,           64 )
EVENTDEF( TOKENIZESOURCE_LOC,        65 )
EVENTDEF( TOKENIZETARGET_LOC,        66 )
EVENTDEF( ADDTOTM_LOC,               67 )
EVENTDEF( FILLCLB_LOC,               68 )
EVENTDEF( DETERMINETMRECORD_LOC,     69 )
EVENTDEF( UPDATETMRECORD_LOC,        70 )
EVENTDEF( COMPAREPUTDATA_LOC,        71 )
EVENTDEF( ADDTMTARGET_LOC,           72 )
EVENTDEF( REPLACETMTARGET_LOC,       73 )
EVENTDEF( TMTXGET_LOC,               74 )
EVENTDEF( GETEXACTMATCH_LOC,         75 )
EVENTDEF( EXACTTEST_LOC,             76 )
EVENTDEF( FILLMATCHTABLE_LOC,        77 )
EVENTDEF( GETFUZZYMATCH_LOC,         78 )
EVENTDEF( FUZZYTEST_LOC,             79 )
EVENTDEF( FILLMATCHENTRY_LOC,        80 )
EVENTDEF( NTMREADWRITESEGMENT_LOC,   81 )
EVENTDEF( NTMLOCKTM_LOC,             82 )
EVENTDEF( NTMCHECKFORUPDATES_LOC,    83 )
EVENTDEF( TMOPEN_LOC,                84 )
EVENTDEF( TMCLOSE_LOC,               85 )
EVENTDEF( TMREPLACE_LOC,             86 )
EVENTDEF( TMGET_LOC,                 87 )
EVENTDEF( TMTXOPEN_LOC,              88 )
EVENTDEF( EQFNTMOPEN_LOC,            89 )
EVENTDEF( EQFNTMCLOSE_LOC,           90 )
EVENTDEF( EQFNTMSIGN_LOC,            91 )
EVENTDEF( UTILSEXIT_LOC,             92 )
EVENTDEF( QDAMLOCSUBSTR_LOC,         93 )
EVENTDEF( QDAMGETSZKEY_LOC,          94 )
EVENTDEF( QDAMCHECKCHECKSUM_LOC,     95 )
EVENTDEF( QDAMFINDRECORD_LOC,        96 )
EVENTDEF( QDAMSETRECDATA_LOC,        97 )
EVENTDEF( EQFDA_LOC,                 98 )
EVENTDEF( DICTCLEANUP_LOC,           99 )
EVENTDEF( EQFDACLOSE_LOC,           100 )
EVENTDEF( ASDOPEN_LOC,              101 )
EVENTDEF( ASDCLOSE_LOC,             102 )
EVENTDEF( ASDTRANSLATE_LOC,         103 )
EVENTDEF( TMUPDSEG_LOC,             104 )
EVENTDEF( DOCEXPCOMMAND_LOC,        105 )
EVENTDEF( DOCUMENTUNLOAD_LOC,       106 )
EVENTDEF( UNLOADDOCUMENT_LOC,       107 )
EVENTDEF( EQFUNSEGREVMARK_LOC,      108 )
EVENTDEF( UNSEGMENT_LOC,            109 )
EVENTDEF( ASDLISTINDEX_LOC,         110 )
EVENTDEF( NTMGETIDFROMNAME_LOC,     111 )
EVENTDEF( TWBEXCEPTION_LOC,         112 )
EVENTLISTEND( TEVENTLOC, DUMMY_LOC )



/**********************************************************************/
/* defines for the tersing to be used....                             */
/**********************************************************************/
#define BTREE_TERSE_HUFFMAN  1
#define BTREE_TERSE_LZSSHMAN 2
#define  QDAM_TERSE_FLAG   0x8000
#define  QDAM_TERSE_FLAGL   0x80000000L



/**********************************************************************/
/* Group/Area of events                                               */
/**********************************************************************/
EVENTLISTSTART( _EVENTGROUP, EventGroups )
EVENTDEF( NONE_GROUP,           0 )
EVENTDEF( TM_GROUP,             1 )
EVENTDEF( DICT_GROUP,           2 )
EVENTDEF( ANALYSIS_GROUP,       3 )
EVENTDEF( MORPH_GROUP,          4 )
EVENTDEF( DOC_GROUP,            5 )
EVENTDEF( GENERAL_GROUP,        6 )
EVENTDEF( DB_GROUP,             7 )
EVENTLISTEND( TEVENTGROUP, DUMMY_GROUP )

#define  MINFREEKEYS        3
// minimum keys to be in a record - otherwise try to reduce the node
#define  MIN_KEY            MINFREEKEYS
// lock a record for fixing it in the memory
#define  BTREELOCKRECORD(x)  ((x)->fLocked=TRUE)
// test if a record is locked
#define  BTREETESTLOCKRECORD(x)  ((x)->fLocked)
// unlock a (previously locked) record
#define  BTREEUNLOCKRECORD(x)  ((x)->fLocked=FALSE)

/**********************************************************************/
/* List of event classes                                              */
/**********************************************************************/
EVENTLISTSTART( _EVENTCLASS, EventClasses )
EVENTDEF( NA_EVENT,             1 )
EVENTDEF( FUNCENTRY_EVENT,      2 )
EVENTDEF( REFRESH_EVENT,        5 )
EVENTDEF( INTFUNCFAILED_EVENT,  6 )
EVENTDEF( MSGBOX_EVENT,         7 )
EVENTDEF( INVOPERATION_EVENT,   8 )
EVENTDEF( READ_EVENT,           9 )
EVENTDEF( WAIT_EVENT,           10 )
EVENTDEF( STATE_EVENT,          11 )
EVENTDEF( FUNCEXIT_EVENT,       12 )
EVENTDEF( CHECKSUM_EVENT,       13 )
EVENTLISTEND( TEVENTCLASS, DUMMY_CLASS )



/* Access Macros */
#define TYPE(x)          ((x)->contents.header.chType)
#define LEAF_NODE        0x00
#define INNER_NODE       0x01
#define ROOT_NODE        0x02
#define DATA_NODE        0x04
#define DATA_NEXTNODE    0x08
#define DATA_KEYNODE     0x10



/**********************************************************************/
/* Event types                                                        */
/**********************************************************************/
#ifndef STRING_EVENT_IDS
typedef enum
{
  ERROR_EVENT,                         // event is caused by an error condition
  INFO_EVENT,                          // informational events (e.g. status events)
  EQFDEBUG_EVENT                       // only logged when debug version is build
} EVENTTYPE;
#endif



#define MAX_LIST           20          // number of recently used records
#define MIN_SIZE          128          // minimum free space requested in entry
#define MAXWASTESIZE      512          // max size  to be wasted due to updates
#define MAX_LOCKREC_SIZE 4000          // max size for locked terms record
#define USERDATA_START   2046          // start of user data
#define INDEX_BUFFERS      20          // index buffers
#define NUMBER_OF_BUFFERS  20          // 20 // number of buffers to be used


#define RESET_VALUE       -3

#define RETRY_COUNT        5           // number of retries for BTREE_IN_USE condition

// max time [ms] to wait for resources currently in use
#define MAX_WAIT_TIME     100

// max number of retries for resources currently in use
#define MAX_RETRY_COUNT    30


/**********************************************************************/
/* Event macros                                                       */
/**********************************************************************/
BOOL UtlEvent2
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC,                      // the event return code or error code
  SHORT sGroup,                        // the event group
  PSZ   pszAddData                     // additional data for event (string)
);

BOOL UtlEvent
(
  SHORT sEventType,                    // type of the event
  SHORT sEventID,                      // the event identifier
  SHORT sEventClass,                   // the event class
  SHORT sEventRC                       // the event return code or error code
);

#define NO_ERREVENTS 
#define NO_INFOEVENTS 
#define NO_DEBUGEVENTS 


#ifndef STRING_EVENT_IDS
  #ifndef NO_ERREVENTS
    #define ERREVENT( id, class, rc ) \
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)) \
      T5LOG( T5ERROR) << ":: id = " << id << "(" << GETNAME(id) << "); class = " <<  class << "(" << GETNAME(class) \
          <<"); rc = " <<  rc ;
  #else
    #define ERREVENT( id, class, rc )
  #endif

  #ifndef NO_ERREVENTS
    #define ERREVENT2( id, class, rc, gr, str ) \
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)) \
      T5LOG( T5ERROR) << ":: id = " << id << "(" << GETNAME(id) << "); class = " <<  class << "(" << GETNAME(class) \
          <<"); rc = " <<  rc <<" (" <<GETNAME(rc) << "); str = " << str << "; gr = " << gr << "(" << GETNAME(gr) << ")"; 
      //UtlEvent2( ERROR_EVENT, id, class, rc, gr, str )
  #else
    #define ERREVENT2( id, class, rc, gr, str )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT( id, class, rc ) \
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)) \
    T5LOG( T5INFO) << ":: id = " << id << "; class = " <<  class << "; rc = " <<  rc ;
      //UtlEvent( INFO_EVENT, id, class, rc )
  #else
    #define INFOEVENT( id, class, rc )
  #endif

  #ifndef NO_INFOEVENTS
    #define INFOEVENT2( id, class, rc, gr, str ) \
      if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)) \
      T5LOG( T5INFO) << ":: id = " << id << "(" << GETNAME(id) << "); class = " <<  class << "(" << GETNAME(class) \
          <<"); rc = " <<  rc <<" (" <<GETNAME(rc) << "); str = " << str << "; gr = " << gr << "(" << GETNAME(gr) << ")"; 
      //UtlEvent2( INFO_EVENT, id, class, rc, gr, str )
  #else
    #define INFOEVENT2( id, class, rc, gr, str )
  #endif

  #ifndef _DEBUG
    //#define NO_DEBUGEVENTS
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT( id, class, rc ) \
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)) \
    T5LOG( T5DEVELOP)<< ":: id = " << (id) << "; class = " << class << "; rc = " <<rc;\
      //UtlEvent( EQFDEBUG_EVENT, id, class, rc )
  #else
    #define DEBUGEVENT( id, class, rc )
  #endif

  #ifndef NO_DEBUGEVENTS
    #define DEBUGEVENT2( id, class, rc, gr, str ) \
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)) \
      T5LOG( T5DEVELOP) <<  ":: id = "<< id <<  "; class = " << class << \
      "; rc = "<<rc<< "; str = "<< str<< "; gr = "<<gr;\
      //UtlEvent2( EQFDEBUG_EVENT, id, class, rc, gr, str )
  #else
    #define DEBUGEVENT2( id, class, rc, gr, str )
  #endif
#endif

typedef enum _DICTCMD
{
  QDAMDICTOPEN,               // dictionary open
  QDAMDICTCREATE,             // dictionary create
  QDAMDICTCLOSE,              // dictionary close
  QDAMDICTSIGN,               // return signature record
  QDAMDICTUPDSIGN,            // update signature record
  QDAMDICTSUBSTR,             // find term containing substring
  QDAMDICTEQUIV,              // find equivalent term
  QDAMDICTEXACT,              // find exact term
  QDAMDICTNEXT,               // return next term
  QDAMDICTPREV,               // return prev term
  QDAMDICTCURR,               // return current term
  QDAMDICTINSERT,             // insert term
  QDAMDICTUPDATE,             // update term
  QDAMDICTDELETE,             // delete term
  QDAMDICTFLUSH,              // resynchronize dict.
  QDAMDICTCOPY,               // copy dict
  QDAMDICTNUMENTRIES,         // find number of terms in dict
  QDAMDICTFIRST,              // position at first entry
  QDAMDICTLOCKDICT,           // lock the dictionary
  QDAMDICTLOCKENTRY,          // lock the entry
  QDAMDICTCLOSEORGANIZE,      // end the organize
  QDAMDICTUPDTIME             // get update status
} DICTCMD;



  #define STARTOFDATA( pBT, pData ) \
    (PBYTE)pData + sizeof(ULONG) 

  #define SETDATALENGTH( pBT, pData, ulLen ) \
            *((PULONG)(pData)) = ulLen; 




static BTREEHEADRECORD header; // Static buffer for database header record


///* Maximum size (entries) of the lookup table (it is a USHORT).
#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x0FFFF
//#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x00FF
/* Initial size (entries) of the lookup table */
#define MIN_NUMBER_OF_LOOKUP_ENTRIES 32

/**********************************************************************/
/* typedef used for all vital information on our new approach...      */
/* NOTE: this structure is limited to COLLATE_SIZE, which is 256      */
/**********************************************************************/


//#define NTMNEXTKEY( pBT )  ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulNextKey
//#define NTMSTARTKEY( pBT ) ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulStartKey

/**********************************************************************/
/* 'Magic word' for record containing locked terms                    */
/**********************************************************************/
static CHAR szLOCKEDTERMSKEY[] = "0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR_W szLOCKEDTERMSKEYW[] = L"0x010x020x03LOCKEDTERMS0x010x020x030x040x050x060x070x080x09";
static CHAR szLockRec[MAX_LOCKREC_SIZE]; // buffer for locked terms


/**********************************************************************/
/* default collating sequence as returned by DosGetCollating for a    */
/* ASCII 850 code page..                                              */
/**********************************************************************/
UCHAR chDefCollate[] = {
     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
    96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90, 123, 124, 125, 126, 127,
    67,  85,  69,  65,  65,  65,  65,  67,  69,  69,  69,  73,  73,  73,  65,  65,
    69,  65,  65,  79,  79,  79,  85,  85,  89,  79,  85,  79,  36,  79, 158,  36,
    65,  73,  79,  85,  78,  78, 166, 167,  63, 169, 170, 171, 172,  33,  34,  34,
   176, 177, 178, 179, 180,  65,  65,  65, 184, 185, 186, 187, 188,  36,  36, 191,
   192, 193, 194, 195, 196, 197,  65,  65, 200, 201, 202, 203, 204, 205, 206,  36,
    68,  68,  69,  69,  69,  73,  73,  73,  73, 217, 218, 219, 220, 221,  73, 223,
    79,  83,  79,  79,  79,  79, 230, 232, 232,  85,  85,  85,  89,  89, 238, 239,
   240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

/**********************************************************************/
/* global struct containing struct. for open dicts ...                */
/**********************************************************************/
//QDAMDICT QDAMDict[MAX_NUM_DICTS];

/**********************************************************************/
/* Table for punctuation detection                                    */
/* The following characters are treated as punctuation and will be    */
/* ignored for word recognition:                                      */
/* .-/[]_                                                             */
/*                                                                    */
/* Users: EQFMORPH.C(MorphWordRecognition)                            */
/*                                                                    */
/* Note:  The character space 0x20 may not be included in this table  */
/*        otherwise the word recognition will fail                    */
/**********************************************************************/
static BOOL fIsPunctuation[256] = {
/*        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F */
/* 0. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 1. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 2. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
/* 3. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 4. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 5. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
/* 6. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 7. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 8. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 9. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* A. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* B. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* C. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* D. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* E. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* F. */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct _STENCODEBITS
{
  USHORT usLen;
  USHORT usVal;
} STENCODEBITS;

// defines for retries in case of BTREE_IN_USE conditions
#define MAX_RETRY_COUNT 30
#define MAX_WAIT_TIME   100
/**********************************************************************/
/*  encoding sequence used                                            */
/*  The following should never be changed......                       */
/*  The general structure used is an algorithm based on HUFFMANN      */
/*  but working with fixed distributions.                             */
/*                                                                    */
/* Algorithm:                                                         */
/*   The algorithm is a variable length based algorithm, varying in   */
/* length from 3 bits to 11 bits.                                     */
/* The most frequent 15 characters will be encoded in 3 to 5 bits.    */
/* The bit sequence used are listed in the following table.           */
/* All others are encoded in 11 bits, i.e. '000' plus the character.  */
/*                                                                    */
/* The last three tables are used to allow fast access to determine   */
/* the encode values to allow decomposition.                          */
/**********************************************************************/
//                                       Leng  Value   Bits
static STENCODEBITS stEncodeBits[16] = { {3,0},     // 000
                                         {3,1},     // 001
                                         {3,2},     // 010
                                         {3,3},     // 011
                                         {4,8},     // 1000
                                         {4,9},     // 1001
                                         {4,12},    // 1100
                                         {4,13},    // 1101
                                         {5,20},    // 10100
                                         {5,21},    // 10101
                                         {5,22},    // 10110
                                         {5,23},    // 10111
                                         {5,28},    // 11100
                                         {5,29},    // 11101
                                         {5,30},    // 11110
                                         {5,31}};   // 11111

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#define PREV(x)             ((x)->contents.header.usPrevious)
#define NEXT(x)             ((x)->contents.header.usNext)
#define PARENT(x)           ((x)->contents.header.usParent)
#define RECORDNUM(x)        ((x)->usRecordNumber)
#define OCCUPIED(x)         ((x)->contents.header.usOccupied)
#define FILLEDUP(x)         ((x)->contents.header.usFilled)

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMKeyCompare      Generic compare function
//------------------------------------------------------------------------------
// Function call:     QDAMKeyCompare( PBTREE,PCHAR, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       This is the generic compare function used
//                    for comparision
//                    It uses the Maptable provided for lower/upercase mapping
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to tree structure
//                    PCHAR                  first key
//                    PCHAR                  second key
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0   keys are equal
//                    <> keys are unequal
//
//------------------------------------------------------------------------------
// Function flow:     while character available in key1
//                      case map characters and find difference
//                      if different then
//                        go to next character touple
//                      else
//                        return collating difference
//                      endif
//                    wend
//                    get difference for last comparison and return collat.diff
//                    return difference
//------------------------------------------------------------------------------
SHORT QDAMKeyCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2                        // pointer to second key
)
{
  SHORT   sDiff;
  CHAR_W  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREE    pBT = pBTIda;
  PBYTE  pCollate = nullptr;// (PBYTE)pBT->chCollate;    // pointer to collating sequence
  PSZ_W  pbKey1 = (PSZ_W) pKey1;
  PSZ_W  pbKey2 = (PSZ_W) pKey2;

  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    while ( (c = *pbKey2) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey2++;
    } /* endwhile */
    while ( (c = *pbKey1) == ' ' || fIsPunctuation[(BYTE)c] )
    {
      pbKey1++;
    } /* endwhile */
// @@ TODO: check collating sequence and fIsPunctuation

    sDiff = *(pCollate+(BYTE)c) - *(pCollate + (BYTE)*pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pCollate + (BYTE)c) - *(pCollate + (BYTE)*pbKey2));
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMTerseInit  Initialize Tersing                        |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMTerseInit( PBTREE, PUCHAR );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This call will build the compression table from          |
//|                   passed frequency table                                   |
//|                   A decode table is built as well.                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE           pointer to btree structure              |
//|                   PUCHAR           pointer to frequent character table     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     init encoding scheme                                     |
//|                   use the first 15 characters of the passed byte aray to   |
//|                     build encoding scheme                                  |
//|                   use the first 15 characters to build decoding scheme     |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

VOID QDAMTerseInit
(
   PBTREE  pBTIda,                               // pointer to btree
   PUCHAR  pComp                                 // pointer to comp table
)
{
   USHORT usI;                                  // index
   PBTREE    pBT = pBTIda;

   /*******************************************************************/
   /* init values                                                     */
   /*******************************************************************/
   memset( pBT->chEncodeVal, (BYTE) stEncodeBits[0].usVal, COLLATE_SIZE);
   memset( pBT->bEncodeLen, (BYTE) stEncodeBits[0].usLen, COLLATE_SIZE);
   /*******************************************************************/
   /* use the 15 first characters to build the encoding sequence      */
   /*******************************************************************/
   for ( usI = 0; usI < 15; usI++ )
   {
     pBT->bEncodeLen[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usLen;
     pBT->chEncodeVal[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usVal;
     pBT->chDecode[ stEncodeBits[ usI+1 ].usVal ] = *(pComp+usI);
   } /* endfor */

   return;
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMKeyCompare      Generic compare function              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMKeyCompare( PBTREE,PULONG,PULONG);                    |
//+----------------------------------------------------------------------------+
//|Description:       This is the generic compare function used                |
//|                   for comparision                                          |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to tree structure         |
//|                   PULONG                 first key                         |
//|                   PULONG                 second key                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0   keys are equal                                       |
//|                   <> keys are unequal                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     return (SHORT) (*pulKey1 - *pulKey2)                     |
//+----------------------------------------------------------------------------+

SHORT NTMKeyCompare
(
    PVOID  pBTIda,                     // pointer to tree structure
    PVOID  pulKey1,                    // pointer to first key
    PVOID  pulKey2                     // pointer to second key
)
{
  TMWCHAR  ulKey1 = *((PWCHAR)pulKey1);
  TMWCHAR  ulKey2 = *((PWCHAR)pulKey2);
  SHORT  sRc;
  pBTIda;                              // avoid compiler warnings

  if (ulKey1 > ulKey2 )
  {
    sRc = 1;
  }
  else if (ulKey1 < ulKey2 )
  {
    sRc = -1;
  }
  else
  {
    sRc = 0;
  } /* endif */
  return sRc;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteHeader       Write initial header
//------------------------------------------------------------------------------
// Function call:     QDAMWriteHeader( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Writes the first record of the (index) file so that
//                    subsequent accesses know what the index is like.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     init initial record (describing the disk)
//                    position ptr to begin of file
//                    write record to disk
//                    check if disk is full
//                    if so, set sRC = BTREE_WRITE_ERROR
//
//------------------------------------------------------------------------------
 SHORT BTREE::QDAMWriteHeader(){
  SHORT  sRc=0;
  BTREEHEADRECORD HeadRecord;          // header record

  DEBUGEVENT2( QDAMWRITEHEADER_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, "" );

  // Write the initial record (describing the index) out to disk
  memset( &HeadRecord, '\0', sizeof(BTREEHEADRECORD));
  memcpy(HeadRecord.chEQF,chEQF, sizeof(chEQF));
  HeadRecord.usFirstNode = usFirstNode;
  HeadRecord.usFirstLeaf = usFirstLeaf;
  HeadRecord.usFreeKeyBuffer = usFreeKeyBuffer;
  HeadRecord.usFreeDataBuffer = usFreeDataBuffer;
  HeadRecord.usFirstDataBuffer = usFirstDataBuffer;  // first data buffer
  HeadRecord.fOpen  = fOpen;                    // open flag set
  memcpy( HeadRecord.DataRecList, DataRecList, MAX_LIST*sizeof(HeadRecord.DataRecList[0]) );
  HeadRecord.fTerse = (EQF_BOOL) fTerse;
  memcpy( HeadRecord.chCollate, &chCollate, sizeof(chCollate) );
  memcpy( HeadRecord.chCaseMap, chCaseMap, COLLATE_SIZE );
  memcpy( HeadRecord.chEntryEncode, chEntryEncode, ENTRYENCODE_LEN );

  /********************************************************************/
  /* Update usNextFreeRecord field of Header and indicate that the    */
  /* field is valid by assigning BTREE_V1 to the bVersion field.      */
  /********************************************************************/
  HeadRecord.usNextFreeRecord = usNextFreeRecord;
  HeadRecord.Flags.f16kRec  = TRUE;
  
  // check if disk is full
  sRc = fb.Write((PVOID) &HeadRecord, sizeof(BTREEHEADRECORD), FILE_BEGIN);

  return sRc;
}


#define ERROR_LOCK_VIOLATION        33
#define ERROR_DRIVE_LOCKED          108 /* drive locked by another process */
#define ERROR_OPEN_FAILED           110 /* open/created failed due to */
                        /* explicit fail command */
#define ERROR_VC_DISCONNECTED       240


// Convert Dos... return codes to BTREE return codes
SHORT QDAMDosRC2BtreeRC
(
  SHORT sDosRC,                        // Dos return code
  SHORT sDefaultRC,                    // RC for default case
  USHORT usOpenFlags                   // open flags of database
)
{

   SHORT sRc = 0;                           // converted return code
  switch ( sDosRC )
  {
     case  NO_ERROR:
       sRc = NO_ERROR;
      break;
    case  ERROR_INVALID_DRIVE:
      sRc = BTREE_INVALID_DRIVE;
      break;
    case  ERROR_OPEN_FAILED :
      sRc = BTREE_OPEN_FAILED;
      break;
    case  ERROR_NETWORK_ACCESS_DENIED:
    case  ERROR_VC_DISCONNECTED:
      sRc = BTREE_NETWORK_ACCESS_DENIED;
      break;
    case ERROR_ACCESS_DENIED:
      sRc = BTREE_ACCESS_ERROR;
      break;
    case ERROR_DRIVE_LOCKED:
    case ERROR_INVALID_ACCESS:
      sRc = BTREE_ACCESS_ERROR;
      break;
    case ERROR_FILE_NOT_FOUND:
      sRc = BTREE_FILE_NOTFOUND;
      break;
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
      sRc = BTREE_IN_USE;
      break;
    default :
      sRc = sDefaultRC;
      break;
  } /* endswitch */

  return( sRc );
} /* end of function QDAMDosRC2BtreeRC  */


SHORT BTREE::QDAMWRecordToDisk_V3
(
   PBTREEBUFFER_V3  pBuffer               // pointer to buffer to write
)
{  
  LONG  lOffset = RECORDNUM(pBuffer) * (long)BTREE_REC_SIZE_V3; // offset in file
  ULONG ulNewOffset;                      // new file pointer position

  INC_REAL_WRITE_COUNT;

 SHORT sRc = fb.Write((PVOID) &pBuffer->contents, BTREE_REC_SIZE_V3,lOffset) ;

  if ( sRc )
  {
    sRc = (sRc == ERROR_DISK_FULL) ? BTREE_DISK_FULL : BTREE_WRITE_ERROR;
    fCorrupted = TRUE;                     // indicate corruption
    ERREVENT2( QDAMRECORDTODISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetUpdCounter       Get database update counter
//------------------------------------------------------------------------------
// Function call:     QDAMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );
//------------------------------------------------------------------------------
// Description:       Get one or more of the the database update counters
//                    from the dummy/locked terms file
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PLONG                  ptr to buffer for update counter
//                    SHORT                  index of requested update counter
//                    SHORT                  number of counters requested
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_READ_RROR   read error from disk
//
//------------------------------------------------------------------------------
// Function flow:     read update counter from dummy file
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecordFromDisk   Read record from disk
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecordFromDisk( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record from disk
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
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most
//                    performance critical routines in the module.
//
//------------------------------------------------------------------------------
// Function flow:     allocate memory for a new buffer and set pBuffer
//                    if still okay
//                      position in file and set sRc;
//                      if still okay then
//                        read buffer from disk, set sRc;
//                      endif
//                      if still okay then
//                        if new record then
//                          init header information
//                        endif
//                        set pointer to next buffer to be looked at
//                      else
//                        set BTREE_READ_ERROR
//                      endif
//------------------------------------------------------------------------------
SHORT BTREE::QDAMReadRecordFromDisk_V3
(
   USHORT         usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL           fNewRec              // allow new records flag
)
{
  //int    iNumBytesRead = 0;                   // number of bytes read
  PBTREEHEADER pHeader;                          // pointer to header
  LONG     lOffset;                              // file offset to be set
  ULONG    ulNewOffset;                          // new position
  PBTREEBUFFER_V3   pBuffer = NULL;
  SHORT    sRc = 0;                             // return code
  PLOOKUPENTRY_V3 pLEntry = NULL;
  DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, "" );
  int resRead = 0;
  
  /********************************************************************/
  /* Allocate space for a new buffer and let pBuffer point to it      */
  /********************************************************************/
  if ( !LookupTable_V3 || ( usNumber >= usNumberOfLookupEntries ))
  {
    SET_AND_LOG(sRc,BTREE_LOOKUPTABLE_CORRUPTED);
  }
  else
  {
    pLEntry = LookupTable_V3 + usNumber;

    /* Safety-Check: is the lookup table entry (ptr. to buffer) for the
                     record to read NULL ? */
    if ( pLEntry->pBuffer )
    {
      /* ptr. isn't NULL: this should never occur */
      SET_AND_LOG(sRc, BTREE_LOOKUPTABLE_CORRUPTED);
    }
    else
    {
      /* Allocate memory for a buffer */
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, (LONG) BTREE_BUFFER_V3 , NOMSG );
      if ( pLEntry->pBuffer )
      {
        (usNumberOfAllocatedBuffers)++;
        pBuffer = pLEntry->pBuffer;
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    INC_REAL_READ_COUNT;

    // Read the record in to the buffer space
    // Mark the next buffer for future allocations
    if ( !sRc )
    {
      lOffset = ((LONG) usNumber) * BTREE_REC_SIZE_V3;

      DEBUGEVENT2( QDAMREADRECORDFROMDISK_LOC, READ_EVENT, usNumber, DB_GROUP, "" );
      resRead = fb.Read((PVOID) &pBuffer->contents, BTREE_REC_SIZE_V3, lOffset);
    } /* endif */

    if ( !sRc )
    {
      pBuffer->usRecordNumber = usNumber;
//    pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
      *ppReadBuffer = pBuffer;
      if ( resRead == FilesystemHelper::FILEHELPER_WARNING_FILE_IS_SMALLER_THAN_REQUESTED)
      {
        /*************************************************************/
        /* we are at the end of the file                             */
        /*************************************************************/
        if ( fNewRec )                // new record mode ???
        {
          memset (&pBuffer->contents, 0, sizeof(BTREERECORD_V3));
          // init this record
          pHeader = &(pBuffer->contents.header);
          pHeader->usOccupied = 0;
          pHeader->usNum = usNumber;
          pHeader->usFilled = sizeof(BTREEHEADER );
          pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );
//        pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
           
          sRc = QDAMWRecordToDisk_V3( pBuffer);

          if (! sRc )
          {
            sRc = QDAMWriteHeader();
          } /* endif */
//         pBuffer->ulCheckSum = QDAMComputeCheckSum( pBuffer );
        }
        else
        {
          ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INVOPERATION_EVENT, 4, DB_GROUP, "" );
          T5LOG(T5ERROR) << "EQF9998: Write of new QDAM record w/o fNewRec set!\nLoc=READRECORDFROMDISK/3\nInternal Error";
          sRc = BTREE_READ_ERROR;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Free record buffer in case of errors so record will be re-read   */
  /* during retry operations                                          */
  /********************************************************************/
  if ( sRc )
  {
    if ( (pLEntry != NULL) && (pLEntry->pBuffer != NULL) )
    {
      UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
      (usNumberOfAllocatedBuffers)--;
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMREADRECORDFROMDISK_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */
  return sRc;
}

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
SHORT  BTREE::QDAMReadRecord_V3
(
   USHORT  usNumber,
   PBTREEBUFFER_V3 * ppReadBuffer,
   BOOL    fNewRec
)
{
  USHORT   i;
  SHORT    sRc = 0;                  // return code
  BOOL     fMemOK = FALSE;
  PLOOKUPENTRY_V3  pLEntry;
  PACCESSCTRTABLEENTRY  pACTEntry;
  DEBUGEVENT2( QDAMREADRECORD_LOC, FUNCENTRY_EVENT, usNumber, DB_GROUP, "" );

  *ppReadBuffer = NULL;
  INC_READ_COUNT;

  /********************************************************************/
  /* If Lookup-Table is too small allocate additional memory for it   */
  /********************************************************************/
  if ( usNumber >= MAX_NUMBER_OF_LOOKUP_ENTRIES )
  {
    /* There is no room for this record number in the lookup table */
    sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
  }
  else if ( !LookupTable_V3 || !AccessCtrTable )
  {
    sRc = BTREE_LOOKUPTABLE_NULL;
  }
  else
  {
    if ( usNumber >= usNumberOfLookupEntries )
    {
      /* The lookup-table entry for the record to read doesn't exist */
      /* Reallocate memory for LookupTable and AccessCounterTable */
      fMemOK = UtlAlloc( (PVOID *)&LookupTable_V3,
              (LONG) usNumberOfLookupEntries * sizeof(LOOKUPENTRY_V3),
              (LONG) (usNumber + 10) * sizeof(LOOKUPENTRY_V3), NOMSG );
      if ( fMemOK==TRUE )
      {
        fMemOK = UtlAlloc( (PVOID *)&AccessCtrTable,
                (LONG) usNumberOfLookupEntries * sizeof(ACCESSCTRTABLEENTRY),
                (LONG) (usNumber + 10) * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
        if ( fMemOK==TRUE )
        {
          usNumberOfLookupEntries = usNumber + 10;
        }
        else
        {
          sRc = BTREE_NO_ROOM;
        } /* endif */
      }
      else
      {
        sRc = BTREE_NO_ROOM;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /******************************************************************/
    /* If record is in memory set ppReadBuffer                        */
    /* else call QDAMReadRecordFromDisk to read it into memory        */
    /******************************************************************/
    pLEntry = LookupTable_V3 + usNumber;

    if ( pLEntry->pBuffer )
    {
      /* Safety-Check: is rec.number = number of rec. to read ? */
      if ( (pLEntry->pBuffer)->usRecordNumber == usNumber )
      {
        /* Record is already in memory */
        *ppReadBuffer = pLEntry->pBuffer;
      }
      else
      {
        /* This should never occur ! */
        SET_AND_LOG(sRc,BTREE_LOOKUPTABLE_CORRUPTED) <<"This should never occur";
      } /* endif */
    }
    else
    {
      /* Record isn't in memory -> read it from disk */
      sRc =  QDAMReadRecordFromDisk_V3( usNumber, ppReadBuffer, fNewRec );
      pACTEntry=AccessCtrTable + usNumber;
      pACTEntry->ulAccessCounter = 0L;
    } /* endif */
  } /* endif */

  /****************************************************************************/
  /* If #calls of QDAMReadRecord is greater than MAX_READREC_CALLS:           */
  /*   write unlocked records with access counter < MAX_READREC_CALLS to disk */
  /*   (if fNeedToWrite is set) and free the allocated memory                 */
  /*   set #calls of QDAMReadRecord = 0                                       */
  /* else increment #calls of QDAMReadRecord                                  */
  /****************************************************************************/
  if ( !sRc )
  {
    if ( ulReadRecCalls >= MAX_READREC_CALLS )
    {
      for ( i=0; !sRc && (i < usNumberOfLookupEntries); i++ )
      {
        pLEntry = LookupTable_V3 + i;
        pACTEntry = AccessCtrTable + i;
        if ( pLEntry->pBuffer && !((pLEntry->pBuffer)->fLocked) && (i!=usNumber)
             && (pACTEntry->ulAccessCounter<MAX_READREC_CALLS) )
        {
            /* write buffer and free allocated space */
            //if ( (pLEntry->pBuffer)->fNeedToWrite )
            {
              sRc = QDAMWRecordToDisk_V3( pLEntry->pBuffer);
            } /* endif */
            if ( !sRc )
            {
              UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L , NOMSG );
              usNumberOfAllocatedBuffers--;
            } /* endif */
        } /* endif */

        if ( pACTEntry->ulAccessCounter<MAX_READREC_CALLS )
        {
          pACTEntry->ulAccessCounter = 0L;
        }
        else
        {
          pACTEntry->ulAccessCounter -= MAX_READREC_CALLS;
        } /* endif */
      } /* endfor */
      ulReadRecCalls = 0L;
    }
    else
    {
      ulReadRecCalls++;
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    /* Increase the access counter for the record just read by  ACCESSBONUSPOINTS */
    pACTEntry=AccessCtrTable + usNumber;
    pACTEntry->ulAccessCounter += ACCESSBONUSPOINTS;
  } /* endif */

  return ( sRc );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       add dictionary to our global list of open dicts
//------------------------------------------------------------------------------
// Parameters:        PSZ    pName,           name of dictionary
//                    PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_MAX_DICTS     maximum of dictionaries exceeded
//------------------------------------------------------------------------------
// Function flow:     loop thru global struct to find open slot.
//                    if too many open dicts
//                      set return code to BTREE_MAX_DICTS
//                    else
//                      copy significant data into free slot
//                    endif
//                    return success
//
//------------------------------------------------------------------------------
SHORT
QDAMAddDict
(
  PSZ  pName,
  PBTREE  pBTIda
)
{
  return 0;
} /* end of function QDAMAddDict */



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockDictLocal
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       try to lock the dictionary for exclusive use
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIda,               pointer to ida
//                    BOOL    fLock                 lock or unlock
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0   success
//                    BTREE_DICT_LOCKED   dictionary could not be locked
//------------------------------------------------------------------------------
// Function flow:      if locking
//                        if open count > 1  then
//                          set return code to BTREE_DICT_LOCKED;
//                        else
//                          set lock flag
//                        endif
//                     else
//                        set lock flag as passed as input parameter
//                     endif
//                     return success
//------------------------------------------------------------------------------
SHORT
BTREE::QDAMDictLockDictLocal
(
  BOOL    fLock                        // lock or unlock
)
{
  SHORT  sRc = 0;
  T5LOG(T5DEVELOP) << "TEMPORARY COMMENTED";
  #ifdef TEMPORARY_COMMENTED
  /********************************************************************/
  /* check if open count > 1, then no exclusive use possible any more */
  /********************************************************************/
  if ( fLock )
  {
    if ( QDAMDict[ usDictNum ].usOpenCount > 1 )
    {
      sRc = BTREE_DICT_LOCKED;
    }
    else
    {
      QDAMDict[ usDictNum ].fDictLock = fLock;
    } /* endif */
  }
  else
  {
    QDAMDict[ usDictNum ].fDictLock = fLock;
  } /* endif */
  #endif

  return sRc;
} /* end of function QDAMDictLockDictLocal */

#define CHECKPBTREE(pBTIda, sRc)                  \
   if ( !pBTIda )                                 \
   {                                              \
     sRc = BTREE_INVALID;                         \
   }                                              \
   else                                           \
   {                                              \
     sRc = pBTIda ? 0 : BTREE_INVALID;    \
   } /* endif */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMRemoveDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       open count of current dict
//------------------------------------------------------------------------------
// Function flow:     find dictionary in list
//                    remove our entry from the list
//                    we closed one dictionary..
//                    if open count=0
//                      clear structure
//                    endif
//                    return number of open count
//------------------------------------------------------------------------------
USHORT
QDAMRemoveDict
(
  PBTREE pBTIda                        // pointer to ida
)
{
  T5LOG(T5DEVELOP) << "TEMPORARY COMMENTED";

  return 0;
} /* end of function QDAMRemoveDict */



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictFlushLocal    Flush records
//------------------------------------------------------------------------------
// Function call:     QDAMDictFlushLocal( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Writes all records necessary to disk and clears
//                    the buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     if BTree is corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      while not rc and not all buffers written
//                        if buffer exists and needs to be written
//                          write buffer to disk
//                        endif
//                      endwhile
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictFlushLocal
(
   PBTREE pBTIda
)
{
  int i = 0;
  SHORT sRc = 0;                                 // return code
  PBTREE  pBT = pBTIda;

  DEBUGEVENT2( QDAMDICTFLUSHLOCAL_LOC, FUNCENTRY_EVENT, 0, DB_GROUP, "" );

  if ( pBT->fCorrupted )
  {
     sRc = BTREE_CORRUPTED;
  }
  else
  {
    if ( pBT->LookupTable_V3 != NULL  )
    {
      BOOL fRecordWritten = FALSE;
      PLOOKUPENTRY_V3 pLEntry = pBT->LookupTable_V3;
      for ( i=0; !sRc && (i < pBT->usNumberOfLookupEntries); i++ )
      {
        if ( pLEntry->pBuffer /*&&  (pLEntry->pBuffer)->fNeedToWrite*/ )
        {
          sRc = pBTIda->QDAMWRecordToDisk_V3( pLEntry->pBuffer);
          fRecordWritten = TRUE;
        } /* endif */
        pLEntry++;
      } /* endfor */
    } /* endif */
  }

  if ( sRc )
  {
     ERREVENT2( QDAMDICTFLUSHLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */
  DEBUGEVENT2( QDAMDICTFLUSHLOCAL_LOC, FUNCEXIT_EVENT, 0, DB_GROUP, "" );

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCloseLocal  close the dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCloseLocal( PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Close the file
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//------------------------------------------------------------------------------
// Function flow:     if corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      Flush all records
//                    endif
//                    if okay then
//                      reset open flag in header and force a write to disk
//                    endif
//                    if okay then
//                      close file and set RC to BTREE_CLOSE_ERROR in case of
//                      problems
//                    endif
//                    if okay then
//                      free space of buffers
//                      free space of BTree
//                    endif
//------------------------------------------------------------------------------

SHORT BTREE::QDAMDictClose()
{
   SHORT sRc = 0;                            // error return

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   CHECKPBTREE( this, sRc );
  
   /*******************************************************************/
   /* decrement the counter and check if we have to physically close  */
   /* the dictionary                                                  */
   /*******************************************************************/
   if ( !sRc && ! QDAMRemoveDict( this ) )
   {
      sRc = QDAMDictFlushLocal( this );

     //  reset open flag in header and force a write to disk
     // open flag will only be set if opened for r/w
     if ( !sRc && fOpen && ! fCorrupted )
     {
        fOpen = FALSE;
        // re-write header record
        if ( sRc == NO_ERROR ) 
          sRc = QDAMWriteHeader();
     } /* endif */
     
     if ( UtlClose(fb.file, FALSE) && !sRc )
     {
        sRc = BTREE_CLOSE_ERROR;
     } /* endif */

     if ( fpDummy )
     {
      UtlClose( fpDummy, FALSE );
     } /* endif */

     /*******************************************************************/
     /* free the allocated buffers                                      */
     /*******************************************************************/
     //if ( pBT  )
     {
      /* free allocated space for lookup-table and buffers */   
      if ( LookupTable_V3 )
      {
        USHORT i;
        PLOOKUPENTRY_V3 pLEntry = LookupTable_V3;

        for ( i=0; i < usNumberOfLookupEntries; i++ )
        {
          if ( pLEntry->pBuffer )
          {
            UtlAlloc( (PVOID *)&(pLEntry->pBuffer), 0L, 0L, NOMSG );
          } /* endif */
          pLEntry++;
        } /* endfor */

        UtlAlloc( (PVOID *)&LookupTable_V3, 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *)&AccessCtrTable, 0L, 0L, NOMSG );
        usNumberOfLookupEntries = 0;
        usNumberOfAllocatedBuffers = 0;
      } /* endif */
       
       

       /*****************************************************************/
       /* free index buffer list                                        */
       /*****************************************************************/
        PBTREEINDEX_V3 pIndexBuffer, pTempIndexBuffer;  // temp ptr for freeing index
        pIndexBuffer = pIndexBuffer_V3;
        while ( pIndexBuffer  )
        {
          pTempIndexBuffer = pIndexBuffer->pNext;
          UtlAlloc( (PVOID *)&pIndexBuffer, 0L, 0L, NOMSG );
          pIndexBuffer = pTempIndexBuffer;
        } /* endwhile */
       
       //UtlAlloc( (PVOID *)&pBTIda,0L, 0L, NOMSG );     // free allocated memory
     } /* endif */
     /********************************************************************/
     /* unlock the dictionary                                            */
     /* -- do not take care about return code...                         */
     /********************************************************************/
     QDAMDictLockDictLocal( FALSE );
   } /* endif */
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictClose    close the dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictClose( PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Close the file
//
//------------------------------------------------------------------------------
// Parameters:        PPBTREE                pointer to btree structure
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type (fRemote or local) call the
//                      appropriate routine
//                      if okay so far then
//                        free space of BTree
//                      endif
//                    endif
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDestroy     Destroy dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDestroy( PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Destroy the file if something went wrong during the
//                    create
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//
//------------------------------------------------------------------------------
// Function flow:     if ptr to BTree does not exist
//                      set RC = BTREE_INVALID
//                    else
//                      if filename exists
//                        call QDAMDictClose
//                        delete the file 'filename'
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDestroy
(
   PBTREE  pBT                   // pointer to generic structure
)
{
   SHORT sRc = 0;                   // return code

   CHAR  chName[ MAX_EQF_PATH ];    // file name

   if ( !pBT )
   {
     sRc = BTREE_INVALID;
   }
   else if (!pBT->fb.fileName.empty() )
   {
     //
     // QDAMDictClose frees up all of the memory associated with a B-tree,
     // we need to save the filename as we'll be deleteing the file by name
     //
     strcpy(chName, pBT->fb.fileName.c_str());
     sRc = pBT->QDAMDictClose( );
     if ( chName[0] )
     {
        UtlDelete( chName, 0L, FALSE);
     }
   }
   return ( sRc );
}


SHORT BTREE::QDAMWriteRecord_V3
(
   PBTREEBUFFER_V3 pBuffer
)
{
   SHORT  sRc = 0;                               // return code

   DEBUGEVENT2( QDAMWRITERECORD_LOC, FUNCENTRY_EVENT, RECORDNUM(pBuffer), DB_GROUP, "" );

   INC_WRITE_COUNT;
   /********************************************************************/
   /* write header with corruption flag -- if not yet done             */
   /********************************************************************/
   sRc = QDAMWriteHeader();

   //  if guard flag is on write it to disk
   //  else set only the flag
   if ( !sRc )
   {
     sRc = QDAMWRecordToDisk_V3( pBuffer);
   } /* endif */      
   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReadRecordFromDisk   Read record from disk
//------------------------------------------------------------------------------
// Function call:     QDAMReadRecordFromDisk( PBTREE, USHORT, PPBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       read the requested record from disk
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
//                    BTREE_LOOKUPTABLE_CORRUPTED lookup table is corrupted
//------------------------------------------------------------------------------
// Side effects:      The read record routine is one of the most
//                    performance critical routines in the module.
//
//------------------------------------------------------------------------------
// Function flow:     allocate memory for a new buffer and set pBuffer
//                    if still okay
//                      position in file and set sRc;
//                      if still okay then
//                        read buffer from disk, set sRc;
//                      endif
//                      if still okay then
//                        if new record then
//                          init header information
//                        endif
//                        set pointer to next buffer to be looked at
//                      else
//                        set BTREE_READ_ERROR
//                      endif
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWRecordToDisk    Write record to disk
//------------------------------------------------------------------------------
// Function call:     QDAMWRecordToDisk( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to flush
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return value (sRc )
//                    position in file to record to be written
//                    if okay so far then
//                      write record to disk and set sRC
//                      if okay then
//                        check if disk was full, i.e.
//                        if written bytes equal BTREE_REC_SIZE then
//                          reset fNeedToWrite flag;
//                          init buffer
//                        else
//                          set return code BTREE_DISK_FULL;
//                          set fCorrupted flag
//                        endif
//                      else
//                        set BTREE_WRITE_ERROR;
//                        set fCorrupted flag
//                      endif
//                    else
//                      set BTREE_WRITE_ERROR;
//                      set fCorrupted flag
//                    endif
//                    return sRc;
//------------------------------------------------------------------------------





//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAllocKeyRecords   Allocate space for key records
//------------------------------------------------------------------------------
// Function call:     QDAMAllocKeyRecords( PBTREEBUFFER, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Allocate a chunk of records for keys
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    USHORT                 number of records to be allocated
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     allocate the number of requested records and
//                    put them into a linked list
//
//------------------------------------------------------------------------------
SHORT BTREE::QDAMAllocKeyRecords
(
   USHORT usNum
)
{
   SHORT  sRc = 0;
   PBTREEHEADER  pHeader;

   //if ( pBT->bRecSizeVersion == BTREE_V3 )
   {
      PBTREEBUFFER_V3  pRecord;
      while ( usNum-- && !sRc )
      {
          usNextFreeRecord++;
          sRc = QDAMReadRecord_V3( usNextFreeRecord, &pRecord, TRUE  );

          if ( !sRc )
          {
            // add free record to the free list
            NEXT( pRecord ) = usFreeKeyBuffer;
            PREV( pRecord ) = 0;
            usFreeKeyBuffer = RECORDNUM( pRecord );

            // init this record
            pHeader = &pRecord->contents.header;
            pHeader->usNum = usFreeKeyBuffer;
            pHeader->usOccupied = 0;
            pHeader->usFilled = sizeof(BTREEHEADER );
            pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

            /*************************************************************/
            /* force write                                               */
            /*************************************************************/

            sRc = QDAMWRecordToDisk_V3( pRecord);
          } /* endif */
      } /* endwhile */
   }
  
   return sRc;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMWriteRecord  Write record
//------------------------------------------------------------------------------
// Function call:     QDAMWriteRecord( PBTREE, PBTREERECORD );
//
//------------------------------------------------------------------------------
// Description:       Write the requested record either in cash ( mark it )
//                    or to disk
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The B-tree to write
//                    PBTREERECORD       The buffer to write
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     init return code
//                    write header with open flag set if not yet done
//                    if guard flag is on then
//                      write it to disk and set sRc
//                    else
//                      set only flag
//                    endif
//                    return sRc
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCreateLocal   Create local Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCreateLocal( PSZ, PSZ, SHORT, PCHAR, USHORT,
//                                         PCHAR, PCHAR, PCHAR, PBTREE);
//------------------------------------------------------------------------------
// Description:       Establishes the basic parameters for searching a
//                    user dictionary.
//                    These parameters are stored in the first record of the
//                    index file so that subsequent accesses
//                    know what the index is like.
//
//                    If no server name is given (NULL pointer or EOS) than
//                    it is tried to open a local dictionary.
//                    If no collating sequence is given (NULL pointer) the
//                    default collating sequence is assumed
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map file
//                    PBTREE *         pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     allocate space for BTREE
//                    if not possible
//                      set Rc = BTREE_NO_ROOM
//                    else
//                      open the index file which holds the BTREE
//                      if ok
//                        set initial structure heading
//                        allocate buffer space (for NUMBER_OF_BUFFERS many)
//                        if not possible
//                          set Rc = BTREE_NO_ROOM
//                        if not Rc
//                          call QDAMAllocTempAreas
//                        endif
//                        if not Rc
//                          write header to file
//                          if ok
//                            write 2nd part of first record
//                          endif
//                          if ok
//                            write out an empty root node
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_OPEN_ERROR
//                      endif
//                      if not Rc
//                        return pointer to BTree
//                      else
//                        destroy BTree
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT BTREE::QDAMDictCreateLocal
(
   PTMX_SIGN pSign,
   ULONG ulStartKey         // translation memory
)
{
  const SHORT sNumberOfKey = 20;
  NTMVITALINFO NtmVitalInfo;          // structure to contain vital info for TM
  NtmVitalInfo.ulStartKey = NtmVitalInfo.ulNextKey = ulStartKey;
  SHORT i;
  SHORT sRc = 0;                       // return code
  USHORT  usAction;                    // used in open of file
    
    // Try to create the index file
  fb.file = fopen(fb.fileName.c_str(), "w+b");
  
  if(!fb.file){
    sRc = -1;
    T5LOG(T5ERROR) << "::Can't create file " << fb.fileName;
  }

  if ( !sRc )
  {
    // set initial structure heading
    UtlTime( &(lTime) );                                     // set open time
    sCurrentIndex = 0;
    usFirstNode=usFirstLeaf = 1;
    usCurrentRecord = 0;
    compare = QDAMKeyCompare;
    usNextFreeRecord = 1;
    usFreeKeyBuffer = 0;
    usFreeDataBuffer = 0;
    usFirstDataBuffer = 0;  // first data buffer
    fOpen  = TRUE;                          // open flag set
    fpDummy = NULLHANDLE;

    /******************************************************************/
    /* do settings depending if we are dealing with a dict or a tm..  */
    /******************************************************************/
    usVersion = NTM_VERSION3;// changed from NTM_VERSION2
    strcpy(chEQF,BTREE_HEADER_VALUE_TM3);
    fTerse = FALSE;
    /******************************************************************/
    /* use the free collating sequence buffer to store our vital info */
    /* Its taken care, that the structure will not jeopardize the     */
    /* header...                                                      */
    /******************************************************************/
    chCollate = NtmVitalInfo;
    /******************************************************************/
    /* new key compare routine ....                                   */
    /******************************************************************/
    compare = NTMKeyCompare;

    /* Allocate space for LookupTable */
    usNumberOfLookupEntries = 0;
    UtlAlloc( (PVOID *)&LookupTable_V3, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V3), NOMSG );

    if ( LookupTable_V3 )
    {
      /* Allocate space for AccessCtrTable */
      UtlAlloc( (PVOID *)&AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
      if ( !AccessCtrTable )
      {
        UtlAlloc( (PVOID *)&LookupTable_V3, 0L, 0L, NOMSG );
        sRc = BTREE_NO_ROOM;
      }
      else
      {
        usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
      } /* endif */
    }
    else
    {
      sRc = BTREE_NO_ROOM;
    } /* endif */
    usNumberOfAllocatedBuffers = 0;
  }


  if (! sRc )
  {
    /****************************************************************/
    /* in order to initialize the area of the first record we       */
    /* write an empty record buffer to the file before writing the  */
    /* header                                                       */
    /****************************************************************/                  
    BTREEBUFFER_V3 pbuffer;
    memset((PVOID) &pbuffer, 0, BTREE_REC_SIZE_V3);
    sRc = fb.Write((PVOID)&pbuffer,BTREE_REC_SIZE_V3, 0);
  }

  if(! sRc ){
    sRc = QDAMWriteHeader();
  }    

  if (! sRc )
  {
    sRc = QDAMDictUpdSignLocal( pSign );
  } /* endif */

  if ( !sRc )
  {
    /*******************************************************************/
    /* Write out an empty root node                                    */
    /*******************************************************************/
    PBTREEBUFFER_V3 pRecord;
    sRc = QDAMReadRecord_V3( usFirstNode, &pRecord, TRUE );
    if ( !sRc )
    {
      TYPE(pRecord) = ROOT_NODE | LEAF_NODE | DATA_KEYNODE;            
      sRc = QDAMWriteRecord_V3(pRecord);
    } /* endif */
    if ( !sRc )
    {
      sRc = QDAMAllocKeyRecords(  1 );
      
    } /* endif */
    if (! sRc )
    {
      usFirstDataBuffer = usNextFreeRecord;
    } /* endif */
  } /* endif */
  
  switch ( sRc )
  {
    case  ERROR_INVALID_DRIVE:
      sRc = BTREE_INVALID_DRIVE;
      break;
    case  ERROR_OPEN_FAILED :
      sRc = BTREE_OPEN_FAILED;
      break;
    case  ERROR_NETWORK_ACCESS_DENIED:
      sRc = BTREE_NETWORK_ACCESS_DENIED;
      break;
    case 0: break;
    default :
      sRc = BTREE_OPEN_ERROR;
      break;
  } /* endswitch */

  if ( !sRc )
  {
     /****************************************************************/
     /* add this dictionary to our list ...                          */
     /****************************************************************/
     sRc = QDAMAddDict( (PSZ)fb.fileName.c_str(), this );
  }
  else
  {
    // leave the return code from create
    // file will not be destroyed if create failed, since then
    // filename is not set
    QDAMDestroy( this );
  } /* endif */
  return( sRc );
}



#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXClose    closes data and index files                 |
//+----------------------------------------------------------------------------+
//|Description:       closes qdam data and index files                         |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXClose( PTMX_CLOSE_IN pTmCloseIn, //input struct         |
//|                           PTMX_CLOSE_OUT pTmCloseOut ) //output struct     |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_CLOSE_IN  pTmCloseIn     input structure              |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_CLOSE_OUT pTmCloseOut   output structure             |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in close out structure                |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| copy compact area record to tm data file                                   |
//| close tm index file                                                        |
//| close tm data file                                                         |
//|                                                                            |
//| return close out structure                                                 |
// ----------------------------------------------------------------------------+



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockStatus
//------------------------------------------------------------------------------
// Function call:     fLocked = QDAMDictLockStatus( pBTIda, pHeadTerm );
//------------------------------------------------------------------------------
// Description:       This call returns the status of the current entry
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIDa   pointer to instance data
//                    PSZ     pHeadTerm  pointer to head term
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   entry is locked
//                    FALSE  entry is free for update
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of all open instances of our dict..
//                    run thru list and check if this headterm is locked ..
//------------------------------------------------------------------------------
BOOL
BTREE::QDAMDictLockStatus
(
  PWCHAR   pKey
)
{
  return( false );
} /* end of function QDAMDictLockStatus */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMIncrUpdCounter      Inrement database update counter
//------------------------------------------------------------------------------
// Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )
//
//------------------------------------------------------------------------------
// Description:       Update one of the update counter field in the dummy
//                    /locked terms file
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    SHORT                  index of counter field
//                    PLONG                  ptr to buffer for new counte value
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read update counter from dummy file
//                    increment update counter
//                    position ptr to begin of file
//                    write update counter to disk
//------------------------------------------------------------------------------
SHORT QDAMIncrUpdCounter
(
   PBTREE     pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of update counter
   PLONG      plNewValue               // ptr to buffer for new counte value
)
{
  SHORT       sRc=0;
  USHORT      usNumBytes;              // number of bytes written or read
  ULONG       ulNewOffset;             // new offset in file
  PBTREE  pBT = pBTIda;
  SHORT       sRetries;                // number of retries
  LONG     lNewUpdCounter;      // buffer for new update counter

  lNewUpdCounter = 0L;
  sRetries = MAX_RETRY_COUNT;
  do
  {
     // Position to requested update counter
    sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                         FILE_BEGIN, &ulNewOffset, FALSE);
    if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );

    // Read current update counter
     if ( !sRc )
     {
      sRc = UtlRead( pBT->fpDummy, (PVOID)&lNewUpdCounter, sizeof(LONG),
                     &usNumBytes, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     // Increment update counter
     if ( !sRc )
     {
       lNewUpdCounter++;
        pBT->alUpdCtr[sIndex] = lNewUpdCounter;;
        if ( plNewValue )
        {
          *plNewValue = pBT->alUpdCtr[sIndex];
        } /* endif */
     } /*endif */

     // Position to requested update counter
     if ( !sRc )
     {
      sRc = UtlChgFilePtr( pBT->fpDummy, (LONG)(sizeof(LONG)*sIndex),
                           FILE_BEGIN, &ulNewOffset, FALSE);
      if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_READ_ERROR, pBT->usOpenFlags );
     } /* endif */

     //  Rewrite update counter
     if ( !sRc )
     {
       sRc = UtlWrite( pBT->fpDummy, (PVOID)&lNewUpdCounter,
                       sizeof(LONG), &usNumBytes, FALSE );
       if (sRc ) sRc = QDAMDosRC2BtreeRC( sRc, BTREE_WRITE_ERROR, pBT->usOpenFlags );
     } /* endif */

    if ( sRc == BTREE_IN_USE )
    {
      UtlWait( MAX_WAIT_TIME );
      sRetries--;
    } /* endif */
  } while ( (sRc == BTREE_IN_USE) && (sRetries > 0) );

  if ( sRc )
  {
    ERREVENT2( QDAMINCRUPDCOUNTER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return sRc;
}




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMPhysLock           Physical lock file
//------------------------------------------------------------------------------
// Function call:     QDAMPhysLock( PBTREE, BOOL, PBOOL );
//
//------------------------------------------------------------------------------
// Description:       lock or unlock the database
//------------------------------------------------------------------------------
// Parameters:        PBTREE             The database to be locked
//                    BOOL               TRUE = LOCK, FALSE = Unlock
//                    PBOOL              ptr to locked flag (set to TRUE if
//                                       locking was successful
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
SHORT QDAMPhysLock
(
   PBTREE         pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
)
{
  LOG_UNIMPLEMENTED_FUNCTION;
  return 0;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdStatus
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdStatus( pBTIda );
//------------------------------------------------------------------------------
// Description:       invalidates the status of the current record
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIda   pointer to instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of instances
//                    run through list and invalidate the current entry for
//                      all instances ...
//                    Our instance data will be updated after this call, so
//                      it's okay to get rid of it, too.
//------------------------------------------------------------------------------
VOID BTREE::QDAMDictUpdStatus()
{
} /* end of function QDAMDictUpdStatus */

PWCHAR QDAMGetszKey_V3
(
   PBTREEBUFFER_V3  pRecord,              // active record
   USHORT  i,                          // get data term
   USHORT  usVersion                   // version of database
)
{
  PBYTE    pData = nullptr, 
  pEndOfRec = (PBYTE)&(pRecord->contents) + BTREE_REC_SIZE_V3;// get max pointer value

  // use record number of passed entry , read in record and pass
  // back pointer
  PUSHORT pusOffset = (PUSHORT) pRecord->contents.uchData;
   
  pusOffset += i;                     // point to key
  int step = sizeof(USHORT ) + sizeof(RECPARAM); // get pointer to data

  if ( (PBYTE)pusOffset > pEndOfRec )
  {
    // offset pointer is out of range
    T5LOG(T5ERROR) << "QDAMGetszKey_V3:: pusOffset > pEndOfRec , pusOffset = " << (long int)pusOffset << 
        "; pEndOfRec = " << (long int)pEndOfRec << "\npusOffset - pEndOfRec = " << (long int) ((PBYTE)pusOffset - pEndOfRec);
    //ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 1, DB_GROUP, "" );
  }
  else
  {
    pData = pRecord->contents.uchData + (*pusOffset) + step ;
    if ( pData > pEndOfRec )
    {
      // data pointer is out of range
      T5LOG(T5ERROR) << "QDAMGetszKey_V3:: data pointer is out of range , pusOffset = " << (long int)pusOffset <<
            "; pEndOfRec = " << (long int)pEndOfRec;
      pData = nullptr;
      //ERREVENT2( QDAMGETSZKEY_LOC, INTFUNCFAILED_EVENT, 2, DB_GROUP, "" );
    } /* endif */
  } /* endif */

  return ( (PWCHAR)pData );
}

RECPARAM  QDAMGetrecData_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid,                           // key number
   USHORT        usVersion                       // version of database
)
{
   RECPARAM      recData;               // data description structure
   PUSHORT  pusOffset;

   // use record number of passed entry , read in record and pass
   // back pointer
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   pusOffset += sMid;                            // point to key
   PCHAR pData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
   pData += sizeof(USHORT );                    // get pointer to datarec

   memcpy( &recData,         (PRECPARAM)  pData,    sizeof( RECPARAM ));
   memcpy( &recData.usNum,   (PUSHORT)    pData,    sizeof( USHORT )  );
   memcpy( &recData.usOffset,(PUSHORT)   (pData+2), sizeof( USHORT )  );
   memcpy( &recData.ulLen,   (PULONG)    (pData+8), sizeof( PULONG )  );
   
   return ( recData );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindRecord      Find Record
//------------------------------------------------------------------------------
// Function call:     QDAMFindRecord( PBTREE, PCHAR, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the leaf node that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------
SHORT BTREE::QDAMFindRecord_V3
(
    PWCHAR  pKey,
    PBTREEBUFFER_V3 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PWCHAR      pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code

  memset(&recData, 0, sizeof(recData));
  sRc = QDAMReadRecord_V3( usFirstNode, ppRecord, FALSE  );
  while ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( *ppRecord, sMid, usVersion );
      if ( pKey2 == nullptr )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*compare)(this, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( *ppRecord, sHigh, usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = QDAMReadRecord_V3( recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMTerseData   Terse Data                               |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMTerseData( PBTREE, PUCHAR, PUSHORT );                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This function will run the passed string against the     |
//|                   compression table.                                       |
//|                   If the resulting string is shorter the compressed one is |
//|                   used, otherwise the original is used.                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PUCHAR                 pointer to data string            |
//|                   PUSHORT                length of the string              |
//|                   PUCHAR                 pointer to output data string     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:      -- use the modified Huffman algorithm to terse data     |
//|                    get length of passed string                             |
//|                    set output pointer and length of output string          |
//|                    while remaining length > 0                              |
//|                      encode byte; increase encodelen appropriatly          |
//|                      if byte filled up move it into data record;           |
//|                      if byte to encode is not one of the majors then       |
//|                        get next byte and put it into encode sequence       |
//|                      endif                                                 |
//|                    endwhile                                                |
//|                    if still something left in encode table                 |
//|                      put it into output string                             |
//|                    endif                                                   |
//|                    if output string shorter than input string then         |
//|                      set return code to TRUE; store length                 |
//|                    endif                                                   |
//|                    return return code                                      |
//+----------------------------------------------------------------------------+

BOOL BTREE::QDAMTerseData
(
   PUCHAR  pData,                   // pointer to data
   PULONG  pulLen                  // length of the string
)
{
   ULONG    ulLen ;                    // length of string
   BOOL     fShorter = FALSE;          // new string is not shorter
   USHORT   usCurByte = 0;             // currently processed byte
   PUCHAR   pTempData;                 // pointer to temp data area
   PUCHAR   pEndData;                  // pointer to end data area
   USHORT   usI;                       // index
   USHORT   usEncodeLen = 0;           // currently encoded bits
   USHORT   usTerseLen;                // length of tersed byte

#if defined(MEASURE)
  ulBeg = pGlobInfoSeg->msecs;
#endif
   // get length of passed string
   ulLen = *pulLen;


   //if ( pTempRecord )
   {
      pTempData = STARTOFDATA( this, &TempRecord[0] );

      pEndData = pTempData + ulLen;             // position  Tersed > untersed
      while ( ulLen > 0)
      {
         usI = *pData ++;
         ulLen --;                               // point to next character

         usTerseLen = (USHORT) bEncodeLen[usI];
         usCurByte = (usCurByte << usTerseLen) + chEncodeVal[usI];

         usEncodeLen = usEncodeLen + usTerseLen;

         /*************************************************************/
         /* byte filled up so move it into data record                */
         /*************************************************************/
         if ( usEncodeLen >= 8  )

         {
           usEncodeLen -= 8;
           if ( pTempData < pEndData  )
           {
             *pTempData = (CHAR) (usCurByte >> usEncodeLen);
             usCurByte -= (*pTempData << usEncodeLen );
           }
           else
           {
             ulLen = 0;       // loop end condition
           } /* endif */
           pTempData ++;
         } /* endif */

         /*************************************************************/
         /* if it is not one of the majors put in the full character  */
         /*************************************************************/
         if ( !chEncodeVal[usI] )
         {
           usCurByte = (usCurByte << 8) + usI;
           if ( pTempData < pEndData  )
           {
             *pTempData = (CHAR) (usCurByte >> usEncodeLen);
             usCurByte -= (*pTempData << usEncodeLen);
           }
           else
           {
             ulLen = 0;       // loop end condition
           } /* endif */
           pTempData ++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* save last chunk if some bits are left...                     */
      /* move them left so they will fill up a full byte              */
      /****************************************************************/
      if ( usEncodeLen )
      {
        if ( pTempData < pEndData  )
        {
          usCurByte = ( usCurByte << ( 8 - usEncodeLen ) );
          *pTempData++ = (CHAR) usCurByte;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* check if compression longer than org data                    */
      /* return TRUE/FALSE accordingly                                */
      /****************************************************************/
      ulLen = pTempData - &TempRecord[0];
      if ( ulLen < *pulLen )
      {
//        *(PUSHORT) pTempRecord = (USHORT)*pulLen;  // insert record length
        SETDATALENGTH( this, &TempRecord[0], *pulLen );
        *pulLen = ulLen;                           // set new length
        fShorter = TRUE;
      } /* endif */

   } /* endif */
#if defined(MEASURE)
  ulTerseEnd += (pGlobInfoSeg->msecs -ulBeg );
#endif
   return ( fShorter );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMNewRecord   Add a new record to the index
//------------------------------------------------------------------------------
// Function call:     QDAMNewRecord( PBTREE, PPBTREERECORD, RECTYPE );
//
//------------------------------------------------------------------------------
// Description:         Get a new record either from the free list
//                      or create a new
//
//------------------------------------------------------------------------------
// Parameters:          PBTREE             B-tree
//                      PBTREERECORD *     the buffer returned
//                      RECTYPE            data or key record
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     if record is a datarecord
//                      if there is a free data buffer
//                        call QDAMReadRecord
//                        increase ptr to next free data buffer
//                      else
//                        read next free record and update ptr
//                      endif
//                    else
//                      if no free key buffer
//                        call QDAMAllocKeyRecords
//                      endif
//                      if ok
//                        read the free key buffer (QDAMReadRecord)
//                        update ptr to next free key buffer
//                      endif
//                    endif
//                    reset info in *ppRecord
//------------------------------------------------------------------------------
SHORT BTREE::QDAMNewRecord_V3
(
   PBTREEBUFFER_V3  * ppRecord,
   RECTYPE         recType          // data or key record
)
{
  SHORT   sRc = 0;                         // return code
  //
  // Try and use a record within the file if there are any.  This saves
  // on disk usage
  //
  *ppRecord = NULL;                       // in case of error
  if ( recType == DATAREC )
  {
     if ( usFreeDataBuffer )
     {
       sRc = QDAMReadRecord_V3( usFreeDataBuffer, ppRecord, FALSE );
       if ( ! sRc )
       {
         usFreeDataBuffer = NEXT( *ppRecord );
       } /* endif */
     }
     else
     {
       if ( usNextFreeRecord == 0xFFFF ) // end reached ???
       {
         sRc = BTREE_LOOKUPTABLE_TOO_SMALL;
       }
       else
       {
         usNextFreeRecord++;
         sRc = QDAMReadRecord_V3( usNextFreeRecord, ppRecord, TRUE );
       }
     }
  }
  else
  {
     if ( !usFreeKeyBuffer)
     {
       sRc = QDAMAllocKeyRecords( 5 );
     } /* endif */

     if ( !sRc )
     {
       sRc = QDAMReadRecord_V3( usFreeKeyBuffer, ppRecord, FALSE  );
       if ( ! sRc )
       {
         usFreeKeyBuffer = NEXT( *ppRecord );
       } /* endif */
     } /* endif */
  } /* endif */

  if ( *ppRecord )
  {
    NEXT(*ppRecord) = 0;                          // reset information
    PREV(*ppRecord) = 0;                          // reset information
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMNEWRECORD_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return ( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddToBuffer       Add Data to buffer
//------------------------------------------------------------------------------
// Function call:     QDAMAddToBuffer( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Add the passed data to the string buffer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be inserted
//                    PCHAR                  buffer for user data
//                    PRECPARAM              contains position of data stored
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if terse flag is on
//                      terse the data
//                    endif
//                    get list for currently used record type
//                    check if sufficient space is available to
//                       fit into one of these records
//                    if it fits
//                      read record
//                      fill in key data, insert reference
//                      set address of key data
//                      mark if tersed or not
//                      write record update list entry
//                    endif
//                    if record is too large or no space left in list of recs
//                       scan list and find empty slot
//                       if not found
//                         remove slot which is most filled
//                       endif
//                       get record and store start position
//                       fill it up to length or record size
//                       update list entry
//                    endif
//                    return record parameters
//
//------------------------------------------------------------------------------
SHORT  BTREE::QDAMAddToBuffer_V3
(
   PBYTE   pData,                      // pointer to data
   ULONG   ulDataLen,                  // length of data to be filled in
   PRECPARAM  precReturn               // pointer to return code
)
{
   SHORT sRc = 0;                      // success indicator

   USHORT  usFilled;                   // number of bytes filled in record
   USHORT  usMaxRec= 0;                // most filled record
   RECPARAM recStart;                  // return code where data are stored
   PRECPARAM pRecParam;                // parameter to stored list
   PRECPARAM pRecTemp ;                // parameter to stored list
   BOOL  fFit = FALSE;                 // indicator if record found
   USHORT  i;                          // index
   PBTREEHEADER  pHeader;              // record header
   PBTREEBUFFER_V3 pRecord = NULL;        // buffer record
   //PBTREEBUFFER_V3 pTempRecord;           // buffer record
   CHAR     chNodeType;                // type of node
   BOOL     fTerse = FALSE;            // data are not tersed
   USHORT   usLastPos;
   ULONG    ulFullDataLen;             // length of record
   PUSHORT  pusOffset;
   PCHAR    pRecData;                  // pointer to record data
   ULONG    ulFitLen;                  // currently used length
   USHORT   usLenFieldSize;            // size of length field

   memset(&recStart, 0, sizeof(recStart));
   memset(&TempRecord[0], 0, sizeof(TempRecord));
   /*******************************************************************/
   /* Enlarge pTempRecord area if it is not large enough to contain   */
   /* the data for this record                                        */
   /*******************************************************************/
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG))
   {
    T5LOG( T5DEBUG) << ":: data len = " << ulDataLen ;
     //CHAR szTemp[8];
     //sprintf( szTemp, "%ld", ulDataLen );
     //INFOEVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 4711, DB_GROUP, szTemp );
   }
   if ( (ulDataLen + sizeof(ULONG)) > TempRecord.size() )
   {
     TempRecord.resize(ulDataLen + sizeof(ULONG));
     //if ( UtlAlloc( (PVOID *)&(pTempRecord), ulTempRecSize, ulDataLen + sizeof(ULONG), NOMSG ) )
     //{
     //  ulTempRecSize = ulDataLen + sizeof(ULONG);
    // }
     //else
     //{
     //  sRc = BTREE_NO_ROOM;
     //} /* endif */
   } /* endif */
  
  usLenFieldSize = sizeof(ULONG);
  pRecParam = DataRecList;
  /*******************************************************************/
  /* determine type of compression....                               */
  /*******************************************************************/
  switch ( fTerse )
  {
    case  BTREE_TERSE_HUFFMAN :
      fTerse = QDAMTerseData(pData, &ulDataLen );
      break;
    default :
      fTerse = FALSE;
      break;
  } /* endswitch */

  chNodeType = DATA_NODE;

  // data should be in pTempRecord for further processing
  // copy them if not yet done.....
  if ( !fTerse )
  {
    memcpy( &TempRecord[0], pData, ulDataLen );
  } /* endif */

  if ( !sRc && (ulDataLen <= BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER)) )
  {
    pRecTemp = pRecParam;
    i = 0;
    //  find slot in list
    usFilled = (USHORT)(BTREE_REC_SIZE_V3 - ulDataLen - 2 * usLenFieldSize);
    while ( !fFit && i < MAX_LIST )
    {
      if ( pRecTemp->usOffset == 0 || pRecTemp->usOffset > usFilled )   // will not fit
      {
        pRecTemp++;
        i++;
      }
      else                                     // data will fit
      {
        fFit = TRUE;
      } /* endif */
    } /* endwhile */

    if ( fFit )
    {
      // store position where data will be stored
      recStart.usNum    = pRecTemp->usNum;

      // get record, copy data and write them
      sRc = QDAMReadRecord_V3( recStart.usNum, &pRecord, FALSE  );
      if ( !sRc )
      {
        BTREELOCKRECORD( pRecord );
        if ( TYPE( pRecord ) != chNodeType )
        {
            sRc = BTREE_CORRUPTED;
            T5LOG(T5ERROR) <<  ":: ERREVENT2( QDAMADDTOBUFFER_LOC, STATE_EVENT, 1, DB_GROUP, "" );, BTREE_CORRUPTED   TYPE( pRecord ) != chNodeType";
        }
        else
        {
          // fill in key data
          usLastPos = pRecord->contents.header.usLastFilled;
          usLastPos = usLastPos - (USHORT)(ulDataLen + usLenFieldSize);
          pData = pRecord->contents.uchData + usLastPos;
          // insert reference               
          *(PULONG) pData = ulDataLen;
          
          memcpy( pData+usLenFieldSize, &TempRecord[0], ulDataLen );

          // set address of key data at next free offset
          pusOffset = (PUSHORT) pRecord->contents.uchData;
          *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

          OCCUPIED(pRecord) += 1;
          pRecord->contents.header.usLastFilled = usLastPos;
          pRecord->contents.header.usFilled = pRecord->contents.header.usFilled + (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

          // mark if tersed or not
          if ( fTerse )
          {
          *(PULONG) pData |= QDAM_TERSE_FLAGL;                 
          } /* endif */
//        pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

          sRc = QDAMWriteRecord_V3(pRecord);
          // update list entry
          if ( !sRc )
          {
            pRecTemp->usOffset = pRecord->contents.header.usFilled;
          } /* endif */
        } /* endif */
        BTREEUNLOCKRECORD( pRecord );
      } /* endif */
    } /* endif */
  } /* endif */

  //  either record is too large or no space left in list of records
  if ( !sRc && !fFit )
  {
    pRecTemp = pRecParam;
    i = 0;
    // scan list and find empty slot
    while ( (i < MAX_LIST) && !fFit )
    {
      if ( pRecTemp->usNum == 0 )     // empty slot found
      {
        fFit = TRUE;
      }
      else
      {
        pRecTemp++;                  // point to next slot
        i++;
      } /* endif */
    } /* endwhile */

    /****************************************************************/
    /* if not found remove slot which is most filled                */
    /****************************************************************/
    if ( !fFit )
    {
      pRecTemp = pRecParam;
      usFilled = 0;
      for ( i=0 ; i < MAX_LIST ;i++ )
      {
        if ( pRecTemp->usOffset > usFilled)
        {
          usMaxRec = i;
          usFilled = pRecTemp->usOffset;
        } /* endif */
        pRecTemp++;                // next record in list
      } /* endfor */
      // set pRecTemp to the selected slot
      pRecTemp = pRecParam + usMaxRec;
    } /* endif */

    // get record and store start position
    memset( pRecTemp, 0 , sizeof(RECPARAM));

    sRc = QDAMNewRecord_V3( &pRecord, DATAREC );
    // fill it up to length or record size
    if ( ! sRc )
    {
      TYPE( pRecord ) = chNodeType;     // indicate that it is a data node
      BTREELOCKRECORD( pRecord );
      pRecData = (PCHAR)(&TempRecord[0] + ulDataLen);
      ulFullDataLen = ulDataLen;         // store original data len
      while ( !sRc && ulDataLen + pRecord->contents.header.usFilled  >= (BTREE_REC_SIZE_V3 - (usLenFieldSize + sizeof(SHORT))))
      {
        /***********************************************************/
        /* get a new record, anchor it and fill data from end      */
        /* new record will be predecessor of already allocated rec.*/
        /***********************************************************/
        PBTREEBUFFER_V3 pTempRecBuff;
        //sRc = QDAMNewRecord_V3( (PBTREEBUFFER_V3*)&TempRecord[0], DATAREC );
        sRc = QDAMNewRecord_V3( &pTempRecBuff, DATAREC );

        if ( !sRc )
        {
          TYPE( pTempRecBuff ) = chNodeType;   // data node
          NEXT( pTempRecBuff ) = RECORDNUM( pRecord );
          PREV( pRecord ) = RECORDNUM( pTempRecBuff );
          TYPE( pRecord ) = DATA_NEXTNODE;              // data node

          // fill in key data
          /*********************************************************/
          /* adjust sizes of relevant data                         */
          /*********************************************************/
          usLastPos = pRecord->contents.header.usLastFilled;
          {
            ULONG ulMax = (ULONG)(BTREE_REC_SIZE_V3 - pRecord->contents.header.usFilled );
            ulFitLen =  ulDataLen < ulMax? ulDataLen : ulMax;
            ulFitLen -= (ULONG)(usLenFieldSize + sizeof(USHORT));
          }
          usLastPos = usLastPos - (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
          pData = pRecord->contents.uchData + usLastPos;
          pRecData -= ulFitLen;
          ulDataLen -= ulFitLen;
          /*********************************************************/
          /* store length and copy data of part contained in record*/
          /*********************************************************/
          *(PULONG) pData = ulFitLen;
          
          memcpy( pData+usLenFieldSize, pRecData,  ulFitLen );
          // set address of key data at next free offset
          pusOffset = (PUSHORT) pRecord->contents.uchData;
          *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

          OCCUPIED(pRecord) += 1;
          pRecord->contents.header.usLastFilled = usLastPos;
          pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                        (USHORT)(ulFitLen + (ULONG)usLenFieldSize);
//           pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

          sRc = QDAMWriteRecord_V3(pRecord);
          BTREEUNLOCKRECORD( pRecord );

          /*********************************************************/
          /* toggle for adressing purposes                         */
          /* now: pRecord will be the free one for later filling   */
          /*********************************************************/
          //pRecord = (PBTREEBUFFER_V3)&TempRecord[0];
          pRecord = pTempRecBuff;
          BTREELOCKRECORD( pRecord );
        } /* endif */
      } /* endwhile */

      /*************************************************************/
      /* write out rest (this is the normal way in most cases      */
      /*************************************************************/
      if ( !sRc )
      {
        pHeader = &(pRecord->contents.header);
        pHeader->usParent = sizeof(BTREEHEADER );

        // store position where data will be stored
        recStart.usNum    = pHeader->usNum;

        // fill in key data
        usLastPos = pRecord->contents.header.usLastFilled;

        usLastPos = usLastPos - (USHORT)(ulDataLen + (ULONG)usLenFieldSize);
        pData = pRecord->contents.uchData + usLastPos;
        pRecData -= ulDataLen;

        // insert reference           
        *(PULONG) pData = ulFullDataLen;
        
        memcpy( pData+usLenFieldSize, pRecData, ulDataLen );

        // set address of key data at next free offset
        pusOffset = (PUSHORT) pRecord->contents.uchData;
        *(pusOffset + OCCUPIED( pRecord )) = usLastPos;

        OCCUPIED(pRecord) += 1;
        pRecord->contents.header.usLastFilled = usLastPos;
        pRecord->contents.header.usFilled = pRecord->contents.header.usFilled +
                                (USHORT)(ulDataLen+usLenFieldSize+sizeof(USHORT));

        // mark if tersed or not
        if ( fTerse )
        {
          *(PULONG) pData |= QDAM_TERSE_FLAGL;          
        } /* endif */


        // write record
//       pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );
        sRc = QDAMWriteRecord_V3(pRecord);
        BTREEUNLOCKRECORD( pRecord );         // free the locking
        // update list entry
        if ( !sRc )
        {
          pRecTemp->usOffset = pHeader->usFilled;
          pRecTemp->usNum    = pHeader->usNum;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  //  return record parameters
  if ( !sRc )
  {
    precReturn->usNum = recStart.usNum - usFirstDataBuffer;
    precReturn->usOffset = OCCUPIED( pRecord ) - 1;
  }
  else
  {
    memset(&recStart, 0, sizeof(RECPARAM));
    precReturn = &recStart;
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMADDTOBUFFER_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return sRc;
}


SHORT QDAMCaseCompare
(
    PVOID pvBT,                        // pointer to tree structure
    PVOID pKey1,                       // pointer to first key
    PVOID pKey2,                       // pointer to second key
    BOOL  fIgnorePunctuation           // ignore punctuation flag
)
{
  SHORT sDiff;
  BYTE  c;
  PBTREE pBTIda = (PBTREE)pvBT;        // pointer to tree structure
  PBTREE    pBT = pBTIda;
  PBYTE  pbKey1 = (PBYTE) pKey1;
  PBYTE  pbKey2 = (PBYTE) pKey2;
  PBYTE  pMap = pBT->chCaseMap;        // pointer to mapping table

  while ( (c = *pbKey1) != 0 )
  {
    /******************************************************************/
    /* ignore the following characters during matching: '/', '-', ' ' */
    /******************************************************************/
    if ( fIgnorePunctuation )
    {
       while ( (c = *pbKey2) == ' ' || fIsPunctuation[c] )
       {
         pbKey2++;
       } /* endwhile */
       while ( (c = *pbKey1) == ' ' || fIsPunctuation[c] )
       {
         pbKey1++;
       } /* endwhile */
    } /* endif */

    sDiff = *(pMap+c) - *(pMap + *pbKey2);
    if ( !sDiff && *pbKey1 && *pbKey2 )
    {
      pbKey1++;
      pbKey2++;
    }
    else
    {
      return ( sDiff );
    } /* endif */
  } /* endwhile */
  return ( *(pMap + c) - *(pMap + *pbKey2));
}

SHORT BTREE::QDAMLocateKey_V3
(
   PBTREEBUFFER_V3 pRecord,                  // record to be dealt with
   PWCHAR pKey,                          // key to be searched
   PSHORT  psKeyPos,                      // located key
   SEARCHTYPE  searchType,                // search type
   PSHORT  psNearPos                      // near position
)
{
  SHORT  sLow;                             // low value
  SHORT  sHigh;                            // high value
  SHORT  sResult;
  SHORT  sMid = 0;                         //
  SHORT  sRc = 0;                          // return value
  PWCHAR  pKey2;                         // pointer to key string
  BOOL   fFound = FALSE;

  *psKeyPos = -1;                         // key not found
  if ( pRecord )
  {
    BTREELOCKRECORD( pRecord );
    sHigh = (SHORT) pRecord->contents.header.usOccupied -1 ;      // counting starts at zero
    sLow = 0;                                    // start here
    
    while ( !fFound && sLow <= sHigh )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( pRecord, sMid, usVersion );

      if ( pKey2 )
      {
        sResult = (*compare)(this, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else if (sResult > 0)
        {
          sLow = sMid+1;                           // Go right
        }else{
          fFound = TRUE;
          // if exact match is required we have to do a strcmp
          // to ensure it and probably check the previous or the
          // next one because our insertion is case insensitive

          /*********************************************************/
          /* checking will be done in any case to return the best  */
          /* matching substring                                    */
          /*********************************************************/
          {
            if (*((PULONG)pKey) == *((PULONG)pKey2))
            {
              *psKeyPos = sMid;
            }
            else
            {
              // try with previous
              if ( sMid > sLow )
              {
                pKey2 = QDAMGetszKey_V3( pRecord, (SHORT)(sMid-1), usVersion );
                if ( pKey2 == NULL )
                {
                  sRc = BTREE_CORRUPTED;
                }
                else if ( *((PULONG)pKey) == *((PULONG)pKey2) )
                {
                  *psKeyPos = sMid-1 ;
                } /* endif */
              } /* endif */
              //  still not found
              if ( !sRc && *psKeyPos == -1 && sMid < sHigh )
              {
                pKey2 = QDAMGetszKey_V3(  pRecord, (SHORT)(sMid+1), usVersion );
                if ( pKey2 == NULL )
                {
                  sRc = BTREE_CORRUPTED;
                }
                else if ( *((PULONG)pKey) == *((PULONG)pKey2) )
                {
                  *psKeyPos = sMid+1 ;
                } /* endif */
              } /* endif */
            } /* endif */
          }
        
          /*********************************************************/
          /* if we are checking only for substrings and didn't     */
          /* find a match yet, we will set the prev. found substrin*/
          /*********************************************************/
          if ( (*psKeyPos == -1) && (searchType == FSUBSTR)  )
          {
            *psKeyPos = sMid;
          } /* endif */
          *psNearPos = *psKeyPos;
        } /* endif */
      }
      else
      {
        fFound = TRUE;
        sRc = BTREE_CORRUPTED;                // tree is corrupted
      } /* endif */
    } /* endwhile */
    if ( !fFound )
    {
      *psNearPos = sLow < (SHORT)OCCUPIED(pRecord)-1 ? sLow : (SHORT)OCCUPIED(pRecord)-1 ;// set nearest pos
    } /* endif */
    BTREEUNLOCKRECORD( pRecord );  
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMLOCATEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFirstEntry      Position at first entry
//------------------------------------------------------------------------------
// Function call:     QDAMFirstEntry ( PBTREE, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       This function will position to the first entry in the
//                    dictionary
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PPBTREEBUFFER          pointer to selected record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EOF_REACHED end or start of dictionary reached
//
//------------------------------------------------------------------------------
// Function flow:     read in root record and set Rc
//                    if okay then
//                      while okay and record not leaf
//                        read in record pointed to by first entry
//                      endwhile
//                      if okay then
//                        set current record and current index
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT  QDAMFirstEntry_V3
(
   PBTREE  pBTIda,
   PBTREEBUFFER_V3  * ppRecord               // pointer to pointer of record
)
{
  SHORT  sRc = 0;
  RECPARAM     recData;
  PBTREE    pBT = pBTIda;

  // read in root record
  sRc = pBTIda->QDAMReadRecord_V3( pBT->usFirstNode, ppRecord, FALSE );
  if ( !sRc )
  {
    while (!sRc && !IS_LEAF(*ppRecord))
    {
      recData = QDAMGetrecData_V3( *ppRecord, 0, pBT->usVersion);
      sRc = pBTIda->QDAMReadRecord_V3( recData.usNum, ppRecord , FALSE );
    } /* endwhile */
    if ( !sRc )
    {
      pBTIda->usCurrentRecord = RECORDNUM(*ppRecord);
      pBT->usFirstLeaf = pBTIda->usCurrentRecord;    // determine first leaf
      pBTIda->sCurrentIndex = 0;
    } /* endif */
  } /* endif */

  return (sRc);
}





//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindParent       Find Parent for record
//------------------------------------------------------------------------------
// Function call:     QDAMFindParent ( PBTREE, PBTREEBUFFER, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locates the record being the parent node to the passed
//                    record.
//
//                    Returns record number of parent node or NULL
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           record for which parent is wanted
//                    PUSHORT                record number of parent node
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------
SHORT QDAMFindParent_V3
(
    PBTREE pBTIda,
    PBTREEBUFFER_V3  pRecord,
    PUSHORT       pusParent                      // record number of parent
)
{
  SHORT         sResult;
  SHORT         sLow;                            // Far left
  SHORT         sHigh;                           // Far right
  SHORT         sMid = 0;                        // Middle
  RECPARAM      recData;                         // data structure
  PCHAR_W       pKey2;                           // pointer to search key
  PCHAR_W       pKey ;                           // pointer to search key
  CHAR_W        chKey[HEADTERM_SIZE];            // key to be found
  SHORT         sRc;                             // return code
  PBTREEBUFFER_V3  pTempRec;                        // temp. record buffer
  USHORT        usRecNum;                        // passed record number
  PBTREE    pBT = pBTIda;

  memset(&recData, 0, sizeof(recData));

  *pusParent = pBT->usFirstNode;                 // set parent to first node
  usRecNum = RECORDNUM( pRecord );
  pKey = QDAMGetszKey_V3( pRecord, 0, pBT->usVersion );
  if ( pKey == NULL )
  {
    sRc = BTREE_CORRUPTED;
  }
  else
  {
    memcpy( chKey, pKey, sizeof(ULONG));      
  } /* endif */

  sRc = pBTIda->QDAMReadRecord_V3( pBT->usFirstNode, &pTempRec, FALSE  );

  while ( !sRc && !IS_LEAF( pTempRec ) && (usRecNum != RECORDNUM(pTempRec)) )
  {
    BTREELOCKRECORD( pTempRec );
    *pusParent = RECORDNUM( pTempRec );          // set parent to active node

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( pTempRec ) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( pTempRec, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, chKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( pTempRec, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( pTempRec  );                        // unlock previous record.

    if ( !sRc )
    {
      sRc = pBTIda->QDAMReadRecord_V3( recData.usNum, &pTempRec, FALSE  );
    } /* endif */
  } /* endwhile */
  return( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCopyKeyTo     Copy Key into new record
//------------------------------------------------------------------------------
// Function call:     QDAMCopyKeyTo( PBTREEBUFFER, SHORT, PBTREEBUFFER, SHORT);
//
//------------------------------------------------------------------------------
// Description:       Copy specified key to new record
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer from where to copy
//                    SHORT                  key to be used
//                    PBTREEBUFFER           record pointer to copy into
//                    SHORT                  key to be used
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     copy the key at the passed offset to the new location
//                    return
//------------------------------------------------------------------------------

VOID QDAMCopyKeyTo_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         i,
   PBTREEBUFFER_V3  pNew,
   SHORT         j,
   USHORT        usVersion             // version of database
)
{
   PUSHORT  pusOldOffset;              // offset of data
   PUSHORT  pusNewOffset;              // new data
   USHORT   usLen;                     // length of data
   USHORT   usLastPos;                 // last position filled
   USHORT   usDataOffs;                // data offset
   PUCHAR   pOldData;                  // pointer to data

   pusOldOffset = (PUSHORT) pRecord->contents.uchData;
   pusNewOffset = (PUSHORT) pNew->contents.uchData;
   usLastPos = pNew->contents.header.usLastFilled;

   usDataOffs = *(pusOldOffset+i);
   if ( usDataOffs)
   {
      pOldData = pRecord->contents.uchData + usDataOffs;
      usLen = *(PUSHORT) pOldData + sizeof(USHORT) + sizeof(RECPARAM);          

      usLastPos = usLastPos - usLen;
      memcpy( pNew->contents.uchData+usLastPos, pOldData, usLen );
      *(pusNewOffset+j) = usLastPos;
      pNew->contents.header.usFilled += (usLen + sizeof(USHORT));
      pNew->contents.header.usLastFilled = usLastPos;
   }
   else
   {
      *(pusNewOffset+j) = 0;
      pNew->contents.header.usFilled += sizeof(USHORT);
   } /* endif */

   return;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMReArrangeKRec    Rearrange key record
//------------------------------------------------------------------------------
// Function call:     QDAMReArrangeKRec( PBTREE, PBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Rearranges the key record, i.e. deletes physically any
//                    logically deleted keys.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           pointer to record
//
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get temp. record
//                    reset info in temp header
//                    for all key values
//                      if key exists
//                        copy key to temp buffer
//                      endif
//                    endfor
//                    copy record data back from temp to usual buffer
//------------------------------------------------------------------------------
VOID QDAMReArrangeKRec_V3
(
  PBTREE        pBTIda,
  PBTREEBUFFER_V3  pRecord
)
{
   PBTREEBUFFER_V3 pNew;
   PBTREEHEADER  pHeader;               // pointer to header
   PUSHORT     pusOffset;
   SHORT i;
   SHORT j;
   PBTREE    pBT = pBTIda;

   // get temp record
   pNew = &pBT->BTreeTempBuffer_V3;
   pHeader = &pNew->contents.header;
   pHeader->usOccupied = 0;
   pHeader->usFilled = sizeof(BTREEHEADER );
   pHeader->usLastFilled = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER );

   // for all key values move them into new record
   pusOffset = (PUSHORT) pRecord->contents.uchData;
   i = 0; j = 0;
   while ( j <  (SHORT) OCCUPIED( pRecord ))
   {
      if ( *(pusOffset+i) )
      {
         QDAMCopyKeyTo_V3( pRecord, i, pNew, j, pBT->usVersion );
         i++;
         j++;
      }
      else
      {
         i++;                                // it is a deleted key
      } /* endif */
   } /* endwhile */
   // copy record data back to avoid misplacement in file
   memcpy( pRecord->contents.uchData, pNew->contents.uchData, FREE_SIZE_V3 );
   pRecord->contents.header.usFilled = pNew->contents.header.usFilled;
   pRecord->contents.header.usLastFilled = pNew->contents.header.usLastFilled;

// pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

   return;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMSplitNode    Split a Node
//------------------------------------------------------------------------------
// Function call:     QDAMSplitNode( PBTREE, PPBTREEBUFFER, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       A node in the index is full, split the node.
//                    Insert the new key and value into the parent node.
//                    Return the node that the key belongs in.
//
//------------------------------------------------------------------------------
// Side effects:      Splitting a record will affect the parent node(s)
//                    of a record, too.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER  *        pointer to active node
//                    PCHAR                  key passed
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     lock the active node-record
//                    if active node is root (it needs to be split)
//                      allocate space for new record
//                      if ok
//                        lock the new record
//                        create a new root
//                        unlock the new record
//                        if ok
//                          update first_leaf information
//                        endif
//                        if ok
//                          write header to disk
//                        endif
//                      endif
//                    endif
//                    if ok
//                      allocate space for new record
//                      if ok
//                        lock new record
//                        if a next rec (child) exists
//                          read the next (child)record
//                          if ok then
//                            reset the ptr to prev. record in child rec.
//                            write this record
//                          endif
//                        endif
//                        if ok
//                          adjust sibling pointers
//                          get key string of middle key in record
//                          if possible
//                            set indicator = TRUE
//                          else
//                            set rc = BTREE_CORRUPTED
//                          endif
//                          if ok
//                            set start no. where to split record
//                            while counter not at last key in record
//                              copy key to new record
//                            endwhile
//                            adjust count of keys
//                            Rearrange the key record
//                            insert pointer to new record into parent node
//                            decide where to add the new key
//                            add key and write this record
//                          endif
//                        endif
//                        unlock new record
//                      endif
//                    endif
//                    unlock temp record
//------------------------------------------------------------------------------
SHORT QDAMSplitNode_V3
(
   PBTREE pBTIda,                // pointer to generic structure
   PBTREEBUFFER_V3 *record,      // pointer to pointer to node
   PWCHAR pKey                 // new key
)
{
  SHORT i,j;
  PBTREEBUFFER_V3 newRecord;
  PBTREEBUFFER_V3 child;
  PBTREEBUFFER_V3 parent = NULL;
  USHORT       usParent;               // number of parent
  PBTREEBUFFER_V3 pRecTemp;               // temporary buffer
  RECPARAM   recKey;                   // position/offset for key
  RECPARAM   recData;                  // position/offset for data
  PWCHAR    pParentKey;               // new key to be inserted
  PUSHORT    pusOffset;                // pointer to offset table
  BOOL       fCompare;                 // indicator where to insert new key
  USHORT     usFreeKeys = 0;               // number of free keys required
  PBTREE    pBT = pBTIda;

  SHORT sRc = 0;                       // success indicator


  //T5LOG(T5FATAL) << "called commented out function QDAMSplitNode_V3";
  //#ifdef TEMPORARY_COMMENTED
  memset( &recKey, 0, sizeof( recKey ) );
  memset( &recData,  0, sizeof( recData ) );

  pRecTemp = *record;
  BTREELOCKRECORD( pRecTemp );

  // if root needs to be split do it first
  if (IS_ROOT(*record))
  {
    sRc = pBTIda->QDAMNewRecord_V3( &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD( newRecord );
      /* We can't simply split a root node, since only one is allowed */
      /* so a new root is created to hold the records                 */
      pBT->usFirstNode = PARENT(*record) = RECORDNUM(newRecord);
      TYPE(newRecord) = ROOT_NODE | INNER_NODE | DATA_KEYNODE;
      PARENT(newRecord) = PREV(newRecord) = NEXT(newRecord) = 0L;
      TYPE(*record) &= ~ROOT_NODE;
      recData.usNum = RECORDNUM(*record);
      recData.usOffset = 0;
      
      //pParentKey = QDAMGetszKey_V3( *record, 0, pBT->usVersion );
      pParentKey = QDAMGetszKey_V3( *record, 0, pBT->usVersion );
      
      if ( pParentKey )
      {
         // store key temporarily, because record with key data is
         // not locked.
         sRc = pBTIda->QDAMInsertKey_V3( newRecord, pParentKey, recKey, recData);
         // might be freed during insert
         BTREELOCKRECORD( pRecTemp );
      }
      else
      {
        sRc = BTREE_CORRUPTED;
        ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 1, DB_GROUP, "" );
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
      // update usFirstLeaf information
      if ( !sRc )
      {
         sRc = QDAMFirstEntry_V3( pBTIda, &newRecord );
      } /* endif */
      if ( !sRc )
      {
         sRc = pBTIda->QDAMWriteHeader();
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
     // allocate space for the new record
    sRc = pBTIda->QDAMNewRecord_V3( &newRecord, KEYREC );
    if ( newRecord )
    {
      BTREELOCKRECORD(newRecord);
      if ( NEXT(*record) )
      {
        sRc = pBTIda->QDAMReadRecord_V3( NEXT(*record), &child, FALSE );
        if ( !sRc )
        {
          PREV(child) = RECORDNUM(newRecord);
          sRc = pBTIda->QDAMWriteRecord_V3(child);
        }
      }

      if ( !sRc )
      {
        /* Adjust Sibling pointers */
        NEXT(newRecord) = NEXT(*record);
        PREV(newRecord) = RECORDNUM(*record);
        NEXT(*record) = RECORDNUM(newRecord);
        PARENT(newRecord) = PARENT(*record);
        TYPE(newRecord) = (CHAR)(TYPE(*record) & ~ROOT_NODE);  // don't copy Root bit

        // Decide where to split the record
        //pParentKey = QDAMGetszKey_V3( *record, (SHORT)(OCCUPIED(*record)/2), pBT->usVersion );
        pParentKey = QDAMGetszKey_V3( *record, (SHORT)(OCCUPIED(*record)/2), pBT->usVersion );
        
        if ( pParentKey )
        {
           fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
           /***********************************************************/
           /* check in which part we will lay                         */
           /***********************************************************/
           if ( fCompare )
           {
              pParentKey = QDAMGetszKey_V3(*record, (SHORT)(OCCUPIED(*record)-MINFREEKEYS), pBT->usVersion);
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = MINFREEKEYS;
              }
              else
              {
                usFreeKeys = OCCUPIED( *record )/2;
              } /* endif */
           }
           else
           {
              pParentKey = QDAMGetszKey_V3( *record,MINFREEKEYS, pBT->usVersion);
              fCompare = ((*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0 ) ;
              if ( fCompare )
              {
                usFreeKeys = OCCUPIED( *record )/2;
              }
              else
              {
                usFreeKeys = OCCUPIED( *record ) -MINFREEKEYS;
              } /* endif */
           } /* endif */
        }
        else
        {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 2, DB_GROUP, "" );
        } /* endif */

        // leave space for at least nn further keys instead of splitting at half
        // regard the fact that IMPORT is normally in alphabetical increasing
        // order
        // in case of reverse copying ( as done in old DAM ) we are in the
        // ELSE case...
        if ( !sRc )
        {
           i = (SHORT) (OCCUPIED( *record ) - usFreeKeys);
           j = 0;
           pusOffset = (PUSHORT) (*record)->contents.uchData;
           while ( i < (SHORT) OCCUPIED( *record ))
           {
             QDAMCopyKeyTo_V3( *record, i, newRecord, j, pBT->usVersion );
             *(pusOffset+i) = 0 ;    // mark it as deleted
             j++;
             i++;
           }
           /* Adjust count of keys */
           OCCUPIED(*record) = OCCUPIED(*record) - usFreeKeys;
           OCCUPIED(newRecord) =  usFreeKeys;
           QDAMReArrangeKRec_V3( pBTIda, *record );
        /* Insert pointer to new record into the parent node */
        /* due to the construction parent MUST be the same   */
           sRc = QDAMFindParent_V3( pBTIda, *record, &usParent );
           if ( !sRc && usParent )
           {
             sRc = pBTIda->QDAMReadRecord_V3( usParent, &parent, FALSE );
           } /* endif */
        } /* endif */

        if ( !sRc )
        {
          recData.usNum = RECORDNUM( newRecord );
          recData.usOffset = 0;
          pParentKey = QDAMGetszKey_V3( newRecord,0, pBT->usVersion );
          if ( pParentKey )
          {
             sRc = pBTIda->QDAMInsertKey_V3( parent, pParentKey, recKey, recData);
          }
          else
          {
             sRc = BTREE_CORRUPTED;
             ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 3, DB_GROUP, "" );
          } /* endif */
        } /* endif */
      } /* endif */

      /* Decide whether to add the new key to the old or new record */
      if ( !sRc )
      {
//       newRecord->ulCheckSum = QDAMComputeCheckSum( newRecord );
//       (*record)->ulCheckSum = QDAMComputeCheckSum( *record );
         pParentKey = QDAMGetszKey_V3( newRecord, 0, pBT->usVersion );
         if ( pParentKey )
         {
           if ( (*(pBT->compare))(pBTIda, pParentKey, pKey) <= 0)
           {
             sRc = pBTIda->QDAMWriteRecord_V3( *record);
             *record = newRecord;                     // add to new record
           }
           else
           {
             sRc = pBTIda->QDAMWriteRecord_V3( newRecord );
           } /* endif */
         }
         else
         {
            sRc = BTREE_CORRUPTED;
            ERREVENT2( QDAMSPLITNODE_LOC, STATE_EVENT, 4, DB_GROUP, "" );
         } /* endif */
      } /* endif */
      BTREEUNLOCKRECORD(newRecord);
    } /* endif */
  } /* endif */
  BTREEUNLOCKRECORD( pRecTemp );

   if ( sRc )
   {
     ERREVENT2( QDAMSPLITNODE_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */
  
  //#endif

  return ( sRc );
}


#define LENGTHOFDATA( pBT, pData ) \
     *((PULONG)(pData)) 



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMUnTerseData      UnTerse Data                        |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMUnTerseData( PBTREE, PUCHAR, USHORT, PUSHORT );      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This function will run the passed string against         |
//|                   the decompression table.                                 |
//|                   If the resulting string will not fit in the passed       |
//|                   data area a warning return code is issued.               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PUCHAR                 pointer to data string            |
//|                   USHORT                 data length on input              |
//|                   PUSHORT                length of the string on output    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                     okay                               |
//|                   BTREE_BUFFER_SMALL    provided buffer too small          |
//|                   BTREE_NO_ROOM         not enough memory                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:      -- use the modified Huffman algorithm to unterse data   |
//|                   while remaining length > 0                               |
//|                     if usStartBit < 8 then                                 |
//|                       get next byte and add it to usCurByte                |
//|                       increase usStartBit                                  |
//|                     endif                                                  |
//|                     get the next three significant bits, adjust usStartBit |
//|                     decode them according to the decode table, i.e.        |
//|                      case 0:                                               |
//|                        get next character, decode it and put it into output|
//|                        string.                                             |
//|                      case 1, 2, 3:                                         |
//|                        just decode the character and put into output string|
//|                      case 4, 6:                                            |
//|                        get the next bit and decode it afterwards           |
//|                      case 5, 7:                                            |
//|                        get the next 2 bits and decode them afterwards      |
//|                      default:                                              |
//|                        we are out of synch, indicate that something went   |
//|                        wrong.                                              |
//|                                                                            |
//|                      endswitch                                             |
//|                   endwhile                                                 |
//|                   if enough space in passed record then                    |
//|                     copy data into passed record                           |
//|                   else                                                     |
//|                     set Rc to BTREE_BUFFER_SMALL                           |
//|                   endif                                                    |
//|                   return Rc                                                |
//+----------------------------------------------------------------------------+
SHORT QDAMUnTerseData
(
   PBTREE  pBTIda,                  //
   PUCHAR  pData,                   // pointer to data
   ULONG   ulDataLen,               // data length (uncompressed)
   PULONG  pulLen                   // length of the string
)
{
   ULONG    ulLen ;                    // length of string
   PUCHAR   pTempData;                 // pointer to temp data area
   USHORT   usCurByte = 0;             // current byte
   USHORT   usStartBit = 0;            // start bit looking at from high to low
   USHORT   usDecode;                  // value to be decoded
   SHORT    sRc = 0;                   // okay;
   PSZ      pInData = (PSZ)pData;           // pointer to input data
   PBTREE    pBT = pBTIda;
#if defined(MEASURE)
  ulBeg =  pGlobInfoSeg->msecs;
#endif

   // get length of passed string
   ulLen = ulDataLen;

   // try to uncompress the passed string ( use pTempData as temporary space)

   //if ( pBT->TempRecord )
   {
     pTempData = &pBT->TempRecord[0];

     while ( ulLen )
     {
       /***************************************************************/
       /* get next byte for decoding                                  */
       /***************************************************************/
       if ( usStartBit < 8 )
       {
         usCurByte = (usCurByte << 8) + *pData++;
         usStartBit += 8;
       } /* endif */

       /***************************************************************/
       /* get three significant bits                                  */
       /***************************************************************/
       usDecode = (usCurByte & us3BitNibble[usStartBit]) >> (usStartBit-3);
       usStartBit -= 3;

       /***************************************************************/
       /* decode the next character                                   */
       /***************************************************************/
       switch ( usDecode )
       {
         case  0:    // get next character
           if ( usStartBit < 8 )
           {
             usCurByte = (usCurByte << 8) + *pData++;
             usStartBit += 8;
           } /* endif */
           *pTempData++ =
              (CHAR) ((usCurByte & us8BitNibble[usStartBit]) >> (usStartBit-8));
           ulLen --;
           usStartBit -= 8;
           if ( usStartBit < 8 )
           {
             usCurByte = (usCurByte << 8) + *pData++;
             usStartBit += 8;
           } /* endif */
           break;
         case  1:
         case  2:
         case  3:    // we are done ...
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         case  4:
         case  6:    // get the next bit and decode it afterwards
           usDecode = (usDecode<<1) +
                     ((usCurByte & us1BitNibble[usStartBit])>> (usStartBit-1));
           usStartBit --;
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         case  5:
         case  7:    // get the next 2 bits and decode it afterwards
           usDecode = (usDecode<<2) +
                    ((usCurByte & us2BitNibble[usStartBit]) >> (usStartBit-2));
           usStartBit -= 2;
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         default :   // we are out of synch, should never happen.....
           sRc = BTREE_CORRUPTED;
           ulLen = 0;
           break;
       } /* endswitch */

     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* copy untersed data back ....                                    */
   /*******************************************************************/
   if ( !sRc  )
   {
     if ( *pulLen >= ulDataLen )
     {
       memcpy( pInData, &pBT->TempRecord[0], *pulLen );
       *pulLen = ulDataLen;
     }
     else
     {
      T5LOG(T5ERROR) << ":: BufferTooSmall 1";
      sRc = BTREE_BUFFER_SMALL;
     } /* endif */
   } /* endif */
#if defined(MEASURE)
  ulUnTerseEnd += (pGlobInfoSeg->msecs - ulBeg);
#endif
   return sRc;
}




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszData    Get Data String
//------------------------------------------------------------------------------
// Function call:     QDAMGetszData( PBTREE, PBTREEBUFFER, PCHAR,
//                                   RECPARAM, RECPARAM );
//------------------------------------------------------------------------------
// Description:       Get the data string for this offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE             pointer to btree structure
//                    PBTREEBUFFER       record where to insert the key
//                    PCHAR              key to be inserted
//                    RECPARAM           position/offset of the key string
//                    RECPARAM           position/offset of data (if data node)
//                                               else position of child record
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if string could be extracted
//                    FALSE  else
//
//------------------------------------------------------------------------------
// Function flow:     read record
//                    copy data
//                    if it is tersed
//                      set flag
//                      remove terse indication
//                      unterse data
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMGetszData_V3
(
   PBTREE    pBTIda,
   RECPARAM  recDataParam,
   PBYTE     pData,
   PULONG    pulDataLen,
   CHAR      chType                             // type of record key/data
)
{
   SHORT sRc = 0;
   PBTREEBUFFER_V3 pRecord;                         // pointer to record
   PBTREEHEADER pHeader;                         // pointer to header
   PCHAR        pTempData = NULL;                // pointer to data pointer
   PCHAR        pStartData = (PCHAR)pData;       // pointer to data pointer
   ULONG        ulLen = 0;                       // length of string
   ULONG        ulTerseLen = 0;                  // length of tersed string
   BOOL         fTerse = FALSE;                  // entry tersed??
   PUSHORT      pusOffset = NULL;                // pointer to offset
   ULONG        ulFitLen;                        // free to be filled length
   USHORT       usNum;                           // record number
   ULONG        ulLZSSLen;
   PBTREE   pBT = pBTIda;
   BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

   USHORT       usLenFieldSize;                            // size of record length field

   // get size of record length field
   usLenFieldSize = sizeof(ULONG);

   recDataParam.usNum = recDataParam.usNum + pBT->usFirstDataBuffer;
   // get record and copy data
   sRc = pBTIda->QDAMReadRecord_V3( recDataParam.usNum, &pRecord, FALSE  );
   if ( !sRc )
   {
      BTREELOCKRECORD( pRecord );
      fRecLocked = TRUE;
      if ( TYPE( pRecord ) != chType )
      {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 1, DB_GROUP, "" );
      }
      else
      {
         pusOffset = (PUSHORT) pRecord->contents.uchData;
         pusOffset += recDataParam.usOffset;        // point to key
         pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);
         if ( *pusOffset > BTREE_REC_SIZE_V3 )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 2, DB_GROUP, "" );
         }
         else
         {           
           ulLen = *((PULONG)pTempData);
           if ( ulLen & QDAM_TERSE_FLAGL )
           {
            fTerse = TRUE;
            ulLen &= ~QDAM_TERSE_FLAGL;
            ulLZSSLen = ulLen;         // length of compressed record ....
           } /* endif */
           // check if length is in a normal range
           if ( ulLen > 134217728L ) // for now assume that no record exceeds a size of 128 MB )
           {
             sRc = BTREE_CORRUPTED;
           }
           pTempData += sizeof(ULONG ); // get pointer to data
#ifdef _DEBUG
           {
             CHAR szTemp[40];
             sprintf( szTemp, "%ld", ulLen );
             INFOEVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, recDataParam.usNum,
                         DB_GROUP, szTemp );
           }
#endif
         
          pHeader = &(pRecord->contents.header);
         } /* endif */

         /*************************************************************/
         /* check if it is a valid length                             */
         /*************************************************************/
         if ( !sRc && (ulLen == 0) )
         {
           sRc = BTREE_CORRUPTED;
           ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, "" );
         } /* endif */
       } /* endif */

       if ( !sRc )
       {
         if ( *pulDataLen == 0 || !pData )
         {
            // give back only length
            if ( fTerse )
            {
              // first field contains real length
              *pulDataLen = LENGTHOFDATA(pBT,pTempData);
            }
            else
            {
               *pulDataLen = ulLen;    // give back only length
            } /* endif */
         }
         else if ( *pulDataLen >= ulLen )
         {
            ulFitLen = BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER) - *pusOffset;
            ulFitLen = ulLen < (ulFitLen - usLenFieldSize) ?  ulLen : (ulFitLen - usLenFieldSize) ;

            if ( fTerse )
            {
               memcpy(pData,pTempData+usLenFieldSize,ulFitLen-usLenFieldSize);
               ulTerseLen = LENGTHOFDATA(pBT,pTempData);
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += (ulFitLen - usLenFieldSize);
            }
            else
            {
               memcpy( pData, pTempData, ulFitLen );
               *pulDataLen = ulLen;
               /**********************************************************/
               /* adjust pointers                                        */
               /**********************************************************/
               ulLen -= ulFitLen;
               pData += ulFitLen;
            } /* endif */


            /**********************************************************/
            /* copy as long as still data are available               */
            /**********************************************************/
            while ( !sRc && ulLen )
            {
               usNum = NEXT( pRecord );
               BTREEUNLOCKRECORD( pRecord );
               fRecLocked = FALSE;
               if ( usNum )
               {
                  sRc = pBTIda->QDAMReadRecord_V3( usNum, &pRecord, FALSE  );
                  if ( !sRc && TYPE( pRecord ) != DATA_NEXTNODE )
                  {
                    sRc = BTREE_CORRUPTED;
                    ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 3, DB_GROUP, "" );
                  } /* endif */
                  if ( !sRc  )
                  {
                     BTREELOCKRECORD( pRecord );
                     fRecLocked = TRUE;
                     pusOffset = (PUSHORT) pRecord->contents.uchData;
                     pTempData = (PCHAR)(pRecord->contents.uchData + *pusOffset);

                     ulFitLen =  LENGTHOFDATA( pBT, pTempData );
                     ulFitLen = ulLen < ulFitLen?  ulLen: ulFitLen ;
                     pTempData += usLenFieldSize;      // get pointer to data

                     memcpy( pData, pTempData, ulFitLen );
                     ulLen -= ulFitLen;
                     pData += ulFitLen;
                  } /* endif */
               }
               else
               {
                 sRc = BTREE_CORRUPTED;
                 ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 4, DB_GROUP, "" );
               } /* endif */
            } /* endwhile */
            if ( !sRc && fTerse )
            {
              switch ( pBT->fTerse )
              {
                case  BTREE_TERSE_HUFFMAN :
                  {
                    sRc = QDAMUnTerseData( pBTIda, (PUCHAR)pStartData, ulTerseLen,
                                           pulDataLen );
                  }
                  break;
                default :
                  sRc = BTREE_CORRUPTED;
                  ERREVENT2( QDAMGETSZDATA_LOC, STATE_EVENT, 5, DB_GROUP, "" );
                  break;
              } /* endswitch */
            } /* endif */
         }
         else
         {
            T5LOG(T5ERROR) << ":: BufferTooSmall 3";
            sRc = BTREE_BUFFER_SMALL;
         } /* endif */

      } /* endif */
      if ( fRecLocked )
      {
        BTREEUNLOCKRECORD( pRecord );
      } /* endif */
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZDATA_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

   return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindChild       Find Child
//------------------------------------------------------------------------------
// Function call:     QDAMFindChild ( PBTREE, PCHAR, USHORT, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the childnode that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    if    ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFindChild_V3
(
    PBTREE pBTIda,
    PCHAR_W  pKey,
    USHORT   usNode,
    PBTREEBUFFER_V3 * ppRecord
)
{
  SHORT         sResult;
  SHORT         sLow;                              // Far left
  SHORT         sHigh;                             // Far right
  SHORT         sMid = 0;                          // Middle
  RECPARAM      recData;                           // data structure
  PCHAR_W       pKey2;                             // pointer to search key
  SHORT         sRc;                               // return code
  PBTREE    pBT = pBTIda;

  memset(&recData, 0, sizeof(recData));
  sRc = pBTIda->QDAMReadRecord_V3( usNode, ppRecord, FALSE  );
  if ( !sRc && !IS_LEAF( *ppRecord ))
  {
    BTREELOCKRECORD( *ppRecord );

    sLow = 0;                                    // start here
    sHigh = (SHORT) OCCUPIED( *ppRecord) -1 ;    // counting starts at zero


    while ( !sRc && (sLow <= sHigh) )
    {
      sMid = (sLow + sHigh)/2;
      pKey2 = QDAMGetszKey_V3( *ppRecord, sMid, pBT->usVersion );
      if ( pKey2 == NULL )
      {
        sRc = BTREE_CORRUPTED;
      }
      else
      {
        sResult = (*pBT->compare)(pBTIda, pKey, pKey2);
        if ( sResult < 0 )
        {
          sHigh = sMid - 1;                        // Go left
        }
        else
        {
          sLow = sMid + 1;                         // Go right
        } /* endif */
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* it is the lower one due the construction of the BTREE          */
    /*  -- according to the while loop it must be the sHIGH value     */
    /* if sHigh < 0 we should use the first one                      */
    /******************************************************************/
    if ( !sRc )
    {
      if ( sHigh == -1 )
      {
        sHigh = 0;
      } /* endif */
      recData = QDAMGetrecData_V3( *ppRecord, sHigh, pBT->usVersion );
    } /* endif */

    BTREEUNLOCKRECORD( *ppRecord );    // unlock previous record.

    if ( !sRc )
    {
      sRc = pBTIda->QDAMReadRecord_V3( recData.usNum, ppRecord, FALSE  );
    } /* endif */
  } /* endif */
  return( sRc );
}


SHORT QDAMChangeKey_V3
(
   PBTREE   pBTIda,                                 // ptr to tree structure
   USHORT   usNode,                                 // start node
   PWCHAR pOldKey,                                // find old key
   PWCHAR pNewKey                                 // find new key
)
{
  PBTREEBUFFER_V3 pRecord;                            // buffer for record
  PBTREEBUFFER_V3 pNewRecord = NULL;                  // buffer for new record
  SHORT i = 0;                                     // index
  SHORT sRc = 0;                                   // return code
  SHORT sNearKey;
  PUSHORT  pusOffset;                              // pointer to offset table
  RECPARAM recKey;                                 // record parameter descrip.


  sRc = pBTIda->QDAMReadRecord_V3( usNode, &pRecord, FALSE  );
  if ( !sRc )
  {
    BTREELOCKRECORD(pRecord);
    if ( !sRc )
    {
       /* Locate the Leaf node that contains the appropriate key */
       sRc = QDAMFindChild_V3( pBTIda, pNewKey, usNode, &pNewRecord );
    } /* endif */
    if ( !sRc )
    {
       // get the key description and insert the new key
       recKey.usNum = pNewRecord->contents.header.usNum;
       sRc = pBTIda->QDAMLocateKey_V3( pNewRecord, pNewKey, &i, FEXACT, &sNearKey);
       if ( !sRc && i != -1 )
       {
          recKey.usOffset = i;
          recKey.ulLen = 0;            // init length
          sRc = pBTIda->QDAMInsertKey_V3( pRecord, pNewKey, recKey, recKey);
       } /* endif */
    } /* endif */
    if ( !sRc )
    {
       sRc = pBTIda->QDAMLocateKey_V3( pRecord, pOldKey, &i, FEXACT, &sNearKey);
    } /* endif */

    if (!sRc && i!= -1)
    {
       // delete the oldkey at offset i
       pusOffset = (PUSHORT) pRecord->contents.uchData;
       *(pusOffset + i) = 0;
       OCCUPIED(pRecord) --;
       // record should be rearranged - it will be written during the insert
       // of the new key
       QDAMReArrangeKRec_V3( pBTIda, pRecord );

    }
    BTREEUNLOCKRECORD(pRecord);
  }

   if ( sRc )
   {
     ERREVENT2( QDAMCHANGEKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

  return sRc;
}


size_t BTREE::GetFileSize()const{
  return FilesystemHelper::GetFileSize(fb.fileName);
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMInsertKey       Insert a key
//------------------------------------------------------------------------------
// Function call:     QDAMInsertKey( PBTREE, PBTREEBUFFER, PCHAR,
//                                    RECPARAM, RECPARAM );
//------------------------------------------------------------------------------
// Description:       Add a key and offset to a record. If there is no room,
//                    the record is split in half and the key is add to the
//                    appropriate half. The key being inserted must be unique.
//------------------------------------------------------------------------------
// Parameters:        PBTREE             pointer to btree structure
//                    PBTREEBUFFER       record where to insert the key
//                    PCHAR              key to be inserted
//                    RECPARAM           position/offset of the key string
//                    RECPARAM           pos.   /offset of data (if data node)
//                                               else position of child record
//
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   if key could be inserted
//                    FALSE  else
//
//------------------------------------------------------------------------------
// Side effects:      Splitting a record will affect the parent node(s) of a
//                    record.
//------------------------------------------------------------------------------
// Function flow:     lock record
//                    check if key is already there
//                    if key found
//                      set RC=BTREE_DUPLICATE_KEY
//                    else
//                      get next free position in record
//                      if node is full
//                        split node before inserting
//                      endif
//                    endif
//                    find proper location in record to insert key
//                    set new current positioon
//                    fill in key data
//                    insert reference, set data information
//                    set address of key data at correct offset
//                    write record
//                    if this is first key in record
//                      adjust above pointers
//                    endif
//                    unlock record
//
//------------------------------------------------------------------------------
SHORT BTREE::QDAMInsertKey_V3
(
   PBTREEBUFFER_V3 pRecord,               // record where key is to be inserted
   PWCHAR     pKey,
   RECPARAM   recKey,                  // position/offset for key
   RECPARAM   recData                  // position/offset for data
)
{
  SHORT i = 0;
  PBTREEBUFFER_V3 pTempRec;
  PWCHAR   pCompKey = NULL;         // key to be compared with
  PWCHAR   pOldKey;                 // old key at first position
  PWCHAR   pNewKey;                 // new key at first position
  BOOL fFound = FALSE;
  SHORT sKeyFound;                    // key found
  SHORT  sNearKey;                     // key found
  SHORT  sRc = 0;                      // return code

  USHORT usLastPos;
  USHORT usKeyLen = 0;                 // length of key
  USHORT usDataRec = 0;                // length of key record
  PBYTE  pData;
  PUSHORT  pusOffset = NULL;
  USHORT _usCurrentRecord;              // current record
  SHORT  _sCurrentIndex;                // current index
  BOOL         fRecLocked = FALSE;    // TRUE if BTREELOCKRECORD has been done

  recKey;                              // get rid of compiler warnings
  //  check if key is already there -- duplicates will not be supported
  sRc = QDAMLocateKey_V3( pRecord, pKey, &sKeyFound, FEXACT, &sNearKey);
  BTREELOCKRECORD( pRecord );
  fRecLocked = TRUE;
  if ( !sRc )
  {
    if ( sKeyFound != -1)
    {
      sRc = BTREE_DUPLICATE_KEY;
    }
    else
    {
      i = (SHORT) OCCUPIED(pRecord);
      usKeyLen = sizeof(ULONG);

      usDataRec = usKeyLen + sizeof(USHORT) + sizeof(RECPARAM);
       
      /* If node is full. Split the node before inserting */
      if ( usDataRec+sizeof(USHORT)+pRecord->contents.header.usFilled > BTREE_REC_SIZE_V3 )
      {
        BTREEUNLOCKRECORD( pRecord );
        fRecLocked = FALSE;
        sRc = QDAMSplitNode_V3( this, &pRecord, pKey );
        if ( !sRc )
        {
          // SplitNode may have passed a new record back
          BTREELOCKRECORD( pRecord );
          fRecLocked = TRUE;

          /* if we split an inner node, the parent of the key we are */
          /* inserting must be changed                               */
          if ( !IS_LEAF( pRecord ) )
          {
            sRc = QDAMReadRecord_V3( recData.usNum, &pTempRec, FALSE );
            if ( ! sRc )
            {
              PARENT( pTempRec ) = RECORDNUM( pRecord );
//            pTempRec->ulCheckSum = QDAMComputeCheckSum( pTempRec );
              sRc = QDAMWriteRecord_V3( pTempRec );
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */


  if ( !sRc )
  {
    i = (SHORT) OCCUPIED(pRecord);               // number of keys in record
    // Insert key at the proper location
    pusOffset = (PUSHORT) pRecord->contents.uchData;
    while ( i > 0 && !fFound )
    {
      pCompKey = QDAMGetszKey_V3( pRecord, (SHORT)(i-1), usVersion );
      if ( pCompKey )
      {
        if ( (*(compare))(this, pKey, pCompKey)  < 0 )
        {
          *(pusOffset+i) = *(pusOffset+i-1);
          i--;
        }
        else
        {
          fFound = TRUE;
        } /* endif */
      }
      else
      {
        sRc = BTREE_CORRUPTED;
        ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 1, DB_GROUP, "" );
      } /* endif */
    } /* endwhile */

    // set new current position
    sCurrentIndex = i;
    usCurrentRecord = RECORDNUM( pRecord );
  } /* endif */

  if ( !sRc )
  {
    CHAR chHeadTerm[128];
    // fill in key data
    usLastPos = pRecord->contents.header.usLastFilled;
    usLastPos = usLastPos - usDataRec;
    pData = pRecord->contents.uchData + usLastPos;

    *(PWCHAR) pData = usKeyLen;
    {
      PBYTE pTarget;
      pTarget = pData + sizeof(USHORT) + sizeof(RECPARAM);
      memcpy(pTarget, (PBYTE) pKey, usKeyLen );
    }
    // set data information      
    memcpy((PRECPARAM) (pData+sizeof(USHORT)), &recData, sizeof(RECPARAM));
      
    // set address of key data at offset i
    *(pusOffset+i) = usLastPos;

    OCCUPIED(pRecord) += 1;
    pRecord->contents.header.usLastFilled = usLastPos;
    pRecord->contents.header.usFilled += usDataRec + sizeof(USHORT);
//  pRecord->ulCheckSum = QDAMComputeCheckSum( pRecord );

    sRc = QDAMWriteRecord_V3(pRecord);
  } /* endif */

  // If this is the first key in the record, we must adjust pointers above
  if ( !sRc )
  {
    if ((i == 0) && (!IS_ROOT(pRecord)))
    {
      pOldKey = QDAMGetszKey_V3( pRecord, 1, usVersion );
      pNewKey = QDAMGetszKey_V3( pRecord, 0, usVersion );
      if ( pOldKey && pNewKey )
      {
        if ( pCompKey && PARENT(pRecord) )
        {
          /**********************************************************/
          /* save old current record since ChangeKey will update it */
          /* (implicit call to QDAMInsertKey) and restore it ...   */
          /**********************************************************/
          _usCurrentRecord = usCurrentRecord;
          _sCurrentIndex   = sCurrentIndex;
          sRc = QDAMChangeKey_V3( this, PARENT(pRecord), pOldKey, pNewKey);
          usCurrentRecord = _usCurrentRecord;
          sCurrentIndex   = _sCurrentIndex;

        } /* endif */
      }
      else
      {
        sRc = BTREE_CORRUPTED;
        ERREVENT2( QDAMINSERTKEY_LOC, STATE_EVENT, 10, DB_GROUP, "" );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( pRecord && fRecLocked )
  {
    BTREEUNLOCKRECORD(pRecord);
  } /* endif */

  if ( sRc )
  {
    ERREVENT2( QDAMINSERTKEY_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetrecData   Get data pointer back
//------------------------------------------------------------------------------
// Function call:     QDAMGetrecData( PBTREEBUFFER, SHORT );
//
//------------------------------------------------------------------------------
// Description:       Get data record pointer back
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           record pointer
//                    SHORT                  key to be used
//
//
//------------------------------------------------------------------------------
// Returncode type:   RECPARAM
//------------------------------------------------------------------------------
// Returncodes:       filled recparam structure
//
//------------------------------------------------------------------------------
// Function flow:     use offset in table and point to data description
//                    copy it data description structure for return
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindRecord      Find Record
//------------------------------------------------------------------------------
// Function call:     QDAMFindRecord( PBTREE, PCHAR, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       Locates the record containing the leaf node that the
//                    given key is in or should be in if inserted.
//
//                    Returns record pointer that might contain the record,
//                    or NULL if it is unable to find it due to lack of buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key passed
//                    PPBTREEBUFFER          pointer to leaf mode
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record of 1st node
//                    while ok and record is not the leaf
//                      lock the record
//                      init for binary search (starting points)
//                      if lower start point < upper start point
//                        do
//                          set fFound
//                          get key string at middle position
//                          compare key found with the passed key
//                          decide whether to go left or right
//                        while not found
//                        if ok
//                          if sResult = '>'
//                            record found (sLow fits)
//                          else
//                            additional check: get key string
//                            compare keys
//                          endif
//                        endif
//                      else
//                        lower start point is position searched for
//                      endif
//                      if ok
//                        get data in data structure
//                        unlock previous record
//                        read record
//                    endwhile
//------------------------------------------------------------------------------


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     UtlSetFHandState/...Hwnd DosSetFHandState: set file state|
//+----------------------------------------------------------------------------+
//|Function call:     UtlSetFHandState( HFILE hf, USHORT fsState, BOOL fMsg ); |
//+----------------------------------------------------------------------------+
//|Description:       Interface function to DosSetFHandState                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   HFILE    hf             handle of an openend file        |
//|                   USHORT   fsState        new state for file               |
//|                   BOOL     fMsg           if TRUE handle errors in utility |
//|                 ( HWND     hwnd )                                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       return code of DosSetFHandState                          |
//+----------------------------------------------------------------------------+
//|Function flow:     call DosSetFHandState                                    |
//+----------------------------------------------------------------------------+
USHORT UtlSetFHandState( HFILE hf, USHORT fsState, BOOL fMsg )
{
  return UtlSetFHandStateHwnd( hf, fsState, fMsg, (HWND)NULL );
}

USHORT UtlSetFHandStateHwnd( HFILE hf, USHORT fsState, BOOL fMsg, HWND hwnd )
{
   USHORT usRetCode = 0;               // function return code
   USHORT usMBCode = 0;                    // message box/UtlError return code

   do {
      DosError(0);
      DosError(1);
      if ( fMsg && usRetCode )
      {
         usMBCode = MB_CANCEL;
         T5LOG(T5ERROR) <<  ":: rc = " << usRetCode;
      } /* endif */
   } while ( fMsg && usRetCode && (usMBCode == MBID_RETRY) ); /* enddo */
   return( usRetCode );
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictOpenLocal   Open local Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictOpenLocal( PSZ, SHORT, BOOL, PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Open a file locally for processing
//
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    SHORT            number of bytes per record
//                    BOOL             TRUE  read/write FALSE  read/only
//                    PPBTREE          pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_ILLEGAL_FILE not a valid dictionary
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     Open the file read/write or readonly
//                    if ok
//                      read in header
//                      if ok
//                        check for validity of the header
//                        if ok
//                          fill tree structure
//                          call UtlQFileInfo to get the next free record
//                          if ok
//                            allocate buffer space
//                            if alloc fails
//                              set Rc = BTREE_NO_ROOM
//                            endif
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_READ_ERROR
//                      endif
//                      if error
//                        close dictionary
//                      endif
//                      if read/write open was done
//                        set open flag and write it into file
//                      endif
//                      get corruption flag and set rc if nec.
//                    else
//                     set Rc = BTREE_OPEN_ERROR
//                    endif
//                    return Rc
//------------------------------------------------------------------------------
SHORT  BTREE::QDAMDictOpenLocal
(
  SHORT sNumberOfBuffers,             // number of buffers
  USHORT usOpenFlags                 // Read Only or Read/Write
)
{
  SHORT     i;
  BTREEHEADRECORD header;
  SHORT sRc = 0;                      // return code
  USHORT  usFlags;                    // set the open flags
  USHORT  usAction;                   // return code from UtlOpen
  USHORT  usNumBytesRead=0;           // number of bytes read
  //PBTREE  pBTIda = this;            // set work pointer to passed pointer
  //PBTREE pBT;                     // set work pointer to passed pointer
  SHORT   sRc1;
  /*******************************************************************/
  /* allocate global area first ...                                  */
  /*******************************************************************/

  usFlags = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE;

  /****************************************************************/
  /* check if immeadiate write of any changed necessary           */
  /****************************************************************/
  fGuard = ( usOpenFlags & ASD_FORCE_WRITE );
  usOpenFlags = usOpenFlags;

  // open the file
  fb.ReadFromFile();

  if ( !sRc )
  {
    // Read in the data for this index, make sure it is an index file
    sRc = fb.Read((PVOID)&header, sizeof(BTREEHEADRECORD), 0);
    //sRc = usNumBytesRead !=sizeof(BTREEHEADRECORD);
    if ( ! sRc )
    {
      memcpy( chEQF, header.chEQF, sizeof(chEQF));
      bVersion = header.Flags.bVersion;

      /**************************************************************/
      /* support either old or new format or TM...                  */
      /**************************************************************/
      if ( strncmp( header.chEQF, BTREE_HEADER_VALUE_TM3,
                    sizeof(BTREE_HEADER_VALUE_TM3) ) == 0 )
      {
      /************************************************************/
      /* check if we are dealing with a TM...                     */
      /* (Check only first 3 characters of BTREE identifier to    */
      /*  ignore the version number)                              */
      /************************************************************/
        usVersion = (USHORT) header.chEQF[3];
        compare =  NTMKeyCompare;

        UtlTime( &(lTime) );                             // set open time
        sCurrentIndex = 0;
        usFirstNode = header.usFirstNode;
        usFirstLeaf = header.usFirstLeaf;
        usCurrentRecord = 0;
        usFreeKeyBuffer = header.usFreeKeyBuffer;
        usFreeDataBuffer = header.usFreeDataBuffer;
        usFirstDataBuffer = header.usFirstDataBuffer;  //  data buffer
        fpDummy = NULLHANDLE;          

        // load usNextFreeRecord either from header record of from file info
        {
          usNextFreeRecord = (USHORT)(fb.data.size()/BTREE_REC_SIZE_V3);
        } /* endif */
        ASDLOG();

        if ( !sRc )
        {
          memcpy(DataRecList, header.DataRecList, MAX_LIST*sizeof(DataRecList[0]));

          memcpy( chEntryEncode,header.chEntryEncode,ENTRYENCODE_LEN );
          fTerse = header.fTerse;
          if ( fTerse == BTREE_TERSE_HUFFMAN )
          {
            QDAMTerseInit( this, chEntryEncode );   // init compression
          } /* endif */

          memcpy( &chCollate, header.chCollate, /*COLLATE_SIZE*/ sizeof(chCollate) );  
          memcpy( chCaseMap, header.chCaseMap, COLLATE_SIZE );
            
          ASDLOG();

          /* Allocate space for AccessCtrTable */
          usNumberOfLookupEntries = 0;
          UtlAlloc( (PVOID *)&AccessCtrTable, 0L, (LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(ACCESSCTRTABLEENTRY), NOMSG );
          if ( !AccessCtrTable )
          {
            sRc = BTREE_NO_ROOM;
          } /* endif */

          /* Allocate space for LookupTable */
          if ( !sRc )
          {
            UtlAlloc( (PVOID *)&LookupTable_V3, 0L,(LONG) MIN_NUMBER_OF_LOOKUP_ENTRIES * sizeof(LOOKUPENTRY_V3), NOMSG );
            if ( LookupTable_V3 )
            {
              usNumberOfLookupEntries = MIN_NUMBER_OF_LOOKUP_ENTRIES;
            }
            else
            {
              sRc = BTREE_NO_ROOM;
            } /* endif */
          } /* endif */

          usNumberOfAllocatedBuffers = 0;
        } /* endif */

        if ( !sRc )
        {
          ASDLOG();
        } /* endif */
      }
      else
      {
        T5LOG(T5ERROR) <<"Can't understand file " << fb.fileName << ". File has illegal structure!";
           sRc = BTREE_ILLEGAL_FILE;
      } /* endif */
    }
    else
    {
      sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, usOpenFlags );
    } /* endif */
    ASDLOG();
    // close in case of error
    if ( sRc  )
    {
      QDAMDictClose();
    } /* endif */
    ASDLOG();

    if ( !sRc && /*fWrite &&*/ !header.fOpen)
    {
      fOpen = TRUE;                       // set open flag
    } /* endif */

    #ifdef TEMPORARY_COMMENTED
    // get the corruption flag but still go ahead - nec. for Organize
    // ignore the corruption flag for shared resources (here we allow
    // open of our database more than once)
    if ( !sRc && header.fOpen &&
        !(pBT->usOpenFlags & (ASD_SHARED | ASD_NOOPENCHECK)) )
    {
      sRc = BTREE_CORRUPTED;

      /**************************************************************/
      /* if in Write Mode opened disable write in case of Corruption*/
      /* and reset it to Read/Only mode                             */
      /**************************************************************/
      if ( fWrite  )
      {
        i  = UtlSetFHandState( pBT->fb.file, OPEN_ACCESS_READONLY, FALSE );
        if ( i )               // handle could not be set read/only
        {
          pBT->fCorrupted = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */
    #endif //TEMPORARY_COMMENTED
    ASDLOG();
  }
  else
  {
    sRc = QDAMDosRC2BtreeRC( sRc, BTREE_OPEN_ERROR, usOpenFlags );
  } /* endif */

  /*******************************************************************/
  /* set BTREE pointer in case it was changed or freed               */
  /*******************************************************************/
  //*ppBTIda = pBTIda;

  /*******************************************************************/
  /* add the dictionary to the open list ...                         */
  /*******************************************************************/
  if ( !sRc || (sRc == BTREE_CORRUPTED) )
  {
    QDAMAddDict( (PSZ)fb.fileName.c_str(), this );
    /*****************************************************************/
    /* add the lock if necessary                                     */
    /*****************************************************************/
    if ( usOpenFlags & ASD_LOCKED )
    {
      QDAMDictLockDictLocal( TRUE );
    } /* endif */
  } /* endif */

  if ( sRc )
  {
    T5LOG(T5ERROR) << ":: sRc" << sRc;
    ERREVENT2( QDAMDICTOPENLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */
  fOpen = true;
  return ( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddToBuffer       Add Data to buffer
//------------------------------------------------------------------------------
// Function call:     QDAMAddToBuffer( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Add the passed data to the string buffer
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be inserted
//                    PCHAR                  buffer for user data
//                    PRECPARAM              contains position of data stored
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if terse flag is on
//                      terse the data
//                    endif
//                    get list for currently used record type
//                    check if sufficient space is available to
//                       fit into one of these records
//                    if it fits
//                      read record
//                      fill in key data, insert reference
//                      set address of key data
//                      mark if tersed or not
//                      write record update list entry
//                    endif
//                    if record is too large or no space left in list of recs
//                       scan list and find empty slot
//                       if not found
//                         remove slot which is most filled
//                       endif
//                       get record and store start position
//                       fill it up to length or record size
//                       update list entry
//                    endif
//                    return record parameters
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszKey    Get Key String
//------------------------------------------------------------------------------
// Function call:     QDAMGetszKey( PBTREEBUFFER, USHORT, PCHAR );
//
//------------------------------------------------------------------------------
// Description:       Get the key string for this offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREEBUFFER           active record
//                    USHORT                 get data for this offset
//
//------------------------------------------------------------------------------
// Returncode type:   PSZ
//------------------------------------------------------------------------------
// Returncodes:       pointer to data
//
//------------------------------------------------------------------------------
// Function flow:     point to key
//                    set data pointer
//                    pass it back
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMLocateKey     Locate a key in passed record
//------------------------------------------------------------------------------
// Function call:     QDAMLocateKey( PBTREE, PBTREEBUFFER, PCHAR, PSHORT,
//                                    SEARCHTYPE, PSHORT );
//------------------------------------------------------------------------------
// Description:       locate a key in the passed record via a binary search
//                    Either pass back the position of the key or -1 if
//                    not found
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           record to be dealt with
//                    PCHAR                  key to be searched
//                    PSHORT                 located key position
//                    SEARCHTYPE             Exact, substring or equivalent
//                    PSHORT                 near position
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       -1         if key is not in record
//                    position   position of key
//
//------------------------------------------------------------------------------
// Side Effects:      record will be temporarily locked and unlocked at
//------------------------------------------------------------------------------
// Function flow:     if record exists
//                      lock record
//                      set upper/lower start points for binary search
//                      if valid area for search
//                        while key not found
//                          get key in the middle of lower/upper
//                          decide where to go on (lower/upper part)
//                          if match found
//                            if exact match is required
//                               ensure it and check prev/next
//                               (insertion is case sensitive)
//                            endif
//                          endif
//                        endwhile
//                        if ok
//                          ensure that end slots are checked
//                        endif
//                      endif
//                      unlock record
//                    endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictInsertLocal   Insert entry
//------------------------------------------------------------------------------
// Function call:     QDAMDictInsertLocal( PBTREE, PCHAR, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Add a key and all associated data.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE          pointer to btree structure
//                    PCHAR           key to be inserted
//                    PCHAR           user data to be associated with the key
//                    USHORT          length of the user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_DATA_RANGE  key to long or too short
//------------------------------------------------------------------------------
// Function flow:     if dictionary corrupted then
//                      set Rc to BTREE_CORRUPTED
//                    else
//                      find the appropriate record and set Rc accordingly
//                    endif
//                    if okay then
//                      add data to buffer and set Rc
//                      if okay then
//                        insert the key and the data and set Rc
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT BTREE::QDAMDictInsertLocal
(
  PWCHAR pKey,             // pointer to key data
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data in bytes
)
{
  SHORT sRc = 0;                            // return code
  RECPARAM   recData;                       // offset/node of data storage
  RECPARAM   recKey;                        // offset/node of key storage
  USHORT     usKeyLen;                      // length of the key

  memset( &recKey, 0, sizeof( recKey ) );
  memset( &recData,  0, sizeof( recData ) );

  if ( fCorrupted )
  {
    sRc = BTREE_CORRUPTED;
  }
  else if ( ulLen == 0)
  {
    sRc = BTREE_DATA_RANGE;
  }

  /*******************************************************************/
  /* check if entry is locked ....                                   */
  /*******************************************************************/
  if ( !sRc && QDAMDictLockStatus( pKey ) )
  {
    sRc = BTREE_ENTRY_LOCKED;
  } /* endif */ 
   
  PBTREEBUFFER_V3  pNode = NULL;
  if ( !sRc )
  {
    usKeyLen = (USHORT) sizeof(ULONG);
    memcpy( (PBYTE)chHeadTerm, (PBYTE)pKey, usKeyLen);//+sizeof(TMWCHAR) );   // save current data
      
    QDAMDictUpdStatus ();
    sRc = QDAMFindRecord_V3( pKey, &pNode );
  } /* endif */

  if ( !sRc )
  {
    BTREELOCKRECORD( pNode );
    sRc = QDAMAddToBuffer_V3(  pData, ulLen, &recData );
    if ( !sRc )
    {
      if(ulLen > TMX_REC_SIZE && T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
        T5LOG(T5ERROR) <<  ":: tried to set bigget ulLen than rec size, ulLen = " << ulLen;
      }
      recData.ulLen = ulLen;
      sRc = QDAMInsertKey_V3 ( pNode, pKey, recKey, recData);
    } /* endif */
    BTREEUNLOCKRECORD( pNode );
    /****************************************************************/
    /* change time to indicate modifications on dictionary...       */
    /****************************************************************/
    lTime ++;
  }
      

  if ( sRc )
  {
    ERREVENT2( QDAMDICTINSERTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdSignLocal Write User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );
//
//------------------------------------------------------------------------------
// Description:       Writes the second part of the first record (user data)
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    USHORT                 length of user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     if tree is corrupted
//                      set sRC = BTREE_CORRUPTED
//                    else
//                      give 1K at beginning as space (UtlChgFilePtr)
//                    endif
//                    if ok then
//                      check if length of user data is correct
//                      if ok then
//                        write length of UserData to disk
//                        check if disk is full
//                        if ok
//                          write pUserData to the disk
//                          check if disk is full
//                        endif
//                      endif
//                      if ok
//                        fill rest up with zeros
//                        check if disk is full
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT BTREE::QDAMDictUpdSignLocal
(
   PTMX_SIGN  pSign                   // pointer to user data
)
{
  ULONG ulLen = sizeof(TMX_SIGN);

  SHORT  sRc=0;                        // return code
  ULONG   ulDataLen;                   // number of bytes to be written
  PCHAR   pchBuffer;                   // pointer to buffer
  LONG    lFilePos;                    // file position to position at
  ULONG   ulNewOffset;                 // new offset
  int writingPosition = 0;
  if ( fCorrupted )
  {
    sRc = BTREE_CORRUPTED;
  }
  else
  {
    // let 2K at beginning as space
    writingPosition = (LONG) USERDATA_START;
    sRc = fb.SetOffset( writingPosition, FILE_BEGIN); 
  } 

  ulDataLen = BTREE_REC_SIZE_V3 - USERDATA_START - sizeof(USHORT);
  ulDataLen = ulLen < ulDataLen? ulLen : ulDataLen;

  if ( ! sRc )
  {
    if ( ulDataLen < ulLen )
    {
      //T5LOG( T5WARNING) <<"TEMPORARY_HARDCODED:: QDAMDictUpdSignLocal ulDataLen < ulLen => ulLen = ulDataLen");
      //ulLen = ulDataLen;
      T5LOG(T5ERROR) <<"QDAMDictUpdSignLocal ulDataLen < ulLen, ulDataLen = " << ulDataLen << ", ulLen = " << ulLen;
      sRc = BTREE_USERDATA;
    }
    else
    {
      if ( !pSign  )
      {
        T5LOG(T5ERROR)<<"pSing is nullptr!"; 
        throw;
      } /* endif */

      /*************************************************************/
      /*  write length of userdata                                 */
      /*************************************************************/
      if ( ! sRc ) {
        USHORT usDataLen = (USHORT)ulDataLen;
        sRc = fb.Write((PVOID) &usDataLen, sizeof(USHORT), writingPosition);         
        // check if disk is full
        if ( ! sRc )
        {
         ulDataLen = usDataLen;
         writingPosition += sizeof(USHORT);
        }
      }
       
       if ( ! sRc)
       {
        /***********************************************************/
        /* write user data itselft                                 */
        /***********************************************************/
        sRc = fb.Write((PVOID) pSign, ulDataLen, writingPosition);
        if ( ! sRc )
        {
          writingPosition += ulDataLen;
        }
       } /* endif */
    } /* endif */
    if ( ! sRc )
    {
      fb.SetOffset(BTREE_REC_SIZE_V3, FILE_BEGIN);
    } /* endif */
  } 

  if ( sRc )
  {
    T5LOG(T5ERROR) << "QDAMDictUpdSignLocal:: sRc = " << sRc;
    ERREVENT2( QDAMUPDSIGNLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
  } 

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictSignLocal    Read User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Gets the second part of the first record ( user data )
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//
//------------------------------------------------------------------------------
// Side effects:      return signature record even if dictionary is corrupted
//
//------------------------------------------------------------------------------
// Function flow:
//                    position to begin of data start in first record
//                    if not possible set  Rc = BTREE_READ_ERROR
//                    if not Rc
//                      read in length of userdata
//                      if ok
//                        if user requests only length
//                          return only length
//                        else
//                          if length not correct
//                            set Rc =BTREE_USERDATA
//                          else
//                            read in user data
//                          endif
//                        endif
//                      else
//                        set Rc = BTREE_READ_ERROR
//                      endif
//                    endif
//------------------------------------------------------------------------------

SHORT BTREE::QDAMDictSignLocal
(
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  SHORT  sRc=0;                        // return code
  USHORT  usNumBytesRead;              // bytes read from disk
  ULONG   ulNewOffset;                 // new offset
  USHORT  usLen;                       // contain length of user record

  //sRc = SkipBytesFromBeginningInFile(pBT->fb.file, USERDATA_START);
  sRc = fb.Read((PVOID) &usLen, sizeof(USHORT), USERDATA_START);
  if(sRc){
    T5LOG(T5ERROR) << "Can't read from file \'"<<fb.fileName<<"\'";
  }else {
     ASDLOG();    
     //sRc = UtlRead( fb.file, (PVOID) &usLen, sizeof(USHORT),
     //               &usNumBytesRead, FALSE );
     //if ( !sRc )
     //if()
     {
        // user requests only length or full data
        if ( ! pUserData  || *pusLen == 0 )
        {
           *pusLen = usLen;         // return only length
        }
        else
        {
           if ( *pusLen <  usLen )
           {
              sRc = BTREE_USERDATA;
           }
           else
           {
              // read in data part
              //sRc = UtlRead( pBT->fb.file, pUserData, usLen,
              //               &usNumBytesRead, FALSE );
              sRc = fb.Read(pUserData, usLen);
              if ( !sRc )
              {
                 *pusLen = usLen;
              }
              else
              {
                T5LOG(T5ERROR) << "Can't read pUserData!";
              } /* endif */
           } /* endif */
        } /* endif */

        ASDLOG();
     }
  } /* endif */
  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCheckDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       check if the specified dictionary is already open
//------------------------------------------------------------------------------
// Parameters:        PSZ    pName,           name of dictionary
//                    PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_DICT_LOCKED   dictionary is locked
//                    BTREE_MAX_DICTS     maximum of dictionaries exceeded
//------------------------------------------------------------------------------
// Function flow:     check if dict is already open
//                    if so use this handle and check for lock;
//                    if locked then
//                      set return code to locked
//                    else
//                      copy significant data into our global structure
//                    endif
//                    if too many open dicts
//                      set return code to BTREE_MAX_DICTS
//                    endif
//                    return success
//
//------------------------------------------------------------------------------
SHORT
BTREE::QDAMCheckDict
(
  PSZ    pName                        // name of dictionary
)
{
  return ( 0 );
} /* end of function QDAMCheckDict */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMLocSubstr   locate the substring in the record
//------------------------------------------------------------------------------
// Function call:     QDAMLocSubStr(PBTREE, PBTREEBUFFER, PCHAR, PCHAR,
//                                  PUSHORT, PCHAR, PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    If no error happened set this location as new
//                    current position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PBTREEBUFFER           currently active key
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     locate the key with option set to FSUBSTR; set Rc
//                    if key found then
//                      check that key fulfills substring option
//                      if not okay
//                        set current position to the nearest key
//                        set Rc  = BTREE_NOT_FOUND
//                      else
//                        set current position to the found key
//                        pass back either length only or already data,
//                         depending on user request.
//                      endif
//                    else
//                      set Rc to BTREE_NOT_FOUND
//                      set current position to the nearest key
//                    endif
//                    return Rc
//------------------------------------------------------------------------------
SHORT QDAMLocSubstr_V3
(
   PBTREE        pBTIda,
   PBTREEBUFFER_V3  pRecord,
   PCHAR_W       pKey,
   PBYTE         pchBuffer,            // space for key data
   PULONG        pulLength,            // in/out length of returned key data
   PBYTE         pchUserData,          // space for user data
   PULONG        pulUserLen            // in/out length of returned user data
)
{
  SHORT  i;                            // index
  SHORT  sNearKey;                     // nearest key
  USHORT   usLen;                      // length of key
  SHORT    sRc  = 0;                   // return code
  PCHAR_W  pKey2;                      // key to be compared with
  RECPARAM recData;
  PBTREE    pBT = pBTIda;

  sRc = pBTIda->QDAMLocateKey_V3( pRecord, pKey, &i, FSUBSTR, &sNearKey);
  if ( !sRc )
  {
     if ( i != -1 )
     {
       sNearKey = i;
     } /* endif */

     // set new current position
     pBTIda->sCurrentIndex = sNearKey;
     pBTIda->usCurrentRecord = RECORDNUM( pRecord );

     BTREELOCKRECORD( pRecord );
     //  check if the key fulfills the substring option
     if ( sNearKey != -1)
     {
       pKey2 = QDAMGetszKey_V3( pRecord, sNearKey, pBT->usVersion );
       if ( pKey2  )
       {
         if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
         {
           if ( sNearKey < (SHORT)OCCUPIED(pRecord)-1 )
           {
             sNearKey++;
           } /* endif */

           pKey2 = QDAMGetszKey_V3( pRecord, sNearKey, pBT->usVersion );
           if ( pKey2 )
           {
             if ( UTF16strnicmp(pKey, pKey2, (USHORT)UTF16strlenCHAR(pKey)) )
             {
               SET_AND_LOG(sRc, BTREE_NOT_FOUND);
             }
             else
             {
               pBTIda->sCurrentIndex = sNearKey;
             } /* endif */
           }
           else
           {
             SET_AND_LOG(sRc, BTREE_NOT_FOUND);
           } /* endif */
         } /* endif */
       }
       else
       {
         sRc = BTREE_CORRUPTED;
         ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 1, DB_GROUP, "" );
       } /* endif */

       if ( !sRc )
       {
         usLen = (USHORT)(UTF16strlenBYTE( pKey2 ) + sizeof(CHAR_W));
         if ( !pchBuffer || *pulLength == 0)
         {
            *pulLength = usLen ;  // give back length only
         }
         else
         {
            if ( *pulLength >= usLen )
            {
               *pulLength = usLen;
               memcpy( pchBuffer, pKey2, usLen );
            }
            else
            {
               //ERREVENT2( QDAMLOCSUBSTR_LOC, STATE_EVENT, 2, DB_GROUP, "" );
               T5LOG(T5ERROR) <<":: BufferTooSmall 4";
               sRc = BTREE_BUFFER_SMALL;
            } /* endif */
         } /* endif */
         if ( !sRc )
         {
            recData = QDAMGetrecData_V3( pRecord, i, pBT->usVersion );
            if ( *pulUserLen == 0 || ! pchUserData )
            {
               *pulUserLen = recData.ulLen;
            }
            else
            {
               sRc =  QDAMGetszData_V3( pBTIda, recData, pchUserData, pulUserLen, DATA_NODE );
             } /* endif */
         } /* endif */
       } /* endif */
     }
     else
     {
        SET_AND_LOG(sRc, BTREE_NOT_FOUND);
     } /* endif */
     BTREEUNLOCKRECORD( pRecord );
  } /* endif */

  return sRc;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMValidateIndex    Find next valid position
//------------------------------------------------------------------------------
// Function call:     QDAMValidateIndex( PBTREE, PPBTREEBUFFER );
//
//------------------------------------------------------------------------------
// Description:       This function will return the next valid position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PPBTREEBUFFER          pointer to selected record
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EOF_REACHED end or start of dictionary reached
//------------------------------------------------------------------------------
// Function flow:     if current index < 0 or current index > occupied then
//                      if current index < 0 then
//                        if previous record available then
//                          read in prev. record and set Rc
//                          if okay then
//                            set current index and current record to new data
//                          endif
//                        else
//                          set current record and index to 0
//                          set Rc to BTREE_EOF_REACHED
//                        endif
//                      elsif current index >= occupied then
//                        if next record available then
//                          set record to next record; current index to 0
//                          read in record and set Rc
//                        else
//                          set current record and index to 0
//                          set Rc to BTREE_EOF_REACHED
//                        endif
//                      endif
//                    endif
//                    return Rc
//
//------------------------------------------------------------------------------
SHORT QDAMValidateIndex_V3
(
   PBTREE         pBTIda,
   PBTREEBUFFER_V3 * ppRecord
)
{
   SHORT    sRc = 0;                   // set return code
   USHORT   usRecord;

   if ( pBTIda->sCurrentIndex < 0 ||
           pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
   {
      if ( pBTIda->sCurrentIndex < 0 )
      {
         if ( PREV( *ppRecord ))
         {
            usRecord = PREV(*ppRecord);
            sRc = pBTIda->QDAMReadRecord_V3( usRecord, ppRecord, FALSE  );
            if (!sRc )
            {
               pBTIda->usCurrentRecord = usRecord;
               pBTIda->sCurrentIndex = (SHORT) (OCCUPIED(*ppRecord) - 1);
            } /* endif */
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      }
      else if ( pBTIda->sCurrentIndex >= (SHORT) OCCUPIED(*ppRecord))
      {
         if ( NEXT(*ppRecord) )
         {
            pBTIda->usCurrentRecord = NEXT(*ppRecord);
            sRc = pBTIda->QDAMReadRecord_V3( pBTIda->usCurrentRecord, ppRecord, FALSE  );
            pBTIda->sCurrentIndex = 0;
         }
         else
         {
            pBTIda->usCurrentRecord = 0;
            pBTIda->sCurrentIndex =  0;
            sRc = BTREE_EOF_REACHED;
         } /* endif */
      } /* endif */
   } /* endif */
   if ( sRc )
   {
     ERREVENT2( QDAMVALIDATEINDEX_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */
   return sRc;
}

RECPARAM  QDAMGetrecKey_V3
(
   PBTREEBUFFER_V3  pRecord,
   SHORT         sMid                            // key number
);

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMNext     Get the next entry back
//------------------------------------------------------------------------------
// Function call:     QDAMNext( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Locate the next entry (by ascending key) and
//                    pass back the associated information
//
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PRECPARAM              position of key
//                    PRECPARAM              position of key data
//                    PRECPARAM              position of data part
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     init passed areas
//                    if BTree exists
//                      if current record = 0
//                        position at first entry
//                      else
//                        read next record
//                      endif
//                      if ok
//                        check and validate the record read
//                        if ok
//                          get associated info
//                        endif
//                      endif
//                    else
//                      set rc = BTREE_INVALID
//                    endif
//
//
//------------------------------------------------------------------------------
SHORT QDAMNext_V3
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V3 pRecord;
   PBTREE    pBT = pBTIda;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      // Step to next index in the chain and read the record                     */
      // if usCurrentRecord = 0 we are at EOF
      if ( !pBTIda->usCurrentRecord )
      {
         sRc = QDAMFirstEntry_V3( pBTIda, &pRecord );
      }
      else
      {
         pBTIda->sCurrentIndex ++;
         sRc = pBTIda->QDAMReadRecord_V3( pBTIda->usCurrentRecord, &pRecord, FALSE  );
      } /* endif */


      if ( !sRc )
      {
        sRc = QDAMValidateIndex_V3( pBTIda, &pRecord );
        if ( !sRc )
        {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V3( pRecord, pBTIda->sCurrentIndex  );
            *precData = QDAMGetrecData_V3( pRecord, pBTIda->sCurrentIndex,
                                        pBT->usVersion );
        } /* endif */
      } /* endif */
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMNEXT_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

  return sRc;
}




//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMGetszKeyParam     Get Key String from param
//------------------------------------------------------------------------------
// Function call:     QDAMGetszKeyParam( PBTREE, RECPARAM, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Get the key string from passed offset.
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    RECPARAM               record position
//                    PCHAR                  key to be retrieved
//                    PUSHORT                length of data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//------------------------------------------------------------------------------
// Function flow:     read record
//                    if ok
//                      copy key string from passed offset to output area
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMGetszKeyParam_V3
(
   PBTREE  pBTIda,               // pointer to btree structure
   RECPARAM  recKey,             // active record
   PCHAR_W   pKeyData,             // pointer to data
   PULONG  pulLen                // length of data
)
{
   PCHAR   pData = NULL;
   PBTREEBUFFER_V3  pRecord;              // active record
   SHORT   sRc = 0;                    // return code
   PBTREE    pBT = pBTIda;
   USHORT  usLen;

   sRc = pBTIda->QDAMReadRecord_V3( recKey.usNum, &pRecord, FALSE  );

   /*******************************************************************/
   /* check length                                                    */
   /*******************************************************************/
   if ( !sRc )
   {
     pData = (PCHAR)(pRecord->contents.uchData + recKey.usOffset);
     usLen = *(PUSHORT) pData;

     // as data is now in Unicode the length of the key may be up to
     // HEADTERM_SIZE *2 !!!
     if ( (usLen < (HEADTERM_SIZE + HEADTERM_SIZE)) && ( *pulLen > usLen ) )
     {
       *pulLen = usLen;       
        pData += sizeof(USHORT ) + sizeof(RECPARAM);       // get pointer to data
       
       memcpy( pKeyData, pData, *pulLen );
     }
     else
     {
       sRc = BTREE_CORRUPTED;
       ERREVENT2( QDAMGETSZKEYPARAM_LOC, STATE_EVENT, 1, DB_GROUP, "" );
     }       
    } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMGETSZKEYPARAM_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

   return ( sRc );
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFirst    Get the first entry back
//------------------------------------------------------------------------------
// Function call:     QDAMFirst( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
//
//------------------------------------------------------------------------------
// Description:       Locate the first entry and pass back the
//                    associated information
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PRECPARAM              position of key
//                    PRECPARAM              position of key data
//                    PRECPARAM              position of data part
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     init passed records
//                    if BTree exists
//                      set current pointer to first record of BTree
//                      if record == 0
//                        set RC = BTREE_EMPTY
//                      else
//                        read record
//                        check that record actually has values in it
//                      endif
//                    else
//                      set rc = BTREE_INVALID
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMFirst_V3
(
   PBTREE     pBTIda,
   PRECPARAM  precBTree,
   PRECPARAM  precKey,
   PRECPARAM  precData
)
{
   SHORT  sRc = 0;
   PBTREEBUFFER_V3 pRecord;
   PBTREE    pBT = pBTIda;

   memset( precBTree, 0, sizeof(RECPARAM));
   memset( precKey, 0, sizeof(RECPARAM));
   memset( precData, 0, sizeof(RECPARAM));

   if ( pBT )
   {
      pBTIda->usCurrentRecord = pBT->usFirstLeaf;
      pBTIda->sCurrentIndex = 0;
      if ( pBTIda->usCurrentRecord == 0)
      {
        sRc = BTREE_EMPTY;
      }
      else
      {
        sRc = pBTIda->QDAMReadRecord_V3(pBTIda->usCurrentRecord, &pRecord, FALSE  );
        if ( !sRc )
        {
          /***********************************************************************/
          /* Check that the record actually has some values in it                */
          /***********************************************************************/
          if ( ! OCCUPIED( pRecord))
          {
            sRc = BTREE_EMPTY;
          }
          else
          {
            precBTree->usOffset = pBTIda->sCurrentIndex;
            precBTree->usNum = pBTIda->usCurrentRecord;
            *precKey = QDAMGetrecKey_V3( pRecord, 0 );
            *precData = QDAMGetrecData_V3( pRecord, 0, pBT->usVersion );
          }
        }
      }
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMFIRST_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */

  return sRc;
}



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictFirstLocal   Get the first entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictFirstLocal(PBTREE,PCHAR,PUSHORT,PCHAR,PUSHORT);
//------------------------------------------------------------------------------
// Description:       Locate the first entry and pass back the
//                    associated information into the user provided
//                    buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     if tree is corrupted
//                      set RC = BTREE_CORRUPTED
//                    else
//                      locate first entry
//                    endif
//                    if ok
//                      set ptr to key data, length of key data
//                    endif
//                    if ok
//                      set ptr to user data, length of user data
//                    endif
//------------------------------------------------------------------------------

SHORT QDAMDictFirstLocal
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
   SHORT     sRc = 0;                  // return code
   RECPARAM  recBTree;                 // record parameter for index data
   RECPARAM  recKey;                   //      ...   for key value
   RECPARAM  recData;                  //      ...   for user value
   PBTREE    pBT = pBTIda;
   ULONG ulKeyLen;

   memset(&recData, 0, sizeof(recData));
   memset(&recKey, 0, sizeof(recKey));
   if ( pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */

   if ( !sRc )
   {
       SHORT sRetries = MAX_RETRY_COUNT;

       do
       {
          ulKeyLen = *pulKeyLen;

          if ( !sRc )
          {
             sRc = QDAMFirst_V3( pBTIda, &recBTree,&recKey, &recData);
          } /* endif */

          if ( !sRc )
          {
             sRc = QDAMGetszKeyParam_V3( pBTIda, recKey, pKeyData, &ulKeyLen );
          }
          if ( !sRc && ulKeyLen )
            {
              ULONG ul = 0l;
              memcpy( pBTIda->chHeadTerm, &ul, sizeof(ULONG) );              
          } /* endif */

           if ( !sRc )
           {
              sRc =  QDAMGetszData_V3( pBTIda, recData, pUserData, pulUserLen, DATA_NODE );             
           } /* endif */

         if ( (sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED) )
         {
           UtlWait( MAX_WAIT_TIME );
           sRetries--;
         } /* endif */
       }
       while( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (sRetries > 0));
       *pulKeyLen = ulKeyLen;
   } /* endif */

   if ( sRc )
   {
     ERREVENT2( QDAMDICTFIRSTLOCAL_LOC, INTFUNCFAILED_EVENT, sRc, DB_GROUP, "" );
   } /* endif */
   return ( sRc );
}



// function EQFNTMOrganizeIndex
//
// re-organizes the index part of a memory by seuentially writing the
// index records into a new file
//
USHORT EQFNTMOrganizeIndex
(
   BTREE          *pbTree,            // ptr to BTREE being organized
   USHORT         usOpenFlags,         // open flags to be used for index file
   ULONG          ulStartKey           // first key to start automatic insert...
)
{
  SHORT          sRc = 0;              // function return code
  //PBTREE         pbTree = (PBTREE)(*ppBTIda);
  PCHAR_W        pchKeyBuffer = NULL;  // buffer for record keys
  ULONG          ulKeyBufSize = 0;     // current size of key buffer (number of characters)
  PBYTE          pbData = NULL;        // buffer for record data
  ULONG          ulDataBufSize = 0;    // current size of record data buffer (number of bytes)
  BOOL           fNewIndexCreated = FALSE; // new-index-has-been-created flag
  CHAR           szNewIndex[MAX_LONGPATH]; // buffer for new index name
  PBTREE         pBtreeOut = NULL;     // structure for output BTREE
  USHORT         usSigLen = 0;         // length of signature record
  ULONG          ulKey;

  // allocate buffer areas
  ulKeyBufSize = 256;
  if ( !UtlAlloc( (PVOID *)&pchKeyBuffer, 0, ulKeyBufSize*sizeof(CHAR_W) , ERROR_STORAGE ) ) 
  {
    LOG_AND_SET_RC(sRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  } /* endif */

  if ( !sRc )
  {
    ulDataBufSize = MAX_INDEX_LEN * sizeof(LONG) * 4;    
    if ( !UtlAlloc( (PVOID *)&pbData, 0, ulDataBufSize, ERROR_STORAGE ) ) 
    {
      LOG_AND_SET_RC(sRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    if ( !UtlAlloc( (PVOID *)&pBtreeOut, 0, sizeof(BTREE), ERROR_STORAGE ) ) 
    {
      LOG_AND_SET_RC(sRc, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
    } /* endif */
  } /* endif */

  // get user data (signature record)
  if ( !sRc )
  {
    //T5LOG( T5WARNING) << "TEMPORARY HARDCODED EQFNTMOrganizeIndex:: usSigLen = (USHORT)ulDataBufSize => usSigLen = (SHORT)ulDataBufSize");
    usSigLen = (USHORT)ulDataBufSize;
    sRc = pbTree->QDAMDictSignLocal( (PCHAR)pbData, &usSigLen );
  } /* endif */

  // check if index is empty
  if ( !sRc )
  {
    ULONG ulDataLen = ulDataBufSize; 
    ULONG ulKeyLen  = sizeof(ULONG) + 1; // ulKeyBufSize;

    sRc = QDAMDictFirstLocal( pbTree, (PCHAR_W)&ulKey, &ulKeyLen, pbData, &ulDataLen );
  } /* endif */

  // cleanup
  if ( pchKeyBuffer ) UtlAlloc( (PVOID *)&pchKeyBuffer, 0, 0, NOMSG );
  if ( pbData ) UtlAlloc( (PVOID *)&pbData, 0, 0, NOMSG );
  if ( pBtreeOut ) UtlAlloc( (PVOID *)pBtreeOut, 0, 0, NOMSG );

  // re-map some return codes..
  switch ( sRc )
  {
    case BTREE_EMPTY:
      sRc = 0;
      break;
    default:
      break;
  } /*endswitch */

  return( (USHORT)sRc );
} /* end of function EQFNTMOrganizeIndex */

