/*! \file
	Description: Functions replacing OS/2 functions in Windows environment

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include <time.h>
#include <locale.h>
#include "EQF.H"
#include "LogWrapper.h"
#include "OSWrapper.h"
//#include <tchar.h>

// temporary defines to compile without DBCS LIB
USHORT dbcscp = 0;

/**********************************************************************/
/* EQFAnsiToOem: do ANSI/ASCII conversion based on the currently      */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/
VOID
EQFAnsiToOem( PSZ pIn, PSZ pOut )
{
  USHORT usInCP, usOutCP;

  usInCP = (USHORT) GetCodePage( ANSI_CP );
  usOutCP = (USHORT) GetCodePage( OEM_CP );

  // code page conversion required???
  if ( usInCP != usOutCP )
  {
    EQFCPAnsiToOem( usInCP, pIn, usOutCP, pOut );
  }
  else
  {
    // copy data without conversion
    if ( pIn != pOut )
    {
      strcpy( pOut, pIn );
    } /* endif */
  } /* endif */
}

#define NO_ERROR 0

/**********************************************************************/
/* EQFCPAnsiToOem: do ANSI/ASCII conversion based on the passed code  */
/*                 pages                                              */
/**********************************************************************/
VOID
EQFCPAnsiToOem( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut )
{
  USHORT usRc;
  PUCHAR pTable;

  usInCP;
  usRc = UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, &pTable, usOutCP );

  if ((usRc == NO_ERROR) && pTable)
  {
    BYTE c;
    while ( (c = *pIn++ ) != NULC)
    {
            *pOut++ = pTable[ c ];
    }
    *pOut = EOS;
  }
  else
  {
    // standard conversion
  }

}


/**********************************************************************/
/* EQFOemToAnsi: do ASCII/ANSI conversion based on the currently      */
/*               selected codepages (based on Regional Settings)      */
/**********************************************************************/

VOID
EQFOemToAnsi( PSZ pIn, PSZ pOut )
{
  USHORT usInCP, usOutCP;

  usInCP = (USHORT) GetCodePage( OEM_CP );
  usOutCP = (USHORT) GetCodePage( ANSI_CP );

  // code page conversion required???
  if ( usInCP != usOutCP )
  {
    EQFCPOemToAnsi( usInCP, pIn, usOutCP, pOut );
  }
  else
  {
    // copy data without conversion
    if ( pIn != pOut )
    {
      strcpy( pOut, pIn );
    } /* endif */
  } /* endif */
}

/**********************************************************************/
/* EQFCPOemToAnsi: do ASCII/ANSI conversion based on the passed code  */
/*                 pages                                              */
/**********************************************************************/
VOID
EQFCPOemToAnsi( USHORT usInCP, PSZ pIn, USHORT usOutCP, PSZ pOut )
{
  USHORT usRc;
  PUCHAR pTable;
  usOutCP;

  usRc = UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pTable, usInCP );

  if ((usRc == NO_ERROR) && pTable)
  {
    // use our conversion routines
    BYTE c;
    while ( (c = *pIn++) != NULC)
    {
            *pOut++ = pTable[ c ];
    }
    *pOut = EOS;
  }
  else
  {
    // standard conversion
    //OemToAnsi( pIn, pOut );
    EQFOemToAnsi( pIn, pOut );
  }
}


/**********************************************************************/
/* Get Codepage appropriate for the selected source and target langs  */
/**********************************************************************/
// should not be used in System-independent handling!
ULONG GetCodePage( USHORT usType )
{
  CHAR   cp[10];
  ULONG  ulReturnCP = 0L;
  if(VLOG_IS_ON(1))
    LogMessage2(INFO, "GetCodePage::usType = ", toStr(usType).c_str());

  switch ( usType )
  {
    case OEM_CP:
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
      ulReturnCP = (ULONG)_ttol (cp);
      if (ulReturnCP == 720L)
      {
        ulReturnCP = 864L;
      }
      else if ((ulReturnCP == 737L) && (GetOEMCP() == 869) && (GetKBCodePage() == 869 ) )
      {
        // fix for sev1 Greek: Win NT problem (01/09/23)
        ulReturnCP = 869L;
      }
      break;
    case ANSI_CP:
    default:
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, cp, sizeof(cp));
      ulReturnCP = (ULONG) _ttol (&cp[0]);
      break;
  } /* endswitch */

  return (ulReturnCP);
}

