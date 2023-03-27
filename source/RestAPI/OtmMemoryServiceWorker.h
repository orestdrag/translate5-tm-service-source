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


/*! \brief constant defining the timeout for memories (in number of seconds)
  When a memory has not been used for the given time it is automatically closed
*/
#define OTMMEMSERVICE_MEMORY_TIMEOUT 300
#define SHUTDOWN_CALLED_FROM_MAIN -2
class OtmMemoryServiceWorker
/*! \brief   This class provides translation memory related services

*/

{
  std::string printTime(time_t time);
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

  /*! \brief Import a memory from a TMX file
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int import
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  int cloneTMLocaly
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Search for matching proposals 
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int search
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  int resourcesInfo(std::string& strOutput, ProxygenService::ProxygenStats& stats);

  std::string tagReplacement(std::string strInData, int& rc);

  /*! \brief Do a concordance search for the given search string
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int concordanceSearch
  (
    std::string  strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

/*! \brief shutdown the service
  \returns http return code
  */
  int shutdownService(int sig = 0);

  /*! \brief Close all open memories
  \returns http return code0 if successful or an error code in case of failures
  */
  int closeAll ( );

  /*! \brief Checks if there is opened memory in import process
  \returns index if import process for any memory is going on, -1 if no
  */
  int GetMemImportInProcess();

  /*! \brief Saves all open and modified memories
  \returns http return code if successful or an error code in case of failures
  */
  int saveAllTmOnDisk( std::string &strOutputParms );

  /*! \brief List all available TMs
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int list
  (
    std::string &strOutputParms
  );

  /*! \brief Update an entry of the memory
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int updateEntry
  (
    std::string strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief Delete an entry of the memory
  \param strMemory name of memory
  \param strInputParms input parameters in JSON format
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int deleteEntry
  (
    std::string strMemory,
    std::string strInputParms,
    std::string &strOutputParms
  );

  /*! \brief delete a memory
  \param strMemory name of memory
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int deleteMem
  (
    std::string strMemory,
    std::string &strOutputParms
  );


  int reorganizeMem
  (
    std::string strMemory,
    std::string &strOutputParms
  );

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

  /*! \brief get the status of a memory
  \param strMemory name of memory
  \param strOutParms on return filled with the output parameters in JSON format
  \returns http return code
  */
  int getStatus
  ( 
    std::string strMemory, 
    std::string &strOutputParms 
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
  int verifyAPISession
  (
  );
//private:
  
  

  /*! \brief build return JSON string in case of errors
    \param iRC error return code
    \param pszErrorMsg error message text
    \param strError JSON string receiving the error information
    \returns 0 if successful or an error code in case of failures
  */
  int buildErrorReturn
  (
    int iRC,
    wchar_t *pszErrorMsg,
    std::wstring &strErrorReturn
  );

  int buildErrorReturn
  (
    int iRC,
    wchar_t *pszErrorMsg,
    std::string &strErrorReturn
  );

  int buildErrorReturn
  (
    int iRC,
    char *pszErrorMsg,
    std::string &strErrorReturn
  );

  /*! \brief convert a long time value into the UTC date/time format
    \param lTime long time value
    \param pszDateTime buffer receiving the converted date time
    \returns 0 
  */
  int convertTimeToUTC( long lTime, char *pszDateTime );

  /*! \brief convert a UTC time value to a OpenTM2 long time value 
      \param pszDateTime buffer containing the UTC date and time
      \param plTime pointer to a long buffer receiving the converted time 
    \returns 0 
  */
  int convertUTCTimeToLong( char *pszDateTime, long *plTime );

  /*! \brief extract a numeric value from a string
      \param pszString string containing the numeric value
      \param iLen number of digits to be processed
      \param piResult pointer to a int value receiving the extracted value
    \returns TRUE if successful and FALSE in case of errors
  */
  bool getValue( char *pszString, int iLen, int *piResult );

  /*! \brief convert a long time value to a date time string in the form YYYY-MM-DD HH:MM:SS
  \param lTime long time value
  \param pszDateTime buffer receiving the convererd date and time
  \returns 0
  */
  int convertTimeToDateTimeString( long lTime, char *pszDateTime );

  /*! \brief convert a date and time in the form YYYY-MM-DD HH:MM:SS to a OpenTM2 long time value
  \param pszDateTime buffer containing the TMX date and time
  \param plTime pointer to a long buffer receiving the converted time
  \returns 0
  */
  int convertDateTimeStringToLong( char *pszDateTime, long *plTime );
  

  /*! \brief build a unique name for of a temporary file 
  \param pszTempFile buffer reiceiving the temporary file name
  \returns 0 is sucessfull or a return code
  */
  int buildTempFileName( char *pszTempFile );


  /* write proposals to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProposals pointer to a MEMPROPOSAL array containing the proposals
  \param iNumOfProposals number of proposals to write to JSON string
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalsToJSONString
  (
    std::wstring &strJSON,
    PMEMPROPOSAL pProposals,
    int iNumOfProposals,
    void *pvData
  );

  /* write a single proposal to a JSON string
  \param strJSON JSON stirng receiving the proposal data
  \param pProp pointer to a MEMPROPOSAL containing the proposal
  \param pData pointer to LOOKUPINMEMORYDATA area (used as buffer for the proposal data)
  \returns 0 is successful
  */
  int addProposalToJSONString
  (
    std::wstring &strJSON,
    PMEMPROPOSAL pProp,
    void *pvData
  );

  /* convert string proposal type to proposal type enum value
  \param pszType proposal type as string
  \returns proposal type as enum value
  */
  MemProposalType getMemProposalType( char *pszType );

  /*! \brief read a binary file and encode it using BASE64 encoding
  \param pszFile fully qualified name of file being encoded
  \param ppStringData adress of a pointer which will receive the (per malloc allocated) pointer to the area for the encoded string
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int encodeFileInBase64( char *pszFile, char **ppStringData, std::string &strError );

  /*! \brief convert a BASE64 encoded string to a binary file
  \param pStringData pointer to the BASE64 data
  \param pszFile fully qualified name of file being written
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int decodeBase64ToFile( const char *pStringData, const char *pszFile, std::string &strError );

  /*! \brief read a binary file into a Byte vector
  \param pszFile fully qualified name of file being read
  \param vFileData vector receiving the file data
  it is up to the caller to free this area using free()
  \param strError string receiving any error message text
  \returns 0 is sucessfull or a return code
  */
  int loadFileIntoByteVector( char *pszFile, std::vector<unsigned char>  &vFileData );


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
