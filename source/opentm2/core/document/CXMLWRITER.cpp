//+----------------------------------------------------------------------------+
//| CXMLWRITER.CPP                                                             |
//+----------------------------------------------------------------------------+
//| Copyright Notice:                                                          |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XML Writer based on the MS .NET XmlWriter API                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//| - UFT8 encoding not implemented yet                                        |
//| - UTF16 encoding not implemented yet                                       |
//| - character escaping not implemented yet                                   |
//| - XML Writer API only partially implemented                                |
//+----------------------------------------------------------------------------+
//
// PVCS Section
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.2 $ ----------- 1 Apr 2009
// GQ: - write BOM for UTF-8 encoding
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.2 $ ----------- 16 Nov 2007
// GQ: - suppress ASCII control characters below 0x20 in XML output (fix for P033028)
//     - replace 0x16 character with blnak (fix for P032908)
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.3 $ ----------- 7 Nov 2006
// GQ: - fixed P028556: Create detailed list for redundant segment list creates an eror by filtering
//       the character hex 1f when writing XML output
// 
// 
// $Revision: 1.2 $ ----------- 16 Aug 2006
// GQ: - added escaping of quote character
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.2 $ ----------- 12 Apr 2006
// GQ: - added methods WriteStartDocType, WriteEntity, WriteEndDocType
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.2 $ ----------- 9 Nov 2005
// GQ: - changed return code of WriteStartDocuemnt method from void to BOOL and added file open error checking
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.3 $ ----------- 19 Aug 2005
// GQ: - fixed type in style sheet name reference
// 
// 
// $Revision: 1.2 $ ----------- 19 Jul 2005
// GQ: - corrected class constructor handling
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.3 $ ----------- 11 Apr 2005
// GQ: - added WriteStylesheet method
//     - corrected bug in character escaping
// 
// 
// $Revision: 1.2 $ ----------- 7 Apr 2005
// GQ: - some fixes in UTF-8 part of WriteRaw method
// 
// 
// $Revision: 1.1 $ ----------- 30 Nov 2004
// GQ: - initial put
// 
// 

  // use non-ISO version of swprintf which was used up to VS2005, once older versions are not used anymore we
  // should use the new version of the function asap
  #define _CRT_NON_CONFORMING_SWPRINTFS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <uchar.h>
//#include <windows.h>

#include "CXMLWRITER.H"
#include "../utilities/LogWrapper.h"
#include "../utilities/OSWrapper.h"
#include "../utilities/FilesystemHelper.h"
#include "../utilities/EncodingHelper.h"

CXmlWriter::CXmlWriter( const char *strFileName )
{
  this->Init();
  SetFileName( strFileName );
}

CXmlWriter::CXmlWriter()
{
  this->Init();
}

void CXmlWriter::Init( void )
{
  this->m_pRoot = NULL;
  this->m_iStackSize = 0;
  this->Indention = 0;
  this->Formatting = None; 
  this->Encoding = UTF16; 
  this->m_iCurIndention = 0;
  this->m_fOpenTag = FALSE;
  this->m_fLFBeforeEnd = TRUE;
  memset(m_Buffer,0,sizeof(m_Buffer));
}

void CXmlWriter::SetFileName( const char *strFileName )
{
  strcpy( m_strFileName, strFileName );
}


BOOL CXmlWriter::WriteStartDocument()
{
  BOOL fOK = TRUE;

  m_hf = FilesystemHelper::OpenFile(m_strFileName, "w", false);

  if ( m_hf )
  {
    // write BOM 
    if ( Encoding == UTF16 )
    {
      //@@HEADER
      fwrite( "\xff\xfe", 1, 2, m_hf );
    }
    else
    {
      //@@HEADER
      fwrite( "\xEF\xBB\xBF", 1, 3, m_hf );
    } /* endif */

    WriteRaw( L"<?xml version=\"1.0\" encoding=\"" );
    if ( Encoding == UTF16 )
    {
      WriteRaw( L"UTF-16" );
    }
    else
    {
      WriteRaw( L"UTF-8" );
    } /* endif */
    WriteRaw( L"\" ?>\n" );
    Push( CXmlWriter::Document, L"doc" );

  }
  else
  {
    fOK = FALSE;
  } /* endif */

  return( fOK );
}

void CXmlWriter::WriteStylesheet( const char *stylesheet )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( stylesheet, &pUnicodeText );
  WriteStylesheet( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteStylesheet( const WCHAR *stylesheet )
{
  WriteRaw( L"<?xml-stylesheet type=\"text/xsl\" href=\"" );
  WriteRaw( stylesheet );
  WriteRaw( L"\" ?>\n" );
}

void CXmlWriter::WriteStartDocType( const WCHAR * type  )
{
  WriteRaw( L"<!DOCTYPE " );
  WriteRaw( type );
  WriteRaw( L"\n" );
  WriteRaw( L"  [\n" );
}

void CXmlWriter::WriteEndDocType()
{
  WriteRaw( L"  ]>\n" );
}

void CXmlWriter::WriteEntity( const WCHAR *name, const WCHAR *value )
{
  WriteRaw( L"       <!ENTITY " );
  WriteRaw( name );
  WriteRaw( L" \"" );

  WriteRaw( value );
  WriteRaw( L"\">\n" );
}


//void CXmlWriter::WriteString( const WCHAR * text )
void CXmlWriter::WriteString( const wchar_t *text )
{
  // close any open tag
  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */
  
  if ( GetCurElement() == CXmlWriter::Tag )
  {
    SetContentFlag();
  } /* endif */
  
  WriteStringInt( text );
}

void CXmlWriter::WriteString( const char * text )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( text, &pUnicodeText );
  WriteString( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteCDataString( const char * text )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( text, &pUnicodeText );
  WriteCDataString( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteCDataString( const WCHAR *text )
{
  // close any open tag
  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */
  
  if ( GetCurElement() == CXmlWriter::Tag )
  {
    SetContentFlag();
  } /* endif */
  
  WriteRaw( L"<![CDATA[" );
  WriteRaw( text );
  WriteRaw( L"]]>" );
}


// write integer value as string
void CXmlWriter::WriteInt( int iValue )
{
  WCHAR szValue[20];
  swprintf( szValue, 20, L"%d", iValue );
  WriteString( szValue );
}

// internal write string methof with character escaping
void CXmlWriter::WriteStringInt( const WCHAR *text )
{
  WCHAR szEscapeChars[20];           
  BOOL fDone = FALSE;

  do
  {
    int iSpecialCharPos = 0;
    szEscapeChars[0] = 0;                        // no escape character sequence yet
    BOOL fEscapeChars = FALSE;

    // find next special character which requires an escape sequence
    do
    {
      switch ( text[iSpecialCharPos] )
      {
        case L'<':
          wcscpy( szEscapeChars, L"&lt;" );
          fEscapeChars = TRUE;
          break;
        case L'>':
          wcscpy( szEscapeChars, L"&gt;" );
          fEscapeChars = TRUE;
          break;
        case L'&':
          wcscpy( szEscapeChars, L"&amp;" );
          fEscapeChars = TRUE;
          break;
        case L'\"':
          wcscpy( szEscapeChars, L"&quot;" );
          fEscapeChars = TRUE;
          break;
        case 0x1F:
          wcscpy( szEscapeChars, L"" );
          fEscapeChars = TRUE;
          break;
        case 0x16:
          // required blank used by IBMA4 markup tables
          wcscpy( szEscapeChars, L" " );
          fEscapeChars = TRUE;
          break;
        case 0:
          // end of input string reached...
          fDone = TRUE;
          break;
        case L'\n':
        case L'\r':
        case L'\t':
          // allowed whitespaces, leave as is...
          iSpecialCharPos++;
          break;
        default:
          if ( text[iSpecialCharPos] < 32 )
          {
            // suppress characters below 0x20
            wcscpy( szEscapeChars, L"_" );
            fEscapeChars = TRUE;
          }
          else
          {
            // no spcial handling required
            iSpecialCharPos++;
          } /* endif */
          break;
      } /*endswitch */
    } while ( !fDone && !fEscapeChars  );

    // write characters up to special character
    if ( iSpecialCharPos ) WriteRaw( text, iSpecialCharPos );

    // write any escape sequence
    if ( fEscapeChars )
    {
      if ( szEscapeChars[0] != 0 )
      {
        WriteRaw( szEscapeChars );
      } /* endif */

      iSpecialCharPos++;
    } /* endif */

    // continue with next part of string
    text += iSpecialCharPos;

  } while ( !fDone && text[0] );
}

// write string as-is 
void CXmlWriter::WriteRaw( const WCHAR * text )
{
  if(text){
    int iLen = wcslen( text );
    WriteRaw( text, iLen );
  }else{
    LogMessage(T5ERROR,"CXmlWriter::WriteRaw, text is NULL");
  }
}

// write string as-is 
void CXmlWriter::WriteRaw( const WCHAR * text, int iLen )
{
  int ret = 0;
  // write string as-is except for LFs which are replaced by CRLF
  do
  {
    // find next LF 
    int iLFPos = 0;
    while ( (text[iLFPos] != L'\n') && iLen ) 
    {
      iLFPos++;
      iLen--;
    } /*endwhile */

    // write text up to LF
    if ( iLFPos )
    {
      if ( Encoding == UTF16 )
      {
        if ( m_hf )
        {
          ret = fwrite( text, sizeof(WCHAR), iLFPos, m_hf );
        } /* endif */
      }
      else
      {
        std::string str = EncodingHelper::convertToUTF8( std::wstring((LPWSTR)text) );
        str = str.substr(0, iLFPos);
        int iBytes = str.size();

        if(m_hf){
          ret = fwrite( str.c_str(), 1, iBytes, m_hf );
        }
      } /* endif */
      m_iColumn += iLFPos;
    } /* endif */

    // handle any LF and prepare for next part of text
    if ( text[iLFPos] == L'\n' )
    {      
      if ( Encoding == UTF16 )
      {
        //fwrite( L"\r\n", sizeof(WCHAR), 2, m_hf );
        fwrite( L"\n", sizeof(WCHAR), 1, m_hf );
      }
      else
      {
        fwrite( "\n", sizeof(char), 1, m_hf );
      } /* endif */
      iLFPos++;
      if ( iLen ) iLen--;
      m_iColumn = 0;
    } /* endif */
    text += iLFPos;
  } while ( iLen );
}

void CXmlWriter::WriteStartAttribute( const char * prefix, const char * localname, const char * ns )
{
  WCHAR *pUnicodePrefix = NULL, *pUnicodeLocalName = NULL, *pUnicodeNameSpace = NULL;

  if ( prefix )    AnsiToUnicode( prefix, &pUnicodePrefix );
  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );

  WriteStartAttribute( pUnicodePrefix, pUnicodeLocalName, pUnicodeNameSpace );

  if ( pUnicodePrefix )    free( pUnicodePrefix );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteStartAttribute( const WCHAR *prefix, const WCHAR *localname, const WCHAR *ns )
{
  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    if ( GetCurElement() == CXmlWriter::Attribute )
    {
      WriteEndAttribute();
    } /* endif */
    WriteRaw( L" " );
    WriteRaw( localname );
    WriteRaw( L"=\"" );
    Push( CXmlWriter::Attribute, localname );
  }
  else
  {
    LogMessage( T5WARNING,__func__,":: wrong context! for attribute ", EncodingHelper::convertToUTF8(localname).c_str());
    // wrong context!
  } /* endif */
}

void CXmlWriter::WriteEndAttribute()
{
  enum _ElementType Type;
  WCHAR *pName;

  if ( (GetCurElement() == CXmlWriter::Attribute) && m_iStackSize )
  {
    Pop( &Type, &pName );

    if ( Type == CXmlWriter::Attribute )
    {
      WriteRaw( L"\"" );
      free( pName );
    } /* endif */
  }
  else
  {
    LogMessage( T5WARNING,__func__,":: wrong context!");
    // wrong context!
  } /* endif */
}

void CXmlWriter::WriteAttributeString( const char *localname, const char * ns, const char *value )
{
  WCHAR *pUnicodeLocalName = NULL;
  WCHAR *pUnicodeNameSpace = NULL;
  WCHAR *pUnicodeValue = NULL;

  if ( localname ) this->AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns ) this->AnsiToUnicode( ns, &pUnicodeNameSpace );
  if ( value ) this->AnsiToUnicode( value, &pUnicodeValue );

  WriteAttributeString( pUnicodeLocalName, pUnicodeNameSpace, pUnicodeValue );

  if ( pUnicodeValue )     free( pUnicodeValue );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteAttributeString( const WCHAR *localname, const WCHAR * ns, const WCHAR *value )
{
  WriteStartAttribute( NULL, localname, ns );
  WriteStringInt( value );
  WriteEndAttribute();
}

void CXmlWriter::WriteIndention( int iIndention )
{
  if ( Formatting == Indented )
  {
    while ( m_iColumn < iIndention )
    {
      WriteRaw( L" " );
    } /*endwhile */
  } /* endif */
}

void CXmlWriter::WriteStartElement( const char * prefix, const char * localname, const char * ns )
{
  WCHAR *pUnicodePrefix = NULL;
  WCHAR *pUnicodeLocalName = NULL;
  WCHAR *pUnicodeNameSpace = NULL;

  if ( prefix )    AnsiToUnicode( prefix, &pUnicodePrefix );
  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );

  WriteStartElement( pUnicodePrefix, pUnicodeLocalName, pUnicodeNameSpace );

  if ( pUnicodePrefix )    free( pUnicodePrefix );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteStartElement( const WCHAR * prefix, const WCHAR * localname, const WCHAR * ns )
{
  // close any open attribute and start element tag
  if ( GetCurElement() == CXmlWriter::Attribute ) 
  {
    WriteEndAttribute();
  } /* endif */

  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag ) 
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */

  if ( Formatting == Indented )
  {
    WriteRaw( L"\n" );
    WriteIndention( m_iCurIndention ); 
  } /* endif */
  m_iCurIndention += Indention;
  WriteRaw( L"<" );
  WriteRaw( localname );
  SetContentFlag();
  Push( CXmlWriter::Tag, localname );
  m_fOpenTag = TRUE;
  m_fLFBeforeEnd = FALSE;

}

void CXmlWriter::WriteElementString( const char * localname, const char * ns, const char * value )
{
  WCHAR *pUnicodeLocalName = NULL, *pUnicodeNameSpace = NULL, *pUnicodeValue = NULL;

  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );
  if ( value )     AnsiToUnicode( value, &pUnicodeValue );

  WriteElementString( pUnicodeLocalName, pUnicodeNameSpace, pUnicodeValue );

  if ( pUnicodeValue )     free( pUnicodeValue );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteElementString( const WCHAR * localname, const WCHAR * ns, const WCHAR * value )
{
  WriteStartElement( NULL, localname, ns );
  WriteString( value );
  WriteEndElement();
}

void CXmlWriter::WriteComment( const char * text )
{
  WCHAR *pUnicodeText = NULL;

  if ( text ) AnsiToUnicode( text, &pUnicodeText );

  WriteComment( pUnicodeText );

  if ( pUnicodeText ) free( pUnicodeText );
}

void CXmlWriter::WriteComment( const WCHAR *text )
{
  WriteRaw( L"<!--" );
  WriteStringInt( text );
  WriteRaw( L"-->\n" );
}

void CXmlWriter::WriteEndElement()
{
  enum _ElementType Type;
  WCHAR *pName;
  BOOL fShortEnd = FALSE;

  if ( GetCurElement() == CXmlWriter::Attribute ) 
  {
    WriteEndAttribute();
  } /* endif */

  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag ) 
  {
    if  ( GetContentFlag() )
    {
      WriteRaw( L">" );
      m_fOpenTag = FALSE;
    }
    else
    {
      // write short end element
      WriteRaw( L" />" );
      m_fOpenTag = FALSE;
      fShortEnd = TRUE;
    } /* endif */

  } /* endif */

  if ( m_iStackSize )
  {
    Pop( &Type, &pName );
    m_iCurIndention -= Indention;
    if ( m_iCurIndention < 0 ) m_iCurIndention = 0;
    if ( !fShortEnd )
    {
      if ( (Formatting == Indented) && m_fLFBeforeEnd )
      {
        WriteRaw( L"\n" );
        WriteIndention( m_iCurIndention );
      } /* endif */
      WriteRaw( L"</" );
      WriteRaw( pName );
      WriteRaw( L">" );
    } /* endif */
    free( pName );
  } /* endif */
  m_fLFBeforeEnd = TRUE;
}

void CXmlWriter::WriteEndDocument()
{
  // close all open elements...
  if ( GetCurElement() == CXmlWriter::Attribute )
  {
    WriteEndAttribute();
  } /* endif */

  while ( m_iStackSize && (GetCurElement() == CXmlWriter::Tag) )
  {
    WriteEndElement();
  } /*endwhile */

  if ( GetCurElement() == CXmlWriter::Document )
  {
    enum _ElementType Type;
    WCHAR *pName;

    Pop( &Type, &pName );
    free( pName );
  } /* endif */

}

void CXmlWriter::Close()
{
  if ( m_hf )
  {
    fclose( m_hf );
  } /* endif */
}

int CXmlWriter::Push( enum _ElementType type, const WCHAR *name )
{
  struct _Element *pNewElement;

  pNewElement = (struct _Element *)malloc( sizeof(struct _Element) );
  memset( pNewElement, 0, sizeof(struct _Element) );
  pNewElement->ElementType = type;
  pNewElement->pPrev = m_pRoot;
  pNewElement->pName = wcsdup( name );
  m_iStackSize++;
  m_pRoot = pNewElement;

  return( 0 );
}

int CXmlWriter::Pop( enum _ElementType *pType, WCHAR **ppName )
{
  int iResult = 0;
  struct _Element *pElement;

  if ( m_iStackSize && m_pRoot )
  {
    pElement = m_pRoot;
    *pType   = pElement->ElementType;
    *ppName = pElement->pName;
    m_iStackSize--;
    m_pRoot = pElement->pPrev ;
    free( pElement );
  }
  else
  {
    iResult = -1;
  } /* endif */

  return( iResult );
}

CXmlWriter::ElementType CXmlWriter::GetCurElement( void )
{
  enum _ElementType Type;

  if ( m_iStackSize && m_pRoot )
  {
    struct _Element *pElement = m_pRoot;
    Type = pElement->ElementType;
  }
  else
  {
    Type = CXmlWriter::Undefined;
  } /* endif */

  return( Type );
}

// set content flag of current stack element
void CXmlWriter::SetContentFlag( void )
{
  if ( m_iStackSize && m_pRoot )
  {
    struct _Element *pElement = m_pRoot;
    pElement->fContent = TRUE;
  } /* endif */
}

// set content flag of current stack element
BOOL CXmlWriter::GetContentFlag( void )
{
  BOOL fElementContent = TRUE;
  if ( m_iStackSize && m_pRoot )
  {
    fElementContent = m_pRoot->fContent;
  } /* endif */
  return( fElementContent );
}

bool is_utf8_multibyte_code_unit(char c) {
  return ((unsigned char)c) >= 0x80;
}

static size_t code_to_utf8(unsigned char *const buffer, const unsigned int code)
{
    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);            /* 110xxxxx */
        buffer[1] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);           /* 1110xxxx */
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[2] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);           /* 11110xxx */
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);  /* 10xxxxxx */
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);   /* 10xxxxxx */
        buffer[3] = 0x80 | (code & 0x3F);          /* 10xxxxxx */
        return 4;
    }
    return 0;
}

wchar_t utf_to_code(unsigned char* bytes, size_t& size){
    wchar_t res = 0;
    //if(bytes[0] & 128 == 0){
    if(bytes[0] < 128){
      size = 1;
      res = bytes[0] & 0x7F;
    //}else if(bytes[0] & 224 == 192){/* 110xxxxx */
    }else if(bytes[0] < 224){
      size = 2;
      //res = bytes[0]  + bytes[1]*8;
      //res = (bytes[0] & 0x1F) << 5;
      //res = res << 1;
      
      //res += (bytes[1] & 0x3F);
      res = bytes[1];
    //}else if(bytes[0] & 240 == 224){/* 1110xxxx */
    }else if(bytes[0] < 240){
      size = 3;
      //res += (bytes[0] & 0x07) << 12;
      //res += (bytes[1] & 0x3f) << 6;
      //res += (bytes[2] & 0x3f);
    //}else if(bytes[0] & 248 == 240){/* 11110xxx */
    }else if(bytes[0] < 248){
      size = 4;
      //res += (bytes[0] & 0x03) << 18;
      //res += (bytes[1] & 0x3f) << 12;
      //res += (bytes[2] & 0x3f) << 6;
      //res += (bytes[3] & 0x3f);
    }else{
      T5LOG(T5ERROR) << "Wrong utf8 code "<< bytes[0];
      size = 1;
      //return bytes[0] & 0x7F;
    }
    //for(int i =0;i<size; i++){
    //  res += bytes[size-i-1] << 8*i;
    //}
    return res;
}

wchar_t utf8_to_code(unsigned char* bytes, size_t size){
  wchar_t res =0;
  //reverse list
  unsigned char* tmp = new unsigned char[size];
  for(int i=0;i<size;i++){
    tmp[size-i-1] = bytes[i];
  }
  for(int i=0;i<size;i++){
    //res += bytes[i] << (i*6) & 0x3F;
    //res += (bytes[i]  << (i*6));
    res += (tmp[i]  << (i*6));
  }
  delete[] tmp;
  return res;
}


// convert given string to Unicode, dynamically allocate buffer for Unicode string 
void CXmlWriter::AnsiToUnicode( const char *pAnsiText, WCHAR **ppUnicodeText )
{  
  T5LOG(T5TRANSACTION) << "pAnsiText = " <<pAnsiText;
  if(!strncmp("Kit_", pAnsiText, 4)){
    T5LOG(T5WARNING)<< "now would crash "<< pAnsiText;
  }
  std::string str(pAnsiText);
  memcpy(&str[0], pAnsiText, str.size());
  //char8_t* pc = (char8_t*)pAnsiText;
  
  std::wstring wstr = EncodingHelper::convertToUTF16(str);
  //int iUnicodeLen = (wstr.size()+1) * sizeof(WCHAR);//sizeof(WCHAR) * strlen(pAnsiText) + 10;
  size_t iUnicodeLen = str.size()+1;
  WCHAR *pBuffer = (WCHAR *)malloc( iUnicodeLen* sizeof(WCHAR) );
  memset(pBuffer, 0, sizeof(WCHAR) * iUnicodeLen);
  //wcsncpy(pBuffer, wstr.c_str(), size);
  //char* psymb = (char*) pBuffer;
  //for(int i = 0, j = 0; i < iUnicodeLen ; i++){
    /*
    *psymb = str[i];
    if(is_utf8_multibyte_code_unit(str[i])){
      //copy up to 4 bytes      
      psymb++;
    }else{
      //if(psymb == (char*)&pBuffer[j]){
      //  *psymb = str[i];
      //}
      //j++;
      psymb = (char*)&pBuffer[++j];
      
    }//*/
  //}
  T5LOG(T5TRANSACTION) << str;
  /*
  for(int i=0, j=0; i<iUnicodeLen-1; j++ ){
    psymb = (char*) &pBuffer[j];
    *psymb = str[i];
    while( i<iUnicodeLen-1 && is_utf8_multibyte_code_unit(str[++i])){
      *psymb = str[i];
      psymb ++;
    }
    /*do{
      *psymb = str[i];
      psymb++;
    }while(is_utf8_multibyte_code_unit(str[i++]))
    //*/
  //}

  /*
  unsigned char uc_buff[4];
  for(int i=0, j=0; i<iUnicodeLen-1; j++){
    int k=0;
    while(i<iUnicodeLen-1 && k<4 && is_utf8_multibyte_code_unit(str[i])){
      uc_buff[k++] = str[i++];
    }
    if(k==0){ 
      psymb = (char*) &pBuffer[j];
      *psymb = str[i++];
    }
    else{ 
      pBuffer[j] = utf8_to_code(uc_buff, k);
    }
  }//*/
  /*
  for(int i=0, j=0; i<iUnicodeLen-1; j++){
    size_t bytesFilled = 0;
    pBuffer[j] = utf_to_code((unsigned char*)pAnsiText + i, bytesFilled);
    i+=bytesFilled;
  }//*/
  //auto str32 = EncodingHelper::convertToUTF32(str);
  wcscpy(pBuffer, wstr.c_str());
  //pBuffer[wstr.size()] = 0;
  *ppUnicodeText = pBuffer;
}
