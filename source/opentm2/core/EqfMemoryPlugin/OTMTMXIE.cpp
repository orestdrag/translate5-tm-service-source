//+----------------------------------------------------------------------------+
//| OTMTMXIE.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2017, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Memory Export/Import in TMX format                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

// use non-ISO version of swprintf which was used up to VS2005, once older versions are not used anymore we
// should use the new version of the function asap
#define _CRT_NON_CONFORMING_SWPRINTFS

#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#include "tm.h"

#include "win_types.h"
#include "LogWrapper.h"
#include "Property.h"
#include "FilesystemHelper.h"
#include "OSWrapper.h"
#include "../../../cmake/git_version.h"

//#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <string>

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
#include <xercesc/framework/MemBufInputSource.hpp>

//#pragma pack( pop )

XERCES_CPP_NAMESPACE_USE



//#pragma pack( push, TM2StructPacking, 1 )

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morphologic functions
#include "EQF.H"                  // General .H for EQF
#include "EQFMEMIE.H"
//#include "EQFSERNO.H"
#include "TMXNames.h"

//#pragma pack( pop, TM2StructPacking )


#include "CXMLWRITER.H"
#include "LanguageFactory.H"
#include "EncodingHelper.h"

// mask for character entitites below 0x20
#define ENTITY_MASK_STARTCHAR_W L'_'
#define ENTITY_MASK_START_LEN 8
#define ENTITY_MASK_START "___XENT_"
#define ENTITY_MASK_START_W L"___XENT_"
#define ENTITY_MASK_END_LEN 8
#define ENTITY_MASK_END "_XENT___"
#define ENTITY_MASK_END_W L"_XENT___"

// size of token buffer
#define TMXTOKBUFSIZE 32000

// size of file read buffer in preprocess step
#define TMX_BUFFER_SIZE 8096

// max length of a segment text

// name to use for undefined languages
#define OTHERLANGUAGES "OtherLanguages"

// name of TM specific prop elements
#define DESCRIPTION_PROP_OLD      "tmgr-description"
#define DESCRIPTION_PROP_W_OLD    L"tmgr-description"
#define DESCRIPTION_PROP          "tmgr:description"
#define DESCRIPTION_PROP_W        L"tmgr:description"

#define MARKUP_PROP_OLD      "tmgr-markup"
#define MARKUP_PROP_W_OLD    L"tmgr-markup"
#define MARKUP_PROP          "tmgr:markup"
#define MARKUP_PROP_W        L"tmgr:markup"

#define LANGUAGE_PROP_OLD    "tmgr-language"
#define LANGUAGE_PROP_W_OLD  L"tmgr-language"
#define LANGUAGE_PROP        "tmgr:language"
#define LANGUAGE_PROP_W      L"tmgr:language"

#define DOCNAME_PROP_OLD    "tmgr-docname"
#define DOCNAME_PROP_W_OLD  L"tmgr-docname"
#define DOCNAME_PROP        "tmgr:docname"
#define DOCNAME_PROP_W      L"tmgr:docname"

#define MACHFLAG_PROP_OLD    "tmgr-MTflag"
#define MACHFLAG_PROP_W_OLD  L"tmgr-MTflag"
#define MACHFLAG_PROP        "tmgr:MTflag"
#define MACHFLAG_PROP_W      L"tmgr:MTflag"

#define TRANSLFLAG_PROP_OLD    "tmgr-translFlag"
#define TRANSLFLAG_PROP_W_OLD  L"tmgr-translFlag"
#define TRANSLFLAG_PROP        "tmgr:translFlag"
#define TRANSLFLAG_PROP_W      L"tmgr:translFlag"

#define SEGNUM_PROP_OLD      "tmgr-segNum"
#define SEGNUM_PROP_W_OLD    L"tmgr-segNum"
#define SEGNUM_PROP          "tmgr:segNum"
#define SEGNUM_PROP_W        L"tmgr:segNum"

#define NOTE_PROP_OLD      "tmgr-note"
#define NOTE_PROP_W_OLD    L"tmgr-note"
#define NOTE_PROP          "tmgr:note"
#define NOTE_PROP_W        L"tmgr:note"

#define NOTESTYLE_PROP_OLD   "tmgr-notestyle"
#define NOTESTYLE_PROP_W_OLD L"tmgr-notestyle"
#define NOTESTYLE_PROP       "tmgr:notestyle"
#define NOTESTYLE_PROP_W     L"tmgr:notestyle"


#define TMMATCHTYPE_PROP            "TM:MatchType"
#define TMMATCHTYPE_PROP_W         L"TM:MatchType"
#define TMMATCHTYPE_PROP_W_OLD     L"TM-MatchType"
#define TMMATCHTYPE_PROP_OLD     "TM-MatchType"

#define MTSERVICE_PROP              "MT:ServiceID"
#define MTSERVICE_PROP_W           L"MT:ServiceID"
#define MTSERVICE_PROP_W_OLD       L"MT-ServiceID"
#define MTSERVICE_PROP_OLD          "MT-ServiceID"

#define MTMETRICNAME_PROP           "MT:MetricName"
#define MTMETRICNAME_PROP_W        L"MT:MetricName"
#define MTMETRICNAME_PROP_W_OLD    L"MT-MetricName"
#define MTMETRICNAME_PROP_OLD       "MT-MetricName"

#define MTMETRICVALUE_PROP          "MT:MetricValue"
#define MTMETRICVALUE_PROP_W       L"MT:MetricValue"
#define MTMETRICVALUE_PROP_W_OLD   L"MT-MetricValue"
#define MTMETRICVALUE_PROP_OLD      "MT-MetricValue"

#define PEEDITDISTANCECHARS_PROP    "PE:EditDistanceChars"
#define PEEDITDISTANCECHARS_PROP_W L"PE:EditDistanceChars"
#define PEEDITDISTANCECHARS_PROP_W_OLD L"PE-EditDistanceChars"
#define PEEDITDISTANCECHARS_PROP_OLD "PE-EditDistanceChars"

#define PEEDITDISTANCEWORDS_PROP    "PE:EditDistanceWords"
#define PEEDITDISTANCEWORDS_PROP_W L"PE:EditDistanceWords"
#define PEEDITDISTANCEWORDS_PROP_W_OLD L"PE-EditDistanceWords"
#define PEEDITDISTANCEWORDS_PROP_OLD "PE-EditDistanceWords"

#define MTFIELDS_PROP               "MT:Fields"
#define MTFIELDS_PROP_W            L"MT:Fields"
#define MTFIELDS_PROP_W_OLD        L"MT-Fields"
#define MTFIELDS_PROP_OLD        "MT-Fields"

#define WORDS_PROP                 "tmgr:words"
#define WORDS_PROP_W               L"tmgr:words"

#define MATCHSEGID_PROP           "tmgr:MatchSegID"
#define MATCHSEGID_PROP_W        L"tmgr:MatchSegID"

#define ERROR_BAD_FORMAT 136464

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
typedef struct _TMXNAMETOID
{
  //CHAR_W   szName[30];                 // name of element
  char       szName[30];
  ELEMENTID ID;                        // ID of element 
} TMXNAMETOID, *PTMXNAMETOID;

TMXNAMETOID TmxNameToID[] = {
     {"tmx",    TMX_ELEMENT},
     {"prop",   PROP_ELEMENT},
     {"header", HEADER_ELEMENT},
     {"tu",     TU_ELEMENT},
     {"tuv",    TUV_ELEMENT},
     {"body",   BODY_ELEMENT},
     {"seg",    SEG_ELEMENT},    

     // paired tags     
     {"bpt",      BPT_ELEMENT},
     {"ept",      EPT_ELEMENT},
     { "g",       G_ELEMENT },
     { "hi",      HI_ELEMENT },
     { "sub",     SUB_ELEMENT },
     { "bx",      BX_ELEMENT },
     { "ex",      EX_ELEMENT },

     // standalone tags
     {"ph",       PH_ELEMENT},
     { "x",       X_ELEMENT },
     { "it",      IT_ELEMENT },
     { "ut",      UT_ELEMENT },
     { "t5:n",      T5_N_ELEMENT },

     {"TMXSentence", TMX_SENTENCE_ELEMENT},
     {"invchar",     INVCHAR_ELEMENT},
     {"",            UNKNOWN_ELEMENT}
};


std::map<ELEMENTID, const std::string> TmxIDToName = {
     { TMX_ELEMENT,   "tmx"    },
     { PROP_ELEMENT,  "prop"   },
     { HEADER_ELEMENT,"header" },
     { TU_ELEMENT,    "tu"     },
     { TUV_ELEMENT,   "tuv"    },
     { BODY_ELEMENT,  "body"   },
     { SEG_ELEMENT,   "seg"    },    

     // paired tags     
     { BPT_ELEMENT,   "bpt"    },
     { EPT_ELEMENT,   "ept"    },
     { G_ELEMENT,     "g"      },
     { HI_ELEMENT,    "hi"     },
     { SUB_ELEMENT,   "sub"    },
     { BX_ELEMENT,    "bx"     },
     { EX_ELEMENT,    "ex"     },

     // standalone tags
     { PH_ELEMENT,   "ph"      },
     { X_ELEMENT,    "x"       },
     { IT_ELEMENT,   "it"      },
     { UT_ELEMENT,   "ut"      },
     { T5_N_ELEMENT, "t5:n"    },

     { TMX_SENTENCE_ELEMENT, "TMXSentence"},
     { INVCHAR_ELEMENT,      "invchar"    },
     { UNKNOWN_ELEMENT,       ""          }
};

enum ACTIVE_SEGMENT{
  SOURCE_SEGMENT  = 0,
  TARGET_SEGMENT  = 1, 
  REQUEST_SEGMENT = 2 //from fuzzy search request
};


typedef ACTIVE_SEGMENT TagLocation;

std::map<ACTIVE_SEGMENT, const std::string> TagLocIDToStr = {
     { SOURCE_SEGMENT,   "SOURCE_SEGMENT"  },
     { TARGET_SEGMENT,   "TARGET_SEGMENT"  },
     { REQUEST_SEGMENT,  "REQUEST_SEGMENT" }
};

char16_t* str16cpy(char16_t* destination, const char16_t* source)
{
    char16_t* temp = destination;
    while((*temp++ = *source++) != 0)
    ;
    return destination;
}	

size_t str16len(const char16_t* source)
{
    char16_t* temp = (char16_t*)source;
    while((*temp++ = *source++) != 0) ;
    return temp-source;
}	 
  
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
    
    //t5n
    std::string t5n_key;
    std::string t5n_value;
  };

// PROP types
typedef enum { TMLANGUAGE_PROP, TMMARKUP_PROP, TMDOCNAME_PROP, MACHINEFLAG_PROP, SEG_PROP, TMDESCRIPTION_PROP, TMNOTE_PROP, TMNOTESTYLE_PROP,
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

// helper function for the access to the name mapping tables
BOOL FindName( PNAMETABLE pTable, const char *pszName, char *pszValue, int iBufSize );

void EscapeXMLChars( PSZ_W pszText, PSZ_W pszBuffer );
BOOL CheckEntityW( PSZ_W pszText, int *piValue, int *piLen );
BOOL CheckEntity( PSZ pszText, int *piValue, int *piLen );
void CopyTextAndProcessMaskedEntitites( PSZ_W pszTarget, PSZ_W pszSource );


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
  TagInfo GenerateReplacingTag(ELEMENTID tagType, AttributeList* attributes, bool saveTagToTagList = true);
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

void TagReplacer::reset(){
  fFuzzyRequest = false;
  iHighestPTId = 500;
  iHighestPHId = 100;
  iHighestPTI = 0;
  sourceTagList.clear();
  targetTagList.clear();
  requestTagList.clear();
}

std::wstring TagReplacer::PrintReplaceTagWithKey(TagInfo& tag){  
  std::wstring rAttrValue = EncodingHelper::convertToUTF16(tag.t5n_key);
  std::replace(rAttrValue.begin(), rAttrValue.end(), '=', '_');
  return rAttrValue;
}

std::wstring TagReplacer::PrintTag(TagInfo& tag){
  std::string res = "<" ;
  bool fClosingTag = false; // for </g> and similar tags
  bool fClosedTag = true; // for <ph/> and similar tags
  
  ELEMENTID tagType = tag.generated_tagType;
  if(fReplaceNumberProtectionTagsWithHashes){
    if(tagType == T5_N_ELEMENT){
      return PrintReplaceTagWithKey(tag);
    }
    else if(fSkipTags){// skip other tags
      return L"";
      }
  }

  if(tagType == T5_N_ELEMENT){
    std::string outputStr = "<" + TmxIDToName[tag.original_tagType] + " id=\"" + std::to_string(tag.original_x) + "\" r=\"" + tag.t5n_key + "\" n=\"" + tag.t5n_value + "\"/>";
    return EncodingHelper::convertToUTF16(outputStr);
  }


  if(fFuzzyRequest){

    int x = 0,// tag.original_x, 
        i = 0;// tag.original_i
        
    if(tag.tagLocation == REQUEST_SEGMENT){
      x = tag.generated_x;
      i = tag.generated_i;
    }else{   
      //find corresponding tag in requests tag
      //try to find matching tag
      std::vector<TagInfo>::iterator matchingRequestTag = std::find_if(requestTagList.begin(), requestTagList.end(),
                                    [&tag](TagInfo& t) { return  t.generated_tagType == tag.generated_tagType
                                                            &&   t.generated_x       == tag.generated_x
                                                            //&&   t.generated_i       == tag.generated_i
                                                            ;
                                    });


      if ( matchingRequestTag != requestTagList.end()){
        i = matchingRequestTag->original_i;
        x = matchingRequestTag->original_x;
        tagType = matchingRequestTag->original_tagType;
        fClosingTag = tag.generated_tagType == EPT_ELEMENT 
                  && tagType != EPT_ELEMENT 
                  && tagType != EX_ELEMENT;        
      }else{
        //tag not found -> maybe should try to find matching tag in the same or paralel segment? 
        //generate new id        
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
          T5LOG(T5ERROR) << ":: not found matching tag in request tag list";    
        }
        x = ++ iHighestRequestsOriginalId;

        //if it's BPT_ELEMENT or EPT_ELEMENT -> generate new I 
        if(tag.generated_tagType != PH_ELEMENT){
          i = ++ iHighestRequestsOriginalI;
        }       
      }
    }
    
    if( tagType < BEGIN_INLINE_TAGS || tagType > END_INLINE_TAGS )
      return L"";
    if(fClosingTag){
      res += '/';      
    }

    res += TmxIDToName[tagType];

    if(x > 0){
      res += " id=\""+toStr(x)+"\"";
    }
    if(i > 0){
      res += " rid=\""+toStr(i)+"\"";
    }
  }else{ // import tag replacement
    res += TmxIDToName[tagType];
    if(tag.generated_x > 0){
      res += " x=\""+toStr(tag.generated_x)+"\"";
    }
    if(tag.generated_i > 0){
      res += " i=\""+toStr(tag.generated_i)+"\"";
    }
  }

  //tag that has slash at the end looks like this: <tag />
  fClosedTag =  tagType == BPT_ELEMENT || 
                tagType == EPT_ELEMENT ||
                tagType ==  PH_ELEMENT ||
                tagType ==  BX_ELEMENT ||
                tagType ==  EX_ELEMENT ||
                tagType ==   X_ELEMENT ||
                tagType ==T5_N_ELEMENT;

  if(!fClosingTag && fClosedTag)
    res += '/';
  res += ">";
  T5LOG( T5DEBUG) <<  ":: generated tag = " <<  res;
  return EncodingHelper::convertToUTF16(res.c_str());
}

bool is_number(std::u16string s)
{
    std::u16string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}


std::string TagReplacer::LogTag(TagInfo & tag){
  bool isClosingTag = tag.generated_tagType == EPT_ELEMENT && tag.original_tagType != EPT_ELEMENT && tag.original_tagType != BX_ELEMENT;
  std::string logMsg = "LogTag::\n original tag = <";
  if(isClosingTag)
    logMsg +="/";
  logMsg += TmxIDToName[tag.original_tagType] + " x = \"" + toStr(tag.original_x) + "\" i = \"" + toStr(tag.original_i) +"\"";
  if(!isClosingTag)
    logMsg +="/";
  logMsg += ">\n generated tag =  <" + TmxIDToName[tag.generated_tagType] + 
          " x=\"" + toStr(tag.generated_x) + "\" i = \"" + toStr(tag.generated_i) + "\"/>" + 
          "\n    tagHasClosingTag = \"" + toStr(tag.fPairedTagClosed) + "\" tagWasAlreadyUsedInTarget = \"" + toStr(tag.fTagAlreadyUsedInTarget) +
          "\" tagLocation = " + TagLocIDToStr[tag.tagLocation];
  return logMsg;
  //T5LOG( T5DEVELOP)<< logMsg;
  //LOG_DEVELOP_MSG << logMsg;
  
  //T5LOG( T5INFO) << "LogTag::\n original tag = <",TmxIDToName[tag.original_tagType].c_str()," x = \"", toStr(tag.original_x).c_str(), "\" i = \"", toStr(tag.original_i).c_str(),
  //        "/>\n    tagHasClosingTag = \"", toStr(tag.fPairedTagClosed).c_str(), "\" tagWasAlreadyUsedInTarget = \"", toStr(tag.fTagAlreadyUsedInTarget).c_str());
  //T5LOG( T5INFO) << " tagLocation = ", TagLocIDToStr[tag.tagLocation].c_str(),"\n generated tag =  <", TmxIDToName[tag.generated_tagType].c_str(), 
  //        " x=\"", toStr(tag.generated_x).c_str(), "\" i = \"",toStr(tag.generated_i).c_str(), "\" />");

}

TagInfo TagReplacer::GenerateReplacingTag(ELEMENTID tagType, AttributeList* attributes, bool saveTagToTagList )
{  
  TagInfo tag;  
  tag.original_tagType = tagType;
  tag.tagLocation = activeSegment;

  if(tagType == T5_N_ELEMENT){
    tag.fPairedTagClosed = true;
    tag.fTagAlreadyUsedInTarget = true;
    tag.generated_tagType = T5_N_ELEMENT;
    // id value could be stored in id attribute or x attribute
    if( char16_t* _r = (char16_t*) attributes->getValue("r") ){
      tag.t5n_key = EncodingHelper::toChar(_r);
    }
    if(char16_t* _n = (char16_t*) attributes->getValue("n") ){
      tag.t5n_value = EncodingHelper::toChar(_n);      
    }    
    if( char16_t* _id = (char16_t*) attributes->getValue("id") ){
      if( is_number(_id) ){
        tag.original_x = std::stoi( EncodingHelper::toChar(_id) );
      }
    }

    LogTag(tag);
    return tag;
  }
  
  if( tagType >= BEGIN_PAIR_TAGS && tagType <= END_PAIR_TAGS){
    if(attributes && tagType != EPT_ELEMENT && tagType != EX_ELEMENT){
      tag.generated_tagType = BPT_ELEMENT;
    }else{
      tag.generated_tagType = EPT_ELEMENT;
    }
  }else if(tagType >= BEGIN_STANDALONE_TAGS && tagType <= END_STANDALONE_TAGS) {
    tag.generated_tagType = PH_ELEMENT;
  }else{
    tag.generated_tagType = UNKNOWN_ELEMENT;
      T5LOG(T5ERROR) << ":: wrong tag used, only bpt, ept and ph tags allowed, tag=" << tagType << 
            ",\n 0<=tag<=9 or tag>=30 -> tmx tags, 20<=tag<=29 -> standalone tags, 10<=tag<=19 -> pair tags, tagName = " << TmxIDToName[tagType];
  }  

  {//save original data if provided
    if( attributes!=nullptr ){
      // id value could be stored in id attribute or x attribute
      if( char16_t* _id = (char16_t*) attributes->getValue("id") ){
        if( is_number(_id) ){
          tag.original_x = std::stoi( EncodingHelper::toChar(_id) );
        }
      }else if(char16_t* _id = (char16_t*) attributes->getValue("x") ){
        if( is_number(_id) ) {
          tag.original_x = std::stoi( EncodingHelper::toChar(_id) );
        }
      }      
       
      if(tag.generated_tagType == BPT_ELEMENT || tag.generated_tagType == EPT_ELEMENT ){ //add i attribute
        // id value could be stored in id attribute or x attribute
        if( char16_t* _i = (char16_t*) attributes->getValue("i") ){
          if( is_number(_i) ) {
            tag.original_i = std::stoi(EncodingHelper::toChar(_i));   
          }    
        }else if( char16_t* _i = (char16_t*) attributes->getValue("rid")){
          if( is_number(_i) ) {
            tag.original_i = std::stoi(EncodingHelper::toChar(_i));
          }
        }  
      }
    }
  }
  
  std::vector<TagInfo>* activeTagList = nullptr;   
  if( activeSegment == REQUEST_SEGMENT ||  activeSegment == SOURCE_SEGMENT ){
    if(activeSegment == REQUEST_SEGMENT){
      activeTagList = &requestTagList;
      if(tag.original_i > iHighestRequestsOriginalI){
        iHighestRequestsOriginalI = tag.original_i;
      }
      if(tag.original_x > iHighestRequestsOriginalId){
        iHighestRequestsOriginalId = tag.original_x;
      }

    }else{
      activeTagList = &sourceTagList;
    }

    tag.fTagAlreadyUsedInTarget = false;
    
    std::vector<TagInfo>::reverse_iterator matchingPairTag = activeTagList->rend();
    ELEMENTID tagTypeToFind = UNKNOWN_ELEMENT;
    if(tag.generated_tagType == BPT_ELEMENT){
      
      // try to find matching ept tag in tag list
      if(tag.original_tagType == BX_ELEMENT){
        tagTypeToFind = EX_ELEMENT;
      }else if(tag.original_tagType == BPT_ELEMENT){
         tagTypeToFind = EPT_ELEMENT;
      }
      if( tagTypeToFind != UNKNOWN_ELEMENT ){
        matchingPairTag = std::find_if(activeTagList->rbegin(), activeTagList->rend(),
                                    [&tag, tagTypeToFind](TagInfo& t) { return t.fPairedTagClosed == false
                                                            &&  t.generated_tagType == EPT_ELEMENT
                                                            &&  t.original_tagType == tagTypeToFind
                                                            &&  t.original_i == tag.original_i;
                                    });
      }

      if( matchingPairTag != activeTagList->rend() ){
        matchingPairTag->fPairedTagClosed = true;            
        tag.fPairedTagClosed = true;
        tag.generated_i =   matchingPairTag->generated_i;
        tag.generated_x = - matchingPairTag->generated_x;
      }else{
        tag.generated_i = ++iHighestPTI;
        tag.generated_x = ++iHighestPTId;
        tag.fPairedTagClosed = false; 
      }
    }else if (tag.generated_tagType == EPT_ELEMENT){
      //try to find matching and use it's attributes
      if(tag.original_tagType == EPT_ELEMENT ){
        tagTypeToFind = BPT_ELEMENT;
      }else if(tag.original_tagType == EX_ELEMENT){
        tagTypeToFind = BX_ELEMENT;

      }else //if()// we have closing tag like </hi> ,that don't have attributes, so we should find
                  // the same original opening tag, that was not closed yet(don't find matching closing tag)
      {
        tagTypeToFind = tag.original_tagType;
      }

      matchingPairTag = std::find_if(activeTagList->rbegin(), activeTagList->rend(),
                                  [&tag, tagTypeToFind](TagInfo& t) { return t.fPairedTagClosed == false
                                                          &&  t.generated_tagType == BPT_ELEMENT
                                                          &&  t.original_tagType == tagTypeToFind
                                                          &&  t.original_i == tag.original_i
                                                          ;
                                  });
      if(matchingPairTag != activeTagList->rend()){
          matchingPairTag->fPairedTagClosed = true;
          tag.fPairedTagClosed = true;
          tag.generated_i =   matchingPairTag->generated_i;     
          tag.generated_x = - matchingPairTag->generated_x;     
      }else{
        tag.generated_i = ++iHighestPTI;
        tag.generated_x = - (++iHighestPTId);
        tag.fPairedTagClosed = false;
      }
    }else{//ph tag
      tag.generated_x = (++iHighestPHId);
      tag.fPairedTagClosed = true;
    }
    
    if(saveTagToTagList){
      activeTagList->push_back(tag);
    }
  }else // if(activeSegment == TARGET_SEGMENT)
  {//generate new attributes or find matching 
    //try to find matching tag in source
    if(sourceTagList.empty() && VLOG_IS_ON(1)){
      T5LOG(T5ERROR) << ":: parsing target tags, but there are no source tags parsed yet! Please check if languages for source tag and TM file is maching in TABLE/languages.xml; tag: " << LogTag(tag);
    }
    std::vector<TagInfo>::iterator matchingSourceTag = std::find_if(sourceTagList.begin(), sourceTagList.end(),
                                  [&tag](TagInfo& i) { return i.fTagAlreadyUsedInTarget == false 
                                                          &&  i.original_x       == tag.original_x
                                                          &&  i.original_tagType == tag.original_tagType;
                                  });
    if ( matchingSourceTag != sourceTagList.end()){
      // mark binded tag as already used in target
      matchingSourceTag->fTagAlreadyUsedInTarget = true;
      tag.fTagAlreadyUsedInTarget = true;
      
      tag.generated_x = matchingSourceTag->generated_x;
      tag.generated_i = matchingSourceTag->generated_i;
      //tag.generated_tagType = matchingSourceTag->generated_tagType;
    }else{
      if(tag.generated_tagType == PH_ELEMENT){
        tag.fPairedTagClosed = true;
      }else{            
        //try to find matched pair tag in target      
        ELEMENTID matchingTagOriginalType;        
        switch (tag.original_tagType)
        {
        case BPT_ELEMENT:
          matchingTagOriginalType = EPT_ELEMENT;
          break;
        case EPT_ELEMENT:
          matchingTagOriginalType = BPT_ELEMENT;
          break;
        case BX_ELEMENT:
          matchingTagOriginalType = EX_ELEMENT;
          break;
        case EX_ELEMENT:
          matchingTagOriginalType = BX_ELEMENT;
          break;
        
        default:
          matchingTagOriginalType = tag.original_tagType;
          break;
        }
        
        ELEMENTID matchingTagGeneratedType = BPT_ELEMENT;
        if(tag.generated_tagType == BPT_ELEMENT){
          matchingTagGeneratedType = EPT_ELEMENT;
        }


        auto matchingTargetTag = std::find_if(targetTagList.rbegin(), targetTagList.rend(),
                                  [&tag, matchingTagOriginalType, matchingTagGeneratedType](TagInfo& t) { 
                                                                                return t.original_tagType == matchingTagOriginalType
                                                                                    && t.generated_tagType == matchingTagGeneratedType
                                                                                    && t.original_i == tag.original_i
                                                                                    && t.fPairedTagClosed == false
                                                          ;
                                  });
      
        
        if(matchingTargetTag != targetTagList.rend()){
          // mark matching tag as which have completed pair
          matchingTargetTag->fPairedTagClosed = true;
          tag.fPairedTagClosed = true;
          tag.generated_i =  matchingTargetTag->generated_i;
          tag.generated_x = -matchingTargetTag->generated_x;
        }else{        
          //generate new id and i
          tag.generated_i = ++iHighestPTI;
          tag.generated_x = ++iHighestPTId;
          if(tag.original_tagType == EPT_ELEMENT){
            tag.generated_x = -tag.generated_x;
          }
        }      
      }
    }    
    if(saveTagToTagList){
      targetTagList.push_back(tag);
    }
  }

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
    T5LOG(T5DEBUG) << "Generated tag logging:" << LogTag(tag);
  }
  return tag;
}

USHORT  MEMINSERTSEGMENT
( 
  LONG lMemHandle, 
  PMEMEXPIMPSEG pSegment 
);


//
// class for our SAX handler
//
class TMXParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  TMXParseHandler(); 
  virtual ~TMXParseHandler();

  // setter functions for import info
  void SetMemInfo( PMEMEXPIMPINFO m_pMemInfo ); 
  void SetImportData(ImportStatusDetails * _pImportDetails) { pImportDetails = _pImportDetails;}
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLOADEDTABLE pTable, PTOKENENTRY pTokBuf, int iTokBufSize ); 
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

  bool fReplaceWithTagsWithoutAttributes;
  size_t getInvalidCharacterErrorCount() const { return _invalidCharacterErrorCount; }

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);
  //void ignorableWhitespace(const XMLCh* const chars, const unsigned int length);
  //void resetDocument();


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);
  void fatalInternalError(const SAXException& exc);
  //void resetErrors();

  
  TagReplacer tagReplacer;
  BOOL fInitialized = false;
  BOOL fCreateNormalizedStr = false;

  USHORT insertSegUsRC{0};

  void setDocumentLocator(const Locator* const locator) override;

private:
  const Locator* m_locator = nullptr;
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
    
    //for enclosing tags skipping. If pFirstBptTag is nullptr->segments don't start from bpt tag-> enclosing tags skipping should be ignored
    bool        fSegmentStartsWithBPTTag;

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
  PLOADEDTABLE pRTFTable;
  PTOKENENTRY pTokBuf;
  int iTokBufSize;
  //ULONG ulCP;

  void fillSegmentInfo( PTMXTUV pSource, PTMXTUV pTarget, PMEMEXPIMPSEG pSegment );

};

BOOL MADGetNextAttr( HADDDATAKEY *ppKey, PSZ_W pszAttrNameBuffer, PSZ_W pszValueBuffer, int iBufSize  );

//+----------------------------------------------------------------------------+
//| Our TMX import export class                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CTMXExportImport
{
  public:
    // constructor/desctructor
	  CTMXExportImport();
	  ~CTMXExportImport();
    // export methods
    USHORT WriteHeader( const char *pszOutFile, PMEMEXPIMPINFO pMemInfo );
    USHORT WriteSegment( PMEMEXPIMPSEG pSegment  );
    USHORT WriteEnd();
    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo, ImportStatusDetails* pImportStatusDetails ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG pMemHandle, ImportStatusDetails*     pImportData  ); 
    USHORT EndImport(); 
    USHORT getLastError( PSZ pszErrorBuffer, int iBufferLength );

  protected:
    USHORT WriteTUV( PSZ pszLanguage, PSZ pszMarkup, PSZ_W pszSegmentData );
    USHORT PreProcessInFile( const char *pszInFile, const char *pszOutFile );


    CXmlWriter m_xw;
    TMXParseHandler *m_handler;          // our SAX handler 
    SAXParser* m_parser;
    XMLPScanToken m_SaxToken; 
    unsigned int m_iSourceSize;          // size of source file
    PTOKENENTRY m_pTokBuf;               // buffer for TaTagTokenize tokens
    CHAR m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    PLOADEDTABLE m_pLoadedTable;         // pointer to currently loaded markup table
    PLOADEDTABLE m_pLoadedRTFTable;      // pointer to loaded RTF tag table
    CHAR m_szInFile[512];                // buffer for input file
    CHAR m_TempFile[540];                // buffer for temporary file name
    BYTE m_bBuffer[TMX_BUFFER_SIZE+1];
    PMEMEXPIMPINFO m_pMemInfo;
    CHAR_W m_szSegBuffer[MAX_SEGMENT_SIZE+1]; // buffer for the processing of segment data
    int  m_currentTu;                    // export: number of currently processed tu
};


//+----------------------------------------------------------------------------+
//| Interface functions called by TranslationManager                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
USHORT EXTMEMEXPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszOutFile,         // fully qualified file name for the exported memory
  PMEMEXPIMPINFO   pMemInfo            // pointer to structure containing general information for the exported memory
)
{
  USHORT           usRC = 0;           // function return code
  CTMXExportImport *pTMXExport = new CTMXExportImport;
  pTMXExport->WriteHeader( pszOutFile, pMemInfo ); 
  *plHandle = (LONG)pTMXExport;
  return( usRC );
} /* end of function EXTMEMEXPORTSTART */

USHORT EXTMEMEXPORTPROCESS
( 
  LONG             lHandle,                 // export/import handle as set by ExtMemExportStart function
  PMEMEXPIMPSEG    pSegment                 // pointer to structure containing the segment data
)
{
  USHORT           usRC = 0;           // function return code
  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;
  pTMXExport->WriteSegment( pSegment ); 
  return( usRC );
} /* end of function EXTMEMEXPORTPROCESS */

//
// EXTMEMEXPORTEND
//
// This function is called once at the end of the memory export
USHORT EXTMEMEXPORTEND
( 
  LONG             lHandle                  // export/import handle as set by ExtMemExportStart function
)
{
  USHORT           usRC = 0;           // function return code
  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;
  pTMXExport->WriteEnd(); 
  delete pTMXExport;
  return( usRC );
} /* end of function EXTMEMEXPORTEND */

USHORT  EXTMEMIMPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszInFile,          // fully qualified file name for the memory being imported
  PMEMEXPIMPINFO   pMemInfo,            // pointer to structure receiving general information for the imported memory
  ImportStatusDetails*     pImportData
)
{
  CTMXExportImport *pTMXExport = new CTMXExportImport;
  USHORT usRC = pTMXExport->StartImport( pszInFile, pMemInfo, pImportData ); 
  *plHandle = (LONG)pTMXExport;

  return( usRC );
} /* end of function EXTMEMIMPORTSTART */

USHORT  EXTMEMIMPORTPROCESS
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PFN_MEMINSERTSEGMENT pfnInsertSegment,    // callback function to insert segments into the memory
  LONG              pMemHandle,               // memory handle (has to be passed to pfnInsertSegment function)
  ImportStatusDetails*     pImportData
)
{
  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;
  USHORT usRC = pTMXExport->ImportNext( MEMINSERTSEGMENT, pMemHandle,    pImportData ); 
  return( usRC );
} /* end of function EXTMEMIMPORTPROCESS */

USHORT  EXTMEMIMPORTEND
( 
  LONG             lHandle                  // Import/import handle as set by ExtMemImportStart function
)
{
  USHORT           usRC = 0;           // function return code
  
  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;
  if(pTMXExport == nullptr) return 0;
  usRC = pTMXExport->EndImport(); 
  delete pTMXExport;

  return( usRC );
} /* end of function EXTMEMIMPORTEND */





//+----------------------------------------------------------------------------+
//| Implementation of TMXImportExport class                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
CTMXExportImport::CTMXExportImport()
{
  m_pTokBuf = (PTOKENENTRY)malloc( TMXTOKBUFSIZE );
  m_pLoadedTable = NULL;
  m_szActiveTagTable[0] = EOS;
}

CTMXExportImport::~CTMXExportImport()
{
  if ( m_pTokBuf ) free( m_pTokBuf );
  if ( m_pLoadedTable ) TAFreeTagTable( m_pLoadedTable );
  if ( m_pLoadedRTFTable ) TAFreeTagTable( m_pLoadedRTFTable );
}

USHORT CTMXExportImport::WriteHeader
( 
  const char *pszOutFile,
  PMEMEXPIMPINFO pMemInfo 
)
{
  this->m_pMemInfo = pMemInfo;
  this->m_currentTu = 1;

  // write TMX header
  m_xw.SetFileName( pszOutFile );
  m_xw.Formatting = CXmlWriter::Indented;
  m_xw.Indention = 2;
  if ( pMemInfo->fUTF16 )
  {
    m_xw.Encoding = CXmlWriter::UTF16;
  }
  else
  {
    m_xw.Encoding = CXmlWriter::UTF8;
  } /* endif */

  m_xw.WriteStartDocument();

  m_xw.WriteStartElement( "tmx" );
  m_xw.WriteAttributeString( "version", "1.4" ); 

  m_xw.WriteStartElement( "header" );
  m_xw.WriteAttributeString( "creationtoolversion", appVersion ); 
  m_xw.WriteAttributeString( "gitCommit", gitHash ); 
  m_xw.WriteAttributeString( "segtype", "sentence" ); 
  m_xw.WriteAttributeString( "adminlang", "en-us" ); 

  {
    // get TMX language name
    CHAR szTMXLang[20];                  // buffer for TMX language
    szTMXLang[0] = EOS;
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getISOName( pMemInfo->szSourceLang, szTMXLang );
    m_xw.WriteAttributeString( "srclang", szTMXLang ); 
  }
 
  m_xw.WriteAttributeString( "o-tmf", "t5memory" ); 
  m_xw.WriteAttributeString( "creationtool", "t5memory" ); 

  // try to get data type of markup
  {
    CHAR  szDataType[40];

    szDataType[0] = EOS;
    FindName( Markup2Type, pMemInfo->szFormat, szDataType, sizeof(szDataType) );
    if ( szDataType[0] )
    {
      m_xw.WriteStartAttribute( "datatype" );
      m_xw.WriteString( szDataType ); 
      m_xw.WriteEndAttribute();
    }
    else
    {
      m_xw.WriteAttributeString( "datatype", "plaintext" ); 
    } /* endif */
  }

  // add memory description as as property
  if ( pMemInfo->szDescription[0] )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", DESCRIPTION_PROP );
    m_xw.WriteString( pMemInfo->szDescription ); 
    m_xw.WriteEndElement(); // prop
  } /* endif */

  m_xw.WriteEndElement(); // header

  m_xw.WriteStartElement( "body" );
  return( 0 );
}

USHORT CTMXExportImport::WriteSegment
( 
  PMEMEXPIMPSEG pSegment  
)
{
  //CHAR             szDocument[EQF_DOCNAMELEN];   // document  
  //BOOL             fMachineTranslation;          // TRUE = segment is a machine translation 
  //CHAR             szAuthor[EQF_AUTHORLEN];      // author of segment


  m_xw.WriteStartElement( "tu" );
  m_xw.WriteStartAttribute( "tuid" );
  m_xw.WriteInt( this->m_currentTu++ ); 
  m_xw.WriteEndAttribute();

  // try to get data type of markup
  {
    CHAR  szDataType[40];

    szDataType[0] = EOS;
    FindName( Markup2Type, pSegment->szFormat, szDataType, sizeof(szDataType) );
    if ( szDataType[0] )
    {
      m_xw.WriteStartAttribute( "datatype" );
      m_xw.WriteString( szDataType ); 
      m_xw.WriteEndAttribute();
    } /* endif */
  }
  
  // creation date/time
  {
    CHAR szTime[30];
    memset(szTime, 0, 30*sizeof(szTime[0]));
    LONG lTime = pSegment->lTime;
    struct tm   *pTimeDate;            // time/date structure

    if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

    pTimeDate = gmtime( &lTime );
    if ( (lTime != 0L) && pTimeDate )   // if gmtime was successful ...
    {
      sprintf( szTime, "%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ", 
               pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
               pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
      m_xw.WriteStartAttribute( "creationdate" );
      m_xw.WriteString( szTime ); 
      m_xw.WriteEndAttribute();
    } /* endif */
  }

  // add segment number as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", SEGNUM_PROP );
  m_xw.WriteInt( pSegment->lSegNum );
  m_xw.WriteEndElement(); // prop

  // add segment number as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", "t5:InternalKey" );
  m_xw.WriteString( pSegment->szInternalKey );
  m_xw.WriteEndElement(); // prop

  // add original tmgr markup as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", MARKUP_PROP );
  m_xw.WriteString( pSegment->szFormat ); 
  m_xw.WriteEndElement(); // prop

  // add document name property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", DOCNAME_PROP );
  m_xw.WriteString( pSegment->szDocument ); 
  m_xw.WriteEndElement(); // prop

  // add translation flag as property
  if ( pSegment->usTranslationFlag == TRANSLFLAG_MACHINE )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", TRANSLFLAG_PROP );
    m_xw.WriteString( "1" ); 
    m_xw.WriteEndElement(); // prop
  } 
  else if ( pSegment->usTranslationFlag == TRANSLFLAG_GLOBMEM )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", TRANSLFLAG_PROP );
    m_xw.WriteString( "2" ); 
    m_xw.WriteEndElement(); // prop
  } /* endif */

  // add segment note as property
  if ( pSegment->szAddInfo[0] != 0 )
  {
    static CHAR_W szBuffer[MAX_SEGMENT_SIZE];
    HADDDATAKEY hKey = MADSearchKey( pSegment->szAddInfo, L"Note" );
    if ( hKey != NULL )
    {
      MADGetAttr( hKey, L"style", szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
      if ( szBuffer[0] != EOS ) 
      {
        m_xw.WriteStartElement( "prop" );
        m_xw.WriteAttributeString( "type", NOTESTYLE_PROP );
        m_xw.WriteString( szBuffer); 
        m_xw.WriteEndElement(); // prop
      } /* endif */         

      MDAGetValueForKey( hKey, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
      if ( szBuffer[0] != EOS ) 
      {
        m_xw.WriteStartElement( "prop" );
        m_xw.WriteAttributeString( "type", NOTE_PROP );
        m_xw.WriteString( szBuffer); 
        m_xw.WriteEndElement(); // prop
      } /* endif */         
    } /* endif */           

    hKey = MADSearchKey( pSegment->szAddInfo, L"MT" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", szAttrNameBuffer );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */

    hKey = MADSearchKey( pSegment->szAddInfo, L"wcnt" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute && (wcsicmp( szAttrNameBuffer, L"words" ) == 0) )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", L"tmgr:words" );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */


    hKey = MADSearchKey( pSegment->szAddInfo, L"MatchSegID" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute && (wcsicmp( szAttrNameBuffer, L"ID" ) == 0) )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", L"tmgr:MatchSegID" );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */
  } /* endif */


  // write source tuv
  WriteTUV( pSegment->szSourceLang, pSegment->szFormat, pSegment->szSource );

  // write target tuv
  WriteTUV( pSegment->szTargetLang, pSegment->szFormat, pSegment->szTarget );

  m_xw.WriteEndElement(); // tu

  return( 0 );
}

// write a TUV
USHORT CTMXExportImport::WriteTUV
( 
  PSZ   pszLanguage,                   // language to be used
  PSZ   pszMarkup,                     // markup of data   
  PSZ_W   pszSegmentData               // actual segment data (UTF-16!)
)
{
  CHAR szTMXLang[20];                  // buffer for TMX language

  T5LOG( T5DEVELOP)<< "WriteTUV:: lang = " << pszLanguage<< "; markup = "<< pszMarkup;
  // get TMX language name
  szTMXLang[0] = EOS;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  pLangFactory->getISOName( pszLanguage, szTMXLang );

  // start TUV
  m_xw.WriteStartElement( "tuv" );
  m_xw.WriteAttributeString( "xml:lang", szTMXLang ); 

  // add original Tmgr language as property
  //m_xw.WriteStartElement( "prop" );
  //m_xw.WriteAttributeString( "type", LANGUAGE_PROP );
  //m_xw.WriteString( pszLanguage ); 
  //m_xw.WriteEndElement(); // prop

  // write segment data and mask inline tags
  m_xw.WriteStartElement( "seg" );
  m_xw.Formatting = CXmlWriter::None;             // no indention within segment

  // write segment data as-is when one of the XLIFF markup tables is being used - but ensure that single ampersands are encoded properly
  {
    PSZ_W pszCurPos, pszAmp;

    m_xw.WriteString( L"" ); // needed to properly close the start element tag
    pszCurPos = pszSegmentData;
    pszAmp = wcschr( pszCurPos, L'&' );
    while ( pszAmp )
    {
      CHAR_W cNext = pszAmp[1];

      // write text up to ampersand
      if ( pszCurPos < pszAmp )
      {
        *pszAmp = 0;
        m_xw.WriteRaw( pszCurPos );
        *pszAmp = L'&';
      } /* endif */

      if ( iswalpha( cNext ) || (cNext == L'#') )
      {
        // ampersand seems to start an entity so write it to output
        pszCurPos = pszAmp + 1;
        m_xw.WriteRaw( L"&");
      }
      else
      {
        m_xw.WriteRaw( L"&amp;");
        pszCurPos = pszAmp + 1;
      }
      pszAmp = wcschr( pszCurPos, L'&' );
    } /* endwhile */
    if ( *pszCurPos != 0 ) 
      m_xw.WriteRaw( pszCurPos );
  }
  
  // end open elements
  m_xw.WriteEndElement(); // seg
  m_xw.Formatting = CXmlWriter::Indented;             // restart indention 
  m_xw.WriteEndElement(); // tuv

  return( 0 );
}

USHORT CTMXExportImport::WriteEnd()
{
  m_xw.WriteEndElement(); // body
  m_xw.WriteEndElement(); // tmx
  m_xw.WriteEndDocument();
  m_xw.Close();
  return( 0 );
}

USHORT CTMXExportImport::StartImport
( 
  const char *pszInFile, 
  PMEMEXPIMPINFO pMemInfo,
  ImportStatusDetails* pImportDetails
)
{
  USHORT usRC = 0;

  this->m_TempFile[0] = EOS;
  this->m_pMemInfo = pMemInfo;
  
  if ( pMemInfo->fCleanRTF )
  {
    TALoadTagTableExHwnd( "EQFRTF", &m_pLoadedRTFTable, FALSE, TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, HWND_FUNCIF );
  } /* endif */

  // preprocess TMX file
  if ( !usRC )
  {
    // setup tempory file name
    Properties::GetInstance()->get_value(KEY_MEM_DIR, this->m_TempFile, sizeof(this->m_TempFile));
    //UtlMakeEQFPath( this->m_TempFile, NULC, MEM_PATH, NULL );
    strcat( this->m_TempFile, BACKSLASH_STR );
    if ( strchr( (PSZ)pszInFile, '/' ) == NULL )
    {
      strcat( this->m_TempFile, (PSZ)pszInFile );
    }
    else
    {
      strcat( this->m_TempFile, UtlGetFnameFromPath( (PSZ)pszInFile ) );
    } /* endif */
    strcat( this->m_TempFile, ".Temp" );

    // call preprocess method
    usRC = this->PreProcessInFile( pszInFile, this->m_TempFile );
  } /* endif */

  // parse the TMX file
  if ( !usRC )
  {
      m_iSourceSize = FilesystemHelper::GetFileSize(this->m_TempFile);

      m_parser = new SAXParser();      // Create a SAX parser object

      // create an instance of our handler
      m_handler = new TMXParseHandler();
      m_handler->SetImportData(pImportDetails);

      // pass memory info to handler
      m_handler->SetMemInfo( m_pMemInfo ); 

      //  install our SAX handler as the document and error handler.
      m_parser->setDocumentHandler( m_handler );
      m_parser->setErrorHandler( m_handler );
      m_parser->setCalculateSrcOfs( TRUE );
      m_parser->setValidationSchemaFullChecking( FALSE );
      m_parser->setDoSchema( FALSE );
      m_parser->setLoadExternalDTD( FALSE );
      m_parser->setValidationScheme( SAXParser::Val_Never );
      m_parser->setExitOnFirstFatalError( FALSE );
      //m_parser->setExitOnFirstFatalError( TRUE );
      
      strcpy( m_szInFile, pszInFile );

    try
    {
      if (!m_parser->parseFirst( this->m_TempFile, m_SaxToken))
      {
          //XERCES_STD_QUALIFIER cerr << "scanFirst() failed\n" << XERCES_STD_QUALIFIER endl;
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_READ_FAULT);
      }
      else
      {
        // parse until header processed of no more data available
        BOOL fContinue  = TRUE;
        while (fContinue && !m_parser->getErrorCount() && !m_handler->IsHeaderDone() )
        {
          fContinue = m_parser->parseNext(m_SaxToken);
        } /*endwhile */

        if ( m_handler->ErrorOccured() )
        {
          m_handler->GetErrorText( pMemInfo->szError, sizeof(pMemInfo->szError) );
          pMemInfo->fError = TRUE;
        } /* endif */

        // stop import if no more data to follow
        if ( !fContinue && !m_handler->IsHeaderDone() )
        {
          m_parser->parseReset(m_SaxToken);
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_BAD_FORMAT);
        } /* endif */

        // get description and source language of memory
        if ( m_handler->IsHeaderDone() )
        {
          // save target memory source language
          CHAR szMemSourceLanguage[50]; 
          strcpy( szMemSourceLanguage, pMemInfo->szSourceLang );
          
          // get description and source language of imported memory
          m_handler->GetDescription( pMemInfo->szDescription, sizeof(pMemInfo->szDescription) );
          m_handler->GetSourceLanguage( pMemInfo->szSourceLang, sizeof(pMemInfo->szSourceLang)  );

          // always use source language of target memory
          if ( szMemSourceLanguage[0] != EOS)
          {
            m_handler->SetSourceLanguage( szMemSourceLanguage );            
          } /* endif */
        } /* endif */
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      LOG_AND_SET_RC(usRC, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_READ_FAULT);
    }

  } /* endif */


  return( usRC );
}

USHORT CTMXExportImport::ImportNext
( 
  PFN_MEMINSERTSEGMENT pfnInsertSegment, 
  LONG MemHandle,
  ImportStatusDetails*     pImportData
)
{
  USHORT usRC = 0;
  int  iIteration = 10;
  BOOL fContinue  = TRUE;
  int errorCode = 0;
  int errorCount = 0;
  m_handler->SetMemInterface( MEMINSERTSEGMENT, MemHandle, m_pLoadedRTFTable, this->m_pTokBuf, TMXTOKBUFSIZE ); 
    try
    {
      while (fContinue && (m_parser->getErrorCount() <= m_handler->getInvalidCharacterErrorCount()) && iIteration )
      {
        fContinue = m_parser->parseNext(m_SaxToken);
        iIteration--;
      } /*endwhile */

      errorCount = m_parser->getErrorCount() - m_handler->getInvalidCharacterErrorCount();

      if(pImportData != nullptr){
        pImportData->invalidSymbolErrors = m_handler->getInvalidCharacterErrorCount();

        // compute current progress
        if ( !errorCount && fContinue )
        {
          int iPos = (int)m_parser->getSrcOffset();

          INT64 iComplete = (INT64)iPos;
          INT64 iTotal = (INT64)m_iSourceSize;
          pImportData->usProgress = (USHORT)(iComplete * 100.0 / iTotal);
        } /* endif */
      }
      if ( m_handler->ErrorOccured() )
      {
        char buff[1024];
        m_handler->GetErrorText( buff, sizeof(buff) );
        size_t occupied = strlen(m_pMemInfo->szError);
        size_t buffLen = strlen(buff);
        strncat(m_pMemInfo->szError, buff, std::min(sizeof(m_pMemInfo->szError) - occupied - 1, buffLen));

        //m_handler->GetErrorText( m_pMemInfo->szError, sizeof(m_pMemInfo->szError) );
        m_pMemInfo->fError = TRUE;
        pImportData->importMsg << buff <<";;; ";
      } /* endif */

      if ( errorCount || !fContinue )
      {
        m_parser->parseReset(m_SaxToken);
        if ( errorCount )
        {
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_MEMIMP_ERROR);
        }
        else
        {
          LOG_AND_SET_RC(usRC, T5INFO, MEM_IMPORT_COMPLETE);
        }
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        errorCode = 5;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        errorCode = 4;
    }
  
  return( usRC );
}

USHORT CTMXExportImport::EndImport()
{
  USHORT usRC = 0;
  delete m_parser;                     // Delete the parser itself
  delete m_handler;                    // delete the handler  
  // remove temporary file
  if ( this->m_TempFile[0] != EOS )
  {
    UtlDelete( this->m_TempFile, 0, FALSE );
  } /* endif */

  return( usRC );
}

USHORT CTMXExportImport::getLastError
( 
  PSZ              pszErrorBuffer,          // pointer to buffer for the error text
  int              iBufferLength            // sizeof buffer in number of characters
)
{
  strncpy( pszErrorBuffer, this->m_pMemInfo->szError, iBufferLength - 1);
  pszErrorBuffer[iBufferLength-1] = 0;
  return( 0 );
}

// replace invalid characters in input file
USHORT CTMXExportImport::PreProcessInFile
( 
  const char *pszInFile, 
  const char *pszOutFile
)
{
  USHORT      usRC = 0;
  FILE        *hIn = NULL;
  FILE        *hOut = NULL;
  BYTE* buff = nullptr;
  
  hIn = fopen( pszInFile, "rb" );
  if ( hIn == NULL )
  {
    LOG_AND_SET_RC(usRC, T5INFO, ERROR_FILE_NOT_FOUND);
  } /* endif */

  if ( !usRC )
  {
    hOut = fopen( pszOutFile, "wb" );
    if ( hOut == NULL )
    {
      LOG_AND_SET_RC(usRC, T5INFO, ERROR_FILE_NOT_FOUND);
    } /* endif */
  } /* endif */

  
  // read/write files until done
  if ( !usRC )
  {
    BOOL fUTF16 = FALSE;

    // check file encoding
    fread( m_bBuffer, 1, 2, hIn );
    if ( (m_bBuffer[0] == 0xFF) && (m_bBuffer[1] == 0xFE) ) 
    {
      // file starts with BOM
      fUTF16 = TRUE;
      // write BOM to output file and advance buffer pointer
      //fwrite( m_bBuffer, 1, 2, hOut );
      //pCurPos += 2;
    }
    else if ( (m_bBuffer[0] != 0) && (m_bBuffer[1] == 0) ) 
    {      
      fUTF16 = TRUE;   // assume UTF16lap
      fseek( hIn, 0, SEEK_SET ); // re-position to begin of file
    }
    else
    {
      fseek( hIn, 0, SEEK_SET ); // re-position to begin of file
    } /* endif */

    
    size_t len = 0;
    if ( !usRC  ){
      fseek(hIn, 0, SEEK_END);
      long fsize = ftell(hIn);
      T5LOG( T5DEBUG) << " Allocating filebuffer with " <<fsize <<" bytes for " << pszInFile;
      fseek(hIn, 0, SEEK_SET);  /* same as rewind(f); */

      buff = new BYTE[fsize+2];
      buff[fsize] = buff[fsize+1] = 0;
      fread(buff, fsize, 1, hIn);
      len = fsize;
    }

    if(!buff){
      usRC = usRC? usRC : -1;
    }
    
    if(hIn){
      fclose(hIn);
      hIn = nullptr;
    }
        
    if(fUTF16){      //convert utf16 to utf8
      //auto data = EncodingHelper::toChar((char16_t*) pCurPos);
      auto u16str = std::u16string ((char16_t*) buff);
      auto data = EncodingHelper::toChar(u16str);

      if (data.find("UTF-16") != std::string::npos)
        data.replace(data.find("UTF-16"), 6, "UTF-8");
      
      strcpy((PSZ) buff, data.c_str());
      len = data.size();
      ((PSZ)buff)[len] = '\0';
    }
    
    PSZ pszCurPos = (PSZ) buff;
    PSZ pszStartPos = pszCurPos;

    while ( *pszCurPos != 0  && pszCurPos-pszStartPos < len)
    {
      // check for entitities with values below 0x20
      if ( *pszCurPos == '&' )
      {
        int iValue = 0;
        int iEntityLen = 0;
        if ( CheckEntity( pszCurPos, &iValue, &iEntityLen ) )
        {
          if ( iValue < 32 )
          {
            // mask entity

            // write chars up to entity character
            fwrite( pszStartPos, 1, pszCurPos - pszStartPos, hOut );

            // write masked entity 
            fputs( ENTITY_MASK_START, hOut );
            fwrite( pszCurPos + 1, 1, iEntityLen - 1, hOut );
            fputs( ENTITY_MASK_END, hOut );

            // set new start position
            pszCurPos += iEntityLen;
            pszStartPos = pszCurPos;
          }
          else
          {
            // leave as-is
            pszCurPos += iEntityLen;
          } /* endif */
        }
        else
        {
          pszCurPos++;
        }
      }
      else
      {
        pszCurPos++;
      } /* endif */    
    } /*endwhile */

    // write remaining characters
    if ( pszCurPos != pszStartPos )
    {
      fwrite( pszStartPos, 1, pszCurPos - pszStartPos, hOut );
    } /* endif */
  } /* endif */


  //cleanup
  if ( hIn )  fclose( hIn );
  if ( hOut ) fclose( hOut );
  if ( buff ){
    delete[] buff;
    buff = nullptr;
  }

  return( usRC );
}




// find the given name and return its value in the supplied buffer
BOOL FindName( PNAMETABLE pTable, const char *pszName, char *pszValue, int iBufSize )
{
  BOOL fFound = FALSE;
  PNAMETABLE  pEntry = pTable;

  while ( !fFound && (pEntry->pszName != NULL) )
  {
    if ( _stricmp( pEntry->pszName, pszName ) == 0 )
    {
      memset( pszValue, 0, iBufSize );
      strncpy( pszValue, pEntry->pszValue, iBufSize - 1 );
      fFound = TRUE;
    }
    else
    {
      // try next entry
      pEntry++;
    } /* endif */
  } /*endwhile */

  return( fFound );
}

//
// Implementation of TMX SAX parser
//
TMXParseHandler::TMXParseHandler()
{
   // allocate buffer areas
  pBuf = (PBUFFERAREAS)malloc( sizeof(BUFFERAREAS) );
  if ( pBuf ) memset( pBuf, 0, sizeof(BUFFERAREAS) );
  fError = FALSE;

  // initialize element stack
  iStackSize = 0;
  iCurElement = 0;
  pStack = NULL;

  pfnInsertSegment = NULL;
  lMemHandle = 0;
  fHeaderDone = FALSE;
  fSource = FALSE;
  fTarget= FALSE;
  fCatchData= FALSE;
  fWithTagging= FALSE;
  fWithTMXTags= FALSE;
  fTMXTagStarted= FALSE;

  // initialize TUV array
  pTuvArray = NULL;
  iCurTuv = 0;
  iTuvArraySize = 0;

  tagReplacer.sourceTagList.reserve(50);
  tagReplacer.targetTagList.reserve(50);
  tagReplacer.requestTagList.reserve(50);
  tagReplacer.activeSegment = SOURCE_SEGMENT;
}

TMXParseHandler::~TMXParseHandler()
{
  if ( pStack ) free( pStack );
  if ( pBuf )   free( pBuf );
  if ( pTuvArray ) free( pTuvArray );
}




bool IsValidXml(std::wstring&& sentence){
  USHORT usRc = 0; 
  std::vector<std::wstring> res;
  
  std::string src = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(sentence) + std::string("</TMXSentence>");
  SAXParser parser;
  TMXParseHandler handler;
  // create an instance of our handler
  handler.fReplaceWithTagsWithoutAttributes = true;
  handler.tagReplacer.activeSegment = SOURCE_SEGMENT;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser.setDocumentHandler(&handler);
  parser.setErrorHandler(&handler);
  parser.setValidationSchemaFullChecking( false );
  parser.setDoSchema( false );
  parser.setLoadExternalDTD( false );
  parser.setValidationScheme( SAXParser::Val_Never );
  parser.setExitOnFirstFatalError( true );
  handler.fInitialized = true;  
  
  xercesc::MemBufInputSource src_buff((const XMLByte *)src.c_str(), src.size(),
                                      "src_buff (in memory)");

  parser.parse(src_buff);
  if(parser.getErrorCount()){
    char buff[512];
    handler.GetErrorText(buff, sizeof(buff));
    std::string errMsg(buff);
    T5LOG(T5ERROR) << ":: error during parsing src : " << errMsg;
    return false;
  }
  return true;
}


std::vector<std::wstring> ReplaceOriginalTagsWithPlaceholdersFunc(std::wstring &&w_src, std::wstring &&w_trg, bool fSkipAttributes)
{
  USHORT usRc = 0; 
  std::vector<std::wstring> res;
  
  std::string src = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_src) + std::string("</TMXSentence>");
  SAXParser *parser = new SAXParser();
  // create an instance of our handler
  TMXParseHandler *handler = new TMXParseHandler();
  handler->fReplaceWithTagsWithoutAttributes = fSkipAttributes;
  handler->tagReplacer.activeSegment = SOURCE_SEGMENT;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser->setDocumentHandler(handler);
  parser->setErrorHandler( handler );
  parser->setValidationSchemaFullChecking( false );
  parser->setDoSchema( false );
  parser->setLoadExternalDTD( false );
  parser->setValidationScheme( SAXParser::Val_Never );
  parser->setExitOnFirstFatalError( false );
  T5LOG( T5DEBUG) << ":: parsing str = \'" << src << "\'";
  xercesc::MemBufInputSource src_buff((const XMLByte *)src.c_str(), src.size(),
                                      "src_buff (in memory)");

  parser->parse(src_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler->GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing src : " << buff;
  }
  std::wstring src_res = handler->GetParsedData();  
  res.push_back(src_res);

  std::string trg;
  if(w_trg.empty() ){
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
      std::string res_src_s = EncodingHelper::convertToUTF8(src_res);
      T5LOG( T5DEBUG) << ":: original source = \n\'" << src << "\'\n replaced with:\n\'" << 
            res_src_s <<"\'\n";
    }
  }else{    
    handler->tagReplacer.activeSegment = TARGET_SEGMENT;
    trg  = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_trg) + std::string("</TMXSentence>");
    xercesc::MemBufInputSource trg_buff((const XMLByte *)trg.c_str(), trg.size(),
                                        "trg_buff (in memory)");
    parser->parse(trg_buff);
    if(parser->getErrorCount()){
      char buff[512];
      handler->GetErrorText(buff, sizeof(buff));
      T5LOG(T5ERROR) << ":: error during parsing trg : " << buff;
    }

    std::wstring trg_res = handler->GetParsedData();    

    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
      std::string res_src_s = EncodingHelper::convertToUTF8(src_res);
      std::string res_trg_s = EncodingHelper::convertToUTF8(trg_res);
      T5LOG( T5DEBUG) << ":: original source = \n\'" << src << "\'\n replaced with:\n\'" << 
              res_src_s << "\'\n target: \'" << trg << "\'\n replaced with:\n\'" << res_trg_s << "\'\n";
    }
    res.push_back(trg_res);
  }
  
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
    T5LOG( T5DEVELOP) <<  ":: source tags: ";
    for(int i = 0; i < handler->tagReplacer.sourceTagList.size(); i++){
      T5LOG(T5DEVELOP) << TagReplacer::LogTag(handler->tagReplacer.sourceTagList[i]);
    }
    T5LOG( T5DEVELOP) << ":: target tags: ";
    for(int i = 0; i < handler->tagReplacer.targetTagList.size(); i++){
      T5LOG(T5DEVELOP) << TagReplacer::LogTag(handler->tagReplacer.targetTagList[i]);
    }

  }
  delete parser;
  delete handler;

  return res;
}


StringTagVariants::StringTagVariants(std::wstring&& w_str){
   // parse and save request
  norm.reserve(MAX_SEGMENT_SIZE);
  original.reserve(MAX_SEGMENT_SIZE);
  genericTags.reserve(MAX_SEGMENT_SIZE);
  npReplaced.reserve(MAX_SEGMENT_SIZE);
  
  SAXParser parser;
  original = w_str;
  // create an instance of our handler
  TMXParseHandler handler;
  
  handler.tagReplacer.fFuzzyRequest = true;
  handler.tagReplacer.activeSegment = REQUEST_SEGMENT;
  //handler.tagReplacer.fReplaceNumberProtectionTagsWithHashes = true;
  handler.fCreateNormalizedStr = true;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser.setDocumentHandler(&handler);
  parser.setErrorHandler(&handler);
  parser.setValidationSchemaFullChecking( false );
  parser.setDoSchema( false );
  parser.setLoadExternalDTD( false );
  parser.setValidationScheme( SAXParser::Val_Never );
  parser.setExitOnFirstFatalError( false );

  std::string req = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_str) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing request str = \'" <<  req << "\'";
  xercesc::MemBufInputSource req_buff((const XMLByte *)req.c_str(), req.size(),
                                      "req_buff (in memory)");
  parser.parse(req_buff);
  if(parser.getErrorCount()){
    char buff[512];
    handler.GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing req : " << buff;
  }else{
    fSuccess = true;
    genericTags = handler.GetParsedData();
    norm = handler.GetParsedNormalizedData();
    npReplaced = handler.GetParsedDataWithReplacedNpTags();
  }  
}

/*
std::vector<std::wstring> GenerateGenericAndNormalizedStrings(std::wstring&& w_str){

  // parse and save request
  SAXParser *parser = new SAXParser();
  // create an instance of our handler
  TMXParseHandler handler;
  
  handler.tagReplacer.fFuzzyRequest = true;
  handler.tagReplacer.activeSegment = REQUEST_SEGMENT;
  //handler.tagReplacer.fReplaceNumberProtectionTagsWithHashes = true;
  handler.fCreateNormalizedStr = true;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser->setDocumentHandler(&handler);
  parser->setErrorHandler(&handler);
  parser->setValidationSchemaFullChecking( false );
  parser->setDoSchema( false );
  parser->setLoadExternalDTD( false );
  parser->setValidationScheme( SAXParser::Val_Never );
  parser->setExitOnFirstFatalError( false );

  std::string req = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_str) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing request str = \'" <<  req << "\'";
  xercesc::MemBufInputSource req_buff((const XMLByte *)req.c_str(), req.size(),
                                      "req_buff (in memory)");
  std::wstring norm, npRepl, generic;
  parser->parse(req_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler.GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing req : " << buff;
  }else{
    generic = handler.GetParsedData();
    norm = handler.GetParsedNormalizedData();
    npRepl = handler.GetParsedDataWithReplacedNpTags();
  }
  std::vector<std::wstring> output;
  output.push_back(generic);
  output.push_back(npRepl);
  output.push_back(norm);
  delete parser;
  return output;
}

std::wstring ReplaceNPTagsWithHashesAndTagsWithGenericTags(std::wstring&& w_str){

  // parse and save request
  SAXParser *parser = new SAXParser();
  std::wstring  res;
  // create an instance of our handler
  {
  TMXParseHandler handler;
  
  handler.tagReplacer.fFuzzyRequest = true;
  handler.tagReplacer.activeSegment = REQUEST_SEGMENT;
  handler.tagReplacer.fReplaceNumberProtectionTagsWithHashes = true;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser->setDocumentHandler(&handler);
  parser->setErrorHandler(&handler);
  parser->setValidationSchemaFullChecking( false );
  parser->setDoSchema( false );
  parser->setLoadExternalDTD( false );
  parser->setValidationScheme( SAXParser::Val_Never );
  parser->setExitOnFirstFatalError( false );

  std::string req = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_str) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing request str = \'" <<  req << "\'";
  xercesc::MemBufInputSource req_buff((const XMLByte *)req.c_str(), req.size(),
                                      "req_buff (in memory)");
  std::wstring norm, npRepl;
  parser->parse(req_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler.GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing req : " << buff;
  }
  delete parser;
  return res;
}


std::wstring ReplaceNPTagsWithHashesAndNormalizeString(std::wstring&& w_str){

  // parse and save request
  SAXParser *parser = new SAXParser();
  // create an instance of our handler
  TMXParseHandler handler;
  
  handler.tagReplacer.fFuzzyRequest = true;
  handler.tagReplacer.activeSegment = REQUEST_SEGMENT;
  handler.tagReplacer.fReplaceNumberProtectionTagsWithHashes = true;
  handler.tagReplacer.fSkipTags = true;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser->setDocumentHandler(&handler);
  parser->setErrorHandler(&handler);
  parser->setValidationSchemaFullChecking( false );
  parser->setDoSchema( false );
  parser->setLoadExternalDTD( false );
  parser->setValidationScheme( SAXParser::Val_Never );
  parser->setExitOnFirstFatalError( false );

  std::string req = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_str) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing request str = \'" <<  req << "\'";
  xercesc::MemBufInputSource req_buff((const XMLByte *)req.c_str(), req.size(),
                                      "req_buff (in memory)");

  parser->parse(req_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler.GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing req : " << buff;
  }
  delete parser;
  return handler.GetParsedData();
}
//*/

std::vector<std::wstring> ReplaceOriginalTagsWithTagsFromRequestFunc(std::wstring&& w_request, std::wstring &&w_src, std::wstring &&w_trg){ 
  USHORT usRc = 0; 
  std::vector<std::wstring> res;

  // parse and save request
  SAXParser *parser = new SAXParser();
  // create an instance of our handler
  TMXParseHandler *handler = new TMXParseHandler();
  
  handler->tagReplacer.fFuzzyRequest = true;
  handler->tagReplacer.activeSegment = REQUEST_SEGMENT;
  XMLPScanToken saxToken;

  //  install our SAX handler as the document and error handler.
  parser->setDocumentHandler(handler);
  parser->setErrorHandler( handler );
  parser->setValidationSchemaFullChecking( false );
  parser->setDoSchema( false );
  parser->setLoadExternalDTD( false );
  parser->setValidationScheme( SAXParser::Val_Never );
  parser->setExitOnFirstFatalError( false );

  std::string req = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_request) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing request str = \'" <<  req << "\'";
  xercesc::MemBufInputSource req_buff((const XMLByte *)req.c_str(), req.size(),
                                      "req_buff (in memory)");

  parser->parse(req_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler->GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing req : " << buff;
  }

  std::wstring req_res = handler->GetParsedData();
  
  // do tag replacement for source and target 
  std::string src = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_src) + std::string("</TMXSentence>");
  T5LOG( T5DEBUG) << ":: parsing str = \'" << src << "\'";
  xercesc::MemBufInputSource src_buff((const XMLByte *)src.c_str(), src.size(),
                                      "src_buff (in memory)");
  handler->tagReplacer.activeSegment = SOURCE_SEGMENT;
  handler->tagReplacer.iHighestPTId = 500;
  handler->tagReplacer.iHighestPHId = 100;
  handler->tagReplacer.iHighestPTI = 0;

  parser->parse(src_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler->GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing src : ", buff;
  }
  std::wstring src_res = handler->GetParsedData();
  
  std::string trg;
  handler->tagReplacer.activeSegment = TARGET_SEGMENT;
  trg  = std::string("<TMXSentence>") + EncodingHelper::convertToUTF8(w_trg) + std::string("</TMXSentence>");
  xercesc::MemBufInputSource trg_buff((const XMLByte *)trg.c_str(), trg.size(),
                                      "trg_buff (in memory)");
  parser->parse(trg_buff);
  if(parser->getErrorCount()){
    char buff[512];
    handler->GetErrorText(buff, sizeof(buff));
    T5LOG(T5ERROR) << ":: error during parsing trg : " << buff;
  }

  std::wstring trg_res = handler->GetParsedData();    

  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
    std::string res_src_s = EncodingHelper::convertToUTF8(src_res);
    std::string res_trg_s = EncodingHelper::convertToUTF8(trg_res);
    T5LOG( T5DEBUG) << ":: original source = \n\'" << src << "\'\n replaced with:\n\'" << 
            res_src_s <<"\'\n target: \'" << trg << "\'\n replaced with:\n\'" << res_trg_s << "\'\n";
  }
  
  res.push_back(src_res);
  res.push_back(trg_res);
  res.push_back(req_res);
  
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
    T5LOG( T5DEBUG) <<  ":: request tags: ";
    for(int i = 0; i < handler->tagReplacer.requestTagList.size(); i++){
      T5LOG(T5DEVELOP) << TagReplacer::LogTag(handler->tagReplacer.requestTagList[i]);
    }
    T5LOG( T5DEBUG) <<  ":: source tags: ";
    for(int i = 0; i < handler->tagReplacer.sourceTagList.size(); i++){
      T5LOG(T5DEVELOP) << TagReplacer::LogTag(handler->tagReplacer.sourceTagList[i]);
    }
    T5LOG( T5DEBUG) <<  ":: target tags: ";
    for(int i = 0; i < handler->tagReplacer.targetTagList.size(); i++){
      T5LOG(T5DEVELOP) << TagReplacer::LogTag(handler->tagReplacer.targetTagList[i]);
    }

  }

  delete parser;
  delete handler;

  return res;
}

std::wstring TMXParseHandler::GetParsedData() const
{
  if (pBuf == nullptr || pBuf->szData == nullptr)
  {
    return L"";
  }
  return pBuf->szData;
}


std::wstring TMXParseHandler::GetParsedNormalizedData() const
{
  if (pBuf == nullptr || pBuf->szNormalizedData == nullptr)
  {
    return L"";
  }
  return pBuf->szNormalizedData;
}


std::wstring TMXParseHandler::GetParsedDataWithReplacedNpTags()const
{
  if (pBuf == nullptr || pBuf->szReplacedNpData == nullptr)
  {
    return L"";
  }
  return pBuf->szReplacedNpData;
}

void TMXParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    std::u16string u16name((char16_t*)name);
    std::string sName = EncodingHelper::convertToUTF8(u16name);
    char* pszName = (char*) sName.c_str();
    int iAttribs = attributes.getLength(); 

    T5LOG( T5DEVELOP) <<  "StartElement: " << pszName;

    Push( &CurElement );

    CurElement.ID = GetElementID( pszName );
    CurElement.PropID = UNKNOWN_PROP;            // reset prop ID of current element 

    switch ( CurElement.ID )
    {
      case TMX_ELEMENT:
        // reset element info
        memset( &CurElement, 0, sizeof(CurElement) );
        this->iNumOfTu = 0;
        break;
      case PROP_ELEMENT:
        // check if this is one of our own props
        CurElement.PropID = UNKNOWN_PROP;
        for( int i = 0; i < iAttribs; i++ )
        {
          u16name = (char16_t*) attributes.getName( i );
          sName = EncodingHelper::convertToUTF8(u16name);
          char* pszAttName = (char*) sName.c_str();
          if ( strcasecmp( pszAttName, "type" ) == 0 )
          {
            //PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            u16name = (char16_t*) attributes.getValue( i );
            sName = EncodingHelper::convertToUTF8(u16name);
            char* pszAttVal = (char*) sName.c_str();

            if ( pszAttVal != NULL )
            {
              if ( (strcasecmp( pszAttVal, MARKUP_PROP ) == 0) || (strcasecmp( pszAttVal, MARKUP_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMMARKUP_PROP;
              }
              else if ( (strcasecmp( pszAttVal, LANGUAGE_PROP ) == 0) || (strcasecmp( pszAttVal, LANGUAGE_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMLANGUAGE_PROP;
              }
              else if ( (strcasecmp( pszAttVal, DOCNAME_PROP ) == 0) || (strcasecmp( pszAttVal, DOCNAME_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMDOCNAME_PROP;
              }
              else if ( (strcasecmp( pszAttVal, MACHFLAG_PROP ) == 0) || (strcasecmp( pszAttVal, MACHFLAG_PROP_OLD ) == 0) )
              {
                CurElement.PropID = MACHINEFLAG_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, TRANSLFLAG_PROP ) == 0) || (strcasecmp( pszAttVal, TRANSLFLAG_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TRANSLATIONFLAG_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, NOTE_PROP ) == 0) || (strcasecmp( pszAttVal, NOTE_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMNOTE_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, NOTESTYLE_PROP ) == 0) || (strcasecmp( pszAttVal, NOTE_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMNOTESTYLE_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, SEGNUM_PROP ) == 0) || (strcasecmp( pszAttVal, SEGNUM_PROP_OLD ) == 0) )
              {
                CurElement.PropID = SEG_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, DESCRIPTION_PROP ) == 0) || (strcasecmp( pszAttVal, DESCRIPTION_PROP_OLD ) == 0) )
              {
                CurElement.PropID = TMDESCRIPTION_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, TMMATCHTYPE_PROP ) == 0 ) || (strcasecmp( pszAttVal, TMMATCHTYPE_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMTMMATCHTYPE_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, MTSERVICE_PROP ) == 0 ) || (strcasecmp( pszAttVal, MTSERVICE_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTSERVICE_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, MTMETRICNAME_PROP ) == 0 ) || (strcasecmp( pszAttVal, MTMETRICNAME_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTMETRICNAME_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, MTMETRICVALUE_PROP ) == 0 ) || (strcasecmp( pszAttVal, MTMETRICVALUE_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTMETRICVALUE_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, PEEDITDISTANCECHARS_PROP ) == 0 ) || (strcasecmp( pszAttVal, PEEDITDISTANCECHARS_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMPEEDITDISTANCECHARS_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, PEEDITDISTANCEWORDS_PROP ) == 0 ) || (strcasecmp( pszAttVal, PEEDITDISTANCEWORDS_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMPEEDITDISTANCEWORDS_PROP;
              } 
              else if ( (strcasecmp( pszAttVal, MTFIELDS_PROP ) == 0 ) || (strcasecmp( pszAttVal, MTFIELDS_PROP_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTFIELDS_PROP;
              } 
              else if ( strcasecmp( pszAttVal, WORDS_PROP ) == 0 )
              {
                CurElement.PropID = TMWORDS_PROP;
              } 
              else if ( strcasecmp( pszAttVal, MATCHSEGID_PROP ) == 0 )
              {
                CurElement.PropID = TMMATCHSEGID_PROP;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
        pBuf->szPropW[0] = 0;                 // reset data buffer
        break;

      case HEADER_ELEMENT:
        // reset element info
        CurElement.fInlineTagging = FALSE;
        CurElement.fInsideTagging = FALSE;
        CurElement.PropID = UNKNOWN_PROP;
        CurElement.szDataType[0] = 0;
        CurElement.szDataType[0] = 0;
        CurElement.szTMLanguage[0] = 0;
        CurElement.szTMMarkup[0] = 0;
        CurElement.szTMXLanguage[0] = 0;

        // scan header attributes, currently only the source language and the data type are of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          u16name = (char16_t*)attributes.getName( i );
          sName = EncodingHelper::convertToUTF8(u16name);
          PSZ pszName = (char*) sName.c_str();
          if ( strcasecmp( pszName, "datatype" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) strcpy( CurElement.szDataType, pszValue );
            XMLString::release( &pszValue );

            // special handling for datatype in header: set datataype in all elements on the stack
            if ( pStack && iCurElement )
            {
              int i = iCurElement;
              while ( i )
              {
                i--;
                strcpy( pStack[i].szDataType, CurElement.szDataType );
              } /*endwhile */
            } /* endif */

          }
          else if ( strcasecmp( pszName, "srclang" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL )
            {
              if ( strcasecmp( pszValue, "*all*" ) == 0 )
              {
                // use source language of memory
                pBuf->szMemSourceLang[0] = 0; 
              }
              else
              {
                TMXLanguage2TMLanguage( "", pszValue, pBuf->szMemSourceLang );
                strncpy(pBuf->szMemSourceIsoLang, pszValue, 9);
                pBuf->szMemSourceIsoLang[9] = 0;
                if ( stricmp( pBuf->szMemSourceLang, OTHERLANGUAGES ) == 0 )
                {
                  pBuf->szMemSourceLang[0] = 0; 
                } /* endif */
              } /* endif */

              XMLString::release( &pszValue );
            } /* endif */
          } /* endif */
        } /* endfor */
        break;
      case TU_ELEMENT:
        // reset segment data 
        usTranslationFlag = TRANSLFLAG_NORMAL;

        pBuf->szNote[0] = 0;
        pBuf->szNoteStyle[0] = 0;
        pBuf->szMTMetrics[0] = 0;
        pBuf->szMatchSegID[0] = 0;
        pBuf->ulWords = 0;
        lTime = 0;
        
        tagReplacer.reset();

        // reset TUV array
        iCurTuv = 0;
        this->iNumOfTu += 1;

        ulSegNo = this->iNumOfTu; // preset segment number

        // scan <tu> attributes, currently the data type, the tuid and the creationdate are of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          u16name = (char16_t*)attributes.getName( i );
          sName = EncodingHelper::convertToUTF8(u16name);
          PSZ pszName = (char*) sName.c_str();
          if ( strcasecmp( pszName, "tuid" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) ulSegNo = atol( pszValue );
            XMLString::release( &pszValue );
          } 
          else if ( strcasecmp( pszName, "datatype" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) strcpy( CurElement.szDataType, pszValue );
            XMLString::release( &pszValue );
          } 
          else if ( strcasecmp( pszName, "creationdate" ) == 0 )
          {
            char *pszDate = XMLString::transcode(attributes.getValue( i ));
            // we currently support only dates in the form YYYYMMDDThhmmssZ
            if ( (pszDate != NULL) && (strlen(pszDate) == 16) )
            {
              int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0, iDaysSince1970 = 0;

              // split string into date/time parts
              BOOL fOK = GetValue( pszDate, 4, &iYear );
              if ( fOK ) fOK = GetValue( pszDate + 4, 2, &iMonth );
              if ( fOK ) fOK = GetValue( pszDate + 6, 2, &iDay );
              if ( fOK ) fOK = GetValue( pszDate + 9, 2, &iHour );
              if ( fOK ) fOK = GetValue( pszDate + 11, 2, &iMin );
              if ( fOK ) fOK = GetValue( pszDate + 13, 2, &iSec );

              // convert date to number of days since 1.1.1970
              {
                iDaysSince1970 = GetYearDay( iDay, iMonth, iYear );
                for( int i = 1970; i < iYear; i++ )
                {
                  iDaysSince1970 += GetDaysOfYear( i );  
                } /* endfor */
                iDaysSince1970 -= 1;              // remove first day (1.1.1970)
              }

              // convert to a long value
              if ( fOK )
              {
                lTime =  iSec                      +
                        (iMin       *         60L) +
                        (iHour      *       3600L) +
                        ((iDaysSince1970) * 86400L);
                //LOG_DEVELOP_MSG << "Do we need to corect for OS/2 format?";
                lTime -= 10800L;                  // correction for OS/2 format: - 3 hours
              } /* endif */
            } /* endif */
            XMLString::release( &pszDate );
          } /* endif */
        } /* endfor */
        break;
      case TUV_ELEMENT:
        // reset language info, any <tuv> needs its own language setting
        CurElement.szTMXLanguage[0] = 0;
        CurElement.szTMLanguage[0] = 0;
        this->fInvalidChars = FALSE;
        this->fInlineTags = FALSE; 
        this->fTMXTagStarted = FALSE;
        this->fWithTMXTags = FALSE;
        this->fWithTagging = FALSE;
        // scan tuv attributes, currently only the language is of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          u16name = (char16_t*)attributes.getName( i );
          sName = EncodingHelper::convertToUTF8(u16name);
          PSZ pszName = (char*) sName.c_str();
          if ( (strcasecmp( pszName, "xml:lang" ) == 0) || (strcasecmp( pszName, "lang" ) == 0) )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            bool fOkLang = false;
            CHAR     _szLang[50];
            if ( pszValue != NULL )
            {
              strcpy( CurElement.szTMXLanguage, pszValue );
              strcpy( pBuf->szLang, pszValue );
              fOkLang = LanguageFactory::getInstance()->getOpenTM2NameFromISO( CurElement.szTMXLanguage, _szLang ) == 0;
            } /* endif */
            
            tagReplacer.activeSegment = SOURCE_SEGMENT;

            if(fOkLang){
              T5LOG( T5DEBUG) <<  ":: _szLang = " << _szLang << "; pBuf->szMemSourceLang = " << pBuf->szMemSourceLang;
              if( strcasecmp( _szLang, pBuf->szMemSourceLang ) ) {
                tagReplacer.activeSegment = TARGET_SEGMENT;
              }
            }else{
              T5LOG(T5ERROR) << ":: language is not supported";
            }

            XMLString::release( &pszValue );
          } /* endif */
        } /* endfor */

        // set fWithTagging switch depending on datatype or TM markup
        fWithTagging = FALSE;
        break;
      case BODY_ELEMENT:
        break;

      case TMX_SENTENCE_ELEMENT:
        //fTargetSegment = false;
      case SEG_ELEMENT:
        CurElement.fInlineTagging = FALSE;
        CurElement.fInsideTagging = FALSE;
        pBuf->szData[0] = 0; // reset data buffer
        pBuf->szNormalizedData[0] = 0;
        pBuf->szReplacedNpData[0] = 0;

        //reset enclosing tags flags
        pBuf->fSegmentStartsWithBPTTag = 0;

        fCatchData = TRUE;    
        break;
      case T5_N_ELEMENT:
      {
      //  T5LOG(T5ERROR) <<"found T5N_element, handling is not implemented yet";
      //  break;
      }
      case PH_ELEMENT:
      case X_ELEMENT:
      case UT_ELEMENT:
      case IT_ELEMENT:
      { 
        int iCurLen = wcslen(pBuf->szData);  

        std::string type;
        if( char16_t* _type = (char16_t*) attributes.getValue("type") ){
          type = EncodingHelper::toChar(_type);       
        }
        if(type == "lb"){
          wcscat(pBuf->szData, L"\n");
        }else{      
          auto tag = tagReplacer.GenerateReplacingTag(CurElement.ID , &attributes); 
          std::wstring replacingTag = tagReplacer.PrintTag(tag);
          
          if(fCreateNormalizedStr && tag.original_tagType == T5_N_ELEMENT){
            std::wstring NpTagFiller = tagReplacer.PrintReplaceTagWithKey(tag);
            wcscat(pBuf->szReplacedNpData, NpTagFiller.c_str());
            wcscat(pBuf->szNormalizedData, NpTagFiller.c_str());
          }

          if((iCurLen + replacingTag.size()) < DATABUFFERSIZE){
            if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
              T5LOG( T5DEBUG) << ":: parsed standalone tag \'<" << sName << ">\' , replaced with " << EncodingHelper::convertToUTF8(replacingTag);
            }
            wcscat(pBuf->szData, replacingTag.c_str());
            fCatchData = false;
            CurElement.fInsideTagging = true;
          }
        }
        break;
      }

      case BPT_ELEMENT:
      case BX_ELEMENT:
      {   
        auto tag = tagReplacer.GenerateReplacingTag(CurElement.ID, &attributes);
        //save tag to tag array

        std::wstring replacingTag = tagReplacer.PrintTag(tag);
        int iCurLen = wcslen(pBuf->szData);
        fCatchData = false;
        CurElement.fInsideTagging = true;
        if((iCurLen + replacingTag.size()) < DATABUFFERSIZE){         
          if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){       
            T5LOG( T5DEBUG) << ":: parsed bpt tag \'<" << sName << ">\' , replaced with " << EncodingHelper::convertToUTF8(replacingTag);
          }
          wcscat(pBuf->szData, replacingTag.c_str());
          if(fCreateNormalizedStr){
            wcscat(pBuf->szReplacedNpData, replacingTag.c_str());
          }
        }
        break;
      }
      case EPT_ELEMENT:
      case EX_ELEMENT:
      {
        std::u16string name;
        fCatchData = false;
        CurElement.fInsideTagging = true;
        
        auto tag = tagReplacer.GenerateReplacingTag(CurElement.ID, &attributes );//, -1 , -1); // tag->i);
        std::wstring replacedTag = tagReplacer.PrintTag(tag);
        int iCurLen = wcslen(pBuf->szData);
        if((iCurLen + replacedTag.size()) < DATABUFFERSIZE){
          if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){           
            T5LOG( T5DEBUG) << ":: parsed \'" << pszName << "\' tag \'<" << sName << ">\' , replaced with " << EncodingHelper::convertToUTF8(replacedTag);
          }
          wcscat(pBuf->szData, replacedTag.c_str());  
          if(fCreateNormalizedStr){
            wcscat(pBuf->szReplacedNpData, replacedTag.c_str());
          }    

        }else{
          T5LOG( T5WARNING) << ":: buffer size overflow for " << EncodingHelper::convertToUTF8(replacedTag);
        }
        break;
      }
      
      case HI_ELEMENT:
      case SUB_ELEMENT:
      case G_ELEMENT:
      {      
        auto tag = tagReplacer.GenerateReplacingTag(CurElement.ID, &attributes);//, -1, -1);
        std::wstring replacingTag = tagReplacer.PrintTag(tag);

        int iCurLen = wcslen(pBuf->szData);
        if((iCurLen + replacingTag.size()) < DATABUFFERSIZE){  
          if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){   
            T5LOG( T5DEBUG) << ":: parsed pair tag \'<" << sName  << ">\' , replaced with " << EncodingHelper::convertToUTF8(replacingTag);
          }
          wcscat(pBuf->szData, replacingTag.c_str());
          if(fCreateNormalizedStr){
            wcscat(pBuf->szReplacedNpData, replacingTag.c_str());
          }
        }else{
          T5LOG( T5WARNING) << ":: buffer size overflow for " << EncodingHelper::convertToUTF8(replacingTag);
        }
        break;
      }
      
      case INVCHAR_ELEMENT:
        // segment contains invalid data
        this->fInvalidChars = TRUE;
        break;
      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEVELOP)){
      for( int i = 0; i < iAttribs; i++ )
      {
        u16name = (char16_t*)attributes.getName( i );
        sName = EncodingHelper::convertToUTF8(u16name);

        u16name = (char16_t*)attributes.getValue( i );
        std::string sVal = EncodingHelper::convertToUTF8(u16name);
        T5LOG( T5DEVELOP)<< "Parsed atribute : "<< i << ", Name = " << sName <<  ", Value = "<<  sVal;
      } /* endfor */
    }
}

void TMXParseHandler::fillSegmentInfo
(
  TMXParseHandler::PTMXTUV pSource,
  TMXParseHandler::PTMXTUV pTarget,
  PMEMEXPIMPSEG    pSegment
)
{
  // fill-in header data
  memset( pSegment, 0, sizeof(MEMEXPIMPSEG) );
  if ( CurElement.lSegNum != 0 )
  {
    pSegment->lSegNum = CurElement.lSegNum; 
  }
  else
  {
    pSegment->lSegNum = (LONG)ulSegNo; 
  } /* endif */     

  if ( CurElement.szTMMarkup[0] )
  {
    strcpy( pSegment->szFormat, CurElement.szTMMarkup );
  }
  else if ( m_pMemInfo->pszMarkupList != NULL )
  {
    // do not set format, will be set later
  }
  else if ( m_pMemInfo->szFormat[0] != EOS )
  {
    // specified format overwrites info in datatype
    strcpy( pSegment->szFormat, m_pMemInfo->szFormat );
  }
  else if ( CurElement.szDataType[0] ) 
  {
    FindName(  Type2Markup, CurElement.szDataType, pSegment->szFormat, sizeof(pSegment->szFormat) );
  } /* endif */

  if ( pBuf->szDocument[0] )
  {
    strcpy( pSegment->szDocument, pBuf->szDocument );
  }
  else
  {
    strcpy( pSegment->szDocument, "none" );
  } /* endif */
  pSegment->usTranslationFlag = usTranslationFlag;
  if ( lTime )
  {
    pSegment->lTime = lTime;
  }
  else
  {
    time( &(pSegment->lTime) );
  } /* endif */

  // source info
  if ( pSource != NULL )
  {
    strcpy( pSegment->szSourceLang, pSource->szLang );
    wcsncpy( pSegment->szSource, pSource->szText, SEGDATABUFLEN );
  } /* endif */
  // target info
  if ( pTarget != NULL )
  {
    strcpy( pSegment->szTargetLang, pTarget->szLang );
    wcsncpy( pSegment->szTarget, pTarget->szText, SEGDATABUFLEN );
  } /* endif */

  // note style and text
  pSegment->szAddInfo[0] = 0;
  int iLeft = SEGDATABUFLEN;
  if ( (pBuf->szNote[0]!= 0) || (pBuf->szNoteStyle[0] != 0)  )
  {
    swprintf( pSegment->szAddInfo, iLeft, L"<Note style=\"%s\">%s</Note>", pBuf->szNoteStyle, pBuf->szNote );
    iLeft = SEGDATABUFLEN - wcslen(pSegment->szAddInfo);
  } /* endif */     

  // MT meta data
  if ( (pBuf->szMTMetrics[0] != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), iLeft, L"<MT%s></MT>", pBuf->szMTMetrics );
    iLeft = SEGDATABUFLEN - wcslen(pSegment->szAddInfo);
  } /* endif */     

  // word count info
  if ( (pBuf->ulWords != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), iLeft, L"<wcnt words=\"%lu\"></wcnt>", pBuf->ulWords );
    iLeft = SEGDATABUFLEN - wcslen(pSegment->szAddInfo);
  } /* endif */     

  // Match segment ID
  if ( (pBuf->szMatchSegID[0] != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), iLeft, L"<MatchSegID ID=\"%s\"/>", pBuf->szMatchSegID );
    iLeft = SEGDATABUFLEN - wcslen(pSegment->szAddInfo);
  } /* endif */     
}

void TMXParseHandler::endElement(const XMLCh* const name )
{
  std::u16string u16name((char16_t*)name);
  std::string sName = EncodingHelper::convertToUTF8(u16name);
  char* pszName = (char*) sName.c_str();

  T5LOG( T5DEBUG) << "endElement: " << pszName;
  ELEMENTID CurrentID = CurElement.ID;
  PROPID    CurrentProp = CurElement.PropID;
  
  try{
  switch ( CurrentID )
  {
    case TMX_ELEMENT:
      // end of data reached...
      {
        int iTUs = this->iNumOfTu;
      }
      break;
    case PROP_ELEMENT:
      // is processed after current element has been removed from stack...
      break;
    case HEADER_ELEMENT:
      fHeaderDone = TRUE;
      break;
    case TU_ELEMENT:
      // TU is complete, check collected data and add memory segment if everything is OK
      {
        int iCurrent = 0;
        PTMXTUV pSourceTuv = NULL;
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){              
          T5LOG( T5DEBUG) <<  "::parsed end of TU_ELEMENT::request tags: ";
          for(int i = 0; i < tagReplacer.requestTagList.size(); i++){
            T5LOG(T5DEVELOP) << TagReplacer::LogTag(tagReplacer.requestTagList[i]);
          }
          T5LOG( T5DEBUG) <<  "::parsed end of TU_ELEMENT:::: source tags: ";
          for(int i = 0; i < tagReplacer.sourceTagList.size(); i++){
            T5LOG(T5DEVELOP) << TagReplacer::LogTag(tagReplacer.sourceTagList[i]);
          }
          T5LOG( T5DEBUG) <<  "::parsed end of TU_ELEMENT:::: target tags: ";
          for(int i = 0; i < tagReplacer.targetTagList.size(); i++){
            T5LOG(T5DEVELOP) << TagReplacer::LogTag(tagReplacer.targetTagList[i]);
          }
        }

        // change markup table to XLIFF when segment data contains TMX inline tags
        //if ( this->fWithTMXTags )
        //{
          strcpy( CurElement.szTMMarkup, "OTMXUXLF" );
        //} /* endif */
        // find <tuv> containing the source data
        iCurrent = 0;
        PTMXTUV pCurrentTuv = pTuvArray;
        while ( (iCurrent < iCurTuv) && (pSourceTuv == NULL) )
        {
          bool isSourceTuv = false;
          if ( pBuf->fUseMajorLanguage ){
            short len1 = strlen(pCurrentTuv->szLang);
            short len2 = strlen(pBuf->szMemSourceLang);
            len1 = std::min(len1, len2);
            isSourceTuv = strncasecmp(pCurrentTuv->szLang, pBuf->szMemSourceLang,len1) == 0;

          }else{
            isSourceTuv = strcasecmp( pCurrentTuv->szLang, pBuf->szMemSourceLang ) == 0 ;
          }
          if ( isSourceTuv )
          { 
            pSourceTuv = pCurrentTuv;
          }
          else
          {
            pCurrentTuv++;
            iCurrent++;
          } /* endif */
        } /*endwhile */

        // loop over all other <tuv>s and add the pairs to the memory
        if ( pSourceTuv == NULL )
        {
          // no TUV matching the memory source language

          // add one invalid segment
          fillSegmentInfo( pTuvArray, NULL, &(pBuf->SegmentData) );
          pBuf->SegmentData.fValid = FALSE;
          strcpy( pBuf->SegmentData.szReason, "No unit matching the memory source language" );
          MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
        }
        else
        {
          iCurrent = 0;
          pCurrentTuv = pTuvArray;
          while ( iCurrent < iCurTuv )
          {
            BOOL fSkipTuv = FALSE;

            if ( pCurrentTuv == pSourceTuv )
            {
              fSkipTuv = TRUE; // skip this TUV as it contains the source text
            } /* endif */

            // GQ 2016-05-13 (fix for P403308): for empty TUVS only: check if there is another TUV for the same language and which contains some data and skip empty tuv in such a case
            if ( !fSkipTuv && (wcslen( pCurrentTuv->szText ) == 0) )
            {
              PTMXTUV pTestTuv = pCurrentTuv + 1;
              int iTestTuv = iCurrent + 1;
              while ( !fSkipTuv && (iTestTuv < iCurTuv) )
              {
                if ( (strcasecmp( pTestTuv->szLang, pCurrentTuv->szLang ) == 0) && (wcslen( pTestTuv->szText ) != 0) )
                {
                  fSkipTuv = TRUE; // skip this TUV as there is another tuv for the same language having text information
                } /* endif */
                iTestTuv++;
                pTestTuv++;
              } /* endwhile */
            } /* endif */

            if ( !fSkipTuv )
            {
              // do any tag removal if requested
              if ( m_pMemInfo->fCleanRTF )
              {
                RemoveRTFTags( pCurrentTuv->szText, m_pMemInfo->fTagsInCurlyBracesOnly );
                RemoveRTFTags( pSourceTuv->szText, m_pMemInfo->fTagsInCurlyBracesOnly );
              } /* endif */

              int iLenCurrent = wcslen( pCurrentTuv->szText );
              int iLenSource  = wcslen( pSourceTuv->szText );

              fillSegmentInfo( pSourceTuv, pCurrentTuv, &(pBuf->SegmentData) );

              if ( pCurrentTuv->fInvalidLang )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "The language \"%s\" is not supported by OpenTM2.", pCurrentTuv->szLang );
              }
              else if ( (iLenCurrent + 1) >= MAX_SEGMENT_SIZE)
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of target text is %ld characters.", iLenCurrent );
              }
              else if ( (iLenSource + 1) >= MAX_SEGMENT_SIZE )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of source text is %ld characters.", iLenSource );
              }
              else if ( pSourceTuv->fInvalidChars )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment source contains invalid characters." );
              }
              else if ( pCurrentTuv->fInvalidChars )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment target contains invalid characters." );
              }
              else
              {
                pBuf->SegmentData.fValid = TRUE;
              } /* endif */

              if(pBuf->SegmentData.fValid == FALSE){
                T5LOG( T5INFO) << ":: invalid segment, reason = " << pBuf->SegmentData.szReason;
              }
              // add segment to memory (if fValid set) or count as skipped segment
              //if ( MEMINSERTSEGMENT != NULL )
              {
                if ( pBuf->SegmentData.fValid )
                {
                  if ( pBuf->SegmentData.szFormat[0] != EOS ) 
                  {
                    // markup table information is already available
                    insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                  }
                  else if ( m_pMemInfo->pszMarkupList != NULL )
                  {
                    if ( pSourceTuv->fInlineTags )
                    {
                      // insert segment data for all specified markups
                      PSZ pszCurrentMarkup = m_pMemInfo->pszMarkupList;
                      while ( *pszCurrentMarkup )
                      {
                        strcpy( pBuf->SegmentData.szFormat, pszCurrentMarkup );
                        insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                        pszCurrentMarkup += strlen(pszCurrentMarkup) + 1;
                      } /* endwhile */                       
                    }
                    else
                    {
                      // segment contains no inline tagging so add it only once using the first markup of the list
                      strcpy( pBuf->SegmentData.szFormat, m_pMemInfo->pszMarkupList );
                      insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                    } /* endif */                       
                  }
                  else if ( m_pMemInfo->szFormat[0] != EOS )
                  {
                    // use supplied markup table for the segment
                    strcpy( pBuf->SegmentData.szFormat, m_pMemInfo->szFormat );
                    insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                  }
                  else
                  {
                    // use default markup table for the segment
                    strcpy( pBuf->SegmentData.szFormat, "OTMANSI" );

                    insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                  } /* endif */                     
                }
                else
                {
                   if ( pBuf->SegmentData.szFormat[0] == EOS ) strcpy( pBuf->SegmentData.szFormat, "OTMANSI" );

                  insertSegUsRC = MEMINSERTSEGMENT( lMemHandle, &(pBuf->SegmentData) );
                } /* endif */
              } /* endif */
            } /* endif */

            // continue with next tuv
            pCurrentTuv++;
            iCurrent++;
          } /*endwhile */
        } /* endif */
      }
      break;


    case TUV_ELEMENT:
      //
      // store collected data in TUV array
      //
      {
        // check if <tuv> contains all required info
        BOOL fTuvValid = TRUE;
        BOOL fTuvLangInvalid = FALSE;
   
        // GQ 2016/01/22 Allow empty data areas in order to allow conversion of memories with empty target segments
        // if ( pBuf->szData[0] == 0 ) fTuvValid = FALSE;          // no data for <tuv>
        
        // no Tmgr language available, check TMX language  
        if ( CurElement.szTMXLanguage[0] == 0 )
        {
          // no languag info at all, <tuv> is invalid
          fTuvValid = FALSE;
        }
        else
        {
          // convert TMX language to Tmgr language name
          LanguageFactory *pLangFactory = LanguageFactory::getInstance();
          
          if ( pLangFactory->getOpenTM2NameFromISO( CurElement.szTMXLanguage, pBuf->szLang ) != 0 )
          {
            fTuvLangInvalid = TRUE;          // language not supported
          } /* endif */
        } /* endif */
        
        // enlarge array if necessary
        if ( fTuvValid && (iCurTuv >= iTuvArraySize) )
        {
          int iNewArraySize = iCurTuv + 3;
          pTuvArray = (PTMXTUV)realloc( pTuvArray, iNewArraySize * sizeof(TMXTUV) );
          if ( pTuvArray )
          {
            iTuvArraySize = iNewArraySize;
          }
          else
          {
            // out of memory
          } /* endif */
        } /* endif */

        // store TUV data
        if ( fTuvValid )
        {
          PTMXTUV pTuvData = pTuvArray + iCurTuv;
          CopyTextAndProcessMaskedEntitites( pTuvData->szText, pBuf->szData );

          strcpy( pTuvData->szLang, pBuf->szLang );
          pTuvData->fInvalidChars = this->fInvalidChars;
          pTuvData->fInlineTags = this->fInlineTags;
          pTuvData->fInvalidLang = fTuvLangInvalid;
          if ( fTuvLangInvalid )
          {
            // remember specified ISO language
            strcpy( pTuvData->szLang, CurElement.szTMXLanguage );
          } /* endif */
          strcpy( pTuvArray->szIsoLang, CurElement.szTMXLanguage );

          iCurTuv++;
        } /* endif */
      }
      break;

    case BODY_ELEMENT:
      break;

    case SEG_ELEMENT:
    case TMX_SENTENCE_ELEMENT:
    {
      // data is complete, stop data catching
      CurElement.fInlineTagging = FALSE;
      fCatchData = FALSE;    
      std::string result = "pBuf is null"; 
      
      //check if we have enclosing tags and skip them if that's true
      if(pBuf && pBuf->szData)
      {
        bool inclosingTagsSkipped = false;
        std::wstring begPairTag, endPairTag;
        begPairTag = endPairTag = L"tag wasn't generated because not enought conditions are true for enclosing tags";
        {//filter out tag as start and end if it's in 
          //<bpt i='1'/>Here is <ph/> <bpt i='2'/>some<ept i='2'/> sentence<ept i='1'/> 
          //to
          //Here is <ph/> <bpt i='2'/>some<ept i='2'/> sentence 
          int len = wcslen(pBuf->szData);

          if(tagReplacer.sourceTagList.size()>=2){
            auto&& firstTag = tagReplacer.sourceTagList[0];
            auto&& lastTag  = tagReplacer.sourceTagList[tagReplacer.sourceTagList.size()-1];

            if(firstTag.generated_tagType == BPT_ELEMENT 
                && lastTag.generated_tagType == EPT_ELEMENT
                && firstTag.generated_i == lastTag.generated_i){

              begPairTag = tagReplacer.PrintTag(firstTag) ;
              endPairTag = tagReplacer.PrintTag(lastTag);            

              const size_t inclosingTagsLenSum = begPairTag.size() + endPairTag.size();

              if( len >= inclosingTagsLenSum ){
                wchar_t* segEnd = &(pBuf->szData[len-endPairTag.size()]);
                if(  wcsncmp(pBuf->szData, begPairTag.c_str(), begPairTag.size()) == 0 
                  && wcsncmp (segEnd,      endPairTag.c_str(), endPairTag.size()) == 0){
                    //skip begin and end tags
                    wcsncpy(pBuf->szData, &(pBuf->szData[begPairTag.size()]), len-(inclosingTagsLenSum));
                    pBuf->szData[len-(inclosingTagsLenSum)] = 0;
                     inclosingTagsSkipped = true;
                }
              }
            }
          }  
        }
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){      
          result = EncodingHelper::convertToUTF8(pBuf->szData);
          T5LOG( T5DEBUG) << ":: parsed end of TMX sentence, result = \'" << result << 
              "\'  ->  fSkipped inclosed tag pair = " << inclosingTagsSkipped <<";\n Generated tags first = \'" <<
              EncodingHelper::convertToUTF8(begPairTag) << "\'\nlast tag =\'" << EncodingHelper::convertToUTF8(endPairTag.c_str()), "\'\n";     
        }
      }
      
      break;  
    }
    case T5_N_ELEMENT:
    {
    //  T5LOG(T5ERROR) <<"found T5N_element, handling is not implemented yet";
    //  break;
    }
    case PH_ELEMENT:
    case X_ELEMENT:
    case UT_ELEMENT:
    case IT_ELEMENT:
    case BPT_ELEMENT:
    case EPT_ELEMENT:
    case BX_ELEMENT:   
    case EX_ELEMENT:
    {
      T5LOG( T5DEBUG) << ":: parsed end of tag \'" << sName << "\'";
      fCatchData = true;
      CurElement.fInsideTagging = false;
      CurElement.fInlineTagging = true;
      break;
    } 
    case HI_ELEMENT:
    case SUB_ELEMENT:
    case G_ELEMENT: 
    {        
      auto tag = tagReplacer.GenerateReplacingTag(CurElement.ID, nullptr);  
      auto replacingTagStr = tagReplacer.PrintTag(tag); 
      int iCurLen = wcslen(pBuf->szData);
      if((iCurLen + replacingTagStr.size()) < DATABUFFERSIZE){            
          T5LOG( T5DEBUG) << ":: parsed end pair tag \'</" << sName << ">\' , replaced with <ept i=\'" << "calculate i";
          wcscat(pBuf->szData, replacingTagStr.c_str());   
          if(fCreateNormalizedStr){
            wcscat(pBuf->szReplacedNpData, replacingTagStr.c_str());
          } 
        }else{
          T5LOG(T5ERROR) << ":: parsed end of pair tag \'" << sName << "\' replacing failed because no space left in buffer";         
        }  
      break;
    }
   
    case INVCHAR_ELEMENT:
      break;
    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */
  }catch(const xercesc::SAXException& exc){
    fatalInternalError(exc);
  }
  Pop( &CurElement );

  // for prop elements we have to set the data in the parent element (i.e. after the element has been removed from the stack)
  if ( CurrentID == PROP_ELEMENT )
  {
    //if ( CurrentProp == TMLANGUAGE_PROP )
    //{
    //  memset( CurElement.szTMLanguage, 0, sizeof(CurElement.szTMLanguage) );
    //  strncpy( CurElement.szTMLanguage, pBuf->szProp, sizeof(CurElement.szTMLanguage) - 1 );
    //}
    //else 
    if ( CurrentProp == TMMARKUP_PROP )
    {
      std::string buff = EncodingHelper::convertToUTF8(pBuf->szPropW);
      int size = std::min(buff.size(), sizeof(CurElement.szTMMarkup) - 1);
      memset( CurElement.szTMMarkup, 0, size + 1 );
      memcpy(CurElement.szTMMarkup, buff.c_str(), size);
      //strncpy( CurElement.szTMMarkup, buff.c_str(), size);
    }
    else if ( CurrentProp == TMDOCNAME_PROP )
    {
      std::string buff = EncodingHelper::convertToUTF8(pBuf->szPropW);
      int size = std::min(buff.size(), sizeof(pBuf->szDocument) - 1);
      memset( pBuf->szDocument, 0, size+1 );
      unsigned char s;
      memcpy( pBuf->szDocument, buff.c_str(), size);
      //strncpy( pBuf->szDocument, buff.c_str(), size );
    }
    else if ( CurrentProp == TMDESCRIPTION_PROP )
    {
      std::string buff = EncodingHelper::convertToUTF8(pBuf->szPropW);
      int size = std::min(buff.size(), sizeof(pBuf->szDescription) - 1);
      memset( pBuf->szDescription, 0, size+1 );
      memcpy( pBuf->szDescription, buff.c_str(), size);
      //strncpy( pBuf->szDescription, buff.c_str(), size);
    }
    else if ( CurrentProp == MACHINEFLAG_PROP )
    {
       usTranslationFlag = TRANSLFLAG_MACHINE;
    }
    else if ( CurrentProp == TRANSLATIONFLAG_PROP )
    {
      if ( pBuf->szPropW[0] == L'1' )
      {
        usTranslationFlag = TRANSLFLAG_MACHINE;
      }
      else if ( pBuf->szPropW[0] == L'2' )
      {
        usTranslationFlag = TRANSLFLAG_GLOBMEM;
      } /* end */         
    }
    else if ( CurrentProp == TMNOTE_PROP )
    {
      wcscpy( pBuf->szNote, pBuf->szPropW );
    }
    else if ( CurrentProp == TMNOTESTYLE_PROP )
    {
      wcscpy( pBuf->szNoteStyle, pBuf->szPropW );
    }
    else if ( CurrentProp == TMTMMATCHTYPE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, TMMATCHTYPE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTSERVICE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTSERVICE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTMETRICNAME_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTMETRICNAME_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTMETRICVALUE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTMETRICVALUE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMPEEDITDISTANCECHARS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, PEEDITDISTANCECHARS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMPEEDITDISTANCEWORDS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, PEEDITDISTANCEWORDS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTFIELDS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTFIELDS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMWORDS_PROP )
    {
      pBuf->ulWords = std::wcstol( pBuf->szPropW, nullptr, 10 );
    }
    else if ( CurrentProp == TMMATCHSEGID_PROP )
    {
      wcscpy( pBuf->szMatchSegID, pBuf->szPropW );
    }
    else if ( CurrentProp == SEG_PROP )
    {
      CurElement.lSegNum = std::wcstoull( pBuf->szPropW,  nullptr, 10 );
    } /* endif */
  } /* endif */
}

void TMXParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  char16_t* pzChars = (char16_t*)chars;
  std::string sChars = EncodingHelper::convertToUTF8(pzChars);
  std::wstring wChars = EncodingHelper::convertToUTF16(sChars.c_str());
  char* c_chars = (char*) sChars.c_str();
  wchar_t* w_chars = (wchar_t*) wChars.c_str();
  int iLength = length;
  int iwLength = wChars.length();

  if ( this->fTMXTagStarted )
  {
    // close open TMX inline tag
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + 2) < DATABUFFERSIZE)
    {
      pBuf->szData[iCurLen++] = '>';
      pBuf->szData[iCurLen] = 0;
    } /* endif */
    this->fTMXTagStarted = FALSE;
  } /* endif */

  T5LOG( T5DEVELOP)<< "TMXParseHandler::characters " <<length << " characters; " << c_chars ;

  if ( (CurElement.ID == PROP_ELEMENT) && (CurElement.PropID != UNKNOWN_PROP) )
  {
    // append data in prop value buffer
    int iwCurLength = wcslen( pBuf->szPropW );
    int iwFree = (sizeof(pBuf->szPropW) / sizeof(CHAR_W)) - iwCurLength - 1;
    if ( iwLength > iwFree ) 
      iwLength = iwFree;

    wcsncpy( pBuf->szPropW + iwCurLength, w_chars, iwLength );
    pBuf->szPropW[iwCurLength+iwLength] = 0;
  }
  else if ( fCatchData && (CurElement.ID == INVCHAR_ELEMENT) )
  {
    // add invalid char
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + 1 + 1) < DATABUFFERSIZE)
    {
      LONG lChar = _wtol( w_chars );
      if ( lChar != 0 )
      {
        USHORT usChar = (USHORT)lChar;
        pBuf->szData[iCurLen] = usChar;
        pBuf->szData[iCurLen+1] = 0;
      } /* endif */
    } /* endif */
  }
  else if ( fCatchData && (fWithTagging || !CurElement.fInlineTagging) )
  {
    // add data to current data buffer 
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + length + 1) < DATABUFFERSIZE)
    {
      // escape any special characters in inline tags
      EscapeXMLChars( w_chars, pBuf->szData + iCurLen );  
      if(fCreateNormalizedStr){
        wcscat(pBuf->szReplacedNpData, pBuf->szData + iCurLen);
        wcscat(pBuf->szNormalizedData, pBuf->szData + iCurLen);
      }    
    } /* endif */
    if ( CurElement.fInlineTagging ) 
        this->fInlineTags = TRUE;
  } /* endif */
}

void TMXParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if(strncmp(message,"invalid character", std::min(strlen(message), strlen("invalid character")))){
      this->fError = TRUE;
      
      T5LOG(T5ERROR) <<  "Fatal Error: " << message <<" at column " <<  col <<" in line "<< line<<"\n";
      sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
      if(pImportDetails){
        pImportDetails->importMsg << "IMPORT: " << this->pBuf->szErrorMessage << "; \n";
      }
    }else{
      if(pImportDetails){
        pImportDetails->importMsg << "IMPORT: INVCHAR: " << message <<" at column " <<  col <<" in line "<< line<<"; \n";
      }
      
      T5LOG(T5WARNING) <<  "IMPORT: INVCHAR: " << message <<" at column " <<  col <<" in line "<< line<<"\n";
      resetErrors(); 
      _invalidCharacterErrorCount++;
    }
    
    XMLString::release( &message );
}


void TMXParseHandler::setDocumentLocator(const Locator* const locator){
  m_locator = locator ;
}

#include <xercesc/sax/Locator.hpp>
void TMXParseHandler::fatalInternalError(const SAXException& exception)
{
  char* message = XMLString::transcode(exception.getMessage());
  std::string msg = message; 
  XMLString::release( &message );
  //if(m_locator){
  //  fatalError(xercesc::SAXParseException(msg.c_str(), m_locator));
  //}else
  {
    long line = 0;//(long)exception.getLineNumber();
    long col = 0; //(long)exception.getColumnNumber();
    if(m_locator){
      line = m_locator->getLineNumber();
      col = m_locator->getColumnNumber();
    }
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, " Fatal internal Error at column %ld in line %ld, import stopped at progress = %i%, errorMsg: %s ", 
            col, line, (int)pImportDetails->usProgress, msg.c_str() );
    msg =  std::string(pBuf->szErrorMessage);
    if(pImportDetails){
      pImportDetails->importMsg << msg;
    }
    
  }
}


void TMXParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Error: %s at column %ld in line %ld", message, col, line );
    if(pImportDetails){
      pImportDetails->importMsg << "IMPORT: " << this->pBuf->szErrorMessage << "; \n";
    }
    T5LOG(T5ERROR) << ": "<< this->pBuf->szErrorMessage;
    XMLString::release( &message );
}

void TMXParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    T5LOG(T5WARNING) << "Warning: "<< message << " at column "<< col << " in line " << line << "\n";
    if(pImportDetails){
        pImportDetails->importMsg <<  "Warning: "<< message << " at column "<< col << " in line " << line << "; \n";
    }
    XMLString::release( &message );
}


// get the ID for a TMX element
ELEMENTID TMXParseHandler::GetElementID( PSZ pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  while ( (IDFound == UNKNOWN_ELEMENT) && (TmxNameToID[i].szName[0] != 0) )
  {
    //if ( _wcsicmp( pszName, TmxNameToID[i].szName ) == 0 )
    if( strcasecmp(pszName, TmxNameToID[i].szName ) == 0)
    {
      IDFound = TmxNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method TMXParseHandler::GetElementID */

void TMXParseHandler::Push( PTMXELEMENT pElement )
{
  // enlarge stack if necessary
  if ( iCurElement >= iStackSize )
  {
    pStack = (PTMXELEMENT)realloc( pStack, (iStackSize + 5) * sizeof(TMXELEMENT) );
    iStackSize += 5;
  } /* endif */

  // add element to stack
  if ( pStack )
  {
    memcpy( pStack + iCurElement, pElement, sizeof(TMXELEMENT) );
    iCurElement++;
  } /* endif */

  return;
} /* end of method TMXParseHandler::Push */

void TMXParseHandler::Pop( PTMXELEMENT pElement )
{
  if ( pStack && iCurElement )
  {
    iCurElement--;
    memcpy( pElement, pStack + iCurElement, sizeof(TMXELEMENT) );
  } /* endif */
  return;
} /* end of method TMXParseHandler::Pop */


void TMXParseHandler::SetMemInfo( PMEMEXPIMPINFO pMemInfo)
{
  this->m_pMemInfo = pMemInfo;
} /* end of method TMXParseHandler::SetNameLists */

// extract a numeric value from a string
BOOL TMXParseHandler::GetValue( PSZ pszString, int iLen, int *piResult )
{
  BOOL fOK = TRUE;
  char szNumber[10];
  char *pszNumber = szNumber;

  *piResult = 0;

  while ( iLen && fOK )
  {
    if ( isdigit(*pszString) )
    {
      *pszNumber++ = *pszString++;
      iLen--;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fOK )
  {
    *pszNumber = '\0';
    *piResult = atoi( szNumber );
  } /* endif */

  return( fOK );
} /* end of method TMXParseHandler::GeValue */

// convert a TMX language name to an TM language name
BOOL TMXParseHandler::TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage )
{
  BOOL fOK = TRUE;

  // preset target buffer
  strcpy( pszResultingLanguage, OTHERLANGUAGES );

  if ( *pszTMLanguage )
  {
    strcpy( pszResultingLanguage, pszTMLanguage );
  }
  else if ( *pszTMXLanguage )
  {
    LanguageFactory *pLangFactory = LanguageFactory::getInstance();
    pLangFactory->getOpenTM2NameFromISO( pszTMXLanguage, pszResultingLanguage );
  } /* endif */

  return( fOK );
} /* end of method TMXParseHandler::GeValue */

void TMXParseHandler::SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsert, LONG lHandle, PLOADEDTABLE pTable, PTOKENENTRY pBuf, int iSize )
{
  pfnInsertSegment = pfnInsert;
  lMemHandle = lHandle;
  pRTFTable = pTable;
  pTokBuf = pTokBuf;
  iTokBufSize = iSize;
} /* end of method TMXParseHandler::SetMemInterface */

void TMXParseHandler::GetDescription( char *pszDescription, int iBufSize )
{
  *pszDescription = 0;

  if ( pBuf && pBuf->szDescription[0] )
  {
    strncpy( pszDescription, pBuf->szDescription, iBufSize );
    pszDescription[iBufSize-1] = 0;
  } /* endif */
} /* end of method TMXParseHandler::GetDescription */

void TMXParseHandler::SetSourceLanguage( char *pszSourceLang)
{
  if ( pBuf )
  {
    strcpy( pBuf->szMemSourceLang, pszSourceLang);
    pBuf->fUseMajorLanguage = (LanguageFactory::getInstance()->findIfPreferedLanguage(pszSourceLang) != -1 );
  } /* endif */
} /* end of method TMXParseHandler::SetSourceLanguage */

void TMXParseHandler::GetSourceLanguage( char *pszSourceLang, int iBufSize )
{
  *pszSourceLang = 0;

  if ( pBuf && pBuf->szMemSourceLang[0] )
  {
    strncpy( pszSourceLang, pBuf->szMemSourceLang, iBufSize );
    pszSourceLang[iBufSize-1] = 0;
  } /* endif */
} /* end of method TMXParseHandler::GetSourceLanguage */

BOOL TMXParseHandler::IsHeaderDone()
{
 return( fHeaderDone );
}

// help functions for date conversion
BOOL TMXParseHandler::IsLeapYear( const int iYear )
{
  if ((iYear % 400) == 0)
    return true;
  else if ((iYear % 100) == 0)
    return false;
  else if ((iYear % 4) == 0)
    return true;
  return false;
}                   

int TMXParseHandler::GetDaysOfMonth( const int iMonth, const int iYear )
{
  int aiDaysOfMonth[13] = {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (iMonth == 2)
  {
    if ( IsLeapYear( iYear ) )
      return 29;
    else
      return 28;
  }
  if (( iMonth >= 1) && (iMonth <= 12))
    return aiDaysOfMonth[iMonth];
  else
  {
    return 0;
  }
}                   

int TMXParseHandler::GetDaysOfYear( const int iYear )
{
  if ( IsLeapYear( iYear ) )
  {
    return 366;
  }
  else
  {
    return 365;
  } /* endif */
}                   

int TMXParseHandler::GetYearDay( const int iDay, const int iMonth, const int iYear )
{
  int iLocalDay = iDay;
  int iLocalMonth = iMonth;

  while ( iLocalMonth > 1)
  {
    iLocalMonth--;
    iLocalDay += GetDaysOfMonth( iLocalMonth, iYear );
  }
  return iLocalDay ;
}                   

BOOL TMXParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void TMXParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->pBuf != NULL )
  {
    if ( this->pBuf->szErrorMessage[0] != '\0' )
    {
      strncpy( pszTextBuffer, this->pBuf->szErrorMessage, iBufSize );
      pszTextBuffer[iBufSize-1] = '\0';
    } /* endif */
  } /* endif */
}

USHORT TMXParseHandler::RemoveRTFTags( PSZ_W pszString, BOOL fTagsInCurlyBracesOnly  )
{
  USHORT usRC = 0;
  PSZ_W  pszOut = pszString;
  BOOL fTagRemoveMode = !fTagsInCurlyBracesOnly;

  while ( *pszString )
  {
    if ( *pszString == L'\\' )
    {
      if ( *(pszString+1) == L'\\' )
      {
        // encoded backslash, convert to a single one
        pszString++;
        *pszOut++ = *pszString++;
      }
      if ( (*(pszString+1) == L'{') || (*(pszString+1) == L'}') )
      {
        // encoded curly brace, convert to a normal one
        pszString++;
        *pszOut++ = *pszString++;
      }
      else if ( iswalpha(*(pszString+1))  || (*(pszString+1) == L'*') )
      {
        if ( fTagRemoveMode  )
        {
        // skip RTF tag
        pszString++;
        if ( *pszString == L'*' ) pszString++;
        if ( *pszString == L'\\' ) pszString++;
        while ( iswalpha( *pszString ) ) pszString++;
        if ( *pszString == L'-') pszString++;
        while ( iswdigit( *pszString ) ) pszString++;
        if ( *pszString == L' ') pszString++;
      }
      else
      {
        // copy backslash to target
        *pszOut++ = *pszString++;
      } /* endif */
    }
      else
      {
        // copy backslash to target
        *pszOut++ = *pszString++;
      } /* endif */
    }
    else if ( *pszString == L'{' )
    {
      // skip curly brace, but leave parameters ( {n} ) as-is
      if ( (*pszString == L'{') && iswdigit(pszString[1]) && (pszString[2] == L'}')   )
      {
        *pszOut++ = *pszString++;
        *pszOut++ = *pszString++;
        *pszOut++ = *pszString++;
      }
      else
      {
        if ( fTagsInCurlyBracesOnly )
        {
          // check if curly brace is followed by RTF tags
          PSZ_W pszTest = pszString + 1;
          while (*pszTest == L' ') pszTest++;
          if ( (*pszTest == L'\\') && (iswalpha(*(pszTest+1))  || (*(pszTest+1) == L'*') ) )
          {
            // skip curly brace and start tag remove mode
            pszString++;
            fTagRemoveMode = TRUE;
          }
          else
          {
            // leave curly brace as-is
            *pszOut++ = *pszString++;
          } /* endif */           
        }
        else
        {
          // skip curly brace
          pszString++;
        } /* endif */           
      } /* endif */         
    }
    else if ( *pszString == L'}' )
    {
      if ( fTagRemoveMode  )
      {
        // skip closing curly brace
        pszString++;
      }
      else
      {
        // copy closing curly brace to target
        *pszOut++ = *pszString++;
      } /* endif */         
      if ( fTagsInCurlyBracesOnly )
      {
        fTagRemoveMode = FALSE;
      } /* endif */         
    }
    else
    {
      *pszOut++ = *pszString++;
    } /* endif */
  }/* endwhile */
  *pszOut = 0;
  return( usRC );
} /* end of function RemoveRTFTags */


//+----------------------------------------------------------------------------+
//| EXP2TMX and TMX2EXP converter                                              |
//+----------------------------------------------------------------------------+
#define BUF_SIZE 8096
#define TMXTOKBUFSIZE 32000

typedef struct _CONVERTERDATA
{
  MEMEXPIMPSEG     Segment;                      // segment data buffer
  MEMEXPIMPINFO    MemInfo;                      // mem import export data area
  FILE             *hfOut;                       // output file handle
  CHAR_W           szBufferW[16200];             // buffer for UTF6 strings
  CHAR             szASCIIBuffer[16200];         // buffer for UTF6 to ANSI/ASCII conversions
  BOOL             fHeaderWritten;               //TRUE = header has written to the memory
  LONG             lSegCounter;                  // valid segment counter
  LONG             lSkippedSegsCounter;          // invalid segment counter
  CHAR             szLogFile[MAX_LONGFILESPEC];  // buffer for log file name

  //-- data for input memory processing --
  CHAR        szInSpec[MAX_LONGFILESPEC];        // input memory specification of user
  CHAR        szInputFile[MAX_LONGFILESPEC];     // buffer for preperation of input memory name
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name current input memory
  CHAR        szBuffer[4096];                    // general purpose buffer
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  FILE        *hInFile;                          // handle of input file
  CHAR_W      szLine[4096];                      // buffer for input line
  CHAR_W      szSegBuffer[32000];                // buffer for EXP segment (from <segment ..> up to </segment>)
  CHAR_W      chInBufW[BUF_SIZE];                // data buffer for read of Unicode data
  CHAR        chInBuf[BUF_SIZE];                 // data buffer for read of ANSI data
  int         iInBufProcessed;                   // number of processed characters in chInBuf
  int         iInBufRead;                        // number characters read into chInBuf
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  BOOL        fAnsi;                             // TRUE = input in ASCII format / FALSE = input in ASCII mode
  CHAR        szInMode[200];                     // buffer for input mode
  CHAR_W      szDescription[1024];               // buffer for memory description
  int         iLineNum;                          // current line number
  int         iSegBufferUsed;                    // number of characters used in szSegBuffer
  int         iSegBufferFree;                    // number of character free in szSegBuffer
  BOOL        fSegBufferFull;                    // TRUE = the segment buffer is full

  //-- data for output memory processing --
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  BOOL        fUTF16Output;                      // TRUE = UTF16, FALSE = UTF8 

  //-- segment data --
  CHAR_W      szSegStart[1024];                  // buffer for segment start string
  CHAR_W      szControl[1024];                   // buffer for control string
  CHAR        szControlAscii[1024];              // ASCII version of control string
  USHORT      usTranslationFlag;                 // type of translation flag
  CHAR_W      szDataType[100];                   // datatype of segment

  // - statistics --
  ULONG       ulSegments;                        // number of segments written to output memory

  // -- other stuff --
  //PLOADEDTABLE pLoadedTable;                     // pointer to currently loaded markup table
  //PTOKENENTRY  pTokBuf;                          // buffer for TaTagTokenize tokens
  //CHAR_W       szActiveTagTable[100];            // buffer for name of loaded tag table
  CHAR           szErrorText[2048];                // error message text in case of errors
  //BOOL         fLog;                             // true = write messages to log file
} CONVERTERDATA, *PCONVERTERDATA;

USHORT WriteStringToMemory( PCONVERTERDATA pData, PSZ_W pszString );
USHORT WriteMemHeader( PCONVERTERDATA pData);
USHORT WriteMemFooter( PCONVERTERDATA pData );

USHORT CheckOutputName( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT CheckExistence( PCONVERTERDATA pData, PSZ pszInMemory );
USHORT GetEncoding( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode );
USHORT GetEncodingEx( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode, PSZ pszSourceLanguage );
USHORT GetNextSegment( PCONVERTERDATA pData, PBOOL pfSegmentAvailable );
USHORT CloseInput( PCONVERTERDATA pData );
USHORT FillBufferW( PCONVERTERDATA pData );
USHORT FillBuffer( PCONVERTERDATA pData );
USHORT ReadLineW( PCONVERTERDATA pData, PSZ_W pszLine, int iSize );
USHORT ReadLine( PCONVERTERDATA pData, PSZ pszLine, int iSize );
USHORT GetLine( PCONVERTERDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF );
PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId );
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag );



// write segment to output file
USHORT APIENTRY WRITEEXPSEGMENT( LONG lMemHandle, PMEMEXPIMPSEG pSegment )
{
  USHORT           usRC = 0;           // function return code
  PCONVERTERDATA   pData = (PCONVERTERDATA)lMemHandle;

  if ( pSegment->fValid )
  {
    if ( !pData->fHeaderWritten )
    {
      WriteMemHeader( pData );
    } /* endif */
    pData->lSegCounter++;
     T5LOG( T5WARNING) << "TEMPORARY_COMMENTED";
    //swprintf( pData->szBufferW, L"<Segment>%10.10ld\r\n", pData->lSegCounter );
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"<Control>\r\n" );
    
     T5LOG(T5ERROR) << "TEMPORARY_COMMENTED WRITEEXPSEGMENT";
    /*swprintf( pData->szBufferW, L"%06ld%s%1.1u%s%016lu%s%S%s%S%s%S%s%S%s%s%s%S",
      pSegment->lSegNum, X15_STRW, pSegment->usTranslationFlag, X15_STRW,
      pSegment->lTime, X15_STRW, pSegment->szSourceLang, X15_STRW, 
      (pData->MemInfo.fSourceSource) ? pSegment->szSourceLang : pSegment->szTargetLang, X15_STRW,
      pSegment->szAuthor, X15_STRW, pSegment->szFormat, X15_STRW, L"na", X15_STRW, pSegment->szDocument );//*/
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"\r\n</Control>\r\n" );
    if ( pSegment->szAddInfo[0] != 0 )
    {
      WriteStringToMemory( pData, L"<AddData>" );
      WriteStringToMemory( pData, pSegment->szAddInfo );
      WriteStringToMemory( pData, L"</AddData>\r\n" );
    } /* endif */       
    WriteStringToMemory( pData, L"<Source>" );
    WriteStringToMemory( pData, pSegment->szSource );
    WriteStringToMemory( pData, L"</Source>\r\n" );
    WriteStringToMemory( pData, L"<Target>" );
    if ( pData->MemInfo.fSourceSource )
    {
      WriteStringToMemory( pData, pSegment->szSource );
    }
    else
    {
      WriteStringToMemory( pData, pSegment->szTarget );
    } /* endif */
    WriteStringToMemory( pData, L"</Target>\r\n" );
    WriteStringToMemory( pData, L"</Segment>\r\n" );
  }
  else
  {
    //if ( pData->hfLog )
    {
      T5LOG(T5INFO) <<"Segment "<< pSegment->lSegNum <<" skipped, reason = " << pSegment->szReason   <<"\nSource=" << pSegment->szSource;
    } /* endif */
    pData->lSkippedSegsCounter++;
  } /* endif */

  return( usRC );
}

// helper function: is character a list delimiter or space
static BOOL isDelimiter( CHAR chTest )
{
 return ( (chTest == '(') || (chTest == ')') || (chTest == ',') || (chTest == ' ') || (chTest == EOS)  );
}


// write string to output memory
USHORT WriteStringToMemory
( 
  PCONVERTERDATA   pData,
  PSZ_W            pszString
)
{
  if ( pData->MemInfo.lOutMode & ASCII_OPT )
  {
    WideCharToMultiByte( CP_OEMCP, 0, pszString, -1, pData->szASCIIBuffer, sizeof(pData->szASCIIBuffer), NULL, NULL );
    fwrite( pData->szASCIIBuffer, strlen(pData->szASCIIBuffer), 1, pData->hfOut );
  }
  else if ( pData->MemInfo.lOutMode & ANSI_OPT )
  {
    WideCharToMultiByte( CP_ACP, 0, pszString, -1, pData->szASCIIBuffer, sizeof(pData->szASCIIBuffer), NULL, NULL );
    fwrite( pData->szASCIIBuffer, strlen(pData->szASCIIBuffer), 1, pData->hfOut );
  }
  else
  {
    fwrite( pszString, wcslen(pszString), sizeof(CHAR_W), pData->hfOut );
  } /* endif */
  return( 0 );
} /* end of function WriteStringToMemory */

USHORT WriteMemHeader( PCONVERTERDATA pData )
{
  if ( !pData->fHeaderWritten )
  {
    WriteStringToMemory( pData, L"<NTMMemoryDb>\r\n" );
    WriteStringToMemory( pData, L"<Description>\r\n" );
    MultiByteToWideChar( CP_ACP, 0, pData->MemInfo.szDescription, -1, pData->szBufferW, sizeof(pData->szBufferW)/sizeof(CHAR_W) );
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"\r\n</Description>\r\n" );
    WriteStringToMemory( pData, L"<Codepage>" );
    if ( pData->MemInfo.lOutMode & ANSI_OPT )
    {
      WriteStringToMemory( pData, L"1" );
    }
    else if ( pData->MemInfo.lOutMode & ASCII_OPT )
    {
      WriteStringToMemory( pData, L"1" );
    }
    else
    {
      WriteStringToMemory( pData, L"UTF16" );
    } /* endif */
    WriteStringToMemory( pData, L"</Codepage>\r\n" );
    pData->fHeaderWritten = TRUE;
  } /* endif */
  return( 0 );
} /* end of function WriteMemHeader */

USHORT WriteMemFooter( PCONVERTERDATA pData )
{
  WriteStringToMemory( pData, L"</NTMMemoryDb>\r\n" );
  return( 0 );
} /* end of function WriteMemHeader */


// check output memory name 
USHORT CheckOutputName( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszOutMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code

  if ( *pszOutMemory == EOS )
  {
    PSZ pszExtension;

    strcpy( pData->szOutMemory, pszInMemory );

    pszExtension = strrchr( pData->szOutMemory, '\\' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory;
    pszExtension = strrchr( pszExtension, '.' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory + strlen(pData->szOutMemory);
    strcpy( pszExtension, ".TMX" );
  }
  else
  {
    strcpy( pData->szOutMemory, pszOutMemory );
  } /* endif */

  return( usRC );
} /* end of function */


  // check existence of input file
USHORT CheckExistence( PCONVERTERDATA pData, PSZ pszInMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code
  HDIR hdir;
  static WIN32_FIND_DATA FindData;    // find data structure

  LOG_UNIMPLEMENTED_FUNCTION;
#ifdef TEMPORARY_COMMENTED
  hdir = FindFirstFile( pszInMemory, &FindData );
  if ( hdir == INVALID_HANDLE_VALUE )
  {
     LOG_AND_SET_RC(usRC, T5INFO, ERROR_FILE_NOT_FOUND);
     sprintf( pData->szErrorText, "Input memory %s could not be opened", pszInMemory );
  }
  else
  {
    FindClose( hdir );
  } /* endif */
  #endif

  return( usRC );
} /* end of function CheckExistence */

// open input memory and get input memory encoding
USHORT GetEncoding( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode )
{
  return( GetEncodingEx( pData, pszInMemory, pszInMode, NULL ) );
} /* end of function GetEncoding */

// get memory source language (UTF16 Version)
USHORT GetSourceLangW( PCONVERTERDATA pData, PSZ pszSourceLanguage )
{
  BOOL fWaitingForControlRecord = TRUE;
  fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );
  while ( fWaitingForControlRecord  && !feof(pData->hInFile) )
  {
    if ( wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
    {
      // get control data
      fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );

      // retrieve source language
      wcscpy( pData->szBufferW, ParseX15W( pData->szLine, 3 ) );
      WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pszSourceLanguage, MAX_LANG_LENGTH, NULL, NULL );

      fWaitingForControlRecord = FALSE;
    }
    else
    {
      fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );
    } /* endif */               
  } /* endwhile */           
  return( 0 );
} /* end of function GetSourceLangW */

// get memory source language 
USHORT GetSourceLang( PCONVERTERDATA pData, PSZ pszSourceLanguage )
{
  BOOL fWaitingForControlRecord = TRUE;
  fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );
  while ( fWaitingForControlRecord  && !feof(pData->hInFile) )
  {
    if ( strnicmp( (PSZ)pData->szLine, "<control>", 9 ) == 0 )
    {
      // get control data
      fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );

      // retrieve source language
      T5LOG(T5ERROR) << "TEMPORARY_COMMENTED GetSourceLangW";
      //strcpy( pszSourceLanguage, UtlParseX15( (PSZ)pData->szLine, 3 ) );

      fWaitingForControlRecord = FALSE;
    }
    else
    {
      fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );
    } /* endif */               
  } /* endwhile */           
  return( 0 );
} /* end of function GetSourceLang */



// open input memory and get input memory encoding and source language
USHORT GetEncodingEx( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode, PSZ pszSourceLanguage )
{
  USHORT      usRC = NO_ERROR;         // function return code

  pszInMode;

  strcpy( pData->szInMemory, pszInMemory );
  pData->hInFile = fopen( pData->szInMemory, "rb" );
  if ( pData->hInFile == NULL )
  {
     LOG_AND_SET_RC(usRC, T5INFO, ERROR_FILE_NOT_FOUND);
     // ShowError( pData, "Error: Input memory %s could not be opened", pszInMemory );
  } /* endif */

  // get first bytes of memory and check for UTF-16 encoding
  if ( usRC == NO_ERROR )
  {
    // get first chunk of file and check for BOM and memory tag
    memset( pData->szBuffer, 0, sizeof(pData->szBuffer) );
    fread( pData->szBuffer, 1, sizeof(pData->szBuffer), pData->hInFile );

    if ( memcmp( pData->szBuffer, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      PSZ_W pszTemp = (PSZ_W)pData->szBuffer + 1;
      pData->fUnicode = TRUE;

      if ( _wcsnicmp( pszTemp, L"<ntmmemorydb>", 13 ) != 0 )
      {
        sprintf( pData->szErrorText, "The format of the file %s is not supported", pData->szInMemory );
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_DATA);
      } /* endif */

      // read ahead up to first control entry
      if ( usRC == 0 )
      {
        fseek( pData->hInFile, 2, SEEK_SET );
        if ( pszSourceLanguage != NULL ) GetSourceLangW( pData, pszSourceLanguage );
      } /* endif */         

      fseek( pData->hInFile, 2, SEEK_SET );

      usRC = FillBufferW( pData );
    }
    else
    {
      PSZ_W pszTemp = (PSZ_W)pData->szBuffer;

      fseek( pData->hInFile, 0, SEEK_SET );

      if ( _wcsnicmp( pszTemp, L"<ntmmemorydb>", 13 ) == 0 )
      {
        pData->fUnicode = TRUE;
        if ( pszSourceLanguage != NULL )
        {
          GetSourceLangW( pData, pszSourceLanguage );
          fseek( pData->hInFile, 0, SEEK_SET );
        } /* endif */         
       usRC = FillBufferW( pData );
      }
      else if ( _strnicmp( pData->szBuffer, "<ntmmemorydb>", 13 ) == 0 )
      {
        pData->fUnicode = FALSE;
        if ( pszSourceLanguage != NULL )
        {
          GetSourceLang( pData, pszSourceLanguage );
          fseek( pData->hInFile, 0, SEEK_SET );
        } /* endif */         
        usRC = FillBuffer( pData );
      }
      else
      {
        sprintf( pData->szErrorText, "The file %s is no memory in valid EXP format", pData->szInMemory );
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_DATA);
      } /* endif */
    } /* endif */
  } /* endif */

  // read memory header up to first segment and check for codepage tag, store any found memory description
  if ( usRC == NO_ERROR )
  {
    BOOL fEOF = FALSE;
    BOOL fDescription = FALSE;

    pData->szLine[0] = 0;
    pData->szDescription[0] = 0;
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    while ( (usRC == NO_ERROR) && !fEOF && 
            (_wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0) &&
            (_wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) != 0))
    {
      if ( _wcsnicmp( pData->szLine, L"<description>", 13 ) == 0 )
      {
        fDescription = TRUE;
      }
      else if ( _wcsnicmp( pData->szLine, L"</description>", 14 ) == 0 )
      {
        fDescription = FALSE;
      }
      else if ( _wcsnicmp( pData->szLine, L"<codepage>", 10 ) == 0 )
      {
        StripTag( pData->szLine + 10, L"</codepage>" );
        if ( _wcsnicmp( pData->szLine + 10, L"ANSI=", 5 ) == 0 ) 
        {
          pData->ulInputCP = _wtol( pData->szLine + 15 );
        }
        else if ( _wcsnicmp( pData->szLine + 10, L"ASCII=", 6 ) == 0 ) 
        {
          pData->ulInputCP = _wtol( pData->szLine + 16 );
        } /* endif */
      } 
      else if ( fDescription )
      {
        wcscat( pData->szDescription, pData->szLine );
      } /* endif */

      pData->szLine[0] = 0;
      usRC = GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
    } /*endwhile */
  } /* endif */

  return( usRC );
} /* end of function GetEncodingEx */

// clear the segment buffer
void ClearSegBuffer( PCONVERTERDATA pData )
{
  pData->iSegBufferUsed = 0;
  pData->iSegBufferFree = sizeof(pData->szSegBuffer)/sizeof(CHAR_W);
  pData->fSegBufferFull = FALSE;
}

// add the current line to the segment buffer
void AddLineToSegBuffer( PCONVERTERDATA pData )
{
  if ( !pData->fSegBufferFull )
  {
    int iAddLen = wcslen( pData->szLine );
    if ( (iAddLen + 5) < pData->iSegBufferFree )// always leave room for "..." and CRLF at the end of the buffer
    {
      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, pData->szLine );
      pData->iSegBufferUsed += iAddLen;
      pData->iSegBufferFree -= iAddLen;

      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, L"\r\n" );
      pData->iSegBufferUsed += 2;
      pData->iSegBufferFree -= 2;
    }
    else
    {
      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, L"...\r\n" );
      pData->iSegBufferUsed += 5;
      pData->fSegBufferFull = TRUE;
      pData->iSegBufferFree = 0;
    } /* endif */
  } /* endif */
}

// concatenate segment data and ensure that size limit is not exceeded
USHORT ConcatenateSegData( PSZ_W pszSegment, PSZ_W pszNewData, int iMaxSize)
{
  int iExistingLen = wcslen( pszSegment );
  int iAddLen = wcslen( pszNewData );
  if ( (iExistingLen + iAddLen) >= iMaxSize )
  {
    return( ERROR_INVALID_SEGMENT );
  }

  wcscpy( pszSegment + iExistingLen, pszNewData );
  return( 0 );
}

// extract next segment from input memory
USHORT GetNextSegment( PCONVERTERDATA pData, PBOOL pfSegmentAvailable )
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fEOF = FALSE;

  BOOL fControlString = FALSE;

  *pfSegmentAvailable = FALSE;

  // skip any lines until the start of a new segment is detected
  while ( !usRC && !fEOF && _wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0 ) 
  {
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
  } /* enwhile */

  // read data until segment is complete
  if ( _wcsnicmp( pData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    int iStartLine = pData->iLineNum;
    PSZ pszError = "";

    // copy all lines of the segment to our segment buffer (for logging in cas of errors)
    ClearSegBuffer( pData );
    AddLineToSegBuffer( pData );

    // reset segment data
    pData->Segment.szSource[0] = 0;
    pData->szControl[0] = 0;
    pData->Segment.szTarget[0] = 0;
    pData->Segment.szAddInfo[0] = 0;
    wcscpy( pData->szSegStart, pData->szLine );

    while ( !usRC && !fEOF && _wcsnicmp( pData->szLine, L"</segment>", 10 ) != 0 ) 
    {
      GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
      AddLineToSegBuffer( pData );

      if ( _wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
      {
        fControlString = TRUE;

        // get the control string
        GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
        AddLineToSegBuffer( pData );
        wcscpy( pData->szControl, pData->szLine );

        if ( fEOF )
        {
          // incomplete segment data
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
          pszError = "Incomplete segment control string";
        }
        else if ( _wcsnicmp( pData->szLine, L"</control>", 10 ) == 0 )
        {
          // missing control string
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
          pszError = "Missing control string";
        }
        else
        {
          // split control string into its elements
          pData->Segment.lSegNum = _wtol( ParseX15W( pData->szControl, 0 ) );
          if ( *(ParseX15W( pData->szControl, 1 )) == L'1' )
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_MACHINE;
          }
          else if ( *(ParseX15W( pData->szControl, 1 )) == L'2' )
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_GLOBMEM;
          }
          else
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_NORMAL;
          } /* endif */
          pData->Segment.lTime = _wtol( ParseX15W( pData->szControl, 2 ) );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 3 ) );

          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szSourceLang, sizeof(pData->Segment.szSourceLang), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 4 ));
 
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szTargetLang, sizeof(pData->Segment.szTargetLang), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 6 ));

          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szFormat, sizeof(pData->Segment.szFormat), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 8 ) );
;
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szDocument, sizeof(pData->Segment.szDocument), NULL, NULL );

          // get contol string end tag and check it
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );
          AddLineToSegBuffer( pData );
          if ( _wcsnicmp( pData->szLine, L"</control>", 10 ) != 0 )
          {
            // missing end control string
            LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
            pszError = "Missing end control tag";
          }

        } /* endif */

      }
      else if ( _wcsnicmp( pData->szLine, L"<source>", 8 ) == 0  )
      {
        // get the segment source
        BOOL fEnd = StripTag( pData->szLine + 8, L"</source>" );
        usRC = ConcatenateSegData( pData->Segment.szSource, pData->szLine + 8, sizeof(pData->Segment.szSource)/sizeof(CHAR_W) );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szSource, L"\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment source text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
            pszError = "Missing </source> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</source>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szSource, pData->szLine, sizeof(pData->Segment.szSource)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment source text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
          pszError = "Missing </source> tag";
        } /* endif */
      }
      else if ( _wcsnicmp( pData->szLine, L"<target>", 8 ) == 0 )
      {
        // get the segment target
        BOOL fEnd = StripTag( pData->szLine + 8, L"</target>" );
        usRC = ConcatenateSegData( pData->Segment.szTarget, pData->szLine + 8, sizeof(pData->Segment.szTarget)/sizeof(CHAR_W) );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szTarget, L"\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment target text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
            pszError = "Missing </target> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</target>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szTarget, pData->szLine, sizeof(pData->Segment.szTarget)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment target text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
          pszError = "Missing </target> tag";
        } /* endif */
      } 
      else if ( wcsnicmp( pData->szLine, L"<adddata>", 9 ) == 0 )
      {
        // get the segment additional data
        BOOL fEnd = StripTag( pData->szLine + 9, L"</adddata>" );
        usRC = ConcatenateSegData( pData->Segment.szAddInfo, pData->szLine + 9, MAX_SEGMENT_SIZE );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szAddInfo, L"\r\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment additional data text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
            pszError = "Missing </adddata> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</adddata>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szAddInfo, pData->szLine, sizeof(pData->Segment.szAddInfo)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment additional data text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
          pszError = "Missing </adddata> tag";
        } /* endif */
      } /* endif */
    } /*endwhile */

    // read next line
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    // test segment data
    if ( !usRC )
    {
      int iSourceLen = wcslen( pData->Segment.szSource );
      int iTargetLen = wcslen( pData->Segment.szTarget );

      if ( !fControlString )
      {
        pszError = "Missing <control>...</control> section";
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
      }
      else if ( iSourceLen >= MAX_SEGMENT_SIZE )
      {
        pszError = "Segment source text is too long";
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
      }
      else if (iTargetLen >= MAX_SEGMENT_SIZE )
      {
        pszError = "Segment target text is too long";
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
      }
      else if ( iSourceLen == 0 )
      {
        pszError = "No segment source text found";
        LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_SEGMENT);
      } /* endif */
    } /* endif */

    if ( usRC )
    {
      sprintf( pData->szErrorText, "The segment beginning in line %ld is invalid, the reason is \"%s\".", iStartLine, pszError );

      //if ( pData->hfLog != NULL )
      {
        T5LOG(T5INFO) <<"The segment beginning in line "<< iStartLine <<" is invalid, the reason is \""<< pszError 
            <<"\".\nSegment data:" <<pData->szSegBuffer;
      } /* endif */
    }
    else
    {
        *pfSegmentAvailable = TRUE;
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function GetNextSegment */

// close input memory
USHORT CloseInput( PCONVERTERDATA pData )
{
  USHORT      usRC = NO_ERROR;         // function return code

  if ( pData->hInFile )
  {
    fclose( pData->hInFile );
    pData->hInFile = NULL;
  } /* endif */
  return( usRC );
} /* end of function CloseInput */

// (re)fill input buffer
USHORT FillBufferW
(
  PCONVERTERDATA pData
)
{
  USHORT      usRC = 0;
  PSZ_W       pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBufW + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBufW, pTemp, iStillInBuf*sizeof(CHAR_W) );
    memset( &pData->chInBufW[iStillInBuf], 0, (BUF_SIZE-iStillInBuf)*sizeof(CHAR_W));
    ulBytesRead = fread( (pData->chInBufW + iStillInBuf), 1, (BUF_SIZE - iStillInBuf)*sizeof(CHAR_W), pData->hInFile );
    pData->iInBufProcessed = 0;
    pData->iInBufRead = (ulBytesRead / sizeof(CHAR_W)) + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    ulBytesRead = fread( pData->chInBufW, 1, BUF_SIZE *sizeof(CHAR_W), pData->hInFile );

    pData->iInBufRead = ulBytesRead / sizeof(CHAR_W);
  } /* endif */

  return( usRC );
} /* end of function FillBufferW */

USHORT FillBuffer
(
  PCONVERTERDATA pData
)
{
  USHORT      usRC = 0;
  PSZ         pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBuf + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBuf, pTemp, iStillInBuf );
    memset( &pData->chInBuf[iStillInBuf], 0, (BUF_SIZE-iStillInBuf) );
    ulBytesRead = fread( (pData->chInBuf + iStillInBuf), 1, (BUF_SIZE - iStillInBuf), pData->hInFile );

    pData->iInBufProcessed = 0;
    pData->iInBufRead = ulBytesRead + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    ulBytesRead = fread( pData->chInBuf, 1, BUF_SIZE, pData->hInFile );

    pData->iInBufRead = ulBytesRead;
  } /* endif */

  return( usRC );
} /* end of function FillBuffer */

USHORT ReadLineW
(
  PCONVERTERDATA pData,
  PSZ_W    pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ_W       pLF;
  PSZ_W       pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = FillBufferW( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBufW + pData->iInBufProcessed;

  // get one line out of buffer
  pLF = wcschr( pTemp, L'\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen * sizeof(CHAR_W) );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == L'\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pData->iInBufProcessed += (iLen+1);
  }
  else
  {
      LOG_AND_SET_RC(usRC, T5INFO, BTREE_EOF_REACHED);
  } /* endif */

  return( usRC );
} /* end of function ReadLineW */


USHORT ReadLine
(
  PCONVERTERDATA pData,
  PSZ      pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ         pLF;
  PSZ         pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = FillBuffer( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBuf + pData->iInBufProcessed;

  // get one line out of buffer
  pLF = strchr( pTemp, '\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == '\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pData->iInBufProcessed += (iLen+1);
  }
  else
  {
      LOG_AND_SET_RC(usRC, T5INFO, BTREE_EOF_REACHED);
  } /* endif */

  return( usRC );
} /* end of function ReadLine */


USHORT GetLine( PCONVERTERDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  )
{
  USHORT usRC = 0;

  *pszLine = 0;
  if ( fUnicode )
  {
    usRC = ReadLineW( pData, pszLine, iSize );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */
  }
  else
  {
    static CHAR szAsciiLine[8096];  

    szAsciiLine[0] = EOS;

    usRC = ReadLine( pData, szAsciiLine, sizeof(szAsciiLine) );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */

    T5LOG(T5ERROR) <<"TEMPORARY COMMENTED GetLine";
    //MultiByteToWideChar( pData->ulInputCP, 0, szAsciiLine, -1, pszLine, iSize-1 );
  } /* endif */

  pData->iLineNum++;
  return( usRC );
}

// helper function to check for and remove the given tag from the end of the string
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag )
{
  BOOL fFound = FALSE;
  PSZ_W pszTest = NULL;

  int  iLen = wcslen( pszLine );
  int  iTagLen = wcslen( pszTag );

  if ( iLen >= iTagLen )
  {
    pszTest = pszLine + (iLen - iTagLen);
  } /* endif */

  if ( pszTest )
  {
    fFound = ( _wcsnicmp( pszTest, pszTag, iTagLen ) == 0 );

    if ( fFound )
    {
      *pszTest = 0;
    } /* endif */
  } /* endif */

  return( fFound );
} /* end of function StripTag */

PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId )
{
   PSZ_W   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the X15 string
      while ((*pszTemp != X15) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or 0x15)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != X15) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of ParseX15 */


// code borrowed from EQFMEMUT.C as the EQFDLL currently does not export this functions
// functions and defines for the access to additional data of proposals (MAD)

// prototypes for private functions
PSZ_W MADSkipName( PSZ_W pszName );
PSZ_W MADSkipWhitespace( PSZ_W pszData );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADCompareKey( PSZ_W pszKey1, PSZ_W pzKey2 );
BOOL  MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 );
BOOL  MADNextAttr( PSZ_W *ppszAttr );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize );


//
// Search a specific key in the additional memory data
//
HADDDATAKEY MADSearchKey( PSZ_W pAddData, PSZ_W pszKey )
{
  HADDDATAKEY pKey = NULL;
  if ( (pAddData != NULL) && (*pAddData != 0) )
  {
    while ( (pKey == NULL) && (*pAddData != 0) )
    {
      // move to begin of next key
      while ( (*pAddData != 0) && (*pAddData != L'<') ) pAddData++;

      // check key
      if ( (*pAddData == L'<')  )
      {
        if ( MADCompareKey( pAddData + 1, pszKey ) )
        {
          pKey = pAddData;
        }
        else
        {
          pAddData++;
        } /* endif */           
      } /* endif */         
    } /* endwhile */       
  } /* endif */     

  return( pKey );
}

//
// retrieve the data associated with a specific key
//
BOOL MDAGetValueForKey( HADDDATAKEY pKey, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fValue = FALSE;
  BOOL fAttrAvailable = FALSE; 

  *pszBuffer = 0;

  // skip any attributes
  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );
  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
  } while ( fAttrAvailable ); /* enddo */   

  if ( *pszAttr == L'>' )
  {
    PSZ_W pszValue = pszAttr + 1;
    while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'<') )
    {
      *pszBuffer++ = *pszValue++;
      iBufSize--;
    }
    *pszBuffer = 0;
    fValue = TRUE;
  } /* endif */     

  if ( !fValue && (pszDefault != NULL) ) wcscpy( pszBuffer, pszDefault );

  return( FALSE );
}

//
// get the value for a specific attribute 
//
BOOL MADGetAttr( HADDDATAKEY pKey, PSZ_W pszAttrName, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fAttrAvailable = FALSE;

  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );

  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
    if ( fAttrAvailable )
    {
      if ( MADCompareAttr( pszAttr, pszAttrName ) )
      {
        MADGetAttrValue( pszAttr, pszBuffer, iBufSize );
        return( TRUE );
      } /* endif */         
    } /* endif */       

  } while ( fAttrAvailable ); /* enddo */   
  if ( pszDefault != NULL ) wcscpy( pszBuffer, pszDefault );
  return( FALSE);
}

//
// get the next attribute/value pair
//
BOOL MADGetNextAttr( HADDDATAKEY *ppKey, PSZ_W pszAttrNameBuffer, PSZ_W pszValueBuffer, int iBufSize  )
{
  PSZ_W pszAttr = *ppKey;
  BOOL fAttrAvailable = FALSE;

  *pszAttrNameBuffer = 0;
  *pszValueBuffer =0;
  if ( *pszAttr == L'<' ) 
  {
    pszAttr++;
    pszAttr = MADSkipName( pszAttr );
  } /* endif */     

  fAttrAvailable = MADNextAttr( &pszAttr );
  if ( fAttrAvailable )
  {
    CHAR_W chTemp;
    PSZ_W pszNameEnd = pszAttr;
    while ( (*pszNameEnd != 0) && (*pszNameEnd != L' ') && (*pszNameEnd != L'>') && (*pszNameEnd != L'=') )  pszNameEnd++;
    chTemp = *pszNameEnd;
    *pszNameEnd = 0;
    wcscpy( pszAttrNameBuffer, pszAttr );
    *pszNameEnd = chTemp;
    MADGetAttrValue( pszAttr, pszValueBuffer, iBufSize );
  } /* endif */   
  *ppKey = pszAttr;
  return( fAttrAvailable );
}

// ======== internal functions for Memory Additional Data processing

// position to next attribute
BOOL MADNextAttr( PSZ_W *ppszAttr )
{
  PSZ_W pszAttr = *ppszAttr;

  // if we are at an attribute already we skip this one
  if ( iswalpha( *pszAttr) || (*pszAttr == L'-') || (*pszAttr == L'_') || (*pszAttr == L':') )
  {
    pszAttr = MADSkipName( pszAttr );
    pszAttr = MADSkipAttrValue( pszAttr );
  } /* endif */     

  pszAttr = MADSkipWhitespace( pszAttr );
  while ( *pszAttr == L' ' ) pszAttr++;

  // set caller's pointer
  *ppszAttr = pszAttr;

  return( iswalpha( *pszAttr ) );
}


// retrieve attribute value
BOOL MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize )
{
  PSZ_W pszValue = MADSkipName( pszAttr );
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'\"') )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( (iBufSize > 1) && iswalpha(*pszValue) || iswdigit(*pszValue) )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
    } /* endif */       

  } /* endif */     
  *pszBuffer = 0;

  return( TRUE );
}

// compare attribute names
BOOL MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszAttr1 != 0) && (*pszAttr2 != 0) && (*pszAttr1 == *pszAttr2) )
  {
    pszAttr1++; pszAttr2++;
  } /* endwhile */     

  fMatch = ( ((*pszAttr1 == 0) || (*pszAttr1 == L' ') || (*pszAttr1 == L'>') || (*pszAttr1 == L'=')) && 
             ((*pszAttr2 == 0) || (*pszAttr2 == L' ') || (*pszAttr2 == L'>') || (*pszAttr2 == L'=')) );

  return( fMatch );
}

// compare two key names
BOOL MADCompareKey( PSZ_W pszKey1, PSZ_W pszKey2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszKey1 != 0) && (*pszKey2 != 0) && (*pszKey1 == *pszKey2) )
  {
    pszKey1++; pszKey2++;
  } /* endwhile */     

  fMatch = ( ((*pszKey1 == 0) || (*pszKey1 == L' ') || (*pszKey1 == L'>')) && ((*pszKey2 == 0) || (*pszKey2 == L' ') || (*pszKey2 == L'>')) );

  return( fMatch );
}

// skip attribute value: loop until end of current attribute value
PSZ_W MADSkipAttrValue( PSZ_W pszValue )
{
  // pszValuze points to first character following the attribute name, eithere the name is followed
  // by an equal sign and the value or the attribute has no value at all
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (*pszValue != 0) && (*pszValue != L'\"') ) pszValue++;
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( iswalpha(*pszValue) || iswdigit(*pszValue) ) pszValue++;
    } /* endif */       

  } /* endif */     
  return( pszValue );
}

// skip name: loop until end of current name
PSZ_W MADSkipName( PSZ_W pszName )
{
  while ( iswalpha(*pszName) || iswdigit(*pszName) || (*pszName == L'-') || (*pszName == L'_') || (*pszName == L':') ) pszName++;
  return( pszName );
}

// skip any whitespace: increase given pointer until non-blank character reached
PSZ_W MADSkipWhitespace( PSZ_W pszData )
{
  while ( *pszData == L' ' ) pszData++;
  return( pszData );
}

// escape characters in segment data to form valid XML
void EscapeXMLChars( PSZ_W pszText, PSZ_W pszBuffer)
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      wcscpy( pszBuffer, L"\r\n" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\r' )
    {
    }
    else if ( *pszText == L'&' )
    {
      wcscpy( pszBuffer, L"&amp;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'<' )
    {
      wcscpy( pszBuffer, L"&lt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'>' )
    {
      wcscpy( pszBuffer, L"&gt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\"' )
    {
      wcscpy( pszBuffer, L"&quot;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( (*pszText == L'\x1F') || (*pszText == L'\t') )
    {
      // suppress some special characters
      *pszBuffer++ = L' ';
    }
    else
    {
      *pszBuffer++ = *pszText;
    } /* endif */
    pszText++;
  } /*endwhile */
  *pszBuffer = 0;
}

BOOL CheckEntityW( PSZ_W pszText, int *piValue, int *piLen )
{
  int iValue = 0;
  int iLen = 0;

  if ( pszText[iLen++] != L'&' ) return( FALSE );
  if ( pszText[iLen++] != L'#' ) return( FALSE );
  
  if ( pszText[iLen] == L'x' )
  {
    iLen++;
    while ( iswxdigit( pszText[iLen] ) )
    {
      CHAR_W c = pszText[iLen++];

      iValue <<= 4;

      if ( (c >= L'0') && (c <= L'9') )
      {
          iValue += c - L'0';
          continue;
      }

      if ( (c >= L'A') && (c <= L'F') )
      {
          iValue += (c - L'A') + 10;
          continue;
      }

      if ( (c >= L'a') && (c <= L'f') )
      {
          iValue += (c - L'a') + 10;
          continue;
      }
      return( FALSE );
    } /* endwhile */
  }
  else
  {
    while ( iswdigit( pszText[iLen] ) )
    {
      CHAR_W c = pszText[iLen++];

      iValue = iValue * 10;

      if ( (c >= L'0') && (c <= L'9') )
      {
          iValue += c - '0';
          continue;
      }
      return( FALSE );
    } /* endwhile */
  } /* endif */

  if ( pszText[iLen] == L';' )
  {
    *piLen = iLen + 1;
    *piValue = iValue;
    return( TRUE );
  } /* endif */

  return( FALSE );
} /* end of function CheckEntityW */

BOOL CheckEntity( PSZ pszText, int *piValue, int *piLen )
{
  int iValue = 0;
  int iLen = 0;

  if ( pszText[iLen++] != '&' ) return( FALSE );
  if ( pszText[iLen++] != '#' ) return( FALSE );
  
  if ( pszText[iLen] == 'x' )
  {
    iLen++;
    while ( isxdigit( pszText[iLen] ) )
    {
      char c = pszText[iLen++];

      iValue <<= 4;

      if ( (c >= '0') && (c <= '9') )
      {
          iValue += c - '0';
          continue;
      }

      if ( (c >= 'A') && (c <= 'F') )
      {
          iValue += (c - 'A') + 10;
          continue;
      }

      if ( (c >= 'a') && (c <= 'f') )
      {
          iValue += (c - 'a') + 10;
          continue;
      }
      return( FALSE );
    } /* endwhile */
  }
  else
  {
    while ( isdigit( pszText[iLen] ) )
    {
      char c = pszText[iLen++];

      iValue = iValue * 10;

      if ( (c >= '0') && (c <= '9') )
      {
          iValue += c - '0';
          continue;
      }
      return( FALSE );
    } /* endwhile */
  } /* endif */

  if ( pszText[iLen] == ';' )
  {
    *piLen = iLen + 1;
    *piValue = iValue;
    return( TRUE );
  } /* endif */

  return( FALSE );
} /* end of function CheckEntity */

void CopyTextAndProcessMaskedEntitites( PSZ_W pszTarget, PSZ_W pszSource )
{
  while ( *pszSource != 0 )
  {
    PSZ_W pszMask = wcsstr( pszSource, ENTITY_MASK_START_W );

    if ( *pszSource == ENTITY_MASK_STARTCHAR_W )
    {
      if ( wcsncmp( pszSource, ENTITY_MASK_START_W, ENTITY_MASK_START_LEN ) == 0 )
      {
        // replace entity mask start with '&'
        pszSource += ENTITY_MASK_START_LEN;
        *pszTarget++ = L'&';
      }
      else if ( wcsncmp( pszSource, ENTITY_MASK_END_W, ENTITY_MASK_END_LEN ) == 0 )
      {
        // skip entity mask end
        pszSource += ENTITY_MASK_END_LEN;
      }
      else
      {
        *pszTarget++ = *pszSource++;
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /* endwhile */
  *pszTarget = 0;
} /* end of function CopyTextAndProcessMaskedEntitites */



int _wcsicmp(const PWCHAR p1, const PWCHAR p2){
  return wcscasecmp(p1, p2);
}

int _stricmp(const char* p1, const char* p2){
  return strcasecmp(p1, p2);
}


int wcsicmp(const PWCHAR p1, const PWCHAR p2){
  return _wcsicmp(p1, p2);
}

int stricmp(const char* p1, const char* p2){
  return _stricmp(p1, p2);
}

int _wcsnicmp(const PWCHAR p1, const PWCHAR p2, const int sz){
  return wcsncasecmp(p1,p2,sz);
}

int wcsnicmp(const PWCHAR p1, const PWCHAR p2, const int sz){
  return _wcsnicmp(p1,p2,sz);
}

int _strnicmp(const char* p1, const char* p2, const int sz){
  return strncasecmp(p1, p2, sz);
}

int strnicmp(const char* p1, const char* p2, const int sz){
  return _strnicmp(p1, p2, sz);
}

long _wtol(
   const wchar_t *str
){
    return std::stol(str);
}


