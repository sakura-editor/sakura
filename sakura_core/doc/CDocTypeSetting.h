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
#ifndef SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_
#define SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�ݒ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �F�ݒ�
struct ColorInfoBase{
	bool		m_bDisp;			//!< �\��
	bool		m_bFatFont;			//!< ����
	bool		m_bUnderLine;		//!< ����
	COLORREF	m_colTEXT;			//!< �����F
	COLORREF	m_colBACK;			//!< �w�i�F
};

//! ���O�ƃC���f�b�N�X�t���F�ݒ�
struct NamedColorInfo : public ColorInfoBase{
	int			m_nColorIdx;		//!< �C���f�b�N�X
	TCHAR		m_szName[64];		//!< ���O
};


typedef NamedColorInfo ColorInfo;


//�f�t�H���g�F�ݒ�
void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex);
int GetDefaultColorInfoCount();



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< �����̐����̍ő咷 -1 */
struct KeyHelpInfo {
	bool		m_bUse;						//!< ������ �g�p����/���Ȃ�
	TCHAR		m_szAbout[DICT_ABOUT_LEN];	//!< �����̐���(�����t�@�C����1�s�ڂ��琶��)
	SFilePath	m_szPath;					//!< �t�@�C���p�X
};
//@@@ 2006.04.10 fon ADD-end

// �Ƃ肠�����R�����g�A�E�g
// #include "../types/CType.h"

#endif /* SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_ */
/*[EOF]*/
