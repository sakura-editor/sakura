/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
