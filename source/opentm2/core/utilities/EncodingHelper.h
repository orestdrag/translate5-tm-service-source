#ifndef _ENCODING_HELPER_H_
#define _ENCODING_HELPER_H_

#include <string>
#include <vector>


#include <tm.h>

class EncodingHelper{
    public:
    /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
    \param strUTF8String string in UTF8 encoding
    \returns string converted to UTF16
    */
    std::wstring static convertToUTF16(std::string strUTF8String);
    std::u16string static toUtf16(std::wstring wstr);
    std::wstring static toWChar(std::u16string u16str);
    std::string static toChar(std::u16string u16str);
    
    std::wstring static convertToUTF32(std::string str);

    /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
    \param strUTF8String string in UTF8 encoding
    \returns string converted to UTF16
    */
    std::string static convertToUTF8( const std::wstring& strUTF16String );

    /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
    \param strUTF8String string in UTF8 encoding
    \returns string converted to UTF16
    */
    std::string static convertToUTF8( const std::u16string& strUTF16String );
    std::string static convertToUTF8( const wchar_t* UTF16String);

    /*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
    \param strText on input string in UTF8 encoding, on output string in ASCII encoding
    */
    void static convertUTF8ToASCII( std::string& strText );

    /*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
    \param strText on input string in ASCII encoding, on output string in UTF8 encoding
    */
    void static convertASCIIToUTF8( std::string& strText );

    std::u16string static toLower(const std::u16string& strText);

    void static Base64Encode (const unsigned char* input, int inSize, std::string& output);
    void static Base64Decode (const std::string& input, unsigned char *& output, int& output_size);

    static std::wstring EscapeXML( std::wstring input );
    static std::string PreprocessSymbolsForXML(const std::string& inputString);
    static std::wstring PreprocessSymbolsForXML(const std::wstring& inputString);

    static std::string  PreprocessUnicodeInput(const std::string& input);
    static std::wstring PreprocessUnicodeInput(const std::wstring& input);

    static std::string RestoreUnicodeInput(const std::string& preprocessedString);
    static std::wstring RestoreUnicodeInput(const std::wstring& preprocessedString);

    static std::wstring stringToWstring(const std::string& str);
    static std::string wstringToString(const std::wstring& str);

};

#endif
