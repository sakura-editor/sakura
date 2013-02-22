/*!	@file
	@brief ���ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <tchar.h>

// �ȉ��� ifdef �u���b�N�� DLL ����ȒP�ɃG�N�X�|�[�g������}�N�����쐬����W���I�ȕ��@�ł��B
// ���� DLL ���̂��ׂẴt�@�C���̓R�}���h���C���Œ�`���ꂽ SAKURA_CORE_EXPORTS �V���{��
// �ŃR���p�C������܂��B���̃V���{���͂��� DLL ���g�p����ǂ̃v���W�F�N�g��ł�����`�łȂ�
// ��΂Ȃ�܂���B���̕��@�ł̓\�[�X�t�@�C���ɂ��̃t�@�C�����܂ނ��ׂẴv���W�F�N�g�� DLL
// ����C���|�[�g���ꂽ���̂Ƃ��� SAKURA_CORE_API �֐����Q�Ƃ��A���̂��߂��� DLL �͂��̃}�N
// ���Œ�`���ꂽ�V���{�����G�N�X�|�[�g���ꂽ���̂Ƃ��ĎQ�Ƃ��܂��B
#ifdef SAKURA_CORE_EXPORTS
#define SAKURA_CORE_API __declspec(dllexport)
#else
#define SAKURA_CORE_API __declspec(dllimport)
#endif

#ifdef SAKURA_NO_DLL	//@@@ 2001.12.30 add MIK
#undef SAKURA_CORE_API
#define SAKURA_CORE_API
#endif	//SAKURA_NO_DLL

#if defined(__BORLANDC__)
#define __forceinline inline
#define _itoa itoa
#define _stricmp stricmp
#define _memicmp memicmp
#endif





/* �E�B���h�E��ID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000

#include "charset/charset.h"

/* �_�C�A���O�\�����@ */ // �A�E�g���C���E�B���h�E�p�ɍ쐬 20060201 aroka
SAKURA_CORE_API enum enumShowDlg {
	SHOW_NORMAL			= 0,
	SHOW_RELOAD			= 1,
	SHOW_TOGGLE			= 2,
};


/* �I��̈�`��p�p�����[�^ */
SAKURA_CORE_API extern const COLORREF	SELECTEDAREA_RGB;
SAKURA_CORE_API extern const int		SELECTEDAREA_ROP2;





//@@@ From Here 2003.05.31 MIK
/*! �^�u�E�C���h�E�p���b�Z�[�W�T�u�R�}���h */
SAKURA_CORE_API enum ETabWndNotifyType {
	TWNT_REFRESH	= 0,		//�ĕ\��
	TWNT_ADD		= 1,		//�E�C���h�E�o�^
	TWNT_DEL		= 2,		//�E�C���h�E�폜
	TWNT_ORDER		= 3,		//�E�C���h�E�����ύX
	TWNT_FILE		= 4,		//�t�@�C�����ύX
	TWNT_MODE_ENABLE= 5,		//�^�u���[�h�L����	//2004.07.14 Kazika �ǉ�
	TWNT_MODE_DISABLE= 6,		//�^�u���[�h������	//2004.08.27 Kazika �ǉ�
	TWNT_WNDPL_ADJUST= 7,		//�E�B���h�E�ʒu���킹	// 2007.04.03 ryoji �ǉ�
};

/*! �o�[�̕\���E��\�� */
SAKURA_CORE_API	enum EBarChangeNotifyType {
	MYBCN_TOOLBAR	= 0,		//�c�[���o�[
	MYBCN_FUNCKEY	= 1,		//�t�@���N�V�����L�[
	MYBCN_TAB		= 2,		//�^�u
	MYBCN_STATUSBAR	= 3,		//�X�e�[�^�X�o�[
};
//@@@ To Here 2003.05.31 MIK

//�^�u�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
//�E�N���b�N���j���[�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


/*!< �F�^�C�v */
//@@@ From Here 2006.12.18 ryoji
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
#define COLOR_ATTRIB_NO_TEXT		0x00000010
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	�\��l
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct SColorAttributeData{
	const TCHAR*			szName;
	unsigned int	fAttribute;
};
SAKURA_CORE_API extern const SColorAttributeData g_ColorAttributeArr[];

//@@@ To Here 2006.12.18 ryoji

/*!< �ݒ�l�̏���E���� */
//	���[���̍���
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

/**	�}�E�X�N���b�N�ƃL�[��`�̑Ή�

	@date 2007.11.04 genta �V�K�쐬�D���l����Ɣ͈̓T�C�Y��`�̂���
*/
enum MOUSEFUNCTION_ASSIGN {
	MOUSEFUNCTION_DOUBLECLICK	= 0,	//!< �_�u���N���b�N
	MOUSEFUNCTION_RIGHT			= 1,	//!< �E�N���b�N
	MOUSEFUNCTION_CENTER		= 2,	//!< ���N���b�N
	MOUSEFUNCTION_LEFTSIDE		= 3,	//!< ���T�C�h�N���b�N
	MOUSEFUNCTION_RIGHTSIDE		= 4,	//!< �E�T�C�h�N���b�N
	MOUSEFUNCTION_TRIPLECLICK	= 5,	//!< �g���v���N���b�N
	MOUSEFUNCTION_QUADCLICK		= 6,	//!< �N�A�h���v���N���b�N
	MOUSEFUNCTION_KEYBEGIN		= 7,	//!< �}�E�X�ւ̊��蓖�Č����{���̃L�[���蓖�Đ擪INDEX
};

// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
enum WRAP_TEXT_WRAP_METHOD {
	WRAP_NO_TEXT_WRAP		= 0,		// �܂�Ԃ��Ȃ��i�X�N���[���o�[���e�L�X�g���ɍ��킹��j
	WRAP_SETTING_WIDTH,					// �w�茅�Ő܂�Ԃ�
	WRAP_WINDOW_WIDTH,					// �E�[�Ő܂�Ԃ�
};

// 2009.07.06 syat	�����J�E���g���@
enum ESelectCountMode {
	SELECT_COUNT_TOGGLE		= 0,	// �����J�E���g���@���g�O��
	SELECT_COUNT_BY_CHAR	= 1,	// �������ŃJ�E���g
	SELECT_COUNT_BY_BYTE	= 2		// �o�C�g���ŃJ�E���g
};

//2007.09.06 kobake �ǉ�
//!��������
enum ESearchDirection{
	SEARCH_BACKWARD = 0, //�O������ (�O������)
	SEARCH_FORWARD  = 1, //������� (��������) (����)
};

//2007.09.06 kobake �ǉ�
struct SSearchOption{
//	ESearchDirection	eDirection;
//	bool	bPrevOrNext;	//!< false==�O������ true==�������
	bool	bRegularExp;	//!< true==���K�\��
	bool	bLoHiCase;		//!< true==�p�啶���������̋��
	bool	bWordOnly;		//!< true==�P��̂݌���

	SSearchOption() : bRegularExp(false), bLoHiCase(false), bWordOnly(false) { }
	SSearchOption(
		bool _bRegularExp,
		bool _bLoHiCase,
		bool _bWordOnly
	)
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
	{
	}
	void Reset()
	{
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	//���Z�q
	bool operator == (const SSearchOption& rhs) const
	{
		//�Ƃ肠����memcmp�ł�����
		return memcmp(this,&rhs,sizeof(*this))==0;
	}
	bool operator != (const SSearchOption& rhs) const
	{
		return !operator==(rhs);
	}

};



//2007.10.02 kobake CEditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
class CEditWnd;
extern CEditWnd* g_pcEditWnd;


HWND G_GetProgressBar();
HINSTANCE G_AppInstance();


///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */




