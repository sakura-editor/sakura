//2002.01.08 aroka  �R���g���[���v���Z�X�ƋN�������̂��߂Ƀ~���[�e�b�N�X����ǉ�
//2006.04.10 ryoji  �R���g���[���v���Z�X�����������������C�x���g�t���O����ǉ�
//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����
//2009.01.17 nasukoji	�}�E�X�T�C�h�{�^���̃L�[�R�[�h��`�ǉ��i_WIN32_WINNT�̓s���ɂ��V�X�e�����Œ�`����Ȃ��ׁj

#pragma once



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �p�t�H�[�}���X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//�o�b�t�@�T�C�Y
const int LINEREADBUFSIZE	= 10240;	//!< �t�@�C������1�s���f�[�^��ǂݍ��ނƂ��̃o�b�t�@�T�C�Y



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �萔�����⏕                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���L������                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define N_SHAREDATA_VERSION		89
#define STR_SHAREDATA_VERSION	"89"
#define	GSTR_SHAREDATA	(_T("SakuraShareData") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_) _T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �~���[�e�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//���T�N���G�f�B�^ (1.2.104.1�ȑO)
#define	GSTR_MUTEX_SAKURA_OLD				_T("MutexSakuraEditor")

//�R���g���[���v���Z�X
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//�m�[�}���v���Z�X����������
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//�m�[�h���쓯��
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �E�B���h�E�N���X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�R���g���[���g���C
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)		_T(STR_SHAREDATA_VERSION))

//���C���E�B���h�E
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

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
