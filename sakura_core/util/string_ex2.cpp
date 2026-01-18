/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "string_ex2.h"
#include "charset/charcode.h"
#include "basis/CEol.h"
#include "mem/CNativeW.h"

wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	wmemcpy( dst, src, src_count );
	return dst + src_count;
}
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src)
{
	return wcs_pushW(dst,dst_count,src,wcslen(src));
}

/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 変換後の文字列を入れるバッファ
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
int cescape(const WCHAR* org, WCHAR* buf, WCHAR cesc, WCHAR cwith)
{
	WCHAR *out = buf;
	for( ; *org != L'\0'; ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = L'\0';
	return int(out - buf);
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
int AddLastChar( WCHAR* pszPath, int nMaxLen, WCHAR c ){
	auto pos = int(wcslen(pszPath));
	// 何もないときは\を付加
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = L'\0';
		return 1;
	}
	// 最後が\でないときも\を付加(日本語を考慮)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = L'\0';
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
	pcEol->SetType( EEolType::none );
	if( *pnBgn >= nDataLen ){
		return nullptr;
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

	pcEol->SetType( EEolType::none );
	if( *pnBgn >= nDataLen ){
		return nullptr;
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

//! データを指定「文字数」以内に切り詰める。戻り値は結果の文字数。
size_t LimitStringLengthW(
	LPCWSTR			pszData,		//!< [in]
	size_t			nDataLength,	//!< [in]
	size_t			nLimitLength,	//!< [in]
	CNativeW&		cmemDes			//!< [out]
)
{
	size_t n = nDataLength;
	if(n>nLimitLength){
		size_t i = 0;
		size_t charSize = CNativeW::GetSizeOfChar(pszData, nDataLength, i);
		for(; i + charSize <= nLimitLength;){
			i += charSize;
			charSize = CNativeW::GetSizeOfChar(pszData, nDataLength, i);
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
	nLineLen = (int)wcslen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= L'0' &&
			pLine[i] <= L'9' ){
			break;
		}
	}
	wmemset( szNumber, 0, int(std::size(szNumber)) );
	if( i >= nLineLen ){
	}else{
		/* 行位置 改行単位行番号(1起点)の抽出 */
		j = 0;
		for( ; i < nLineLen && j + 1 < int(std::size(szNumber)); ){
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
			wmemset( szNumber, 0, int(std::size(szNumber)) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < int(std::size(szNumber)); ){
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
