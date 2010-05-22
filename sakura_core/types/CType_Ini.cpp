#include "stdafx.h"
#include "types/CType.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"

/* 設定ファイル */
//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
void CType_Ini::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("設定ファイル") );
	_tcscpy( pType->m_szTypeExts, _T("ini,inf,cnf,kwd,col") );
	
	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );				/* 行コメントデリミタ */
	pType->m_cLineComment.CopyTo( 1, L";", -1 );				/* 行コメントデリミタ2 */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
}
