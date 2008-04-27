#include "stdafx.h"
#include "types/CType.h"

/* PL/SQL */
void CType_Sql::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("PL/SQL") );
	_tcscpy( pType->m_szTypeExts, _T("sql,plsql") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"--", -1 );					/* 行コメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );	/* ブロックコメントデリミタ */
	pType->m_nStringType = 1;										/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	wcscpy( pType->m_szIndentChars, L"|★" );						/* その他のインデント対象文字 */
	pType->m_nKeyWordSetIdx[0] = 2;									/* キーワードセット */
	pType->m_nDefaultOutline = OUTLINE_PLSQL;						/* アウトライン解析方法 */
}
