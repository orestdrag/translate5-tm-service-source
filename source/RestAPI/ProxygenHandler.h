/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

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
 private:
  ProxygenStats* const stats_{nullptr};

  std::unique_ptr<folly::IOBuf> body_;

  void parseRequestBody();
  folly::IOBufQueue requestBody_;
  std::string jsonBody_; // Parsed JSON body
  //folly::io::Cursor cursor;// Cursor to iterate over requestBody_


  void processRequest();

  void handleAttachedFile();

  std::unique_ptr<proxygen::HTTPMessage> headers_;

#ifdef TIME_MEASURES
  //time_t startingTime;
  //time_t endingTime;
  std::chrono::milliseconds start_ms, end_ms;
#endif

  void sendResponse()noexcept;


  folly::IOBufQueue bodyQueue_{folly::IOBufQueue::cacheChainLength()};

  struct MultipartPart {
    std::unordered_map<std::string, std::string> headers;
    std::string body;
  };

  void handleFilePart(const MultipartPart& part) {
    // Handle file upload
    auto fileData = part.body;
    // Save the file or process it as needed
  }

  void handleJSONPart(const MultipartPart& part) {
    // Handle JSON data
    //auto jsonData = part.body;
    // Parse and process the JSON data
    //auto json = folly::parseJson(jsonData);
    // ...
  }

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
    return "--" + boundary; // Prefix with "--" as per multipart form data boundary format
  }

   std::vector<MultipartPart> parseMultipart(const std::string& body, const std::string& boundary) {
    std::vector<MultipartPart> parts;
    std::istringstream stream(body);
    std::string line;
    MultipartPart part;
    bool isBody = false;

    while (std::getline(stream, line)) {
      if (line.find(boundary) != std::string::npos) {
        if (!part.body.empty()) {
          parts.push_back(std::move(part));
          part = MultipartPart();
        }
        isBody = false;
      } else if (line == "\r") {
        isBody = true;
      } else if (!isBody) {
        auto delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
          std::string headerName = line.substr(0, delimiterPos);
          std::string headerValue = line.substr(delimiterPos + 2);
          part.headers[headerName] = headerValue;
        }
      } else {
        part.body += line + "\n";
      }
    }
    if (!part.body.empty()) {
      parts.push_back(std::move(part));
    }

    return parts;
  }
};

} // namespace ProxygenService
//#endif
