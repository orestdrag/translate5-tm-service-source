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
        //{ ProxygenHandler::COMMAND::IMPORT_MEM_INTERNAL_FORMAT, "IMPORT_MEM_INTERNAL_FORMAT" },
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

    switch(this->command){
      case COMMAND::STATUS_MEM:
      {
        iRC = pMemService->getStatus( memName, strResponseBody );
        responseText = "OK";
        iRC = 200;
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
        iRC = 200;
        pMemService->tagReplacement(str, iRC);
        //builder->status(400, "WRONG REQUEST");
        break;
      }
      case COMMAND::EXPORT_MEM:
      case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
      {
        iRC = pMemService->getMem( memName, requestAcceptHeader,  vMemData);
        builder->body(std::string(vMemData.begin(), vMemData.end()));
        responseText = "OK";
        iRC = 200;
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
        iRC = 200;
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
    case COMMAND::CREATE_MEM:
    case COMMAND::IMPORT_MEM:
    //case COMMAND::IMPORT_MEM_INTERNAL_FORMAT:
    {
      if(body_ == nullptr){
        body_ = std::move(body);
      }else{
        //body_->appendToChain(std::move(body));
        //deprecated call of previous line, to compile with docker
        body_->prependChain(std::move(body));
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
    //}else if(command == COMMAND::IMPORT_MEM_INTERNAL_FORMAT)
    //{
    //  iRC = pMemService->import( memName, (char*) body_->data(), strResponseBody );
    }else if(command == COMMAND::CREATE_MEM){    
      body_->coalesce();  
      iRC = pMemService->createMemory( (char*) body_->data(), strResponseBody );
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
