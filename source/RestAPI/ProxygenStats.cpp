
#include "ProxygenStats.h"
#include "ProxygenHandler.h"
namespace ProxygenService{


int ProxygenStats::recordRequest(int command) {
  switch(command){
    case ProxygenHandler::COMMAND::CREATE_MEM: 
    {
      createMemReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::DELETE_MEM: 
    {
      deleteMemReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::EXPORT_MEM: 
    case ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
    {
      exportMemReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::IMPORT_MEM: 
    case ProxygenHandler::COMMAND::IMPORT_TMX_LOCALY:
    {        
      importMemReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::STATUS_MEM: 
    {        
      statusMemReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::REORGANIZE_MEM:
    {
      reorganizeMemRequestCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::FUZZY: 
    {        
      fuzzyReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::CONCORDANCE: 
    {        
      concordanceReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::UPDATE_ENTRY: 
    {        
      updateEntryReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::DELETE_ENTRY: 
    {        
      deleteEntryReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::SAVE_ALL_TM_ON_DISK: 
    {        
      saveAllTmsReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::LIST_OF_MEMORIES: 
    {        
      getListOfMemoriesReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::RESOURCE_INFO: 
    {        
      resourcesReqCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::CLONE_TM_LOCALY:
    {
      cloneLocalyCount_++;
      break;
    }
    case ProxygenHandler::COMMAND::UNKNOWN_COMMAND:{
      unrecognizedRequestCount_++;
      break;
    }    
    default:{
      otherRequestCount_++;
      break;
    }
  }
  return reqCount_++;
}


int ProxygenStats::addRequestTime(int command,milliseconds time) { 
  switch(command){
    case ProxygenHandler::COMMAND::CREATE_MEM: 
    {
      createMemSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::DELETE_MEM: 
    {
      deleteMemSumTime_+= time;
      break;
    }
    case ProxygenHandler::COMMAND::EXPORT_MEM: 
    case ProxygenHandler::COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
    {
      exportMemSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::IMPORT_TMX_LOCALY:
    case ProxygenHandler::COMMAND::IMPORT_MEM: 
    {        
      importMemSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::STATUS_MEM: 
    {        
      statusMemSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::REORGANIZE_MEM:
    {
      reorganizeMemSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::FUZZY: 
    {        
      fuzzyReqSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::CONCORDANCE: 
    {        
      concordanceSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::UPDATE_ENTRY: 
    {        
      updateEntrySumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::DELETE_ENTRY: 
    {        
      deleteEntrySumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::SAVE_ALL_TM_ON_DISK: 
    {        
      saveAllTmsSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::LIST_OF_MEMORIES: 
    {        
      getListOfMemoriesSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::RESOURCE_INFO: 
    {        
      resourcesSumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::CLONE_TM_LOCALY:
    {
      cloneLocalySumTime_ += time;
      break;
    }
    case ProxygenHandler::COMMAND::UNKNOWN_COMMAND:{
      unrecognizedRequestSumTime_ += time;
      break;
    }    
    default:
    {
      otherSumTime_ += time;
      break;
    }
  }
  executionTime_ += time;
  return 0;
}

}//namespace
