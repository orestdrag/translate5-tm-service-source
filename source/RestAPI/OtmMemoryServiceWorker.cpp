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

#include "opentm2/core/utilities/PropertyWrapper.H"
#include "opentm2/core/utilities/FilesystemHelper.h"
#include <thread>
#include <utility>

#include <xercesc/util/PlatformUtils.hpp>
#include "../cmake/git_version.h"
#include "opentm2/core/utilities/ThreadingWrapper.h"
#include "opentm2/core/utilities/LanguageFactory.H"
#include "EqfMemoryPlugin.h"

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

  //instance->hSession = 0;
	return &_instance;
}

/*! \brief OtmMemoryServiceWorker constructor
*/
OtmMemoryServiceWorker::OtmMemoryServiceWorker()
{
}


/*! \brief Data area for the processing of the deleteMemory function
*/
typedef struct _DELETEMEMORYDATA
{
  char szMemory[260];
  wchar_t szError[512];
} DELETEMEMORYDATA, *PDELETEMEMORYDATA;



/*! \brief Data area for the processing of the createMemory function
*/
typedef struct _CREATEMEMORYDATA
{
  char szMemory[260];
  wchar_t szMemoryW[260]; // memory name in UTF-16 encoding
  char szIsoSourceLang[40];
  char szOtmSourceLang[40];
  wchar_t szError[512];
} CREATEMEMORYDATA, *PCREATEMEMORYDATA;


/*! \brief Data area for the processing of the importMemory function
*/
typedef struct _IMPORTMEMORYDATA
{
  HSESSION hSession;
  OtmMemoryServiceWorker *pMemoryServiceWorker;
  char szMemory[260];
  char szInFile[260];
  char szError[512];
  //ushort * pusImportPersent = nullptr;
  ImportStatusDetails* importDetails = nullptr;
  //OtmMemoryServiceWorker::std::shared_ptr<OPENEDMEMORY>  pMem = nullptr;
} IMPORTMEMORYDATA, *PIMPORTMEMORYDATA;




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


/*! \brief Data area for the processing of the lookupInMemory function (also used by searchMemory and updateMemory)
*/
typedef struct _LOOKUPINMEMORYDATA
{
  char szMemory[260];
  wchar_t szSource[2050];
  wchar_t szTarget[2050];
  char szIsoSourceLang[40];
  char szIsoTargetLang[40];
  char szOtmSourceLang[40];
  char szOtmTargetLang[40];
  int lSegmentNum;
  char szDocName[260];
  char szMarkup[128];
  wchar_t szContext[2050];
  wchar_t szAddInfo[2050];
  wchar_t szError[512];
  char szType[256];
  char szAuthor[80];
  char szDateTime[40];
  char szSearchMode[40];
  char szSearchPos[80];
  int iNumOfProposals;
  int iSearchTime;
  wchar_t szSearchString[2050];
} LOOKUPINMEMORYDATA, *PLOOKUPINMEMORYDATA;



int OtmMemoryServiceWorker::init(){
  return verifyAPISession();
}

  /*! \brief Verify OpenTM2 API session
    \returns 0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::verifyAPISession
(
)
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
int OtmMemoryServiceWorker::buildErrorReturn
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

int OtmMemoryServiceWorker::buildErrorReturn
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

int OtmMemoryServiceWorker::buildErrorReturn
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




/*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date and time
  \returns 0 
*/
int OtmMemoryServiceWorker::convertTimeToUTC( LONG lTime, char *pszDateTime )
{
  struct tm   *pTimeDate;            // time/date structure

  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

  pTimeDate = gmtime( (time_t *)&lTime );
  if ( pTimeDate->tm_isdst == 1 )
  {
    // correct summertime offset
    lTime -= 3600; 
    pTimeDate = gmtime( (time_t *)&lTime );
    
  }
  if ( (lTime != 0L) && pTimeDate )   // if gmtime was successful ...
  {
    sprintf( pszDateTime, "%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ", 
             pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
             pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
  }
  else
  {
    *pszDateTime = 0;
  } /* endif */

  return( 0 );
}

/*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
\param lTime long time value
\param pszDateTime buffer receiving the converted date and time
\returns 0
*/
int OtmMemoryServiceWorker::convertTimeToDateTimeString( LONG lTime, char *pszDateTime )
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


/*! \brief convert a UTC time value to a OpenTM2 long time value 
    \param pszDateTime buffer containing the UTC date and time
    \param plTime pointer to a long buffer receiving the converted time 
  \returns 0 
*/
int OtmMemoryServiceWorker::convertUTCTimeToLong( char *pszDateTime, PLONG plTime )
{
  *plTime = 0;

  // we currently support only dates in the form UTC format YYYYMMDDThhmmssZ
  if ( (pszDateTime != NULL) && (strlen(pszDateTime) == 16) )
  {
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    // split string into date/time parts
    BOOL fOK = getValue( pszDateTime, 4, &iYear );
    if ( fOK ) fOK = getValue( pszDateTime + 4, 2, &iMonth );
    if ( fOK ) fOK = getValue( pszDateTime + 6, 2, &iDay );
    if ( fOK ) fOK = getValue( pszDateTime + 9, 2, &iHour );
    if ( fOK ) fOK = getValue( pszDateTime + 11, 2, &iMin );
    if ( fOK ) fOK = getValue( pszDateTime + 13, 2, &iSec );

    if ( fOK )
    {
      struct tm timeStruct;
      memset( &timeStruct, 0, sizeof(timeStruct) );
      timeStruct.tm_hour = iHour;
      timeStruct.tm_min = iMin;
      timeStruct.tm_sec = iSec;
      timeStruct.tm_mday = iDay;
      timeStruct.tm_mon = iMonth - 1;
      timeStruct.tm_year = iYear - 1900;

      *plTime = mktime( &timeStruct );

      // as mktime is always using the local time zone, we have to adjust the time to UTC time
      time_t lLocalTime = 0;
      time_t lGMTime = 0;
      time( &lGMTime );
      lLocalTime = mktime( localtime( &lGMTime ) );
      lGMTime = mktime( gmtime( &lGMTime ) );
      LONG lDiff = lLocalTime - lGMTime;
      *plTime += lDiff;

      *plTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)

    } /* endif */
  } /* endif */
  return( 0 );
}

/*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
\param pszDateTime buffer containing the TMX date and time
\param plTime pointer to a long buffer receiving the converted time
\returns 0
*/
int OtmMemoryServiceWorker::convertDateTimeStringToLong( char *pszDateTime, PLONG plTime )
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

/*! \brief extract a numeric value from a string
    \param pszString string containing the numeric value
    \param iLen number of digits to be processed
    \param piResult pointer to a int value receiving the extracted value
  \returns TRUE if successful and FALSE in case of errors
*/
bool OtmMemoryServiceWorker::getValue( char *pszString, int iLen, int *piResult )
{
  bool fOK = true;
  char szNumber[10];
  char *pszNumber = szNumber;

  *piResult = 0;

  while ( iLen && fOK )
  {
    if ( isdigit(*pszString) )
    {
      *pszNumber++ = *pszString++;
      iLen--;
    }
    else
    {
      fOK = false;
    } /* endif */
  } /*endwhile */

  if ( fOK )
  {
    *pszNumber = '\0';
    *piResult = atoi( szNumber );
  } /* endif */

  return( fOK );
} 


int OtmMemoryServiceWorker::cloneTMLocaly
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = 0;
  START_WATCH
  
  // parse json data
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  std::string newName, name, value;
  while ( iRC == 0 )
  {
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 ){
      if( strcasecmp (name.c_str(), "newName") == 0 ){
         newName = value;          
      }else{
          T5LOG(T5WARNING) << "::JSON parsed unused data: name = " << name << "; value = " << value;
      }
    }
  }
  if(iRC == 2002){
    iRC = 0;
  }
  if(!iRC && newName.empty()){
    strOutputParms = "\'newName\' parameter was not provided or was empty";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }

  std::string srcTmdPath, srcMemPath, srcTmiPath, dstTmdPath, dstTmiPath, dstMemPath;
  char memDir[255];
  if(!iRC){
    iRC = properties_get_str(KEY_MEM_DIR, memDir, 254);
    if(iRC){
      strOutputParms = "can't read MEM_DIR path from properties";
      T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
      iRC = 500;
    }
  }
  if(!iRC){
    srcTmdPath = memDir + strMemory + ".TMD";
    srcTmiPath = memDir + strMemory + ".TMI";
    srcMemPath = memDir + strMemory + ".MEM";
    dstTmdPath = memDir + newName + ".TMD";
    dstTmiPath = memDir + newName + ".TMI";
    dstMemPath = memDir + newName + ".MEM";
  }

  // check mem if exists
  if(!iRC && FilesystemHelper::FileExists(srcMemPath.c_str()) == false){
    strOutputParms = "\'srcMemPath\' = " + srcMemPath + " not found";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }
  if(!iRC && FilesystemHelper::FileExists(srcTmdPath.c_str()) == false){
    strOutputParms = "\'srcTmdPath\' = " +  srcTmdPath + " not found";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }
  if(!iRC && FilesystemHelper::FileExists(srcTmiPath.c_str()) == false){
    strOutputParms = "\'srcTmiPath\' = " +  srcTmiPath + " not found";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }

  // check mem if is not in import state
  std::shared_ptr<OPENEDMEMORY>  pMem; 
  
  //LONG lHandle = 0;
  //BOOL fClose = false;
  //MEMORY_STATUS lastImportStatus = AVAILABLE_STATUS; // to restore in case we would break import before calling closemem
  //MEMORY_STATUS lastStatus = AVAILABLE_STATUS;

  if(!iRC){
    pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  
    
    if(pMem == nullptr){
    // tm is probably not opened, buf files presence was checked before, so it should be "AVAILABLE" status
    //  strOutputParms = "\'newName\' " + strMemory +" was not found in memory list";
    //  T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    //  iRC = 500;
    }else{
      // close the memory - if open
      if(pMem->eImportStatus == IMPORT_RUNNING_STATUS){
           strOutputParms = "src tm \'" + strMemory +"\' is in import status. Repeat request later.";
          T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
         iRC = 500;
      }else if ( pMem->eStatus == OPEN_STATUS )
      {
        if(pMem->lHandle){
          //EqfCloseMem( this->hSession, pMem->lHandle, 0 );
          //pMem->lHandle = 0;
        }
      }else if(pMem->eStatus != AVAILABLE_STATUS ){
         strOutputParms = "src tm \'" + strMemory +"\' is not available nor opened";
         T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
         iRC = 500;
      }
    }
  }
  
  // check if new name is available(is not occupied)
  if(!iRC && FilesystemHelper::FileExists(dstMemPath.c_str()) == true){
    strOutputParms = "\'dstMemPath\' = " +  dstMemPath + " already exists";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }
  if(!iRC && FilesystemHelper::FileExists(dstTmdPath.c_str()) == true){
    strOutputParms = "\'dstTmdPath\' = " +  dstTmdPath + " already exists";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }
  if(!iRC && FilesystemHelper::FileExists(dstTmiPath.c_str()) == true){
    strOutputParms = "\'dstTmiPath\' = " +  dstTmiPath + " already exists";
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }

  //flush filebuffers before clonning
  if(FilesystemHelper::FilebufferExists(srcTmdPath)){
    if(!iRC && (iRC = FilesystemHelper::FilesystemHelper::WriteBuffToFile(srcTmdPath))){
      strOutputParms = "Can't flush src filebuffer, iRC = " + toStr(iRC)  + "; \'srcTmdPath\' = " + srcTmdPath;
      T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
      iRC = 500;
    }
  }
  if(FilesystemHelper::FilebufferExists(srcTmiPath)){
    if(!iRC && (iRC = FilesystemHelper::FilesystemHelper::WriteBuffToFile(srcTmiPath))){
      strOutputParms = "Can't flush src filebuffer, iRC = " + toStr(iRC)  + "; \'srcTmiPath\' = " + srcTmiPath;
      T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
      iRC = 500;
    }
  }
  /*
  if(!iRC && (iRC = FilesystemHelper::FilesystemHelper::WriteBuffToFile(srcMemPath))){
    strOutputParms = "Can't flush src filebuffer, iRC = " + toStr(iRC)  + "; \'srcMemPath\' = " + srcMemPath;
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 500;
  }//*/

  // clone .mem .tmi and .tmd files 
  if(!iRC && (iRC = FilesystemHelper::CloneFile(srcMemPath, dstMemPath))){
    strOutputParms = "Can't clone file, iRC = " + toStr(iRC)  + "; \'srcMemPath\' = " + srcMemPath + "; \'dstMemPath\' = " +  dstMemPath;
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 501;
  }
  if(!iRC && (iRC = FilesystemHelper::CloneFile(srcTmdPath, dstTmdPath))){
    strOutputParms = "Can't clone file, iRC = " + toStr(iRC)  + "; \'srcTmdPath\' = " + srcTmdPath + "; \'dstTmdPath\' = " +  dstTmdPath;
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 501;
  }
  if(!iRC && (iRC = FilesystemHelper::CloneFile(srcTmiPath, dstTmiPath))){
    strOutputParms = "Can't clone file, iRC = " + toStr(iRC)  + "; \'srcTmiPath\' = " + srcTmiPath + "; \'dstTmiPath\' = " +  dstTmiPath;
    T5LOG(T5ERROR) << strOutputParms << "; for request for mem "<< strMemory <<"; with body = ", strInputParms ;
    iRC = 501;
  }

  //if there was error- delete files
  if(iRC == 501){
    if(FilesystemHelper::FileExists(dstTmiPath.c_str())) FilesystemHelper::DeleteFile(dstTmiPath.c_str());
    if(FilesystemHelper::FileExists(dstTmdPath.c_str())) FilesystemHelper::DeleteFile(dstTmdPath.c_str());
    if(FilesystemHelper::FileExists(dstMemPath.c_str())) FilesystemHelper::DeleteFile(dstMemPath.c_str());
    iRC = 500;
  }
  if(!iRC){
    EqfMemoryPlugin::GetInstance()->addMemoryToList(newName.c_str());
  }
  if( pMem != nullptr )
  {
    //fClose = true;
    //lHandle =          pMem->lHandle; 
    //lastStatus =       pMem->eStatus;
    //lastImportStatus = pMem->eImportStatus;

    //pMem->lHandle = 0;
    //pMem->eStatus = AVAILABLE_STATUS;
    //pMem->eImportStatus = AVAILABLE_STATUS; //IMPORT_RUNNING_STATUS;
    //pMem->dImportProcess = 0;
  }

  if(iRC == 0 ){
    strOutputParms = newName + " was cloned successfully";
    iRC = 200;
  }

  strOutputParms = "{\n\t\"msg\": \"" + strOutputParms + "\",\n\t\"time\": \"" + watch.print()+ "\n}"; 


  STOP_WATCH
  //PRINT_WATCH

  return iRC;
}
/*! \brief Import a memory from a TMX file
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns 0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::import
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  //EncodingHelper::convertUTF8ToASCII( strMemory );
  T5LOG( T5INFO) << "+POST " << strMemory << "/import\n";
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    return( NOT_FOUND );
  }

  // find the memory to our memory list
  std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  LONG lHandle = 0;
  BOOL fClose = false;
  MEMORY_STATUS lastImportStatus = AVAILABLE_STATUS; // to restore in case we would break import before calling closemem
  MEMORY_STATUS lastStatus = AVAILABLE_STATUS;
  if ( pMem != nullptr )
  {
    // close the memory - when open
    if ( pMem->eStatus == OPEN_STATUS )
    {
      fClose = true;
      lHandle =          pMem->lHandle; //
      lastStatus =       pMem->eStatus;
      lastImportStatus = pMem->eImportStatus;

      pMem->lHandle = 0;
      pMem->eStatus = AVAILABLE_STATUS;
      pMem->eImportStatus = IMPORT_RUNNING_STATUS;
      //pMem->dImportProcess = 0;
    }
  }
  else
  {
    size_t requiredMemory = 0;
    {
      char memFolder[260];
      properties_get_str(KEY_MEM_DIR, memFolder, 260);
      std::string path;

      path = memFolder + strMemory;
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMI"));
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMD"));
      requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".MEM"));

      //requiredMemory += FilesystemHelper::GetFileSize( szTempFile ) * 2;
      requiredMemory += strInputParms.size() * 2;
    }
    //TODO: add to requiredMemory value that would present changes in mem files sizes after import done 

    // cleanup the memory list (close memories not used for a longer time)
    size_t memLeftAfterOpening = TMManager::GetInstance()->cleanupMemoryList(requiredMemory);
    if(VLOG_IS_ON(1)){
      T5LOG( T5INFO) << ":: memory: " << strMemory << "; required RAM:" 
          << requiredMemory << "; RAM left after opening mem: " << memLeftAfterOpening;
    }
    //requiredMemory = 0;
    // find a free slot in the memory list
    std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->getFreeSlotPointer(requiredMemory);

    // handle "too many open memories" condition
    if ( pMem == nullptr )
    {
      wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Error: too many open translation memory databases";
      buildErrorReturn( iRC, errMsg, strOutputParms );
      return( INTERNAL_SERVER_ERROR );
    } /* endif */

    pMem->lHandle = 0;
    pMem->eStatus = AVAILABLE_STATUS;
    pMem->eImportStatus = IMPORT_RUNNING_STATUS;
    //pMem->dImportProcess = 0;
    strcpy( pMem->szName, strMemory.c_str() );
  }
  T5LOG( T5DEBUG) <<  "status for " << strMemory << " was changed to import";
  // extract TMX data
  std::string strTmxData;
  int loggingThreshold = -1; //0-develop(show all logs), 1-debug+, 2-info+, 3-warnings+, 4-errors+, 5-fatals only
  
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  if ( parseHandle == NULL )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing or incorrect JSON data in request body";
    buildErrorReturn( iRC, errMsg, strOutputParms );
        
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;

    return( BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( iRC == 0 )
  {
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      //T5LOG( T5INFO) << "parsed json name = ", name.c_str(), "; value = ", value.c_str());
      if ( strcasecmp( name.c_str(), "tmxData" ) == 0 )
      {
        strTmxData = value;
        //break;
      }else if(strcasecmp(name.c_str(), "loggingThreshold") == 0){
        loggingThreshold = std::stoi(value);
        T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::import::set new threshold for logging" << loggingThreshold;
        T5Logger::GetInstance()->SetLogLevel(loggingThreshold);        
      }else{
        T5LOG( T5WARNING) << "JSON parsed unexpected name, " << name;
      }
    }else if(iRC != 2002){// iRC != INFO_ENDOFPARAMETERLISTREACHED
        T5LOG(T5ERROR) << "failed to parse JSON \"" << strInputParms <<"\", iRC = " << iRC;
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );
  if ( strTmxData.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing TMX data";
    buildErrorReturn( iRC, errMsg, strOutputParms );
  
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;

    return( BAD_REQUEST );
  } /* endif */

  // setup temp file name for TMX file 
  char szTempFile[PATH_MAX];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0  )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Could not create file name for temporary data";
    buildErrorReturn( -1, errMsg, strOutputParms );
    
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;
    return( INTERNAL_SERVER_ERROR );
  }

  T5LOG( T5INFO) << "OtmMemoryServiceWorker::import::+   Temp TMX File is " << szTempFile;

  // decode TMX data and write it to temp file
  std::string strError;
  iRC = decodeBase64ToFile( strTmxData.c_str(), szTempFile, strError ) ;
  if ( iRC != 0 )
  {
    strError = "OtmMemoryServiceWorker::import::" + strError;
    buildErrorReturn( iRC, (char *)strError.c_str(), strOutputParms );
    
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;
    return( INTERNAL_SERVER_ERROR );
  }
  
  //success in parsing request data-> close mem if needed
  if(lHandle && fClose){
        EqfCloseMem( this->hSession, lHandle, 0 );
  }

  // start the import background process
  PIMPORTMEMORYDATA pData = new( IMPORTMEMORYDATA );
  strcpy( pData->szInFile, szTempFile );
  strcpy( pData->szMemory, strMemory.c_str() );

  if(pMem->importDetails == nullptr){
    pMem->importDetails = new ImportStatusDetails;
  }
  
  pData->importDetails = pMem->importDetails;
  pData->importDetails->reset();
  pData->hSession = hSession;
  pData->pMemoryServiceWorker = this;

  //importMemoryProcess(pData);//to do in same thread
  std::thread worker_thread(importMemoryProcess, pData);
  worker_thread.detach();

  return( CREATED );
}

/*! \brief Create a new memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns 0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::createMemory
(
  std::string strInputParms,
  std::string &strOutputParms
)
{
  JSONFactory *factory = JSONFactory::getInstance();
  CreateMemRequestData requestData(strInputParms);  
  requestData._rc_ = verifyAPISession();
  if ( requestData._rc_ != 0 )
  {
    buildErrorReturn( requestData._rc_, this->szLastError, strOutputParms );
    return( requestData._rc_ );
  } /* endif */

  if ( requestData.strMemB64EncodedData.empty() ) // create empty tm
  {
    T5LOG(T5DEBUG) << "( MemName = "<<requestData.strMemName <<", pszDescription = "<<requestData.strMemDescription<<", pszSourceLanguage = "<<requestData.strSrcLang<<" )";

    // Check memory name syntax
    if ( requestData._rc_ == NO_ERROR )
    { 
      if ( !FilesystemHelper::checkFileName( requestData.strMemName ))
      {
        T5LOG(T5ERROR) <<   "::ERROR_INV_LONGNAME::" << requestData.strMemName;
        requestData._rc_ = ERROR_MEM_NAME_INVALID;
      } /* endif */
    } /* endif */

    // check if TM exists already
    if ( requestData._rc_ == NO_ERROR )
    {
      int logLevel = T5Logger::GetInstance()->suppressLogging();
      if ( TMManager::GetInstance()->exists( NULL, (PSZ)requestData.strMemName.c_str() ) )
      {
        T5Logger::GetInstance()->desuppressLogging(logLevel);
        T5LOG(T5ERROR) <<  "::ERROR_MEM_NAME_EXISTS:: TM with this name already exists: " << requestData.strMemName;
        requestData._rc_ = ERROR_MEM_NAME_EXISTS;
      }
      T5Logger::GetInstance()->desuppressLogging(logLevel);
    } /* endif */


    // check if source language is valid
    if ( requestData._rc_ == NO_ERROR )
    {
      SHORT sID = 0;    
      if ( MorphGetLanguageID( (PSZ)requestData.strSrcLang.c_str(), &sID ) != MORPH_OK )
      {
        requestData._rc_ = ERROR_PROPERTY_LANG_DATA;
        T5LOG(T5ERROR) << "MorhpGetLanguageID returned error, usRC = ERROR_PROPERTY_LANG_DATA;";
      } /* endif */
    } /* endif */

    // create memory database
    if (requestData._rc_==NO_ERROR){
      EqfMemoryPlugin::GetInstance()->createMemory( requestData);
    }     
  }
  else
  {
     T5LOG( T5INFO) << "int OtmMemoryServiceWorker::createMemory():: strData is not empty -> setup temp file name for ZIP package file ";
    // setup temp file name for ZIP package file 
    char szTempFile[PATH_MAX];
    requestData._rc_ = buildTempFileName( szTempFile );
    if ( requestData._rc_ != 0 )
    {
      wchar_t errMsg[] = L"Could not create file name for temporary data";
      buildErrorReturn( -1, errMsg, strOutputParms );
      return( INTERNAL_SERVER_ERROR );
    }

    T5LOG( T5INFO) << "+   Temp binary file is " << szTempFile ;

    // decode binary data and write it to temp file
    std::string strError;
    requestData._rc_ = decodeBase64ToFile( requestData.strMemB64EncodedData.c_str(), szTempFile, strError );
    if ( requestData._rc_ != 0 )
    {
      buildErrorReturn( requestData._rc_, (char *)strError.c_str(), strOutputParms );
      return( INTERNAL_SERVER_ERROR );
    }
    requestData._rc_ = TMManager::GetInstance()->APIImportMemInInternalFormat( requestData.strMemName.c_str(), szTempFile, 0 );
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
        DeleteFile( szTempFile );
    }
  }

  if(requestData._rc_ == ERROR_MEM_NAME_EXISTS){
    strOutputParms = "{\"" + requestData.strMemName + "\": \"ERROR_MEM_NAME_EXISTS\" }" ;
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::createMemo()::usRC = " << requestData._rc_ << " iRC = ERROR_MEM_NAME_EXISTS";
    return requestData._rc_;
  }else if ( requestData._rc_ != 0 )
  {
    
    char szError[PATH_MAX];
    unsigned short usRC = 0;
    EqfGetLastError( this->hSession, &usRC, szError, sizeof( szError ) );
    //buildErrorReturn( iRC, szError, strOutputParms );

    strOutputParms = "{\"" + requestData.strMemName + "\": \"" ;
    switch ( usRC )
    {
      case ERROR_MEM_NAME_INVALID:
        requestData._rest_rc_ = CONFLICT;
        strOutputParms += "CONFLICT";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " iRC = CONFLICT";
        break;
      case TMT_MANDCMDLINE:
      case ERROR_NO_SOURCELANG:
      case ERROR_PROPERTY_LANG_DATA:
        requestData._rest_rc_ = BAD_REQUEST;
        strOutputParms += "BAD_REQUEST";
        T5LOG(T5ERROR) << "::usRC = " <<  usRC << " iRC = BAD_REQUEST";
        break;
      default:
        requestData._rest_rc_ = INTERNAL_SERVER_ERROR;
        strOutputParms += "INTERNAL_SERVER_ERROR";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " iRC = INTERNAL_SERVER_ERROR";
        break;
    }
    strOutputParms +="\"}";
    return( requestData._rest_rc_ );
  }else{
    factory->startJSON( strOutputParms );
    factory->addParmToJSON( strOutputParms, "name", requestData.strMemName );
    factory->terminateJSON( strOutputParms );
    return( OK );
  }
}

/* write a single proposal to a JSON string
\param strJSON JSON stirng receiving the proposal data
\param pProp pointer to a MEMPROPOSAL containing the proposal
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
int OtmMemoryServiceWorker::addProposalToJSONString
(
  std::wstring &strJSON,
  PMEMPROPOSAL pProp,
  void *pvData
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  PLOOKUPINMEMORYDATA pData = (PLOOKUPINMEMORYDATA)pvData;

  pJsonFactory->addElementStartToJSONW( strJSON );

  pJsonFactory->addParmToJSONW( strJSON, L"source", pProp->szSource );
  pJsonFactory->addParmToJSONW( strJSON, L"target", pProp->szTarget );
  pJsonFactory->addParmToJSONW( strJSON, L"segmentNumber", pProp->lSegmentNum );
  pJsonFactory->addParmToJSONW( strJSON, L"id", EncodingHelper::convertToUTF16(pProp->szId).c_str());
  pJsonFactory->addParmToJSONW( strJSON, L"documentName", EncodingHelper::convertToUTF16(pProp->szDocName).c_str() );
  pJsonFactory->addParmToJSONW( strJSON, L"documentShortName", EncodingHelper::convertToUTF16(pProp->szDocShortName).c_str() );
  EqfGetIsoLang( this->hSession, pProp->szSourceLanguage, pData->szIsoSourceLang );
  pJsonFactory->addParmToJSONW( strJSON, L"sourceLang", EncodingHelper::convertToUTF16( pData->szIsoSourceLang ).c_str() );
  EqfGetIsoLang( this->hSession, pProp->szTargetLanguage, pData->szIsoSourceLang );
  pJsonFactory->addParmToJSONW( strJSON, L"targetLang", EncodingHelper::convertToUTF16( pData->szIsoSourceLang ).c_str() );

  switch ( pProp->eType )
  {
    case GLOBMEMORY_PROPTYPE: wcscpy( pData->szSource, L"GlobalMemory" ); break;
    case GLOBMEMORYSTAR_PROPTYPE: wcscpy( pData->szSource, L"GlobalMemoryStar" ); break;
    case MACHINE_PROPTYPE: wcscpy( pData->szSource, L"MachineTranslation" ); break;
    case MANUAL_PROPTYPE: wcscpy( pData->szSource, L"Manual" ); break;
    default: wcscpy( pData->szSource, L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"type", pData->szSource );

  switch ( pProp->eMatch )
  {
    case EXACT_MATCHTYPE: wcscpy( pData->szSource, L"Exact" ); break;
    case EXACTEXACT_MATCHTYPE: wcscpy( pData->szSource, L"ExactExact" ); break;
    case EXACTSAMEDOC_MATCHTYPE: wcscpy( pData->szSource, L"ExactSameDoc" ); break;
    case FUZZY_MATCHTYPE: wcscpy( pData->szSource, L"Fuzzy" ); break;
    case REPLACE_MATCHTYPE: wcscpy( pData->szSource, L"Replace" ); break;
    default: wcscpy( pData->szSource, L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"matchType", pData->szSource );

  MultiByteToWideChar( CP_OEMCP, 0, pProp->szTargetAuthor, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"author", pData->szSource );

  convertTimeToUTC( pProp->lTargetTime, pData->szDateTime );
  MultiByteToWideChar( CP_OEMCP, 0, pData->szDateTime, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"timestamp", pData->szSource );

  pJsonFactory->addParmToJSONW( strJSON, L"matchRate", pProp->iFuzziness );
  pJsonFactory->addParmToJSONW( strJSON, L"fuzzyWords", pProp->iWords );
  pJsonFactory->addParmToJSONW( strJSON, L"fuzzyDiffs", pProp->iDiffs );

  MultiByteToWideChar( CP_OEMCP, 0, pProp->szMarkup, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"markupTable", pData->szSource );

  pJsonFactory->addParmToJSONW( strJSON, L"context", pProp->szContext );

  pJsonFactory->addParmToJSONW( strJSON, L"additionalInfo", pProp->szAddInfo );

  pJsonFactory->addElementEndToJSONW( strJSON );

  return( 0 );
}


/* write proposals to a JSON string 
\param strJSON JSON stirng receiving the proposal data
\param pProposals pointer to a MEMPROPOSAL array containing the proposals
\param iNumOfProposals number of proposals to write to JSON string
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
int OtmMemoryServiceWorker::addProposalsToJSONString
(
  std::wstring &strJSON,
  PMEMPROPOSAL pProposals,
  int iNumOfProposals,
  void *pvData
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  PLOOKUPINMEMORYDATA pData = (PLOOKUPINMEMORYDATA)pvData;
  
  pJsonFactory->addArrayStartToJSONW( strJSON );
  for ( int i = 0; i < iNumOfProposals; i++ )
  {
    PMEMPROPOSAL pProp = pProposals + i;
    addProposalToJSONString( strJSON, pProp, pvData );
  } /* endfor */
  pJsonFactory->addArrayEndToJSONW( strJSON  );
  
  return( 0 );
}

template<typename T>
void AddToJson(std::stringstream& ss, const char* key, T value, bool fAddSeparator){
  ss << "\"" << key << "\" : ";

  //if (std::is_same<T, int>::value)
  if(std::is_arithmetic<T>::value) // if it's number - skip quotes
  {
    ss << value;
  }else{
    ss << "\"" << value << "\"";
  }
  
  if(fAddSeparator)
    ss << ",";

  ss << "\n";
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

int OtmMemoryServiceWorker::resourcesInfo(std::string& strOutput, ProxygenService::ProxygenStats& stats){
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutput );
    return( BAD_REQUEST );
  } /* endif */
  std::stringstream ssOutput;

  //open json
  ssOutput << "{\n";

  //the list of currently opened TMs (those loaded into the memory)
  //for every loaded TM: The memory it allocates
  { 

    auto fbs = FilesystemHelper::getFileBufferInstance();
    size_t total = 0, fSize;
    std::string fName;

    ssOutput << "\"filebuffers\": [\n";
    for (auto it = fbs->cbegin(); it != fbs->cend(); )
    {
        //fSize = it->second.data.size();
        fSize = it->second.data.capacity();
        fName = it->first;
        //fName = FilesystemHelper::parseFilename(it->first);
        
        ssOutput << "{ ";
        AddToJson(ssOutput, "name", fName, true );
        AddToJson(ssOutput, "size", fSize, false );
        ssOutput << " }";

        total += fSize;
        it++;
        if( it != fbs->cend()){
            ssOutput << ",\n";
        }
    }   

    ssOutput << "\n ],\n"; 
    AddToJson(ssOutput, "totalOccupiedByFilebuffersRAM", total, true );
  }

  //in addition the memory that is used by the service by other means
  // it's better to use system calls

  //the currently configured max usable memory
  int availableRam = 0, threshold = 0, workerThreads = 0, timeout = 0;
  char buff[255];
  //properties_get_str(KEY_OTM_DIR, szOtmDirPath);
  properties_get_int(KEY_ALLOWED_RAM, availableRam);// saving in megabytes to avoid int overflow
  properties_get_int(KEY_TRIPLES_THRESHOLD, threshold);
  properties_get_int(KEY_NUM_OF_THREADS, workerThreads);
  properties_get_int(KEY_TIMEOUT_SETTINGS, timeout);
  properties_get_str(KEY_RUN_DATE, buff,254);
  double vm_usage, resident_set;
  mem_usage(vm_usage, resident_set); 

  AddToJson(ssOutput, "Run date", buff, true );
  AddToJson(ssOutput, "Build date", buildDate, true );
  AddToJson(ssOutput, "Git commit info", gitHash, true );
  AddToJson(ssOutput, "Version", appVersion, true );
  AddToJson(ssOutput, "Worker threads", workerThreads, true );
  AddToJson(ssOutput, "Timeout(ms)", timeout, true );
  
  AddToJson(ssOutput, "Resident set", resident_set, true );
  AddToJson(ssOutput, "Virtual memory usage", vm_usage, true );
  {
    ssOutput << "\"Requests\": {\n";
    AddToJson(ssOutput, "RequestCount", stats.getRequestCount(), true );
    AddToJson(ssOutput, "CreateMemRequestCount", stats.getCreateMemRequestCount(), true );
    AddToJson(ssOutput, "DeleteMemRequestCount", stats.getDeleteMemRequestCount(), true );
    AddToJson(ssOutput, "ImportMemRequestCount", stats.getImportMemRequestCount(), true );
    AddToJson(ssOutput, "ExportMemRequestCount", stats.getExportMemRequestCount(), true );
    AddToJson(ssOutput, "CloneTmLocalyRequestCount", stats.getCloneLocalyCount(), true);
    AddToJson(ssOutput, "ReorganizeRequestCount", stats.getReorganizeRequestCount(), true);
    AddToJson(ssOutput, "StatusMemRequestCount", stats.getStatusMemRequestCount(), true );
    AddToJson(ssOutput, "FuzzyRequestCount", stats.getFuzzyRequestCount(), true );
    AddToJson(ssOutput, "ConcordanceRequestCount", stats.getConcordanceRequestCount(), true );
    AddToJson(ssOutput, "UpdateEntryRequestCount", stats.getUpdateEntryRequestCount(), true );
    AddToJson(ssOutput, "DeleteEntryRequestCount", stats.getDeleteEntryRequestCount(), true );
    AddToJson(ssOutput, "SaveAllTmsRequestCount", stats.getSaveAllTmsRequestCount(), true );
    AddToJson(ssOutput, "ListOfMemoriesRequestCount", stats.getListOfMemoriesRequestCount(), true );
    AddToJson(ssOutput, "ResourcesRequestCount", stats.getResourcesRequestCount(), true);
    AddToJson(ssOutput, "OtherRequestCount", stats.getOtherRequestCount(), true );
    AddToJson(ssOutput, "UnrecognizedRequestsCount", stats.getUnrecognizedRequestCount(), false);
    ssOutput << "\n },\n"; 
  }

  
  //AddToJson(ssOutput, "Log level(internal)", GetLogLevel(), true );
  //AddToJson(ssOutput, "CPU used by process", getCurrentCPUUsageByProcess(), true);
  //AddToJson(ssOutput, "VirtualMem used by process(in KB)", getVirtualMemUsageKBValue(), true);
  
  AddToJson(ssOutput, "RAM limit(MB)", availableRam, false );

  //close json
  ssOutput << "\n}";

  strOutput = ssOutput.str();
  iRC = 200;
  return iRC;
}


std::string OtmMemoryServiceWorker::tagReplacement(std::string strInData, int& rc){
  rc = 0;
  std::string strSrcData, strTrgData, strReqData;
 
  strSrcData.reserve( strInData.size() + 1 );
  strTrgData.reserve( strInData.size() + 1 );
  strReqData.reserve( strInData.size() + 1 );   
  std::wstring wstr;

  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInData, &rc );
  if ( parseHandle == NULL )
  {
    wchar_t errMsg[] = L"Missing or incorrect JSON data in request body";
    wstr = errMsg;
    //buildErrorReturn( iRC, errMsg, strOutputParms );
    rc = BAD_REQUEST;
    return "";
  }else{ 
    std::string name;
    std::string value;
    while ( rc == 0 )
    {
      rc = factory->parseJSONGetNext( parseHandle, name, value );
      if ( rc == 0 )
      {      
        T5LOG( T5DEBUG) << "::JSON parsed src = " << name << "; size = " << strSrcData.size();
        if ( strcasecmp( name.c_str(), "src" ) == 0 )
        {
          strSrcData = value;
        }else if( strcasecmp( name.c_str(), "trg" ) == 0 ){
          strTrgData = value;
        }else if( strcasecmp (name.c_str(), "req") == 0 ){
          strReqData = value;          
        }else{
          T5LOG( T5WARNING) <<  "::JSON parsed unused data: name = " <<  name << "; value = " << value;
        }
      }
    } /* endwhile */
    factory->parseJSONStop( parseHandle );
  }
  
  auto result =  replaceString(  EncodingHelper::convertToUTF16(strSrcData.c_str()).c_str(), 
                                              EncodingHelper::convertToUTF16(strTrgData.c_str()).c_str(),
                                              EncodingHelper::convertToUTF16(strReqData.c_str()).c_str(), &rc);

  
  wstr = L"{\n ";
  std::wstring segmentLocations[] = {L"source", L"target", L"request"};
  for(int index = 0; index < result.size(); index++){
    wstr += L"\'" + segmentLocations[index] + L"\' :\'" + result[index] + L"\',\n ";
  }
  wstr += L"\n}";
  std::string strResponseBody =  EncodingHelper::convertToUTF8(wstr);
  
  return strResponseBody;
}

/*! \brief Search for matching proposals
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::search
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

    // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strInputParms.c_str() );
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );
  JSONFactory *factory = JSONFactory::getInstance();
  int loggingThreshold = -1;
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSource ), sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) },
                                                   { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( pData->lSegmentNum ), 0 },
                                                   { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDocName ), sizeof( pData->szDocName ) },
                                                   { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
                                                   { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },
                                                   { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMarkup ), sizeof( pData->szMarkup ) },
                                                   { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szContext ), sizeof( pData->szContext ) / sizeof( pData->szContext[0] ) },
                                                   { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
                                                   { L"loggingThreshold", JSONFactory::INT_PARM_TYPE,        &loggingThreshold, 0 },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParmsW, parseControl );
  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    wchar_t errMsg[] = L"Error: Parsing of input parameters failed";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  
  if ( pData->szMarkup[0] == 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::search::No markup requested -> using OTMXUXLF";
    // use default markup table if none given
    strcpy( pData->szMarkup, "OTMXUXLF" );
  } /* end */
  
  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Too many proposals requested, the maximum value is 20";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  
  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::search::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold);
  }

  // get the handle of the memory 
  long lHandle = 0;
  int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemory, &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // prepare the memory lookup
  PMEMPROPOSAL pSearchKey = new (MEMPROPOSAL);
  memset( pSearchKey, 0, sizeof( *pSearchKey ) );
  wcscpy( pSearchKey->szSource, pData->szSource );
  strcpy( pSearchKey->szDocName, pData->szDocName );
  pSearchKey->lSegmentNum = pData->lSegmentNum;
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pSearchKey->szSourceLanguage, &pSearchKey->fIsoSourceLangIsPrefered );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pSearchKey->szTargetLanguage, &pSearchKey->fIsoTargetLangIsPrefered );
  strcpy( pSearchKey->szMarkup, pData->szMarkup );
  wcscpy( pSearchKey->szContext, pData->szContext );

  std::vector<std::string> sourceLangs;
  //if(pSearchKey->fIsoSourceLangIsPrefered){
  //  sourceLangs = GetListOfLanguagesFromFamily(pData->szIsoSourceLang);
  //}
  
  //if(sourceLangs.empty()){
    sourceLangs.push_back(pSearchKey->szSourceLanguage);
  //}

  if ( pData->iNumOfProposals == 0 )
  {
    //pData->iNumOfProposals = std::min( 5 * (int)sourceLangs.size(), 20);
    pData->iNumOfProposals = 5;
  }

  PMEMPROPOSAL pFoundProposals = new( MEMPROPOSAL[pData->iNumOfProposals] );
  memset( pFoundProposals, 0, sizeof( MEMPROPOSAL ) * pData->iNumOfProposals );
  // do the lookup and handle the results
  int ProposalSpacesLeft = pData->iNumOfProposals;
  int iNumOfProposals;
  int i = 0;
  //for(int i = 0; i < sourceLangs.size() && iRC == 0; i++){
    iNumOfProposals = ProposalSpacesLeft;
    strcpy(pSearchKey->szSourceLanguage, sourceLangs[i].c_str());
    int j = pData->iNumOfProposals - ProposalSpacesLeft;
    iRC = EqfQueryMem( this->hSession, lHandle, pSearchKey, &iNumOfProposals, &pFoundProposals[j], GET_EXACT );
    ProposalSpacesLeft -= iNumOfProposals;
    if(ProposalSpacesLeft <= 0 ){
      T5LOG( T5WARNING) <<  ":: not all list of lang was checked before allocated proposals space reached end, allocated " << pData->iNumOfProposals << 
        ", proposal spaces, checked " << i << " of " << sourceLangs.size() << " languages ";
      //break;
    }
    if(iRC != 0){
      T5LOG( T5WARNING) << ":: fuzzy search of mem returned error, rc = " << iRC << "; sourceLange = "<< sourceLangs[i] ;
    }
  //}
  iNumOfProposals = pData->iNumOfProposals - ProposalSpacesLeft;
  
  if ( iRC == 0 )
  {
    std::wstring strOutputParmsW;
    factory->startJSONW( strOutputParmsW );
    factory->addParmToJSONW( strOutputParmsW, L"ReturnValue", iRC );
    factory->addParmToJSONW( strOutputParmsW, L"ErrorMsg", L"" );
    factory->addParmToJSONW( strOutputParmsW, L"NumOfFoundProposals", iNumOfProposals );
    if ( iNumOfProposals > 0 )
    {
      factory->addNameToJSONW( strOutputParmsW, L"results" );
      addProposalsToJSONString( strOutputParmsW, pFoundProposals, iNumOfProposals, (void *)pData );
    } /* endif */

    factory->terminateJSONW( strOutputParmsW );
    strOutputParms = EncodingHelper::convertToUTF8( strOutputParmsW );
    iRC = OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = INTERNAL_SERVER_ERROR;
  } /* endif */

  if ( pSearchKey ) delete pSearchKey;
  if ( pFoundProposals ) delete[] pFoundProposals;
  delete pData;

  return( iRC );
}


/*! \brief Do a concordance search for the given search string
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::concordanceSearch
(
  std::string  strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strInputParms.c_str() );
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );
  JSONFactory *factory = JSONFactory::getInstance();
  int loggingThreshold = -1;
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"searchString",   JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSearchString ), sizeof( pData->szSearchString ) / sizeof( pData->szSearchString[0] ) },
  { L"searchType",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szSearchMode ), sizeof( pData->szSearchMode ) },
  { L"searchPosition", JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szSearchPos ), sizeof( pData->szSearchPos ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },
  { L"numResults",     JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
  { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( pData->iNumOfProposals ), 0 },
  { L"msSearchAfterNumResults", JSONFactory::INT_PARM_TYPE, &( pData->iSearchTime ), 0 },
  { L"loggingThreshold", JSONFactory::INT_PARM_TYPE,        &loggingThreshold, 0 },
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = factory->parseJSON( strInputParmsW, parseControl );
  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Error: Parsing of input parameters failed";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->szSearchString[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Error: Missing search string";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Error: Too many proposals requested, the maximum value is 20";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals == 0 )
  {
    pData->iNumOfProposals = 5;
  }

  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::concordanceSearch::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold);
  }
    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemory, &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != OK )
  {
    std::wstring wstr = L"OtmMemoryServiceWorker::concordanceSearch::";
    wstr += pData->szError;
    buildErrorReturn( iRC, (wchar_t*)wstr.c_str(), strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // do the search and handle the results
  LONG lOptions = 0;
  if ( strcasecmp( pData->szSearchMode, "Source" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT;
  }
  else if ( strcasecmp( pData->szSearchMode, "Target" ) == 0 )
  {
    lOptions |= SEARCH_IN_TARGET_OPT;
  }
  else if ( strcasecmp( pData->szSearchMode, "SourceAndTarget" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT | SEARCH_IN_TARGET_OPT;
  } /* endif */
  lOptions |= SEARCH_CASEINSENSITIVE_OPT;


  {
    bool fOk = false;
    if( strlen( pData->szIsoSourceLang) ){
      LanguageFactory::LANGUAGEINFO srcLangInfo;
      fOk = LanguageFactory::getInstance()->getLanguageInfo( pData->szIsoSourceLang, &srcLangInfo );
      if(fOk){
        if(srcLangInfo.fisPreferred){
          lOptions |= SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT;
        }else{
          lOptions |= SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT;
        }
      }else{
        T5LOG( T5WARNING) << ":: src lang could not be found: " << pData->szIsoSourceLang;
      }
    }
    if( strlen( pData->szIsoTargetLang) ){
      LanguageFactory::LANGUAGEINFO trgLangInfo;
      fOk = LanguageFactory::getInstance()->getLanguageInfo( pData->szIsoTargetLang, &trgLangInfo );
      if(fOk){
        if(trgLangInfo.fisPreferred){
          lOptions |= SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT;
        }else{
          lOptions |= SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT;
        }
      }else{
        T5LOG( T5WARNING) << ":: target lang could not be found: " << pData->szIsoTargetLang;
      }
    }
  }


  PMEMPROPOSAL pProposal = new( MEMPROPOSAL );
  std::wstring strProposals;
  
  // loop until end reached or enough proposals have been found
  int iFoundProposals = 0;
  int iActualSearchTime = 0; // for the first call run until end of TM or one proposal has been found
  do
  {
    memset( pProposal, 0, sizeof( *pProposal ) );
    iRC = EqfSearchMem( this->hSession, lHandle, pData->szSearchString, pData->szIsoSourceLang, pData->szIsoTargetLang,
      pData->szSearchPos, pProposal, iActualSearchTime, lOptions );
    iActualSearchTime = pData->iSearchTime;
    if ( iRC == 0 )
    {
      addProposalToJSONString( strProposals, pProposal, (void *)pData );
      iFoundProposals++;
    }
  } while ( ( iRC == 0 ) && ( iFoundProposals < pData->iNumOfProposals ) );

  if ( iFoundProposals || (iRC == ENDREACHED_RC) || (iRC == TIMEOUT_RC) )
  {
    std::wstring strOutputParmsW;
    factory->startJSONW( strOutputParmsW );
    
    //TODO: REPLACE LATER WITH just adding RC 
    if(iRC && iRC!=ENDREACHED_RC && iRC!=TIMEOUT_RC){
      factory->addParmToJSONW( strOutputParmsW, L"ReturnValue", iRC );
    }else{
      factory->addParmToJSONW( strOutputParmsW, L"ReturnValue", 0 );
    }
    if ( iRC == ENDREACHED_RC )
    {
      factory->addParmToJSONW( strOutputParmsW, L"NewSearchPosition" );
    }
    else
    {      
      std::wstring searchPos = EncodingHelper::convertToUTF16(pData->szSearchPos);
      //const char*  searchSubPos = &(pData->szSearchPos[searchPos.size()+1]);
      //searchPos += ';' + searchSubPos;
      factory->addParmToJSONW( strOutputParmsW, L"NewSearchPosition", searchPos );
    }
    if ( iFoundProposals > 0 )
    {
      factory->addNameToJSONW( strOutputParmsW, L"results" );
      factory->addArrayStartToJSONW( strOutputParmsW );
      strOutputParmsW.append( strProposals );
      factory->addArrayEndToJSONW( strOutputParmsW );
    } /* endif */

    factory->addParmToJSONW( strOutputParmsW, L"ErrorMsg", L"\0");
    factory->terminateJSONW( strOutputParmsW );
    strOutputParms = EncodingHelper::convertToUTF8( strOutputParmsW );
    iRC = OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = INTERNAL_SERVER_ERROR;
  } /* endif */

  //if ( pFoundProposals ) delete pFoundProposals;
  if ( pProposal ) delete pProposal;
  delete pData;

  return( iRC );
}




/*! \brief shutdown the service
  \returns http return code
  */
  int OtmMemoryServiceWorker::shutdownService(int sig){    
    this->fServiceIsRunning = true;
    std::thread([this, sig]() 
        {
           sleep(3);
           int j= 3;
           while(int i = TMManager::GetInstance()->GetMemImportInProcess() != -1){
            if( ++j % 15 == 0){
              T5LOG(T5WARNING) << "SHUTDOWN:: memory still in import..waiting 15 sec more...  shutdown request was = "<< j* 15;
            }
            T5LOG(T5DEBUG) << "SHUTDOWN:: memory still in import..waiting 1 sec more...  i = "<< i; 
            
            sleep(1);
           }

           this->fServiceIsRunning = false;
           if(sig != SHUTDOWN_CALLED_FROM_MAIN){
            exit(sig);
           }
        }).detach();
    return 200;
  }

/*! \brief Saves all open and modified memories
  \returns http return code0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::saveAllTmOnDisk( std::string &strOutputParms ){  
  int iRC = verifyAPISession();
  if(iRC) {
    strOutputParms = "Error: Can't save tm files on disk";
    return iRC;
  };
  std::string files; 
  
  FilesystemHelper::FlushAllBuffers(&files);
  strOutputParms = "{\n   \'saved files\': \'" + files + "\' \n}";
  return OK;
}


/*! \brief Saves all open and modified memories
  \returns http return code0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::closeAll(){
  int rc = TMManager::GetInstance()->closeAll();
  return rc;
}

/*! \brief List all available TMs
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::list
(
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();

  auto buildError = [&] (int errCode, bool getErrorFromEqf = false) mutable -> int{
    unsigned short usRC = 0;
    if(getErrorFromEqf)
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    else
      usRC = iRC;
    buildErrorReturn( usRC, this->szLastError, strOutputParms );
    return errCode;
  };

  if ( iRC != 0 )
    buildError(BAD_REQUEST);
  
  // get buffer size required for the list of TMs
  LONG lBufferSize = 0;
  iRC = EqfListMem( this->hSession, NULL, &lBufferSize );

  if ( iRC != 0 )
    return buildError(INTERNAL_SERVER_ERROR, true);

  // get the list of TMs
  PSZ pszBuffer = new char[lBufferSize];
  pszBuffer[0] = 0;//terminated to avoid garbage output

  iRC = EqfListMem( this->hSession, pszBuffer, &lBufferSize );
  if ( iRC != 0 )
  {
    delete pszBuffer;
    return buildError(INTERNAL_SERVER_ERROR, true);
  } /* endif */

  // add all TMs to reponse area
  std::stringstream jsonSS;
  std::istringstream buffer(pszBuffer);
  std::string strName;

  jsonSS << "{[";
  int elementCount = 0;

  while (std::getline(buffer, strName, ','))
  {
    if(elementCount)//skip delim for first elem
      jsonSS << ',';
    
    // add name to response area
    jsonSS << "{" << "name:" << strName << "}";
    elementCount++;
  } /* endwhile */

  jsonSS << "]}";
  strOutputParms = jsonSS.str();
  T5LOG( T5INFO) << "OtmMemoryServiceWorker::list()::strOutputParams = " << strOutputParms << "; iRC = OK";
  iRC = OK;
  return iRC;
}


/*! \brief Update an entry of the memory
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::updateEntry
(
  std::string strMemory,
  std::string strInputParms,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );

    // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strInputParms.c_str() );
  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );

  auto loggingThreshold = -1;
       
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSource ), sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szTarget ), sizeof( pData->szTarget ) / sizeof( pData->szTarget[0] ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },  

  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( pData->lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDocName ), sizeof( pData->szDocName ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szType ), sizeof( pData->szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szAuthor ), sizeof( pData->szAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMarkup ), sizeof( pData->szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szContext ), sizeof( pData->szContext ) / sizeof( pData->szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDateTime ), sizeof( pData->szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szAddInfo ), sizeof( pData->szAddInfo ) / sizeof( pData->szAddInfo[0] ) },
  { L"loggingThreshold",JSONFactory::INT_PARM_TYPE        , &(loggingThreshold), 0},
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = pJsonFactory->parseJSON( strInputParmsW, parseControl );

  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    wchar_t errMsg[] = L"Error: Parsing of input parameters failed";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szTarget[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    strcpy( pData->szMarkup, "OTMXUXLF");
  } /* end */

  if(loggingThreshold >=0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::updateEntry::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold); 
  }

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemory, &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // prepare the proposal data
  PMEMPROPOSAL pProp = new ( MEMPROPOSAL );
  memset( pProp, 0, sizeof( *pProp ) );
  int rc = 0;
  auto replacedInput = replaceString( pData->szSource , pData->szTarget, L"", &rc );
  //wcscpy( pProp->szSource, pData->szSource );
  //wcscpy( pProp->szTarget, pData->szTarget );
  if(!rc){
    wcscpy( pProp->szSource, replacedInput[0].c_str());
    wcscpy( pProp->szTarget, replacedInput[1].c_str());
  }else{
    strOutputParms = "Error in xml in source or target!";
    return rc;
    //wcscpy( pProp->szSource, pData->szSource);
    //wcscpy( pProp->szTarget, pData->szTarget);
  }
  pProp->lSegmentNum = pData->lSegmentNum;
  strcpy( pProp->szDocName, pData->szDocName );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pProp->szSourceLanguage );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pProp->szTargetLanguage );
  pProp->eType = this->getMemProposalType( pData->szType );
  strcpy( pProp->szTargetAuthor, pData->szAuthor ); 
  strcpy( pProp->szMarkup, pData->szMarkup );  
  wcscpy( pProp->szContext, pData->szContext );
  LONG lTime = 0;
  if ( pData->szDateTime[0] != 0 )
  {
    // use provided time stamp
    convertUTCTimeToLong( pData->szDateTime, &(pProp->lTargetTime) );
  }
  else
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    LONG            lTimeStamp;             // buffer for current time
    time( (time_t*)&lTimeStamp );
    lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    this->convertTimeToUTC( lTimeStamp, pData->szDateTime );
  }
  wcscpy( pProp->szAddInfo, pData->szAddInfo );

  // update the memory
  iRC = EqfUpdateMem( this->hSession, lHandle, pProp, 0 );
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pProp;
    delete pData;
    return( INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  //std::string strOutputParms;
  std::string str_src = EncodingHelper::convertToUTF8(pData->szSource );
  std::string str_trg = EncodingHelper::convertToUTF8(pData->szTarget );

  std::wstring strOutputParmsW;
  pJsonFactory->startJSON( strOutputParms );
  pJsonFactory->addParmToJSON( strOutputParms, "sourceLang", pData->szIsoSourceLang );
  pJsonFactory->addParmToJSON( strOutputParms, "targetLang", pData->szIsoTargetLang );

  pJsonFactory->addParmToJSONW( strOutputParmsW, L"source", pData->szSource );
  pJsonFactory->addParmToJSONW( strOutputParmsW, L"target", pData->szTarget );
  strOutputParms += ",\n" + EncodingHelper::convertToUTF8(strOutputParmsW.c_str());// +", ";

  
  pJsonFactory->addParmToJSON( strOutputParms, "documentName", pData->szDocName );
  pJsonFactory->addParmToJSON( strOutputParms, "segmentNumber", pData->lSegmentNum );
  pJsonFactory->addParmToJSON( strOutputParms, "markupTable", pData->szMarkup );
  pJsonFactory->addParmToJSON( strOutputParms, "timeStamp", pData->szDateTime );
  pJsonFactory->addParmToJSON( strOutputParms, "author", pData->szAuthor );
  pJsonFactory->terminateJSON( strOutputParms );
  //strOutputParms = EncodingHelper::convertToUTF8( strOutputParmsW.c_str() );

  iRC = OK;
  delete pProp;
  delete pData;

  return( iRC );
}

/*! \brief Delete an entry of the memory
\param strMemory name of memory
\param strInputParms input parameters in JSON format
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::deleteEntry
(
  std::string strMemory,
  std::string strInputParms,
  std::string &strOutputParms
){
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );

    // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strInputParms.c_str() );
  // parse input parameters
  PLOOKUPINMEMORYDATA pData = new( LOOKUPINMEMORYDATA );
  memset( pData, 0, sizeof( LOOKUPINMEMORYDATA ) );

  auto loggingThreshold = -1;
       
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szSource ), sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szTarget ), sizeof( pData->szTarget ) / sizeof( pData->szTarget[0] ) },
  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( pData->lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDocName ), sizeof( pData->szDocName ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoSourceLang ), sizeof( pData->szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szIsoTargetLang ), sizeof( pData->szIsoTargetLang ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szType ), sizeof( pData->szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szAuthor ), sizeof( pData->szAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szMarkup ), sizeof( pData->szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szContext ), sizeof( pData->szContext ) / sizeof( pData->szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( pData->szDateTime ), sizeof( pData->szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( pData->szAddInfo ), sizeof( pData->szAddInfo ) / sizeof( pData->szAddInfo[0] ) },
  { L"loggingThreshold",JSONFactory::INT_PARM_TYPE        , &(loggingThreshold), 0},
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  iRC = pJsonFactory->parseJSON( strInputParmsW, parseControl );

  if ( iRC != 0 )
  {
    iRC = ERROR_INTERNALFUNCTION_FAILED;
    wchar_t errMsg[] = L"Error: Parsing of input parameters failed";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szTarget[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 || strcasecmp( pData->szMarkup, "translate5" ) == 0)
  {
    strcpy(pData->szMarkup, "OTMXUXLF");
  } /* end */

  if(loggingThreshold >=0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::updateEntry::set new threshold for logging" <<loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold); 
  }

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC =  TMManager::GetInstance()->getMemoryHandle( strMemory,  &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // prepare the proposal data
  PMEMPROPOSAL pProp = new ( MEMPROPOSAL );
  memset( pProp, 0, sizeof( *pProp ) );
  wcscpy( pProp->szSource, pData->szSource );
  wcscpy( pProp->szTarget, pData->szTarget );
  pProp->lSegmentNum = pData->lSegmentNum;
  strcpy( pProp->szDocName, pData->szDocName );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pProp->szSourceLanguage );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pProp->szTargetLanguage );
  pProp->eType = this->getMemProposalType( pData->szType );
  strcpy( pProp->szTargetAuthor, pData->szAuthor );
  strcpy( pProp->szMarkup, pData->szMarkup );  
  wcscpy( pProp->szContext, pData->szContext );
  LONG lTime = 0;
  if ( pData->szDateTime[0] != 0 )
  {
    // use provided time stamp
    convertUTCTimeToLong( pData->szDateTime, &(pProp->lTargetTime) );
  }
  else
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    LONG            lTimeStamp;             // buffer for current time
    time( (time_t*)&lTimeStamp );
    lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    this->convertTimeToUTC( lTimeStamp, pData->szDateTime );
  }
  wcscpy( pProp->szAddInfo, pData->szAddInfo );

  std::string errorStr;
  errorStr.reserve(1000);
  // update the memory delete entry
  TMManager::GetInstance()->APIUpdateDeleteMem( lHandle, pProp, 0 , &errorStr[0]);
  //iRC = EqfUpdateDeleteMem( this->hSession, lHandle, pProp, 0,  &errorStr[0]);
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    auto w_error_str = EncodingHelper::convertToUTF16(errorStr.c_str());
    EqfGetLastErrorW( this->hSession, &usRC, (wchar_t*)w_error_str.c_str(), w_error_str.size());
    // pData->szError , sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pProp;
    delete pData;
    return( INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  std::string str_src = EncodingHelper::convertToUTF8(pData->szSource );
  std::string str_trg = EncodingHelper::convertToUTF8(pData->szTarget );

  pJsonFactory->startJSON( strOutputParms );
  pJsonFactory->addParmToJSON( strOutputParms, "sourceLang", pData->szIsoSourceLang );
  pJsonFactory->addParmToJSON( strOutputParms, "targetLang", pData->szIsoTargetLang );
  pJsonFactory->addParmToJSON( strOutputParms, "source", str_src.c_str());
  pJsonFactory->addParmToJSON( strOutputParms, "target", str_trg.c_str() );
  pJsonFactory->addParmToJSON( strOutputParms, "documentName", pData->szDocName );
  pJsonFactory->addParmToJSON( strOutputParms, "segmentNumber", pData->lSegmentNum );
  pJsonFactory->addParmToJSON( strOutputParms, "markupTable", pData->szMarkup );
  pJsonFactory->addParmToJSON( strOutputParms, "timeStamp", pData->szDateTime );
  pJsonFactory->addParmToJSON( strOutputParms, "author", pData->szAuthor );
  pJsonFactory->terminateJSON( strOutputParms );

  iRC = OK;
  delete pProp;
  delete pData;

  return( iRC );
}

int OtmMemoryServiceWorker::reorganizeMem
(
  std::string strMemory,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    T5LOG(T5ERROR) << "verifyAPISession fails:: iRC = " << iRC << "; strOutputParams = "<<
      strOutputParms << "; szLastError = " << EncodingHelper::convertToUTF8(this->szLastError);
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    T5LOG(T5ERROR) <<" error:: iRC = "<< iRC << "; strOutputParams = "<<
      strOutputParms << "; szLastError = "<< "Missing name of memory";
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  JSONFactory *pJsonFactory = JSONFactory::getInstance();

  // close memory if it is open
  std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  if ( pMem != nullptr )
  {
    // close the memory and remove it from our list
    TMManager::GetInstance()->removeFromMemoryList( pMem );
  } /* endif */

  // reorganize the memory
  if ( !iRC )
  {
    do
    {
      iRC = EqfOrganizeMem( this->hSession, (PSZ)strMemory.c_str()  );
    } while ( iRC == CONTINUE_RC );
  } /* endif */
  
  if ( iRC == ERROR_MEMORY_NOTFOUND )
  {
    strOutputParms = "{\"" + strMemory + "\": \"not found\" }";
    return( iRC = NOT_FOUND );
  }
  else if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    
    T5LOG(T5ERROR) << "fails:: iRC = " << iRC << "; strOutputParams = " << strOutputParms << "; szLastError = " <<
         EncodingHelper::convertToUTF8(this->szLastError);

    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( INTERNAL_SERVER_ERROR );
  }else{
    strOutputParms = "{\"" + strMemory + "\": \"reorganized\" }";
  }

  T5LOG(T5INFO) << "::success, memName = " << strMemory;
  iRC = OK;

  return( iRC );

}


/*! \brief delete a memory
\param strMemory name of memory
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::deleteMem
(
  std::string strMemory,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::deleteMem::verifyAPISession fails:: iRC = " << iRC << "; strOutputParams = "<< 
      strOutputParms <<  "; szLastError = " << EncodingHelper::convertToUTF8(this->szLastError);
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::deleteMem error:: iRC = " << iRC << "; strOutputParams = " << 
      strOutputParms << "; szLastError = ", "Missing name of memory";
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  JSONFactory *pJsonFactory = JSONFactory::getInstance();

  // close memory if it is open
  std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  if ( pMem != nullptr )
  {
    // close the memory and remove it from our list
    TMManager::GetInstance()->removeFromMemoryList( pMem );
  } /* endif */

  // delete the memory
  iRC = EqfDeleteMem( this->hSession, (PSZ)strMemory.c_str() );
  if ( iRC == ERROR_MEMORY_NOTFOUND )
  {
    strOutputParms = "{\"" + strMemory + "\": \"not found\" }";
    return( iRC = NOT_FOUND );
  }
  else if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::deleteMem::EqfDeleteMem fails:: iRC = " << iRC << "; strOutputParams = " << 
      strOutputParms <<  "; szLastError = " << EncodingHelper::convertToUTF8(this->szLastError) ;

    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( INTERNAL_SERVER_ERROR );
  }else{
    strOutputParms = "{\"" + strMemory + "\": \"deleted\" }";
  }

  T5LOG( T5INFO) <<"OtmMemoryServiceWorker::deleteMem::success, memName = " << strMemory;
  iRC = OK;

  return( iRC );
}

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
  T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem::=== getMem request, memory = " << strMemory << "; format = " << strType;
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem::Error: no valid API session" ;
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem::Error: no memory name specified" ;
    return( BAD_REQUEST );
  } /* endif */

  // close memory if it is open
  std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  if ( pMem != nullptr )
  {
    // close the memory and remove it from our list
    TMManager::GetInstance()->removeFromMemoryList( pMem );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem::Error: memory does not exist, memName = " << strMemory;
    return( NOT_FOUND );
  }

  // get a temporary file name for the memory package file or TMX file
  char szTempFile[PATH_MAX];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0 )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem:: Error: creation of temporary file for memory data failed" ;
    return( INTERNAL_SERVER_ERROR );
  }

  // export the memory in internal format
  if ( strType.compare( "application/xml" ) == 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem:: mem = " <<  strMemory << "; supported type found application/xml, tempFile = " << szTempFile;
    iRC = EqfExportMem( this->hSession, (PSZ)strMemory.c_str(), szTempFile, TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem:: Error: EqfExportMem failed with rc=" << iRC << ", error message is " <<  EncodingHelper::convertToUTF8( this->szLastError);
      return( INTERNAL_SERVER_ERROR );
    }
  }
  else if ( strType.compare( "application/zip" ) == 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem:: mem = "<< strMemory << "; supported type found application/zip(NOT TESTED YET), tempFile = " << szTempFile;
    iRC = EqfExportMemInInternalFormat( this->hSession, (PSZ)strMemory.c_str(), szTempFile, 0 );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem:: Error: EqfExportMemInInternalFormat failed with rc=" <<iRC << ", error message is " << EncodingHelper::convertToUTF8( this->szLastError);
      return( INTERNAL_SERVER_ERROR );
    }
  }
  else
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem:: Error: the type " << strType << " is not supported" ;
    return( NOT_ACCEPTABLE );
  }

  // fill the vMemData vector with the content of zTempFile
  iRC = this->loadFileIntoByteVector( szTempFile, vMemData );

  //cleanup
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( szTempFile );
  }
  
  if ( iRC != 0 )
  {
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::getMem::  Error: failed to load the temporary file, fName = " <<  szTempFile ;
    return( INTERNAL_SERVER_ERROR );
  }

  return( OK );
}


std::string OtmMemoryServiceWorker::printTime(time_t time){
  char buff[255];
  LONG lTimeStamp = (LONG) time;
  lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
  this->convertTimeToUTC( lTimeStamp, buff );
  //std::string res =  asctime(gmtime(&time));
  return buff;
}

/*! \brief get the status of a memory
\param strMemory name of memory
\param strOutParms on return filled with the output parameters in JSON format
\returns http return code
*/
int OtmMemoryServiceWorker::getStatus
(
  std::string strMemory,
  std::string &strOutputParms
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  JSONFactory *factory = JSONFactory::getInstance();

  // check if memory is contained in our list
  std::shared_ptr<OPENEDMEMORY>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  
  if ( pMem != nullptr )
  {
    // set status value
    std::string pszStatus = "";
    switch ( pMem->eImportStatus )
    {
      case IMPORT_RUNNING_STATUS: pszStatus = "import"; break;
      case IMPORT_FAILED_STATUS: pszStatus = "failed"; break;
      default: pszStatus = "available"; break;
    }
    // return the status of the memory
    factory->startJSON( strOutputParms );
    factory->addParmToJSON( strOutputParms, "status", "open" );
    factory->addParmToJSON( strOutputParms, "tmxImportStatus", pszStatus );
    if(pMem->importDetails != nullptr){
      factory->addParmToJSON( strOutputParms, "importProgress", pMem->importDetails->usProgress );
      factory->addParmToJSON( strOutputParms, "importTime", pMem->importDetails->importTimestamp );
      factory->addParmToJSON( strOutputParms, "segmentsImported", pMem->importDetails->segmentsImported );
      factory->addParmToJSON( strOutputParms, "invalidSegments", pMem->importDetails->invalidSegments );
      factory->addParmToJSON( strOutputParms, "invalidSymbolErrors", pMem->importDetails->invalidSymbolErrors );
      factory->addParmToJSON( strOutputParms, "importErrorMsg", pMem->importDetails->importMsg.str() );
    }
    factory->addParmToJSON( strOutputParms, "lastAccessTime", printTime(pMem->tLastAccessTime) );
    if ( ( pMem->eImportStatus == IMPORT_FAILED_STATUS ) && ( pMem->pszError != NULL ) )
    {
      factory->addParmToJSON( strOutputParms, "ErrorMsg", pMem->pszError );
    }
    factory->terminateJSON( strOutputParms );
    return( OK );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (char *)strMemory.c_str() ) != 0 )
  {
    factory->startJSON( strOutputParms );
    factory->addParmToJSON( strOutputParms, "status", "not found" );
    factory->terminateJSON( strOutputParms );
    return( NOT_FOUND );
  }

  factory->startJSON( strOutputParms );
  factory->addParmToJSON( strOutputParms, "status", "available" );
  factory->terminateJSON( strOutputParms );
  return( OK );
}

MemProposalType OtmMemoryServiceWorker::getMemProposalType( char *pszType )
{
  if ( strcasecmp( pszType, "GlobalMemory" ) == 0 )
  {
    return( GLOBMEMORY_PROPTYPE );
  }
  else if ( strcasecmp( pszType, "GlobalMemoryStar" ) == 0 )
  {
    return( GLOBMEMORYSTAR_PROPTYPE );
  }
  else if ( strcasecmp( pszType, "MachineTranslation" ) == 0 )
  {
    return( MACHINE_PROPTYPE );
  }
  else if ( strcasecmp( pszType, "Manual" ) == 0 )
  {
    return( MANUAL_PROPTYPE );
  } /* endif */
  return( UNDEFINED_PROPTYPE );
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


/*! \brief build a unique name for of a temporary file
\param pszTempFile buffer reiceiving the temporary file name
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::buildTempFileName( char *pszTempFile )
{ 
  int iRC = 0;
  std::string sTempPath;
  // setup temp file name for TMX file 
  {
    char szTempPath[PATH_MAX];
    properties_get_str(KEY_OTM_DIR, szTempPath, PATH_MAX);
    sTempPath = szTempPath;
    sTempPath += "/TEMP/";
    iRC = properties_get_str_or_default(KEY_TEMP_DIR, szTempPath, PATH_MAX, sTempPath.c_str());
    sTempPath = szTempPath;
  }

  if ( iRC != PROPERTY_USED_DEFAULT_VALUE && iRC != PROPERTY_NO_ERRORS )
  {
    return iRC;
  }
  else
  {
    iRC = 0;
    if(!FilesystemHelper::DirExists(sTempPath)){
      iRC = FilesystemHelper::CreateDir(sTempPath);
    }

    int i = 0;
    sTempPath += "OTM";
    std::string checkName = sTempPath;
    while( i < 1000 ){
      checkName = sTempPath + std::to_string(i/100) + std::to_string(i%100/10) + std::to_string(i%10);
      auto files = FilesystemHelper::FindFiles( checkName );
      
      if(files.size() == 0){// we can use this name
        T5LOG( T5INFO) << "OtmMemoryServiceWorker::buildTempFileName::Temp file's Name found :" << checkName ;
        strcpy(pszTempFile, checkName.c_str());
        break;
      }
      i++;
    }
    if( i >= 1000){
      T5LOG(T5ERROR) << "OtmMemoryServiceWorker::buildTempFileName::TO_DO::All temp names is already used - delete some of them";
    }

  }
  return( iRC );
}

/*! \brief read a binary file and encode it using BASE64 encoding
\param pszFile fully qualified name of file being encoded
\param ppStringData adress of a pointer which will receive the (per malloc allocated) pointer to the area for the encoded string
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::encodeFileInBase64( char *pszFile, char **ppStringData, std::string &strError )
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
int OtmMemoryServiceWorker::decodeBase64ToFile( const char *pStringData, const char *pszFile, std::string &strError )
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

/*! \brief read a binary file into a Byte vector
\param pszFile fully qualified name of file being read
\param vFileData vector receiving the file data
it is up to the caller to free this area using free()
\param strError string receiving any error message text
\returns 0 is sucessfull or a return code
*/
int OtmMemoryServiceWorker::loadFileIntoByteVector( char *pszFile, std::vector<unsigned char>  &vFileData )
{
  int iRC = 0;

  // open file
  HFILE hFile = FilesystemHelper::OpenFile(pszFile, "r", false);//CreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == NULL )
  {
    iRC = GetLastError();
    T5LOG(T5ERROR) <<"   Error: open of file "<< pszFile << " failed with rc=" << iRC << "\n";
    return( iRC );
  }

  // get the size of file
  DWORD dwFileSize = FilesystemHelper::GetFileSize(hFile);//GetFileSize( hFile, &dwFileSizeHigh );

  // adjust size of Byte vector
  vFileData.resize( dwFileSize );

  // read file into byte vector
  DWORD dwBytesRead = 0;
  if ( !ReadFile( hFile, &vFileData[0], dwFileSize, &dwBytesRead, NULL ) )
  {
    CloseFile( &hFile );
    iRC = GetLastError();
    T5LOG(T5ERROR) << "   Error: reading of "<< dwFileSize << " bytes from file "<< pszFile <<" failed with rc=" << iRC << "\n";
    return( iRC );
  }

  // close file
  CloseFile( &hFile );

  return( iRC );
}

void importMemoryProcess( void *pvData )
{
  PIMPORTMEMORYDATA pData = (PIMPORTMEMORYDATA)pvData;
  // call the OpenTM2 API function
  pData->szError[0] = 0;
  int iRC = (int)EqfImportMem( pData->hSession, pData->szMemory, pData->szInFile, TMX_OPT | COMPLETE_IN_ONE_CALL_OPT, pData->szError, pData->importDetails);

  // handle any error
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    T5LOG(T5ERROR) <<"importMemoryProcess:: error = ", pData->szError;
  }

  // update memory status
  //pData->pMemoryServiceWorker->importDone( pData->szMemory, iRC, pData->szError );
  TMManager::GetInstance()->importDone( std::string(pData->szMemory),iRC, pData->szError );
 
  // cleanup
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( pData->szInFile );
  }
  delete( pData );

  return;
}

