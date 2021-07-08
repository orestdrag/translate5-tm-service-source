#include "OSWrapper.h"
#include "LogWrapper.h"
#include <stdlib.h>

int GetLocaleInfo(
        LCID   Locale,
        LCTYPE LCType,
        LPSTR  lpLCData,
        int    cchData
    ){
        
        return 0;
    }

UINT GetOEMCP(){
    return 0;

}


unsigned long int _ttol(const char* source){
    return atoi(source);
}
