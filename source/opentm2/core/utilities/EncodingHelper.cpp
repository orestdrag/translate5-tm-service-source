#include "EncodingHelper.h"
#include "LogWrapper.h"


#include <codecvt>
#include <locale>
template <class Facet>
struct deletable_facet : Facet{
  using Facet::Facet;//inherit constructors
  ~deletable_facet(){};
};

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
*/
std::wstring EncodingHelper::convertToUTF16( const std::string& strUTF8String )
{ 
  std::wstring_convert<deletable_facet<std::codecvt<wchar_t, char, std::mbstate_t>>> wconv;
  std::wstring wstr = wconv.from_bytes(strUTF8String);
	return wstr;
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

std::string EncodingHelper::convertToUTF8( const std::u16string& strUTF16String ){
    std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(strUTF16String);

	return u8_conv;
}


/*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
\param strText on input string in UTF8 encoding, on output string in ASCII encoding
\returns string converted to UTF16
*/
void EncodingHelper::convertUTF8ToASCII( std::string& strText )
{
  LogMessage2(WARNING,"EncodingHelper::convertUTF8ToASCII( std::string& strText ) is not implemented, strText = ", strText.c_str());
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
  int iUTF16Len;
  int iASCIILen;
  int iUTF8Len = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_UTF8, 0, strText.c_str(), iUTF8Len, &strUTF16[0], iUTF16Len );
  iASCIILen = WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iASCIILen + 1 );
  WideCharToMultiByte( CP_OEMCP, 0, strUTF16.c_str(), iUTF16Len, pszNewData, iASCIILen, 0, 0 );
  strText = pszNewData;
  free( pszNewData );
  #endif //TO_BE_REPLACED_WITH_LINUX_CODE
}

/*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
\param strText on input string in ASCII encoding, on output string in UTF8 encoding
*/
void EncodingHelper::convertASCIIToUTF8( std::string& strText )
{
  LogMessage2(WARNING,"EncodingHelper::convertASCIIToUTF8( std::string& strText ) is not implemented, strText = ", strText.c_str());

#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
  int iUTF16Len;
  int iUTF8Len;
  int iASCIILen = (int)strText.length() + 1;
  char *pszNewData = NULL;
  iUTF16Len = MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, 0, 0 );
  std::wstring strUTF16( iUTF16Len, L'\0' );
  MultiByteToWideChar( CP_OEMCP, 0, strText.c_str(), iASCIILen, &strUTF16[0], iUTF16Len );
  iUTF8Len = WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, 0, 0, 0, 0 );
  pszNewData = (char *)malloc( iUTF8Len + 1);
  WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iUTF16Len, pszNewData, iUTF8Len, 0, 0 );

  strText = pszNewData;
  free( pszNewData );
  #endif //TO_BE_REPLACED_WITH_LINUX_CODE
}
