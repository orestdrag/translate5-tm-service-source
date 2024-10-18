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
  headers_ = std::move(req);

  T5LOG(T5DEBUG) << "received request url:\"" << pRequest->strUrl <<"\"; type " << searchInCommandToStringsMap(pRequest->command);  
 
  std::string requestAcceptHeader = headers.getSingleOrEmpty("Accept");
  const auto& contentType = headers.getSingleOrEmpty("Content-Type");

  pRequest->responseHandler = downstream_;   

  //if(IMPORT_MEM_STREAM == pRequest->command)
  if(IMPORT_MEM == pRequest->command)
  {

    #ifdef v_1
    auto contentType = msg->getHeaders().getSingleOrEmpty("Content-Type");
    
    if (contentType.find("multipart/form-data") != std::string::npos) {
        boundary = getBoundaryFromHeaders(msg->getHeaders());//extractBoundaryFromHeaders(msg->getHeaders());
    }
    // Store headers for later use
    requestHeaders_ = msg->getHeaders();
    #endif
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
    if (contentType.find("multipart/form-data") == std::string::npos) {
      pRequest->buildErrorReturn(400, "Invalid Content-Type", 400);
    }else{
      auto pImportRequest = (ImportRequestData*) pRequest;

      std::string reservedName = pImportRequest->reserveName();
      // Open a file to store the uploaded file data
      fileStream.open(reservedName, std::ios::binary | std::ios::app);
      if (!fileStream.is_open()) {
        pRequest->buildErrorReturn(500, "Failed to open file", 500);
      }
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

size_t ProxygenHandler::processBodyChunk(folly::ByteRange& cursor, size_t pos) {
    // Check for boundary
    std::string boundaryLine = "--" + boundary;
    auto boundaryRange = folly::StringPiece(boundaryLine);  // Convert std::string to StringPiece
    size_t boundaryPos = cursor.find(boundaryRange, pos);   // Use the find method
    //size_t boundaryPos = cursor.find(boundaryLine, pos);
    
    if (boundaryPos != std::string::npos) {
        // We've reached the boundary, stop processing the body
        size_t bodySize = boundaryPos - pos;
        fileStream.write(reinterpret_cast<const char*>(cursor.subpiece(pos, bodySize).data()), bodySize);
        
        // Reset state to parse the next part
        isProcessingBody = false;
        currentPart = MultipartPart();
        return boundaryPos + boundaryLine.length();
    }

    // No boundary yet, stream the remaining content
    fileStream.write(reinterpret_cast<const char*>(cursor.subpiece(pos).data()), cursor.size() - pos);
    return cursor.size();
}

void ProxygenHandler::parseHeaders(folly::ByteRange& cursor, size_t& pos) {
    std::string line;
    while (pos < cursor.size()) {
        char c = cursor[pos++];
        if (c == '\n' || c == '\r') {
            if (line.empty()) {
                // End of headers
                isProcessingBody = true;
                return;
            }

            auto delimiterPos = line.find(": ");
            if (delimiterPos != std::string::npos) {
                std::string headerName = line.substr(0, delimiterPos);
                std::string headerValue = line.substr(delimiterPos + 2);
                currentPart.headers[headerName] = headerValue;
            }

            line.clear();  // Prepare for the next line
        } else {
            line += c;  // Continue reading header line
        }
    }
}

void ProxygenHandler::processMultipartChunks(std::unique_ptr<folly::IOBuf>& body) {
    std::string boundaryLine = "--" + boundary;
    std::string endBoundaryLine = boundaryLine + "--";
    
    auto cursor = body->coalesce();
    size_t pos = 0;

    while (pos < cursor.size()) {
        if (!isProcessingBody) {
            // Searching for boundary
            auto boundaryRange = folly::StringPiece(boundaryLine);  // Convert std::string to StringPiece
            size_t boundaryPos = cursor.find(boundaryRange, pos);   // Use the find method
            //size_t boundaryPos = cursor.find(boundaryLine, pos);
            if (boundaryPos == std::string::npos) {
                // Boundary not found, store in leftoverBuffer and exit
                leftoverBuffer = folly::IOBuf::copyBuffer(cursor.subpiece(pos).data(), cursor.size() - pos);
                return;
            }
            
            // Parse headers after boundary
            pos = boundaryPos + boundaryLine.length();
            parseHeaders(cursor, pos);
        }

        // Process body
        if (isProcessingBody) {
            pos = processBodyChunk(cursor, pos);
        }
    }
}

/*
void ProxygenHandler::onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
    auto contentType = msg->getHeaders().getSingleOrEmpty("Content-Type");
    
    if (contentType.find("multipart/form-data") != std::string::npos) {
        boundary = getBoundaryFromHeaders(msg->getHeaders());//extractBoundaryFromHeaders(msg->getHeaders());
    }
    // Store headers for later use
    requestHeaders_ = msg->getHeaders();
}

//*/


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

#ifdef ver1
  if(COMMAND::IMPORT_MEM == pRequest->command)
  {
    auto parts = parseMultipart(body, boundary);
    for (auto& part : parts) {
      if (part.headers.empty()) {
        continue; // Skip if headers are empty
      }
      if (part.headers.at("Content-Disposition").find("filename=") != std::string::npos) {
        auto data = iobufToVector(part.body);
        fileStream.write(reinterpret_cast<const char*>(data.data()), data.size());
      } else if (part.headers.at("Content-Disposition").find("name=\"json_data\"") != std::string::npos) {
        // Handle JSON part
        pRequest->strBody += iobufToString(part.body);
      }else{
        int a= 0;
      }
    }
    body = body->pop();
  }else{
    bodyQueue_.append(std::move(body));
  }  
#elseifdef ver2
  if (COMMAND::IMPORT_MEM == pRequest->command) {
      if (leftoverBuffer) {
          // Append new data to leftover buffer
          leftoverBuffer->prependChain(std::move(body));
          body = std::move(leftoverBuffer);
      }

      // Process the body in chunks as it comes in
      processMultipartChunks(body);

  } else {
      bodyQueue_.append(std::move(body));  // Default case for other commands
  }
#else 

  if(COMMAND::IMPORT_MEM == pRequest->command){
  BodyPart prevProcessedBodyPart = processedBodyPart;

  //auto headers = parseMultipartHeaders(body);  // Parse just the headers
  auto parts = parseMultipart(body, boundary);
  /*
  if (headers.count("Content-Disposition")) {
    if (headers.at("Content-Disposition").find("filename=") != std::string::npos) {
      processedBodyPart = FILE;  // We are processing a file part
    } else if (headers.at("Content-Disposition").find("name=\"json_data\"") != std::string::npos) {
      processedBodyPart = JSON;  // We are processing JSON data
    }
  }//*/

// for (auto& part : parts) {
    //if (part.headers.empty()) {
    //  continue; // Skip if headers are empty
    //}
    //if (part.headers.count("Content-Disposition")) {
    //  if (part.headers.at("Content-Disposition").find("filename=") != std::string::npos) {
 //   if(FILE == part.bodyPart){
 //   //    processedBodyPart = FILE;
 //       auto data = iobufToVector(part.body);
 //       fileStream.write(reinterpret_cast<const char*>(data.data()), data.size());
 //   //  } else if (part.headers.at("Content-Disposition").find("name=\"json_data\"") != std::string::npos) {
 //   }else if(JSON == part.bodyPart){
        // Handle JSON part
//        processedBodyPart = JSON;
 //       pRequest->strBody += iobufToString(part.body);
 //     }else{
    //    processedBodyPart = OTHER;
 //       int a= 0;
//     }
    //}
    /*else{
      // Handle the body based on the flag
      if (FILE == processedBodyPart) {
        // Write file data to the file stream
        auto data = iobufToVector(body);  // Convert IOBuf to vector
        fileStream.write(reinterpret_cast<const char*>(data.data()), data.size());
      } else if(JSON == processedBodyPart) {
        // Append the body to the JSON string
        pRequest->strBody += iobufToString(part.body);  // Convert IOBuf to string and append
      }
    }//*/
  //}
  //if(processedBodyPart != prevProce)
  
/*
  // Handle the body based on the flag
  if (FILE == processedBodyPart) {
    // Write file data to the file stream
    auto data = iobufToVector(body);  // Convert IOBuf to vector
    fileStream.write(reinterpret_cast<const char*>(data.data()), data.size());
  } else if(JSON == processedBodyPart) {
    // Append the body to the JSON string
    pRequest->strBody += iobufToString(body);  // Convert IOBuf to string and append
 // }
  //*/
  }else{
    bodyQueue_.append(std::move(body));  // Default case for other commands
  }
#endif

}

void ProxygenHandler::onEOM() noexcept {  
  if(pRequest && pRequest->command >= COMMAND::START_COMMANDS_WITH_BODY)
  {
    auto body = bodyQueue_.move();
    {// not import stream 
      if(body){
        pRequest->strBody = body->moveToFbString().toStdString();
      }else if(COMMAND::EXPORT_MEM_TMX_STREAM == pRequest->command 
       || COMMAND::IMPORT_MEM == pRequest->command){
        // After processing, reset any state if necessary
        processedBodyPart = OTHER;  // Reset flag after finishing file part
        if (fileStream.is_open()) {
            fileStream.close();
        }
       } else{
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
    //if(COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command){
    //   downstream_->sendEOM();
    //}else
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
        
        builder->header("Content-Type", "application/octet-stream"); 
        contDisposition << "attachment; filename=\"" << pRequest->strMemName  << ".tmx\"";
        builder->header("Content-Disposition", contDisposition.str());
        builder->header("NextInternalKey", ((ExportRequestData*) pRequest)->nextInternalKey);
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
      if ( COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM == pRequest->command 
       && (( (ExportRequestData*) pRequest)->vMemData.size())
      )
      {
        ExportRequestData* pExpReqData = (ExportRequestData*)pRequest;
        builder->body(vectorToIobuf(pExpReqData->vMemData));
      }else if(pRequest->outputMessage.size()){
        builder->body(pRequest->outputMessage);
      } else{
        builder->body("{}");
      }
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
