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

namespace proxygen {
class ResponseHandler;
}

namespace ProxygenService {

class ProxygenStats;

class ProxygenHandler : public proxygen::RequestHandler {
 public:
  explicit ProxygenHandler(ProxygenStats* stats);

  void onRequest(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  ProxygenStats* const stats_{nullptr};

  std::unique_ptr<folly::IOBuf> body_;
};

} // namespace ProxygenService
//#endif
