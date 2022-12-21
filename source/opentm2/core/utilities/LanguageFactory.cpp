/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "EQF.H"                  // General .H for EQF
#include "LogWrapper.h"
#include "PropertyWrapper.H"
#include "EncodingHelper.h"
// the Win32 Xerces build requires the default structure packing...
//#pragma pack( push )
//#pragma pack(8)

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <string>
#include <algorithm>

//#pragma pack( pop )

XERCES_CPP_NAMESPACE_USE


#include "../utilities/LogWriter.h"
#include "LanguageFactory.H"


//
// class for our SAX handler which parses the XML file containing the language settings
//
class LangParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  LangParseHandler();
  virtual ~LangParseHandler();

  // setter functions 
  void SetLangList( std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTableIn );

  // getter functions 
  bool ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  //  handlers for the SAX DocumentHandler interface
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);

  //  handlers for the SAX ErrorHandler interface
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

private:
  // IDs of XML elelements
  typedef enum { LANGUAGES_ELEMENT, LANGUAGE_ELEMENT, NAME_ELEMENT, 
                 ISO_ELEMENT, PREFERRED_ELEMENT, GROUP_ELEMENT,
                 COMMENTS_ELEMENT, UNKNOWN_ELEMENT
               } ELEMENTID;

  typedef struct _XMLNAMETOID
  {
    std::u16string   u16Name;                 // name of element
    LangParseHandler::ELEMENTID ID;                        // ID of element 
  } XMLNAMETOID, *PXMLNAMETOID;

  LanguageFactory::LANGUAGEINFO CurElement;
  ELEMENTID CurID;
  std::vector<ELEMENTID> vELementStack;
  std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTable;
  CHAR_W szDataW[512];
  CHAR szData[512];
  char szErrorText[256];

  bool fError;                         // TRUE = parsing ended with error

  ELEMENTID GetElementID( const std::u16string& u16name );
};

/*! \brief constructor	 */
LanguageFactory::LanguageFactory(void)
{
  LogMessage1(INFO, "Logging of language name and language properties related information. Starting LanguageFactory" );

  // try to load the external language list
  char szFile[256];
  properties_get_str(KEY_OTM_DIR, szFile, 255);
  strcat(szFile, "/TABLE/languages.xml");

  int iRC = loadLanguageList( szFile );
  
  if ( iRC != 0  || vLanguageList.size() == 0)
  {
    LogMessage5(ERROR, "Load of external language list file \""
      , szFile, " failed with error code ",toStr(iRC).c_str(),", switching to built-in language list" );
    vLanguageList.clear();
    throw;
  }
  else
  {
    LogMessage5(INFO, "Load of external language list file \"", szFile, "\" successfull, the list contains ", toStr(vLanguageList.size()).c_str()," entries" );
  }
}

/*! \brief destructor	 */
LanguageFactory::~LanguageFactory(void)
{
  if ( pvLogWriter != NULL )
  {
    LogWriter *pLogWriter = (LogWriter *)pvLogWriter;
    pLogWriter->write( "Stopping LanguageFactory" );
    pLogWriter->close();
    delete pLogWriter;
  }
}

/*! \brief This static method returns a pointer to the LanguageFactory object.
	The first call of the method creates the LanguageFactory instance.
*/
LanguageFactory* LanguageFactory::getInstance()
{
	static LanguageFactory factory;
	return &factory;
}

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid source language
*/
bool LanguageFactory::isSourceLanguage
(
  const char *pszLanguage
)
{
  return true;
  //return this->findLanguage( pszLanguage ) >= 0;
}

/* \brief Checks if the given language is a valid target language
   \param pszLanguage OpenTM2 language name
   \returns true when the specified language is a valid target language
*/
bool LanguageFactory::isTargetLanguage
(
  const char *pszLanguage
)
{
  return true;
  //return this->findLanguage( pszLanguage ) >= 0;
}

/*! \brief Provide a list of all available languages
\param eType type of languages to be listed
\param pfnCallBack callback function to be called for each language
\param pvData caller's data pointer, is passed to callback function
\param fWithDetails TRUE = supply language details, when this flag is set, 
the pInfo parameter of the callback function is set otherwise it is NULL
\returns number of provided languages
*/
int LanguageFactory::listLanguages(
  LANGUAGETYPE eType,
	PFN_LISTLANGUAGES_CALLBACK pfnCallBack,			  
	void *pvData,
	bool fWithDetails
)
{
  int iLanguages = 0;
  int i = 0;
  int iMax = vLanguageList.size();
  while( i < iMax )
  {
    bool fAddLanguage = FALSE;
    switch( eType )
    {
      case SOURCE_LANGUAGE_TYPE:
        fAddLanguage = true;//vLanguageList[i].isSourceLanguage;
        break;

      case TARGET_LANGUAGE_TYPE:
        fAddLanguage = true;//vLanguageList[i].isTargetLanguage;
        break;

      case ALL_LANGUAGES_TYPE:
      default:
        fAddLanguage = TRUE;
        break;
    }

    if ( fAddLanguage )
    {
      pfnCallBack( pvData, vLanguageList[i].szName, fWithDetails ? &(vLanguageList[i]) : NULL );
      iLanguages++;
    }
    i++;
  }

  return( iLanguages );
}

/* \brief Get the ISO language identifier for a OpenTM2 language name
   \param pszLanguage OpenTM2 language name
   \param pszISO buffer for the language ISO identifier
   \returns 0 when successful or an error code
*/
int LanguageFactory::getISOName
(
  const char *pszLanguage,
  char *pszISO
)
{
  int i = this->findLanguage( pszLanguage );
  if ( i < 0 )
  {
    strcpy( pszISO, "??" );
    return( ERROR_LANGUAGENOTFOUND );
  }

  strcpy( pszISO, vLanguageList[i].szName );

  return( 0 );
}

int LanguageFactory::getISOName
(
  std::string& strLanguage,
  std::string& strISO
)
{
  int i = this->findLanguage( strLanguage.c_str() );
  if ( i < 0 )
  {
    strISO = "??";
    return( ERROR_LANGUAGENOTFOUND );
  }

  strISO = vLanguageList[i].szName;

  return( 0 );
}


std::vector<std::string> LanguageFactory::getListOfLanguagesFromTheSameFamily(PSZ lang){
  std::vector<std::string> res;
  int i = 0, iMax = vLanguageList.size();
  while( i < iMax )
  {
    if ( compareISO( lang, vLanguageList[i].szName) == 0 )
    {
      res.push_back(vLanguageList[i].szName);
    }
    i++;
  } /* endwhile */
  return res;
}

/* \brief Get the OpenTM2 language name for an ISO language identifier
   \param pszISO language ISO identifier
   \param pszLanguage buffer for OpenTM2 language name
   \returns 0 when successful or an error code
*/
int LanguageFactory::getOpenTM2NameFromISO
(
  const char *pszISO,
  char *pszLanguage, 
  bool* pfPrefered
)
{
  bool fPrefered = false;
  int i = this->findISO( pszISO, fPrefered);
  if ( i < 0 )
  {
    LogMessage2(ERROR, "LanguageFactory::getOpenTM2NameFromISO()::ERROR_LANGUAGENOTFOUND, pszISO = ", pszISO);
    return( ERROR_LANGUAGENOTFOUND );
  }

  if(pfPrefered != nullptr){
    *pfPrefered = fPrefered;
  }

  strcpy( pszLanguage, vLanguageList[i].szName );

  return( 0 );
}

int LanguageFactory::getOpenTM2NameFromISO
(
  std::string& strISO,
  std::string& strLanguage
)
{
  bool fPrefered;
  int i = this->findISO( strISO.c_str(), fPrefered );
  if ( i < 0 )
  {
    return( ERROR_LANGUAGENOTFOUND );
  }

  strLanguage = vLanguageList[i].szName;

  return( 0 );
}




/* \brief Checks if the given language name is contained in our language list
   \param pszLanguage OpenTM2 language name
   \param fAdjustLangName when TRUE the language name in the buffer is replaced by the correct language name
   \returns TRUE when the specified language is valid 
*/
bool LanguageFactory::isValidLanguage
(
  char *pszLanguage,
  bool fAdjustLangName
)
{
  int i = this->findLanguage( pszLanguage );
  if ( (i >= 0) && fAdjustLangName )
  {
    strcpy( pszLanguage, vLanguageList[i].szName );
  } /* endif */

  return( i >= 0 );
}

bool LanguageFactory::isTheSameLangGroup(
  const char* lang1,
  const char* lang2
)
{
  LANGUAGEINFO lang1info, lang2info;
  if( getLanguageInfo(lang1, &lang1info) == false )
  {
    LogMessage3(WARNING,__func__, ":: can't found lang1info for ", lang1);
  }
  if( getLanguageInfo(lang2, &lang2info) == false )
  {
    LogMessage3(WARNING,__func__, ":: can't found lang1info for ", lang2);
  }
  return  strcmp(lang1info.szLangGroup, lang2info.szLangGroup) == 0;
}
/* \brief Get all information available for the given language
   \param pszLanguage OpenTM2 language name
   \param pInfo a pointer to a LANGUAGEINFO buffer
   \returns TRUE when the info for the specified language could be retrieved
*/
bool LanguageFactory::getLanguageInfo
(
  const char *pszLanguage,
  PLANGUAGEINFO pInfo
)
{
  int i = this->findLanguage( pszLanguage );
  if ( i < 0 ) return( FALSE );

  memcpy( pInfo, &(vLanguageList[i]), sizeof(LANGUAGEINFO) );

  return( TRUE );
}


int LanguageFactory::findIfPreferedLanguage
(
  const char *pszLanguage
)
{
  int i = 0;
  int mid, st = 0, end = vpPreferedLanguageList.size() -1;
  while ( st <= end)  //binary search since languages was sorted
  {  
      mid = ( st + end ) / 2;   
      int cmp = strcasecmp(vpPreferedLanguageList[mid]->szName, pszLanguage);
      if (cmp == 0)  
        return vpPreferedLanguageList[mid] - &vLanguageList[0];
      else if ( cmp < 0)  
          st = mid + 1; 
      else if ( cmp > 0 )  
          end = mid - 1; 
  }  
  return -1;  
}

int LanguageFactory::findPreferedLangForThisLang
(
  const char *pszLanguage
)
{
  int i = 0;
  int mid, st = 0, end = vpPreferedLanguageList.size() -1;
  while ( st <= end)  //binary search since languages was sorted
  {  
      mid = ( st + end ) / 2;   
      int cmp = compareISO(vpPreferedLanguageList[mid]->szName, pszLanguage);
      if (cmp == 0)  
        return vpPreferedLanguageList[mid] - &vLanguageList[0];
      else if ( cmp < 0)  
          st = mid + 1; 
      else if ( cmp > 0 )  
          end = mid - 1; 
  }  
  return -1;  
}

/* \brief Find the entry for the specified OpenTM2 language name 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::findLanguage
(
  const char *pszLanguage
)
{
  int i = 0;
  int mid, st = 0, end = vLanguageList.size() -1;
  while ( st <= end)  //binary search since languages was sorted
  {  
      mid = ( st + end ) / 2;   
      int cmp = strcasecmp(vLanguageList[mid].szName, pszLanguage);
      if (cmp == 0)  
        return mid;
      else if ( cmp < 0)  
          st = mid + 1; 
      else if ( cmp > 0 )  
          end = mid - 1; 
  }  

  //LANGUAGEINFO* elem = (LANGUAGEINFO*) std::bsearch( &langInf,
  //              vLanguageList.data(),
  //              vLanguageList.size(),
  //              sizeof(LANGUAGEINFO),
  //              compareLanguageInfo);
  //int iMax = vLanguageList.size();
  //while( i < iMax )
  //{
  //  if ( strcasecmp( pszLanguage, vLanguageList[i].szName ) == 0 )
  //  {
  //    return( i );
  //  }
  //  i++;
  //}
  //if(elem != nullptr){
  //  return vLanguageList.data() - elem;
  //}
  return( -1 );
}

/* \brief Compare two ISO language names, ignoring an country specific suffix when required
   \param pszISO1 first ISO language name
   \param pszISO2 second ISO language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::compareISO
(
  const char *pszISO1, const char *pszISO2
)
{

   int iResult = strcasecmp( pszISO1, pszISO2 );
   if ( iResult == 0 ) return( iResult );

   // return when both ISO names have no country ID
   PSZ pszDash1 = (PSZ)strchr( pszISO1, '-' );
   if ( pszDash1 == NULL ) pszDash1 = (char*)strchr( pszISO1, '_' );
   PSZ pszDash2 = (PSZ)strchr( pszISO2, '-' );
   if ( pszDash2 == NULL ) pszDash2 = (char*)strchr( pszISO2, '_' );
   if ( (pszDash1 == NULL) && (pszDash2 == NULL)  ) return( iResult );

   // compare ISO names without country suffix
   char szLang1[100];
   char szLang2[100];

   if ( pszDash1 )
   {
     Utlstrccpy( szLang1, (PSZ)pszISO1, *pszDash1 );
   }
   else
   {
     strcpy( szLang1, (PSZ)pszISO1);
   } /* endif */
   if ( pszDash2 )
   {
     Utlstrccpy( szLang2, (PSZ)pszISO2, *pszDash2 );
   }
   else
   {
     strcpy( szLang2, (PSZ)pszISO2);
   } /* endif */
 
   return ( strcasecmp( szLang1, szLang2 ) );
}

/* \brief Find the entry for the specified ISO language ID 
   \param pszLanguage OpenTM2 language name
   \returns -1 when not found or index into language table
*/
int LanguageFactory::findISO
(
  const char *pszISO,bool &fPrefered
)
{
  //int iFound = -1;
  //int i = 0;
  //int iMax = vLanguageList.size();
  //int iPreferred = findPreferedLanguage(pszISO);//-1;
  //int iBestMatch = findLanguage(pszISO);
  /*
  if(iMax<=0){
    LogMessage1(ERROR,"LanguageFactory::findISO():: language list size is 0");
    return (iFound);
  }
  
  
  while( i < iMax )
  {
    if ( compareISO( pszISO, vLanguageList[i].szName ) == 0 )
    {
      if ( strcasecmp( pszISO, vLanguageList[i].szName ) == 0 )
      {
        // perfect match, return if this is the preferred language
        if ( vLanguageList[i].fisPreferred )
        {
          fPrefered = true;
          return( i );
        }
        iBestMatch = i;
      }
      if ( vLanguageList[i].fisPreferred )
      {
        iPreferred = i;
      }
      iFound = i;
    }
    i++;
  } /* endwhile */
  int iBestMatch = findIfPreferedLanguage(pszISO);
  fPrefered = true;

  if( iBestMatch == -1 ){
    fPrefered = false;
    iBestMatch = findLanguage(pszISO);
  }
  
  if( iBestMatch == -1 ){
    iBestMatch = findPreferedLangForThisLang(pszISO);
  }

  LogMessage4(DEVELOP, "LanguageFactory::findISO()::fPrefered = ", toStr(fPrefered).c_str(), "; iBestMatch = ", toStr(iBestMatch).c_str());
  
  return( iBestMatch );
}


unsigned short LanguageFactory::loadLanguageList( const char *pszLangList )
{
  unsigned short usRC = 0;

  // parse the XML file containing the language properties
  if ( !usRC )
  {
    SAXParser* parser = new SAXParser();      // Create a SAX parser object

    // create an instance of our handler
    LangParseHandler *handler = new LangParseHandler();


    //  install our SAX handler as the document and error handler.
    parser->setDocumentHandler( handler );
    parser->setErrorHandler( handler );
    parser->setValidationSchemaFullChecking( false );
    parser->setDoSchema( false );
    parser->setLoadExternalDTD( false );
    parser->setValidationScheme( SAXParser::Val_Never );
    parser->setExitOnFirstFatalError( false );
    handler->SetLangList( &vLanguageList );

    try
    {
      parser->parse( pszLangList );
    }
    catch (const OutOfMemoryException& )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

      usRC = ERROR_READ_FAULT;
    }
    /*
    if( usRC == 0 ){
      LogMessage2(INFO, __func__, ":: languages was read from file, sorting...");
      std::sort(vLanguageList.begin(), vLanguageList.end(), [&]( LANGUAGEINFO& a, LANGUAGEINFO& b) {
            LogMessage2(INFO, "a.szName = ", a.szName);
            LogMessage2(INFO, "b.szName = ", b.szName);
            return  strcmp(a.szName, b.szName);
        } );    
    }
    //*/
    //*
    if( usRC == 0 ){
      LogMessage2(INFO, __func__, ":: languages was read from file, sorting...");
      std::qsort(&vLanguageList[0], vLanguageList.size(), sizeof(LANGUAGEINFO), compareLanguageInfo);    
      usRC = false == std::is_sorted(vLanguageList.begin(), vLanguageList.end(), 
          []( LANGUAGEINFO a, LANGUAGEINFO b) {//values should be unique
            return  strcmp(a.szName, b.szName) < 0;
          } ) ;
    }

    if( usRC == 0){
      for(LANGUAGEINFO& lang : vLanguageList ){
        if(lang.fisPreferred){
          vpPreferedLanguageList.push_back(&lang);
        }
      }
    }
    
    //*/
  } /* endif */


  return( usRC );
}


//
// implementation of SAX parser for the parsing of the language control file
//
LangParseHandler::LangParseHandler()
{
  fError = FALSE;
  pvLangTable = NULL;
}

LangParseHandler::~LangParseHandler()
{
}

void LangParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    std::u16string u16name((char16_t*)name);
    CurID = GetElementID( u16name );
    vELementStack.push_back( CurID );
    szDataW[0] = 0;
    
    switch ( CurID )
    {
      case LANGUAGE_ELEMENT:
      {
        // reset element info
        memset( &(CurElement), 0, sizeof(CurElement) );
        break;
      }
      case NAME_ELEMENT:
      case PREFERRED_ELEMENT:
      case GROUP_ELEMENT:
      case COMMENTS_ELEMENT:
      case LANGUAGES_ELEMENT:
      {
        break;
      }
      case UNKNOWN_ELEMENT:
      default:
        if(CheckLogLevel(DEVELOP)){    
          std::string sName = EncodingHelper::convertToUTF8(u16name);
          LogMessage5(WARNING, "LangParseHandler::startElement::unknown_element found: ", 
            EncodingHelper::convertToUTF8(u16name).c_str(), "; id = ", toStr(CurID).c_str() , "; skipping...");
        }
        break;
    } /*endswitch */
}

void LangParseHandler::endElement(const XMLCh* const name )
{
  CurID = vELementStack.back();
  vELementStack.pop_back();

  // convert collected data to ASCII
  std::string sData = EncodingHelper::convertToUTF8(this->szDataW);
  
  switch ( CurID )
  {
    case LANGUAGE_ELEMENT:
      // add new language element to the language table
      if ( this->CurElement.szName[0] != '\0' )
      {
        this->pvLangTable->push_back( this->CurElement );
      }
      break;
    case NAME_ELEMENT:
      if(this->CurElement.szName[0] != '\0'){
        LogMessage5(WARNING, __func__, "::name element was parsed for the second time, first :", this->CurElement.szName, "; second = ", sData.c_str());
      }
      strncpy( this->CurElement.szName, sData.c_str(), sizeof(this->CurElement.szName) );
      break;
    case PREFERRED_ELEMENT:
      this->CurElement.fisPreferred = (strcasecmp( sData.c_str(), "yes" ) == 0);
      break;
    case GROUP_ELEMENT:
      strncpy( this->CurElement.szLangGroup, sData.c_str(), sizeof(this->CurElement.szLangGroup) );
      break;
    case COMMENTS_ELEMENT:
      if(this->CurElement.szAddInfo[0] != '\0'){
        LogMessage5(WARNING, __func__, "::comments element was parsed for the second time, first :", this->CurElement.szAddInfo, "; second = ", sData.c_str());
      }
      strncpy( this->CurElement.szAddInfo, sData.c_str(), sizeof(this->CurElement.szAddInfo) );
      break;
    case LANGUAGES_ELEMENT:
      break;
    case UNKNOWN_ELEMENT:
    default:
      //if(CheckLogLevel(DEVELOP)){
        LogMessage3(WARNING,__func__,":: UNKNOWN_ELEMENT FOUND: ", sData.c_str());
      //}
      break;
  } /*endswitch */
}

void LangParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  auto wstr = EncodingHelper::toWChar(std::u16string((char16_t*)chars).c_str());

  // add data to current data buffer 
  int iCurLen = wcslen( szDataW );
  if ( (iCurLen + wstr.size() + 1) < (sizeof(szData)/sizeof(szData[0])))
  {
    wcsncpy( szDataW + iCurLen, wstr.c_str(), wstr.size() );
    szDataW[iCurLen+wstr.size()] = 0;
  }else{
    if(CheckLogLevel(DEVELOP)){
      auto str = EncodingHelper::convertToUTF8(wstr.c_str());
      LogMessage3(ERROR, __func__ , ":: (iCurLen + length + 1) >= (sizeof(szData)/sizeof(szData[0])), characters = ", str.c_str());
    }
  } /* endif */
}

void LangParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    LogMessage2(ERROR, "LangParseHandler::fatalError:: ", this->szErrorText);
    XMLString::release( &message );
}

void LangParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Error: %s at column %ld in line %ld", message, col, line );
    LogMessage2(ERROR, "LangParseHandler::error:: ", this->szErrorText);
    XMLString::release( &message );
}

void LangParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Warning: %s at column %ld in line %ld", message, col, line );
    LogMessage2(WARNING, "LangParseHandler::warning:: ", this->szErrorText);
    XMLString::release( &message );
}


// get the ID for a XML element
LangParseHandler::ELEMENTID LangParseHandler::GetElementID( const std::u16string& u16name )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;
static XMLNAMETOID XmlNameToID[] =
{ { u"languages",  LANGUAGES_ELEMENT },
  { u"language",   LANGUAGE_ELEMENT }, 
  { u"name",       NAME_ELEMENT }, 
  { u"preferred",  PREFERRED_ELEMENT }, 
  { u"group",      GROUP_ELEMENT }, 
  { u"comments",   COMMENTS_ELEMENT }, 
  { u"",           UNKNOWN_ELEMENT } };

  std::u16string u16name_lower = EncodingHelper::toLower(u16name);

  while ( (IDFound == UNKNOWN_ELEMENT) && !(XmlNameToID[i].u16Name.empty()) )
  {
    if ( u16name_lower == EncodingHelper::toLower(XmlNameToID[i].u16Name) )
    {
      IDFound = XmlNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method LangParseHandler::GetElementID */

bool LangParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void LangParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->szErrorText[0] != '\0' )
  {
    strncpy( pszTextBuffer, this->szErrorText, iBufSize );
    pszTextBuffer[iBufSize-1] = '\0';
  } /* endif */
}

void LangParseHandler::SetLangList( std::vector<LanguageFactory::LANGUAGEINFO> *pvLangTableIn )
{
  this->pvLangTable = pvLangTableIn;
}


