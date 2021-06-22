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
  int res  = properties_get_str(pszKey, pszBuffer, iBufSize);
  if( res != PROPERTY_NO_ERRORS )
    strncpy(pszBuffer, pszDefault, iBufSize);
  return( res );
} /* end of function GetStringFromRegistry */

// get a integer from the registry
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault )
{  
  int iResult = 0;

  int res  = properties_get_int(pszKey, iResult);
  if ( res != PROPERTY_NO_ERRORS )
      iResult = iDefault;

  return( iResult );
} /* end of function GetIntFromRegistry */

// write a string to the registry
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue )
{
  int res = properties_add_str(pszKey, pszValue);
  if(res == PROPERTY_ERROR_STR_KEY_ALREADY_EXISTS)
    res = properties_set_str(pszKey, pszValue);
  
  return res == PROPERTY_NO_ERRORS;
} /* end of function WriteStringToRegistry */

// write a int to the registry
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue )
{
  int res = properties_add_int(pszKey, iValue);
  if(res == PROPERTY_ERROR_INT_KEY_ALREADY_EXISTS)
    res = properties_set_int(pszKey, iValue);
  
  return res == PROPERTY_NO_ERRORS;

} /* end of function WriteIntToRegistry */
