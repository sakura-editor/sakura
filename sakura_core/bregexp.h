/*     bregexp.h      
	external use header file 
						1999.11.22  T.Baba
*/
/*
 *	2002.08.24	modified by K2
 *	2006.08.28	modified by K.Takata
 */
// 2007.09.13 kobake ACODEビルドでUNICODEコールされるように変更


#ifdef _BREGEXP_
/* for internal use */
#define BREGEXPAPI	__declspec(dllexport) 
#define BREGCONST
#else
/* for external use */
#define BREGEXPAPI	__declspec(dllimport) 
#define BREGCONST	const
#endif


typedef struct bregexp {
	BREGCONST WCHAR *outp;		/* result string start ptr  */
	BREGCONST WCHAR *outendp;	/* result string end ptr    */
	BREGCONST int   splitctr;	/* split result counter     */
	BREGCONST WCHAR **splitp;	/* split result pointer ptr     */
	int rsv1;					/* reserved for external use    */
	WCHAR *parap;				/* parameter start ptr ie. "s/xxxxx/yy/gi"  */
	WCHAR *paraendp;			/* parameter end ptr     */
	WCHAR *transtblp;			/* translate table ptr   */
	WCHAR **startp;				/* match string start ptr   */
	WCHAR **endp;				/* match string end ptr     */
	int nparens;				/* number of parentheses */
} BREGEXP_W;

#if defined(_BREGEXP_) || defined(_K2REGEXP_)
typedef int (__stdcall *BCallBack)(int kind, int value, int index);
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#ifdef _K2REGEXP_
/* K2Editor */
BREGEXPAPI
int BMatchW(WCHAR *str, WCHAR *target, WCHAR *targetstartp, WCHAR *targetendp,
		int one_shot,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstW(WCHAR *str, WCHAR *target, WCHAR *targetstartp, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg, BCallBack callback);
#else
/* Original */
BREGEXPAPI
int BMatchW(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstW(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);

/* Sakura Editor */
BREGEXPAPI
int BMatchExW(WCHAR *str, WCHAR *targetbegp, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstExW(WCHAR *str, WCHAR *targetbegp, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
#endif


BREGEXPAPI
int BTransW(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSplitW(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		int limit, BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
void BRegfreeW(BREGEXP_W *rx);

BREGEXPAPI
WCHAR *BRegexpVersionW(void);


#if defined(__cplusplus)
}
#endif

