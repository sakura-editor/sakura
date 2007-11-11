#pragma once

class CViewFont{
public:
	CViewFont();
	virtual ~CViewFont();

	void UpdateFont();
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine ) const;		/* フォントを選ぶ */

	HFONT GetFontHan() const
	{
		return m_hFont_HAN;
	}
private:
	HFONT			m_hFont_HAN;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT;	/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_UL;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT_UL;	/* 現在のフォントハンドル */
};

