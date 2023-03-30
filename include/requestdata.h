#ifndef _REQUEST_DATA_H_INCLUDED_
#define _REQUEST_DATA_H_INCLUDED_

#include <string>
#include <memory>
#include "tm.h"

/*! \brief Error return codes
  */
#define ERROR_INTERNALFUNCTION_FAILED  14001
#define ERROR_INPUT_PARMS_INVALID      14002
#define ERROR_TOO_MANY_OPEN_MEMORIES   14003
#define ERROR_MEMORY_NOT_FOUND         14004


class RequestData{
    int requestType = 0;

protected:
    //RequestData(); // json was parsed in sub class
public:
    std::string strMemName;
    std::string strSrcLang;

    //requests field
    std::string strUrl;
    std::string strBody;


    //return fields
    std::string outputMessage;
    std::string errorMsg;
    int _rc_; 
    int _rest_rc_;
    //timestamp
    //RequestData(std::string json);
    std::weak_ptr <OtmMemory> memory;


    int buildErrorReturn
    (
        int iRC,
        wchar_t *pszErrorMsg
    );

    int buildErrorReturn
        (
        int iRC,
        char *pszErrorMsg
    );

};


class CreateMemRequestData: public RequestData{
    public:
    std::string strMemDescription;
    std::string strMemB64EncodedData; // for import in internal format
    
    CreateMemRequestData(std::string json);
    
};


#endif //_REQUEST_DATA_H_INCLUDED_