/*! \file

 \brief wrapper and helper functions for memory plugin

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_MORPH            // morphologic functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_TP              // editor functions
#include "../pluginmanager/PluginManager.h"
#include "../pluginmanager/OtmMemoryPlugin.h"
#include "../pluginmanager/OtmMemory.h"
#include "../utilities/LogWrapper.h"
#include "MemoryFactory.h"
#include "EQFMORPI.H"

// EQF.H is included by otmmemory.h
// #include <eqf.h>                  // General Translation Manager include file EQF:H 

extern "C"
{
#include <OTMFUNC.H>               // header file of OpenTM2 API functions
}

#include "vector"
#include <wctype.h>

#include "win_types.h"
#include "LogWrapper.h"
//#include "EQFFUZZ.H"
// prototypes for helper functions


/* \brief Initialize the Memory Plugin Mapper
*/


__declspec(dllexport)
void InitTMPluginWrapper()
{
  // access plugin manager
	// PluginManager* thePluginManager = PluginManager::getInstance();

}

/*
// callback function to insert memory names into a listbox
int AddMemToListBox( PVOID pvData, char *pszName, OtmMemoryPlugin::PMEMORYINFO pInfo  )
{
#if 0
  static char szMemName[MAX_LONGFILESPEC];

  HWND hwndLB = (HWND)pvData;

  pInfo;

  strcpy( szMemName, pszName );
  OEMTOANSI( szMemName );
  INSERTITEMHWND( hwndLB, szMemName );
#endif
  return( 0 );
}//*/



/*   Function Name:   FillMemoryListBox */
USHORT FillMemoryListBox
(
 PLISTCOMMAREA    pCommArea,
 WPARAM           mp1,           // contains the list box handle
 LPARAM           mp2            // contains the address to the object name
)

{
  BOOL              fOK = TRUE;      // Process return code

  pCommArea;

  // Check whether object name is correct
  if ( strcasecmp((const char *) PVOIDFROMMP2(mp2), MEMORY_ALL) )
  {
     fOK = FALSE;
  } /* endif */

  if ( fOK )
  {
    // Stop list box enabeling
    //ENABLEUPDATEHWND_FALSE( (HWND)mp1 );

    // GQ 2014/05/12: Use MemoryFactory to get the list of memories (old code has been commented out and left as reference in case of problems)

    // old code start
    //if ( fOK )
    //{
    //  SHORT  sItemCount;
    //  SHORT  sItemIndex;

    //  // The window handle seems to be Ok
    //  // Establish pointer to entry in TM list and to work string
    //  sItemCount = QUERYITEMCOUNTHWND( pCommArea->hwndLB );

    //  if ( sItemCount != LIT_ERROR )
    //  {
    //    for ( sItemIndex = 0; sItemIndex < sItemCount; sItemIndex++ )
    //    {
    //      if ( WinSendMsg( pCommArea->hwndLB, LM_EQF_QUERYITEMSTATE,
    //                       MP1FROMSHORT( sItemIndex ), NULL ) )
    //      {
    //         // Win2000 corrupts our data if we use LB_GETTEXT so use own message instead
    //  //         QUERYITEMTEXTHWND( pCommArea->hwndLB, sItemIndex, pCommArea->szBuffer );
    //  SendMessage( pCommArea->hwndLB, LM_EQF_QUERYITEMTEXT, (WPARAM)sItemIndex, (LPARAM)pCommArea->szBuffer );

    //  INSERTITEMHWND( (HWND) mp1, UtlParseX15( pCommArea->szBuffer, 0 ) );
    //        usItemRcCount++;
    //      } /* endif */
    //    } /* endfor */
    //  } /* endif */
    // old code end

      //MemoryFactory *pFactory = MemoryFactory::getInstance();
      //pFactory->listMemories( AddMemToListBox, (PVOID)mp1, FALSE );
      

      // Enable the memory database list box
      //ENABLEUPDATEHWND_TRUE( (HWND)mp1 );
  } /* endif */

  //return  ( QUERYITEMCOUNTHWND( ((HWND)mp1)) );
} /* end of function FillMemoryListBox */


/*
SHORT EQFNTMSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  pBTIda; pUserData; pusLen;

//  	return theMemory->CPP_EQFNTMSign(  pBTIda,  pUserData,  pusLen);
    return( 1 );

}//*/

USHORT NTMOrganizeIndexFile
(
  PTMX_CLB pTmClb               // ptr to control block,
)
{
  pTmClb;
//	return theMemory->CPP_NTMOrganizeIndexFile(  pTmClb);
    return( 1 );

}


/**********************************************************************/
/* macro to calculate the number of tokens in the list ...            */
/* and to adjust ends of tokens                                       */
/**********************************************************************/
#define  NUMBEROFTOKENS(usLenStr, pTokenList)   \
         {                                      \
           PFUZZYTOK pTest = pTokenList;        \
           usLenStr = 0;                        \
           while ( pTest->ulHash )              \
           {                                    \
             usLenStr++;                        \
             pTest++;                           \
           } /* endwhile */                     \
         }

USHORT TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
)
{
  htm; szMemPath; pstPutIn; ulUpdKey; usUpdTarget; usFlags; usMsgHandling;

//	return theMemory->CPP_TmUpdSegW(  htm,  szMemPath,  pstPutIn,  ulUpdKey,  usUpdTarget,  usFlags,  usMsgHandling);
    return( 1 );

}




// memory utility functions


PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADCompareKey( PSZ_W pszKey1, PSZ_W pzKey2 );
BOOL  MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 );
BOOL  MADNextAttr( PSZ_W *ppszAttr );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize );
PSZ_W MADSkipName( PSZ_W pszName );
PSZ_W MADSkipWhitespace( PSZ_W pszData );

#ifdef TEMPORARY_COMMENTED

//
// retrieve the data associated with a specific key
//
BOOL MDAGetValueForKey( HADDDATAKEY pKey, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fValue = FALSE;
  BOOL fAttrAvailable = FALSE;

  *pszBuffer = 0;

  // skip any attributes
  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );
  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
  } while ( fAttrAvailable ); /* enddo */

  if ( *pszAttr == L'>' )
  {
    PSZ_W pszValue = pszAttr + 1;
    while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'<') )
    {
      if ( *pszValue == L'&' )
      {
        if ( wcsncmp( pszValue, L"&lt;", 4 ) == 0 )
        {
          *pszBuffer++ = L'<';
          pszValue += 4;
          iBufSize--;
        }
        else if ( wcsncmp( pszValue, L"&gt;", 4 ) == 0 )
        {
          *pszBuffer++ = L'>';
          pszValue += 4;
          iBufSize--;
        }
        else if ( wcsncmp( pszValue, L"&amp;", 5 ) == 0 )
        {
          *pszBuffer++ = L'&';
          pszValue += 5;
          iBufSize--;
        }
        else
        {
          *pszBuffer++ = *pszValue++;
          iBufSize--;
        } /* endif */         
      }
      else
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      } /* endif */         
    }
    *pszBuffer = 0;
    fValue = TRUE;
  } /* endif */

  if ( !fValue && (pszDefault != NULL) ) wcscpy( pszBuffer, pszDefault );

  return( FALSE );
}

//
// Search a specific key in the additional memory data
//
HADDDATAKEY MADSearchKey( PSZ_W pAddData, PSZ_W pszKey )
{
  HADDDATAKEY pKey = NULL;
  if ( (pAddData != NULL) && (*pAddData != 0) )
  {
    while ( (pKey == NULL) && (*pAddData != 0) )
    {
      // move to begin of next key
      while ( (*pAddData != 0) && (*pAddData != L'<') ) pAddData++;

      // check key
      if ( (*pAddData == L'<')  )
      {
        if ( MADCompareKey( pAddData + 1, pszKey ) )
        {
          pKey = pAddData;
        }
        else
        {
          pAddData++;
        } /* endif */
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( pKey );
}
#endif

//
// Delete a key with all its atttributes and data
//
BOOL MADDeleteKey( HADDDATAKEY pKey )
{
  PSZ_W pszCurPos = pKey;
  BOOL fAttrAvailable = FALSE;
  PSZ_W pszTagName = NULL;
  PSZ_W pszTagNameEnd = NULL;

  // check key parameter
  if ( (pszCurPos == NULL) || (*pszCurPos == 0) )
    return( FALSE );

  // skip tag name
  if ( *pszCurPos == L'<' ) pszCurPos++;
  pszTagName = pszCurPos;
  pszTagNameEnd = pszCurPos = MADSkipName( pszCurPos );
  do
  {
    fAttrAvailable = MADNextAttr( &pszCurPos );
  } while ( fAttrAvailable ); /* enddo */

  // find end of key data
  if ( (*pszCurPos == L'/') &&  (*(pszCurPos+1) == L'>') )
  {
    // a selfcontained tag... so remove everything up to end of the tag
    PSZ_W pszSource = pszCurPos + 2;
    PSZ_W pszTarget = pKey;
    while ( *pszSource ) *pszTarget++ = *pszSource++;
    *pszTarget = 0;
    return( TRUE );
  }

  // find ending tag
  // note: this is a very simple approach which only looks for the ending tag and does not interpret the data in any way...
  while ( *pszCurPos != 0 )
  {
    // move to begin of next tag
    while ( (*pszCurPos != 0) && (*pszCurPos != L'<') ) *pszCurPos++;

    // check for end tag start
    if ( (*pszCurPos == L'<') &&  (*(pszCurPos+1) == L'/') )
    {
      // check if we have the correct end tag
      if ( MADCompareKey( pszCurPos + 2, pszTagName ) )
      {
        // end tag found, now skip end tag name and closing pointy brace
        pszCurPos = MADSkipName( pszCurPos + 2 );
        pszCurPos = MADSkipWhitespace( pszCurPos );
        if ( *pszCurPos == L'>' ) pszCurPos++;

        // remove all data between (and including) start tag and end tag
        PSZ_W pszSource = pszCurPos;
        PSZ_W pszTarget = pKey;
        while ( *pszSource ) *pszTarget++ = *pszSource++;
        *pszTarget = 0;

        // removal of tag has been completed
        return( TRUE );
      } /* endif */
    } /* endif */
    pszCurPos++;
  } /* endwhile */

  return( FALSE );
}

#ifdef TEMPORARY_COMMENTED

//
// get the value for a specific attribute
//
BOOL MADGetAttr( HADDDATAKEY pKey, PSZ_W pszAttrName, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fAttrAvailable = FALSE;

  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );

  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
    if ( fAttrAvailable )
    {
      if ( MADCompareAttr( pszAttr, pszAttrName ) )
      {
        MADGetAttrValue( pszAttr, pszBuffer, iBufSize );
        return( TRUE );
      } /* endif */
    } /* endif */

  } while ( fAttrAvailable ); /* enddo */
  if ( pszDefault != NULL ) wcscpy( pszBuffer, pszDefault );
  return( FALSE);
}

#endif //TEMPORARY_COMMENTED

// Add a match segment ID to the additional data section
BOOL MADAddMatchSegID( PSZ_W pszAddData, PSZ_W pszMatchIDPrefix, ULONG ulNum, BOOL fForce )
{
  HADDDATAKEY hKey;
  BOOL fMatchIDAdded = FALSE;

  // find any existing match segment ID
  hKey = MADSearchKey( pszAddData, MATCHSEGID_KEY );
  if ( (hKey != NULL) && fForce )
  {
    MADDeleteKey( hKey );
    hKey = NULL;
  } /* endif */

  // add match segment ID if non exists or the existing has been deleted and there is enough room left in the AddInfo buffer
  if ( hKey == NULL )
  {
    int iCurLen = wcslen(pszAddData);
    if ( (wcslen(pszMatchIDPrefix) + iCurLen + 23) < MAX_SEGMENT_SIZE )
    {
      swprintf( pszAddData + iCurLen, wcslen(pszAddData) - iCurLen, L"<MatchSegID ID=\"%s%lu\"/>", pszMatchIDPrefix, ulNum );
      fMatchIDAdded  = TRUE;
    } /* endif */
  } /* endif */
  return( fMatchIDAdded );
}

// prepare the match segment ID prefix using the provided TM_ID and StoreID
BOOL MADPrepareMatchSegIDPrefix( PSZ pszTM_ID, PSZ pszStoreID, PSZ pszMatchID )
{
  BOOL fMatchIDPrepared = FALSE;
  if ( (pszTM_ID != NULL) && (*pszTM_ID != EOS) )
  {
    fMatchIDPrepared = TRUE;

    while( *pszTM_ID != EOS )
    {
      if ( *pszTM_ID == '_' )
      { 
        *pszMatchID++ = '+';
      }
      else if ( (*pszTM_ID == '<') || (*pszTM_ID == '>') )
      { 
        *pszMatchID++ = '-';
      }
      else
      {
        *pszMatchID++ = *pszTM_ID;
      } /* endif */
      pszTM_ID++;
    } /* endwhile */
    *pszMatchID++ = '_';
  } /* endif */

  if ( (pszStoreID != NULL) && (*pszStoreID != EOS) )
  {
    fMatchIDPrepared = TRUE;

    while( *pszStoreID != EOS )
    {
      if ( *pszStoreID == '_' )
      { 
        *pszMatchID++ = '+';
      }
      else if ( (*pszStoreID == '<') || (*pszStoreID == '>') )
      { 
        *pszMatchID++ = '-';
      }
      else
      {
        *pszMatchID++ = *pszStoreID;
      } /* endif */
      pszStoreID++;
    } /* endwhile */
    *pszMatchID++ = '_';
  } /* endif */

  *pszMatchID = EOS;

  return( fMatchIDPrepared );
} /* end of MADPrepareMatchSegIDPrefix */


#ifdef TEMPORARY_COMMENTED

// ======== internal functions for Memory Additional Data processing



// position to next attribute
BOOL MADNextAttr( PSZ_W *ppszAttr )
{
  PSZ_W pszAttr = *ppszAttr;

  // if we are at an attribute already we skip this one
  if ( iswalpha( *pszAttr) )
  {
    pszAttr = MADSkipName( pszAttr );
    pszAttr = MADSkipAttrValue( pszAttr );
  } /* endif */

  pszAttr = MADSkipWhitespace( pszAttr );
  while ( *pszAttr == L' ' ) pszAttr++;

  // set caller's pointer
  *ppszAttr = pszAttr;

  return( iswalpha( *pszAttr ) );
}

// retrieve attribute value
BOOL MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize )
{
  PSZ_W pszValue = MADSkipName( pszAttr );
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'\"') )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( (iBufSize > 1) && iswalpha(*pszValue) || iswdigit(*pszValue) )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
    } /* endif */

  } /* endif */
  *pszBuffer = 0;

  return( TRUE );
}


// compare attribute names
BOOL MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszAttr1 != 0) && (*pszAttr2 != 0) && (*pszAttr1 == *pszAttr2) )
  {
    pszAttr1++; pszAttr2++;
  } /* endwhile */

  fMatch = ( ((*pszAttr1 == 0) || (*pszAttr1 == L' ') || (*pszAttr1 == L'>') || (*pszAttr1 == L'=')) &&
             ((*pszAttr2 == 0) || (*pszAttr2 == L' ') || (*pszAttr2 == L'>') || (*pszAttr2 == L'=')) );

  return( fMatch );
}


// compare two key names
BOOL MADCompareKey( PSZ_W pszKey1, PSZ_W pszKey2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszKey1 != 0) && (*pszKey2 != 0) && (*pszKey1 == *pszKey2) )
  {
    pszKey1++; pszKey2++;
  } /* endwhile */

  fMatch = ( ((*pszKey1 == 0) || (*pszKey1 == L' ') || (*pszKey1 == L'>')) && ((*pszKey2 == 0) || (*pszKey2 == L' ') || (*pszKey2 == L'>')) );

  return( fMatch );
}


// skip attribute value: loop until end of current attribute value
PSZ_W MADSkipAttrValue( PSZ_W pszValue )
{
  // pszValuze points to first character following the attribute name, eithere the name is followed
  // by an equal sign and the value or the attribute has no value at all
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (*pszValue != 0) && (*pszValue != L'\"') ) pszValue++;
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( iswalpha(*pszValue) || iswdigit(*pszValue) ) pszValue++;
    } /* endif */

  } /* endif */
  return( pszValue );
}


// skip name: loop until end of current name
PSZ_W MADSkipName( PSZ_W pszName )
{
  while ( iswalpha(*pszName) || iswdigit(*pszName) || *pszName==L':' ) pszName++;
  return( pszName );
}

// skip any whitespace: increase given pointer until non-blank character reached
PSZ_W MADSkipWhitespace( PSZ_W pszData )
{
  while ( *pszData == L' ' ) pszData++;
  return( pszData );
}



USHORT MemOpenProp( HPROP * hProp,        // pointer to property handle
                    PVOID * pProp,        // pointer to property pointer
                    PSZ     pszPropName,  // pointer to property name
                    PSZ     pszPropPath,  // pointer to property path
                    USHORT  usMode,       // open mode either PROP_READ or PROP_WRITE
                    BOOL    fMsg )        // Message flag
{
  /* This function opens properties for write or read access and sets property
     handle and pointer to the properties. fOk is TRUE if everything is Ok
     and FALSE if somthing went wrong. The function issues an error message
     in case of failur  */

  EQFINFO     ErrorInfo;                   // Property handler error info
  USHORT      fOk = TRUE;                  // Function return code

  if( (*hProp = OpenProperties( pszPropName,
                                 pszPropPath,
                                 PROP_ACCESS_READ, &ErrorInfo))== NULL)
  {
    fOk = FALSE;
  } /* endif */

  if ( fOk && usMode == PROP_ACCESS_WRITE )
  {
    if( !SetPropAccess( *hProp, PROP_ACCESS_WRITE))
      fOk = FALSE;
  } /* endif */

  if ( fOk )
  {
    if( (*pProp = MakePropPtrFromHnd( *hProp ))== NULL )
      fOk = FALSE;
  } /* endif */

  if ( !fOk && fMsg )
  {
    // The properties could not be accessed display error message
    UtlError( ERROR_OPEN_TM_PROPERTIES, MB_CANCEL, 1, &pszPropName, EQF_ERROR );
  } /* endif */
  return fOk;
} /* end of function MemOpenProp */

typedef struct _FILLTABLE_DATA
{
 CHAR      szPath[MAX_EQF_PATH];         // Work string   60 char
 CHAR      szFilePath[MAX_PATH144];      // Work string  144 char
 LANGUAGE  szCompare;                    // Work string to compare 20 char
}FILLTABLE_DATA, * PFILLTABLE_DATA;

/*-----------------------------------------------------------------------------
   Function Name:  MemFillTableLB

   Description:    This function fills a list boxes with all available
                   Markup languages, Noise lists, Source- or Target-
                   languages. Last used values may be used.

   Call:           USHORT MemFillTableLB
                   (
                     HWND   hListBox,
                     USHORT usBoxType,
                     PSZ    pszLastUsed
                   )

   Where:          - hListBox is the window handle of the list box
                     to be filled.
                   - usBoxType specifies the type of table to be searched
                     and type of listbox to be filled.
                     The following table types can be specified:
                      FORMAT_TABLE         (Markup language)
                      EXCLUSION_LIST       (Word list, Noise list)
                      SOURCE_LANGUAGES     (active source languages)
                      TARGET_LANGUAGES     (active target languages)
                      ALL_SOURCE_LANGUAGES (all possible source languages)
                      ALL_TARGET_LANGUAGES (all possible target languages)
                   - pszLastUsed specifies a pointer to a zero terminated
                     string which is the object name where the last used
                     values are stored. The last used values must be
                     stored in properties on the EQF system drive in
                     the structure MEM_LAST_USED This value can also
                     be a NULL string to indicate that no default values
                     should be used.

   Returns:        Number of entries found. 0 means no entry found or error.

   Example:        The following example fills a list box with the ID
                   DID_CD_FORMAT. The list box is part of a dialog box.
                   The list box will contain all available markup
                   language tables available in the system. The properties
                   szLastUsed will be searched for last used values.

                   MemFillTableLB( WinWindowFromID(hDlg, DID_CD_FORMAT),
                                   FORMAT_TABLE,
                                   szLastUsed);
  +---------------------------------------------------------------------------+
*/

USHORT MemFillTableLB( HWND   hListBox,
                       USHORT usBoxType,
                       PSZ    pszLastUsed
                       )
{
  USHORT           usRc = TRUE;           // Process control return code
  USHORT           usNumbOfItems = 0;     // Number of Items found
  SHORT            sSelectItem = 0;       // Index of item which should be selected
  PMEM_LAST_USED   pLastUsed = NULL;      // Pointer to the default values
  PFILLTABLE_DATA  pFillTableIDA  = NULL; // Pointer to FillTable Ida
  PSZ              pszPath = NULL;        // Pointer to pFillTableIDA->szPath
  PSZ              pszFilePath = NULL;    // Pointer to pFillTableIDA->szFilePath
  PSZ              pszCompare = NULL;     // Pointer to pFillTableIDA->szCompare
  HPROP            hProp = NULL;          // Property handle
  EQFINFO          ErrorInfo;             // Property handler error info

  //--- Disable the appropriate listbox
  //WinEnableWindow( hListBox, FALSE );
  //--- allocate storage for IDA
  usRc = (USHORT)UtlAlloc( (PVOID *) &pFillTableIDA, 0L, (LONG)sizeof( FILLTABLE_DATA ),
                   ERROR_STORAGE );
  if ( usRc )
  {

    // Assign pointer to full path of tables on primary drive
    pszPath    = UtlMakeEQFPath( pFillTableIDA->szPath, NULC, TABLE_PATH, NULL );
    strcat( pszPath, "\\");

    // Assign pointer to full path for DosFind....
    pszFilePath = pFillTableIDA->szFilePath;

    // Assign pointer to a LANGUAGE area and an area for the function GetLangFileLine
    pszCompare  = pFillTableIDA->szCompare;

    // Access the last used values if available from the property handler
    if ( pszLastUsed != NULL )
    {
      UtlMakeEQFPath( pszFilePath, NULC, SYSTEM_PATH, NULL );
      hProp = OpenProperties( pszLastUsed, pszFilePath,
                              PROP_ACCESS_READ, &ErrorInfo );
      // Address the property area
      if (hProp)
      {
        if ( (pLastUsed = (PMEM_LAST_USED) MakePropPtrFromHnd( hProp ))== NULL)
        {
          usRc = FALSE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // If usRc. Collect the list box entries
  // and put them in the appropriate list box
  if ( usRc )
  {
    // Pre-prepare the path to search the appropriate file(s)
    strcpy( pszFilePath, pszPath );
    switch ( usBoxType )
    {
      //-----------------------------------------------------------------------
      case FORMAT_TABLE:
      case EXCLUSION_LIST:
      case SOURCE_LANGUAGES:
      case TARGET_LANGUAGES:
      case ALL_SOURCE_LANGUAGES:
      case ALL_TARGET_LANGUAGES:
        /* at first check for eventual last used value */
        if ( pLastUsed )
        {
          switch ( usBoxType )
          {
            case FORMAT_TABLE:
              strcpy( pszCompare, pLastUsed->szFormat );
              break;
            case EXCLUSION_LIST:
              strcpy( pszCompare, pLastUsed->szExclusion );
              break;
            case SOURCE_LANGUAGES:
            case ALL_SOURCE_LANGUAGES:
              strcpy( pszCompare, pLastUsed->szSourceLang );
              break;
            case TARGET_LANGUAGES:
            case ALL_TARGET_LANGUAGES:
              strcpy( pszCompare, pLastUsed->szTargetLang );
              break;
          } /* endswitch */
        } /* endif */

        usNumbOfItems = UtlFillTableLB( hListBox, usBoxType );
        break;
    } /* end switch */

    // If usRc, Select last used item or top item
    if ( usRc && ( pszLastUsed != NULL ) )
    {
      //CBSEARCHSELECTHWND( sSelectItem, hListBox, pszCompare );
    } /* endif */
  } /* endif */

  // Close the last used value properties if they had been open
  if (hProp)
  {
    CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
  } /* endif */

  // Free the allocated storage
  UtlAlloc( (PVOID *) &pFillTableIDA, 0L, 0L, NOMSG );

  if ( !usRc )
  {
    usNumbOfItems = 0;
  } /* endif */

  // Enable the appropriate listbox again
  //WinEnableWindow( hListBox, TRUE );

  return usNumbOfItems;
} /* end of function MemFillTableLB */

#endif

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     NTMDocMatch                                              |
//+----------------------------------------------------------------------------+
//|Function call:     NTMDocMatch( pszDoc1ShortName, pszDoc1LongName,          |
//|                                pszDoc2ShortName, pszDoc2LongName );        |
//+----------------------------------------------------------------------------+
//|Description:       Check if the give documents are identical. If the        |
//|                   long document names are available the long names         |
//|                   are compare else the comparism is based on the           |
//|                   short names.                                             |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pszDoc1ShortName        ptr to short name of 1st doc |
//|                   PSZ pszDoc1LongName         ptr to long name of 1st doc  |
//|                   PSZ pszDoc2ShortName        ptr to short name of 2nd doc |
//|                   PSZ pszDoc2LongName         ptr to long name of 2nd doc  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL       TRUE  = the document names match              |
//|                              FALSE = the document names do not match       |
//+----------------------------------------------------------------------------+
BOOL NTMDocMatch
(
  PSZ pszDoc1ShortName,                // ptr to short name of 1st doc
  PSZ pszDoc1LongName,                 // ptr to long name of 1st doc
  PSZ pszDoc2ShortName,                // ptr to short name of 2nd doc
  PSZ pszDoc2LongName                  // ptr to long name of 2nd doc
)
{
  BOOL fMatch = FALSE;                 // function return code

  // check if we can compare the long names
  if ( (pszDoc1LongName != NULL) && (pszDoc1LongName[0] != EOS) &&
       (pszDoc2LongName != NULL) && (pszDoc2LongName[0] != EOS) )
  {
    // base the match on the long names
    fMatch = (strcmp( pszDoc1LongName, pszDoc2LongName ) == 0 );
  }
  else
  {
    // use the short names for the match
    fMatch = (strcmp( pszDoc1ShortName, pszDoc2ShortName ) == 0 );
  } /* endif */
  return( fMatch );
} /* end of function NTMDocMatch */

#ifdef TEMPORARY_COMMENTED

USHORT NTMTokenizeW( PVOID, PSZ_W, PULONG, PVOID *, USHORT, USHORT);

typedef struct _NTMGETMATCHLEVELDATA
{
  CHAR             szSegmentTagTable[MAX_EQF_PATH];        // tag table name for segment markup
  CHAR             szProposalTagTable[MAX_EQF_PATH];       // tag table name for proposal markup
  CHAR_W           szSegment[8096];                        // buffer for segment data
  BYTE             bTokenBuffer[40000];                    // buffer for tokenizer in replace match detection
                                                           // this buffer has to hold token entries with a size of 18 bytes each
                                                           // theoratically a segmet could contain 2047 tokens....
  BYTE             bBuffer[40000];                         // general purpose buffer, also used for fuzzy token table (18 byte per entry)
  CHAR             szLang1[MAX_LANG_LENGTH];               // language buffer 1
  CHAR             szLang2[MAX_LANG_LENGTH];               // language buffer 2
} NTMGETMATCHLEVELDATA, *PNTMGETMATCHLEVELDATA;

// static data of NTMGetMatchLevelData
static char szLastSourceLang[MAX_LANG_LENGTH] = "";
static char szLastTargetLang[MAX_LANG_LENGTH] = "";
static ULONG ulLastSrcOemCP = 0;
static ULONG ulLastTgtOemCP = 0;               

// build tag table path name
void NTMMakeTagTablename( PSZ pszMarkup, PSZ pszTagTableName )
{
  UtlMakeEQFPath( pszTagTableName, NULC, TABLE_PATH, NULL );
  strcat( pszTagTableName, BACKSLASH_STR );
  strcat( pszTagTableName, pszMarkup );
  strcat( pszTagTableName, EXT_OF_FORMAT );
  return;
}

void NTMCopyAndClean( PSZ_W pszTarget, PSZ_W pszSource )
{
  while ( *pszSource )
  {
    // ignore carriage returns 
    if ( *pszSource == L'\r' )
    {
      pszSource++;
      if ( (*pszSource != L'\n') )
      {
        *pszTarget++ = L'\n';
      } /* endif */
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /*endwhile */
  *pszTarget = 0;
}



// get match level function ( called by nonDDE API to evaluate the match level
USHORT NTMGetMatchLevel
( 
  PEQFSEGINFO      pSegment,           // pointer to segment info
  PEQFSEGINFO      pProposal,          // pointer to memory proposal info
  SHORT            *psMatchLevel,      // pointer to caller's match level field
  SHORT            *psMatchState,      // pointer to caller's match state field
  LONG             lOptions            // options to be used by the function 
)
{
  USHORT           usRC = 0;                     // function return code, 0 = OK
  PTMX_SENTENCE    pSentence = NULL;
  PTMX_SENTENCE    pMatchSentence = NULL;
  PTMX_SENTENCE    pMatchTarget = NULL;
  ULONG            ulParm = 0;                   // set here specific parameters such as RESPECT_CRLF
  BOOL             fStringEqual = FALSE;         // TRUE = strings are equal
  ULONG            ulSrcOemCP = 0;               // OEM codepage for source language
  ULONG            ulTgtOemCP = 0;               // OEM codepage for target language
  PTMX_SUBSTPROP   pSubstProp = NULL;            // data area for genreic inline tag replacment 
  SHORT            sLangID = 0;                  // morph. ID for source language
  USHORT           usFuzzy = 0;                  // buffer for computed fuziness
  BOOL             fTagTableEqual = FALSE;       // TRUE = tag tables are equal
  PNTMGETMATCHLEVELDATA pData = NULL;
  BOOL             fGenericTagReplacement = FALSE; // TRUE = apply generic inline tag replacement
  USHORT           usWords = 0;                  // number of words in segment
  BOOL             fSourceAndTargetEqual = FALSE; // TRUE = source and target of propsal are equal
  BOOL             fSourceSource = FALSE;        // TRUE = source and target language of propsal are equal


#ifdef MEASURETIME
  LARGE_INTEGER liStart;
  LARGE_INTEGER liPrepare;
  LARGE_INTEGER liTokenize;
  LARGE_INTEGER liGetLang;
  LARGE_INTEGER liMorphAct;
  LARGE_INTEGER liComp;
  LARGE_INTEGER liGetFuzzy;
  LARGE_INTEGER liGeneric;
  LARGE_INTEGER liReplace;
  DWORD dwStart, dwEnd;
#endif

#ifdef MATCHLEVEL_LOGGING
  FILE *hfLog = NULL;
#endif

#ifdef MATCHLEVEL_LOGGING
  {
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, BACKSLASH_STR );
    strcat( szLogFile, "MATCHLEVEL.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      PPROPSYSTEM pSysProp;             // ptr to EQF system properties
      pSysProp = MakePropPtrFromHnd( EqfQuerySystemPropHnd());

      fprintf( hfLog, "*** EqfGetMatchLevel ***\n" );
      fprintf( hfLog, "   segment               = \"%S\"\n", pSegment->szSource  );
      fprintf( hfLog, "   segment source lang   = \"%s\"\n", pSegment->szSourceLanguage  );
      fprintf( hfLog, "   proposal source       = \"%S\"\n", pProposal->szSource  );
      fprintf( hfLog, "   proposal target       = \"%S\"\n", pProposal->szTarget  );
      fprintf( hfLog, "   proposal source lang  = \"%s\"\n", pProposal->szSourceLanguage  );
      fprintf( hfLog, "   proposal target lang  = \"%s\"\n", pProposal->szTargetLanguage  );
      fprintf( hfLog, "   default target lang   = \"%s\" CP=%lu\n", pSysProp->szSystemPrefLang, pSysProp->ulSystemPrefCP );
    } /* endif */
  }
#endif

#ifdef MEASURETIME
  dwStart = GetTickCount();
  QueryPerformanceCounter( &liStart );
#endif

  // stop processing if languages do not match 
  if ( !usRC )
  {
    CHAR szLang1[MAX_LANG_LENGTH];
    CHAR szLang2[MAX_LANG_LENGTH];

    Utlstrccpy( szLang1, pSegment->szSourceLanguage, '(' );
    Utlstrccpy( szLang2, pProposal->szSourceLanguage, '(' );
    if ( strcasecmp( szLang1, szLang2 ) != 0 )
    {
      *psMatchState = NONE_MATCHSTATE;
      *psMatchLevel = 0;
      return( 0 );
    } /* endif */

    Utlstrccpy( szLang1, pSegment->szTargetLanguage, '(' );
    Utlstrccpy( szLang2, pProposal->szTargetLanguage, '(' );
    if ( strcasecmp( szLang1, szLang2 ) != 0 )
    {
      *psMatchState = NONE_MATCHSTATE;
      *psMatchLevel = 0;
      return( 0 );
    } /* endif */
  } /* endif */

  // allocate our data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pData, 0L, (LONG)sizeof(NTMGETMATCHLEVELDATA), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

  // allocate sentence structures
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchTarget );

  // allocate gerenic inline replacement tag data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;


  // check if generic inline tag replacement should be used
  if ( lOptions & NO_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched off using lOptions
    fGenericTagReplacement = FALSE;
  }
  else if ( lOptions & USE_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched on using lOptions
    fGenericTagReplacement = TRUE;
  }
  else
  {
    // base on setting in system properties
    PPROPSYSTEM pSysProp;             // ptr to EQF system properties
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    fGenericTagReplacement = !pSysProp->fNoGenericMarkup;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liPrepare );
#endif

  if ( strcmp( pSegment->szSourceLanguage, szLastSourceLang ) == 0 )
  {
    ulSrcOemCP = ulLastSrcOemCP;
  }
  else
  {
    ulLastSrcOemCP = ulSrcOemCP = GetLangOEMCP( pSegment->szSourceLanguage );
    strcpy( szLastSourceLang, pSegment->szSourceLanguage );
  } /* endif */

  if ( strcmp( pSegment->szTargetLanguage, pSegment->szSourceLanguage ) == 0 )
  {
    ulTgtOemCP = ulSrcOemCP;
    fSourceSource = TRUE;         // source and target languages are equal

  }
  else if ( strcmp( pSegment->szTargetLanguage, szLastTargetLang ) == 0 )
  {
    ulTgtOemCP = ulLastTgtOemCP;
  }
  else
  {
    ulLastTgtOemCP = ulTgtOemCP = GetLangOEMCP( pSegment->szTargetLanguage );
    strcpy( szLastTargetLang, pSegment->szTargetLanguage );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetLang );
#endif

  if ( !usRC )
  {
    fSourceAndTargetEqual = UTF16strcmp( pProposal->szSource, pProposal->szTarget ) == 0;

    // tokenize source segment and match, resulting in normalized string and tag table record
    NTMMakeTagTablename( pSegment->szMarkup, pData->szSegmentTagTable );
    NTMMakeTagTablename( pProposal->szMarkup, pData->szProposalTagTable );

    //  UTF16strcpy( pSentence->pInputString, pSegment->szSource );
    NTMCopyAndClean( pSentence->pInputString, pSegment->szSource );
    usRC = TokenizeSourceEx( NULL, pSentence, pData->szSegmentTagTable, pSegment->szSourceLanguage, TM_MAJ_VERSION, ulSrcOemCP );

    // UTF16strcpy( pMatchSentence->pInputString, pProposal->szSource );
    NTMCopyAndClean( pMatchSentence->pInputString, pProposal->szSource );
    if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchSentence, pData->szProposalTagTable, pProposal->szSourceLanguage, TM_MAJ_VERSION, ulSrcOemCP );
    if ( !fSourceAndTargetEqual)
    {
      UTF16strcpy( pMatchTarget->pInputString, pProposal->szTarget );
      if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchTarget, pData->szProposalTagTable, pProposal->szTargetLanguage, TM_MAJ_VERSION, ulTgtOemCP );
    } /* endif */

#ifdef MEASURETIME
    QueryPerformanceCounter( &liTokenize );
#endif

    if ( MorphGetLanguageID( pSegment->szSourceLanguage, &sLangID ) != 0 )
    {
      PSZ pszParm = pSegment->szSourceLanguage;
      usRC = ERROR_INV_LANGUAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */

#ifdef MEASURETIME
    QueryPerformanceCounter( &liMorphAct );
#endif
  }


  if ( !usRC )
  {
    fTagTableEqual = (strcasecmp( pSegment->szMarkup, pProposal->szMarkup ) == 0);

    // check if strings are equal
    if ( ulParm & GET_RESPECTCRLF )   
    {
	    BOOL fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 )== 0L);
	    fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
	    if (fStringEqual && !fRespectCRLFStringEqual)
	    {  
        // there is a LF difference!
        fStringEqual = fRespectCRLFStringEqual;
	    }
    }
    else
    { 
      fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
    } /* endif*/

#ifdef MEASURETIME
  QueryPerformanceCounter( &liComp );
#endif

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, "   fStringEqual          = %s\n", fStringEqual ? "true" : "false " );
    } /* endif */
#endif
  } /* endif*/

  // compute fuzziness
  if ( !usRC )
  {
    BOOL fFuzzynessOK;
    fFuzzynessOK = TMFuzzynessEx( pSegment->szMarkup, pSegment->szSource, pProposal->szSource, sLangID, &usFuzzy, ulSrcOemCP, &usWords );
    if ( usFuzzy != 100) fStringEqual = FALSE;
#ifdef MATCHLEVEL_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "  results of fuzziness test\n" );
    fprintf( hfLog, "   fFuzzynessOK          = %s\n", fFuzzynessOK ? "true" : "false " );
    fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
    fprintf( hfLog, "   ulSrcOemCP            = %lu\n", ulSrcOemCP );
    fprintf( hfLog, "   sLangID               = %d\n", sLangID );
  } /* endif */
#endif
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetFuzzy );
#endif

  // perform generic inline tag replacement
  if ( !fStringEqual && !usRC && fGenericTagReplacement )
  {
    BOOL fSubstAll = FALSE;

    // do not enter generic inline tag replacement when proposal does not contain any matches
    PTMX_TAGTABLE_RECORD pTagRecord = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
    if ( pTagRecord->ulRecordLen > sizeof(TMX_TAGTABLE_RECORD) )
    {
      strcpy( pSubstProp->szSourceTagTable, pSegment->szMarkup );
      strcpy( pSubstProp->szSourceLanguage, pSegment->szSourceLanguage);
//      UTF16strcpy( pSubstProp->szSource, pSegment->szSource );
      NTMCopyAndClean( pSubstProp->szSource, pSegment->szSource );
      pSubstProp->pTagsSource = (PTMX_TAGTABLE_RECORD)pSentence->pTagRecord;

//      UTF16strcpy( pSubstProp->szPropSource, pProposal->szSource );
      NTMCopyAndClean( pSubstProp->szPropSource, pProposal->szSource );
      pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

//      UTF16strcpy( pSubstProp->szPropTarget, pProposal->szTarget );
      NTMCopyAndClean( pSubstProp->szPropTarget, pProposal->szTarget );
      if ( fSourceAndTargetEqual )
      {
        pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
      }
      else
      {
        pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchTarget->pTagRecord;
      } /* endif */

      strcpy( pSubstProp->szPropTagTable, pProposal->szMarkup );
      strcpy( pSubstProp->szTargetLanguage, pProposal->szTargetLanguage );

		  fSubstAll =  NTMTagSubst( pSubstProp, ulSrcOemCP, ulTgtOemCP );
#ifdef MATCHLEVEL_LOGGING
      if ( hfLog )
      {
        fprintf( hfLog, "  generic inline tag replacement\n" );
        fprintf( hfLog, "   pSubstProp->szSource  = %S\n", pSubstProp->szSource);
        fprintf( hfLog, "   pSubstProp->szPropSource = %S\n", pSubstProp->szPropSource);
        fprintf( hfLog, "   pSubstProp->szPropTarget = %S\n", pSubstProp->szPropTarget);
        fprintf( hfLog, "   ulSrcOemCP            = %lu\n", ulSrcOemCP );
        fprintf( hfLog, "   ulTgtOemCP            = %lu\n", ulTgtOemCP );
        fprintf( hfLog, "   fSubstAll             = %s\n", fSubstAll ? "true" : "false " );
      } /* endif */
#endif

      if ( fSubstAll )
      {
        // recompute fuzziness
        BOOL fFuzzynessOK;
        fFuzzynessOK = TMFuzzynessEx( pSegment->szMarkup, pSegment->szSource, pSubstProp->szPropSource, sLangID, &usFuzzy, ulSrcOemCP , 0);
#ifdef MATCHLEVEL_LOGGING
        if ( hfLog )
        {
          fprintf( hfLog, "  results of fuzziness test after generic inline tag replacement\n" ); 
          fprintf( hfLog, "   fFuzzynessOK          = %s\n", fFuzzynessOK ? "true" : "false " );
          fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
        } /* endif */
#endif
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGeneric );
#endif

  // return result to caller
  if ( !usRC )
  {
    *psMatchLevel = (SHORT)usFuzzy;
#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
    if ( usFuzzy == 100 )
    {
      // check type of exact match
      if ( (pSegment->lSegNumber == pProposal->lSegNumber ) && 
            (strcasecmp( pSegment->szDocument, pProposal->szDocument ) == 0) )
      {
        *psMatchState = EXACTEXACT_MATCHSTATE;
      }
      else
      {
        *psMatchState = EXACT_MATCHSTATE;
      } /* endif */
#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " after setting match state\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
    }
    else
    {
      // do checking for a replacement match
      USHORT usReplMatch = 0;             // buffer for replace flag
      PVOID  pvTagTable = NULL;
      SHORT  sTgtLangID = 0;               // morph. ID for target language
      USHORT usNewFuzzy = 0;

      usRC = TALoadTagTableHwnd( pSegment->szMarkup, (PLOADEDTABLE *)&pvTagTable, FALSE, TRUE, HWND_FUNCIF );

      if ( !usRC )
      {
        if ( strcmp( pProposal->szTargetLanguage, pProposal->szSourceLanguage ) == 0 )
        {
          sTgtLangID = sLangID;
        }
        else if ( MorphGetLanguageID( pProposal->szTargetLanguage, &sTgtLangID ) != 0 )
        {
          PSZ pszParm = pProposal->szTargetLanguage;
          usRC = ERROR_INV_LANGUAGE;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " before prepare Fuzzy prop\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
      // EQFBPrepareFuzzyProp modifies the target so we have to use
      // a temporary copy of the target segment
      if ( !usRC )
      {
        UTF16strcpy( pData->szSegment, pProposal->szTarget );
        usReplMatch = EQFBPrepareFuzzyPropEx2(
          pSegment->szSource,                        // pointer to source
          pProposal->szSource,                       // pointer to source of proposal
          pData->szSegment,                          // pointer to Translation
          sLangID,                                   // source language Id..
          sTgtLangID,                                // target language id..
          &usNewFuzzy,                               // we are looking for fuzzy level
          pvTagTable,                                // source document tag table
          pvTagTable,                                // target document tag table
          pData->bBuffer,                            // buffer for input
          pData->bTokenBuffer,                       // buffer for tokens
          ulSrcOemCP,
          ulTgtOemCP);
      } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, " after prepare Fuzzy prop\n" );
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
    } /* endif */
#endif
      if ( pvTagTable ) TAFreeTagTable( (PLOADEDTABLE)pvTagTable );

      if ( usReplMatch == PROP_REPLACED )
      {
        *psMatchState = REPLACE_MATCHSTATE;  
      }
      else if ( usReplMatch == PROP_REPLACED_FUZZY )
      {
        *psMatchState = FUZZYREPLACE_MATCHSTATE;
      }
      else 
      {
        // check fuzziness
        LONG lFuzzinessThreshHold = 5000;

        if ( usWords < 5 )
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL  );
        }
        else if ( usWords >= 15 )
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL );
        }
        else
        {
          lFuzzinessThreshHold = (LONG)UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL );
        } /* endif */

        if ( (usFuzzy * 100) >= lFuzzinessThreshHold )
        {
          *psMatchState = FUZZY_MATCHSTATE;
        }
        else
        {
          *psMatchState = NONE_MATCHSTATE;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MATCHLEVEL_LOGGING
    if ( hfLog )
    {
      fprintf( hfLog, "  returning results to caller\n" ); 
      fprintf( hfLog, "   usFuzzy               = %u\n", usFuzzy );
      fprintf( hfLog, "   *psMatchLevel         = %d\n", *psMatchLevel );
      fprintf( hfLog, "   *psMatchState         = %d\n", *psMatchState  );
    } /* endif */
#endif

#ifdef MEASURETIME
  QueryPerformanceCounter( &liReplace );
  dwEnd = GetTickCount();
#endif

#ifdef MEASURETIME
  {
    CHAR szLogFile[MAX_EQF_PATH];
    FILE *hfLog = NULL;

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\MATCHLEVELTIME.LOG" );

    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "NtmGetMatchLevel time log\n" );
      fprintf( hfLog, "Segment = \"%S\"\n", pSegment->szSource );
      fprintf( hfLog, "MemSeg  = \"%S\"\n", pProposal->szSource );
      LogWritePerfTime( hfLog, &liStart, &liPrepare, "Preparation" );
      LogWritePerfTime( hfLog, &liPrepare, &liGetLang, "GetLangID" );
      LogWritePerfTime( hfLog, &liGetLang, &liTokenize, "Tokenize" );
      LogWritePerfTime( hfLog, &liGetLang, &liMorphAct, "MorphAct" );
      LogWritePerfTime( hfLog, &liMorphAct, &liComp, "StringCompare" );
      LogWritePerfTime( hfLog, &liComp, &liGetFuzzy, "Fuzziness" );
      LogWritePerfTime( hfLog, &liGetFuzzy, &liGeneric, "GenericITR" );
      LogWritePerfTime( hfLog, &liGeneric, &liReplace, "ReplaceMatch" );
      LogWritePerfTime( hfLog, &liStart, &liReplace, "" );

      fprintf( hfLog, "Total time using GetTickCount : %ld ms\n", (long)(dwEnd - dwStart) );

      fclose( hfLog );
    } /* endif */
  }
#endif

  // cleanup
  NTMFreeSubstProp( pSubstProp );
  NTMFreeSentenceStructure( pSentence );
  NTMFreeSentenceStructure( pMatchSentence );
  NTMFreeSentenceStructure( pMatchTarget );
  if ( pData ) UtlAlloc( (PVOID*)&pData, 0L, 0L, NOMSG);

#ifdef MATCHLEVEL_LOGGING
  if ( hfLog )
  {
    fprintf( hfLog, "*** end of EqfGetMatchLevel, rc = %u ***\n", usRC );
    fclose( hfLog );
  } /* endif */
#endif

  return( usRC );
} /* end of function NTMGetMatchLevel */ 

SHORT NTMSimpleGetMatchLevel
(
  WCHAR            *pszSegmentText,    // segment text
  WCHAR            *pszProposalText,   // memory proposal text
  PSZ              pszLanguage,        // source language of segment and memory proposal
  PSZ              pszSegmentMarkup,   // markup table name of segment
  PSZ              pszProposalMarkup,  // markup table name of memory proposal
  LONG             lOptions,           // options to be used by the API call
  PVOID            *pvData            // adress of our data area pointer 

)
{
  USHORT           usRC = 0;                     
  PTMX_SENTENCE    pSentence = NULL;
  PTMX_SENTENCE    pMatchSentence = NULL;
  ULONG            ulParm = 0;                   // set here specific parameters such as RESPECT_CRLF
  BOOL             fStringEqual = FALSE;         // TRUE = strings are equal
  ULONG            ulSrcOemCP = 0;               // OEM codepage for source language
  ULONG            ulTgtOemCP = 0;               // OEM codepage for target language
  SHORT            sLangID = 0;                  // morph. ID for source language
  USHORT           usFuzzy = 0;                  // buffer for computed fuziness
  BOOL             fTagTableEqual = FALSE;       // TRUE = tag tables are equal
  PNTMGETMATCHLEVELDATA pData = NULL;
  BOOL             fGenericTagReplacement = FALSE; // TRUE = apply generic inline tag replacement
  USHORT           usWords = 0;                  // number of words in segment
  BOOL             fSourceAndTargetEqual = FALSE; // TRUE = source and target of propsal are equal
  BOOL             fSourceSource = FALSE;        // TRUE = source and target language of propsal are equal


#ifdef MEASURETIME
  LARGE_INTEGER liStart;
  LARGE_INTEGER liPrepare;
  LARGE_INTEGER liTokenize;
  LARGE_INTEGER liGetLang;
  LARGE_INTEGER liMorphAct;
  LARGE_INTEGER liComp;
  LARGE_INTEGER liGetFuzzy;
  LARGE_INTEGER liGeneric;
  LARGE_INTEGER liReplace;
  DWORD dwStart, dwEnd;
#endif
  
  pvData;

#ifdef MEASURETIME
  dwStart = GetTickCount();
  QueryPerformanceCounter( &liStart );
#endif

  // allocate our data area
  if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pData, 0L, (LONG)sizeof(NTMGETMATCHLEVELDATA), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

  // allocate sentence structures
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pSentence );
  if ( !usRC ) usRC = NTMAllocSentenceStructure( &pMatchSentence );

  // stop processing if languages do not match 
  if ( !usRC )
  {
    Utlstrccpy( pData->szLang1, pszLanguage, '(' );
    Utlstrccpy( pData->szLang2, pszLanguage, '(' );
  } /* endif */

  // check if generic inline tag replacement should be used
  if ( lOptions & NO_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched off using lOptions
    fGenericTagReplacement = FALSE;
  }
  else if ( lOptions & USE_GENERIC_INLINETAG_REPL_OPT )
  {
    // switched on using lOptions
    fGenericTagReplacement = TRUE;
  }
  else
  {
    // base on setting in system properties
    PPROPSYSTEM pSysProp;             // ptr to EQF system properties
    pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
    fGenericTagReplacement = !pSysProp->fNoGenericMarkup;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liPrepare );
#endif

  if ( strcmp( pszLanguage, szLastSourceLang ) == 0 )
  {
    ulSrcOemCP = ulLastSrcOemCP;
  }
  else
  {
    ulLastSrcOemCP = ulSrcOemCP = GetLangOEMCP( pszLanguage );
    strcpy( szLastSourceLang, pszLanguage );
  } /* endif */

    ulTgtOemCP = ulSrcOemCP;
    fSourceSource = TRUE;         // source and target languages are equal

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetLang );
#endif

  fSourceAndTargetEqual = TRUE;

  // tokenize source segment and match, resulting in normalized string and tag table record
  NTMMakeTagTablename( pszSegmentMarkup, pData->szSegmentTagTable );
  NTMMakeTagTablename( pszProposalMarkup, pData->szProposalTagTable );

  UTF16strcpy( pSentence->pInputString, pszSegmentText );
  usRC = TokenizeSourceEx( NULL, pSentence, pData->szSegmentTagTable, pszLanguage, TM_MAJ_VERSION, ulSrcOemCP );
  UTF16strcpy( pMatchSentence->pInputString, pszProposalText );
  if ( !usRC ) usRC = TokenizeSourceEx( NULL, pMatchSentence, pData->szProposalTagTable, pszLanguage, TM_MAJ_VERSION, ulSrcOemCP );

#ifdef MEASURETIME
  QueryPerformanceCounter( &liTokenize );
#endif

  if ( MorphGetLanguageID( pszLanguage, &sLangID ) != 0 )
  {
    PSZ pszParm = pszLanguage;
    usRC = ERROR_INV_LANGUAGE;
    UtlErrorHwnd( ERROR_INV_LANGUAGE, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liMorphAct );
#endif


  fTagTableEqual = (strcasecmp( pszSegmentMarkup, pszProposalMarkup ) == 0);

  // check if strings are equal
  if ( ulParm & GET_RESPECTCRLF )   
  {
	  BOOL fRespectCRLFStringEqual = (UtlCompIgnSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 )== 0L);
	  fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
	  if (fStringEqual && !fRespectCRLFStringEqual)
	  {  
      // there is a LF difference!
      fStringEqual = fRespectCRLFStringEqual;
	  }
  }
  else
  { 
    fStringEqual = (UtlCompIgnWhiteSpaceW( pMatchSentence->pNormString, pSentence->pNormString, 0 ) == 0L);
  } /* endif*/

#ifdef MEASURETIME
  QueryPerformanceCounter( &liComp );
#endif

  // compute fuzziness
  if ( !usRC )
  {
    BOOL fFuzzynessOK;
    fFuzzynessOK = TMFuzzynessEx( pszSegmentMarkup, pszSegmentText, pszProposalText, sLangID, &usFuzzy, ulSrcOemCP, &usWords );
    if ( usFuzzy != 100) fStringEqual = FALSE;
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGetFuzzy );
#endif

  // perform generic inline tag replacement
  if ( !fStringEqual && !usRC && fGenericTagReplacement )
  {
    BOOL fSubstAll = FALSE;

    // do not enter generic inline tag replacement when proposal does not contain any matches
    PTMX_TAGTABLE_RECORD pTagRecord = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;
    if ( pTagRecord->ulRecordLen > sizeof(TMX_TAGTABLE_RECORD) )
    {
      PTMX_SUBSTPROP   pSubstProp = NULL;            // data area for genreic inline tag replacment 

      // allocate gerenic inline replacement tag data area
      if ( !usRC ) usRC = (UtlAlloc( (PVOID*)&pSubstProp, 0L, (LONG)sizeof(TMX_SUBSTPROP), NOMSG)) ? 0 : ERROR_NOT_ENOUGH_MEMORY;

      strcpy( pSubstProp->szSourceTagTable, pszSegmentMarkup );
      strcpy( pSubstProp->szSourceLanguage, pszLanguage);
      UTF16strcpy( pSubstProp->szSource, pszSegmentText );
      pSubstProp->pTagsSource = (PTMX_TAGTABLE_RECORD)pSentence->pTagRecord;

      UTF16strcpy( pSubstProp->szPropSource, pszProposalText );
      pSubstProp->pTagsPropTarget = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

      pSubstProp->pTagsPropSource = (PTMX_TAGTABLE_RECORD)pMatchSentence->pTagRecord;

      strcpy( pSubstProp->szPropTagTable, pszProposalMarkup );
      strcpy( pSubstProp->szTargetLanguage, pszLanguage );

		  fSubstAll =  NTMTagSubst( pSubstProp, ulSrcOemCP, ulTgtOemCP );

      if ( fSubstAll )
      {
        // recompute fuzziness
        BOOL fFuzzynessOK;
        fFuzzynessOK = TMFuzzynessEx( pszSegmentMarkup, pSubstProp->szPropSource, pSubstProp->szSource, sLangID, &usFuzzy, ulSrcOemCP , 0);
      } /* endif */

      NTMFreeSubstProp( pSubstProp );
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liGeneric );
#endif

  // return result to caller
  if ( !usRC )
  {
    if ( usFuzzy == 100 )
    {
    }
    else
    {
      // do checking for a replacement match
      USHORT usReplMatch = 0;             // buffer for replace flag
      PVOID  pvTagTable = NULL;
      SHORT  sTgtLangID = 0;               // morph. ID for target language
      USHORT usNewFuzzy = 0;

      usRC = TALoadTagTableHwnd( pszSegmentMarkup, (PLOADEDTABLE *)&pvTagTable, FALSE, TRUE, HWND_FUNCIF );

      if ( !usRC )
      {
        sTgtLangID = sLangID;
      } /* endif */

      // EQFBPrepareFuzzyProp modifies the target so we have to use
      // a temporary copy of the target segment
      if ( !usRC )
      {
        UTF16strcpy( pData->szSegment, pszProposalText );
        usReplMatch = EQFBPrepareFuzzyPropEx2(
          pszSegmentText,                        // pointer to source
          pszProposalText,                       // pointer to source of proposal
          pData->szSegment,                          // pointer to Translation
          sLangID,                                   // source language Id..
          sTgtLangID,                                // target language id..
          &usNewFuzzy,                               // we are looking for fuzzy level
          pvTagTable,                                // source document tag table
          pvTagTable,                                // target document tag table
          pData->bBuffer,                            // buffer for input
          pData->bTokenBuffer,                       // buffer for tokens
          ulSrcOemCP,
          ulTgtOemCP);
      } /* endif */

      if ( pvTagTable ) TAFreeTagTable( (PLOADEDTABLE)pvTagTable );

      if ( usReplMatch == PROP_REPLACED )
      {
        usFuzzy = usNewFuzzy;
      }
      else if ( usReplMatch == PROP_REPLACED_FUZZY )
      {
        usFuzzy = usNewFuzzy;
      } /* endif */
    } /* endif */
  } /* endif */

#ifdef MEASURETIME
  QueryPerformanceCounter( &liReplace );
  dwEnd = GetTickCount();
#endif

#ifdef MEASURETIME
  {
    CHAR szLogFile[MAX_EQF_PATH];
    FILE *hfLog = NULL;

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    UtlMkDir( szLogFile, 0L, FALSE );
    strcat( szLogFile, "\\MATCHLEVELTIME.LOG" );

    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "NtmSimpleGetMatchLevel time log\n" );
      fprintf( hfLog, "Segment = \"%S\"\n", pszSegmentText );
      fprintf( hfLog, "MemSeg  = \"%S\"\n", pszProposalText );
      fprintf( hfLog, "MatchLevel = %u\n", usFuzzy );
      LogWritePerfTime( hfLog, &liStart, &liPrepare, "Preparation" );
      LogWritePerfTime( hfLog, &liPrepare, &liGetLang, "GetLangID" );
      LogWritePerfTime( hfLog, &liGetLang, &liTokenize, "Tokenize" );
      LogWritePerfTime( hfLog, &liGetLang, &liMorphAct, "MorphAct" );
      LogWritePerfTime( hfLog, &liMorphAct, &liComp, "StringCompare" );
      LogWritePerfTime( hfLog, &liComp, &liGetFuzzy, "Fuzziness" );
      LogWritePerfTime( hfLog, &liGetFuzzy, &liGeneric, "GenericITR" );
      LogWritePerfTime( hfLog, &liGeneric, &liReplace, "ReplaceMatch" );
      LogWritePerfTime( hfLog, &liStart, &liReplace, "" );

      fprintf( hfLog, "Total time using GetTickCount : %ld ms\n", (long)(dwEnd - dwStart) );

      fclose( hfLog );
    } /* endif */
  }
#endif

  // cleanup
  NTMFreeSentenceStructure( pSentence );
  NTMFreeSentenceStructure( pMatchSentence );
  if ( pData ) UtlAlloc( (PVOID*)&pData, 0L, 0L, NOMSG);

  return( (usRC == 0) ? (SHORT)usFuzzy : ((SHORT)usRC * -1) );
}

SHORT NTMSimpleGetMatchLevelCleanup
(
  PVOID            *pvData             // adress of our data area pointer 
)
{
  pvData;

  return( 0 );
}


//static  ULONG  ulRandom[MAX_RANDOM];   // random sequence




//static 
VOID MakeHashValue ( PULONG, USHORT, PSZ_W, PULONG );

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SplitTokens                                              |
//+----------------------------------------------------------------------------+
//|Function call:     SplitTokens(PFUZZYTOK, USHORT, USHORT, USHORT, PSZ)      |
//+----------------------------------------------------------------------------+
//|Description:       build pstCurrent and split token if it contains          |
//|                   digits or punctuations                                   |
//+----------------------------------------------------------------------------+
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PFUZZYTOK                                                |
//+----------------------------------------------------------------------------+
//|Returncodes:       pstCurrent  points to next free position                 |
//+----------------------------------------------------------------------------+
//|Function flow:     splitting of tokens added on request,                    |
//|                   now deleted again (July 93)                              |
//+----------------------------------------------------------------------------+

//static 
PFUZZYTOK SplitTokens
 (
  PFUZZYTOK  pstCurrent,
  USHORT     usStart,
  SHORT      sType,
  USHORT     usLength,
  PSZ_W      pString
 )
{
  CHAR_W    chTemp;
  pstCurrent->pData = pString;
  pstCurrent->usStart = usStart;
  pstCurrent->sType   = sType;
  pstCurrent->usStop  = usStart + usLength - 1;
  pstCurrent->fConnected = FALSE;
  chTemp = *(pString + usLength);
  *(pString + usLength) = EOS;
  MakeHashValue ( ulRandom, MAX_RANDOM,
                  pString , &pstCurrent->ulHash );
  *(pString + usLength) = chTemp;
  pstCurrent++;

  return (pstCurrent);
} /* end of function SplitTokens */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PrepareTokens                                            |
//+----------------------------------------------------------------------------+
//|Function call:     fOK = PrepareTokens( pDoc, pString, sId, &pTokList );    |
//+----------------------------------------------------------------------------+
//|Description:       prepare the token list and build hash values for them    |
//+----------------------------------------------------------------------------+
//|Parameters:        PVOID         pTagTable      ptr to tagtable             |
//|                   PBYTE         pInBuf         temp work buffer            |
//|                   PBYTE         pTokBuf        token buffer                |
//|                   PSZ           pString,       ptr to string               |
//|                   SHORT         sLanguageId,   language ID                 |
//|                   PFUZZYTOK   * ppTransTokList resulting list of tokens    |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE     everything okay                                 |
//|                   FALSE    allocation went wrong                           |
//+----------------------------------------------------------------------------+
//|Function flow:     if not done yet                                          |
//|                     prepare random sequence (according to K.Wirth)         |
//|                   call EQFTagTokenize to tokenize the passed string        |
//|                   build tokenlist, i.e. convert tokens/text strings to     |
//|                      entries in start-stop table                           |
//|                      (Note: for text tokens returned we have to call       |
//|                             MorphTokenize to split them up further )       |
//|                   allocate memory for start/stop table and copy list into  |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
//static 
BOOL
PrepareTokens
(
  PLOADEDTABLE  pTagTable,
  PBYTE         pInBuf,
  PBYTE         pTokBuf,
  PSZ_W         pString,               // pointer to string to be tokenized
  SHORT         sLanguageId,           // language ID
  PFUZZYTOK   * ppTransTokList,         // resulting list of tokens
  ULONG         ulOemCP
)
{
  PCHAR_W     pRest;                   // ptr to start of not-processed bytes
  USHORT      usColPos = 0;            // column pos used by EQFTagTokenize
  PTOKENENTRY pTok;                    // ptr for token table processing
  USHORT      usCurOffs;               // current offset into segment
  ULONG       ulLength;                // length of start/stop table
  PFUZZYTOK   pstCurrent;              // ptr to entries of start/stop table
  CHAR_W      chTemp;                  // temp character
  BOOL        fOK = TRUE;              // success indicator
  USHORT      usRandomIndex;           // index in random sequence
  USHORT      usListSize;              // size of buffer
  PTERMLENOFFS  pTermList;             // ptr to created term list
  PTERMLENOFFS  pActTerm;              // actual term
  USHORT       usRC;                   // return code
  SHORT        sNumTags;               // number of tags       /* KIT0857A    */
  BOOL         fTag;                   // currently in tagging /* KIT0857A    */
  PFUZZYTOK    pstAct;                 // ptr start/stop table /* KIT0857A    */
  PSZ_W        pStart;

   if ( !ulRandom[0] )
   {
     /********************************************************************/
     /* random sequences, see e.g. the book of Wirth...                  */
     /********************************************************************/
     ulRandom[0] = 0xABCDEF01;
     for (usRandomIndex = 1; usRandomIndex < MAX_RANDOM; usRandomIndex++)
     {
         ulRandom[usRandomIndex] = ulRandom[usRandomIndex - 1] * 5 + 0xABCDEF01;
     }
   } /* endif */
  /********************************************************************/
  /* run TATagTokenize to find tokens ....                            */
  /********************************************************************/
  TATagTokenizeW(pString,
                 pTagTable,
                 TRUE,
                 &pRest,
                 &usColPos,
                 (PTOKENENTRY) pTokBuf,
                 TOK_BUFFER_SIZE / sizeof(TOKENENTRY) );

  /********************************************************************/
  /* build tokenlist, i.e.                                            */
  /* convert tokens/text strings to entries in start-stop table       */
  /*                                                                  */
  /* Rational: use input buffer for temporary list ....               */
  /*           this is large enough, we can avoid any checking...     */
  /********************************************************************/
  pstCurrent = (PFUZZYTOK) pInBuf;  // use input buffer for table
  pTok = (PTOKENENTRY) pTokBuf;
  usCurOffs = 0;
  pStart = pTok->pDataStringW;
  while ( (pTok->sTokenid != ENDOFLIST) )
  {
    if ( pTok->sTokenid == TEXT_TOKEN )
    {
      usListSize = 0;
      pTermList = NULL;

      chTemp = *(pTok->pDataStringW+pTok->usLength);
      *(pTok->pDataStringW+pTok->usLength) = EOS;

      usRC = MorphTokenizeW( sLanguageId, pTok->pDataStringW,
                            &usListSize, (PVOID *)&pTermList,
                            MORPH_OFFSLIST, ulOemCP );

      *(pTok->pDataStringW+pTok->usLength) = chTemp;

      if ( pTermList )
      {
        pActTerm = pTermList;
        if ( pTermList->usLength )
        {
          while ( pActTerm->usLength )
          {
            pString = pTok->pDataStringW + pActTerm->usOffset;
            /**********************************************************/
            /* ignore the linefeeds and tabs in the matching          */
            /**********************************************************/
            if ( (*pString != LF) && (*pString != 0x09))
            {
              pstCurrent = SplitTokens(pstCurrent,
                                       (USHORT)(usCurOffs + pActTerm->usOffset),
                                       TEXT_TOKEN,
                                       pActTerm->usLength,
                                       pString);
            } /* endif */
            pActTerm++;
          } /* endwhile */
        }
        else
        {
          pstCurrent = SplitTokens(pstCurrent,
                                   usCurOffs,
                                   pTok->sTokenid,
                                   pTok->usLength,
                                   pTok->pDataStringW);

        } /* endif */
      } /* endif */
      /****************************************************************/
      /* free allocated resource ...                                  */
      /****************************************************************/
      UtlAlloc( (PVOID *)&pTermList, 0L, 0L, NOMSG );
    }
    else
    {
      pstCurrent =  SplitTokens(pstCurrent,
                               usCurOffs,
                               pTok->sTokenid,
                               pTok->usLength,
                               pTok->pDataStringW);
    } /* endif */
    /****************************************************************/
    /* adjust current offset to point to new offset in string...    */
    /****************************************************************/
    pTok++;
    usCurOffs = (USHORT)(pTok->pDataStringW - pStart);
  } /* endwhile */

  // terminate start/stop table
  memset( pstCurrent, 0, sizeof(  FUZZYTOK ));


  /********************************************************************/
  /* get number of tags ...                                           */
  /********************************************************************/
  sNumTags = (SHORT) pTagTable->pTagTable->uNumTags;
  /********************************************************************/
  /* streamline tokenlist, i.e. treat tags and their attributes as    */
  /* one unit                                                         */
  /*    pstAct -- points to active one                                */
  /*    pstCurrent  -- points to next one...                          */
  /********************************************************************/
  pstCurrent = (PFUZZYTOK) pInBuf;         // use input buffer for table
  pstAct = pstCurrent;
  /********************************************************************/
  /* set tag indication                                               */
  /********************************************************************/
  fTag = ((pstCurrent->sType < sNumTags ) &&
          (pstCurrent->sType >= 0));
  pstCurrent++;                                  // point to next entry

  while ( pstCurrent->usStop )
  {
    if ( pstCurrent->sType < sNumTags )          // tags or text;
    {
      fTag = ( pstCurrent->sType >= 0 );
      pstAct++;
      memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
      pstCurrent++;
    }
    else                                        // it is an attribute
    {
      /****************************************************************/
      /* if we are in a tag -- join it, i.e. update the length        */
      /****************************************************************/
      if ( fTag )
      {
        pstAct->usStop = pstCurrent->usStop;
        pstAct->ulHash += pstCurrent->ulHash;
        pstCurrent++;
      }
      else
      {
        pstAct++;
        memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
        pstCurrent++;
      } /* endif */
    } /* endif */
  } /* endwhile */
  /********************************************************************/
  /* put in end indication                                            */
  /********************************************************************/
  pstAct++;
  memcpy( pstAct, pstCurrent, sizeof( FUZZYTOK ));
  /********************************************************************/
  /* have to point to next one, because we use it as length parameter */
  /********************************************************************/
  pstAct++;

  /********************************************************************/
  /* allocate memory for start/stop table and copy list into it       */
  /********************************************************************/
  *ppTransTokList = NULL;              // init token list..
  ulLength = (ULONG)(((PBYTE) pstAct) - pInBuf);

  fOK = UtlAlloc( (PVOID *)ppTransTokList, 0L, ulLength, ERROR_STORAGE );
  if ( fOK )
  {
    memcpy( *ppTransTokList, pInBuf, ulLength );
  } /* endif */

  return( fOK );
} /* end of function 
  */
#endif

#ifdef TEMPORARY_COMMENTED
//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBCallLCS                                              |
//+----------------------------------------------------------------------------+
//|Function call:     EQFBCallLCS     ( PFUZZYTOK,PFUZZYTOK,PREPLLIST)         |
//+----------------------------------------------------------------------------+
//|Description:       fill structure needed in LCS and call LCS                |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK    pPropTokList,   pointer to token list       |
//|                   PFUZZYTOK    pTransTokList,  pointer to token list       |
//|                   PREPLLIST  * ppReplaceList   pointer to replace list     |
//|                   BOOL         fCompareAll     TRUE is strncmp nec.        |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       1       success                                          |
//|                   0       error                                            |
//+----------------------------------------------------------------------------+
//|Function flow:      set up pointer array                                    |
//|                    return success indicator                                |
//+----------------------------------------------------------------------------+
BOOL EQFBCallLCS
(
  PFUZZYTOK    pTokenList1,             // ptr to token list1
  PFUZZYTOK    pTokenList2,                      // ptr to token list2
  USHORT       usLenStr1,
  USHORT       usLenStr2,
  BOOL         fCompareAll
)
{
  BOOL         fOK = TRUE;             // success indicator
  PFUZZYTOK    pTestToken;
  LCSTOKEN     LCSString1;
  LCSTOKEN     LCSString2;
  PFUZZYTOK    pBackList1;
  PFUZZYTOK    pBackList2 = NULL;
  SHORT        k;


  fOK = UtlAlloc( (PVOID *)&pBackList1,
                  0L,
                  (LONG) (usLenStr1 +1) * sizeof(FUZZYTOK),
                  ERROR_STORAGE );
  if (fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pBackList2,
                    0L,
                    (LONG) (usLenStr2 +1) * sizeof(FUZZYTOK),
                    ERROR_STORAGE );
  } /* endif */
  if ( fOK )
  {
    /******************************************************************/
    /* fill backward tokenlists                                       */
    /******************************************************************/
    pTestToken = pTokenList1;
    for ( k=(usLenStr1-1); k >= 0 ; k-- )
    {
      *(pBackList1+k) = *pTestToken;
      pTestToken++;
    } /* endfor */

    pTestToken = pTokenList2;
    for ( k = (usLenStr2-1); k >= 0 ; k-- )
    {
      *(pBackList2+k) = *pTestToken;
      pTestToken++;
    } /* endfor */

    /******************************************************************/
    /* call recursive function to find shortest edit script           */
    /******************************************************************/
    LCSString1.pTokenList = pTokenList1;
    LCSString1.pBackList = pBackList1;
    LCSString1.sStart = 0;
    LCSString1.sStop = usLenStr1;
    LCSString1.sTotalLen = usLenStr1;

    LCSString2.pTokenList = pTokenList2;
    LCSString2.pBackList = pBackList2;
    LCSString2.sStart = 0;
    LCSString2.sStop = usLenStr2;
    LCSString2.sTotalLen = usLenStr2;

    LCS ( LCSString1,LCSString2, fCompareAll);

    UtlAlloc( (PVOID *)&pBackList1, 0L, 0L, NOMSG);
    UtlAlloc( (PVOID *)&pBackList2, 0L, 0L, NOMSG);

  } /* endif */
  return fOK;
} /* end of function EQFBCallLCS */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFBMarkModDelIns                                        |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       convert output from LCS into fuzzytoks with              |
//|                   MARK_MODIFIED, MARK_INSERTED, MARK_DELETED               |
//+----------------------------------------------------------------------------+
//|Parameters:        PFUZZYTOK   pTokenList1,     // tokenlist from LCS       |
//|                   PFUZZYTOK   pTokenList2,     // tokenlist from LCS       |
//|                   PFUZZYTOK * ppFuzzyTgt,      // returned token of str 1  |
//|                   PFUZZYTOK * ppFuzzyTok,      // returned token of str2   |
//|                   USHORT      usLenStr1,       // # of toks in TokenList1  |
//|                   USHORT      usLenStr2        // # of toks in tokenlist2  |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    every thing okay                                 |
//|                   FALSE   else                                             |
//+----------------------------------------------------------------------------+
//|Side effects:      The returned token list has to be freed by the caller    |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate ppFuzzy1 ppFuzzy2                               |
//|                   convert output from LCS into PFUZZYTOKS with             |
//|                   MARK_MODIFIED, MARK_DELETED, MARK_INSERTED               |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
//static 
BOOL
EQFBMarkModDelIns
(
    PFUZZYTOK   pTokenList1,
    PFUZZYTOK   pTokenList2,
    PFUZZYTOK  * ppFuzzy1,
    PFUZZYTOK  * ppFuzzy2,
    USHORT      usLenStr1,
    USHORT      usLenStr2
)

{
  BOOL         fOK;
  PFUZZYTOK    pTestToken;             // pointer to token lists 2
  USHORT       usStringA, usStringB;   // indices into strings (A=1, B=2)
  PFUZZYTOK    pT1Token;               // pointer to token lists 1

  LONG         lTokBufLen  = (LONG)(usLenStr1 + usLenStr2 + 1);
  LONG         lTokBufSize = lTokBufLen * sizeof(FUZZYTOK);
  LONG         lTokBufUsed = 0;

  fOK = UtlAlloc( (PVOID *)ppFuzzy2, 0L, lTokBufSize, ERROR_STORAGE );
  if (fOK ) fOK = UtlAlloc( (PVOID *)ppFuzzy1, 0L, lTokBufSize, ERROR_STORAGE );

  if ( fOK )
  {
    pTestToken = *ppFuzzy2;
    pT1Token = *ppFuzzy1;
    usStringA = usStringB = 0;
    while ((usStringA < usLenStr1) && (usStringB < usLenStr2) )
    {
      if ( ((pTokenList2+usStringB)->sType == MARK_EQUAL)
           && ((pTokenList1+usStringA)->sType == MARK_EQUAL)    )
      {
         *pTestToken = *(pTokenList2+usStringB);
         pTestToken++;
         lTokBufUsed++;
         *pT1Token = *(pTokenList1+usStringA);
         pT1Token++;

         usStringA++; usStringB++;
      }
      else if ((pTokenList1+usStringA)->sType == MARK_EQUAL)
      {
        *pTestToken = *(pTokenList2+usStringB);
        pTestToken->sType = MARK_INSERTED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList2+usStringB);
        pT1Token->sType = MARK_DELETED;
        pT1Token++;

        usStringB++;
      }
      else if ((pTokenList2+usStringB)->sType == MARK_EQUAL)
      {
        *pTestToken = *(pTokenList1+usStringA);
        pTestToken->sType = MARK_DELETED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList1+usStringA);
        pT1Token->sType = MARK_INSERTED;
        pT1Token++;

        usStringA++;
      }
      else
      {
        *pTestToken = *(pTokenList2+usStringB);
        pTestToken->sType = MARK_MODIFIED;
        pTestToken++;
        lTokBufUsed++;
        *pT1Token = *(pTokenList1+usStringA);
        pT1Token->sType = MARK_MODIFIED;
        pT1Token++;

        usStringA++;
        usStringB++;
      } /* endif */
    } /* endwhile */

    while ( usStringA < usLenStr1   )
    {
      *pTestToken = *(pTokenList1+usStringA);
      pTestToken->sType = MARK_DELETED;
      pTestToken++;
      lTokBufUsed++;
      *pT1Token = *(pTokenList1+usStringA);
      pT1Token->sType = MARK_INSERTED;
      pT1Token++;

      usStringA++;
    } /* endwhile */
    while ( usStringB < usLenStr2   )
    {
      *pTestToken = *(pTokenList2+usStringB);
      pTestToken->sType = MARK_INSERTED;
      pTestToken++;
      lTokBufUsed++;

      *pT1Token = *(pTokenList2+usStringB);
      pT1Token->sType = MARK_DELETED;
      pT1Token++;
      usStringB++;
    } /* endwhile */
  } /* endif */

  return( fOK );
} /* end of function EQFBMarkModDelIns */
#endif



BOOL
PrepareTokens
(
  PLOADEDTABLE  pTagTable,
  PBYTE         pInBuf,
  PBYTE         pTokBuf,
  PSZ_W         pString,               // pointer to string to be tokenized
  SHORT         sLanguageId,           // language ID
  PFUZZYTOK   * ppTransTokList,         // resulting list of tokens
  ULONG         ulOemCP
);




BOOL EQFBCallLCS
(
  PFUZZYTOK    pTokenList1,             // ptr to token list1
  PFUZZYTOK    pTokenList2,                      // ptr to token list2
  USHORT       usLenStr1,
  USHORT       usLenStr2,
  BOOL         fCompareAll
);

BOOL EQFBMarkModDelIns
(
    PFUZZYTOK   pTokenList1,
    PFUZZYTOK   pTokenList2,
    PFUZZYTOK  * ppFuzzy1,
    PFUZZYTOK  * ppFuzzy2,
    USHORT      usLenStr1,
    USHORT      usLenStr2
);


/**********************************************************************/
/* find the differences in two strings (w/o cleanup if consecutives   */
/* delete and inserts)                                                */
/**********************************************************************/
BOOL
EQFBFindDiffEx
(
  PVOID pTable,                        // pointer to loaded tagtable
  PBYTE pInBuf,                        // pointer to input buffer
  PBYTE pTokBuf,                       // pointer to temp token buffer
  PSZ_W pString1,                      // first string passed
  PSZ_W pString2,                      // second string
  SHORT sLanguageId,                   // language id to be used for pString1
  PVOID * ppFuzzyTok,                  // returned token list of pString2
  PVOID * ppFuzzyTgt,                   // returned token list of pString1
  ULONG   ulOemCP
)
{
  BOOL fOK = TRUE;                     // success indicator
  USHORT       usLenStr1 = 0;          // length of string 1 (in num. of tokens)
  USHORT       usLenStr2 = 0;          // length of string 2 (in num. of tokens)
  PFUZZYTOK    pTokenList2 = NULL;     // pointer to token lists
  PFUZZYTOK    pTokenList1 = NULL;     // pointer to token lists
  PFUZZYTOK    pToken;                 // pointer to token lists
  PLOADEDTABLE pTagTable = (PLOADEDTABLE) pTable;

  /******************************************************************/
  /* prepare tokens for String1 and string 2                        */
  /******************************************************************/
  fOK = PrepareTokens( //pDoc,
                       pTagTable,
                       pInBuf,
                       pTokBuf,
                       pString1, sLanguageId, &pTokenList1, ulOemCP );
  if ( fOK )
  {
    fOK = PrepareTokens( // pDoc,
                         pTagTable,
                         pInBuf,
                         pTokBuf,
                         pString2, sLanguageId, &pTokenList2, ulOemCP );
  } /* endif */
  if (fOK )
  {
    /********************************************************************/
    /* get number of tokens in strings                                  */
    /********************************************************************/
    NUMBEROFTOKENS(usLenStr1, pTokenList1);
    NUMBEROFTOKENS(usLenStr2, pTokenList2);

    /******************************************************************/
    /* call LCS and compare tokens with strncmp                       */
    /******************************************************************/
    fOK = EQFBCallLCS(pTokenList1, pTokenList2,
                      usLenStr1, usLenStr2, TRUE);
  } /* endif */
  /********************************************************************/
  /* get number of tokens in strings                                  */
  /********************************************************************/
  if ( fOK )
  {
    /****************************************************************/
    /* adjust length to avoid non-marked spaces between marked tokens*/
    /****************************************************************/
    pToken = pTokenList1;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */

    pToken = pTokenList2;                   //current pointers
    while ( pToken->ulHash )
    {
      if ( (pToken->usStop < (pToken+1)->usStart) )
      {
        pToken->usStop = (pToken+1)->usStart - 1;
      } /* endif */
      pToken++;
    } /* endwhile */
    /****************************************************************/
    /* allocate space for fuzzy tokens ,.,,,                        */
    /* this area MUST be freed by the calling application           */
    /****************************************************************/
    fOK = EQFBMarkModDelIns( pTokenList1, pTokenList2,
                          (PFUZZYTOK *)ppFuzzyTgt, (PFUZZYTOK *)ppFuzzyTok,
                         usLenStr1, usLenStr2);

  } /* endif */
  /********************************************************************/
  /* free any allocated resources                                     */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pTokenList1, 0L, 0L, NOMSG );
  UtlAlloc( (PVOID *)&pTokenList2, 0L, 0L, NOMSG );
  return( fOK );
} /* end of function EQFBFindDiffEx */

static
BOOL CheckForAlloc( PTMX_SENTENCE pSentence, PTMX_TERM_TOKEN * ppTermTokens, USHORT usEntries );

static
BOOL CheckForAlloc
(
   PTMX_SENTENCE      pSentence,
   PTMX_TERM_TOKEN  * ppTermTokens,
   USHORT             usEntries
)
{
  LONG     lFilled = 0L;
  USHORT   usNewAlloc = 0;
  BOOL     fOK = TRUE;
  PTMX_TERM_TOKEN  pTermTokens;

  pTermTokens = *ppTermTokens;
  lFilled = ( (PBYTE)pTermTokens - (PBYTE)pSentence->pTermTokens);
  if ( (pSentence->lTermAlloc - lFilled) <= (LONG)(usEntries * sizeof(TMX_TERM_TOKEN)) )
  {
    //remember offset of pTagEntry
    lFilled = pTermTokens - pSentence->pTermTokens;
    if (usEntries == 1)
    {
      usNewAlloc = TOK_SIZE;
    }
    else
    {
      usNewAlloc = usEntries * sizeof(TMX_TERM_TOKEN);
    }
    //allocate another 4k for pTagRecord
    fOK = UtlAlloc( (PVOID *) &pSentence->pTermTokens, pSentence->lTermAlloc,
                    pSentence->lTermAlloc + (LONG)usNewAlloc, NOMSG );
    if ( fOK )
    {
      pSentence->lTermAlloc += (LONG)usNewAlloc;

      //set new position of pTagEntry
      pTermTokens = pSentence->pTermTokens + lFilled;
    } /* endif */
  } /* endif */
  *ppTermTokens= pTermTokens;
  return(fOK);
}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TokenizeSource      Split a segment into tags and words   
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all tags and terms found.   
//------------------------------------------------------------------------------
// Function call:     TokenizeSource( PTMX_CLB pClb,  //ptr to ctl block        
//                                PTMX_SENTENCE pSentence, //ptr sent struct    
//                                PSZ pTagTableName )  //name of tag table      
//------------------------------------------------------------------------------
// Input parameter:   PTMX_CLB  pClb           control block                    
//                    PTMX_SENTENCE pSentence  sentence structure               
//                    PSZ      pTagTableName  name of tag table                 
//                    USHORT usVersion       version of TM                      
//------------------------------------------------------------------------------
// Output parameter:                                                            
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully               
//                    other       MORPH_ error code, see EQFMORPH.H for a list  
//------------------------------------------------------------------------------
// Prerequesits:                                                                
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:                                                                     
//------------------------------------------------------------------------------
// Function flow:                                                               
//     allocate storage                                                         
//     tokenize string with correct tag table                                   
//     process outputed token list                                              
//     if tag                                                                   
//       remember position of tag and add to entry structure                    
//       add length of tag to structure                                         
//       add string to structure                                                
//     else if text                                                             
//       morph tokenize returning list of terms                                 
//       add term info to structure                                             
//                                                                              
//------------------------------------------------------------------------------
USHORT TokenizeSource
(
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion                     // version of TM
)
{
  ULONG ulSrcCP = GetLangOEMCP( pSourceLang);
  return( TokenizeSourceEx( pClb, pSentence, pTagTableName, pSourceLang, usVersion, ulSrcCP ) );
}

USHORT TokenizeSourceEx
(
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion,                    // version of TM
   ULONG  ulSrcCP                       // OEM CP of source language     
)
{
    return( TokenizeSourceEx2( pClb, pSentence, pTagTableName, pSourceLang, usVersion, ulSrcCP, 0 ) );
}

USHORT TokenizeSourceEx2
(
   PTMX_CLB pClb,                       // pointer to control block (Null if called outside of Tm functions)
   PTMX_SENTENCE pSentence,             // pointer to sentence structure
   PSZ pTagTableName,                   // name of tag table
   PSZ pSourceLang,                     // source language
   USHORT usVersion,                    // version of TM
   ULONG  ulSrcCP,                      // OEM CP of source language     
   int iMode                            // mode to be passed to create protect table function
)
{

  PVOID     pTokenList = NULL;         // ptr to token table
  BOOL      fOK;                       // success indicator
  PBYTE     pTagEntry;                 // pointer to tag entries
  PTMX_TERM_TOKEN pTermTokens;         // pointer to term tokens
  PLOADEDTABLE pTable = NULL;          // pointer to tagtable
  PTMX_TAGTABLE_RECORD pTagRecord;     // pointer to record
  USHORT    usLangId;                  // language id
  USHORT    usRc = NO_ERROR;           // returned value
  USHORT    usFilled;                  // counter
  USHORT    usTagEntryLen;             // length indicator
  CHAR      szString[MAX_FNAME];       // name without extension
  USHORT    usStart;                   // position counter
  PSTARTSTOP pStartStop = NULL;        // ptr to start/stop table
  int        iIterations = 0;
  USHORT     usAddEntries = 0;
  PSZ_W  pszNormStringPos = pSentence->pNormString; // current pNpormString output pointer

  pSourceLang;                         // avoid compiler warnings

  /********************************************************************/
  /* normalize \r\n combinations in input string ..                   */
  /********************************************************************/
  {
    PSZ_W  pSrc = pSentence->pInputString;
    PSZ_W  pTgt = pSentence->pInputString;
    CHAR_W c;

    while ( (c = *pSrc++) != NULC )
    {
      /****************************************************************/
      /* in case of \r we have to do some checking, else only copy..  */
      /****************************************************************/
      if ( c == '\r' )
      {
        if ( *pSrc == '\n' )
        {
          /************************************************************/
          /* ignore the character                                     */
          /************************************************************/
        }
        else
        {
          *pTgt++ = c;
        } /* endif */
      }
      else
      {
        *pTgt++ = c;
      } /* endif */
    } /* endwhile */
    *pTgt = EOS;
  }

  //allocate 4K pTokenlist for TaTagTokenize
  fOK = UtlAlloc( (PVOID *) &(pTokenList), 0L, (LONG)TOK_SIZE, NOMSG );

  if ( !fOK )
  {
    usRc = ERROR_NOT_ENOUGH_MEMORY;
  }
  else
  {
    LogMessage(DEVELOP, "TokenizeSourceEx2");
    pTagRecord = pSentence->pTagRecord;
    pTermTokens = pSentence->pTermTokens;
    pTagEntry = (PBYTE)pTagRecord;
    auto len = &(pSentence->lTermAlloc) - (long)pSentence;
    pTagEntry += sizeof(TMX_TAGTABLE_RECORD);
    
    RECLEN(pTagRecord) = 0;
    pTagRecord->usFirstTagEntry = (USHORT)(pTagEntry - (PBYTE)pTagRecord);

    //get id of tag table, call
    Utlstrccpy( szString, UtlGetFnameFromPath( pTagTableName ), DOT );

    pTagRecord->usTagTableId = 0;

    //get lang id of source lang for morphtokenize call
    usLangId = 0;
    if ( !usRc )
    {
      //load tag table for tokenize function
      usRc = TALoadTagTableExHwnd( szString, &pTable, FALSE,
                                   TALOADUSEREXIT | TALOADPROTTABLEFUNC |
                                   TALOADCOMPCONTEXTFUNC,
                                   FALSE, NULLHANDLE );
      if ( usRc )
      {
        USHORT usAction = UtlQueryUShort( QS_MEMIMPMRKUPACTION);
        usRc = ERROR_TA_ACC_TAGTABLE;
        if ( usAction == MEMIMP_MRKUP_ACTION_RESET ) {
           CHAR  *ptrMarkup ;
           ptrMarkup = strstr( pTagTableName, szString ) ;
           if ( ptrMarkup ) {
              strcpy( ptrMarkup, "OTMUTF8.TBL" ) ;
              strcpy( szString, "OTMUTF8" ) ;
              usRc = TALoadTagTableExHwnd( szString, &pTable, FALSE,
                                           TALOADUSEREXIT | TALOADPROTTABLEFUNC |
                                           TALOADCOMPCONTEXTFUNC,
                                           FALSE, NULLHANDLE );
              if ( usRc )
                 usRc = ERROR_TA_ACC_TAGTABLE;
           } 
        } else
        if ( usAction == MEMIMP_MRKUP_ACTION_SKIP ) {
           usRc = SEG_SKIPPED_BAD_MARKUP ;
        }
      } /* endif */

    } /* endif */

    if ( !usRc )
    {
      // build protect start/stop table for tag recognition
       usRc = TACreateProtectTableWEx( pSentence->pInputString, pTable, 0,
                                   (PTOKENENTRY)pTokenList,
                                   TOK_SIZE, &pStartStop,
                                   pTable->pfnProtTable,
                                   pTable->pfnProtTableW, ulSrcCP, iMode);


      while ((iIterations < 10) && (usRc == EQFRS_AREA_TOO_SMALL))
      {
        // (re)allocate token buffer
        LONG lOldSize = (usAddEntries * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;
        LONG lNewSize = ((usAddEntries+128) * sizeof(TOKENENTRY)) + (LONG)TOK_SIZE;

        if (UtlAlloc((PVOID *) &pTokenList, lOldSize, lNewSize, NOMSG) )
        {
          usAddEntries += 128;
          iIterations++;
        }
        else
        {
          iIterations = 10;    // force end of loop
        } /* endif */
        // retry tokenization
        if (iIterations < 10 )
        {
           usRc = TACreateProtectTableWEx( pSentence->pInputString, pTable, 0,
                                      (PTOKENENTRY)pTokenList,
                                       (USHORT)lNewSize, &pStartStop,
                                       pTable->pfnProtTable,
                                       pTable->pfnProtTableW, ulSrcCP, iMode );
        } /* endif */

      } /* endwhile */
    } /* endif */

    if ( !usRc )
    {
      USHORT usEntries = 0; // number of entries in start stop table

      PSTARTSTOP pEntry = pStartStop;
      while ( (pEntry->usStart != 0) ||
              (pEntry->usStop != 0)  ||
              (pEntry->usType != 0) )
      {
        switch ( pEntry->usType )
        {
          case UNPROTECTED_CHAR :
            // handle translatable text
            {
              PFLAGOFFSLIST pTerm;                   // pointer to terms list
              PFLAGOFFSLIST pTermList = NULL;        // pointer to offset/length term list
              ULONG     ulListSize = 0;
              CHAR_W    chTemp;                      // buffer for character values
              USHORT    usTokLen = pEntry->usStop - pEntry->usStart + 1;

              chTemp = pSentence->pInputString[pEntry->usStop+1];
              pSentence->pInputString[pEntry->usStop+1] = EOS;

              usRc = NTMMorphTokenizeW( usLangId,
                                       pSentence->pInputString + pEntry->usStart,
                                       &ulListSize, (PVOID *)&pTermList,
                                       MORPH_FLAG_OFFSLIST, usVersion );

              pSentence->pInputString[pEntry->usStop+1] = chTemp;

              if ( usRc == MORPH_OK )
              {
                pTerm = pTermList;
                fOK = CheckForAlloc( pSentence, &pTermTokens, 1 );
                if ( !fOK )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                  usStart = pEntry->usStart;
                  if ( pTerm )
                  {
                      while ( pTerm->usLen && !usRc )
                      {
                        if ( !(pTerm->lFlags & TF_NEWSENTENCE) )
                        {                         
                          fOK = CheckForAlloc( pSentence, &pTermTokens, 1 );
                          if ( !fOK )
                          {
                            usRc = ERROR_NOT_ENOUGH_MEMORY;
                          }
                          else
                          {
                            pTermTokens->usOffset = pTerm->usOffs + usStart;
                            pTermTokens->usLength = pTerm->usLen;
                            pTermTokens->usHash = 0;
                            pTermTokens++;
                          }
                        } /* endif */
                        pTerm++;
                      } /* endwhile */
                  } /* endif */
                  memcpy( pszNormStringPos, pSentence->pInputString + pEntry->usStart, usTokLen * sizeof(CHAR_W));
                  pSentence->usNormLen =  pSentence->usNormLen + usTokLen;
                  pszNormStringPos += usTokLen;
                } /* endif */
              } /* endif */
              UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
            } /* end case UNPROTECTED_CHAR */
            break;
          default :
            // handle not-translatable data
            {
              // if next start/stop-entry is a protected one ...
              if ( ((pEntry+1)->usStart != 0) &&
                   ((pEntry+1)->usType != UNPROTECTED_CHAR) )
              {
                // enlarge next entry and ignore current one
                (pEntry+1)->usStart = pEntry->usStart;
              }
              else
              {
                // add tag data
                usTagEntryLen = sizeof(TMX_TAGENTRY) +
                                (pEntry->usStop - pEntry->usStart + 1) * sizeof(CHAR_W);
                if ( (pSentence->lTagAlloc - (pTagEntry - (PBYTE)pSentence->pTagRecord))
                                                       <= (SHORT)usTagEntryLen )
                {
                  LONG lBytesToAlloc = get_max( ((LONG)TOK_SIZE), ((LONG)usTagEntryLen) );

                  //remember offset of pTagEntry
                  usFilled = (USHORT)(pTagEntry - (PBYTE)pSentence->pTagRecord);

                  //allocate another 4k for pTagRecord
                  fOK = UtlAlloc( (PVOID *) &pSentence->pTagRecord, pSentence->lTagAlloc,
                                  pSentence->lTagAlloc + lBytesToAlloc, NOMSG );
                  if ( fOK )
                  {
                    pSentence->lTagAlloc += lBytesToAlloc;

                    //set new position of pTagEntry
                    pTagEntry = (PBYTE)(pSentence->pTagRecord) + usFilled;
                  } /* endif */
                } /* endif */

                if ( !fOK )
                {
                  usRc = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                  ((PTMX_TAGENTRY)pTagEntry)->usOffset = pEntry->usStart;
                  ((PTMX_TAGENTRY)pTagEntry)->usTagLen =
                    (pEntry->usStop - pEntry->usStart + 1);
                  memcpy( &(((PTMX_TAGENTRY)pTagEntry)->bData),
                          pSentence->pInputString + pEntry->usStart,
                          ((PTMX_TAGENTRY)pTagEntry)->usTagLen * sizeof(CHAR_W));
                  pTagEntry += usTagEntryLen;
                } /* endif */
              } /* endif */
            } /* end default */
            break;
        } /* endswitch */
        pEntry++;
        usEntries++;
      } /* endwhile */

      /********************************************************/
      /* check if we filled at least one term token -- if not */
      /* use a dummy token - just to get an exact match ...   */
      /********************************************************/
      if ( pTermTokens == pSentence->pTermTokens )
      {
        pTermTokens->usOffset = 0;
        pTermTokens->usLength = (USHORT)UTF16strlenCHAR( pSentence->pInputString );
        pTermTokens->usHash = 0;
        pTermTokens++;
      } /* endif */

      fOK = CheckForAlloc( pSentence, &pTermTokens, 3 );
      if ( !fOK )
      {
        usRc = ERROR_NOT_ENOUGH_MEMORY;
      }

      //set total tag record length
      RECLEN(pTagRecord) = pTagEntry - (PBYTE)pTagRecord;
    } /* endif */
  } /* endif */

  //release allocated memory
  if ( pStartStop ) UtlAlloc( (PVOID *) &pStartStop, 0L, 0L, NOMSG );
  if ( pTokenList ) UtlAlloc( (PVOID *) &pTokenList, 0L, 0L, NOMSG );

  //free tag table / decrement use count
  if ( pTable != NULL )
  {
    TAFreeTagTable( pTable );
  } /* endif */

  return ( usRc );
}

#ifdef TEMPORARY_COMMENTED
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     MorphTokenize       Split a segment into terms            
//------------------------------------------------------------------------------
// Description:       Tokenizes a string and stores all terms found in a        
//                    term list.                                                
//                    :p.                                                       
//                    Depending on the type of list requested                   
//                    a term list is either a series of null-terminated strings 
//                    which is terminated by another null value or a list       
//                    of offset and length values terminated by a null entry.   
//                    See the sample for the layout of a term list.             
//                    :p.                                                       
//                    If the term list pointer is NULL and the term list size   
//                    is zero, the term list is allocated.                      
//                    The term list is enlarged if not all terms fit into       
//                    the current list.                                         
//------------------------------------------------------------------------------
// Function call:     usRC = MorphTokenize( SHORT sLanguageID, PSZ pszInData,   
//                                          PUSHORT pusBufferSize,              
//                                          PVOID *ppTermList,                  
//                                          USHORT usListType);                 
//------------------------------------------------------------------------------
// Input parameter:   SHORT    sLanguageID    language ID                       
//                    PSZ      pszInData      pointer to input segment          
//                    USHORT   usListType     type of term list                 
//                                              MORPH_ZTERMLIST or              
//                                              MORPH_OFFSLIST                  
//------------------------------------------------------------------------------
// Output parameter:  PUSHORT  pusBufferSize  address of variable containing    
//                                               size of term list buffer       
//                    PVOID    *ppTermList    address of caller's term list     
//                                               pointer                        
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       MORPH_OK    function completed successfully               
//                    other       MORPH_ error code, see EQFMORPH.H for a list  
//------------------------------------------------------------------------------
// Prerequesits:      sLanguageID must have been obtained using the             
//                    MorphGetLanguageID function.                              
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Samples:           USHORT usListSize = 0;     // allocated list size         
//                    PSZ    pszList = NULL;     // ptr to list area            
//                    SHORT  sLangID;            // language ID                 
//                                                                              
//                    usRC = MorphGetLanguageID( "English(U.S.)", &sLangID );   
//                    usRC = MorphTokenize( sLangID, "Mary had a little lamb",  
//                                          &usListSize, &pszList,              
//                                          MORPH_ZTERMLIST );                  
//                                                                              
//                    after this call the area pointed to by pszList would      
//                    contain the following term list:                          
//                                                                              
//                    "Mary\0had\0a\0little\0lamb\0\0"                          
//------------------------------------------------------------------------------
// Function flow:     check input data                                          
//                    get language control block pointer                        
//                    call tokenize function of language exit                   
//                    return function return code                               
//------------------------------------------------------------------------------
USHORT NTMMorphTokenize
(
   SHORT    sLanguageID,               // language ID
   PSZ      pszInData,                 // pointer to input segment
   PUSHORT  pusBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  USHORT    usTermBufUsed = 0;         // number of bytes used in term buffer

  sLanguageID;                         // avoid compiler warning
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pusBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pusBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

///********************************************************************/
///* Get language control block pointer  -- not needed                */
///********************************************************************/
//if ( usRC == MORPH_OK )
//{
//  usRC = MorphGetLCB( sLanguageID, &pLCB );
//} /* endif */


  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
      usRC = NTMTokenize( NULL,
                          pszInData,
                          pusBufferSize,
                          ppTermList,
                          usListType,
                          usVersion );
    }
    else
    {
      usRC = MorphAddTermToList( (PSZ *)ppTermList,
                                 pusBufferSize,
                                 &usTermBufUsed,
                                 (PSZ)EMPTY_STRING,
                                 0,
                                 0,
                                 0L,
                                 usListType );
    } /* endif */
  } /* endif */

  return( usRC );

} /* endof MorphTokenize */
#endif



USHORT MorphAddTermToList2W
(
  PSZ_W    *ppList,                    // ptr to term list pointer
  PULONG   pulSize,                    // alloc. size of term list in # of w's
  PULONG   pulUsed,                    // used # of w's in term list
  PSZ_W    pszTerm,                    // ptr to new term being added to list
  USHORT   usLength,                   // length of term in # of w's
  USHORT   usOffs,                     // offset of term in # of w's???
  ULONG    ulFlags,                    // flags describing the term
  USHORT   usListType                  // type of list
)
{
  USHORT   usRC = MORPH_OK;            // function return code
  ULONG    ulDataLenBytes = 0;         // length of data to add in # of bytes
  ULONG    ulDataLen = 0;              // length of data to add in w's
  BOOL     fOK  = TRUE;                // internal OK flag

  PBYTE    pList = (PBYTE)*ppList;     // ptr to term list
  PSZ_W    *pAnchor = NULL;            // ptr to anchor for list

  /********************************************************************/
  /* position to last block for large term lists                      */
  /********************************************************************/
  if ( usListType == MORPH_LARGE_ZTERMLIST )
  {
    PSZ_W  pNextList;

    // use caller's list pointer as default anchor
    pAnchor = ppList;

    if ( pList != NULL )
    {
      pNextList = *((PSZ_W *)pList);

      while ( pNextList != NULL )
      {
        pAnchor = (PSZ_W *)pList;
        pList = (PBYTE)pNextList;
        pNextList = *((PSZ_W *)pList);
      } /* endwhile */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* get required size for new entry                                  */
  /********************************************************************/
  switch ( usListType )
  {
    case MORPH_OFFSLIST :
      /******************************************************************/
      /* space for offset and length of term plus end of term list      */
      /* indicator which is a zero offset and length value              */
      /******************************************************************/
      ulDataLenBytes = 4 * sizeof(USHORT);
      break;

    case MORPH_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    case MORPH_FLAG_OFFSLIST :
      /******************************************************************/
      /* space for flag, offset and length of term plus end of term list*/
      /* indicator which is a zero flag, offset and length value        */
      /******************************************************************/
      //ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(USHORT));
      ulDataLenBytes = (2 * sizeof(ULONG)) + (4 * sizeof(wchar_t));
      break;

    case MORPH_FLAG_ZTERMLIST :
      /******************************************************************/
      /* space for flag, term, term delimiter and end of term list      */
      /* indicator which is a empty flag and a term delimiter           */
      /******************************************************************/
      ulDataLenBytes = sizeof(ULONG) + (usLength + 1)*sizeof(CHAR_W)
                      + sizeof(ULONG) + sizeof(CHAR_W);
      break;

    case MORPH_LARGE_ZTERMLIST :
      /******************************************************************/
      /* space for term, term delimiter and end of term list indicator  */
      /******************************************************************/
      ulDataLenBytes = (usLength + 1 + 1)*sizeof(CHAR_W);
      break;

    default :
      fOK = FALSE;
      break;
  } /* endswitch */

  /********************************************************************/
  /* enlarge term list if necessary                                   */
  /********************************************************************/
  ulDataLen = ulDataLenBytes / sizeof(CHAR_W);        // # of w's needed
  if ( fOK && ((*pulUsed + ulDataLen) > *pulSize) )
  {
    LONG lOldLen = 0L;
    LONG max = TERMLIST_INCR > ulDataLen? TERMLIST_INCR : ulDataLen ;
    LONG lNewLen = ((LONG)*pulSize) + max ;

    LONG lMaxLen = (usListType == MORPH_LARGE_ZTERMLIST) ?
                                          MORPH_LARGE_ZTERMLIST_LEN : MAX_ALLOC;
    lNewLen = lNewLen * sizeof(CHAR_W);         //  len in # of bytes
    lOldLen = (*pulSize) * sizeof(CHAR_W);
    if ( lNewLen >= lMaxLen )
    {
      // for large termlists allocate another buffer else set error condition
      if ( usListType == MORPH_LARGE_ZTERMLIST )
      {
        lNewLen = (LONG)sizeof(PSZ_W) +
                   (LONG)sizeof(CHAR_W) * max;
        *pulSize = 0;                  // no size yet
        lOldLen = 0L;
        pAnchor  = (PSZ_W *)pList;       // use current list as anchor
      }
    } /* endif */

    if ( fOK )
    {
      fOK = UtlAlloc( (PVOID *)&pList, lOldLen, lNewLen, NOMSG );
      if ( fOK )
      {  // set caller's termlist pointer
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           *pAnchor = (PSZ_W)pList;
           if (*pulSize == 0 )         // new block???
           {
             *pulUsed = sizeof(PSZ_W)/ sizeof(CHAR_W);   // leave room for pointer to next block
           } /* endif */
           *pulSize = lNewLen / sizeof(CHAR_W);
         }
         else
         {
           *pulSize = lNewLen / sizeof(CHAR_W);  //@@+= max( TERMLIST_INCR, ulDataLength );??
           *ppList = (PSZ_W)pList;
         } /* endif */
      }
      else
      {
         usRC = MORPH_NO_MEMORY;

         // free the term list in case of erros
         if ( usListType == MORPH_LARGE_ZTERMLIST )
         {
           pList = (PBYTE)*ppList;
           while ( pList != NULL )
           {
             PSZ_W pNextList = *((PSZ_W *)pList);
             UtlAlloc( (PVOID *)&pList, 0L, 0L, NOMSG );
             pList = (PBYTE)pNextList;
           } /* endwhile */
         }
         else
         {
           UtlAlloc( (PVOID *)ppList, 0L, 0L, NOMSG );
         } /* endif */
         *ppList = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* add term to term list (pList is a PBYTE ptr                      */
  /********************************************************************/
  if ( fOK )
  {
    ULONG  ulByteUsedInList = (*pulUsed) * sizeof(CHAR_W);
    switch ( usListType )
    {
      case MORPH_OFFSLIST :
        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += 1;//sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1;//sizeof(USHORT)/sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_LARGE_ZTERMLIST :
      case MORPH_ZTERMLIST :
        if ( usLength )
        {
          memcpy( (PBYTE)(pList + ulByteUsedInList),
                  (PBYTE) pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ((*pulUsed) * sizeof(CHAR_W)))) = EOS;        /* 10-11-16 */
        *pulUsed += 1;
        ulByteUsedInList += sizeof(CHAR_W);
        break;

      case MORPH_FLAG_OFFSLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        *((PUSHORT)(pList + ulByteUsedInList)) = usLength;
        *pulUsed += 1;//TEMPORARY_HARDCODED sizeof(USHORT)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);

        *((PUSHORT)(pList + ulByteUsedInList)) = usOffs;
        *pulUsed += 1; //sizeof(USHORT) / sizeof(CHAR_W);
        ulByteUsedInList += sizeof(USHORT);
        break;

      case MORPH_FLAG_ZTERMLIST :
        *((PULONG)(pList + ulByteUsedInList)) = ulFlags;
        *pulUsed += sizeof(ULONG)/ sizeof(CHAR_W);
        ulByteUsedInList += sizeof(ULONG);

        if ( usLength )
        {
          memcpy( pList + ulByteUsedInList,
                 (PBYTE)pszTerm, usLength*sizeof(CHAR_W) );
          *pulUsed += usLength;
          ulByteUsedInList += usLength * sizeof(CHAR_W);
        } /* endif */
        *((PSZ_W)(pList + ulByteUsedInList)) = EOS;     /* 10-11-16 */
        *pulUsed += 1;
        break;

      default :
        break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /* return usRC to calling function                                  */
  /********************************************************************/
  return( usRC );

} /* end of function MorphAddTermToList2W */

USHORT NTMTokenizeW
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
);

USHORT NTMMorphTokenizeW
(
   SHORT    sLanguageID,               // language ID
   PSZ_W    pszInData,                 // pointer to input segment
   PULONG   pulBufferSize,             // address of variable containing size of
                                       //    term list buffer
   PVOID    *ppTermList,               // address of caller's term list pointer
   USHORT   usListType,                // type of term list MORPH_ZTERMLIST or
                                       //    MORPH_OFFSLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT    usRC = MORPH_OK;           // function return code
  ULONG     ulTermBufUsed = 0;         // number of bytes used in term buffer

  sLanguageID;                         // avoid compiler warning
  /********************************************************************/
  /* Check input data                                                 */
  /********************************************************************/
  if ( (pszInData == NULL)     ||
       (pulBufferSize == NULL) ||
       (ppTermList == NULL)    ||
       ((*ppTermList == NULL) && (*pulBufferSize != 0) ) )
  {
    usRC = MORPH_INV_PARMS;
  } /* endif */

///********************************************************************/
///* Get language control block pointer  -- not needed                */
///********************************************************************/
//if ( usRC == MORPH_OK )
//{
//  usRC = MorphGetLCB( sLanguageID, &pLCB );
//} /* endif */


  /********************************************************************/
  /* call language exit to tokenize the input data                    */
  /********************************************************************/
  if ( usRC == MORPH_OK )
  {
    if ( *pszInData != EOS )
    {
      usRC = NTMTokenizeW( NULL,
                          pszInData,
                          pulBufferSize,
                          ppTermList,
                          usListType,
                          usVersion );
    }
    else
    {
      usRC = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                    pulBufferSize,
                                    &ulTermBufUsed,
                                   (PSZ_W)EMPTY_STRING,
                                    0,
                                    0,
                                    0L,
                                    usListType );
    } /* endif */
  } /* endif */

  return( usRC );

} /* endof NTMMorphTokenizeW */


#ifdef TEMPORARY_COMMENTED
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     TOKENIZE                                                  
//------------------------------------------------------------------------------
// Description:       Tokenization of an input segment. The tokenized terms     
//                    are returned in form of a term list.                      
//------------------------------------------------------------------------------
// Function call:     TOKENIZE( LX_CB_P pNLPCB,                                 
//                              PSZ pszInData,                                  
//                              USHORT usDataLength,                            
//                              PUSHORT pusTermListSize, PVOID *ppTermList,     
//                              USHORT usListType );                            
//------------------------------------------------------------------------------
// Input parameter:   LX_CB_P  pNLPCB          ptr to NLP control Block         
//                    PSZ      pszInData       ptr to data being tokenized      
//                    PUSHORT  pusTermListSize address of term list size var    
//                    PVOID    *ppTermList     address of term list pointer     
//                    USHORT   usListTyp       type of term list:               
//                                             MORPH_ZTERMLIST or               
//                                             MORPH_OFFSLIST                   
//                                             at moment only MORPH_ZTERMLIST   
//                                             assumed!!!!!                     
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       see include file EQFMORPH.H for list of return codes      
//------------------------------------------------------------------------------
// Prerequesits:      pvLangCB must have been created using the INIT function   
//------------------------------------------------------------------------------
// Side effects:      none                                                      
//------------------------------------------------------------------------------
// Function flow:                                                               
//------------------------------------------------------------------------------
USHORT NTMTokenize
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszInData,                  // IN : ptr to data being tokenized
  PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT     usReturn = 0;             // returncode
  USHORT     usTermBufUsed = 0;        // amount of space used in term buffer
  PSZ        pTerm;                    // pointer to current token string
  PSZ        pszCurPos;                // current position in input data
  BOOL       fOffsList;                // TRUE = return a offset/length list
  LONG       lFlags;                   // flags for current term/token
  BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
  BOOL       fSkip, fNewSentence,      // token processing flags
             fSingleToken, fEndOfToken;
  UCHAR      c;
  UCHAR      d;


  pvLangCB;                            // avoid compiler warnings

  fOffsList  = (usListType == MORPH_OFFSLIST) ||
               (usListType == MORPH_FLAG_OFFSLIST);
  pszCurPos = pszInData;

  /********************************************************************/
  /* Always assume start of a new sentence                            */
  /********************************************************************/
  if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
       (usListType == MORPH_FLAG_OFFSLIST) )
  {
    usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                   pusTermListSize,
                                   &usTermBufUsed,
                                   " ",
                                   1,
                                   0, // no offset possible
                                   TF_NEWSENTENCE,
                                   usListType );
  } /* endif */

  /********************************************************************/
  /* Initialize processing flags                                      */
  /********************************************************************/
  pTerm    = pszCurPos;
  fAllCaps = TRUE;
  fAlNum   = TRUE;
  fNumber  = TRUE;

  /********************************************************************/
  /* Work on input data                                               */
  /********************************************************************/
  while ( !usReturn && ((c = *pszCurPos)!= NULC) )
  {
    /******************************************************************/
    /* Get type of processing required for current character          */
    /******************************************************************/
    fSkip         = FALSE;
    fNewSentence  = FALSE;
    fSingleToken  = FALSE;
    fEndOfToken   = FALSE;

    switch ( c )
    {
      case '.' :
      case '!' :
      case '?' :
        switch ( pszCurPos[1] )
        {
          case ' ':
          case '\n':
          case '\0':
            fNewSentence = TRUE;
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            if ( c != '.' )
            {
              fEndOfToken  = TRUE;
              fSingleToken  = TRUE;
            }
            else
            {
              fAllCaps = FALSE;
              fAlNum   = FALSE;
            } /* endif */
            break;
        } /* endswitch */
        break;

      case '\n' :
      case ' ' :
        fEndOfToken  = TRUE;
        fSkip        = TRUE;
        break;

      case '$' :
      case '#' :
        fAllCaps = FALSE;
        fAlNum   = FALSE;
        break;

      case ':' :
        if ( usVersion == TM_VERSION_1 )
        {
          fAllCaps = FALSE;
          fAlNum   = FALSE;
        }
        else
        {
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        } /* endif */
        break;

      case '/' :
      case '\\' :
      case ',' :
      case ';' :
        switch ( pszCurPos[1] )
        {
          case ' ':
          case '\n':
          case '\0':
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            fAllCaps = FALSE;
            fAlNum   = FALSE;
            fNumber  = FALSE;
            break;
        } /* endswitch */
        break;

      case '-' :            // dash within word or as 'Gedankenstrich'
        if ( ((d = pszCurPos[1]) == ' ') || (d == '\n') || ( d == '\0' ) )
        {
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        } /* endif */
        break;

      case '(' :
      case ')' :
      case '\"' :
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        break;

      default:
        if ( fAlNum ) //&& !chIsText[c] )
        {
			PUCHAR pTextTable;
			UtlQueryCharTable( IS_TEXT_TABLE, &pTextTable );
			if ( !pTextTable[c] )
			{
				fAlNum = FALSE;
			}
        } /* endif */
        if ( fNumber && !isdigit(c) )
        {
          fNumber  = FALSE;
        } /* endif */
        if ( fAllCaps && (!isalpha(c) || islower(c)) )
        {
          fAllCaps = FALSE;
        } /* endif */
        break;
    } /* endswitch */

    /******************************************************************/
    /* Terminate current token and start a new one if requested       */
    /******************************************************************/
    if ( fEndOfToken && (pTerm < pszCurPos) )
    {
      lFlags = 0L;
      if ( fAllCaps ) lFlags |= TF_ALLCAPS;
      if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
      if ( fNumber )  lFlags |= TF_NUMBER;
      usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                     pusTermListSize,
                                     &usTermBufUsed,
                                     pTerm,
                                     (USHORT)(pszCurPos - pTerm),
                                     (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                     lFlags,
                                     usListType );
      pTerm    = pszCurPos;
      fAllCaps = TRUE;
      fAlNum   = TRUE;
      fNumber  = TRUE;
    } /* endif */

    /******************************************************************/
    /* Handle single character tokens                                 */
    /******************************************************************/
    if ( fSingleToken && !usReturn )
    {
      usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                     pusTermListSize,
                                     &usTermBufUsed,
                                     pszCurPos,
                                     1,
                                     (USHORT)(fOffsList ? (pszCurPos-pszInData) : 0),
                                     TF_NOLOOKUP,
                                     usListType );
      pTerm    = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Handle start of a new sentence                                 */
    /******************************************************************/
    if ( fNewSentence && !usReturn )
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
           (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                       pusTermListSize,
                                       &usTermBufUsed,
                                       " ",
                                       1,
                                       0, // no offset possible
                                       TF_NEWSENTENCE,
                                       usListType );
      } /* endif */
    } /* endif */


    /******************************************************************/
    /* Handle skip flag                                               */
    /******************************************************************/
    if ( fSkip )
    {
      pTerm = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Continue with next character                                   */
    /******************************************************************/
    if ( c != EOS ) pszCurPos++;

  } /* endwhile */

  /********************************************************************/
  /* Process any pending token                                        */
  /********************************************************************/
  if ( !usReturn && (pTerm < pszCurPos) )
  {
    lFlags = 0L;
    if ( fAllCaps ) lFlags |= TF_ALLCAPS;
    if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
    if ( fNumber )  lFlags |= TF_NUMBER;
    usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                   pusTermListSize,
                                   &usTermBufUsed,
                                   pTerm,
                                   (USHORT)(pszCurPos - pTerm),
                                   (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                   lFlags,
                                   usListType );
  } /* endif */

  /*****************************************************************/
  /* terminate the term list                                       */
  /*****************************************************************/
  if ( !usReturn )
  {
    usReturn = MorphAddTermToList( (PSZ *)ppTermList,
                                   pusTermListSize,
                                   &usTermBufUsed,
                                   (PSZ)EMPTY_STRING,
                                   0,
                                   0,
                                   0L,
                                   usListType );
  } /* endif */

  return (usReturn);

} /* end of function TOKENIZE */
#endif

USHORT NTMTokenizeW
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ_W    pszInData,                  // IN : ptr to data being tokenized
  PULONG   pulTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
)
{
  USHORT     usReturn = 0;             // returncode
  ULONG      ulTermBufUsed = 0;        // amount of space used in term buffer
  PSZ_W      pTerm;                    // pointer to current token string
  PSZ_W      pszCurPos;                // current position in input data
  BOOL       fOffsList;                // TRUE = return a offset/length list
  LONG       lFlags;                   // flags for current term/token
  BOOL       fAllCaps, fAlNum, fNumber;// character classification flags
  BOOL       fSkip, fNewSentence,      // token processing flags
             fSingleToken, fEndOfToken;
  CHAR_W     c;
  CHAR_W     d;


  pvLangCB;                            // avoid compiler warnings

  fOffsList  = (usListType == MORPH_OFFSLIST) ||
               (usListType == MORPH_FLAG_OFFSLIST);
  pszCurPos = pszInData;

  /********************************************************************/
  /* Always assume start of a new sentence                            */
  /********************************************************************/
  if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
       (usListType == MORPH_FLAG_OFFSLIST) )
  {
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   L" ",
                                   1,
                                   0, // no offset possible
                                   TF_NEWSENTENCE,
                                   usListType );
  } /* endif */

  /********************************************************************/
  /* Initialize processing flags                                      */
  /********************************************************************/
  pTerm    = pszCurPos;
  fAllCaps = TRUE;
  fAlNum   = TRUE;
  fNumber  = TRUE;

  /********************************************************************/
  /* Work on input data                                               */
  /********************************************************************/
  while ( !usReturn && ((c = *pszCurPos)!= NULC) )
  {
    /******************************************************************/
    /* Get type of processing required for current character          */
    /******************************************************************/
    fSkip         = FALSE;
    fNewSentence  = FALSE;
    fSingleToken  = FALSE;
    fEndOfToken   = FALSE;

    switch ( c )
    {
      case '.' :
      case '!' :
      case '?' :
        switch ( pszCurPos[1] )
        {
          case ' ':
          case '\n':
          case '\0':
            fNewSentence = TRUE;
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            if ( c != '.' )
            {
              fEndOfToken  = TRUE;
              fSingleToken  = TRUE;
            }
            else
            {
              fAllCaps = FALSE;
              fAlNum   = FALSE;
            } /* endif */
            break;
        } /* endswitch */
        break;

      case '\n' :
      case ' ' :
        fEndOfToken  = TRUE;
        fSkip        = TRUE;
        break;

      case '$' :
      case '#' :
        fAllCaps = FALSE;
        fAlNum   = FALSE;
        break;

      case ':' :
        
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        
        break;

      case '/' :
      case '\\' :
      case ',' :
      case ';' :
        switch ( pszCurPos[1] )
        {
          case ' ':
          case '\n':
          case '\0':
            fEndOfToken  = TRUE;
            fSingleToken  = TRUE;
            break;
          default:
            fAllCaps = FALSE;
            fAlNum   = FALSE;
            fNumber  = FALSE;
            break;
        } /* endswitch */
        break;

      case '-' :            // dash within word or as 'Gedankenstrich'
        if ( ((d = pszCurPos[1]) == ' ') || (d == '\n') || ( d == '\0' ) )
        {
          fEndOfToken  = TRUE;
          fSingleToken  = TRUE;
        } /* endif */
        break;

      case '(' :
      case ')' :
      case '\"' :
        fEndOfToken  = TRUE;
        fSingleToken  = TRUE;
        break;

      default:
        if ( fAlNum ) //&& !chIsText[c] )
        {
			PUCHAR pTextTable;
			
      UtlQueryCharTable( IS_TEXT_TABLE, &pTextTable );
			
      if ( !pTextTable[c] )
			{
				fAlNum = FALSE;
			}
        } /* endif */
        if ( fNumber && !iswdigit(c) )
        {
          fNumber  = FALSE;
        } /* endif */
        if ( fAllCaps && (!iswalpha(c) || iswlower(c)) )
        {
          fAllCaps = FALSE;
        } /* endif */
        break;
    } /* endswitch */

    /******************************************************************/
    /* Terminate current token and start a new one if requested       */
    /******************************************************************/
    if ( fEndOfToken && (pTerm < pszCurPos) )
    {
      lFlags = 0L;
      if ( fAllCaps ) lFlags |= TF_ALLCAPS;
      if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
      if ( fNumber )  lFlags |= TF_NUMBER;
      usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                     pulTermListSize,
                                     &ulTermBufUsed,
                                     pTerm,
                                     (USHORT)(pszCurPos - pTerm),
                                     (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                     lFlags,
                                     usListType );
      pTerm    = pszCurPos;
      fAllCaps = TRUE;
      fAlNum   = TRUE;
      fNumber  = TRUE;
    } /* endif */

    /******************************************************************/
    /* Handle single character tokens                                 */
    /******************************************************************/
    if ( fSingleToken && !usReturn )
    {
      usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                     pulTermListSize,
                                     &ulTermBufUsed,
                                     pszCurPos,
                                     1,
                                     (USHORT)(fOffsList ? (pszCurPos-pszInData) : 0),
                                     TF_NOLOOKUP,
                                     usListType );
      pTerm    = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Handle start of a new sentence                                 */
    /******************************************************************/
    if ( fNewSentence && !usReturn )
    {
      if ( (usListType == MORPH_FLAG_ZTERMLIST) ||
           (usListType == MORPH_FLAG_OFFSLIST) )
      {
        usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                       pulTermListSize,
                                       &ulTermBufUsed,
                                       L" ",
                                       1,
                                       0, // no offset possible
                                       TF_NEWSENTENCE,
                                       usListType );
      } /* endif */
    } /* endif */


    /******************************************************************/
    /* Handle skip flag                                               */
    /******************************************************************/
    if ( fSkip )
    {
      pTerm = pszCurPos + 1;
    } /* endif */

    /******************************************************************/
    /* Continue with next character                                   */
    /******************************************************************/
    if ( c != EOS ) pszCurPos++;

  } /* endwhile */

  /********************************************************************/
  /* Process any pending token                                        */
  /********************************************************************/
  if ( !usReturn && (pTerm < pszCurPos) )
  {
    lFlags = 0L;
    if ( fAllCaps ) lFlags |= TF_ALLCAPS;
    if ( !fAlNum )  lFlags |= TF_NOLOOKUP;
    if ( fNumber )  lFlags |= TF_NUMBER;
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   pTerm,
                                   (USHORT)(pszCurPos - pTerm),
                                   (USHORT)(fOffsList ? (pTerm-pszInData) : 0),
                                   lFlags,
                                   usListType );
  } /* endif */

  /*****************************************************************/
  /* terminate the term list                                       */
  /*****************************************************************/
  if ( !usReturn )
  {
    usReturn = MorphAddTermToList2W( (PSZ_W *)ppTermList,
                                   pulTermListSize,
                                   &ulTermBufUsed,
                                   (PSZ_W)EMPTY_STRING,
                                   0,
                                   0,
                                   0L,
                                   usListType );
  } /* endif */

  return (usReturn);

} /* end of function TOKENIZE */


