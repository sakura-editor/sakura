/*     bregexp.h      
	external use header file 
						1999.11.22  T.Baba
*/
/*
 *	2002.08.24	modified by K2
 *	2006.08.28	modified by K.Takata
 */
// 2007.09.13 kobake ACODE�r���h��UNICODE�R�[�������悤�ɕύX


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
int BMatchW2(WCHAR *str, WCHAR *target, WCHAR *targetstartp, WCHAR *targetendp,
		int one_shot,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstW2(WCHAR *str, WCHAR *target, WCHAR *targetstartp, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg, BCallBack callback);
#else
/* Original */
BREGEXPAPI
int BMatchW2(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstW2(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);

/* Sakura Editor */
BREGEXPAPI
int BMatchExW2(WCHAR *str, WCHAR *targetbegp, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSubstExW2(WCHAR *str, WCHAR *targetbegp, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
#endif


BREGEXPAPI
int BTransW2(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
int BSplitW2(WCHAR *str, WCHAR *target, WCHAR *targetendp,
		int limit, BREGEXP_W **rxp, WCHAR *msg);
BREGEXPAPI
void BRegfreeW2(BREGEXP_W *rx);

BREGEXPAPI
WCHAR *BRegexpVersionW2(void);


#if defined(__cplusplus)
}
#endif

