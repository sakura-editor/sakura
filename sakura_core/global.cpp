/*!	@file
	@brief �����񋤒ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "global.h"

//2007.10.02 kobake CEditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
CEditWnd* g_pcEditWnd = NULL;


/*! �I��̈�`��p�p�����[�^ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;

/*!
  ini�̐F�ݒ��ԍ��łȂ�������ŏ����o���B(added by Stonee, 2001/01/12, 2001/01/15)
  �z��̏��Ԃ͋��L���������̃f�[�^�̏��Ԃƈ�v���Ă���B

  @note ���l�ɂ������I�Ή���global.h�ōs���Ă���̂ŎQ�Ƃ̂��ƁB(Mar. 7, 2001 jepro)
  CShareData����global�Ɉړ�
*/
const SColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// �L�����b�g		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IME�L�����b�g	// 2006.12.07 ryoji
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca �J�[�\���ʒu�c��
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	//2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca �w�茅�c��
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK ���p���l�̋���
	{_T("FND"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK �����L�[���[�h3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("URL"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	//DIFF�ǉ�	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	//DIFF�ύX	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	//DIFF�폜	//@@@ 2002.06.01 MIK
	{_T("BRC"), 0},	//�Ί���	// 02/09/18 ai Add
	{_T("MRK"), 0},	//�u�b�N�}�[�N	// 02/10/16 ai Add
	{_T("LAST"), 0}	// Not Used
};



/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( _tcscmp( name, g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * �C���f�b�N�X�ԍ�����J���[���ɕϊ�����
 */
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}





HWND G_GetProgressBar()
{
	if(CEditWnd::Instance()){
		return CEditWnd::Instance()->m_cStatusBar.GetProgressHwnd();
	}
	else{
		return NULL;
	}
}

HINSTANCE G_AppInstance()
{
	return CNormalProcess::Instance()->GetProcessInstance();
}
