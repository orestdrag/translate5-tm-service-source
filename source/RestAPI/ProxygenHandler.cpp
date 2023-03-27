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

const std::map<const ProxygenHandler::COMMAND,const char*> CommandToStringsMap {
        { ProxygenHandler::COMMAND::UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
        { ProxygenHandler::COMMAND::LIST_OF_MEMORIES, "LIST_OF_MEMORIES" },
        { ProxygenHandler::COMMAND::SAVE_ALL_TM_ON_DISK, "SAVE_ALL_TM_ON_DISK" },
        { ProxygenHandler::COMMAND::SHUTDOWN, "SHUTDOWN" },
        { ProxygenHandler::COMMAND::DELETE_MEM, "DELETE_MEM" },
        { ProxygenHandler::COMMAND::EXPORT_MEM, "EXPORT_MEM" },
        { ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { ProxygenHandler::COMMAND::STATUS_MEM, "STATUS_MEM" },
        { ProxygenHandler::COMMAND::RESOURCE_INFO, "RESOURCE_INFO" },
        { ProxygenHandler::COMMAND::CREATE_MEM, "CREATE_MEM" },
        { ProxygenHandler::COMMAND::FUZZY, "FUZZY" },
        { ProxygenHandler::COMMAND::CONCORDANCE, "CONCORDANCE" },
        { ProxygenHandler::COMMAND::DELETE_ENTRY, "DELETE_ENTRY" },
        { ProxygenHandler::COMMAND::UPDATE_ENTRY, "UPDATE_ENTRY" },
        { ProxygenHandler::COMMAND::TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { ProxygenHandler::COMMAND::IMPORT_MEM, "IMPORT_MEM" },
        { ProxygenHandler::COMMAND::REORGANIZE_MEM, "REORGANIZE_MEM" },
        { ProxygenHandler::COMMAND::CLONE_TM_LOCALY, "CLONE_MEM"}
    };


void ProxygenHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
  builder = new ResponseBuilder(downstream_);
  auto methodStr = req->getMethodString ();
  auto method = req->getMethod ();
  reqUrl = req->getURL () ;
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

  int id = stats_->recordRequest(this->command);
  T5Logger::GetInstance()->SetLogInfo(this->command);
  if(CommandToStringsMap.find(this->command) == CommandToStringsMap.end()){
    T5Logger::GetInstance()->SetLogBuffer(std::string("Error during ") + toStr(this->command) + " request, id = " + toStr(id));
  }else{
    T5Logger::GetInstance()->SetLogBuffer(std::string("Error during ") + CommandToStringsMap.find(this->command)->second + " request, id = " + toStr(id));
  }
  if(memName.empty() == false){
    T5Logger::GetInstance()->AddToLogBuffer(std::string(", for memory \"") + memName + "\"");
  }

  if(this->command < COMMAND::START_COMMANDS_WITH_BODY ){ // we handle here only requests without body

    switch(this->command){
      case COMMAND::STATUS_MEM:
      {
        iRC = pMemService->getStatus( memName, strResponseBody );
        break;
      }
        
      case COMMAND::LIST_OF_MEMORIES:
      {
        int ret = pMemService->list( strResponseBody );
        iRC = 200;
        break;
      }
      case COMMAND::TAGREPLACEMENTTEST:
      {
        iRC = 200;
        pMemService->tagReplacement(strResponseBody, iRC);
        break;
      }
      case COMMAND::EXPORT_MEM:
      case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
      {
        iRC = pMemService->getMem( memName, requestAcceptHeader,  vMemData);
        strResponseBody = std::string(vMemData.begin(), vMemData.end());
        iRC = 200;
        break;
      }

      case COMMAND::REORGANIZE_MEM:
      {
        iRC = 500;
        //strResponseBody = "{\n\t\"msg\": \"endpoint is not implemented\"\n}";
        iRC = pMemService->reorganizeMem( memName, strResponseBody);
        break;
      }
      
      case COMMAND::DELETE_MEM:
      {
        if(fWriteRequestsAllowed == false){
          iRC = 423;
          break;
        }
        iRC = pMemService->deleteMem( memName, strResponseBody );
        break;
      }
      case COMMAND::SAVE_ALL_TM_ON_DISK:
      {
        iRC = pMemService->saveAllTmOnDisk( strResponseBody );
        iRC = 200;
        break;
      }
      case COMMAND::RESOURCE_INFO:{
        iRC = pMemService->resourcesInfo(strResponseBody, *stats_ );
        iRC = 200;
        break;
      }
      case COMMAND::SHUTDOWN:
      { 
          fWriteRequestsAllowed = false;
          iRC = pMemService->saveAllTmOnDisk( strResponseBody );
          //check tms is in import status
          //close log file
          if(iRC == 200){
            pMemService->closeAll();
            T5Logger::GetInstance()->LogStop();          
            iRC = pMemService->shutdownService();
          }else{
            fWriteRequestsAllowed = true;
          }
          //builder->status(400, "WRONG REQUEST");
        break;
      }
      default:
      {
        
        break;
      }
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
  if(command >= COMMAND::START_COMMANDS_WITH_BODY){

    std::string strInData; 
    //strInData = ss_body.str();

    body_->coalesce();  
    strInData = (char*) body_->data(); 

    //fix garbage in json 
    size_t json_end = strInData.find("\n}") ;
    if(json_end > 0 && json_end != std::string::npos){
      //strInData[json_end + 1] = '\0';
      strInData = strInData.substr(0, json_end + 2);
    }

    std::string truncatedInput = strInData.size() > 3000 ? strInData.substr(0, 3000) : strInData;
    T5Logger::GetInstance()->SetBodyBuffer(", with body = \n\"" + truncatedInput +"\"\n");
    RequestData *pRequest = nullptr;
    switch(this->command){
      case COMMAND::CREATE_MEM:
      {
        if(fWriteRequestsAllowed == false){
          iRC = 423;
          break;
        }
        pRequest = new CreateMemRequestData(strInData);
        pRequest->strUrl = reqUrl;
        pRequest->run();
        strResponseBody = pRequest->outputMessage;
        iRC = pRequest->_rest_rc_;
        break;
      }
      case COMMAND::IMPORT_MEM:
      {
        if(fWriteRequestsAllowed == false){
          iRC = 423;
          break;
        }
        iRC = pMemService->import( memName, strInData, strResponseBody );
        break; 
      }
      case COMMAND::FUZZY:
      {
        iRC = pMemService->search( memName, strInData, strResponseBody );
        break;
      }
      case COMMAND::CONCORDANCE:
      {
        iRC = pMemService->concordanceSearch( memName, strInData, strResponseBody );
        break;
      }
      case COMMAND::UPDATE_ENTRY:
      {
        if(fWriteRequestsAllowed == false){
          break;
        }
        iRC = pMemService->updateEntry( memName, strInData,  strResponseBody );
        break;
      }
      case COMMAND::DELETE_ENTRY:
      {
        if(fWriteRequestsAllowed == false){
          iRC = 423;
          break;
        }
        iRC = pMemService->deleteEntry( memName, strInData,  strResponseBody );
        break;
      }
      case COMMAND::CLONE_TM_LOCALY:{
        if(fWriteRequestsAllowed == false){
          iRC = 423;
          break;
        }
        iRC = pMemService->cloneTMLocaly(memName, strInData, strResponseBody);
        break;
      }
      default:
      {
        iRC = 400;
        break;
      }
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
    switch(iRC){
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
    
    //T5LOG( T5DEBUG) <<  ":: command = ", 
    //            CommandToStringsMap.find(this->command)->second, ", memName = ", memName.c_str());
    if(strResponseBody.size())
      builder->body(strResponseBody);
    //builder->send();


    T5Logger::GetInstance()->ResetLogBuffer();
    builder->sendWithEOM();

}


} // namespace ProxygenService
//#endif
