//+----------------------------------------------------------------------------+
//|EQFUTLNG.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#define INCL_EQF_MORPH            // general morph functions
#define INCL_EQF_TM               // general Transl. Memory functions
#include <EQF.H>                  // General Translation Manager include file
//#include <EQFLNG00.H>             // specific defines for Language properties
#include <EQFSETUP.H>             // specific defines for Language properties

#include "LanguageFactory.H"
#include "../utilities/EncodingHelper.h"
#include "../utilities/LogWrapper.h"


/*
 ** UtlQuerySysLangFile
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *    Scans the system language file for the given language and,
 *    if found, set the names for the resource, the name of the
 *    help file and the name of the message file or returns FALSE
 *    if the language was not found. The returned names are fully
 *    qualified path names except for the name of the resource
 *    file.
 *
 *  RETURNS:
 *        BOOL  fOK   TRUE = language was found and file names have been
 *                                               set
 *                FALSE = language not found in file, file names have
 *                                                not been set
 */

BOOL UtlQuerySysLangFile
(
  PSZ              pszLanguage,        // language to search in file
  PSZ              pszResFile,         // buffer for name of resource file
  PSZ              pszHelpFile,        // buffer for name of help file
  PSZ              pszMsgFile          // buffer for name of message file
)
{
  CHAR     szFileName[MAX_EQF_PATH];   // buffer for file name
  BOOL     fOK = TRUE;                 // function return code
  PSZ      pszLangFileBuffer = NULL;   // buffer for in-memory copy of language file
  PSZ      pszCurLine;                 // ptr to current line in file
  BOOL     fFound = FALSE;
  ULONG    ulLength;                   // length of language file
  USHORT   usI;                        // loop index
  CHAR     szDrive[MAX_DRIVE];         // buffer for drive list
  CHAR     szSysPath[MAX_EQF_PATH];    // global system path
  CHAR     EqfSystemPath[MAX_EQF_PATH];// global system path

  /********************************************************************/
  /* initialize result fields                                         */
  /********************************************************************/
  *pszResFile  = EOS;
  *pszHelpFile = EOS;
  *pszMsgFile  = EOS;

  /********************************************************************/
  /* build name of system language file                               */
  /********************************************************************/
  GetStringFromRegistry( APPL_Name, KEY_OTM_DIR, szDrive, sizeof( szDrive  ), "" );
  GetStringFromRegistry( APPL_Name, KEY_Path, szSysPath, sizeof( szSysPath), "" );

  sprintf( EqfSystemPath, "%s",  szSysPath );
  sprintf( szFileName, "%s/%s/%s",  EqfSystemPath, TABLEDIR, SYSLANGFILE);

  /********************************************************************/
  /* load system language file into memory                            */
  /********************************************************************/
  fOK = UtlLoadFileL( szFileName, (PVOID *)&pszLangFileBuffer,
                     &ulLength, FALSE, FALSE );

  /********************************************************************/
  /* preprocess data in language file: replace CR by space            */
  /********************************************************************/
  if ( fOK )
  {
    for ( usI = 0 ; usI < ulLength; usI++ )
    {
      if ( pszLangFileBuffer[usI] == CR )
      {
        pszLangFileBuffer[usI] = SPACE;
      } /* endif */
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /* search for given language in file                                */
  /********************************************************************/
  if ( fOK )
  {
    pszCurLine = pszLangFileBuffer;

    pszLangFileBuffer[ulLength-1] = EOS; // force end string delimiter

    while ( !fFound && (*pszCurLine != EOS) )
    {
      if ( *pszCurLine == '*' )
      {
        /**************************************************************/
        /* ignore comment line                                        */
        /**************************************************************/
      }
      else
      {
        /**************************************************************/
        /* check language                                             */
        /**************************************************************/
        PSZ      pszStart;

        /**************************************************************/
        /* handle language names enclosed in double quotes            */
        /**************************************************************/
        if ( *pszCurLine == '\"' )
        {
          pszStart = pszCurLine + 1;
        }
        else
        {
          pszStart = pszCurLine;
        } /* endif */

        /**************************************************************/
        /* compare the language name                                  */
        /**************************************************************/
        if ( strncasecmp( pszStart, pszLanguage, strlen(pszLanguage) ) == 0 )
        {
          PSZ pszTarget;

          /************************************************************/
          /* Language has been found, now extract file names          */
          /************************************************************/

          /************************************************************/
          /* first of all skip language                               */
          /************************************************************/
          if ( *pszCurLine == DOUBLEQUOTE )
          {
            /**********************************************************/
            /* Skip language enclosed in double quotes                */
            /**********************************************************/
            pszCurLine++;
            while ( (*pszCurLine != LF ) &&
                    (*pszCurLine != EOS) &&
                    (*pszCurLine != DOUBLEQUOTE) )
            {
              pszCurLine++;
            } /* endwhile */
            if ( *pszCurLine == DOUBLEQUOTE )
            {
              pszCurLine++;
            } /* endif */
          }
          else
          {
            /**********************************************************/
            /* Skip language delimited by comma                       */
            /**********************************************************/
            while ( (*pszCurLine != LF ) &&
                    (*pszCurLine != EOS) &&
                    (*pszCurLine != COMMA) )
            {
              pszCurLine++;
            } /* endwhile */
          } /* endif */
          if ( *pszCurLine == COMMA ) pszCurLine++;

          /************************************************************/
          /* now extract message file name                            */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszMsgFile, EqfSystemPath );
          strcat( pszMsgFile, BACKSLASH_STR );
          strcat( pszMsgFile, MSGDIR );
          strcat( pszMsgFile, BACKSLASH_STR );
          strcat( pszMsgFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;

          /************************************************************/
          /* now extract help file name                               */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszHelpFile, EqfSystemPath );
          strcat( pszHelpFile, BACKSLASH_STR );
          strcat( pszHelpFile, MSGDIR );
          strcat( pszHelpFile, BACKSLASH_STR );
          strcat( pszHelpFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;


          /************************************************************/
          /* now extract resource file name                           */
          /************************************************************/
          pszTarget = szFileName;
          while ( (*pszCurLine != LF ) &&
                  (*pszCurLine != EOS) &&
                  (*pszCurLine != COMMA) )
          {
            *pszTarget++ = *pszCurLine++;
          } /* endwhile */
          *pszTarget = EOS;
          UtlStripBlanks( szFileName );
          strcpy( pszResFile, szFileName );
          if ( *pszCurLine == COMMA ) pszCurLine++;

          fFound = TRUE;
          fOK = (*pszResFile != EOS) &&
                (*pszMsgFile != EOS) &&
                (*pszHelpFile != EOS);
        }
        else
        {
          /************************************************************/
          /* languages do not match, ignore current line              */
          /************************************************************/
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* skip current line                                            */
      /****************************************************************/
      while ( (*pszCurLine != LF ) &&   (*pszCurLine != EOS) )
      {
        pszCurLine++;
      } /* endwhile */
      if ( *pszCurLine == LF )
      {
        pszCurLine++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  /********************************************************************/
  /* cleanup                                                          */
  /********************************************************************/
  if ( pszLangFileBuffer ) 
    UtlAlloc( (PVOID *)&pszLangFileBuffer, 0L, 0L, NOMSG );

  return( fOK && fFound );
} /* end of function TWBQuerySysLangFile */

// same as GetLangxxxCP but returns unadjusted codepage
ULONG GetOrgLangCP
(
 PSZ pLanguage,
 BOOL  fASCII
)
{
  if(T5Logger::GetInstance()->CheckLogLevel(T5INFO)){
      T5LOG( T5WARNING) <<  "::called commented out function that would return 0";
  }
  return 1;
}

////////////////////////////////////////////////////////////////
// Get Codepage appropriate for given language ( not OS-dependent!)
///////////////////////////////////////////////////////////////////
typedef struct LANGUAGE_CACHE
{
  CHAR        szLanguage[MAX_LANG_LENGTH];
  ULONG       ulCodePage;
  unsigned int uiCounter;
} LANGUAGE_CACHE, *PLANGUAGE_CACHE;

#define MAX_LANGCACHE 10

static LANGUAGE_CACHE LangCache[MAX_LANGCACHE];
static int iLangCacheUsed = 0;                   // number of used entries in laguage cache
static unsigned int uiLangCacheCounter = 1;      // update counter for language cache entries
static char szChachedLang1[MAX_LANG_LENGTH] = "";
static char szChachedLang2[MAX_LANG_LENGTH] = "";
static ULONG ulCachedCP1 = 0;
static ULONG ulCachedCP2 = 0;

static char* strupr(char *str)
{
    char *tmp = str;
    while(*tmp) {
        *tmp = toupper((unsigned char)*tmp);
        ++tmp;
    }
    return str;
}



// Get the currently set codepage.
PSZ GetSystemLanguage( VOID )
{
    PPROPSYSTEM pPropSys = GetSystemPropPtr();

    return &pPropSys->szSystemPrefLang[0];
}

// check if given language name is valid
BOOL isValidLanguage( PSZ pszLanguage, BOOL fAdjustLangName )
{
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  return( pLangFactory->isValidLanguage( pszLanguage, fAdjustLangName ) );
}

// get the group a given language belongs to
BOOL GetLanguageGroup( PSZ pszLanguage, PSZ pszGroup )
{
  BOOL fOK = TRUE;
  LanguageFactory::LANGUAGEINFO LangInfo;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();

  fOK = pLangFactory->getLanguageInfo( pszLanguage, &LangInfo );
  if ( fOK ) strcpy( pszGroup, LangInfo.szLangGroup );
  //if ( fOK ) strcpy( pszGroup, LangInfo.szName );
  return( fOK );
}
