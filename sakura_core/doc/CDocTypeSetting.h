/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

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
	bool		m_bBoldFont = false;		//!< 太字
	bool		m_bUnderLine = false;		//!< 下線
};

//! 色属性
struct SColorAttr{
	COLORREF	m_cTEXT = 0;			//!< 文字色
	COLORREF	m_cBACK = 0;			//!< 背景色
};

//! 色設定
struct ColorInfoBase{
	bool		m_bDisp = false;			//!< 表示
	SFontAttr	m_sFontAttr;		//!< フォント属性
	SColorAttr	m_sColorAttr;		//!< 色属性
};

//! 名前とインデックス付き色設定
struct ColorInfo : public ColorInfoBase{
	using SName = StaticString<64>;
	int			m_nColorIdx = 0;		//!< インデックス
	SName		m_szName;				//!< 名前
};

//デフォルト色設定
void	GetDefaultColorInfo(ColorInfo* pColorInfo, size_t index) noexcept;
void	GetDefaultColorInfoName(ColorInfo* pColorInfo, size_t nIndex) noexcept;
int		GetDefaultColorInfoCount() noexcept;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           辞書                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< 辞書の説明の最大長 -1 */
struct KeyHelpInfo {
	using SAbout = StaticString<DICT_ABOUT_LEN>;
	bool		m_bUse = false;				//!< 辞書を 使用する/しない
	SAbout		m_szAbout;					//!< 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	m_szPath;					//!< ファイルパス
};
//@@@ 2006.04.10 fon ADD-end

#endif /* SAKURA_CDOCTYPESETTING_87013082_2E52_4683_8CEE_499218F2D584_H_ */
