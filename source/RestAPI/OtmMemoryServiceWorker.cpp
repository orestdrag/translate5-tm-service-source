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

/*! \brief OtmMemoryServiceWorker constructor
*/
OtmMemoryServiceWorker::OtmMemoryServiceWorker()
{
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

/*! \brief build return JSON string in case of errors
  \param iRC error return code
  \param pszErrorMessage error message text
  \param strError JSON string receiving the error information
  \returns 0 if successful or an error code in case of failures
*/
int buildErrorReturn
(
  int iRC,
  wchar_t *pszErrorMsg,
  std::wstring &strErrorReturn
)
{
  JSONFactory *factory = JSONFactory::getInstance();
  int i = strErrorReturn.size();
  factory->startJSONW( strErrorReturn );
  i = strErrorReturn.size();
  factory->addParmToJSONW( strErrorReturn, L"ReturnValue", iRC );
  i = strErrorReturn.size();
  factory->addParmToJSONW( strErrorReturn, L"ErrorMsg", pszErrorMsg );
  i = strErrorReturn.size();
  factory->terminateJSONW( strErrorReturn );
  i = strErrorReturn.size();
  return( 0 );
}

int buildErrorReturn
(
	int iRC,
	wchar_t *pszErrorMsg,
	std::string &strErrorReturn
)
{
	std::wstring strUTF16;
	buildErrorReturn( iRC, pszErrorMsg, strUTF16 );
  strErrorReturn = EncodingHelper::convertToUTF8( strUTF16 );
  //strUTF16 += L" [utf16]";
  //T5LOG(T5ERROR) << strUTF16);
  T5LOG(T5ERROR) << strErrorReturn << ", ErrorCode = " << iRC;
	return(0);
}

int buildErrorReturn
(
  int iRC,
  char *pszErrorMsg,
  std::string &strErrorReturn
)
{
  JSONFactory *factory = JSONFactory::getInstance();
  int i = strErrorReturn.size();
  factory->startJSON( strErrorReturn );
  i = strErrorReturn.size();
  factory->addParmToJSON( strErrorReturn, "ReturnValue", iRC );
  i = strErrorReturn.size();
  factory->addParmToJSON( strErrorReturn, "ErrorMsg", pszErrorMsg );
  i = strErrorReturn.size();
  factory->terminateJSON( strErrorReturn );
  i = strErrorReturn.size();
  return( 0 );
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


int addProposalToJSONString ( std::wstring &strJSON, PMEMPROPOSAL pProp, void *pvData );



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





/*! \brief get the data of a memory in binary form or TMX form
\param strMemory name of memory
\param strType type of the requested memory data
\param vMemData vector<Byte> receiving the memory data
\returns http return code
*/
int OtmMemoryServiceWorker::getMem
(
  std::string strMemory,
  std::string strType,
  std::vector<unsigned char> &vMemData
)
{
  return( OK );
}


std::string printTime(time_t time){
  char buff[255];
  LONG lTimeStamp = (LONG) time;
  lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
  convertTimeToUTC( lTimeStamp, buff );
  //std::string res =  asctime(gmtime(&time));
  return buff;
}


std::vector<std::wstring> OtmMemoryServiceWorker::replaceString(std::wstring&& src_data, std::wstring&& trg_data, std::wstring&& req_data,  int* rc){ 
  std::vector<std::wstring> response;
  *rc = 0;
  try {        
        *rc = verifyAPISession();
        if(*rc == 0){
          response = EncodingHelper::ReplaceOriginalTagsWithPlaceholders(std::move(src_data), std::move(trg_data), std::move(req_data) );
        }
    }
    //catch (const XMLException& toCatch) {
      catch(...){
        *rc = 400;
        //return( ERROR_NOT_READY );
    }
  return response;
}



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


void importMemoryProcess( void* pvData )
{
  PIMPORTMEMORYDATA pData = (PIMPORTMEMORYDATA)pvData;
  // call the OpenTM2 API function
  pData->szError[0] = 0;
  int iRC =(int)EqfImportMem( pData->mem, pData->szInFile, TMX_OPT | COMPLETE_IN_ONE_CALL_OPT, pData->szError);

  // handle any error
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    T5LOG(T5ERROR) <<"importMemoryProcess:: error = ", pData->szError;
  }

  // update memory status
  //pData->pMemoryServiceWorker->importDone( pData->szMemory, iRC, pData->szError );
  TMManager::GetInstance()->importDone( pData->mem,iRC, pData->szError );
 
  // cleanup
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( pData->szInFile );
  }
  delete( pData );

  return;
}



