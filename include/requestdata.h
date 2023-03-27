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
    bool fValid = false;
    std::string strMemName;

    //requests field
    std::string strUrl;
    std::string strBody;


    //return fields
    std::string outputMessage;
    std::string errorMsg;
    int _rc_; 
    int _rest_rc_;
    //timestamp


    RequestData(std::string json, std::string memName): strBody(json), strMemName(memName) {}
    virtual ~RequestData() = default;
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

protected:
    virtual int parseJSON() = 0;
    virtual int checkData() = 0;
};


class CreateMemRequestData: public RequestData{
    protected:
    int parseJSON() override ;
    int checkData() override ;
    public:
    std::string strSrcLang;
    std::string strMemDescription;
    std::string strMemB64EncodedData; // for import in internal format
    
    CreateMemRequestData(std::string json): RequestData(json, ""){
        parseJSON();
        checkData();
    }
    
};

class StatusMemRequestData: public RequestData{
    public:
    StatusMemRequestData(std::string memName): RequestData("", memName){
        parseJSON();
        checkData();
    }
};

class DeleteMemRequestData: public RequestData{
    public:
    DeleteMemRequestData(std::string memName): RequestData("", memName){
        parseJSON();
        checkData();
    }
};


class ConcordanceSearchRequestData: public RequestData{
    private:

    public:
    ConcordanceSearchRequestData(std::string json, std::string memName): RequestData(json, memName) {
        parseJSON();
        checkData();
    }
};


class FuzzySearchRequestData: public RequestData{
    private:
    
    public:
    FuzzySearchRequestData(std::string json, std::string memName): RequestData(json, memName) {
        parseJSON();
        checkData();
    }
};

class DeleteEntryRequestData: public RequestData{
    private:
    
    public:
    DeleteEntryRequestData(std::string json, std::string memName): RequestData(json, memName) {
        parseJSON();
        checkData();
    }
};

class UpdateEntryRequestData: public RequestData{
    private:
  
    public:
    UpdateEntryRequestData(std::string json, std::string memName): RequestData(json, memName) {
        parseJSON();
        checkData();
    }
};


#endif //_REQUEST_DATA_H_INCLUDED_