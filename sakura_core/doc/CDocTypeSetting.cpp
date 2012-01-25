// 2000.10.08 JEPRO  �w�i�F��^����RGB(255,255,255)��(255,251,240)�ɕύX(ῂ�������������)
// 2000.12.09 Jepro  note: color setting (�ڍׂ� CshareData.h ���Q�Ƃ̂���)
// 2000.09.04 JEPRO  �V���O���N�H�[�e�[�V����������ɐF�����蓖�Ă邪�F�����\���͂��Ȃ�
// 2000.10.17 JEPRO  �F�����\������悤�ɕύX(�ŏ���FALSE��TRUE)
// 2008.03.27 kobake �吮��

#include "StdAfx.h"
#include "CDocTypeSetting.h"


//! �F�ݒ�(�ۑ��p)
struct ColorInfoIni {
	const TCHAR*	m_pszName;			//!< ���ږ�
	ColorInfoBase	m_sColorInfo;		//!< �F�ݒ�
};

static ColorInfoIni ColorInfo_DEFAULT[] = {
//	���ږ�,									�\��,		����,		����,		�����F,					�w�i�F,
	_T("�e�L�X�g"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
	_T("���[���["),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ),
	_T("�J�[�\��"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	_T("�J�[�\��(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	_T("�J�[�\���s�A���_�[���C��"),			TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("�J�[�\���ʒu�c��"),					FALSE,		FALSE,		FALSE,		RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ),	// 2007.09.09 Moca
	_T("�s�ԍ�"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	_T("�s�ԍ�(�ύX�s)"),					TRUE,		TRUE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	_T("TAB�L��"),							TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)���Z���O���[�ɕύX
	_T("���p��")		,					FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2002.04.28 Add by KK
	_T("���{���"),						TRUE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
	_T("�R���g���[���R�[�h"),				TRUE,		FALSE,		FALSE,		RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ),
	_T("���s�L��"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ),
	_T("�܂�Ԃ��L��"),						TRUE,		FALSE,		FALSE,		RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("�w�茅�c��"),						FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2005.11.08 Moca
	_T("EOF�L��"),							TRUE,		FALSE,		FALSE,		RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ),
	_T("���p���l"),							FALSE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)��ύX  Mar.10, 2001 �W���͐F�Ȃ���
	_T("�Ί��ʂ̋����\��"),					FALSE,		TRUE,		FALSE,		RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ),	// 02/09/18 ai
	_T("�I��͈�"),							TRUE,		FALSE,		FALSE,		RGB(  49, 106, 197 ),	RGB(  49, 106, 197 ),	//2011.05.18
	_T("����������"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ),
	_T("����������2"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ),
	_T("����������3"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ),
	_T("����������4"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ),
	_T("����������5"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ),
	_T("�R�����g"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�V���O���N�H�[�e�[�V����������"),	TRUE,		FALSE,		FALSE,		RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ),
	_T("�_�u���N�H�[�e�[�V����������"),		TRUE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
	_T("URL"),								TRUE,		FALSE,		TRUE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h1"),					TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h2"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
	_T("�����L�[���[�h3"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
	_T("�����L�[���[�h4"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h5"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h6"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h7"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h8"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h9"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("�����L�[���[�h10"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("���K�\���L�[���[�h1"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h2"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h3"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h4"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h5"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h6"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h7"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h8"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h9"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("���K�\���L�[���[�h10"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("DIFF�����\��(�ǉ�)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("DIFF�����\��(�ύX)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("DIFF�����\��(�폜)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("�u�b�N�}�[�N"),						TRUE ,		FALSE,		FALSE,		RGB( 255, 251, 240 ),	RGB(   0, 128, 192 ),	// 02/10/16 ai
};

void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex)
{
	ColorInfoBase* p = pColorInfo;
	*p = ColorInfo_DEFAULT[nIndex].m_sColorInfo; //ColorInfoBase
	_tcscpy(pColorInfo->m_szName, ColorInfo_DEFAULT[nIndex].m_pszName);
	pColorInfo->m_nColorIdx = nIndex;
}

int GetDefaultColorInfoCount()
{
	return _countof(ColorInfo_DEFAULT);
}
