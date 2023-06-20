/**
* \file OtmMemoryService.H
*
* \brief Header file for OtmMemoryService class
*
*	Copyright Notice:
*
*	Copyright (C) 2016, QSoft GmbH. All rights reserved
*	
**/
#ifndef _OtmMemoryServiceWorker_H_
#define _OtmMemoryServiceWorker_H_

#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include "ProxygenStats.h"
//#include "EQFFUNCI.H"
#include "tm.h"
#include "requestdata.h"


  /*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date time
    \returns 0 
  */
  int convertTimeToUTC( long lTime, char *pszDateTime );
  
/*! \brief constant defining the timeout for memories (in number
. of seconds)
  When a memory has not been used for the given time it is automatically closed
*/
#define OTMMEMSERVICE_MEMORY_TIMEOUT 300
#define SHUTDOWN_CALLED_FROM_MAIN -2
class OtmMemoryServiceWorker
/*! \brief   This class provides translation memory related services*/
{
public:
/*! \brief This static method returns a pointer to the OtmMemoryService object.
	  The first call of the method creates the OtmMemoryService instance.
  */
	static OtmMemoryServiceWorker* getInstance();
	
  int init();

  std::atomic_bool fServiceIsRunning{0};

  int GetMemImportInProcess();

  /* change the status of a memory in our memory list at the end of an import
  \param pszMemory name of the memory
  \param
  */
  void importDone( char *pszMemory, int iRC, char *pszError );

  std::vector<std::wstring> replaceString(std::wstring&& src_data, std::wstring&& trg_data, std::wstring&& req_data,  int* rc);
  /*! \brief Verify OpenTM2 API session
      \returns 0 if successful or an error code in case of failures
    */
  int verifyAPISession();

/*! \brief OpenTM2 API session handle*/
	HSESSION hSession = 0;

/*! \brief Last return code*/
  int iLastRC;
  
/*! \brief Buffer for last error message*/
  wchar_t szLastError[4000];
};


#endif
