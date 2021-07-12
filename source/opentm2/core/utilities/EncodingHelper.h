#ifndef _ENCODING_HELPER_H_
#define _ENCODING_HELPER_H_

#include <string>


class EncodingHelper{
    public:
    /*! \brief convert a UTF8 std::string to a UTF16 std::wstring
    \param strUTF8String string in UTF8 encoding
    \returns string converted to UTF16
    */
    std::wstring static convertToUTF16(const std::string& strUTF8String);

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

    /*! \brief convert a UTF8 std::string to a ASCII std::string (on spot conversion)
    \param strText on input string in UTF8 encoding, on output string in ASCII encoding
    */
    void static convertUTF8ToASCII( std::string& strText );

    /*! \brief convert a ASCII std::string to UTF8 std::string (on spot conversion)
    \param strText on input string in ASCII encoding, on output string in UTF8 encoding
    */
    void static convertASCIIToUTF8( std::string& strText );

    std::u16string static toLower(const std::u16string& strText);
};

#endif
