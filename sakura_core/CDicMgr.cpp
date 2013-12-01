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

#include "StdAfx.h"
#include <stdio.h>
#include "CDicMgr.h"
#include "CMemory.h" // 2002/2/10 aroka ヘッダ整理
#include "CRunningTimer.h"
#include "etc_uty.h"
#include "file.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

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
	const char*			pszKey,				//!< 検索キーワード
	const int			nCmpLen,			//!< 検索キーワードの長さ
	CMemory**			ppcmemKey,			//!< 見つかったキーワード．呼び出し元の責任で解放する．
	CMemory**			ppcmemMean,			//!< 見つかったキーワードに対応する辞書内容．呼び出し元の責任で解放する．
	const TCHAR*		pszKeyWordHelpFile,	//!< キーワードヘルプファイルのパス名
	int*				pLine				//!< 見つかったキーワードのキーワードヘルプファイル内での行番号
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDicMgr::Search" );
#endif
	long	i;
	const char*	pszDelimit = " /// ";
	char*	pszWork;
	int		nRes;
	char*	pszToken;
	const char*	pszKeySeps = ",\0";


	/* 辞書ファイル */
	if( pszKeyWordHelpFile[0] == _T('\0') ){
		return FALSE;
	}
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	FILE* pFile = _tfopen_absini( pszKeyWordHelpFile, _T("r") );
	if(!pFile){
		return FALSE;
	}

	char	szLine[LINEREADBUFSIZE];
	for(int line=1 ;NULL != fgets( szLine, _countof(szLine), pFile ); line++ ){	// 2006.04.10 fon
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += _tcslen( pszDelimit );

			/* 最初のトークンを取得します。 */
			pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
				nRes = my_strnicmp( pszKey, pszToken, nCmpLen );	// 2006.04.10 fon
				if( 0 == nRes ){
					int nLen = (int)_tcslen(pszWork);
					for( i = 0; i < nLen; ++i ){
						if( pszWork[i] == '\r' ||
							pszWork[i] == '\n' ){
							pszWork[i] = '\0';
							break;
						}
					}
					//キーワードのセット
					*ppcmemKey = new CMemory;	// 2006.04.10 fon
					(*ppcmemKey)->SetString( pszToken );
					//意味のセット
					*ppcmemMean = new CMemory;
					(*ppcmemMean)->SetString( pszWork );

					fclose( pFile );
					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return FALSE;
}





/*
||  入力補完キーワードの検索
||
||  ・見つかった候補をすべて返す(改行で区切って返す)
||  ・指定された候補の最大数を超えると処理を中断する
||  ・見つかった数を返す
||
*/
int CDicMgr::HokanSearch(
	const char* 	pszKey,
	bool			bHokanLoHiCase,	//!< 英大文字小文字を同一視する
	CMemory**		ppcmemKouho,
	int				nMaxKouho,		//!< Max候補数(0==無制限)
	const TCHAR*	pszKeyWordFile
)
{
	int		nKeyLen;
	int		nKouhoNum;
	int		nRet;
	*ppcmemKouho = NULL;
	if( pszKeyWordFile[0] == _T('\0') ){
		return 0;
	}

	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	FILE* pFile = _tfopen_absini( pszKeyWordFile, _T("r") );
	if(!pFile){
		return 0;
	}
	nKouhoNum = 0;
	nKeyLen = _tcslen( pszKey );
	char	szLine[1024];
	while( NULL != fgets( szLine, _countof(szLine), pFile ) ){
		if( nKeyLen > (int)lstrlen( szLine ) ){
			continue;
		}
		if( szLine[0] == ';' ){
			continue;
		}
		if( szLine[nKeyLen] == '\r' || szLine[nKeyLen] == '\n' ){
			continue;
		}

		if( bHokanLoHiCase ){	/* 英大文字小文字を同一視する */
			nRet = my_memicmp( pszKey, szLine, nKeyLen );
		}else{
			nRet = memcmp( pszKey, szLine, nKeyLen );
		}
		if( 0 == nRet ){
			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CMemory;
				(*ppcmemKouho)->SetString( szLine );
			}else{
				(*ppcmemKouho)->AppendString( szLine );
			}
			++nKouhoNum;
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				break;
			}
		}
	}
	fclose( pFile );
	return nKouhoNum;
}


/*[EOF]*/
