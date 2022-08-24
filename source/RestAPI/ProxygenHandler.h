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
#include "OtmMemoryServiceWorker.h"

namespace proxygen {
class ResponseHandler;
}

namespace ProxygenService {

class ProxygenStats;

class ProxygenHandler : public proxygen::RequestHandler {
 public:
 
  explicit ProxygenHandler(ProxygenStats* stats);
  explicit ProxygenHandler(ProxygenStats* stats, std::string memoryName, std::string urlCommand);

  void onRequest(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

  enum class COMMAND {
    UNKNOWN_COMMAND,
    LIST_OF_MEMORIES,
    SAVE_ALL_TM_ON_DISK,
    SHUTDOWN,    
    DELETE_MEM,
    EXPORT_MEM,
    EXPORT_MEM_INTERNAL_FORMAT,
    STATUS_MEM,

    START_COMMANDS_WITH_BODY,
    CREATE_MEM = START_COMMANDS_WITH_BODY, 
    FUZZY,
    CONCORDANCE,
    DELETE_ENTRY,
    UPDATE_ENTRY,
    TAGREPLACEMENTTEST,
    IMPORT_MEM,
    IMPORT_MEM_INTERNAL_FORMAT
  };

  COMMAND command;

  std::string memName;
  std::string errorStr;

  std::string strResponseBody;
  std::vector<unsigned char> vMemData;
  int iRC = 400;
  std::string responseText = "WRONG_REQUEST";
  proxygen::ResponseBuilder* builder = nullptr;
  OtmMemoryServiceWorker* pMemService  = nullptr;
 private:
  ProxygenStats* const stats_{nullptr};

  std::unique_ptr<folly::IOBuf> body_;

};

} // namespace ProxygenService
//#endif
