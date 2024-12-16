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

/**********************************************************************/
/**********************************************************************/
/* Internal functions                                                 */
/**********************************************************************/
/**********************************************************************/


EqfMemory* EqfMemoryPlugin::initTM(
  const std::string& memName,	
  size_t requiredMemory,	 
  unsigned short usAccessMode, 
  bool fReorganizeOnly
){
  USHORT usRC = 0;
  // use old memory open code
  //std::string path = FilesystemHelper::GetMemDir() + memName + EXT_OF_TMDATA;
  EqfMemory* pMemory = new EqfMemory(memName);
  if(pMemory){
  //  usRC = pMemory->Load();
    pMemory->setExpecedSizeInRAM(requiredMemory);
    pMemory->updateLastUseTime();
    pMemory->eStatus = WAITING_FOR_LOADING_STATUS;
    pMemory->usAccessMode |= ASD_ORGANIZE;
    //pMemory->fReorganizeOnly = fReorganizeOnly;
  }else{
    LOG_AND_SET_RC(usRC, T5WARNING, ERROR_NOT_ENOUGH_MEMORY);
  }  
  return pMemory;
} /* end */     


/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
int EqfMemoryPlugin::closeMemory(
	EqfMemory *pMemory			 
)
{
  T5LOG(T5ERROR) <<  "unimplemented funtion";
  return( 0 );
}


///*! \brief set description of a memory
//  \param pszName name of the memory
//  \param pszDesc description information
//	\returns 0 if successful or error return code
//*/
int EqfMemoryPlugin::setDescription(const char* pszName, const char* pszDesc)
{
  return 0;
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
