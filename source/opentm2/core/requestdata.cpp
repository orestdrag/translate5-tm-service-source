#include "requestdata.h"
#include "tm.h"
#include "otm.h"
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
    // create memory database
    if (fValid){
      T5LOG( T5DEBUG) << ":: create the memory" ;    
      EqfMemory* pNewMem = new EqfMemory(strMemName);
      ULONG ulKey;
      bool fOK = true;

      _rc_ = NTMCreateLongNameTable( pNewMem );
      
      fOK = (_rc_ == NO_ERROR );
      if ( !fOK )
      {
        _rc_ = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
        //build name and extension of tm data file

        //fill signature record structure
        strcpy( pNewMem->stTmSign.szName, strMemName.c_str() );
        UtlTime( &(pNewMem->stTmSign.lTime) );
        strcpy( pNewMem->stTmSign.szSourceLanguage,
                strSrcLang.c_str() );

        //TODO - replace version with current t5memory version
        pNewMem->stTmSign.bMajorVersion = TM_MAJ_VERSION;
        pNewMem->stTmSign.bMinorVersion = TM_MIN_VERSION;
        strcpy( pNewMem->stTmSign.szDescription,
                strMemDescription.c_str() );

        //call create function for data file
        pNewMem->usAccessMode = 1;//ASD_LOCKED;         // new TMs are always in exclusive access...
        
        _rc_ = pNewMem->pstTmBtree->EQFNTMCreate((PCHAR) &(pNewMem->stTmSign), sizeof(TMX_SIGN),
                            FIRST_KEY);
      
        if ( _rc_ == NO_ERROR )
        {
          //insert initialized record to tm data file
          ulKey = AUTHOR_KEY;
          _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                    (PBYTE)pNewMem->pAuthors, TMX_TABLE_SIZE );

          if ( _rc_ == NO_ERROR )
          {
            ulKey = FILE_KEY;
            _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                        (PBYTE)pNewMem->pFileNames, TMX_TABLE_SIZE );     
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            ulKey = TAGTABLE_KEY;
            _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                        (PBYTE)pNewMem->pTagTables, TMX_TABLE_SIZE );
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            ulKey = LANG_KEY;
            _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                    (PBYTE)pNewMem->pLanguages, TMX_TABLE_SIZE );
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            int size = sizeof( MAX_COMPACT_SIZE-1 );//OLD, probably bug
            size = MAX_COMPACT_SIZE-1 ;
            //initialize and insert compact area record
            memset( pNewMem->bCompact, 0, size );

            ulKey = COMPACT_KEY;
            _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                                pNewMem->bCompact, size);  

          } /* endif */

          // add long document table record
          if ( _rc_ == NO_ERROR )
          {
            ulKey = LONGNAME_KEY;
            // write long document name buffer area to the database
            _rc_ = pNewMem->pstTmBtree->EQFNTMInsert(&ulKey,
                                (PBYTE)pNewMem->pLongNames->pszBuffer,
                                pNewMem->pLongNames->ulBufUsed );        
      
          } /* endif */

          // create language group table
          if ( _rc_ == NO_ERROR )
          {
            _rc_ = NTMCreateLangGroupTable( pNewMem );
            
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            //fill signature record structure
            strcpy( pNewMem->stTmSign.szName, strMemName.c_str() );

            //HERE .TMI file is created
            _rc_ = pNewMem->pstInBtree->EQFNTMCreate((PCHAR) &(pNewMem->stTmSign),
                                sizeof( TMX_SIGN ),
                                START_KEY );
                                
          } /* endif */

          if(_rc_ == NO_ERROR){        
            //filesystem_flush_buffers(pFullName);  
            //filesystem_flush_buffers(CreateIn.stTmCreate.szIndexName);
          }/* endif */

        } /* endif */

        if ( _rc_ )
        {
          //something went wrong during create or insert so delete data file
          UtlDelete( (PSZ)pNewMem->pstInBtree->fb.fileName.c_str(), 0L, FALSE );
        } /* endif */
      } /* endif */

      if ( _rc_ )
      {
        //free allocated memory
        UtlAlloc( (PVOID *) &(pNewMem->pLanguages), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *) &(pNewMem->pAuthors), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *) &(pNewMem->pTagTables), 0L, 0L, NOMSG );
        UtlAlloc( (PVOID *) &(pNewMem->pFileNames), 0L, 0L, NOMSG );
        T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 37 NTMDestroyLongNameTable( pTmClb );";
    #ifdef TEMPORARY_COMMENTED
        NTMDestroyLongNameTable( pNewMem );
        #endif
        //UtlAlloc( (PVOID *) &pTmClb, 0L, 0L, NOMSG );
      } /* endif */

      //set return values
        // setup memory properties
        //this->createMemoryProperties( (PSZ)strMemName.c_str(), strMemPath, (PSZ)strMemDescription.c_str(), (PSZ)strSrcLang.c_str() );
        
        // create memory object if create function completed successfully
        //pNewMemory = new EqfMemory( this, htm, (PSZ)strMemName.c_str() );
        // add memory info to our internal memory list
        //if ( pNewMemory != nullptr ) this->addToList( strMemPath );
        
      }//createMemory code
      //return( (EqfMemory *)pNewMemory );
      

      // check mem
//      if ( pNewMemory == NULL ){
//        _rc_ = getLastError( strLastError );
//        T5LOG(T5ERROR) << "TMManager::createMemory()::EqfMemoryPlugin::GetInstance()->getType() == OtmPlugin::eTranslationMemoryType->::pMemory == NULL, strLastError = " <<  _rc_;
//      }
//      else{
        T5LOG( T5INFO) << "::Create successful ";
        // get memory object name
//        char szObjName[MAX_LONGFILESPEC];
//        _rc_ = TMManager::GetInstance()->getObjectName( pNewMemory, szObjName, sizeof(szObjName) );
//      }
    
      //--- Tm is created. Close it. 
//      if ( pNewMemory != NULL )
//      {
//        T5LOG( T5INFO ) << "::Tm is created. Close it";
//        TMManager::GetInstance()->closeMemory( pNewMemory );
//        pNewMemory = NULL;
//      }
      T5LOG( T5INFO) << " done, usRC = " << _rc_ ;

      //iRC = MemFuncCreateMem( strName.c_str(), "", szOtmSourceLang, 0);
      //pData->fComplete = TRUE;   // one-shot function are always complete   

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