/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCTYPESETTING_87013082_2E52_4683_8CEE_499218F2D584_H_
#define SAKURA_CDOCTYPESETTING_87013082_2E52_4683_8CEE_499218F2D584_H_
#pragma once

#include "basis/CMyString.h"	//SFilePath

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! フォント属性
struct SFontAttr{
	bool		m_bBoldFont;		//!< 太字
	bool		m_bUnderLine;		//!< 下線
};

//! 色属性
struct SColorAttr{
	COLORREF	m_cTEXT;			//!< 文字色
	COLORREF	m_cBACK;			//!< 背景色
};

//! 色設定
struct ColorInfoBase{
	bool		m_bDisp;			//!< 表示
	SFontAttr	m_sFontAttr;		//!< フォント属性
	SColorAttr	m_sColorAttr;		//!< 色属性
};

//! 名前とインデックス付き色設定
struct ColorInfo : public ColorInfoBase{
	int			m_nColorIdx;		//!< インデックス
	WCHAR		m_szName[64];		//!< 名前
};

//デフォルト色設定
void GetDefaultColorInfo( ColorInfo* pColorInfo, int nIndex );
void GetDefaultColorInfoName( ColorInfo* pColorInfo, int nIndex );
int GetDefaultColorInfoCount();

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           辞書                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< 辞書の説明の最大長 -1 */
struct KeyHelpInfo {
	bool		m_bUse;						//!< 辞書を 使用する/しない
	WCHAR		m_szAbout[DICT_ABOUT_LEN];	//!< 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	m_szPath;					//!< ファイルパス
};
//@@@ 2006.04.10 fon ADD-end
#endif /* SAKURA_CDOCTYPESETTING_87013082_2E52_4683_8CEE_499218F2D584_H_ */
