//2002.01.08 aroka  �R���g���[���v���Z�X�ƋN�������̂��߂Ƀ~���[�e�b�N�X����ǉ�
//2006.04.10 ryoji  �R���g���[���v���Z�X�����������������C�x���g�t���O����ǉ�
//2007.09.05 kobake ANSI�łƏՓ˂�����邽�߁A���O�ύX
//2007.09.20 kobake ANSI�ł�UNICODE�łŕʂ̖��O��p����

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
//                      �~���[�e�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_MUTEX_SAKURA					(_T("MutexSakuraEditor")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//�C�x���g
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �E�B���h�E�N���X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�R���g���[���g���C
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//���C���E�B���h�E
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//�r���[
#define	GSTR_VIEWNAME		_T("EditorClient")


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���L������                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_CSHAREDATA	(_T("CShareData") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


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
