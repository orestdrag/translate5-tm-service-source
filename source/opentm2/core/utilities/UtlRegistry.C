// helper functions to retrieve registry values
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#include "EQF.H"
#include "SystemFunctionsWrapper.h"
#include <string>

// get a string from the registry
BOOL GetStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault )
{
  std::string sAppl = pszAppl;
  std::string sKey = pszKey;
  std::string sResult;

  BOOL fOK = SystemFunctionsWrapper::GetStringFromRegistry(sAppl, sKey, sResult);
  if ( fOK )
  {
    strncpy(pszBuffer, sResult.c_str(),iBufSize);
  }else{
    strncpy( pszBuffer, pszDefault, iBufSize );
  } 

  return( fOK );
} /* end of function GetStringFromRegistry */

// get a integer from the registry
int GetIntFromRegistry( PSZ pszAppl, PSZ pszKey, int iDefault )
{
  int iResult = 0;
  std::string sAppl = pszAppl;
  std::string sKey = pszKey;

  BOOL fOK = SystemFunctionsWrapper::GetIntFromRegistry(sAppl, sKey, iResult);
  if ( !fOK )
  {
      iResult = iDefault;
  } 

  return( iResult );
} /* end of function GetIntFromRegistry */

// write a string to the registry
BOOL WriteStringToRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszValue )
{
  std::string sAppl = pszAppl;
  std::string sKey = pszKey;
  std::string sValue = pszValue;
  BOOL fOK = SystemFunctionsWrapper::WriteStringToRegistry(sAppl, sKey, sValue);

  return( fOK );
} /* end of function WriteStringToRegistry */

// write a int to the registry
BOOL WriteIntToRegistry( PSZ pszAppl, PSZ pszKey, int iValue )
{
  std::string sAppl = pszAppl;
  std::string sKey = pszKey;
  BOOL fOK = SystemFunctionsWrapper::WriteIntToRegistry(sAppl, sKey, iValue);

  return( fOK );
} /* end of function WriteIntToRegistry */
