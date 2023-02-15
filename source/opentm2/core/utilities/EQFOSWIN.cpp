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

#define NO_ERROR 0

/**********************************************************************/
/* Get Codepage appropriate for the selected source and target langs  */
/**********************************************************************/
// should not be used in System-independent handling!
ULONG GetCodePage( USHORT usType )
{
  CHAR   cp[10];
  ULONG  ulReturnCP = 0L;
  if(VLOG_IS_ON(1))
    LogMessage( T5DEBUG, "GetCodePage::usType = ", toStr(usType).c_str());

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

