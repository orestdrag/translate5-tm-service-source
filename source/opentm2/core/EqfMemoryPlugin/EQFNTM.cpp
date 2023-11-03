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

USHORT C_TmInfoHwnd( HTM, PSZ, USHORT, PTMX_INFO_IN, PTMX_INFO_OUT, USHORT, HWND );


// utility to get the property file extension
char *GetPropFileExtension( char *pszMemPath )
{
  return( EXT_OF_MEM );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmCreate
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmCreate( PSZ         pszPathMem,
//                              HTM         *htm
//                              PSZ         pszSourceLang  )
//------------------------------------------------------------------------------
// Description:       TM interface function to create a TM
//------------------------------------------------------------------------------
// Parameters:        pszPathMem     - (in)  full TM name x:\eqf\mem\mem.tmd
//                    *htm           - (out) TM handle
//                    hModel         - (in)  model handle
//                    pszServer      - (in)  server name or empty string
//                    pszUserID      - (in)  LAN USERID or empty string
//                    pszSourceLang  - (in)  source language or empty string
//                    pszDescription - (in)  TM description or empty string
//                    usMsgHandling  - (in)  message handling parameter
//                                           TRUE:  display error message
//                                           FALSE: display no error message
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     TmClose
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmClose( HTM        htm,
//                             PSZ        szMemPath,
//                             USHORT     usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to close a TM
//------------------------------------------------------------------------------
// Parameters:        htm           - (in) TM handle returned from open
//                    szMemPath     - (in) full TM name x:\eqf\mem\mem.tmd
//                    usMsgHandling - (in) message handling parameter
//                                         TRUE:  display error message
//                                         FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
TmClose( HTM        htm,               //(in) TM handle returned from open
         PSZ        szMemPath,         //(in) full TM name x:\eqf\mem\mem.tmd
         USHORT     usMsgHandling,     //(in) message handling parameter
                                   //     TRUE:  display error message
                                   //     FALSE: display no error message
         HWND       hwnd )         //(in) handle for error messages
{
  USHORT            usRc;               //function return code
  PTMX_CLOSE_IN     pstCloseIn  = NULL; //close input structure
  PTMX_CLOSE_OUT    pstCloseOut = NULL; //close output structure
  BOOL              fOk;                //process flag
  USHORT            usQRc;              //rc from EqfSend2Handler
  SERVERNAME        szServer;           //local var for server name

  //ERREVENT( TMCLOSE_LOC, ERROR_EVENT, 0 );

  /********************************************************************/
  /* initialze function return code                                   */
  /********************************************************************/
  LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);

  /********************************************************************/
  /* do processing only when a valid TM handle is passed              */
  /********************************************************************/
  if ( htm != NULLHANDLE )
  {
    /******************************************************************/
    /* allocate storage for TMX_CLOSE_IN and TMX_CLOSE_OUT            */
    /******************************************************************/
    fOk = UtlAlloc( (PVOID *) &pstCloseIn, 0L,
                    (LONG)( sizeof( TMX_CLOSE_IN ) +
                            sizeof( TMX_CLOSE_OUT ) ),
                    FALSE );
    if ( fOk )
    {
      /****************************************************************/
      /* assign memory to pointer pstCloseOut                         */
      /****************************************************************/
      pstCloseOut = (PTMX_CLOSE_OUT)(pstCloseIn + 1);

      /****************************************************************/
      /* call U code to pass TM command to server or handle it local  */
      /****************************************************************/
      usRc = TmtXClose ( (EqfMemory*)htm, pstCloseIn, pstCloseOut );

      /****************************************************************/
      /* if an error occured call MemRcHandling in dependency of      */
      /* the message flag to display error message                    */
      /****************************************************************/
      if ( usMsgHandling && usRc )
      {
        usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
      } /* endif */

      // cleanup
      UtlAlloc( (PVOID *) &pstCloseIn, 0L, 0L, NOMSG );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* no valid TM handle was passed (handle is NULL)                 */
    /* handle this as no error                                        */
    /******************************************************************/
    LOG_AND_SET_RC(usRc, T5INFO, NO_ERROR);
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMCLOSE_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function TmClose */



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
        LOG_AND_SET_RC(usRc, T5INFO, ERROR_NOT_ENOUGH_MEMORY);
      } /* endif */
    } /* endif */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMGET_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* End of function TmGet */


USHORT
C_TmInfoHwnd( HTM           htm,            //(in)  TM handle
        PSZ           szMemPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        USHORT        usInfoLevel,    //(in)  information level
                                      //        TMINFO_SIGNATURE
        PTMX_INFO_IN  pstInfoIn,      //(in)  pointer to info input structure
        PTMX_INFO_OUT pstInfoOut,     //(out) pointer to info output structure
        USHORT        usMsgHandling,  //(in)  message handling parameter
                                      //      TRUE:  display error message
                                      //      FALSE: display no error message
        HWND          hwnd )          //(in)  handle for error messages
{
  USHORT      usRc;                   //function return code
  USHORT      usQRc;                  //rc from EqfSend2Handler
  SERVERNAME  szServer;               //local var for server name

  /********************************************************************/
  /* fill the TMX_INFO_IN structure                                   */
  /********************************************************************/
  pstInfoIn->usInfoLevel = usInfoLevel;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 44 usRc = TmtXInfo( (EqfMemory*)htm, pstInfoOut );";
#ifdef TEMPORARY_COMMENTED
  usRc = TmtXInfo( (EqfMemory*)htm, pstInfoOut );
  #endif
//usRc = U( htm,
//          (PXIN)pstInfoIn,               // Pointer to input structure
//          (PXOUT)pstInfoOut,
//          NEW_TM );

  if ( usMsgHandling && usRc )
  {
    T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 45 usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );";
#ifdef TEMPORARY_COMMENTED
    usRc = MemRcHandlingHwnd( usRc, szMemPath, &htm, NULL, hwnd );
    #endif
  } /* endif */
  return usRc;
} /* End of function TmInfo */


