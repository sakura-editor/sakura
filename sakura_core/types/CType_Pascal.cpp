#include "stdafx.h"
#include "types/CType.h"

/* Pascal */
//Mar. 10, 2001 JEPRO	半角数値を色分け表示
void CType_Pascal::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Pascal") );
	_tcscpy( pType->m_szTypeExts, _T("dpr,pas") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );					/* 行コメントデリミタ */		//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"{", L"}" );	/* ブロックコメントデリミタ */	//Nov. 5, 2000 JEPRO 追加
	pType->m_cBlockComment.SetBlockCommentRule( 1, L"(*", L"*)" );	/* ブロックコメントデリミタ2 */	//@@@ 2001.03.10 by MIK
	pType->m_nStringType = 1;										/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO 追加
	pType->m_nKeyWordSetIdx[0] = 8;									/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;			//@@@ 2001.11.11 upd MIK
}
