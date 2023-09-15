//+----------------------------------------------------------------------------+
//|EQFNTCR.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   G.Dreckschmidt                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  New Translation Memory                                        |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.2 $ ----------- 11 Jan 2005
// GQ: - implicitely open created files in ASD_LOCKED mode
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
// 
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.2 $ ----------- 18 Sep 2001
// -- RJ: Cleanup
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 14 Feb 2000
// - added handling for language group table
//
//
// $Revision: 1.3 $ ----------- 10 Jan 2000
// - added support for large name tables
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFNTCR.CV_   1.2   13 May 1997 10:24:26   BUILD  $
 *
 * $Log:   J:\DATA\EQFNTCR.CV_  $
 *
 *    Rev 1.2   13 May 1997 10:24:26   BUILD
 * - handling for splitted usVersion field of signature record
 *
 *    Rev 1.1   24 Feb 1997 11:49:36   BUILD
 * - added support for long document names
 *
 *    Rev 1.0   09 Jan 1996 09:20:18   BUILD
 * Initial revision.
*/
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_MORPH
#define INCL_EQF_DAM
#define INCL_EQF_ASD
#include <EQF.H>                  // General Translation Manager include file

#define INCL_EQFMEM_DLGIDAS
#include <tm.h>               // Private header file of Translation Memory
#include <EQFMORPI.H>
#include "FilesystemWrapper.h"
#include "LogWrapper.h"
//+----------------------------------------------------------------------------+
