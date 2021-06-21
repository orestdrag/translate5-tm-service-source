

#ifndef _SYSTEMFUNCTIONSWRAPPER_H_INCLUDED
#define _SYSTEMFUNCTIONSWRAPPER_H_INCLUDED
#include <string>
//#include "win_types.h"

class SystemFunctionsWrapper{
public:
    static bool GetStringFromRegistry( std::string sAppl, std::string sKey, std::string& sResult);
    static bool GetIntFromRegistry   ( std::string sAppl, std::string sKey, int& iResult);
    static bool WriteStringToRegistry( std::string sAppl, std::string sKey, std::string sValue );
    static bool WriteIntToRegistry   ( std::string sAppl, std::string sKey, int iValue );
};

#endif