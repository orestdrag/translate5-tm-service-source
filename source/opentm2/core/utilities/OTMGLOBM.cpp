//+----------------------------------------------------------------------------+
//| OTMGLOBM.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//| 	Copyright (C) 1990-2014, International Business Machines                 |
//| 	Corporation and others. All rights reserved                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Global Memory processing functions                            |
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

#include "CXMLWRITER.H"

XERCES_CPP_NAMESPACE_USE

#ifndef CPPTEST
extern "C"
{
#endif
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
  #include <OTMGLOBMEM.H>           // Global memory defines and prototypes

  //#pragma pack( pop, TM2StructPacking )
#ifndef CPPTEST
}
#endif

#ifdef _DEBUG
  #define SGMLDITA_LOGGING
#endif

//
// class for our SAX handler
//

#define MAX_CAPTURE_LEN 8096


class CTIDListParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  CTIDListParseHandler( PGMOPTIONFILE pListIn );
  virtual ~CTIDListParseHandler();

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const unsigned int length);
  
  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

  PGMOPTIONFILE GetLoadedList();

private:
  enum { FROM_ELEMENT, TO_ELEMENT } m_CurrentElement;
  BOOL        m_fCaptureText;                    // TRUE = capture text to our text buffer
  CHAR_W      m_szTextBuffer[MAX_CAPTURE_LEN];   // text capture buffer
  GM_CTID_OPTIONS m_CurEntry;                    // currently processed CTID list entry 
  PGMOPTIONFILE m_pList;                         // ptr to current CTID list
};

//
// Implementation of CTID List SAX parser
//
CTIDListParseHandler::CTIDListParseHandler( PGMOPTIONFILE pListIn )
{
  m_pList = pListIn;
}

CTIDListParseHandler::~CTIDListParseHandler()
{
}

void CTIDListParseHandler::startElement
(
  const XMLCh* const name, 
  AttributeList& attributes
)
{
  PSZ_W pszName = (PSZ_W)name;

  int iAttribs = attributes.getLength(); 


  m_szTextBuffer[0] = EOS;                       // clear text buffer

  if ( wcsicmp( pszName, L"ProjectId" ) == 0 )
  {
    // start a entry
    memset( &m_CurEntry, 0, sizeof(m_CurEntry) );
    for( int i = 0; i < iAttribs; i++ )
    {
      PSZ_W pszName = (PSZ_W)attributes.getName( i );
      PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
      if ( pszValue != NULL )
      {
        if ( wcsicmp( pszName, L"name" ) == 0)
        {
          wcsncpy( m_CurEntry.szProjectID, pszValue, PROJECTID_SIZE );
        }
        else if ( wcsicmp( pszName, L"tmbArchived" ) == 0)
        {
          if ( wcslen( pszValue ) == 1 )
          {
            if ( *pszValue == L'0' )
            {
              m_CurEntry.fTmbArchived = FALSE;
            }
            else if ( *pszValue == L'1' )
            {
              m_CurEntry.fTmbArchived = TRUE;
            } /* endif */                 
          } /* endif */                 
        }
        else if ( wcsicmp( pszName, L"Iuc" ) == 0)
        {
          m_CurEntry.chICU = *pszValue;
        }
        else if ( wcsicmp( pszName, L"division" ) == 0)
        {
          wcsncpy( m_CurEntry.szDivision, pszValue, DIVISION_SIZE );
        } 
        else if ( wcsicmp( pszName, L"numberOfWords" ) == 0)
        {
          m_CurEntry.lWords = _wtol( pszValue );
        } 
        else if ( wcsicmp( pszName, L"numberOfSegments" ) == 0)
        {
          m_CurEntry.lSegments = _wtol( pszValue );
        } 
        else if ( wcsicmp( pszName, L"cfmArrivalDate" ) == 0)
        {
          wcsncpy( m_CurEntry.szArrivalDate, pszValue, ARRDATE_SIZE );
        } 
        else if ( wcsicmp( pszName, L"processingFlag" ) == 0 )
        {
          if ( wcsicmp( pszValue, L"Substitute" ) == 0 )
          {
            m_CurEntry.Option = GM_SUBSTITUTE_OPT;
          } 
          else if ( (wcsicmp( pszValue, L"GlobMem" ) == 0) || (wcsicmp( pszValue, L"Hamster" ) == 0) )
          {
            m_CurEntry.Option = GM_HFLAG_OPT;
          } 
          else if ( (wcsicmp( pszValue, L"GlobMemStar" ) == 0) || (wcsicmp( pszValue, L"HamsterStar" ) == 0))
          {
            m_CurEntry.Option = GM_HFLAGSTAR_OPT;
          } 
          else if ( wcsicmp( pszValue, L"Exclude" ) == 0 )
          {
            m_CurEntry.Option = GM_EXCLUDE_OPT;
          } 
          else 
          {
            m_CurEntry.Option = GM_EXCLUDE_OPT;
          } /* endif */                 
        } /* endif */
      } /* endif */
    } /* endfor */
  } 
  else  if ( wcsicmp( pszName, L"generator" ) == 0 )
  {
    m_fCaptureText = TRUE; 
    m_szTextBuffer[0] = 0; 
  } 
  else  if ( wcsicmp( pszName, L"generatorVersion" ) == 0 )
  {
    m_fCaptureText = TRUE; 
    m_szTextBuffer[0] = 0; 
  } 
  else  if ( wcsicmp( pszName, L"creationDate" ) == 0 )
  {
    m_fCaptureText = TRUE; 
    m_szTextBuffer[0] = 0; 
  } 
  else  if ( wcsicmp( pszName, L"metadataFileName" ) == 0 )
  {
    m_fCaptureText = TRUE; 
    m_szTextBuffer[0] = 0; 
  } 
  else  if ( wcsicmp( pszName, L"inputSpecs" ) == 0 )
  {
    m_fCaptureText = TRUE; 
    m_szTextBuffer[0] = 0; 
  } 
  else  if ( wcsicmp( pszName, L"Filter" ) == 0 )
  {
    for( int i = 0; i < iAttribs; i++ )
    {
      PSZ_W pszName = (PSZ_W)attributes.getName( i );
      PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
      if ( pszValue != NULL )
      {
        if ( wcsicmp( pszName, L"name" ) == 0)
        {
          wcsncpy( m_pList->szFilterName, pszValue, HEADERINFO_SIZE );
        }
        else if ( wcsicmp( pszName, L"globMemFolder" ) == 0)
        {
          if ( wcslen( pszValue ) == 1 )
          {
            if ( *pszValue == L'Y' )
            {
              m_pList->fGlobMemFolder = FALSE;
            }
            else if ( *pszValue == L'N' )
            {
              m_pList->fGlobMemFolder = TRUE;
            } /* endif */                 
          } /* endif */                 
        }
      } /* endif */
    } /* endfor */
  } /* endif */

} /* end of method CTIDListParseHandler::startElement */

void CTIDListParseHandler::endElement(const XMLCh* const name )
{
  PSZ_W pszName = (PSZ_W)name;

  m_fCaptureText = FALSE;                        // stop text capturing

  if ( wcsicmp( pszName, L"ProjectId" ) == 0 )
  {
    // add new entry to our table
    if ( m_CurEntry.szProjectID[0] != 0 )
    {
      // enlarge table if necessary
      if ( m_pList->lNumOfEntries >= m_pList->lMaxNumOfEntries )
      {
        int iOldSize = m_pList->lMaxNumOfEntries * sizeof(GM_CTID_OPTIONS) + sizeof(GMOPTIONFILE);
        int iNewSize = (m_pList->lMaxNumOfEntries + 10) * sizeof(GM_CTID_OPTIONS) + sizeof(GMOPTIONFILE);
        if ( UtlAlloc( (PVOID *)&m_pList, iOldSize, iNewSize, ERROR_STORAGE ) )
        {
          m_pList->lMaxNumOfEntries += 10;
        }
        else
        {
          m_pList->fErrorInList = TRUE;
        } /* endif */           
      } /* endif */         

      // add the new entry
      if ( m_pList->lNumOfEntries < m_pList->lMaxNumOfEntries )
      {
        memcpy( &(m_pList->Options[m_pList->lNumOfEntries]), &m_CurEntry, sizeof(GM_CTID_OPTIONS) );
        m_pList->lNumOfEntries += 1;
      } /* endif */         
    } /* endif */       
  }
  else  if ( wcsicmp( pszName, L"generator" ) == 0 )
  {
    wcsncpy( m_pList->szGenerator, m_szTextBuffer, HEADERINFO_SIZE );
  } 
  else  if ( wcsicmp( pszName, L"generatorVersion" ) == 0 )
  {
    wcsncpy( m_pList->szGeneratorVersion, m_szTextBuffer, HEADERINFO_SIZE );
  } 
  else  if ( wcsicmp( pszName, L"creationDate" ) == 0 )
  {
    wcsncpy( m_pList->szCreationDate, m_szTextBuffer, HEADERINFO_SIZE );
  } 
  else  if ( wcsicmp( pszName, L"metadataFileName" ) == 0 )
  {
    wcsncpy( m_pList->szMetadataFileName, m_szTextBuffer, HEADERINFO_SIZE );
  } 
  else  if ( wcsicmp( pszName, L"inputSpecs" ) == 0 )
  {
    wcsncpy( m_pList->szInputSpecs, m_szTextBuffer, HEADERINFO_SIZE );
  } /* endif */
} /* end of method CTIDListParseHandler::endElement */

void CTIDListParseHandler::characters(const XMLCh* const chars, const unsigned int length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  // store characters in our buffer
  if ( m_fCaptureText )
  {
    int iCurLen = wcslen( m_szTextBuffer );
    if ( iCurLen + length >= MAX_CAPTURE_LEN )
    {
      if ( !m_pList->fErrorInList )
      {
        m_pList->fErrorInList = TRUE;            
        strcpy( m_pList->szErrorText, "A string in the list is too large" );
      } /* endif */
    }
    else
    {
      wcsncpy( m_szTextBuffer + iCurLen, pszChars, length );
      m_szTextBuffer[iCurLen + length] = 0;
    } /* endif */
  } /* endif */
} /* end of method CTIDListParseHandler::characters */

void CTIDListParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    m_pList->fErrorInList = TRUE;
    strcpy( m_pList->szErrorText, message );
    XMLString::release( &message );
}

void CTIDListParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    m_pList->fErrorInList = TRUE;
    strcpy( m_pList->szErrorText, message );
    XMLString::release( &message );
}

void CTIDListParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    XMLString::release( &message );
}

PGMOPTIONFILE CTIDListParseHandler::GetLoadedList()
{
  return( m_pList );
}
