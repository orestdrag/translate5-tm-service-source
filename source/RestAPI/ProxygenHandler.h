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

#ifdef TIME_MEASURES
  //time_t startingTime;
  //time_t endingTime;
  std::chrono::milliseconds start_ms, end_ms;
#endif

  void sendResponse()noexcept;
};

} // namespace ProxygenService
//#endif
