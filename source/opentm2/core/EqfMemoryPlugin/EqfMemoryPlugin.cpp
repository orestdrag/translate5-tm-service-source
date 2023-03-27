/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "PluginManager.h"
#include "PropertyWrapper.H"
#include "EqfMemoryPlugin.h"
#include "EqfMemory.h"
#include "tm.h"
#include "EQFSETUP.H"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
#include "EQF.H"

#include "string"
#include "vector"
#include <fstream>

// some constant values
static char *pszPluginName = "EqfMemoryPlugin";
static char *pszShortDescription = "TranslationMemoryPlugin";
static char *pszLongDescription	= "This is the standard Translation-Memory implementation";
static char *pszVersion = "1.0";
static char *pszSupplier = "International Business Machines Corporation";
//EqfMemoryPlugin* EqfMemoryPlugin::_instance = NULL;

EqfMemoryPlugin::EqfMemoryPlugin()
{
    name = pszPluginName;
    shortDesc = pszShortDescription;
    longDesc = pszLongDescription;
    version = pszVersion;
    supplier = pszSupplier;
    descrType   = "Local Standard Translation Memory";

    iLastError  = 0;
    pluginType  = OtmPlugin::eTranslationMemoryType;
    usableState = OtmPlugin::eUsable;
    UtlGetCheckedEqfDrives( szSupportedDrives );

    this->refreshMemoryList();
}

EqfMemoryPlugin::~EqfMemoryPlugin()
{
}

EqfMemoryPlugin* EqfMemoryPlugin::GetInstance(){
  static EqfMemoryPlugin instance;
  return &instance;
}

const char* EqfMemoryPlugin::getName()
{
	return name.c_str();
}

const char* EqfMemoryPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfMemoryPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfMemoryPlugin::getVersion()
{
	return version.c_str();
}

const char* EqfMemoryPlugin::getSupplier()
{
	return supplier.c_str();
}

const char* EqfMemoryPlugin::getDescriptiveMemType()
{
	return descrType.c_str();
}

/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
int EqfMemoryPlugin::getListOfSupportedDrives( char *pszDriveListBuffer )
{
  strcpy( pszDriveListBuffer, szSupportedDrives );
  return( 0 );
};




void EqfMemoryPlugin::createMemory(CreateMemRequestData& request){
  bool fOK = true;
  T5LOG( T5DEBUG) << ":: create the memory" ;    
  //pMemory = pFactory->createMemory( szPlugin, pszMemName, ( pszDescription == NULL ) ? szEmpty : pszDescription, pszSourceLanguage,NULL, false, &iRC );
  //T5LOG( T5INFO) << "::pszMemoryName = " << pszMemoryName <<  ", pszSourceLanguage = " << pszSourceLanguage <<
  //  ", pszDescription = " << pszDescription ;
  strLastError = "";
  iLastError = 0;

  auto pMemory = createMemory((PSZ)request.strMemName.c_str(), (PSZ)request.strSrcLang.c_str(), (PSZ)request.strMemDescription.c_str(), false, nullptr );
  //pMemory = EqfMemoryPlugin::GetInstance()->createMemory( (PSZ)requestData.strMemName.c_str() , (PSZ)requestData.strSrcLang.c_str(), (PSZ)requestData.strMemDescription.c_str(), FALSE, NULLHANDLE );
  if ( pMemory == NULL ){
  //if ( request.memory.expired() ){ 
    request._rc_ = getLastError( strLastError );
    T5LOG(T5ERROR) << "TMManager::createMemory()::EqfMemoryPlugin::GetInstance()->getType() == OtmPlugin::eTranslationMemoryType->::pMemory == NULL, strLastError = " <<  request._rc_;
    fOK = FALSE;
  }
  else{
    T5LOG( T5INFO) << "::Create successful ";
  }
  // get memory object name
  if ( fOK )
  {
    char szObjName[MAX_LONGFILESPEC];
    TMManager::GetInstance()->getObjectName( pMemory, szObjName, sizeof(szObjName) );
  } 

  //--- Tm is created. Close it. 
  if ( pMemory != NULL )
  {
    T5LOG( T5INFO ) << "::Tm is created. Close it";
    TMManager::GetInstance()->closeMemory( pMemory );
    pMemory = NULL;
  }
  
  if(request._rc_  == 0){
    request._rc_  = (!fOK);
  }
  T5LOG( T5INFO) << " done, usRC = " << request._rc_ ;
}

/*! \brief Create a new translation memory
  \param pszName name of the new memory
	\param pszSourceLang source language
	\param pszDescription description of the memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* EqfMemoryPlugin::createMemory(
	const char* pszName,			  
	const char* pszSourceLang,
	const char* pszDescription,
	BOOL bMsgHandling,
	HWND hwnd
)
{
  EqfMemory *pNewMemory = NULL;        // new memory object
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;
  USHORT usMsgHandling = (USHORT)bMsgHandling;
  BOOL fReserved = FALSE;

  // build memory path and reserve a short name
  this->makeMemoryPath( pszName, strMemPath, &fReserved );

  // use old memory create code
  TmCreate(  (PSZ)strMemPath.c_str(), &htm,  NULL, "",  "",  (PSZ)pszSourceLang,  (PSZ)pszDescription,  usMsgHandling,  hwnd );


  // setup memory properties
  this->createMemoryProperties( pszName, strMemPath, pszDescription, pszSourceLang );
  
  // create memory object if create function completed successfully
  pNewMemory = new EqfMemory( this, htm, (PSZ)pszName );


  // add memory info to our internal memory list
  if ( pNewMemory != NULL ) this->addToList( strMemPath );
  
  // cleanup
  if ( (pNewMemory == NULL) && fReserved )
  {
    std::string strPropName ;

    this->makePropName( strMemPath, strPropName );
    UtlDelete( (char *)strPropName.c_str(), 0, FALSE );
  } /* endif */

  return( (OtmMemory *)pNewMemory );
}

/*! \brief Open an existing translation memory
  \param pszName name of the existing memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
OtmMemory* EqfMemoryPlugin::openMemory(
	const char* pszName,			 
	BOOL bMsgHandling,
	HWND hwnd,
  unsigned short usAccessMode
)
{
  OtmMemory *pMemory = NULL;
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;

  // find memory in our list
 std::shared_ptr<OPENEDMEMORY>  pInfo = this->findMemory( (PSZ)pszName );
  if ( pInfo != NULL )
  {
    // use old memory open code
    USHORT usMsgHandling = (USHORT)bMsgHandling;
    
    std::string path = pInfo->szFullPath;

    USHORT usRC = TmOpen(  (char*)path.c_str(), &htm,  usAccessMode, 0, usMsgHandling,  hwnd );

    // create memory object if create function completed successfully
    if ( (usRC == 0) || ((usRC == BTREE_CORRUPTED) //*
    && (usAccessMode == FOR_ORGANIZE)
    //*/ 
    ))
    {
      pMemory = new EqfMemory( this, htm, (PSZ)pszName );
    }
    else
    {
      T5LOG(T5ERROR) << "EqfMemoryPlugin::openMemory:: TmOpen fails, fName = "<< pInfo->szFullPath<< "; error = "<< this->strLastError<<"; iLastError = "<< 
          this->iLastError;
      //handleError( (int)usRC, (PSZ)pszName, NULL, (PSZ)pInfo->szFullPath, this->strLastError, this->iLastError );
      if ( htm != 0 ) 
        TmClose( htm, NULL,  FALSE,  NULL );
    } /* end */       
  } /* end */     
  else
  {
    // no memory found
    handleError( ERROR_MEMORY_NOTFOUND, (PSZ)pszName, NULL, pInfo->szFullPath, this->strLastError, this->iLastError );
  }

  return( pMemory );
}

/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
int EqfMemoryPlugin::closeMemory(
	OtmMemory *pMemory			 
)
{
  if ( pMemory == NULL ) return( -1 );

  EqfMemory *pMem = (EqfMemory *)pMemory;
  HTM htm = pMem->getHTM();
  
	int iRC = TmClose( htm, NULL,  FALSE,  NULL );

  // refresh memory info
  std::string strMemName;
  pMem->getName( strMemName );
 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (char *)strMemName.c_str() );
  if ( pMemInfo.get() != NULL )
  {
    std::string strPropName;
    std::string pathName = pMemInfo.get()->szFullPath;
    this->makePropName( pathName, strPropName );
    const char* pszName = strrchr( (char*)strPropName.c_str(), '/' );
    pszName = (pszName == NULL) ? (char*)strPropName.c_str() : pszName + 1;
    this->fillInfoStructure( (PSZ)pszName, pMemInfo );  
  }else{
    T5LOG(T5ERROR) <<  "EqfMemoryPlugin::closeMemory pMemInfo == NULL";
  }

  delete( pMemory );  

  return( iRC );
}


/*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
int EqfMemoryPlugin::listMemories(
	OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
)
{

  for ( std::size_t i = 0; i < m_MemInfoVector.size(); i++ )
  {
    std::shared_ptr<OPENEDMEMORY>  pInfo = m_MemInfoVector[i];
    //OPENEDMEMORY *  pInfo = (m_MemInfoVector[i]).get();
    //pfnCallBack( pvData, pInfo.get()->szName, fWithDetails ? pInfo : std::make_shared<OPENEDMEMORY>(nullptr) );
    pfnCallBack( pvData, pInfo.get()->szName, fWithDetails ? pInfo.get() : nullptr );
  } /* end */     
  return( m_MemInfoVector.size() );
}

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
int EqfMemoryPlugin::getMemoryInfo(
	const char* pszName,
 std::shared_ptr<OPENEDMEMORY>  pInfo
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (PSZ)pszName );
  if ( pMemInfo.get() != NULL )
  {
    memcpy( pInfo.get(), pMemInfo.get(), sizeof(OPENEDMEMORY) );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
    memset( pInfo.get(), 0, sizeof(OPENEDMEMORY) );
  } /* endif */
  return( iRC );
}

///*! \brief set description of a memory
//  \param pszName name of the memory
//  \param pszDesc description information
//	\returns 0 if successful or error return code
//*/
int EqfMemoryPlugin::setDescription(const char* pszName, const char* pszDesc)
{
    if(pszName==NULL || pszDesc==NULL)
        return 0;

   std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (PSZ)pszName );
    if ( pMemInfo.get() == NULL )
    {
      // no memory found
      handleError( ERROR_MEMORY_NOTFOUND, (PSZ)pszName, NULL, NULL, this->strLastError, this->iLastError );
      return( ERROR_MEMORY_NOTFOUND );
    }

    // firstly change in memory
    int tLen = sizeof(pMemInfo.get()->szDescription)/sizeof(pMemInfo.get()->szDescription[0])-1;
    strncpy( pMemInfo.get()->szDescription, pszDesc, tLen );
    pMemInfo.get()->szDescription[tLen] = '\0';

    // then change in disk
    char szPathMem[512];
    memset(szPathMem, 0, 512);
    
    //UtlMakeEQFPath( szPathMem, NULC, PROPERTY_PATH, NULL );
    properties_get_str(KEY_MEM_DIR, szPathMem, 512);
    strcat(szPathMem, "/");
    Utlstrccpy( szPathMem+strlen(szPathMem), UtlGetFnameFromPath( pMemInfo.get()->szFullPath ), DOT );
    strcat( szPathMem, EXT_OF_MEM );

    ULONG ulRead;
    PPROP_NTM pProp = NULL;
    BOOL fOK = UtlLoadFileL( szPathMem, (PVOID*)&pProp, &ulRead, FALSE, FALSE );

    if(!fOK || pProp == NULL)
        return -1;

    int length = sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1;
    strncpy(pProp->stTMSignature.szDescription, pszDesc, length);
    pProp->stTMSignature.szDescription[length]='\0';

    int res = UtlWriteFileL( szPathMem, ulRead, (PVOID)pProp, FALSE );

    // relase memory
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );

    return res;
}

/*! \brief provides a list of the memory data files

     This method returns a list of the files which together form the specific memory.
     If there are no real physical files also a dummy name can be and
     the contents of this dummy file can be generated dynamically when the getMemoryPart
     method is applied on this memory.

     The file names are passed to the getMemoryPart method to extract the memory data in
     binary form.

    \param pszName name of the memory
    \param pFileListBuffer  pointer to a buffer receiving the file names as a comma separated list
    \param iBufferSize      size of buffer in number of bytes

  	\returns 0 or error code in case of errors
*/
int EqfMemoryPlugin::getMemoryFiles
(
  const char* pszName,
  char *pFileListBuffer,
  int  iBufferSize
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  *pFileListBuffer = '\0';

 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (PSZ)pszName );

  if ( pMemInfo.get() != NULL )
  {
    // get memory property file name
    std::string strPropName = pMemInfo.get()->szFullPath;
    std::string strMemName = strPropName.substr(0, strPropName.size()-4); 
    std::string strMemDataFile = strMemName + EXT_OF_TMDATA ;
    std::string strMemIndexFile = strMemName + EXT_OF_TMINDEX ;
    std::string res = strPropName + ',' + strMemDataFile + ',' + strMemIndexFile;

    if ( res.size() <= iBufferSize )
    {
      strcpy( pFileListBuffer, res.c_str() ); 
    }
    else
    {
      iRC = OtmMemoryPlugin::eBufferTooSmall;
    } /* endif */
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief import a memory using a list of memory data files

    This method imports the binary files of a memory. The files have been created and
    filled using the getMemoryPart method.

    This method should delete the memory data files at the end of the processing- 

    When the processing of the memory files needs more time, the method
    should process the task in small units in order to prevent blocking of the
    calling application. To do this the method should return
    OtmMemoryPugin::eRepeat and should use the pPrivData pointer to anchor
    a private data area to keep track of the current processing step. The method will
    be called repetetively until the import has been completed.

   \param pszMemoryName    name of the memory 
   \param pFileList        pointer to a buffer containing the fully qualified memory data files as a comma separated list
   \param iOptions         processing options, one or more of the IMPORTFROMMEMFILES_..._OPT values ORed together
                           
   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfMemoryPlugin::importFromMemoryFiles
(
  const char *pszMemoryName,
  const char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  LOG_UNIMPLEMENTED_FUNCTION;
  throw;
  return( -1 );
}



/*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
int EqfMemoryPlugin::renameMemory(
	const char* pszOldName,
  const char* pszNewName
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (PSZ)pszOldName );
  if ( pMemInfo.get() != NULL )
  {
    // get new short name for memory
    BOOL fIsNew = FALSE;
    char szShortName[MAX_FILESPEC];
    
    T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 16 ObjLongToShortName( pszNewName, szShortName, TM_OBJECT, &fIsNew )";
#ifdef TEMPORARY_COMMENTED
    ObjLongToShortName( pszNewName, szShortName, TM_OBJECT, &fIsNew );
    #endif

    if ( !fIsNew ) return( OtmMemory::ERROR_MEMORYEXISTS );

    // get memory property file name
    std::string strPropName;
    std::string strMemPath = pMemInfo.get()->szFullPath;
    this->makePropName( strMemPath, strPropName ); 

    // rename index file
    char szIndexPath[MAX_LONGFILESPEC];
    char szNewPath[MAX_LONGFILESPEC];
    strcpy( szIndexPath, pMemInfo.get()->szFullPath );
    char *pszExt = strrchr( szIndexPath, DOT );
    strcpy( szNewPath, pMemInfo.get()->szFullPath );
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    strcpy( strrchr( szIndexPath, DOT ), EXT_OF_TMINDEX );
    strcat( szNewPath, EXT_OF_TMINDEX );
    
    rename( szIndexPath, szNewPath ) ;

    // rename data file
    strcpy( szNewPath, pMemInfo.get()->szFullPath );
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    strcat( szNewPath, EXT_OF_TMDATA );
    rename( pMemInfo.get()->szFullPath, szNewPath ) ;

    // adjust data file name in memory info area
    strcpy( pMemInfo.get()->szFullPath, szNewPath ) ;

    // rename the property file
    strcpy( szNewPath, strPropName.c_str() ); 
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    strcat( szNewPath, EXT_OF_MEM  );
    rename( strPropName.c_str(), szNewPath ) ;

    // update property file
    PPROP_NTM pstMemProp = NULL;
    ULONG ulRead = 0;
    if ( UtlLoadFileL( szNewPath, (PVOID *)&pstMemProp, &ulRead, FALSE, FALSE ) )
    {      
      // update name in signature structure
      strcpy( pstMemProp->stTMSignature.szName, szShortName );

      // re-write property file
      UtlWriteFileL( szNewPath, ulRead, pstMemProp, FALSE );

      // free property area
      UtlAlloc( (PVOID *)&pstMemProp, 0, 0, NOMSG );
    } /* endif */     

    // update memory name in info data
    strcpy( pMemInfo.get()->szName, pszNewName );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
int EqfMemoryPlugin::deleteMemory(
	const char* pszName			  
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
 std::shared_ptr<OPENEDMEMORY>  pMemInfo;

  // get the memory info index int the vector and use it to get memory info
  int idx = this->findMemoryIndex(pszName);
  if( idx>=0 && idx<((int)m_MemInfoVector.size()) )
      pMemInfo = m_MemInfoVector[idx];

  if ( (pMemInfo.get() != NULL) && (pMemInfo.get()->szFullPath[0] != EOS) )
  {
    // delete the property file
    T5LOG( T5DEBUG) << "EqfMemoryPlugin::deleteMemory:: try to delete property file: " << pMemInfo.get()->szFullPath ;

    UtlDelete( pMemInfo.get()->szFullPath, 0L, FALSE );

    char szPath[MAX_LONGFILESPEC];
    strcpy( szPath, pMemInfo.get()->szFullPath );
    strcpy( strrchr( szPath, DOT ), EXT_OF_TMINDEX );
    // delete index file
    T5LOG( T5DEBUG) <<"EqfMemoryPlugin::deleteMemory:: try to delete index file: "<< szPath ;
    UtlDelete( szPath, 0L, FALSE );
    
    // delete data file
    strcpy( strrchr( szPath, DOT ), EXT_OF_TMDATA );
    T5LOG( T5DEBUG) << "EqfMemoryPlugin::deleteMemory:: try to delete data file: "<< szPath ;    
    UtlDelete( szPath, 0L, FALSE );

    // remove memory infor from our memory info vector
    m_MemInfoVector.erase(m_MemInfoVector.begin( )+idx);
  }
  else
  {
    T5LOG(T5ERROR) << "EqfMemoryPlugin::deleteMemory:: OtmMemoryPlugin::eMemoryNotFound name = "<< pszName ;
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Clear (i.e. remove all entries) a translation memory
  \param pszName name of the memory being cleared
	\returns 0 if successful or error return code
*/
int EqfMemoryPlugin::clearMemory(
	const char* pszName			  
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( (PSZ)pszName );
  if ( pMemInfo.get() != NULL )
  {
    // delete data and index file
    UtlDelete( pMemInfo.get()->szFullPath, 0L, FALSE );

    char szIndexPath[MAX_LONGFILESPEC];
    strcpy( szIndexPath, pMemInfo.get()->szFullPath );
    char *pszExt = strrchr( szIndexPath, DOT );
    strcpy( strrchr( szIndexPath, DOT ), EXT_OF_TMINDEX );
    UtlDelete( szIndexPath, 0L, FALSE );

    // use TmtXCreate to create new data and index file
    PTMX_CREATE_IN pTmCreateIn = new (TMX_CREATE_IN);
    memset( pTmCreateIn, 0, sizeof(TMX_CREATE_IN) );

    strcpy( pTmCreateIn->stTmCreate.szDataName, pMemInfo.get()->szFullPath );
    strcpy( pTmCreateIn->stTmCreate.szSourceLanguage, pMemInfo.get()->szSourceLanguage );
    strcpy( pTmCreateIn->stTmCreate.szSourceLanguage, pMemInfo.get()->szSourceLanguage );
    strcpy( pTmCreateIn->stTmCreate.szDescription, pMemInfo.get()->szDescription );

    PTMX_CREATE_OUT pTmCreateOut = new (TMX_CREATE_OUT);
    memset( pTmCreateOut, 0, sizeof(TMX_CREATE_OUT) );

    iRC = (int)TmtXCreate( pTmCreateIn, pTmCreateOut );

    free( pTmCreateIn );
    free( pTmCreateOut );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* EqfMemoryPlugin::createTempMemory(
	const char* pszPrefix,			  
	const char* pszName,			  
	const char* pszSourceLang,
	BOOL bMsgHandling,
	HWND hwnd
)
{
  EqfMemory *pNewMemory = NULL;        // new memory object
  HTM htm = NULL;                      // memory handle 
  USHORT usRC = 0;

  bMsgHandling;

T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 18 // use old temporary memory create code  usRC = TMCreateTempMem( pszPrefix, pszName, &htm, NULL, pszSourceLang, hwnd );";
#ifdef TEMPORARY_COMMENTED
  // use old temporary memory create code
  usRC = TMCreateTempMem( pszPrefix, pszName, &htm, NULL, pszSourceLang, hwnd );
#endif //TEMPORARY_COMMENTED
  
  // create memory object if create function completed successfully
  if ( usRC == 0 )
  {
    pNewMemory = new EqfMemory( this, htm, (PSZ)pszName );
  } /* end */     

  return( (OtmMemory *)pNewMemory );
}

/*! \brief close and delete a temporary memory
  \param pMemory pointer to memory objject
*/
void EqfMemoryPlugin::closeTempMemory(
	OtmMemory *pMemory
)
{
  std::string strName;

  // get the memory name
  pMemory->getName( strName );

  // close the memory
  this->closeMemory( pMemory );

T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 19 // use old temporary memory delete code  TMDeleteTempMem( (PSZ)strName.c_str() );";
#ifdef TEMPORARY_COMMENTED
  // use old temporary memory delete code
  TMDeleteTempMem( (PSZ)strName.c_str() );
#endif //TEMPORARY_COMMENTED
  
  return;
}


/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int EqfMemoryPlugin::getLastError
(
  std::string &strError
)
{
  strError = this->strLastError;
  return( this->iLastError );
}

  /*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int EqfMemoryPlugin::getLastError
(
  char *pszError,
  int iBufSize
)
{
  strncpy( pszError, this->strLastError.c_str(), iBufSize );
  return( this->iLastError );
}

/* private methods */

/*! \brief Refresh the internal list of translation memory dbs
*/
void EqfMemoryPlugin::refreshMemoryList()
{
  //if ( this->pMemList != NULL )
  //{
  //    // delete all the elements in vector
  //    for (std::vector<std::shared_ptr<OPENEDMEMORY> >::iterator iter = this->pMemList->begin();
  //        iter != this->pMemList->end();
  //        iter++
  //        )
  //    {
  //        delete *iter;
  //    }
  //   // then clear the vector, without above step, its' a memory leak
  //   this->pMemList->clear();
  //}
  //else
  //{
  //    return;
  //}
  // clear the old vector
  m_MemInfoVector.clear();

  
  char mem_dir[MAX_EQF_PATH];
  {
    //prepare path for searching
    properties_get_str(KEY_MEM_DIR, mem_dir, MAX_EQF_PATH);
    strncpy(this->szBuffer, mem_dir, MAX_EQF_PATH );
    sprintf( this->szBuffer + strlen(szBuffer), "%s%s", DEFAULT_PATTERN_NAME, EXT_OF_MEM );
  }

  auto files = FilesystemHelper::FindFiles(this->szBuffer);
  int errcode = 0;
  if(FilesystemHelper::GetLastError() == FilesystemHelper::FILEHELPER_ERROR_CANT_OPEN_DIR){
      errcode = FilesystemHelper::CreateDir(mem_dir);
      if( FilesystemHelper::GetLastError() != FilesystemHelper::FILEHELPER_NO_ERROR ){
        T5LOG(T5FATAL) << ":: error with filesystem helper, errcode = " << errcode;
        throw;
      }
  }  

  for(auto file: files){
    addToList(file);
  }

  return;
} /* end of method RefreshMemoryList */

int tryStrCpy(char* dest, const char* src, const char* def){
  if(src && strlen(src)){
    strcpy(dest,src);
    return 1;
  }
  else{
    strcpy(dest, def);
    return 2;
  }
  
}

/*! \brief Fill memory info structure from memory properties
  \param pszPropName name of the memory property file (w/o path) 
	\param pInfo pointer to memory info structure
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfMemoryPlugin::fillInfoStructure
(
   char *pszPropName,
   std::shared_ptr<OPENEDMEMORY>  pInfo
)
{
  if(pInfo==0 || pszPropName==0){
    T5LOG(T5ERROR) <<  "EqfMemoryPlugin::fillInfoStructure():: pInfo==0 || pszPropName==0";
    return false;
  }

  PROP_NTM prop;

  memset( pInfo.get(), 0, sizeof(OPENEDMEMORY) );

  // init it, if not meet some condition ,it will be set to false
  pInfo->fEnabled = TRUE;

  std::string mem_path;
  int errCode = 0;
  {
    char path[MAX_EQF_PATH];
    errCode = properties_get_str(KEY_MEM_DIR, path, MAX_EQF_PATH);
    if(errCode){
      T5LOG(T5ERROR) << "EqfMemoryPlugin::fillInfoStructure():: errCode = " << errCode;
      return errCode;
    }
    if(strlen(path) && path[strlen(path)-1] != '/')
      strcat(path, "/");
    strcat(path, pszPropName);
    mem_path = path;
    mem_path = FilesystemHelper::FixPath(mem_path);
  }
  
  std::string nameWithoutExtention = pszPropName;
  auto dot = nameWithoutExtention.rfind('.');
  if(dot != std::string::npos){
    nameWithoutExtention = nameWithoutExtention.substr(0, dot);
  }
  dot = mem_path.rfind('.');
  std::string dataFilePath;
  std::string indexFilePath;
  if(dot != std::string::npos){
    mem_path = mem_path.substr(0, dot);
    dataFilePath = mem_path + EXT_OF_TMDATA;
    indexFilePath = mem_path + EXT_OF_TMINDEX;
    mem_path = mem_path + EXT_OF_MEM;
  }
  //strcpy( pInfo->szFullPath, prop.szFullMemName);
  //strcpy( pInfo->szName, prop.stPropHead.szName);
  strcpy( pInfo->szFullPath, mem_path.c_str());
  strcpy( pInfo->szName, nameWithoutExtention.c_str());
  strcpy( pInfo->szFullDataFilePath, dataFilePath.c_str());
  strcpy(pInfo->szFullIndexFilePath, indexFilePath.c_str());

  auto memFile = FilesystemHelper::OpenFile(mem_path,"rb", true );
  auto pData = FilesystemHelper::GetFilebufferData(mem_path);
  
  if(pData == NULL || pData->size()<sizeof(PROP_NTM)){
    T5LOG(T5ERROR) << ":: pData == NULL || pData->size()<sizeof(PROP_NTM) for " << mem_path;
    return -1;
  }

  PPROP_NTM pProp = (PPROP_NTM) (&((*pData)[0]));
  
  strcpy(pInfo->szDescription, pProp->stTMSignature.szDescription );
  strcpy( pInfo->szSourceLanguage, pProp->stTMSignature.szSourceLanguage );
  
  strcpy( pInfo->szPlugin, this->name.c_str());
  strcpy( pInfo->szDescrMemoryType, this->descrType.c_str());
  //strcpy( pInfo->szOwner, "");

  pInfo->ulSize = FilesystemHelper::GetFileSize(mem_path);
  pInfo->ulSize += FilesystemHelper::GetFileSize(dataFilePath);
  pInfo->ulSize += FilesystemHelper::GetFileSize(indexFilePath);
  FilesystemHelper::CloseFile(memFile);

  return( errCode == 0 );

}

/*! \brief Find memory in our memory list and return pointer to memory info 
  \param pszName name of the memory  
	\returns std::shared_ptr<OPENEDMEMORY>  pInfo pointer to memory info or NULL in case of errors
*/
std::shared_ptr<OPENEDMEMORY>  EqfMemoryPlugin::findMemory
(
   const char *pszName
)
{
  int idx = findMemoryIndex(pszName);
  if( idx>=0 && idx<((int)m_MemInfoVector.size()) )
  {
      return m_MemInfoVector[idx];
  }
  return nullptr;
}

// return the memory index of the specified memory name
int EqfMemoryPlugin::findMemoryIndex
(
   const char *pszName
)
{
    for ( int i = 0; i < (int)m_MemInfoVector.size(); i++ )
    {
        OPENEDMEMORY *  pInfo = (m_MemInfoVector[i]).get();
        if ( strcasecmp( pszName, pInfo->szName) == 0 )
        {
            return i;
        }
    }
  
    return (-1);
}


bool EqfMemoryPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

//__attribute__((visibility("default")))
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
  
  EqfMemoryPlugin* plugin = EqfMemoryPlugin::GetInstance();
  eRc = manager->registerPlugin((OtmPlugin*) plugin);
  USHORT usRC = (USHORT) eRc;
  return usRC;
}

//#endif

/*! \brief Build the path name for a memory
  \param pszName (long) name of the new memory
	\param string receiving the memory path name
	\returns TRUE when successful 
*/
BOOL EqfMemoryPlugin::makeMemoryPath( const char* pszName, std::string &strPathName, PBOOL pfReserved )
{
  BOOL fOK = FALSE;
  OBJLONGTOSHORTSTATE ObjState;

  CreateMemFile( (PSZ)pszName , &ObjState);

  if ( pfReserved != NULL ) 
      *pfReserved = ObjState == OBJ_IS_NEW;

  if ( ObjState == OBJ_IS_NEW ){
      char buff[255];
      properties_get_str(KEY_MEM_DIR, buff, 255);
      strPathName = buff;      
      strPathName += "/" + std::string(pszName) + EXT_OF_TMDATA;
      fOK = true;
  }else{
    strPathName = pszName;
    fOK = MemCreatePath( (char*) strPathName.c_str() );
  }
  
  strPathName = FilesystemHelper::FixPath(strPathName);

  return( fOK );
}

/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pszDescription memory description
	\param pszSourceLanguage memory source language
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfMemoryPlugin::createMemoryProperties( const char* pszName, std::string &strPathName, const char* pszDescription, const char* pszSourceLanguage )
{
  BOOL fOK = TRUE;
  PPROP_NTM pProp = NULL;
  USHORT usPropSize = get_max( sizeof(PROP_NTM), MEM_PROP_SIZE );

  fOK = UtlAlloc( (void **)&pProp, 0, usPropSize, NOMSG );

  // init memory
  memset((void*)pProp,0,usPropSize);
  if ( fOK )
  {
    // fill properties file
    pProp->stPropHead.usClass = PROP_CLASS_MEMORY;
    pProp->stPropHead.chType = PROP_TYPE_NEW;

    char * fname = UtlGetFnameFromPath(  strPathName.c_str() );
    Utlstrccpy( pProp->stPropHead.szName, fname, DOT );
    strcpy(pProp->stTMSignature.szName, pProp->stPropHead.szName);
    strcat( pProp->stPropHead.szName, EXT_OF_MEM );

    strncpy( pProp->stTMSignature.szDescription, 
             pszDescription, 
             sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1);
    strncpy( pProp->stTMSignature.szSourceLanguage, pszSourceLanguage, 
             sizeof(pProp->stTMSignature.szSourceLanguage)/sizeof(pProp->stTMSignature.szSourceLanguage[0])-1);

    pProp->stTMSignature.bMajorVersion = TM_MAJ_VERSION;
    pProp->stTMSignature.bMinorVersion = TM_MIN_VERSION;
    strcpy( pProp->szNTMMarker, NTM_MARKER );
    UtlTime( &pProp->stTMSignature.lTime );
    pProp->usThreshold = TM_DEFAULT_THRESHOLD;

    // write properties to disk
    std::string strPropName;
    this->makePropName( strPathName, strPropName );

    //WritePropFile(cstr, (PVOID)pProp, sizeof(PROPSYSTEM));
    T5LOG( T5INFO) << "createMemoryProperties called for file " << strPropName << " with fsize = " << usPropSize;    
    fOK = UtlWriteFile( (char *)strPropName.c_str() , usPropSize, (PVOID)pProp, FALSE );

    UtlAlloc( (void **)&pProp, 0, 0, NOMSG );
  } /* endif */     
  return( fOK );
}

/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pvOldProperties existing property file to be used for the fields of the new properties
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfMemoryPlugin::createMemoryProperties( const char* pszName, std::string &strPathName, void *pvOldProperties )
{
  BOOL fOK = TRUE;
  PPROP_NTM pProp = NULL;
  PPROP_NTM pOldProp = (PPROP_NTM)pvOldProperties;
  USHORT usPropSize = get_max( sizeof(PROP_NTM), MEM_PROP_SIZE );

  fOK = UtlAlloc( (void **)&pProp, 0, usPropSize, NOMSG );

  // init memory
  memset((void*)pProp,0,usPropSize);
  if ( fOK )
  {
    // fill properties file
    pProp->stPropHead.usClass = PROP_CLASS_MEMORY;
    pProp->stPropHead.chType = PROP_TYPE_NEW;

    T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 23 strncpy( pProp->stPropHead.szPath, strPathName.c_str(),  sizeof(pProp->stPropHead.szPath)/sizeof(pProp->stPropHead.szPath[0]));";
#ifdef TEMPORARY_COMMENTED
    strncpy( pProp->stPropHead.szPath, strPathName.c_str(), 
             sizeof(pProp->stPropHead.szPath)/sizeof(pProp->stPropHead.szPath[0]));
    Utlstrccpy( pProp->stPropHead.szName, UtlSplitFnameFromPath( pProp->stPropHead.szPath ), DOT );
 
    strcat( pProp->stPropHead.szName, EXT_OF_MEM );

    UtlSplitFnameFromPath( pProp->stPropHead.szPath );
    //in case of overflow. change these strcpy to strncpy
    strncpy( pProp->szFullMemName, strPathName.c_str(), sizeof(pProp->szFullMemName)/sizeof(pProp->szFullMemName[0])-1);
    strncpy( pProp->szLongName, pszName, sizeof(pProp->szLongName)/sizeof(pProp->szLongName[0])-1);
    strncpy( pProp->stTMSignature.szDescription, pOldProp->stTMSignature.szDescription, 
             sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1);
    strncpy( pProp->stTMSignature.szSourceLanguage, pOldProp->stTMSignature.szSourceLanguage, 
             sizeof(pProp->stTMSignature.szSourceLanguage)/sizeof(pProp->stTMSignature.szSourceLanguage[0])-1);
    strcpy( pProp->stTMSignature.szUserid, pOldProp->stTMSignature.szUserid );
    #endif

    pProp->stTMSignature.bMajorVersion = pOldProp->stTMSignature.bMajorVersion;
    pProp->stTMSignature.bMinorVersion = pOldProp->stTMSignature.bMinorVersion;
    strcpy( pProp->szNTMMarker, NTM_MARKER );
    pProp->stTMSignature.lTime = pOldProp->stTMSignature.lTime;
    pProp->usThreshold = pOldProp->usThreshold ;

    // write properties to disk
    std::string strPropName;
    this->makePropName( strPathName, strPropName );
    fOK = UtlWriteFile( (char *)strPropName.c_str() , usPropSize, (PVOID)pProp, FALSE );
    UtlAlloc( (void **)&pProp, 0, 0, NOMSG );
  } /* endif */     
  return( fOK );
}

/*! \brief Make the fully qualified property file name for a memory
  \param strPathName reference to the memory path name
  \param strPropName reference to the string receiving the property file name
	\returns 0 when successful
*/
int EqfMemoryPlugin::makePropName( std::string &strPathName, std::string &strPropName )
{
  char szFullPropName[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szFullPropName, NULC, PROPERTY_PATH, NULL );
  properties_get_str(KEY_MEM_DIR, szFullPropName, MAX_LONGFILESPEC-1);
  strcat( szFullPropName, BACKSLASH_STR );
  Utlstrccpy( szFullPropName + strlen(szFullPropName), UtlGetFnameFromPath( (char *)strPathName.c_str() ), DOT );
  strcat( szFullPropName, EXT_OF_MEM );
  strPropName = szFullPropName;
  return( 0 );
}
 
/*! \brief Add memory to our internal memory lisst
  \param strPathName reference to the memory path name
	\returns 0 when successful
*/
int EqfMemoryPlugin::addToList( std::string &strPathName )
{
  return( this->addToList( (PSZ)strPathName.c_str() ) );
}

/*! \brief Add memory to our internal memory lisst
  \param pszPropname pointer to property file name
	\returns 0 when successful
*/
int EqfMemoryPlugin::addToList( char *pszPropName )
{
 std::shared_ptr<OPENEDMEMORY> pInfo =  std::make_shared<OPENEDMEMORY>( OPENEDMEMORY() );
  if(pInfo.get() != 0)
  {
    // find the property name when pszPropName is fully qualified
    const char* pszName = strrchr( pszPropName, '/' );
    pszName = (pszName == NULL) ? pszPropName : pszName + 1;
    if ( this->fillInfoStructure( (PSZ)pszName, pInfo ) )
    {
      m_MemInfoVector.push_back( pInfo);
    }
    else
    {
    //  delete( pInfo );
    }
  }
  return( 0 );
}

/*! \brief make Index filename from memory data file name
  \param pszMemPath pointer to memory data file name
  \param pszIndexFileName pointer to a buffer for the memory index file name
	\returns 0 when successful
*/
int EqfMemoryPlugin::makeIndexFileName( char *pszMemPath, char *pszIndexFileName , bool fTmdFile)
{
  char *pszExt = NULL;
  strcpy( pszIndexFileName, pszMemPath );
  pszExt = strrchr( pszIndexFileName, DOT );
  if ( fTmdFile ) strcpy( pszExt, EXT_OF_TMDATA );
  else  strcpy( pszExt, EXT_OF_TMINDEX );
  
  return( 0 );
}

/*! \brief make Index filename from memory data file name
  \param strMemPath reference to a string containing the memory data file name
  \param strIndexFileName reference to a string receiving the memory index file name
	\returns 0 when successful
*/
int EqfMemoryPlugin::makeIndexFileName( std::string &strMemPath, std::string &strIndexFileName )
{
  size_t ExtPos = strMemPath.find_last_of( '.' );
  strIndexFileName =strMemPath.substr( 0, ExtPos );
  strIndexFileName.append( EXT_OF_TMINDEX );
  return( 0 );
}

/*! \brief Private data area for the memory import from data files */
typedef struct _MEMIMPORTFROMFILEDATA
{
  EqfMemory *pInputMemory;             // input memory
  EqfMemory *pOutputMemory;            // output memory
  OtmProposal  Proposal;               // currently processed proposal
  std::string strPropFile;             // property file of the imorted memory
  std::string strDataFile;             // data file of the imorted memory
  std::string strIndexFile;            // index file of the imorted memory
  char  szBuffer[1014];                // general purpose buffer

} MEMIMPORTFROMFILEDATA, *PMEMIMPORTFROMFILEDATA;

/*! \brief Initialize the import of a memory using a list of memory data files


   \param pszMemoryName    name of the memory 
   \param pFileList        pointer to a buffer containing the fully qualified memory data files as a comma separated list
   \param iOptions         processing options, one or more of the IMPORTFROMMEMFILES_..._OPT values ORed together
                           
   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfMemoryPlugin::importFromMemFilesInitialize
(
  const char *pszMemoryName,
  const char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
 std::shared_ptr<OPENEDMEMORY>  pMemInfo = this->findMemory( pszMemoryName );
  std::string strPropFile = "";
  std::string strDataFile = "";
  std::string strIndexFile = "";

  iOptions;

  // extract names of the imported files from the file list
  {
     // loop over imported files and extract them
     std::string strFiles = pFileList;
     size_t Pos = 0;
     BOOL fComplete = FALSE;
     while ( !fComplete )
     {
       // extract next file name from list
       std::string strCurFile;
       size_t End = strFiles.find_first_of( ',', Pos );
       if ( End == std::string::npos )
       {
         strCurFile = strFiles.substr( Pos, std::string::npos );
         fComplete = TRUE;
       }
       else
       {
         strCurFile = strFiles.substr( Pos, End - Pos );
         Pos = End + 1;
       } /* endif */     

       // get position of file extention in current file
       size_t ExtPos = strCurFile.find_last_of( '.' );

       // process current file name
       if ( strCurFile.compare( ExtPos, std::string::npos, EXT_OF_MEM ) == 0 )
       {
          strPropFile = strCurFile;   
       }
       else if ( strCurFile.compare( ExtPos, std::string::npos, EXT_OF_TMDATA ) == 0 )
       {
          strDataFile = strCurFile;   
       }
       else if ( strCurFile.compare( ExtPos, std::string::npos, EXT_OF_TMINDEX ) == 0 )
       {
          strIndexFile = strCurFile;   
       }
       else
       {
         // delete unknown file type
         UtlDelete( (PSZ)strCurFile.c_str(), 0L, FALSE );
       } /* endif */
     } /* endwhile */        
  }


  if ( pMemInfo.get() != NULL )
  {
    PMEMIMPORTFROMFILEDATA pData = NULL;

    // this memory exists already and we have to merge the data from the memory data files into
    // the existing memory, so prepare the merge of the memory

    // allocate and anchor our memory merge data structure
    pData = new( MEMIMPORTFROMFILEDATA );
    if ( pData == NULL ) return( OtmMemoryPlugin::eNotEnoughMemory );
    *ppPrivateData = (PVOID)pData;

    // store memory file names in private data area
    pData->strPropFile = strPropFile;
    pData->strDataFile = strDataFile;
    pData->strIndexFile = strIndexFile;

    // open the target memory
    pData->pOutputMemory = (EqfMemory *)this->openMemory( pszMemoryName, FALSE, NULLHANDLE, EXCLUSIVE );
    if ( pData->pOutputMemory == NULL )
    {
      return( this->iLastError );
    } /* end */       

    // open the data file of the imported memory file
    // use old memory open code
    HTM htm = 0;
    // commented temporarily to link the plugin
    USHORT usRC = TmOpen( (PSZ)strDataFile.c_str(), &htm,  EXCLUSIVE, 0, FALSE,  NULLHANDLE );
    if ( usRC != 0 )
    {
      handleError( (int)usRC, (char *)strDataFile.c_str(), NULL, (char *)strDataFile.c_str(), this->strLastError, this->iLastError );

      // close the target memory which has already been opened 
      if ( pData->pOutputMemory )
      {
        this->closeMemory( pData->pOutputMemory );
        pData->pOutputMemory = NULL;
      }
      
      return( this->iLastError );      
    } /* end */   

    pData->pInputMemory = new EqfMemory( this, htm, (PSZ)pszMemoryName  );

    // get first proposal and copy it to output memory
    int iMemRC = pData->pInputMemory->getFirstProposal( pData->Proposal );
    if ( iMemRC == NO_ERROR)
    {
      pData->pOutputMemory->putProposal( pData->Proposal );
      iRC = OtmMemoryPlugin::eRepeat;
    }
    else if ( (iMemRC == BTREE_EOF_REACHED) || (iMemRC == OtmMemory::INFO_ENDREACHED) )
    {
      // nothing more to do
      this->importFromMemFilesEndProcessing( ppPrivateData );
    }
    else
    {
      // something failed, report error and go to end processing
      handleError( iMemRC, (char *)strDataFile.c_str(), NULL, (char *)strDataFile.c_str(), this->strLastError, this->iLastError );
      iRC = this->iLastError;

      // close the target memory which has already been opened 
      if ( pData->pOutputMemory )
      {
        this->closeMemory( pData->pOutputMemory );
        pData->pOutputMemory = NULL;
      }
    }
  }
  else
  {
    // this is a new memory, so we can move the files to the approbriate location and create a new property file for it
     std::string strMemPath;

     // build memory path for the imported memory
     this->makeMemoryPath( pszMemoryName, strMemPath );

      // process property file
      {
        PPROP_NTM pProp = NULL;
        USHORT usLen = 0;
        UtlLoadFile( (PSZ)strPropFile.c_str(), (PVOID *)&pProp, &usLen, FALSE, FALSE );
        if ( pProp != NULL )
        {
          // setup new memory properties based on imported memory property file
          this->createMemoryProperties( pszMemoryName, strMemPath, (void *)pProp );

          // delete imported property file
          UtlDelete( (PSZ)strPropFile.c_str(), 0L, FALSE );

          // free memory of loaded property file
          UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        } /* end */             
       }

       // process data file
       {
         UtlMove( (PSZ)strDataFile.c_str(), (PSZ)strMemPath.c_str(), 0L, FALSE );
       }

       // process index file
       {
         std::string strNewIndexFile;
         this->makeIndexFileName( strMemPath, strNewIndexFile );
         UtlMove( (PSZ)strIndexFile.c_str(), (PSZ)strNewIndexFile.c_str(), 0L, FALSE );
       } /* endif */

       // add memory to our memory list
       this->addToList( strMemPath );
  } /* endif */
  return( iRC );
}

/*! \brief Continue the import of a memory using a list of memory data files


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfMemoryPlugin::importFromMemFilesContinueProcessing
(
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  int iMemRC = NO_ERROR;
  PMEMIMPORTFROMFILEDATA pData = (PMEMIMPORTFROMFILEDATA)*ppPrivateData;

  for ( int i = 1; i < 10; i++ )
  {
    iMemRC = pData->pInputMemory->getNextProposal( pData->Proposal );
    if ( iMemRC == NO_ERROR )
    {
      pData->pOutputMemory->putProposal( pData->Proposal );
    }
    else
    {
      break;
    }
  } /* endfor */     

  if ( iMemRC == NO_ERROR)
  {
    iRC = OtmMemoryPlugin::eRepeat;
  }
  else if ( iMemRC == OtmMemory::INFO_ENDREACHED )
  {
    this->importFromMemFilesEndProcessing( ppPrivateData );
    iRC = OtmMemoryPlugin::eSuccess;
  }
  else
  {
    this->importFromMemFilesEndProcessing( ppPrivateData );
    iRC = iMemRC;
  }
  return( iRC );
}

  /*! \brief Terminate the import of a memory using a list of memory data files and do a cleanup


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             any other value is an error code
 */
int EqfMemoryPlugin::importFromMemFilesEndProcessing
(
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  PMEMIMPORTFROMFILEDATA pData = (PMEMIMPORTFROMFILEDATA)*ppPrivateData;

  if ( pData != NULL )
  {
    // close memories
    if ( pData->pOutputMemory != NULL ) this->closeMemory( pData->pOutputMemory );

    if ( pData->pInputMemory != NULL ) 
    {
      HTM htm = pData->pInputMemory->getHTM();
T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 29 TmClose( htm, NULL,  FALSE,  NULL );";
#ifdef TEMPORARY_COMMENTED
      TmClose( htm, NULL,  FALSE,  NULL );
#endif //TEMPORARY_COMMENTED

      delete( pData->pInputMemory );  
    } /* end */       

    // delete memory data files
    if ( pData->strPropFile.length() != 0 ) UtlDelete( (PSZ)pData->strPropFile.c_str(), 0L, FALSE );
    if ( pData->strDataFile.length() != 0 ) UtlDelete( (PSZ)pData->strDataFile.c_str(), 0L, FALSE );
    if ( pData->strIndexFile.length() != 0 ) UtlDelete( (PSZ)pData->strIndexFile.c_str(), 0L, FALSE );

    // free data area
    free( pData );
  } /* end */     

  return( iRC );
}


/* \brief add a new memory information to memory list
   \param pszName memory name
   \param chToDrive drive letter
   \returns 0 if success
*/
int EqfMemoryPlugin::addMemoryToList(const char* pszName)
{
    if( pszName && findMemory(pszName) == NULL){
        // only could be added when its property exists
        std::string pathName = pszName;
        pathName += EXT_OF_TMDATA;
        addToList( pathName );
        return 0;
    }
    
    // already exist, just return
    return -1;
}


/* \brief remove a memory information from memory list
   \param  pszName memory name
   \returns 0 if success
*/
int EqfMemoryPlugin::removeMemoryFromList(const char* pszName)
{
    if(pszName==NULL)
        return -1;

    int idx = findMemoryIndex(pszName);
    if(idx == -1)
        return -1;
    
    m_MemInfoVector.erase(m_MemInfoVector.begin( )+idx);

    return 0;
}



/*! \brief Handle a return code from the memory functions and create the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name or NULL if not available
    \param pszMemPath fully qualified memory path name or NULL if not available
    \param strLastError reference to string object receiving the message text
    \param iLastError reference to a integer variable receiving the error code
  	\returns original or modified error return code
*/
int EqfMemoryPlugin::handleError( int iRC, char *pszMemName, char *pszMarkup, char *pszMemPath, std::string &strLastError, int &iLastError )
{
  char*          pReplAddr[2];

  if ( iRC == 0 ) return( iRC );

  char *pszErrorTextBuffer = (char *)malloc(8096);
  if ( pszErrorTextBuffer == NULL ) return( iRC );
  pszErrorTextBuffer[0] = '\0';
  
  pReplAddr[0] = pszMemName;

  switch ( iRC )
  {
   case ERROR_TA_ACC_TAGTABLE:
     if (pszMarkup != NULL)
        pReplAddr[0] = pszMarkup;
     else
        strcpy(pReplAddr[0], "n/a");
     UtlGetMsgTxt( ERROR_TA_ACC_TAGTABLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_SHARING_VIOLATION:
   case BTREE_DICT_LOCKED:
   case BTREE_ENTRY_LOCKED:
   case BTREE_ACCESS_ERROR:
     UtlGetMsgTxt( ERROR_MEM_NOT_ACCESSIBLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_NO_MORE_MEMORY_AVAILABLE:
   case ERROR_NOT_ENOUGH_MEMORY:
   case BTREE_NO_ROOM:
   case BTREE_NO_BUFFER:
     UtlGetMsgTxt( ERROR_STORAGE, pszErrorTextBuffer, 0, NULL );
     break;

   case VERSION_MISMATCH:
   case CORRUPT_VERSION_MISMATCH:
   case ERROR_VERSION_NOT_SUPPORTED:
     UtlGetMsgTxt( ERROR_MEM_VERSION_NOT_SUPPORTED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case FILE_MIGHT_BE_CORRUPTED:
   case BTREE_CORRUPTED:
   case BTREE_USERDATA:
   case ERROR_OLD_PROPERTY_FILE:
     UtlGetMsgTxt( ITM_TM_NEEDS_ORGANIZE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case BTREE_DISK_FULL:
   case BTREE_WRITE_ERROR   :
   case DISK_FULL:
     {
       char szDrive[2];
       if ( pszMemPath != NULL )
       {
         szDrive[0] = *pszMemPath;
         szDrive[1] = '\0';
       }
       if (pszMemPath != NULL)
        pReplAddr[0] = szDrive;
       else
        strcpy(pReplAddr[0], "n/a");
       UtlGetMsgTxt( ERROR_DISK_FULL_MSG, pszErrorTextBuffer, 1, pReplAddr );
     }
     break;

   case DB_FULL:
   case BTREE_LOOKUPTABLE_TOO_SMALL:
     UtlGetMsgTxt( ERROR_MEM_DB_FULL, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case FILE_ALREADY_EXISTS:
     UtlGetMsgTxt( ERROR_MEM_NAME_INVALID, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case NOT_REPLACED_OLD_SEGMENT:
     UtlGetMsgTxt( ERROR_MEM_NOT_REPLACED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_FILE_SCREWED_UP:
   case NOT_A_MEMORY_DATABASE:
   case BTREE_ILLEGAL_FILE:
     UtlGetMsgTxt( ERROR_MEM_DESTROYED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_FILE_NOT_FOUND:
   case BTREE_FILE_NOTFOUND:
     UtlGetMsgTxt( ERROR_TM_FILE_NOT_FOUND, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_TM_OPENED_EXCLUSIVELY:
     UtlGetMsgTxt( ERROR_TM_OPENED_EXCLUSIVELY, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_PROP_EXIST:
     UtlGetMsgTxt( ERROR_PROP_EXIST, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_PROP_WRITE_ERROR:
     UtlGetMsgTxt( ERROR_PROP_WRITE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_PROPERTIES_NOT_OPENED:
     UtlGetMsgTxt( ERROR_OPEN_TM_PROPERTIES, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_NETWORK_ACCESS_DENIED :
   case BTREE_NETWORK_ACCESS_DENIED:
     UtlGetMsgTxt( ERROR_ACCESS_DENIED_MSG, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case SEGMENT_BUFFER_FULL:
   case BTREE_BUFFER_SMALL:
     UtlGetMsgTxt( ERROR_MEM_SEGMENT_TOO_LARGE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_INTERNAL :                                      
     UtlGetMsgTxt( ERROR_INTERNAL, pszErrorTextBuffer, 0, NULL );
     break;                                                   

   case ERROR_INTERNAL_PARM:
     UtlGetMsgTxt( ERROR_INTERNAL_PARM, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case  BTREE_IN_USE:
     UtlGetMsgTxt( ERROR_MEM_NOT_ACCESSIBLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_TM_OPENED_SHARED:
   case ERROR_PATH_NOT_FOUND:
   case ERROR_INVALID_DRIVE:
   case BTREE_INVALID_DRIVE :
   default:
     {
       char szError[20];
       sprintf( szError, "%ld", iRC );
       pReplAddr[1] = szError;
       T5LOG(T5ERROR) <<  ":: error " << iRC;
       UtlGetMsgTxt( ERROR_MEM_UNDEFINED, pszErrorTextBuffer, 2, pReplAddr );
     }
 }

 strLastError = pszErrorTextBuffer;
 iLastError = iRC;
 free( pszErrorTextBuffer );

 return( iRC );
}

/* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
int EqfMemoryPlugin::replaceMemory( const char* pszReplace, const char* pszReplaceWith )
{
 std::shared_ptr<OPENEDMEMORY>  pInfoReplace = this->findMemory( pszReplace );
  
  if ( pszReplace == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, (PSZ)pszReplace, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

 std::shared_ptr<OPENEDMEMORY>  pInfoReplaceWith = this->findMemory( pszReplaceWith );
  if ( pszReplaceWith == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, (PSZ)pszReplaceWith, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

  // delete and move data file
  UtlDelete( pInfoReplace->szFullPath, 0L, FALSE );
  UtlMove( pInfoReplaceWith->szFullPath, pInfoReplace->szFullPath, 0L, FALSE );

  char szSource[MAX_LONGFILESPEC];
  char szTarget[MAX_LONGFILESPEC];

  // delete and move index file
  this->makeIndexFileName( pInfoReplaceWith->szFullPath, szSource );
  this->makeIndexFileName( pInfoReplace->szFullPath, szTarget );
  UtlDelete( szTarget, 0L, FALSE );
  UtlMove( szSource, szTarget, 0L, FALSE );

   // delete and move index file
  this->makeIndexFileName( pInfoReplaceWith->szFullPath, szSource, true );
  this->makeIndexFileName( pInfoReplace->szFullPath, szTarget, true );
  UtlDelete( szTarget, 0L, FALSE );
  UtlMove( szSource, szTarget, 0L, FALSE );

  int loglevel = T5Logger::GetInstance()->suppressLogging();
  // delete all remaining files using the normal memory delete
  deleteMemory( pszReplaceWith );
  T5Logger::GetInstance()->desuppressLogging(loglevel);
  return( 0 );
}


unsigned short getPluginInfo( POTMPLUGININFO pPluginInfo )
{
  strcpy( pPluginInfo->szName, pszPluginName );
  strcpy( pPluginInfo->szShortDescription, pszShortDescription );
  strcpy( pPluginInfo->szLongDescription, pszLongDescription );
  strcpy( pPluginInfo->szVersion, pszVersion );
  strcpy( pPluginInfo->szSupplier, pszSupplier );
  pPluginInfo->eType = OtmPlugin::eTranslationMemoryType;
  strcpy( pPluginInfo->szDependencies, "" );
  pPluginInfo->iMinOpenTM2Version = -1;
  return( 0 );
}
