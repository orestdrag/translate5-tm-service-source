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

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {}

const std::map<const ProxygenHandler::COMMAND,const char*> CommandToStringsMap {
        { ProxygenHandler::COMMAND::UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
        { ProxygenHandler::COMMAND::LIST_OF_MEMORIES, "LIST_OF_MEMORIES" },
        { ProxygenHandler::COMMAND::CREATE_MEM, "CREATE_MEM" },
        { ProxygenHandler::COMMAND::DELETE_MEM, "DELETE_MEM" },
        { ProxygenHandler::COMMAND::IMPORT_MEM, "IMPORT_MEM" },
        { ProxygenHandler::COMMAND::EXPORT_MEM, "EXPORT_MEM" },
        { ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { ProxygenHandler::COMMAND::STATUS_MEM, "STATUS_MEM" },
        { ProxygenHandler::COMMAND::RESOURCE_INFO, "RESOURCE_INFO" },
        { ProxygenHandler::COMMAND::FUZZY, "FUZZY" },
        { ProxygenHandler::COMMAND::CONCORDANCE, "CONCORDANCE" },
        { ProxygenHandler::COMMAND::DELETE_ENTRY, "DELETE_ENTRY" },
        { ProxygenHandler::COMMAND::UPDATE_ENTRY, "UPDATE_ENTRY" },
        { ProxygenHandler::COMMAND::TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { ProxygenHandler::COMMAND::SHUTDOWN, "SHUTDOWN" },
        { ProxygenHandler::COMMAND::SAVE_ALL_TM_ON_DISK, "SAVE_ALL_TM_ON_DISK" }
    };


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

  stats_->recordRequest(this->command);
  SetLogInfo(this->command);
  SetLogBuffer(std::string("Error during ") + CommandToStringsMap.find(this->command)->second + " request");
  if(memName.empty() == false){
    AddToLogBuffer(std::string(", for memory \"") + memName + "\"");
  }

  if(this->command < COMMAND::START_COMMANDS_WITH_BODY ){ // we handle here only requests without body

    switch(this->command){
      case COMMAND::STATUS_MEM:
      {
        iRC = pMemService->getStatus( memName, strResponseBody );
        responseText = "OK";
        iRC = 200;
        builder->body(strResponseBody);
        break;
      }
        
      case COMMAND::LIST_OF_MEMORIES:
      {
        int ret = pMemService->list( strResponseBody );
        builder->body(strResponseBody);
        responseText = "OK";
        iRC = 200;
        break;
      }
      case COMMAND::TAGREPLACEMENTTEST:
      {
        std::string str;
        iRC = 200;
        pMemService->tagReplacement(str, iRC);
        builder->body(str);
        break;
      }
      case COMMAND::EXPORT_MEM:
      case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
      {
        iRC = pMemService->getMem( memName, requestAcceptHeader,  vMemData);
        builder->body(std::string(vMemData.begin(), vMemData.end()));
        responseText = "OK";
        iRC = 200;
        break;
      }
      
      case COMMAND::DELETE_MEM:
      {
        iRC = pMemService->deleteMem( memName, strResponseBody );
        builder->body(strResponseBody);
        break;
      }
      case COMMAND::SAVE_ALL_TM_ON_DISK:
      {
        iRC = pMemService->saveAllTmOnDisk( strResponseBody );
        responseText = "OK";
        iRC = 200;
        break;
      }
      case COMMAND::RESOURCE_INFO:{
        iRC = pMemService->resourcesInfo(strResponseBody, *stats_ );
        builder->body(strResponseBody);
        responseText = "OK"; 
        //iRC = 200;
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
    properties_get_str_or_default(KEY_APP_VERSION, version, 20, "unknown");
    builder->header("t5memory-version", version);   
    builder->header("Content-Type", "application/json");

    //LogMessage5(DEBUG, __func__, ":: command = ", 
    //          CommandToStringsMap.find(this->command)->second, ", memName = ", memName.c_str());

    
    builder->sendWithEOM();
    ResetLogBuffer();
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
    AddToLogBuffer(", with body = \n\"" + truncatedInput +"\"\n");
    
    switch(this->command){
      case COMMAND::CREATE_MEM:
      {
        iRC = pMemService->createMemory( strInData, strResponseBody );
        break;
      }
      case COMMAND::IMPORT_MEM:
      {
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
        iRC = pMemService->updateEntry( memName, strInData,  strResponseBody );
        break;
      }
      case COMMAND::DELETE_ENTRY:
      {
        iRC = pMemService->deleteEntry( memName, strInData,  strResponseBody );
        break;
      }
      default:
      {
        iRC = 400;
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
    
    //LogMessage5(DEBUG, __func__, ":: command = ", 
    //            CommandToStringsMap.find(this->command)->second, ", memName = ", memName.c_str());
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
  //ResetLogBuffer();
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}
} // namespace ProxygenService
//#endif
