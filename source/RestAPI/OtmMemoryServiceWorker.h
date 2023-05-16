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
  
/*! \brief constant defining the timeout for memories (in number of seconds)
  When a memory has not been used for the given time it is automatically closed
*/
#define OTMMEMSERVICE_MEMORY_TIMEOUT 300
#define SHUTDOWN_CALLED_FROM_MAIN -2
class OtmMemoryServiceWorker
/*! \brief   This class provides translation memory related services

*/

{
public:

  /*! \brief OtmMemoryServiceWorker constructor
  */
  OtmMemoryServiceWorker();


/*! \brief This static method returns a pointer to the OtmMemoryService object.
	  The first call of the method creates the OtmMemoryService instance.
  */
	static OtmMemoryServiceWorker* getInstance();

	
  int init();


  std::atomic_bool fServiceIsRunning{0};


  int GetMemImportInProcess();

  /*! \brief Saves all open and modified memories
  \returns http return code if successful or an error code in case of failures
  */
  int saveAllTmOnDisk( std::string &strOutputParms );

  /*! \brief get the data of a memory in binary form or TMX form
  \param strMemory name of memory
  \param strType type of the requested memory data
  \param vMemData vector<Byte> receiving the memory data
  \returns http return code
  */
  int getMem
  (
    std::string strMemory,
    std::string strType,
    std::vector<unsigned char> &vMemData
  );

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
//private:
  
  

  /*! \brief build return JSON string in case of errors
    \param iRC error return code
    \param pszErrorMsg error message text
    \param strError JSON string receiving the error information
    \returns 0 if successful or an error code in case of failures
  */
  //int buildErrorReturn( int iRC, wchar_t *pszErrorMsg, std::wstring &strErrorReturn );

  //int buildErrorReturn( int iRC, wchar_t *pszErrorMsg, std::string &strErrorReturn );

  //int buildErrorReturn( int iRC, char *pszErrorMsg,  std::string &strErrorReturn );


  /*! \brief convert a UTC time value to a OpenTM2 long time value 
      \param pszDateTime buffer containing the UTC date and time
      \param plTime pointer to a long buffer receiving the converted time 
    \returns 0 
  */
  //int convertUTCTimeToLong( char *pszDateTime, long *plTime );

  /*! \brief extract a numeric value from a string
      \param pszString string containing the numeric value
      \param iLen number of digits to be processed
      \param piResult pointer to a int value receiving the extracted value
    \returns TRUE if successful and FALSE in case of errors
  */
  //bool getValue( char *pszString, int iLen, int *piResult );

  /*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
  \param lTime long time value
  \param pszDateTime buffer receiving the convererd date and time
  \returns 0
  */
  //int convertTimeToDateTimeString( long lTime, char *pszDateTime );

  /*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
  \param pszDateTime buffer containing the TMX date and time
  \param plTime pointer to a long buffer receiving the converted time
  \returns 0
  */
  //int convertDateTimeStringToLong( char *pszDateTime, long *plTime );
  


  /* write proposals to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProposals pointer to a MEMPROPOSAL array containing the proposals
  \param iNumOfProposals number of proposals to write to JSON string
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalsToJSONString( std::wstring &strJSON, PMEMPROPOSAL pProposals, int iNumOfProposals, void *pvData );

  /* write a single proposal to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProp pointer to a MEMPROPOSAL containing the proposal
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalToJSONString ( std::wstring &strJSON, PMEMPROPOSAL pProp, void *pvData );

/*! \brief OpenTM2 API session handle
*/
	HSESSION hSession = 0;

/*! \brief Last return code
*/
  int iLastRC;
  
/*! \brief Buffer for last error message
*/
  wchar_t szLastError[4000];

};


#endif
