#ifndef _PROPOSAL_H_INCLUDED_
#define _PROPOSAL_H_INCLUDED_

#include <vector>
#include <iostream>


/*! \brief maximum size of segment data */
#define OTMPROPOSAL_MAXSEGLEN 2048
/*! \brief maximum size of names */
#define OTMPROPOSAL_MAXNAMELEN 256


class TMX_SENTENCE;

/*! \brief Data class for the transport of memory proposals
 *
 * 
 */
class OtmProposal
{  

public:

/*! \brief Constructors */
	OtmProposal();

/*! \brief Destructor */
	~OtmProposal();

  /*! \enum eProposalType
	Proposal types.
*/
	enum eProposalType
	{
		eptUndefined,			/*!< proposal type has not been set or is not available */
		eptManual,	  		/*!< proposal was produced by manual translation */
		eptMachine,	    	/*!< proposal was produced by automatic (machine) translation */
		eptGlobalMemory, 	/*!< proposal is from global memory  */
    eptGlobalMemoryStar /*!< proposal is from global memory, to be marked with an asterisk  */
	};

  /*! \enum eMatchType
	Match types.
*/
	enum eMatchType
	{
		emtUndefined,			/*!< match type has not been set or is not available */
		emtExact,	    		/*!< proposal is an exact match */
		emtExactExact,		/*!< proposal is an exact match, document name and segment number are exact also */
		emtExactSameDoc,	/*!< proposal is an exact match from the same document */
		emtFuzzy,	    		/*!< proposal is a fuzzy match */
		emtReplace 	    	/*!< proposal is a replace match */
	};

  /* operations */

  /* \brief clear all proposal fields 
   */
  void clear();

  /* setters and getters */

   /* \brief get the key of the proposal
     The internal key is the memory internal identifier of the proposal
     \param pszBuffer Pointer to buffer receiving the proposal key
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getInternalKey( char *pszBuffer, int iBufSize );
  
  
  int SetProposalKey 
  (
    unsigned long   ulKey,
    unsigned short  usTargetNum
  );

  /* \brief set the proposal key
     \param pszBuffer Pointer to buffer containing the proposal key
   */
  void setInternalKey( char *pszBuffer );
  
  std::string getInternalKey()const;
  	

  /* \brief get length of proposal source text 
  	 \returns Number of characters in proposal source text
   */
  int getSourceLen();

  std::string GetUpdateTimeStr();

  /* \brief get proposal source text 
     \param pszBuffer Pointer to buffer receiving the proposal source text
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getSource( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal source text
     \param pszBuffer Pointer to buffer containing the proposal source text
   */
  void setSource( wchar_t *pszBuffer );

  /* \brief get length of proposal target text 
  	 \returns Number of characters in proposal target text
   */
  int getTargetLen();

  /* \brief get proposal target text   
     \param pszBuffer Pointer to buffer receiving the proposal target text
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getTarget( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal target text
     \param pszBuffer Pointer to buffer containing the proposal target text
   */
  void setTarget( wchar_t *pszBuffer );
  	
  /* \brief get proposal ID
     \param pszBuffer Pointer to buffer receiving the proposal ID
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getID( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal ID 
     \param pszBuffer Pointer to buffer containing the proposal ID
   */
  void setID( char *pszBuffer );
  	

  /* \brief get proposal document name

    \param pszBuffer Pointer to buffer receiving the document name
    \param iBufSize Size of the buffer in number of characters
  	\returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getDocName( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal document short name
     \param pszBuffer Pointer to buffer containing the document name
   */
  void setDocName( char *pszBuffer );
  	

  /* \brief get proposal segment number
   	\returns proposal segment number
   */
  long getSegmentId();
  	
  /* \brief set the proposal segment number
     \param lSegmentId new segment number of proposal
   */
  void setSegmentId( long lSegmentId );
  	
  /* \brief get proposal source language
     \param pszBuffer Pointer to buffer receiving the proposal source language
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getSourceLanguage( char *pszBuffer, int iBufSize );
  int getOriginalSourceLanguage( char* pszBuffer, int iBufSize );
  /* \brief set the proposal source language
     \param pszBuffer Pointer to buffer containing the proposal source language
   */
  void setSourceLanguage( char *pszBuffer );
  void setOriginalSourceLanguage( char* pszBuffer);

  /* \brief get proposal target language
     \param pszBuffer Pointer to buffer receiving the proposal target language
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getTargetLanguage( char *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal target language
     \param pszBuffer Pointer to buffer containing the proposal target language
   */
  void setTargetLanguage( char *pszBuffer );

  /* \brief get proposal type
     \returns proposal type
   */
  eProposalType getType();
  	
  /* \brief set the proposal type
     \param eType new type of the proposal
   */
  void setType( eProposalType eType );

  /* \brief get match type
     \returns proposal type
   */
  eMatchType getMatchType();
  	
  /* \brief set the match type
     \param eType new type of the proposal
   */
  void setMatchType( eMatchType eType );

  /* \brief get name of proposal author
     \param pszBuffer Pointer to buffer receiving the name of the proposal author
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getAuthor( char *pszBuffer, int iBufSize );
  	
  /* \brief set the name of the proposal author
     \param pszBuffer Pointer to buffer containing the name of the proposal author
   */
  void setAuthor( char *pszBuffer );

  /* \brief get proposal time stamp
     \returns proposal segment number
   */
  long getUpdateTime();
  	
  /* \brief set the proposal time stamp
     \param lTime new time stamp of proposal
   */
  void setUpdateTime( long lUpdateTime );

  /* \brief get proposal fuzziness
     \returns proposal fuzziness
   */
  int getFuzziness();

  /* \brief set the proposal fuzziness
     \param lFuzzinessTime new fuzziness of proposal
   */
  void setFuzziness( long iFuzziness );

  /* \brief get proposal diffs count from fuzzy calculations
    \returns proposal diffs count from fuzzy calculations
  */
  int getDiffs();

  /* \brief set the proposal diffs count from fuzzy calculations
     \param iDiffs new diffs count from fuzzy calculations of proposal
   */
  void setDiffs( long iDiffs );

    /* \brief get proposal words count during fuzzy calculations
     \returns proposal fuzziness
   */
  int getWords();

  /* \brief set the proposal words count during fuzzy calculations
     \param iWords new words count during fuzzy calculations of proposal
   */
  void setWords( long iWords );

  /* \brief get markup table name (format)
     \param pszBuffer Pointer to buffer receiving the name of the markup table name
     \param iBufSize Size of the buffer in number of characters
     \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getMarkup( char *pszBuffer, int iBufSize );
  	
  /* \brief set markup table name (format)
     \param pszBuffer Pointer to buffer containing the markup table name
   */
  void setMarkup( char *pszBuffer );

  /* \brief get length of proposal context
  	 \returns Number of characters in proposal context
   */
  int getContextLen();

  /* \brief get proposal Context 
     \param pszBuffer Pointer to buffer receiving the proposal context
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getContext( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal context
     \param pszBuffer Pointer to buffer containing the proposal context
   */
  void setContext( wchar_t *pszBuffer );

  /* \brief get proposal context ranking
  	\returns proposal context ranking
   */
  int getContextRanking();
  	
  /* \brief set the proposal context ranking
     \param iContextRanking context ranking for the proposal (0..100)
   */
  void setContextRanking( int iContextRanking );

  /* \brief get length of proposal AddInfo text 
  	 \returns Number of characters in proposal AddInfo text
   */
  int getAddInfoLen();

  /* \brief get additional info
     \param pszBuffer Pointer to buffer receiving the additional info
     \param iBufSize Size of the buffer in number of characters
  	 \returns Number of characters copied to pszBuffer including the terminating null character
   */
  int getAddInfo( wchar_t *pszBuffer, int iBufSize );
  	
  /* \brief set the proposal additional information
     \param pszBuffer Pointer to buffer containing the additional info
   */
  void setAddInfo( wchar_t *pszBuffer );

  /* \brief set the proposal memory index
     \param iIndex new value for the memory index
  */
  void setMemoryIndex( int iIndex );

  /* \brief get memory index
     \returns memory index of this proposal
  */
  int getMemoryIndex();

  /* \brief set the DITA replacement list
     \param pList new value for the DITA replacement list
  */
  void setReplacementList( long pList );

  /* \brief get DITA replacement list
     \returns pointer to DITA replacement list
  */
  long getReplacementList();

  /* \brief check if proposal match type is one of the exact match types
   */
  bool isExactMatch();

  /* \brief check if proposal is empty (i.e. has not been used)
   */
  bool isEmpty();

  bool isSourceLangIsPrefered();
  void setIsSourceLangIsPrefered(bool fPref);
  /* \brief check if source and target of proposal is equal
   */
  bool isSourceAndTargetEqual();

  /* \brief check if target strings are identical
     \param otherProposal pointer to second proposal 
  */
  bool isSameTarget( OtmProposal *otherProposal );

  /*! \brief Clear the data of proposals stored in a vector
      \param Proposals reference to a vector containing the proposals
  */
  void static clearAllProposals(
    std::vector<OtmProposal> &Proposals
  );

  /*! \brief Get the number of filled proposals in a proposal list
      \param Proposals reference to a vector containing the proposals
  */
  int static getNumOfProposals(
    std::vector<OtmProposal> &Proposals
  );


  /* \brief assignment operator to copy the datafields from one
    OtmProposal to another one
     \param copyme reference to OtmProposal being copied
   */
  OtmProposal &operator=( const OtmProposal &copyme );

  friend std::ostream & operator<<( std::ostream & o, OtmProposal & proposal );

  std::string getProposalKey();

//protected:

  /*! \brief ID of this proposal */
	char szId[OTMPROPOSAL_MAXNAMELEN];

  /*! \brief Source string of memory proposal  (UTF-16) */
  wchar_t szSource[OTMPROPOSAL_MAXSEGLEN];
	//std::wstring strSource;

	/*! \brief Target string of memory proposal (UTF-16). */
	//std::wstring strTarget;
  wchar_t szTarget[OTMPROPOSAL_MAXSEGLEN];

	/*! \brief Name of document from which the proposal comes from. */
	//std::string strDocName;
	char szDocName[OTMPROPOSAL_MAXNAMELEN];

  /*! \briefCustom Id provided for the segment in request to track segments in response in multifuzzy*/
	char szCustomId[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief Segment number within the document from which the proposal comes from. */
  long lSegmentId;                  

	/*! \brief source language. */
  //std::string strSourceLanguage;
	char szSourceLanguage[OTMPROPOSAL_MAXNAMELEN];
  char szOriginalSourceLanguage[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief target language. */
  //std::string strTargetLanguage;
  char szTargetLanguage[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief origin or type of the proposal. */
  eProposalType eType;

	/*! \brief match type of the proposal. */
  eMatchType eMatch;

	/*! \brief Author of the proposal. */
  // std::string strTargetAuthor;   
  char szTargetAuthor[OTMPROPOSAL_MAXNAMELEN];

	/*! \brief Update time stamp of the proposal. */
  long    lTargetTime;

	/*! \brief Fuzziness of the proposal. */
  int iFuzziness, iDiffs, iWords;

	/*! \brief Markup table (format) of the proposal. */
  char szMarkup[OTMPROPOSAL_MAXNAMELEN];

  /*! \brief Context information of the proposal */
  //std::wstring strContext;  
  wchar_t szContext[OTMPROPOSAL_MAXSEGLEN+1];

  /*! \brief Additional information of the proposal */
  //std::wstring strAddInfo; 
  wchar_t szAddInfo[OTMPROPOSAL_MAXSEGLEN+1];

  /*! \brief Proposal data has been filled flag */
  bool fFilled; 

  /*! \brief Index of memory when looking up in a list of memories */
  int iMemoryIndex; 

  /*! \brief ranking of the context information (0..100) */
  int iContextRanking; 

  /*! \brief list of replacement values */
  long pvReplacementList;

  
  //searchproposal
  TMX_SENTENCE* pInputSentence = nullptr;

  char szMemory[260];
  char szIsoSourceLang[MAX_LANG_LENGTH];
  char szIsoTargetLang[MAX_LANG_LENGTH];
  wchar_t szError[512];
  //char szType[256];
  //char szDateTime[40];
  char szSearchMode[40];
  char szSearchPos[80];
  int iNumOfProposals;
  int iSearchTime;
  wchar_t szSearchString[2050];
  

  // 
  TMCursor currentInternalKey;
  TMCursor nextInternalKey;

  int moveInternalKeyToTheNextPosition() {
          currentInternalKey = nextInternalKey; 
          nextInternalKey.reset();
          return 0;
  }
  bool nextKeyExists()
  {
    return true;
  }

  int setSequentialAccessKey(  char *pszKey);

//otm_put
  //USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  //ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  //TIME_L    lTime;                             //time stamp
  BOOL      fMarkupChanged;                    // Markup does not exist, changed to OTMUTF8 during import
//otm_put end

    
  /*! \brief is source lang is marked as prefered in languages.xml */
  bool fIsoSourceLangIsPrefered = false;
  /*! \brief is target lang is marked as prefered in languages.xml */
  bool fIsoTargetLangIsPrefered = false;

  //searchproposal

  //char errorStr[1000];
};

#include "tm.h"



OtmProposal::eProposalType getMemProposalType( char *pszType );

#endif //_PROPOSAL_H_INCLUDED_
