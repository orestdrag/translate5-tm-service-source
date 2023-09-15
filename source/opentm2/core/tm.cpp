
#include <chrono>
#include <thread>
#include "tm.h"
#include "LogWrapper.h"
#include "Property.h"
#include "EQFMSG.H"
#include "EQF.H"
#include "FilesystemHelper.h"
#include "tm.h"



/*! \brief Checks if there is opened memory in import process
  \returns index if import process for any memory is going on, -1 if no
  */
int TMManager::GetMemImportInProcess(){
  T5LOG(T5FATAL) << "fix this function after refactoring!";
  for( int i = 0; i < (int)EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
  {
    if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i]->eImportStatus == IMPORT_RUNNING_STATUS)
    {
      T5LOG( T5INFO) << ":: memory in import process, name = " << EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i]->szName ;
      return i;
    }
  } /* endfor */
  return -1;
}

size_t TMManager::CalculateOccupiedRAM(){
  char memFolder[260];
  size_t UsedMemory = 0;
  #ifdef CALCULATE_ONLY_MEM_FILES
  Properties::GetInstance()->get_value(KEY_MEM_DIR, memFolder, 260);
  std::string path;
  for(int i = 0; i < EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size() ;i++){
    if(EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName != 0){
      path = memFolder;
      path += EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i].szName;
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMI"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".TMD"));
      UsedMemory += FilesystemHelper::GetFilebufferSize( std::string(path + ".MEM"));
    }
  }
  #else
  for(const auto& mem: tms){
    UsedMemory += mem.second->GetRAMSize();
  }
  //UsedMemory += FilesystemHelper::GetTotalFilebuffersSize();
  UsedMemory += MEMORY_RESERVED_FOR_SERVICE;
  #endif

  T5LOG( T5INFO) << ":: calculated occupied ram = " << UsedMemory/1000000 << " MB";
  return UsedMemory;
}

/*! \brief close any memories which haven't been used for a long time
  \returns 0
*/
size_t TMManager::CleanupMemoryList(size_t memoryRequested)
{  
  int AllowedMBMemory = 500;
  Properties::GetInstance()->get_value(KEY_ALLOWED_RAM, AllowedMBMemory);
  size_t AllowedMemory = AllowedMBMemory * 1000000;    
  size_t memoryNeed = memoryRequested + CalculateOccupiedRAM();
  T5LOG(T5DEBUG) << "CleanupMemoryList was called, memory need = "<< memoryNeed <<"; Memory requested = " << memoryRequested<<"; memoryAllowed = " << AllowedMemory;
  
  if(memoryNeed < AllowedMemory){
    return( AllowedMemory - memoryNeed );
  }else{
    T5LOG(T5DEBUG) << "CleanupMemoryList was called, memory more than available, mem need = "<< memoryNeed <<"; Memory requested = " << memoryRequested<<"; memoryAllowed = " << AllowedMemory;
  }

  time_t curTime;
  time( &curTime );
  std::multimap <time_t, std::weak_ptr<EqfMemory>>  openedMemoriesSortedByLastAccess;
  for(const auto& tm: tms){
    openedMemoriesSortedByLastAccess.insert({tm.second->tLastAccessTime, tm.second});
  }

  T5LOG(T5DEBUG) << "there are  "<< openedMemoriesSortedByLastAccess.size()<<" open in RAM";

  for(auto it = openedMemoriesSortedByLastAccess.begin(); memoryNeed >= AllowedMemory && it != openedMemoriesSortedByLastAccess.end(); it++){
    std::string tmName;
    {
      auto tm = it->second.lock();
      tm->UnloadFromRAM();
      tmName = tm->szName;
      tms.erase(tmName);
      T5LOG(T5DEBUG) <<"erasing mem with name " <<tm->szName <<"; use count = " << tm.use_count();
    }
    for(int i = 0; i<5; i++){
      if(it->second.expired()){
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if(it->second.expired() == false){
      T5LOG(T5INFO) << "TM with name \'" << tmName << " was stil not unloaded from RAM after 500 miliseconds after deleting it from TMManager!";
    }

    memoryNeed = memoryRequested + CalculateOccupiedRAM();
  }

  return AllowedMemory > memoryNeed ? AllowedMemory - memoryNeed : 0;
}


// update memory status
void EqfMemory::importDone(int iRC, char *pszError )
{
  eStatus = OPEN_STATUS;
  if ( iRC == 0 )
  {
    eImportStatus = OPEN_STATUS;
    TmBtree.fb.Flush();
    InBtree.fb.Flush();
    T5LOG( T5INFO) <<"OtmMemoryServiceWorker::importDone:: success, memName = " << szName;
  }
  else
  {
    eImportStatus = IMPORT_FAILED_STATUS; 
    strError =  pszError;
    T5LOG(T5ERROR) << "OtmMemoryServiceWorker::importDone:: memName = " << szName <<", import failed: " << strError << " import details = " << importDetails->toString() ;
  }
}


// update memory status
void EqfMemory::reorganizeDone(int iRC, char *pszError )
{
  eStatus = OPEN_STATUS;
  if ( iRC == 0 )
  {
    eImportStatus = AVAILABLE_STATUS;
    TmBtree.fb.Flush();
    InBtree.fb.Flush();
    T5LOG( T5TRANSACTION) <<":: success, memName = " << szName;
  }
  else
  {
    eImportStatus = REORGANIZE_FAILED_STATUS;
    strError =  pszError;
    T5LOG(T5ERROR) << ":: memName = " << szName <<", reorganize failed: " << pszError << " import details = " << importDetails->toString() ;
  }
}

/*! \brief Close all open memories
\returns http return code0 if successful or an error code in case of failures
*/
int TMManager::closeAll()
{
  for ( int i = 0; i < ( int )EqfMemoryPlugin::GetInstance()->m_MemInfoVector.size(); i++ )
  {
    if ( EqfMemoryPlugin::GetInstance()->m_MemInfoVector[i]->szName[0] != 0 )
    {
      //removeFromMemoryList( i );
    }
  } /* endfor */
  return( 0 );
}



/*! \file

 \brief Memory factory class wrapper 

 This class provides static methods for the creation of EqfMemory objects and
 access to standard memory functions

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_MORPH            // morphologic functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions



//#define LOGGING

#include "../pluginmanager/PluginManager.h"
#include "tm.h"
#include "../utilities/OSWrapper.h"
#include "tm.h"
//#include "OptionsDialog.H"
#include "LogWrapper.h"
#include "FilesystemHelper.h"
#include "Property.h"
#include "LanguageFactory.H"

#include "vector"
#include <string>
#include <climits>

// default memory plugins
#define DEFAULTMEMORYPLUGIN "EqfMemoryPlugin"

//P403634
#define DEFAULTSHAREDMEMORYPLUGIN "EqfSharedMemoryPlugin"
//#define DEFAULTSHAREDMEMORYPLUGIN "EqfSharedMemPlugin"


/*! \brief This static method returns a pointer to the TMManager object.
	The first call of the method creates the TMManager instance.
*/

TMManager* TMManager::GetInstance(){
  static TMManager tmm;
  return &tmm;
}

std::shared_ptr<EqfMemory> TMManager::CreateNewEmptyTM(const std::string& strMemName, const std::string& strSrcLang, const std::string& strMemDescription, int& _rc_){
  std::shared_ptr<EqfMemory> NewMem;
  ULONG ulKey = 0;
  if ( _rc_ == NO_ERROR )
  {
    T5LOG( T5DEBUG) << ":: create the memory" ;    
    NewMem = std::make_shared<EqfMemory> (strMemName);
    if ( NewMem == nullptr ){
      _rc_ = ERROR_NOT_ENOUGH_MEMORY;
    }
  }
  
  if(_rc_ == NO_ERROR)
  {
    NewMem->stTmSign.bGlobVersion = T5GLOBVERSION;
    NewMem->stTmSign.bMajorVersion = T5MAJVERSION;
    NewMem->stTmSign.bMinorVersion = T5MINVERSION;
    //build name and extension of tm data file

    //fill signature record structure
    strcpy( NewMem->stTmSign.szName, NewMem->TmBtree.fb.fileName.c_str() );
    UtlTime( &(NewMem->stTmSign.lTime) );
    UtlTime( &(NewMem->stTmSign.creationTime) );
    strcpy( NewMem->stTmSign.szSourceLanguage,
            strSrcLang.c_str() );

    strcpy( NewMem->stTmSign.szDescription,
            strMemDescription.c_str() );

    //call create function for data file
    NewMem->usAccessMode = 1;//ASD_LOCKED;         // new TMs are always in exclusive access...
    
    _rc_ = NewMem->TmBtree.QDAMDictCreateLocal( &(NewMem->stTmSign),FIRST_KEY );
  }
  if ( _rc_ == NO_ERROR )
  {
    //insert initialized record to tm data file
    ulKey = AUTHOR_KEY;
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
              (PBYTE)&NewMem->Authors, TMX_TABLE_SIZE );
  }
  if ( _rc_ == NO_ERROR )
  {
    ulKey = FILE_KEY;
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
                (PBYTE)&NewMem->FileNames, TMX_TABLE_SIZE );     
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    ulKey = TAGTABLE_KEY;
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
                (PBYTE)&NewMem->TagTables, TMX_TABLE_SIZE );
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    ulKey = LANG_KEY;
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
            (PBYTE)&NewMem->Languages, TMX_TABLE_SIZE );
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    int size = MAX_COMPACT_SIZE-1 ;
    //initialize and insert compact area record
    memset( NewMem->bCompact, 0, size );

    ulKey = COMPACT_KEY;
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
                        NewMem->bCompact, size);  

  } /* endif */

  // add long document table record
  if ( _rc_ == NO_ERROR )
  {
    ulKey = LONGNAME_KEY;
    // write long document name buffer area to the database
    _rc_ = NewMem->TmBtree.EQFNTMInsert(&ulKey,
                        (PBYTE)NewMem->pLongNames->pszBuffer,
                        NewMem->pLongNames->ulBufUsed );        

  } /* endif */

  // create language group table
  if ( _rc_ == NO_ERROR )
  {
    _rc_ =  NewMem->NTMCreateLangGroupTable();    
  } /* endif */

  if ( _rc_ == NO_ERROR )
  {
    //fill signature record structure
    strcpy( NewMem->stTmSign.szName, NewMem->InBtree.fb.fileName.c_str() );
    _rc_ = NewMem->InBtree.QDAMDictCreateLocal(&NewMem->stTmSign, START_KEY );                        
  } /* endif */

  if(_rc_ == NO_ERROR){   
    NewMem->TmBtree.fb.Flush();
    NewMem->InBtree.fb.Flush();     
  }else {
    //something went wrong during create or insert so delete data file
    UtlDelete( (PSZ)NewMem->InBtree.fb.fileName.c_str(), 0L, FALSE );
    UtlDelete((PSZ) NewMem->TmBtree.fb.fileName.c_str(), 0L, FALSE);
  } /* endif */    
  return NewMem;
}


/* \brief Closed a previously opened memory
   \param pMemory pointer to memory object beign closed
	 \returns 0 when successful or error code
*/
int TMManager::closeMemory
(
  EqfMemory *pMemory
)
{
  int iRC = 0;

  if ( pMemory == NULL  ){
    T5LOG(T5ERROR) <<"TMManager::closeMemory, pMemory is NULL";
    return( -1 );
  }
  
  // build memory object name
  char* pszObjName = NULL;
  UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
  strcpy( pszObjName, EqfMemoryPlugin::GetInstance()->getName() );
  strcat( pszObjName, ":" );
  pMemory->getName( pszObjName + strlen(pszObjName), MAX_LONGFILESPEC );

  // close the memory
  iRC = EqfMemoryPlugin::GetInstance()->closeMemory( pMemory );

  // send a properties changed msg to memory handler
  EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pszObjName ));

  if ( pszObjName != NULL ) {
      UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
  }
 
  return( iRC );
}



/*! \brief Physically delete a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
  \param strError return error message with it
	returns 0 if successful or error return code
*/
int TMManager::DeleteTM(
  const std::string& memoryName,
  std::string &strError
)
{
  int _rc_ = 0;
  //close if open
  //if files exists
   if (_rc_ = TMManager::GetInstance()->TMExistsOnDisk(memoryName) )
  {
    strError = "{\"" + memoryName + "\": \"not found(error " + std::to_string(_rc_) + ")\" }";
    return( _rc_ = NOT_FOUND );
  }

  //_rc_ = 
  //TMManager::GetInstance()->CloseTM(memoryName);

  // delete the memory
  if( !_rc_){
    if(FilesystemHelper::DeleteFile(FilesystemHelper::GetTmdPath(memoryName))){
      _rc_ |= 16;
    }

    //check tmi file
    if(FilesystemHelper::DeleteFile(FilesystemHelper::GetTmiPath(memoryName))){
      _rc_ |= 8;
    }    
  }
  return _rc_;
}


int TMManager::RenameTM(
  const std::string& oldMemoryName,
  const std::string& newMemoryName,
  std::string &strError
)
{
  int _rc_ = 0;
  //close if open
  //if files exists
  if (_rc_ = TMManager::GetInstance()->TMExistsOnDisk(oldMemoryName) )
  {
    strError = "{\"" + oldMemoryName + "\": \"not found(error " + std::to_string(_rc_) + ")\" }";
    return( _rc_ = NOT_FOUND );
  }

  if ( (_rc_ = TMManager::GetInstance()->TMExistsOnDisk(newMemoryName)) == 0 )
  {
    strError = "{\"" + newMemoryName + "\": \"already exists (error " + std::to_string(_rc_) + ")\" }";
    return( _rc_ = NOT_FOUND );
  }else{
    _rc_ = 0;
  }


  TMManager::GetInstance()->CloseTM(oldMemoryName);

  // move/rename the memory
  if( !_rc_){
    if(FilesystemHelper::MoveFile(FilesystemHelper::GetTmdPath(oldMemoryName), FilesystemHelper::GetTmdPath(newMemoryName))){
      _rc_ |= 16;
    }

    //check tmi file
    if(FilesystemHelper::MoveFile(FilesystemHelper::GetTmiPath(oldMemoryName),  FilesystemHelper::GetTmiPath(newMemoryName))){
      _rc_ |= 8;
    }    
  }
  return _rc_;
}

/*! \brief Show error message for the last error
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory causing the problem
  memory object name (pluginname + colon + memoryname)
  \param pMemory pointer to existing memory object or NULL if not available
  \param hwndErrMsg handle of parent window message box
*/
void TMManager::showLastError(
  char *pszPluginName,
  char *pszMemoryName,
  EqfMemory *pMemory,
  HWND hwndErrMsg
)
{
  pszPluginName; pszMemoryName;

  if ( pMemory != NULL )
  {
    // retrieve last error from memory plugin
    this->iLastError = pMemory->getLastError( this->strLastError );
  } /* endif */     

 // show error message
  char* pszParm = (char*)this->strLastError.c_str();
  T5LOG(T5ERROR) <<"ERROR in TMManager::showLastError:: " << pszParm <<  "; iLastError = " << this->iLastError;
}

std::string& TMManager::getLastError( EqfMemory *pMemory, int& iLastError, std::string& strError)
{
    if ( pMemory != NULL )
        this->iLastError = pMemory->getLastError( this->strLastError );

    iLastError = this->iLastError;
    strError = this->strLastError;
    return strError;
}

/*! \brief Get the sort order key for a memory match

  \param Proposal reference to a proposal for which the sort key is evaluated
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal &Proposal )
{
  return( getProposalSortKey( Proposal, -1, 0, FALSE ) ); 
}


/*! \brief Get the sort order key for a memory match

 the current order is: Exact-Exact -> Exact -> Global memory(Exact) -> Replace -> Fuzzy -> Machine
 We use the folloging sort key values
 - Exact-ContextSameDoc       = 1400
 - Exact-Context              = 1300
 - Exact-Exact                = 1200
 - Exact-Context(Context<100) =  900-999
 - Exact-SameDoc              =  801 (800 when flag LATESTPROPOSAL_FIRST has been specified)
 - Exact                      =  800
 - Global memory (Exact)      =  700
 - Global memory Star(Exact)  =  600
 - Fuzzy                      =  301-399
 - Machine                    =  200 (400 at the end of the table)
 - empty entry                =  0

  Special handling for MT display factor (if given):
  - 0%  Machine =  200
  - 100%  Machine =  400
  - 1%-99% Machine =  3xx (xx = MT display factor)

  Special mode at the end of the match table:

  When the end of the table is reached MT proposals get a key higher than fuzzy ones to keep at least one MT match in the table 
  
  \param Proposal reference to a proposal for which the sort key is evaluated
  \param iMTDisplayFactor the machine translation display factor, -1 to ignore the factor
  \param usContextRanking the context ranking for the proposal
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  \param lOptions options for the sort key generation
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal &Proposal, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions )
{
  // when no context value is given, use context ranking of proposal
  if ( usContextRanking == 0 )
  {
    usContextRanking = (USHORT)Proposal.getContextRanking();
  } /* endif */
  return( getProposalSortKey( Proposal.getMatchType(), Proposal.getType(), Proposal.getFuzziness(), iMTDisplayFactor, usContextRanking, fEndOfTable, lOptions  ) ); 
}

 /*! \brief Get the sort order key for a memory match
  \param MatchType match type of the match
  \param ProposalType type of the proposal
  \param iFuzziness fuzziness of the proposal
  \param iMTDisplayFactor the machine translation display factor, -1 to use the system MT display factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  \param lOptions options for the sort key generation
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int TMManager::getProposalSortKey(  OtmProposal::eMatchType MatchType, OtmProposal::eProposalType ProposalType, 
    int iFuzziness, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable, LONG lOptions  )
{

  // dummy (=empty) entries
  if ( (MatchType == OtmProposal::emtUndefined) || (iFuzziness == 0) )
  {
    return( 0 );  
  } /* endif */  

  // use system MT display factor if none has been specified
  if ( iMTDisplayFactor == -1 )
  {
    iMTDisplayFactor = UtlQueryULong( QL_MTDISPLAYFACTOR );
  }

  // normal matches
  if ( ProposalType == OtmProposal::eptManual )
  {
    // exact matches with correct context have the highest priority
    if ( (usContextRanking == 100) && 
         ( (MatchType == OtmProposal::emtExact) || 
           (MatchType == OtmProposal::emtExactSameDoc) ||
           (MatchType == OtmProposal::emtExactExact) ) )
    {
      if ( (MatchType == OtmProposal::emtExactSameDoc) ||  (MatchType == OtmProposal::emtExactExact) )
      {
        return( 1400 ); // context match from same document
      }
      else
      {
        return( 1300 ); // context match from a different document
      }
    }
    else if ( (usContextRanking > 0) && (usContextRanking < 100) )
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactSameDoc )
      {
        return( 900 + usContextRanking  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 900 + usContextRanking -1  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    }
    else
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactSameDoc )
      {
        if ( lOptions & LATESTPROPOSAL_FIRST )
        {
          // treat proposal from same document as normal exact proposal to ensure that the exact proposals are sorted on their timestamp correctly
          return( 800 );  
        }
        else
        {
          return( 801 );  
        } /* endif */
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 800 );  
      } /* endif */  
      // fuzzy and replace matches
      if ( MatchType == OtmProposal::emtReplace )
      {
        return( 400 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    } /* endif */       
  } /* endif */  

  // machine matches
  if ( ProposalType == OtmProposal::eptMachine )
  {
    if ( (iMTDisplayFactor <= 0) || (iMTDisplayFactor > 100) ) // invalid (>100)  / not specified (-1) or 0
    {
      if ( fEndOfTable )
      {
        return( 400 );  
      }
      else
      {
        return( 200 );  
      } /* endif */         
    } 
    else if ( iMTDisplayFactor == 100 )
    {
      return( 400 );  
    }
    else 
    {
      return( 300 + iMTDisplayFactor );  
    } /* endif */       
  } /* endif */  

  // global memory matches
  if ( ProposalType == OtmProposal::eptGlobalMemory )
  {
    if ( iFuzziness >= 100 )
    {
      return( 700 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // global memory star matches
  if ( ProposalType == OtmProposal::eptGlobalMemoryStar )
  {
    if ( iFuzziness >= 100 )
    {
      return( 600 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // fuzzy and replace matches
  if ( ProposalType == OtmProposal::emtReplace )
  {
    return( 400 );  
  } /* endif */  
  if ( ProposalType == OtmProposal::emtFuzzy )
  {
    return( 300 + iFuzziness );  
  } /* endif */  

  // anything else...
  return( 0 );
} /* end of method TMManager::getProposalSortKey */


/*! \brief Get the object name for the memory
  \param pMemory pointer to the memory object
  \param pszObjName pointer to a buffer for the object name
  \param iBufSize size of the object name buffer
  \returns 0 when successful or the error code
*/
int TMManager::getObjectName( EqfMemory *pMemory, char *pszObjName, int iBufSize )
{
  EqfMemoryPlugin *pPlugin;

  if ( pMemory == NULL )
  {
      this->iLastError = ERROR_MEMORYOBJECTISNULL;
      this->strLastError = "Internal error, supplied memory object is null";
      return( this->iLastError );
  } /* endif */     

  pPlugin = (EqfMemoryPlugin *)pMemory->getPlugin();
  if ( pPlugin == NULL )
  {
      this->iLastError = ERROR_PLUGINNOTAVAILABLE;
      this->strLastError = "Internal error: No plugin found for memory object";
      return( this->iLastError );
  } /* endif */     

  const char *pszPluginName = pPlugin->getName();
  std::string strMemName;
  pMemory->getName( strMemName );
  if ( ((int)strlen( pszPluginName ) + (int)strMemName.length() + 2) > iBufSize)
  {
    return( ERROR_BUFFERTOOSMALL );
  } /* endif */     

  strcpy( pszObjName, pszPluginName );
  strcat( pszObjName, ":" );
  strcat( pszObjName, strMemName.c_str() );

  return( 0 );
}

/*! \brief Get the plugin name and the memory name from a memory object name
  \param pszObjName pointer to the memory object name
  \param pszPluginName pointer to the buffer for the plugin name or 
    NULL if no plugin name is requested
  \param iPluginBufSize size of the buffer for the plugin name
  \param pszluginName pointer to the buffer for the plugin name or
    NULL if no memory name is requested
  \param iNameBufSize size of the buffer for the memory name
  \returns 0 when successful or the error code
*/
int TMManager::splitObjName( char *pszObjName, char *pszPluginName, int iPluginBufSize, char *pszMemoryName, int iNameBufSize  )
{
  T5LOG(T5FATAL) << "commented funtion";
  return( 0 );
}

// helper functions 

/* \brief Get memory plugin with the given pkugin name
   \param pszPlugin plugin-name
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *TMManager::getPlugin
(
  const char *pszPluginName
)
{
  // find plugin using plugin name
  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    for ( std::size_t i = 0; i < pluginList->size(); i++ )
    {
      OtmPlugin *pluginCurrent = (OtmPlugin *)(*pluginList)[i];
      if ( strcmp( pluginCurrent->getName(), pszPluginName ) == 0 )
      {
        return ( pluginCurrent );
      } /* endif */         
    } /* endfor */       
  } /* endif */       

  return( NULL );
}

void TMManager::refreshPluginList() 
{
  if ( this->pluginList != NULL ) delete( this->pluginList );

  this->szDefaultMemoryPlugin[0] = '\0';
  
  // access plugin manager
  PluginManager* thePluginManager = PluginManager::getInstance();

  // iterate through all memory plugins and store found plugins in pluginList
  EqfMemoryPlugin * curPlugin = NULL;
  pluginList = new std::vector<EqfMemoryPlugin *>;
  int i = 0;
  do
  {
    i++;
    curPlugin = (EqfMemoryPlugin*) thePluginManager->getPlugin(OtmPlugin::eTranslationMemoryType, i );
    if ( curPlugin != NULL ) 
    {
      pluginList->push_back( curPlugin );
      if ( strcasecmp( curPlugin->getName(), DEFAULTMEMORYPLUGIN ) == 0 )
      {
        strcpy( this->szDefaultMemoryPlugin, curPlugin->getName() );
      }
    }
  }  while ( curPlugin != NULL ); /* end */     

  // if default plugin is not available use first plugin as default
  if ( (this->szDefaultMemoryPlugin[0] == '\0') && (pluginList->size() != 0) )
  {
	  strcpy( this->szDefaultMemoryPlugin, (*pluginList)[0]->getName() );
  }
}




/*! \brief Replace a memory with the data from another memory
  This method bevaves like deleting the replace memory and renaming the
  replaceWith memory to the name of the replace memory without the overhead of the
  actual delete and rename operations
  \param pszPluginName name of plugin of the memory
  \param pszReplace name of the memory being replaced
  \param pszReplaceWith name of the memory replacing the pszReplace memory
	returns 0 if successful or error return code
*/
int TMManager::ReplaceMemory
(
  const std::string& strReplace,
  const std::string& strReplaceWith,
  std::string& outputMsg
)
{
  int iRC = EqfMemoryPlugin::eSuccess;

  if(TMExistsOnDisk(strReplace)){
    iRC = EqfMemoryPlugin::eMemoryNotFound;
    return iRC;
  } /* endif */
  if(TMExistsOnDisk(strReplaceWith)){
    iRC = EqfMemoryPlugin::eMemoryNotFound;
    return iRC;
  } /* endif */

  iRC = TMManager::GetInstance()->DeleteTM( strReplace, outputMsg );
  if(!iRC){
    iRC = TMManager::GetInstance()->RenameTM( strReplaceWith, strReplace, outputMsg );
  }
  if(!iRC){
    iRC = !TMManager::GetInstance()->IsMemoryLoaded(strReplace);
  }
  if(!iRC){
    iRC = TMManager::GetInstance()->tms[strReplace]->ReloadFromDisk();
  }
  
  // broadcast deleted memory name for replaceWith memory
  if ( iRC == EqfMemoryPlugin::eSuccess )
  {
    strcat( this->szMemObjName, strReplaceWith.c_str() );
  }
  return( iRC );
}




/*! \brief process the API call: EqfQueryMem and lookup a segment in the memory
  \param lHandle handle of a previously opened memory
  \param pSearchKey pointer to a MemProposal structure containing the searched criteria
  \param *piNumOfProposals pointer to the number of requested memory proposals, will be changed on return to the number of proposals found
  \param pProposals pointer to a array of MemProposal structures receiving the search results
  \param lOptions processing options 
  \returns 0 if successful or an error code in case of failures
*/
USHORT TMManager::APIQueryMem
(
  EqfMemory*        pMem,          
  PMEMPROPOSAL pSearchKey, 
  int         *piNumOfProposals,
  PMEMPROPOSAL pProposals, 
  LONG        lOptions     
)
{
  USHORT usRC = 0;


  return( usRC );
}

wchar_t* wcsupr(wchar_t *str)
{       
  int len = wcslen(str);
  for(int i=0; i<len; i++) {
    str[i] = toupper(str[i]);
  }
  return str;
}

ULONG GetTickCount(){
  struct timespec t; 
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_nsec;
}


/*! \brief process the API call: EqfUpdateDeleteMem and deletes a segment in the memory
    \param lHandle handle of a previously opened memory
    \param pProposalToDelete pointer to an MemProposal structure containing the segment data
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */


// data structure for the APIListMem function and the insert memory callback function
typedef struct _APILISTMEMDATA
{
  const char*   pszBuffer;   // pointer to buffer for memory names or NULL to compute the size of the required buffer
  LONG  lBufSize;  // size of the buffer
  LONG *plLength;    // pointer to a variable containing the current length of the data in the buffer
} APILISTMEMDATA, *PAPILISTMEMDATA;

// callback function to add memory names to a list or update the length of the list
int AddMemToList( void *pvData, char *pszName,std::shared_ptr<EqfMemory>  pInfo )
{
  PAPILISTMEMDATA pData = ( PAPILISTMEMDATA )pvData;
  LONG lNameLen = strlen( pszName );

  if ( pData->pszBuffer != NULL )
  {
    if ( (*(pData->plLength) + lNameLen + 1) <= pData->lBufSize )
    {
      if ( *(pData->plLength) != 0 )
      {
        strcpy( (PSZ)pData->pszBuffer + *(pData->plLength), (PSZ)"," );
        *( pData->plLength ) += 1;
      }
      strcpy( (PSZ)pData->pszBuffer + *(pData->plLength), (PSZ)pszName );
      *( pData->plLength ) += lNameLen;
    }
  }
  else
  {
    *( pData->plLength ) += lNameLen + 1;
  }
  return( 0 );
}


/*! \brief search a string in a proposal
  \param pProposal pointer to the proposal 
  \param pszSearch pointer to the search string (when fIngoreCase is being used, this strign has to be in uppercase)
  \param lSearchOptions combination of search options
  \returns TRUE if the proposal contains the searched string otherwise FALSE is returned
*/
BOOL searchInProposal
( 
  OtmProposal *pProposal,
  PSZ_W pszSearch,
  LONG lSearchOptions
)
{
  BOOL   fFound = FALSE;

  wchar_t szSegmentText[MAX_SEGMENT_SIZE +1];

  if ( !fFound && (lSearchOptions & SEARCH_IN_SOURCE_OPT) )
  {
    pProposal->getSource( szSegmentText, MAX_SEGMENT_SIZE );
    if ( lSearchOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( szSegmentText );
    if ( lSearchOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( szSegmentText );
    fFound = findString( szSegmentText, pszSearch );
  }

  if ( !fFound && (lSearchOptions & SEARCH_IN_TARGET_OPT)  )
  {
    pProposal->getTarget( szSegmentText, MAX_SEGMENT_SIZE );
    if ( lSearchOptions & SEARCH_CASEINSENSITIVE_OPT ) wcsupr( szSegmentText );
    if ( lSearchOptions & SEARCH_WHITESPACETOLERANT_OPT ) normalizeWhiteSpace( szSegmentText );
    fFound = findString( szSegmentText, pszSearch );
  }

  return( fFound );
}

/*! \brief find the given string in the provided data
  \param pszData pointer to the data being searched
  \param pszSearch pointer to the search string
  \returns TRUE if the data contains the searched string otherwise FALSE is returned
*/
BOOL findString
( 
  PSZ_W pszData,
  PSZ_W pszSearch
)
{
  CHAR_W chChar;
  CHAR_W c;
  BOOL   fFound = FALSE;
  BOOL   fEnd = FALSE;

  while ( !fFound && !fEnd )
  {
    // Scan for the 1st letter of the target string don't use strchr, it is too slow                   
    chChar = *pszSearch;
    while ( ((c = *pszData) != 0) && (chChar != c) ) pszData++;

    if ( *pszData == 0 )
    {
      fEnd = TRUE;
    }
    else
    {
      // check for complete match
      if ( compareString( pszData, pszSearch ) == 0)
      {
        fFound = TRUE;
      }
      else         // no match; go on if possible
      {
        pszData++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  return( fFound );
}

/*! \brief check if search string matches current data
  \param pData pointer to current position in data area
  \param pSearch pointer to search string
  \returns 0 if search string matches data
*/
SHORT compareString
(
  PSZ_W   pData,
  PSZ_W   pSearch
)
{
  SHORT  sRc = 0;                               // init strings are equal
  CHAR_W c, d;
  while (((d = *pSearch) != NULC) && ((c = *pData) != NULC) )
  {
    if ( c == d )
    {
      pData++; pSearch++;
    }
    else
    {
      sRc = c-d;
      break;
    } /* endif */
  } /* endwhile */
  if (*pSearch )
  {
    sRc = -1;
  } /* endif */
  return sRc;
}

/*! \brief normalize white space within a string by replacing single or multiple white space occurences to a single blank
  \param pszString pointer to the string being normalized
  \returns 0 in any case
*/
SHORT normalizeWhiteSpace
(
  PSZ_W   pszData
)
{
  PSZ_W pszTarget = pszData;
  while ( *pszData )
  {
    CHAR_W c = *pszData;
    if ( (c == BLANK) || (c == LF) || (c == CR) || (c == 0x09) )
    {
      do
      {
        pszData++;
        c = *pszData;
      } while ( (c == BLANK) || (c == LF) || (c == CR) || (c == 0x09) );
      *pszTarget++ = BLANK;
    } 
    else
    {
      *pszTarget++ = *pszData++;
    } /* endif */
  } /* endwhile */
  *pszTarget = 0;
  return 0;
}

/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyMemProposalToOtmProposal( PMEMPROPOSAL pProposal, OtmProposal *pOtmProposal )
{
  if ( (pProposal == NULL) || (pOtmProposal == NULL) ) return;

  pOtmProposal->setID( pProposal->szId );
  pOtmProposal->setSource( pProposal->szSource );
  pOtmProposal->setTarget( pProposal->szTarget );
  pOtmProposal->setDocName( pProposal->szDocName );
  pOtmProposal->setDocShortName( pProposal->szDocShortName );
  pOtmProposal->setSegmentNum( pProposal->lSegmentNum );
  pOtmProposal->setSourceLanguage( pProposal->szSourceLanguage );
  pOtmProposal->setIsSourceLangIsPrefered( pProposal->fIsoSourceLangIsPrefered);
  pOtmProposal->setTargetLanguage( pProposal->szTargetLanguage );

  switch( pProposal->eType )
  {
    case MANUAL_PROPTYPE: pOtmProposal->setType( OtmProposal::eptManual ); break;
    case MACHINE_PROPTYPE: pOtmProposal->setType( OtmProposal::eptMachine ); break;
    case GLOBMEMORY_PROPTYPE: pOtmProposal->setType( OtmProposal::eptGlobalMemory ); break;
    case GLOBMEMORYSTAR_PROPTYPE: pOtmProposal->setType( OtmProposal::eptGlobalMemoryStar ); break;
    default: pOtmProposal->setType( OtmProposal::eptUndefined ); break;
  } /* endswitch */
  switch( pProposal->eMatch )
  {
    case EXACT_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExact ); break;
		case EXACTEXACT_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExactExact ); break;
    case EXACTSAMEDOC_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtExactSameDoc ); break;
    case FUZZY_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtFuzzy ); break;
    case REPLACE_MATCHTYPE: pOtmProposal->setMatchType( OtmProposal::emtReplace ); break;
    default: pOtmProposal->setMatchType( OtmProposal::emtUndefined ); break;
  } /* endswitch */
  pOtmProposal->setAuthor( pProposal->szTargetAuthor );
  pOtmProposal->setUpdateTime( pProposal->lTargetTime );
  pOtmProposal->setFuzziness( pProposal->iFuzziness );
  pOtmProposal->setMarkup( pProposal->szMarkup );
  pOtmProposal->setContext( pProposal->szContext );
  pOtmProposal->setAddInfo( pProposal->szAddInfo );
}

/*! \brief copy the data of a MEMPROPOSAL structure to a OtmProposal object
  \param pMemProposal pointer to MEMPROPOSAL structure 
  \param pOtmProposal pointer to OtmProposal object
  \returns 0 in any case
*/
void copyOtmProposalToMemProposal( OtmProposal *pOtmProposal, PMEMPROPOSAL pProposal  )
{
  if ( (pProposal == NULL) || (pOtmProposal == NULL) ) return;

  pOtmProposal->getID( pProposal->szId, sizeof(pProposal->szId) );
  pOtmProposal->getSource( pProposal->szSource, sizeof(pProposal->szSource)/sizeof(CHAR_W) );
  pOtmProposal->getTarget( pProposal->szTarget, sizeof(pProposal->szTarget)/sizeof(CHAR_W) );
  pOtmProposal->getDocName( pProposal->szDocName, sizeof(pProposal->szDocName) );
  pOtmProposal->getDocShortName( pProposal->szDocShortName, sizeof(pProposal->szDocShortName) );
  pProposal->lSegmentNum = pOtmProposal->getSegmentNum();

  pOtmProposal->getOriginalSourceLanguage( pProposal->szSourceLanguage, sizeof(pProposal->szSourceLanguage) );
  pOtmProposal->getTargetLanguage( pProposal->szTargetLanguage, sizeof(pProposal->szTargetLanguage) );
  switch( pOtmProposal->getType() )
  {
    case OtmProposal::eptManual: pProposal->eType = MANUAL_PROPTYPE; break;
    case OtmProposal::eptMachine: pProposal->eType = MACHINE_PROPTYPE; break;
    case OtmProposal::eptGlobalMemory: pProposal->eType = GLOBMEMORY_PROPTYPE; break;
    case OtmProposal::eptGlobalMemoryStar: pProposal->eType = GLOBMEMORYSTAR_PROPTYPE; break;
    default: pProposal->eType = UNDEFINED_PROPTYPE; break;
  } /* endswitch */
  switch( pOtmProposal->getMatchType() )
  {
    case OtmProposal::emtExact: pProposal->eMatch = EXACT_MATCHTYPE; break;
    case OtmProposal::emtExactExact: pProposal->eMatch = EXACTEXACT_MATCHTYPE; break;
    case OtmProposal::emtExactSameDoc : pProposal->eMatch = EXACTSAMEDOC_MATCHTYPE; break;
    case OtmProposal::emtFuzzy: pProposal->eMatch = FUZZY_MATCHTYPE; break;
    case OtmProposal::emtReplace: pProposal->eMatch = REPLACE_MATCHTYPE; break;
    default: pProposal->eMatch = UNDEFINED_MATCHTYPE; break;
  } /* endswitch */
  pOtmProposal->getAuthor( pProposal->szTargetAuthor, sizeof(pProposal->szTargetAuthor) );
  pProposal->lTargetTime = pOtmProposal->getUpdateTime();
  pProposal->iFuzziness  = pOtmProposal->getFuzziness();
  pProposal->iDiffs = pOtmProposal->getDiffs();
  pProposal->iWords = pOtmProposal->getWords();
  pOtmProposal->getMarkup( pProposal->szMarkup, sizeof(pProposal->szMarkup) );
  pOtmProposal->getContext( pProposal->szContext, sizeof(pProposal->szContext)/sizeof(CHAR_W) );
  pOtmProposal->getAddInfo( pProposal->szAddInfo, sizeof(pProposal->szAddInfo)/sizeof(CHAR_W) );
}


std::string FilesystemHelper::GetTmdPath(const std::string& memName){
  return FilesystemHelper::GetMemDir() + memName  + EXT_OF_TMDATA;
}

std::string FilesystemHelper::GetTmiPath(const std::string& memName){
  return FilesystemHelper::GetMemDir() + memName  + EXT_OF_TMINDEX;
}

int TMManager::TMExistsOnDisk(const std::string& tmName, bool logErrorIfNotExists){  
  int logLevel= 0;
  if(!logErrorIfNotExists) logLevel = T5Logger::GetInstance()->suppressLogging();
  //check tmd file
  int res = TMM_NO_ERROR;
  if(FilesystemHelper::FileExists(FilesystemHelper::GetTmdPath(tmName)) == false){
    res |= TMM_TMD_NOT_FOUND;
  }

  //check tmi file
  if(FilesystemHelper::FileExists(FilesystemHelper::GetTmiPath(tmName)) == false){
    res |= TMM_TMI_NOT_FOUND;
  }
  if(!logErrorIfNotExists) T5Logger::GetInstance()->desuppressLogging(logLevel);
  return res;
}


bool TMManager::IsMemoryLoaded(const std::string& strMemName){
  return tms.count(strMemName);
}

int TMManager::OpenTM(const std::string& strMemName){
  if(IsMemoryLoaded(strMemName)){
    return 0;
  }
  size_t requiredMemory = 0;
  if(int res = TMExistsOnDisk(strMemName)){
    T5LOG(T5ERROR) << "TM is not found, name = " << strMemName <<"; res = " << res;
    return 404;
  }else{
    requiredMemory += FilesystemHelper::GetFileSize( FilesystemHelper::GetTmdPath(strMemName));
    requiredMemory += FilesystemHelper::GetFileSize( FilesystemHelper::GetTmiPath(strMemName));
    requiredMemory *= 1.2;
    //requiredMemory += FilesystemHelper::GetFileSize( szTempFile ) * 2;
    //requiredMemory += strBody.size() * 2;
  }
  //TODO: add to requiredMemory value that would present changes in mem files sizes after import done 

  // cleanup the memory list (close memories not used for a longer time)
  size_t memLeftAfterOpening = CleanupMemoryList(requiredMemory);
  if(VLOG_IS_ON(1)){
    T5LOG( T5INFO) << ":: memory: " << strMemName << "; required RAM:" 
        << requiredMemory << "; RAM left after opening mem: " << memLeftAfterOpening;
  }
  //requiredMemory = 0;
  // find a free slot in the memory list
  //std::shared_ptr<EqfMemory>  pMem = TMManager::GetInstance()->GetFreeSlotPointer(requiredMemory);

  // handle "too many open memories" condition
  //if ( pMem == nullptr )
    
  if(memLeftAfterOpening <= 0){
    //buildErrorReturn( _rc_, "OtmMemoryServiceWorker::import::Error: too many open translation memory databases" );
    return( INTERNAL_SERVER_ERROR );
  } /* endif */
  std::shared_ptr<EqfMemory> pMem( EqfMemoryPlugin::GetInstance()->openMemoryNew(strMemName));
  if(!pMem){
    T5LOG(T5ERROR) << "::Can't open the file \'"<< strMemName<<"\'";
    return 1;
  }
  tms[strMemName] = pMem;
  return 0; 
}

int TMManager::AddMem(const std::shared_ptr<EqfMemory> NewMem){
  if(NewMem == nullptr || NewMem->szName == "\0"){
    return -1;
  }
  if(IsMemoryLoaded(NewMem->szName)){
    return -2;
  }
  tms[NewMem->szName] = NewMem;
  return 0;
}

int TMManager::CloseTM(const std::string& strMemName){
  if(!IsMemoryLoaded(strMemName)){
    return 404;
  }
  tms.erase(strMemName);
  return 0;
}

std::shared_ptr<EqfMemory> TMManager::requestServicePointer(const std::string& strMemName, COMMAND command){
  std::lock_guard<std::mutex> l{mutex_requestTM};
  std::shared_ptr<EqfMemory> mem;
  if(IsMemoryLoaded(strMemName)){
    if(command == STATUS_MEM){
      return tms[strMemName];
    }
  }
  return nullptr;
}

std::shared_ptr<EqfMemory> TMManager::requestReadOnlyTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack){
  int rc = 0;
  std::lock_guard<std::mutex> l{mutex_requestTM};
  if(!IsMemoryLoaded(strMemName)){
    rc = OpenTM(strMemName);
  }
  std::shared_ptr<EqfMemory>  mem;
  if(rc){
    //return;
  }else if(tms.count(strMemName)){
    mem = tms[strMemName];
  } 
  if(mem){
    //check if there are any Write pointers
    rc = mem->writeCnt.use_count()>1;

    T5LOG(T5DEBUG) <<"writeCnt = " << mem->writeCnt.use_count();
    if(rc){
      mem.reset();
    }
  }
  //
  
  if(mem){
    refBack = mem->readOnlyCnt;
    T5LOG(T5DEBUG) <<"readOnlyCnt = " << mem->readOnlyCnt.use_count();
  }
  return mem;
}

std::shared_ptr<EqfMemory> TMManager::requestWriteTMPointer(const std::string& strMemName, std::shared_ptr<int>& refBack){
  int rc = 0;
  std::lock_guard<std::mutex> l{mutex_requestTM};
  if(!IsMemoryLoaded(strMemName)){
    rc = OpenTM(strMemName);
  }

  std::shared_ptr<EqfMemory>  mem; 
  if(rc){ //Memory failed to open
    //return;
  }else if(tms.count(strMemName)){
    mem = tms[strMemName];
    //check if there are any Write pointers
    rc = mem->writeCnt.use_count() > 1;
    refBack = mem->writeCnt;
  }else{
    rc = 2;
  }
  //
  if(!rc){ // we have some Write process;
    //mem.reset();
    rc = mem->readOnlyCnt.use_count() > 1;
  }else{
  }

  if(rc){ // we have some Read process
    mem.reset();
    refBack.reset();
  }
  //else{}
  //refBack = mem->readOnlyCnt;
  return mem;
}
