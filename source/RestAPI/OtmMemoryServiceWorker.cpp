//+----------------------------------------------------------------------------+
//| OtmMemoryService.CPP                                                       |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (c) 2020, QSoft GmbH. All rights reserved.                  |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Processes the requests from the web service frame work        |
//+----------------------------------------------------------------------------+
//

#include <time.h>
#include <stdarg.h>
#include <string>
#include <cstring>
#include <strings.h>
#include <linux/limits.h>

#include "ThreadingWrapper.h"
//#include <restbed>
#include "OtmMemoryServiceWorker.h"
#include "OTMMSJSONFactory.h"
#include "tm.h"
#include "EQFMSG.H"
#include "win_types.h"
#include "opentm2/core/utilities/FilesystemWrapper.h"
#include "opentm2/core/utilities/LogWrapper.h"
#include <sstream>
#include "opentm2/core/utilities/EncodingHelper.h"

#include "opentm2/core/utilities/Property.h"
#include "opentm2/core/utilities/FilesystemHelper.h"
#include <thread>
#include <utility>

#include <xercesc/util/PlatformUtils.hpp>
#include "../cmake/git_version.h"
#include "opentm2/core/utilities/ThreadingWrapper.h"
#include "opentm2/core/utilities/LanguageFactory.H"

#include "opentm2/core/utilities/Stopwatch.hpp"
#include "EQFMORPH.H"
#include "requestdata.h"



// import memory process
void importMemoryProcess( void *pvData );

/*! \brief This static method returns a pointer to the OtmMemoryService object.
	The first call of the method creates the OtmMemoryService instance.
*/
OtmMemoryServiceWorker* OtmMemoryServiceWorker::getInstance()
{
  static OtmMemoryServiceWorker _instance;

	return &_instance;
}

/*! \brief Data area for the processing of the openMemory function
*/
typedef struct _OPENMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} OPENMEMORYDATA, *POPENMEMORYDATA;

/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CLOSEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} CLOSEMEMORYDATA, *PCLOSEMEMORYDATA;





int OtmMemoryServiceWorker::init(){
  return verifyAPISession();
}

  /*! \brief Verify OpenTM2 API session
    \returns 0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::verifyAPISession()
{
  if ( this->hSession != 0 ){ 
    return( 0 );
  }

  T5LOG( T5DEBUG) << "Initializing API Session";
  this->iLastRC = EqfStartSession( &(this->hSession) );
  T5LOG( T5DEBUG) << "[verifyAPISession] EqfStartSession ret: "  << this->iLastRC;

  if ( this->iLastRC != 0 ) {
    T5LOG(T5ERROR) << "OpenTM2 API session could not be started, the return code is" <<  this->iLastRC ;
  }else{
      TMManager::GetInstance()->hSession = hSession;
      try {
          xercesc::XMLPlatformUtils::Initialize();
      }
      catch (const xercesc::XMLException& toCatch) {
          toCatch;
          T5LOG(T5ERROR) << ":: cant init Xercesc";
          return( ERROR_NOT_READY );
      }
  }
  
  return( this->iLastRC );
}




/*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
\param lTime long time value
\param pszDateTime buffer receiving the converted date and time
\returns 0
*/
int convertTimeToDateTimeString( LONG lTime, char *pszDateTime )
{
  struct tm   *pTimeDate;            // time/date structure

  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

  pTimeDate = gmtime( (time_t *)&lTime );
  if ( ( lTime != 0L ) && pTimeDate )   // if gmtime was successful ...
  {
    sprintf( pszDateTime, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
      pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
      pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
  }
  else
  {
    *pszDateTime = 0;
  } /* endif */

  return( 0 );
}



bool getValue( char *pszString, int iLen, int *piResult );

/*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
\param pszDateTime buffer containing the TMX date and time
\param plTime pointer to a long buffer receiving the converted time
\returns 0
*/
int convertDateTimeStringToLong( char *pszDateTime, PLONG plTime )
{
  *plTime = 0;

  // we currently support only dates in the form YYYYMMDDThhmmssZ
  if ( ( pszDateTime != NULL ) && ( strlen( pszDateTime ) == 16 ) )
  {
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    // split string into date/time parts
    BOOL fOK = getValue( pszDateTime, 4, &iYear );
    if ( fOK ) fOK = getValue( pszDateTime + 5, 2, &iMonth );
    if ( fOK ) fOK = getValue( pszDateTime + 8, 2, &iDay );
    if ( fOK ) fOK = getValue( pszDateTime + 11, 2, &iHour );
    if ( fOK ) fOK = getValue( pszDateTime + 14, 2, &iMin );
    if ( fOK ) fOK = getValue( pszDateTime + 17, 2, &iSec );

    if ( fOK )
    {
      struct tm timeStruct;
      memset( &timeStruct, 0, sizeof( timeStruct ) );
      timeStruct.tm_hour = iHour;
      timeStruct.tm_min = iMin;
      timeStruct.tm_sec = iSec;
      timeStruct.tm_mday = iDay;
      timeStruct.tm_mon = iMonth - 1;
      timeStruct.tm_year = iYear - 1900;

      *plTime = mktime( &timeStruct );

      *plTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)

    } /* endif */
  } /* endif */
  return( 0 );
}





template<typename T>
void AddObjToJson(std::stringstream& ss, const char* key, T value, bool fAddSeparator){
  ss << "{ \"" << key << "\" : ";

  //if (std::is_same<T, int>::value)
  if(std::is_arithmetic<T>::value) // if it's number - skip quotes
  {
    ss << value;
  }else{
    ss << "\"" << value << "\"";
  }
  ss << " }";
  if(fAddSeparator)
    ss << ",";

  ss << "\n";
}

/*! \brief Saves all open and modified memories
  \returns http return code0 if successful or an error code in case of failures
  
int OtmMemoryServiceWorker::closeAll(){
  int rc = TMManager::GetInstance()->closeAll();
  return rc;
}//*/



std::string printTime(time_t time){
  char buff[255];
  LONG lTimeStamp = (LONG) time;
  lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
  convertTimeToUTC( lTimeStamp, buff );
  //std::string res =  asctime(gmtime(&time));
  return buff;
}


#include <tm.h>


/*! \brief read a binary file and encode it using BASE64 encoding
\param pszFile fully qualified name of file being encoded
\param ppStringData adress of a pointer which will receive the (per malloc allocated) pointer to the area for the encoded string
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int FilesystemHelper::EncodeFileInBase64( char *pszFile, char **ppStringData, std::string &strError )
{
  int iRC = 0;

  auto file = FilesystemHelper::OpenFile(pszFile, "r", false);
  size_t fSize = FilesystemHelper::GetFileSize(file);
  int bytesRead = 0;
  if(fSize == 0){

  }else{
    unsigned char* pData = new unsigned char[fSize];
    FilesystemHelper::ReadFile(file, pData, fSize, bytesRead);
    FilesystemHelper::CloseFile(file);

    if(bytesRead != fSize){

    }else{    
      std::string encodedData;
      EncodingHelper::Base64Encode(pData, fSize, encodedData);
      if(encodedData.empty())
      {
        iRC = GetLastError();
        strError = "encoding of BASE64 data failed";
        T5LOG(T5ERROR) << "encodeBase64ToFile()::ecnoding of BASE64 data failed, iRC = " << iRC;
        return( iRC );
      }
    }

    // cleanup
    delete[] pData ;
  }
  return( iRC );
}

/*! \brief convert a BASE64 encoded string to a binary file
\param pStringData pointer to the BASE64 data
\param pszFile fully qualified name of file being written
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int FilesystemHelper::DecodeBase64ToFile( const char *pStringData, const char *pszFile, std::string &strError )
{
  int iRC = 0;

  // get decoded length of data
  DWORD dwDecodedLength = 0;
  std::string sData(pStringData);
  unsigned char* pData = NULL;
  int pDataSize = 0;
  EncodingHelper::Base64Decode(sData, pData, pDataSize);
  if(pDataSize == 0 || pData == NULL)
  {
    iRC = GetLastError();
    strError = "decoding of BASE64 data failed";
    T5LOG(T5ERROR) << "decodeBase64ToFile()::decoding of BASE64 data failed, iRC = " << iRC ;
    return( iRC );
  }

  auto file = FilesystemHelper::OpenFile(pszFile, "w+", false);
  FilesystemHelper::WriteToFile(file, pData, pDataSize);
  FilesystemHelper::CloseFile(file);
  // cleanup
  delete[] pData ;

  return( iRC );
}

//*
int FilesystemHelper::WriteToFile( const std::string& pszFile, const std::string& Data, std::string &strError )
{
  int iRC = 0;
  if(Data.empty() || pszFile.empty())
  {
    strError = "can't save tmx into the file \'" + (pszFile) +"\'   - data or file are empty";
    T5LOG(T5ERROR) << strError << iRC ;
    return( -1);
  }

  auto file = FilesystemHelper::OpenFile(pszFile.c_str(), "w+", false);
  FilesystemHelper::WriteToFile(file, &Data[0], Data.size());
  FilesystemHelper::CloseFile(file);

  return( iRC );
}
//*/

DECLARE_bool(useTimedMutexesForReorganizeAndImport);

void importMemoryProcess( void* pvData )
{
  IMPORTMEMORYDATA* pData = (IMPORTMEMORYDATA*)pvData;
  #ifdef TIME_MEASURES
  milliseconds start_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
  #endif 
  // call the OpenTM2 API function
  if(!pData || !pData->mem){
    T5LOG(T5ERROR) << "pData or pMem is null";
    return;
  }
  
  if(!FLAGS_useTimedMutexesForReorganizeAndImport){
    pData->tmLockTimeout.setTimeout_ms(0);
  }

  TimedMutexGuard l(pData->mem->tmMutex, pData->tmLockTimeout, "tmMutex import", __func__, __LINE__);
  if(pData->tmLockTimeout.failed()){
    pData->tmLockTimeout.addToErrMsg(".Failed to lock tm list:", __func__, __LINE__);
    return; 
  }

  pData->mem->setActiveRequestCommand(COMMAND::IMPORT_MEM);

  pData->szError[0] = 0;
  if(!pData->mem->importDetails){
    pData->mem->importDetails = new ImportStatusDetails;
  }
  pData->mem->importDetails->inclosingTagsBehaviour = pData->inclosingTagsBehaviour;
  
  int iRC =(int)EqfImportMem( pData, TMX_OPT | COMPLETE_IN_ONE_CALL_OPT);

  // handle any error
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    T5LOG(T5ERROR) <<"importMemoryProcess:: error = ", pData->szError;
  }

  // update memory status
  pData->mem->importDone( iRC, pData->szError );
 
  // cleanup
  if(pData->fDeleteTmx && T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    std::string tempFile =  pData->szInFile;
    FilesystemHelper::DeleteFile( tempFile.c_str(), true );
    tempFile += ".Temp";
    FilesystemHelper::DeleteFile(tempFile.c_str(), true);
  }

  #ifdef TIME_MEASURES
  milliseconds end_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
  milliseconds time = end_ms-start_ms;   
  T5LOG(T5TRANSACTION) /*<<"id = " << id*/ << "; exection time = " <<  std::chrono::duration<double>(time).count();
  ProxygenStats::getInstance()->addRequestTime(ProxygenService::ProxygenHandler::COMMAND::IMPORT_MEM, time);
  #endif

  //pData->mem->resetActiveRequestCommand();
  delete( pData );

  return;
}


#include "EQFDASD.H"

void reorganizeMemoryProcess( void* pvData )
{
  PFCTDATA    pData = (PFCTDATA)pvData;                      // ptr to function data area
  PPROCESSCOMMAREA pCommArea = (PPROCESSCOMMAREA)pData->pvMemOrganizeCommArea; // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA =  (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;            // pointer to instance area
  #ifdef TIME_MEASURES
  milliseconds start_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
  #endif 
  // call the OpenTM2 API function
  pData->szError[0] = 0;
  if(!pRIDA || !pRIDA->pMem){
    T5LOG(T5ERROR) << "pRIDA or pMem is null";
    return;
  }
  
  if(!FLAGS_useTimedMutexesForReorganizeAndImport){
    pData->tmLockTimeout.setTimeout_ms(0);
  }

  TimedMutexGuard l(pRIDA->pMem->tmMutex, pData->tmLockTimeout, "tmMutex reorganize", __func__, __LINE__);
  if(pData->tmLockTimeout.failed()){
    pData->tmLockTimeout.addToErrMsg(".Failed to lock tm list:", __func__, __LINE__);
    return; 
  }

  pRIDA->pMem->setActiveRequestCommand(COMMAND::REORGANIZE_MEM);

  int _rc_ = 0;
  pRIDA->pMem->usAccessMode |= ASD_ORGANIZE;
  while((_rc_ == NO_ERROR || _rc_ == CONTINUE_RC) && !pData->fComplete  )
  {      
    // continue current organize process
    switch ( pRIDA->NextTask )
    {
      case MEM_START_ORGANIZE:
        EQFMemOrganizeStart( pCommArea );
        break;

      case MEM_ORGANIZE_TASK:
        EQFMemOrganizeProcess( pCommArea );
        break;

      case MEM_END_ORGANIZE:
        pData->fComplete = TRUE;
        EQFMemOrganizeEnd( pCommArea);
        if ( pRIDA->pszNameList ) UtlAlloc( (PVOID *)pRIDA->pszNameList, 0L, 0L, NOMSG );
        break;
    } /* endswitch */
    _rc_ = pRIDA->usRC;
  };

  // handle any error
  if ( _rc_ != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    T5LOG(T5ERROR) <<"reorganizeMemoryProcess:: error = " << pData->szError << "; _rc_ = " << std::to_string(_rc_);
  }
  // update memory status
  if(pRIDA && pRIDA->pMem){
    pRIDA->pMem->reorganizeDone(_rc_, pData->szError );
    pRIDA->pMem.reset();
    pRIDA->memRef.reset();
  }

  // cleanup
  //if(pData->fDeleteSourceTmx && T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
  //  DeleteFile( pData->szInFile );
  //}
  #ifdef TIME_MEASURES
  milliseconds end_ms = duration_cast< milliseconds >( system_clock::now().time_since_epoch() );
  milliseconds time = end_ms-start_ms;   
  T5LOG(T5TRANSACTION) /*<<"id = " << id*/ << "; exection time = " <<  std::chrono::duration<double>(time).count();
  ProxygenStats::getInstance()->addRequestTime(ProxygenService::ProxygenHandler::COMMAND::IMPORT_MEM, time);
  #endif
  
  if(pCommArea) delete( pCommArea );
  if(pRIDA) delete( pRIDA );
  if(pData) delete( pData );

  return;
}
