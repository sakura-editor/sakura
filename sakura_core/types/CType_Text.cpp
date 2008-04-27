#include "stdafx.h"
#include "types/CType.h"

/* テキスト */
//Sep. 20, 2000 JEPRO テキストの規定値を80→120に変更(不具合一覧.txtがある程度読みやすい桁数)
//Nov. 15, 2000 JEPRO PostScriptファイルも読めるようにする
//Jan. 12, 2001 JEPRO readme.1st も読めるようにする
//Feb. 12, 2001 JEPRO .err エラーメッセージ
//Nov.  6, 2002 genta docはMS Wordに譲ってここからは外す（関連づけ防止のため）
//Nov.  6, 2002 genta log を追加
void CType_Text::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("テキスト") );
	_tcscpy( pType->m_szTypeExts, _T("txt,log,1st,err,ps") );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(120);					/* 折り返し桁数 */
	pType->m_nDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
	pType->m_bKinsokuHead = FALSE;								/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = FALSE;								/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = FALSE;								/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = FALSE;								/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"!%),.:;?]}￠°’”‰′″℃、。々〉》」』】〕゛゜ゝゞ・ヽヾ！％），．：；？］｝｡｣､･ﾞﾟ￠" );		/* 行頭禁則 */	//@@@ 2002.04.13 MIK 
	wcscpy( pType->m_szKinsokuTail, L"$([{￡\\‘“〈《「『【〔＄（［｛｢￡￥" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK 
}
