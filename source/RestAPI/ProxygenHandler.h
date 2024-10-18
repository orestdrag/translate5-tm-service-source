/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fstream>
#include <folly/Memory.h>
//#ifdef TEMPORARY_COMMENTED
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <map>
#include <string>
#include <time.h>
#include "OtmMemoryServiceWorker.h"
#include "ProxygenStats.h"
#include <folly/io/Cursor.h>


using namespace std::chrono;

namespace proxygen {
class ResponseHandler;
}

namespace ProxygenService {
  
  enum BodyPart{
    OTHER,
    JSON,
    FILE
  };


// Function to convert folly::IOBuf to std::string
std::string iobufToString(const std::unique_ptr<folly::IOBuf>& buf);


#define TIME_MEASURES
class ProxygenHandler : public proxygen::RequestHandler {
 public:
  RequestData *pRequest = nullptr;

  explicit ProxygenHandler(ProxygenStats* stats);
  explicit ProxygenHandler(ProxygenStats* stats, std::string memoryName, std::string urlCommand);

  void onRequest(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

  std::string errorStr;
  std::string responseText = "WRONG_REQUEST";
  
  proxygen::ResponseBuilder* builder = nullptr;
  OtmMemoryServiceWorker* pMemService  = nullptr;
  std::ofstream fileStream;

 private:
  ProxygenStats* const stats_{nullptr};
  std::string boundary;

  std::unique_ptr<folly::IOBuf> body_;

  void parseRequestBody();
  folly::IOBufQueue requestBody_;
  std::string jsonBody_; // Parsed JSON body
  //folly::io::Cursor cursor;// Cursor to iterate over requestBody_


  std::unique_ptr<folly::IOBuf> leftoverBuffer;  // Buffer for handling partial data
  bool isProcessingBody = false;  // Tracks if currently processing body

  bool headersParsed = false;
  BodyPart processedBodyPart = OTHER;
  bool endBoundaryLineWasUsed = false;


  std::unique_ptr<proxygen::HTTPMessage> headers_;

#ifdef TIME_MEASURES
  //time_t startingTime;
  //time_t endingTime;
  std::chrono::milliseconds start_ms, end_ms;
#endif

  void sendResponse()noexcept;


  folly::IOBufQueue bodyQueue_{folly::IOBufQueue::cacheChainLength()};

  std::string getBoundaryFromHeaders(const proxygen::HTTPMessage& headers) {
    auto contentType = headers.getHeaders().getSingleOrEmpty("Content-Type");
    if (contentType.empty()) {
      return "";
    }
    std::string boundaryPrefix = "boundary=";
    auto boundaryPos = contentType.find(boundaryPrefix);
    if (boundaryPos == std::string::npos) {
      return "";
    }
    std::string boundary = contentType.substr(boundaryPos + boundaryPrefix.length());
    if (boundary.front() == '"' && boundary.back() == '"') {
      boundary = boundary.substr(1, boundary.length() - 2);
    }
    return boundary;//"--" + boundary; // Prefix with "--" as per multipart form data boundary format
  }

 void parseMultipart(std::unique_ptr<folly::IOBuf>& body, const std::string& boundary) {  
    std::string boundaryLine = "--" + boundary;
    std::string endBoundaryLine = boundaryLine + "--";

    std::string bodyStr = body->moveToFbString().toStdString();
    size_t pos = 0;
    size_t nextPos = 0;

    while(true){
        bool isBody = false;
        std::string line;
        std::unique_ptr<folly::IOBuf> partBody = folly::IOBuf::create(0);

        nextPos = bodyStr.find(boundaryLine, pos);
        if(std::string::npos == nextPos)
        {
          break;
        }

        size_t nextEndPos = bodyStr.find(endBoundaryLine, pos);

        if(FILE == processedBodyPart && endBoundaryLineWasUsed){
          endBoundaryLineWasUsed = false;
          //isBody = true;
          std::string bodyContent = bodyStr.substr(0, nextPos);
          processedBodyPart = FILE;
          auto data = folly::IOBuf::copyBuffer(bodyContent.data(), bodyContent.size());
          //auto data = iobufToVector(bodyContent.data());
          fileStream.write(reinterpret_cast<const char*>(data->data()), data->length());
          //processedBodyPart = OTHER;
          continue;
        }else if(JSON == processedBodyPart && endBoundaryLineWasUsed){
          endBoundaryLineWasUsed = false;
          //isBody = true;
          std::string bodyContent = bodyStr.substr(0, nextPos);
          pRequest->strBody += bodyContent;
          //processedBodyPart = OTHER;
          continue;
        }
        

        pos = nextPos + boundaryLine.length();

        // Skip any leading CRLF or whitespace
        while (pos < bodyStr.size() && (bodyStr[pos] == '\r' || bodyStr[pos] == '\n')) {
            pos++;
        }

        nextPos = bodyStr.find(boundaryLine, pos);
  
        if (nextPos == std::string::npos) {
            nextPos = bodyStr.find(endBoundaryLine, pos);
            endBoundaryLineWasUsed = true;
        }

        std::string partStr = bodyStr.substr(pos, nextPos - pos);
        if (partStr.empty()) {
            continue;
        }

        std::istringstream partStream(partStr);
        

        while (std::getline(partStream, line)) {
            if (line == "\r" || line.empty()) {
                // End of headers, start of body
                isBody = true;
                continue;
            }

            if (!isBody) {
                // Parse headers
                auto delimiterPos = line.find(": ");
                if (delimiterPos != std::string::npos) {
                    std::string headerName = line.substr(0, delimiterPos);
                    std::string headerValue = line.substr(delimiterPos + 2);
                    if (headerName.find("Content-Disposition") != std::string::npos) {
                      if (headerValue.find("filename=") != std::string::npos) {
                        processedBodyPart = FILE;
                      } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                        // Handle JSON part
                        processedBodyPart = JSON;
                      }else{
                        processedBodyPart = OTHER;
                      }
                    }
                    //part.headers[headerName] = headerValue;
                }
            } else {
                // Handle body as a block of data, not line by line
                size_t bodyStart = partStr.find("\r\n\r\n") + 4; // End of headers
                if(FILE == processedBodyPart){
                  std::string bodyContent = partStr.substr(bodyStart);
                  auto data = folly::IOBuf::copyBuffer(bodyContent.data(), bodyContent.size());
                  fileStream.write(reinterpret_cast<const char*>(data->data()), data->length());
                }else if(JSON == processedBodyPart){
                  std::string bodyContent = partStr.substr(bodyStart);
                  pRequest->strBody += bodyContent;//iobufToString(part.body);
                }
                break;
            }
        }
        // Move position for the next part
        pos = nextPos + boundaryLine.length();
    }
}

  
};

} // namespace ProxygenService
//#endif
