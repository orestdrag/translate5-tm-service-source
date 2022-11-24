#pragma once

#include <stdint.h>
#include <atomic> 
//#include "ProxygenHandler.h"

using namespace std;

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

  virtual void recordRequest(int command) ;

  virtual uint64_t getRequestCount() {
    return reqCount_;
  }
  
  virtual uint64_t getCreateMemRequestCount() {
    return createMemReqCount_;
  }
  
  virtual uint64_t getDeleteMemRequestCount() {
    return deleteMemReqCount_;
  }

  virtual uint64_t getImportMemRequestCount() {
    return importMemReqCount_;
  }
  virtual uint64_t getExportMemRequestCount() {
    return exportMemReqCount_;
  }
  virtual uint64_t getStatusMemRequestCount() {
    return statusMemReqCount_;
  }
  virtual uint64_t getFuzzyRequestCount() {
    return fuzzyReqCount_;
  }
  virtual uint64_t getConcordanceRequestCount() {
    return concordanceReqCount_;
  }
  virtual uint64_t getUpdateEntryRequestCount() {
    return updateEntryReqCount_;
  }
  virtual uint64_t getDeleteEntryRequestCount() {
    return deleteEntryReqCount_;
  }
  virtual uint64_t getSaveAllTmsRequestCount() {
    return saveAllTmsReqCount_;
  }
  virtual uint64_t getListOfMemoriesRequestCount() {
    return getListOfMemoriesReqCount_;
  }
  virtual uint64_t getResourcesRequestCount() {
    return resourcesReqCount_;
  }
  virtual uint64_t getUnrecognizedRequestCount(){
    return unrecognizedRequestCount_;
  }
  virtual uint64_t getOtherRequestCount(){
    return otherRequestCount_;
  }

 private:
  atomic_uint64_t reqCount_{0};


  atomic_uint64_t createMemReqCount_{0};
  atomic_uint64_t deleteMemReqCount_{0};
  atomic_uint64_t exportMemReqCount_{0};
  atomic_uint64_t importMemReqCount_{0};
  atomic_uint64_t statusMemReqCount_{0};
  
  atomic_uint64_t fuzzyReqCount_{0};
  atomic_uint64_t concordanceReqCount_{0};
  atomic_uint64_t updateEntryReqCount_{0};
  atomic_uint64_t deleteEntryReqCount_{0};

  atomic_uint64_t saveAllTmsReqCount_{0};
  atomic_uint64_t getListOfMemoriesReqCount_{0};
  atomic_uint64_t resourcesReqCount_{0};

  atomic_uint64_t otherRequestCount_{0};
  atomic_uint64_t unrecognizedRequestCount_{0};

};

} // namespace ProxygenService
