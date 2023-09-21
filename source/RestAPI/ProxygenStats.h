#pragma once

#include <stdint.h>
#include <atomic> 
//#include "ProxygenHandler.h"

using namespace std;

//namespace ProxygenService {

/**
 * Just some dummy class containing request count. Since we keep
 * one instance of this in each class, there is no need of
 * synchronization
 */
class ProxygenStats {
 public:
  virtual ~ProxygenStats() {
  }

  ///input- command id, output - requests id

  virtual int recordRequest(int command) ;

  virtual uint64_t getRequestCount() {
    return reqCount_;
  }
  
  virtual uint64_t  getReorganizeRequestCount(){
    return reorganizeMemRequestCount_;
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

  virtual uint64_t getImportLocalMemRequestCount(){
    return importLocalReqCount_;
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

  virtual uint64_t getCloneLocalyCount(){
    return cloneLocalyCount_;
  }

  virtual uint64_t getOpenedTMCount(){
    return openedTm_;
  }
  virtual uint64_t getClosedTMCount(){
    return closedTm_;
  }

 private:
  atomic_uint64_t reqCount_{0};


  atomic_uint64_t createMemReqCount_{0};
  atomic_uint64_t deleteMemReqCount_{0};
  atomic_uint64_t exportMemReqCount_{0};
  atomic_uint64_t importMemReqCount_{0};
  atomic_uint64_t importLocalReqCount_{0};
  atomic_uint64_t statusMemReqCount_{0};
  
  atomic_uint64_t fuzzyReqCount_{0};
  atomic_uint64_t concordanceReqCount_{0};
  atomic_uint64_t updateEntryReqCount_{0};
  atomic_uint64_t deleteEntryReqCount_{0};

  atomic_uint64_t saveAllTmsReqCount_{0};
  atomic_uint64_t getListOfMemoriesReqCount_{0};
  atomic_uint64_t resourcesReqCount_{0};
  atomic_uint64_t cloneLocalyCount_{0};
  atomic_uint64_t reorganizeMemRequestCount_{0};

  atomic_uint64_t otherRequestCount_{0};
  atomic_uint64_t unrecognizedRequestCount_{0};

  atomic_uint64_t openedTm_{0};
  atomic_uint64_t closedTm_{0};
};

//} // namespace ProxygenService
