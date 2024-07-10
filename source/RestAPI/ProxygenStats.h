#pragma once

#include <stdint.h>
#include <atomic> 
#include <time.h>
//#include "ProxygenHandler.h"
#include <chrono>


using namespace std;
using namespace std::chrono;

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
  virtual int addRequestTime(int command, milliseconds time);
  virtual int recordRequest(int command) ;

  virtual uint64_t getRequestCount() {
    return reqCount_;
  }
  virtual milliseconds getExecutionTime() {
    return executionTime_;
  }
  
  virtual uint64_t  getReorganizeRequestCount(){
    return reorganizeMemRequestCount_;
  }
  virtual milliseconds  getReorganizeExecutionTime(){
    return reorganizeMemSumTime_;
  }

  virtual uint64_t getCreateMemRequestCount() {
    return createMemReqCount_;
  }
  virtual milliseconds getCreateMemSumTime() {
    return createMemSumTime_;
  }
  
  virtual uint64_t getDeleteMemRequestCount() {
    return deleteMemReqCount_;
  } 
  virtual milliseconds getDeleteMemSumTime() {
    return deleteMemSumTime_;
  }

  virtual uint64_t getImportMemRequestCount() {
    return importMemReqCount_;
  } 
  virtual milliseconds getImportMemSumTime() {
    return importMemSumTime_;
  }

  virtual uint64_t getImportLocalMemRequestCount(){
    return importLocalReqCount_;
  }

  virtual uint64_t getExportMemRequestCount() {
    return exportMemReqCount_;
  }
  virtual milliseconds getExportMemSumTime() {
    return exportMemSumTime_;
  }

  virtual uint64_t getStatusMemRequestCount() {
    return statusMemReqCount_;
  }
  virtual milliseconds getStatusMemSumTime() {
    return statusMemSumTime_;
  }

  virtual uint64_t getFuzzyRequestCount() {
    return fuzzyReqCount_;
  }
  virtual milliseconds getFuzzySumTime() {
    return fuzzyReqSumTime_;
  }
  
  virtual uint64_t getConcordanceRequestCount() {
    return concordanceReqCount_;
  }
  virtual milliseconds getConcordanceSumTime() {
    return concordanceSumTime_;
  }

  virtual uint64_t getUpdateEntryRequestCount() {
    return updateEntryReqCount_;
  }
  virtual milliseconds getUpdateEntrySumTime() {
    return updateEntrySumTime_;
  }
  
  virtual uint64_t getDeleteEntryRequestCount() {
    return deleteEntryReqCount_;
  }
  virtual milliseconds getDeleteEntrySumTime() {
    return deleteEntrySumTime_;
  }

  virtual uint64_t getDeleteEntriesReorganizeCount() {
    return deleteEntryReorganizeReqCount_;
  }
  virtual milliseconds getDeleteEntriesReorganizeSumTime(){
    return deleteEntryReorganizeSumTime_;
  }

  virtual uint64_t getSaveAllTmsRequestCount() {
    return saveAllTmsReqCount_;
  }
 virtual milliseconds getSaveAllTmsSumTime() {
    return saveAllTmsSumTime_;
  }

  virtual uint64_t getListOfMemoriesRequestCount() {
    return getListOfMemoriesReqCount_;
  }
  virtual milliseconds getListOfMemoriesSumTime() {
    return getListOfMemoriesSumTime_;
  }

  virtual uint64_t getResourcesRequestCount() {
    return resourcesReqCount_;
  }
  virtual milliseconds getResourcesSumTime() {
    return resourcesSumTime_;
  }

  virtual milliseconds getUnrecognizedSumTime(){
    return unrecognizedRequestSumTime_;
  }
  virtual uint64_t getUnrecognizedRequestCount(){
    return unrecognizedRequestCount_;
  }

  virtual milliseconds getEntrySumTime(){
    return getEntrySumTime_;
  }
  virtual uint64_t getEntryRequestCount(){
    return getEntryReqCount_;
  }

  virtual uint64_t getOtherRequestCount(){
    return otherRequestCount_;
  }
  virtual milliseconds getOtherSumTime(){
    return otherSumTime_;
  }

  virtual uint64_t getCloneLocalyCount(){
    return cloneLocalyCount_;
  }
  virtual milliseconds getCloneLocalySumTime(){
    return cloneLocalySumTime_;
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
  atomic_uint64_t deleteEntryReorganizeReqCount_{0};
  atomic_uint64_t exportMemReqCount_{0};
  atomic_uint64_t importMemReqCount_{0};
  atomic_uint64_t importLocalReqCount_{0};
  atomic_uint64_t statusMemReqCount_{0};
  
  atomic_uint64_t fuzzyReqCount_{0};
  atomic_uint64_t concordanceReqCount_{0};
  atomic_uint64_t updateEntryReqCount_{0};
  atomic_uint64_t deleteEntryReqCount_{0};
  atomic_uint64_t getEntryReqCount_{0};

  atomic_uint64_t saveAllTmsReqCount_{0};
  atomic_uint64_t getListOfMemoriesReqCount_{0};
  atomic_uint64_t resourcesReqCount_{0};
  atomic_uint64_t cloneLocalyCount_{0};
  atomic_uint64_t reorganizeMemRequestCount_{0};

  atomic_uint64_t otherRequestCount_{0};
  atomic_uint64_t unrecognizedRequestCount_{0};
  
  milliseconds executionTime_{0};
  milliseconds createMemSumTime_{0};
  milliseconds deleteMemSumTime_{0};
  milliseconds exportMemSumTime_{0};
  milliseconds importMemSumTime_{0};
  milliseconds statusMemSumTime_{0};
  
  milliseconds fuzzyReqSumTime_{0};
  milliseconds concordanceSumTime_{0};
  milliseconds getEntrySumTime_{0};
  milliseconds updateEntrySumTime_{0};
  milliseconds deleteEntrySumTime_{0};
  milliseconds deleteEntryReorganizeSumTime_{0};

  milliseconds saveAllTmsSumTime_{0};
  milliseconds getListOfMemoriesSumTime_{0};
  milliseconds resourcesSumTime_{0};
  milliseconds cloneLocalySumTime_{0};
  milliseconds reorganizeMemSumTime_{0};

  milliseconds otherSumTime_{0};
  milliseconds unrecognizedRequestSumTime_{0};

  atomic_uint64_t openedTm_{0};
  atomic_uint64_t closedTm_{0};
};

//} // namespace ProxygenService
