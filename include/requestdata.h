#ifndef _REQUEST_DATA_H_INCLUDED_
#define _REQUEST_DATA_H_INCLUDED_

#include <string>
#include <memory>
#include <atomic>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ExMessageHandler.h>

#include "tm.h"

#include "lowlevelotmdatastructs.h"
#include "RestAPI/ProxygenStats.h"
    

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
    ProxygenStats* stats = nullptr;
    proxygen::ResponseHandler* responseHandler = nullptr;
    //proxygen::HTTPMessage response;

    int _id_ = 0;
    bool fValid = false;
    bool fRunning = true;
    std::string strMemName;

    //requests field
    std::string strUrl;
    std::string strBody;
    std::string requestAcceptHeader;


    //return fields
    std::string outputMessage;
    //std::string errorMsg;
    int _rc_ = 0; 
    int _rest_rc_ = 0;
    //timestamp


    RequestData(COMMAND cmnd, const std::string& json, const std::string& memName): strBody(json), strMemName(memName), command(cmnd) {}
    RequestData(COMMAND cmnd): command(cmnd){}
    virtual ~RequestData() = default;
    //std::weak_ptr <EqfMemory> memory;

    int buildErrorReturn(int iRC, const wchar_t *pszErrorMsg, int rest_rc = INTERNAL_SERVER_ERROR);
    int buildErrorReturn(int iRC, const char *pszErrorMsg, int rest_rc = INTERNAL_SERVER_ERROR);
    int buildRet(int ret);

    COMMAND command = UNKNOWN_COMMAND;
    bool isStreamingRequest()const{
        return command == EXPORT_MEM_TMX_STREAM 
        || command == EXPORT_MEM_INTERNAL_FORMAT_STREAM;
    }
    
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
    std::vector<unsigned char> binTmData; // for import in internal format
    std::string strTmData;
    std::string strTempFile;
    bool isBase64;
    
    CreateMemRequestData(const std::string& json): RequestData(COMMAND::CREATE_MEM,json, ""){}
    CreateMemRequestData(bool b64 = false): RequestData(COMMAND::CREATE_MEM) {isBase64 = b64;};

protected:
    int parseJSON() override;
    int checkData() override ;
    int execute() override ;
};




class ListTMRequestData:public RequestData{
public:
    ListTMRequestData(): RequestData(COMMAND::LIST_OF_MEMORIES) {};
protected:
    int parseJSON() override { return 0;};
    int checkData() override { return 0;};
    int execute() override  ;
};


#include "../source/RestAPI/ProxygenStats.h"
//namespace ProxygenService;
//class ProxygenService::ProxygenStats;
/*! \brief Data area for the processing of the importMemory function
*/
class IMPORTMEMORYDATA
{
public:
  //HSESSION hSession;
  //OtmMemoryServiceWorker *pMemoryServiceWorker;
  char szMemory[260];
  char szInFile[260];
  char szError[512];
  std::shared_ptr<EqfMemory> mem;
  BOOL fDeleteTmx = false;
  //BOOL fImpoortFromFile = 0;
  //folly::IOBuf fileData;
  std::string fileData;
  ProxygenStats* stats_ = nullptr;
  InclosingTagsBehaviour inclosingTagsBehaviour = InclosingTagsBehaviour::saveAll;
  //ushort * pusImportPersent = nullptr;
  //ImportStatusDetails* importDetails = nullptr;
  //OtmMemoryServiceWorker::std::shared_ptr<EqfMemory>  pMem = nullptr;
};

class ImportRequestData:public RequestData{
public:
    ImportRequestData(bool Base64TMX = true): RequestData(COMMAND::IMPORT_MEM) { isBase64 = Base64TMX; };

    std::string strTmxData;
    bool isBase64;
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute() override   ;

    BOOL fClose = false;
    //MEMORY_STATUS lastImportStatus = AVAILABLE_STATUS; // to restore in case we would break import before calling closemem
    //MEMORY_STATUS lastStatus = AVAILABLE_STATUS;
    InclosingTagsBehaviour inclosingTagsBehaviour;
    IMPORTMEMORYDATA* pData = nullptr;
    std::string strTempFile;
    long timeout = 0;
};

#include <folly/io/IOBufQueue.h>
class ImportStreamRequestData:public RequestData{
public:
    ImportStreamRequestData(): RequestData(COMMAND::IMPORT_MEM_STREAM) { };
    
    //folly::IOBuf fileData;
    std::string fileData;
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute() override   ;

    BOOL fClose = false;
    InclosingTagsBehaviour inclosingTagsBehaviour;
    IMPORTMEMORYDATA* pData = nullptr;
    long timeout = 0;
};

class ImportLocalRequestData:public RequestData{
public:
    ImportLocalRequestData(): RequestData(COMMAND::IMPORT_LOCAL_MEM) {};
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute() override   ;

    BOOL fClose = false;
    IMPORTMEMORYDATA* pData = nullptr;
    std::string strInputFile;
};

class SaveAllTMsToDiskRequestData: public RequestData{
public:
    SaveAllTMsToDiskRequestData(): RequestData(COMMAND::SAVE_ALL_TM_ON_DISK){};
protected: 
    int parseJSON() override { return 0; };
    int checkData() override { return 0; };
    int execute() override;
};

class ShutdownRequestData:public RequestData{
public:
    ShutdownRequestData(): RequestData(COMMAND::SHUTDOWN) {};
    int sig = 0;

    static void CheckImportFlushTmsAndShutdown(int signal);
protected:
    int parseJSON() override { return 0;};
    int checkData() override { return 0;};
    int execute()   override   ;
};


//class ProxygenStats;
class ResourceInfoRequestData:public RequestData{
public:
    ProxygenStats* pStats = nullptr;
    ResourceInfoRequestData(): RequestData(COMMAND::RESOURCE_INFO) {};
protected:
    int parseJSON() override { return 0;};
    int checkData() override { return 0;};
    int execute() override ;
};



class TagRemplacementRequestData:public RequestData{
public:
    TagRemplacementRequestData(): RequestData(COMMAND::TAGREPLACEMENTTEST) {};
protected:
    int parseJSON() override {return 0;};
    int checkData() override { return 0;};
    int execute() override ;
};


class CloneTMRequestData:public RequestData{
public:
    CloneTMRequestData(): RequestData(COMMAND::CLONE_TM_LOCALY) {};
protected:
    
    std::string newName, srcTmdPath, srcTmiPath, dstTmdPath, dstTmiPath;
    
    int parseJSON() override;
    int checkData() override;
    int execute  () override;
};


enum struct SearchType{
    NONE = 0,
    EXACT = 1,
    CONCORDANCE = 2
};

#include "tm.h"

struct SearchFilterFactory{

  //author: SEARCHED_STRING
  //authorSearchMode: concordance|exact
  //additionalInfo: SEARCHED_STRING
  //additionalInfoSearchMode: concordance|exact
  //documentName: SEARCHED_STRING
  //documentNameSearchMode: concordance|exact
  //timestampSpanStart: STAMP
  //timestampSpanEnd: STAMP
  //context: We still need to decide, when we know, what is really saved in here. Therefore we omit context for now in the implementation of this issue.

  //int caseInsensetive = 0, whitespaceTolerant = 0;
  
  //FilterOptions getSearchOptions(){
  //  long res = 0;
  //  return res;
  //}

  std::string source;
  ProposalFilter::FilterType sourceSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions sourceSearchOptions = 0;

  std::string target;
  ProposalFilter::FilterType targetSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions targetSearchOptions = 0;

  std::string author;
  ProposalFilter::FilterType authorSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions authorSearchOptions = 0;

  std::string document;
  ProposalFilter::FilterType documentSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions documentSearchOptions = 0;

  std::string addInfo;
  ProposalFilter::FilterType addInfoSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions addInfoSearchOptions = 0;

  std::string context;
  ProposalFilter::FilterType contextSearchMode = ProposalFilter::FilterType::NONE;
  FilterOptions contextSearchOptions = 0;

  TIME_L timestampSpanStart = 0;
  TIME_L timestampSpanEnd = 0;

  int parseJSON(std::string& w_str);
  std::string checkParsedModes();
  std::vector<ProposalFilter> getListOfFilters();
};

class ReorganizeRequestData:public RequestData{
public:
    ReorganizeRequestData(): RequestData(COMMAND::REORGANIZE_MEM) {};
protected:
    int parseJSON() override {return 0;};
    int checkData() override;
    int execute  () override;
};


class UnknownRequestData: public RequestData{
    public: 
    UnknownRequestData(): RequestData(UNKNOWN_COMMAND){};

    protected:
    int parseJSON() override {return 0;};
    int checkData() override {return 0;};
    int execute()   override {
        _rest_rc_ = 404;
        std::string msg = "Url \"" + strUrl + "\" was not parsed correctly";
        return buildErrorReturn(404, msg.c_str(), 404);
    };
};

class StatusMemRequestData: public RequestData{
    public:
    StatusMemRequestData(const std::string& memName): RequestData(STATUS_MEM, "", memName) {};
    StatusMemRequestData(): RequestData(STATUS_MEM){};
    int parseJSON() override {return 0;};
    int checkData() override ;
    int execute()   override ;
};

class DeleteMemRequestData: public RequestData{
    public:
    DeleteMemRequestData(const std::string& memName): RequestData(DELETE_MEM,"", memName){};
    DeleteMemRequestData(): RequestData(DELETE_MEM){};

protected:
    int parseJSON() override {return 0;}
    int checkData() override {return 0;};
    int execute  () override;
};

#include "Proposal.h"

class ConcordanceSearchRequestData: public RequestData{
    public:
    ConcordanceSearchRequestData(const std::string& json, const std::string& memName): RequestData(CONCORDANCE, json, memName){};
    ConcordanceSearchRequestData(): RequestData(CONCORDANCE){};
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute()   override ;


    LONG lOptions = 0;
    OtmProposal Data;
};


class ConcordanceExtendedSearchRequestData: public RequestData{
    public:
    ConcordanceExtendedSearchRequestData(const std::string& json, const std::string& memName): RequestData(CONCORDANCE, json, memName){};
    ConcordanceExtendedSearchRequestData(): RequestData(CONCORDANCE){};
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute()   override ;


    SearchFilterFactory searchFilterFactory;
    OtmProposal Data;
    LONG lOptions = 0;
    BOOL fCountInsteadOfReturnSegments = false;
    BOOL fCombineAsLogicalOr = false;
};

class FuzzySearchRequestData: public RequestData{    
    public:
    FuzzySearchRequestData(const std::string& json, const std::string& memName): RequestData(FUZZY,json, memName){};
    FuzzySearchRequestData(): RequestData(FUZZY){};

protected:
    OtmProposal Data ;
    char szDateTime[100];
    char szType[100];

    int parseJSON() override ;
    int checkData() override ;
    int execute()   override ;
};

class DeleteEntryRequestData: public RequestData{
    private:
    
    public:
    DeleteEntryRequestData(const std::string& json, const std::string& memName): RequestData(DELETE_ENTRY, json, memName) {};
    DeleteEntryRequestData(): RequestData(DELETE_ENTRY){}

    BOOL fSave2Disk = 1;
protected:
    OtmProposal Data;
    char szDateTime[100];
    char szType[100];

    int parseJSON() override ;
    int checkData() override ;
    int execute  () override ;
};

class DeleteEntriesReorganizeRequestData: public RequestData{
    private:

    
    public:
    DeleteEntriesReorganizeRequestData(const std::string& json, const std::string& memName): RequestData(DELETE_ENTRIES_REORGANIZE, json, memName) {};
    DeleteEntriesReorganizeRequestData(): RequestData(DELETE_ENTRIES_REORGANIZE){}

protected:
    SearchFilterFactory searchFilterFactory;

    int parseJSON() override { 
        _rc_ = searchFilterFactory.parseJSON(strBody); 
        if(_rc_ == 1001){buildErrorReturn(_rc_,"Can't parse start timestamp", BAD_REQUEST);}
        else if(_rc_ == 1002){buildErrorReturn(_rc_,"Can't parse end timestamp", BAD_REQUEST);}
        return _rc_;
        };
    int checkData() override ;
    int execute  () override ;
};

class UpdateEntryRequestData: public RequestData{
    public:
    UpdateEntryRequestData(const std::string& json, const std::string& memName): RequestData(UPDATE_ENTRY, json, memName) {};
    UpdateEntryRequestData(): RequestData(UPDATE_ENTRY){};

    BOOL fSave2Disk = 1;
protected:
    
    int parseJSON() override;
    int checkData() override;
    int execute  () override;

    char szDateTime[100];
    char szType[100];

    OtmProposal Data;

  };

class ExportRequestData: public RequestData{
    private:
    int PrepareTMZip();
    int ExportZip();
    int ExportZipStream();
    int ExportTmx();
    
    
    char szKey[20];
    int numberOfRequestedProposals = 0;
    int loggingThreshold = 0;
    FCTDATA fctdata;
    MEM_EXPORT_IDA IDA;         // Pointer to the export IDA
    PROCESSCOMMAREA CommArea;   // ptr to commmunication area

    USHORT PrepExport();
    USHORT ExportProc();
    std::string strTempFile;
    public:

    //std::string requestFormat;//application/xml or application/binary
    ExportRequestData(const std::string& format, const std::string& memName): RequestData(EXPORT_MEM_TMX, "", memName){ requestAcceptHeader = format;};
    ExportRequestData(): RequestData(EXPORT_MEM_TMX){};
    ExportRequestData(COMMAND command): RequestData(command){};
protected:
    int parseJSON() override ;
    int checkData() override ;
    int execute() override ;

};

#endif //_REQUEST_DATA_H_INCLUDED_