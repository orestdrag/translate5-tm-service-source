//+----------------------------------------------------------------------------+
//|EQFNTOP.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>
#include <EQFEVENT.H>             // event logging
#include "LogWrapper.h"

#include "requestdata.h"


int EqfMemory::getErrorCode()const{
  return lastErrorCode;
}

int EqfMemory::setErrorCode(int rc){
  return lastErrorCode = rc;
}

void EqfMemory::reserErrorCode(){
  lastErrorCode = 0;
}

std::string StatusToString(int eStatus)
{
  switch(eStatus){
    case OPEN_STATUS:{
      return "open";
      break;
    }
    case WAITING_FOR_LOADING_STATUS:{
      return "waiting for loading";
      break;
    }
    case FAILED_TO_OPEN_STATUS:{
      return "failed to open";
      break;
    }    
    case LOADING_STATUS:{
      return "loading";
      break;
    }

    case IMPORT_FAILED_STATUS:{
      return "import failed";
      break;
    }
    case IMPORT_RUNNING_STATUS:{
      return "import running";
      break;
    }

    case REORGANIZE_FAILED_STATUS:{
      return "reorganize failed";
      break;
    }
    case REORGANIZE_RUNNING_STATUS:{
      return "reorganize running";
      break;
    }

    case AVAILABLE_STATUS:{
      return "available";
      break;
    }
    default:{
      return "unknown " + toStr(eStatus) + " status";
      break;
    }
  }
}


COMMAND EqfMemory::getActiveRequestCommand()
{
  //auto r = pActiveRequest.lock();
  //r = pActiveRequest;
  //if(r){
  //  return r->command;
  //}
  return activeCommand;
  //return COMMAND::UNKNOWN_COMMAND;
}


void EqfMemory::setActiveRequestCommand(COMMAND command)
{
  activeCommand = command;
}

void EqfMemory::resetActiveRequestCommand()
{
  activeCommand = UNKNOWN_COMMAND;
}

std::string EqfMemory::getStatusString()const
{
  return StatusToString(eStatus);
}

std::string commandToString(){

}
std::string EqfMemory::getActiveRequestString()const
{
  //if (auto request = pActiveRequest.lock()) {
  //  return searchInCommandToStringsMap(request->command);
  //}
  if(activeCommand)  return searchInCommandToStringsMap(activeCommand);
  return "";    
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TmtXOpen     opens   data and index files                |
//+----------------------------------------------------------------------------+
//|Description:       Opens   qdam data and index files                        |
//+----------------------------------------------------------------------------+
//|Function call:  TmtXOpen  ( PTMX_OPEN_IN pTmOpenIn, //input struct          |
//|                            PTMX_OPEN_OUT pTmOpenOut ) //output struct      |
//+----------------------------------------------------------------------------+
//|Input parameter: PTMX_OPEN_IN pTmOpenIn     input structure                 |
//+----------------------------------------------------------------------------+
//|Output parameter: PTMX_OPEN_OUT pTmOpenOut   output structure               |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes: identical to return code in open out structure                 |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//| allocate control block     - NTM_CLB                                       |
//| allocate tag table, file name, author and compact area blocks in control   |
//|  block                                                                     |
//| open tm data file                                                          |
//| get signature record contents and store in control block                   |
//| get tag table, author, file name and compact area record from tm data file |
//|  and add to control block                                                  |
//| open index file                                                            |
//|                                                                            |
//| return open out structure                                                  |
// ----------------------------------------------------------------------------+
DECLARE_bool(allowLoadingMultipleTmsSimultaneously);
USHORT EqfMemory::Load(){
  USHORT usRC = LoadMem();
  // create memory object if create function completed successfully
  if ( (usRC != 0) && (usRC != BTREE_CORRUPTED) /*&& (usAccessMode == FOR_ORGANIZE)*/){
    T5LOG(T5ERROR) << "EqfMemoryPlugin::openMemory:: TmOpen fails, fName = "<< szName<< "; error = "<< this->strLastError<<"; iLastError = "<< 
        this->iLastError << "; rc = " << usRC;
  }
  return usRC;
}

USHORT EqfMemory::LoadMem()
{
  //TimedMutexGuard l(tmMutex);
  if(isLoaded())
  {
    return 0;
  }

  if(isLoading())
  {
    return 0;
  }

  //if(isFailedToLoad()){
  //  T5LOG()
 // }
  
  if(false == FLAGS_allowLoadingMultipleTmsSimultaneously){
    T5LOG(T5INFO) << "Locking loadingTm mutex for "<< szName;
    FilesystemHelper::loadingTm.lock();
    T5LOG(T5INFO) << "Locked loadingTm mutex for "<< szName;
  }
  eStatus = LOADING_STATUS;

  BOOL fOK;                      //success indicator
  USHORT usRc = NO_ERROR;        //return value
  USHORT usRc1 = NO_ERROR;       //return value
  ULONG  ulLen;                  //length indicator
 
  //if(!usRc)
  {
    //TmBtree.fb.fileName = pTmOpenIn->stTmOpen.szDataName;
    //InBtree.fb.fileName = pTmOpenIn->stTmOpen.szIndexName;
    //call open function for data file
    
    usRc1 = TmBtree.EQFNTMOpen((USHORT)(usAccessMode | ASD_FORCE_WRITE) );
    if ( (usRc1 == NO_ERROR) || (usRc1 == BTREE_CORRUPTED) )
    {
      //get signature record and add to control block
      USHORT usLen = sizeof( TMX_SIGN );

      usRc = TmBtree.EQFNTMSign((PCHAR) &(stTmSign), &usLen );

      if ( usRc == NO_ERROR )
      {
        int T5MAJVERSION_MIN_SUPPORTED = 5;
        int T5MAJVERSION_MAX_SUPPORTED = T5MAJVERSION;
        int T5MINVERSION_MIN_SUPPORTED = 60;
        int T5MINVERSION_MAX_SUPPORTED = T5MINVERSION;

        if ( stTmSign.bGlobVersion > T5GLOBVERSION  
            || stTmSign.bMajorVersion > T5MAJVERSION_MAX_SUPPORTED 
            //|| (stTmSign.bMajorVersion == T5MAJVERSION_MAX_SUPPORTED && stTmSign.bMinorVersion > T5MINVERSION_MAX_SUPPORTED )
            )
        {
          T5LOG(T5ERROR) << "TM was created in newer vertions of t5memory, v" 
                  << stTmSign.bGlobVersion << "." << stTmSign.bMajorVersion << "." << stTmSign.bMinorVersion;
          usRc = ERROR_VERSION_NOT_SUPPORTED;
        }
        else if (stTmSign.bGlobVersion < T5GLOBVERSION  
            || stTmSign.bMajorVersion < T5MAJVERSION_MIN_SUPPORTED 
            || (stTmSign.bMajorVersion == T5MAJVERSION_MIN_SUPPORTED && stTmSign.bMinorVersion < T5MINVERSION_MAX_SUPPORTED ) )
        {
          T5LOG(T5ERROR) << "TM was created in older vertions of t5memory, v"<<stTmSign.bGlobVersion<<"."<<stTmSign.bMajorVersion<<"."<<stTmSign.bMinorVersion;
          usRc = VERSION_MISMATCH;
        } /* endif */
      }
      else if ( usAccessMode & ASD_ORGANIZE )
      {
        // allow to continue even if signature record is corrupted
        usRc = NO_ERROR;
      } /* endif */

      if ( (usRc == NO_ERROR) || (usRc == VERSION_MISMATCH) )
      {
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {          
          ulLen =  MAX_COMPACT_SIZE-1;
          //get compact area and add to control block
          USHORT usTempRc =  TmBtree.EQFNTMGet( COMPACT_KEY, (PCHAR)bCompact, &ulLen );

          // in organize mode allow continue if compact area is corrupted
          if ( (usTempRc != NO_ERROR) && (usTempRc != VERSION_MISMATCH) )
          {
            usTempRc = BTREE_CORRUPTED;
          } /* endif */

          if ( usTempRc == BTREE_CORRUPTED )
          {
            memset( bCompact, 0, sizeof(bCompact) );
          } /* endif */
          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */


        //get languages and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          DEBUGEVENT( TMTXOPEN_LOC, STATE_EVENT, 2 );

          //call to obtain exact length of record
          ulLen = 0;
          USHORT usTempRc = NTMLoadNameTable( LANG_KEY,
                                      &Languages, &ulLen );

          if ( usTempRc == BTREE_READ_ERROR ) 
              usTempRc = BTREE_CORRUPTED;
          usTempRc = NTMCreateLangGroupTable();

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get file names and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( FILE_KEY,
                                       &FileNames,
                                       &ulLen );

          // in organize mode allow continue if file name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            //FileNames.ulAllocSize = TMX_TABLE_SIZE;
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get authors and add to control block
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( AUTHOR_KEY,
                                       &Authors, &ulLen );

          // in organize mode allow continue if author name area is corrupted
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {          
            //Authors.ulAllocSize = TMX_TABLE_SIZE;            
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get tag tables and add to control block
      if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          USHORT usTempRc = NTMLoadNameTable( TAGTABLE_KEY,
                                       &TagTables, &ulLen );


          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;
          if ( usTempRc == BTREE_CORRUPTED )
          {
            //usTempRc = TM_FILE_SCREWED_UP; // cannot continue if no tag tables
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */

        //get long document name table
        if ( (usRc == NO_ERROR) ||
             (usRc == BTREE_CORRUPTED) ||
             (usRc == VERSION_MISMATCH) )
        {
          // error in memory allocations will force end of open!
          USHORT usTempRc = NTMCreateLongNameTable();
          if ( usTempRc == BTREE_READ_ERROR ) usTempRc = BTREE_CORRUPTED;

          // now read any long name table from database, if there is
          // none (=older TM before TOP97) write our empty one to the
          // Translation Memory
          if ( usTempRc == NO_ERROR )
          {
            usTempRc = NTMReadLongNameTable();

            switch ( usTempRc)
            {
              case ERROR_NOT_ENOUGH_MEMORY:
                // leave return code as is
                break;
              case NO_ERROR:
                // O.K. no problems at all
                break;
              case BTREE_NOT_FOUND :
              T5LOG(T5ERROR) <<  ":: TEMPORARY_COMMENTED temcom_id = 48 usTempRc = NTMWriteLongNameTable();";
#ifdef TEMPORARY_COMMENTED
                // no long name tabel yet,create one ...
                usTempRc = NTMWriteLongNameTable();
                #endif
                break;
              default:
                // read of long name table failed, assume TM is corrupted
                //if ( pTmOpenIn->stTmOpen.usAccess == FOR_ORGANIZE )
                {
                } /* endif */
                usTempRc = BTREE_CORRUPTED;
                break;
            } /* endswitch */
          } /* endif */

          // just for debugging purposes: check if short and long name table have the same numbe rof entries
          if ( usTempRc == NO_ERROR )
          {
            if ( LongNames.stTableEntry.size() != FileNames.ulMaxEntries )
            {
              // this is strange...
              int iSetBreakPointHere = 1;
            }
          } /* endif */

          if ( usTempRc != NO_ERROR )
          {
            usRc = usTempRc;
          } /* endif */
        } /* endif */
      } /* endif */

      //add threshold to control block
      usThreshold = 33;//pTmOpenIn->stTmOpen.usThreshold;


      if ( (usRc == NO_ERROR) ||
           (usRc == BTREE_CORRUPTED) ||
           (usRc == VERSION_MISMATCH) )
      {
        //call open function for index file
        USHORT usIndexRc = InBtree.EQFNTMOpen(usAccessMode );
        if ( usIndexRc != NO_ERROR  && !isOpenedOnlyForReorganize())
        {
          usRc = usIndexRc;
        } /* endif */
      } /* endif */

      if ( usRc == NO_ERROR)
      {
        // no error during open of index, use return code of EQFNTMOPEN for
        // data file
        usRc = usRc1;
      }
      else
      {
        // open of index failed or index is corrupted so leave usRc as-is;
        // data file will be closed in cleanup code below
      } /* endif */
    }
    else
    {
      // error during open of data file, use return code of EQFNTMOPEN for
      // data file
      usRc = usRc1;
    } /* endif */

  } /* endif */

  /********************************************************************/
  /* Ensure that all required data areas have been loaded and the     */
  /* database files have been opened                                  */
  /********************************************************************/

  if ( (usRc != NO_ERROR) &&
       (usRc != BTREE_CORRUPTED) &&
       (usRc != VERSION_MISMATCH) )
  {
    TmBtree.QDAMDictClose();
    InBtree.QDAMDictClose();

    T5LOG(T5ERROR) << "Failed to open tm \""<< this->szName << "\" with rc = " << usRc;
    setErrorCode(usRc);
    eStatus = FAILED_TO_OPEN_STATUS;
  }else{
    eStatus = OPEN_STATUS;
  } /* endif */

  if(false == FLAGS_allowLoadingMultipleTmsSimultaneously){
    FilesystemHelper::loadingTm.unlock();
    T5LOG(T5INFO) << "Unlocked loadingTm mutex for "<< szName;
  }
  return( usRc );
}


