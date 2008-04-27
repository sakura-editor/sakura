#include "stdafx.h"
#include "types/CType.h"

/* TeX */
//Oct. 31, 2000 JEPRO TeX  ユーザに贈る
//Oct. 31, 2000 JEPRO TeX ユーザに贈る	//Mar. 10, 2001 JEPRO 追加
void CType_Tex::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("TeX") );
	_tcscpy( pType->m_szTypeExts, _T("tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"%", -1 );				/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_TEX;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 9;								/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 10;							/* キーワードセット2 */	//Jan. 19, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
}
