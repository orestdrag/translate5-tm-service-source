/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxygenHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "ProxygenStats.h"

using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");

namespace ProxygenService {

ProxygenHandler::ProxygenHandler(ProxygenStats* stats) : stats_(stats) {
}

void ProxygenHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
  stats_->recordRequest();
  ResponseBuilder builder(downstream_);
  builder.status(200, "OK");
  if (FLAGS_request_number) {
    builder.header("Request-Number",
                   folly::to<std::string>(stats_->getRequestCount()));
  }
  req->getHeaders().forEach([&](std::string& name, std::string& value) {
    builder.header(folly::to<std::string>("x-proxygen-", name), value);
  });
  builder.send();
}

void ProxygenHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  ResponseBuilder(downstream_).body(std::move(body)).send();
}

void ProxygenHandler::onEOM() noexcept {
  ResponseBuilder(downstream_).sendWithEOM();
}

void ProxygenHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void ProxygenHandler::requestComplete() noexcept {
  delete this;
}

void ProxygenHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}
} // namespace ProxygenService
