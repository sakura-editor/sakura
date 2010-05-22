#pragma once


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
