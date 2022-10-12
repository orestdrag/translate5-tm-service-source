/*! match list processing functions
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
*/


int TAMatchListInit
(
  PTAINPUT    pTAIn
);



int TAMatchListTerminate
(
  PTAINPUT    pTAIn
);


int TAMatchListProcess
(
  PTAINPUT    pTAIn,
  PSZ_W       pszSegment,
  PSZ_W       pszProposal,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness
);


int TABalanceList
(
  PSZ         pszFolder,                         // folder object name
  PSZ         pszDocs,                           // ptr to list of documents
  USHORT      usFiles                            // number of source files
);

