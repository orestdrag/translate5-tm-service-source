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




void processMultipartChunkNew1(std::unique_ptr<folly::IOBuf>& body) {
    // Access the raw binary data directly
    const uint8_t* data = body->data();
    size_t bodySize = body->length();

    size_t pos = 0;
    size_t nextPos = 0;

    // Process the chunk part by part
    while (pos < bodySize) {
        bool isBody = false;

        isBody = fBoundaryWasMissingInPreviousBodyChunk;
        fBoundaryWasMissingInPreviousBodyChunk = false;

        // Find the boundary line in this chunk
        nextPos = findBoundary(data, pos, bodySize, boundaryLine);
        if (nextPos == std::string::npos) {
            nextPos = pos;
        } else {
            if (isBody) {
                // Content data before the next boundary
                size_t contentSize = nextPos - pos;
                const uint8_t* contentData = data + pos;

                if (processedBodyPart == FILE) {
                    // Write binary data directly to the file
                    fileStream.write(reinterpret_cast<const char*>(contentData), contentSize);
                } else if (processedBodyPart == JSON) {
                    // Append JSON data (assuming it's text)
                    pRequest->strBody.append(reinterpret_cast<const char*>(contentData), contentSize);
                }
                isBody = false;
            }
            pos = nextPos + boundaryLine.length();
        }

        // Skip leading CRLF or whitespace
        while (pos < bodySize && (data[pos] == '\r' || data[pos] == '\n')) {
            pos++;
        }

        // Read until the next boundary or end boundary
        nextPos = findBoundary(data, pos, bodySize, boundaryLine);
        if (nextPos == std::string::npos) {
            nextPos = findBoundary(data, pos, bodySize, endBoundaryLine);
        }

        size_t partSize = (nextPos == std::string::npos) ? bodySize - pos : nextPos - pos;
        const uint8_t* partData = data + pos;

        if (partSize == 0) {
            continue;
        }

        // Parse headers and body within the chunk
        // Use a state machine or similar mechanism to parse headers and the body
        std::istringstream partStream(std::string(reinterpret_cast<const char*>(partData), partSize));
        std::string line;
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
                        } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                            processedBodyPart = JSON;
                        } else {
                            processedBodyPart = OTHER;
                        }
                    }
                }
            } else {
                size_t bodyStart = partStream.str().find("\r\n\r\n") + 4; // Skip headers
                if (FILE == processedBodyPart) {
                    // Write binary data directly to disk in chunks
                    fileStream.write(reinterpret_cast<const char*>(partData + bodyStart), partSize - bodyStart);
                } else if (JSON == processedBodyPart) {
                    pRequest->strBody.append(reinterpret_cast<const char*>(partData + bodyStart), partSize - bodyStart);
                }
            }
        }

        // Move position for the next part
        pos = nextPos + boundaryLine.length();
    }
}

void processMultipartChunkNew2(std::unique_ptr<folly::IOBuf>& body) {
    // Directly access IOBuf data for binary handling
    size_t bodySize = body->length();
    const uint8_t* bodyData = body->data();

    size_t pos = 0;
    size_t nextPos = 0;

    // Process the chunk part by part
    while (pos < bodySize) {
        bool isBody = fBoundaryWasMissingInPreviousBodyChunk;
        fBoundaryWasMissingInPreviousBodyChunk = false;

        // Find the boundary line in this chunk
        nextPos = findBinaryBoundary(bodyData, bodySize, pos, boundaryLine);
        if (nextPos == std::string::npos) {
            nextPos = pos;
        } else {
            if (isBody) {  // Start of the body is in previous data
                const uint8_t* contentStart = bodyData;
                size_t contentSize = nextPos;
                
                if (processedBodyPart == FILE) {
                    fileStream.write(reinterpret_cast<const char*>(contentStart), contentSize);
                } else if (processedBodyPart == JSON) {
                    pRequest->strBody.append(reinterpret_cast<const char*>(contentStart), contentSize);
                    pRequest->strBody.push_back('\n');
                }
                isBody = false;
            }
            pos = nextPos + boundaryLine.size();
        }

        // Skip leading CRLF or whitespace
        while (pos < bodySize && (bodyData[pos] == '\r' || bodyData[pos] == '\n')) {
            pos++;
        }

        // Read until the next boundary or end boundary
        nextPos = findBinaryBoundary(bodyData, bodySize, pos, boundaryLine);
        if (nextPos == std::string::npos) {
            nextPos = findBinaryBoundary(bodyData, bodySize, pos, endBoundaryLine);
        }

        size_t partSize = (nextPos == std::string::npos) ? (bodySize - pos) : (nextPos - pos);
        const uint8_t* partStart = bodyData + pos;

        if (partSize == 0) {
            continue;
        }

        // Parse headers and body within the chunk
        const uint8_t* lineStart = partStart;
        const uint8_t* lineEnd;
        size_t lineLength;
        bool isBodySection = false;
        
        while (lineStart < partStart + partSize) {
            lineEnd = findLineEnd(lineStart, partStart + partSize - lineStart);
            lineLength = lineEnd - lineStart;

            if (!isBodySection) {
                if (lineLength == 0 || ((*lineStart == '\r' || *lineStart == '\n') && lineLength == 1)) {
                    isBodySection = true;
                    lineStart = lineEnd + 1;  // Move to the start of the body content
                    continue;
                }

                // Parse headers
                auto delimiterPos = std::find(lineStart, lineEnd, ':');
                if (delimiterPos != lineEnd) {
                    std::string headerName(reinterpret_cast<const char*>(lineStart), delimiterPos - lineStart);
                    std::string headerValue(reinterpret_cast<const char*>(delimiterPos + 2), lineEnd - delimiterPos - 2);

                    if (headerName.find("Content-Disposition") != std::string::npos) {
                        if (headerValue.find("filename=") != std::string::npos) {
                            processedBodyPart = FILE;
                        } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                            processedBodyPart = JSON;
                        } else {
                            processedBodyPart = OTHER;
                        }
                    }
                }
            } else {
                if (processedBodyPart == FILE) {
                    fileStream.write(reinterpret_cast<const char*>(lineStart), lineLength);
                } else if (processedBodyPart == JSON) {
                    pRequest->strBody.append(reinterpret_cast<const char*>(lineStart), lineLength);
                    pRequest->strBody.push_back('\n');
                }
            }
            lineStart = lineEnd + 1;
        }

        // Move position for the next part
        pos = nextPos + boundaryLine.size();
    }
}



void processMultipartChunkOld(std::unique_ptr<folly::IOBuf>& body) {
    // Convert chunk to a string (or use IOBuf directly for better performance)
    std::string bodyChunk = body->moveToFbString().toStdString();
    bodyPart = bodyChunk; 
    size_t bodySize = bodyChunk.length();
    T5LOG(T5TRANSACTION) << "processing body with len:" << bodySize << ", body:" << bodyChunk;

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
                T5LOG(T5TRANSACTION) << "writting data to the file len:" << content.size() << ", data:" << content;
                fileStream.write(content.data(), content.size());
            } else if (JSON == processedBodyPart) {
              T5LOG(T5TRANSACTION) << "parsed json data:" << content.size() << ", data:" << content;
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
                    std::string fileContent = line + '\n';
                    T5LOG(T5TRANSACTION) << "writting line to the file len:" << line.size() << ", data:" << line;
                    fileStream.write(fileContent.data(), fileContent.size());
                } else if (JSON == processedBodyPart) {
                    // Accumulate JSON content (or process it incrementally)
                    T5LOG(T5TRANSACTION) << "parsed json, len:" << line.size() << ", data:" << line;
                    pRequest->strBody += line + '\n';
                }
            }
        }

        // Move position for the next part
        pos = nextPos + boundaryLine.length();
    }
}

/*
void processMultipartChunkOld2(std::unique_ptr<folly::IOBuf>& body) {
    // Convert chunk to a string (or use IOBuf directly for better performance)
    size_t bodySize = body->length();
    const uint8_t* bodyData = body->data();
    std::string log_str(reinterpret_cast<const char*>(bodyData), bodySize);
    T5LOG(T5TRANSACTION) << " data: " << log_str;
    //std::string bodyChunk = body->moveToFbString().toStdString();
    //bodyPart = bodyChunk; 
    //size_t bodySize = bodyChunk.length();
    //T5LOG(T5TRANSACTION) << "processing body with len:" << bodySize << ", body:" << bodyChunk;

    //if(bool fEndBody = bodyChunk.find(endBoundaryLine) != std::string::npos){
    //  int g = 0;
    //}
    size_t pos = 0;
    size_t nextPos = 0;

    // Process the chunk part by part
    while (pos < bodySize) {
        bool isBody = false;
        std::string line;

        isBody = fBoundaryWasMissingInPreviousBodyChunk;
        fBoundaryWasMissingInPreviousBodyChunk = false;

        // Find the boundary line in this chunk
        nextPos = findBinaryBoundary(bodyData, bodySize, pos, boundaryLine);
        //nextPos = bodyChunk.find(boundaryLine, pos);
        if (std::string::npos == nextPos) {
          nextPos = pos;
          //  break;  // No more parts in this chunk
        }else{
          if(isBody)
          {//start of the body is still previous data
            //std::string content = bodyChunk.substr(0, nextPos);
            //uint8_t content = 

            if (FILE == processedBodyPart) {
                fileStream.write(reinterpret_cast<const char*>(bodyData), nextPos);
                //fileStream.write(content.data(), content.size());
            } else if (JSON == processedBodyPart) {
              //T5LOG(T5TRANSACTION) << "parsed json data:" << content.size() << ", data:" << content;
              std::string content; 
              content = std::string(reinterpret_cast<const char*>(bodyData), nextPos);
              pRequest->strBody += content + '\n';
            }
            isBody = false;
          }
          pos = nextPos + boundaryLine.length();
        }
        // Skip leading CRLF or whitespace
        while (pos < bodySize && (bodyData[pos] == '\r' || bodyData[pos] == '\n')) {
            pos++;
        }

        // Read until the next boundary or end boundary
        //nextPos = bodyChunk.find(boundaryLine, pos);
        nextPos = findBinaryBoundary(bodyData, bodySize, pos, boundaryLine);
        if (nextPos == std::string::npos) {
          //nextPos = bodyChunk.find(endBoundaryLine, pos);
          nextPos = findBinaryBoundary(bodyData, bodySize, pos, endBoundaryLine);
        }else{
          int a = 0;
        }

        //size_t partStart = 0;
        //size_t partEnd = 0;

        if(nextPos == std::string::npos) {          
          nextPos = bodySize;
          fBoundaryWasMissingInPreviousBodyChunk = true;
        }else{
          //nextPos = nextPos + pos;

        }
        
        //partStart = pos;
        //partEnd = nextPos;

        //if (partStr.empty()) {
        //    continue;
        //}

        // Parse headers and body within the chunk
        //std::istringstream partStream(partStr);

        //while (partStart < partEnd)//std::getline(partStream, line)) 
        bool isBody = fBoundaryWasMissingInPreviousBodyChunk;
        const uint8_t* lineStart = bodyData + pos;
        const uint8_t* partEnd = bodyData + bodySize;//bodyData+nextPos;
        const uint8_t* lineEnd = findLineEnd(lineStart, partEnd-lineStart);
        while(lineStart < partEnd)
        {            

          std::string partStr(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
          T5LOG(T5TRANSACTION) << "partStr = " << partStr;

          if (!isBody) {
              std::string headerValue(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
              if (headerValue == "\r" || headerValue == "\n" || headerValue == "\r\n" || headerValue.empty()) {
                  isBody = true;
                  lineStart += headerValue.size(); //*lineEnd == '\r'? lineEnd+2 : lineEnd + 1;
                  lineEnd = findLineEnd(lineStart, partEnd-lineStart);
                  continue;
              }
              // Parse headers
              if (headerValue.find("filename=") != std::string::npos) {
                processedBodyPart = FILE;
                //isBody = true;
              } else if (headerValue.find("name=\"json_data\"") != std::string::npos) {
                processedBodyPart = JSON;
                //  isBody = true;
              } else {
                //processedBodyPart = OTHER;
              }
              //lineStart += headerValue.size();
          } else {
              if(findBinaryBoundary(lineStart, lineEnd-lineStart, 0, boundaryLine) != std::string::npos 
                || findBinaryBoundary(lineStart, lineEnd-lineStart, 0, endBoundaryLine) != std::string::npos)
              {
                processedBodyPart = OTHER;
                isBody = false;
              }
              //if (line == "\r" || line.empty()) {
              //  continue;
              //}
              // Handle body based on the part type (write to file or process JSON)
              //size_t bodyStart = partStr.find("\r\n\r\n") + 4;  // Skip headers
              if (FILE == processedBodyPart) {
                  // Write file content directly to disk in chunks
                  //std::string fileContent = line;
                  
                  //T5LOG(T5TRANSACTION) << "writting line to the file len:" << line.size() << ", data:" << line;
                  //fileStream.write(fileContent.data(), fileContent.size());
                  //fileStream.write(reinterpret_cast<const char*>(bodyData + partStart), partEnd-partStart);
                  fileStream.write(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
                  //partStart = partEnd;
              } else if (JSON == processedBodyPart) {
                  // Accumulate JSON content (or process it incrementally)
                  std::string json_data(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
                  T5LOG(T5TRANSACTION) << "parsed json, len:" << json_data.size() << ", json_data:" << json_data;
                  pRequest->strBody += json_data;// + '\n';
                  
              }
          }

          lineStart = lineEnd;// *lineEnd == '\r'? lineEnd+2 : lineEnd + 1;
          lineEnd = findLineEnd(lineStart, partEnd-lineStart);
      }

      // Move position for the next part
      //pos = nextPos + boundaryLine.length();
      pos = bodySize;
    }
}//*/


void processMultipartChunk(std::unique_ptr<folly::IOBuf>& body) {
    // Convert chunk to a string (or use IOBuf directly for better performance)
    size_t bodySize = body->length();
    const uint8_t* bodyData = body->data();
    std::string log_str(reinterpret_cast<const char*>(bodyData), bodySize);
    T5LOG(T5TRANSACTION) << " data: " << log_str;
    
    bool isBody = fBoundaryWasMissingInPreviousBodyChunk;

    const uint8_t* lineStart = bodyData;
    const uint8_t* partEnd = bodyData + bodySize;
    const uint8_t* lineEnd = findLineEnd(lineStart, partEnd-lineStart);

    while(lineStart < partEnd)
    {            
        std::string partStr(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
        T5LOG(T5TRANSACTION) << "partStr = " << partStr;

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
            } else {
                //processedBodyPart = OTHER;
            }
                //lineStart += headerValue.size();
        } else {
            if(findBinaryBoundary(lineStart, lineEnd-lineStart, 0, boundaryLine) != std::string::npos
                || findBinaryBoundary(lineStart, lineEnd-lineStart, 0, endBoundaryLine) != std::string::npos){
                    if(findBinaryBoundary(lineStart, lineEnd-lineStart, 0, endBoundaryLine) != std::string::npos){
                        int a = 0;
                    }
                processedBodyPart = OTHER;
                fBoundaryWasMissingInPreviousBodyChunk = isBody = false;
            }else if (FILE == processedBodyPart) 
            {
                // Write file content directly to disk in chunks                  
                fileStream.write(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
                //partStart = partEnd;
            } else if (JSON == processedBodyPart) {
                // Accumulate JSON content (or process it incrementally)
                std::string json_data(reinterpret_cast<const char*>(lineStart), lineEnd-lineStart);
                T5LOG(T5TRANSACTION) << "parsed json, len:" << json_data.size() << ", json_data:" << json_data;
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
