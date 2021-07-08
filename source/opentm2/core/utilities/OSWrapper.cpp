#include "OSWrapper.h"
#include "LogWrapper.h"
#include <stdlib.h>
#include <locale.h>
#include <string.h>

int GetLocaleInfo(
        LCID   Locale,
        LCTYPE LCType,
        LPSTR  lpLCData,
        int    cchData
    ){
        if( Locale == LOCALE_USER_DEFAULT){
            switch (LCType)
            {
            case LOCALE_IDEFAULTCODEPAGE:
                strncpy(lpLCData,"866",cchData);
                return 0;
                break;

            case LOCALE_IDEFAULTANSICODEPAGE:
                strncpy(lpLCData,"1251",cchData);
                return 0;
                break;

            default:
                LogMessage3(ERROR, "GetLocaleInfo():: LOCALE_USER_DEFAULT :: LCType {", intToA(LCType), "} is not implemented");
                break;
            }
        }else{
            LogMessage3(ERROR, "GetLocaleInfo():: Locale {", intToA(Locale), "} is not implemented");
        }    
        return -1;
    }

UINT GetOEMCP(){
    return 0;

}


unsigned long int _ttol(const char* source){
    return atoi(source);
}
