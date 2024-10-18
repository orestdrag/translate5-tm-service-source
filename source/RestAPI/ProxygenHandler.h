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

    boundaryLine = "--" + boundary;
    endBoundaryLine = boundaryLine + "--";
    return boundary;//"--" + boundary; // Prefix with "--" as per multipart form data boundary format
  }

 void parse(std::unique_ptr<folly::IOBuf>& body) {
    std::string boundaryLine = "--" + boundary;
    std::string endBoundaryLine = boundaryLine + "--";
    folly::io::Cursor cursor(body.get());
    std::string line;
    BodyPart currentPart = BodyPart::OTHER;

    //while (cursor.readLine(line)) {
      while (cursor.canAdvance(1)) {
            // Accumulate data into a line
            line.clear();
            char ch;
            while (cursor.canAdvance(1)) {
                ch = cursor.read<char>();
                if (ch == '\n') {
                    break;
                } else if (ch != '\r') {
                    line.push_back(ch);
                }
            }
        if (line == boundaryLine) {
            // Start of a new part
            currentPart = BodyPart::OTHER;
        } else if (line == endBoundaryLine) {
            // End of multipart message
            break;
        } else if (line.find("Content-Disposition") != std::string::npos) {
            // Header line
            if (line.find("filename=") != std::string::npos) {
                currentPart = BodyPart::FILE;
                // Extract filename and open file stream
                //auto filenamePos = line.find("filename=") + 9;
                //auto filenameEnd = line.find("\"", filenamePos);
                //std::string filename = line.substr(filenamePos, filenameEnd - filenamePos);
                //fileStream.open(filename, std::ios::binary);
            } else if (line.find("name=\"json_data\"") != std::string::npos) {
                currentPart = BodyPart::JSON;
            }
        } else {
            // Body line
            if (currentPart == BodyPart::FILE && fileStream.is_open()) {
                fileStream << line << "\n";
            } else if (currentPart == BodyPart::JSON) {
                pRequest->strBody += line + "\n";
            }
        }
    }
}

bool fBoundaryWasMissingInPreviousBodyChunk = false;

void processMultipartChunk(std::unique_ptr<folly::IOBuf>& body) {
    

    // Convert chunk to a string (or use IOBuf directly for better performance)
    std::string bodyChunk = body->moveToFbString().toStdString();
    bodyPart = bodyChunk; 
    size_t bodySize = bodyChunk.length();
    if(bool fEndBody = bodyChunk.find(endBoundaryLine) != std::string::npos){
      int g = 0;
    }
    size_t pos = 0;
    size_t nextPos = 0;

    // Process the chunk part by part
    while (pos < bodySize) {
        bool isBody = false;
        std::string line;

        isBody = fBoundaryWasMissingInPreviousBodyChunk;
        fBoundaryWasMissingInPreviousBodyChunk = false;

        // Find the boundary line in this chunk
        nextPos = bodyChunk.find(boundaryLine, pos);
        if (std::string::npos == nextPos) {
          nextPos = pos;
          //  break;  // No more parts in this chunk
        }else{
          if(isBody)
          {//start of the body is still previous data
            std::string content = bodyChunk.substr(0, nextPos);

            if (FILE == processedBodyPart) {
                fileStream.write(content.data(), content.size());
            } else if (JSON == processedBodyPart) {
                pRequest->strBody += content + '\n';
            }
            isBody = false;
          }
          pos = nextPos + boundaryLine.length();
        }
        // Skip leading CRLF or whitespace
        while (pos < bodyChunk.size() && (bodyChunk[pos] == '\r' || bodyChunk[pos] == '\n')) {
            pos++;
        }

        // Read until the next boundary or end boundary
        nextPos = bodyChunk.find(boundaryLine, pos);
        if (nextPos == std::string::npos) {
            nextPos = bodyChunk.find(endBoundaryLine, pos);
        }else{
          int a = 0;
        }

        std::string partStr;

        if(nextPos == std::string::npos) {
          partStr = bodyChunk.substr(pos);
          nextPos = bodySize;
          //isBody = true;
          fBoundaryWasMissingInPreviousBodyChunk = true;
        }else{
          partStr = bodyChunk.substr(pos, nextPos - pos);
        }
        if (partStr.empty()) {
            continue;
        }

        // Parse headers and body within the chunk
        std::istringstream partStream(partStr);
        while (std::getline(partStream, line)) {            
            if (!isBody) {
                if (line == "\r" || line.empty()) {
                    isBody = true;
                    continue;
                }
                // Parse headers
                auto delimiterPos = line.find(": ");
                if (delimiterPos != std::string::npos) {
                    std::string headerName = line.substr(0, delimiterPos);
                    std::string headerValue = line.substr(delimiterPos + 2);
                    if (headerName.find("Content-Disposition") != std::string::npos) {
                        if (headerValue.find("filename=") != std::string::npos) {
                            processedBodyPart = FILE;
                            //openFileStream(headerValue);  // Open file stream based on file name
                        } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                            processedBodyPart = JSON;
                        } else {
                            processedBodyPart = OTHER;
                        }
                    }
                    //part.headers[headerName] = headerValue;
                }
            } else {
                if (line == "\r" || line.empty()) {
                  continue;
                }
                // Handle body based on the part type (write to file or process JSON)
                size_t bodyStart = partStr.find("\r\n\r\n") + 4;  // Skip headers
                if (FILE == processedBodyPart) {
                    // Write file content directly to disk in chunks
                    //std::string fileContent = partStr.substr(bodyStart);
                    std::string fileContent = line;
                    fileStream.write(fileContent.data(), fileContent.size());
                } else if (JSON == processedBodyPart) {
                    // Accumulate JSON content (or process it incrementally)
                    //std::string jsonContent = partStr.substr(bodyStart);
                    //pRequest->strBody += jsonContent;
                    pRequest->strBody += line + '\n';
                }
            }
        }

        // Move position for the next part
        pos = nextPos + boundaryLine.length();
    }
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
        

        size_t nextEndPos = bodyStr.find(endBoundaryLine, pos);
        /*
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
        }//*/
        
        if(std::string::npos == nextPos)
        {
          //isBody = true;
          nextPos = bodyStr.length();
          //break;
        }else{
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
