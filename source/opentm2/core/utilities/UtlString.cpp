/*!
 UtlString.c - string functions
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include <time.h>
//#include <tchar.h>

#include "EQF.H"                  // General Translation Manager include file
#include "EQFSHAPE.H"
#include "EQFUTCHR.H"
#include "Utility.h"
#include "tm.h"
#include "win_types.h"
#include "LogWrapper.h"
#include "OSWrapper.h"


// parse string using given character
PSZ_W UtlParseCharW (PSZ_W pszX15String, SHORT sStringId, CHAR_W chParse )
{
   PSZ_W   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the string
      while ((*pszTemp != chParse) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or chParse)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != chParse) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of UtlParseCharW */

//------------------------------------------------------------------------------
// Function Name:     Utlstrccpy             strcpy up to a given character     
//------------------------------------------------------------------------------
// Description:       Copies a string from source to target like the C          
//                    function strcpy, but stops if a given character           
//                    is encountered.                                           
//------------------------------------------------------------------------------
// Function Call:     Utlstrccpy( pszTarget, pszSource, chChar )                
//------------------------------------------------------------------------------
// Parameters:        'pszTarget' points to the buffer for the target string    
//                    'pszSource' points to the source string                   
//                    'chChar' is the character which will stop the copy        
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       ptr to target path                                        
PSZ Utlstrccpy( PSZ pszTarget, PSZ pszSource, CHAR chStop )
{
   PSZ pTemp;

   pTemp = pszTarget;
   while ( *pszSource && (*pszSource != chStop) )
   {
      *pszTarget++ = *pszSource++;
   } /* endwhile */
   *pszTarget = NULC;
   return( pTemp );
}

//------------------------------------------------------------------------------
// Function name:     UtlStripBlanks         Remove leading and trailing blanks 
//------------------------------------------------------------------------------
// Function call:     UtlStripBlanks( PSZ pszString );                          
//------------------------------------------------------------------------------
// Description:       Removes leading and trailing blanks from the supplied     
//                    string. The passed string is overwritten with the         
//                    stripped string.                                          
//------------------------------------------------------------------------------
// Input parameter:   PSZ      pszString      string to be procecessed          
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Side effects:      pszString is overwritten with the resulting string        
//------------------------------------------------------------------------------
VOID UtlStripBlanks
(
  PSZ    pszString                     // pointer to inpit/output string
)
{
  PSZ    pszSource;                    // source position pointer
  PSZ    pszTarget;                    // target position pointer
  PSZ    pszLastNonBlank;              // last non-blank position

  /********************************************************************/
  /* Skip leading blanks                                              */
  /********************************************************************/
  pszSource = pszTarget = pszString;
  pszLastNonBlank = pszString - 1;
  while ( *pszSource == BLANK )
  {
    pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Copy remaining characters                                        */
  /********************************************************************/
  while ( *pszSource )
  {
    if ( *pszSource != BLANK )
    {
      pszLastNonBlank = pszTarget;
    } /* endif */
    *pszTarget++ = *pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Truncate string at last non-blank position                       */
  /********************************************************************/
  pszLastNonBlank++;
  *pszLastNonBlank = EOS;

} /* end of function UtlStripBlanks */

//------------------------------------------------------------------------------
// Function name:     UtlLowUpInit                                              
//------------------------------------------------------------------------------
// Function call:     UtlLowUpInit( usCountry, usCodePage );                    
//------------------------------------------------------------------------------
// Description:       This function will build up 2 256 character arrays        
//                    containing the upper and lower character tables           
//------------------------------------------------------------------------------
// Parameters:        USHORT usCountry          country code                    
//                    USHORT usCodePage         code page                       
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Side effects:      if EQFLowUpInit is called with one or both parameters     
//                    set to 0, the currently active country and code page is   
//                    taken.                                                    
//------------------------------------------------------------------------------
VOID
UtlLowUpInit()
{
  USHORT  i;                           // index

  /********************************************************************/
  /* init tables with default                                         */
  /********************************************************************/
  for ( i=0; i<256; i++ )
  {
    chEQFLower[ i ] = chEQFUpper[ i ] = (CHAR) i;
  } /* endfor */
  /********************************************************************/
  /* add a string end character -- just in case we have to use it     */
  /*   with string functions                                          */
  /********************************************************************/
  chEQFLower[ 256 ] = chEQFUpper[ 256 ] = EOS;
  LOG_TEMPORARY_COMMENTED_W_INFO("use the ANSILOW function");
  // use the ANSILOW function
  /*
  //AnsiUpper( (PSZ)&(chEQFUpper[1]) );
 
  EQFLOG_TEMPORARY_COMMENTED_W_INFO ("OEMTOANSI"); //OEMTOANSI ( (PSZ)&(chEQFLower[1]), (PSZ)&(chEQFLower[1]) );
  //AnsiLower( (PSZ)&(chEQFLower[1]) );
  //*/
  return;
} /* end of function UtlLowUpInit */


//------------------------------------------------------------------------------
// Function name:     UtlLower                                                  
//------------------------------------------------------------------------------
// Function call:     UtlLower( pData );                                        
//------------------------------------------------------------------------------
// Description:       this function will lowercase the passed string            
//------------------------------------------------------------------------------
// Parameters:        PSZ  pData          pointer to data                       
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       pointer to the converted string                           
//------------------------------------------------------------------------------
// Prerequesits:      UtlLowUpInit have to be run previously                    
//------------------------------------------------------------------------------
PSZ
UtlLower
(
  PSZ  pData                           // pointer to data
)
{
  T5LOG(T5FATAL) << ":: Called not implemented func";
  PSZ  pTempData = pData;              // get pointer to data
  BYTE c;                              // active byte

  while ( (c = *pTempData) != NULC )
  {
    *pTempData++ = chEQFLower[ c ];
  } /* endwhile */

  return pData;
} /* end of function UtlLower */


PSZ_W
UtlLowerW
(
  PSZ_W  pData                           // pointer to data
)
{
    //CharLowerW(pData);
T5LOG(T5FATAL) << ":: Called not implemented func";
  return pData;
} /* end of function UtlLower */


//------------------------------------------------------------------------------
// Function name:     UtlUpper                                                  
//------------------------------------------------------------------------------
// Function call:     UtlUpper( pData );                                        
//------------------------------------------------------------------------------
// Description:       this function will uppercase the passed string            
//------------------------------------------------------------------------------
// Parameters:        PSZ  pData          pointer to data                       
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       pointer to the converted string                           
//------------------------------------------------------------------------------
// Prerequesits:      UtlLowUpInit have to be run previously                    
//------------------------------------------------------------------------------
PSZ
UtlUpper
(
  PSZ  pData                           // pointer to data
)
{
  T5LOG(T5FATAL) << ":: Called not implemented func";
  PSZ  pTempData = pData;              // get pointer to data
  BYTE c;                              // active byte

  while ( (c = *pTempData) != NULC )
  {
    *pTempData++ = chEQFUpper[ c ];
  } /* endwhile */

  return pData;
} /* end of function UtlUpper */


///////////////////////////////////////////////////////////////////////////////
///  UTF16strcpy   copy an Unicode UTF16 string to another location         ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strcpy( PSZ_W pszTarget, PSZ_W pszSource )
{
  wcscpy(pszTarget, pszSource);
  return( pszTarget );
}



PSZ_W UTF16strccpy( PSZ_W pszTarget, PSZ_W pszSource, CHAR_W chStop )
{
  T5LOG(T5FATAL) << ":: Called not implemented func";
  PSZ_W pusTarget = pszTarget;
  PSZ_W pusSource = pszSource;

  while ( *pusSource  && (*pusSource != chStop) )
  {
    *pusTarget++ = *pusSource++;
  } /* endwhile */
  *pusTarget = 0;
  return( pszTarget );
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strcat   concatenate an Unicode UTF16 string to then end of string///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strcat( PSZ_W pszTarget, PSZ_W pszSource )
{
  int iTargetLen = UTF16strlenCHAR( pszTarget );
  UTF16strcpy( pszTarget + iTargetLen, pszSource );
  return( pszTarget );
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strcmp   compare two Unicode UTF16 strings                        ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strcmp( PSZ_W pszString1, PSZ_W pszString2 )
{
  return wcscmp(pszString1, pszString2);
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strncmp   compare two Unicode UTF16 strings up to usLen chars     ///
///////////////////////////////////////////////////////////////////////////////
int UTF16strncmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen )
{
	LONG lLen = usLen;

	return (UTF16strncmpL(pszString1, pszString2, lLen));
}

int UTF16strncmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen )
{
  int iResult = 0;
  PUSHORT pusString1 = (PUSHORT)pszString1;
  PUSHORT pusString2 = (PUSHORT)pszString2;

  while ( (iResult == 0) && (lLen > 0))
  {
  lLen--;

    if ( *pusString1 == 0)
    {
      if ( *pusString2 == 0 )
      {
        break;                         // strings are identical
      }
      else
      {
        iResult = -1;                  // string1 ends while string2 continues
      } /* endif */
    }
    else if ( *pusString2 == 0 )
    {
      iResult = 1;                     // string2 ends while string1 continues
    }
    else
    {
      iResult =  *pusString1++ - *pusString2++;
    } /* endif */
  } /* endwhile */
  return( iResult);
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strncmp   compare two Unicode UTF16 strings up to usLen chars     ///
///   without respect of upper/lowercase                                    ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strnicmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen )
{
  return( wcsncasecmp( pszString1, pszString2, usLen));
}

int UTF16strnicmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen )
{
  return( wcsncasecmp( pszString1, pszString2, lLen));
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strlen   get the length in bytes of an Unicode UTF16 string       ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strlenCHAR( PSZ_W pszString )
{
  int iLen = wcslen(pszString);
  return( iLen );
}


int UTF16strlenBYTE( PWCHAR pszString )
{
  int iLen = wcslen(pszString) * sizeof(TMWCHAR);  
  return( iLen );
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strrev    reverse a Unicode string                                ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strrev( PSZ_W pszString )
{
  T5LOG(T5FATAL) << ":: Called not implemented func";
  ULONG ulLen = UTF16strlenCHAR( pszString );
  USHORT usPos = 0;
  ULONG  ulMid = ulLen/2;
  CHAR_W ch;

  for ( usPos=0; usPos < ulMid; usPos++ )
  {
  ch = pszString[usPos];
  pszString[usPos] = pszString[ulLen-usPos-1];
  pszString[ulLen-usPos-1] = ch;
  }

  return( pszString);
}

///////////////////////////////////////////////////////////////////////////////
///  Unicode2ASCII  convert unicode string to Ansi                          ///
///////////////////////////////////////////////////////////////////////////////

PSZ Unicode2ASCII( PSZ_W pszUni, PSZ pszASCII, ULONG ulCP )
{
  // it is assumed, that pszASCII is large enough to hold the converted string
  PSZ_W    pTemp = NULL;
  USHORT   usCP = (USHORT) ulCP;
  if ( pszUni && pszASCII )
  {
	if (*pszUni == EOS)
	{
		*pszASCII = EOS;
    }
    else
    {
		if (!usCP)
		{
		  usCP = 1;
		}
		
		WideCharToMultiByte( usCP, 0, pszUni, -1,
								 pszASCII, MAX_SEGMENT_SIZE, NULL, NULL );

    } /* endif */
  }
  else if (pszASCII)
  {
  *pszASCII = EOS;
  }
  return( pszASCII );
}

ULONG Unicode2ASCIIBuf( PSZ_W pszUni, PSZ pszASCII, ULONG ulLen, LONG lBufLen, ULONG ulCP)
{
	LONG lRc = 0;
	ULONG ulOutPut = 0;

	ulOutPut = Unicode2ASCIIBufEx(pszUni, pszASCII, ulLen, lBufLen, ulCP, &lRc);
	return(ulOutPut);
}

// ulLen = # of char-W's in pszUni!
// internal function, used only in this source-file
ULONG Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG ulLen, LONG lBufLen,
                          ULONG ulCP, PLONG plRc)
{
	static CHAR_W szUniTemp[ MAX_SEGMENT_SIZE ];
	ULONG ulOutPut = 0;
	USHORT usCP = 1;
	LONG lRc = 0;

	if ( pszUni && pszASCII )
	{
		PSZ_W pTemp = NULL;
		USHORT usCP = (USHORT) ulCP;

		// back to 864
		*pszASCII = 0;
		if (ulLen)
		{
      ulOutPut = WideCharToMultiByte( usCP, 0, pszUni, ulLen,
                      pszASCII, lBufLen, NULL, NULL );

      if (!ulOutPut )
      {
        //lRc = GetLastError();
      }
		} /* endif ulLen */
	}
	else if (pszASCII)
	{
		*pszASCII = EOS;
	}

	*plRc = lRc;

  return( ulOutPut );
}


///////////////////////////////////////////////////////////////////////////////
///  ASCII2Unicode convert ASCII string to Unicode                          ///
///////////////////////////////////////////////////////////////////////////////

PWCHAR ASCII2Unicode( PSZ pszASCII, PWCHAR pszUni, ULONG  ulCP )
{
  mbstowcs(pszUni, pszASCII, MAX_SEGMENT_SIZE);
  return( pszUni );
}

ULONG ASCII2UnicodeBuf( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP )
{
	LONG  lBytesLeft = 0;
	ULONG ulOutPut = 0;

	ulOutPut = ASCII2UnicodeBufEx( pszASCII, pszUni, ulLen, ulCP, FALSE, NULL, &lBytesLeft );
	// For consistent behaviour with old ASCII2UnicodeBuf function where dwFlags was 0:
	// Within TM : FileRead expects to get EOS at position ulOutPut
    if (lBytesLeft && (ulOutPut < ulLen))
    {
       *(pszUni+ulOutPut) = EOS;
       ulOutPut++;
    }
	return(ulOutPut);
}

// ulLen in number of CHAR_W's which can be in pszUni!
ULONG ASCII2UnicodeBufEx( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP,
                          BOOL fMsg, PLONG plRc, PLONG plBytesLeft )
{
  ULONG  ulOutPut = 0;
  ULONG  ulTempCP = ulCP;
  LONG   lRc = 0;

  if ( pszASCII && pszUni )
  {
    if (!ulTempCP)
    {
       ulTempCP = 1;
    }

    *pszUni = EOS;
    if (ulLen)
    {
T5LOG(T5ERROR) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 75 ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS,   pszASCII, ulLen,  pszUni, ulLen );";

     }
  }
  else if (pszUni)
  {
    *pszUni = 0;
  }
  if (plRc)
  {
     *(plRc) = lRc;
  }
  
  // this function is always called with fMsg==FALSE, so the following code-block
  // is eliminated
  fMsg;
//  if (fMsg && lRc)
//  { CHAR szTemp[10];
//	PSZ pszTemp = szTemp;
//	sprintf(szTemp, "%d", lRc);
//    UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
//  }

  return( ulOutPut );
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strncpy   copy up to ulLen wide characters                        ///
///////////////////////////////////////////////////////////////////////////////
PWCHAR UTF16strncpy(PWCHAR pusTarget, PWCHAR pusSource, LONG lLen)
{
  // usLen is number of CHAR_W's!!
  PSZ_W pszTarget = pusTarget;
  while ( *pusSource != 0 && (lLen > 0))
  {
    *pusTarget++ = *pusSource++;
    lLen--;
  } /* endwhile */
  if ( lLen > 0 )
  {
    *pusTarget = EOS;
  }
  return( pszTarget );

}

///////////////////////////////////////////////////////////////////////////////
///  UTF16stricmp   copy up to usLen wide characters                        ///
///////////////////////////////////////////////////////////////////////////////

int UTF16stricmp(PWCHAR pusTarget, PWCHAR pusSource)
{
  //T5LOG(T5FATAL) << "UTF16stricmp is not implemented";
  //return -1;
  return wcscasecmp( pusTarget, pusSource );
  
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16memset   initializes ulLen wide characters with c                 ///
///////////////////////////////////////////////////////////////////////////////
PWCHAR UTF16memset( PWCHAR pusString, TMWCHAR c, USHORT usNum )
{
  PSZ_W p = pusString;
  while ( usNum -- )
    *pusString++ = c;
  return p;
}
PWCHAR UTF16memsetL( PWCHAR pusString, TMWCHAR c, ULONG ulNum )
{
  PSZ_W p = pusString;
  while ( ulNum -- )
    *pusString++ = c;
  return p;
}


// Convert FE to 06 in Arabic Strings
// Input: CHAR_W *   ptr to string
//        bufferlen  Lengt of string

void BidiConvertFETo06
(
  CHAR_W * lpWideCharStr,
  ULONG Length
) //lpWideCharStr is a Zero terminated buffer.
{
  ULONG i = 0;
  CHAR_W  c;
  for ( i=0; i<Length; i++)
  {
    c = lpWideCharStr[i];

    if ( (c >= 0xFE80) && (c <= 0xFEF3 ) )
    {
        lpWideCharStr[i] =  (CHAR_W)(TabFEto06 [ (c - 0xFE80) ] );

    // Substitute character only if input char has not already been NULL
    // function is used for converting contents of buffers too, and there
    // may be NULL's in the buffers!

       if (lpWideCharStr[i] == 0x0000)
              lpWideCharStr[i] = 0x001A;
    }
  }
}

// Reverse direction (from Unicode to 864)
void BidiConvert06ToFE(LPWSTR lpWideCharStr, int Length) //lpWideCharStr is a Zero terminated buffer.
{
  T5LOG(T5FATAL) << "TEMPORARY_COMMENTED in called function BidiConvert06ToFE, because of basic TMWCHAR data type migration";
  for (int i=0; i<Length; i++)
  {
#ifdef TEMPORARY_COMMENTED
    if ( (lpWideCharStr[i] >= 0x0621) && (lpWideCharStr[i] <= 0x064A ) )
        lpWideCharStr[i] = (CHAR_W)(Tab06ToFE [ (lpWideCharStr[i] - 0x0621) ] );
    #endif

  }
}

//ulLen in number of CHAR_W's which are in pszUni!
ULONG UtlDirectUnicode2AnsiBufInternal( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                             ULONG ulAnsiCP, PLONG plRc, PSZ pszTemp )
{
  static CHAR_W szUniTemp[ MAX_SEGMENT_SIZE ];
  ULONG ulOutPut = 0;
  USHORT usAnsiCP = 1;
  LONG   lRc = 0;

  if (plRc)
  {
	  *plRc = 0;
  }

  if ( pszUni && pszAnsi )
  {
	// do special handling for Arabic to get rid of shaping to allow for conversion
	// back to 864/1256 ?? nec or not??

	*pszAnsi = EOS;
	if (ulLen)
	{
		ulOutPut = WideCharToMultiByte( usAnsiCP, 0, (LPWSTR)pszUni, ulLen,
											pszAnsi, lBufLen, NULL, NULL );
		if (plRc && !ulOutPut)
		{
T5LOG(T5ERROR) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 76 lRc = GetLastError();";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
			lRc = GetLastError();
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
		  *(plRc) = lRc;
		}
		if (pszTemp && lRc)
		{
		   sprintf(pszTemp, "%d", lRc);
// UtlError-call moved to UtlString2.c
//		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );

		}
    }
  }
  else if (pszAnsi)
  {
    *pszAnsi = EOS;
  }
  return( ulOutPut );     // # of bytes written
}

// ulLen in number of CHAR_W's which can be in pszUni!
// rc ulOutPut2 in # of char-w's
ULONG UtlDirectAnsi2UnicodeBufInternal( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen,
                              ULONG ulAnsiCP, PLONG plRc, PLONG plBytesLeft, PSZ pszTemp)
{
  ULONG  ulOutPut = 0;
  ULONG  ulTempCP = 1;
  LONG   lRc = 0;

  if ( pszAnsi && pszUni )
  {
    T5LOG( T5WARNING) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 77 ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS, pszAnsi, ulLen,  pszUni, ulLen );";
  }
  else if (pszUni)
  {
    *pszUni = 0;
  }
  return( ulOutPut );   // # of bytes written
}

///////////////////////////////////////////////////////////////////////////////
///  UtlDirectUnicode2Ansi  convert unicode string to Ansi                          ///
///////////////////////////////////////////////////////////////////////////////

PSZ UtlDirectUnicode2Ansi( PSZ_W pszUni, PSZ pszAnsi, ULONG ulAnsiCP )
{
  // it is assumed, that pszAnsi is large enough to hold the converted string
  USHORT   usCP = 1;
  int      iRC = 0;
  if ( pszUni && pszAnsi )
  {
	if (*pszUni == EOS)
	{
		*pszAnsi = EOS;
  }
  else
  {
		iRC = WideCharToMultiByte( usCP, 0, (LPWSTR)pszUni, -1,
								 pszAnsi, MAX_SEGMENT_SIZE, NULL, NULL );
    
  LOG_TEMPORARY_COMMENTED_W_INFO("// possible: ERROR_INSUFFICIENT_BUFFER /ERROR_INVALID_FLAGS / ERROR_INVALID_PARAMETER");
#ifdef TEMPORARY_COMMENTED
		if ( iRC == 0) iRC = GetLastError();

		// possible: ERROR_INSUFFICIENT_BUFFER /ERROR_INVALID_FLAGS / ERROR_INVALID_PARAMETER
		if ( iRC == ERROR_INVALID_PARAMETER )
		{
		  // codepage not supported by OS, use default code page for conversion
		  MultiByteToWideChar( CP_ACP, 0, pszAnsi, -1, pszUni, MAX_SEGMENT_SIZE );
		} /* endif */
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
    } /* endif */
  }
  else if (pszAnsi)
  {
    *pszAnsi = EOS;
  }
  return( pszAnsi );
}


//+----------------------------------------------------------------------------+
// External function                                                            
//+----------------------------------------------------------------------------+
// Function name:     UtlQueryCharTable   Return ptr to character table         
//+----------------------------------------------------------------------------+
// Description:       Returns a pointer to the requested character conversion   
//                    or identification table.                                  
//+----------------------------------------------------------------------------+
// Function call:     usRC = UtlQueryCharTable( CHARTABLEID TableID,            
//                                              PUCHAR      *ppTable );         
//+----------------------------------------------------------------------------+
// Input parameter:   CHARTABLEID  TableID    ID of requested table             
//+----------------------------------------------------------------------------+
// Output parameter:  PUCHAR       *ppTable   address of caller's table pointer 
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT                                                    
//+----------------------------------------------------------------------------+
// Returncodes:       NO_ERROR             function completed successfully      
//                    ERROR_INVALID_DATA   no table with the given ID available 
//+----------------------------------------------------------------------------+
// Prerequesits:      none                                                      
//+----------------------------------------------------------------------------+
// Side effects:      none                                                      
//+----------------------------------------------------------------------------+
// Function flow:     set caller's table pointer to requested table or          
//                      set return code for unknown tables                      
//                    return function return code                               
//+----------------------------------------------------------------------------+
USHORT UtlQueryCharTable
(
  CHARTABLEID TableID,                 // ID of requested table
  PUCHAR      *ppTable                 // address of caller's table pointer
)
{
  return( UtlQueryCharTableEx( TableID, ppTable, 0 ) );
} /* end of function UtlQueryCharTable */

USHORT UtlQueryCharTableLang
(
  CHARTABLEID TableID,
  PUCHAR      *ppTable,
  PSZ         pszLanguage
)
{
  return(UtlQueryCharTableEx(TableID, ppTable, (USHORT)1));
} /* end of function UtlQueryCharTableLang */


USHORT UtlQueryCharTableEx
(
  CHARTABLEID TableID,                 // ID of requested table
  PUCHAR      *ppTable,                // address of caller's table pointer
  USHORT      usCodePage               // code page to be used or 0
)
{
  USHORT      usRC = NO_ERROR;

  switch ( TableID )
  {
    case IS_TEXT_TABLE :
      *ppTable = chIsText;
      break;

    case ANSI_TO_ASCII_TABLE :
      {
        DOSVALUE usCP;
        PUCHAR   pTable;

        if ( usCodePage != 0 )
        {
          usCP = usCodePage;
        }
        else
        {
           TCHAR        cp [6];
           //GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
           //usCP = (USHORT)_ttol (cp);
        } /* endif */
        switch ( usCP )
        {
          case 852  : pTable = chAnsiToPC852; break;
          case 855  : pTable = chAnsiToPC855; break;
          case 866  : pTable = chAnsiToPC866; break;
          case 869  : pTable = chAnsiToPC869; break;
          case 915  :
          case 28595: pTable = chAnsiToPC915; break;
          case 857  : pTable = chAnsiToPC857; break;
          case 862  : pTable = chAnsiToPC862; break;
          case 813  : pTable = chAnsiToPC813; break;
          case 737  :
T5LOG( T5DEBUG) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 78 if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869)) { // fix for sev1 Greek: Win NT problem (01/09/23)";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
            if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869))
            { // fix for sev1 Greek: Win NT problem (01/09/23)
                  usCP = 869;
                  pTable = chAnsiToPC869;
            }
            else
            {
                pTable = chAnsiToPC737;
            } /* endif */
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
            break;
          case 775  : pTable = chAnsiToPC775; break;
          case 864  : pTable = chAnsiToPC864; break;  // Arabic OS/2
          case 720  : pTable = chAnsiToPC864; break;  // Arabic Windows
          case 874  : pTable = NULL;          break; //Thai
          case 932 : pTable  = NULL;         break; //Jap
          case 936 : pTable = NULL;          break; //Chin-s
          case 950 : pTable = NULL;          break; // Chin-t
          case 943 : pTable  = NULL;         break; //Jap2
          case 949 : pTable = NULL;          break; //Korean
          case 1251: pTable = NULL;          break; // Belarusian + Ukrainian
          case 28603:                               //ISO 8859-13 is identical to 921
          case 921  : pTable = chAnsiToPC921; break; //Lithuanian, Latvian, Estonian
          case 850  : pTable = chAnsiToPC850; break;
          default   :
             pTable = chAnsiToPC850;
             LOG_AND_SET_RC(usRC, T5INFO, ERROR_NOT_READY);
             break;

        } /* endswitch */

        *ppTable = pTable;
      }
      break;

    case ASCII_TO_ANSI_TABLE :
      {
        DOSVALUE usCP;
        PUCHAR   pTable;
        PUCHAR   pInvTable;
        SHORT i;
        if ( usCodePage != 0 )
        {
          usCP = usCodePage;
        }
        else
        {
          TCHAR        cp [6];
          //GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
          //usCP = (USHORT)_ttol (cp);
        } /* endif */
        switch ( usCP )
        {
          case 852  : pTable = chAnsiToPC852;  pInvTable = chPC852ToAnsi; break;
          case 855  : pTable = chAnsiToPC855;  pInvTable = chPC855ToAnsi; break;
          case 866  : pTable = chAnsiToPC866;  pInvTable = chPC866ToAnsi; break;
          case 869  : pTable = chAnsiToPC869;  pInvTable = chPC869ToAnsi; break;
          case 28595:
          case 915  : pTable = chAnsiToPC915;  pInvTable = chPC915ToAnsi; break;
          case 857  : pTable = chAnsiToPC857;  pInvTable = chPC857ToAnsi; break;
          case 862  : pTable = chAnsiToPC862;  pInvTable = chPC862ToAnsi; break;
          case 813  : pTable = chAnsiToPC813;  pInvTable = chPC813ToAnsi; break;
          case 737  :
T5LOG( T5DEBUG) << ":: TO_BE_REPLACED_WITH_LINUX_CODE id = 79 if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869) ) { // fix for sev1 Greek: Win NT problem (01/09/23)";
#ifdef TO_BE_REPLACED_WITH_LINUX_CODE
            if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869) )
            { // fix for sev1 Greek: Win NT problem (01/09/23)
              usCP = 869;
              pTable = chAnsiToPC869;
              pInvTable = chPC869ToAnsi;
            }
            else
            {
              pTable = chAnsiToPC737;
              pInvTable = chPC737ToAnsi;
            } /* endif */
#endif //TO_BE_REPLACED_WITH_LINUX_CODE
            break;
          case 775  : pTable = chAnsiToPC775;  pInvTable = chPC775ToAnsi; break;
          case 864  : pTable = chAnsiToPC864;  pInvTable = chPC864ToAnsi; break;  // Arabic OS/2
          case 720  : pTable = chAnsiToPC864;  pInvTable = chPC864ToAnsi; break;    // Arabic Windows
          case 874  : pTable = NULL;           pInvTable = NULL;          break;  // Thai
          case 932  : pTable = NULL;           pInvTable = NULL;          break;  // Jap
          case 936  : pTable = NULL;           pInvTable = NULL;          break;  // Chin
          case 950  : pTable = NULL;           pInvTable = NULL;          break;  // Chin
          case 949  : pTable = NULL;           pInvTable = NULL;          break;  // Korean
          case 1251 : pTable = NULL;           pInvTable = NULL;          break;  // Belarusian + Ukrainian
          case 28603:
          case 921  : pTable = chAnsiToPC921;  pInvTable = chPC921ToAnsi; break;  // Baltic 921-1257
          case 850  : pTable = chAnsiToPC850;  pInvTable = chPC850ToAnsi; break;
          default   :
             pTable = chAnsiToPC850;
             pInvTable = chPC850ToAnsi;
             LOG_AND_SET_RC(usRC, T5INFO, ERROR_NOT_READY);
             break;
        } /* endswitch */

        // chPC864ToAnsi already filled up -- therefore it need not te be created
        if ( pInvTable && pTable && (pInvTable != &chPC864ToAnsi[0]))
        {
          memset( pInvTable, 0, 256 );
          for ( i = 0; i < 256; i++ )
          {
            pInvTable[pTable[i]] = (CHAR)i;
          } /* endfor */
          // any empty slots should be filled up with blanks
          for (i = 1; i < 256; i++)
          {
                 if (pInvTable[i] == 0)
             pInvTable[i] = ' '; //i;
          }
        } /* endif */
        *ppTable = pInvTable;
      }
      break;

    default :
      LOG_AND_SET_RC(usRC, T5INFO, ERROR_INVALID_DATA);
      break;
  } /* endswitch */

  return( usRC );
} /* end of UtlQueryCharTableEx */


//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function Name:     UtlReplString          Replace a string value             
//+----------------------------------------------------------------------------+
// Description:       Replaces a MAT string value.                              
//                    This procedure is NOT a public procedure!                 
//+----------------------------------------------------------------------------+
// Function Call:     VOID UtlReplString( SHORT sID, PSZ pszString );           
//+----------------------------------------------------------------------------+
// Parameters:        sID is the string ID (QST_...)                            
//                    pszString is the new value for the string                 
//+----------------------------------------------------------------------------+
// Returncode type:   VOID                                                      
//+----------------------------------------------------------------------------+
// Function flow:     if ID is in valid range then                              
//                      free old value in internal buffers                      
//                      store copy of string in internal buffers                
//                    endif                                                     
//+----------------------------------------------------------------------------+
VOID UtlReplString( SHORT sID, PSZ pszString )
{
  PSZ  pszBuffer;                      // buffer for new string value

   if ( (sID > QST_FIRST) && (sID < QST_LAST) )
   {
      if ( UtlAlloc( (PVOID *)&pszBuffer, 0L,
                     (LONG)  get_max( MIN_ALLOC, strlen(pszString)+1 ),
                     ERROR_STORAGE ) )
      {
        USHORT      usTask = UtlGetTask();
        strcpy( pszBuffer, pszString );

        if ( UtiVar[usTask].pszQueryArea[sID] )
        {
           UtlAlloc( (PVOID *)&(UtiVar[usTask].pszQueryArea[sID]), 0L, 0L, NOMSG );
        } /* endif */

        UtiVar[usTask].pszQueryArea[sID] = pszBuffer;
      } /* endif */
   } /* endif */
}

//ulLen in number of CHAR_W's which are in pszUni!
ULONG UtlDirectUnicode2AnsiBuf( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                             ULONG ulAnsiCP, BOOL fMsg, PLONG plRc )
{
	ULONG ulOutPut = 0;
	CHAR szTemp[10];
	PSZ pszTemp = szTemp;

	ulOutPut = UtlDirectUnicode2AnsiBufInternal( pszUni, pszAnsi, ulLen, lBufLen,
												ulAnsiCP, plRc, pszTemp);
	if (fMsg && *plRc)
	{  
		UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
	}
	return( ulOutPut );     // # of bytes written
}

// ulLen in number of CHAR_W's which can be in pszUni!
// rc ulOutPut2 in # of char-w's
ULONG UtlDirectAnsi2UnicodeBuf( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen,
                              ULONG ulAnsiCP, BOOL fMsg, PLONG plRc, PLONG plBytesLeft)
{
	ULONG  ulOutPut = 0;
	CHAR szTemp[10];
	PSZ pszTemp = szTemp;

	ulOutPut = UtlDirectAnsi2UnicodeBufInternal( pszAnsi, pszUni, ulLen,
												ulAnsiCP, plRc, plBytesLeft, 
												pszTemp);

	if (fMsg && *plRc)
	{  
		UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
	}
	return( ulOutPut );   // # of bytes written
}


//+----------------------------------------------------------------------------+
// External function                                                            
//+----------------------------------------------------------------------------+
// Function name:     UtlCompIgnWhiteSpace                                      
//+----------------------------------------------------------------------------+
// Function call:     UtlCompIgnWhiteSpace(PSZ, PZS, USHORT)                    
//+----------------------------------------------------------------------------+
// Description:       this function will return 0 if both strings differ only   
//                    in blanks or linefeeds. ( one blank/LF / CRLF is seen as  
//                    equal to one CRLF / LF / more blanks)                     
//                    BUT if string1 has one blank between two other characters 
//                    and string2 has the two characters without any kind of    
//                    whitespace between, the strings are not equal             
//                    Example:                                                  
//     string1:       <a href=a.htm#Transfer calls">                            
//     string2:       <a href=a.htm#Transfercalls">        NOT EQUAL            
//     string1:       <a href=a.htm#Transfer calls">                            
//     string2:       <a href=a.htm#Transfer    calls">        EQUAL            
//     string1:       This is \r\nOk                                            
//     string2:       This    is \nOk                          EQUAL            
// P016340: string1:  \x0AThis is Ok                                            
//     string2:       This    is \x0AOk                        EQUAL            
//+----------------------------------------------------------------------------+
// Parameters:        PSZ      pString1     string to be compared               
//                    PSZ      pString2     string to be compared               
//                    USHORT   usLen        length of longer string             
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT                                                     
//+----------------------------------------------------------------------------+
// Returncodes:       0               strings are the same                      
//                    number !=0      strings are different                     
//+----------------------------------------------------------------------------+
// Function flow:     _                                                         
//+----------------------------------------------------------------------------+
LONG UtlCompIgnWhiteSpaceW( PSZ_W pD1, PSZ_W pD2, ULONG ulLen , PINT whitespaceDiff)
{
  LONG   lRc = 0;
  ULONG  ulI = 0;
  INT ws1 = 0, ws2 = 0;// to count whitespaces
  CHAR_W c, d;
  BOOL fNullTerminated = FALSE;
  BOOL   ulIncreaseI = FALSE;

  if (ulLen == 0 )
  {
    fNullTerminated = TRUE;
    ULONG ulLenTmp = UTF16strlenCHAR(pD2);
    ulLen    = UTF16strlenCHAR(pD1);
    if (ulLenTmp > ulLen )
    {
      ulLen = ulLenTmp;
    } /* endif */
  } /* endif */

  int wspaceDiff = 0;
  // RJ: skip leading whitespaces in both strings
  if ( ulLen )
  {
    while ( UtlIsWhiteSpaceW( *pD1 ) && (ulI < ulLen) )
    {
      pD1++;
      ulI++;
      ws1++;
    }
    while ( UtlIsWhiteSpaceW( *pD2 ) ) {
      pD2++;
      ws2++;
    }
    wspaceDiff = abs(ws1-ws2);
  }  /* endif */

  while ( ulI++ < ulLen )
  {
    c = *pD1; d = *pD2;
    if ( UtlIsWhiteSpaceW(c) && UtlIsWhiteSpaceW(d) )
    {
      pD1++; pD2++;
      /****************************************************************/
      /* skip any consecutive white spaces                            */
      /****************************************************************/
      ulIncreaseI = FALSE;
      ws1 = ws2 = 0;
      while ( UtlIsWhiteSpaceW( *pD1 ) && (ulI < ulLen) )
      {
        ws1++;
        pD1++;
        ulI++;
        ulIncreaseI = TRUE;
      } /* endwhile */
      while ( UtlIsWhiteSpaceW( *pD2 ) )
      {
        ws2++;
        pD2++;
      } /* endwhile */
      
      wspaceDiff += abs(ws1-ws2);
      // if we reached the end of our string do a sRc setting
      // IV000162: i.e.RTF Tag "\line ": add ulIncreaseI
      // otherwise tags ending both with one blank are recognized as different!
      if (ulIncreaseI && (ulI >= ulLen ))
      {
        lRc = (*pD1 - *pD2);
      }
    }
    else if ( c == d )
    {
      pD1++; pD2++;
    }
    else
    {
      ulI = ulLen;   // stop loop;
      lRc = ( c-d );
    } /* endif */
  } /* endwhile */

  // GQ: check if both strings have been processed completely (but ignore
  // any trailing whitespace characters
  if ( !lRc && fNullTerminated )
  {
    ws1 = ws2 = 0;
    while ( *pD1 && UtlIsWhiteSpaceW( *pD1 ) ) { pD1++; ws1++; }
    while ( *pD2 && UtlIsWhiteSpaceW( *pD2 ) ) { pD2++; ws2++; }
    
    wspaceDiff += abs(ws1 - ws2);
    if ( *pD1 )
    {
      lRc = 1;
    }
    else if ( *pD2 )
    {
      lRc = -1;
    } /* endif */
  } /* endif */
  
  if( whitespaceDiff ){ 
    if(lRc){
      *whitespaceDiff= 0;
    }else{
      while( *pD1++ ){ wspaceDiff++; }
      while( *pD2++ ){ wspaceDiff++; }
      *whitespaceDiff = wspaceDiff;
    }
  } 
  
  return lRc;
}

LONG UtlCompIgnSpaceW( PSZ_W pD1, PSZ_W pD2, ULONG ulLen, PINT whitespaceDiff)
{
  LONG  lRc = 0;
  ULONG ulI = 0;
  CHAR_W c, d;

  if (ulLen == 0 )
  {
    ULONG  ulLenTmp;

    ulLenTmp = UTF16strlenCHAR(pD2);
    ulLen    = UTF16strlenCHAR(pD1);
    if (ulLenTmp > ulLen )
    {
      ulLen = ulLenTmp;
    } /* endif */
  } /* endif */

  while ( ulI++ < ulLen )
  {
    c = *pD1; d = *pD2;
    if ( (c == ' ') && (d == ' ') )
    {
      pD1++; pD2++;
      /****************************************************************/
      /* skip any consecutive white spaces                            */
      /****************************************************************/
      while ( *pD1 == ' ' )
      {
        pD1++;
        ulI++;
      } /* endwhile */
      while ( *pD2 == ' ' )
      {
        pD2++;
      } /* endwhile */
    }
    else if ( c == d )
    {
      pD1++; pD2++;
    }
    else
    {
      ulI = ulLen;   // stop loop;
      lRc = ( c-d );
    } /* endif */
  } /* endwhile */
  return lRc;
}

BOOL UtlIsWhiteSpaceW( CHAR_W c )
{
  BOOL isWhiteSpace = ( (c == L' ') || (c == L'\r') || (c == L'\n') || (c == L'\t') );
  return ( isWhiteSpace );
}




