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
TmGetW(HTM            htm,             //(in)  TM handle
       PSZ            szMemPath,       //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W  pstGetIn,        //(in)  pointer to get input structure
       PTMX_GET_OUT_W pstGetOut,       //(out) pointer to get output structure
       USHORT         usMsgHandling );  //(in)  message handling parameter

USHORT C_TmInfoHwnd( HTM, PSZ, USHORT, PTMX_INFO_IN, PTMX_INFO_OUT, USHORT, HWND );

USHORT
NTMGetThresholdFromProperties( PSZ,
                               PUSHORT,
                               USHORT );



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
// Function name:     TmOpen
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    TmOpen( PSZ        szMemFullPath,
//                            HTM        *phtm,
//                            USHORT     usAccess,
//                            USHORT     usLocation,
//                            USHORT     usMsgHandling )
//------------------------------------------------------------------------------
// Description:       TM interface function to open a TM
//------------------------------------------------------------------------------
// Parameters:        szMemFullPath  - (in)  full TM name x:\eqf\mem\mem.tmd
//                    *phtm          - (out) TM handle
//                    usAccess       - (in)  access mode: NON_EXCLUSIVE
//                                                        EXCLUSIVE
//                                                        FOR_ORGANIZE
//                    usLocation     - (in)  location:    TM_LOCAL
//                                                        TM_REMOTE
//                                                        TM_LOCAL_REMOTE
//                    usMsgHandling  - (in)  message handling parameter
//                                           TRUE:  display error message
//                                           FALSE: display no error message
//------------------------------------------------------------------------------

USHORT
TmOpen( PSZ        szMemFullPath,      //(in)  full TM name x:\eqf\mem\mem.tmd
        HTM        *phtm,              //(out) TM handle
        USHORT     usAccess,           //(in)  access mode: NON_EXCLUSIVE
                                       //                   EXCLUSIVE
                                       //                   FOR_ORGANIZE
        USHORT     usLocation,         //(in)  location:    TM_LOCAL
                                       //                   TM_REMOTE
                                       //                   TM_LOCAL_REMOTE
        USHORT     usMsgHandling,      //(in)  message handling parameter
                                       //      TRUE:  display error message
                                       //      FALSE: display no error message
        HWND       hwnd )              //(in)  handle for error messages
{
  USHORT            usRc;              //function return code
  BOOL              fOk;               //process flag
  PTMX_OPEN_IN      pstOpenIn  = NULL; //open input structure
  PTMX_OPEN_OUT     pstOpenOut = NULL; //open output structure
  USHORT            usUserPriviliges;  //returned from UtlGetLANUserID
  PSZ               pszTemp;           //temp ptr for UtlGetFnameFromPath

  DEBUGEVENT( TMOPEN_LOC, FUNCENTRY_EVENT, 0 );

  /********************************************************************/
  /* initialize function return code                                  */
  /********************************************************************/
  usRc = ERROR_NOT_ENOUGH_MEMORY;

  /********************************************************************/
  /* initialize TM handle                                             */
  /********************************************************************/
  *phtm = NULLHANDLE;

  /********************************************************************/
  /* allocate storage for TMX_OPEN_IN and TMX_OPEN_OUT                */
  /********************************************************************/
  fOk = UtlAlloc( (PVOID *) &pstOpenIn, 0L,
                  (LONG)( sizeof( TMX_OPEN_IN ) +
                          sizeof( TMX_OPEN_OUT ) ),
                  FALSE );
  if ( fOk )
  {
    /******************************************************************/
    /* set usRc to NO_ERROR                                           */
    /******************************************************************/
    usRc = NO_ERROR;

    /******************************************************************/
    /* assign memory to pointer pstOpenOut                           */
    /******************************************************************/
    pstOpenOut = (PTMX_OPEN_OUT)(pstOpenIn + 1);
    
    pstOpenIn->stTmOpen.szServer[0] = EOS;
    if ( usLocation != TM_LOCAL )
    {
      T5LOG(T5FATAL) << "NOT LOCAL TM IS NOT SUPPORTED";
      fOk = false;
    }

    if ( fOk )
    {
      /****************************************************************/
      /* fill the TMX_OPEN_IN structure                               */
      /* stPrefixIn.usLengthInput                                     */
      /* stPrefixIn.usTmCommand                                       */
      /* stTmOpen.szDataName                                          */
      /* stTmOpen.szIndexName                                         */
      /* stTmOpen.szServer                                            */
      /* stTmOpen.szUserID                                            */
      /* stTmOpen.usAccess                                            */
      /* stTmOpen.usThreshold                                         */
      /****************************************************************/
      pstOpenIn->stPrefixIn.usLengthInput = sizeof( TMX_OPEN_IN );
      pstOpenIn->stPrefixIn.usTmCommand = TMC_OPEN;
      strcpy( pstOpenIn->stTmOpen.szDataName, szMemFullPath );
      Utlstrccpy( pstOpenIn->stTmOpen.szIndexName, szMemFullPath, DOT );

      
      strcpy(pstOpenIn->stTmOpen.szIndexName,pstOpenIn->stTmOpen.szDataName);
      char * temp = strrchr(pstOpenIn->stTmOpen.szIndexName,'.');
      strcpy(temp, ".TMI");
    
      temp = strrchr(pstOpenIn->stTmOpen.szDataName,'.');
      strcpy(temp, ".TMD");
      
      pstOpenIn->stTmOpen.szServer[0] = EOS;
      pstOpenIn->stTmOpen.usAccess = usAccess;

      /****************************************************************/
      /* if a source TM should be opened during folder import and it  */
      /* is the source TM which contains the IMPORT path it is not    */
      /* needed to get the threshold from the properties because      */
      /* for this TM only TmExtract calls are used.                   */
      /* For this the threshold is not needed.                        */
      /****************************************************************/
      if ( strstr( szMemFullPath, IMPORTDIR ) == NULL )
      {
        /**************************************************************/
        /* IMPORTPATH not found                                       */
        /* call function to get threshold valur from the TM properties  */
        /****************************************************************/
        if (false && (usRc = NTMGetThresholdFromProperties
                       ( szMemFullPath,
                        &pstOpenIn->stTmOpen.usThreshold,
                        usMsgHandling)) != NO_ERROR )
        {
          /**************************************************************/
          /* error from NTMGetThresholdFromProperties                   */
          /* stop further processing                                    */
          /* set usMsgHandling to FALSE because in case of an error     */
          /* the message is displayed by NTMGetThresholdFromProperties  */
          /**************************************************************/
          fOk           = FALSE;
          usMsgHandling = FALSE;
          T5LOG(T5ERROR) << "Error in :: NTMGetThresholdFromProperties fails, memName = " <<  szMemFullPath << ", usRC = " << usRc;
          DEBUGEVENT( TMOPEN_LOC, STATE_EVENT, 1 );
        } /* endif */
      } /* endif */

      if ( fOk )
      {
        /**************************************************************/
        /* call U code to pass TM command to server or handle it local*/
        /**************************************************************/
        usRc = TmtXOpen ( pstOpenIn, pstOpenOut );

        /**************************************************************/
        /* return pointer to TM CLB as handle                         */
        /* pstOpenOut->pstTmClb is a NULL pointer in error case       */
        /**************************************************************/
        *phtm = pstOpenOut->pstTmClb;
        switch ( usRc )
        {
          //-------------------------------------------------------------------
          case NO_ERROR:
            break;
          //-------------------------------------------------------------------
          case BTREE_CORRUPTED:
          case VERSION_MISMATCH :
            break;
          //-------------------------------------------------------------------
          default:
            /**********************************************************/
            /* set TM handle to NULL means that the TM is closed      */
            /* and stop further processing                            */
            /**********************************************************/
            *phtm = NULLHANDLE;
             fOk = FALSE;
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */

    /******************************************************************/
    /* if memory for TMX_OPEN_IN and TMX_OPEN_OUT was allocated       */
    /* free the memory                                                */
    /******************************************************************/
    if ( pstOpenIn )
    {
      UtlAlloc( (PVOID *) &pstOpenIn, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* if an error occured and message handling flag is set             */
  /* call MemRcHandling to display error message                      */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    /******************************************************************/
    /* display error message                                          */
    /******************************************************************/
    DEBUGEVENT( TMOPEN_LOC, STATE_EVENT, 2 );
    DEBUGEVENT( TMOPEN_LOC, ERROR_EVENT, usRc );
    usRc = MemRcHandlingHwnd( usRc, szMemFullPath, phtm, NULL, hwnd );
    switch ( usRc )
    {
        case BTREE_CORRUPTED:
        case VERSION_MISMATCH :
          if ( *phtm )
          {
             TmClose( *phtm, szMemFullPath, FALSE, 0 );
          } /* endif */
          *phtm = NULLHANDLE;
          break;
      default:
        break;
    } /* endswitch */
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMOPEN_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function TmOpen */

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
  usRc = ERROR_NOT_ENOUGH_MEMORY;

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
      /* fill the TMX_CLOSE_IN structure                              */
      /* stPrefixIn.usLengthInput                                     */
      /* stPrefixIn.usTmCommand                                       */
      /* stTmClb                                                      */
      /****************************************************************/
      pstCloseIn->stPrefixIn.usLengthInput = sizeof( TMX_CLOSE_IN );
      pstCloseIn->stPrefixIn.usTmCommand   = TMC_CLOSE;

      /****************************************************************/
      /* call U code to pass TM command to server or handle it local  */
      /****************************************************************/
      usRc = TmtXClose ( (PTMX_CLB)htm, pstCloseIn, pstCloseOut );

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
    usRc = NO_ERROR;
  } /* endif */

  if ( usRc != NO_ERROR )
  {
    ERREVENT( TMCLOSE_LOC, ERROR_EVENT, usRc );
  } /* endif */

  return usRc;
} /* end of function TmClose */



USHORT
TmGetW(HTM            htm,             //(in)  TM handle
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
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /* the TMX_GET_IN structure must not be filled it is provided       */
  /* by the caller                                                    */
  /********************************************************************/
  pstGetIn->stPrefixIn.usLengthInput = sizeof( TMX_GET_IN_W );
  pstGetIn->stPrefixIn.usTmCommand   = TMC_GET;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  PTMX_CLB ptmx = (PTMX_CLB)htm;
  usRc = TmtXGet( ptmx, pstGetIn, pstGetOut );

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

  /********************************************************************/
  /* if an error occured call MemRcHandling in dependency of          */
  /* the message flag to display error message                        */
  /********************************************************************/
  if ( usMsgHandling && usRc )
  {
    usRc = MemRcHandling( usRc, szMemPath, &htm, NULL );
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
  /* stPrefixIn.usLengthInput                                         */
  /* stPrefixIn.usTmCommand                                           */
  /********************************************************************/
  pstInfoIn->stPrefixIn.usLengthInput = sizeof( TMX_INFO_IN );
  pstInfoIn->stPrefixIn.usTmCommand   = TMC_INFO;
  pstInfoIn->usInfoLevel = usInfoLevel;

  /********************************************************************/
  /* call U code to pass TM command to server or handle it local      */
  /********************************************************************/
  T5LOG(T5ERROR) << ":: TEMPORARY_COMMENTED temcom_id = 44 usRc = TmtXInfo( (PTMX_CLB)htm, pstInfoOut );";
#ifdef TEMPORARY_COMMENTED
  usRc = TmtXInfo( (PTMX_CLB)htm, pstInfoOut );
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



//------------------------------------------------------------------------------
// Function name:     NTMGetThresholdFromProperties
//------------------------------------------------------------------------------
// Function call:     USHORT
//                    NTMGetThresholdFromProperties( PSZ     pszMemFullPath,
//                                                   PUSHORT pusThreshold,
//                                                   USHORT  usMsgHandling )
//------------------------------------------------------------------------------
// Description:       retrieves the threshold from the TM property file
//------------------------------------------------------------------------------
// Parameters:        pszMemFullPath - (in)  full TM name x:\eqf\mem\mem.tmd
//                    pusThreshold   - (out) threshold retrieved from property
//                    usMsgHandling  - (in)  message handling parameter
//                                            TRUE:  display error message
//                                            FALSE: display no error message
//------------------------------------------------------------------------------
USHORT
NTMGetThresholdFromProperties
  ( PSZ       pszMemFullPath,  //(in)  full TM name x:\eqf\mem\mem.tmd
    PUSHORT   pusThreshold,    //(out) threshold retrieved from property
    USHORT    usMsgHandling )  //(in)  message handling parameter
                               //TRUE:  display error message
                               //FALSE: display no error message
{
  CHAR      szSysPath[MAX_EQF_PATH];      //EQF system path X:\EQF
  szSysPath[0] = '\0';
  PSZ       pszTemp;                      //temp ptr for property name
  CHAR      szPropertyName[MAX_FILESPEC]; //property name TMTEST.MEM
  HPROP     hProp;                        //handle of TM properties
  PPROP_NTM pProp;                        //pointer to TM properties
  EQFINFO   ErrorInfo;                    //rc from OpenProperties
  USHORT    usRc;                         //funtion rc

  /********************************************************************/
  /* get the TM name (w/o ext) from the the full TM path and append   */
  /* the TM property extension                                        */
  /********************************************************************/
  pszTemp = UtlGetFnameFromPath( pszMemFullPath);
  Utlstrccpy( szPropertyName, pszTemp, DOT );
  strcat( szPropertyName, GetPropFileExtension(pszMemFullPath));

  /********************************************************************/
  /* get the EQF system path  X:\EQF                                  */
  /* NTMOpenProperties and therefore OpenProperties needs only the    */
  /* system path and the TM property name with ext                    */
  /********************************************************************/
  Properties::GetInstance()->get_value(KEY_MEM_DIR, szSysPath, MAX_EQF_PATH);

  /********************************************************************/
  /* open the properties of the TM                                    */
  /********************************************************************/
  usRc = NTMOpenProperties( &hProp,
                            (PVOID *)&pProp,
                            szPropertyName,
                            szSysPath,
                            PROP_ACCESS_READ,
                            usMsgHandling );

  if ( usRc == NO_ERROR || usRc == ERROR_OLD_PROPERTY_FILE )
  {
    /******************************************************************/
    /* if no error, return the threshold from the TM properties       */
    /******************************************************************/
    *pusThreshold = pProp->usThreshold;
    
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo);
    
  } /* endif */

  return usRc;
} /* end of function NTMGetThresholdFromProperties */



VOID  TMX_PUT_IN_ASCII2Unicode( PTMX_PUT_IN pstPutIn, PTMX_PUT_IN_W pstPutInW, ULONG cp )
{
  PTMX_PUT   pstTmPut = &pstPutIn->stTmPut;
  PTMX_PUT_W pstTmPutW= &pstPutInW->stTmPut;

  ASCII2Unicode( pstTmPut->szSource, pstTmPutW->szSource, cp );
  ASCII2Unicode( pstTmPut->szTarget, pstTmPutW->szTarget, cp );

  strcpy( pstTmPutW->szSourceLanguage, pstTmPut->szSourceLanguage );
  strcpy( pstTmPutW->szTargetLanguage, pstTmPut->szTargetLanguage );
  strcpy( pstTmPutW->szAuthorName, pstTmPut->szAuthorName );
  pstTmPutW->usTranslationFlag = pstTmPut->usTranslationFlag;
  strcpy( pstTmPutW->szFileName, pstTmPut->szFileName );
  strcpy( pstTmPutW->szLongName, pstTmPut->szLongName );
  pstTmPutW->ulSourceSegmentId = pstTmPut->usSourceSegmentId;
  strcpy( pstTmPutW->szTagTable, pstTmPut->szTagTable );
  pstTmPutW->lTime = pstTmPut->lTime;

  memcpy( &pstPutInW->stPrefixIn, &pstPutIn->stPrefixIn, sizeof( TMX_PREFIX_IN ));
}


VOID  TMX_PUT_OUT_ASCII2Unicode( PTMX_PUT_OUT pstPutOut, PTMX_PUT_OUT_W pstPutOutW )
{
  memcpy( &pstPutOutW->stPrefixOut, &pstPutOut->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
}

VOID  TMX_PUT_OUT_Unicode2ASCII( PTMX_PUT_OUT_W pstPutOutW, PTMX_PUT_OUT pstPutOut )
{
  memcpy( &pstPutOut->stPrefixOut, &pstPutOutW->stPrefixOut, sizeof( TMX_PREFIX_OUT ));
}
