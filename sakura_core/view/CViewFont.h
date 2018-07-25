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
#ifndef SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_
#define SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_

#include "doc/CDocTypeSetting.h" // ColorInfo !!

class CViewFont{
public:
	CViewFont(const LOGFONT *plf, bool bMiniMap = false)
	{
		m_bMiniMap = bMiniMap;
		CreateFont(plf);
	}
	virtual ~CViewFont()
	{
		DeleteFont();
	}

	void UpdateFont(const LOGFONT *plf)
	{
		DeleteFont();
		CreateFont(plf);
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
	void CreateFont(const LOGFONT *plf);
	void DeleteFont();

	HFONT	m_hFont_HAN;			/* 現在のフォントハンドル */
	HFONT	m_hFont_HAN_BOLD;		/* 現在のフォントハンドル(太字) */
	HFONT	m_hFont_HAN_UL;			/* 現在のフォントハンドル(下線) */
	HFONT	m_hFont_HAN_BOLD_UL;	/* 現在のフォントハンドル(太字、下線) */

	LOGFONT	m_LogFont;
	bool	m_bMiniMap;
};

#endif /* SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_ */
/*[EOF]*/
