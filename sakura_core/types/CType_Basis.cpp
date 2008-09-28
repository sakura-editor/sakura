#include "stdafx.h"
#include "types/CType.h"
#include "view/colors/CColorStrategy.h"

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("基本") );
	_tcscpy( pType->m_szTypeExts, _T("") );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);			// 折り返し桁数
	pType->m_eDefaultOutline = OUTLINE_TEXT;					// アウトライン解析方法
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// シングルクォーテーション文字列を色分け表示しない	//Oct. 17, 2000 JEPRO
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない	//Sept. 4, 2000 JEPRO
}
