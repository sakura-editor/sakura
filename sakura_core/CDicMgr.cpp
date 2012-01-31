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
	
	@param[in] pszKey 検索キーワード
	@param[in] nCmpLen 検索キーワードの長さ
	@param[out] ppcmemKey 見つかったキーワード．呼び出し元の責任で解放する．
	@param[out] ppcmemMean 見つかったキーワードに対応する辞書内容．呼び出し元の責任で解放する．
	@param[in] pszKeyWordHelpFile キーワードヘルプファイルのパス名
	@param[out] pLine 見つかったキーワードのキーワードヘルプファイル内での行番号

	@date 2006.04.10 fon 検索ヒット行を返す引数pLineを追加
*/
BOOL CDicMgr::Search( const char* pszKey, const int nCmpLen, CMemory** ppcmemKey, CMemory** ppcmemMean, const char* pszKeyWordHelpFile, int *pLine )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDicMgr::Search" );
#endif
//	char	szDir[_MAX_PATH];
//	long	lPathLen;
	long	i;
	FILE*	pFile;
	char	szLine[LINEREADBUFSIZE];
	char*	pszDelimit = " /// ";
	char*	pszWork;
	int		nRes;
	char*	pszToken;
	char*	pszKeySeps = ",\0";


	/* 辞書ファイル */
	if( 0 >= lstrlen( pszKeyWordHelpFile ) ){
		return FALSE;
	}
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	pFile = _tfopen_absini( pszKeyWordHelpFile, "r" );
	if( NULL == pFile ){
		return FALSE;
	}
	for(int line=1 ;NULL != fgets( szLine, sizeof(szLine), pFile ); line++ ){	// 2006.04.10 fon
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += lstrlen( pszDelimit );

			/* 最初のトークンを取得します。 */
			pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
				nRes = _strnicmp( pszKey, pszToken, nCmpLen );	// 2006.04.10 fon
				if( 0 == nRes ){
					for( i = 0; i < (int)lstrlen(pszWork); ++i ){
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
//					(*ppcmemMean)->SetData( pszWork, lstrlen(pszWork) );
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
			const char* pszKey,
			BOOL		bHokanLoHiCase,	/*英大文字小文字を同一視する*/
			CMemory**	ppcmemKouho,
			int			nMaxKouho,	//Max候補数(0==無制限)
			const char* pszKeyWordFile
)
{
	FILE*	pFile;
	char	szLine[1024];
	int		nKeyLen;
	int		nKouhoNum;
	int		nRet;
	*ppcmemKouho = NULL;
	if( 0 >= lstrlen( pszKeyWordFile ) ){
		return 0;
	}
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	pFile = _tfopen_absini( pszKeyWordFile, "r" );
	if( NULL == pFile ){
		return 0;
	}
	nKouhoNum = 0;
	nKeyLen = lstrlen( pszKey );
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) ){
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
			nRet = memicmp( pszKey, szLine, nKeyLen );
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
