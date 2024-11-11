//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//#include <shlobj.h>               // folder browse function
#include <time.h>                 // C time related functions

#define INCL_EQF_TM               // general Transl. Memory functions
#include "EQF.H"
#include "OTMAPI.H"
//#include "EQFSERNO.H"
#include "EQFEVENT.H"
#include "EQFSETUP.H"
#include "Utility.h"
#include "EQFUTMDI.H"               // MDI Utilities
//#include "EQFFOL.H"
//#include "EQFUTILS.ID" // IDs used by dialog utilities
#include "EQFTA.H"                  // required for TAAdjustWhiteSpace prototype

#include "Property.h"
#include "win_types.h"
#include "FilesystemWrapper.h"
#include "LogWrapper.h"
#include "OSWrapper.h"

// includes for Xalan XSLT

// the Win32 Xerces/Xalan build requires the default structure packing...
//#pragma pack( push )
//#pragma pack(8)
#include <iostream>
#ifdef XALANCODEDOESWORK
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xercesc/util/PlatformUtils.hpp>
  #include <xalanc/XalanTransformer/XalanTransformer.hpp>
#endif
//#pragma pack( pop )


#define NATIONAL_SETTINGS     "intl"

#define MAX_DIALOGS					100		// max. number of modeless dialogs
#define MAX_DISPATCH_MSGS 			30		// number of msgs processed per dispatch
static HWND hwndDlgs[MAX_DIALOGS];			// array for handles of modeless dialogs
static SHORT  sRegisteredDlgs = 0;			// number of registered dialogs
static FILE   *hFFSTFile = NULL;       // UtlAlloc log file

//static FARPROC lpfnOldButtonProc = NULL;// ptr to original button proc
LONG FAR PASCAL EqfHelpButtonProc     (HWND, UINT, WPARAM, LPARAM);

static HANDLE hinstWFWDriver = 0;			// instance handle for Windows network driver

#ifndef ORD_MNETNETWORKENUM
	#define ORD_MNETNETWORKENUM		33
#endif
#ifndef ORD_MNETGETNETINFO
	#define ORD_MNETGETNETINFO		37
#endif
#ifndef MNM_NET_PRIMARY						// this resides in multinet.h from WFW SDK
	#define MNM_NET_PRIMARY			0x0001	// Network is primary network (Windows network)
#endif
#ifndef ORD_MNETSETNEXTTARGET
	#define ORD_MNETSETNEXTTARGET	38
#endif

/*  assume a maximum of five multinet drivers */
#define MAX_MNETS				5
static HANDLE hinstMNet[MAX_MNETS] = { NULL};
static int MNets = 0;           /* our count of networks installed */
static HANDLE primary = 0;              /* the primary network */

typedef WORD (WINAPI *LPMNETSETNEXTTARGET)( HANDLE hNetwork);
typedef WORD (WINAPI *LPMNETNETWORKENUM)( HANDLE FAR *hNetwork);
typedef WORD (WINAPI *LPMNETGETNETINFO)( HANDLE hNetwork, WORD FAR *lpwNetInfo,
        LPSTR lpszButton, WORD FAR *lpcbButton, HANDLE FAR *lphInstance);


/*!
	Function Name:     UtlPlugIn              Plug a new entry into a list

	Description:       Links entries to a list

	Parameters:        1. PPLUG - ptr to new plug to link in list
					2. PPLUG - ptr to backward plug
					3. PPLUG - ptr to foreward plug

	Returncode type:   PPLUG

	Returncodes:       ptr to new plug

	Function flow:     set forward and backward pointers in new plug
					set forward pointer of backward plug to new plug
					set backward pointer of forawrd plug to new plug
*/
PPLUG UtlPlugIn( PPLUG pNew, PPLUG pBackward, PPLUG pForeward )
{
    pNew->Bw = pBackward;
    pNew->Fw = pForeward;
    if( pBackward )
      pBackward->Fw = pNew;
    if ( pForeward )
      pForeward->Bw = pNew;
    return( pNew );
}

/*!
	Function Name:     UtlPlugOut             Remove an entry from a list

	Description:       Unlinks an entry from a list

	Parameters:        1. PPLUG - ptr to plug to get unlinked

	Returncode type:   PPLUG

	Returncodes:       ptr to unlinked plug

	Function flow:     set backward pointer in forward plug to backward pointer
					set forward pointer in backward plug to forward pointer
*/
PPLUG UtlPlugOut( PPLUG pPlug)
{
    if( pPlug->Bw)
      pPlug->Bw->Fw = pPlug->Fw;
    if( pPlug->Fw)
      pPlug->Fw->Bw = pPlug->Bw;
    return( pPlug);
}

/*!
	Function Name:     UtlSplitFnameFromPath  Split a path at filename

	Description:       Locate and return a ptr to filename in full path

	Parameters:        1. PSZ - path

	Returncode type:   PSZ

	Returncodes:       - ptr to filename or NULP if not found

	Note on usage:     Source string is !!! modified
					The separating \ between dir path and filename gets
					replaced by a \0 to make the dir path a usable C string

	Samples:           ps = "D:\EQF\TEST.F00");
					pf = UtlSplitFnameFromPath( ps);
					will set pf to "TEST.F00" and ps to "D:\EQF"
	----------------------------------------------------------------------------+
	Function flow:     Replace last backslash with EOS and return pointer to
						next character
*/
PSZ UtlSplitFnameFromPath( PSZ path)
{
    char *p;
    if( (p = strrchr( path, BACKSLASH)) != NULL )
       *p++ = EOS;
    return( p);
}

/*!
	Function Name:     UtlGetFnameFromPath    Get the position of the file name

	Description:       Locate and return a ptr to filename in full path

	Parameters:        1. PSZ - path

	Returncode type:   PSZ

	Returncodes:       - ptr to filename or NULP if not found

	Note on usage:     Source string is not modified

	Samples:           pf = UtlGetFnameFromPath( "D:\EQF\TEST.F00");
					will set pf to "TEST.F00"

	Function flow:     Return pointer to position of last backslash plus one
*/
char* UtlGetFnameFromPath( const char*  path)
{
  const char*  pszFileName = NULL;                   // ptr to begin of filename

  if( (pszFileName = strrchr( path, BACKSLASH)) != NULL )
    pszFileName++;
  else
    pszFileName = path;
  return( (char*)pszFileName );
}

//+----------------------------------------------------------------------------+
//External function
//+----------------------------------------------------------------------------+
//Function Name:     UtlCompareDate
//+----------------------------------------------------------------------------+
//Description:
//
//+----------------------------------------------------------------------------+
//Function Call:     SHORT UtlCompareDate *pfDateFirst,
//                                        *pfDateSecond )
//+----------------------------------------------------------------------------+
//Parameters:        - *pfDateFirst  first date to be compared
//                   - *pfDateSecond second date to be compared
//+----------------------------------------------------------------------------+
//Return Values:     LONG returns difference between *pfDateFirst and
//                        *pfDateSecond  ( *pfDateFirst - *pfDateSecond)
//+----------------------------------------------------------------------------+
//Returncode type:   LONG
//+----------------------------------------------------------------------------+
//Returncodes:       return > 0  First newer Second
//                   return = 0  First equal Second
//                   return < 0  First older Second
//+----------------------------------------------------------------------------+
//Function flow:     generate LONG from *pfDateFirst
//                   generate LONG from *pfDateSecond
//                   get difference
//+----------------------------------------------------------------------------+
LONG UtlCompareDate( FDATE *pfDateFirst, FDATE *pfDateSecond )
{
   LONG   sCompare;                             // return code
   LONG   uDayFirst;
   LONG   uDaySecond;

   uDayFirst = (pfDateFirst->year * 400) +
               (pfDateFirst->month * 31) +
               pfDateFirst->day;


   uDaySecond = (pfDateSecond->year * 400) +
                (pfDateSecond->month * 31) +
                pfDateSecond->day;

   sCompare = uDayFirst - uDaySecond;

   return( sCompare );
} /* endof UtlCompareDate */

//+----------------------------------------------------------------------------+
//External function
//+----------------------------------------------------------------------------+
//Function Name:     UtlCompareTime
//+----------------------------------------------------------------------------+
//Description:
//
//+----------------------------------------------------------------------------+
//Function Call:     SHORT UtlCompareTime *pfTimeFirst,
//                                        *pfTimeSecond )
//+----------------------------------------------------------------------------+
//Parameters:        - *pfTimeFirst  first time to be compared
//                   - *pfTimeSecond second time to be compared
//+----------------------------------------------------------------------------+
//Return Values:     LONG returns difference between *pfTimeFirst and
//                        *pfTimeSecond  ( *pfTimeFirst - *pfTimeSecond)
//+----------------------------------------------------------------------------+
//Returncode type:   LONG
//+----------------------------------------------------------------------------+
//Returncodes:       return > 0  First newer Second
//                   return = 0  First equal Second
//                   return < 0  First older Second
//+----------------------------------------------------------------------------+
//Function flow:     generate LONG from *pfTimeFirst
//                   generate LONG from *pfTimeSecond
//                   get difference
//+----------------------------------------------------------------------------+
LONG UtlCompareTime( FTIME *pfTimeFirst, FTIME *pfTimeSecond )
{
   LONG   sCompare;                             // return code
   LONG   uTimeFirst;
   LONG   uTimeSecond;

   uTimeFirst = (pfTimeFirst->hours * 3600) +
                (pfTimeFirst->minutes * 60) +
                pfTimeFirst->twosecs;

   uTimeSecond = (pfTimeSecond->hours * 3600) +
                 (pfTimeSecond->minutes * 60) +
                 pfTimeSecond->twosecs;
   sCompare =  uTimeFirst - uTimeSecond ;
   return( sCompare );
} /* endof UtlCompareTime */



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlDispatch            Dispatch a series of messages
//+----------------------------------------------------------------------------+
// Description:       Peek/Dispatch messages until message queue is empty or
//                    WM_QUIT is encountered or MAX_DISPATCH_MSGS messages
//                    have been dispatched.
//+----------------------------------------------------------------------------+
// Function Call:     UtlDispatch()
//+----------------------------------------------------------------------------+
// Parameters:        none
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
// Function flow:     while messages to process and
//                          message count has not been exceeded
//                      if message is WM_QUIT then
//                        return
//                      endif
//                      get message
//                      dispatch message
//                      decrement dispatch message count
//                    endwhile
//+----------------------------------------------------------------------------+
VOID UtlDispatch( VOID )
{
  LOG_UNIMPLEMENTED_FUNCTION;
}


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlTime           Return current time as ULONG value
//+----------------------------------------------------------------------------+
// Description:       Gets the current time using the C library function time
//                    and does any necessar corrections.
//+----------------------------------------------------------------------------+
// Function Call:     ULONG  UtlTime( PULONG pulTime );
//+----------------------------------------------------------------------------+
// Parameters:        pulTime points to buffer for the time value and may be
//                    NULL
//+----------------------------------------------------------------------------+
// Returncode type:   ULONG
//+----------------------------------------------------------------------------+
// Returncodes:       current time as ULONG
//+----------------------------------------------------------------------------+
LONG UtlTime( PLONG plTime )
{
  LONG            lTime;             // buffer for current time

  time( (time_t*) &lTime );
  lTime -= 10800L;                    // correction: - 3 hours
  if ( plTime != NULL )
  {
    *plTime = lTime;
  } /* endif */
  return( lTime );
} /* end of function UtlTime */


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function Name:     UtlCompFDates     Compares the update times of two files
//+----------------------------------------------------------------------------+
// Description:       Compares the file update times of two files
//                    and stores the result of the compare in the supplied
//                    variable
//+----------------------------------------------------------------------------+
// Function Call:     USHORT UtlComFDates( PSZ pszFile1, PSZ pszFile2,
//                                         PSHORT psResult, BOOL fMsg );
//+----------------------------------------------------------------------------+
// Parameters:        PSZ pszFile1   fully qualified path of first file
//                    PSZ pszFile2   fully qualified path of second file
//                    PSHORT psResult address of result buffer
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0     function completed successfully
//                    other DOS error return codes
//+----------------------------------------------------------------------------+
USHORT UtlCompFDates
(
  PSZ    pszFile1,                     // fully qualified path first file
  PSZ    pszFile2,                     // fully qualified path of second file
  PSHORT psResult,                     // address of result buffer
  BOOL   fMsg                          // error handling flag
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  FILESTATUS  stStatus1;               // File status information of 1st file
  FILESTATUS  stStatus2;               // File status information of 2nd file
  LONG        lResult = 0L;            // buffer for result value

  /********************************************************************/
  /* Get date and time of files                                       */
  /********************************************************************/
  usRC = UtlQPathInfo( pszFile1, 1, (PBYTE)&stStatus1, sizeof(stStatus1),
                       0L, fMsg );
  if ( usRC == NO_ERROR )
  {
    usRC = UtlQPathInfo( pszFile2, 1, (PBYTE)&stStatus2, sizeof(stStatus2),
                         0L, fMsg );
  } /* endif */

  /********************************************************************/
  /* Compare file dates                                               */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    lResult = UtlCompareDate( &(stStatus1.fdateLastWrite),
                              &(stStatus2.fdateLastWrite) );
    if ( lResult == 0L )
    {
      lResult = UtlCompareTime( &(stStatus1.ftimeLastWrite),
                                &(stStatus2.ftimeLastWrite) );
    } /* endif */
    *psResult = (SHORT)lResult;
  } /* endif */

  return usRC;
} /* end of function UtlCompFDates */


// function searching first character causing a conversion error and showing an
// approbriate error message
const int MB_ERR_INVALID_CHARS = 1234;//just to compile
USHORT UtlFindAndShowConversionError
(
  PSZ    pszBuffer,
  ULONG  ulLen,
  ULONG  ulCP
)
{
  CHAR_W chBufOut[10];
  USHORT usRC = NO_ERROR;

  ULONG ulOutPut;

  // always use 932 when 943 is specified
  if ( ulCP == 943 ) ulCP = 932;

  while ( !usRC && ulLen )
  {
    ulOutPut = MultiByteToWideChar( ulCP, MB_ERR_INVALID_CHARS, pszBuffer, 1, chBufOut, 1 );

    // try conversion together with next byte, might be a DBCS character
    if ( !ulOutPut && ulLen )
    {
      ulOutPut = MultiByteToWideChar( ulCP, MB_ERR_INVALID_CHARS, pszBuffer, 2, chBufOut, 2 );
      if ( ulOutPut )
      {
        pszBuffer++;
        ulLen--;
      } /* endif */
    } /* endif */

    if ( !ulOutPut )
    {
      PSZ  pszParms[3];
      CHAR szRC[10], szCP[10], szCharacter[20];
      int iCh = (UCHAR)*pszBuffer;
T5LOG(T5ERROR) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 74 usRC = (USHORT)GetLastError();";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
      usRC = (USHORT)GetLastError();
#endif //TO_BE_REPLACED_WITH_LINUX_CODE

      sprintf( szCharacter, "%d (hex %2.2X)", iCh, iCh );
      sprintf( szCP, "%lu", ulCP );
      sprintf( szRC, "%u", usRC );

      pszParms[0] = szCharacter;
      pszParms[1] = szCP;
      pszParms[2] = szRC;

      UtlError( ERROR_DATA_CONVERSION_EXINFO, MB_CANCEL, 3, pszParms, EQF_ERROR );
    }
    else
    {
      pszBuffer++;
      ulLen--;
    } /* endif */
  } /* endwhile */
  return( usRC );
} /* end of function UtlFindAndShowConversionError */

// function checking a given codepage value by doing a test conversion
BOOL UtlIsValidCP( ULONG ulCP )
{
  // do a test conversion using the supplied codepage
  CHAR_W c_W[10];
	LONG  lBytesLeft = 0L;
	LONG  lRc = 0;
	ULONG ulOutPut = 0;

  ulCP = ADJUSTCP( ulCP );

  ulOutPut = UtlDirectAnsi2UnicodeBuf( "abcdefgh", c_W, 8, ulCP, FALSE, &lRc, &lBytesLeft);

  return( (ulOutPut != 0) && (ulCP != 0) );
} /* end of function UtlIsValidCP */



/*! UtlInitUtils           Initialization of utilities       
	Description:       General utilities initialization routine                 
	Function Call:     BOOL UtlInitUtils( HAB hab );                            
	Parameters:        - hab is the application anchor block handle             
	Prerequesits:      none                                                     
	Returncode type:   BOOL                                                     
	Returncodes:       TRUE (always)                                            
	Note on usage:     This function may only be called once by EQFSTART!       
					Do never call this function from anythere else!          
	Function flow:     query date format from OS2.INI                           
					query date seperator from OS2.INI                        
					query time format from OS2.INI                           
					query time seperator from OS2.INI                        
					query codes for AM and PM from OS2.INI                   
					load or create event list                                
*/
BOOL UtlInitUtils( HAB hab )
{
   bool fOK = TRUE;
   char chTemp[2];
   int iTemp;
   unsigned short  usId = UtlGetTask();
   short i;

   // set version info in registry
   char version[20];
   //--- get profile information --- 
   int retCode = Properties::GetInstance()->get_value_or_default("iDate", iTemp, MDY_DATE_FORMAT);
   UtiVar[usId].usDateFormat = iTemp;

   retCode = Properties::GetInstance()->get_value_or_default("iTime", chTemp, 2, "/");
   UtiVar[usId].usDateFormat = (unsigned short)strlen(chTemp);
   UtiVar[usId].chDateSeperator = chTemp[0];

   retCode = Properties::GetInstance()->get_value_or_default("iTime", iTemp, S_12_HOURS_TIME_FORMAT);
   UtiVar[usId].usTimeFormat = iTemp;
   retCode = Properties::GetInstance()->get_value_or_default("sTime", chTemp, 2, ":");
   UtiVar[usId].chTimeSeperator = chTemp[0];
    
   retCode = Properties::GetInstance()->get_value_or_default("s1159", UtiVar[usId].szTime1159, sizeof(UtiVar[usId].szTime1159), "AM");

   retCode = Properties::GetInstance()->get_value_or_default("s2359", UtiVar[usId].szTime2359, sizeof(UtiVar[usId].szTime2359), "PM");

   /**********************************************************************/
   /* init lower and upper case in table 1.= COUNTRY 2. = CODE PAGE      */
   /* 0 = DEFAULT                                                        */
   /**********************************************************************/
   //UtlLowUpInit( );

   /*******************************************************************/
   /* Clear drive type array                                          */
   /*******************************************************************/
   for ( i = 0 ; i < 26; i++ )
   {
     UtiVar[usId].sDriveType[i] = -1;
   } /* endfor */

   
   fOK = ObjHandlerInitForBatch();
   if ( fOK ) 
    fOK = PropHandlerInitForBatch();

   // Add exit procedure for this DLL
   return( fOK );
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     TAAdjustWhiteSpace                               |
//+----------------------------------------------------------------------------+
//|Description:       Adjust the whitespace of a segment; i.e. ensure          |
//|                   that the target segment has the same leading and or      |
//|                   trailing whitespace as the source segment                |
//   GQ 2004/10/21    Do not touch whitespaces adjusted by the adjust leading
//                    whitespaces part of the function when processing trailing
//                    whitespaces.
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     pszSourceSeg  ptr to source segment data         |
//|                   PSZ     pszTargetSeg  ptr to target segment data         |
//|                   PSZ     *ppszNewTargetSeg ptr to ptr for new target seg. |
//|                                 if the ptr is NULL a new buffer            |
//|                                 is allocated (in the length of the         |
//|                                 new target segment)                        |
//|                                 if the ptr is not NULL it must             |
//|                                 point to a buffer large enough to          |
//|                                 contain a segment (MAX_SEGMENT_SIZE).      |
//|                   BOOL    fLeadingWS process leading whitespace flag       |
//|                   BOOL    fTrailingWS process trailing whitespace flag     |
//|                   PBOOL   pfChanged TRUE if target segment has been        |
//|                                 changed                                    |
//|                                 FALSE if no change is required (in this    |
//|                                 case no new target segment is allocated!)  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL    FALSE  in case of errors (=UtlAlloc failed)      |
//|                           TRUE   everything O.K.                           |
//+----------------------------------------------------------------------------+


#define TAISWHITESPACE( c ) \
    ( (c == LF) || (c == CR) || (c == SPACE) || (c == 0x09) )

static BOOL TACheckForWS
(
	CHAR_W c,
	PSZ_W  pWSList
)
{
	BOOL fIsWS = FALSE;

	fIsWS = TAISWHITESPACE( c ) ;

	if (pWSList && !fIsWS)
	{
		while (*pWSList && !fIsWS)
		{
			if (c == *pWSList)
			{
				fIsWS = TRUE;
		    }
		    else
		    {
				pWSList++;
		    }
	    } /* endwhile */
    } /* endif */


	return(fIsWS);
}


BOOL TAAdjustWhiteSpace
(
  PSZ_W         pszSourceSeg,            // ptr to source segment data
  PSZ_W         pszTargetSeg,            // ptr to target segment
  PSZ_W         *ppszNewTargetSeg,       // ptr to ptr of output buffer
  BOOL        fLeadingWS,              // process leading whitespace flag
  BOOL        fTrailingWS,             // process trailing whitespace flag
  PBOOL       pfChanged,                // TRUE if target has been changed
  PSZ_W       pWSList
)
{
  BOOL        fOK = TRUE;              // function return code
  PSZ_W         pszTargetWhiteSpace=NULL;// points to last whitespace in target
  PSZ_W         pszSourceWhiteSpace=NULL;// points to last whitespace in source
  PSZ_W         pszTempSegW = NULL;
  int         iAdjustLen = 0;          // length of adjusted leading whitespace

  // initialize callers change flag
  *pfChanged = FALSE;

  // allocate buffer for temporary segment
  fOK = UtlAlloc( (PVOID *)&pszTempSegW, 0L, (2L*(LONG)MAX_SEGMENT_SIZE * sizeof(CHAR_W)),
                           ERROR_STORAGE );

  // handle leading whitespace
  if ( fOK && fLeadingWS )
  {
    CHAR_W chFirstNonWSSource;
    CHAR_W chFirstNonWSTarget;
    PSZ_W pszNonWSSource = pszSourceSeg;
    PSZ_W pszNonWSTarget = pszTargetSeg;


    // position to first non-whitespace character in source and target
    while ( TACheckForWS(*pszNonWSSource, pWSList) ) pszNonWSSource++;
    while ( TACheckForWS(*pszNonWSTarget, pWSList) ) pszNonWSTarget++;

    // temporarly terminate segment at found locations
    chFirstNonWSSource = *pszNonWSSource;
    *pszNonWSSource = EOS;
    chFirstNonWSTarget = *pszNonWSTarget;
    *pszNonWSTarget = EOS;

    // compare leading whitespace and adjust if necessary
    if ( UTF16strcmp( pszSourceSeg, pszTargetSeg ) != 0 )
    {
      // copy whitespace of source segment to our temp seg buffer
      UTF16strcpy( pszTempSegW, pszSourceSeg );

      iAdjustLen = UTF16strlenCHAR( pszTempSegW );

      // restore end of leading whitespace
      *pszNonWSSource = chFirstNonWSSource;
      *pszNonWSTarget = chFirstNonWSTarget;

      // copy data of target segment to our temp seg buffer
      UTF16strcat( pszTempSegW, pszNonWSTarget );

      // set caller's changed flag
      *pfChanged = TRUE;
    }
    else
    {
      iAdjustLen = UTF16strlenCHAR( pszTargetSeg );

      // restore end of leading whitespace
      *pszNonWSSource = chFirstNonWSSource;
      *pszNonWSTarget = chFirstNonWSTarget;
    } /* endif */
  } /* endif */

  // if nothing has been changed copy complete target segment to
  // our temp seg buffer
  if ( fOK && !*pfChanged )
  {
    UTF16strcpy( pszTempSegW, pszTargetSeg );
  } /* endif */

  // locate trailing whitespace in target segment
  if ( fOK && fTrailingWS )
  {
    int    iLen;

    iLen = UTF16strlenCHAR(pszTempSegW);
    if ( iLen > iAdjustLen )
    {
      pszTargetWhiteSpace = pszTempSegW + iLen - 1;
      while ( (iLen > iAdjustLen) && TACheckForWS(*pszTargetWhiteSpace, pWSList) )
      {
        pszTargetWhiteSpace--;
        iLen--;
      } /* endwhile */
      pszTargetWhiteSpace++;             // point to first byte of whitespace
    } /* endif */
  } /* endif */

  // locate trailing whitespace in source segment
  if ( fOK && fTrailingWS )
  {
    int      iLen;

    iLen = UTF16strlenCHAR(pszSourceSeg);
    if ( iLen > iAdjustLen )
    {
      pszSourceWhiteSpace = pszSourceSeg + iLen - 1;
      while ( (iLen > iAdjustLen) && TACheckForWS(*pszSourceWhiteSpace, pWSList) )
      {
        pszSourceWhiteSpace--;
        iLen--;
      } /* endwhile */
      pszSourceWhiteSpace++;             // point to first byte of whitespace
    } /* endif */
  } /* endif */

  // setup new target if trailing whitespace differs
  if ( fOK && fTrailingWS && pszSourceWhiteSpace && pszTargetWhiteSpace &&
       UTF16strcmp( pszSourceWhiteSpace, pszTargetWhiteSpace ) != 0 )
  {
    // build new target segment
    *pszTargetWhiteSpace = EOS;
    UTF16strcat( pszTempSegW, pszSourceWhiteSpace );

    // set caller's changed flag
    *pfChanged = TRUE;
  } /* endif */

  // return modified temp segment to caller
  if ( fOK && *pfChanged )
  {
    LONG lNewLen = UTF16strlenCHAR(pszTempSegW) + 1;

    // if changed segment exceeds maximum segment size...
    if ( lNewLen > MAX_SEGMENT_SIZE )
    {
      // .. ignore the changes
      *pfChanged = FALSE;
    }
    else
    {
      if ( *ppszNewTargetSeg == NULL )
      {
        LONG lNewLen = UTF16strlenCHAR(pszTempSegW) + 1;
        // GQ: allocate segment at least large enough for the :NONE. tag!
        fOK = UtlAlloc( (PVOID *)ppszNewTargetSeg, 0L,
                        get_max( lNewLen * sizeof(CHAR_W), (sizeof(EMPTY_TAG)+4)), ERROR_STORAGE );
      } /* endif */

      if ( fOK )
      {
        UTF16strcpy( *ppszNewTargetSeg, pszTempSegW );
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pszTempSegW ) UtlAlloc( (PVOID *)&pszTempSegW, 0L, 0L, NOMSG );

  return( fOK );
} /* end of function TAAdjustWhiteSpace */

