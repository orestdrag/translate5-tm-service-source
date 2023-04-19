#ifndef _REQUEST_DATA_H_INCLUDED_
#define _REQUEST_DATA_H_INCLUDED_

#include <string>
#include <memory>
#include "tm.h"

#include "lowlevelotmdatastructs.h"


class EqfMemory;
class JSONFactory;

class RequestData{
protected:
    std::shared_ptr<EqfMemory> mem;
    int requestType = 0;
    static JSONFactory json_factory;
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
    int _rc_=0; 
    int _rest_rc_=0;
    //timestamp


    RequestData(const std::string& json, const std::string& memName): strBody(json), strMemName(memName) {}
    virtual ~RequestData() = default;
    //std::weak_ptr <EqfMemory> memory;

    int buildErrorReturn(int iRC, wchar_t *pszErrorMsg);

    int buildErrorReturn(int iRC,char *pszErrorMsg);

    
    int run();
protected:
    virtual int parseJSON() = 0;
    virtual int checkData() = 0;
    virtual int execute() = 0;
};



class CreateMemRequestData: public RequestData{
    protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute() override ;

    int importInInternalFomat();
    int createNewEmptyMemory();

  
    public:
    
    std::string strSrcLang;
    std::string strMemDescription;
    std::string strMemB64EncodedData; // for import in internal format
    
    CreateMemRequestData(const std::string& json): RequestData(json, ""){}
    
};

class StatusMemRequestData: public RequestData{
    public:
    StatusMemRequestData(const std::string& memName): RequestData("", memName){};
};

class DeleteMemRequestData: public RequestData{
    public:
    DeleteMemRequestData(const std::string& memName): RequestData("", memName){};
};


class ConcordanceSearchRequestData: public RequestData{
    public:
    ConcordanceSearchRequestData(const std::string& json, const std::string& memName): RequestData(json, memName){};
};


class FuzzySearchRequestData: public RequestData{    
    public:
    FuzzySearchRequestData(const std::string& json, const std::string& memName): RequestData(json, memName){};
};

class DeleteEntryRequestData: public RequestData{
    private:
    
    public:
    DeleteEntryRequestData(const std::string& json, const std::string& memName): RequestData(json, memName) {};
};

class UpdateEntryRequestData: public RequestData{
    private:
  
    public:
    UpdateEntryRequestData(const std::string& json, const std::string& memName): RequestData(json, memName) {};
    int parseJSON() override ;
    int checkData() override ;
    int execute() override ;
};

class ExportRequestData: public RequestData{
    private:
    int ExportZip();
    int ExportTmx();
    
    std::string requestFormat;//application/xml or application/binary
    FCTDATA fctdata;
    MEM_EXPORT_IDA IDA;         // Pointer to the export IDA
    PROCESSCOMMAREA CommArea;   // ptr to commmunication area

    USHORT PrepExport();
    USHORT ExportProc();
    std::string strTempFile;
    public:
    ExportRequestData(const std::string& format, const std::string& memName): RequestData("", memName),  requestFormat(format){};
    int parseJSON() override {return 0;};
    int checkData() override ;
    int execute() override ;

};

#endif //_REQUEST_DATA_H_INCLUDED_