﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CVIEWFONT_650DC4AC_DEA2_4652_B47A_6CA998C2F9EF_H_
#define SAKURA_CVIEWFONT_650DC4AC_DEA2_4652_B47A_6CA998C2F9EF_H_
#pragma once

#include "doc/CDocTypeSetting.h" // ColorInfo !!

class CViewFont{
	using Me = CViewFont;

public:
	CViewFont(const LOGFONT *plf, bool bMiniMap = false)
	{
		m_bMiniMap = bMiniMap;
		CreateFonts(plf);
	}
	CViewFont(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CViewFont(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CViewFont()
	{
		DeleteFonts();
	}

	void UpdateFont(const LOGFONT *plf)
	{
		DeleteFonts();
		CreateFonts(plf);
	}

	HFONT ChooseFontHandle( int fontNo, SFontAttr sFontAttr ) const;		/* フォントを選ぶ */

	HFONT GetFontHan() const
	{
		return m_hFont_HAN;
	}

	const LOGFONT& GetLogfont(int FontNo = 0) const
	{
		return m_LogFont;
	}

private:
	void CreateFonts( const LOGFONT *plf );
	void DeleteFonts( void );

	HFONT	m_hFont_HAN;			/* 現在のフォントハンドル */
	HFONT	m_hFont_HAN_BOLD;		/* 現在のフォントハンドル(太字) */
	HFONT	m_hFont_HAN_UL;			/* 現在のフォントハンドル(下線) */
	HFONT	m_hFont_HAN_BOLD_UL;	/* 現在のフォントハンドル(太字、下線) */

	LOGFONT	m_LogFont;
	bool	m_bMiniMap;
};
#endif /* SAKURA_CVIEWFONT_650DC4AC_DEA2_4652_B47A_6CA998C2F9EF_H_ */
