#ifndef _REQUEST_DATA_H_INCLUDED_
#define _REQUEST_DATA_H_INCLUDED_

#include <string>
#include <memory>
#include "tm.h"

#include "lowlevelotmdatastructs.h"

    
     enum COMMAND {
        UNKNOWN_COMMAND,
        LIST_OF_MEMORIES,
        SAVE_ALL_TM_ON_DISK,
        SHUTDOWN,    
        DELETE_MEM,
        EXPORT_MEM,
        EXPORT_MEM_INTERNAL_FORMAT,
        REORGANIZE_MEM,
        STATUS_MEM,
        RESOURCE_INFO,

        START_COMMANDS_WITH_BODY,
        CREATE_MEM = START_COMMANDS_WITH_BODY, 
        FUZZY,
        CONCORDANCE,
        DELETE_ENTRY,
        UPDATE_ENTRY,
        TAGREPLACEMENTTEST,
        IMPORT_MEM,
        CLONE_TM_LOCALY,
        //IMPORT_MEM_INTERNAL_FORMAT
    };

class EqfMemory;
class JSONFactory;

class RequestData{
protected:
    std::shared_ptr<int> memRef;
    int loggingThreshold = -1;
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
    std::string requestAcceptHeader;


    //return fields
    std::string outputMessage;
    std::string errorMsg;
    int _rc_=0; 
    int _rest_rc_=0;
    //timestamp


    RequestData(COMMAND cmnd, const std::string& json, const std::string& memName): strBody(json), strMemName(memName), command(cmnd) {}
    RequestData(COMMAND cmnd): command(cmnd){}
    virtual ~RequestData() = default;
    //std::weak_ptr <EqfMemory> memory;

    int buildErrorReturn(int iRC, wchar_t *pszErrorMsg);

    int buildErrorReturn(int iRC,char *pszErrorMsg);


    COMMAND command = UNKNOWN_COMMAND;
    
    int run();
protected:
    virtual int parseJSON() = 0;
    virtual int checkData() = 0;
    virtual int execute() = 0;
    bool getValue( char *pszString, int iLen, int *piResult );
    int requestTM();

    bool isWriteRequest();
    bool isReadOnlyRequest();
    bool isServiceRequest();
};



class CreateMemRequestData: public RequestData{
    int importInInternalFomat();
    int createNewEmptyMemory();

  
    public:
    
    std::string strSrcLang;
    std::string strMemDescription;
    std::string strMemB64EncodedData; // for import in internal format
    
    CreateMemRequestData(const std::string& json): RequestData(COMMAND::CREATE_MEM,json, ""){}
    CreateMemRequestData(): RequestData(COMMAND::CREATE_MEM) {};

protected:
    int parseJSON() override;
    int checkData() override ;
    int execute() override ;
};

class ListTMRequestData:public RequestData{
public:
    ListTMRequestData(): RequestData(COMMAND::LIST_OF_MEMORIES) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};

class ImportRequestData:public RequestData{
public:
    ImportRequestData(): RequestData(COMMAND::IMPORT_MEM) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};

class DeleteTMRequestData:public RequestData{
public:
    DeleteTMRequestData(): RequestData(COMMAND::DELETE_MEM) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};



class ShutdownRequestData:public RequestData{
public:
    ShutdownRequestData(): RequestData(COMMAND::SHUTDOWN) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};

class ResourceInfoRequestData:public RequestData{
public:
    ResourceInfoRequestData(): RequestData(COMMAND::RESOURCE_INFO) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};



class TagRemplacementRequestData:public RequestData{
public:
    TagRemplacementRequestData(): RequestData(COMMAND::TAGREPLACEMENTTEST) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};


class CloneTMRequestData:public RequestData{
public:
    CloneTMRequestData(): RequestData(COMMAND::CLONE_TM_LOCALY) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};

class StatusRequestData:public RequestData{
public:
    StatusRequestData(): RequestData(COMMAND::STATUS_MEM) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};

class ReorganizeRequestData:public RequestData{
public:
    ReorganizeRequestData(): RequestData(COMMAND::REORGANIZE_MEM) {};
protected:
    int parseJSON() override { return -1;};
    int checkData() override { return -1;};
    int execute() override   { return -1;};
};


class UnknownRequestData: public RequestData{
    public: 
    UnknownRequestData(): RequestData(UNKNOWN_COMMAND){};

    protected:
    int parseJSON() override {return -1;};
    int checkData() override {return -1;};
    int execute()   override {return -1;};
};

class StatusMemRequestData: public RequestData{
    public:
    StatusMemRequestData(const std::string& memName): RequestData(STATUS_MEM, "", memName) {};
    StatusMemRequestData(): RequestData(STATUS_MEM){};
    int parseJSON() override {return 0;};
    int checkData() override {return -1;};
    int execute()   override {return -1;};
};

class DeleteMemRequestData: public RequestData{
    public:
    DeleteMemRequestData(const std::string& memName): RequestData(DELETE_MEM,"", memName){};
    DeleteMemRequestData(): RequestData(DELETE_MEM){};

protected:
    int parseJSON() override {return 0;}
    int checkData() override {return -1;};
    int execute() override   {return -1;};
};


class ConcordanceSearchRequestData: public RequestData{
    public:
    ConcordanceSearchRequestData(const std::string& json, const std::string& memName): RequestData(CONCORDANCE, json, memName){};
    ConcordanceSearchRequestData(): RequestData(CONCORDANCE){};
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute()   override ;
};


class FuzzySearchRequestData: public RequestData{    
    public:
    FuzzySearchRequestData(const std::string& json, const std::string& memName): RequestData(FUZZY,json, memName){};
    FuzzySearchRequestData(): RequestData(FUZZY){};

protected:
    LOOKUPINMEMORYDATA Data ;
    int parseJSON() override ;
    int checkData() override ;
    int execute()   override ;
};

class DeleteEntryRequestData: public RequestData{
    private:
    
    public:
    DeleteEntryRequestData(const std::string& json, const std::string& memName): RequestData(DELETE_ENTRY, json, memName) {};
    DeleteEntryRequestData(): RequestData(DELETE_ENTRY){}

protected:
    int parseJSON() override {return -1;};
    int checkData() override {return -1;};
    int execute() override   {return -1;};
};

class UpdateEntryRequestData: public RequestData{
    private:
    
    public:
    UpdateEntryRequestData(const std::string& json, const std::string& memName): RequestData(UPDATE_ENTRY, json, memName) {};
    UpdateEntryRequestData(): RequestData(UPDATE_ENTRY){};
protected:
    int parseJSON() override;
    int checkData() override;
    int execute  () override;
    
    LOOKUPINMEMORYDATA Data;
    MEMPROPOSAL Prop;
    //EqfMemory* lHandle = 0;
  };

class ExportRequestData: public RequestData{
    private:
    int ExportZip();
    int ExportTmx();
    
    FCTDATA fctdata;
    MEM_EXPORT_IDA IDA;         // Pointer to the export IDA
    PROCESSCOMMAREA CommArea;   // ptr to commmunication area

    USHORT PrepExport();
    USHORT ExportProc();
    std::string strTempFile;
    public:

    //std::string requestFormat;//application/xml or application/binary
    ExportRequestData(const std::string& format, const std::string& memName): RequestData(EXPORT_MEM, "", memName){ requestAcceptHeader = format;};
    ExportRequestData(): RequestData(EXPORT_MEM){};
protected:
    int parseJSON() override {return 0;}
    int checkData() override ;
    int execute() override ;

};

#endif //_REQUEST_DATA_H_INCLUDED_