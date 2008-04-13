#pragma once

class CViewFont{
public:
	CViewFont();
	virtual ~CViewFont();

	void UpdateFont();
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine ) const;		/* �t�H���g��I�� */

	HFONT GetFontHan() const
	{
		return m_hFont_HAN;
	}
private:
	HFONT			m_hFont_HAN;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT;	/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_UL;		/* ���݂̃t�H���g�n���h�� */
	HFONT			m_hFont_HAN_FAT_UL;	/* ���݂̃t�H���g�n���h�� */
};

