// helper functions to retrieve registry values
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include "EQF.H"
#include <string>
#include "PropertyWrapper.H"

// get a string from the registry
BOOL GetStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault )
{
  BOOL fOK = properties_get_str(pszKey, pszBuffer, iBufSize);
  if(!fOK)
    strncpy(pszBuffer, pszDefault, iBufSize);
  return( fOK );
} /* end of function GetStringFromRegistry */

// get a integer from the registry
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault )
{  
  int iResult = 0;

  bool fOK = properties_get_int(pszKey, iResult);
  if ( !fOK )
      iResult = iDefault;

  return( iResult );
} /* end of function GetIntFromRegistry */

// write a string to the registry
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue )
{
  bool fOK = properties_add_str(pszKey, pszValue);
  if(!fOK)
    fOK = properties_set_str(pszKey, pszValue);

  return( fOK );
} /* end of function WriteStringToRegistry */

// write a int to the registry
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue )
{
  bool fOK = properties_add_int(pszKey, iValue);
  if(!fOK)
    fOK = properties_set_int(pszKey, iValue);

  return( fOK );

} /* end of function WriteIntToRegistry */
