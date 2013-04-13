/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CDocTypeManager.h"


/*!
	ファイル名から、ドキュメントタイプ（数値）を取得する
	
	@param pszFilePath [in] ファイル名
	
	拡張子を切り出して GetDocumentTypeOfExt に渡すだけ．
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfPath( const TCHAR* pszFilePath )
{
	TCHAR	szExt[_MAX_EXT];
	TCHAR	szName[_MAX_FNAME];
	TCHAR*	pszExt = szExt;

	if( NULL != pszFilePath && pszFilePath[0] ){
		_tsplitpath( pszFilePath, NULL, NULL, szName, szExt );
		// 2重拡張子探索
		TCHAR* pFileExt = _tcschr( szName, '.' );
		if( pFileExt ){
			pFileExt++;
			auto_strcat( pFileExt, pszExt );
		}else{
			if( 0 == pszExt[0] ){
				// 拡張子がファイルにない
				pFileExt = szName;
			}else{
				pFileExt = pszExt + 1;
			}
		}
		return GetDocumentTypeOfExt( pFileExt );
	}
	return CTypeConfig(0);
}


/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の.は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
	@date 2012.10.22 Moca ２重拡張子, 拡張子なしに対応
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfExt( const TCHAR* pszExt )
{
	const TCHAR	pszSeps[] = _T(" ;,");	// separator

	int		i;
	TCHAR*	pszToken;
	TCHAR	szText[MAX_TYPES_EXTS];

	for( i = 0; i < MAX_TYPES; ++i ){
		CTypeConfig nType(i);
		_tcscpy( szText, CDocTypeManager().GetTypeSetting(nType).m_szTypeExts );
		pszToken = _tcstok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == _tcsicmp( pszExt, pszToken ) ){
				return CTypeConfig(i);	//	番号
			}
			pszToken = _tcstok( NULL, pszSeps );
		}
	}
	const TCHAR* pFileExt = _tcschr( pszExt, _T('.') );
	if( pFileExt && pFileExt[1] ){
		return GetDocumentTypeOfExt( pFileExt + 1 );
	}
	return CTypeConfig(0);	//	ハズレ
}
