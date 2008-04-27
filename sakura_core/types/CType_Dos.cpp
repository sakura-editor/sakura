#include "stdafx.h"
#include "types/CType.h"

/* MS-DOSバッチファイル */
void CType_Dos::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("MS-DOSバッチファイル") );
	_tcscpy( pType->m_szTypeExts, _T("bat") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"REM ", -1 );	/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 7;					/* キーワードセット */
}
