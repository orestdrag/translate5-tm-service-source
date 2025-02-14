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
  size_t bodyPartsReceived = 0;
  std::string bodyPart;

  // Global or class-level atomic counter

  explicit ProxygenHandler();
  explicit ProxygenHandler(std::string memoryName, std::string urlCommand);

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
  std::string boundary, boundaryLine, endBoundaryLine;

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

    boundaryLine = "--" + boundary;
    endBoundaryLine = boundaryLine + "--";
    return boundary;//"--" + boundary; // Prefix with "--" as per multipart form data boundary format
  }


bool fBoundaryWasMissingInPreviousBodyChunk = false;

size_t findBoundary(const uint8_t* data, size_t startPos, size_t dataSize, const std::string& boundary) {
    for (size_t i = startPos; i <= dataSize - boundary.size(); ++i) {
        if (memcmp(data + i, boundary.data(), boundary.size()) == 0) {
            return i;
        }
    }
    return std::string::npos;
}

#include <cstring>

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

size_t findBinaryBoundary(const uint8_t* data, size_t dataSize, size_t start, const std::string& boundary) {
    if (start >= dataSize) {
        return std::string::npos;
    }

    std::string trimmedBoundary = boundary;//trim(boundary);

    const uint8_t* boundaryData = reinterpret_cast<const uint8_t*>(trimmedBoundary.data());
    size_t boundarySize = trimmedBoundary.size();
    if(boundarySize >= dataSize){
        return std::string::npos;
    }

    for (size_t i = start; i <= dataSize - boundarySize; ++i) {
        if (std::memcmp(data + i, boundaryData, boundarySize) == 0) {
            return i;  // Found the boundary, return position.
        }
    }

    // Boundary not found in the data.
    return std::string::npos;
}

const uint8_t* findLineEnd(const uint8_t* start, size_t maxLength) {
    for (size_t i = 0; i < maxLength; ++i) {
        if (start[i] == '\n') {
            return start + i + 1;  // Return pointer after '\n'.
        } else if (start[i] == '\r' && i + 1 < maxLength && start[i + 1] == '\n') {
            return start + i + 2;  // Return pointer after '\r\n'.
        }
    }
    return start + maxLength;  // No line end found within maxLength.
}


void processMultipartChunk(std::unique_ptr<folly::IOBuf>& body) {
    // Convert chunk to a string (or use IOBuf directly for better performance)
    size_t bodySize = body->length();
    const uint8_t* bodyData = body->data();
    
    bool isBody = fBoundaryWasMissingInPreviousBodyChunk;

    const uint8_t* lineStart = bodyData;
    const uint8_t* partEnd = bodyData + bodySize;
    const uint8_t* lineEnd = findLineEnd(lineStart, partEnd-lineStart);

    while(lineStart < partEnd)
    {
        if (!isBody) {
            std::string headerValue(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
            if (headerValue == "\r" || headerValue == "\n" || headerValue == "\r\n" || headerValue.empty()) {
                fBoundaryWasMissingInPreviousBodyChunk = isBody = true;
                lineStart += headerValue.size(); //*lineEnd == '\r'? lineEnd+2 : lineEnd + 1;
                lineEnd = findLineEnd(lineStart, partEnd-lineStart);
                continue;
            }
            
            // Parse headers
            if (headerValue.find("filename=") != std::string::npos) {
                processedBodyPart = FILE;
            } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                processedBodyPart = JSON;
            }
        } else {
            if(findBinaryBoundary(lineStart, lineEnd-lineStart, 0, boundaryLine) != std::string::npos
            || findBinaryBoundary(lineStart, lineEnd-lineStart, 0, endBoundaryLine) != std::string::npos){                    
                processedBodyPart = OTHER;
                fBoundaryWasMissingInPreviousBodyChunk = isBody = false;
            }else if (FILE == processedBodyPart) 
            {
                // Write file content directly to disk in chunks                  
                fileStream.write(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
            } else if (JSON == processedBodyPart) {
                // Accumulate JSON content (or process it incrementally)
                std::string json_data(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
                pRequest->strBody += json_data;;
            }
        }

        lineStart = lineEnd;
        lineEnd = findLineEnd(lineStart, partEnd-lineStart);
    }
}
  
};

} // namespace ProxygenService
//#endif
