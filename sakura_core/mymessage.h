/*!	@file
	@brief �A�v���P�[�V������`�̃��b�Z�[�W
	
	��ɃG�f�B�^�E�B���h�E�ƃR���g���[���v���Z�X�̒ʐM�Ɏg����B

	@author Norio Nakatani
	@date 1998/5/15 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include <windows.h>

//wParam: X
//lParam: Y
#define MYWM_DOSPLIT        (WM_APP+1500)	

//wParam: �Ȃ񂩂̃C���f�b�N�X
//lParam: bool���ۂ�����
#define MYWM_SETACTIVEPANE  (WM_APP+1510)

//!�ݒ肪�ύX���ꂽ���Ƃ̒ʒm���b�Z�[�W
//wParam:���g�p
//lParam:���g�p
#define MYWM_CHANGESETTING  (WM_APP+1520)

//! �^�X�N�g���C����̒ʒm���b�Z�[�W
#define MYWM_NOTIFYICON		(WM_APP+100)



/*! �g���C����G�f�B�^�ւ̏I���v�� */
#define	MYWM_CLOSE			(WM_APP+200)

/*! �g���C����G�f�B�^�ւ̕ҏW�t�@�C����� �v���ʒm */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/*! �J�[�\���ʒu�ύX�ʒm */
#define	MYWM_SETCARETPOS	(WM_APP+204)
/// MYWM_SETCARETPOS���b�Z�[�W��LPARAM
enum e_PM_SETCARETPOS_SELECTSTATE {
	PM_SETCARETPOS_NOSELECT		= 0, /*!<�I������ */
	PM_SETCARETPOS_SELECT		= 1, /*!<�I���J�n�E�ύX */
	PM_SETCARETPOS_KEEPSELECT	= 2, /*!<���݂̑I����Ԃ�ۂ��Ĉړ� */
};

/*! �J�[�\���ʒu�擾�v�� */
#define	MYWM_GETCARETPOS	(WM_APP+205)

//! �e�L�X�g�ǉ��ʒm(���L�f�[�^�o�R)
//wParam:���g�p
//lParam:���g�p
#define	MYWM_ADDSTRING		(WM_APP+206)

/*! �^�O�W�����v���ʒm */
#define	MYWM_SETREFERER		(WM_APP+207)

/*! �s(���s�P��)�f�[�^�̗v�� */
#define	MYWM_GETLINEDATA	(WM_APP+208)


/*! �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
#define	MYWM_DELETE_ME		(WM_APP+209)

/*! �V�����ҏW�E�B���h�E�̍쐬�˗�(�R�}���h���C����n��) */
#define	MYWM_OPENNEWEDITOR	(WM_APP+210)

//�w���v���ۂ�����
#define	MYWM_HTMLHELP			(WM_APP+212)

/*! �^�u�E�C���h�E�p���b�Z�[�W */
#define	MYWM_TAB_WINDOW_NOTIFY	(WM_APP+213)	//@@@ 2003.05.31 MIK

/*! �o�[�̕\���E��\���ύX���b�Z�[�W */
#define	MYWM_BAR_CHANGE_NOTIFY	(WM_APP+214)	//@@@ 2003.06.10 MIK

/*! �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W */
#define	MYWM_UIPI_CHECK	(WM_APP+215)	//@@@ 2007.06.07 ryoji

/*! �|�b�v�A�b�v�E�B���h�E�̕\���ؑ֎w�� */
#define MYWM_SHOWOWNEDPOPUPS (WM_APP+216)	//@@@ 2007.10.22 ryoji

/*! �v���Z�X�̏���A�C�h�����O�ʒm */
#define MYWM_FIRST_IDLE (WM_APP+217)	//@@@ 2008.04.19 ryoji

/*! �Ǝ��̃h���b�v�t�@�C���ʒm */
#define MYWM_DROPFILES (WM_APP+218)	//@@@ 2008.06.18 ryoji

/* �ĕϊ��Ή� */ // 20020331 aroka
#ifndef WM_IME_REQUEST
#define MYWM_IME_REQUEST 0x288  // ==WM_IME_REQUEST
#else
#define MYWM_IME_REQUEST WM_IME_REQUEST
#endif
#define MSGNAME_ATOK_RECONVERT TEXT("Atok Message for ReconvertString")
#define RWM_RECONVERT TEXT("MSIMEReconvert")


