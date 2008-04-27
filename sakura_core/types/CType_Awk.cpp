#include "stdafx.h"
#include "types/CType.h"

/* awk */
void CType_Awk::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("AWK") );
	_tcscpy( pType->m_szTypeExts, _T("awk") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );		/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 6;						/* キーワードセット */
}
