#include "stdafx.h"
#include "types/CType.h"

/* リッチテキスト */
//JUl. 10, 2001 JEPRO WinHelp作るのにいるケンね
//Jul. 10, 2001 JEPRO 追加
void CType_Rich::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("リッチテキスト") );
	_tcscpy( pType->m_szTypeExts, _T("rtf") );

	//設定
	pType->m_nDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 15;							/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;		/* 半角数値を色分け表示 */
	pType->m_nStringType = 0;									/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;	//ダブルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;		//URLにアンダーラインを引かない
}
