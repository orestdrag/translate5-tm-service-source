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
#include "../opentm2/core/utilities/Property.h"

#include "ProxygenStats.h"
#include "OtmMemoryServiceWorker.h"
#include <atomic>
#include "LogWrapper.h"


using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");


namespace ProxygenService {

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {
  TMManager::GetInstance()->fWriteRequestsAllowed = true;
}

constexpr  std::initializer_list<std::pair<const COMMAND, const char*>> CommandToStringsMap {
        { UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
        { LIST_OF_MEMORIES, "LIST_OF_MEMORIES" },
        { SAVE_ALL_TM_ON_DISK, "SAVE_ALL_TM_ON_DISK" },
        { SHUTDOWN, "SHUTDOWN" },
        { DELETE_MEM, "DELETE_MEM" },
        { EXPORT_MEM, "EXPORT_MEM" },
        { EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { STATUS_MEM, "STATUS_MEM" },
        { RESOURCE_INFO, "RESOURCE_INFO" },
        { CREATE_MEM, "CREATE_MEM" },
        { FUZZY, "FUZZY" },
        { CONCORDANCE, "CONCORDANCE" },
        { DELETE_ENTRY, "DELETE_ENTRY" },
        { DELETE_ENTRIES_REORGANIZE, "DELETE_ENTRIES_REORGANIZE" },
        { UPDATE_ENTRY, "UPDATE_ENTRY" },
        { TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { IMPORT_MEM, "IMPORT_MEM" },
        { IMPORT_LOCAL_MEM, "IMPORT_LOCAL_MEM" },
        { REORGANIZE_MEM, "REORGANIZE_MEM" },
        { CLONE_TM_LOCALY, "CLONE_MEM"}
    };

const char* searchInCommandToStringsMap(const COMMAND& key) {
    for (auto&& pair : CommandToStringsMap) {
        if (pair.first == key) {
            return pair.second ; // Found the key, no need to continue searching
        }
    }
    return "UNKNOWN";
}

void ProxygenHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
#ifdef TIME_MEASURES 
  start_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
  //time(&startingTime);
#endif

  builder = new ResponseBuilder(downstream_);
  auto methodStr = req->getMethodString ();
  auto method = req->getMethod ();
  if(!pRequest){
    pRequest = new UnknownRequestData();
  }
  pRequest->strUrl = req->getURL () ;
  auto path = req->getPath () ;
  auto queryString   = req->getQueryString () ;
  auto headers = req->getHeaders();
  pMemService = OtmMemoryServiceWorker::getInstance();
  auto command = pRequest->command;

  T5LOG(T5DEBUG) << "received request url:\"" << pRequest->strUrl <<"\"; type " << searchInCommandToStringsMap(pRequest->command);  
 
  std::string requestAcceptHeader = headers.getSingleOrEmpty("Accept");
  if(pRequest->command == COMMAND::EXPORT_MEM){
    if(requestAcceptHeader == "application/xml"){
      pRequest->command = COMMAND::EXPORT_MEM;   
    }else if(requestAcceptHeader == "application/zip"){
      pRequest->command = COMMAND::EXPORT_MEM_INTERNAL_FORMAT;
    }else{
      errorStr = "export format not specified in Accept header";
      pRequest->command = COMMAND::UNKNOWN_COMMAND;
    }
  }

  pRequest->requestAcceptHeader = requestAcceptHeader;

  pRequest->_id_ = stats_->recordRequest(pRequest->command);
  
  T5Logger::GetInstance()->SetLogInfo(pRequest->command);
  T5Logger::GetInstance()->SetLogBuffer(std::string("Error during ") + searchInCommandToStringsMap(pRequest->command) + " request, id = " + toStr(pRequest->_id_));

  if(pRequest->strMemName.empty() == false){
    T5Logger::GetInstance()->AddToLogBuffer(std::string(", for memory \"") + pRequest->strMemName + "\"");
  }

  if(pRequest->command < COMMAND::START_COMMANDS_WITH_BODY ){ // we handle here only requests without body
    pRequest->run();
    sendResponse();
  }
}

void ProxygenHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  //ss_body << (char* ) body->data();
  if(body_ == nullptr){
    body_ = std::move(body);
  }else{
    body_->appendToChain(std::move(body));
  }
}

void ProxygenHandler::onEOM() noexcept {  
  if(pRequest && pRequest->command >= COMMAND::START_COMMANDS_WITH_BODY)
  {
    body_->coalesce();      
    pRequest->strBody = (char*) body_->data();
     //fix garbage in json 
    if(pRequest->strBody.empty()){
      pRequest->_rest_rc_ = 404;
    }else if(TMManager::GetInstance()->fWriteRequestsAllowed == false){
      pRequest->_rest_rc_ = 423;  
    }else{
      size_t json_end = pRequest->strBody.find("\n}") ;
      if(json_end > 0 && json_end != std::string::npos){
        pRequest->strBody = pRequest->strBody.substr(0, json_end + 2);
      }
      std::string truncatedInput = pRequest->strBody.size() > 3000 ? pRequest->strBody.substr(0, 3000) : pRequest->strBody;
      T5Logger::GetInstance()->SetBodyBuffer(", with body = \n\"" + truncatedInput +"\"\n");
      pRequest->run();
    }
 
    sendResponse();
  }
}

void ProxygenHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void ProxygenHandler::requestComplete() noexcept {
  //ResetLogBuffer();
  if(pRequest){
    while(pRequest->fRunning){};
    delete pRequest;
    pRequest = nullptr;
  }
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}

//#define LOG_SEND_RESPONSE 1
void ProxygenHandler::sendResponse()noexcept{

    #ifdef TIME_MEASURES
    //time(&endingTime);
    //time_t time = endingTime - startingTime;
    
    end_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
    milliseconds time = end_ms-start_ms;   

    #ifdef LOG_SEND_RESPONSE
     T5LOG(T5TRANSACTION) <<"id = " << pRequest->_id_ << "; exection time = " <<  std::chrono::duration<double>(time).count();
    #endif
    
    stats_->addRequestTime(pRequest->command, time);
    #endif

    switch(pRequest->_rest_rc_){
      case 200:
      case 201:
      {
        responseText = "OK";
        break;
      }
      case 423:
      {
        //responseText 
        pRequest->outputMessage = "{\n\"msg\": \"WRITE REQUESTS ARE NOT ALLOWED\",\n\"rc\" = 423\n}\n" ;
        responseText = "Locked";
        break;
      }
      case 400:
      {
        responseText = "Bad Request";
      }
      //case 500:
      //{
      //}
      case 500:
      default:
      {
        pRequest->_rest_rc_ = 500;
        responseText = "Internal Server Error";
        break;
      }
    }
    builder->status(pRequest->_rest_rc_, responseText);
    if (FLAGS_request_number) {
      builder->header("Request-Number",
                    folly::to<std::string>(pRequest->_id_));
    }
    
    #ifdef TIME_MEASURES
    builder->header("Execution-time",
                    folly::to<std::string>( std::chrono::duration<double>(time).count()));

    #endif
    //req->getHeaders().forEach([&](std::string& name, std::string& value) {
    //  builder->header(folly::to<std::string>("x-proxygen-", name), value);
    //});

    std::string appVersion = std::to_string(T5GLOBVERSION) + ":" + std::to_string(T5MAJVERSION) + ":" + std::to_string(T5MINVERSION)  ;
    builder->header("t5memory-version", appVersion);  
    if(pRequest->command == COMMAND::EXPORT_MEM_INTERNAL_FORMAT){
      builder->header("Content-Type", "application/zip");
    } else{
      builder->header("Content-Type", "application/json");
    }
    
    //T5LOG( T5DEBUG) <<  ":: command = ", 
    //            CommandToStringsMap.find(this->command)->second, ", pRequest->strMemName = ", pRequest->strMemName.c_str());
    if(pRequest->outputMessage.size())
      builder->body(pRequest->outputMessage);
    //builder->send();
    //delete pRequest;
    //pRequest = nullptr;
    pRequest->fRunning = false;
    T5Logger::GetInstance()->ResetLogBuffer();
    builder->sendWithEOM();

}


} // namespace ProxygenService
//#endif
