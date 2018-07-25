#include "StdAfx.h"
#include "string_ex2.h"
#include "charset/charcode.h"
#include "CEol.h"

wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	auto_memcpy( dst, src, src_count );
	return dst + src_count;
}
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src)
{
	return wcs_pushW(dst,dst_count,src,wcslen(src));
}
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src, size_t src_count)
{
	return wcs_pushW(dst,dst_count,to_wchar(src));
}
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src)
{
	return wcs_pushA(dst,dst_count,src,strlen(src));
}




/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 返還後の文字列を入れるバッファ
	@param cesc  [in] エスケープしないといけない文字
	@param cwith [in] エスケープに使う文字
	
	@retval 出力したバイト数 (Unicodeの場合は文字数)

	文字列中にそのまま使うとまずい文字がある場合にその文字の前に
	エスケープキャラクタを挿入するために使う．

	@note 変換後のデータは最大で元の文字列の2倍になる
	@note この関数は2バイト文字の考慮を行っていない

	@author genta
	@date 2002/01/04 新規作成
	@date 2002/01/30 genta &専用(dupamp)から一般の文字を扱えるように拡張．
		dupampはinline関数にした．
	@date 2002/02/01 genta bugfix エスケープする文字とされる文字の出力順序が逆だった
	@date 2004/06/19 genta Generic mapping対応
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}




/*!	文字列が指定された文字で終わっていなかった場合には
	末尾にその文字を付加する．

	@param pszPath [i/o]操作する文字列
	@param nMaxLen [in]バッファ長
	@param c [in]追加したい文字
	@retval  0 \が元から付いていた
	@retval  1 \を付加した
	@retval -1 バッファが足りず、\を付加できなかった
	@date 2003.06.24 Moca 新規作成
*/
int AddLastChar( TCHAR* pszPath, int nMaxLen, TCHAR c ){
	int pos = _tcslen( pszPath );
	// 何もないときは\を付加
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = _T('\0');
		return 1;
	}
	// 最後が\でないときも\を付加(日本語を考慮)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = _T('\0');
		return 1;
	}
	return 0;
}



/* CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEol*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* 改行コードがあった */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* 行終端子の種類を調べる */
			pcEol->SetTypeByString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

/*!
	GetNextLineのwchar_t版
	GetNextLineより作成
	static メンバ関数
*/
const wchar_t* GetNextLineW(
	const wchar_t*	pData,		//!< [in]	検索文字列
	int				nDataLen,	//!< [in]	検索文字列の文字数
	int*			pnLineLen,	//!< [out]	1行の文字数を返すただしEOLは含まない
	int*			pnBgn,		//!< [i/o]	検索文字列のオフセット位置
	CEol*			pcEol,		//!< [out]	EOL
	bool			bExtEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// 改行コードがあった
		if( WCODE::IsLineDelimiter(pData[i], bExtEol) ){
			// 行終端子の種類を調べる
			pcEol->SetTypeByString(&pData[i], nDataLen - i);
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}
#if 0 // 未使用
/*
	行端子の種類を調べるUnicodeBE版
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ(wchar_tの長さ)
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
static EEolType GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen )
{
	/*! 行終端子のデータの配列(UnicodeBE版) 2000.05.30 Moca */
	static const wchar_t* aEolTable[EOL_TYPE_NUM] = {
		L"",									// EOL_NONE
		(const wchar_t*)"\x00\x0d\x00\x0a\x00",	// EOL_CRLF
		(const wchar_t*)"\x00\x0a\x00",			// EOL_LF
		(const wchar_t*)"\x00\x0d\x00"			// EOL_CR
	};

	/* 改行コードの長さを調べる */

	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		CEol cEol((EEolType)i);
		if( cEol.GetLen()<=nDataLen && 0==auto_memcmp(pszData,aEolTable[i],cEol.GetLen()) ){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*!
	GetNextLineのwchar_t版(ビックエンディアン用)
	GetNextLineより作成
	static メンバ関数
*/
const wchar_t* GetNextLineWB(
	const wchar_t*	pData,	//!< [in]	検索文字列
	int			nDataLen,	//!< [in]	検索文字列の文字数
	int*		pnLineLen,	//!< [out]	1行の文字数を返すただしEOLは含まない
	int*		pnBgn,		//!< [i/o]	検索文字列のオフセット位置
	CEol*		pcEol		//!< [i/o]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// 改行コードがあった
		if( pData[i] == (wchar_t)0x0a00 || pData[i] == (wchar_t)0x0d00 ){
			// 行終端子の種類を調べる
			pcEol->SetType( GetEOLTypeUniBE( &pData[i], nDataLen - i ) );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}
#endif

/*! 指定長以下のテキストに切り分ける

	@param pText     [in] 切り分け対象となる文字列へのポインタ
	@param nTextLen  [in] 切り分け対象となる文字列全体の長さ
	@param nLimitLen [in] 切り分ける長さ
	@param pnLineLen [out] 実際に取り出された文字列の長さ
	@param pnBgn     [i/o] 入力: 切り分け開始位置, 出力: 取り出された文字列の次の位置

	@note 2003.05.25 未使用のようだ
*/
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeA::GetSizeOfChar( pText, nTextLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}





//! データを指定「文字数」以内に切り詰める。戻り値は結果の文字数。
int LimitStringLengthW(
	const wchar_t*	pszData,		//!< [in]
	int				nDataLength,	//!< [in]
	int				nLimitLength,	//!< [in]
	CNativeW&		cmemDes			//!< [out]
)
{
	int n=nDataLength;
	if(n>nLimitLength){
		int i = 0;
		int charSize = CNativeW::GetSizeOfChar(pszData, nDataLength, i);
		for(; i + charSize <= nLimitLength;){
			i += charSize;
			charSize = CNativeW::GetSizeOfChar(pszData, nDataLength, i);
		}
		n = i;
	}
	cmemDes.SetString(pszData,n);
	return n;
}

//! データを指定「文字数」以内に切り詰める。戻り値は結果の文字数。
int LimitStringLengthA(
	const char*		pszData,		//!< [in]
	int				nDataLength,	//!< [in]
	int				nLimitLength,	//!< [in]
	CNativeA&		cmemDes			//!< [out]
)
{
	int n=nDataLength;
	if(n>nLimitLength){
		int i = 0;
		int charSize = CNativeA::GetSizeOfChar(pszData, nDataLength, i);
		for(; i + charSize <= nLimitLength;){
			i += charSize;
			charSize = CNativeA::GetSizeOfChar(pszData, nDataLength, i);
		}
		n = i;
	}
	cmemDes.SetString(pszData,n);
	return n;
}



void GetLineColumn( const wchar_t* pLine, int* pnJumpToLine, int* pnJumpToColumn )
{
	int		i;
	int		j;
	int		nLineLen;
	wchar_t	szNumber[32];
	nLineLen = wcslen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= L'0' &&
			pLine[i] <= L'9' ){
			break;
		}
	}
	wmemset( szNumber, 0, _countof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* 行位置 改行単位行番号(1起点)の抽出 */
		j = 0;
		for( ; i < nLineLen && j + 1 < _countof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= L'0' &&
				pLine[i] <= L'9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = _wtoi( szNumber );

		/* 桁位置 改行単位行先頭からのバイト数(1起点)の抽出 */
		if( i < nLineLen && pLine[i] == ',' ){
			wmemset( szNumber, 0, _countof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < _countof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= L'0' &&
					pLine[i] <= L'9' ){
					continue;
				}
				break;
			}
			*pnJumpToColumn = _wtoi( szNumber );
		}
	}
	return;
}









/*
	scanf的安全スキャン

	使用例:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//結果: a[0]=1, a[1]=23, a[2]=4 となる。
*/
int scan_ints(
	const wchar_t*	pszData,	//!< [in]
	const wchar_t*	pszFormat,	//!< [in]
	int*			anBuf		//!< [out]
)
{
	//要素数
	int num = 0;
	const wchar_t* p = pszFormat;
	while(*p){
		if(*p==L'%')num++;
		p++;
	}

	//スキャン
	int dummy[32];
	memset(dummy,0,sizeof(dummy));
	int nRet = swscanf(
		pszData,pszFormat,
		&dummy[ 0],&dummy[ 1],&dummy[ 2],&dummy[ 3],&dummy[ 4],&dummy[ 5],&dummy[ 6],&dummy[ 7],&dummy[ 8],&dummy[ 9],
		&dummy[10],&dummy[11],&dummy[12],&dummy[13],&dummy[14],&dummy[15],&dummy[16],&dummy[17],&dummy[18],&dummy[19],
		&dummy[20],&dummy[21],&dummy[22],&dummy[23],&dummy[24],&dummy[25],&dummy[26],&dummy[27],&dummy[28],&dummy[29]
	);

	//結果コピー
	for(int i=0;i<num;i++){
		anBuf[i]=dummy[i];
	}

	return nRet;
}

