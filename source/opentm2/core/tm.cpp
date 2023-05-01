#include "tm.h"
#include "LogWrapper.h"
#include "Property.h"
#include "EQFMSG.H"
#include "EQF.H"
#include "FilesystemHelper.h"
#include "tm.h"

std::shared_ptr<EqfMemory>  TMManager::findOpenedMemory(const std::string& memName){
    int index = findMemoryInList(memName);
    if(index != -1)
        return EqfMemoryPlugin::GetInstance()->m_MemInfoVector[index];
    return nullptr;
}

/*! \brief find a memory in our list of active memories
  \param pszMemory name of the memory
  \returns index in the memory table if successful, -1 if memory is not contained in the list
*/
int TMManager::findMemoryInList( const std::string& memName )
{
  if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size()==0){
    T5LOG( T5WARNING) <<"findMemoryInList:: EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size == 0";
  }
  // 
  for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
  {
    if ( strcasecmp( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName, memName.c_str() ) == 0 )
    {
      return( i );
    } /* endif */
  } /* endfor */
  return( -1 );
}

/*! \brief Checks if there is opened memory in import process
  \returns index if import process for any memory is going on, -1 if no
  */
int TMManager::GetMemImportInProcess(){
  for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
  {
    if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->eImportStatus == IMPORT_RUNNING_STATUS)
    {
      T5LOG( T5INFO) << ":: memory in import process, name = " << EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName ;
      return i;
    }
  } /* endfor */
  return -1;
}

/*! \brief find a free slot in our list of active memories, add a new entry if none found
  \returns index of the next free slot in the memory table or -1 if there is no free slot and the max number of entries has been reached
*/
int TMManager::getFreeSlot(size_t memoryRequested)
{
  size_t UsedMemory = 0;
  int AllowedMemoryMB;
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, AllowedMemoryMB);
  size_t AllowedMemory = (size_t)AllowedMemoryMB * 1000000;

  #ifdef CALCULATE_ONLY_MEM_FILES
  std::string path; 
  char memFolder[260];
  Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);

  for(int i = 0; i < EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ;i++){
    path = memFolder;
    path += EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName;
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
  }
  #else
  UsedMemory = calculateOccupiedRAM() + memoryRequested;
  #endif

  T5LOG( T5DEBUG) << ":: " << UsedMemory << " bytes is used from " << AllowedMemory << " allowed";

  // add a new entry when the maximum list size has not been reached yet
  //if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() < OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES )
  if( UsedMemory < AllowedMemory)
  {
    // first look for a free slot in the existing list
    for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
    {
      if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName[0] == 0 )
      {
        return( i );
      } /* endif */
    } /* endfor */
    
    EqfMemoryPlugin::GetInstance()->m_MemInfoVector.push_back( std::make_shared<EqfMemory>( EqfMemory()) );
    return( EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() - 1 );
  } /* endif */  

  return( -1 );
}


std::shared_ptr<EqfMemory>  TMManager::getFreeSlotPointer(size_t memoryRequested)
{
  size_t UsedMemory = 0;
  int AllowedMemoryMB;
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, AllowedMemoryMB);
  size_t AllowedMemory = (size_t)AllowedMemoryMB * 1000000;

  #ifdef CALCULATE_ONLY_MEM_FILES
  std::string path; 
  char memFolder[260];
  Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);

  for(int i = 0; i < EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ;i++){
    path = memFolder;
    path += EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName;
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
  }
  #else
  UsedMemory = calculateOccupiedRAM() + memoryRequested;
  #endif

  T5LOG( T5DEBUG) << ":: " << UsedMemory << " bytes is used from " << AllowedMemory << " allowed";

  // add a new entry when the maximum list size has not been reached yet
  //if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() < OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES )
  if( UsedMemory < AllowedMemory)
  {
    // first look for a free slot in the existing list
    for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
    {
      if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName[0] == 0 )
      {
        return( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i] );
      } /* endif */
    } /* endfor */
    EqfMemoryPlugin::GetInstance()->m_MemInfoVector.push_back( std::make_shared<EqfMemory> (EqfMemory()) );
    return( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() - 1] );
  } /* endif */  

  return( nullptr );
}


size_t TMManager::calculateOccupiedRAM(){
  char memFolder[260];
  size_t UsedMemory = 0;
  #ifdef CALCULATE_ONLY_MEM_FILES
  Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);
  std::string path;
  for(int i = 0; i < EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ;i++){
    if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName != 0){
      path = memFolder;
      path += EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName;
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
    }
  }
  #else
  UsedMemory += FilesystemHelper::GetTotalFilebuffersSize();
  UsedMemory += MEMORY_RESERVED_FOR_SERVICE;
  #endif

  T5LOG( T5INFO) << ":: calculated occupied ram = " << UsedMemory/1000000 << " MB";
  return UsedMemory;
}



size_t TMManager::CalculateOccupiedRAM(){
  char memFolder[260];
  size_t UsedMemory = 0;
  #ifdef CALCULATE_ONLY_MEM_FILES
  Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);
  std::string path;
  for(int i = 0; i < EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ;i++){
    if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName != 0){
      path = memFolder;
      path += EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName;
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
    }
  }
  #else
  for(const auto& mem: tms){
    UsedMemory += mem.second.get()->GetRAMSize();
  }
  //UsedMemory += FilesystemHelper::GetTotalFilebuffersSize();
  UsedMemory += MEMORY_RESERVED_FOR_SERVICE;
  #endif

  T5LOG( T5INFO) << ":: calculated occupied ram = " << UsedMemory/1000000 << " MB";
  return UsedMemory;
}

/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
size_t TMManager::cleanupMemoryList(size_t memoryRequested)
{  
  int AllowedMBMemory = 500;
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, AllowedMBMemory);
  size_t AllowedMemory = AllowedMBMemory * 1000000;    
  size_t memoryNeed = memoryRequested + calculateOccupiedRAM();

  if(memoryNeed < AllowedMemory){
    return( AllowedMemory - memoryNeed );
  }

  time_t curTime;
  time( &curTime );
  std::multimap <time_t, int>  openedMemoriesSortedByLastAccess;
  for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ; i++ ){
    if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName[0] != 0 )
    {
      openedMemoriesSortedByLastAccess.insert({EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->tLastAccessTime, i});
    }
  }

  for(auto it = openedMemoriesSortedByLastAccess.begin(); memoryNeed >= AllowedMemory && it != openedMemoriesSortedByLastAccess.end(); it++){
    T5LOG( T5INFO) << ":: removing memory  \'"<< EqfMemoryPlugin::GetInstance()->m_MemInfoVector[it->second].get()->szName << "\' that wasns\'t used for " << (curTime - EqfMemoryPlugin::GetInstance()->m_MemInfoVector[it->second].get()->tLastAccessTime) <<  " seconds" ;
    removeFromMemoryList(it->second);
    memoryNeed = memoryRequested + calculateOccupiedRAM();
  }

  return( AllowedMemory - memoryNeed );
}


/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
size_t TMManager::CleanupMemoryList(size_t memoryRequested)
{  
  int AllowedMBMemory = 500;
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, AllowedMBMemory);
  size_t AllowedMemory = AllowedMBMemory * 1000000;    
  size_t memoryNeed = memoryRequested + CalculateOccupiedRAM();

  if(memoryNeed < AllowedMemory){
    return( AllowedMemory - memoryNeed );
  }

  time_t curTime;
  time( &curTime );
  std::multimap <time_t, std::string>  openedMemoriesSortedByLastAccess;
  for(const auto& tm: tms){
  //for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ; i++ ){
  //  if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName[0] != 0 )
    {
      openedMemoriesSortedByLastAccess.insert({tm.second.get()->tLastAccessTime, tm.first});
    }
  }

  for(auto it = openedMemoriesSortedByLastAccess.begin(); memoryNeed >= AllowedMemory && it != openedMemoriesSortedByLastAccess.end(); it++){
    //T5LOG( T5INFO) << ":: removing memory  \'"<< EqfMemoryPlugin::GetInstance()->m_MemInfoVector[it->second].get()->szName << "\' that wasns\'t used for " << (curTime - EqfMemoryPlugin::GetInstance()->m_MemInfoVector[it->second].get()->tLastAccessTime) <<  " seconds" ;
    //RemoveFromMemoryList(it->second);
    tms[it->second].get()->UnloadFromRAM();
    memoryNeed = memoryRequested + CalculateOccupiedRAM();
  }

  return AllowedMemory > memoryNeed ? AllowedMemory - memoryNeed : 0;
}

/*! \brief get the handle for a memory, if the memory is not opened yet it will be openend
\param pszMemory name of the memory
\param plHandle buffer for the memory handle
\param pszError buffer for an error message text in case of failures
\param iErrorBufSize size of the error text buffer in number of characters
\param piReturnCode pointer to a buffer for the OpenTM2 error code
\returns OK if successful or an HTTP error code in case of failures
*/
int TMManager::getMemoryHandle( const std::string& pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize, int *piErrorCode )
{
  *piErrorCode = 0;
  std::shared_ptr<EqfMemory>  pMem = findOpenedMemory( pszMemory );
  if ( pMem != nullptr )
  {
    switch ( pMem->eStatus )
    {
      case OPEN_STATUS:
        *plHandle = pMem->lHandle;
        time( &( pMem->tLastAccessTime ) );
        return( OK );
        break;
      case IMPORT_RUNNING_STATUS:
        wcsncpy( pszError, L"TM is busy, import is running", iErrorBufSize );
        *piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( PROCESSING );
        break;
      case IMPORT_FAILED_STATUS:
        wcsncpy( pszError, L"TM import failed, memory may be not usable", iErrorBufSize );
        *piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( INTERNAL_SERVER_ERROR );
        break;
      case AVAILABLE_STATUS:
        {
          // open the memory
          LONG lHandle = 0;
          *piErrorCode = EqfOpenMem( this->hSession, (char*) pszMemory.c_str(), &lHandle, 0 );
          if ( *piErrorCode != 0 )
          {
            unsigned short usRC = 0;
            EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
            return( INTERNAL_SERVER_ERROR );
          } /* endif */
          pMem->lHandle = lHandle;
          strcpy( pMem->szName, pszMemory.c_str() );
          time( &( pMem->tLastAccessTime ) );
          pMem->eStatus = OPEN_STATUS;
          //if ( pMem->pszError != NULL ) free( pMem->pszError );
          //pMem->pszError = NULL;
          *plHandle = lHandle;
          return( OK );
        }
        break;
      default:
        wcsncpy( pszError, L"TM is in undefined state", iErrorBufSize );
        //*piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( INTERNAL_SERVER_ERROR );
        break;
    }
  } /* endif */


  size_t requiredMemory = 0;
  {
    char memFolder[260];
    Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);
    std::string path;

    path = memFolder;
    path += pszMemory;
    if(FilesystemHelper::FileExists(std::string(path + ".TMD"))){
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMI"));
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMD"));
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".MEM"));
    }
  } 

  // cleanup the memory list (close memories not used for a longer time)
  size_t memLeftAfterOpening = cleanupMemoryList(requiredMemory);
  if(VLOG_IS_ON(1)){
    T5LOG( T5INFO) << ":: memory: " << pszMemory << "; required RAM:" << requiredMemory << "; allowed RAM left after opening mem: " << memLeftAfterOpening;
  }
  // find a free slot in the memory list
  pMem = getFreeSlotPointer(requiredMemory);

  // handle "too many open memories" condition
  if ( pMem == nullptr )
  {
    wcscpy( pszError, L"Error: too many open translation memory databases" );
    return( INTERNAL_SERVER_ERROR );
  } /* endif */

  // open the memory
  LONG lHandle = 0;
  *piErrorCode = EqfOpenMem( this->hSession, (char*) pszMemory.c_str(), &lHandle, 0 );
  if ( *piErrorCode != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
    return( INTERNAL_SERVER_ERROR );
  } /* endif */

  // add opened memory to the memory list
  pMem->lHandle = lHandle;
  strcpy( pMem->szName, pszMemory.c_str() );
  time( &(pMem->tLastAccessTime) );
  pMem->eStatus = OPEN_STATUS;
  pMem->eImportStatus = OPEN_STATUS;
  pMem->pszError = NULL;
  *plHandle = lHandle;

  return( OK );
}


// update memory status
void TMManager::importDone(std::shared_ptr<EqfMemory> mem, int iRC, char *pszError )
{
  if ( iRC == 0 )
  {
    mem->eImportStatus = OPEN_STATUS;
    mem->TmBtree.fb.Flush();
    mem->InBtree.fb.Flush();
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::importDone:: success, memName = " << mem->szName;
  }
  else
  {
    mem->eImportStatus = IMPORT_FAILED_STATUS;
    mem->pszError = new char[strlen( pszError ) + 1];
    strcpy( mem->pszError, pszError );
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::importDone:: memName = " << mem->szName <<", import failed: " << pszError << " import details = " << mem->importDetails->toString() ;
  }
}



/*! \brief Close all open memories
\returns http return code0 if successful or an error code in case of failures
*/
int TMManager::closeAll()
{
  for ( int i = 0; i < ( int )EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
  {
    if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].get()->szName[0] != 0 )
    {
      removeFromMemoryList( i );
    }
  } /* endfor */
  return( 0 );
}




/*! \brief close a memory and remove it from the open list
    \param iIndex index of memory in the open list
  \returns 0 
*/
int TMManager::removeFromMemoryList( int iIndex )
{
  std::shared_ptr<EqfMemory>  pMem = EqfMemoryPlugin::GetInstance()->m_MemInfoVector[iIndex];
  LONG lHandle = pMem->lHandle;
  int i=0;
  while(pMem.get()->eStatus == IMPORT_RUNNING_STATUS || pMem.get()->eImportStatus == IMPORT_RUNNING_STATUS ){
    sleep(1);
    if(i++ % 10 == 0){
      T5LOG( T5WARNING) << ":: waiting for closing memory with name = \'" << pMem.get()->szName << "\', for " << i << " seconds";
    }
  }
  // remove the memory from the list
  pMem.get()->lHandle = 0;
  pMem.get()->tLastAccessTime = 0;
  pMem.get()->szName[0] = 0;
  if ( pMem->pszError ) delete[]  pMem->pszError ;
  if( pMem.get()->importDetails != nullptr){
    delete pMem.get()->importDetails;
    pMem.get()->importDetails = nullptr;
  }
  pMem.get()->pszError = NULL;

  // close the memory
  EqfCloseMem( this->hSession, lHandle, 0 );

  return( 0 );
} 




/*! \brief close a memory and remove it from the open list
    \param iIndex index of memory in the open list
  \returns 0 
*/
int TMManager::removeFromMemoryList( std::shared_ptr<EqfMemory>  pMem )
{
  LONG lHandle = pMem->lHandle;
  int i=0;
  while(pMem->eStatus == IMPORT_RUNNING_STATUS || pMem->eImportStatus == IMPORT_RUNNING_STATUS ){
    sleep(1);
    if(i++ % 10 == 0){
      T5LOG( T5WARNING) << ":: waiting for closing memory with name = \'" << pMem->szName << "\', for " << i << " seconds";
    }
  }
  // remove the memory from the list
  pMem->lHandle = 0;
  pMem->tLastAccessTime = 0;
  pMem->szName[0] = 0;
  if ( pMem->pszError ) delete[]  pMem->pszError ;
  if( pMem->importDetails != nullptr){
    delete pMem->importDetails;
    pMem->importDetails = nullptr;
  }
  pMem->pszError = NULL;

  // close the memory
  EqfCloseMem( this->hSession, lHandle, 0 );

  return( 0 );
} 



/*! \file

 \brief Memory factory class wrapper 

 This class provides static methods for the creation of EqfMemory objects and
 access to standard memory functions

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_MORPH            // morphologic functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions



//#define LOGGING

#include "../pluginmanager/PluginManager.h"
#include "tm.h"
#include "../utilities/OSWrapper.h"
#include "tm.h"
//#include "OptionsDialog.H"
#include "LogWrapper.h"
#include "FilesystemHelper.h"
#include "Property.h"
#include "ZipHelper.h"
#include "LanguageFactory.H"

#include "vector"
#include <string>
#include <climits>

// default memory plugins
#define DEFAULTMEMORYPLUGIN "EqfMemoryPlugin"

//P403634
#define DEFAULTSHAREDMEMORYPLUGIN "EqfSharedMemoryPlugin"
//#define DEFAULTSHAREDMEMORYPLUGIN "EqfSharedMemPlugin"


/*! \brief This static method returns a pointer to the TMManager object.
	The first call of the method creates the TMManager instance.
*/

TMManager* TMManager::GetInstance(){
    static TMManager tmm;
    return &tmm;
}

/* \brief Open an existing memory
   \param pszPluginName Name of the plugin or NULL
   \param pszMemoryName Name of the memory
   \param usOpenFlags Open mode flags
   \param pMemory pointer to the opened memory object
   \paran piErrorCode optional pointer to a buffer for error codes
	 \returns pointer to memory object or NULL in case of errors
*/

EqfMemory *TMManager::openMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  unsigned short usOpenFlags,
  int *piErrorCode
)
{
  this->strLastError = "";
  this->iLastError = 0;
  T5LOG( T5INFO) << "TMManager::openMemory, pszMemoryName = " << pszMemoryName;

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  EqfMemory *pMemory = EqfMemoryPlugin::GetInstance()->openMemory((char *)strMemoryName.c_str() , FALSE, NULLHANDLE, usOpenFlags );
  if ( pMemory == NULL ){
        *piErrorCode = this->iLastError = EqfMemoryPlugin::GetInstance()->getLastError( this->strLastError );
        T5LOG(T5ERROR) << "TMManager::openMemory, Open of local memory " << strMemoryName << " failed, the return code is " << this->iLastError;
  }    

  return( pMemory );
}

/* \brief Get information from an existing memory
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory or memory object name (pluginname + colon + memory name)
   \param pInfo pointer to caller EqfMemory structure
   \returns 0 when successful or error code
*/
int TMManager::getMemoryInfo
(
  char *pszPluginName,
  char *pszMemoryName,
  std::shared_ptr<EqfMemory>  pInfo
)
{
  int iRC = 0;
  this->strLastError = "";
  this->iLastError = 0;
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
    T5LOG( T5INFO) <<" TMManager::getMemoryInfo:: Get info for memory " << pszMemoryName;
  }

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  iRC = EqfMemoryPlugin::GetInstance()->getMemoryInfo( (char *)strMemoryName.c_str(), pInfo );
  if ( iRC != 0 ) {
    iRC = this->iLastError = EqfMemoryPlugin::GetInstance()->getLastError( this->strLastError );
  }
  
  if ( iRC != 0 )
  {
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
      T5LOG(T5ERROR) << " TMManager::getMemoryInfo  Could not retrieve information for local memory "
          << strMemoryName << " using plugin " <<  
        EqfMemoryPlugin::GetInstance()->getName() << ", the return code is " << this->iLastError;
    }
  }
  else
  {
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
      T5LOG( T5INFO) <<" TMManager::getMemoryInfo  info retrieval was successful" ;
    }
  } /* end */     

  return( iRC );
}


/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
   \param pszOwner owner of the newly created memory
   \param bInvisible don't display memory in memory loist window when true, 
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
EqfMemory *TMManager::createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  char chDrive,
  char *pszOwner,
  bool bInvisible,
  int *piErrorCode
)
{
  EqfMemory *pMemory = NULL;
  this->strLastError = "";
  this->iLastError = 0;
  T5LOG(T5DEBUG) << "Create memory " << pszMemoryName ;

  if ( piErrorCode != NULL ) *piErrorCode = 0;   

  // use the plugin to create the memory
  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );  
  pMemory = EqfMemoryPlugin::GetInstance()->createMemory( (char *)strMemoryName.c_str(), pszSourceLanguage, pszDescription );

  if ( pMemory == NULL)
  {
    this->iLastError = EqfMemoryPlugin::GetInstance()->getLastError( this->strLastError );
    T5LOG(T5DEBUG) << "   Create failed, with message \""<< this->strLastError << "\"";
    if ( piErrorCode != NULL ) *piErrorCode = this->iLastError;
  }
  else if ( !bInvisible )
  {
    // send created notifcation
    PSZ pszObjName = NULL;
    UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
    strcpy( pszObjName, EqfMemoryPlugin::GetInstance()->getName() );
    strcat( pszObjName, ":" );
    strcat( pszObjName, pszMemoryName );
    EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED, MP1FROMSHORT(  clsMEMORYDB  ), MP2FROMP( pszObjName ));
    if ( pszObjName != NULL ) UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
    T5LOG(T5DEBUG) << "   Create successful" ;
  } /* endif */     

  return( pMemory );
}

/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param pszOwner owner of the newly created memory
   \param bInvisible don't display memory in memory loist window when true, 
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
EqfMemory *TMManager::createMemory
(
  const char *pszPluginName,
  const char *pszMemoryName,
  const char *pszDescription,
  const char *pszSourceLanguage,
  const char *pszOwner,
  bool bInvisible,
  int *piErrorCode
)
{  
  T5LOG( T5INFO) << "::pszMemoryName = " << pszMemoryName <<  ", pszSourceLanguage = " << pszSourceLanguage <<
      ", pszDescription = " << pszDescription ;
  this->strLastError = "";
  this->iLastError = 0;

  if ( piErrorCode != nullptr ) 
      *piErrorCode = 0;

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );
  EqfMemory * pMemory = EqfMemoryPlugin::GetInstance()->createMemory( (char *)strMemoryName.c_str(), pszSourceLanguage, pszDescription );
  if ( pMemory == NULL ){
     this->iLastError = EqfMemoryPlugin::GetInstance()->getLastError( this->strLastError );
     T5LOG(T5ERROR) << "TMManager::createMemory()::EqfMemoryPlugin::GetInstance()->getType() == OtmPlugin::eTranslationMemoryType->::pMemory == NULL, strLastError = " <<  strLastError;
     
     if ( piErrorCode != NULL ) 
        *piErrorCode = this->iLastError;
  }
  else{
    T5LOG( T5INFO) << "::Create successful ";
  }
  return( pMemory );
}

/* \brief List memories from all memory plugins
   \param pfnCallBack callback function to be called for each memory
	 \param pvData caller's data pointetr, is passed to callback function
	 \param fWithDetails TRUE = supply memory details, when this flag is set, 
   the pInfo parameter of the callback function is set otherwise it is NULL
 	 \returns number of provided memories
*/
int TMManager::listMemories
(
	EqfMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
)
{
  int iMemories = 0;
  iMemories += EqfMemoryPlugin::GetInstance()->listMemories( pfnCallBack, pvData, fWithDetails );     
  return( iMemories );
}

/* \brief Closed a previously opened memory
   \param pMemory pointer to memory object beign closed
	 \returns 0 when successful or error code
*/
int TMManager::closeMemory
(
  EqfMemory *pMemory
)
{
  int iRC = 0;

  if ( pMemory == NULL  ){
    T5LOG(T5ERROR) <<"TMManager::closeMemory, pMemory is NULL";
    return( -1 );
  }
  
  // build memory object name
  char* pszObjName = NULL;
  UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
  strcpy( pszObjName, EqfMemoryPlugin::GetInstance()->getName() );
  strcat( pszObjName, ":" );
  pMemory->getName( pszObjName + strlen(pszObjName), MAX_LONGFILESPEC );

  // close the memory
  iRC = EqfMemoryPlugin::GetInstance()->closeMemory( pMemory );

  // send a properties changed msg to memory handler
  EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pszObjName ));

  if ( pszObjName != NULL ) {
      UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
  }
 
  return( iRC );
}


/*! \brief Physically delete a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int TMManager::deleteMemory(
  char *pszPluginName,
  char *pszMemoryName
)
{
  if(pszPluginName == NULL){
    if(VLOG_IS_ON(1))
      T5LOG( T5INFO) <<"TMManager::deleteMemory::pszPluginName = NULL";
  }else if(pszMemoryName == NULL){
    T5LOG(T5ERROR) <<"TMManager::deleteMemory::pszPluginName = " << pszPluginName <<"; pszMemoryName = NULL";
  }else{
    T5LOG( T5INFO) <<"TMManager::deleteMemory::pszPluginName = " << pszPluginName <<"; pszMemoryName = "<< pszMemoryName;
  }
  std::string strError;
  int iRC = deleteMemory(pszPluginName,pszMemoryName, strError);
  if(iRC != EqfMemoryPlugin::eSuccess)
    T5LOG(T5ERROR) <<"deleteMemory::strError = " << strError;
  return iRC;
}

/*! \brief Physically delete a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
  \param strError return error message with it
	returns 0 if successful or error return code
*/
int TMManager::deleteMemory(
  char *pszPluginName,
  char *pszMemoryName,
  std::string &strError
)
{
  int iRC = EqfMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszMemoryName );

  if ( plugin != NULL )
  {
    std::string strMemoryName;
    this->getMemoryName( pszMemoryName, strMemoryName );

    // use the given plugin to delete local memory
    iRC = EqfMemoryPlugin::GetInstance()->deleteMemory( (char *)strMemoryName.c_str());
    if ( iRC != 0 ) EqfMemoryPlugin::GetInstance()->getLastError(strError);

    // broadcast deleted memory name
    if ( iRC == EqfMemoryPlugin::eSuccess )
    {
      strcpy( this->szMemObjName, plugin->getName() );
      strcat( this->szMemObjName,  ":" ); 
		  strcat( this->szMemObjName, pszMemoryName );
	  }
  }
  else
  {
    strError = "Memory Not Found";
    iRC = EqfMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}


/*! \brief Check if memory exists
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being cleared or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
BOOL TMManager::exists(
  const char *pszPluginName,
  const char *pszMemoryName
)
{
  BOOL fExists = FALSE;

  std::shared_ptr<EqfMemory> pInfo = std::make_shared<EqfMemory>(EqfMemory());
  if (pszMemoryName && EqfMemoryPlugin::GetInstance()->getMemoryInfo( pszMemoryName, pInfo ) == 0 )
  {
    fExists = TRUE;
  } /* endif */         
  //delete( pInfo );
  return( fExists );
}

/*! \brief Show error message for the last error
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory causing the problem
  memory object name (pluginname + colon + memoryname)
  \param pMemory pointer to existing memory object or NULL if not available
  \param hwndErrMsg handle of parent window message box
*/
void TMManager::showLastError(
  char *pszPluginName,
  char *pszMemoryName,
  EqfMemory *pMemory,
  HWND hwndErrMsg
)
{
  pszPluginName; pszMemoryName;

  if ( pMemory != NULL )
  {
    // retrieve last error from memory plugin
    this->iLastError = pMemory->getLastError( this->strLastError );
  } /* endif */     

 // show error message
  char* pszParm = (char*)this->strLastError.c_str();
  T5LOG(T5ERROR) <<"ERROR in TMManager::showLastError:: " << pszParm <<  "; iLastError = " << this->iLastError;
}

std::string& TMManager::getLastError( EqfMemory *pMemory, int& iLastError, std::string& strError)
{
    if ( pMemory != NULL )
        this->iLastError = pMemory->getLastError( this->strLastError );

    iLastError = this->iLastError;
    strError = this->strLastError;
    return strError;
}

/*! \brief Get the sort order key for a memory match

  \param Proposal reference to a proposal for which the sort key is evaluated
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal &Proposal )
{
  return( getProposalSortKey( Proposal, -1, 0, FALSE ) ); 
}


/*! \brief Get the sort order key for a memory match

 the current order is: Exact-Exact -> Exact -> Global memory(Exact) -> Replace -> Fuzzy -> Machine
 We use the folloging sort key values
 - Exact-ContextSameDoc       = 1400
 - Exact-Context              = 1300
 - Exact-Exact                = 1200
 - Exact-Context(Context<100) =  900-999
 - Exact-SameDoc              =  801 (800 when flag LATESTPROPOSAL_FIRST has been specified)
 - Exact                      =  800
 - Global memory (Exact)      =  700
 - Global memory Star(Exact)  =  600
 - Fuzzy                      =  301-399
 - Machine                    =  200 (400 at the end of the table)
 - empty entry                =  0

  Special handling for MT display factor (if given):
  - 0%  Machine =  200
  - 100%  Machine =  400
  - 1%-99% Machine =  3xx (xx = MT display factor)

  Special mode at the end of the match table:

  When the end of the table is reached MT proposals get a key higher than fuzzy ones to keep at least one MT match in the table 
  
  \param Proposal reference to a proposal for which the sort key is evaluated
  \param iMTDisplayFactor the machine translation display factor, -1 to ignore the factor
  \param usContextRanking the context ranking for the proposal
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  \param lOptions options for the sort key generation
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal &Proposal, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions )
{
  // when no context value is given, use context ranking of proposal
  if ( usContextRanking == 0 )
  {
    usContextRanking = (USHORT)Proposal.getContextRanking();
  } /* endif */
  return( getProposalSortKey( Proposal.getMatchType(), Proposal.getType(), Proposal.getFuzziness(), iMTDisplayFactor, usContextRanking, fEndOfTable, lOptions  ) ); 
}

 /*! \brief Get the sort order key for a memory match
  \param MatchType match type of the match
  \param ProposalType type of the proposal
  \param iFuzziness fuzziness of the proposal
  \param iMTDisplayFactor the machine translation display factor, -1 to use the system MT display factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  \param lOptions options for the sort key generation
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal::eMatchType MatchType, OtmProposal::eProposalType ProposalType, 
    int iFuzziness, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions  )
{

  // dummy (=empty) entries
  if ( (MatchType == OtmProposal::emtUndefined) || (iFuzziness == 0) )
  {
    return( 0 );  
  } /* endif */  

  // use system MT display factor if none has been specified
  if ( iMTDisplayFactor == -1 )
  {
    iMTDisplayFactor = UtlQueryULong( QL_MTDISPLAYFACTOR );
  }

  // normal matches
  if ( ProposalType == OtmProposal::eptManual )
  {
    // exact matches with correct context have the highest priority
    if ( (usContextRanking == 100) && 
         ( (MatchType == OtmProposal::emtExact) || 
           (MatchType == OtmProposal::emtExactSameDoc) ||
           (MatchType == OtmProposal::emtExactExact) ) )
    {
      if ( (MatchType == OtmProposal::emtExactSameDoc) ||  (MatchType == OtmProposal::emtExactExact) )
      {
        return( 1400 ); // context match from same document
      }
      else
      {
        return( 1300 ); // context match from a different document
      }
    }
    else if ( (usContextRanking > 0) && (usContextRanking < 100) )
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactSameDoc )
      {
        return( 900 + usContextRanking  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 900 + usContextRanking -1  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    }
    else
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactSameDoc )
      {
        if ( lOptions & LATESTPROPOSAL_FIRST )
        {
          // treat proposal from same document as normal exact proposal to ensure that the exact proposals are sorted on their timestamp correctly
          return( 800 );  
        }
        else
        {
          return( 801 );  
        } /* endif */
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 800 );  
      } /* endif */  
      // fuzzy and replace matches
      if ( MatchType == OtmProposal::emtReplace )
      {
        return( 400 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    } /* endif */       
  } /* endif */  

  // machine matches
  if ( ProposalType == OtmProposal::eptMachine )
  {
    if ( (iMTDisplayFactor <= 0) || (iMTDisplayFactor > 100) ) // invalid (>100)  / not specified (-1) or 0
    {
      if ( fEndOfTable )
      {
        return( 400 );  
      }
      else
      {
        return( 200 );  
      } /* endif */         
    } 
    else if ( iMTDisplayFactor == 100 )
    {
      return( 400 );  
    }
    else 
    {
      return( 300 + iMTDisplayFactor );  
    } /* endif */       
  } /* endif */  

  // global memory matches
  if ( ProposalType == OtmProposal::eptGlobalMemory )
  {
    if ( iFuzziness >= 100 )
    {
      return( 700 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // global memory star matches
  if ( ProposalType == OtmProposal::eptGlobalMemoryStar )
  {
    if ( iFuzziness >= 100 )
    {
      return( 600 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // fuzzy and replace matches
  if ( ProposalType == OtmProposal::emtReplace )
  {
    return( 400 );  
  } /* endif */  
  if ( ProposalType == OtmProposal::emtFuzzy )
  {
    return( 300 + iFuzziness );  
  } /* endif */  

  // anything else...
  return( 0 );
} /* end of method TMManager::getProposalSortKey */

/*! \brief Get name of default memory plugin
	\returns pointer to name of default memory plugin
*/
const char * TMManager::getDefaultMemoryPlugin(
)
{
  return( this->szDefaultMemoryPlugin );
}


/*! \brief Get the object name for the memory
  \param pMemory pointer to the memory object
  \param pszObjName pointer to a buffer for the object name
  \param iBufSize size of the object name buffer
  \returns 0 when successful or the error code
*/
int TMManager::getObjectName( EqfMemory *pMemory, char *pszObjName, int iBufSize )
{
  EqfMemoryPlugin *pPlugin;

  if ( pMemory == NULL )
  {
      this->iLastError = ERROR_MEMORYOBJECTISNULL;
      this->strLastError = "Internal error, supplied memory object is null";
      return( this->iLastError );
  } /* endif */     

  pPlugin = (EqfMemoryPlugin *)pMemory->getPlugin();
  if ( pPlugin == NULL )
  {
      this->iLastError = ERROR_PLUGINNOTAVAILABLE;
      this->strLastError = "Internal error: No plugin found for memory object";
      return( this->iLastError );
  } /* endif */     

  const char *pszPluginName = pPlugin->getName();
  std::string strMemName;
  pMemory->getName( strMemName );
  if ( ((int)strlen( pszPluginName ) + (int)strMemName.length() + 2) > iBufSize)
  {
    return( ERROR_BUFFERTOOSMALL );
  } /* endif */     

  strcpy( pszObjName, pszPluginName );
  strcat( pszObjName, ":" );
  strcat( pszObjName, strMemName.c_str() );

  return( 0 );
}

/*! \brief Get the plugin name and the memory name from a memory object name
  \param pszObjName pointer to the memory object name
  \param pszPluginName pointer to the buffer for the plugin name or 
    NULL if no plugin name is requested
  \param iPluginBufSize size of the buffer for the plugin name
  \param pszluginName pointer to the buffer for the plugin name or
    NULL if no memory name is requested
  \param iNameBufSize size of the buffer for the memory name
  \returns 0 when successful or the error code
*/
int TMManager::splitObjName( char *pszObjName, char *pszPluginName, int iPluginBufSize, char *pszMemoryName, int iNameBufSize  )
{
  if ( pszObjName == NULL )
  {
    this->iLastError = ERROR_MEMORYOBJECTISNULL;
    this->strLastError = "Internal error: No memory object name specified";
    return( this->iLastError );
  } /* endif */     

  char *pszNamePos = strchr( pszObjName, ':' );
  if ( pszNamePos == NULL )
  {
    this->iLastError = ERROR_INVALIDOBJNAME;
    this->strLastError = "Internal error: Invalid memory object name";
    return( this->iLastError );
  } /* endif */     

  if ( pszPluginName != NULL )
  {
    int iPluginNameLen = pszNamePos - pszObjName;
    if ( iPluginNameLen >= iPluginBufSize )
    {
      this->iLastError = ERROR_BUFFERTOOSMALL;
      this->strLastError = "Internal error: Buffer too small";
      return( this->iLastError );
    } /* endif */     
    strncpy( pszPluginName, pszObjName, iPluginNameLen );
    pszPluginName[iPluginNameLen] = 0;
  } /* end */     

  if ( pszMemoryName != NULL )
  {
    pszNamePos += 1;
    int iMemoryNameLen = strlen( pszNamePos );
    if ( iMemoryNameLen >= iNameBufSize )
    {
      this->iLastError = ERROR_BUFFERTOOSMALL;
      this->strLastError = "Internal error: Buffer too small";
      return( this->iLastError );
    } /* endif */     
    strcpy( pszMemoryName, pszNamePos );
  } /* end */     
  return( 0 );
}

// helper functions 

/* \brief Get memory plugin with the given pkugin name
   \param pszPlugin plugin-name
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *TMManager::getPlugin
(
  const char *pszPluginName
)
{
  // find plugin using plugin name
  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    for ( std::size_t i = 0; i < pluginList->size(); i++ )
    {
      OtmPlugin *pluginCurrent = (OtmPlugin *)(*pluginList)[i];
      if ( strcmp( pluginCurrent->getName(), pszPluginName ) == 0 )
      {
        return ( pluginCurrent );
      } /* endif */         
    } /* endfor */       
  } /* endif */       

  return( NULL );
}

  /* \brief Find memory plugin for this memory using input
    data or find memory testing all memory plugins
   \param pszPlugin plugin-name or NULL if not available
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *TMManager::findPlugin
(
  const char *pszPluginName,
  const char *pszMemoryName
)
{
  OtmPlugin *plugin = NULL;

  // find plugin using plugin name
  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    plugin = this->getPlugin( pszPluginName );
    if ( plugin != NULL )
    {
      return( plugin );
    }
    else
    {
      this->iLastError = ERROR_PLUGINNOTAVAILABLE;
      this->strLastError = "Error: selected plugin " + std::string( pszPluginName ) + " is not available";
      if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG))  
        T5LOG(T5ERROR) << "TMManager::findPlugin()::"<< this->strLastError;
    } /* endif */       
  } /* endif */     

  // check for memory object names
  if ( (pszMemoryName != NULL) && (*pszMemoryName != '\0') )
  {
    char *pszColon  = strchr( (PSZ)pszMemoryName, ':' );
    if ( pszColon != NULL )
    {
      // split object name into plugin name and memory name
      std::string strPlugin = pszMemoryName;
      strPlugin.erase( pszColon - pszMemoryName );
      plugin = this->getPlugin( strPlugin.c_str() );
      if ( plugin != NULL )
      {
        return( plugin );
      }
      else
      {
        this->iLastError = ERROR_PLUGINNOTAVAILABLE;
        this->strLastError = "Error: selected plugin " + strPlugin + " is not available";
        if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)) T5LOG(T5ERROR) << "TMManager::findPlugin()::"<< this->strLastError;
      } /* endif */       
    }
    else
    { 
     std::shared_ptr<EqfMemory> pInfo = std::make_shared<EqfMemory>(EqfMemory());

      // find plugin by querying memory info from all available plugins
      for ( std::size_t i = 0; i < pluginList->size(); i++ )
      {
        EqfMemoryPlugin *pluginCurrent = (*pluginList)[i];
        if ( pluginCurrent->getMemoryInfo( (PSZ)pszMemoryName, pInfo ) == 0 )
        {
          plugin = (OtmPlugin *)pluginCurrent;
          break;
        } /* endif */         
      } /* endfor */       
      
      //delete( pInfo );

      if ( plugin == NULL )
      {
        this->iLastError = ERROR_MEMORY_NOTFOUND;//ERROR_PLUGINNOTAVAILABLE
        this->strLastError = "Translation Memory "+std::string(pszMemoryName)+" was not found.";//"Error: no memory plugin for memory " + std::string(pszMemoryName) + " found or memory does not exist";
        //if(VLOG_IS_ON(1)){
          T5LOG(T5ERROR) << "TMManager::findPlugin()::"<< this->strLastError;
        //}
      } /* endif */       
    } /* endif */       
  }
  else
  {
    this->iLastError = ERROR_MISSINGPARAMETER;
    this->strLastError = "Error: Missing memory name";
    T5LOG(T5ERROR) << "TMManager::findPlugin()::"<< this->strLastError;
  } /* endif */     

  return( plugin );
}

/* \brief Get the memory name from a potential memory object name
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \param strMemoryName reference to string receiving memory name without any plugin name
   \returns 0
*/
int TMManager::getMemoryName
(
  const char *pszMemoryName,
  std::string &strMemoryName
)
{
 if ( (pszMemoryName == NULL) || (*pszMemoryName == '\0') ){
    T5LOG(T5ERROR) << "TMManager::getMemoryName()::(pszMemoryName == NULL) || (*pszMemoryName == '\0'), pszMemoryName = " << pszMemoryName;
    return( -1 );
 }
 const char *pszColon  = strchr( pszMemoryName, ':' );
 if ( pszColon != NULL )
 {
   strMemoryName = pszColon + 1;
 }
 else
 {
   strMemoryName = pszMemoryName;
 }
 return( 0 );
}

void TMManager::refreshPluginList() 
{
  if ( this->pluginList != NULL ) delete( this->pluginList );

  this->szDefaultMemoryPlugin[0] = '\0';
  
  // access plugin manager
  PluginManager* thePluginManager = PluginManager::getInstance();

  // iterate through all memory plugins and store found plugins in pluginList
  EqfMemoryPlugin * curPlugin = NULL;
  pluginList = new std::vector<EqfMemoryPlugin *>;
  int i = 0;
  do
  {
    i++;
    curPlugin = (EqfMemoryPlugin*) thePluginManager->getPlugin(OtmPlugin::eTranslationMemoryType, i );
    if ( curPlugin != NULL ) 
    {
      pluginList->push_back( curPlugin );
      if ( strcasecmp( curPlugin->getName(), DEFAULTMEMORYPLUGIN ) == 0 )
      {
        strcpy( this->szDefaultMemoryPlugin, curPlugin->getName() );
      }
    }
  }  while ( curPlugin != NULL ); /* end */     

  // if default plugin is not available use first plugin as default
  if ( (this->szDefaultMemoryPlugin[0] == '\0') && (pluginList->size() != 0) )
  {
	  strcpy( this->szDefaultMemoryPlugin, (*pluginList)[0]->getName() );
  }
}

const char* GetFileExtention(std::string file){
  auto lastDot = file.rfind('.');
  if(lastDot>0){
    return &file[lastDot];
  }else{
    return &file[0];
  }
}


/*! \brief Replace a memory with the data from another memory
  This method bevaves like deleting the replace memory and renaming the
  replaceWith memory to the name of the replace memory without the overhead of the
  actual delete and rename operations
  \param pszPluginName name of plugin of the memory
  \param pszReplace name of the memory being replaced
  \param pszReplaceWith name of the memory replacing the pszReplace memory
	returns 0 if successful or error return code
*/
int TMManager::replaceMemory
(
  char *pszPluginName,
  char *pszReplace,
  char *pszReplaceWith
)
{
  int iRC = EqfMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszReplace );

  if ( plugin != NULL )
  {
    // use the given plugin to replace the memory, when not supported try the delete-and-rename approach
    if ( plugin->getType() == EqfMemoryPlugin::eTranslationMemoryType )
    {
      iRC = ((EqfMemoryPlugin *)plugin)->replaceMemory( pszReplace, pszReplaceWith );
      if ( iRC == EqfMemoryPlugin::eNotSupported )
      {
        iRC = ((EqfMemoryPlugin *)plugin)->deleteMemory( pszReplace );
        if ( iRC == 0 ) iRC = ((EqfMemoryPlugin *)plugin)->renameMemory( pszReplaceWith, pszReplace );
      }
      if ( iRC != 0 ) ((EqfMemoryPlugin *)plugin)->getLastError(this->strLastError);
    }

    // broadcast deleted memory name for replaceWith memory
    if ( iRC == EqfMemoryPlugin::eSuccess )
    {
      strcpy( this->szMemObjName, plugin->getName() );
      strcat( this->szMemObjName,  ":" ); 
		  strcat( this->szMemObjName, pszReplaceWith );
      //EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT(clsMEMORYDB), MP2FROMP(this->szMemObjName) );
	  }
  }
  else
  {
    iRC = EqfMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}


/*! \brief process the API call: EqfImportMemInInternalFormat and import a memory using the internal memory files
  \param pszMemory name of the memory being imported
  \param pszMemoryPackage name of a ZIP archive containing the memory files
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIImportMemInInternalFormat
(
  const char* pszMemoryName,
  const char* pszMemoryPackage,
  LONG        lOptions 
)
{
  int iRC = 0;

  if ( (pszMemoryName == NULL) || (*pszMemoryName == EOS) )
  {
    T5LOG(T5ERROR) <<   "::TA_MANDTM::if ( (pszMemoryName == NULL) || (*pszMemoryName == EOS))";
    return( TA_MANDTM );
  } /* endif */

  if ( (pszMemoryPackage == NULL) || (*pszMemoryPackage == EOS) )
  {
    T5LOG(T5ERROR) <<   "::FUNC_MANDINFILE::  if ( (pszMemoryPackage == NULL) || (*pszMemoryPackage == EOS) )";
    return( FUNC_MANDINFILE );
  } /* endif */

  // make temporary directory for the memory files of the package
  char szTempDir[MAX_PATH];
  Properties::GetInstance()->get_value( KEY_OTM_DIR, szTempDir, MAX_PATH);
  std::string memDir = szTempDir;
  memDir  +=  "/MEM/";
  strcat( szTempDir, "/TEMP/" );
  FilesystemHelper::CreateDir( szTempDir );
  strcat( szTempDir, "/MemImp/" );
  FilesystemHelper::CreateDir( szTempDir );
  strcat( szTempDir, pszMemoryName );
  strcat (szTempDir, "/");
  FilesystemHelper::CreateDir( szTempDir );

  // unpzip the package
  ZipHelper::ZipExtract( pszMemoryPackage, szTempDir );
  
  {   
    auto files = FilesystemHelper::GetFilesList(szTempDir);
    for( auto file: files ){
      if(file.size() <= 4){
        continue;
      }
      std::string targetFName = memDir + pszMemoryName + GetFileExtention(file);
      if(FilesystemHelper::FileExists(targetFName.c_str())){
         T5LOG(T5ERROR) << ":: file exists, fName = " << targetFName;
         iRC = -1;
         break;
      }
    }

    if( !iRC ){
      for( auto file:files){
        if(file.size() <= 4){
          continue;
        }
        std::string targetFName = memDir + pszMemoryName + GetFileExtention(file);
        std::string oldFName = szTempDir + file;
        FilesystemHelper::MoveFile(oldFName, targetFName);
      }
      
      std::string memFilePath = memDir + pszMemoryName + EXT_OF_MEM;
      if(FilesystemHelper::FileExists(memFilePath.c_str())){
        T5LOG( T5INFO) << ":: memory added to list, mem name = "<< pszMemoryName;
        EqfMemoryPlugin::GetInstance()->addMemoryToList(pszMemoryName);
      }else{
        T5LOG(T5ERROR) << ":: memory not added to list, mem path = " << memFilePath;
      }
    }
  }

  
  // delete any files left over and remove the directory
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){
    FilesystemHelper::RemoveDirWithFiles( szTempDir );
  }

  return( (USHORT)iRC );
}

/*! \brief process the API call: EqfExportMemInInternalFormat and export a memory to a ZIP package
  \param pszMemory name of the memory being exported
  \param pszMemoryPackage name of a new ZIP archive receiving the memory files
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIExportMemInInternalFormat
(
  const char* pszMemoryName,
  const char* pszMemoryPackage,
  LONG        lOptions 
)
{
  int iRC = 0;  
  PSZ pszMemName = (PSZ) pszMemoryName;
  if ( (pszMemName == NULL) || (*pszMemName == EOS) )
  {
    T5LOG(T5ERROR) <<   "::TA_MANDTM::";
    return( TA_MANDTM );
  } /* endif */

  if ( (pszMemoryPackage == NULL) || (*pszMemoryPackage == EOS) )
  {
    T5LOG(T5ERROR) <<   "::FUNC_MANDINFILE::";
    return( FUNC_MANDINFILE );
  } /* endif */

  // check if memory exists
  if ( !this->exists( NULL, pszMemName ) )
  {
    T5LOG(T5ERROR) <<   "::ERROR_MEMORY_NOTFOUND::", (pszMemName);
    return( ERROR_MEMORY_NOTFOUND );
  } /* endif */

  // get the memory plugin for the memory
  OtmPlugin* pPlugin = findPlugin( NULL, pszMemName  );
  if( pPlugin == NULL )
  {
    this->iLastError = ERROR_MEMORYOBJECTISNULL;
    this->strLastError = "Internal error: No plugin found for memory";
    return( (USHORT)this->iLastError );
  } /* endif */     

  // get the list of memory files
  const int iFileListBufferSize = 8000;
  char *pszFileList = new char[iFileListBufferSize];
  if( pPlugin->getType() ==  OtmPlugin::eTranslationMemoryType )
  {
    iRC = ((EqfMemoryPlugin *)pPlugin)->getMemoryFiles( pszMemName, pszFileList, iFileListBufferSize );
  }
  else
  {
    iRC = ERROR_VERSION_NOT_SUPPORTED;
  }
  if( iRC != 0  )
  {
    this->iLastError = ERROR_BUFFERTOOSMALL;
    this->strLastError = "Internal error: Failed to get list of memory files";
    delete[] pszFileList;
    return( (USHORT)this->iLastError );
  } /* endif */     

  // add the files to the package
  UtlZipFiles( pszFileList, pszMemoryPackage );
  delete[] pszFileList;

  return( (USHORT)iRC );
}

/*! \brief process the API call: EqfOpenMem and open the specified translation memory
  \param pszMemory name of the memory being opened
  \param plHandle buffer to a long value receiving the handle of the opened memory or -1 in case of failures
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIOpenMem
(
  const char*         pszMemoryName, 
  LONG        *plHandle,
  LONG        lOptions 
)
{
  PSZ pszMemName = (PSZ)pszMemoryName;
  if ( (pszMemoryName == NULL) || (*pszMemoryName == EOS) )
  {
    T5LOG(T5ERROR) <<   "::TA_MANDTM";
    return( TA_MANDTM );
  } /* endif */

  if ( plHandle == NULL )
  {
    char* pszParm = "pointer to memory handle";
    T5LOG(T5ERROR) <<  ":: DDE_MANDPARAMISSING::" << pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  int iRC = 0;
  EqfMemory *pMem = this->openMemory( NULL, pszMemName, 0, &iRC );
  if ( pMem == NULL )
  {
    this->showLastError( NULL, pszMemName, NULL, HWND_FUNCIF );
    return( (USHORT)iRC );
  } /* endif */


  // find an empty slot in our handle-to-memory-object table
  LONG lIndex = 0;
  while( (lIndex < (LONG)pHandleToMemoryList->size()) && ((*pHandleToMemoryList)[lIndex] != NULL) ) lIndex++;

  // add new entry to list if no free slot is available
  if ( lIndex >= (LONG)pHandleToMemoryList->size() )
  {
    pHandleToMemoryList->resize( lIndex + 10, NULL );
  } /* endif */

  (*pHandleToMemoryList)[lIndex] = pMem;

  *plHandle = createHandle( lIndex, pMem );

  return( 0 );
}

/*! \brief process the API call: EqfCloseMem and close the translation memory referred by the handle
  \param lHandle handle of a previously opened memory
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APICloseMem
(
  LONG        lHandle,
  LONG        lOptions 
)
{
  EqfMemory *pMem = handleToMemoryObject( lHandle );
  if ( pMem == NULL )
  {
    if(VLOG_IS_ON(1))  T5LOG(T5ERROR) <<  "::FUNC_INVALID_MEMORY_HANDLE:: if ( pMem == NULL ): tried to close TM that was already closed";
    return( FUNC_INVALID_MEMORY_HANDLE );
  } /* endif */

  USHORT usRC = (USHORT)this->closeMemory( pMem );

  return( usRC );
}

/*! \brief process the API call: EqfQueryMem and lookup a segment in the memory
  \param lHandle handle of a previously opened memory
  \param pSearchKey pointer to a MemProposal structure containing the searched criteria
  \param *piNumOfProposals pointer to the number of requested memory proposals, will be changed on return to the number of proposals found
  \param pProposals pointer to a array of MemProposal structures receiving the search results
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIQueryMem
(
  EqfMemory*        pMem,          
  PMEMPROPOSAL pSearchKey, 
  int         *piNumOfProposals,
  PMEMPROPOSAL pProposals, 
  LONG        lOptions     
)
{
  USHORT usRC = 0;
  std::vector<OtmProposal *> vProposals;
  for ( int i = 0; i < *piNumOfProposals; i++ ) vProposals.push_back( new(OtmProposal) );

  if ( pSearchKey == NULL )
  {
    char* pszParm = "pointer to search key";
    T5LOG(T5ERROR) <<  ":: DDE_MANDPARAMISSING::" << pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  if ( pProposals == NULL )
  {
    char* pszParm = "pointer to proposal array";
    T5LOG(T5ERROR) <<  ":: DDE_MANDPARAMISSING::" << pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  //EqfMemory *pMem = handleToMemoryObject( lHandle );

  if ( pMem == NULL )
  {
    T5LOG(T5ERROR) <<   "::FUNC_INVALID_MEMORY_HANDLE";
    return( FUNC_INVALID_MEMORY_HANDLE );
  } /* endif */

  OtmProposal SearchKey;
  copyMemProposalToOtmProposal( pSearchKey, &SearchKey );
  usRC = (USHORT)pMem->searchProposal( SearchKey, vProposals, lOptions );
  *piNumOfProposals = OtmProposal::getNumOfProposals( vProposals );

  for( int i = 0; i < *piNumOfProposals; i++ ) copyOtmProposalToMemProposal( vProposals[i], pProposals + i );

  for ( size_t i=0; i < vProposals.size(); i++ ) delete( vProposals[i] );

  return( usRC );
}

wchar_t* wcsupr(wchar_t *str)
{       
    int len = wcslen(str);
    for(int i=0; i<len; i++) {
        str[i] = toupper(str[i]);
    }
    return str;
}

ULONG GetTickCount(){
  struct timespec t; 
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_nsec;
}

/*! \brief process the API call: EqfSearchMem and search the given text string in the memory
  \param lHandle handle of a previously opened memory
  \param pszSearchString pointer to the search string (in UTF-16 encoding)
  \param pszStartPosition pointer to a buffer (min size = 20 charachters) containing the start position, on completion this buffer is filled with the next search position
  \param pvProposal pointer to an OtmProposal object receiving the next matching segment
  \param lSearchTime max time in milliseconds to search for a matching proposal, 0 for no search time restriction
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APISearchMem
(
  LONG        lHandle,                 
  wchar_t  *pszSearchString,
  const char* pszSrcLang, 
  const char* pszTrgLang,
  const char*         pszStartPosition,
  PMEMPROPOSAL pProposal,
  LONG        lSearchTime,
  LONG        lOptions
)
{
  USHORT usRC = 0;
  int iRC = 0;                         // code returned from memory object methods
  BOOL fFound = FALSE;                 // found-a-matching-memory-proposal flag
  OtmProposal *pOtmProposal = new (OtmProposal);

  if ( (pszSearchString == NULL) || (*pszSearchString  == EOS)  )
  {
    char* pszParm = "Error in TMManager::APISearchMem::Search string is null";
    T5LOG(T5ERROR) <<  " DDE_MANDPARAMISSING"<< pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  if ( pszStartPosition == NULL ) 
  {
    char* pszParm = "Error in TMManager::APISearchMem::pointer to start position is null";
    T5LOG(T5ERROR) << ":: DDE_MANDPARAMISSING::" << pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  if ( pProposal == NULL )
  {
    char* pszParm = "Error in TMManager::APISearchMem::Error in ::pointer to proposal is null";
    T5LOG(T5ERROR) << ":: DDE_MANDPARAMISSING::" <<  pszParm;
    return DDE_MANDPARAMISSING;
  } /* endif */

  EqfMemory *pMem = handleToMemoryObject( lHandle );
  if ( pMem == NULL )
  {
    T5LOG(T5ERROR) <<   "::FUNC_INVALID_MEMORY_HANDLE:: pMem is null" ;
    return( FUNC_INVALID_MEMORY_HANDLE );
  } /* endif */

  DWORD dwSearchStartTime = 0;
  if ( lSearchTime != 0 ) dwSearchStartTime = GetTickCount();

  // get first or next proposal
  if ( *pszStartPosition == EOS )
  {
    iRC = pMem->getFirstProposal( *pOtmProposal );
  }
  else
  {
    pMem->setSequentialAccessKey((PSZ) pszStartPosition );
    iRC = pMem->getNextProposal( *pOtmProposal );
  } /* endif */

  // prepare searchstring
  wcscpy( m_szSearchString, pszSearchString );
  if ( lOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( m_szSearchString );
  if ( lOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( m_szSearchString );

  bool fOneOrMoreIsFound = false; 
  while ( !fFound && ( iRC == 0 ) )
  {
    fFound = searchInProposal( pOtmProposal, m_szSearchString, lOptions );
     //check langs
     if( fFound ) 
     { // filter by src lang
      if (lOptions & SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT)
      {
        char lang[50];
        pOtmProposal->getSourceLanguage(lang, 50);
        fFound = strcasecmp(lang, pszSrcLang ) == 0;
      }else if(lOptions & SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT){
        char lang[50];
        pOtmProposal->getSourceLanguage(lang, 50);
        fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, pszSrcLang);
      }
    }
    if ( fFound )
    {
      if (lOptions & SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT)
      {
        char lang[50];
        pOtmProposal->getTargetLanguage(lang, 50);
        fFound = strcasecmp(lang, pszTrgLang ) == 0;
      }else if(lOptions & SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT){
        char lang[50];
        pOtmProposal->getTargetLanguage(lang, 50);
        fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, pszTrgLang);
      }
    }
    if ( fFound )
    {
      fOneOrMoreIsFound = true;
      copyOtmProposalToMemProposal( pOtmProposal , pProposal );
    }
    else
    { 
      //add check if we have at least one result before stop because of timeout 
      if ( lSearchTime != 0 )
      {
        LONG lElapsedMillis = 0;
        DWORD dwCurTime = GetTickCount();
        if ( dwCurTime < dwSearchStartTime )
        {
          // an overflow occured
          lElapsedMillis = (LONG)(dwCurTime + (ULONG_MAX - dwSearchStartTime));
        }
        else
        {
          lElapsedMillis = (LONG)(dwCurTime - dwSearchStartTime);
        } /* endif */
        if ( lElapsedMillis > lSearchTime  && fOneOrMoreIsFound )
        {
          iRC = TIMEOUT_RC;
        }
      }
      if ( iRC == 0 )
      {
        iRC = pMem->getNextProposal( *pOtmProposal );
      }
    }
  } /* endwhile */

  // search given string in proposal
  if ( fFound || (iRC == TIMEOUT_RC) )
  {
    pMem->getSequentialAccessKey( (PSZ)pszStartPosition, 20 );
    usRC = (USHORT)iRC;
  } /* endif */
  else if ( iRC == EqfMemory::INFO_ENDREACHED )
  {
    usRC = ENDREACHED_RC;
  }
  else
  {
    usRC = (USHORT)iRC;
  }

  delete( pOtmProposal );
  return( usRC );
}


/*! \brief process the API call: EqfUpdateMem and update a segment in the memory
  \param lHandle handle of a previously opened memory
    \param pNewProposal pointer to an MemProposal structure containing the segment data
  \param lOptions processing options
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIUpdateMem
(
  EqfMemory*        lHandle,
  PMEMPROPOSAL pNewProposal,
  LONG        lOptions
)
{
  OtmProposal *pOtmProposal = new ( OtmProposal );

  if ( ( pNewProposal == NULL ) )
  {
    T5LOG(T5ERROR) <<  ":: DDE_MANDPARAMISSING:: pointer to proposal is null";
    return DDE_MANDPARAMISSING;
  } /* endif */

 // EqfMemory *pMem = handleToMemoryObject( lHandle );
  if ( lHandle == NULL )
  {
    return( INVALIDFILEHANDLE_RC );
  } /* endif */
  
  strcpy(pNewProposal->szDocShortName , pNewProposal->szDocName);
  copyMemProposalToOtmProposal( pNewProposal, pOtmProposal );

  USHORT usRC = (USHORT)lHandle->putProposal( *pOtmProposal );

  delete( pOtmProposal );

  return( usRC );
}



/*! \brief process the API call: EqfUpdateDeleteMem and deletes a segment in the memory
    \param lHandle handle of a previously opened memory
    \param pProposalToDelete pointer to an MemProposal structure containing the segment data
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */


// data structure for the APIListMem function and the insert memory callback function
typedef struct _APILISTMEMDATA
{
  const char*   pszBuffer;   // pointer to buffer for memory names or NULL to compute the size of the required buffer
  LONG  lBufSize;  // size of the buffer
  LONG *plLength;    // pointer to a variable containing the current length of the data in the buffer
} APILISTMEMDATA, *PAPILISTMEMDATA;

// callback function to add memory names to a list or update the length of the list
int AddMemToList( void *pvData, char *pszName,std::shared_ptr<EqfMemory>  pInfo )
{
  PAPILISTMEMDATA pData = ( PAPILISTMEMDATA )pvData;
  LONG lNameLen = strlen( pszName );

  if ( pData->pszBuffer != NULL )
  {
    if ( (*(pData->plLength) + lNameLen + 1) <= pData->lBufSize )
    {
      if ( *(pData->plLength) != 0 )
      {
        strcpy( (PSZ)pData->pszBuffer + *(pData->plLength), (PSZ)"," );
        *( pData->plLength ) += 1;
      }
      strcpy( (PSZ)pData->pszBuffer + *(pData->plLength), (PSZ)pszName );
      *( pData->plLength ) += lNameLen;
    }
  }
  else
  {
    *( pData->plLength ) += lNameLen + 1;
  }
  return( 0 );
}

/*! \brief List the name of all memories
\param hSession the session handle returned by the EqfStartSession call
\param pszBuffer pointer to a buffer reiceiving the comma separated list of memory names or NULL to get the required length for the list
\param plLength pointer to a variable containing the size of the buffer area, on return the length of name list is stored in this variable
\returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIListMem
(
  const char*         pszBuffer,
  LONG        *plLength
)
{
  APILISTMEMDATA Data;

  Data.pszBuffer = pszBuffer;
  Data.lBufSize = *plLength;
  Data.plLength = plLength;
  *plLength = 0;

  for ( std::size_t i = 0; i < pluginList->size(); i++ )
  {
    EqfMemoryPlugin *pluginCurrent = ( *pluginList )[i];

    //pluginCurrent->listMemories( AddMemToList, (void *)&Data, FALSE );
    
    //pluginCurrent->listMemories( AddMemToList, (void *)&Data, TRUE );
  } /* endfor */

  return( 0 );
}

/*! \brief get the index into the memory object table from a memory handle
  \param lHandle handle of a previously opened memory
  \returns index into the memory object table
*/
LONG TMManager::getIndexFromHandle
(
  LONG        lHandle
)
{
  return( lHandle & 0x000007FF );
}

/*! \brief get the checksum from a memory handle
  \param lHandle handle of a previously opened memory
  \returns checksum of the memory object
*/
LONG TMManager::getCheckSumFromHandle
(
  LONG        lHandle
)
{
  return( lHandle & 0xFFFFF800 );
}

/*! \brief compute the checksum for a memory object
  \param pMemory pointer to a memory object
  \returns memory object checksum
*/
LONG TMManager::computeMemoryObjectChecksum
(
  EqfMemory *pMemory
)
{
  // compute checksum based on t3he individual byte values of the memory object pointer
  LONG lCheckSum = 0;
  INT64 iPointerValue = (INT64)pMemory;  // use int64 value to enable code for 64bit pointers
  int iSize = sizeof(*pMemory);
  while( iSize > 0 )
  {
    lCheckSum += (LONG)((iPointerValue  & 0x0000000F) * iSize);
    iPointerValue = iPointerValue >> 4;
    iSize--;
  } /* endwhile */

  // make room at end of checksum for the 11 bit value of the index
  lCheckSum = lCheckSum << 11;

  return( lCheckSum );
}

/*! \brief compute the checksum for a memory object
  \param lHandle handel referring to the memory object
  \returns memory object pointer or NULL if the given handle is invalid
*/
EqfMemory *TMManager::handleToMemoryObject
(
  LONG lHandle
)
{
  LONG lCheckSum = getCheckSumFromHandle( lHandle );
  LONG lIndex = getIndexFromHandle( lHandle );

  if ( (lIndex < 0) || (lIndex >= (LONG)pHandleToMemoryList->size()) )
  {
    return( NULL );
  } /* endif */

  if ( (*pHandleToMemoryList)[lIndex] == NULL  )
  {
    return( NULL );
  } /* endif */

  LONG lElementCheckSum = computeMemoryObjectChecksum( (*pHandleToMemoryList)[lIndex] );
  if ( lCheckSum  != lElementCheckSum )
  {
    return( NULL );
  } /* endif */

  return( (*pHandleToMemoryList)[lIndex] );
}

/*! \brief convert a memory object and the index into the memory oject table to a memory handle
  \param lIndex index into the memory object table
  \param pMemory pointer to a memory object
  \returns index into the memory object table
*/
LONG TMManager::createHandle
(
  LONG        lIndex,
  EqfMemory   *pMemory
)
{
  LONG lCheckSum = computeMemoryObjectChecksum( pMemory );

  return( lCheckSum | lIndex );
}

/*! \brief search a string in a proposal
  \param pProposal pointer to the proposal 
  \param pszSearch pointer to the search string (when fIngoreCase is being used, this strign has to be in uppercase)
  \param lSearchOptions combination of search options
  \returns TRUE if the proposal contains the searched string otherwise FALSE is returned
*/
BOOL searchInProposal
( 
  OtmProposal *pProposal,
  PSZ_W pszSearch,
  LONG lSearchOptions
)
{
  BOOL   fFound = FALSE;

  wchar_t szSegmentText[MAX_SEGMENT_SIZE +1];

  if ( !fFound && (lSearchOptions & SEARCH_IN_SOURCE_OPT) )
  {
    pProposal->getSource( szSegmentText, MAX_SEGMENT_SIZE );
    if ( lSearchOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( szSegmentText );
    if ( lSearchOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( szSegmentText );
    fFound = findString( szSegmentText, pszSearch );
  }

  if ( !fFound && (lSearchOptions & SEARCH_IN_TARGET_OPT)  )
  {
    pProposal->getTarget( szSegmentText, MAX_SEGMENT_SIZE );
    if ( lSearchOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( szSegmentText );
    if ( lSearchOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( szSegmentText );
    fFound = findString( szSegmentText, pszSearch );
  }

  return( fFound );
}

/*! \brief find the given string in the provided data
  \param pszData pointer to the data being searched
  \param pszSearch pointer to the search string
  \returns TRUE if the data contains the searched string otherwise FALSE is returned
*/
BOOL findString
( 
  PSZ_W pszData,
  PSZ_W pszSearch
)
{
  CHAR_W chChar;
  CHAR_W c;
  BOOL   fFound = FALSE;
  BOOL   fEnd = FALSE;

  while ( !fFound && !fEnd )
  {
    // Scan for the 1st letter of the target string don't use strchr, it is too slow                   
    chChar = *pszSearch;
    while ( ((c = *pszData) != 0) && (chChar != c) ) pszData++;

    if ( *pszData == 0 )
    {
      fEnd = TRUE;
    }
    else
    {
      // check for complete match
      if ( compareString( pszData, pszSearch ) == 0)
      {
        fFound = TRUE;
      }
      else         // no match; go on if possible
      {
        pszData++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  return( fFound );
}

/*! \brief check if search string matches current data
  \param pData pointer to current position in data area
  \param pSearch pointer to search string
  \returns 0 if search string matches data
*/
SHORT compareString
(
  PSZ_W   pData,
  PSZ_W   pSearch
)
{
  SHORT  sRc = 0;                               // init strings are equal
  CHAR_W c, d;
  while (((d = *pSearch) != NULC) && ((c = *pData) != NULC) )
  {
    if ( c == d )
    {
      pData++; pSearch++;
    }
    else
    {
      sRc = c-d;
      break;
    } /* endif */
  } /* endwhile */
  if (*pSearch )
  {
    sRc = -1;
  } /* endif */
  return sRc;
}

/*! \brief normalize white space within a string by replacing single or multiple white space occurences to a single blank
  \param pszString pointer to the string being normalized
  \returns 0 in any case
*/
SHORT normalizeWhiteSpace
(
  PSZ_W   pszData
)
{
  PSZ_W pszTarget = pszData;
  while ( *pszData )
  {
    CHAR_W c = *pszData;
    if ( (c == BLANK) || (c == LF) || (c == CR) || (c == 0x09) )
    {
      do
      {
        pszData++;
        c = *pszData;
      } while ( (c == BLANK) || (c == LF) || (c == CR) || (c == 0x09) );
      *pszTarget++ = BLANK;
    } 
    else
    {
      *pszTarget++ = *pszData++;
    } /* endif */
  } /* endwhile */
  *pszTarget = 0;
  return 0;
}

/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyMemProposalToOtmProposal( PMEMPROPOSAL pProposal, OtmProposal *pOtmProposal )
{
  if ( (pProposal == NULL) || (pOtmProposal == NULL) ) return;

  pOtmProposal->setID( pProposal->szId );
  pOtmProposal->setSource( pProposal->szSource );
  pOtmProposal->setTarget( pProposal->szTarget );
  pOtmProposal->setDocName( pProposal->szDocName );
  pOtmProposal->setDocShortName( pProposal->szDocShortName );
  pOtmProposal->setSegmentNum( pProposal->lSegmentNum );
  pOtmProposal->setSourceLanguage( pProposal->szSourceLanguage );
  pOtmProposal->setIsSourceLangIsPrefered( pProposal->fIsoSourceLangIsPrefered);
  pOtmProposal->setTargetLanguage( pProposal->szTargetLanguage );

  switch( pProposal->eType )
  {
    case MANUAL_PROPTYPE: pOtmProposal->setType( OtmProposal::eptManual ); break;
    case MACHINE_PROPTYPE: pOtmProposal->setType( OtmProposal::eptMachine ); break;
    case GLOBMEMORY_PROPTYPE: pOtmProposal->setType( OtmProposal::eptGlobalMemory ); break;
    case GLOBMEMORYSTAR_PROPTYPE: pOtmProposal->setType( OtmProposal::eptGlobalMemoryStar ); break;
    default: pOtmProposal->setType( OtmProposal::eptUndefined ); break;
  } /* endswitch */
  switch( pProposal->eMatch )
  {
    case EXACT_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExact ); break;
		case EXACTEXACT_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExactExact ); break;
    case EXACTSAMEDOC_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExactSameDoc ); break;
    case FUZZY_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtFuzzy ); break;
    case REPLACE_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtReplace ); break;
    default: pOtmProposal->setMatchType( OtmProposal::emtUndefined ); break;
  } /* endswitch */
  pOtmProposal->setAuthor( pProposal->szTargetAuthor );
  pOtmProposal->setUpdateTime( pProposal->lTargetTime );
  pOtmProposal->setFuzziness( pProposal->iFuzziness );
  pOtmProposal->setMarkup( pProposal->szMarkup );
  pOtmProposal->setContext( pProposal->szContext );
  pOtmProposal->setAddInfo( pProposal->szAddInfo );
}

/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyOtmProposalToMemProposal( OtmProposal *pOtmProposal, PMEMPROPOSAL pProposal  )
{
  if ( (pProposal == NULL) || (pOtmProposal == NULL) ) return;

  pOtmProposal->getID( pProposal->szId, sizeof(pProposal->szId) );
  pOtmProposal->getSource( pProposal->szSource, sizeof(pProposal->szSource)/sizeof(CHAR_W) );
  pOtmProposal->getTarget( pProposal->szTarget, sizeof(pProposal->szTarget)/sizeof(CHAR_W) );
  pOtmProposal->getDocName( pProposal->szDocName, sizeof(pProposal->szDocName) );
  pOtmProposal->getDocShortName( pProposal->szDocShortName, sizeof(pProposal->szDocShortName) );
  pProposal->lSegmentNum = pOtmProposal->getSegmentNum();

  pOtmProposal->getOriginalSourceLanguage( pProposal->szSourceLanguage, sizeof(pProposal->szSourceLanguage) );
  pOtmProposal->getTargetLanguage( pProposal->szTargetLanguage, sizeof(pProposal->szTargetLanguage) );
  switch( pOtmProposal->getType() )
  {
    case OtmProposal::eptManual: pProposal->eType = MANUAL_PROPTYPE; break;
    case OtmProposal::eptMachine: pProposal->eType = MACHINE_PROPTYPE; break;
    case OtmProposal::eptGlobalMemory: pProposal->eType = GLOBMEMORY_PROPTYPE; break;
    case OtmProposal::eptGlobalMemoryStar: pProposal->eType = GLOBMEMORYSTAR_PROPTYPE; break;
    default: pProposal->eType = UNDEFINED_PROPTYPE; break;
  } /* endswitch */
  switch( pOtmProposal->getMatchType() )
  {
    case OtmProposal::emtExact: pProposal->eMatch = EXACT_MATCHTYPE; break;
    case OtmProposal::emtExactExact: pProposal->eMatch = EXACTEXACT_MATCHTYPE; break;
    case OtmProposal::emtExactSameDoc : pProposal->eMatch = EXACTSAMEDOC_MATCHTYPE; break;
    case OtmProposal::emtFuzzy: pProposal->eMatch = FUZZY_MATCHTYPE; break;
    case OtmProposal::emtReplace: pProposal->eMatch = REPLACE_MATCHTYPE; break;
    default: pProposal->eMatch = UNDEFINED_MATCHTYPE; break;
  } /* endswitch */
  pOtmProposal->getAuthor( pProposal->szTargetAuthor, sizeof(pProposal->szTargetAuthor) );
  pProposal->lTargetTime = pOtmProposal->getUpdateTime();
  pProposal->iFuzziness  = pOtmProposal->getFuzziness();
  pProposal->iDiffs = pOtmProposal->getDiffs();
  pProposal->iWords = pOtmProposal->getWords();
  pOtmProposal->getMarkup( pProposal->szMarkup, sizeof(pProposal->szMarkup) );
  pOtmProposal->getContext( pProposal->szContext, sizeof(pProposal->szContext)/sizeof(CHAR_W) );
  pOtmProposal->getAddInfo( pProposal->szAddInfo, sizeof(pProposal->szAddInfo)/sizeof(CHAR_W) );
}


std::string TMManager::GetTmdPath(const std::string& memName){
  return FilesystemHelper::GetMemDir() + memName  + EXT_OF_TMDATA;
}

std::string TMManager::GetTmiPath(const std::string& memName){
  return FilesystemHelper::GetMemDir() + memName  + EXT_OF_TMINDEX;
}

int TMManager::TMExistsOnDisk(const std::string& tmName, bool logErrorIfNotExists){  
  
  int logLevel= 0;
  if(!logErrorIfNotExists) logLevel = T5Logger::GetInstance()->suppressLogging();
  //check tmd file
  int res = TMM_NO_ERROR;
  if(FilesystemHelper::FileExists(GetTmdPath(tmName)) == false){
    res |= TMM_TMD_NOT_FOUND;
  }

  //check tmi file
  if(FilesystemHelper::FileExists(GetTmiPath(tmName)) == false){
    res |= TMM_TMI_NOT_FOUND;
  }
  if(!logErrorIfNotExists) T5Logger::GetInstance()->desuppressLogging(logLevel);
  return res;
}


bool TMManager::IsMemoryLoaded(const std::string& strMemName){
  return tms.count(strMemName);
}

int TMManager::OpenTM(const std::string& strMemName){
  if(IsMemoryLoaded(strMemName)){
    return 0;
  }
  size_t requiredMemory = 0;
    {
      requiredMemory += FilesystemHelper::GetFileSize( GetTmdPath(strMemName));
      requiredMemory += FilesystemHelper::GetFileSize( GetTmiPath(strMemName));
      requiredMemory *= 1.2;
      //requiredMemory += FilesystemHelper::GetFileSize( szTempFile ) * 2;
      //requiredMemory += strBody.size() * 2;
    }
    //TODO: add to requiredMemory value that would present changes in mem files sizes after import done 

    // cleanup the memory list (close memories not used for a longer time)
    size_t memLeftAfterOpening = CleanupMemoryList(requiredMemory);
    if(VLOG_IS_ON(1)){
      T5LOG( T5INFO) << ":: memory: " << strMemName << "; required RAM:" 
          << requiredMemory << "; RAM left after opening mem: " << memLeftAfterOpening;
    }
    //requiredMemory = 0;
    // find a free slot in the memory list
    //std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->GetFreeSlotPointer(requiredMemory);

    // handle "too many open memories" condition
    //if ( pMem == nullptr )
    
    if(memLeftAfterOpening <= 0){
      //buildErrorReturn( _rc_, "OtmMemoryServiceWorker::import::Error: too many open translation memory databases" );
      return( INTERNAL_SERVER_ERROR );
    } /* endif */
  std::shared_ptr<EqfMemory> pMem( EqfMemoryPlugin::GetInstance()->openMemoryNew(strMemName));
  if(!pMem){
    T5LOG(T5ERROR) << "::Can't open the file \'"<< strMemName<<"\'";
    return 1;
  }
  tms[strMemName] = pMem; 
}

int TMManager::AddMem(const std::shared_ptr<EqfMemory> NewMem){
  if(NewMem.get() == nullptr || NewMem.get()->szName == "\0"){
    return -1;
  }
  if(IsMemoryLoaded(NewMem.get()->szName)){
    return -2;
  }
  tms[NewMem.get()->szName] = NewMem;
  return 0;

}

int TMManager::CloseTM(const std::string& strMemName){

}


int TMManager::DeleteTM(const std::string& strMemName, std::string& outputMsg){
  /*
  if ( (pMemInfo.get() != NULL) && (pMemInfo.get()->szFullPath[0] != EOS) )
  {
    // delete the property file
    T5LOG( T5DEBUG) << "EqfMemoryPlugin::deleteMemory:: try to delete property file: " << pMemInfo.get()->szFullPath ;

    UtlDelete( pMemInfo.get()->szFullPath, 0L, FALSE );

    char szPath[MAX_LONGFILESPEC];
    strcpy( szPath, pMemInfo.get()->szFullPath );
    strcpy( strrchr( szPath, DOT ), EXT_OF_TMINDEX );
    // delete index file
    T5LOG( T5DEBUG) <<"EqfMemoryPlugin::deleteMemory:: try to delete index file: "<< szPath ;
    UtlDelete( szPath, 0L, FALSE );
    
    // delete data file
    strcpy( strrchr( szPath, DOT ), EXT_OF_TMDATA );
    T5LOG( T5DEBUG) << "EqfMemoryPlugin::deleteMemory:: try to delete data file: "<< szPath ;    
    UtlDelete( szPath, 0L, FALSE );

    // remove memory infor from our memory info vector
    m_MemInfoVector.erase(m_MemInfoVector.begin( )+idx);
  }
  //*/
  return 0;

}

std::shared_ptr<EqfMemory> TMManager::requestServicePointer(const std::string& strMemName, COMMAND command){
  std::shared_ptr<EqfMemory> mem;
  if(IsMemoryLoaded(strMemName)){
    if(command == STATUS_MEM){
      return tms[strMemName];
    }
  }
  return nullptr;
}
std::shared_ptr<EqfMemory> TMManager::requestReadOnlyTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack){
  int rc = 0;
  if(!IsMemoryLoaded(strMemName)){
    rc = OpenTM(strMemName);
  }
  auto mem = tms[strMemName];
  if(rc){
    //return;
  }else{
    //check if there are any Write pointers
    rc = mem.get()->writeCnt.use_count()>1;
  }
  //
  if(rc){
    //return nullptr;
  }
  refBack = mem.get()->readOnlyCnt;
  return mem;
}

std::shared_ptr<EqfMemory> TMManager::requestWriteTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack){
  int rc = 0;
  if(!IsMemoryLoaded(strMemName)){
    rc = OpenTM(strMemName);
  }

  auto mem = tms[strMemName];
  if(rc){ //Memory failed to open
    //return;
  }else{
    //check if there are any Write pointers
    //rc = mem.get()->writeCnt.use_count() > 1;
  }
  //
  if(rc){ // we have some Write process;
    //return nullptr;
  }else{
    //rc = mem.get()->readOnlyCnt.use_count() > 1;
  }

  if(rc){ // we have some Read process

  }
  //refBack = mem.get()->readOnlyCnt;
  return mem;
}
