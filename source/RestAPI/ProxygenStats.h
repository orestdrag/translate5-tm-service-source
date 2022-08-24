/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace ProxygenService {

/**
 * Just some dummy class containing request count. Since we keep
 * one instance of this in each class, there is no need of
 * synchronization
 */
class ProxygenStats {
 public:
  virtual ~ProxygenStats() {
  }

  // NOTE: We make the following methods `virtual` so that we can
  //       mock them using Gmock for our C++ unit-tests. ProxygenStats
  //       is an external dependency to handler and we should be
  //       able to mock it.

  virtual void recordRequest() {
    ++reqCount_;
  }

  virtual uint64_t getRequestCount() {
    return reqCount_;
  }

 private:
  uint64_t reqCount_{0};
};

} // namespace ProxygenService
