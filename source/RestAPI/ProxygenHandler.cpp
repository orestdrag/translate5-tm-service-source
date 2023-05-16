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

std::atomic_bool fWriteRequestsAllowed;

using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");


namespace ProxygenService {

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {
  fWriteRequestsAllowed = true;
}

const std::map<const COMMAND,const char*> CommandToStringsMap {
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
        { UPDATE_ENTRY, "UPDATE_ENTRY" },
        { TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { IMPORT_MEM, "IMPORT_MEM" },
        { REORGANIZE_MEM, "REORGANIZE_MEM" },
        { CLONE_TM_LOCALY, "CLONE_MEM"}
    };


void ProxygenHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
  builder = new ResponseBuilder(downstream_);
  auto methodStr = req->getMethodString ();
  auto method = req->getMethod ();
  pRequest->strUrl = req->getURL () ;
  auto path = req->getPath () ;
  auto queryString   = req->getQueryString () ;
  auto headers = req->getHeaders();
  pMemService = OtmMemoryServiceWorker::getInstance();
 
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

  int id = stats_->recordRequest(pRequest->command);
  T5Logger::GetInstance()->SetLogInfo(pRequest->command);
  if(CommandToStringsMap.find(pRequest->command) == CommandToStringsMap.end()){
    T5Logger::GetInstance()->SetLogBuffer(std::string("Error during ") + toStr(pRequest->command) + " request, id = " + toStr(id));
  }else{
    T5Logger::GetInstance()->SetLogBuffer(std::string("Error during ") + CommandToStringsMap.find(pRequest->command)->second + " request, id = " + toStr(id));
  }
  if(pRequest->strMemName.empty() == false){
    T5Logger::GetInstance()->AddToLogBuffer(std::string(", for memory \"") + pRequest->strMemName + "\"");
  }

  if(pRequest->command < COMMAND::START_COMMANDS_WITH_BODY ){ // we handle here only requests without body
    if(pRequest->command == COMMAND::SHUTDOWN){
      fWriteRequestsAllowed = false;
      //pRequest->_rest_rc_ = pMemService->saveAllTmOnDisk( pRequest->outputMessage );
      auto saveTmRD = SaveAllTMsToDiskRequestData();
      saveTmRD.run();
      //check tms is in import status
      //close log file
      if(saveTmRD._rest_rc_ == 200){
        //pMemService->closeAll();
        T5Logger::GetInstance()->LogStop();          
        pRequest->run();
      }else{
        fWriteRequestsAllowed = true;
      }
    }else{
      pRequest->run();
    }
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
  if(pRequest->command >= COMMAND::START_COMMANDS_WITH_BODY)
  {
    body_->coalesce();      
    pRequest->strBody = (char*) body_->data(); 
    
    if(fWriteRequestsAllowed == false){
      pRequest->_rest_rc_ = 423;  
    }else{
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
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}

void ProxygenHandler::sendResponse()noexcept{
    switch(pRequest->_rest_rc_){
      case 200:
      case 201:
      {
        responseText = "OK";
        break;
      }
      case 423:
      {
        responseText = "WRITE REQUESTS ARE NOT ALLOWED";
        break;
      }
      //case 400:
      default:
      {
        responseText = "WRONG REQUEST";
        break;
      }
    }
    builder->status(pRequest->_rest_rc_, responseText);
    if (FLAGS_request_number) {
      builder->header("Request-Number",
                    folly::to<std::string>(stats_->getRequestCount()));
    }
    
    //req->getHeaders().forEach([&](std::string& name, std::string& value) {
    //  builder->header(folly::to<std::string>("x-proxygen-", name), value);
    //});

    char version[20];
    Properties::GetInstance()->get_value_or_default(KEY_APP_VERSION, version, 20, "");
    builder->header("t5memory-version", version);  
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


    T5Logger::GetInstance()->ResetLogBuffer();
    builder->sendWithEOM();

}


} // namespace ProxygenService
//#endif
