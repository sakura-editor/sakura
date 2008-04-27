#include "stdafx.h"
#include "types/CType.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ユーザに贈る
//Jul. 08, 2001 JEPRO 追加
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Perl") );
	_tcscpy( pType->m_szTypeExts, _T("cgi,pl,pm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* 行コメントデリミタ */
	pType->m_nDefaultOutline = OUTLINE_PERL;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 12;								/* キーワードセット2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//対括弧の強調をデフォルトON	//Sep. 21, 2002 genta
}
