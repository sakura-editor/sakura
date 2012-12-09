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
#ifndef SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_
#define SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_

//2002.01.08 aroka  �R���g���[���v���Z�X�ƋN�������̂��߂Ƀ~���[�e�b�N�X����ǉ�
//2006.04.10 ryoji  �R���g���[���v���Z�X�����������������C�x���g�t���O����ǉ�
//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
//2009.01.17 nasukoji	�}�E�X�T�C�h�{�^���̃L�[�R�[�h��`�ǉ��i_WIN32_WINNT�̓s���ɂ��V�X�e�����Œ�`����Ȃ��ׁj
//2009.02.11 ryoji		���L����������ǉ�, NUM_TO_STR�}�N���ǉ�
//2010.08.21 Moca		Win64��萔�ŕ���

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �萔�����⏕                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//���l�萔�̕����� 2009.02.11 ryoji
#define _NUM_TO_STR(n) #n
#define NUM_TO_STR(n) _NUM_TO_STR(n)

//�f�o�b�O���ʁA�萔�T�t�B�b�N�X 2007.09.20 kobake
#ifdef _DEBUG
	#define _DEBUG_SUFFIX_ "_DEBUG"
#else
	#define _DEBUG_SUFFIX_ ""
#endif

//�r���h�R�[�h���ʁA�萔�T�t�B�b�N�X 2007.09.20 kobake
#ifdef _UNICODE
	#define _CODE_SUFFIX_ "W"
#else
	#define _CODE_SUFFIX_ "A"
#endif

// �^�[�Q�b�g�}�V������ 2010.08.21 Moca �ǉ�
#ifdef _WIN64
	#define CON_SKR_MACHINE_SUFFIX_ "M64"
#else
	#define CON_SKR_MACHINE_SUFFIX_ ""
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���L������                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!	���L�������̃o�[�W����
/*!
	���L�������̃o�[�W�����ԍ��B���L�������̌`����ύX�����Ƃ��͂�����1���₷�B

	���̒l�͋��L�������̃o�[�W�����t�B�[���h�Ɋi�[����A�قȂ�\���̋��L��������
	�g���G�f�B�^�������ɋN�����Ȃ��悤�ɂ���B

	�ݒ�\�Ȓl�� 1�`unsinged int�̍ő�l

	@sa Init()

	Version 1�`24:
	���낢��
	
	Version 25:
	m_bStopsBothEndsWhenSearchWord�ǉ�
	
	Version 26:
	MacroRec�ɁAm_bReloadWhenExecute��ǉ� 2002/03/11 YAZAKI
	EditInfo�ɁAm_szDocType�ǉ� Mar. 7, 2002 genta
	
	Version 27:
	STypeConfig�ɁAm_szOutlineRuleFilename��ǉ� 2002.04.01 YAZAKI 
	
	Version 28:
	PRINTSETTING�ɁAm_bPrintKinsokuHead�Am_bPrintKinsokuTail��ǉ� 2002.04.09 MIK
	STypeConfig�ɁAm_bKinsokuHead�Am_bKinsokuTail�Am_szKinsokuHead�Am_szKinsokuTail��ǉ� 2002.04.09 MIK

	Version 29:
	PRINTSETTING�ɁAm_bPrintKinsokuRet��ǉ� 2002.04.13 MIK
	STypeConfig�ɁAm_bKinsokuRet��ǉ� 2002.04.13 MIK

	Version 30:
	PRINTSETTING�ɁAm_bPrintKinsokuKuto��ǉ� 2002.04.17 MIK
	STypeConfig�ɁAm_bKinsokuKuto��ǉ� 2002.04.17 MIK

	Version 31:
	Common�ɁAm_bStopsBothEndsWhenSearchParagraph��ǉ� 2002/04/26 YAZAKI

	Version 32:
	Common����STypeConfig�ցAm_bAutoIndent�Am_bAutoIndent_ZENSPACE���ړ� 2002/04/30 YAZAKI

	Version 33:
	Common�ɁAm_lf_kh��ǉ� 2002/05/21 ai
	m_nDiffFlgOpt��ǉ� 2002.05.27 MIK
	STypeConfig-Color��COLORIDX_DIFF_APPEND,COLORIDX_DIFF_CHANGE,COLORIDX_DIFF_DELETE��ǉ�

	Version 34:
	STypeConfig��m_bUseDocumentIcon �ǉ��D 2002.09.10 genta
	
	Version 35:
	Common��m_nLineNumRightSpace �ǉ��D2002.09.18 genta

	Version 36:
	Common��m_bGrepKanjiCode_AutoDetect���폜�Am_nGrepCharSet��ǉ� 2002/09/21 Moca

	Version 37:
	STypeConfig��LineComment�֘A��m_cLineComment�ɕύX�D  @@@ 2002.09.23 YAZAKI
	STypeConfig��BlockComment�֘A��m_cBlockComments�ɕύX�D@@@ 2002.09.23 YAZAKI

	Version 38:
	STypeConfig��m_bIndentLayout��ǉ�. @@@ 2002.09.29 YAZAKI
	2002.10.01 genta m_nIndentLayout�ɖ��O�ύX

	Version 39:
	Common��m_nFUNCKEYWND_GroupNum��ǉ��D 2002/11/04 Moca

	Version 40:
	�t�@�C�����ȈՕ\�L�֘A��ǉ��D 2002/12/08�`2003/01/15 Moca

	Version 41:
	STypeConfig��m_szTabViewString�T�C�Y�g��
	m_nWindowSizeX/Y m_nWindowOriginX/Y�ǉ� 2003.01.26 aroka

	Version 42:
	STypeConfig�ɓƎ�TAB�}�[�N�t���O�ǉ� 2003.03.28 MIK

	Version 43:
	�ŋߎg�����t�@�C���E�t�H���_�ɂ��C�ɓ����ǉ� 2003.04.08 MIK

	Version 44:
	Window Caption������̈��Common�ɒǉ� 2003.04.05 genta

	Version 45:
	�^�O�t�@�C���쐬�p�R�}���h�I�v�V�����ۑ��̈�(m_nTagsOpt,m_szTagsCmdLine)��ǉ� 2003.05.12 MIK

	Version 46:
	�ҏW�E�C���h�E���C���A�^�u�E�C���h�E�p���ǉ�

	Version 47:
	�t�@�C������̕⊮��STypeConfig�ɒǉ� 2003.06.28 Moca

	Version 48:
	Grep���A���^�C���\���ǉ� 2003.06.28 Moca

	Version 49:
	�t�@�C������IsDebug�ǉ� (�^�u�\���p) 2003.10.13 MIK
	
	Version 50:
	�E�B���h�E�ʒu�Œ�E�p����ǉ� 2004.05.13 Moca

	Version 51:
	�^�O�W�����v�@�\�ǉ� 2004/06/21 novice

	Version 52:
	�O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O�ǉ�	2004.10.03 genta

	Version 53:
	���݂��Ȃ��t�@�C�����J�����Ƃ����ꍇ�Ɍx������t���O�̒ǉ�	2004.10.09 genta

	Version 54:
	�}�E�X�T�C�h�{�^���Ή� 2004/10/10 novice

	Version 55:
	�}�E�X���{�^���Ή� 2004/10/11 novice

	Version 56:
	�C���N�������^���T�[�`(Migemo path�p) 2004/10/13 isearch
	
	Version 57:
	�����L�[���[�h�w��g�� 2005/01/13 MIK
	
	Version 58:
	�����L�[���[�h�Z�b�g�ϒ����蓖�� 2005/01/25 Moca

	Version 59:
	�}�N�����𑝂₵�� 2005/01/30 genta
	
	Version 60:
	�L�[���[�h�w��^�O�W�����v����ۑ� 2005/04/03 MIK

	Version 61:
	���s�ōs���̋󔒂��폜����I�v�V����(�^�C�v�ʐݒ�) 2005/10/11 ryoji

	Version 62:
	�o�b�N�A�b�v�t�H���_ 2005.11.07 aroka

	Version 63:
	�w�茅�c���\���ǉ� 2005.11.08 Moca

	Version 64:
	�^�u�̋@�\�g��(�����A�A�C�R���\��) 2006/01/28 ryoji
	�A�E�g�v�b�g�E�B���h�E�ʒu 2006.02.01 aroka

	Version 65:
	�^�u�ꗗ���\�[�g���� 2006/05/10 ryoji

	Version 66:
	�L�[���[�h�w���v�@�\�g�� 2006.04.10 fon
		�L�[���[�h�w���v�@�\�ݒ��ʃ^�u�Ɉړ�, �����̕������ɑΉ�, �L�����b�g�ʒu�L�[���[�h�����ǉ�

	Version 67:
	�L�����b�g�F�w���ǉ� 2006.12.07 ryoji

	Version 68:
	�t�@�C���_�C�A���O�̃t�B���^�ݒ� 2006.11.16 ryoji

	Version 69:
	�u���ׂĒu���v�͒u���̌J�Ԃ� 2006.11.16 ryoji

	Version 70:
	[���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� 2006.12.25 ryoji

	Version 71:
	�^�u����铮��𐧌䂷��I�v�V����2��ǉ� 2007.02.11 genta
		m_bRetainEmptyTab, m_bCloseOneWinInTabMode

	Version 72:
	�^�u�ꗗ���t���p�X�\������ 2007.02.28 ryoji

	Version 73:
	�ҏW�E�B���h�E�ؑ֒� 2007.04.03 ryoji

	Version 74:
	�J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji

	Version 75:
	�}�E�X�z�C�[���ŃE�B���h�E�؂�ւ� 2006.03.26 ryoji

	Version 76:
	�^�u�̃O���[�v�� 2007.06.20 ryoji

	Version 77:
	ini�t�H���_�ݒ� 2007.05.31 ryoji

	Version 78:
	�G�f�B�^�|�g���C�Ԃł�UI���������m�F�̂��߂̃o�[�W�������킹 2007.06.07 ryoji

	Version 79:
	�O���R�}���h���s�̃I�v�V�����g�� 2007.04.29 maru

	Version 80:
	���K�\�����C�u�����̐؂�ւ� 2007.07.22 genta

	Version 81:
	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji

	Version 82:
	���C�����[�h�\��t�����\�ɂ��� 2007.10.08 ryoji

	Version 83:
	�I���Ȃ��ŃR�s�[���\�ɂ��� 2007.11.18 ryoji

	Version 84:
	��ʃL���b�V�����g�� 2007.09.09 Moca

	Version 85:
	�L�[���[�h�Z�b�g�ő�l���� 2007.12.01 genta

	Version 86:
	�^�C�v�ʐݒ�ő�l���� 2007.12.13 ryoji

	Version 87:
	�e�L�X�g�̐܂�Ԃ����@�ǉ� 2008.05.30 nasukoji

	Version 88:
	�}�E�X�̒��{�^���������̃z�C�[���X�N���[���ł̓y�[�W�X�N���[������ 2008.10.07 nasukoji

	Version 89:
	�z�C�[���X�N���[���𗘗p�����y�[�W�X�N���[���E���X�N���[���Ή��iVer.88�ǉ������͍폜�j 2009.01.17 nasukoji

	Version 90:
	STypeConfig��m_szTabViewString�T�C�Y�g���iVersion 41�j�̖߂� 2009.02.11 ryoji

	Version 91
	�u�J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx���v�@�\�ǉ�  2009.05.24

	Version 92
	�f�t�H���g�����R�[�h��CESU-8���ʃI�v�V�����Ή�		2009.06.15

	Version 93
	�I�𕶎����J�E���g�P�ʃI�v�V�����Ή�				2009.06.25

	Version 94
	��Ǔ_�Ԃ牺������ 2009.08.07 ryoji

	Version 95
	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p) 2009.08.15 nasukoji

	Version 96:
	�������s�}�N�� 2006/08/31 ryoji

	Version 97:
	��DPI�p�Ƀt�H���g�T�C�Y�i1/10�|�C���g�P�ʁj��ǉ� 2009/10/01 ryoji

	Version 98:
	WSH�v���O�C�� 2010/03/03 syat

	Version 99:
	Backup Removable Media �Ή�  2010/5/27 Uchi

	Version 100:
	ICON�����ׂẴR�}���h�Ɋ��蓖�Ă�	2010/6/12 Uchi

	Version 101:
	���C���j���[�ݒ�	2010/5/18 Uchi

	Version 102:
	Plugin��ini�t�@�C���ւ̏������݂̕�����	2010/7/15 Uchi

	Version 103:
	�A�E�g���C����͉�ʂ̃h�b�L���O�@�\�ǉ� 2010.06.05 ryoji

	Version 104:
	�f�t�H���g�̉��s�R�[�h��BOM 2011.01.24 ryoji

	Version 105:
	�w�i�摜�\�� 2010.09.17 Moca

	Version 106:
	�㏑���֎~���o���͕ҏW�֎~�ɂ��� 2011.02.19 ryoji

	Version 107:
	�I��͈͐F�w�� 2011.05.18 Moca

	Version 108:
	MRU�Ƀ^�C�v�ʐݒ�ǉ� 2011.06.13 Moca

	Version 109:
	�P�ꌟ���ŕ������� 2011.11.15 ds14050, syat

	Version 110:
	���s�R�[�h��ϊ����ē\��t���� 2011.11.22 salarm, Moca
	
	Version 111:
	�^�u�o�[�t�H���g�w�� 2011.12.04 Moca
	
	Version 112:
	���͕⊮�v���O�C�� 2011.06.24 Moca

	Version 113:
	�}�N����~�_�C�A���O 2011.08.04 syat

	Version 114:
	CCodeMediator/CESI��CEditDoc�ˑ��̏C�� 2012.10.12 Moca

	Version 115:
	�����E�u���L�[���[�h�̕����񒷐����P�p/���̃r���[�̌��������������p�� 2011.12.18 Moca

	Version 116:
	MRU���O���X�g 2012.10.30 Moca

	Version 117:
	�����Ǎ����x�� 2012.11.20 Uchi

	Version 118:
	���K�\���L�[���[�h�ϒ� 2012.12.01 Moca

	Version 119:
	CommonSetting_KeyBind �̃����o�ϐ��ǉ� 2012.11.25 aroka

	Version 120:
	�E�[�ŐܕԂ����̂Ԃ牺�����B����ǉ� 2012.11.30 Uchi

	-- �������ꂽ�̂Ō��ɖ߂��i1000�`1023���g�p�ς݁j 	2008.11.16 nasukoji
	-- Version 1000:
	-- �o�[�W����1000�ȍ~��{�Ɠ����܂ł̊ԁA�g�킹�Ă��������B���Ȃ�p�ɂɍ\�����ύX�����Ǝv����̂ŁBby kobake 2008.03.02

*/
#define N_SHAREDATA_VERSION		120
#define STR_SHAREDATA_VERSION	NUM_TO_STR(N_SHAREDATA_VERSION)
#define	GSTR_SHAREDATA	(_T("SakuraShareData") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_) _T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �~���[�e�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//���T�N���G�f�B�^ (1.2.104.1�ȑO)
#define	GSTR_MUTEX_SAKURA_OLD				_T("MutexSakuraEditor")

//�R���g���[���v���Z�X
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//�m�[�}���v���Z�X����������
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//�m�[�h���쓯��
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �E�B���h�E�N���X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�R���g���[���g���C
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)		_T(STR_SHAREDATA_VERSION))

//���C���E�B���h�E
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//�r���[
#define	GSTR_VIEWNAME		(_T("EditorClient")												_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���\�[�X                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	Dec. 2, 2002 genta
//	�Œ�t�@�C����
#define FN_APP_ICON  _T("my_appicon.ico")
#define FN_GREP_ICON _T("my_grepicon.ico")
#define FN_TOOL_BMP  _T("my_icons.bmp")

//	�W���A�v���P�[�V�����A�C�R�����\�[�X��
#ifdef _DEBUG
	#define ICON_DEFAULT_APP IDI_ICON_DEBUG
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#else
	#define ICON_DEFAULT_APP IDI_ICON_STD
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �p�t�H�[�}���X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//�o�b�t�@�T�C�Y
const int LINEREADBUFSIZE	= 10240;	//!< �t�@�C������1�s���f�[�^��ǂݍ��ނƂ��̃o�b�t�@�T�C�Y


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �t���O                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#define _SHIFT	0x00000001
#define _CTRL	0x00000002
#define _ALT	0x00000004



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���b�Z�[�W                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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

//! �e�L�X�g�ǉ��ʒm(���L�f�[�^�o�R)
// wParam:�ǉ����镶����(WCHAR�P��)
// lParam:���g�p
// @2010.05.11 Moca ���b�Z�[�W�ǉ�
#define	MYWM_ADDSTRINGLEN_W (WM_APP+219)

/*! �A�E�g���C����͉�ʂ̒ʒm */
#define MYWM_OUTLINE_NOTIFY (WM_APP+220)	//@@@ 2010.06.06 ryoji

/* �ĕϊ��Ή� */ // 20020331 aroka
#ifndef WM_IME_REQUEST
#define MYWM_IME_REQUEST 0x288  // ==WM_IME_REQUEST
#else
#define MYWM_IME_REQUEST WM_IME_REQUEST
#endif
#define MSGNAME_ATOK_RECONVERT TEXT("Atok Message for ReconvertString")
#define RWM_RECONVERT TEXT("MSIMEReconvert")

#define	VK_XBUTTON1		0x05	// �}�E�X�T�C�h�{�^��1�iWindows2000�ȍ~�Ŏg�p�\�j	// 2009.01.17 nasukoji
#define	VK_XBUTTON2		0x06    // �}�E�X�T�C�h�{�^��2�iWindows2000�ȍ~�Ŏg�p�\�j	// 2009.01.17 nasukoji

#endif /* SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_ */
/*[EOF]*/
