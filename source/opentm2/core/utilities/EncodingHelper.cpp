#include "EncodingHelper.h"
#include "LogWrapper.h"
#include "OSWrapper.h"

#include "Base64.h"

#include <codecvt>
#include <locale>


#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
XERCES_CPP_NAMESPACE_USE

template <class Facet>
struct deletable_facet : Facet{
  using Facet::Facet;//inherit constructors
  ~deletable_facet(){};
};

void EscapeXMLChars( wchar_t* pszText, wchar_t* pszBuffer );

std::wstring EncodingHelper::EscapeXML( std::wstring input ){
  wchar_t buff[4096];  
  EscapeXMLChars((wchar_t*)input.c_str(), buff);
  return buff;
}


std::vector<std::wstring> ReplaceOriginalTagsWithTagsFromRequestFunc(std::wstring&& w_request, std::wstring &&w_src, std::wstring &&w_trg);
std::vector<std::wstring> ReplaceOriginalTagsWithPlaceholdersFunc(std::wstring &&w_src, std::wstring &&w_trg, bool fSkipAttributes = false);
std::vector<std::wstring> EncodingHelper::ReplaceOriginalTagsWithPlaceholders(std::wstring &&w_src, std::wstring &&w_trg, std::wstring &&w_req)
{
  if(w_req.empty()){
    return ReplaceOriginalTagsWithPlaceholdersFunc(std::move(w_src), std::move(w_trg));
  }
  return ReplaceOriginalTagsWithTagsFromRequestFunc(std::move(w_req),std::move(w_src), std::move(w_trg));
}


/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
*/
std::wstring EncodingHelper::convertToUTF16(std::string&& strUTF8String )
{ 
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
  std::wstring wstr = convert.from_bytes( strUTF8String );
	return wstr;
}

std::u16string EncodingHelper::toUtf16(std::wstring&& wstr){
  std::string u8str = EncodingHelper::convertToUTF8(wstr);
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8str);
}


std::wstring EncodingHelper::toWChar(std::u16string u16str){
  auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  return convertToUTF16(u8str.c_str());
}


std::string EncodingHelper::toChar(std::u16string u16str){
  auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  return u8str;
}

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
\param strUTF8String string in UTF8 encoding
\returns string converted to UTF16
*/
std::string EncodingHelper::convertToUTF8(const std::wstring& strUTF16String)
{
  std::u16string u16str( strUTF16String.begin(), strUTF16String.end() );
  return convertToUTF8(u16str);
}

std::string EncodingHelper::convertToUTF8(const wchar_t* UTF16String)
{
  std::wstring wstr(UTF16String);
  return convertToUTF8(wstr);
}

std::string EncodingHelper::convertToUTF8( const std::u16string& strUTF16String ){
  if(strUTF16String.empty()){
    return {};
  }
  std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(strUTF16String);
	return u8_conv;
}


/*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
\param strText on input string in UTF8 encoding, on output string in ASCII encoding
\returns string converted to UTF16
*/
void EncodingHelper::convertUTF8ToASCII( std::string& strText )
{
  //LogMessage( T5WARNING,"EncodingHelper::convertUTF8ToASCII( std::string& strText ) is not implemented, strText = ", strText.c_str());
  int iUTF16Len;
  int iASCIILen;
  int iUTF8Len = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, &strUTF16[0], iUTF16Len );
  iASCIILen = WideCharToMultiByte( CP_UTF8, 0, (LPWSTR)strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iASCIILen + 1 );
  WideCharToMultiByte( CP_OEMCP, 0, (LPWSTR)strUTF16.c_str(), iUTF16Len, pszNewData, iASCIILen, 0, 0 );
  strText = pszNewData;
  free( pszNewData );
}

/*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
\param strText on input string in ASCII encoding, on output string in UTF8 encoding
*/
void EncodingHelper::convertASCIIToUTF8( std::string& strText )
{
  int iUTF16Len;
  int iUTF8Len;
  int iASCIILen = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, &strUTF16[0], iUTF16Len );
  iUTF8Len = WideCharToMultiByte( CP_UTF8, 0, (LPWSTR)strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iUTF8Len + 1);
  WideCharToMultiByte( CP_UTF8, 0, (LPWSTR)strUTF16.c_str(), iUTF16Len, pszNewData, iUTF8Len, 0, 0 );

  strText = pszNewData;
  free( pszNewData );
}


std::u16string EncodingHelper::toLower(const std::u16string& strText){
    std::u16string ldata;
    ldata = strText;
    /*
    auto const& ct = std::use_facet<std::ctype<char16_t>>(std::locale());
    for (std::u16string::const_iterator it = strText.begin(); it != strText.end(); ++it)
    {
        ldata.push_back( ct.tolower(*it) );
    }//*/
    return ldata;
}



void EncodingHelper::Base64Decode (const std::string& input, unsigned char*& output, int& output_size){
  output = base64_decode(input, output_size);
}

void EncodingHelper::Base64Encode (const unsigned char* input, int inSize, std::string& output){
  output = base64_encode(input, inSize);
}
