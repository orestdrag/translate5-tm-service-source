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
  if(isReadOnlyRequest())
  {

  }else if(isWriteRequest())
  {

  }else{
    
  }

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
        strcpy( pNewMem->stTmSign.szName, pNewMem->TmBtree.fb.fileName.c_str() );
        UtlTime( &(pNewMem->stTmSign.lTime) );
        strcpy( pNewMem->stTmSign.szSourceLanguage,
                strSrcLang.c_str() );

        //TODO - replace version with current t5memory version
        pNewMem->stTmSign.bGlobVersion = T5GLOBVERSION;
        pNewMem->stTmSign.bMajorVersion = T5MAJVERSION;
        pNewMem->stTmSign.bMinorVersion = T5MINVERSION;
        strcpy( pNewMem->stTmSign.szDescription,
                strMemDescription.c_str() );

        //call create function for data file
        pNewMem->usAccessMode = 1;//ASD_LOCKED;         // new TMs are always in exclusive access...
        
        _rc_ = pNewMem->TmBtree.QDAMDictCreateLocal( &(pNewMem->stTmSign),FIRST_KEY );
      
        if ( _rc_ == NO_ERROR )
        {
          //insert initialized record to tm data file
          ulKey = AUTHOR_KEY;
          _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
                    (PBYTE)&pNewMem->Authors, TMX_TABLE_SIZE );

          if ( _rc_ == NO_ERROR )
          {
            ulKey = FILE_KEY;
            _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
                        (PBYTE)&pNewMem->FileNames, TMX_TABLE_SIZE );     
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            ulKey = TAGTABLE_KEY;
            _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
                        (PBYTE)&pNewMem->TagTables, TMX_TABLE_SIZE );
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            ulKey = LANG_KEY;
            _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
                    (PBYTE)&pNewMem->Languages, TMX_TABLE_SIZE );
          } /* endif */

          if ( _rc_ == NO_ERROR )
          {
            int size = sizeof( MAX_COMPACT_SIZE-1 );//OLD, probably bug
            size = MAX_COMPACT_SIZE-1 ;
            //initialize and insert compact area record
            memset( pNewMem->bCompact, 0, size );

            ulKey = COMPACT_KEY;
            _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
                                pNewMem->bCompact, size);  

          } /* endif */

          // add long document table record
          if ( _rc_ == NO_ERROR )
          {
            ulKey = LONGNAME_KEY;
            // write long document name buffer area to the database
            _rc_ = pNewMem->TmBtree.EQFNTMInsert(&ulKey,
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
            strcpy( pNewMem->stTmSign.szName, pNewMem->InBtree.fb.fileName.c_str() );

            _rc_ = pNewMem->InBtree.QDAMDictCreateLocal(&pNewMem->stTmSign, START_KEY );
                                
          } /* endif */

          if(_rc_ == NO_ERROR){   
            pNewMem->TmBtree.fb.Flush();
            pNewMem->InBtree.fb.Flush();     
            //filesystem_flush_buffers(pFullName);  
            //filesystem_flush_buffers(CreateIn.stTmCreate.szIndexName);
          }/* endif */

        } /* endif */

        if ( _rc_ )
        {
          //something went wrong during create or insert so delete data file
          UtlDelete( (PSZ)pNewMem->InBtree.fb.fileName.c_str(), 0L, FALSE );
        } /* endif */
      } /* endif */

      if ( _rc_ )
      {
        //free allocated memory
        NTMDestroyLongNameTable( pNewMem );
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

      //_rc_ = MemFuncCreateMem( strName.c_str(), "", szOtmSourceLang, 0);
      //Data.fComplete = TRUE;   // one-shot function are always complete   

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
    _rc_ = OtmMemoryServiceWorker::getInstance()->decodeBase64ToFile( strMemB64EncodedData.c_str(), strTempFile.c_str(), strError );
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
    EqfGetLastError( NULL /*this->hSession*/, &usRC, szError, sizeof( szError ) );
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




int ExportRequestData::checkData(){
  T5LOG( T5INFO) <<"::getMem::=== getMem request, memory = " << strMemName << "; format = " << requestFormat;
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
  if ( requestFormat.compare( "application/xml" ) == 0 )
  {
    T5LOG( T5INFO) <<"::getMem:: mem = " <<  strMemName << "; supported type found application/xml, tempFile = " << strTempFile;
    //_rc_ = EqfExportMem( this->hSession, (PSZ)strMemName.c_str(), (PSZ)strTempFile.c_str(), TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
    ExportTmx();
    if ( _rc_ != 0 )
    {
      //unsigned short usRC = 0;
      //EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
      //T5LOG(T5ERROR) <<"::getMem:: Error: EqfExportMem failed with rc=" << _rc_ << ", error message is " <<  EncodingHelper::convertToUTF8( this->szLastError);
      return( _rest_rc_ = INTERNAL_SERVER_ERROR );
    }
  }
  //else if ( requestFormat.compare( "application/zip" ) == 0 )
  //{
    //T5LOG( T5INFO) <<"::getMem:: mem = "<< strMemory << "; supported type found application/zip(NOT TESTED YET), tempFile = " << szTempFile;
    //_rc_ = EqfExportMemInInternalFormat( this->hSession, (PSZ)strMemory.c_str(), (PSZ)strTempFile.c_str(), 0 );
    //if ( _rc_ != 0 )
    //{
    //  unsigned short usRC = 0;
    //  EqfGetLastErrorW( this->hSession, &usRC, this->szLastError, sizeof( this->szLastError ) / sizeof( this->szLastError[0] ) );
    //  T5LOG(T5ERROR) <<"::getMem:: Error: EqfExportMemInInternalFormat failed with rc=" <<_rc_ << ", error message is " << EncodingHelper::convertToUTF8( this->szLastError);
    //  return( INTERNAL_SERVER_ERROR );
    //}
  //}
  else
  {
    T5LOG(T5ERROR) <<"::getMem:: Error: the type " << requestFormat << " is not supported" ;
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
    _rc_ = fctdata.MemFuncPrepExport( (PSZ)strMemName.c_str(), (PSZ)strTempFile.c_str(), TMX_UTF8_OPT | OVERWRITE_OPT | COMPLETE_IN_ONE_CALL_OPT );
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

  //EncodingHelper::convertUTF8ToASCII( strMemory );

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

  // get the handle of the memory 
  //int httpRC = TMManager::GetInstance()->getMemoryHandle(strMemName, &lHandle, Data.szError, sizeof( Data.szError ) / sizeof( Data.szError[0] ), &_rc_ );
  lHandle = EqfMemoryPlugin::GetInstance()->openMemoryNew(strMemName);
  //if ( httpRC != OK )
  if(!lHandle)
  {
    buildErrorReturn( _rc_, Data.szError );
    return -1;//( httpRC );
  } /* endif */
  return 0;
}




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
  _rc_ = EqfUpdateMem( OtmMemoryServiceWorker::getInstance()->hSession, lHandle, &Prop, 0 );
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



/*! \brief convert a UTC time value to a OpenTM2 long time value 
    \param pszDateTime buffer containing the UTC date and time
    \param plTime pointer to a long buffer receiving the converted time 
  \returns 0 
*/
int RequestData::convertUTCTimeToLong( char *pszDateTime, PLONG plTime )
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



/*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date and time
  \returns 0 
*/
int RequestData::convertTimeToUTC( LONG lTime, char *pszDateTime )
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

/*

/*! \brief extract a numeric value from a string
    \param pszString string containing the numeric value
    \param iLen number of digits to be processed
    \param piResult pointer to a int value receiving the extracted value
  \returns TRUE if successful and FALSE in case of errors
*/
bool RequestData::getValue( char *pszString, int iLen, int *piResult )
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







