/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _LanguageFactory_H_
#define _LanguageFactory_H_

#include <string>
#include <vector>
#include <map>

class LanguageFactory
/*! \brief   This class provides a language name related utility factory 

*/

{
public:

/*! \brief Error code definition
*/
  static const int ERROR_LANGUAGENOTFOUND     = 9001;  // the specified language was not found in the languages table
  static const int ERROR_MISSINGPARAMETER     = 9002;  // one or more paramters are missing

	/*! \brief constructor	 */
	LanguageFactory(void);

	/*! \brief destructor	 */
	~LanguageFactory(void);

/*! \brief The layout of the language information provided per language
*/
typedef struct _LANGUAGEINFO
{
  char szName[128];                    // language name used in Opentm2
  char szLangGroup[128];               // language group of the language (will be used as language in the memory lookup)
  char szAddInfo[255];                 // additional language information or comments
  bool fisPreferred;                   // TRUE = this is the preferred language
} LANGUAGEINFO, *PLANGUAGEINFO;

  static int compareLanguageInfo(const void * p1, const void * p2 ) {
            LANGUAGEINFO* a = (LANGUAGEINFO*)p1;
            LANGUAGEINFO* b = (LANGUAGEINFO*)p2; 
            return  strcasecmp(a->szName, b->szName);
  } 
  /*! \brief type of requested languages used by listLanguages method
  */
  typedef enum _LANGUAGETYPE { SOURCE_LANGUAGE_TYPE, TARGET_LANGUAGE_TYPE, ALL_LANGUAGES_TYPE } LANGUAGETYPE;

  /*! \brief type of callback function for listLanguages method */
  typedef int (*PFN_LISTLANGUAGES_CALLBACK )(void *pvData, char *pszName, PLANGUAGEINFO pInfo  );

  /*! \brief Provide a list of all available languages
  \param eType type of languages to be listed
  \param pfnCallBack callback function to be called for each language
  \param pvData caller's data pointer, is passed to callback function
  \param fWithDetails TRUE = supply language details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
  \returns number of provided languages
  */
  int listLanguages(
    LANGUAGETYPE eType,
	  PFN_LISTLANGUAGES_CALLBACK pfnCallBack,			  
	  void *pvData,
	  bool fWithDetails
  );


/*! \brief This static method returns a pointer to the LanguageFactory object.
	The first call of the method creates the LanguageFactory instance.
*/
	static LanguageFactory* getInstance();

/* \brief Get the ISO language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszISO buffer for the language ISO identifier
   \returns 0 when successful or an error code
*/
int getISOName
(
  const char *pszLanguage,
  char *pszISO
);
int getISOName
(
  std::string& strLanguage,
  std::string& strISO
);

bool isPreferedLang(const char* lang);

bool isMatchingLangs(
  const char* strMemLang,//can be prefered
  const char* strSegLang
);


std::vector<std::string> getListOfLanguagesFromTheSameFamily(PSZ lang);

/* \brief Get the OpenTM2 language name for an ISO language identifier
   \param pszISO language ISO identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int getOpenTM2NameFromISO
(
  const char *pszISO,
  char *pszLanguage,
  bool* pfPrefered = nullptr
);
int getOpenTM2NameFromISO
(
  std::string& strISO,
  std::string& strLanguage
);




/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid source language
*/
bool isSourceLanguage
(
  const char *pszLanguage
);

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid target language
*/
bool isTargetLanguage
(
  const char *pszLanguage
);

/* \brief Checks if the given language name is contained in our language list
   \param pszLanguage OpenTM2 language name
   \param fAdjustLangName when TRUE the language name in the buffer is replaced by the correct language name
   \returns TRUE when the specified language is valid 
*/
bool isValidLanguage
(
  char *pszLanguage,
  bool fAdjustLangName
);

bool isTheSameLangGroup
(
  const char* lang1,
  const char* lang2
);

/* \brief Get all information available for the given language
   \param pszLanguage OpenTM2 language name
   \param pInfo a pointer to a LANGUAGEINFO buffer
   \returns TRUE when the info for the specified language could be retrieved
*/
bool getLanguageInfo
(
  const char *pszLanguage,
  PLANGUAGEINFO pInfo
);
  /* \brief Find the entry for the specified OpenTM2 language name 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int findLanguage
(
  const char *pszLanguage
);

//check list of prefered languages for exact match
int findIfPreferedLanguage
(
  const char *pszLanguage
);

//check prefered lang from the same family, for en-US this would be en etc.
int findPreferedLangForThisLang
(
  const char *pszLanguage
);
private:


/* \brief Find the entry for the specified ISO language ID 
   \param pszISO ISO language ID 
   \returns -1 when not found or index into language table
*/
int findISO
(
  const char *pszLanguage,
  bool &fPrefered
);


/* \brief Compare two ISO language names, ignoring an country specific suffix when required
   \param pszISO1 first ISO language name
   \param pszISO2 second ISO language name
   \returns -1 when not found or index into language table
*/
int compareISO
(
  const char *pszISO1, const char *pszISO2
);


  /*! \brief Pointer to the instance of the MemoryFactory object (singleton).
*/
	static LanguageFactory* pInstance;

  /*! \brief Pointer to the loaded language information list
*/
  std::vector<LANGUAGEINFO> vLanguageList;
  std::vector<LANGUAGEINFO*> vpPreferedLanguageList;
  //std::map<std::string, LANGUAGEINFO> vLanguageMap;

  unsigned short loadLanguageList
  ( 
    const char *pszInFile
  );

};


#endif
