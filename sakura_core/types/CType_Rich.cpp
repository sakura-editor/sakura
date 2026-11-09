/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

/* リッチテキスト */
//JUl. 10, 2001 JEPRO WinHelp作るのにいるケンね
//Jul. 10, 2001 JEPRO 追加
void CType_Rich::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("リッチテキスト") );
	wcscpy( pType->m_szTypeExts, LTEXT("rtf") );

	//設定
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 15;							/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_URL].m_bDisp = false;		//URLにアンダーラインを引かない
}

//Jul. 10, 2001 JEPRO 追加
const WCHAR* g_ppszKeywordsRTF[] = {
	LTEXT("\\ansi"),
	LTEXT("\\b"),
	LTEXT("\\bin"),
	LTEXT("\\box"),
	LTEXT("\\brdrb"),
	LTEXT("\\brdrbar"),
	LTEXT("\\brdrdb"),
	LTEXT("\\brdrdot"),
	LTEXT("\\brdrl"),
	LTEXT("\\brdrr"),
	LTEXT("\\brdrs"),
	LTEXT("\\brdrsh"),
	LTEXT("\\brdrt"),
	LTEXT("\\brdrth"),
	LTEXT("\\cell"),
	LTEXT("\\cellx"),
	LTEXT("\\cf"),
	LTEXT("\\chftn"),
	LTEXT("\\clmgf"),
	LTEXT("\\clmrg"),
	LTEXT("\\colortbl"),
	LTEXT("\\deff"),
	LTEXT("\\f"),
	LTEXT("\\fi"),
	LTEXT("\\field"),
	LTEXT("\\fldrslt"),
	LTEXT("\\fonttbl"),
	LTEXT("\\footnote"),
	LTEXT("\\fs"),
	LTEXT("\\i"),
	LTEXT("\\intbl"),
	LTEXT("\\keep"),
	LTEXT("\\keepn"),
	LTEXT("\\li"),
	LTEXT("\\line"),
	LTEXT("\\mac"),
	LTEXT("\\page"),
	LTEXT("\\par"),
	LTEXT("\\pard"),
	LTEXT("\\pc"),
	LTEXT("\\pich"),
	LTEXT("\\pichgoal"),
	LTEXT("\\picscalex"),
	LTEXT("\\picscaley"),
	LTEXT("\\pict"),
	LTEXT("\\picw"),
	LTEXT("\\picwgoal"),
	LTEXT("\\plain"),
	LTEXT("\\qc"),
	LTEXT("\\ql"),
	LTEXT("\\qr"),
	LTEXT("\\ri"),
	LTEXT("\\row"),
	LTEXT("\\rtf"),
	LTEXT("\\sa"),
	LTEXT("\\sb"),
	LTEXT("\\scaps"),
	LTEXT("\\sect"),
	LTEXT("\\sl"),
	LTEXT("\\strike"),
	LTEXT("\\tab"),
	LTEXT("\\tqc"),
	LTEXT("\\tqr"),
	LTEXT("\\trgaph"),
	LTEXT("\\trleft"),
	LTEXT("\\trowd"),
	LTEXT("\\trqc"),
	LTEXT("\\trql"),
	LTEXT("\\tx"),
	LTEXT("\\ul"),
	LTEXT("\\uldb"),
	LTEXT("\\v"),
	LTEXT("\\wbitmap"),
	LTEXT("\\wbmbitspixel"),
	LTEXT("\\wbmplanes"),
	LTEXT("\\wbmwidthbytes"),
	LTEXT("\\wmetafile"),
	LTEXT("bmc"),
	LTEXT("bml"),
	LTEXT("bmr"),
	LTEXT("emc"),
	LTEXT("eml"),
	LTEXT("emr")
};
int g_nKeywordsRTF = int(std::size(g_ppszKeywordsRTF));
