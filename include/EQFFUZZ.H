
#ifndef _EQFFUZZ_H_
#define _EQFFUZZ_H_

#define ALPHABETIC   1
#define NUMERIC      2
#define NOTALNUM     3

#ifdef _DEBUG
//  #define EQFBFUZZ_LOG
#endif

/**********************************************************************/
/* the following structure specifies a 'snake' from the point (x,y)   */
/* to the point (u,v)                                                 */
/**********************************************************************/
typedef struct _SNAKEPTS
{
  SHORT sX;                         // x-edge of start of diagonal
  SHORT sY;                         // y-edge of start of diagonal
  SHORT sU;                         // x-edge of stop of diagonal
  SHORT sV;                         // y-edge of stop of diagonal
} SNAKEPTS, *PSNAKEPTS;

typedef struct _LCSTOKEN
{
  PFUZZYTOK pTokenList;
  PFUZZYTOK pBackList;
  SHORT sStart;
  SHORT sStop;
  SHORT sTotalLen;
} LCSTOKEN, *PLSCTOKEN;




//static 
SHORT CompFuzzyTok (   PFUZZYTOK , PFUZZYTOK,  BOOL );

/**********************************************************************/
/* prototypes for static functions ...                                */
/**********************************************************************/
//static 
VOID MakeHashValue ( PULONG, USHORT, PSZ_W, PULONG );



//static 
BOOL PrepareTokens ( PLOADEDTABLE, PBYTE, std::vector<TOKENENTRY>&, PSZ_W, SHORT, PFUZZYTOK *, ULONG );
//static 
BOOL FuzzyReplace ( PSZ_W, PSZ_W, PSZ_W, PREPLLIST, PREPLLIST );
//static 
VOID  TransferSource( PSZ_W, PSZ_W, PSZ_W );
//static 
SHORT TokStrCompare ( PFUZZYTOK, PFUZZYTOK);
//static 
PFUZZYTOK SplitTokens (PFUZZYTOK, USHORT, SHORT, int, PSZ_W);
//static  
//static 
SHORT Snake ( PFUZZYTOK, PFUZZYTOK, SHORT, SHORT, SHORT, SHORT, BOOL);
//static 
SHORT FindMiddleSnake( PFUZZYTOK, PFUZZYTOK, PFUZZYTOK, PFUZZYTOK,
                             SHORT, SHORT, PSNAKEPTS, BOOL );
//static 
VOID LCS( LCSTOKEN, LCSTOKEN , BOOL);
//static 
BOOL EQFBTokCountDiff(PFUZZYTOK, PFUZZYTOK,
                             USHORT, USHORT, PUSHORT);
//static
 BOOL EQFBMarkModDelIns( PFUZZYTOK, PFUZZYTOK, PFUZZYTOK*, PFUZZYTOK *,
                               USHORT, USHORT);
//static 
void EQFBSimplifyAndCountMarks (PFUZZYTOK, PUSHORT, PUSHORT);

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



#endif //_EQFFUZZ_H_



