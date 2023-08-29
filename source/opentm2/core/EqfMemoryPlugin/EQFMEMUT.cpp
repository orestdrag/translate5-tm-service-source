//------------------------------------------------------------------------------
//EQFMEMUT.C
//------------------------------------------------------------------------------
//Copyright Notice:
//
//      Copyright (C) 1990-2015, International Business Machines
//      Corporation and others. All rights reserved
//------------------------------------------------------------------------------
//Description: Utility programs for the memory database
//             administrative system
//------------------------------------------------------------------------------

#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_LIST             // terminology list functions
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <EQF.H>                  // General Translation Manager include file

#include "EQFDDE.H"               // Batch mode definitions
#include <tm.h>               // Private header file of Translation Memory
#include <EQFQDAMI.H>
#include "LogWrapper.h"


/*---------------------------------------------------------------------*\
                           Read AB Grouping
+-----------------------------------------------------------------------+
  Function name      : ReadABGrouping
  Description        : Builds the AB grouping array
  Function Prototype : ReadABGrouping(pszFileName, pszLanguage,
                                                   abABGrouping)
+-----------------------------------------------------------------------+
  Implementation remarks
  ----------------------
  This function reads the language data from the language file and
  builds the AB Grouping array.

  The language file is in the following format:
  Lines beginning with an asterix are comment lines and are ignored.
  Blank lines are ignored.
  Other lines are considered as language information, where the
  information is grouped as follows:
  First line is the language name in English (With no imbedded blanks)
  The language name case is ignored.
  Subsequent lines begin with a number followed by a colon, blanks,
  and a string of characters belonging to the group.
  The groups can appear in any order. And more then once.
  It is an error to have a group number larger than GROUP_NUM-1, or
  to have a letter appearing more than once.
  Characters in the codepage, not appearing in the language data are
  not part of the alphabet and are assigned GROUP_NUM.
  A group is terminated by the line  99: end of group
  e.g.
  English.
  0: abBC
  1: eE
  2: gqQt
  .
  * Ye comment.
  1: Ac
  7: LmMn
  99: end of group

  Return Codes:
  -------------
  OK (0)       - The command ended successfully.
  LANG_FILE_NOT_FOUND - The language file was not found.
  LANGUAGE_NOT_FOUND - The source language does not exist in the
                       language file.
  LANG_FILE_LINE_TOO_LONG - A file line exceeded the allowed length.
  LANG_GROUP_NUM_TOO_LARGE - A non-existing group number in file.
  LANG_CHAR_APPEARS_TWICE - A character was found in 2 group lines.
  Other values - An API call failed.

  Function Calls
  --------------
  GetLangFileLine

  API calls:
  ----------

  C function calls:
  -----------------
\*---------------------------------------------------------------------*/
//USHORT
//ReadABGrouping (PSZ pszFileName,        /* The language file name......*/
//                PSZ pszLanguage,        /* The language name...........*/
//                ABGROUP abABGrouping)   /* AB grouping array...........*/
//{
//  /*-------------------------------------------------------------------*/
//  USHORT      rc;                       /* Returned Rc.....................*/
//  USHORT      usGroup = 0;              /* The group number................*/
//  LANG_LINE   szLine,                   /* Buffer for file reads...........*/
//              szToken;                  /* Buffer for tokens. .............*/
//  PCHAR       pch;                      /* An index variable...............*/
//  BOOL        fMore = TRUE;             /* A GP flag.......................*/
//  PBYTE       pFileData = NULL;         /* pointer to file data            */
//  PBYTE       pLngData;                 /* temp pointer to file data       */
//  ULONG       ulLength;
//  PSZ         pszToken;
//
//  /*-------------------------------------------------------------------*/
//  /* Load the language file. ..........................................*/
//  rc = (USHORT)UtlLoadFileL( pszFileName, (PVOID *)&pFileData, &ulLength, FALSE, FALSE);
//
//  if (rc == TRUE)
//  {
//    pLngData = pFileData;
//
//    /* Read through the file until you find the language name. ........*/
//    do
//    {
//      rc = UtlGetLangFileLine( &pLngData, szLine, MAX_LINE_LENGTH);
//      if (rc == OK)
//      {
//        /* A line was read, compare the language name. ................*/
//        /* Extract the name from the line, and compare to input........*/
//        UtlStripBlanks( szLine );
//        if (!_strcmpi(szLine, pszLanguage))
//        {
//          /* We've found the language tag. ............................*/
//          /* Initialize the abGrouping array...........................*/
//          memset(abABGrouping, (BYTE)GROUP_NUM, CODEPAGE_SIZE-1);
//
//          /* Now read the group data and assign to array...............*/
//          do
//          {
//            rc = UtlGetLangFileLine( &pLngData, szLine, MAX_LINE_LENGTH);
//            if (rc == OK)
//            {
//              /* we've read a line, parse it to get group data. .......*/
//              /* at first isolate group number */
//              pszToken = strtok( szLine, ": " );
//              if ( pszToken )
//              {
//                usGroup =  (USHORT)atoi( pszToken );
//              } /* endif */
//
//              /* then read the grouping data */
//              pszToken = strtok( NULL, ": \r\n" );
//              if ( pszToken )
//              {
//                strcpy( szToken, pszToken );
//              }
//              else
//              {
//                szToken[0] = EOS;
//              } /* endif */
//
//              /* now check that no more tokens are on the line */
//              pszToken = strtok( NULL, ": \r\n" );
//
//              /* if more or less then 2 tokens read stop processing ...*/
//              if ( (pszToken == NULL) && (usGroup > 0) &&
//                   (strlen( szToken ) > 0) )
//              {
//                /* Check that the group value is within the limits.....*/
//                if (usGroup >= GROUP_NUM)
//                {
//                  rc = LANG_GROUP_NUM_TOO_LARGE;
//                }
//                else
//                {
//                  /* The line was parsed, set the array values.........*/
//                  for (pch = szToken; (*pch != NULC); pch++)
//                  {
//                     if (abABGrouping[*pch] != (BYTE)GROUP_NUM)
//                     {
//                       /* The character appeared already...............*/
//                       *pch = NULC;
//                       rc = LANG_CHAR_APPEARS_TWICE;
//                     }
//                     else
//                       abABGrouping[*pch] = (BYTE)usGroup;
//                  }
//                }
//                /* If we had a LANG_GROUP or a LANG_CHAR error, leave..*/
//                fMore = (rc == OK);
//              }
//              else
//                fMore = FALSE; /* finito :-) */
//            } /* Endif UtlGetLangFileLine was OK */
//            else /* An error in UtlGetLangFileLine */
//            {
//              if (rc == TMERR_EOF)
//              {
//                /* No error, it's just the EOF. .......................*/
//                 rc = OK;
//              }
//              fMore = FALSE;
//            } /* Endelse UtlGetLangFileLine not OK */
//          } while (fMore);
//        } /* Endif found language */
//      } /* Endif UtlGetLangFileLine was OK - to find Language */
//      else /* rc <> OK */
//      {
//        /* An error or EOF. ...........................................*/
//        rc = (rc == TMERR_EOF) ? LANGUAGE_NOT_FOUND : rc;
//        fMore = FALSE;
//      } /* Endelse not OK */
//    } while (fMore);
//  } /* Endif UtlLoadFile was OK */
//
//  /* free memory used for file data */
//  UtlAlloc( (PVOID *) &pFileData, 0L, 0L, NOMSG );
//
//  /* Change the return code for file_not_found.........................*/
//  if (rc == ERROR_OPEN_FAILED)
//    rc = LANG_FILE_NOT_FOUND;
//  return (rc);
//} /* End of ReadABGrouping */


typedef struct _CONVERSIONAREA
{
  BTREE BtreeIn;                      // structure for input BTREE
  BTREE BtreeOut;                     // structure for output BTREE
  TMWCHAR  szKey[1024];
  BYTE    bData[0xFFFFFF];
  TMX_SIGN TMXSign;                    // signature record
  char szOutFile[MAX_LONGFILESPEC];
  CHAR szBackupName[MAX_EQF_PATH];
} CONVERSIONAREA, *PCONVERSIONAREA;

