
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
      case COMMAND::EXPORT_MEM: 
      case COMMAND::EXPORT_MEM_INTERNAL_FORMAT:
      {
        exportMemReqCount_++;
        break;
      }
      case COMMAND::IMPORT_MEM: 
      {        
        importMemReqCount_++;
        break;
      }
      case COMMAND::IMPORT_LOCAL_MEM:
      {
        importLocalReqCount_++;
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
    return ++reqCount_;
  }
//}
