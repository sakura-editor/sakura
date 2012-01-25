/*
	2008.05.18 kobake CShareData から分離
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

	if( NULL != pszFilePath && 0 < (int)_tcslen( pszFilePath ) ){
		_tsplitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == _T('.') )
			return GetDocumentTypeOfExt( szExt + 1 );
		else
			return GetDocumentTypeOfExt( szExt );
	}
	return CTypeConfig(0);
}


/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の,は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
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
	return CTypeConfig(0);	//	ハズレ
}
