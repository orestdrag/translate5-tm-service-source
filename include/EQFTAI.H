////////////////////////////////////////////////////////////////////////////////
// EQFTAI.H     Internal header file for analysis functions                    /
////////////////////////////////////////////////////////////////////////////////
// Copyright Notice:                                                           /
//                                                                             /
//       Copyright (C) 1990-2016, International Business Machines              |
//       Corporation and others. All rights reserved                           |
////////////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/* This include file requires EQFDDE.H!!!                             */
/**********************************************************************/
#ifndef EQFDDE_INCLUDED
  #include "EQFDDE.H"
#endif


#define PATHCATFILE "%s\\%s"           // concatenate path and file
#define PATHCATFILECATEXTENSION "%s\\%s%s" // concatenate path, file and extension
#define FILEEND     "\r\n"          // fileend CR LF


/**********************************************************************/
/* Strings for functions in segmentation user exit                    */
/**********************************************************************/
#define  EQFPRESEG_EXIT     "EQFPRESEG"
#define  EQFPRESEG2_EXIT    "EQFPRESEG2"
#define  EQFPRESEGEX_EXIT   "EQFPRESEGEX"
#define  EQFPOSTSEG_EXIT    "EQFPOSTSEG"
#define  EQFPOSTSEG2_EXIT   "EQFPOSTSEG2"
#define  EQFPOSTSEGEX_EXIT  "EQFPOSTSEGEX"
#define  EQFPOSTSEGW_EXIT   "EQFPOSTSEGW"
#define  EQFPOSTSEGWEX_EXIT "EQFPOSTSEGWEX"
#define  EQFPOSTTM_EXIT     "EQFPOSTTM"
#define  EQFPOSTTM2_EXIT    "EQFPOSTTM2"
#define  EQFPOSTTMW_EXIT    "EQFPOSTTMW"
#define  EQFPREUNSEG_EXIT   "EQFPREUNSEG"
#define  EQFPOSTUNSEG_EXIT  "EQFPOSTUNSEG"
#define  EQFPREUNSEG2_EXIT  "EQFPREUNSEG2"
#define  EQFPREUNSEGW_EXIT  "EQFPREUNSEGW"
#define  EQFPOSTUNSEG2_EXIT "EQFPOSTUNSEG2"
#define  EQFPOSTUNSEGW_EXIT "EQFPOSTUNSEGW"
#define  EQFPROTTABLE_EXIT  "EQFPROTTABLE"
#define  EQFPROTTABLEW_EXIT  "EQFPROTTABLEW"
#define  EQFCHECKSEG_EXIT   "EQFCHECKSEG"
#define  EQFCHECKSEGW_EXIT   "EQFCHECKSEGW"
#define  EQFCHECKSEGEXW_EXIT   "EQFCHECKSEGEXW"
#define  EQFSHOWTRANS_EXIT  "EQFSHOW"
#define  EQFTOCGOTO_EXIT    "EQFTOCGOTO"
#define  EQFGETSEGCONTEXT_EXIT  "EQFGETSEGCONTEXT"
#define  EQFUPDATECONTEXT_EXIT  "EQFUPDATECONTEXT"
#define  EQFCOMPARECONTEXT_EXIT "EQFCOMPARECONTEXT"
#define  EQFFORMATCONTEXT_EXIT  "EQFFORMATCONTEXT"
#define  EQFQUERYEXITINFO_EXIT  "EQFQUERYEXITINFO"
#define  EQFCHECKSEGTYPE_EXIT   "EQFCHECKSEGTYPE"


/*----------- limits, capacity, boundaries ------------------------------------*/
#define MAX_NUM_TOKENS      400            // maximum number of tokens
#define MAX_ALLOC_TOKENS    400
#define MAX_NUM_SEGTOKENS   (5 * MAX_NUM_TOKENS)
#define MAX_ALLOC_SEGTOKENS (MAX_NUM_SEGTOKENS )
#define MAXTAGLEN           512            // maximum length of tag

#define BOOKVARSTART        '&'            // start character of bookmaster vars
#define BLANK               ' '
#define ON                    1
#define OFF                   0

#define BUFFERSIZE        16192            // Length of the text buffer
#define GML_TAG_LEN          18
#define TOLST_REPLY_SIZE  64000L           // length of tolstoy reply size
#define SLIDER_INCREMENT    101                  // number of increments on slider bar
#define OUTPUT_MEM_NUM        6                // size output memory db structur

#define MAX_SEG_CLASSES       9            // number of seg. classes

//
// definitions for most used segment table of redundancy counting
// GQ: the filtering is now done in the report code, the values have been
//     changed to the minimal value (frequency) and the maximum value (segments)
// ==============================================================

// number of segments in the most used segment table
// old value: #define MAX_MOSTUSEDSEGS      100   
#define MAX_MOSTUSEDSEGS      0x7FFF

// frequency threshhold for segments to appear in most used segment table
// old value: #define MAX_MOSTUSEDSEGSTHRESHHOLD  3
#define MAX_MOSTUSEDSEGSTHRESHHOLD  2


/** defines and data for the protect specific section processing */

/** max number of protect section tags 
    Note: enlarge this when new tag groups are added to the tables located in EQFTSEGM.CPP  */
#define MAXNUMOFPROTECTTAGGROUPS 8

/** tag IDs for usage with individual section protection flags */
typedef enum _PROTECTTAGID { XMPTAGID, SCREENTAGID, MSGNUMTAGID, CODEBLOCKTAGID, METATAGID, ENDOFTAGLIST } PROTECTTAGID;

typedef struct _XMPTAG
{
  CHAR_W szTag[60];
  int    iLen;
  PROTECTTAGID Id;
} XMPTAG, *PXMPTAG;

typedef struct _XMPMARKUPS
{
  char   szMarkup[20];
  PXMPTAG pStartTags;
  PXMPTAG pEndTags;
} XMPMARKUPS, *PXMPMARKUPS;

/**********************************************************************/
/* structure for tag table extract (tag name + tag ID)                */
/**********************************************************************/
typedef struct _TBTAGENTRY
{
   PSZ_W      pszTag;                    // ptr to tag name
   PSZ_W      pszEndDel;                 // ptr to tag end delimiter
   SHORT    sID;                       // associated tag ID
   USHORT   usLength;                  // length of length-terminated tags
   USHORT   usColPos;                  // column position or 0
   unsigned short fAttr       : 1;     // tag-has-attributes flag
   unsigned short fTranslInfo : 1;     // tag-has-translatable-info flag
   unsigned short fReserved   : 14;    // filler, fills bit flags up to 16bit
} TBTAGENTRY, *PTBTAGENTRY;

PNODEAREA  TACreateTagTree
(
  PLOADEDTABLE pTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);
PNODEAREA  TACreateAttrTree
(
  PLOADEDTABLE pTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);
PNODEAREA TACreateNodeTree
(
  PTBTAGENTRY pTags,
  ULONG       ulNoOfTags,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);

/**********************************************************************/
/* Structure of stack for processing of multiple substitution tags    */
/* and attributes                                                     */
/**********************************************************************/
typedef struct _MSSTACK
{
  PTREENODE   pNode;                   // ptr to active node
} MSSTACK, *PMSSTACK;

/**********************************************************************/
/* Structure of stack to store retry positions if a selectet path is  */
/* not unique                                                         */
/**********************************************************************/
typedef struct _RETRYSTACK
{
  PTREENODE   pNode;                   // ptr to active node
  PSZ_W       pszData;                 // ptr to current position in imput data
  PSZ_W       pszTag;                  // ptr to current tag start position
} RETRYSTACK, *PRETRYSTACK;

/**********************************************************************/
/* Structure of tokenize status area                                  */
/**********************************************************************/
typedef struct _TOKSTATUS
{
  /*====================== general flags and variables ===============*/
  BOOL        fMsg;                    // display-error-messages flag
  USHORT      usColPos;                // current column position
  USHORT      usStartColPos;           // start column position of tag/attr
  /*===================== wildcard characters ========================*/
  CHAR_W        chMultSubst;             // multiple substitution character
  CHAR_W        chSingleSubst;           // single substitution character
  /*================= multiple substitution stack ====================*/
  USHORT      usMSStackSize;           // size of stack in # of elements
  PMSSTACK    pMSStack;                // ptr to stack
  /*==================== retry other path stack ======================*/
  USHORT      usRetryStackSize;        // size of stack in # of elements
  PRETRYSTACK pRetryStack;             // ptr to stack
  /*======================== tag table ===============================*/
  PLOADEDTABLE pTable;                 // ptr to loaded tag table
} TOKSTATUS, *PTOKSTATUS;

BOOL TAMatchTag
(
  PTOKSTATUS  pTokStatus,              // ptr to tokenize status area
  PSZ_W       *ppszData,               // pointer to current position in data
  PTREENODE   pNode,                   // root node of node tree for matching
  PTOKENENTRY pToken,                  // ptr to token buffer for results
  USHORT      usFlags                  // flags; see following defines
);

#define MATCH_SKIP_WHITESPACE 0x0001   // skip whitespace before tags
#define MATCH_SKIP_NOTAG_DATA 0x0002   // skip all data before tags
#define MATCH_QUOTED_STRINGS  0x0004   // do handling for quoted strings
#define MATCH_SKIP_DATA       0x0008   // skip unknown data
#define MATCH_DATA_COMPLETE   0x0010   // passed data are complete


#define TIMER     1
#define TIMEOUT   500
#define MAX_TITLE 100                  //length of title of analysis instance
