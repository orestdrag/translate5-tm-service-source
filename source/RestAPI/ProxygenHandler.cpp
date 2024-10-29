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
#include "lowlevelotmdatastructs.h"


using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");


namespace ProxygenService {

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {
  TMManager::GetInstance()->fWriteRequestsAllowed = true;
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
  headers_ = std::move(req);

  T5LOG(T5DEBUG) << "received request url:\"" << pRequest->strUrl <<"\"; type " << searchInCommandToStringsMap(pRequest->command);  
 
  std::string requestAcceptHeader = headers.getSingleOrEmpty("Accept");
  const auto& contentType = headers.getSingleOrEmpty("Content-Type");
  if (contentType.find("multipart/form-data") != std::string::npos) {
    pRequest->fMultipartFormData = true;    
  }

  pRequest->responseHandler = downstream_;   


  if( pRequest->fMultipartFormData
    ||  IMPORT_MEM == pRequest->command
    || IMPORT_MEM_STREAM == pRequest->command
    || EXPORT_MEM_TMX_STREAM == pRequest->command
    || EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command)
  {
    std::string reservedName = pRequest->reserveFilename();
    // Open a file to store the uploaded file data
    fileStream.open(reservedName, std::ios::binary | std::ios::app);
    if (!fileStream.is_open()) {
      pRequest->buildErrorReturn(500, "Failed to open file", 500);
    }
  }
  
  if(IMPORT_MEM == pRequest->command || IMPORT_MEM_STREAM == pRequest->command
    || (pRequest->fMultipartFormData && CREATE_MEM == pRequest->command))
  {
    if (headers_) {
      boundary = getBoundaryFromHeaders(*headers_);
      
      
      if (boundary.empty()) {
        proxygen::ResponseBuilder(downstream_)
          .status(400, "Bad Request")
          .sendWithEOM();
        return;
      }
  }
    // reserve filename
    if (IMPORT_MEM == pRequest->command && contentType.find("multipart/form-data") == std::string::npos) {
      pRequest->buildErrorReturn(400, "Invalid Content-Type", 400);
    }
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
}


// Function to convert folly::IOBuf to std::vector<unsigned char>
std::vector<unsigned char> iobufToVector(const std::unique_ptr<folly::IOBuf>& buf) {
  std::vector<unsigned char> data;
  for (auto range = buf->begin(); range != buf->end(); range++) {
    data.insert(data.end(), range->begin(), range->end());
  }
  return data;
}



std::unique_ptr<folly::IOBuf> vectorToIobuf(const std::vector<unsigned char>& data) {
    // Create an IOBuf with the data from the vector
    auto buf = folly::IOBuf::copyBuffer(data.data(), data.size());
    return buf;
}



std::string iobufToString(const std::unique_ptr<folly::IOBuf>& buf){
  std::string data;
  for (auto range = buf->begin(); range != buf->end(); range++) {
    data.append(reinterpret_cast<const char*>(range->begin()), range->size());
  }
  return data;
}

std::unordered_map<std::string, std::string> parseMultipartHeaders(const std::unique_ptr<folly::IOBuf>& body) {
    std::unordered_map<std::string, std::string> headers;

    // Convert IOBuf to string for easier processing
    std::string bodyString = iobufToString(body);//folly::IOBufToString(body.get());

    // Split the body into lines
    std::istringstream stream(bodyString);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Check for the end of headers (typically a blank line)
        if (line.empty()) {
            break;
        }

        // Find the colon that separates the header name and value
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            headers[name] = value; // Store the header name and value
        }
    }

    return headers;
}


void ProxygenHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if(body){
    ++bodyPartsReceived;
  
    if(pRequest->fMultipartFormData){
      processMultipartChunk(body);
    }else{
      bodyQueue_.append(std::move(body));  // Default case for other commands
    }
  }
}

void ProxygenHandler::onEOM() noexcept {  
  if (fileStream.is_open()) {
    fileStream.close();
  }

  if(pRequest && pRequest->command >= COMMAND::START_COMMANDS_WITH_BODY)
  {
    auto body = bodyQueue_.move();
    { 
     if(COMMAND::IMPORT_MEM_STREAM == pRequest->command  ||
        COMMAND::IMPORT_MEM == pRequest->command){
        // After processing, reset any state if necessary
        processedBodyPart = OTHER;  // Reset flag after finishing file part
      }

      if(body){
        pRequest->strBody = body->moveToFbString().toStdString();
      }
      
      if(!pRequest->strBody.empty()){
        size_t json_end = pRequest->strBody.find("\n}") ;
        if(json_end > 0 && json_end != std::string::npos){
          pRequest->strBody = pRequest->strBody.substr(0, json_end + 2);
        }else if(false == pRequest->strBody.empty()){
          pRequest->buildErrorReturn(117, "Request canceled:: Json body should be send in pretty format only", 400);
          T5LOG(T5ERROR) << "received json in non pretty format - canceling request;";
        }
      }else if(//body is optional for this requests
           COMMAND::IMPORT_MEM
        || COMMAND::IMPORT_MEM_STREAM
        || COMMAND::EXPORT_MEM_TMX_STREAM
      ){
        //do nothing
      }else{
        pRequest->buildErrorReturn(400, "Missing body", 400);
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
    //TimedMutexGuard l(pRequest->request_mutex);
    //while(pRequest->fRunning){};
    delete pRequest;
    pRequest = nullptr;
  }
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept { 
  if(pRequest){
    //while(pRequest->fRunning){};
    //TimedMutexGuard l(pRequest->request_mutex);
    delete pRequest;
    pRequest = nullptr;
  }
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
    
    {
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
      if(COMMAND::EXPORT_MEM_INTERNAL_FORMAT == pRequest->command  && pRequest->isSuccessful()){
        builder->header("Content-Type", "application/zip");
      } else if(COMMAND::EXPORT_MEM_TMX == pRequest->command  && pRequest->isSuccessful()){
        builder->header("Content-Type", "application/xml");
      } if(COMMAND::EXPORT_MEM_TMX_STREAM == pRequest->command && pRequest->isSuccessful()){
        // Add headers to the response
        std::stringstream contDisposition;
        ExportRequestData* exp_request = (ExportRequestData*) pRequest;
        
        builder->header("Content-Type", "application/octet-stream"); 
        contDisposition << "attachment; filename=\"" << pRequest->strMemName  << ".tmx\"";
        builder->header("Content-Disposition", contDisposition.str());
        builder->header("NextInternalKey", exp_request->nextInternalKey);
      }else if (COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command  && pRequest->isSuccessful()){

        std::stringstream contDisposition;
        contDisposition << "attachment; filename=\"" << pRequest->strMemName  << ".tm\"";
        builder->header("Content-Type", "application/octet-stream");
        builder->header("Content-Disposition", contDisposition.str()); 
        
      }else{
        builder->header("Content-Type", "application/json");
      }
      
      //T5LOG( T5DEBUG) <<  ":: command = ", 
      //            CommandToStringsMap.find(this->command)->second, ", pRequest->strMemName = ", pRequest->strMemName.c_str());
      if( pRequest->isSuccessful() && (
          COMMAND::EXPORT_MEM_TMX == pRequest->command
        || COMMAND::EXPORT_MEM_TMX_STREAM == pRequest->command
        || COMMAND::EXPORT_MEM_INTERNAL_FORMAT == pRequest->command
        || COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command
        )
      ){
        // Respond with OK status
        pRequest->sendStreamFile(builder);
      }else if(pRequest->outputMessage.size()){
        builder->body(pRequest->outputMessage);
        builder->sendWithEOM();
      } else{
        builder->body("{}");
        builder->sendWithEOM();
      }
    }
    pRequest->fRunning = false;
    T5Logger::GetInstance()->ResetLogBuffer();
}

} // namespace ProxygenService
//#endif
