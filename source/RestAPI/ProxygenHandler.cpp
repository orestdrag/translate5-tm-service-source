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
        { EXPORT_MEM_TMX, "EXPORT_MEM_TMX" },
        { EXPORT_MEM_INTERNAL_FORMAT, "EXPORT_MEM_INTERNAL_FORMAT" },
        { EXPORT_MEM_TMX_STREAM, "EXPORT_MEM_TMX_STREAM" },
        { EXPORT_MEM_INTERNAL_FORMAT_STREAM, "EXPORT_MEM_INTERNAL_FORMAT_STREAM" },
        { STATUS_MEM, "STATUS_MEM" },
        { RESOURCE_INFO, "RESOURCE_INFO" },
        { CREATE_MEM, "CREATE_MEM" },
        { FUZZY, "FUZZY" },
        { CONCORDANCE, "CONCORDANCE" },
        { DELETE_ENTRY, "DELETE_ENTRY" },
        { GET_ENTRY, "GET_ENTRY" },
        { DELETE_ENTRIES_REORGANIZE, "DELETE_ENTRIES_REORGANIZE" },
        { UPDATE_ENTRY, "UPDATE_ENTRY" },
        { TAGREPLACEMENTTEST, "TAGREPLACEMENTTEST" } ,
        { IMPORT_MEM, "IMPORT_MEM" },
        { IMPORT_MEM_STREAM, "IMPORT_MEM_STREAM" },
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


//#include <proxygen/httpserver/HTTPServer.h>
//#include <proxygen/httpserver/RequestHandler.h>
//#include <proxygen/httpserver/RequestHandlerFactory.h>

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
  pRequest->responseHandler = downstream_;   

  if(pRequest->isStreamingRequest()){ 
  }else if(EXPORT_MEM_TMX == pRequest->command){
    if(requestAcceptHeader == "application/xml"){
      pRequest->command = COMMAND::EXPORT_MEM_TMX;   
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
  headers_ = std::move(req);
}

void ProxygenHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  //ss_body << (char* ) body->data();
  if(COMMAND::IMPORT_MEM_STREAM == pRequest->command){
    //getUpload
    //((ImportStreamRequestData*) pRequest)->fileData.appendToChain(body.get());//std::move(body));
  }
  bodyQueue_.append(std::move(body));
  //if(body_ == nullptr){
  //  body_ = std::move(body);
  //}else{
  //  body_->appendToChain(std::move(body));
  //}
  
}
// Function to convert folly::IOBuf to std::vector<unsigned char>
std::vector<unsigned char> iobufToVector(const std::unique_ptr<folly::IOBuf>& buf) {
  std::vector<unsigned char> data;
  for (auto range = buf->begin(); range != buf->end(); range++) {
    data.insert(data.end(), range->begin(), range->end());
  }
  return data;
}

// Function to convert folly::IOBuf to std::string
std::string iobufToString(const std::unique_ptr<folly::IOBuf>& buf) {
  std::string data;
  for (auto range = buf->begin(); range != buf->end(); range++) {
    data.append(reinterpret_cast<const char*>(range->begin()), range->size());
  }
  return data;
}

void ProxygenHandler::onEOM() noexcept {  
  if(pRequest && pRequest->command >= COMMAND::START_COMMANDS_WITH_BODY)
  {
    auto body = bodyQueue_.move();
    if(
          (COMMAND::IMPORT_MEM == pRequest->command 
          &&  (false == ((ImportRequestData*)pRequest)->isBase64))
        || 
          (COMMAND::CREATE_MEM == pRequest->command 
          &&  (false == ((CreateMemRequestData*)pRequest)->isBase64))
        )
      {
      if (nullptr == body) 
      {
        pRequest->buildErrorReturn(400, "Requests body not found!", 400);
      }else{
        std::string boundary;
        if (headers_) {
          boundary = getBoundaryFromHeaders(*headers_);
          if (boundary.empty()) {
            proxygen::ResponseBuilder(downstream_)
              .status(400, "Bad Request")
              .sendWithEOM();
            return;
          }
        }
        auto parts = parseMultipart(body, boundary);
        for (auto& part : parts) {
          if (part.headers.at("Content-Disposition").find("filename=") != std::string::npos) {
            // Handle file part
            //((ImportStreamRequestData*) pRequest)->fileData = part.body;
            if(COMMAND::IMPORT_MEM  == pRequest->command){
              ((ImportRequestData*) pRequest)->strTmxData = iobufToString(part.body);
            }else if (COMMAND::CREATE_MEM  == pRequest->command){
              ((CreateMemRequestData*) pRequest)->binTmData = iobufToVector(part.body);
            }
          } else if (part.headers.at("Content-Disposition").find("name=\"json_data\"") != std::string::npos) {
            // Handle JSON part
            pRequest->strBody = iobufToString(part.body);
          }
        }        
      }
    }else{// not import stream 
      if(body){
        pRequest->strBody = body->moveToFbString().toStdString();
      }else if(COMMAND::EXPORT_MEM_TMX_STREAM != pRequest->command){
        pRequest->buildErrorReturn(400, "Missing body", 400);
      }
    }

    if(pRequest && !pRequest->_rest_rc_){
      size_t json_end = pRequest->strBody.find("\n}") ;
      if(json_end > 0 && json_end != std::string::npos){
        pRequest->strBody = pRequest->strBody.substr(0, json_end + 2);
      }else if(false == pRequest->strBody.empty()){
        pRequest->buildErrorReturn(117, "Request canceled:: Json body should be send in pretty format only", 400);
        T5LOG(T5ERROR) << "received json in non pretty format - canceling request;";
      }
    }

    if(pRequest && !pRequest->_rest_rc_){
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
    //while(pRequest->fRunning){};
    delete pRequest;
    pRequest = nullptr;
  }
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept { 
  if(pRequest){
    //while(pRequest->fRunning){};
    delete pRequest;
    pRequest = nullptr;
  }
  delete this;
}

void ProxygenHandler::parseRequestBody() {
  // Parse JSON body from requestBody_
  //cursor.clone(&requestBody_.);
  //jsonBody_ = cursor.clone()->moveToFbString().toStdString();
}

void ProxygenHandler::processRequest(){
  // Process the JSON body (jsonBody_) and any attached file
  // Example: Log or handle the JSON data
  T5LOG(T5INFO) << "Received JSON body: " << jsonBody_;

  // Example: Handle the attached file if present
  handleAttachedFile();
}

void ProxygenHandler::handleAttachedFile() {
    // Access and process the attached file if available
    //auto uploadStream = downstream_->getRequestBody();
    //if (uploadStream) {
    //    folly::io::Cursor cursor(uploadStream);
    //    while (!cursor.isAtEnd()) {
            //auto buf = cursor.read();
            // Example: Process 'buf' (e.g., save to file, analyze data)
            // saveToFile(buf);
    //    }
   // }
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
    
    
    //if(COMMAND::EXPORT_MEM_TMX_STREAM == pRequest->command){
      //do nothing
    //}else 
    //if(pRequest->isStreamingRequest()){
    if(COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command){
      downstream_->sendEOM();
    }else{
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
      if(COMMAND::EXPORT_MEM_INTERNAL_FORMAT == pRequest->command ){
        builder->header("Content-Type", "application/zip");
      } else if(COMMAND::EXPORT_MEM_TMX == pRequest->command){
        builder->header("Content-Type", "application/xml");
      } if(COMMAND::EXPORT_MEM_TMX_STREAM == pRequest->command){
        // Add headers to the response
        std::stringstream contDisposition;
        
        builder->header("Content-Type", "application/octet-stream"); 
        contDisposition << "attachment; filename=\"" << pRequest->strMemName  << ".tmx\"";
        builder->header("Content-Disposition", contDisposition.str());
        builder->header("NextInternalKey", ((ExportRequestData*) pRequest)->nextInternalKey);
      }else {
        builder->header("Content-Type", "application/json");
      }
      
      //T5LOG( T5DEBUG) <<  ":: command = ", 
      //            CommandToStringsMap.find(this->command)->second, ", pRequest->strMemName = ", pRequest->strMemName.c_str());
      if(pRequest->outputMessage.size())
        builder->body(pRequest->outputMessage);
      //builder->send();
      //delete pRequest;
      //pRequest = nullptr;
      builder->sendWithEOM();
    }
    pRequest->fRunning = false;
    T5Logger::GetInstance()->ResetLogBuffer();
  

}


} // namespace ProxygenService
//#endif
