/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "PluginManager.h"
#include "Property.h"
#include "tm.h"
#include "EQFSETUP.H"
#include "FilesystemHelper.h"
#include "LogWrapper.h"
#include "EQF.H"

#include "string"
#include "vector"
#include <fstream>
#include "requestdata.h"
#include "FilesystemWrapper.h"

// some constant values
static char *pszPluginName = "EqfMemoryPlugin";
static char *pszShortDescription = "TranslationMemoryPlugin";
static char *pszLongDescription	= "This is the standard Translation-Memory implementation";
static char *pszVersion = "1.0";
static char *pszSupplier = "International Business Machines Corporation";
//EqfMemoryPlugin* EqfMemoryPlugin::_instance = NULL;

EqfMemoryPlugin::EqfMemoryPlugin()
{
    pluginType = eTranslationMemoryType;
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


/**********************************************************************/
/**********************************************************************/
/* Internal functions                                                 */
/**********************************************************************/
/**********************************************************************/
//------------------------------------------------------------------------------
// Function name:     NTMFillCreateInStruct
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    NTMFillCreateInStruct( HTM             hModel,
//                                           PSZ             pszPathMem,
//                                           PSZ             pszServer,
//                                           PSZ             pszUserID,
//                                           PSZ             pszSourceLang,
//                                           PSZ             pszDescription,
//                                           PTMX_CREATE_IN  pstCreateIn,
//                                           USHORT          usMsgHandling )
//                                           HWND            hwnd )
//------------------------------------------------------------------------------
// Description:       Fills the TMX_CREATE_IN structure from the passed
//                    parameters or from a model handle.
//------------------------------------------------------------------------------
// Parameters:
//  hModel         - (in)  model handle
//  pszPathMem     - (in)  full TM name x:\eqf\mem\mem.tmd
//  pszServer      - (in)  model handle
//  pszUserID      - (in)  server name or empty str (when no model handle)
//  pszSourceLang  - (in)  LAN USERID or empty str  (when no model handle)
//  pszDescription - (in)  source language or empty str (when no model handle)
//  pstCreateIn    - (in)  TM description or empty str (when no model handle)
//  usMsgHandling  - (in)  message handling parameter
//                         TRUE:  display error message
//                         FALSE: display no error message
//------------------------------------------------------------------------------
// Prerequesits:      htm must be a valid TM handle returned from TmOpen
//                    The TM referenced by htm must be open.
//------------------------------------------------------------------------------
USHORT
NTMFillCreateInStruct( const char*     pszPathMem,
                       const char*     pszSourceLang,
                       const char*     pszDescription,
                       PTMX_CREATE_IN  pstCreateIn)
{
  USHORT        usRc=NO_ERROR;
  
  /******************************************************************/
  /* no model handle get data for TMX_CREATE_IN stucture from input */
  /******************************************************************/
  /******************************************************************/
  /* fill the TMX_CREATE_IN structure                               */
  /* szDataName         - TM path and name X:\EQFD\MEM\MEM.TMD      */
  /* szIndexName        - TM path and name X:\EQFD\MEM\MEM.TMI      */
  /* szServer           - server name                               */
  /* szUserID           - LAN userid                                */
  /* szSourceLanguage   - source language name                      */
  /* szDescription      - description text                          */
  /* usThreshold        - threshold for return of fuzzy matches     */
  /* bLangTable         - language table                            */
  /******************************************************************/
  strcpy( pstCreateIn->stTmCreate.szDataName,       pszPathMem      );

  /**************************************************************/
  /* check if the extension of the passed TM name is a extension*/
  /* for a temporary TM used  by TM organize                    */
  /**************************************************************/
  if ( !strcmp( strrchr( pszPathMem, '.'), EXT_OF_TMDATA ) )
  {
    /************************************************************/
    /* TM is no temporaray TM, use "normal" index name          */
    /************************************************************/
    CopyFilePathReplaceExt(pstCreateIn->stTmCreate.szIndexName, pszPathMem, EXT_OF_TMINDEX);
  }
  else
  {
    /************************************************************/
    /* TM is a temporary TM, use temporary index name           */
    /************************************************************/
    CopyFilePathReplaceExt(pstCreateIn->stTmCreate.szIndexName, pszPathMem, EXT_OF_TEMP_TMINDEX);
  } /* endif */

  strcpy( pstCreateIn->stTmCreate.szSourceLanguage, pszSourceLang   );
  strcpy( pstCreateIn->stTmCreate.szDescription,    pszDescription  );
  pstCreateIn->stTmCreate.usThreshold = TM_DEFAULT_THRESHOLD;
  
  return usRc;
} /* end of function NTMFillCreateInStruct */






/*! \brief Create a new translation memory
  \param pszName name of the new memory
	\param pszSourceLang source language
	\param pszDescription description of the memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
EqfMemory* EqfMemoryPlugin::createMemory(
	const char* pszName,			  
	const char* pszSourceLang,
	const char* pszDescription
)
{
  EqfMemory *pNewMemory = NULL;        // new memory object
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;
  BOOL fReserved = FALSE;
  USHORT usRc = 0;
  // build memory path and reserve a short name
  this->makeMemoryPath( pszName, strMemPath, &fReserved );
  
  TMX_CREATE_IN   CreateIn;    //pointer to create input structure
  TMX_CREATE_OUT  CreateOut;   //pointer to create output structure
  // use old memory create code
  usRc = NTMFillCreateInStruct( strMemPath.c_str(), //call function to fill TMC_CREATE_IN structure
                                pszSourceLang,
                                pszDescription,
                                &CreateIn );
  usRc = TmtXCreate( &CreateIn, &CreateOut ); //call U code to pass TM command to server or handle it local   
                              
  htm = (HTM)CreateOut.pstTmClb;//no error,  return pointer to TM CLB as handle  

  // setup memory properties
  this->createMemoryProperties( pszName, strMemPath, pszDescription, pszSourceLang );
  
  // create memory object if create function completed successfully
  pNewMemory = new EqfMemory( htm, (PSZ)pszName );


  // add memory info to our internal memory list
  if ( pNewMemory != NULL ) this->addToList( strMemPath );
  
  // cleanup
  if ( (pNewMemory == NULL) && fReserved )
  {
    std::string strPropName ;

    this->makePropName( strMemPath, strPropName );
    UtlDelete( (char *)strPropName.c_str(), 0, FALSE );
  } /* endif */

  return( (EqfMemory *)pNewMemory );
}

EqfMemory* EqfMemoryPlugin::openMemoryNew(
  const std::string& memName			 
  //,unsigned short usAccessMode
){
  USHORT usRC = 0;
  // use old memory open code
  //std::string path = FilesystemHelper::GetMemDir() + memName + EXT_OF_TMDATA;
  EqfMemory* pMemory = new EqfMemory(memName);
  if(pMemory){
    usRC = pMemory->OpenX();
  }  
  // create memory object if create function completed successfully
  if ( (usRC != 0) && (usRC != BTREE_CORRUPTED) /*&& (usAccessMode == FOR_ORGANIZE)*/){
    T5LOG(T5ERROR) << "EqfMemoryPlugin::openMemory:: TmOpen fails, fName = "<< memName<< "; error = "<< this->strLastError<<"; iLastError = "<< 
        this->iLastError;
    //if ( pMemory != 0 ) 
    //  TmClose( pMemory, NULL,  FALSE,  NULL );
  } /* end */       
  return pMemory;
} /* end */     

/*! \brief Open an existing translation memory
  \param pszName name of the existing memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
EqfMemory* EqfMemoryPlugin::openMemory(
	const char* pszName,			 
	BOOL bMsgHandling,
	HWND hwnd,
  unsigned short usAccessMode
)
{
  EqfMemory *pMemory = NULL;
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;

  // find memory in our list
 std::shared_ptr<EqfMemory>  pInfo = this->findMemory( (PSZ)pszName );
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
      pMemory = new EqfMemory( htm, (PSZ)pszName );
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
	EqfMemory *pMemory			 
)
{
  if ( pMemory == NULL ) return( -1 );

  EqfMemory *pMem = (EqfMemory *)pMemory;
  HTM htm = pMem->getHTM();
  
	int iRC = TmClose( htm, NULL,  FALSE,  NULL );

  // refresh memory info
  std::string strMemName;
  pMem->getName( strMemName );
 std::shared_ptr<EqfMemory>  pMemInfo = this->findMemory( (char *)strMemName.c_str() );
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

  //delete( pMemory );  

  return( iRC );
}

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
int EqfMemoryPlugin::getMemoryInfo(
	const char* pszName,
 std::shared_ptr<EqfMemory>  pInfo
)
{
  int iRC = EqfMemoryPlugin::eSuccess;
 std::shared_ptr<EqfMemory>  pMemInfo = this->findMemory( (PSZ)pszName );
  if ( pMemInfo.get() != NULL )
  {
    memcpy( pInfo.get(), pMemInfo.get(), sizeof(EqfMemory) );
  }
  else
  {
    iRC = EqfMemoryPlugin::eMemoryNotFound;
    memset( pInfo.get(), 0, sizeof(EqfMemory) );
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

   std::shared_ptr<EqfMemory>  pMemInfo = this->findMemory( (PSZ)pszName );
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
    Properties::GetInstance()->get_value(KEY_MEM_DIR, szPathMem, 512);
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
  int iRC = EqfMemoryPlugin::eSuccess;
  *pFileListBuffer = '\0';

 std::shared_ptr<EqfMemory>  pMemInfo = this->findMemory( (PSZ)pszName );

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
      iRC = EqfMemoryPlugin::eBufferTooSmall;
    } /* endif */
  }
  else
  {
    iRC = EqfMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
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
  int iRC = EqfMemoryPlugin::eSuccess;
 std::shared_ptr<EqfMemory>  pMemInfo = this->findMemory( (PSZ)pszOldName );
  if ( pMemInfo.get() != NULL )
  {
    // get new short name for memory
    BOOL fIsNew = FALSE;
    char szShortName[MAX_FILESPEC];
    
    T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 16 ObjLongToShortName( pszNewName, szShortName, TM_OBJECT, &fIsNew )";
#ifdef TEMPORARY_COMMENTED
    ObjLongToShortName( pszNewName, szShortName, TM_OBJECT, &fIsNew );
    #endif

    if ( !fIsNew ) return( EqfMemory::ERROR_MEMORYEXISTS );

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
    iRC = EqfMemoryPlugin::eMemoryNotFound;
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
  int iRC = EqfMemoryPlugin::eSuccess;
 std::shared_ptr<EqfMemory>  pMemInfo;

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
    T5LOG(T5ERROR) << "EqfMemoryPlugin::deleteMemory:: EqfMemoryPlugin::eMemoryNotFound name = "<< pszName ;
    iRC = EqfMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief close and delete a temporary memory
  \param pMemory pointer to memory objject
*/
void EqfMemoryPlugin::closeTempMemory(
	EqfMemory *pMemory
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
   std::shared_ptr<EqfMemory>  pInfo
)
{
  if(pInfo==0 || pszPropName==0){
    T5LOG(T5ERROR) <<  "EqfMemoryPlugin::fillInfoStructure():: pInfo==0 || pszPropName==0";
    return false;
  }

  PROP_NTM prop;

  memset( pInfo.get(), 0, sizeof(EqfMemory) );

  // init it, if not meet some condition ,it will be set to false
  pInfo->fEnabled = TRUE;

  std::string mem_path;
  int errCode = 0;
  {
    char path[MAX_EQF_PATH];
    errCode = Properties::GetInstance()->get_value(KEY_MEM_DIR, path, MAX_EQF_PATH);
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
	\returns std::shared_ptr<EqfMemory>  pInfo pointer to memory info or NULL in case of errors
*/
std::shared_ptr<EqfMemory>  EqfMemoryPlugin::findMemory
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
        EqfMemory *  pInfo = (m_MemInfoVector[i]).get();
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
      Properties::GetInstance()->get_value(KEY_MEM_DIR, buff, 255);
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

    pProp->stTMSignature.bGlobVersion = T5GLOBVERSION;
    pProp->stTMSignature.bMajorVersion = T5MAJVERSION;
    pProp->stTMSignature.bMinorVersion = T5MINVERSION;
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

/*! \brief Make the fully qualified property file name for a memory
  \param strPathName reference to the memory path name
  \param strPropName reference to the string receiving the property file name
	\returns 0 when successful
*/
int EqfMemoryPlugin::makePropName( std::string &strPathName, std::string &strPropName )
{
  char szFullPropName[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szFullPropName, NULC, PROPERTY_PATH, NULL );
  Properties::GetInstance()->get_value(KEY_MEM_DIR, szFullPropName, MAX_LONGFILESPEC-1);
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
 std::shared_ptr<EqfMemory> pInfo =  std::make_shared<EqfMemory>( EqfMemory() );
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
 std::shared_ptr<EqfMemory>  pInfoReplace = this->findMemory( pszReplace );
  
  if ( pszReplace == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, (PSZ)pszReplace, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

 std::shared_ptr<EqfMemory>  pInfoReplaceWith = this->findMemory( pszReplaceWith );
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
