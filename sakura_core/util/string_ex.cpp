/*! @file */
#include "StdAfx.h"
#include "string_ex.h"
#include "charset/charcode.h"
#include "util/std_macro.h"
#include <limits.h>

int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag );



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           文字                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       拡張・独自実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	大文字小文字を同一視する文字列比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２

	@retval 0	一致
 */
int __cdecl my_stricmp( const char *s1, const char *s2 )
{
	/* チェックする文字数をuint最大に設定する */
	//return my_internal_icmp( s1, s2, (unsigned int)(~0), 0, true );
	return my_internal_icmp( s1, s2, UINT_MAX, 0, true );
}

/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２
	@param n [in] 文字長

	@retval 0	一致
 */
int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n )
{
	return my_internal_icmp( s1, s2, (unsigned int)n, 1, true );
}


LPWSTR wcscpyn(LPWSTR lpString1,LPCWSTR lpString2,int iMaxLength)
{
	size_t len2=wcslen(lpString2);
	if((int)len2>iMaxLength-1)len2=iMaxLength-1;
	wmemcpy(lpString1,lpString2,len2);
	lpString1[len2]=L'\0';
	return lpString1;
}


/*
	TCHAR と WCHAR または ACHAR の変換関数
*/

ACHAR* tcstostr( ACHAR* dest, const TCHAR* src, size_t count){
	TCHAR* pr = const_cast<TCHAR*>(src);
	ACHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<ACHAR>(*pr);
		++pw;
	}
	return pw;
}
WCHAR* tcstostr( WCHAR* dest, const TCHAR* src, size_t count){
	TCHAR* pr = const_cast<TCHAR*>(src);
	WCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<WCHAR>(*pr);
		++pw;
	}
	return pw;
}

TCHAR* strtotcs( TCHAR* dest, const ACHAR* src, size_t count )
{
	ACHAR* pr = const_cast<ACHAR*>(src);
	TCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}
TCHAR* strtotcs( TCHAR* dest, const WCHAR* src, size_t count )
{
	WCHAR* pr = const_cast<WCHAR*>(src);
	TCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}


/*! 文字数制限機能付きstrncpy

	コピー先のバッファサイズから溢れないようにstrncpyする。
	バッファが不足する場合には2バイト文字の切断もあり得る。
	末尾の\0は付与されないが、コピーはコピー先バッファサイズ-1までにしておく。

	@param dst [in] コピー先領域へのポインタ
	@param dst_count [in] コピー先領域のサイズ
	@param src [in] コピー元
	@param src_count [in] コピーする文字列の末尾

	@retval 実際にコピーされたコピー先領域の1つ後を指すポインタ

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	auto_memcpy( dst, src, src_count );
	return dst + src_count;
}


const wchar_t* wcsistr( const wchar_t* s1, const wchar_t* s2 )
{
	size_t len2=wcslen(s2);
	const wchar_t* p=s1;
	const wchar_t* q=wcschr(s1,L'\0')-len2;
	while(p<=q){
		if(auto_memicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

const char* stristr(const char* s1, const char* s2)
{
	//$ 日本語考慮してないので、あんまり役に立たない版。stristr_jを使うのが望ましい。
	size_t len2=strlen(s2);
	const char* p=s1;
	const char* q=strchr(s1,L'\0')-len2;
	while(p<=q){
		if(auto_memicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strchri→strichr_j
*/
const char* strichr_j( const char* s1, char c2 )
{
	if(c2==0)return ::strchr(s1,0); //文字列終端を探すためにc2に0を渡した場合も、正しく処理されるように。 2007.10.16 kobake

	int C2 = my_toupper( c2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_toupper( *p1 ) == C2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strchr→strchr_j
*/
const char* strchr_j(const char* str, char c)
{
	if(c==0)return ::strchr(str,0); //文字列終端を探すためにcに0を渡した場合も、正しく処理されるように。 2007.10.16 kobake

	for( const char* p1 = str; *p1; p1++ ){
		if( *p1 == c ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()の2byte code対応版

	@date 2005.04.07 MIK 新規作成
	@date 2007.10.21 kobake 関数名変更: my_strstr→strstr_j
*/
const char* strstr_j(const char* s1, const char* s2)
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( strncmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()の大文字小文字同一視版

	@note
	Windows APIにあるStrStrIはIE4が入っていないPCでは使用不可のため
	独自に作成

	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strstri→stristr_j
*/
const char* stristr_j( const char* s1, const char* s2 )
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_strnicmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           互換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#if (defined(_MSC_VER) && _MSC_VER<1400) \
	|| (defined(__MINGW32__) && (!defined(MINGW_HAS_SECURE_API) || MINGW_HAS_SECURE_API != 1)) //VS2005より前なら

errno_t wcscat_s(wchar_t* szDst, size_t nDstCount, const wchar_t* szSrc)
{
	// 本物は _set_invalid_parameter_handler で設定されたハンドラが起動します
	if(!szDst)return EINVAL;
	if(!szSrc)return EINVAL;

	size_t nDstLen=wcslen(szDst);
	if(nDstLen>=nDstCount)return EINVAL;

	size_t nSrcCount=wcslen(szSrc)+1;
	wchar_t* p=&szDst[nDstLen];           //追加場所
	int nRestCount = nDstCount-(p-szDst); //szDstに追加できる要素数

	//はみ出さない
	if((int)nSrcCount<=nRestCount){
		wmemmove(p,szSrc,nSrcCount);
	}
	//はみ出す
	else{
		return ERANGE;
		//wmemmove(p,szSrc,nRestCount-1); p[nRestCount-1]=L'\0';
	}

	return 0;
}

errno_t strcat_s(char *dest, size_t num, const char *src)
{
	if(!dest || !src) return EINVAL;
	size_t size1 = strnlen(dest, num);
	if(size1 == num) return EINVAL; // destが未終了
	if(num <= size1+strlen(src)) return ERANGE;
	strcat(dest, src);
	return 0;
}
errno_t strcpy_s(char *dest, size_t num, const char *src)
{
	if(!dest || !src) return EINVAL;
	if(num <= strlen(src)) return ERANGE;
	strcpy(dest, src);
	return 0;
}
errno_t wcscpy_s(wchar_t *dest, size_t num, const wchar_t *src)
{
	if(!dest || !src) return EINVAL;
	if(num <= wcslen(src)) return ERANGE;
	wcscpy(dest, src);
	return 0;
}
errno_t strncpy_s(char *dest, size_t num, const char *src, size_t count)
{
	if(!dest || !src) return EINVAL;
	if(num == 0) return EINVAL;
	if(count == _TRUNCATE) {
		memcpy(dest, src, t_min(num, strlen(src)+1));
		dest[num-1] = '\0';
	} else {
		if(num <= count) { *dest = '\0'; return EINVAL; }
		memcpy(dest, src, count+1);
	}
	return 0;
}
errno_t wcsncpy_s(wchar_t *dest, size_t num, const wchar_t *src, size_t count)
{
	if(!dest || !src) return EINVAL;
	if(num == 0) return EINVAL;
	if(count == _TRUNCATE) {
		wmemcpy(dest, src, t_min(num, wcslen(src)+1));
		dest[num-1] = L'\0';
	} else {
		if(num <= count) { *dest = L'\0'; return EINVAL; }
		wmemcpy(dest, src, count+1);
	}
	return 0;
}
size_t strnlen(const char *str, size_t num)
{
	for(size_t i = 0; i < num; ++i)
	{
		if(str[i] == '\0') return i;
	}
	return num;
}
size_t wcsnlen(const wchar_t *str, size_t num)
{
	for(size_t i = 0; i < num; ++i)
	{
		if(str[i] == L'\0') return i;
	}
	return num;
}
int vsprintf_s(char *buf, size_t num, const char *fmt, va_list vaarg)
{
	// 手抜き
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = '\0';
	return _vsnprintf(buf, num-1, fmt, vaarg);
}
int vswprintf_s(wchar_t *buf, size_t num, const wchar_t *fmt, va_list vaarg)
{
	// 手抜き
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnwprintf(buf, num-1, fmt, vaarg);
}
int vsnprintf_s(char *buf, size_t num, size_t count, const char *fmt, va_list vaarg)
{
	// 手抜き
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnprintf(buf, num-1, fmt, vaarg);
}
int _vsnwprintf_s(wchar_t *buf, size_t num, size_t count, const wchar_t *fmt, va_list vaarg)
{
	// 手抜き
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnwprintf(buf, num-1, fmt, vaarg);
}
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      文字コード変換                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//SJIS→UNICODE。終端にL'\0'を付けてくれる版。
size_t mbstowcs2(wchar_t* dst,const char* src,size_t dst_count)
{
	size_t ret=::mbstowcs(dst,src,dst_count-1);
	dst[ret]=L'\0';
	return ret;
}
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount)
{
	int ret=MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcCount,
		pDst,
		nDstCount-1
	);
	pDst[ret]=L'\0';
	return (size_t)ret;
}

//UNICODE→SJIS。終端に'\0'を付けてくれる版。
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count)
{
	size_t ret=::wcstombs(dst,src,dst_count-1);
	dst[ret]='\0';
	return ret;
}

//SJIS→UNICODE。戻り値はnew[]で確保して返す。
wchar_t* mbstowcs_new(const char* src)
{
	size_t new_length=mbstowcs(NULL,src,0);
	wchar_t* ret=new wchar_t[new_length+1];
	mbstowcs(ret,src,new_length);
	ret[new_length]=L'\0';
	return ret;
}
wchar_t* mbstowcs_new(const char* pSrc, int nSrcLen, int* pnDstLen)
{
	//必要な領域サイズ
	int nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);
	
	//確保
	wchar_t* pNew = new wchar_t[nNewLength+1];

	//変換
	nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pNew,
		nNewLength
	);
	pNew[nNewLength] = L'\0';
	if( pnDstLen ){
		*pnDstLen = nNewLength;
	}
	return pNew;
}

//UNICODE→SJIS。戻り値はnew[]で確保して返す。
char* wcstombs_new(const wchar_t* src)
{
	return wcstombs_new(src,wcslen(src));
}
//戻り値はnew[]で確保して返す。
char* wcstombs_new(const wchar_t* pSrc,int nSrcLen)
{
	//必要な領域サイズ
	int nNewLength = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//確保
	char* pNew = new char[nNewLength+1];

	//変換
	nNewLength = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pNew,
		nNewLength,
		NULL,
		NULL
	);
	pNew[nNewLength] = '\0';

	return pNew;
}

//SJIS→UNICODE。戻り値はvectorとして返す。
void mbstowcs_vector(const char* src, std::vector<wchar_t>* ret)
{
	mbstowcs_vector(src,strlen(src),ret);
}

//※戻り値retにおいて、ret->size()が文字列長ではないことに注意。正しくは、(ret->size()-1)が文字列長となる。
void mbstowcs_vector(const char* pSrc, int nSrcLen, std::vector<wchar_t>* ret)
{
	//必要な容量
	int nNewLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);

	//確保
	ret->resize(nNewLen+1);

	//変換
	nNewLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		&(*ret)[0],
		nNewLen
	);
	(*ret)[nNewLen]=L'\0';
}


//UNICODE→SJIS。戻り値はvectorとして返す。
void wcstombs_vector(const wchar_t* src, std::vector<char>* ret)
{
	wcstombs_vector(src,wcslen(src),ret);
}
void wcstombs_vector(const wchar_t* pSrc, int nSrcLen, std::vector<char>* ret)
{
	//必要な容量
	int nNewLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//確保
	ret->resize(nNewLen + 1);

	//変換
	nNewLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		&(*ret)[0],
		nNewLen,
		NULL,
		NULL
	);
	(*ret)[nNewLen]='\0';
}

#ifdef _UNICODE
	size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount)
	{
		wcsncpy_s(wszDst, nDstCount, tszSrc, _TRUNCATE);
		return wcslen(wszDst);
	}
	size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount)
	{
		return wcstombs2(szDst, tszSrc, nDstCount);
	}
	size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount)
	{
		wcsncpy_s(tszDst, nDstCount, wszSrc, _TRUNCATE);
		return wcslen(tszDst);
	}
	size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount)
	{
		return mbstowcs2(tszDst, szSrc, nDstCount);
	}
	int _tctomb(const TCHAR* p,ACHAR* mb)
	{
		return wctomb(mb,*p);
	}
	int _tctowc(const TCHAR* p,WCHAR* wc)
	{
		*wc=*p;
		return 1;
	}
#else
	size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount)
	{
		return mbstowcs2(wszDst, tszSrc, nDstCount);
	}
	size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount)
	{
		strncpy_s(szDst, nDstCount, tszSrc, _TRUNCATE);
		return strlen(szDst);
	}
	size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount)
	{
		return wcstombs2(tszDst, wszSrc, nDstCount);
	}
	size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount)
	{
		strncpy_s(tszDst, nDstCount, szSrc, _TRUNCATE);
		return strlen(tszDst);
	}
	int _tctomb(const TCHAR* tc,ACHAR* mb)
	{
		mb[0]=tc[0];
		if(_IS_SJIS_1(tc[0])){ mb[1]=tc[1]; return 2; }
		return 1;
	}
	int _tctowc(const TCHAR* tc,WCHAR* wc)
	{
		return mbtowc(wc,tc,_IS_SJIS_1(tc[0])?2:1);
	}
#endif



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          メモリ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int wmemicmp(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=skr_towlower(*p1++)-skr_towlower(*p2++);	//非ASCIIも変換
		if(n!=0)return n;
	}
	return 0;
}

int wmemicmp(const WCHAR* p1,const WCHAR* p2)
{
	return wmemicmp(p1,p2, t_max(wcslen(p1), wcslen(p2)));
}

int wmemicmp_ascii(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=my_towlower(*p1++)-my_towlower(*p2++);	//ASCIIのみ変換（高速）
		if(n!=0)return n;
	}
	return 0;
}






/*!
	空白を含むファイル名を考慮したトークンの分割
	
	先頭にある連続した区切り文字は無視する．
	
	@return トークン

	@date 2004.02.15 みく   最適化
	@date 2007.10.21 kobake テンプレート化
*/
//$ いちいち手間かかる。。
namespace{
	template <class T> struct Charset{};
	template <> struct Charset<ACHAR>{ static const ACHAR QUOT= '"'; };
	template <> struct Charset<WCHAR>{ static const WCHAR QUOT=L'"'; };
}
template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	//[in] 文字列バッファ(終端があること)
	int					nLen,		//[in] 文字列の長さ
	int*				pnOffset,	//[in,out] オフセット
	const CHAR_TYPE*	pDelimiter	//[in] 区切り文字
)
{
	int i = *pnOffset;
	CHAR_TYPE* p;

	do {
		bool bFlag = false;	//ダブルコーテーションの中か？
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == Charset<CHAR_TYPE>::QUOT ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( auto_strchr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//空のトークンなら次を探す
	return p;
}
//インスタンス化
template ACHAR* my_strtok(ACHAR*,int,int*,const ACHAR*);
template WCHAR* my_strtok(WCHAR*,int,int*,const WCHAR*);




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef MY_ICMP_MBS
int my_mbtoupper2( int c );
int my_mbtolower2( int c );
int my_mbisalpha2( int c );
#endif  /* MY_ICMP_MBS */

#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目を大文字に変換する。
	@param c [in] 変換する文字コード

	@note
		0x8260 - 0x8279 : Ａ...Ｚ
		0x8281 - 0x829a : ａ...ｚ

	@return 変換された文字コード
*/
int my_mbtoupper2( int c )
{
	if( c >= 0x81 && c <= 0x9a ) return c - (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */



#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目を小文字に変換する。
	@param c [in] 変換する文字コード

	@return 変換された文字コード
*/
int my_mbtolower2( int c )
{
	if( c >= 0x60 && c <= 0x79 ) return c + (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */



#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目か調べる。
	@param c [in] 検査する文字コード

	@retval 1	全角アルファベット２バイト目である
	@retval 0	ちがう
*/
int my_mbisalpha2( int c )
{
	if( (c >= 0x60 && c <= 0x79) || (c >= 0x81 && c <= 0x9a) ) return 1;
	return 0;
}
#endif  /* MY_ICMP_MBS */




/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1   [in] 文字列１
	@param s2   [in] 文字列２
	@param n    [in] 文字長
	@param dcount  [in] ステップ値 (1=strnicmp,memicmp, 0=stricmp)
	@param flag [in] 文字列終端チェック (true=stricmp,strnicmp, false=memicmp)

	@retval 0	一致
	@date 2002.11.29 Moca 0以外の時の戻り値を，「元の値の差」から「大文字としたときの差」に変更
 */
int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
//	2002.11.29 Moca 元の値を保持する必要がなくなったため *_lo, *_upを削除
//	int	c1, c1_lo, c1_up;
//	int	c2, c2_lo, c2_up;
	int 	c1, c2;
	bool	prev1, prev2; /* 前の文字が SJISの１バイト目か */
#ifdef MY_ICMP_MBS
	bool	mba1, mba2;
#endif  /* MY_ICMP_MBS */

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = false;
#ifdef MY_ICMP_MBS
	mba1 = mba2 = false;
#endif  /* MY_ICMP_MBS */

	/* 指定長だけ繰り返す */
	for(i = n; i > 0; i -= dcount)
	{
		/* 比較対象となる文字を取得する */
//		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
//		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);
		c1 = (int)((unsigned int)*p1);
		c2 = (int)((unsigned int)*p2);

		/* 2002.11.29 Moca 文字列の終端に達したか調べる部分 は後方へ移動 */

		/* 文字１の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev1 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev1 = false;
#ifdef MY_ICMP_MBS
			/* 全角文字のアルファベット */
			if( mba1 ){
				mba1 = false;
				if( my_mbisalpha2( c1 ) ){
					c1 = my_mbtoupper2( c1 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c1) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev1 = true;
#ifdef MY_ICMP_MBS
			if( c1 == 0x82 ) mba1 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c1 = my_toupper(c1);
		}

		/* 文字２の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev2 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev2 = false;
#ifdef MY_ICMP_MBS
			/* 全角文字のアルファベット */
			if( mba2 ){
				mba2 = false;
				if( my_mbisalpha2( c2 ) ){
					c2 = my_mbtoupper2( c2 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c2) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev2 = true;
#ifdef MY_ICMP_MBS
			if( c2 == 0x82 ) mba2 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c2 = my_toupper(c2);
		}

		/* 比較する */
//		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* 戻り値は元の文字の差 */
		if( c1 - c2 ) return c1 - c2;	/* 戻り値は大文字に変換した文字の差 */

		/* 2002.11.29 Moca 戻り値を変更したことにより，小文字→大文字変換の後に移動
		   片方だけ NULL文字 の場合は上の比較した時点で return するためその処理は不要 */
		if( flag ){
			/* 文字列の終端に達したか調べる */
			if( ! c1 ) return 0;
		}
		/* ポインタを進める */
		p1++;
		p2++;
	}

	return 0;
}


// skr_towupper() / skr_tolower()
//
// 2010.09.28 ryoji
// BugReport/64: towupper(c) によって U+00e0-U+00fc と U+0020 が同一視される問題の対策
// VC のランタイムは c < 256 の条件ではなぜか locale に対応した "ANSI 系の" 変換テーブル引きを行っている模様
// （Unicode 系変換関数なのに locale が "Japanese" だと c < 256 の範囲では SJIS 用らしき変換テーブルが使われる）
// それでは都合が悪いので c < 256 範囲の変換に "English"(Windows-1252) locale を利用する。
//   ・Unicode の最初の 256 個の符号位置は Windows-1252 の親戚の ISO-8859-1 由来。
//   ・相違は 0x80-0x9F の区間で、Windows-1252 では図形文字、ISO-8859-1(Unicode) では制御文字。
// ※ ランタイムの towupper(c)/tolower(c) が将来期待する動作になったとしてもこの方法を使い続けて問題無いはず
int skr_towupper( int c )
{
#if defined(_MSC_VER) && _MSC_VER>=1400 //VS2005以降なら
	static wchar_t szMap[256];	// c < 256 用の変換テーブル
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towupper( i );	// 自前で変換
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// 無変換（制御コード部）
		for( ; i < 255; i++ ) szMap[i] = _towupper_l( (wchar_t)i, locale );	// "English"localeで変換
		szMap[255] = 0x0178;	// Windows-1252 だと 0x9f(制御文字域) にマップしてしまうので
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towupper( (wchar_t)c );
}

int skr_towlower( int c )
{
#if defined(_MSC_VER) && _MSC_VER>=1400 //VS2005以降なら
	static wchar_t szMap[256];	// c < 256 用の変換テーブル
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towlower( i );	// 自前で変換
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// 無変換（制御コード部）
		for( ; i < 256; i++ ) szMap[i] = _towlower_l( (wchar_t)i, locale );	// "English"localeで変換
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towlower( (wchar_t)c );
}


//! wcがasciiなら0-127のまま返す。それ以外は0を返す。
inline static uchar_t wc_to_c(wchar_t wc)
{
#if 0
//! wcがSJIS1バイト文字ならcharに変換して0～255を返す。SJIS2バイト文字なら0を返す。
	char buf[3]={0,0,0};
	int ret=wctomb(buf,wc);
	if(ret==-1)return 0;   //エラー
	if(buf[1]!=0)return 0; //エラー扱い
	return buf[0] <= 0x7F ? buf[0]: 0; //1バイトで表せたので、これを返す  2011.12.17 バッファオーバーランの修正
#endif
	// 2011.12.15 wctombを使わない版
	if(wc <= 0x7F){
		return (uchar_t)wc;
	}
	return 0;
}

/*!
	文字列がURLかどうかを検査する。
	
	@retval TRUE URLである
	@retval FALSE URLでない
	
	@note 関数内に定義したテーブルは必ず static const 宣言にすること(性能に影響します)。
		url_char の値は url_table の配列番号+1 になっています。
		新しい URL を追加する場合は #define 値を修正してください。
		url_table は頭文字がアルファベット順になるように並べてください。

	2002.01.24 MIK
	2007.10.23 kobake UNICODE対応。//$ wchar_t専用のテーブル(または判定ルーチン)を用意したほうが効率は上がるはずです。
*/
BOOL IsURL(
	const wchar_t*	pszLine,	//!< [in]  文字列
	int				nLineLen,	//!< [in]  文字列の長さ
	int*			pnMatchLen	//!< [out] URLの長さ
)
{
	// TODO: この関数は、UNCアドレスも含めて見直した方がよさげ by berryzplus
	struct _url_table_t {
		wchar_t	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* アルファベット順 */
		{ L"file://",		7,	false }, /* 1 */
		{ L"ftp://",		6,	false }, /* 2 */
		{ L"gopher://",		9,	false }, /* 3 */
		{ L"http://",		7,	false }, /* 4 */
		{ L"https://",		8,	false }, /* 5 */
		{ L"mailto:",		7,	true  }, /* 6 */
		{ L"news:",			5,	false }, /* 7 */
		{ L"nntp://",		7,	false }, /* 8 */
		{ L"prospero://",	11,	false }, /* 9 */
		{ L"telnet://",		9,	false }, /* 10 */
		{ L"tp://",			5,	false }, /* 11 */	//2004.02.02
		{ L"ttp://",		6,	false }, /* 12 */	//2004.02.02
		{ L"wais://",		7,	false }, /* 13 */
		{ L"{",				0,	false }  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* テーブルの保守性を高めるための定義 */
	constexpr char urF = 1;
	constexpr char urG = 3;
	constexpr char urH = 4;
	constexpr char urM = 6;
	constexpr char urN = 7;
	constexpr char urP = 9;
	constexpr char urT = 10;
	constexpr char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* あと128バイト犠牲にすればif文を2箇所削除できる */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const wchar_t *p = pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( wc_to_c(*p)==0 ) return FALSE;	/* 2バイト文字 */
	if( 0 < url_char[wc_to_c(*p)] ){	/* URL開始文字 */
		for(urlp = &url_table[url_char[wc_to_c(*p)]-1]; urlp->name[0] == wc_to_c(*p); urlp++){	/* URLテーブルを探索 */
			if( (urlp->length <= nLineLen) && (auto_memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URLヘッダは一致した */
				p += urlp->length;	/* URLヘッダ分をスキップする */
				if( urlp->is_mail ){	/* メール専用の解析へ */
					if( IsMailAddress(p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* 通常の解析へ */
					if( wc_to_c(*p)==0 || (!(url_char[wc_to_c(*p)])) ) break;	/* 終端に達した */
				}
				if( i == urlp->length ) return FALSE;	/* URLヘッダだけ */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen); // TODO: FALSE じゃなくて？
	// この関数の作成目的は「指定した文字列がURLかどうかを判定すること」だったと推定される。
	// 先行する「mailto:」のないメールアドレスはURLではないのでここはFALSEを返すべき。
	// 「URLとメールアドレスにリンクを付けたい」を叶えるための措置と考えられるが、もっといい解決策がありそうに思う。
	// とはいえ、すぐに着手できそうな状況ではないので、備忘目的でコメントだけ残しておく。 by berryzplus
}

// 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept;

// 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept;


/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す
	@date 2016.04.27 記号類を許可
	@date 2018.09.09 RFC準拠
*/
BOOL IsMailAddress( const wchar_t* pszBuf, int nBufLen, int* pnAddressLength )
{
	// RFC5321による mailbox の最大文字数
	constexpr ptrdiff_t MAX_MAILBOX = 255; //255オクテット

	// mailboxの最小文字数（これより短いと構成要素を含めなくなる）
	//   例) a@z.jp
	//   1(@手前) + 1(@) + 1(ドメイン) + 1(.) + 2(TLD/ccTLD) = 6
	constexpr ptrdiff_t MIN_MAILBOX = 6;

	// 想定しないパラメータは前半チェックの前に弾く
	if (pszBuf == nullptr || nBufLen < MIN_MAILBOX) return FALSE;

	// メールアドレスには必ず＠が含まれる
	const wchar_t* pszAtmark;

	// メールアドレス前半部分(＠の手前)をチェックする
	if (!IsMailAddressLocalPart(pszBuf, pszBuf + nBufLen, &pszAtmark)) {
		return FALSE;
	}

	// メールアドレスの終了位置を受け取るポインタを宣言する
	const wchar_t* pszEndOfMailBox;

	// メールアドレス後半部分(＠の後ろ)をチェックする
	if (!IsMailAddressDomain(pszAtmark, pszBuf + nBufLen, &pszEndOfMailBox))
	{
		return FALSE;
	}

	// 全体の長さが制限を超えていないかチェックする
	const auto cchAddressLength = pszEndOfMailBox - pszBuf;
	if (MAX_MAILBOX < cchAddressLength)
	{
		return FALSE; // 文字数オーバー
	}

	if (pnAddressLength != nullptr)
	{
		*pnAddressLength = cchAddressLength;
	}
	return TRUE;
}

/*!
 * 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
 *
 * 高速化のため単純化した条件でチェックしている
 * 参照する標準は RFC5321
 * @see http://srgia.com/docs/rfc5321j.html
 */
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept
{
	// RFC5321による local-part の最大文字数
	constexpr ptrdiff_t MAX_LOCAL_PART = 64; //64オクテット

	// 関数仕様
	assert(pszStart != pszEnd); // 長さ0の文字列をチェックしてはならない
	assert(pszStart < pszEnd); // 開始位置と終了位置は逆転してはならない

	// 出力値を初期化する
	*ppszAtmark = nullptr;

	// 文字列が二重引用符で始まっているかチェックして結果を保存
	const bool quoted = (L'"' == *pszStart);

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszStart + (quoted ? 1 : 0);
	auto pszScanEnd = std::min(pszStart + MAX_LOCAL_PART + 1, pszEnd);

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszScanEnd)
	{
		switch (*pszScan)
		{
		case L'@':
			if (pszStart == pszScan)
			{
				return false; // local-partは1文字以上なのでNG
			}
			if (quoted)
			{
				return false; // 二重引用符で始まる場合、終端にも二重引用符が必要なのでNG
			}
			*ppszAtmark = pszScan;
			return true; // ここが正常終了
		case L'\\': // エスケープ記号
			if (pszScan + 1 == pszScanEnd || pszScan[1] < L'\x20' || L'\x7E' < pszScan[1])
			{
				return false;
			}
			pszScan++; // エスケープ記号の分1文字進める
			break;
		case L'"': // 二重引用符
			if (quoted && pszScan + 1 < pszScanEnd && L'@' == pszScan[1])
			{
				*ppszAtmark = &pszScan[1];
				return true; // ここは準正常終了。正常終了とはあえて区別しない。
			}
			return false; // 末尾以外に現れるエスケープされてない二重引用符は不正
		}
		pszScan++;
	}
	return false; // 文字数オーバー
}

/*!
 * 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
 */
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept
{
	// ccTLDの最小文字数
	constexpr ptrdiff_t MIN_TLD = 2;

	// ドメインの最小文字数
	constexpr ptrdiff_t MIN_DOMAIN = 3;

	// ドメインの最大文字数
	constexpr ptrdiff_t MAX_DOMAIN = 63;

	// 関数仕様
	assert(pszAtmark < pszEnd); // @位置と終了位置は逆転してはならない
	assert(L'@' == *pszAtmark); // @位置にある文字は@でなければならない

	// 出力値を初期化する
	*ppszEndOfMailBox = nullptr;

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszAtmark + 1;
	if (pszScan == pszEnd)
	{
		return false; // @の後ろが0文字、長さが足りない
	}

	auto dotCount = 0;
	auto domainLength = 0;
	auto prevHyphen = false;

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszEnd)
	{
		switch (*pszScan)
		{
		case L'.': // ドット記号
			if (dotCount == 0 && domainLength < MIN_DOMAIN)
			{
				return false; // ドメイン名の最小文字数は3なのでNG
			}
			if (0 < dotCount && domainLength < MIN_TLD)
			{
				// これはco.jpなどを正しく認識させるために必要。
				return false; // ドットで区切られる部分の最小文字数は2なのでNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンに続くドットはNG
			}
			dotCount++;
			domainLength = 0;
			prevHyphen = false;
			break;
		case L'-': // ハイフン記号
			if (domainLength == 0)
			{
				return false; // ドットに続くハイフンはNG
			}
			if (prevHyphen)
			{
				return false; // 連続するハイフンはNG
			}
			domainLength++;
			prevHyphen = true;
			break;
		default:
			if (dotCount == 0)
			{
				return false; // ドメイン部には一つ以上のドット記号が必要なのでNG
			}
			if (domainLength == 0)
			{
				return false; // ドットで終わるドメインはNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンで終わるドメインはNG
			}
			*ppszEndOfMailBox = pszScan;
			return true; // ここも正常終了
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
		case L'G':
		case L'H':
		case L'I':
		case L'J':
		case L'K':
		case L'L':
		case L'M':
		case L'N':
		case L'O':
		case L'P':
		case L'Q':
		case L'R':
		case L'S':
		case L'T':
		case L'U':
		case L'V':
		case L'W':
		case L'X':
		case L'Y':
		case L'Z':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'g':
		case L'h':
		case L'i':
		case L'j':
		case L'k':
		case L'l':
		case L'm':
		case L'n':
		case L'o':
		case L'p':
		case L'q':
		case L'r':
		case L's':
		case L't':
		case L'u':
		case L'v':
		case L'w':
		case L'x':
		case L'y':
		case L'z':
			domainLength++;
			prevHyphen = false;
			break;
		}
		pszScan++;
		if (MAX_DOMAIN < domainLength)
		{
			return false; // 文字数オーバー
		}
	}
	if (pszScan == pszEnd)
	{
		*ppszEndOfMailBox = pszScan;
		return true; // ここが正常終了
	}
	return false;
}
