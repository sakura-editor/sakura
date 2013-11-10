/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/12/24  �V�K�쐬
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, ������, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "CEditApp.h"
#include "env/DLLSHAREDATA.h"
#include "view/colors/EColorIndexType.h"
#include "util/shell.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���b�Z�[�W����                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

typedef INT_PTR (CPropTypes::*DISPATCH_EVENT_TYPE)(HWND,UINT,WPARAM,LPARAM);

// ���ʃ_�C�A���O�v���V�[�W��
INT_PTR CALLBACK PropTypesCommonProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, DISPATCH_EVENT_TYPE pDispatch)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->*pDispatch)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->*pDispatch)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

// �e��_�C�A���O�v���V�[�W��
typedef	INT_PTR (CPropTypes::*pDispatchPage)( HWND, UINT, WPARAM, LPARAM );
#define GEN_PROPTYPES_CALLBACK(FUNC,CLASS) \
INT_PTR CALLBACK FUNC(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ \
	return PropTypesCommonProc(hwndDlg,uMsg,wParam,lParam,reinterpret_cast<pDispatchPage>(&CLASS::DispatchEvent)); \
}
GEN_PROPTYPES_CALLBACK(PropTypesScreen,		CPropTypesScreen)
GEN_PROPTYPES_CALLBACK(PropTypesWindow,		CPropTypesWindow)
GEN_PROPTYPES_CALLBACK(PropTypesColor,		CPropTypesColor)
GEN_PROPTYPES_CALLBACK(PropTypesSupport,	CPropTypesSupport)
GEN_PROPTYPES_CALLBACK(PropTypesRegex,		CPropTypesRegex)
GEN_PROPTYPES_CALLBACK(PropTypesKeyHelp,	CPropTypesKeyHelp)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CPropTypes::CPropTypes()
{
	{
		assert( sizeof(CPropTypesScreen)  - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesWindow)  - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesColor)   - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesSupport) - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesRegex)   - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesKeyHelp) - sizeof(CPropTypes) == 0 );
	}

	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX
	m_pCKeyWordSetMgr = &(GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr);

	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hwndThis  = NULL;		/* ���̃_�C�A���O�̃n���h�� */
	m_nPageNum = 0;

	// 2005.11.30 Moca �J�X�^���F��ݒ�E�ێ�
	int i;
	for( i = 0; i < _countof(m_dwCustColors); i++ ){
		m_dwCustColors[i] = RGB( 255, 255, 255 );
	}

	((CPropTypesScreen*)(this))->CPropTypes_Screen();
}

CPropTypes::~CPropTypes()
{
}

/* ������ */
void CPropTypes::Create( HINSTANCE hInstApp, HWND hwndParent )
{
	m_hInstance = hInstApp;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
}

struct TypePropSheetInfo {
	const TCHAR* szTabname;									//!< TAB�̕\����
	unsigned int resId;										//!< Property sheet�ɑΉ�����Dialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};

// �L�[���[�h�F�^�C�v�ʐݒ�^�u����(�v���p�e�B�V�[�g)
/* �v���p�e�B�V�[�g�̍쐬 */
INT_PTR CPropTypes::DoPropertySheet( int nPageNum )
{
	INT_PTR				nRet;
	int					nIdx;

	// 2001/06/14 Start by asa-o: �^�C�v�ʐݒ�Ɏx���^�u�ǉ�
	// 2001.11.17 add start MIK �^�C�v�ʐݒ�ɐ��K�\���L�[���[�h�^�u�ǉ�
	// 2006.04.10 fon ADD-start �^�C�v�ʐݒ�Ɂu�L�[���[�h�w���v�v�^�u��ǉ�
	// 2013.03.10 aroka ADD-start �^�C�v�ʐݒ�Ɂu�E�B���h�E�v�^�u��ǉ�
	static const TypePropSheetInfo TypePropSheetInfoList[] = {
		{ _T("�X�N���[��"), 		IDD_PROP_SCREEN,	PropTypesScreen },
		{ _T("�J���["), 			IDD_PROP_COLOR,		PropTypesColor },
		{ _T("�E�B���h�E"), 		IDD_PROP_WINDOW,	PropTypesWindow },
		{ _T("�x��"), 				IDD_PROP_SUPPORT,	PropTypesSupport },
		{ _T("���K�\���L�[���[�h"),	IDD_PROP_REGEX,		PropTypesRegex },
		{ _T("�L�[���[�h�w���v"),	IDD_PROP_KEYHELP,	PropTypesKeyHelp }
	};

	// 2005.11.30 Moca �J�X�^���F�̐擪�Ƀe�L�X�g�F��ݒ肵�Ă���
	m_dwCustColors[0] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT;
	m_dwCustColors[1] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK;

	PROPSHEETPAGE		psp[_countof(TypePropSheetInfoList)];
	for( nIdx = 0; nIdx < _countof(TypePropSheetInfoList); nIdx++ ){
		assert( TypePropSheetInfoList[nIdx].szTabname != NULL );

		PROPSHEETPAGE *p = &psp[nIdx];
		memset_raw( p, 0, sizeof_raw( *p ) );
		p->dwSize      = sizeof_raw( *p );
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = m_hInstance;
		p->pszTemplate = MAKEINTRESOURCE( TypePropSheetInfoList[nIdx].resId );
		p->pszIcon     = NULL;
		p->pfnDlgProc  = TypePropSheetInfoList[nIdx].DProc;
		p->pszTitle    = TypePropSheetInfoList[nIdx].szTabname;
		p->lParam      = (LPARAM)this;
		p->pfnCallback = NULL;
	}

	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );

	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 �^�C�v�ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = m_hInstance;
	psh.pszIcon    = NULL;
	psh.pszCaption = _T("�^�C�v�ʐݒ�");	// Sept. 8, 2000 jepro �P�Ȃ�u�ݒ�v����ύX
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage �� unsigned �Ȃ̂ŕ��ɂȂ�Ȃ�
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}
	else if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}
	else{
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
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // �f�t�H���g����
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			_T("CPropTypes::DoPropertySheet()���ŃG���[���o�܂����B\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()���s�B\n")
			_T("\n")
			_T("%ts\n"),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �w���v */
//2001.05.18 Stonee �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
//2001.07.03 JEPRO  �x���^�u�̃w���v��L����
//2001.11.17 MIK    IDD_PROP_REGEX
void CPropTypes::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP_SCREEN:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);			break;
	case IDD_PROP_COLOR:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);			break;
	case IDD_PROP_WINDOW:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_WINDOW);			break;
	case IDD_PROP_SUPPORT:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	}
}
