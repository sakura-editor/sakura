#include "stdafx.h"
#include "types/CType.h"

/* Visual Basic */
//JUl. 10, 2001 JEPRO VB   ユーザに贈る
//Jul. 09, 2001 JEPRO 追加 //Dec. 16, 2002 MIK追加 // Feb. 19, 2006 genta .vb追加
void CType_Vb::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Visual Basic") );
	_tcscpy( pType->m_szTypeExts, _T("bas,frm,cls,ctl,pag,dob,dsr,vb") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"'", -1 );				/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_VB;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 13;							/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 14;							/* キーワードセット2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* 半角数値を色分け表示 */
	pType->m_nStringType = 1;									/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
}
