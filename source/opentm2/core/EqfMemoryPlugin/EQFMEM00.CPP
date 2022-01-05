//------------------------------------------------------------------------------
//  EQFMEM00.C - EQF Memory List Handler
//------------------------------------------------------------------------------
//  Copyright Notice:
//
//      Copyright (C) 1990-2016, International Business Machines
//      Corporation and others. All rights reserved


#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_DAM

#include "../pluginmanager/PluginManager.h"
#include "../pluginmanager/OtmMemoryPlugin.h"
#include "../pluginmanager/OtmMemory.h"
#include "LogWrapper.h"
#include "MemoryFactory.h"
#include "MemoryUtil.h"

// EQF.H is included by otmmemory.h
// #include <EQF.H>                  // General Translation Manager include file EQF:H 

#include <EQFDDE.H>                  // batch mode definitions
#define INCL_EQFMEM_DLGIDAS
#include "EQFMEM.ID"                 // Translation Memory IDs
#include "EQFCOLW.ID"                // column width IDs

BOOL EQFTMMaintain( PSZ );

#include <EQFTMI.H>               // Private header file of Translation Memory

// new function to replace CREATE_PATH task message
BOOL MemCreatePath( PSZ pszString )
{
  USHORT          usRc;

  OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  usRc = (USHORT)-1;
  if(pInfo != NULL && pFactory!= NULL)
     usRc = (USHORT)pFactory->getMemoryInfo( NULL, pszString, pInfo );

  if (usRc == 0)
  {
	  strcpy( pszString,pInfo->szName);
  }
  else
  {
     *pszString = EOS;
  }
  if(pInfo != NULL)
      delete pInfo;

  return (usRc==0);
} /* end of function MemCreatePath */

//------------------------------------------------------------------------------
//Internal function
//------------------------------------------------------------------------------
//Function name:     MemFuncDeleteMem
//------------------------------------------------------------------------------
//Description:       Deletes a Translation Memory in function I/F mode
//------------------------------------------------------------------------------
//Input parameter:  PSZ pszMemName  ptr to name of memory
//------------------------------------------------------------------------------
//Returncode type:   USHORT              error code or 0 if success
//------------------------------------------------------------------------------

__declspec(dllexport)
USHORT MemFuncDeleteMem( PSZ pszMemName )
{
   BOOL            fOK = TRUE;         // return value
   USHORT          usRC = NO_ERROR;    // function return code

   // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
        LogMessage(ERROR, "Error in MemFuncDeleteMem::(pszMemName == NULL) || (*pszMemName == EOS) ");
     } /* endif */
   } /* endif */


   OtmMemoryPlugin::MEMORYINFO Info;
   MemoryFactory *pFactory = MemoryFactory::getInstance();
   // check if there is a TM with the given name
   if ( fOK )
   {
       // check if memory existed
        usRC = (USHORT)pFactory->getMemoryInfo( NULL, pszMemName, &Info );
        if (usRC != 0)
        {
            fOK = FALSE;
                          
          LogMessage4(ERROR, "Error in MemFuncDeleteMem::check if memory existed error:: memName =  ", pszMemName, " not found; usRC = ", toStr(usRC).c_str());
        }
   }

   // Delete the TM
   if ( fOK )
   {
     int iRC = pFactory->deleteMemory( NULL,pszMemName);
     fOK = (iRC==0)?TRUE:FALSE;
     
   }

   /*******************************************************************/
   /* Cleanup                                                         */
   /*******************************************************************/
   if ( !fOK )
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   }

   return( usRC );
} /* end of function MemFuncDeleteMem  */
