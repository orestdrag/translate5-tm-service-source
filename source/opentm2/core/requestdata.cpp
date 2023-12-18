
#include <thread>
#include "requestdata.h"
#include "tm.h"
#include "otm.h"
#include "../../RestAPI/OTMMSJSONFactory.h"
#include "../../RestAPI/OtmMemoryServiceWorker.h"
#include "LogWrapper.h"
#include "EncodingHelper.h"
#include "EQFMORPH.H"
#include "FilesystemHelper.h"
#include "Property.h"
#include "OSWrapper.h"
#include "LanguageFactory.H"
#include <set>
#include "../../cmake/git_version.h"
#include "Stopwatch.hpp"
#include "EQFFUNCI.H"

JSONFactory RequestData::json_factory;


/*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date and time
  \returns 0 
*/
int convertTimeToUTC( LONG lTime, char *pszDateTime )
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



/*! \brief extract a numeric value from a string
    \param pszString string containing the numeric value
    \param iLen number of digits to be processed
    \param piResult pointer to a int value receiving the extracted value
  \returns TRUE if successful and FALSE in case of errors
*/
bool getValue( char *pszString, int iLen, int *piResult )
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

/*! \brief convert a UTC time value to a OpenTM2 long time value 
    \param pszDateTime buffer containing the UTC date and time
    \param plTime pointer to a long buffer receiving the converted time 
  \returns 0 
*/
int convertUTCTimeToLong( char *pszDateTime, PLONG plTime )
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


#include <tm.h>

/*
std::unique_ptr<RequestTagReplacer> replaceString(std::wstring&& src_data, std::wstring&& trg_data, std::wstring&& req_data,  int* rc){ 
  std::unique_ptr<RequestTagReplacer> response;
  *rc = 0;
  try {        
        *rc = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
        if(*rc == 0){
          response = EncodingHelper::ReplaceOriginalTagsWithPlaceholders(std::move(src_data), std::move(trg_data), std::move(req_data) );
        }
    }
    //catch (const XMLException& toCatch) {
      catch(...){
        *rc = 400;
        //return( ERROR_NOT_READY );
    }
  return std::move(response);
}//*/




/*! \brief build return JSON string in case of errors
  \param _rc_ error return code
  \param pszErrorMessage error message text
  \param strError JSON string receiving the error information
  \returns 0 if successful or an error code in case of failures
*/
int RequestData::buildErrorReturn
(
  int rc,
  const wchar_t *pszErrorMsg
)
{
  std::wstring wMsg(pszErrorMsg);
  std::string strMsg = EncodingHelper::convertToUTF8( wMsg );
  return buildErrorReturn(rc, strMsg.c_str());
}

int RequestData::buildErrorReturn
(
  int rc,
  const char *pszErrorMsg
)
{
  if(!_rc_) _rc_ = rc;
  json_factory.startJSON( outputMessage );
  json_factory.addParmToJSON( outputMessage, "ReturnValue", rc );
  json_factory.addParmToJSON( outputMessage, "ErrorMsg", pszErrorMsg );
  json_factory.terminateJSON( outputMessage );
  if(_rest_rc_ == 0) _rest_rc_ = BAD_REQUEST;
  T5LOG(T5ERROR) << outputMessage;
  return( rc );
}


int RequestData::requestTM(){
  //check if memory is loaded to tmmanager
  if(isReadOnlyRequest())
  {
    mem = TMManager::GetInstance()->requestReadOnlyTMPointer(strMemName, memRef);
  }else if(isWriteRequest())
  {
    mem = TMManager::GetInstance()->requestWriteTMPointer(strMemName, memRef);
  }

  if(mem.get()== nullptr)
  {
  //  buildErrorReturn( _rc_, Data.szError );
    fValid = isServiceRequest();
  }else{
    fValid = true;
    
    if(  command == EXPORT_MEM 
      || command == EXPORT_MEM_INTERNAL_FORMAT
      || command == CLONE_TM_LOCALY
      || command == REORGANIZE_MEM
      )
    {
      mem->FlushFilebuffers();
    }
  }  

  return fValid == 0;
}

bool RequestData::isWriteRequest(){
  return //command == COMMAND::CLONE_TM_LOCALY ||
      //|| command == COMMAND::DELETE_MEM
       command == COMMAND::DELETE_ENTRY
      || command == COMMAND::UPDATE_ENTRY
      //|| command == COMMAND::CREATE_MEM// should be handled as service command
      || command == COMMAND::IMPORT_MEM
      || command == COMMAND::REORGANIZE_MEM
      || command == COMMAND::IMPORT_LOCAL_MEM
      ;
}


bool RequestData::isReadOnlyRequest(){
  return command == COMMAND::FUZZY
      || command == COMMAND::CONCORDANCE
      || command == COMMAND::EXPORT_MEM
      || command == COMMAND::EXPORT_MEM_INTERNAL_FORMAT
     ;
}


bool RequestData::isServiceRequest(){
  return !isWriteRequest() && !isReadOnlyRequest();
  
  //return
         COMMAND::LIST_OF_MEMORIES
      || COMMAND::SAVE_ALL_TM_ON_DISK 
      || COMMAND::TAGREPLACEMENTTEST
      || COMMAND::STATUS_MEM
      || COMMAND::SHUTDOWN
      || COMMAND::RESOURCE_INFO
      || COMMAND::CREATE_MEM
   //?   || COMMAND::CLONE_TM_LOCALY
   //?   || COMMAND::DETELE_MEM
  ;   
}

int RequestData::buildRet(int res){
  if(!_rest_rc_){
    if(_rc_ == 0){
      _rest_rc_ = 200;
    }else if(_rc_>= 100 && _rc_ <1000){
      _rest_rc_ = _rc_;
    }else{
      _rc_ = 500;
    }
  }

  if(outputMessage.empty()){
    if(_rest_rc_ >=200 && _rest_rc_ <300){
      outputMessage = "{\n\t\"msg\": \"success\"\n}";
    }else{
      //TODO: change to json
      outputMessage = "{\n\t\"msg\": \"emptyMessage, rc = "  + std::to_string(_rc_) + ", rest_rc = " + std::to_string(_rest_rc_) + "\"\n}";
    }
  }
  T5LOG(T5DEBUG) << outputMessage;
  return 0;
}

int RequestData::run(){   
  int res = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if(!res) res = parseJSON();
  if(!res) res = checkData();

  if(!res) res = requestTM();
  
  if(!res && fValid) 
    res = execute();
  buildRet(res);
  //reset pointers
  if(mem != nullptr){
    mem.reset();
  }
  if(memRef != nullptr){
    memRef.reset();
  }
  return res;
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

int CreateMemRequestData::createNewEmptyMemory(){
  // create memory database
  if(!_rc_){
    mem = TMManager::GetInstance()->CreateNewEmptyTM(strMemName, strSrcLang, strMemDescription, _rc_);
  }
  T5LOG( T5INFO) << " done, usRC = " << _rc_ ;
  if(!_rc_){
    TMManager::GetInstance()->AddMem(mem);
  }
  if(mem != nullptr){
    mem.reset();
  }
  return 0;
}

const char* GetFileExtention(std::string file){
  auto lastDot = file.rfind('.');
  if(lastDot>0){
    return &file[lastDot];
  }else{
    return &file[0];
  }
}

int CreateMemRequestData::importInInternalFomat(){
  T5LOG( T5INFO) << ":: strData is not empty -> setup temp file name for ZIP package file ";
  // setup temp file name for ZIP package file 

  strTempFile =  FilesystemHelper::BuildTempFileName();
  if (strTempFile.empty()  )
  {
    buildErrorReturn( -1, "Could not create file name for temporary data" );
    return( INTERNAL_SERVER_ERROR );
  }

  T5LOG( T5INFO) << "+   Temp binary file is " << strTempFile ;

  // decode binary data and write it to temp file
  std::string strError;
  _rc_ = FilesystemHelper::DecodeBase64ToFile( strMemB64EncodedData.c_str(), strTempFile.c_str(), strError );
  if ( _rc_ != 0 )
  {
      buildErrorReturn( _rc_, (char *)strError.c_str() );
      return( INTERNAL_SERVER_ERROR );
  }

  // make temporary directory for the memory files of the package
  std::string tempMemUnzipFolder = FilesystemHelper::GetTempDir();
  FilesystemHelper::CreateDir( FilesystemHelper::GetTempDir() );
  tempMemUnzipFolder += "MemImp/";
  FilesystemHelper::CreateDir( tempMemUnzipFolder );
  tempMemUnzipFolder += strMemName + '/';
  FilesystemHelper::CreateDir( tempMemUnzipFolder );

  // unpzip the package
  FilesystemHelper::ZipExtract( strTempFile, tempMemUnzipFolder );
  
      
  auto files = FilesystemHelper::GetFilesList(tempMemUnzipFolder.c_str());
  std::string memDir = FilesystemHelper::GetMemDir();
  for( auto file: files ){
    if(file.size() <= 4){
      continue;
    }
    std::string targetFName = memDir + strMemName + GetFileExtention(file);
    if(FilesystemHelper::FileExists(targetFName.c_str())){
      T5LOG(T5ERROR) << ":: file exists, fName = " << targetFName;
      _rc_ = -1;
      break;
    }
  }

  if( !_rc_ ){
    for( auto file:files){
      if(file.size() <= 4){
        continue;
      }
      std::string targetFName = memDir + strMemName + GetFileExtention(file);
      std::string oldFName = tempMemUnzipFolder + file;
      FilesystemHelper::MoveFile(oldFName, targetFName);
    }
    
    if(_rc_ = TMManager::GetInstance()->TMExistsOnDisk(strMemName)){
      T5LOG( T5INFO) << ":: memory file not found after import in internal format, mem name = "<< strMemName << "; rc = " << _rc_;
      _rc_ = -2;
    }      
  }

  // delete any files left over and remove the directory
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    FilesystemHelper::RemoveDirWithFiles( tempMemUnzipFolder );
    FilesystemHelper::DeleteFile( strTempFile );
  }
  return _rc_;
}

int CreateMemRequestData::checkData(){
  if ( !_rc_ && strMemName.empty() )
  {
    return buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing memory name input parameter" );
  } /* end */
  
  if ( !_rc_ && strSrcLang.empty() && strMemB64EncodedData.empty())// it's import in internal format
  {
    return buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source language input parameter" );
  } /* end */

  // convert the ISO source language to a OpenTM2 source language name
  char szOtmSourceLang[40];
  if(!_rc_ && strSrcLang.empty() == false ){        
    szOtmSourceLang[0] = 0;//terminate to avoid garbage
    bool fPrefered = false;
    EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, (PSZ)strSrcLang.c_str(), szOtmSourceLang, &fPrefered );
    
    if ( szOtmSourceLang[0] == 0 )
    {
      std::string err =  "Error: Could not convert language " + strSrcLang + "to OpenTM2 language name";
      return buildErrorReturn( ERROR_INPUT_PARMS_INVALID, (PSZ)err.c_str() );
    } /* end */
  }

  T5LOG(T5DEBUG) << "( MemName = "<<strMemName <<", pszDescription = "<<strMemDescription<<", pszSourceLanguage = "<<strSrcLang<<" )";
  // Check memory name syntax
  if ( _rc_ == NO_ERROR )
  { 
    if ( !FilesystemHelper::checkFileName( strMemName ))
    {
      std::string errMsg = "::ERROR_INV_NAME::" + strMemName ;
      buildErrorReturn( ERROR_MEM_NAME_INVALID,  errMsg.c_str() );
      return _rc_;
    } /* endif */
  } /* endif */

  // check if TM exists already
  if ( _rc_ == NO_ERROR )
  {
    int res = TMManager::GetInstance()->TMExistsOnDisk(strMemName, false );
    if ( res != TMManager::TMM_TM_NOT_FOUND )
    {
      std::string errMsg = "::ERROR_MEM_NAME_EXISTS:: TM with this name already exists: " +  strMemName + "; res = " + std::to_string(res) ;
      buildErrorReturn( ERROR_MEM_NAME_EXISTS, errMsg.c_str());
      return _rc_;
    }
  } /* endif */


  // check if source language is valid
  if ( _rc_ == NO_ERROR )
  {
    SHORT sID = 0;    
    if ( MorphGetLanguageID( (PSZ)strSrcLang.c_str(), &sID ) != MORPH_OK )
    {
      std::string errMsg = "MorhpGetLanguageID returned error, usRC = ERROR_PROPERTY_LANG_DATA::" +  strSrcLang ;
      buildErrorReturn( ERROR_PROPERTY_LANG_DATA, errMsg.c_str());
      return _rc_;
    } /* endif */
  } /* endif */
  //fValid = !_rest_rc_ && !_rc_;
  return _rest_rc_;
}

int CreateMemRequestData::parseJSON(){
  // parse input parameters
  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    return buildErrorReturn( _rc_, "Missing or incorrect JSON data in request body" );
  } /* end */
  std::string name;
  std::string value;
  while ( _rc_ == 0 )
  {
    _rc_ = json_factory.parseJSONGetNext( parseHandle, name, value );
    if ( _rc_ == 0 )
    {      
      if ( strcasecmp( name.c_str(), "data" ) == 0 )
      {
        strMemB64EncodedData = value;
        T5LOG( T5DEBUG) << "JSON parsed name = " << name << "; size = " << value.size();
      }
      else
      {
        T5LOG( T5DEBUG) << "JSON parsed name = " << name << "; value = " << value;
        if ( strcasecmp( name.c_str(), "name" ) == 0 )
        {
          strMemName = value;
        }
        else if ( strcasecmp( name.c_str(), "sourceLang" ) == 0 )
        {
            strSrcLang = value;
        }else if(strcasecmp(name.c_str(), "loggingThreshold") == 0){
            int loggingThreshold = std::stoi(value);
            T5LOG( T5WARNING) <<"import::set new threshold for logging" <<loggingThreshold ;
            T5Logger::GetInstance()->SetLogLevel(loggingThreshold);        
        }else{
            T5LOG( T5WARNING) << "JSON parsed unused data: name = " << name << "; value = " << value;
        }
      }
    }
  } /* endwhile */
  json_factory.parseJSONStop( parseHandle );
  if(_rc_ == 2002) _rc_=0;
  if (_rc_) buildErrorReturn (_rc_, "Error during parsing json");
  return _rc_;
}

int CreateMemRequestData::execute(){
  if ( strMemB64EncodedData.empty() ) // create empty tm
  {
    createNewEmptyMemory();
  }else{
    importInInternalFomat();
  }
     
  if(_rc_ == ERROR_MEM_NAME_EXISTS){
    outputMessage = "{\"" + strMemName + "\": \"ERROR_MEM_NAME_EXISTS\" }" ;
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::createMemo()::usRC = " << _rc_ << " _rc_ = ERROR_MEM_NAME_EXISTS";
    return _rc_;
  }else if ( _rc_ != 0 )
  {    
    char szError[PATH_MAX];
    unsigned short usRC = 0;
    EqfGetLastError( NULL /*OtmMemoryServiceWorker::getInstance()->hSession*/, &usRC, szError, sizeof( szError ) );
    //buildErrorReturn( _rc_, szError, outputMessage );

    outputMessage = "{\"" + strMemName + "\": \"" ;
    switch ( usRC )
    {
      case ERROR_MEM_NAME_INVALID:
        _rest_rc_ = CONFLICT;
        outputMessage += "CONFLICT";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " _rc_ = CONFLICT";
        break;
      case TMT_MANDCMDLINE:
      case ERROR_NO_SOURCELANG:
      case ERROR_PROPERTY_LANG_DATA:
        _rest_rc_ = BAD_REQUEST;
        outputMessage += "BAD_REQUEST";
        T5LOG(T5ERROR) << "::usRC = " <<  usRC << " _rc_ = BAD_REQUEST";
        break;
      default:
        _rest_rc_ = INTERNAL_SERVER_ERROR;
        outputMessage += "INTERNAL_SERVER_ERROR";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " _rc_ = INTERNAL_SERVER_ERROR";
        break;
    }
    outputMessage +="\"}";
    return( _rest_rc_ );
  }else{
    json_factory.startJSON( outputMessage );
    json_factory.addParmToJSON( outputMessage, "name", strMemName );
    json_factory.terminateJSON( outputMessage );
    return( OK );
  }
}

// import memory process
void importMemoryProcess( void *pvData );

// reorganize memory process
void reorganizeMemoryProcess( void *pvData );

int ImportRequestData::parseJSON(){
  T5LOG( T5INFO) << "+POST " << strMemName << "/import\n";
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, "ImportRequestData::parseJSON, rc is not 0 at the start of function" );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "import::Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */

  // check if memory exists
  if(int existscode = TMManager::GetInstance()->TMExistsOnDisk(strMemName))
  {
    std::string msg = "import::Missing tm files on disk, code=";
    msg += std::to_string(existscode);
    buildErrorReturn( 404, msg.c_str());
    return( _rest_rc_ = NOT_FOUND );
  }

  // find the memory to our memory list
  // extract TMX data
  int loggingThreshold = -1; //0-develop(show all logs), 1-debug+, 2-info+, 3-warnings+, 4-errors+, 5-fatals only
  
  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( _rc_, "import::Missing or incorrect JSON data in request body" );
    return( BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( _rc_ == 0 )
  {
    _rc_ = json_factory.parseJSONGetNext( parseHandle, name, value );
    if ( _rc_ == 0 )
    {
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
    }else if(_rc_ != 2002){// _rc_ != INFO_ENDOFPARAMETERLISTREACHED
      std::string msg = "failed to parse JSON, _rc_ = " + std::to_string(_rc_);
      return buildErrorReturn(_rc_, msg.c_str());
    }
  } /* endwhile */
  json_factory.parseJSONStop( parseHandle );
  if(_rc_ == 2002) _rc_ == 0;
  return 0; 
}

int ListTMRequestData::execute(){
  if ( _rc_ != 0 )
  {
    return buildErrorReturn(_rc_, "Error code is not 0 before executing listtm main code");
  }
  // add all TMs to reponse area
  std::stringstream jsonSS;
  std::istringstream buffer;
  std::string strName;

  jsonSS << "{\n\t\"Open\":[";
  int elementCount = 0;
  std::set<std::string> printedTMNames;
  for(auto& tmem: TMManager::GetInstance()->tms)
  {
    if(elementCount)//skip delim for first elem
      jsonSS << ',';
    
    // add name to response area
    jsonSS << "{ " << "\"name\": \"" << tmem.first << "\" }";
    printedTMNames.emplace(tmem.first);
    elementCount++;
  } /* endwhile */

  jsonSS << "],\n\t\"Available on disk\": [";

  // get list of files
  std::string memPath = FilesystemHelper::GetMemDir() + '*';
  auto Files = FilesystemHelper::FindFiles(memPath);
  elementCount = 0;
  for(auto& file: Files){
    std::string memName = FilesystemHelper::RemovePathAndExtention(file);
    if(!TMManager::GetInstance()->TMExistsOnDisk(memName)){
      if(!printedTMNames.count(memName)){
        if(elementCount){//skip delim for first elem
          jsonSS << ',';
        }
        
        // add name to response area
        jsonSS << "{ " << "\"name\": \"" << memName << "\" }";
        printedTMNames.emplace(memName);
        elementCount++;
      }
    }
  }
  jsonSS <<"]\n}";
  outputMessage = jsonSS.str();
  return _rc_;
}

int ImportRequestData::checkData(){
  if ( strTmxData.empty() )
  {
    buildErrorReturn( _rc_, "import::Missing TMX data" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* endif */

  // setup temp file name for TMX file 
  strTempFile =  FilesystemHelper::BuildTempFileName();
  if (strTempFile.empty()  )
  {
    buildErrorReturn( -1, "import::Could not create file name for temporary data" );
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }

  T5LOG( T5INFO) << "import::+   Temp TMX File is " << strTempFile;

  // decode TMX data and write it to temp file
  std::string strError;
  _rc_ = FilesystemHelper::DecodeBase64ToFile( strTmxData.c_str(), strTempFile.c_str(), strError ) ;
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, (char *)strError.c_str() );
    //restore status
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }
  return 0;
  
}

int SaveAllTMsToDiskRequestData::execute(){
  if(_rc_) {
    outputMessage = "Error: Can't save tm files on disk";
    return _rc_;
  };
  std::string tms; 
  
  int count = 0;
  for(const auto& tm: TMManager::GetInstance()->tms){
    if(count++)
      tms += ", ";
    tm.second->FlushFilebuffers();
    tms += tm.first;
  }
  outputMessage = "{\n   \'saved " + std::to_string(count) +" tms\': \'" + tms + "\' \n}";
  return OK;
}

int ImportRequestData::execute(){
  if ( mem == nullptr )
  {
    return 404;
  }
    // close the memory - when open
  if ( mem->eStatus != OPEN_STATUS )
  {
    return 500;
  }
  lastStatus =       mem->eStatus;
  lastImportStatus = mem->eImportStatus;

  mem->eStatus = IMPORT_RUNNING_STATUS;
  mem->eImportStatus = IMPORT_RUNNING_STATUS;
  //mem->dImportProcess = 0;

  T5LOG( T5DEBUG) <<  "status for " << strMemName << " was changed to import";
  // start the import background process
  pData = new( IMPORTMEMORYDATA );
  pData->fDeleteTmx = true;
  strcpy( pData->szInFile, strTempFile.c_str() );
  strcpy( pData->szMemory, strMemName.c_str() );

  if(mem->importDetails == nullptr){
    mem->importDetails = new ImportStatusDetails;
  }
  
  mem->importDetails->reset();
  pData->mem = mem;

  //importMemoryProcess(pData);//to do in same thread
  std::thread worker_thread(importMemoryProcess, pData);
  worker_thread.detach();

  return( CREATED );
}

int ImportLocalRequestData::parseJSON(){
  T5LOG( T5INFO) << "+POST " << strMemName << "/importLocal\n";
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, "ImportRequestData::parseJSON, rc is not 0 at the start of function" );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "import::Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */

  // check if memory exists
  if(int existscode = TMManager::GetInstance()->TMExistsOnDisk(strMemName))
  {
    std::string msg = "import::Missing tm files on disk, code=";
    msg += std::to_string(existscode);
    buildErrorReturn( 404, msg.c_str());
    return( _rest_rc_ = NOT_FOUND );
  }

  // find the memory to our memory list
  // extract TMX data
  int loggingThreshold = -1; //0-develop(show all logs), 1-debug+, 2-info+, 3-warnings+, 4-errors+, 5-fatals only
  
  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( _rc_, "import::Missing or incorrect JSON data in request body" );
    return( BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( _rc_ == 0 )
  {
    _rc_ = json_factory.parseJSONGetNext( parseHandle, name, value );
    if ( _rc_ == 0 )
    {
      if ( strcasecmp( name.c_str(), "localPath" ) == 0 )
      {
        strInputFile = value;
        //break;
      }else if(strcasecmp(name.c_str(), "loggingThreshold") == 0){
        loggingThreshold = std::stoi(value);
        T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::import::set new threshold for logging" << loggingThreshold;
        T5Logger::GetInstance()->SetLogLevel(loggingThreshold);        
      }else{
        T5LOG( T5WARNING) << "JSON parsed unexpected name, " << name;
      }
    }else if(_rc_ != 2002){// _rc_ != INFO_ENDOFPARAMETERLISTREACHED
      std::string msg = "failed to parse JSON, _rc_ = " + std::to_string(_rc_);
      return buildErrorReturn(_rc_, msg.c_str());
    }
  } /* endwhile */
  json_factory.parseJSONStop( parseHandle );
  if(_rc_ == 2002) _rc_ == 0;
  return 0; 
}

int ImportLocalRequestData::checkData(){
  if (strInputFile.empty()  )
  {
    buildErrorReturn( -1, "importLocal::Filename of inputfile is empty" );
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }

  T5LOG( T5INFO) << "importLocal::+  TMX File is " << strInputFile;
  if(!FilesystemHelper::FileExists(strInputFile)){
    return  buildErrorReturn( 404, "importLocal::InputFile not found" );
  }
  return 0;
  
}

int ImportLocalRequestData::execute(){
  if ( mem == nullptr )
  {
    return 404;
  }
    // close the memory - when open
  if ( mem->eStatus != OPEN_STATUS )
  {
    return 500;
  }
  lastStatus =       mem->eStatus;
  lastImportStatus = mem->eImportStatus;

  mem->eStatus = IMPORT_RUNNING_STATUS;
  mem->eImportStatus = IMPORT_RUNNING_STATUS;
  //mem->dImportProcess = 0;

  T5LOG( T5DEBUG) <<  "status for " << strMemName << " was changed to import";
  // start the import background process
  pData = new( IMPORTMEMORYDATA );
  strcpy( pData->szInFile, strInputFile.c_str() );
  strcpy( pData->szMemory, strMemName.c_str() );

  if(mem->importDetails == nullptr){
    mem->importDetails = new ImportStatusDetails;
  }
  
  mem->importDetails->reset();
  pData->mem = mem;

  //importMemoryProcess(pData);//to do in same thread
  std::thread worker_thread(importMemoryProcess, pData);
  worker_thread.detach();

  return( CREATED );
}

int ReorganizeRequestData::parseJSON(){
  return 0;
}

int ReorganizeRequestData::checkData(){
  if ( strMemName.empty() )
  {
    T5LOG(T5ERROR) <<" error:: _rc_ = "<< _rc_ << "; strOutputParams = "<<
      outputMessage << "; szLastError = "<< "Missing name of memory";
    buildErrorReturn( _rc_, "Missing name of memory");
    return( BAD_REQUEST );
  } 
  return 0;
}

// Prepare the organize of a TM in function call mode
USHORT MemFuncOrganizeProcess
(
  PFCTDATA    pData                    // function I/F session data
);

int ReorganizeRequestData::execute(){
  if ( mem == nullptr )
  {
    return 404;
  }
    // close the memory - when open
  if ( mem->eStatus != OPEN_STATUS )
  {
    return 500;
  }

  mem->eStatus = REORGANIZE_RUNNING_STATUS;
  mem->eImportStatus = REORGANIZE_RUNNING_STATUS;
  
  PFCTDATA    pData = new FCTDATA;            // ptr to function data area
  PPROCESSCOMMAREA pCommArea = new PROCESSCOMMAREA;   // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA = new MEM_ORGANIZE_IDA;      // pointer to instance area

  // validate session handle
  //_rc_ = FctValidateSession( OtmMemoryServiceWorker::getInstance()->hSession, &pData ); 

  // reorganize the memory
  if ( !_rc_ )
  {    
    // prepare TM organize process
    // Fill  IDA with necessary values
    pCommArea->pUserIDA = pRIDA;
    pRIDA->pMem = mem;
    pRIDA->memRef = memRef;
    pRIDA->usRC = NO_ERROR;
    strcpy( pRIDA->szMemName, mem->szName );
    pRIDA->fBatch = TRUE;
    pRIDA->hwndErrMsg = HWND_FUNCIF;
    pRIDA->NextTask = MEM_START_ORGANIZE;

    // enable organize process if OK
    pData->fComplete = FALSE;
    pData->sLastFunction = FCT_EQFORGANIZEMEM;
    pData->pvMemOrganizeCommArea = pCommArea;    
    pRIDA->pszNameList = 0;
    if(mem->importDetails == nullptr){
      mem->importDetails = new ImportStatusDetails;
    }
    
    mem->importDetails->reset();
    LONG lCurTime = 0;  
    time( &lCurTime );
                  
    pRIDA->pMem->importDetails->lReorganizeStartTime = lCurTime;
    mem->importDetails->fReorganize = true;

    mem.reset();
    memRef.reset();
  } 

  //worker thread 
  if(!_rc_){
    //reorganizeMemoryProcess(pData);//to do in same thread
    std::thread worker_thread(reorganizeMemoryProcess, pData);
    worker_thread.detach();
  }else{
    delete pData;
    delete pCommArea;
    delete pCommArea;
  }
  
  if ( _rc_ == ERROR_MEMORY_NOTFOUND )
  {
    outputMessage = "{\"" + strMemName + "\": \"not found\" }";
    return( _rc_ = NOT_FOUND );
  }
  /*
  else if ( _rc_ != 0 )
  {
    unsigned short usRC = 0;

    wchar_t szLastError[4000];
    EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, szLastError, sizeof( szLastError ) / sizeof( szLastError[0] ) );
    
    T5LOG(T5ERROR) << "fails:: _rc_ = " << _rc_ << "; strOutputParams = " << outputMessage << "; szLastError = " <<
         EncodingHelper::convertToUTF8(szLastError);
    std::string strLastError = EncodingHelper::convertToUTF8(szLastError);
    buildErrorReturn( _rc_, strLastError.c_str() );
    mem->eStatus = OPEN_STATUS;
    mem->eImportStatus = REORGANIZE_FAILED_STATUS;
    mem->strError = strLastError;
    return( INTERNAL_SERVER_ERROR );
  }else{
    mem->eStatus = OPEN_STATUS;
    mem->eImportStatus = AVAILABLE_STATUS;
    outputMessage = "{\"" + strMemName + "\": \"reorganized\" }";
  } //*/

  T5LOG(T5INFO) << "::success, memName = " << strMemName;
  _rc_ = OK;

  return( _rc_ );

}


int DeleteMemRequestData::execute(){
  if ( strMemName.empty() )
  {
    T5LOG(T5ERROR) <<"::deleteMem error:: _rc_ = " << _rc_ << "; strOutputParams = " << 
      outputMessage << "; szLastError = ", "Missing name of memory";
    buildErrorReturn( _rc_, "Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */
  
  TMManager::GetInstance()->DeleteTM(strMemName, outputMessage);

  if ( _rc_ != 0 )
  {
    unsigned short usRC = 0;
    //EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    
    T5LOG(T5ERROR) <<"DeleteMem fails:: _rc_ = " << _rc_ << "; strOutputParams = " << 
      outputMessage <<  "; szLastError = ";// << EncodingHelper::convertToUTF8(this->szLastError) ;

    //buildErrorReturn( _rc_, this->szLastError, outputMessage );
    return( INTERNAL_SERVER_ERROR );
  }else{
    outputMessage = "{\"" + strMemName + "\": \"deleted\" }";
  }

  T5LOG( T5INFO) <<"::deleteMem::success, memName = " << strMemName;
  _rc_ = OK;

  return( _rc_ );
}


int TagRemplacementRequestData::execute(){
  _rc_ = 0;
  std::string strSrcData, strTrgData, strReqData;
 
  strSrcData.reserve( strBody.size() + 1 );
  strTrgData.reserve( strBody.size() + 1 );
  strReqData.reserve( strBody.size() + 1 );   
  std::wstring wstr;

  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    wchar_t errMsg[] = L"Missing or incorrect JSON data in request body";
    wstr = errMsg;
    //buildErrorReturn( _rc_, errMsg, outputMessage );
    return _rest_rc_ = BAD_REQUEST;
  }else{ 
    std::string name;
    std::string value;
    while ( _rc_ == 0 )
    {
      _rc_ = json_factory.parseJSONGetNext( parseHandle, name, value );
      if ( _rc_ == 0 )
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
    json_factory.parseJSONStop( parseHandle );
  }

  auto result = std::make_unique<RequestTagReplacer>(std::move(EncodingHelper::convertToUTF16(strSrcData.c_str())), 
                                                     std::move(EncodingHelper::convertToUTF16(strTrgData.c_str())),
                                                     std::move(EncodingHelper::convertToUTF16(strReqData.c_str())));
                            
  //auto  =  replaceString( &_rc_);

  
  wstr = L"{\n ";
  std::wstring segmentLocations[] = {L"source", L"target", L"request"};

  wstr += L"\'source\' :\'" + result->getSrcWithTagsFromRequest() + L"\',\n ";
  wstr += L"\'target\' :\'" + result->getTrgWithTagsFromRequest() + L"\',\n ";
  wstr += L"\'request\' :\'" + result->getReqGenericTagStr() + L"\',\n ";

  wstr += L"\n}";
  outputMessage =  EncodingHelper::convertToUTF8(wstr);
  return _rest_rc_ = 200;
}

int CloneTMRequestData::parseJSON(){
  // parse json data
  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  std::string name, value;
  while ( _rc_ == 0 )
  {
    _rc_ = json_factory.parseJSONGetNext( parseHandle, name, value );
    if ( _rc_ == 0 ){
      if( strcasecmp (name.c_str(), "newName") == 0 ){
        newName = value;          
      }else{
        T5LOG(T5WARNING) << "::JSON parsed unused data: name = " << name << "; value = " << value;
      }
    }
  }
  if(_rc_ == 2002){
    _rc_ = 0;
  }
  if(_rc_){
    buildErrorReturn(_rc_, "Error during parsing json");
  }
  return _rc_;
}

int CloneTMRequestData::checkData(){
  if(!_rc_ && newName.empty()){
    std::string msg = "\'newName\' parameter was not provided or was empty";
    msg +=  "; for request for mem " + strMemName + "; with body = " + strBody ;
    
    return buildErrorReturn(-1, msg.c_str());
  }
  char memDir[255];
  if(!_rc_){
    _rc_ = Properties::GetInstance()->get_value(KEY_MEM_DIR, memDir, 254);
    if(_rc_){
      std::string msg = "can't read MEM_DIR path from properties";
      msg += outputMessage + "; for request for mem " + strMemName  + "; with body = "+ strBody ;
      
      return buildErrorReturn(-1, msg.c_str());
    }
  }
  if(!_rc_){
    srcTmdPath = memDir + strMemName + ".TMD";
    srcTmiPath = memDir + strMemName + ".TMI";
    dstTmdPath = memDir + newName + ".TMD";
    dstTmiPath = memDir + newName + ".TMI";
  }

  // check mem if exists
  if(!_rc_ && FilesystemHelper::FileExists(srcTmdPath.c_str()) == false){
    std::string msg = "\'srcTmdPath\' = " +  srcTmdPath + " not found";
    msg += "; for request for mem " + strMemName + "; with body = " + strBody ;
    
    return buildErrorReturn(-1, msg.c_str());
  }
  if(!_rc_ && FilesystemHelper::FileExists(srcTmiPath.c_str()) == false){
    std::string msg = "\'srcTmiPath\' = " +  srcTmiPath + " not found";
    msg += "; for request for mem " + strMemName  + "; with body = " + strBody ;
    
    return buildErrorReturn(-1, msg.c_str());
  }

  // check mem if is not in import state
  if(!_rc_){  
    
    if(mem == nullptr){
    // tm is probably not opened, buf files presence was checked before, so it should be "AVAILABLE" status
    //  outputMessage = "\'newName\' " + strMemName +" was not found in memory list";
    //  T5LOG(T5ERROR) << outputMessage << "; for request for mem "<< strMemName <<"; with body = ", strBody ;
    //  _rc_ = 500;
    }else{
      // close the memory - if open
      if(mem->eImportStatus == IMPORT_RUNNING_STATUS){
           std::string msg = "src tm \'" + strMemName +"\' is in import status. Repeat request later; for request for mem " 
            + strMemName + "; with body = " + strBody ;
          return buildErrorReturn(-1, msg.c_str());
      }else if ( mem->eStatus == OPEN_STATUS )
      {

      }else if(mem->eStatus != AVAILABLE_STATUS ){
         std::string msg = "src tm \'" + strMemName +"\' is not available nor opened; for request for mem " 
            + strMemName + "; with body = " + strBody ; 
        return buildErrorReturn(-1, msg.c_str());
      }
    }
  }

  // check if new name is available(is not occupied)
  if(!_rc_ && FilesystemHelper::FileExists(dstTmdPath.c_str()) == true){
    std::string msg = "\'dstTmdPath\' = " +  dstTmdPath + " already exists; for request for mem "
      + strMemName + "; with body = " + strBody ;
    return buildErrorReturn(-1, msg.c_str());
  }
  if(!_rc_ && FilesystemHelper::FileExists(dstTmiPath.c_str()) == true){
    std::string msg = "\'dstTmiPath\' = " +  dstTmiPath + " already exists" +"; for request for mem " 
      + strMemName + "; with body = " + strBody ;    
    return buildErrorReturn(-1, msg.c_str());
  }
  return _rc_;
}

int CloneTMRequestData::execute(){
  START_WATCH
  std::string msg;
  
  // clone .tmi and .tmd files   
  if(!_rc_ && (_rc_ = FilesystemHelper::CloneFile(srcTmdPath, dstTmdPath))){
    msg = "Can't clone file, _rc_ = " + toStr(_rc_)  + "; \'srcTmdPath\' = " + srcTmdPath + "; \'dstTmdPath\' = " +  dstTmdPath;
      + "; for request for mem " + strMemName + "; with body = " + strBody ;
    _rc_ = 501;
  }
  if(!_rc_ && (_rc_ = FilesystemHelper::CloneFile(srcTmiPath, dstTmiPath))){
    msg = "Can't clone file, _rc_ = " + toStr(_rc_)  + "; \'srcTmiPath\' = " + srcTmiPath + "; \'dstTmiPath\' = " +  dstTmiPath;
      + "; for request for mem " + strMemName + "; with body = " + strBody ;
    _rc_ = 501;
  }

  //if there was error- delete files
  if(_rc_ == 501){
    if(FilesystemHelper::FileExists(dstTmiPath.c_str())) FilesystemHelper::DeleteFile(dstTmiPath.c_str());
    if(FilesystemHelper::FileExists(dstTmdPath.c_str())) FilesystemHelper::DeleteFile(dstTmdPath.c_str());
    _rc_ = 500;
  }
  if(!_rc_){
    
    //EqfMemoryPlugin::GetInstance()->addMemoryToList(newName.c_str());
  }else{
    return buildErrorReturn(_rc_, msg.c_str());
  }
  if( mem != nullptr )
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

  if(_rc_ == 0 ){
    outputMessage = newName + " was cloned successfully";
    _rest_rc_ = 200;
  }

  outputMessage = "{\n\t\"msg\": \"" + outputMessage + "\",\n\t\"time\": \"" + watch.getString()+ "\n}"; 

  STOP_WATCH
  //PRINT_WATCH

  return _rc_;
}


std::string printTime(time_t time);
int StatusMemRequestData::checkData() {
  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "Missing name of memory" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* endif */
  return 0;
};



void AddRequestDataToJson(std::stringstream& ss, std::string reqType, milliseconds sumTime, size_t requestCount)
{
  ss << "\n[\n";
  std::string paramName = reqType + "ReqCount";
  AddToJson(ss, paramName.c_str(), requestCount, true);
  paramName = reqType + "SumTime(sec)";
  double sec = std::chrono::duration<double>(sumTime).count();
  AddToJson(ss, paramName.c_str(), sec, true);
  paramName = reqType + "AvrgReqTime";
  AddToJson(ss, paramName.c_str(), requestCount? sec/requestCount : 0, false);
  ss << "\n]";
}


int StatusMemRequestData::execute() {
  // check if memory is contained in our list
  if (  TMManager::GetInstance()->IsMemoryLoaded(strMemName) )
  {
    mem = TMManager::GetInstance()->requestServicePointer(strMemName, command);
    // set status value
    std::string pszStatus = "";
    switch ( mem->eImportStatus )
    {
      case IMPORT_RUNNING_STATUS: pszStatus = "import"; break;
      case REORGANIZE_RUNNING_STATUS: pszStatus = "reorganize"; break;
      case IMPORT_FAILED_STATUS: pszStatus = "import failed"; break;
      case REORGANIZE_FAILED_STATUS: pszStatus = "reorganize failed"; break;
      default: pszStatus = "available"; break;
    }
    // return the status of the memory
    json_factory.startJSON( outputMessage );
    json_factory.addParmToJSON( outputMessage, "status", "open" );
    if(mem->importDetails != nullptr){
      
      json_factory.addParmToJSON( outputMessage, mem->importDetails->fReorganize? "reorganizeStatus":"tmxImportStatus", pszStatus );
      json_factory.addParmToJSON( outputMessage, mem->importDetails->fReorganize? "reorganizeTime":"importProgress", mem->importDetails->usProgress );
      json_factory.addParmToJSON( outputMessage, mem->importDetails->fReorganize? "reorganizeTime":"importTime", mem->importDetails->importTimestamp );
      json_factory.addParmToJSON( outputMessage, mem->importDetails->fReorganize? "segmentsReorganized":"segmentsImported", mem->importDetails->segmentsImported );
      json_factory.addParmToJSON( outputMessage, "invalidSegments", mem->importDetails->invalidSegments );
      std::string invalidSegmRCs;
      for(auto [errCode, errCount]: mem->importDetails->invalidSegmentsRCs){
        invalidSegmRCs += std::to_string(errCode) + ":" + std::to_string(errCount) +"; ";
      }


      std::string firstInvalidSegments;
      int i=0;
      for(auto [segNum, invSegErrCode] : mem->importDetails->firstInvalidSegmentsSegNums){
        firstInvalidSegments += std::to_string(++i) + ":" + std::to_string(segNum) + ":" + std::to_string(invSegErrCode) + "; ";
      }
      json_factory.addParmToJSON( outputMessage, "invalidSegmentsRCs", invalidSegmRCs);
      json_factory.addParmToJSON( outputMessage, "firstInvalidSegments", firstInvalidSegments);
      json_factory.addParmToJSON( outputMessage, "invalidSymbolErrors", mem->importDetails->invalidSymbolErrors );
      json_factory.addParmToJSON( outputMessage, mem->importDetails->fReorganize? "reorganizeErrorMsg":"importErrorMsg", mem->importDetails->importMsg.str() );
    }
    json_factory.addParmToJSON( outputMessage, "lastAccessTime", printTime(mem->tLastAccessTime) );
    json_factory.addParmToJSON( outputMessage, "creationTime", printTime(mem->stTmSign.creationTime) );
    std::string creationT5MVersion = std::to_string(mem->stTmSign.bGlobVersion) 
                              + ":"+ std::to_string(mem->stTmSign.bMajorVersion) 
                              + ":" + std::to_string(mem->stTmSign.bMinorVersion);
    json_factory.addParmToJSON( outputMessage, "tmCreatedInT5M_version", creationT5MVersion.c_str() );
    if ( ( (mem->eImportStatus == IMPORT_FAILED_STATUS) 
        || (mem->eImportStatus == REORGANIZE_FAILED_STATUS)) && ( !mem->strError.empty() ) )
    {
      json_factory.addParmToJSON( outputMessage, "ErrorMsg", mem->strError.c_str() );
    }
    json_factory.terminateJSON( outputMessage );
    return( OK );
  } /* endif */

  // check if memory exists
  if(int res = TMManager::GetInstance()->TMExistsOnDisk(strMemName))
  {
    json_factory.startJSON( outputMessage );
    json_factory.addParmToJSON( outputMessage, "status", "not found" );
    json_factory.addParmToJSON( outputMessage, "res", res);
    json_factory.terminateJSON( outputMessage );
    return( NOT_FOUND );
  }

  json_factory.startJSON( outputMessage );
  json_factory.addParmToJSON( outputMessage, "status", "available" );
  json_factory.terminateJSON( outputMessage );
  return( OK );
};

int ShutdownRequestData::execute(){
    if(pfWriteRequestsAllowed) *pfWriteRequestsAllowed = false;
    //pMemService->closeAll();
    T5Logger::GetInstance()->LogStop();  
    TMManager::GetInstance()->fServiceIsRunning = true;
    
    std::thread([this]() 
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

      TMManager::GetInstance()->fServiceIsRunning = false;
      auto saveTmRD = SaveAllTMsToDiskRequestData();
      saveTmRD.run();
      //check tms is in import status
      //close log file
      if(saveTmRD._rest_rc_ == 200){
        T5Logger::GetInstance()->LogStop();
      }
      if(sig != SHUTDOWN_CALLED_FROM_MAIN){
        exit(sig);
      }
    }).detach();
    return 200;
  
}

/* write a single proposal to a JSON string
\param strJSON JSON stirng receiving the proposal data
\param pProp pointer to a MEMPROPOSAL containing the proposal
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
/*
int addProposalToJSONString
(
  std::wstring &strJSON,
  OtmProposal& Data,
  bool fAddFuzzyInfo = false
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  wchar_t wbuff [OTMPROPOSAL_MAXSEGLEN+1];

  pJsonFactory->addElementStartToJSONW( strJSON );

  pJsonFactory->addParmToJSONW( strJSON, L"source", Data.szSource );
  pJsonFactory->addParmToJSONW( strJSON, L"target", Data.szTarget );
  pJsonFactory->addParmToJSONW( strJSON, L"segmentNumber", Data.lSegmentNum );
  pJsonFactory->addParmToJSONW( strJSON, L"id", EncodingHelper::convertToUTF16(Data.szId).c_str());
  pJsonFactory->addParmToJSONW( strJSON, L"documentName", EncodingHelper::convertToUTF16(Data.szDocName).c_str() );
  EqfGetIsoLang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szSourceLanguage, Data.szIsoSourceLang );
  pJsonFactory->addParmToJSONW( strJSON, L"sourceLang", EncodingHelper::convertToUTF16( Data.szIsoSourceLang ).c_str() );
  EqfGetIsoLang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szTargetLanguage, Data.szIsoTargetLang );
  pJsonFactory->addParmToJSONW( strJSON, L"targetLang", EncodingHelper::convertToUTF16( Data.szIsoTargetLang ).c_str() );

  wbuff[0] = '\0';
  switch ( Data.eType )
  {
    case GLOBMEMORY_PROPTYPE: wcscpy( wbuff , L"GlobalMemory" ); break;
    case GLOBMEMORYSTAR_PROPTYPE: wcscpy( wbuff , L"GlobalMemoryStar" ); break;
    case MACHINE_PROPTYPE: wcscpy( wbuff , L"MachineTranslation" ); break;
    case MANUAL_PROPTYPE: wcscpy( wbuff , L"Manual" ); break;
    default: wcscpy( wbuff , L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"type", wbuff  );
  
  if(fAddFuzzyInfo){
    wbuff[0] = '\0';
    switch ( Data.eMatch )
    {
      case EXACT_MATCHTYPE: wcscpy( wbuff , L"Exact" ); break;
      case EXACTEXACT_MATCHTYPE: wcscpy( wbuff , L"ExactExact" ); break;
      case EXACTSAMEDOC_MATCHTYPE: wcscpy( wbuff , L"ExactSameDoc" ); break;
      case FUZZY_MATCHTYPE: wcscpy( wbuff , L"Fuzzy" ); break;
      case REPLACE_MATCHTYPE: wcscpy( wbuff , L"Replace" ); break;
      default: wcscpy( wbuff , L"undefined" ); break;
    }
    pJsonFactory->addParmToJSONW( strJSON, L"matchType", wbuff  );
  }
  MultiByteToWideChar( CP_OEMCP, 0, Data.szTargetAuthor, -1, wbuff , sizeof( wbuff  ) / sizeof( wbuff [0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"author", wbuff  );

  convertTimeToUTC( Data.lTargetTime, Data.szDateTime );
  MultiByteToWideChar( CP_OEMCP, 0, Data.szDateTime, -1, wbuff , sizeof( wbuff  ) / sizeof( wbuff [0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"timestamp", wbuff  );
  
  if(fAddFuzzyInfo){
    pJsonFactory->addParmToJSONW( strJSON, L"matchRate", Data.iFuzziness );
    pJsonFactory->addParmToJSONW( strJSON, L"fuzzyWords", Data.iWords );
    pJsonFactory->addParmToJSONW( strJSON, L"fuzzyDiffs", Data.iDiffs );
  }

  MultiByteToWideChar( CP_OEMCP, 0, Data.szMarkup, -1, wbuff , sizeof( wbuff  ) / sizeof( wbuff [0] ) );
  pJsonFactory->addParmToJSONW( strJSON, L"markupTable", wbuff  );

  pJsonFactory->addParmToJSONW( strJSON, L"context", Data.szContext );

  pJsonFactory->addParmToJSONW( strJSON, L"additionalInfo", Data.szAddInfo );

  Data.getInternalKey(buff, OTMPROPOSAL_MAXSEGLEN);
  pJsonFactory->addParmToJSONW( strJSON, L"internalKey", EncodingHelper::convertToUTF16(buff).c_str() );

  pJsonFactory->addElementEndToJSONW( strJSON );

  return( 0 );
}
//*/


int addProposalToJSONString
(
  std::wstring &strJSON,
  OtmProposal& Data,
  bool fAddFuzzyInfo = false
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();

  pJsonFactory->addElementStartToJSONW( strJSON );
  char buff[OTMPROPOSAL_MAXSEGLEN+1];
  wchar_t wbuff [OTMPROPOSAL_MAXSEGLEN+1];
  if(Data.pInputSentence){
    pJsonFactory->addParmToJSONW( strJSON, L"source", Data.pInputSentence->pStrings->getGenericTagStrC() );
    pJsonFactory->addParmToJSONW( strJSON, L"sourceNPRepl", Data.pInputSentence->pStrings->getNormStrC() );
    pJsonFactory->addParmToJSONW( strJSON, L"sourceNorm", Data.pInputSentence->pStrings->getNormStrC() );
    pJsonFactory->addParmToJSONW( strJSON, L"target", Data.pInputSentence->pStrings->getGenericTargetStrC() ); 
  }else{
    pJsonFactory->addParmToJSONW( strJSON, L"source", Data.szSource );
    pJsonFactory->addParmToJSONW( strJSON, L"target", Data.szTarget ); 
  } 
  pJsonFactory->addParmToJSONW( strJSON, L"segmentNumber", Data.getSegmentNum() );
  pJsonFactory->addParmToJSONW( strJSON, L"id", Data.szId);
  pJsonFactory->addParmToJSONW( strJSON, L"documentName", Data.szDocName );
  pJsonFactory->addParmToJSONW( strJSON, L"sourceLang", Data.szSourceLanguage );
  pJsonFactory->addParmToJSONW( strJSON, L"targetLang", Data.szTargetLanguage );

  wbuff[0] = '\0';
  switch ( Data.getType() )
  {
    case OtmProposal::eptGlobalMemory: wcscpy( wbuff, L"GlobalMemory" ); break;
    case OtmProposal::eptGlobalMemoryStar: wcscpy( wbuff , L"GlobalMemoryStar" ); break;
    case OtmProposal::eptMachine: wcscpy( wbuff , L"MachineTranslation" ); break;
    case OtmProposal::eptManual: wcscpy( wbuff , L"Manual" ); break;
    default: wcscpy( wbuff , L"undefined" ); break;
  }
  pJsonFactory->addParmToJSONW( strJSON, L"type", wbuff  );

  pJsonFactory->addParmToJSONW( strJSON, L"author", Data.szTargetAuthor);
  pJsonFactory->addParmToJSONW( strJSON, L"timestamp", Data.szDateTime);
  pJsonFactory->addParmToJSONW( strJSON, L"markupTable", Data.szMarkup);
  pJsonFactory->addParmToJSONW( strJSON, L"context", Data.szContext);
  pJsonFactory->addParmToJSONW( strJSON, L"additionalInfo", Data.szAddInfo );
  Data.getInternalKey(buff, OTMPROPOSAL_MAXSEGLEN);
  pJsonFactory->addParmToJSONW( strJSON, L"internalKey", EncodingHelper::convertToUTF16(buff).c_str() );
  
  if(fAddFuzzyInfo){
    wbuff[0] = '\0';
    switch ( Data.getMatchType() )
    {
      case OtmProposal::emtExact: wcscpy( wbuff , L"Exact" ); break;
      case OtmProposal::emtExactExact: wcscpy( wbuff , L"ExactExact" ); break;
      case OtmProposal::emtExactSameDoc: wcscpy( wbuff , L"ExactSameDoc" ); break;
      case OtmProposal::emtFuzzy: wcscpy( wbuff , L"Fuzzy" ); break;
      case OtmProposal::emtReplace: wcscpy( wbuff , L"Replace" ); break;
      default: wcscpy( wbuff , L"undefined" ); break;
    }
    pJsonFactory->addParmToJSONW( strJSON, L"matchType", wbuff  );
    pJsonFactory->addParmToJSONW( strJSON, L"matchRate", Data.getFuzziness() );
    pJsonFactory->addParmToJSONW( strJSON, L"fuzzyWords", Data.getWords() );
    pJsonFactory->addParmToJSONW( strJSON, L"fuzzyDiffs", Data.getDiffs() );
  }

  pJsonFactory->addElementEndToJSONW( strJSON );

  return( 0 );
}


int ResourceInfoRequestData::execute(){
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
  
  size_t total = 0, fSize = 0, count = 0;
  std::string fName;
  ssOutput << "\"tms\": [\n";
  for ( auto tm : TMManager::GetInstance()->tms)
  {
    if(count){
      ssOutput << ",";
    }
    fSize = tm.second->GetRAMSize();
    fName = tm.first;

    ssOutput << "\n{ ";
    AddToJson(ssOutput, "name", tm.first, true );
    AddToJson(ssOutput, "size", fSize, false );
    ssOutput << " }";

    total += fSize;
    count++;
  }   

  ssOutput << "\n ],\n"; 
  AddToJson(ssOutput, "totalOccupiedByTMsInRAM", total, true );
  
  //in addition the memory that is used by the service by other means
  // it's better to use system calls

  //the currently configured max usable memory
  int availableRam = 0, threshold = 0, workerThreads = 0, timeout = 0;
  char buff[255];
  //Properties::GetInstance()->get_value(KEY_OTM_DIR, szOtmDirPath);
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, availableRam);// saving in megabytes to avoid int overflow
  Properties::GetInstance()->get_value(KEY_TRIPLES_THRESHOLD, threshold);
  Properties::GetInstance()->get_value(KEY_NUM_OF_THREADS, workerThreads);
  Properties::GetInstance()->get_value(KEY_TIMEOUT_SETTINGS, timeout);
  Properties::GetInstance()->get_value(KEY_RUN_DATE, buff,254);
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

  if(pStats != nullptr){
    milliseconds sumTime;
    size_t requestCount;

    {
      ssOutput << "\"Requests\": {\n";
      AddToJson(ssOutput, "RequestCount", pStats->getRequestCount(), true );

      double sec = std::chrono::duration<double>(pStats->getExecutionTime()).count();
      AddToJson(ssOutput, "RequestExecutionSumTime(sec)", sec, true);

      requestCount = pStats->getCreateMemRequestCount();
      sumTime = pStats->getCreateMemSumTime();
      AddRequestDataToJson(ssOutput, "CreateMem", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getDeleteEntryRequestCount();
      sumTime = pStats->getDeleteEntrySumTime();
      AddRequestDataToJson(ssOutput, "DeleteMem", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getImportMemRequestCount();
      sumTime = pStats->getImportMemSumTime();
      AddRequestDataToJson(ssOutput, "ImportMem", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getExportMemRequestCount();
      sumTime = pStats->getExportMemSumTime();
      AddRequestDataToJson(ssOutput, "ExportMem", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getCloneLocalyCount();
      sumTime = pStats->getCloneLocalySumTime();
      AddRequestDataToJson(ssOutput, "CloneTmLocaly", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getReorganizeRequestCount();
      sumTime = pStats->getReorganizeExecutionTime();
      AddRequestDataToJson(ssOutput, "Reorganize", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getStatusMemRequestCount();
      sumTime = pStats->getStatusMemSumTime();
      AddRequestDataToJson(ssOutput, "StatusMem", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getFuzzyRequestCount();
      sumTime = pStats->getFuzzySumTime();
      AddRequestDataToJson(ssOutput, "Fuzzy", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getConcordanceRequestCount();
      sumTime = pStats->getConcordanceSumTime();
      AddRequestDataToJson(ssOutput, "Concordance", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getUpdateEntryRequestCount();
      sumTime = pStats->getUpdateEntrySumTime();
      AddRequestDataToJson(ssOutput, "UpdateEntry", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getDeleteEntryRequestCount();
      sumTime = pStats->getDeleteEntrySumTime();
      AddRequestDataToJson(ssOutput, "DeleteEntry", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getReorganizeRequestCount();
      sumTime = pStats->getReorganizeExecutionTime();
      AddRequestDataToJson(ssOutput, "Reorganize", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getSaveAllTmsRequestCount();
      sumTime = pStats->getSaveAllTmsSumTime();
      AddRequestDataToJson(ssOutput, "SaveAllTms", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getListOfMemoriesRequestCount();
      sumTime = pStats->getListOfMemoriesSumTime();
      AddRequestDataToJson(ssOutput, "ListOfMemories", sumTime, requestCount);
      ssOutput <<",";
      
      requestCount = pStats->getResourcesRequestCount();
      sumTime = pStats->getResourcesSumTime();
      AddRequestDataToJson(ssOutput, "Resources", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getOtherRequestCount();
      sumTime = pStats->getOtherSumTime();
      AddRequestDataToJson(ssOutput, "Other", sumTime, requestCount);
      ssOutput <<",";

      requestCount = pStats->getUnrecognizedRequestCount();
      sumTime = pStats->getUnrecognizedSumTime();
      AddRequestDataToJson(ssOutput, "Unrecognized", sumTime, requestCount);
      ssOutput << "\n },\n"; 
    }
  
  }

  
  //AddToJson(ssOutput, "Log level(internal)", GetLogLevel(), true );
  //AddToJson(ssOutput, "CPU used by process", getCurrentCPUUsageByProcess(), true);
  //AddToJson(ssOutput, "VirtualMem used by process(in KB)", getVirtualMemUsageKBValue(), true);
  
  AddToJson(ssOutput, "RAM limit(MB)", availableRam, false );

  //close json
  ssOutput << "\n}";

  outputMessage = ssOutput.str();
  _rest_rc_ = 200;
  return _rest_rc_;
}

int ExportRequestData::checkData(){
  T5LOG( T5INFO) <<"::getMem::=== getMem request, memory = " << strMemName << "; format = " << requestAcceptHeader;
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    return buildErrorReturn(_rc_ , "::getMem::Error: no valid API session" );
  } /* endif */

  if ( strMemName.empty() )
  {
    return buildErrorReturn(_rc_ , "::getMem::Error: no memory name specified" );
  } /* endif */


  if(!_rc_)
  {
    int res =  TMManager::GetInstance()->TMExistsOnDisk(strMemName, true );
    if ( res != TMManager::TMM_NO_ERROR )
    {
      std::string errMsg = ":getMem::Error: memory does not exist, memName = " + strMemName + "; res= " + std::to_string(res);
      buildErrorReturn(res, errMsg.c_str()); 
      return( _rest_rc_ =  NOT_FOUND );
    }
  }
  return _rc_;
}

int ExportRequestData::execute(){
  // get a temporary file name for the memory package file or TMX file
  strTempFile = FilesystemHelper::BuildTempFileName();
  if ( _rc_ != 0 )
  {
    buildErrorReturn(_rc_, "::getMem:: Error: creation of temporary file for memory data failed");
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }
  // export the memory in internal format
  if ( requestAcceptHeader.compare( "application/xml" ) == 0 )
  {
    T5LOG( T5INFO) <<"::getMem:: mem = " <<  strMemName << "; supported type found application/xml, tempFile = " << strTempFile;
    ExportTmx();
    if ( _rc_ != 0 )
    {
      //unsigned short usRC = 0;
      //EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      std::string strErrMsg = "::getMem:: Error: EqfExportMem failed with rc=" + std::to_string(_rc_) + ", error message is " ;//  EncodingHelper::convertToUTF8( this->szLastError);
      buildErrorReturn(_rc_, strErrMsg.c_str());
      return( _rest_rc_ = INTERNAL_SERVER_ERROR );
    }
  }
  else if ( requestAcceptHeader.compare( "application/zip" ) == 0 )
  {
    T5LOG( T5INFO) <<"::getMem:: mem = "<< strMemName << "; supported type found application/zip(NOT TESTED YET), tempFile = " << strTempFile;
    ExportZip();
    if ( _rc_ != 0 )
    {
      //unsigned short usRC = 0;
      //EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      std::string strErrMsg = "::getMem:: Error: EqfExportMemInInternalFormat failed with rc=" + std::to_string(_rc_) + ", error message is " ;//  EncodingHelper::convertToUTF8( this->szLastError);
      buildErrorReturn(_rc_, strErrMsg.c_str());
      return( _rest_rc_ = INTERNAL_SERVER_ERROR );
    }
  }
  else
  {
    std::string strErrMsg = "::getMem:: Error: the type " + requestAcceptHeader + " is not supported" ;
    buildErrorReturn(_rc_, strErrMsg.c_str());
    return( _rest_rc_ = NOT_ACCEPTABLE );
  }

  // fill the vMemData vector with the content of zTempFile
  std::vector<unsigned char> vMemData;
  _rc_ = FilesystemHelper::LoadFileIntoByteVector( strTempFile, vMemData );
  outputMessage = std::string(vMemData.begin(), vMemData.end());
  //cleanup
  if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
    FilesystemHelper::DeleteFile( strTempFile );
  }
  
  if ( _rc_ != 0 )
  {
    std::string strErrMsg = "::getMem::  Error: failed to load the temporary file, fName = " +  strTempFile ;
    buildErrorReturn(_rc_, strErrMsg.c_str());
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }

  return( OK );

}

int ExportRequestData::ExportZip(){
  // check if memory exists
  if(_rc_ = TMManager::GetInstance()->TMExistsOnDisk(strMemName) != NO_ERROR){
    return _rc_;
  }
  // add the files to the package
  ZIP* pZip = FilesystemHelper::ZipOpen( strTempFile , 'w' );
  FilesystemHelper::ZipAdd( pZip, FilesystemHelper::GetTmdPath(strMemName) );
  FilesystemHelper::ZipAdd( pZip, FilesystemHelper::GetTmiPath(strMemName) );  
  FilesystemHelper::ZipClose( pZip );

  return _rc_;
}

int ExportRequestData::ExportTmx(){
  
  //mem
  // call TM export
  if ( _rc_ == NO_ERROR )
  {
    memset( &fctdata, 0, sizeof( FCTDATA ) );
    fctdata.fComplete = TRUE;
    fctdata.usExportProgress = 0;
    _rc_ = fctdata.MemFuncPrepExport( (PSZ)strMemName.c_str(), (PSZ)strTempFile.c_str(), TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT, mem );
  } 

  if ( _rc_ == 0 )
  {
    while ( !fctdata.fComplete )
    {
      _rc_ = fctdata.MemFuncExportProcess(  );
    }
  }
  if(_rc_){
    T5LOG(T5ERROR) <<  "end of function EqfExportMem with error code::  RC = " << _rc_;
  }else{ 
    T5LOG( T5DEBUG) << "end of function EqfExportMem::success ";
  }
  return 0;
}

int UpdateEntryRequestData::parseJSON(){
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    return buildErrorReturn( BAD_REQUEST, "can't verifyAPISession" );
  } /* endif */

  if ( strMemName.empty() )
  {
    return buildErrorReturn( BAD_REQUEST, "Missing name of memory");
  } /* endif */

  // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  // parse input parameters
  Data.clearProposal();    
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szSource ), sizeof( Data.szSource ) / sizeof( Data.szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szTarget ), sizeof( Data.szTarget ) / sizeof( Data.szTarget[0] ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoSourceLang ), sizeof( Data.szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoTargetLang ), sizeof( Data.szIsoTargetLang ) },  

  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( Data.lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDocName ), sizeof( Data.szDocName ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szType ), sizeof( Data.szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szTargetAuthor ), sizeof( Data.szTargetAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szMarkup ), sizeof( Data.szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szContext ), sizeof( Data.szContext ) / sizeof( Data.szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDateTime ), sizeof( Data.szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szAddInfo ), sizeof( Data.szAddInfo ) / sizeof( Data.szAddInfo[0] ) },
  { L"loggingThreshold",JSONFactory::INT_PARM_TYPE        , &(loggingThreshold), 0},
  { L"save2disk",      JSONFactory::INT_PARM_TYPE         , &(fSave2Disk), 0 },
  { L"recordKey",      JSONFactory::INT_PARM_TYPE         , &(Data.recordKey), 0 },
  { L"targetKey",      JSONFactory::INT_PARM_TYPE         , &(Data.targetKey), 0 },
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  _rc_ = json_factory.parseJSON( strInputParmsW, parseControl );

  if ( _rc_ != 0 )
  {
    buildErrorReturn( ERROR_INTERNALFUNCTION_FAILED, "Error: Parsing of input parameters failed" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* end */

  return 0;
}

int UpdateEntryRequestData::checkData(){
  if ( Data.szSource[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source text" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  if ( Data.szTarget[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing target text" );
    return( _rest_rc_ =  BAD_REQUEST );
  } /* end */
  if ( Data.szIsoSourceLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID,  "Error: Missing source language");
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  if ( Data.szIsoTargetLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing target language" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* end */
  if ( Data.szMarkup[0] == 0 )
  {
    strcpy( Data.szMarkup, "OTMXUXLF");
  } /* end */

  if(loggingThreshold >=0){
    T5LOG( T5WARNING) <<"updateEntry::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold); 
  }
  return 0;
}


void copyMemProposalToOtmProposal( PMEMPROPOSAL pProposal, OtmProposal *pOtmProposal );

int UpdateEntryRequestData::execute(){
  // prepare the proposal data
  if(Data.szSource && Data.szTarget){
    Data.pInputSentence  = new TMX_SENTENCE(std::make_unique<StringTagVariants> (Data.szSource , Data.szTarget));
  }
  if(!_rc_ && Data.pInputSentence->pStrings->isParsed()){
  }else{
    buildErrorReturn(_rc_, "Error in xml in source or target!");
    return _rc_;
  }

  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoSourceLang, Data.szSourceLanguage );
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoTargetLang, Data.szTargetLanguage );

  if(Data.szSourceLanguage[0] == '\0' ){
    buildErrorReturn(404, "Error in input - srcLang is not supported!");
    return _rc_ = 404;
  }
  if(Data.szTargetLanguage[0] == '\0' ){
    buildErrorReturn(404, "Error in input - trgLang is not supported!");
    return _rc_ = 404;
  }
  
  Data.eType = getMemProposalType(Data.szType );

  if (Data.szDateTime[0] == 0 )
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    time( (time_t*)&Data.lTargetTime );
    Data.lTargetTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    convertTimeToUTC(Data.lTargetTime, Data.szDateTime);
  }else{
    // use provided time stamp
    convertUTCTimeToLong(Data.szDateTime, &(Data.lTargetTime) );
  }
  // update the memory
  TMX_EXT_OUT_W TmPutOut;                      // ptr to TMX_EXT_OUT_W structure
  memset( &TmPutOut, 0, sizeof(TMX_EXT_OUT_W) );

  if(T5Logger::GetInstance()->CheckLogLevel(T5INFO)){
    std::string source = EncodingHelper::convertToUTF8(Data.szSource);
    T5LOG( T5INFO) <<"EqfMemory::putProposal, source = " << source;
  }
  /********************************************************************/
  /* fill the TMX_PUT_IN prefix structure                             */
  /* the TMX_PUT_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  //if(Data.recordKey && Data.targetKey){
  //  _rc_ = mem->TmtXUpdSeg(  &Data, &TmPutOut, 0 );
  //}else{
    _rc_ = mem->TmtXReplace ( Data, &TmPutOut );
  //}
  if ( _rc_ != 0 ){
      return buildErrorReturn(_rc_, "EqfMemory::putProposal result is error ");   
      //handleError( _rc_, this->szName, TmPutIn.stTmPut.szTagTable );
  }else if(fSave2Disk){
    mem->FlushFilebuffers();
  }

  if ( ( _rc_ == 0 ) &&
       ( Data.fMarkupChanged ) ) {
    return buildErrorReturn(SEG_RESET_BAD_MARKUP, "SEG_RESET_BAD_MARKUP") ;
  }
  
  if ( _rc_ != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC,Data.szError, sizeof(Data.szError ) / sizeof(Data.szError[0] ) );
    buildErrorReturn( _rc_,Data.szError );
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  std::wstring outputMessageW;
  json_factory.startJSONW( outputMessageW );
  
  //json_factory.addParmToJSON( outputMessage, "sourceLang",Data.szSourceLanguage );
  //json_factory.addParmToJSON( outputMessage, "targetLang",Data.szTargetLanguage );

  //json_factory.addParmToJSONW( outputMessageW, L"source",Data.szSource );
  //json_factory.addParmToJSONW( outputMessageW, L"target",Data.szTarget );
  //outputMessage += ",\n" + EncodingHelper::convertToUTF8(outputMessageW.c_str());// +", ";

  /*
  json_factory.addParmToJSON( outputMessage, "documentName", Data.szDocName );
  json_factory.addParmToJSON( outputMessage, "segmentNumber", Data.lSegmentNum );
  json_factory.addParmToJSON( outputMessage, "internalKey", Data.szInternalKey);
  json_factory.addParmToJSON( outputMessage, "markupTable", Data.szMarkup );
  json_factory.addParmToJSON( outputMessage, "timeStamp", Data.szDateTime );
  json_factory.addParmToJSON( outputMessage, "author", Data.szTargetAuthor );
  //*/
  addProposalToJSONString(outputMessageW, Data);
  json_factory.terminateJSONW( outputMessageW );
  outputMessage = EncodingHelper::convertToUTF8(outputMessageW.c_str());

  //if(!inputSentence) delete inputSentence;
  return( _rc_ );
}





int DeleteEntryRequestData::parseJSON(){
  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "Missing name of memory" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* endif */
    // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  // parse input parameters
  Data.clearProposal();

  auto loggingThreshold = -1;
       
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szSource ), sizeof( Data.szSource ) / sizeof( Data.szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szTarget ), sizeof( Data.szTarget ) / sizeof( Data.szTarget[0] ) },
  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( Data.lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDocName ), sizeof( Data.szDocName ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoSourceLang ), sizeof( Data.szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoTargetLang ), sizeof( Data.szIsoTargetLang ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szType ), sizeof( Data.szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szTargetAuthor ), sizeof( Data.szTargetAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szMarkup ), sizeof( Data.szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szContext ), sizeof( Data.szContext ) / sizeof( Data.szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDateTime ), sizeof( Data.szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szAddInfo ), sizeof( Data.szAddInfo ) / sizeof( Data.szAddInfo[0] ) },
  { L"loggingThreshold",JSONFactory::INT_PARM_TYPE        , &(loggingThreshold), 0},
  { L"recordKey",      JSONFactory::INT_PARM_TYPE         , &(Data.recordKey), 0 },
  { L"targetKey",      JSONFactory::INT_PARM_TYPE         , &(Data.targetKey), 0 },
  { L"save2disk",      JSONFactory::INT_PARM_TYPE         , &(fSave2Disk), 0},
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  _rc_ = json_factory.parseJSON( strInputParmsW, parseControl );  
  if ( _rc_ )
  {
    buildErrorReturn( _rc_, "Error during parsing json: Parsing of input parameters failed" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* endif */
  return( _rc_ );
}

int DeleteEntryRequestData::checkData(){
  if(Data.recordKey || Data.targetKey){
    if(Data.recordKey && Data.targetKey){
      return 0;
    }else{
      buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: to delete entry by key you should point both recordKey and targetKey" );
      return( BAD_REQUEST );
    }
  }
  
  if ( Data.szSource[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source text" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szTarget[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing target text" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szIsoSourceLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source language" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szIsoTargetLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing target language" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szMarkup[0] == 0 || strcasecmp( Data.szMarkup, "translate5" ) == 0)
  {
    strcpy(Data.szMarkup, "OTMXUXLF");
  } /* end */
  
  if(loggingThreshold >=0){
    T5LOG( T5WARNING) <<"::updateEntry::set new threshold for logging" <<loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold); 
  }  
  return 0;
}

int DeleteEntryRequestData::execute(){
  auto hSession = OtmMemoryServiceWorker::getInstance()->hSession;
  // prepare the proposal data
  if ( Data.szDateTime[0] != 0 )
  {
    // use provided time stamp
    convertUTCTimeToLong( Data.szDateTime, &(Data.lTargetTime) );
  }
  else
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    LONG            lTimeStamp;             // buffer for current time
    time( (time_t*)&lTimeStamp );
    lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    convertTimeToUTC( lTimeStamp, Data.szDateTime );
  }

  std::string errorStr;
  //errorStr.reserve(1000);
  // update the memory delete entry
  
  TMX_EXT_OUT_W TmPutOut;
  //bool internalKey = false;
  
  memset( &TmPutOut, 0, sizeof(TmPutOut) );
  
  if(Data.recordKey && Data.targetKey){
    //internalKey = true;
    _rc_ = mem->TmtXDelSegmByKey(Data, &TmPutOut);
  }else{
    EqfGetOpenTM2Lang( hSession, Data.szIsoSourceLang, Data.szSourceLanguage );
    EqfGetOpenTM2Lang( hSession, Data.szIsoTargetLang, Data.szTargetLanguage );
    Data.eType = getMemProposalType( Data.szType );
    if ( !_rc_ ){ 
      Data.pInputSentence = new TMX_SENTENCE(std::make_shared<StringTagVariants>(Data.szSource, Data.szTarget));
      _rc_ = mem->TmtXDelSegm ( Data, &TmPutOut );
    }
  }

  if(_rc_ == 6020){
    //seg not found
    errorStr = "Segment not found";
  }


  std::wstring strOutputParmsW;
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, errorStr.c_str() );
    return( INTERNAL_SERVER_ERROR );
  } else{ 
    if(fSave2Disk){
      mem->FlushFilebuffers();
    }
    // return the entry data
    json_factory.addParmToJSONW( strOutputParmsW, L"fileFlushed", fSave2Disk );
    json_factory.addNameToJSONW( strOutputParmsW, L"results" );

    OtmProposal output;
    mem->ExtOutToOtmProposal(&TmPutOut, output);
    time( (time_t*)&output.lTargetTime );
    output.lTargetTime -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    convertTimeToUTC(output.lTargetTime, output.szDateTime);
    addProposalToJSONString( strOutputParmsW, output );
  }


  
  outputMessage = EncodingHelper::convertToUTF8( strOutputParmsW );
  _rc_ = OK;
  return _rc_;
}

int FuzzySearchRequestData::parseJSON(){
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, "can't verifyAPISession" );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */

    // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  
  Data.clearProposal();
  int loggingThreshold = -1;
  JSONFactory::JSONPARSECONTROL parseControl[] = { { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szSource ), sizeof( Data.szSource ) / sizeof( Data.szSource[0] ) },
                                                   { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( Data.lSegmentNum ), 0 },
                                                   { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDocName ), sizeof( Data.szDocName ) },
                                                   { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoSourceLang ), sizeof( Data.szIsoSourceLang ) },
                                                   { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoTargetLang ), sizeof( Data.szIsoTargetLang ) },
                                                   { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szMarkup ), sizeof( Data.szMarkup ) },
                                                   { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szContext ), sizeof( Data.szContext ) / sizeof( Data.szContext[0] ) },
                                                   { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( Data.iNumOfProposals ), 0 },
                                                   { L"loggingThreshold", JSONFactory::INT_PARM_TYPE,        &loggingThreshold, 0 },
                                                   { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  _rc_ = json_factory.parseJSON( strInputParmsW, parseControl );
  return _rc_;
}

int FuzzySearchRequestData::checkData(){
  if ( _rc_ != 0 )
  {
    buildErrorReturn( ERROR_INTERNALFUNCTION_FAILED, "Error: Parsing of input parameters failed" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* end */

  if ( Data.szSource[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source text input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */

  if ( Data.szIsoSourceLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing source language input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  
  if ( Data.szIsoTargetLang[0] == 0 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Missing target language input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  
  if ( Data.szMarkup[0] == 0 )
  {
    T5LOG( T5INFO) <<"::search::No markup requested -> using OTMXUXLF";
    // use default markup table if none given
    strcpy( Data.szMarkup, "OTMXUXLF" );
  } /* end */
  
  if ( Data.iNumOfProposals > 20 )
  {
    buildErrorReturn( ERROR_INPUT_PARMS_INVALID, "Error: Too many proposals requested, the maximum value is 20" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */

  
  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"::search::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold);
  }

  return _rc_;
}


int addProposalsToJSONString
(
  std::wstring &strJSON,
  std::vector<OtmProposal>& vProposals,
  int iNumOfProposals,
  //OtmProposal* pData,
  bool fAddFuzzyInfo = false
)
{
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  
  pJsonFactory->addArrayStartToJSONW( strJSON );
  for ( int i = 0; i < iNumOfProposals; i++ )
  {
    addProposalToJSONString( strJSON, vProposals[i], fAddFuzzyInfo);
  } /* endfor */
  pJsonFactory->addArrayEndToJSONW( strJSON  );
  
  return( 0 );
}


int FuzzySearchRequestData::execute(){
 // prepare the memory lookup
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoSourceLang, Data.szSourceLanguage, &Data.fIsoSourceLangIsPrefered );
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoTargetLang, Data.szTargetLanguage, &Data.fIsoTargetLangIsPrefered );

  if ( Data.iNumOfProposals == 0 )
  {
    //Data.iNumOfProposals = std::min( 5 * (int)sourceLangs.size(), 20);
    Data.iNumOfProposals = 5;
  }

  //std::vector<MEMPROPOSAL> vFoundProposals(Data.iNumOfProposals);
  //memset( &vFoundProposals[0], 0, sizeof( MEMPROPOSAL ) * Data.iNumOfProposals );
  // do the lookup and handle the results
  // call the memory factory to process the request

  std::vector<OtmProposal> vProposals(Data.iNumOfProposals);
  if ( _rc_ == NO_ERROR )
  {
    TMX_GET_IN_W GetIn;
    TMX_GET_OUT_W GetOut;
    memset( &GetIn, 0, sizeof(TMX_GET_IN_W) );
    memset( &GetOut, 0, sizeof(TMX_GET_OUT_W) );

    mem->OtmProposalToGetIn( Data, &GetIn );
    GetIn.stTmGet.usConvert = MEM_OUTPUT_ASIS;
    GetIn.stTmGet.usRequestedMatches = (USHORT)vProposals.size();
    GetIn.stTmGet.ulParm = GET_EXACT;
    GetIn.stTmGet.pvGMOptList = mem->pvGlobalMemoryOptions;

    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
      auto str = EncodingHelper::convertToUTF8(GetIn.stTmGet.szSource);
      T5LOG( T5DEBUG) <<"EqfMemory::OtmProposal::*** method: OtmProposal, looking for " <<  str;
    } 

    _rc_ = mem->TmtXGet ( &GetIn, &GetOut);
  
    if ( _rc_ == 0 )
    {
      T5LOG( T5DEBUG) <<"EqfMemory::OtmProposal::   lookup complete, found " << GetOut.usNumMatchesFound << " proposals"   ;
      wchar_t szRequestedString[2049];
      Data.getSource( szRequestedString, sizeof(szRequestedString) );

      for ( int i = 0; i < (int)vProposals.size(); i++ )
      {
        if ( i >= GetOut.usNumMatchesFound )
        {
          vProposals[i].clear();
        }
        else
        {
          if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)){
            auto strSource = EncodingHelper::convertToUTF8(GetOut.stMatchTable[i].szSource );
            T5LOG( T5DEBUG) <<"EqfMemory::OtmProposal::   proposal " << i << ": match=" << GetOut.stMatchTable[i].usMatchLevel << ", source=", strSource;
          }
          //replace tags for proposal
          auto result = std::make_unique<RequestTagReplacer>(GetOut.stMatchTable[i].szSource, GetOut.stMatchTable[i].szTarget, 
                                                                                      szRequestedString); 
          wcscpy(GetOut.stMatchTable[i].szSource, result->getSrcWithTagsFromRequestC());
          wcscpy(GetOut.stMatchTable[i].szTarget, result->getTrgWithTagsFromRequestC());

          if( GetOut.stMatchTable[i].usMatchLevel >= 100){
            //correct match rate for exact match based on whitespace difference
            int wsDiff = 0;
            UtlCompIgnWhiteSpaceW(szRequestedString, GetOut.stMatchTable[i].szSource, 0, &wsDiff);
            GetOut.stMatchTable[i].usMatchLevel -= wsDiff; 
          }
          mem->MatchToOtmProposal( GetOut.stMatchTable + i, &vProposals[i] );
        } /* end */         
      } /* end */       
    }
    else
    {
      T5LOG( T5DEBUG) <<"EqfMemory::SearchProposal::  lookup failed, rc=" << _rc_;
    } /* end */     

    //if ( _rc_ != 0 ) mem->handleError( _rc_, mem->szName, GetIn.stTmGet.szTagTable );

    Data.iNumOfProposals = OtmProposal::getNumOfProposals( vProposals );
  } /* endif */

  if(_rc_ != 0){
    buildErrorReturn(_rc_, ":: fuzzy search of mem returned error, rc = ") ;
  }
  
  if ( _rc_ == 0 )
  {
    std::wstring strOutputParmsW;
    json_factory.startJSONW( strOutputParmsW );
    json_factory.addParmToJSONW( strOutputParmsW, L"ReturnValue", _rc_ );
    json_factory.addParmToJSONW( strOutputParmsW, L"ErrorMsg", L"" );
    json_factory.addParmToJSONW( strOutputParmsW, L"NumOfFoundProposals",  Data.iNumOfProposals );
    if (  Data.iNumOfProposals > 0 )
    {
      json_factory.addNameToJSONW( strOutputParmsW, L"results" );
      addProposalsToJSONString( strOutputParmsW, vProposals,  Data.iNumOfProposals, true );
    } /* endif */

    json_factory.terminateJSONW( strOutputParmsW );
    outputMessage = EncodingHelper::convertToUTF8( strOutputParmsW );
    _rc_ = OK;
  }
  else
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, Data.szError, sizeof( Data.szError ) / sizeof( Data.szError[0] ) );
    buildErrorReturn( usRC, Data.szError );
    _rc_ = INTERNAL_SERVER_ERROR;
  } /* endif */

  return( _rc_ );
}

int ConcordanceSearchRequestData::parseJSON(){
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, "can't verifyAPISession" );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "::concordanceSearch::Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */

  // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  
  JSONFactory *factory = JSONFactory::getInstance();
  int loggingThreshold = -1;
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"searchString",   JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szSearchString ), sizeof( Data.szSearchString ) / sizeof( Data.szSearchString[0] ) },
  { L"searchType",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szSearchMode ), sizeof( Data.szSearchMode ) },
  { L"searchPosition", JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szSearchPos ), sizeof( Data.szSearchPos ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoSourceLang ), sizeof( Data.szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoTargetLang ), sizeof( Data.szIsoTargetLang ) },
  { L"numResults",     JSONFactory::INT_PARM_TYPE,          &( Data.iNumOfProposals ), 0 },
  { L"numOfProposals", JSONFactory::INT_PARM_TYPE,          &( Data.iNumOfProposals ), 0 },
  { L"msSearchAfterNumResults", JSONFactory::INT_PARM_TYPE, &( Data.iSearchTime ), 0 },
  { L"loggingThreshold", JSONFactory::INT_PARM_TYPE,        &loggingThreshold, 0 },
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  _rc_ = json_factory.parseJSON( strInputParmsW, parseControl );
  if(_rc_){
    buildErrorReturn( _rc_, "::concordanceSearch::json parsing failed" );
    return( BAD_REQUEST );
  }
  return 0;
}

int ConcordanceSearchRequestData::checkData(){
  if ( _rc_ != 0 )
  {
    _rc_ = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( _rc_, "::concordanceSearch::Error: Parsing of input parameters failed" );
    return( BAD_REQUEST );
  } /* end */

  if ( Data.szSearchString[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "::concordanceSearch::Error: Missing search string" );
    return( BAD_REQUEST );
  } /* end */

  if ( Data.iNumOfProposals > 20 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "::concordanceSearch::Error: Too many proposals requested, the maximum value is 20" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.iNumOfProposals == 0 )
  {
    Data.iNumOfProposals = 5;
  }

  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"::concordanceSearch::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold);
  }

  // do the search and handle the results
  if ( strcasecmp( Data.szSearchMode, "Source" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT;
  }
  else if ( strcasecmp( Data.szSearchMode, "Target" ) == 0 )
  {
    lOptions |= SEARCH_IN_TARGET_OPT;
  }
  else if ( strcasecmp( Data.szSearchMode, "SourceAndTarget" ) == 0 )
  {
    lOptions |= SEARCH_IN_SOURCE_OPT | SEARCH_IN_TARGET_OPT;
  } /* endif */
  lOptions |= SEARCH_CASEINSENSITIVE_OPT;

  bool fOk = false;
  if( strlen( Data.szIsoSourceLang) ){
    LanguageFactory::LANGUAGEINFO srcLangInfo;
    fOk = LanguageFactory::getInstance()->getLanguageInfo( Data.szIsoSourceLang, &srcLangInfo );
    if(fOk){
      if(srcLangInfo.fisPreferred){
        lOptions |= SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT;
      }else{
        lOptions |= SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT;
      }
    }else{
      std::string msg = "::concordanceSearch::Error: :: src lang could not be found: " ;
      msg += Data.szIsoSourceLang;
      buildErrorReturn( _rc_, (PSZ)msg.c_str() );
      return( BAD_REQUEST );
    }
  }
  if( strlen( Data.szIsoTargetLang) ){
    LanguageFactory::LANGUAGEINFO trgLangInfo;
    fOk = LanguageFactory::getInstance()->getLanguageInfo( Data.szIsoTargetLang, &trgLangInfo );
    if(fOk){
      if(trgLangInfo.fisPreferred){
        lOptions |= SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT;
      }else{
        lOptions |= SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT;
      }
    }else{
      std::string msg = "::concordanceSearch::Error: :: target lang could not be found: ";
      msg += Data.szIsoTargetLang;
      buildErrorReturn( _rc_, (PSZ)msg.c_str() );
      return( BAD_REQUEST );
    }
  }else{
    fOk = true;
    //buildErrorReturn( _rc_, "::concordanceSearch::Error: ::  "  );
  }
  return 0;
}

ULONG GetTickCount();
wchar_t* wcsupr(wchar_t *str);

int ConcordanceSearchRequestData::execute(){
  std::wstring strProposals;
  
  OtmProposal OProposal;
  // loop until end reached or enough proposals have been found
  int iFoundProposals = 0;
  int iActualSearchTime = 0; // for the first call run until end of TM or one proposal has been found
  do
  {
    {
      BOOL fFound = FALSE;                 // found-a-matching-memory-proposal flag
      
      if ((* Data.szSearchString  == EOS)  )
      {
        char* pszParm = "Error in TMManager::Search string is null";
        T5LOG(T5ERROR) <<  " DDE_MANDPARAMISSING"<< pszParm;
        return DDE_MANDPARAMISSING;
      } /* endif */

      DWORD dwSearchStartTime = 0;
      if ( iActualSearchTime != 0 ) dwSearchStartTime = GetTickCount();

      // get first or next proposal
      if ( *Data.szSearchPos == EOS )
      {
        _rc_ = mem->getFirstProposal( OProposal );
      }
      else
      {
        mem->setSequentialAccessKey((PSZ) Data.szSearchPos );
        _rc_ = mem->getNextProposal( OProposal );
      } /* endif */

      // prepare searchstring
      if ( lOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( Data.szSearchString );
      if ( lOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( Data.szSearchString );

      bool fOneOrMoreIsFound = false; 
      while ( !fFound && ( _rc_ == 0 ) )
      {
        fFound = searchInProposal( &OProposal, Data.szSearchString, lOptions );
        //check langs
        if( fFound ) 
        { // filter by src lang
          if (lOptions & SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT)
          {
            char lang[50];
            OProposal.getSourceLanguage(lang, 50);
            fFound = strcasecmp(lang, Data.szIsoSourceLang ) == 0;
          }else if(lOptions & SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT){
            char lang[50];
            OProposal.getSourceLanguage(lang, 50);
            fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, Data.szIsoSourceLang);
          }
        }
        if ( fFound )
        {
          if (lOptions & SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT)
          {
            char lang[50];
            OProposal.getTargetLanguage(lang, 50);
            fFound = strcasecmp(lang, Data.szIsoTargetLang ) == 0;
          }else if(lOptions & SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT){
            char lang[50];
            OProposal.getTargetLanguage(lang, 50);
            fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, Data.szIsoTargetLang);
          }
        }
        if ( fFound )
        {
          fOneOrMoreIsFound = true;
        }
        else
        { 
          //add check if we have at least one result before stop because of timeout 
          if ( iActualSearchTime != 0 )
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
            if ( lElapsedMillis > iActualSearchTime  && fOneOrMoreIsFound )
            {
              _rc_ = TIMEOUT_RC;
            }
          }
          if ( _rc_ == 0 )
          {
            _rc_ = mem->getNextProposal( OProposal );
          }
        }
      } /* endwhile */

      // search given string in proposal
      if ( fFound || (_rc_ == TIMEOUT_RC) )
      {
        mem->getSequentialAccessKey( Data.szSearchPos, 20 );
      } /* endif */
      else if ( _rc_ == EqfMemory::INFO_ENDREACHED )
      {
        _rc_ = ENDREACHED_RC;
      }
      else{}
    }
    
    iActualSearchTime = Data.iSearchTime;
    if ( _rc_ == 0 )
    {
      addProposalToJSONString( strProposals, OProposal );
      iFoundProposals++;
    }
  } while ( ( _rc_ == 0 ) && ( iFoundProposals < Data.iNumOfProposals ) );

  if ( iFoundProposals || (_rc_ == ENDREACHED_RC) || (_rc_ == TIMEOUT_RC) )
  {
    std::wstring strOutputParmsW;
    json_factory.startJSONW( strOutputParmsW );
    json_factory.addParmToJSONW( strOutputParmsW, L"ReturnValue", _rc_ == ENDREACHED_RC? L"ENDREACHED_RC" : _rc_==TIMEOUT_RC? L"TIMEOUT_RC": L"FOUND");
    if ( _rc_ == ENDREACHED_RC )
    {
      json_factory.addParmToJSONW( strOutputParmsW, L"NewSearchPosition" );
    }
    else
    {
      json_factory.addParmToJSONW( strOutputParmsW, L"NewSearchPosition", Data.szSearchPos );
    }
    if ( iFoundProposals > 0 )
    {
      json_factory.addNameToJSONW( strOutputParmsW, L"results" );
      json_factory.addArrayStartToJSONW( strOutputParmsW );
      strOutputParmsW.append( strProposals );
      json_factory.addArrayEndToJSONW( strOutputParmsW );
    } /* endif */

    json_factory.terminateJSONW( strOutputParmsW );
    outputMessage = EncodingHelper::convertToUTF8( strOutputParmsW );
    _rest_rc_ = 200;
    return 0;
  }
  else
  {
    //unsigned short usRC = 0;
    //EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, Data.szError, sizeof( Data.szError ) / sizeof( Data.szError[0] ) );
    buildErrorReturn( _rc_, Data.szError );
    _rest_rc_ = INTERNAL_SERVER_ERROR;
  } /* endif */

  return( _rc_ );
}
