#include "requestdata.h"
#include "tm.h"

#include "../../RestAPI/OTMMSJSONFactory.h"
#include "../../RestAPI/OtmMemoryServiceWorker.h"
#include "LogWrapper.h"
#include "EncodingHelper.h"
#include "EQFMORPH.H"
#include "FilesystemHelper.h"

JSONFactory RequestData::json_factory;

/*! \brief build return JSON string in case of errors
  \param iRC error return code
  \param pszErrorMessage error message text
  \param strError JSON string receiving the error information
  \returns 0 if successful or an error code in case of failures
*/
int RequestData::buildErrorReturn
(
  int iRC,
  wchar_t *pszErrorMsg
)
{
  std::wstring w_msg;
  json_factory.startJSONW( w_msg );
  json_factory.addParmToJSONW( w_msg, L"ReturnValue", iRC );
  json_factory.addParmToJSONW( w_msg, L"ErrorMsg", pszErrorMsg );
  json_factory.terminateJSONW( w_msg );

  errorMsg = EncodingHelper::convertToUTF8( w_msg );
  
  T5LOG(T5ERROR) << errorMsg << ", ErrorCode = " << iRC;
  return( 0 );
}

int RequestData::buildErrorReturn
(
  int iRC,
  char *pszErrorMsg
)
{
  json_factory.startJSON( errorMsg );
  json_factory.addParmToJSON( errorMsg, "ReturnValue", iRC );
  json_factory.addParmToJSON( errorMsg, "ErrorMsg", pszErrorMsg );
  json_factory.terminateJSON( errorMsg );
  
  T5LOG(T5ERROR) << errorMsg << ", ErrorCode = " << iRC;
  return( 0 );
}

int RequestData::run(){
    int res = OtmMemoryServiceWorker::getInstance()->verifyAPISession();
    if(!res) res = parseJSON();
    if(!res) res = checkData();
    if(!res && fValid) res = execute();
    if ( res != 0 )
    {
        buildErrorReturn( res, ""/*szLastError*/ );
        return( res );
    } /* endif */
    return res;
}

int CreateMemRequestData::createNewEmptyMemory(){
    T5LOG(T5DEBUG) << "( MemName = "<<strMemName <<", pszDescription = "<<strMemDescription<<", pszSourceLanguage = "<<strSrcLang<<" )";

    // Check memory name syntax
    if ( _rc_ == NO_ERROR )
    { 
      if ( !FilesystemHelper::checkFileName( strMemName ))
      {
        T5LOG(T5ERROR) <<   "::ERROR_INV_LONGNAME::" << strMemName;
        _rc_ = ERROR_MEM_NAME_INVALID;
      } /* endif */
    } /* endif */

    // check if TM exists already
    if ( _rc_ == NO_ERROR )
    {
      int logLevel = T5Logger::GetInstance()->suppressLogging();
      if ( TMManager::GetInstance()->exists( NULL, (PSZ)strMemName.c_str() ) )
      {
        T5Logger::GetInstance()->desuppressLogging(logLevel);
        T5LOG(T5ERROR) <<  "::ERROR_MEM_NAME_EXISTS:: TM with this name already exists: " << strMemName;
        _rc_ = ERROR_MEM_NAME_EXISTS;
      }
      T5Logger::GetInstance()->desuppressLogging(logLevel);
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

    // create memory database
    if (_rc_==NO_ERROR){
      EqfMemoryPlugin::GetInstance()->createMemory( *this );
    }    

}

int CreateMemRequestData::importInInternalFomat(){
    T5LOG( T5INFO) << "createMemory():: strData is not empty -> setup temp file name for ZIP package file ";
    // setup temp file name for ZIP package file 
    char szTempFile[PATH_MAX];
    _rc_ = OtmMemoryServiceWorker::getInstance()->buildTempFileName( szTempFile );
    if ( _rc_ != 0 )
    {
        wchar_t errMsg[] = L"Could not create file name for temporary data";
        buildErrorReturn( -1, errMsg );
        return( INTERNAL_SERVER_ERROR );
    }

    T5LOG( T5INFO) << "+   Temp binary file is " << szTempFile ;

    // decode binary data and write it to temp file
    std::string strError;
    _rc_ = OtmMemoryServiceWorker::getInstance()->decodeBase64ToFile( strMemB64EncodedData.c_str(), szTempFile, strError );
    if ( _rc_ != 0 )
    {
        buildErrorReturn( _rc_, (char *)strError.c_str() );
        return( INTERNAL_SERVER_ERROR );
    }
    _rc_ = TMManager::GetInstance()->APIImportMemInInternalFormat( strMemName.c_str(), szTempFile, 0 );
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == false){ //for DEBUG and DEVELOP modes leave file in fs
        FilesystemHelper::DeleteFile( szTempFile );
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
    if(!_rest_rc_){
      std::string memPath = FilesystemHelper::GetMemDir() + strMemName + EXT_OF_TMDATA;
      _rc_ = NTMFillCreateInStruct( memPath.c_str(), //call function to fill TMC_CREATE_IN structure
                                szOtmSourceLang,
                                strMemDescription.c_str(),
                                &CreateIn );
      if(_rc_){
         _rest_rc_ = BAD_REQUEST;
      }
    }
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
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::createMemo()::usRC = " << _rc_ << " iRC = ERROR_MEM_NAME_EXISTS";
    return _rc_;
  }else if ( _rc_ != 0 )
  {
    
    char szError[PATH_MAX];
    unsigned short usRC = 0;
    EqfGetLastError( NULL /*this->hSession*/, &usRC, szError, sizeof( szError ) );
    //buildErrorReturn( iRC, szError, outputMessage );

    outputMessage = "{\"" + strMemName + "\": \"" ;
    switch ( usRC )
    {
      case ERROR_MEM_NAME_INVALID:
        _rest_rc_ = CONFLICT;
        outputMessage += "CONFLICT";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " iRC = CONFLICT";
        break;
      case TMT_MANDCMDLINE:
      case ERROR_NO_SOURCELANG:
      case ERROR_PROPERTY_LANG_DATA:
        _rest_rc_ = BAD_REQUEST;
        outputMessage += "BAD_REQUEST";
        T5LOG(T5ERROR) << "::usRC = " <<  usRC << " iRC = BAD_REQUEST";
        break;
      default:
        _rest_rc_ = INTERNAL_SERVER_ERROR;
        outputMessage += "INTERNAL_SERVER_ERROR";
        T5LOG(T5ERROR) << "::usRC = " << usRC << " iRC = INTERNAL_SERVER_ERROR";
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