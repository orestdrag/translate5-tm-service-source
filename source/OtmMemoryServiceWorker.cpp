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
#include <restbed>
#include "OtmMemoryServiceWorker.h"
#include "OTMMSJSONFactory.h"
#include "OTMFUNC.H"
#include "EQFMSG.H"
#include "EQFTM.H"
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


// import memory process
void importMemoryProcess( void *pvData );

/** Initialize the static instance variable */
OtmMemoryServiceWorker* OtmMemoryServiceWorker::instance = 0;

/*! \brief This static method returns a pointer to the OtmMemoryService object.
	The first call of the method creates the OtmMemoryService instance.
*/
OtmMemoryServiceWorker* OtmMemoryServiceWorker::getInstance()
{
	if (instance == 0)
	{
		instance = new OtmMemoryServiceWorker();
    instance->hSession = 0;
  }
	return instance;
}

/*! \brief OtmMemoryServiceWorker constructor
*/
OtmMemoryServiceWorker::OtmMemoryServiceWorker()
{
  hfLog = NULL;
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

  LogMessage(INFO, "Initializing API Session");
  this->iLastRC = EqfStartSession( &(this->hSession) );
  LogMessage2(INFO, "[verifyAPISession] EqfStartSession ret: " , toStr(this->iLastRC).c_str());

  if ( this->iLastRC != 0 ) {
    LogMessage2(ERROR, "OpenTM2 API session could not be started, the return code is" , toStr( this->iLastRC ).c_str());
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
  //LogMessage(ERROR, strUTF16);
  LogMessage(ERROR, strErrorReturn.c_str());
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




/*! \brief find a memory in our list of active memories
  \param pszMemory name of the memory
  \returns index in the memory table if successful, -1 if memory is not contained in the list
*/
int OtmMemoryServiceWorker::findMemoryInList( const char *pszMemory )
{
  // for debugging only: cause an exception when the special memory name "XX_CauseAnException_XX" is being used
#ifdef _DEBUG
  if ( strcmp( pszMemory, "XX_CauseAnException_XX" ) == 0 )
  {
    int *pi = NULL;
    return( *pi );
  } /* endif */
#endif
  if(this->vMemoryList.size()==0){
    LogMessage(WARNING,"findMemoryInList:: vMemoryList.size == 0");
  }
  // 
  for( int i = 0; i < (int)this->vMemoryList.size(); i++ )
  {
    if ( strcasecmp( this->vMemoryList[i].szName, pszMemory ) == 0 )
    {
      return( i );
    } /* endif */
  } /* endfor */
  return( -1 );
}

/*! \brief find a free slot in our list of active memories, add a new entry if none found
  \returns index of the next free slot in the memory table or -1 if there is no free slot and the max number of entries has been reached
*/
int OtmMemoryServiceWorker::getFreeSlot(size_t memoryRequested)
{
  size_t UsedMemory = 0;
  int AllowedMemoryMB;
  properties_get_int(KEY_ALLOWED_RAM, AllowedMemoryMB);
  size_t AllowedMemory = AllowedMemoryMB * 1000000;

  #ifdef CALCULATE_ONLY_MEM_FILES
  std::string path; 
  char memFolder[260];
  properties_get_str(KEY_MEM_DIR, memFolder, 260);

  for(int i = 0; i < vMemoryList.size() ;i++){
    path = memFolder;
    path += this->vMemoryList[i].szName;
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
    UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
  }
  #else
  UsedMemory = calculateOccupiedRAM() + memoryRequested;
  #endif

  LogMessage6(TRANSACTION, __func__,":: ", toStr(UsedMemory).c_str(), " bytes is used from ", toStr(AllowedMemory).c_str(), " allowed");

  // add a new entry when the maximum list size has not been reached yet
  //if ( this->vMemoryList.size() < OTMMEMSERVICE_MAX_NUMBER_OF_OPEN_MEMORIES )
  if( UsedMemory < AllowedMemory)
  {
    // first look for a free slot in the existing list
    for( int i = 0; i < (int)this->vMemoryList.size(); i++ )
    {
      if ( this->vMemoryList[i].szName[0] == 0 )
      {
        return( i );
      } /* endif */
    } /* endfor */
    
    OtmMemoryServiceWorker::OPENEDMEMORY NewEntry;
    memset( &NewEntry, 0, sizeof(NewEntry) );
    this->vMemoryList.push_back( NewEntry );
    return( this->vMemoryList.size() - 1 );
  } /* endif */  

  return( -1 );
}


size_t OtmMemoryServiceWorker::calculateOccupiedRAM(){
  char memFolder[260];
  size_t UsedMemory = 0;
  #ifdef CALCULATE_ONLY_MEM_FILES
  properties_get_str(KEY_MEM_DIR, memFolder, 260);
  std::string path;
  for(int i = 0; i < vMemoryList.size() ;i++){
    if(this->vMemoryList[i].szName != 0){
      path = memFolder;
      path += this->vMemoryList[i].szName;
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
    }
  }
  #else
  UsedMemory += FilesystemHelper::GetTotalFilebuffersSize();
  UsedMemory += MEMORY_RESERVED_FOR_SERVICE;
  #endif

  LogMessage4(INFO, __func__,":: calculated occupied ram = ", toStr(UsedMemory/1000000).c_str(), " MB");
  return UsedMemory;
}


/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
size_t OtmMemoryServiceWorker::cleanupMemoryList(size_t memoryRequested)
{  
  int AllowedMBMemory = 500;
  properties_get_int(KEY_ALLOWED_RAM, AllowedMBMemory);
  size_t AllowedMemory = AllowedMBMemory * 1000000;    
  size_t memoryNeed = memoryRequested + calculateOccupiedRAM();

  if(memoryNeed < AllowedMemory){
    return( AllowedMemory - memoryNeed );
  }

  time_t curTime;
  time( &curTime );
  std::multimap <time_t, int>  openedMemoriesSortedByLastAccess;
  for( int i = 0; i < (int)this->vMemoryList.size() ; i++ ){
    if ( vMemoryList[i].szName[0] != 0 )
    {
      openedMemoriesSortedByLastAccess.insert({vMemoryList[i].tLastAccessTime, i});
    }
  }

  for(auto it = openedMemoriesSortedByLastAccess.begin(); memoryNeed >= AllowedMemory && it != openedMemoriesSortedByLastAccess.end(); it++){
    LogMessage6(INFO, __func__,":: removing memory  \'", vMemoryList[it->second].szName, "\' that wasns\'t used for ", toStr(curTime - vMemoryList[it->second].tLastAccessTime).c_str(), " seconds" );
    removeFromMemoryList(it->second);
    memoryNeed = memoryRequested + calculateOccupiedRAM();
  }

  return( AllowedMemory - memoryNeed );
}

/*! \brief get the handle for a memory, if the memory is not opened yet it will be openend
\param pszMemory name of the memory
\param plHandle buffer for the memory handle
\param pszError buffer for an error message text in case of failures
\param iErrorBufSize size of the error text buffer in number of characters
\param piReturnCode pointer to a buffer for the OpenTM2 error code
\returns OK if successful or an HTTP error code in case of failures
*/
int OtmMemoryServiceWorker::getMemoryHandle( char *pszMemory, PLONG plHandle, wchar_t *pszError, int iErrorBufSize, int *piErrorCode )
{
  *piErrorCode = 0;
  int iIndex = findMemoryInList( pszMemory );
  if ( iIndex != -1 )
  {
    switch ( this->vMemoryList[iIndex].eStatus )
    {
      case OPEN_STATUS:
        *plHandle = this->vMemoryList[iIndex].lHandle;
        time( &( this->vMemoryList[iIndex].tLastAccessTime ) );
        return( restbed::OK );
        break;
      case IMPORT_RUNNING_STATUS:
        wcsncpy( pszError, L"TM is busy, import is running", iErrorBufSize );
        *piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( restbed::PROCESSING );
        break;
      case IMPORT_FAILED_STATUS:
        wcsncpy( pszError, L"TM import failed, memory may be not usable", iErrorBufSize );
        *piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( restbed::INTERNAL_SERVER_ERROR );
        break;
      case AVAILABLE_STATUS:
        {
          // open the memory
          LONG lHandle = 0;
          *piErrorCode = EqfOpenMem( this->hSession, pszMemory, &lHandle, 0 );
          if ( *piErrorCode != 0 )
          {
            unsigned short usRC = 0;
            EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
            return( restbed::INTERNAL_SERVER_ERROR );
          } /* endif */
          this->vMemoryList[iIndex].lHandle = lHandle;
          strcpy( this->vMemoryList[iIndex].szName, pszMemory );
          time( &( this->vMemoryList[iIndex].tLastAccessTime ) );
          this->vMemoryList[iIndex].eStatus = OPEN_STATUS;
          //if ( this->vMemoryList[iIndex].pszError != NULL ) free( this->vMemoryList[iIndex].pszError );
          //this->vMemoryList[iIndex].pszError = NULL;
          *plHandle = lHandle;
          return( restbed::OK );
        }
        break;
      default:
        wcsncpy( pszError, L"TM is in undefined state", iErrorBufSize );
        //*piErrorCode = ERROR_MEM_NOT_ACCESSIBLE;
        return( restbed::INTERNAL_SERVER_ERROR );
        break;
    }
  } /* endif */


  size_t requiredMemory = 0;
  {
    char memFolder[260];
    properties_get_str(KEY_MEM_DIR, memFolder, 260);
    std::string path;

    path = memFolder;
    path += pszMemory;
    requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMI"));
    requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".TMD"));
    requiredMemory += FilesystemHelper::GetFileSize( std::string(path + ".MEM"));
  } 

  // cleanup the memory list (close memories not used for a longer time)
  size_t memLeftAfterOpening = cleanupMemoryList(requiredMemory);

  LogMessage7(TRANSACTION,__func__,":: memory: ", pszMemory, "; required RAM:", toStr(requiredMemory).c_str(),"; RAM left after opening mem: ", toStr(memLeftAfterOpening).c_str());

  // find a free slot in the memory list
  iIndex = getFreeSlot(requiredMemory);

  // handle "too many open memories" condition
  if ( iIndex == -1 )
  {
    wcscpy( pszError, L"Error: too many open translation memory databases" );
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // open the memory
  LONG lHandle = 0;
  *piErrorCode = EqfOpenMem( this->hSession, pszMemory, &lHandle, 0 );
  if ( *piErrorCode != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pszError, (unsigned short)iErrorBufSize );
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // add opened memory to the memory list
  this->vMemoryList[iIndex].lHandle = lHandle;
  strcpy( this->vMemoryList[iIndex].szName, pszMemory );
  time( &(this->vMemoryList[iIndex].tLastAccessTime) );
  this->vMemoryList[iIndex].eStatus = OPEN_STATUS;
  this->vMemoryList[iIndex].eImportStatus = OPEN_STATUS;
  this->vMemoryList[iIndex].pszError = NULL;
  *plHandle = lHandle;

  return( restbed::OK );
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

/*! \brief close a memory and remove it from the open list
    \param iIndex index of memory in the open list
  \returns 0 
*/
int OtmMemoryServiceWorker::removeFromMemoryList( int iIndex )
{
  LONG lHandle = this->vMemoryList[iIndex].lHandle;

  // remove the memory from the list
  this->vMemoryList[iIndex].lHandle = 0;
  this->vMemoryList[iIndex].tLastAccessTime = 0;
  this->vMemoryList[iIndex].szName[0] = 0;
  if ( this->vMemoryList[iIndex].pszError ) delete[]  this->vMemoryList[iIndex].pszError ;
  this->vMemoryList[iIndex].pszError = NULL;

  // close the memory
  EqfCloseMem( this->hSession, lHandle, 0 );

  return( 0 );
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
  std::string &strInputParms,
  std::string &strOutputParms
)
{
  //EncodingHelper::convertUTF8ToASCII( strMemory );
  LogMessage3(INFO, "+POST ", strMemory.c_str(),"/import\n");
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    return( restbed::NOT_FOUND );
  }

  // find the memory to our memory list
  int iIndex = this->findMemoryInList( strMemory.c_str() );
  LONG lHandle = 0;
  BOOL fClose = false;
  MEMORY_STATUS lastImportStatus = AVAILABLE_STATUS; // to restore in case we would break import before calling closemem
  MEMORY_STATUS lastStatus = AVAILABLE_STATUS;
  if ( iIndex != -1 )
  {
    // close the memory - when open
    if ( this->vMemoryList[iIndex].eStatus == OPEN_STATUS )
    {
      fClose = true;
      lHandle =          this->vMemoryList[iIndex].lHandle; //
      lastStatus =       this->vMemoryList[iIndex].eStatus;
      lastImportStatus = this->vMemoryList[iIndex].eImportStatus;

      this->vMemoryList[iIndex].lHandle = 0;
      this->vMemoryList[iIndex].eStatus = AVAILABLE_STATUS;
      this->vMemoryList[iIndex].eImportStatus = IMPORT_RUNNING_STATUS;
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
    size_t memLeftAfterOpening = cleanupMemoryList(requiredMemory);
    LogMessage7(TRANSACTION,__func__,":: memory: ", strMemory.c_str(), "; required RAM:", 
        toStr(requiredMemory).c_str(),"; RAM left after opening mem: ", toStr(memLeftAfterOpening).c_str());

    // find a free slot in the memory list
    iIndex = getFreeSlot(requiredMemory);

    // handle "too many open memories" condition
    if ( iIndex == -1 )
    {
      wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Error: too many open translation memory databases";
      buildErrorReturn( iRC, errMsg, strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    } /* endif */

    this->vMemoryList[iIndex].lHandle = 0;
    this->vMemoryList[iIndex].eStatus = AVAILABLE_STATUS;
    this->vMemoryList[iIndex].eImportStatus = IMPORT_RUNNING_STATUS;
    strcpy( this->vMemoryList[iIndex].szName, strMemory.c_str() );
  }
  LogMessage4(TRANSACTION, __func__, "status for ", strMemory.c_str() , " was changed to import");
  // extract TMX data
  std::string strTmxData;
  int loggingThreshold = -1; //0-develop(show all logs), 1-debug+, 2-info+, 3-warnings+, 4-errors+, 5-fatals only
  
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  if ( parseHandle == NULL )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing or incorrect JSON data in request body";
    buildErrorReturn( iRC, errMsg, strOutputParms );
        
    this->vMemoryList[iIndex].lHandle = lHandle;
    this->vMemoryList[iIndex].eStatus = lastStatus;
    this->vMemoryList[iIndex].eImportStatus = lastImportStatus;

    return( restbed::BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( iRC == 0 )
  {
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      //LogMessage4(INFO, "parsed json name = ", name.c_str(), "; value = ", value.c_str());
      if ( strcasecmp( name.c_str(), "tmxData" ) == 0 )
      {
        strTmxData = value;
        //break;
      }else if(strcasecmp(name.c_str(), "loggingThreshold") == 0){
        loggingThreshold = std::stoi(value);
        LogMessage2(WARNING,"OtmMemoryServiceWorker::import::set new threshold for logging", toStr(loggingThreshold).c_str());
        SetLogLevel(loggingThreshold);        
      }else{
        LogMessage2(WARNING, "JSON parsed unexpected name, ", name.c_str());
      }
    }else if(iRC != 2002){// iRC != INFO_ENDOFPARAMETERLISTREACHED
        LogMessage4(ERROR, "failed to parse JSON \"", strInputParms.c_str(), "\", iRC = ", toStr(iRC).c_str());
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );
  if ( strTmxData.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing TMX data";
    buildErrorReturn( iRC, errMsg, strOutputParms );
  
    //restore status
    this->vMemoryList[iIndex].lHandle = lHandle;
    this->vMemoryList[iIndex].eStatus = lastStatus;
    this->vMemoryList[iIndex].eImportStatus = lastImportStatus;

    return( restbed::BAD_REQUEST );
  } /* endif */

  // setup temp file name for TMX file 
  char szTempFile[PATH_MAX];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0  )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Could not create file name for temporary data";
    buildErrorReturn( -1, errMsg, strOutputParms );
    
    //restore status
    this->vMemoryList[iIndex].lHandle = lHandle;
    this->vMemoryList[iIndex].eStatus = lastStatus;
    this->vMemoryList[iIndex].eImportStatus = lastImportStatus;
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  LogMessage2(INFO, "OtmMemoryServiceWorker::import::+   Temp TMX File is ", szTempFile);

  // decode TMX data and write it to temp file
  std::string strError;
  iRC = decodeBase64ToFile( strTmxData.c_str(), szTempFile, strError ) ;
  if ( iRC != 0 )
  {
    strError = "OtmMemoryServiceWorker::import::" + strError;
    buildErrorReturn( iRC, (char *)strError.c_str(), strOutputParms );
    
    //restore status
    this->vMemoryList[iIndex].lHandle = lHandle;
    this->vMemoryList[iIndex].eStatus = lastStatus;
    this->vMemoryList[iIndex].eImportStatus = lastImportStatus;
    return( restbed::INTERNAL_SERVER_ERROR );
  }
  
  //success in parsing request data-> close mem if needed
  if(lHandle && fClose){
        EqfCloseMem( this->hSession, lHandle, 0 );
  }

  // start the import background process
  PIMPORTMEMORYDATA pData = new( IMPORTMEMORYDATA );
  strcpy( pData->szInFile, szTempFile );
  strcpy( pData->szMemory, strMemory.c_str() );
  pData->hSession = hSession;
  pData->pMemoryServiceWorker = this;

  //importMemoryProcess(pData);//to do in same thread
  std::thread worker_thread(importMemoryProcess, pData);
  worker_thread.detach();

  return( restbed::CREATED );
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
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( iRC );
  } /* endif */

  // parse input parameters
  std::string strData;
  std::string strName;
  std::string strSourceLang;
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strInputParms, &iRC );
  if ( parseHandle == NULL )
  {
    wchar_t errMsg[] = L"Missing or incorrect JSON data in request body";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( iRC == 0 )
  {
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {      
      if ( strcasecmp( name.c_str(), "data" ) == 0 )
      {
        strData = value;
        LogMessage4(DEBUG, "JSON parsed name = ", name.c_str(), "; size = ", toStr(strData.size()));
      }
      else{
        LogMessage4(DEBUG, "JSON parsed name = ", name.c_str(), "; value = ",value.c_str());
        if ( strcasecmp( name.c_str(), "name" ) == 0 )
        {
          strName = value;
        }
        else if ( strcasecmp( name.c_str(), "sourceLang" ) == 0 )
        {
          strSourceLang = value;
        }else if(strcasecmp(name.c_str(), "loggingThreshold") == 0){
          int loggingThreshold = std::stoi(value);
          LogMessage2(WARNING,"OtmMemoryServiceWorker::import::set new threshold for logging", toStr(loggingThreshold).c_str());
          SetLogLevel(loggingThreshold);        
        }else{
          LogMessage4(WARNING, "JSON parsed unused data: name = ", name.c_str(), "; value = ",value.c_str());
        }
      }
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );

  if ( strName.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing memory name input parameter", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( strSourceLang.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, L"Error: Missing source language input parameter", strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */

  // convert the ISO source language to a OpenTM2 source language name
  char szOtmSourceLang[40];
  szOtmSourceLang[0] = 0;//terminate to avoid garbage
  EqfGetOpenTM2Lang( this->hSession, (PSZ)strSourceLang.c_str(), szOtmSourceLang );
 
  if ( szOtmSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    swprintf( this->szLastError, 200, L"Error: Could not convert language %ls to OpenTM2 language name", EncodingHelper::convertToUTF16(strSourceLang.c_str()).c_str() );
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* end */


  // either create an empty memory or build the memory using binary input data
  //EncodingHelper::convertUTF8ToASCII( strName );

  if ( strData.empty() )
  {
    LogMessage(INFO, "int OtmMemoryServiceWorker::createMemory():: strData is empty -> EqfCreateMem()");
    iRC = (int)EqfCreateMem( this->hSession, (PSZ)strName.c_str(), 0, (PSZ)szOtmSourceLang, 0 );
  }
  else
  {
     LogMessage(INFO, "int OtmMemoryServiceWorker::createMemory():: strData is not empty -> setup temp file name for ZIP package file ");
    // setup temp file name for ZIP package file 
    char szTempFile[PATH_MAX];
    iRC = buildTempFileName( szTempFile );
    if ( iRC != 0 )
    {
      wchar_t errMsg[] = L"Could not create file name for temporary data";
      buildErrorReturn( -1, errMsg, strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    }

    LogMessage2(INFO, "+   Temp binary file is ", szTempFile );

    // decode binary data and write it to temp file
    std::string strError;
    iRC = decodeBase64ToFile( strData.c_str(), szTempFile, strError );
    if ( iRC != 0 )
    {
      buildErrorReturn( iRC, (char *)strError.c_str(), strOutputParms );
      return( restbed::INTERNAL_SERVER_ERROR );
    }

    iRC = (int)EqfImportMemInInternalFormat( this->hSession, (PSZ)strName.c_str(), szTempFile, 0 );
    if(CheckLogLevel(DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
        DeleteFile( szTempFile );
    }
  }

  if ( iRC != 0 )
  {
    char szError[PATH_MAX];
    unsigned short usRC = 0;
    EqfGetLastError( this->hSession, &usRC, szError, sizeof( szError ) );
    buildErrorReturn( iRC, szError, strOutputParms );

    switch ( usRC )
    {
      case ERROR_MEM_NAME_INVALID:
        iRC = restbed::CONFLICT;
        LogMessage3(ERROR, "OtmMemoryServiceWorker::createMemo()::usRC = ", toStr(usRC).c_str()," iRC = restbed::CONFLICT");
        break;
      case TMT_MANDCMDLINE:
      case ERROR_NO_SOURCELANG:
      case ERROR_PROPERTY_LANG_DATA:
        iRC = restbed::BAD_REQUEST;
        LogMessage3(ERROR, "OtmMemoryServiceWorker::createMemo()::usRC = ", toStr(usRC).c_str()," iRC = restbed::BAD_REQUEST");
        break;
      default:
        iRC = restbed::INTERNAL_SERVER_ERROR;
        LogMessage3(ERROR, "OtmMemoryServiceWorker::createMemo()::usRC = ", toStr(usRC).c_str()," iRC = restbed::INTERNAL_SERVER_ERROR");
        break;
    }
    return( iRC );
  }

  factory->startJSON( strOutputParms );

// TODO investigate how much do we need this
  //EncodingHelper::convertASCIIToUTF8( strName );

  factory->addParmToJSON( strOutputParms, "name", strName );
  factory->terminateJSON( strOutputParms );
  return( restbed::OK );
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

  convertTimeToUTC( pProp->lTargetTime, pData->szDocName );
  MultiByteToWideChar( CP_OEMCP, 0, pData->szDocName, -1, pData->szSource, sizeof( pData->szSource ) / sizeof( pData->szSource[0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"timestamp", pData->szSource );

  pJsonFactory->addParmToJSONW( strJSON, L"matchRate", pProp->iFuzziness );

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
    return( restbed::BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
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
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language input parameter";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    LogMessage(INFO,"OtmMemoryServiceWorker::search::No markup requested -> using OTMANSI");
    // use default markup table if none given
    strcpy( pData->szMarkup, "OTMANSI" );
  } /* end */
  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Too many proposals requested, the maximum value is 20";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals == 0 )
  {
    pData->iNumOfProposals = 5;
  }
  if(loggingThreshold >= 0){
    LogMessage2(WARNING,"OtmMemoryServiceWorker::search::set new threshold for logging", toStr(loggingThreshold).c_str());
    SetLogLevel(loggingThreshold);
  }

  // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
  {
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pData;
    return( httpRC );
  } /* endif */

  // prepare the memory lookup
  PMEMPROPOSAL pSearchKey = new (MEMPROPOSAL);
  PMEMPROPOSAL pFoundProposals = new( MEMPROPOSAL[pData->iNumOfProposals] );
  memset( pSearchKey, 0, sizeof( *pSearchKey ) );
  memset( pFoundProposals, 0, sizeof( MEMPROPOSAL ) * pData->iNumOfProposals );
  wcscpy( pSearchKey->szSource, pData->szSource );
  strcpy( pSearchKey->szDocName, pData->szDocName );
  pSearchKey->lSegmentNum = pData->lSegmentNum;
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoSourceLang, pSearchKey->szSourceLanguage );
  EqfGetOpenTM2Lang( this->hSession, pData->szIsoTargetLang, pSearchKey->szTargetLanguage );
  strcpy( pSearchKey->szMarkup, pData->szMarkup );
  wcscpy( pSearchKey->szContext, pData->szContext );

  // do the lookup and handle the results
  int iNumOfProposals = pData->iNumOfProposals;
  iRC = EqfQueryMem( this->hSession, lHandle, pSearchKey, &iNumOfProposals, pFoundProposals, GET_EXACT );
  
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
    iRC = restbed::OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = restbed::INTERNAL_SERVER_ERROR;
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
    return( restbed::BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
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
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSearchString[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Error: Missing search string";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals > 20 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::concordanceSearch::Error: Too many proposals requested, the maximum value is 20";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->iNumOfProposals == 0 )
  {
    pData->iNumOfProposals = 5;
  }

  if(loggingThreshold >= 0){
    LogMessage2(WARNING,"OtmMemoryServiceWorker::concordanceSearch::set new threshold for logging", toStr(loggingThreshold).c_str());
    SetLogLevel(loggingThreshold);
  }
    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
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

  PMEMPROPOSAL pProposal = new( MEMPROPOSAL );
  std::wstring strProposals;

  // loop until end reached or enough proposals have been found
  int iFoundProposals = 0;
  int iActualSearchTime = 0; // for the first call run until end of TM or one proposal has been found
  do
  {
    memset( pProposal, 0, sizeof( *pProposal ) );
    iRC = EqfSearchMem( this->hSession, lHandle, pData->szSearchString, pData->szSearchPos, pProposal, iActualSearchTime, lOptions );
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
    iRC = restbed::OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( usRC, pData->szError, strOutputParms );
    iRC = restbed::INTERNAL_SERVER_ERROR;
  } /* endif */

  //if ( pFoundProposals ) delete pFoundProposals;
  if ( pProposal ) delete pProposal;
  delete pData;

  return( iRC );
}


/*! \brief Close all open memories
\returns http return code0 if successful or an error code in case of failures
*/
int OtmMemoryServiceWorker::closeAll
(
)
{
  for ( int i = 0; i < ( int )this->vMemoryList.size(); i++ )
  {
    if ( this->vMemoryList[i].szName[0] != 0 )
    {
      removeFromMemoryList( i );
    }
  } /* endfor */
  return( 0 );
}

/*! \brief Set the log file handle
\param hfLog log file handle or NULL to stop logging
*/
void OtmMemoryServiceWorker::setLogFile
(
  FILE *hfLogIn
)
{
  this->hfLog = hfLogIn;
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
    buildError(restbed::BAD_REQUEST);
  
  // get buffer size required for the list of TMs
  LONG lBufferSize = 0;
  iRC = EqfListMem( this->hSession, NULL, &lBufferSize );

  if ( iRC != 0 )
    return buildError(restbed::INTERNAL_SERVER_ERROR, true);

  // get the list of TMs
  PSZ pszBuffer = new char[lBufferSize];
  pszBuffer[0] = 0;//terminated to avoid garbage output

  iRC = EqfListMem( this->hSession, pszBuffer, &lBufferSize );
  if ( iRC != 0 )
  {
    delete pszBuffer;
    return buildError(restbed::INTERNAL_SERVER_ERROR, true);
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
  LogMessage3(INFO, "OtmMemoryServiceWorker::list()::strOutputParams = ", strOutputParms.c_str(), "; iRC = restbed::OK");
  iRC = restbed::OK;
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
    return( restbed::BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
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
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szTarget[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing markup table";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */

  if(loggingThreshold >=0){
    LogMessage2(WARNING,"OtmMemoryServiceWorker::updateEntry::set new threshold for logging", toStr(loggingThreshold).c_str());
    SetLogLevel(loggingThreshold); 
  }

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
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
  if ( pData->szMarkup[0] == 0 )
  {
    // use default
    strcpy( pProp->szMarkup, "OTMANSI" );
  }
  else if ( strcasecmp( pData->szMarkup, "translate5" ) == 0 )
  {
    strcpy( pProp->szMarkup, "OTMXUXLF" );
  }
  else
  {
    strcpy( pProp->szMarkup, pData->szMarkup );
  }
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
    return( restbed::INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  //std::string strOutputParms;
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
  //strOutputParms = EncodingHelper::convertToUTF8( strOutputParmsW );

  iRC = restbed::OK;
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
)
{
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
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
    return( restbed::BAD_REQUEST );
  } /* end */

  if ( pData->szSource[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szTarget[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target text";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoSourceLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing source language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szIsoTargetLang[0] == 0 )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    wchar_t errMsg[] = L"Error: Missing target language";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    delete pData;
    return( restbed::BAD_REQUEST );
  } /* end */
  if ( pData->szMarkup[0] == 0 )
  {
    strcpy(pData->szMarkup, "OTMXUXLF");
    //strcpy( pProp->szMarkup, "OTMANSI" );
    //iRC = ERROR_INPUT_PARMS_INVALID;
    //wchar_t errMsg[] = L"Error: Missing markup table";
    //buildErrorReturn( iRC, errMsg, strOutputParms );
    //delete pData;
    //return( restbed::BAD_REQUEST );
  } /* end */

  if(loggingThreshold >=0){
    LogMessage2(WARNING,"OtmMemoryServiceWorker::updateEntry::set new threshold for logging", toStr(loggingThreshold).c_str());
    SetLogLevel(loggingThreshold); 
  }

    // get the handle of the memory 
  long lHandle = 0;
  int httpRC = this->getMemoryHandle( (char *)strMemory.c_str(), &lHandle, pData->szError, sizeof( pData->szError ) / sizeof( pData->szError[0] ), &iRC );
  if ( httpRC != restbed::OK )
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
  if ( strcasecmp( pData->szMarkup, "translate5" ) == 0 )
  {
    strcpy( pProp->szMarkup, "OTMXUXLF" );
  }
  else
  {
    strcpy( pProp->szMarkup, pData->szMarkup );
  }
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
  iRC = EqfUpdateDeleteMem( this->hSession, lHandle, pProp, 0,  &errorStr[0]);
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    auto w_error_str = EncodingHelper::convertToUTF16(errorStr.c_str());
    EqfGetLastErrorW( this->hSession, &usRC, (wchar_t*)w_error_str.c_str(), w_error_str.size());
    // pData->szError , sizeof( pData->szError ) / sizeof( pData->szError[0] ) );
    buildErrorReturn( iRC, pData->szError, strOutputParms );
    delete pProp;
    delete pData;
    return( restbed::INTERNAL_SERVER_ERROR );
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

  iRC = restbed::OK;
  delete pProp;
  delete pData;

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
    LogMessage6(ERROR,"OtmMemoryServiceWorker::deleteMem::verifyAPISession fails:: iRC = ", toStr(iRC).c_str(), "; strOutputParams = ", 
      strOutputParms.c_str(), "; szLastError = ", EncodingHelper::convertToUTF8(this->szLastError).c_str());
    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    LogMessage6(ERROR,"OtmMemoryServiceWorker::deleteMem error:: iRC = ", toStr(iRC).c_str(), "; strOutputParams = ", 
      strOutputParms.c_str(), "; szLastError = ", "Missing name of memory");
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  JSONFactory *pJsonFactory = JSONFactory::getInstance();

  // close memory if it is open
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
  } /* endif */

  // delete the memory
  iRC = EqfDeleteMem( this->hSession, (PSZ)strMemory.c_str() );
  if ( iRC == ERROR_MEMORY_NOTFOUND )
  {
    return( iRC = restbed::NOT_FOUND );
  }
  else if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    
    LogMessage6(ERROR,"OtmMemoryServiceWorker::deleteMem::EqfDeleteMem fails:: iRC = ", toStr(iRC).c_str(), "; strOutputParams = ", 
      strOutputParms.c_str(), "; szLastError = ", EncodingHelper::convertToUTF8(this->szLastError).c_str());

    buildErrorReturn( iRC, this->szLastError, strOutputParms );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  LogMessage2(INFO,"OtmMemoryServiceWorker::deleteMem::success, memName = ", strMemory.c_str());
  iRC = restbed::OK;


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
  restbed::Bytes &vMemData
)
{
  LogMessage4(INFO,"OtmMemoryServiceWorker::getMem::=== getMem request, memory = ", strMemory.c_str(), "; format = ", strType.c_str());
  int iRC = verifyAPISession();
  if ( iRC != 0 )
  {
    LogMessage(INFO,"OtmMemoryServiceWorker::getMem::Error: no valid API session" );
    return( restbed::BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    LogMessage(ERROR,"OtmMemoryServiceWorker::getMem::Error: no memory name specified" );
    return( restbed::BAD_REQUEST );
  } /* endif */

  // close memory if it is open
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // close the memory and remove it from our list
    removeFromMemoryList( iIndex );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (PSZ)strMemory.c_str() ) != 0 )
  {
    LogMessage2(ERROR,"OtmMemoryServiceWorker::getMem::Error: memory does not exist, memName = ", strMemory.c_str() );
    return( restbed::NOT_FOUND );
  }

  // get a temporary file name for the memory package file or TMX file
  char szTempFile[PATH_MAX];
  iRC = buildTempFileName( szTempFile );
  if ( iRC != 0 )
  {
    LogMessage(ERROR,"OtmMemoryServiceWorker::getMem:: Error: creation of temporary file for memory data failed" );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  // export the memory in internal format
  if ( strType.compare( "application/xml" ) == 0 )
  {
    LogMessage4(INFO,"OtmMemoryServiceWorker::getMem:: mem = ", strMemory.c_str(),"; supported type found application/xml, tempFile = ", szTempFile);
    iRC = EqfExportMem( this->hSession, (PSZ)strMemory.c_str(), szTempFile, TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      LogMessage4(ERROR,"OtmMemoryServiceWorker::getMem:: Error: EqfExportMem failed with rc=", toStr(iRC).c_str(), ", error message is ", EncodingHelper::convertToUTF8( this->szLastError).c_str() );
      return( restbed::INTERNAL_SERVER_ERROR );
    }
  }
  else if ( strType.compare( "application/zip" ) == 0 )
  {
    LogMessage4(INFO,"OtmMemoryServiceWorker::getMem:: mem = ", strMemory.c_str(),"; supported type found application/zip(NOT TESTED YET), tempFile = ", szTempFile);
    iRC = EqfExportMemInInternalFormat( this->hSession, (PSZ)strMemory.c_str(), szTempFile, 0 );
    if ( iRC != 0 )
    {
      unsigned short usRC = 0;
      EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      LogMessage4(ERROR,"OtmMemoryServiceWorker::getMem:: Error: EqfExportMemInInternalFormat failed with rc=",toStr(iRC).c_str(),", error message is ", EncodingHelper::convertToUTF8( this->szLastError).c_str() );
      return( restbed::INTERNAL_SERVER_ERROR );
    }
  }
  else
  {
    LogMessage3(ERROR,"OtmMemoryServiceWorker::getMem:: Error: the type ", strType.c_str()," is not supported" );
    return( restbed::NOT_ACCEPTABLE );
  }

  // fill the vMemData vector with the content of zTempFile
  iRC = this->loadFileIntoByteVector( szTempFile, vMemData );

  //cleanup
  if(CheckLogLevel(DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( szTempFile );
  }
  
  if ( iRC != 0 )
  {
    LogMessage2(ERROR, "OtmMemoryServiceWorker::getMem::  Error: failed to load the temporary file, fName = ", szTempFile );
    return( restbed::INTERNAL_SERVER_ERROR );
  }

  return( restbed::OK );
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
    return( restbed::BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemory );
  if ( strMemory.empty() )
  {
    wchar_t errMsg[] = L"Missing name of memory";
    buildErrorReturn( iRC, errMsg, strOutputParms );
    return( restbed::BAD_REQUEST );
  } /* endif */

  JSONFactory *factory = JSONFactory::getInstance();

  // check if memory is contained in our list
  int iIndex = this->findMemoryInList( (char *)strMemory.c_str() );
  if ( iIndex != -1 )
  {
    // set status value
    std::string pszStatus = "";
    switch ( this->vMemoryList[iIndex].eImportStatus )
    {
      case IMPORT_RUNNING_STATUS: pszStatus = "import"; break;
      case IMPORT_FAILED_STATUS: pszStatus = "failed"; break;
      default: pszStatus = "available"; break;
    }
    // return the status of the memory
    factory->startJSON( strOutputParms );
    factory->addParmToJSON( strOutputParms, "tmxImportStatus", pszStatus );
    if ( ( this->vMemoryList[iIndex].eImportStatus == IMPORT_FAILED_STATUS ) && ( this->vMemoryList[iIndex].pszError != NULL ) )
    {
      factory->addParmToJSON( strOutputParms, "ErrorMsg", this->vMemoryList[iIndex].pszError );
    }
    factory->terminateJSON( strOutputParms );
    return( restbed::OK );
  } /* endif */

  // check if memory exists
  if ( EqfMemoryExists( this->hSession, (char *)strMemory.c_str() ) != 0 )
  {
    return( restbed::NOT_FOUND );
  }

  factory->startJSON( strOutputParms );
  factory->addParmToJSON( strOutputParms, "tmxImportStatus", "available" );
  factory->terminateJSON( strOutputParms );
  return( restbed::OK );
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
        xercesc::XMLPlatformUtils::Initialize();
        *rc = verifyAPISession();
        if(*rc == 0){
          response = EncodingHelper::ReplaceOriginalTagsWithPlaceholders(std::move(src_data), std::move(trg_data), std::move(req_data) );
        }
        xercesc::XMLPlatformUtils::Terminate();
    }
    //catch (const XMLException& toCatch) {
      catch(...){
        *rc = 500;
        //return( ERROR_NOT_READY );
    }
  return response;
}

// update memory status
void OtmMemoryServiceWorker::importDone( char *pszMemory, int iRC, char *pszError )
{
  int iIndex = this->findMemoryInList( pszMemory );
  if ( iIndex != -1 )
  {
    if ( iRC == 0 )
    {
      vMemoryList[iIndex].eImportStatus = AVAILABLE_STATUS;
      LogMessage2(INFO,"OtmMemoryServiceWorker::importDone:: success, memName = ", pszMemory);
    }
    else
    {
      vMemoryList[iIndex].eImportStatus = IMPORT_FAILED_STATUS;
      vMemoryList[iIndex].pszError = new char[strlen( pszError ) + 1];
      strcpy( vMemoryList[iIndex].pszError, pszError );
      LogMessage4(ERROR, "OtmMemoryServiceWorker::importDone:: memName = ", pszMemory,", import failed: ", pszError);
    }
  }else{
    LogMessage3(ERROR, "OtmMemoryServiceWorker::importDone:: memName = ", pszMemory, " not found ");
  }
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
    while(i<1000){
      checkName = sTempPath + std::to_string(i/100) + std::to_string(i%100/10) + std::to_string(i%10);
      auto files = FilesystemHelper::FindFiles(checkName);
      
      if(files.size() == 0){// we can use this name
        LogMessage2(INFO, "OtmMemoryServiceWorker::buildTempFileName::Temp file's Name found :", checkName.c_str());
        strcpy(pszTempFile, checkName.c_str());
        break;
      }
      i++;
    }
    if( i >= 1000){
      LogMessage(ERROR, "OtmMemoryServiceWorker::buildTempFileName::TO_DO::All temp names is already used - delete some of them");
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
          LogMessage2(ERROR, "encodeBase64ToFile()::ecnoding of BASE64 data failed, iRC = ", toStr(iRC).c_str());
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
    LogMessage2(ERROR, "decodeBase64ToFile()::decoding of BASE64 data failed, iRC = ", toStr(iRC).c_str());
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
int OtmMemoryServiceWorker::loadFileIntoByteVector( char *pszFile, restbed::Bytes &vFileData )
{
  int iRC = 0;

  // open file
  HFILE hFile = FilesystemHelper::OpenFile(pszFile, "r", false);//CreateFile( pszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == NULL )
  {
    iRC = GetLastError();
    if ( hfLog ) fprintf( hfLog, "   Error: open of file %s failed with rc=%d\n", pszFile, iRC );
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
    if ( hfLog ) fprintf( hfLog, "   Error: reading of %ld bytes from file %s failed with rc=%ld\n", dwFileSize, pszFile, iRC );
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
  int iRC = (int)EqfImportMem( pData->hSession, pData->szMemory, pData->szInFile, TMX_OPT | COMPLETE_IN_ONE_CALL_OPT, pData->szError);

  // handle any error
  if ( iRC != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastError( pData->hSession, &usRC, pData->szError, sizeof( pData->szError ) );
    LogMessage2(ERROR,"importMemoryProcess:: error = ", pData->szError);
  }

  // update memory status
  pData->pMemoryServiceWorker->importDone( pData->szMemory, iRC, pData->szError );

  // cleanup
  if(CheckLogLevel(DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    DeleteFile( pData->szInFile );
  }
  delete( pData );

  return;
}

