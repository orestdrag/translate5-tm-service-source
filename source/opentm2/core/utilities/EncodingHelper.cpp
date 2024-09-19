#include "EncodingHelper.h"
#include "LogWrapper.h"
#include "OSWrapper.h"

#include "Base64.h"

#include <codecvt>
#include <locale>

#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
XERCES_CPP_NAMESPACE_USE


std::u16string utf32ToUtf16(const std::u32string& utf32Str) {
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t utf16Length = 0;
    
    // First, find out the size of the UTF-16 buffer needed
    u_strFromUTF32(nullptr, 0, &utf16Length, reinterpret_cast<const UChar32*>(utf32Str.data()), utf32Str.size(), &errorCode);
    if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
        throw std::runtime_error("Error determining UTF-16 buffer size");
    }

    errorCode = U_ZERO_ERROR;
    std::u16string utf16Str(utf16Length, 0);
    
    // Convert UTF-32 to UTF-16
    u_strFromUTF32(reinterpret_cast<UChar*>(&utf16Str[0]), utf16Length, nullptr, reinterpret_cast<const UChar32*>(utf32Str.data()), utf32Str.size(), &errorCode);
    if (U_FAILURE(errorCode)) {
        throw std::runtime_error("Error converting UTF-32 to UTF-16");
    }
    
    return utf16Str;
}
std::u32string utf16ToUtf32(const std::u16string& utf16Str) {
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t utf32Length = 0;
    
    // First, find out the size of the UTF-32 buffer needed
    u_strToUTF32(nullptr, 0, &utf32Length, reinterpret_cast<const UChar*>(utf16Str.c_str()), utf16Str.size(), &errorCode);
    if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
        throw std::runtime_error("Error determining UTF-32 buffer size");
    }

    errorCode = U_ZERO_ERROR;
    std::u32string utf32Str(utf32Length, 0);
    
    // Convert UTF-16 to UTF-32
    u_strToUTF32(reinterpret_cast<UChar32*>(&utf32Str[0]), utf32Length, nullptr, reinterpret_cast<const UChar*>(utf16Str.c_str()), utf16Str.size(), &errorCode);
    if (U_FAILURE(errorCode)) {
        throw std::runtime_error("Error converting UTF-16 to UTF-32");
    }
    
    return utf32Str;
}

std::string utf32ToUtf8(const std::u32string& utf32Str) {
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t utf8Length = 0;
    
    // First, find out the size of the UTF-8 buffer needed
    u_strToUTF8(nullptr, 0, &utf8Length, reinterpret_cast<const UChar*>(utf32Str.data()), utf32Str.size(), &errorCode);
    if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
        throw std::runtime_error("Error determining UTF-8 buffer size");
    }

    errorCode = U_ZERO_ERROR;
    std::string utf8Str(utf8Length, 0);
    
    // Convert UTF-32 to UTF-8
    u_strToUTF8(&utf8Str[0], utf8Length, nullptr, reinterpret_cast<const UChar*>(utf32Str.data()), utf32Str.size(), &errorCode);
    if (U_FAILURE(errorCode)) {
        throw std::runtime_error("Error converting UTF-32 to UTF-8");
    }
    
    return utf8Str;
}

std::u32string utf8ToUtf32(const std::string& utf8Str) {
    UErrorCode errorCode = U_ZERO_ERROR;
    int32_t utf32Length = 0;
    
    // First, find out the size of the UTF-32 buffer needed
    u_strFromUTF8(nullptr, 0, &utf32Length, utf8Str.c_str(), utf8Str.size(), &errorCode);
    if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
        throw std::runtime_error("Error determining UTF-32 buffer size");
    }

    errorCode = U_ZERO_ERROR;
    std::vector<UChar32> utf32Buffer(utf32Length);
    
    // Convert UTF-8 to UTF-32
    u_strFromUTF8(reinterpret_cast<UChar*>(utf32Buffer.data()), utf32Length, nullptr, utf8Str.c_str(), utf8Str.size(), &errorCode);
    if (U_FAILURE(errorCode)) {
        throw std::runtime_error("Error converting UTF-8 to UTF-32");
    }
    
    return std::u32string(utf32Buffer.begin(), utf32Buffer.end());
}

std::string utf16ToUtf8(const std::u16string& utf16Str) {
  UErrorCode errorCode = U_ZERO_ERROR;
  int32_t utf8Length = 0;
   
  // First, find out the size of the UTF-8 buffer needed
  u_strToUTF8(nullptr, 0, &utf8Length, reinterpret_cast<const UChar*>(utf16Str.c_str()), utf16Str.size(), &errorCode);
  if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
    throw std::runtime_error("Error determining UTF-8 buffer size");
  }

  errorCode = U_ZERO_ERROR;
  std::string utf8Str(utf8Length, 0);
   
  // Convert UTF-16 to UTF-8
  u_strToUTF8(&utf8Str[0], utf8Length, nullptr, reinterpret_cast<const UChar*>(utf16Str.c_str()), utf16Str.size(), &errorCode);
  if (U_FAILURE(errorCode)) {
      throw std::runtime_error("Error converting UTF-16 to UTF-8");
  }
    
  return utf8Str;
}

std::u16string utf8ToUtf16(const std::string& utf8Str) {
  UErrorCode errorCode = U_ZERO_ERROR;
  int32_t utf16Length = 0;
    
    // First, find out the size of the UTF-16 buffer needed
  u_strFromUTF8(nullptr, 0, &utf16Length, utf8Str.c_str(), utf8Str.size(), &errorCode);
  if (errorCode != U_BUFFER_OVERFLOW_ERROR) {
      throw std::runtime_error("Error determining UTF-16 buffer size");
  }

  errorCode = U_ZERO_ERROR;
  std::u16string utf16Str(utf16Length, 0);
  
  // Convert UTF-8 to UTF-16
  u_strFromUTF8(reinterpret_cast<UChar*>(&utf16Str[0]), utf16Length, nullptr, utf8Str.c_str(), utf8Str.size(), &errorCode);
  if (U_FAILURE(errorCode)) {
      throw std::runtime_error("Error converting UTF-8 to UTF-16");
  }
    
  return utf16Str;
}

#include <unicode/unistr.h>  // For ICU's UnicodeString
#include <string>
#include <unicode/ustring.h>

std::string icu_wstring_to_utf8(const std::wstring& wstr) {
  icu::UnicodeString ustr;
  UErrorCode error = U_ZERO_ERROR;

  // Step 1: Obtain the size of the buffer required for the UnicodeString.
  int32_t requiredSize;
  u_strFromUTF32(nullptr, 0, &requiredSize, reinterpret_cast<const UChar32*>(wstr.c_str()), wstr.length(), &error);
  if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
    // Handle the error (e.g., log or throw an exception)
    return {};
  }

  // Step 2: Allocate the buffer inside the UnicodeString.
  auto buff = ustr.getBuffer(requiredSize);

  // Step 3: Convert std::wstring (UTF-32) to ICU's UnicodeString (UTF-16).
  error = U_ZERO_ERROR;
  u_strFromUTF32(buff, requiredSize, nullptr, (const UChar32*)wstr.c_str(), wstr.length(), &error);
  if (U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
    // Handle the error (e.g., log or throw an exception)
    return {};
  }

  // Step 4: Convert the ICU UnicodeString to a UTF-8 std::string.
  std::string res;
  ustr.releaseBuffer(requiredSize);  // Finalize the UnicodeString buffer
  ustr.toUTF8String(res);

  return res;
}


std::wstring icu_utf8_to_wstring(const std::string& utf8_str) {
  UErrorCode errorCode = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(utf8_str);
  
  std::wstring wstr;

  int32_t requiredSize;
  UErrorCode error = U_ZERO_ERROR;

  // obtain the size of string we need
  u_strToWCS(nullptr, 0, &requiredSize, ustr.getBuffer(), ustr.length(), &error);

  if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize <<"; src = "<< utf8_str;    
    // Handle error if needed
    return {};
  }

  // resize accordingly (this will not include any terminating null character, but it also doesn't need to either)
  wstr.resize(requiredSize);
  error = U_ZERO_ERROR;

  // copy the UnicodeString buffer to the std::wstring.
  u_strToWCS((wchar_t*)wstr.data(), wstr.size(), nullptr, ustr.getBuffer(), ustr.length(), &error);
  if(U_FAILURE(error)){
    T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize <<"; src = "<< utf8_str;  
  }

  return wstr;
}

std::u16string icu_wstring_to_utf16(const std::wstring& wstr) {
  UErrorCode error = U_ZERO_ERROR;
  int32_t requiredSize;

  // Step 1: Determine the size of the resulting buffer for the UTF-16 data.
  u_strFromUTF32(nullptr, 0, &requiredSize, (const UChar32*)wstr.c_str(), wstr.length(), &error);

  if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;    
    // Handle error if needed
    return {};
  }

  // Step 2: Create a buffer to store the UTF-16 string.
  std::u16string utf16Str(requiredSize, 0);

  // Step 3: Convert the std::wstring (UTF-32) to UTF-16.
  error = U_ZERO_ERROR;
  u_strFromUTF32((UChar*)utf16Str.data(), utf16Str.size(), nullptr,
                 (const UChar32*)wstr.c_str(), wstr.length(), &error);

  if (U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
    // Handle error if needed
    return {};
  }

  return utf16Str;
}

std::wstring icu_utf16_to_wstring(const std::u16string& u16str) {
  UErrorCode error = U_ZERO_ERROR;
  int32_t requiredSize;

  // Step 1: Determine the size of the resulting buffer for the UTF-32 data.
  u_strToUTF32(nullptr, 0, &requiredSize, (const UChar*)u16str.c_str(), u16str.length(), &error);
  if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
    // Handle error if needed
    return {};
  }

  // Step 2: Create a buffer to store the UTF-32 (std::wstring).
  std::wstring wstr(requiredSize, 0);

  // Step 3: Convert the UTF-16 (std::u16string) to UTF-32 (std::wstring).
  error = U_ZERO_ERROR;
  u_strToUTF32((UChar32*)wstr.data(), wstr.size(), nullptr,
              (UChar*)u16str.c_str(), u16str.length(), &error);
  if (U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
    // Handle error if needed    
    return {};
  }

  return wstr;
}


std::u16string icu_string_to_utf16(const std::string& str) {
  UErrorCode error = U_ZERO_ERROR;
  int32_t requiredSize;

  // Step 1: Determine the size of the resulting buffer for the UTF-16 data.
  u_strFromUTF8(nullptr, 0, &requiredSize, str.c_str(), str.length(), &error);
  if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize <<"; src = "<< str;        
    // Handle error if needed
    return {};
  }

  // Step 2: Create a buffer to store the UTF-16 (std::u16string).
  std::u16string u16str(requiredSize, 0);

  // Step 3: Convert the UTF-8 (std::string) to UTF-16 (std::u16string).
  error = U_ZERO_ERROR;
  u_strFromUTF8((UChar*)u16str.data(), u16str.size(), nullptr, str.c_str(), str.length(), &error);
  if (U_FAILURE(error)) {
    T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize <<"; src = "<< str;
    // Handle error if needed
    return {};
  }

  return u16str;
}


std::string icu_utf16_to_string(const std::u16string& u16str) {
    UErrorCode error = U_ZERO_ERROR;
    int32_t requiredSize;

    // Step 1: Determine the size of the resulting buffer for the UTF-8 data.
    u_strToUTF8(nullptr, 0, &requiredSize, (const UChar*)u16str.c_str(), u16str.length(), &error);
    if (U_BUFFER_OVERFLOW_ERROR != error && U_FAILURE(error)) {
        // Handle error if needed
      T5LOG(T5ERROR)<<"Convertion failed before buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
      return {};
    }

    // Step 2: Create a buffer to store the UTF-8 (std::string).
    std::string str(requiredSize, 0);

    // Step 3: Convert the UTF-16 (std::u16string) to UTF-8 (std::string).
    error = U_ZERO_ERROR;
    u_strToUTF8((char*)str.data(), str.size(), nullptr, (const UChar*)u16str.c_str(), u16str.length(), &error);
    if (U_FAILURE(error)) {
      T5LOG(T5ERROR)<<"Convertion failed after buffer allocation, errCode = " << error << ", errName = " << u_errorName(error) << "; requiredSize = " << requiredSize;//<<"; src = "<< src;
        
      // Handle error if needed
      return {};
    }

    return str;
}

/*
std::string utf16_to_utf8(const std::u16string& utf16_str) {
    icu::UnicodeString icu_str(reinterpret_cast<const UChar*>(utf16_str.data()), utf16_str.length());
    std::string utf8_str;
    icu_str.toUTF8String(utf8_str);
    return utf8_str;
}


std::u16string utf8_to_utf16(const std::string& utf8_str) {
    icu::UnicodeString icu_str = icu::UnicodeString::fromUTF8(utf8_str);
    std::u16string utf16_str;
    icu_str.toUTF16(utf16_str);
    return utf16_str;
}

std::string utf32_to_utf8(const std::u32string& utf32_str) {
    icu::UnicodeString icu_str(reinterpret_cast<const UChar*>(utf32_str.data()), utf32_str.length());
    std::string utf8_str;
    icu_str.toUTF8String(utf8_str);
    return utf8_str;
}

/*
std::u32string utf8_to_utf32(const std::string& utf8_str) {
    icu::UnicodeString icu_str = icu::UnicodeString::fromUTF8(utf8_str);
    std::u32string utf32_str;
    icu_str.toUTF32String(utf32_str);
    return utf32_str;
}
//*/
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


bool EncodingHelper::endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.rfind(suffix) == str.size() - suffix.size();
}

std::string EncodingHelper::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool EncodingHelper::endsWithIgnoreCase(const std::string& str, const std::string& suffix) {
    std::string lowerStr = toLowerCase(str);
    std::string lowerSuffix = toLowerCase(suffix);
    return lowerStr.size() >= lowerSuffix.size() && lowerStr.rfind(lowerSuffix) == lowerStr.size() - lowerSuffix.size();
}

/*
std::string to_utf8(const std::u16string &s)
{
  std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
  std::string res;
  try {
    //res = utf16ToUtf8(s);
    res = conv.to_bytes(s);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'" //<< s 
      << "\' : " << e.what() << std::endl;
  }
  return res;
//return conv.to_bytes(s);
}

std::string to_utf8(const std::u32string &s)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  std::string res;
  try {
    res = conv.to_bytes(s);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'" //<< s 
      << "\' : " << e.what() << std::endl;
  }
  return res;
  //return conv.to_bytes(s);
}

std::u16string to_utf16(const std::string &s)
{
  std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> conv;
  std::u16string res;
  try {
    res = conv.from_bytes(s);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'" << s<< "\' : " << e.what() << std::endl;
  }
  return res;
  //return convert.from_bytes(s);
}

std::u32string to_utf32(const std::string &s)
{

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  try {
      return conv.from_bytes(s);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'" << s<< "\' : " << e.what() << std::endl;
    return u32string(); // Return an empty wide string on error
  }
  //std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  //return conv.from_bytes(s);
}

std::u16string to_utf16(const std::u32string &s)
{
  return to_utf16(to_utf8(s));
}

std::u32string to_utf32(const std::u16string &s) {
  return to_utf32(to_utf8(s));
}
//*/


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
std::wstring EncodingHelper::convertToWChar(std::string s )
{ 
  //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  std::wstring res;
  try {
    //res = conv.from_bytes(s);
    res = icu_utf8_to_wstring(s);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'" << s 
      << "\' : " << e.what() << std::endl;
  }
  return res;
  //std::wstring wstr = convert.from_bytes( strUTF8String );
	//return wstr;
}

std::u16string EncodingHelper::toUtf16(std::wstring wstr){
  ///std::string u8str = EncodingHelper::convertToUTF8(wstr);
  //std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  std::u16string res;
  try {
    res = icu_wstring_to_utf16(wstr);
    //res = conv.from_bytes(u8str);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'wstr"// << u8str 
      << "\' : " << e.what() << std::endl;
  }
  return res;
 // return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8str);
}


std::wstring EncodingHelper::toWChar(std::u16string u16str){
  //std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  std::wstring wstr;
  try {
    wstr = icu_utf16_to_wstring(u16str);
    //str = conv.to_bytes(u16str);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'16str"// << u8str 
      << "\' : " << e.what() << std::endl;
  }
  //return res;
  //auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  //return convertToWChar(str.c_str());
  return wstr;
}


//std::locale loc(
//      std::locale(),
//      new std::codecvt_utf16<wchar_t, 0x10FFFF, std::consume_header>);

std::string EncodingHelper::toChar(std::u16string u16str){
  //auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t,  0x10FFFF,std::consume_header>, char16_t>{}.to_bytes(u16str);
  //std::wstring_convert<std::codecvt_utf8_utf16<char16_t,  0x10FFFF,std::consume_header>, char16_t> conv;
  std::string res;
  try {
    res = icu_utf16_to_string(u16str);
    //res = conv.to_bytes(u16str);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'u16str"// << u8str 
      << "\' : " << e.what() << std::endl;
  }
  return res;
  //auto u8str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str);
  //return u8str;
}

/*! \brief convert a UTF8 std::string to a UTF16 std::wstring
\param strUTF8String string in UTF8 encoding
\returns string converted to UTF16
*/
std::string EncodingHelper::convertToUTF8(const std::wstring& strUTF32String)
{
  std::string res;
  try {
    //res = conv.from_bytes(s);
    res = icu_wstring_to_utf8(strUTF32String);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'utf32"// << s 
      << "\' : " << e.what() << std::endl;
  }
  return res;
  //std::wstring wstr = convert.from_bytes( strUTF8String );
	//return wstr;
  //std::u16string u16str( strUTF32String.begin(), strUTF32String.end() );
  //return convertToUTF8(u16str);
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
  //std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(strUTF16String);
  //std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  std::string res;
  try {
    res = icu_utf16_to_string(strUTF16String);
    //res = conv.to_bytes(strUTF16String);
  } catch (const std::range_error& e) {
    //std::cerr << "Conversion error: " << e.what() << std::endl;
    T5LOG(T5ERROR) << "Conversion error:  for \'u16str" //<< u8_conv 
      << "\' : " << e.what() << std::endl;
  }
  return res;
	//return u8_conv;
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
