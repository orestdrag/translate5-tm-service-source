/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxygenHandler.h"

//#ifdef TEMPORARY_COMMENTED
#include <proxygen/httpserver/RequestHandler.h>
#include "../opentm2/core/utilities/LogWrapper.h"
#include "../opentm2/core/utilities/PropertyWrapper.H"

#include "ProxygenStats.h"
#include "OtmMemoryServiceWorker.h"

using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");

namespace ProxygenService {

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {
}

const std::map<const ProxygenHandler::COMMAND,const char*> CommandToStringsMap {
        { ProxygenHandler::COMMAND::UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
        { ProxygenHandler::COMMAND::LIST_OF_MEMORIES, "LIST_OF_MEMORIES" },
        { ProxygenHandler::COMMAND::CREATE_MEM, "CREATE_MEM" },
        { ProxygenHandler::COMMAND::DELETE_MEM, "DELETE_MEM" },
        { ProxygenHandler::COMMAND::IMPORT_MEM, "IMPORT_MEM" },
        { ProxygenHandler::COMMAND::IMPORT_MEM_INTERNAL_FORMAT, "IMPORT_MEM_INTERNAL_FORMAT" },
        { ProxygenHandler::COMMAND::EXPORT_MEM, "EXPORT_MEM" },
        { ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { ProxygenHandler::COMMAND::STATUS_MEM, "STATUS_MEM" },
        { ProxygenHandler::COMMAND::FUZZY, "FUZZY" },
        { ProxygenHandler::COMMAND::CONCORDANCE, "CONCORDANCE" },
        { ProxygenHandler::COMMAND::DELETE_ENTRY, "DELETE_ENTRY" },
        { ProxygenHandler::COMMAND::UPDATE_ENTRY, "UPDATE_ENTRY" },
        { ProxygenHandler::COMMAND::TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { ProxygenHandler::COMMAND::SHUTDOWN, "SHUTDOWN" },
        { ProxygenHandler::COMMAND::SAVE_ALL_TM_ON_DISK, "SAVE_ALL_TM_ON_DISK" }
    };

//static OtmMemoryServiceWorker *pMemService = NULL;


#ifdef RESTBED_CODE 

void delete_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );
  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );

  LogMessage2(RequestTransactionLogLevel, "processing DELETE MEMORY request, mem = ", strTM.c_str() );

  string strResponseBody;
  //int rc = 200;
  int rc = pMemService->deleteMem( strTM, strResponseBody );

  char version[20];
  properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
  session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );

  LogMessage4(RequestAdditionalLogLevel,"DELETE MEMORY request::Done, , RC=",toStr(rc).c_str(),"; strResponseBody=", strResponseBody.c_str() );
}

void get_memory_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  std::string strAccept = request->get_header( "Accept", "" );
  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );

  LogMessage5(RequestTransactionLogLevel, "processing EXPORT MEMORY request, Accept = \"", strAccept.c_str(), "\"; mem = \"", strTM.c_str(), "\"");

  restbed::Bytes vMemData;
  int rc = pMemService->getMem( strTM, strAccept, vMemData );
  
  char version[20];
  properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
  session->close( rc, vMemData, { { "Content-Length", ::to_string( vMemData.size() ) },{ "Content-Type", strAccept.c_str() },{ szVersionID, version } } );
  LogMessage2(RequestAdditionalLogLevel,"EXPORT memory request::Done, RC = ", toStr(rc).c_str());
}

void get_method_handler(const shared_ptr< Session > session)
{
	const auto request = session->get_request();

  LogMessage(RequestTransactionLogLevel, "processing GET LIST OF MEMORIES request");
  std::string strResponseBody = "Sample text";
  int ret = pMemService->list( strResponseBody );

  char version[20];
  properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
  session->close( OK, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
  LogMessage4(RequestAdditionalLogLevel, "GET LIST OF MEMORIES done, strResponceBody: \"", strResponseBody.c_str(), "\" RC = ", toStr(OK).c_str());
}

void getStatus_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  string strTM = request->get_path_parameter( "id", "" );
  restoreBlanks( strTM );

  LogMessage3(RequestTransactionLogLevel, "processing GET MEM STATUS request, mem=\"",strTM.c_str() ,"\"");

  string strResponseBody;
  int iRC = pMemService->getStatus( strTM, strResponseBody );

  char version[20];
  properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
  session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" }, { szVersionID, version } } );
  
} 


void saveAllOpenedTMService_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();
  LogMessage(RequestTransactionLogLevel, "processing saveAllOpenedTMService_method_handler");

  string strResponseBody;
  int iRC = pMemService->saveAllTmOnDisk( strResponseBody );

  char version[20];
  properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
  session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" }, { szVersionID, version } } );
}

bool IsMemImportInProcess(){
  int memInImportProcess = pMemService->GetMemImportInProcess();
  return memInImportProcess >= 0;
}

void shutdownService_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();
  int iRC;
  string strResponseBody;
   
  {
    if( request->has_query_parameter("dontsave") ){
     LogMessage(RequestTransactionLogLevel, "called shutdownService_method_handler with saving turned off");
      strResponseBody = "{\n    'responce': 'shuting down service without saving tms'\n}";
    }else{
      LogMessage(RequestTransactionLogLevel, "called shutdownService_method_handler with saving turned on");
      saveAllOpenedTMService_method_handler(session);
      //iRC = pMemService->saveAllTmOnDisk( strResponseBody );
    }

    //int iRC = pMemService->shutdownService( strResponseBody );
    //session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" }, { szVersionID, STR_DRIVER_LEVEL_NUMBER } } );
    //session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" } } );
  }

  //wait till all import processes would end
  int i = 0;
  while(IsMemImportInProcess() && i<600){
    sleep(1000);
    i++;
  }
  xercesc::XMLPlatformUtils::Terminate();
  StopOtmMemoryService();
  session->close( iRC, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" } } );
}

// replace plus signs in string with blanks
void restoreBlanks( std::string &strInOut )
{
  int iLen = strInOut.length();
  for ( int i = 0; i < iLen; i++ )
  {
    if ( strInOut[i] == '+' )
    {
      strInOut[i] = ' ';
    }
  }
}

void postImport_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );
    string strInData = string( body->begin(), body->end() );
    string strResponseBody;
    if(CheckLogLevel(RequestTransactionLogLevel)){
      std::string truncatedInput = strInData.size() > 3000 ? strInData.substr(0, 3000) : strInData;
      LogMessage6(RequestTransactionLogLevel, "processing MEM IMPORT request, Memory name = \"", strTM.c_str(), "\"\n Input(truncated) = \n\"", truncatedInput.c_str(), 
      "\"\n content length = ", toStr(body->size()).c_str());
    }

    int rc = pMemService->import( strTM, strInData, strResponseBody );
    
    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    
    LogMessage5(RequestAdditionalLogLevel, "MEM IMPORT::Done, RC=", toStr(rc).c_str(),", Output:\n-----\n", strResponseBody.c_str() ,"\n-----\n====\n");
  } );
}

void post_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    string strInData = string( body->begin(), body->end() );
    LogMessage4(RequestTransactionLogLevel, "processing CREATE MEM request,",
             //" content type, content type=\"", strType.c_str(),"\" content length = ", toStr(content_length).c_str(), 
            " Input: \"", strInData.c_str(), "\"" );
    string strResponseBody;
    
    int rc = pMemService->createMemory( strInData, strResponseBody );
    
    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    
    LogMessage4(RequestAdditionalLogLevel, "CREATE MEM::done, RC=", toStr(rc).c_str(),"; Output: ", strResponseBody.c_str());
  } );
}


void postTagReplacement_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();
  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  LogMessage4(RequestAdditionalLogLevel, "processing TAG REPLACEMENT request, content type=\"",strType.c_str(),"\", content length=", toStr( content_length ).c_str() );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    int rc = 0;
    string strSrcData, strTrgData, strReqData;
    string strInData = string( body->begin(), body->end() );
    strSrcData.reserve( strInData.size() + 1 );
    strTrgData.reserve( strInData.size() + 1 );
    strReqData.reserve( strInData.size() + 1 );   
    wstring wstr;

    JSONFactory *factory = JSONFactory::getInstance();
    void *parseHandle = factory->parseJSONStart( strInData, &rc );
    if ( parseHandle == NULL )
    {
      wchar_t errMsg[] = L"Missing or incorrect JSON data in request body";
      wstr = errMsg;
      //buildErrorReturn( iRC, errMsg, strOutputParms );
      return( restbed::BAD_REQUEST );
    }else{ 
      std::string name;
      std::string value;
      while ( rc == 0 )
      {
        rc = factory->parseJSONGetNext( parseHandle, name, value );
        if ( rc == 0 )
        {      
          LogMessage5(RequestAdditionalLogLevel, __func__,"::JSON parsed src = ", name.c_str(), "; size = ", toStr(strSrcData.size()));
          if ( strcasecmp( name.c_str(), "src" ) == 0 )
          {
            strSrcData = value;
          }else if( strcasecmp( name.c_str(), "trg" ) == 0 ){
            strTrgData = value;
          }else if( strcasecmp (name.c_str(), "req") == 0 ){
            strReqData = value;          
          }else{
            LogMessage5(WARNING,__func__, "::JSON parsed unused data: name = ", name.c_str(), "; value = ",value.c_str());
          }
        }
      } /* endwhile */
      factory->parseJSONStop( parseHandle );
    }
    
    auto result =  pMemService->replaceString(  EncodingHelper::convertToUTF16(strSrcData.c_str()).c_str(), 
                                                EncodingHelper::convertToUTF16(strTrgData.c_str()).c_str(),
                                                EncodingHelper::convertToUTF16(strReqData.c_str()).c_str(), &rc);

    
    wstr = L"{\n ";
    std::wstring segmentLocations[] = {L"source", L"target", L"request"};
    for(int index = 0; index < result.size(); index++){
      wstr += L"\'" + segmentLocations[index] + L"\' :\'" + result[index] + L"\',\n ";
    }
    wstr += L"\n};";
    string strResponseBody =  EncodingHelper::convertToUTF8(wstr);

    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");

    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },
      { "Content-Type", "application/json" },
      { szVersionID, version } } );
    LogMessage5(RequestAdditionalLogLevel,  "Tag replacement::done, RC=",toStr(rc).c_str(),"\nOutput:\n-----\n", strResponseBody.c_str() ,"\n----\n====\n");
  });
}

void postFuzzySearch_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );

    string strInData = string( body->begin(), body->end() );
    string strResponseBody = "{}";
    LogMessage5(RequestTransactionLogLevel, "processing FUZZY SEARCH request Memory name = \"", strTM.c_str() ,"\"\nInput:\n-----\n\"",strInData.c_str(),"\"\n----" );
    int rc = pMemService->search( strTM, strInData, strResponseBody );

    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    LogMessage5(RequestAdditionalLogLevel,  "FUZZY SEARCH::Done! RC=",toStr(rc).c_str(),"\nOutput:\n-----\n", strResponseBody.c_str() ,"\n----\n====\n");
  } );
}

void postConcordanceSearch_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );

    string strInData = string( body->begin(), body->end() );
    string strResponseBody;
    LogMessage6(RequestTransactionLogLevel, "processing CONCORDANCE SEARCH request ", "Memory name = \"", strTM.c_str() ,"\"\nInput:\n-----\n\"",strInData.c_str(),"\"\n----" );
    
    int rc = pMemService->concordanceSearch( strTM, strInData, strResponseBody );
    
    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    LogMessage4(RequestAdditionalLogLevel,"CONCORDANCE SEARCH::Done! RC = ", toStr(rc).c_str(), "; Output = ", strResponseBody.c_str());
  } );
}

void postEntry_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );

    string strInData = string( body->begin(), body->end() );
    string strResponseBody;

    LogMessage5(RequestTransactionLogLevel, "processing ENTRY UPDATE request, Memory name=\"",strTM.c_str(),"\"\nInput:\n-----\n", strInData.c_str(),"\n----\n" );
    int rc = pMemService->updateEntry( strTM, strInData, strResponseBody );
    
    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    
    LogMessage7(RequestAdditionalLogLevel,"ENTRY UPDATE::...Done, RC=",toStr(rc).c_str(),"\nOutput:\n-----\n", strResponseBody.c_str(),"\n----\nFor TM \'",strTM.c_str(),"\'====\n");
  } );
}


void postEntryDelete_method_handler( const shared_ptr< Session > session )
{
  const auto request = session->get_request();

  size_t content_length = request->get_header( "Content-Length", 0 );
  std::string strType = request->get_header( "Content-Type", "" );

  session->fetch( content_length, []( const shared_ptr< Session >& session, const Bytes& body )
  {
    const auto request = session->get_request();
    string strTM = request->get_path_parameter( "id", "" );
    restoreBlanks( strTM );

    string strInData = string( body->begin(), body->end() );
    string strResponseBody;

    LogMessage5(RequestTransactionLogLevel, "processing ENTRY DELETE request, Memory name=\"",strTM.c_str(),"\"\nInput:\n-----\n", strInData.c_str(),"\n----\n" );
    int rc = pMemService->deleteEntry( strTM, strInData, strResponseBody );
    
    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    session->close( rc, strResponseBody, { { "Content-Length", ::to_string( strResponseBody.length() ) },{ "Content-Type", "application/json" },{ szVersionID, version } } );
    
    LogMessage7(RequestAdditionalLogLevel, "ENTRY DELETE ::Done, RC=",toStr(rc).c_str(),"\nOutput:\n-----\n", strResponseBody.c_str(),"\n----\nFor TM \'",strTM.c_str(),"\'====\n");
  } );
}
#endif // RESTBED_CODE 

//std::string tagReplacement(std::string strInData, int& rc);



void ProxygenHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
  builder = new ResponseBuilder(downstream_);
  auto methodStr = req->getMethodString ();
  auto method = req->getMethod ();
  auto url = req->getURL () ;
  auto path = req->getPath () ;
  auto queryString   = req->getQueryString () ;
  auto headers = req->getHeaders();
  pMemService = OtmMemoryServiceWorker::getInstance();
 
  std::string requestAcceptHeader = headers.getSingleOrEmpty("Accept");
  if(command == COMMAND::EXPORT_MEM){
    if(requestAcceptHeader == "application/xml"){
      command = COMMAND::EXPORT_MEM;   
    }else if(requestAcceptHeader == "application/zip"){
      command = COMMAND::EXPORT_MEM_INTERNAL_FORMAT;
    }else{
      errorStr = "export format not specified in Accept header";
      command = COMMAND::UNKNOWN_COMMAND;
    }
  }

  stats_->recordRequest();
  

  if(this->command < COMMAND::START_COMMANDS_WITH_BODY ){ // we handle here only requests without body

    //ResponseBuilder builder(downstream_);
    
    //std::string strResponseBody;
    //int iRC = 400;
    //std::string responseText = "WRONG_REQUEST";

    switch(this->command){
      case COMMAND::STATUS_MEM:
      {
        iRC = pMemService->getStatus( memName, strResponseBody );
        responseText = "OK";
        //builder->status(iRC, "OK");
        //builder->header("Content-Type", "application/json");
        builder->body(strResponseBody);
        break;
      }
        
      case COMMAND::LIST_OF_MEMORIES:
      {
        int ret = pMemService->list( strResponseBody );
        builder->body(strResponseBody);
        responseText = "OK";
        iRC = 200;
        //builder->status(200, "OK");
        break;
      }
      case COMMAND::TAGREPLACEMENTTEST:
      {
        std::string str;
        pMemService->tagReplacement(str, iRC);
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::EXPORT_MEM:
      {
        iRC = pMemService->getMem( memName, "application/xml",  vMemData);
        builder->body(std::string(vMemData.begin(), vMemData.end()));
        responseText = "OK";
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
      {
        
        //if(requestAcceptHeader == "application/xml"){
          
        //}else if(requestAcceptHeader == "application/json"){

        //}else{

        //}
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::DELETE_MEM:
      {
        iRC = pMemService->deleteMem( memName, strResponseBody );
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::SAVE_ALL_TM_ON_DISK:
      {
        iRC = pMemService->saveAllTmOnDisk( strResponseBody );
        responseText = "OK";
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::SHUTDOWN:
      {
          iRC = pMemService->saveAllTmOnDisk( strResponseBody );
          pMemService->closeAll();
          //close log file
          LogStop();

          //TO DO:STOP SERVER HERE 
        //break; 
        //iRC = pMemService->shutdownService();
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      default:
      {
        
        break;
      }
    }
    
    builder->status(iRC, responseText);
    if (FLAGS_request_number) {
      builder->header("Request-Number",
                    folly::to<std::string>(stats_->getRequestCount()));
    }
    req->getHeaders().forEach([&](std::string& name, std::string& value) {
      builder->header(folly::to<std::string>("x-proxygen-", name), value);
    });

    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    builder->header("t5memory-version", version);   
    builder->header("Content-Type", "application/json");

    LogMessage5(DEBUG, __func__, ":: command = ", 
              CommandToStringsMap.find(this->command)->second, ", memName = ", memName.c_str());

    //builder->send();
    builder->sendWithEOM();//builder->send();
  }
}

void ProxygenHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
 
  //std::string strInputBodyData;//( body-> );
  auto req_data = body->data();
  
  if(CheckLogLevel(DEBUG)){
    //std::string truncatedInput = strInData.size() > 3000 ? strInData.substr(0, 3000) : strInData;
    //LogMessage8(RequestTransactionLogLevel, "processing ",""," request, Memory name = \"", strTM.c_str(), "\"\n Input(truncated) = \n\"", truncatedInput.c_str(), 
    //"\"\n content length = ", toStr(body->size()).c_str());
  }

  
  
  
  switch(this->command){
    case COMMAND::CREATE_MEM:{
      iRC = pMemService->createMemory( (char*) req_data, strResponseBody );
      break;
    }
    case COMMAND::IMPORT_MEM:
    case COMMAND::IMPORT_MEM_INTERNAL_FORMAT:
    {
      if(body_ == nullptr){
        body_ = std::move(body);
      }else{
        //body_.append(body->begin(), body->end());
        body_->appendToChain(std::move(body));
        //body_->insertAfterThisOne(std::move(body));
        //body_->
        //body_->appendTo(req_data);
      }
      break; 
    }
    case COMMAND::FUZZY:
    {
      iRC =pMemService->search( memName, (char*) req_data, strResponseBody );
      break;
    }
    case COMMAND::CONCORDANCE:
    {
      iRC = pMemService->concordanceSearch( memName, (char*) req_data, strResponseBody );
      break;
    }
    case COMMAND::UPDATE_ENTRY:
    {
      iRC = pMemService->updateEntry( memName, (char*) req_data,  strResponseBody );
      break;
    }
    case COMMAND::DELETE_ENTRY:
    {
      iRC = pMemService->deleteEntry( memName, (char*) req_data,  strResponseBody );
      break;
    }
    default:
    {
      iRC = 400;
      break;
    }
  }

  
  //ResponseBuilder(downstream_).body(std::move(body)).send();
}

void ProxygenHandler::onEOM() noexcept {
  //ResponseBuilder builder(downstream_);
  if(command >= COMMAND::START_COMMANDS_WITH_BODY){
    if(command == COMMAND::IMPORT_MEM ) 
    {
      body_->coalesce();
      iRC = pMemService->import( memName, (char*) body_->data(), strResponseBody );
    }else if(command == COMMAND::IMPORT_MEM_INTERNAL_FORMAT)
    {
      iRC = pMemService->import( memName, (char*) body_->data(), strResponseBody );
    }
    builder->status(iRC, responseText);
    if (FLAGS_request_number) {
      builder->header("Request-Number",
                    folly::to<std::string>(stats_->getRequestCount()));
    }
    //req->getHeaders().forEach([&](std::string& name, std::string& value) {
    //  builder->header(folly::to<std::string>("x-proxygen-", name), value);
    //});

    char version[20];
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "");
    builder->header("t5memory-version", version);  
    if(command == COMMAND::EXPORT_MEM_INTERNAL_FORMAT){
      builder->header("Content-Type", "application/zip");
    } else{
      builder->header("Content-Type", "application/json");
    }
    LogMessage5(DEBUG, __func__, ":: command = ", 
                CommandToStringsMap.find(this->command)->second, ", memName = ", memName.c_str());
    if(strResponseBody.size())
      builder->body(strResponseBody);
    //builder->send();
    builder->sendWithEOM();
  }
}

void ProxygenHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void ProxygenHandler::requestComplete() noexcept {
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}
} // namespace ProxygenService
//#endif
