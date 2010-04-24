/*!	@file
	@brief CDicMgrクラス

	@author Norio Nakatani
	@date	1998/11/05 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, Moca
	Copyright (C) 2003, Moca
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include "CDicMgr.h"
#include "mem/CMemory.h" // 2002/2/10 aroka ヘッダ整理
#include "debug/CRunningTimer.h"
#include "io/CTextStream.h"
using namespace std;

CDicMgr::CDicMgr()
{
	return;
}




CDicMgr::~CDicMgr()
{
	return;
}




/*!
	キーワードの検索
	最初に見つかったキーワードの意味を返す

	@date 2006.04.10 fon 検索ヒット行を返す引数pLineを追加
*/
BOOL CDicMgr::Search(
	const wchar_t*		pszKey,				//!< 検索キーワード
	const int			nCmpLen,			//!< 検索キーワードの長さ
	CNativeW**			ppcmemKey,			//!< 見つかったキーワード．呼び出し元の責任で解放する．
	CNativeW**			ppcmemMean,			//!< 見つかったキーワードに対応する辞書内容．呼び出し元の責任で解放する．
	const TCHAR*		pszKeyWordHelpFile,	//!< キーワードヘルプファイルのパス名
	int*				pLine				//!< 見つかったキーワードのキーワードヘルプファイル内での行番号
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDicMgr::Search" );
#endif
	long	i;
	wchar_t*	pszDelimit = L" /// ";
	wchar_t*	pszWork;
	int		nRes;
	wchar_t*	pszToken;
	wchar_t*	pszKeySeps = L",\0";


	/* 辞書ファイル */
	if( 0 >= _tcslen( pszKeyWordHelpFile ) ){
		return FALSE;
	}
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	CTextInputStream_AbsIni in(pszKeyWordHelpFile);
	if(!in){
		return FALSE;
	}

	wchar_t	szLine[LINEREADBUFSIZE];
	for(int line=1 ; in; line++ ){	// 2006.04.10 fon
		//1行読み込み
		{
			wstring tmp = in.ReadLineW(); //NULL != fgetws( szLine, _countof(szLine), pFile );
			wcsncpy_s(szLine,_countof(szLine),tmp.c_str(), _TRUNCATE);
			// auto_strlcpy(szLine,tmp.c_str(), _countof(szLine));
		}

		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += wcslen( pszDelimit );

			/* 最初のトークンを取得します。 */
			pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
				nRes = _wcsnicmp( pszKey, pszToken, nCmpLen );	// 2006.04.10 fon
				if( 0 == nRes ){
					for( i = 0; i < (int)wcslen(pszWork); ++i ){
						if( pszWork[i] == L'\r' ||
							pszWork[i] == L'\n' ){
							pszWork[i] = L'\0';
							break;
						}
					}
					//キーワードのセット
					*ppcmemKey = new CNativeW;	// 2006.04.10 fon
					(*ppcmemKey)->SetString( pszToken );
					//意味のセット
					*ppcmemMean = new CNativeW;
					(*ppcmemMean)->SetString( pszWork );

					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = wcstok( NULL, pszKeySeps );
			}
		}
	}
	return FALSE;
}





/*
||  入力補完キーワードの検索
||
||  ・見つかった候補をすべて返す(改行で区切って返す) -> ppcmemKouho
||  ・指定された候補の最大数を超えると処理を中断する
||  ・見つかった数を返す
||
*/
int CDicMgr::HokanSearch(
	const wchar_t*	pszKey,
	BOOL			bHokanLoHiCase,	//!< 英大文字小文字を同一視する
	CNativeW**		ppcmemKouho,	//!< [out] 候補リスト。改行(\n)区切り。
	int				nMaxKouho,		//!< Max候補数(0==無制限)
	const TCHAR*	pszKeyWordFile
)
{
	int		nKeyLen;
	int		nKouhoNum;
	int		nRet;
	*ppcmemKouho = NULL;
	if( 0 >= _tcslen( pszKeyWordFile ) ){
		return 0;
	}

	CTextInputStream_AbsIni in(pszKeyWordFile);
	if(!in){
		return 0;
	}
	nKouhoNum = 0;
	nKeyLen = wcslen( pszKey );
	wstring szLine;
	while( in ){
		szLine = in.ReadLineW(); // NULL != fgetws( szLine, _countof(szLine), pFile ) ){
		if( nKeyLen > (int)szLine.length() ){
			continue;
		}

		//コメント無視
		if( szLine[0] == L';' )continue;

		//空行無視
		if( szLine[0] == L'\0' )continue;

		if( bHokanLoHiCase ){	/* 英大文字小文字を同一視する */
			nRet = auto_memicmp( pszKey, szLine.c_str(), nKeyLen );
		}else{
			nRet = auto_memcmp( pszKey, szLine.c_str(), nKeyLen );
		}
		if( 0 == nRet ){
			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CNativeW;
			}
			(*ppcmemKouho)->AppendString( szLine.c_str() );
			(*ppcmemKouho)->AppendString( L"\n" );
			++nKouhoNum;
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				break;
			}
		}
	}
	in.Close();
	return nKouhoNum;
}



