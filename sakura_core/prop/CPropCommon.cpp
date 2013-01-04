/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�S�ʁv�y�[�W

	@author Norio Nakatani
	@date 1998/12/24 �V�K�쐬
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, hor, Stonee, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, MIK, Moca, ������
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "CEditApp.h"
#include "util/shell.h"
#include "sakura_rc.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
int	CPropCommon::SearchIntArr( int nKey, int* pnArr, int nArrNum )
{
	int i;
	for( i = 0; i < nArrNum; ++i ){
		if( nKey == pnArr[i] ){
			return i;
		}
	}
	return -1;
}


/*!
	�v���p�e�B�y�[�W���Ƃ�Window Procedure�������Ɏ�邱�Ƃ�
	�����̋��ʉ���_�����D

	@param DispatchPage �^��Window Procedure�̃����o�֐��|�C���^
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CPropCommon::DlgProc(
	INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE*	pPsp;
	CPropCommon*	pCPropCommon;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropCommon = ( CPropCommon* )(pPsp->lParam);
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropCommon = ( CPropCommon* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}
//	To Here Jun. 2, 2001 genta

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CPropCommon::CPropCommon()
{
//	int		i;
//	long	lPathLen;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hwndThis  = NULL;		/* ���̃_�C�A���O�̃n���h�� */
	m_nPageNum = ID_PAGENUM_GENERAL;

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	m_pszHelpFile = CEditApp::getInstance()->GetHelpFilePath();

	return;
}





CPropCommon::~CPropCommon()
{
}





/* ������ */
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
void CPropCommon::Create( HWND hwndParent, CImageListMgr* cIcons, CSMacroMgr* pMacro, CMenuDrawer* pMenuDrawer )
{
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_pcIcons = cIcons;
	m_pcSMacro = pMacro;

	// 2007.11.02 ryoji �}�N���ݒ��ύX�������ƁA��ʂ���Ȃ��ŃJ�X�^�����j���[�A�c�[���o�[�A
	//                  �L�[���蓖�Ẳ�ʂɐ؂�ւ������Ɋe��ʂŃ}�N���ݒ�̕ύX�����f�����悤�A
	//                  m_Common.m_sMacro.m_MacroTable�i���[�J�������o�j��m_cLookup������������
	m_cLookup.Init( m_Common.m_sMacro.m_MacroTable, &m_Common );	//	�@�\���E�ԍ�resolve�N���X�D

//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	m_pcMenuDrawer = pMenuDrawer;

	return;
}






//	From Here Jun. 2, 2001 genta
/*!
	�u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ���
	�ێ�����\����
*/
struct ComPropSheetInfo {
	const TCHAR* szTabname;									//!< TAB�̕\����
	unsigned int resId;										//!< Property sheet�ɑΉ�����Dialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	�L�[���[�h�F���ʐݒ�^�u����(�v���p�e�B�V�[�g)
/*! �v���p�e�B�V�[�g�̍쐬
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
int CPropCommon::DoPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int					nRet;
	int					nIdx;
	int					i;

	//	From Here Jun. 2, 2001 genta
	//	Feb. 11, 2007 genta URL��TAB�Ɠ��ꊷ��	// 2007.02.13 �����ύX�iTAB��WIN�̎��Ɂj
	//!	�u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ��̔z��D
	//	�����ύX Win,Toolbar,Tab,Statusbar�̏��ɁAFile,FileName ����	2008/6/22 Uchi 
	//	DProc�̕ύX	2010/5/9 Uchi
	static ComPropSheetInfo ComPropSheetInfoList[] = {
		{ _T("�S��"), 				IDD_PROP_GENERAL,	CPropGeneral::DlgProc_page },
		{ _T("�E�B���h�E"),			IDD_PROP_WIN,		CPropWin::DlgProc_page },
		{ _T("���C�����j���["),		IDD_PROP_MAINMENU,	CPropMainMenu::DlgProc_page },	// 2010/5/8 Uchi
		{ _T("�c�[���o�["),			IDD_PROP_TOOLBAR,	CPropToolbar::DlgProc_page },
		{ _T("�^�u�o�["),			IDD_PROP_TAB,		CPropTab::DlgProc_page },
		{ _T("�X�e�[�^�X�o�["),		IDD_PROP_STATUSBAR,	CPropStatusbar::DlgProc_page},	// �����R�[�h�\���w��	2008/6/21	Uchi
		{ _T("�ҏW"),				IDD_PROP_EDIT,		CPropEdit::DlgProc_page },
		{ _T("�t�@�C��"),			IDD_PROP_FILE,		CPropFile::DlgProc_page },
		{ _T("�t�@�C�����\��"),		IDD_PROP_FNAME,		CPropFileName::DlgProc_page},
		{ _T("�o�b�N�A�b�v"),		IDD_PROP_BACKUP,	CPropBackup::DlgProc_page },
		{ _T("����"),				IDD_PROP_FORMAT,	CPropFormat::DlgProc_page },
		{ _T("����"),				IDD_PROP_GREP,		CPropGrep::DlgProc_page },	// 2006.08.23 ryoji �^�C�g���ύX�iGrep -> �����j
		{ _T("�L�[���蓖��"),		IDD_PROP_KEYBIND,	CPropKeybind::DlgProc_page },
		{ _T("�J�X�^�����j���["),	IDD_PROP_CUSTMENU,	CPropCustmenu::DlgProc_page },
		{ _T("�����L�[���[�h"),		IDD_PROP_KEYWORD,	CPropKeyword::DlgProc_page },
		{ _T("�x��"),				IDD_PROP_HELPER,	CPropHelper::DlgProc_page },
		{ _T("�}�N��"),				IDD_PROP_MACRO,		CPropMacro::DlgProc_page },
		{ _T("�v���O�C��"),			IDD_PROP_PLUGIN,	CPropPlugin::DlgProc_page },
	};

	PROPSHEETPAGE		psp[32];
	for( nIdx = 0, i = 0; i < _countof(ComPropSheetInfoList) && nIdx < 32 ; i++ ){
		if( ComPropSheetInfoList[i].szTabname != NULL ){
			PROPSHEETPAGE *p = &psp[nIdx];
			memset_raw( p, 0, sizeof_raw( *p ) );
			p->dwSize      = sizeof_raw( *p );
			p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
			p->hInstance   = G_AppInstance();
			p->pszTemplate = MAKEINTRESOURCE( ComPropSheetInfoList[i].resId );
			p->pszIcon     = NULL;
			p->pfnDlgProc  = ComPropSheetInfoList[i].DProc;
			p->pszTitle    = ComPropSheetInfoList[i].szTabname;
			p->lParam      = (LPARAM)this;
			p->pfnCallback = NULL;
			nIdx++;
		}
	}
	//	To Here Jun. 2, 2001 genta

	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );
	
	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	JEPROtest Sept. 30, 2000 ���ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = G_AppInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = _T("���ʐݒ�");
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage �� unsigned �Ȃ̂ŕ��ɂȂ�Ȃ�
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}else
	if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}else{
		psh.nStartPage = nPageNum;
	}
	if( psh.nPages - 1 < psh.nStartPage ){
		psh.nStartPage = psh.nPages - 1;
	}

	psh.ppsp = psp;
	psh.pfnCallback = NULL;

	nRet = MyPropertySheet( &psh );	// 2007.05.24 ryoji �Ǝ��g���v���p�e�B�V�[�g
	if( -1 == nRet ){
		TCHAR*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// �f�t�H���g����
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			_T("CPropCommon::DoPropertySheet()���ŃG���[���o�܂����B\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()���s\n")
			_T("\n")
			_T("%ts\n"),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}

/*!	ShareData����ꎞ�̈�֐ݒ���R�s�[����
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheet����ړ�
*/
void CPropCommon::InitData( void )
{
	m_Common = m_pShareData->m_Common;

	//2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			m_Types_nKeyWordSetIdx[i][j] = CTypeConfig(i)->m_nKeyWordSetIdx[j];
		}
	}
}

/*!	ShareData �� �ݒ��K�p�E�R�s�[����
	@note ShareData�ɃR�s�[���邾���Ȃ̂ŁC�X�V�v���Ȃǂ́C���p���鑤�ŏ������Ă��炤
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheet����ړ�
*/
void CPropCommon::ApplyData( void )
{
	m_pShareData->m_Common = m_Common;

	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		//2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			CTypeConfig(i)->m_nKeyWordSetIdx[j] = m_Types_nKeyWordSetIdx[i][j];
		}
	}
}



/* �w���v */
//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
void CPropCommon::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP_GENERAL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//	�ύX�͏�����̍s(Sept. 9, 2000)�ōs���Ă���
//	case IDD_PROP1P5:
//		nContextID = 84;
//		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	// From Here Sept. 9, 2000 JEPRO ���ʐݒ�̃w���v�{�^���������Ȃ��Ȃ��Ă����������ȉ��̒ǉ��ɂ���ďC��
	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_TAB:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TAB);
		break;
	case IDD_PROP_STATUSBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_STATUSBAR);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	// To Here Sept. 9, 2000
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;
	case IDD_PROP_FNAME:	// 2002.12.09 Moca FNAME�ǉ�
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FNAME);
		break;
	case IDD_PROP_PLUGIN:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_PLUGIN);
		break;
	case IDD_PROP_MAINMENU:	//@@@ 2010/6/2 Uchi
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MAINMENU);
		break;

	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, m_pszHelpFile, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	}
	return;
}
