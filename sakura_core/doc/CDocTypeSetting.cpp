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

// 2000.10.08 JEPRO  背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
// 2000.12.09 Jepro  note: color setting (詳細は CshareData.h を参照のこと)
// 2000.09.04 JEPRO  シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
// 2000.10.17 JEPRO  色分け表示するように変更(最初のFALSE→TRUE)
// 2008.03.27 kobake 大整理

#include "StdAfx.h"
#include "CDocTypeSetting.h"


//! 色設定(保存用)
struct ColorInfoIni {
	int				m_nNameId;			//!< 項目名
	ColorInfoBase	m_sColorInfo;		//!< 色設定
};

static ColorInfoIni ColorInfo_DEFAULT[] = {
//	項目名,									表示,		太字,		下線,		文字色,					背景色,
	{ STR_COLOR_TEXT,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_RULER,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_CURSOR,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ) } } },	// 2006.12.07 ryoji
	{ STR_COLOR_CURSOR_IMEON,				{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ) } } },	// 2006.12.07 ryoji
	{ STR_COLOR_CURSOR_LINE_BG,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 255, 255, 128 ) } } },	// 2012.11.21 Moca
	{ STR_COLOR_CURSOR_LINE,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CURSOR_COLUMN,				{ FALSE,	{ FALSE,	FALSE },	{ RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ) } } },	// 2007.09.09 Moca
	{ STR_COLOR_NOTE_LINE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_LINE_NO,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_LINE_NO_CHANGE,				{ TRUE,		{ TRUE,		FALSE },	{ RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ) } } },
	{ STR_COLOR_EVEN_LINE_BG,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0,   0 ),	RGB( 243, 243, 243 ) } } },	// 2013.12.30 Moca
	{ STR_COLOR_TAB,						{ TRUE,		{ FALSE,	FALSE },	{ RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ) } } },	//Jan. 19, 2001 JEPRO RGB(192,192,192)より濃いグレーに変更
	{ STR_COLOR_HALF_SPACE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } }, //2002.04.28 Add by KK
	{ STR_COLOR_FULL_SPACE,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CTRL_CODE,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_CR,							{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_WRAP_MARK,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_VERT_LINE,					{ FALSE,	{ FALSE,	FALSE },	{ RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ) } } }, //2005.11.08 Moca
	{ STR_COLOR_EOF,						{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ) } } },
	{ STR_COLOR_NUMBER,						{ FALSE,	{ FALSE,	FALSE },	{ RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)を変更  Mar.10, 2001 標準は色なしに
	{ STR_COLOR_BRACKET,					{ FALSE,	{ TRUE,		FALSE },	{ RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ) } } },	// 02/09/18 ai
	{ STR_COLOR_SELECTED_AREA,				{ TRUE,		{ FALSE,	FALSE },	{ RGB(  49, 106, 197 ),	RGB(  49, 106, 197 ) } } },	//2011.05.18
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
	{ STR_COLOR_KEYWORD2,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
	{ STR_COLOR_KEYWORD3,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
	{ STR_COLOR_KEYWORD4,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD5,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD6,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD7,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD8,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD9,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_KEYWORD10,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ) } } },
	{ STR_COLOR_REGEX_KEYWORD1,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD2,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD3,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD4,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD5,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD6,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD7,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD8,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD9,				{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_REGEX_KEYWORD10,			{ FALSE,	{ FALSE,	FALSE },	{ RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ) } } },	//@@@ 2001.11.17 add MIK
	{ STR_COLOR_DIFF_ADD,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0,   0, 210 ),	RGB( 162, 208, 255 ) } } },	//@@@ 2002.06.01 MIK
	{ STR_COLOR_DIFF_CNG,					{ TRUE,		{ FALSE,	FALSE },	{ RGB(   0, 111,   0 ),	RGB( 189, 253, 192 ) } } },	//@@@ 2002.06.01 MIK
	{ STR_COLOR_DIFF_DEL,					{ TRUE,		{ FALSE,	FALSE },	{ RGB( 213, 106,   0 ),	RGB( 255, 233, 172 ) } } },	//@@@ 2002.06.01 MIK
	{ STR_COLOR_BOOKMARK,					{ TRUE ,	{ FALSE,	FALSE },	{ RGB( 255, 251, 240 ),	RGB(   0, 128, 192 ) } } },	// 02/10/16 ai
	{ STR_COLOR_PAGEVIEW,					{ TRUE ,	{ FALSE,	FALSE },	{ RGB( 255, 251, 240 ),	RGB( 190, 230, 255 ) } } },

};

void GetDefaultColorInfo( ColorInfo* pColorInfo, int nIndex )
{
	assert( nIndex < _countof(ColorInfo_DEFAULT) );

	ColorInfoBase* p = pColorInfo;
	*p = ColorInfo_DEFAULT[nIndex].m_sColorInfo; //ColorInfoBase
	GetDefaultColorInfoName( pColorInfo, nIndex );
	pColorInfo->m_nColorIdx = nIndex;
}

void GetDefaultColorInfoName( ColorInfo* pColorInfo, int nIndex )
{
	assert( nIndex < _countof(ColorInfo_DEFAULT) );

	_tcscpy(pColorInfo->m_szName, LS( ColorInfo_DEFAULT[nIndex].m_nNameId ) );
}

int GetDefaultColorInfoCount()
{
	return _countof(ColorInfo_DEFAULT);
}
