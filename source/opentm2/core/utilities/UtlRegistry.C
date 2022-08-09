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
  int res  = properties_get_str_or_default(pszKey, pszBuffer, iBufSize,pszDefault);
  return( res );
} /* end of function GetStringFromRegistry */

// get a integer from the registry
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault )
{  
  int iResult = 0;
  int res  = properties_get_int_or_default(pszKey, iResult, iDefault);
  return( iResult );
} /* end of function GetIntFromRegistry */

// write a string to the registry
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue )
{
  int res = properties_set_str_anyway(pszKey, pszValue);
  
  return res == PROPERTY_NO_ERRORS;
} /* end of function WriteStringToRegistry */

// write a int to the registry
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue )
{
  int res = properties_set_int_anyway(pszKey, iValue);
  
  return res == PROPERTY_NO_ERRORS;

} /* end of function WriteIntToRegistry */
