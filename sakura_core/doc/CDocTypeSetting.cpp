// 2000.10.08 JEPRO  �w�i�F��^����RGB(255,255,255)��(255,251,240)�ɕύX(ῂ�������������)
// 2000.12.09 Jepro  note: color setting (�ڍׂ� CshareData.h ���Q�Ƃ̂���)
// 2000.09.04 JEPRO  �V���O���N�H�[�e�[�V����������ɐF�����蓖�Ă邪�F�����\���͂��Ȃ�
// 2000.10.17 JEPRO  �F�����\������悤�ɕύX(�ŏ���FALSE��TRUE)
// 2008.03.27 kobake �吮��

#include "StdAfx.h"
#include "CDocTypeSetting.h"


//! �F�ݒ�(�ۑ��p)
struct ColorInfoIni {
	int				m_nNameId;			//!< ���ږ�
	ColorInfoBase	m_sColorInfo;		//!< �F�ݒ�
};

static ColorInfoIni ColorInfo_DEFAULT[] = {
//	���ږ�,									�\��,		����,		����,		�����F,					�w�i�F,
	STR_COLOR_TEXT,					TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_RULER,				TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ),
	STR_COLOR_CURSOR,				TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	STR_COLOR_CURSOR_IMEON,			TRUE,		FALSE,		FALSE,		RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	STR_COLOR_CURSOR_LINE_BG,		FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255, 128 ),	// 2012.11.21 Moca
	STR_COLOR_CURSOR_LINE,			TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	STR_COLOR_CURSOR_COLUMN,		FALSE,		FALSE,		FALSE,		RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ),	// 2007.09.09 Moca
	STR_COLOR_LINE_NO,				TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	STR_COLOR_LINE_NO_CHANGE,		TRUE,		TRUE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	STR_COLOR_TAB,					TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)���Z���O���[�ɕύX
	STR_COLOR_HALF_SPACE,			FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2002.04.28 Add by KK
	STR_COLOR_FULL_SPACE,			TRUE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
	STR_COLOR_CTRL_CODE,			TRUE,		FALSE,		FALSE,		RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_CR,					TRUE,		FALSE,		FALSE,		RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ),
	STR_COLOR_WRAP_MARK,			TRUE,		FALSE,		FALSE,		RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ),
	STR_COLOR_VERT_LINE,			FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2005.11.08 Moca
	STR_COLOR_EOF,					TRUE,		FALSE,		FALSE,		RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ),
	STR_COLOR_NUMBER,				FALSE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)��ύX  Mar.10, 2001 �W���͐F�Ȃ���
	STR_COLOR_BRACKET,				FALSE,		TRUE,		FALSE,		RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ),	// 02/09/18 ai
	STR_COLOR_SELECTED_AREA,		TRUE,		FALSE,		FALSE,		RGB(  49, 106, 197 ),	RGB(  49, 106, 197 ),	//2011.05.18
	STR_COLOR_SEARCH_WORD1,			TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ),
	STR_COLOR_SEARCH_WORD2,			TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ),
	STR_COLOR_SEARCH_WORD3,			TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ),
	STR_COLOR_SEARCH_WORD4,			TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ),
	STR_COLOR_SEARCH_WORD5,			TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ),
	STR_COLOR_COMMENT,				TRUE,		FALSE,		FALSE,		RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_SINGLE_QUOTE,			TRUE,		FALSE,		FALSE,		RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ),
	STR_COLOR_DOUBLE_QUOTE,			TRUE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
	STR_COLOR_HERE_DOCUMENT,		FALSE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
	STR_COLOR_URL,					TRUE,		FALSE,		TRUE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD1,				TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD2,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
	STR_COLOR_KEYWORD3,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
	STR_COLOR_KEYWORD4,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD5,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD6,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD7,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD8,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD9,				TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_KEYWORD10,			TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	STR_COLOR_REGEX_KEYWORD1,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD2,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD3,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD4,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD5,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD6,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD7,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD8,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD9,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_REGEX_KEYWORD10,		FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	STR_COLOR_DIFF_ADD,				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	STR_COLOR_DIFF_CNG,				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	STR_COLOR_DIFF_DEL,				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	STR_COLOR_BOOKMARK,				TRUE ,		FALSE,		FALSE,		RGB( 255, 251, 240 ),	RGB(   0, 128, 192 ),	// 02/10/16 ai
};

void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex)
{
	ColorInfoBase* p = pColorInfo;
	*p = ColorInfo_DEFAULT[nIndex].m_sColorInfo; //ColorInfoBase
	_tcscpy(pColorInfo->m_szName, to_tchar(LSW( ColorInfo_DEFAULT[nIndex].m_nNameId )));
	pColorInfo->m_nColorIdx = nIndex;
}

int GetDefaultColorInfoCount()
{
	return _countof(ColorInfo_DEFAULT);
}
