/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

// 2000.10.08 JEPRO  背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
// 2000.12.09 Jepro  note: color setting (詳細は CshareData.h を参照のこと)
// 2000.09.04 JEPRO  シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
// 2000.10.17 JEPRO  色分け表示するように変更(最初のFALSE→TRUE)
// 2008.03.27 kobake 大整理

#include "StdAfx.h"
#include "doc/CDocTypeSetting.h"

#include "CSelectLang.h"
#include "String_define.h"

//! 色設定(保存用)
struct ColorInfoIni {
	WORD			m_nNameId;			//!< 項目名
	ColorInfoBase	m_sColorInfo;		//!< 色設定
};

//キーワード：デフォルトカラー設定
constexpr std::array<ColorInfoIni, 57> defaultColorInfo = {{
//	項目名,									表示,		太字,		下線,		文字色,					背景色,
	{ STR_COLOR_TEXT,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_RULER,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_CURSOR,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CURSOR_IMEON,				{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CURSOR_LINE_BG,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 255, 128 ) } } },
	{ STR_COLOR_CURSOR_LINE,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CURSOR_COLUMN,				{ FALSE,	{ FALSE,	FALSE },	{ RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_NOTE_LINE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_LINE_NO,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_LINE_NO_CHANGE,				{ TRUE,		{ TRUE,		FALSE },	{ RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_EVEN_LINE_BG,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 243, 243, 243 ) } } },
	{ STR_COLOR_TAB,						{ TRUE,		{ FALSE,	FALSE },	{ RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_HALF_SPACE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_FULL_SPACE,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CTRL_CODE,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CR,							{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_WRAP_MARK,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_VERT_LINE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_EOF,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ) } } },
	{ STR_COLOR_NUMBER,						{ FALSE,	{ FALSE,	FALSE },	{ RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_BRACKET,					{ FALSE,	{ TRUE,		FALSE },	{ RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_SELECTED_AREA,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(  49, 106, 197 ),	RGB(  49, 106, 197 ) } } },
	{ STR_COLOR_SEARCH_WORD1,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ) } } },
	{ STR_COLOR_SEARCH_WORD2,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ) } } },
	{ STR_COLOR_SEARCH_WORD3,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ) } } },
	{ STR_COLOR_SEARCH_WORD4,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ) } } },
	{ STR_COLOR_SEARCH_WORD5,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ) } } },
	{ STR_COLOR_COMMENT,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_SINGLE_QUOTE,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_DOUBLE_QUOTE,				{ TRUE,		{ FALSE,	FALSE },	{ RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_HERE_DOCUMENT,				{ FALSE,	{ FALSE,	FALSE },	{ RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_URL,						{ TRUE,		{ FALSE,	TRUE  },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD1,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD2,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD3,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD4,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD5,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD6,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD7,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD8,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD9,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD10,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD1,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD2,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD3,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD4,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD5,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD6,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD7,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD8,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD9,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD10,			{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_DIFF_ADD,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 210 ),	RGB( 162, 208, 255 ) } } },
	{ STR_COLOR_DIFF_CNG,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 111,   0 ),	RGB( 189, 253, 192 ) } } },
	{ STR_COLOR_DIFF_DEL,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 213, 106,   0 ),	RGB( 255, 233, 172 ) } } },
	{ STR_COLOR_BOOKMARK,					{ TRUE ,	{ FALSE,	FALSE },	{ RGB( 255, 251, 240 ),	RGB(   0, 128, 192 ) } } },
	{ STR_COLOR_PAGEVIEW,					{ TRUE ,	{ FALSE,	FALSE },	{ RGB( 255, 251, 240 ),	RGB( 190, 230, 255 ) } } },
}};

void ColorInfo::SetDefault(size_t nIndex)
{
	m_nColorIdx = WORD(nIndex);

	const auto name = GetColorName(nIndex);
	wcscpy_s(m_szName, name.c_str());

	const auto& defaultData = defaultColorInfo[int(nIndex)].m_sColorInfo;
	m_bDisp = defaultData.m_bDisp;
	m_sFontAttr = defaultData.m_sFontAttr;
	m_sColorAttr = defaultData.m_sColorAttr;
}

void GetDefaultColorInfoName( ColorInfo* pColorInfo, int nIndex )
{
	const auto name = ColorInfo::GetColorName(nIndex);
	wcscpy_s(pColorInfo->m_szName, name.c_str());
}

/* static */ std::wstring ColorInfo::GetColorName(size_t nIndex)
{
	assert(nIndex < std::size(defaultColorInfo));
	return LS(defaultColorInfo[nIndex].m_nNameId);
}
