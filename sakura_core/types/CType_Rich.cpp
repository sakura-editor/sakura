/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
	_tcscpy( pType->m_szTypeName, _T("リッチテキスト") );
	_tcscpy( pType->m_szTypeExts, _T("rtf") );

	//設定
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0]  = 15;							/* キーワードセット */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;		/* 半角数値を色分け表示 */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_URL].m_bDisp = false;		//URLにアンダーラインを引かない
}



//Jul. 10, 2001 JEPRO 追加
const wchar_t* g_ppszKeywordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i",
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};
int g_nKeywordsRTF = _countof(g_ppszKeywordsRTF);
