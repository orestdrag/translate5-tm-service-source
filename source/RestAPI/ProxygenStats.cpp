
#include "ProxygenStats.h"
#include "ProxygenHandler.h"
//namespace ProxygenService{


int ProxygenStats::recordRequest(int command) {
  switch(command){
    case COMMAND::CREATE_MEM: 
    {
      createMemReqCount_++;
      break;
    }
    case COMMAND::DELETE_MEM: 
    {
      deleteMemReqCount_++;
      break;
    }
    case COMMAND::EXPORT_MEM_TMX: 
    case COMMAND::EXPORT_MEM_TMX_STREAM:
    case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
    case COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM:
    {
      exportMemReqCount_++;
      break;
    }
    case COMMAND::IMPORT_MEM: 
    case COMMAND::IMPORT_LOCAL_MEM:
    {        
      importMemReqCount_++;
      break;
    }
    case COMMAND::STATUS_MEM: 
    {        
      statusMemReqCount_++;
      break;
    }
    case COMMAND::REORGANIZE_MEM:
    {
      reorganizeMemRequestCount_++;
      break;
    }
    case COMMAND::FUZZY: 
    {        
      fuzzyReqCount_++;
      break;
    }
    case COMMAND::CONCORDANCE: 
    {        
      concordanceReqCount_++;
      break;
    }
    case COMMAND::UPDATE_ENTRY: 
    {        
      updateEntryReqCount_++;
      break;
    }
    case COMMAND::DELETE_ENTRY: 
    {        
      deleteEntryReqCount_++;
      break;
    }
    case COMMAND::DELETE_ENTRIES_REORGANIZE: 
    {        
      deleteEntryReorganizeReqCount_ ++;
      break;
    }
    case COMMAND::SAVE_ALL_TM_ON_DISK: 
    {        
      saveAllTmsReqCount_++;
      break;
    }
    case COMMAND::LIST_OF_MEMORIES: 
    {        
      getListOfMemoriesReqCount_++;
      break;
    }
    case COMMAND::RESOURCE_INFO: 
    {        
      resourcesReqCount_++;
      break;
    }
    case COMMAND::CLONE_TM_LOCALY:
    {
      cloneLocalyCount_++;
      break;
    }
    case COMMAND::UNKNOWN_COMMAND:{
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
    case COMMAND::CREATE_MEM: 
    {
      createMemSumTime_ += time;
      break;
    }
    case COMMAND::DELETE_MEM: 
    {
      deleteMemSumTime_+= time;
      break;
    }
    case COMMAND::EXPORT_MEM_TMX: 
    case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
    case COMMAND::EXPORT_MEM_TMX_STREAM:
    case COMMAND::EXPORT_MEM_INTERNAL_FORMAT_STREAM:
    {
      exportMemSumTime_ += time;
      break;
    }
    case COMMAND::IMPORT_LOCAL_MEM:
    case COMMAND::IMPORT_MEM: 
    {        
      importMemSumTime_ += time;
      break;
    }
    case COMMAND::STATUS_MEM: 
    {        
      statusMemSumTime_ += time;
      break;
    }
    case COMMAND::REORGANIZE_MEM:
    {
      reorganizeMemSumTime_ += time;
      break;
    }
    case COMMAND::FUZZY: 
    {        
      fuzzyReqSumTime_ += time;
      break;
    }
    case COMMAND::CONCORDANCE: 
    {        
      concordanceSumTime_ += time;
      break;
    }
    case COMMAND::UPDATE_ENTRY: 
    {        
      updateEntrySumTime_ += time;
      break;
    }
    case COMMAND::DELETE_ENTRY: 
    {        
      deleteEntrySumTime_ += time;
      break;
    }
    case COMMAND::DELETE_ENTRIES_REORGANIZE: 
    {        
      deleteEntryReorganizeSumTime_ += time;
      break;
    }
    case COMMAND::SAVE_ALL_TM_ON_DISK: 
    {        
      saveAllTmsSumTime_ += time;
      break;
    }
    case COMMAND::LIST_OF_MEMORIES: 
    {        
      getListOfMemoriesSumTime_ += time;
      break;
    }
    case COMMAND::RESOURCE_INFO: 
    {        
      resourcesSumTime_ += time;
      break;
    }
    case COMMAND::CLONE_TM_LOCALY:
    {
      cloneLocalySumTime_ += time;
      break;
    }
    case COMMAND::UNKNOWN_COMMAND:{
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

//}//namespace
