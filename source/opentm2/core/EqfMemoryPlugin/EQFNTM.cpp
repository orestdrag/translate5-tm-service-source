//------------------------------------------------------------------------------
// EQFNTM.C
//------------------------------------------------------------------------------
// Description: TranslationMemory interface functions.
//------------------------------------------------------------------------------
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
// Entry Points:
// C TmCreate
// C TmOpen
// C TmClose
// C TmReplace
// C TmGet
// C TmExtract
// C TmInfo
// C TmDeleteTm
// C TmDeleteSegment
//------------------------------------------------------------------------------
// Externals:
//------------------------------------------------------------------------------
// Internals:
// C NTMGetThresholdFromProperties
// C NTMFillCreateInStruct
//------------------------------------------------------------------------------

/**********************************************************************/
/* needed header files                                                */
/**********************************************************************/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#include <EQF.H>                  // General Translation Manager include file

#include "LogWrapper.h"
#include <EQFSETUP.H>             // Directory name defines and other
#include "EQFDDE.H"               // Batch mode definitions
#include <tm.h>               // Private header file of Translation Memory
#include <EQFEVENT.H>             // Event logging
#include "FilesystemWrapper.h"
#include "Property.h"

#ifdef _DEBUG
  //#define SESSIONLOG
#endif
/**********************************************************************/
/* prototypes for internal functions                                  */
/**********************************************************************/


USHORT
TmGetW(EqfMemory*            htm,             //(in)  TM handle
       PSZ            szMemPath,       //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W  pstGetIn,        //(in)  pointer to get input structure
       PTMX_GET_OUT_W pstGetOut,       //(out) pointer to get output structure
       USHORT         usMsgHandling )  //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
{
  USHORT      usRc;                    //U code rc
  USHORT      usQRc;                   //EqfSend2Handler
  SERVERNAME  szServer;                //var for server name
  BOOL        fOk;                     //rc from UtlAlloc
  PSZ_W       pszTempString;           //temp string for conversion of CRLF
  USHORT      usI;                     //index var for for loop

  DEBUGEVENT( TMGET_LOC, FUNCENTRY_EVENT, 0 );

  /********************************************************************/
  /* fill the TMX_GET_IN structure                                    */
  /* the TMX_GET_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  usRc = TmtXGet( htm, pstGetIn, pstGetOut );

  if ( (usRc == NO_ERROR) && pstGetOut->usNumMatchesFound )
  {
    /******************************************************************/
    /* convert the output according to convert flag                   */
    /******************************************************************/
    if ( pstGetIn->stTmGet.usConvert != MEM_OUTPUT_ASIS )
    {
      /****************************************************************/
      /* allocate storage for temp string                             */
      /****************************************************************/
      fOk = UtlAlloc( (PVOID *) &pszTempString, 0L,
                      (ULONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W),
                      NOMSG );
      if ( fOk )
      {
        /**************************************************************/
        /* loop over all found matches returned in get out struct     */
        /**************************************************************/
        for ( usI=0 ; usI < pstGetOut->usNumMatchesFound; usI++ )
        {
          /************************************************************/
          /* convert source string                                    */
          /************************************************************/
          T5LOG(T5FATAL) <<"TEMPORARY_COMMENTED in TmGetW::NTMConvertCRLFW";
#ifdef TEMPORARY_COMMENTED
          NTMConvertCRLFW( pstGetOut->stMatchTable[usI].szSource,
                          pszTempString,
                          pstGetIn->stTmGet.usConvert );

          /************************************************************/
          /* convert target string                                    */
          /************************************************************/
          NTMConvertCRLFW( pstGetOut->stMatchTable[usI].szTarget,
                          pszTempString,
                          pstGetIn->stTmGet.usConvert );
          #endif
        } /* endfor */
        /**************************************************************/
        /* free storage for temp string                               */
        /**************************************************************/
        UtlAlloc( (PVOID *) &pszTempString, 0L, 0L, NOMSG );
      }
      else
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMGET_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* End of function TmGet */




