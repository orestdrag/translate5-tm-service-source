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

  //instance->hSession = 0;
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
    iRC = Properties::GetInstance()->get_value(KEY_MEM_DIR, memDir, 254);
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
  std::shared_ptr<EqfMemory>  pMem; 
  
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
  return( 0 );
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

int OtmMemoryServiceWorker::resourcesInfo(std::string& strOutput, ProxygenStats& stats){
  return 0;
}


std::string OtmMemoryServiceWorker::tagReplacement(std::string strInData, int& rc){
  return "";
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
  return 0;
}




/*! \brief shutdown the service
  \returns http return code
  */
  int OtmMemoryServiceWorker::shutdownService(int sig){    
    return 0;
  }

/*! \brief Saves all open and modified memories
  \returns http return code0 if successful or an error code in case of failures
  */
int OtmMemoryServiceWorker::saveAllTmOnDisk( std::string &strOutputParms ){  
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
  return 0;
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
  
  return 0;
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
  return 0;
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
  std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
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
  std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
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
  std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
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
  std::string strTempFile =  FilesystemHelper::BuildTempFileName();
  if (strTempFile.empty()  )
  {
    T5LOG(T5ERROR) <<"OtmMemoryServiceWorker::getMem:: Error: creation of temporary file for memory data failed" ;
    return( INTERNAL_SERVER_ERROR );
  }

  // export the memory in internal format
  if ( strType.compare( "application/xml" ) == 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem:: mem = " <<  strMemory << "; supported type found application/xml, tempFile = " << strTempFile;
    iRC = EqfExportMem( this->hSession, (PSZ)strMemory.c_str(), (PSZ)strTempFile.c_str(), TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
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
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::getMem:: mem = "<< strMemory << "; supported type found application/zip(NOT TESTED YET), tempFile = " << strTempFile;
    iRC = EqfExportMemInInternalFormat( this->hSession, (PSZ)strMemory.c_str(), (PSZ)strTempFile.c_str(), 0 );
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
  iRC = FilesystemHelper::LoadFileIntoByteVector( strTempFile, vMemData );

  //cleanup
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( strTempFile.c_str() );
  }
  
  if ( iRC != 0 )
  {
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::getMem::  Error: failed to load the temporary file, fName = " <<  strTempFile ;
    return( INTERNAL_SERVER_ERROR );
  }

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
  std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemory);
  
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



