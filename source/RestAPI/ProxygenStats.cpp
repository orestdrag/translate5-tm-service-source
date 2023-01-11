
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
      {        
        importMemReqCount_++;
        break;
      }
      case ProxygenHandler::COMMAND::STATUS_MEM: 
      {        
        statusMemReqCount_++;
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
}
