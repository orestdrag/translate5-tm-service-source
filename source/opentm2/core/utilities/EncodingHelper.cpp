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


std::string to_utf8(const std::u16string &s)
{
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
    return conv.to_bytes(s);
}

std::string to_utf8(const std::u32string &s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

std::u16string to_utf16(const std::string &s)
{
    std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
    return convert.from_bytes(s);
}

std::u32string to_utf32(const std::string &s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

std::u16string to_utf16(const std::u32string &s)
{
    return to_utf16(to_utf8(s));
}

std::u32string to_utf32(const std::u16string &s) {
    return to_utf32(to_utf8(s));
}


std::wstring read_with_bom(std::istream & src)
{

    enum encoding {
        encoding_utf32be = 0,
        encoding_utf32le,
        encoding_utf16be,
        encoding_utf16le,
        encoding_utf8,
        encoding_ascii,
    };

    std::vector<std::string> boms = {
        std::string("\x00\x00\xFE\xFF", 4),
        std::string("\xFF\xFE\x00\x00", 4),
        std::string("\xFE\xFF", 2),
        std::string("\xFF\xFE", 2),
        std::string("\xEF\xBB\xBF", 3)
    };

    std::string buffer((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());

    encoding enc = encoding_ascii;

    for (unsigned int i = 0; i < boms.size(); ++i) {
        std::string testBom = boms[i];
        if (buffer.compare(0, testBom.length(), testBom) == 0) {
            enc = encoding(i);
            buffer = buffer.substr(testBom.length());
            break;
        }
    }

    switch (enc) {
    case encoding_utf32be:
    {
        if (buffer.length() % 4 != 0) {
            throw std::logic_error("size in bytes must be a multiple of 4");
        }
        int count = buffer.length() / 4;
        std::wstring temp = std::wstring(count, 0);
        for (int i = 0; i < count; ++i) {
            temp[i] = static_cast<char32_t>(buffer[i * 4 + 3] << 0 | buffer[i * 4 + 2] << 8 | buffer[i * 4 + 1] << 16 | buffer[i * 4 + 0] << 24);
        }
        return temp;
    }
    case encoding_utf32le:
    {
        if (buffer.length() % 4 != 0) {
            throw std::logic_error("size in bytes must be a multiple of 4");
        }
        int count = buffer.length() / 4;
        std::wstring temp = std::wstring(count, 0);
        for (int i = 0; i < count; ++i) {
            temp[i] = static_cast<char32_t>(buffer[i * 4 + 0] << 0 | buffer[i * 4 + 1] << 8 | buffer[i * 4 + 2] << 16 | buffer[i * 4 + 3] << 24);
        }
        return temp;
    }
    /*
    case encoding_utf16be:
    {
        if (buffer.length() % 2 != 0) {
            throw std::logic_error("size in bytes must be a multiple of 2");
        }
        int count = buffer.length() / 2;
        std::u16string temp = std::u16string(count, 0);
        for (int i = 0; i < count; ++i) {
            temp[i] = static_cast<char16_t>(buffer[i * 2 + 1] << 0 | buffer[i * 2 + 0] << 8);
        }
        return to_utf32(temp);
    }
    case encoding_utf16le:
    {
        if (buffer.length() % 2 != 0) {
            throw std::logic_error("size in bytes must be a multiple of 2");
        }
        int count = buffer.length() / 2;
        std::u16string temp = std::u16string(count, 0);
        for (int i = 0; i < count; ++i) {
            temp[i] = static_cast<char16_t>(buffer[i * 2 + 0] << 0 | buffer[i * 2 + 1] << 8);
        }
        return to_utf32(temp);
    }//*/
    default:
        return std::wstring();//to_utf32(buffer);
    }
}

std::wstring EncodingHelper::convertToUTF32(std::string inputStr ){
  std::stringstream ss;
  ss<<inputStr;
  
  return read_with_bom(ss);
}

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
*/
std::wstring EncodingHelper::convertToWChar(std::string strUTF8String )
{ 
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
  std::wstring wstr = convert.from_bytes( strUTF8String );
	return wstr;
}

std::u16string EncodingHelper::toUtf16(std::wstring wstr){
  std::string u8str = EncodingHelper::convertToUTF8(wstr);
  return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8str);
}


std::wstring EncodingHelper::toWChar(std::u16string u16str){
  auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  return convertToWChar(u8str.c_str());
}


//std::locale loc(
//      std::locale(),
//      new std::codecvt_utf16<wchar_t, 0x10FFFF, std::consume_header>);

std::string EncodingHelper::toChar(std::u16string u16str){
  auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t,  0x10FFFF,std::consume_header>, char16_t>{}.to_bytes(u16str);
  //auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  return u8str;
}

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
\param strUTF8String string in UTF8 encoding
\returns string converted to UTF16
*/
std::string EncodingHelper::convertToUTF8(const std::wstring& strUTF32String)
{
  std::u16string u16str( strUTF32String.begin(), strUTF32String.end() );
  return convertToUTF8(u16str);
}

std::string EncodingHelper::convertToUTF8(const wchar_t* UTF32String)
{
  std::wstring wstr(UTF32String);
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
  //T5LOG( T5WARNING) <<"EncodingHelper::convertUTF8ToASCII( std::string& strText ) is not implemented, strText = ", strText.c_str());
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
