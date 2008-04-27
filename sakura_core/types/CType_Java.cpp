#include "stdafx.h"
#include "types/CType.h"

/* Java */
void CType_Java::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Java") );
	_tcscpy( pType->m_szTypeExts, _T("java,jav") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* 行コメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );	/* ブロックコメントデリミタ */
	pType->m_nKeyWordSetIdx[0] = 4;									/* キーワードセット */
	pType->m_nDefaultOutline = OUTLINE_JAVA;						/* アウトライン解析方法 */
	pType->m_nSmartIndent = SMARTINDENT_CPP;						/* スマートインデント種別 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			//半角数値を色分け表示	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトONに	//Sep. 21, 2002 genta
}
