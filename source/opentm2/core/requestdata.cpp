#include "requestdata.h"
#include "tm.h"

#include "../../RestAPI/OTMMSJSONFactory.h"
#include "LogWrapper.h"
#include "EncodingHelper.h"



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
  JSONFactory *factory = JSONFactory::getInstance();
  std::wstring w_msg;
  factory->startJSONW( w_msg );
  factory->addParmToJSONW( w_msg, L"ReturnValue", iRC );
  factory->addParmToJSONW( w_msg, L"ErrorMsg", pszErrorMsg );
  factory->terminateJSONW( w_msg );

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
  JSONFactory *factory = JSONFactory::getInstance();
  factory->startJSON( errorMsg );
  factory->addParmToJSON( errorMsg, "ReturnValue", iRC );
  factory->addParmToJSON( errorMsg, "ErrorMsg", pszErrorMsg );
  factory->terminateJSON( errorMsg );
  
  T5LOG(T5ERROR) << errorMsg << ", ErrorCode = " << iRC;
  return( 0 );
}


CreateMemRequestData::CreateMemRequestData(std::string inputJson){
// parse input parameters
  //std::string strData, strName, strSourceLang, strDescription;
  int iRC = 0;
  JSONFactory *factory = JSONFactory::getInstance();
  void *parseHandle = factory->parseJSONStart( inputJson, &iRC );
  if ( parseHandle == NULL )
  {
    buildErrorReturn( iRC, "Missing or incorrect JSON data in request body" );
    _rest_rc_ =  BAD_REQUEST ;
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
  factory->parseJSONStop( parseHandle );

  if ( !_rest_rc_ && strMemName.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, "Error: Missing memory name input parameter" );
    _rest_rc_ = BAD_REQUEST ;
  } /* end */
  if ( !_rest_rc_ && strSrcLang.empty() )
  {
    iRC = ERROR_INPUT_PARMS_INVALID;
    buildErrorReturn( iRC, "Error: Missing source language input parameter" );
    _rest_rc_= BAD_REQUEST ;
  } /* end */
}