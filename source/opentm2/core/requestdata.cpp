
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
#include "OSWrapper.h"
#include "LanguageFactory.H"
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



std::vector<std::wstring> replaceString(std::wstring&& src_data, std::wstring&& trg_data, std::wstring&& req_data,  int* rc){ 
  std::vector<std::wstring> response;
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
  return response;
}


MemProposalType getMemProposalType( char *pszType )
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


/*! \brief build return JSON string in case of errors
  \param _rc_ error return code
  \param pszErrorMessage error message text
  \param strError JSON string receiving the error information
  \returns 0 if successful or an error code in case of failures
*/
int RequestData::buildErrorReturn
(
  int _rc_,
  wchar_t *pszErrorMsg
)
{
  std::wstring w_msg;
  json_factory.startJSONW( w_msg );
  json_factory.addParmToJSONW( w_msg, L"ReturnValue", _rc_ );
  json_factory.addParmToJSONW( w_msg, L"ErrorMsg", pszErrorMsg );
  json_factory.terminateJSONW( w_msg );

  errorMsg = EncodingHelper::convertToUTF8( w_msg );
  
  T5LOG(T5ERROR) << errorMsg << ", ErrorCode = " << _rc_;
  return( 0 );
}

int RequestData::buildErrorReturn
(
  int _rc_,
  char *pszErrorMsg
)
{
  json_factory.startJSON( errorMsg );
  json_factory.addParmToJSON( errorMsg, "ReturnValue", _rc_ );
  json_factory.addParmToJSON( errorMsg, "ErrorMsg", pszErrorMsg );
  json_factory.terminateJSON( errorMsg );
  
  T5LOG(T5ERROR) << errorMsg << ", ErrorCode = " << _rc_;
  return( 0 );
}


int RequestData::requestTM(){
  //check if memory is loaded to tmmanager
  if(isReadOnlyRequest())
  {
    mem = TMManager::GetInstance()->requestReadOnlyTMPointer(strMemName, memRef);
  }else if(isWriteRequest())
  {
    mem = TMManager::GetInstance()->requestWriteTMPointer(strMemName, memRef);
  }else{
    fValid = true;
    return 0;
  }

  // get the handle of the memory 
  //int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemName, &lHandle, Data.szError, sizeof( Data.szError ) / sizeof( Data.szError[0] ), &_rc_ );
  //lHandle = EqfMemoryPlugin::GetInstance()->openMemoryNew(strMemName);
  //if ( httpRC != OK )
  //if(!lHandle)
  if(mem.get()== nullptr)
  {
  //  buildErrorReturn( _rc_, Data.szError );
    return !isServiceRequest();//( httpRC );
  } /* endif */
  fValid = true;
  return 0;
}

bool RequestData::isWriteRequest(){
  return command == COMMAND::CLONE_TM_LOCALY
      || command == COMMAND::DELETE_MEM
      || command == COMMAND::DELETE_ENTRY
      || command == COMMAND::UPDATE_ENTRY
      //|| command == COMMAND::CREATE_MEM// should be handled as service command
      || command == COMMAND::IMPORT_MEM
      || command == COMMAND::REORGANIZE_MEM
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
  ;   
}

int RequestData::run(){
    int res = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
    if(!res) res = parseJSON();
    if(!res) res = checkData();

    if(!res) res = requestTM();
    
    
    if(!res && fValid) 
      res = execute();
    if ( res != 0 )
    {
        buildErrorReturn( res, ""/*szLastError*/ );
        return( res );
    } /* endif */
    return res;
}

int CreateMemRequestData::createNewEmptyMemory(){
  // create memory database
  ULONG ulKey;
  std::shared_ptr<EqfMemory> NewMem;
  _rc_ = fValid? 0 : 1;

  if ( _rc_ == NO_ERROR )
  {
    T5LOG( T5DEBUG) << ":: create the memory" ;    
    NewMem = std::make_shared<EqfMemory> (EqfMemory(strMemName));
    _rc_ = NewMem.get()->NTMCreateLongNameTable();
  }
  if ( _rc_ == NO_ERROR )
  {
    //build name and extension of tm data file

    //fill signature record structure
    strcpy( NewMem.get()->stTmSign.szName, NewMem.get()->TmBtree.fb.fileName.c_str() );
    UtlTime( &(NewMem.get()->stTmSign.lTime) );
    strcpy( NewMem.get()->stTmSign.szSourceLanguage,
            strSrcLang.c_str() );

    //TODO - replace version with current t5memory version
    NewMem.get()->stTmSign.bGlobVersion = T5GLOBVERSION;
    NewMem.get()->stTmSign.bMajorVersion = T5MAJVERSION;
    NewMem.get()->stTmSign.bMinorVersion = T5MINVERSION;
    strcpy( NewMem.get()->stTmSign.szDescription,
            strMemDescription.c_str() );

    //call create function for data file
    NewMem.get()->usAccessMode = 1;//ASD_LOCKED;         // new TMs are always in exclusive access...
    
    _rc_ = NewMem.get()->TmBtree.QDAMDictCreateLocal( &(NewMem.get()->stTmSign),FIRST_KEY );
  }
  if ( _rc_ == NO_ERROR )
  {
    //insert initialized record to tm data file
    ulKey = AUTHOR_KEY;
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
              (PBYTE)&NewMem.get()->Authors, TMX_TABLE_SIZE );
  }
  if ( _rc_ == NO_ERROR )
  {
    ulKey = FILE_KEY;
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
                (PBYTE)&NewMem.get()->FileNames, TMX_TABLE_SIZE );     
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    ulKey = TAGTABLE_KEY;
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
                (PBYTE)&NewMem.get()->TagTables, TMX_TABLE_SIZE );
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    ulKey = LANG_KEY;
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
            (PBYTE)&NewMem.get()->Languages, TMX_TABLE_SIZE );
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    int size = sizeof( MAX_COMPACT_SIZE-1 );//OLD, probably bug
    size = MAX_COMPACT_SIZE-1 ;
    //initialize and insert compact area record
    memset( NewMem.get()->bCompact, 0, size );

    ulKey = COMPACT_KEY;
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
                        NewMem.get()->bCompact, size);  

  } /* endif */

  // add long document table record
  if ( _rc_ == NO_ERROR )
  {
    ulKey = LONGNAME_KEY;
    // write long document name buffer area to the database
    _rc_ = NewMem.get()->TmBtree.EQFNTMInsert(&ulKey,
                        (PBYTE)NewMem.get()->pLongNames->pszBuffer,
                        NewMem.get()->pLongNames->ulBufUsed );        

  } /* endif */

  // create language group table
  if ( _rc_ == NO_ERROR )
  {
    _rc_ =  NewMem.get()->NTMCreateLangGroupTable();
    
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    //fill signature record structure
    strcpy( NewMem.get()->stTmSign.szName, NewMem.get()->InBtree.fb.fileName.c_str() );

    _rc_ = NewMem.get()->InBtree.QDAMDictCreateLocal(&NewMem.get()->stTmSign, START_KEY );
                        
  } /* endif */

  if(_rc_ == NO_ERROR){   
    NewMem.get()->TmBtree.fb.Flush();
    NewMem.get()->InBtree.fb.Flush();     
  }else {
    //something went wrong during create or insert so delete data file
    //UtlDelete( (PSZ)NewMem.get()->InBtree.fb.fileName.c_str(), 0L, FALSE );
    UtlDelete((PSZ) NewMem.get()->TmBtree.fb.fileName.c_str(), 0L, FALSE);
    //free allocated memory
    NewMem.get()->NTMDestroyLongNameTable();
  } /* endif */       

  T5LOG( T5INFO) << " done, usRC = " << _rc_ ;
  TMManager::GetInstance()->AddMem(NewMem);
}

int CreateMemRequestData::importInInternalFomat(){
    T5LOG( T5INFO) << "createMemory():: strData is not empty -> setup temp file name for ZIP package file ";
    // setup temp file name for ZIP package file 
  
    std::string strTempFile =  FilesystemHelper::BuildTempFileName();
    if (strTempFile.empty()  )
    {
        wchar_t errMsg[] = L"Could not create file name for temporary data";
        buildErrorReturn( -1, errMsg );
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
    _rc_ = TMManager::GetInstance()->APIImportMemInInternalFormat( strMemName.c_str(), strTempFile.c_str(), 0 );
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
        FilesystemHelper::DeleteFile( strTempFile );
    }
}

int CreateMemRequestData::checkData(){
    if ( !_rest_rc_ && strMemName.empty() )
    {
        _rc_ = ERROR_INPUT_PARMS_INVALID;
        buildErrorReturn( _rc_, "Error: Missing memory name input parameter" );
        _rest_rc_ = BAD_REQUEST ;
        return _rest_rc_;
    } /* end */
    if ( !_rest_rc_ && strSrcLang.empty() )
    {
        _rc_ = ERROR_INPUT_PARMS_INVALID;
        buildErrorReturn( _rc_, "Error: Missing source language input parameter" );
        _rest_rc_= BAD_REQUEST ;
        return _rest_rc_;
    } /* end */

    // convert the ISO source language to a OpenTM2 source language name
    char szOtmSourceLang[40];
    if(!_rest_rc_ ){
        
        szOtmSourceLang[0] = 0;//terminate to avoid garbage
        bool fPrefered = false;
        EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, (PSZ)strSrcLang.c_str(), szOtmSourceLang, &fPrefered );
        
        if ( szOtmSourceLang[0] == 0 )
        {
            _rc_ = ERROR_INPUT_PARMS_INVALID;
            std::string err =  "Error: Could not convert language " + strSrcLang + "to OpenTM2 language name";
            buildErrorReturn( _rc_, (PSZ)err.c_str() );
            _rest_rc_ =  BAD_REQUEST ;
            return _rest_rc_;
        } /* end */
    }

    T5LOG(T5DEBUG) << "( MemName = "<<strMemName <<", pszDescription = "<<strMemDescription<<", pszSourceLanguage = "<<strSrcLang<<" )";
    // Check memory name syntax
    if ( _rc_ == NO_ERROR )
    { 
      if ( !FilesystemHelper::checkFileName( strMemName ))
      {
        T5LOG(T5ERROR) <<   "::ERROR_INV_NAME::" << strMemName;
        _rc_ = ERROR_MEM_NAME_INVALID;
      } /* endif */
    } /* endif */

    // check if TM exists already
    if ( _rc_ == NO_ERROR )
    {
      int res = NewTMManager::GetInstance()->TMExistsOnDisk(strMemName, false );
      if ( res != NewTMManager::TMM_TM_NOT_FOUND )
      {
        T5LOG(T5ERROR) <<  "::ERROR_MEM_NAME_EXISTS:: TM with this name already exists: " << strMemName << "; res = "<< res;
        _rc_ = ERROR_MEM_NAME_EXISTS;
      }
    } /* endif */


    // check if source language is valid
    if ( _rc_ == NO_ERROR )
    {
      SHORT sID = 0;    
      if ( MorphGetLanguageID( (PSZ)strSrcLang.c_str(), &sID ) != MORPH_OK )
      {
        _rc_ = ERROR_PROPERTY_LANG_DATA;
        T5LOG(T5ERROR) << "MorhpGetLanguageID returned error, usRC = ERROR_PROPERTY_LANG_DATA;";
      } /* endif */
    } /* endif */
    fValid = !_rest_rc_ && !_rc_;
    return _rest_rc_;
}

int CreateMemRequestData::parseJSON(){
// parse input parameters
  //std::string strData, strName, strSourceLang, strDescription;
  void *parseHandle = json_factory.parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( _rc_, "Missing or incorrect JSON data in request body" );
    _rest_rc_ =  BAD_REQUEST ;
    return _rest_rc_;
  } /* end */
  //if(_rest_rc_){
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
            else{
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
                T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::import::set new threshold for logging" <<loggingThreshold ;
                T5Logger::GetInstance()->SetLogLevel(loggingThreshold);        
            }else{
                T5LOG( T5WARNING) << "JSON parsed unused data: name = " << name << "; value = " << value;
            }
            }
        }
    } /* endwhile */
    json_factory.parseJSONStop( parseHandle );
  //}
  if(_rc_ == 2002) _rc_=0;
  return _rest_rc_;
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
  //OtmMemoryServiceWorker::std::shared_ptr<EqfMemory>  pMem = nullptr;
} IMPORTMEMORYDATA, *PIMPORTMEMORYDATA;


// import memory process
void importMemoryProcess( void *pvData );

int ImportRequestData::parseJSON(){
  T5LOG( T5INFO) << "+POST " << strMemName << "/import\n";
  if ( _rc_ != 0 )
  {
    //buildErrorReturn( _rc_, this->szLastError, strOutputParms );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    wchar_t errMsg[] = L"OtmMemoryServiceWorker::import::Missing name of memory";
    buildErrorReturn( _rc_, errMsg );
    return( BAD_REQUEST );
  } /* endif */

  // check if memory exists
  //if ( EqfMemoryExists( this->OtmMemoryServiceWorker::getInstance()->hSession, (PSZ)strMemName.c_str() ) != 0 )
  if(NewTMManager::GetInstance()->TMExistsOnDisk(strMemName))
  {
    return( NOT_FOUND );
  }

  // find the memory to our memory list
  //std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->findOpenedMemory( strMemName);
  
  // extract TMX data
  std::string strTmxData;
  int loggingThreshold = -1; //0-develop(show all logs), 1-debug+, 2-info+, 3-warnings+, 4-errors+, 5-fatals only
  
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( strBody, &_rc_ );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::import::Missing or incorrect JSON data in request body" );
        
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;

    return( BAD_REQUEST );
  } /* end */

  std::string name;
  std::string value;
  while ( _rc_ == 0 )
  {
    _rc_ = factory->parseJSONGetNext( parseHandle, name, value );
    if ( _rc_ == 0 )
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
    }else if(_rc_ != 2002){// _rc_ != INFO_ENDOFPARAMETERLISTREACHED
        T5LOG(T5ERROR) << "failed to parse JSON \"" << strBody <<"\", _rc_ = " << _rc_;
    }
  } /* endwhile */
  factory->parseJSONStop( parseHandle );
  if ( strTmxData.empty() )
  {
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::import::Missing TMX data" );
  
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;

    return( BAD_REQUEST );
  } /* endif */

  // setup temp file name for TMX file 
  std::string strTempFile =  FilesystemHelper::BuildTempFileName();
  if (strTempFile.empty()  )
  {
    buildErrorReturn( -1, "OtmMemoryServiceWorker::import::Could not create file name for temporary data" );
    
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;
    return( INTERNAL_SERVER_ERROR );
  }

  T5LOG( T5INFO) << "OtmMemoryServiceWorker::import::+   Temp TMX File is " << strTempFile;

  // decode TMX data and write it to temp file
  std::string strError;
  _rc_ = FilesystemHelper::DecodeBase64ToFile( strTmxData.c_str(), strTempFile.c_str(), strError ) ;
  if ( _rc_ != 0 )
  {
    strError = "OtmMemoryServiceWorker::import::" + strError;
    buildErrorReturn( _rc_, (char *)strError.c_str() );
    
    //restore status
    pMem->lHandle = lHandle;
    pMem->eStatus = lastStatus;
    pMem->eImportStatus = lastImportStatus;
    return( INTERNAL_SERVER_ERROR );
  }
  
  //success in parsing request data-> close mem if needed
  //if(lHandle && fClose){
  //      EqfCloseMem( OtmMemoryServiceWorker::getInstance()->hSession, lHandle, 0 );
  //}

  // start the import background process
  PIMPORTMEMORYDATA pData = new( IMPORTMEMORYDATA );
  strcpy( pData->szInFile, strTempFile.c_str() );
  strcpy( pData->szMemory, strMemName.c_str() );

  if(pMem->importDetails == nullptr){
    pMem->importDetails = new ImportStatusDetails;
  }
  
  pData->importDetails = pMem->importDetails;
  pData->importDetails->reset();
  pData->hSession = OtmMemoryServiceWorker::getInstance()->hSession;
  pData->pMemoryServiceWorker = nullptr;//this;

  //importMemoryProcess(pData);//to do in same thread
  std::thread worker_thread(importMemoryProcess, pData);
  worker_thread.detach();

  return( CREATED );
}


int ImportRequestData::checkData(){
  EqfMemory* pMem = mem.get();
  if ( pMem != nullptr )
  {
    // close the memory - when open
    if ( pMem->eStatus == OPEN_STATUS )
    {
      fClose = true;
      lastStatus =       pMem->eStatus;
      lastImportStatus = pMem->eImportStatus;

      pMem->eStatus = AVAILABLE_STATUS;
      pMem->eImportStatus = IMPORT_RUNNING_STATUS;
      //pMem->dImportProcess = 0;
    }
  }
  else
  {
    

    pMem->eStatus = AVAILABLE_STATUS;
    pMem->eImportStatus = IMPORT_RUNNING_STATUS;
    //pMem->dImportProcess = 0;
    strcpy( pMem->szName, strMemName.c_str() );
  }
  T5LOG( T5DEBUG) <<  "status for " << strMemName << " was changed to import";
}

int ImportRequestData::execute(){

}

int ExportRequestData::checkData(){
  T5LOG( T5INFO) <<"::getMem::=== getMem request, memory = " << strMemName << "; format = " << requestAcceptHeader;
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    T5LOG( T5INFO) <<"::getMem::Error: no valid API session" ;
    return(_rest_rc_ =  BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    T5LOG(T5ERROR) <<"::getMem::Error: no memory name specified" ;
    return( _rest_rc_ = BAD_REQUEST );
  } /* endif */


  if(!_rc_)
  {
    int res =  NewTMManager::GetInstance()->TMExistsOnDisk(strMemName, true );
    if ( res != NewTMManager::TMM_NO_ERROR )
    {
      T5LOG(T5ERROR) <<"::getMem::Error: memory does not exist, memName = " << strMemName<< "; res= " << res;
      return( _rest_rc_ =  NOT_FOUND );
    }
  }
  fValid = true;
}

int ExportRequestData::execute(){
  // get a temporary file name for the memory package file or TMX file
  strTempFile = FilesystemHelper::BuildTempFileName();
  if ( _rc_ != 0 )
  {
    T5LOG(T5ERROR) <<"::getMem:: Error: creation of temporary file for memory data failed" ;
    return( _rest_rc_ = INTERNAL_SERVER_ERROR );
  }
  // export the memory in internal format
  if ( requestAcceptHeader.compare( "application/xml" ) == 0 )
  {
    T5LOG( T5INFO) <<"::getMem:: mem = " <<  strMemName << "; supported type found application/xml, tempFile = " << strTempFile;
    //_rc_ = EqfExportMem( OtmMemoryServiceWorker::getInstance()->hSession, (PSZ)strMemName.c_str(), (PSZ)strTempFile.c_str(), TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
    ExportTmx();
    if ( _rc_ != 0 )
    {
      //unsigned short usRC = 0;
      //EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      //T5LOG(T5ERROR) <<"::getMem:: Error: EqfExportMem failed with rc=" << _rc_ << ", error message is " <<  EncodingHelper::convertToUTF8( this->szLastError);
      return( _rest_rc_ = INTERNAL_SERVER_ERROR );
    }
  }
  //else if ( requestAcceptHeader.compare( "application/zip" ) == 0 )
  //{
    //T5LOG( T5INFO) <<"::getMem:: mem = "<< strMemName << "; supported type found application/zip(NOT TESTED YET), tempFile = " << szTempFile;
    //_rc_ = EqfExportMemInInternalFormat( OtmMemoryServiceWorker::getInstance()->hSession, (PSZ)strMemName.c_str(), (PSZ)strTempFile.c_str(), 0 );
    //if ( _rc_ != 0 )
    //{
    //  unsigned short usRC = 0;
    //  EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    //  T5LOG(T5ERROR) <<"::getMem:: Error: EqfExportMemInInternalFormat failed with rc=" <<_rc_ << ", error message is " << EncodingHelper::convertToUTF8( this->szLastError);
    //  return( INTERNAL_SERVER_ERROR );
    //}
  //}
  else
  {
    T5LOG(T5ERROR) <<"::getMem:: Error: the type " << requestAcceptHeader << " is not supported" ;
    return( NOT_ACCEPTABLE );
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
    T5LOG(T5ERROR) << "::getMem::  Error: failed to load the temporary file, fName = " <<  strTempFile ;
    return( INTERNAL_SERVER_ERROR );
  }

  return( OK );

}

int ExportRequestData::ExportZip(){
  return -1;
}

int ExportRequestData::ExportTmx(){
  
  //mem
  // call TM export
  if ( _rc_ == NO_ERROR )
  {
    //if ( !( lOptions & COMPLETE_IN_ONE_CALL_OPT ) ) Data.sLastFunction = FCT_EQFEXPORTMEM;
    //_rc_ = MemFuncExportMem( pData, pszMemName, pszOutFile, lOptions );    
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
    buildErrorReturn( _rc_, "can't verifyAPISession" );
    return( BAD_REQUEST );
  } /* endif */

  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "Missing name of memory");
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemName );

  // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  // parse input parameters
  memset( &Data, 0, sizeof( LOOKUPINMEMORYDATA ) );
      
  JSONFactory *pJsonFactory = JSONFactory::getInstance();
  JSONFactory::JSONPARSECONTROL parseControl[] = { 
  { L"source",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szSource ), sizeof( Data.szSource ) / sizeof( Data.szSource[0] ) },
  { L"target",         JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szTarget ), sizeof( Data.szTarget ) / sizeof( Data.szTarget[0] ) },
  { L"sourceLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoSourceLang ), sizeof( Data.szIsoSourceLang ) },
  { L"targetLang",     JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szIsoTargetLang ), sizeof( Data.szIsoTargetLang ) },  

  { L"segmentNumber",  JSONFactory::INT_PARM_TYPE,          &( Data.lSegmentNum ), 0 },
  { L"documentName",   JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDocName ), sizeof( Data.szDocName ) },
  { L"type",           JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szType ), sizeof( Data.szType ) },
  { L"author",         JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szAuthor ), sizeof( Data.szAuthor ) },
  { L"markupTable",    JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szMarkup ), sizeof( Data.szMarkup ) },
  { L"context",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szContext ), sizeof( Data.szContext ) / sizeof( Data.szContext[0] ) },
  { L"timeStamp",      JSONFactory::ASCII_STRING_PARM_TYPE, &( Data.szDateTime ), sizeof( Data.szDateTime ) },
  { L"addInfo",        JSONFactory::UTF16_STRING_PARM_TYPE, &( Data.szAddInfo ), sizeof( Data.szAddInfo ) / sizeof( Data.szAddInfo[0] ) },
  { L"loggingThreshold",JSONFactory::INT_PARM_TYPE        , &(loggingThreshold), 0},
  { L"",               JSONFactory::ASCII_STRING_PARM_TYPE, NULL, 0 } };

  _rc_ = json_factory.parseJSON( strInputParmsW, parseControl );
  return 0;
}

int UpdateEntryRequestData::checkData(){
  if ( _rc_ != 0 )
  {
    _rc_ = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( _rc_, "Error: Parsing of input parameters failed" );
    return( BAD_REQUEST );
  } /* end */

  if ( Data.szSource[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing source text" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szTarget[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing target text" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szIsoSourceLang[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_,  "Error: Missing source language");
    return( BAD_REQUEST );
  } /* end */
  if ( Data.szIsoTargetLang[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing target language" );
    return( BAD_REQUEST );
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
  memset( &Prop, 0, sizeof( Prop ) );
  int rc = 0;
  auto replacedInput = replaceString( Data.szSource , Data.szTarget, L"", &rc );
  //wcscpy( Prop.szSource, Data.szSource );
  //wcscpy( Prop.szTarget, Data.szTarget );
  if(!rc){
    wcscpy( Prop.szSource, replacedInput[0].c_str());
    wcscpy( Prop.szTarget, replacedInput[1].c_str());
  }else{
    outputMessage = "Error in xml in source or target!";
    return rc;
    //wcscpy( Prop.szSource, Data.szSource);
    //wcscpy( Prop.szTarget, Data.szTarget);
  }
  Prop.lSegmentNum = Data.lSegmentNum;
  strcpy( Prop.szDocName, Data.szDocName );
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoSourceLang, Prop.szSourceLanguage );
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoTargetLang, Prop.szTargetLanguage );
  Prop.eType = getMemProposalType(Data.szType );
  strcpy( Prop.szTargetAuthor,Data.szAuthor ); 
  strcpy( Prop.szMarkup,Data.szMarkup );  
  wcscpy( Prop.szContext,Data.szContext );
  LONG lTime = 0;
  if (Data.szDateTime[0] != 0 )
  {
  // use provided time stamp
    convertUTCTimeToLong(Data.szDateTime, &(Prop.lTargetTime) );
  }
  else
  {
    // a lTime value of zero automatically sets the update time
    // so refresh the time stamp (using OpenTM2 very special time logic...)
    // and convert the time to a date time string
    LONG            lTimeStamp;             // buffer for current time
    time( (time_t*)&lTimeStamp );
    lTimeStamp -= 10800L; // correction: - 3 hours (this is a tribute to the old OS/2 times)
    convertTimeToUTC( lTimeStamp,Data.szDateTime );
  }
  wcscpy( Prop.szAddInfo,Data.szAddInfo );

  // update the memory
  OtmProposal OtmProposal;
  strcpy(Prop.szDocShortName , Prop.szDocName);
  copyMemProposalToOtmProposal( &Prop, &OtmProposal );
  //USHORT usRC = (USHORT)mem.get()->putProposal( *pOtmProposal );
  
  TMX_PUT_IN_W  TmPutIn;                       // ptr to TMX_PUT_IN_W structure
  TMX_PUT_OUT_W TmPutOut;                      // ptr to TMX_PUT_OUT_W structure
  memset( &TmPutIn, 0, sizeof(TMX_PUT_IN_W) );
  memset( &TmPutOut, 0, sizeof(TMX_PUT_OUT_W) );

  OtmProposalToPutIn( OtmProposal, &TmPutIn );

  if(T5Logger::GetInstance()->CheckLogLevel(T5INFO)){
    std::string source = EncodingHelper::convertToUTF8(TmPutIn.stTmPut.szSource);
    T5LOG( T5INFO) <<"EqfMemory::putProposal, source = " << source;
  }
  /********************************************************************/
  /* fill the TMX_PUT_IN prefix structure                             */
  /* the TMX_PUT_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  _rc_ = TmtXReplace ( mem.get(), &TmPutIn, &TmPutOut );

  if ( _rc_ != 0 ){
      T5LOG(T5ERROR) <<  "EqfMemory::putProposal result = " << _rc_;   
      //handleError( _rc_, this->szName, TmPutIn.stTmPut.szTagTable );
  }else{
    mem.get()->TmBtree.fb.Flush();
    mem.get()->InBtree.fb.Flush();
  }

  if ( ( _rc_ == 0 ) &&
       ( TmPutIn.stTmPut.fMarkupChanged ) ) {
     _rc_ = SEG_RESET_BAD_MARKUP ;
  }
  
  if ( _rc_ != 0 )
  {
    unsigned short usRC = 0;
    EqfGetLastErrorW( OtmMemoryServiceWorker::getInstance()->hSession, &usRC,Data.szError, sizeof(Data.szError ) / sizeof(Data.szError[0] ) );
    buildErrorReturn( _rc_,Data.szError );
    return( INTERNAL_SERVER_ERROR );
  } /* endif */

  // return the entry data
  //std::string outputMessage;
  std::string str_src = EncodingHelper::convertToUTF8(Data.szSource );
  std::string str_trg = EncodingHelper::convertToUTF8(Data.szTarget );

  std::wstring outputMessageW;
  json_factory.startJSON( outputMessage );
  json_factory.addParmToJSON( outputMessage, "sourceLang",Data.szIsoSourceLang );
  json_factory.addParmToJSON( outputMessage, "targetLang",Data.szIsoTargetLang );

  json_factory.addParmToJSONW( outputMessageW, L"source",Data.szSource );
  json_factory.addParmToJSONW( outputMessageW, L"target",Data.szTarget );
  outputMessage += ",\n" + EncodingHelper::convertToUTF8(outputMessageW.c_str());// +", ";


  json_factory.addParmToJSON( outputMessage, "documentName", Data.szDocName );
  json_factory.addParmToJSON( outputMessage, "segmentNumber", Data.lSegmentNum );
  json_factory.addParmToJSON( outputMessage, "markupTable", Data.szMarkup );
  json_factory.addParmToJSON( outputMessage, "timeStamp", Data.szDateTime );
  json_factory.addParmToJSON( outputMessage, "author", Data.szAuthor );
  json_factory.terminateJSON( outputMessage );
  //outputMessage = EncodingHelper::convertToUTF8( strOutputParmsW.c_str() );

  _rc_ = OK;

  return( _rc_ );
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
  
  memset( &Data, 0, sizeof( LOOKUPINMEMORYDATA ) );
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
    _rc_ = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( _rc_, "Error: Parsing of input parameters failed" );
    return( _rest_rc_ = BAD_REQUEST );
  } /* end */

  if ( Data.szSource[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing source text input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */

  if ( Data.szIsoSourceLang[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing source language input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  
  if ( Data.szIsoTargetLang[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Missing target language input parameter" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */
  
  if ( Data.szMarkup[0] == 0 )
  {
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::search::No markup requested -> using OTMXUXLF";
    // use default markup table if none given
    strcpy( Data.szMarkup, "OTMXUXLF" );
  } /* end */
  
  if ( Data.iNumOfProposals > 20 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "Error: Too many proposals requested, the maximum value is 20" );
    return(  _rest_rc_ = BAD_REQUEST );
  } /* end */

  
  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::search::set new threshold for logging" << loggingThreshold;
    T5Logger::GetInstance()->SetLogLevel(loggingThreshold);
  }

  // get the handle of the memory 
  //long lHandle = 0;
  //int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemName, &lHandle, Data.szError, sizeof( Data.szError ) / sizeof( Data.szError[0] ), &_rc_ );
  //if ( httpRC != OK )
  //{
  //  buildErrorReturn( _rc_, Data.szError );
  //  return(  _rest_rc_ = httpRC );
  //} /* endif */
  return _rc_;
}


/* write a single proposal to a JSON string
\param strJSON JSON stirng receiving the proposal data
\param pProp pointer to a MEMPROPOSAL containing the proposal
\param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
\returns 0 is successful
*/
int addProposalToJSONString
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
  EqfGetIsoLang( OtmMemoryServiceWorker::getInstance()->hSession, pProp->szSourceLanguage, pData->szIsoSourceLang );
  pJsonFactory->addParmToJSONW( strJSON, L"sourceLang", EncodingHelper::convertToUTF16( pData->szIsoSourceLang ).c_str() );
  EqfGetIsoLang( OtmMemoryServiceWorker::getInstance()->hSession, pProp->szTargetLanguage, pData->szIsoSourceLang );
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
int addProposalsToJSONString
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


int FuzzySearchRequestData::execute(){
 // prepare the memory lookup
  PMEMPROPOSAL pSearchKey = new (MEMPROPOSAL);
  memset( pSearchKey, 0, sizeof( *pSearchKey ) );
  wcscpy( pSearchKey->szSource, Data.szSource );
  strcpy( pSearchKey->szDocName, Data.szDocName );
  pSearchKey->lSegmentNum = Data.lSegmentNum;
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoSourceLang, pSearchKey->szSourceLanguage, &pSearchKey->fIsoSourceLangIsPrefered );
  EqfGetOpenTM2Lang( OtmMemoryServiceWorker::getInstance()->hSession, Data.szIsoTargetLang, pSearchKey->szTargetLanguage, &pSearchKey->fIsoTargetLangIsPrefered );
  strcpy( pSearchKey->szMarkup, Data.szMarkup );
  wcscpy( pSearchKey->szContext, Data.szContext );

  if ( Data.iNumOfProposals == 0 )
  {
    //Data.iNumOfProposals = std::min( 5 * (int)sourceLangs.size(), 20);
    Data.iNumOfProposals = 5;
  }

  PMEMPROPOSAL pFoundProposals = new( MEMPROPOSAL[Data.iNumOfProposals] );
  memset( pFoundProposals, 0, sizeof( MEMPROPOSAL ) * Data.iNumOfProposals );
  // do the lookup and handle the results
  // call the memory factory to process the request
  if ( _rc_ == NO_ERROR )
  {
    _rc_ =  TMManager::GetInstance()->APIQueryMem( mem.get(), pSearchKey, & Data.iNumOfProposals, pFoundProposals, GET_EXACT );
  } /* endif */

  if(_rc_ != 0){
    T5LOG( T5WARNING) << ":: fuzzy search of mem returned error, rc = " << _rc_ ;
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
      addProposalsToJSONString( strOutputParmsW, pFoundProposals,  Data.iNumOfProposals, (void *)&Data );
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

  if ( pSearchKey ) delete pSearchKey;
  if ( pFoundProposals ) delete[] pFoundProposals;
  return( _rc_ );
}

int ConcordanceSearchRequestData::parseJSON(){
  _rc_ = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
  if ( _rc_ != 0 )
  {
    buildErrorReturn( _rc_, "can't verifyAPISession" );
    return( BAD_REQUEST );
  } /* endif */

  //EncodingHelper::convertUTF8ToASCII( strMemName );
  if ( strMemName.empty() )
  {
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::concordanceSearch::Missing name of memory" );
    return( BAD_REQUEST );
  } /* endif */

  // parse input parameters
  std::wstring strInputParmsW = EncodingHelper::convertToUTF16( strBody.c_str() );
  
  memset( &Data, 0, sizeof( LOOKUPINMEMORYDATA ) );
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
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::concordanceSearch::json parsing failed" );
    return( BAD_REQUEST );
  }
  return 0;
}

int ConcordanceSearchRequestData::checkData(){
  if ( _rc_ != 0 )
  {
    _rc_ = ERROR_INTERNALFUNCTION_FAILED;
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::concordanceSearch::Error: Parsing of input parameters failed" );
    return( BAD_REQUEST );
  } /* end */

  if ( Data.szSearchString[0] == 0 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::concordanceSearch::Error: Missing search string" );
    return( BAD_REQUEST );
  } /* end */

  if ( Data.iNumOfProposals > 20 )
  {
    _rc_ = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( _rc_, "OtmMemoryServiceWorker::concordanceSearch::Error: Too many proposals requested, the maximum value is 20" );
    return( BAD_REQUEST );
  } /* end */
  if ( Data.iNumOfProposals == 0 )
  {
    Data.iNumOfProposals = 5;
  }

  if(loggingThreshold >= 0){
    T5LOG( T5WARNING) <<"OtmMemoryServiceWorker::concordanceSearch::set new threshold for logging" << loggingThreshold;
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
  }
  fValid = fOk;
  return 0;

}

ULONG GetTickCount();
wchar_t* wcsupr(wchar_t *str);

int ConcordanceSearchRequestData::execute(){
  std::wstring strProposals;

  // loop until end reached or enough proposals have been found
  int iFoundProposals = 0;
  int iActualSearchTime = 0; // for the first call run until end of TM or one proposal has been found
  do
  {
    memset( &Proposal, 0, sizeof( Proposal ) );
    //_rc_ = EqfSearchMem( OtmMemoryServiceWorker::getInstance()->hSession, mem.get(), Data.szSearchString, Data.szSearchPos, &Proposal, iActualSearchTime, lOptions );
    {
      BOOL fFound = FALSE;                 // found-a-matching-memory-proposal flag
      OtmProposal *pOtmProposal = new (OtmProposal);
      EqfMemory* pMem = mem.get();

      if ((* Data.szSearchString  == EOS)  )
      {
        char* pszParm = "Error in TMManager::APISearchMem::Search string is null";
        T5LOG(T5ERROR) <<  " DDE_MANDPARAMISSING"<< pszParm;
        return DDE_MANDPARAMISSING;
      } /* endif */

      DWORD dwSearchStartTime = 0;
      if ( iActualSearchTime != 0 ) dwSearchStartTime = GetTickCount();

      // get first or next proposal
      if ( *Data.szSearchPos == EOS )
      {
        _rc_ = pMem->getFirstProposal( *pOtmProposal );
      }
      else
      {
        pMem->setSequentialAccessKey((PSZ) Data.szSearchPos );
        _rc_ = pMem->getNextProposal( *pOtmProposal );
      } /* endif */

      // prepare searchstring
      if ( lOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( Data.szSearchString );
      if ( lOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( Data.szSearchString );

      bool fOneOrMoreIsFound = false; 
      while ( !fFound && ( _rc_ == 0 ) )
      {
        fFound = searchInProposal( pOtmProposal, Data.szSearchString, lOptions );
        //check langs
        if( fFound ) 
        { // filter by src lang
          if (lOptions & SEARCH_EXACT_MATCH_OF_SRC_LANG_OPT)
          {
            char lang[50];
            pOtmProposal->getSourceLanguage(lang, 50);
            fFound = strcasecmp(lang, Data.szIsoSourceLang ) == 0;
          }else if(lOptions & SEARCH_GROUP_MATCH_OF_SRC_LANG_OPT){
            char lang[50];
            pOtmProposal->getSourceLanguage(lang, 50);
            fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, Data.szIsoSourceLang);
          }
        }
        if ( fFound )
        {
          if (lOptions & SEARCH_EXACT_MATCH_OF_TRG_LANG_OPT)
          {
            char lang[50];
            pOtmProposal->getTargetLanguage(lang, 50);
            fFound = strcasecmp(lang, Data.szIsoTargetLang ) == 0;
          }else if(lOptions & SEARCH_GROUP_MATCH_OF_TRG_LANG_OPT){
            char lang[50];
            pOtmProposal->getTargetLanguage(lang, 50);
            fFound = LanguageFactory::getInstance()->isTheSameLangGroup(lang, Data.szIsoTargetLang);
          }
        }
        if ( fFound )
        {
          fOneOrMoreIsFound = true;
          copyOtmProposalToMemProposal( pOtmProposal , &Proposal );
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
            _rc_ = pMem->getNextProposal( *pOtmProposal );
          }
        }
      } /* endwhile */

      // search given string in proposal
      if ( fFound || (_rc_ == TIMEOUT_RC) )
      {
        pMem->getSequentialAccessKey( Data.szSearchPos, 20 );
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
      addProposalToJSONString( strProposals, &Proposal, (void *)&Data );
      iFoundProposals++;
    }
  } while ( ( _rc_ == 0 ) && ( iFoundProposals < Data.iNumOfProposals ) );

  if ( iFoundProposals || (_rc_ == ENDREACHED_RC) || (_rc_ == TIMEOUT_RC) )
  {
    std::wstring strOutputParmsW;
    json_factory.startJSONW( strOutputParmsW );
//    json_factory.addParmToJSONW( strOutputParmsW, L"ReturnValue", _rc_ );
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

  //if ( pFoundProposals ) delete pFoundProposals;

  return( _rc_ );
}


