//+----------------------------------------------------------------------------+
//|EQFTMTAG.H    Internal header file for Translation Memory SGML tags         |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

// Define statements used by programs working with the external
// format of translation memories
#define MEM_SEGMENT_BEGIN_TAGW    L"<Segment>"           // Memory DB segment begin tag
#define MEM_SEGMENT_END_TAGW      L"</Segment>\n"      // Memory DB segment end tag
#define MEM_SOURCE_BEGIN_TAGW     L"<Source>"            // Memory DB source begin tag
#define MEM_SOURCE_END_TAGW       L"</Source>\n"       // Memory DB source end tag
#define MEM_TARGET_BEGIN_TAGW     L"<Target>"            // Memory DB Target begin tag
#define MEM_TARGET_END_TAGW       L"</Target>\n"       // Memory DB Target end tag
#define MEM_CONTEXT_BEGIN_TAGW    L"<Context>"           // Memory DB context begin tag
#define MEM_CONTEXT_END_TAGW      L"</Context>\n"      // Memory DB context end tag
#define MEM_TARGET_BEGIN_TAGW     L"<Target>"            // Memory DB Target begin tag
#define MEM_TARGET_END_TAGW       L"</Target>\n"       // Memory DB Target end tag
//#define MEM_CONTROL_BEGIN_TAG    "<Control>\n"        // Memory DB Control begin tag
//#define MEM_CONTROL_END_TAG      "\n</Control>\n"   // Memory DB Control end tag
#define MEM_BEGIN_TAGW            L"<MemoryDb>\n"      // Memory DB begin tag
#define MEM_END_TAGW              L"</MemoryDb>\n"     // Memory DB end tag
#define NTM_BEGIN_TAGW            L"<NTMMemoryDb>\n"   // new Memory DB begin tag /*@NTM*/
#define NTM_END_TAGW              L"</NTMMemoryDb>\n"  // new Memory DB end tag   /*@NTM*/
#define NTM_DESCRIPTION_BEGIN_TAGW L"<Description>\n"  // description begin tag   /*@NTM*/
#define NTM_DESCRIPTION_END_TAGW   L"\n</Description>\n" // description end tag     /*@NTM*/
#define MEM_ADDINFO_BEGIN_TAGW    L"<AddData>"           // Memory DB additional information begin tag
#define MEM_ADDINFO_END_TAGW      L"</AddData>\n"      // Memory DB additional information end tag

#define MEM_AUTHOR_BEGIN_TAGW    L"<Author>"           // Memory DB additional information begin tag
#define MEM_AUTHOR_END_TAGW      L"</Author>\n"      // Memory DB additional information end tag

#define MEM_SEGMENT_BEGIN_TAG    "<Segment>"           // Memory DB segment begin tag
#define MEM_SEGMENT_END_TAG      "</Segment>\n"      // Memory DB segment end tag
#define MEM_SOURCE_BEGIN_TAG     "<Source>"            // Memory DB source begin tag
#define MEM_SOURCE_END_TAG       "</Source>\n"       // Memory DB source end tag
#define MEM_TARGET_BEGIN_TAG     "<Target>"            // Memory DB Target begin tag
#define MEM_TARGET_END_TAG       "</Target>\n"       // Memory DB Target end tag
#define MEM_CONTROL_BEGIN_TAG    "<Control>\n"        // Memory DB Control begin tag
#define MEM_CONTROL_END_TAG      "\n</Control>\n"   // Memory DB Control end tag
#define MEM_BEGIN_TAG            "<MemoryDb>\n"      // Memory DB begin tag
#define MEM_END_TAG              "</MemoryDb>\n"     // Memory DB end tag
#define NTM_BEGIN_TAG            "<NTMMemoryDb>\n"   // new Memory DB begin tag /*@NTM*/
#define NTM_END_TAG              "</NTMMemoryDb>\n"  // new Memory DB end tag   /*@NTM*/
#define NTM_DESCRIPTION_BEGIN_TAG "<Description>\n"  // description begin tag   /*@NTM*/
#define NTM_DESCRIPTION_END_TAG   "\n</Description>\n" // description end tag     /*@NTM*/
#define MEM_CONTROL_BEGIN_TAGW    L"<Control>\n"        // Memory DB Control begin tag
#define MEM_CONTROL_END_TAGW      L"\n</Control>\n"   // Memory DB Control end tag

#define NTM_CODEPAGE_BEGIN_TAGW L"<Codepage>"          
#define NTM_CODEPAGE_END_TAGW   L"</Codepage>\n" 

#define NTM_CODEPAGE_ASCII_VALUE L"ASCII=%d"          
#define NTM_CODEPAGE_ANSI_VALUE  L"ANSI=%d"          
#define NTM_CODEPAGE_UTF16_VALUE  L"UTF16"          


