//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2015, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

//#include <tchar.h>
#include "EQF.H"
#include "LogWrapper.h"
#include "OSWrapper.h"

BOOL IsDBCS_CP( ULONG ulCP )
{
  BOOL   fIsDBCS = FALSE;

  switch(ulCP)
  {
//    case 874:                          // Thai  -- in TP5.5.2 under Unicode not necessary anymore
    case 932:                          // Japanese
    case 943:                          // Japanese2
    case 936:                          // Simplified Chinese
    case 949:                          // Korean
    case 950:                          // Chinese (Traditional)
    case 1351:                         //
      fIsDBCS = TRUE;
      break;
  } /* endswitch */

  return(fIsDBCS);
}

