/*!	@file
	@brief Dialog Box�̊��N���X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "dlg/CDialog.h"
#include "debug/Debug.h"/// 2002/2/10 aroka �w�b�_����
#include "CEditApp.h"
#include "util/os.h"
#include "util/shell.h"

/* �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK MyDialogProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
)
{
	CDialog* pCDialog;
	switch( uMsg ){
	case WM_INITDIALOG:
		pCDialog = ( CDialog* )lParam;
		if( NULL != pCDialog ){
			return pCDialog->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCDialog = ( CDialog* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCDialog ){
			return pCDialog->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}


/*!	�R���X�g���N�^

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
CDialog::CDialog()
{
//	MYTRACE_A( "CDialog::CDialog()\n" );
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hWnd  = NULL;			/* ���̃_�C�A���O�̃n���h�� */
	m_hwndSizeBox = NULL;
	m_lParam = NULL;
	m_nShowCmd = SW_SHOW;
	m_xPos = -1;
	m_yPos = -1;
	m_nWidth = -1;
	m_nHeight = -1;

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	m_szHelpFile = CEditApp::Instance()->GetHelpFilePath();

	return;

}
CDialog::~CDialog()
{
//	MYTRACE_A( "CDialog::~CDialog()\n" );
	CloseDialog( 0 );
	return;
}

//! ���[�_���_�C�A���O�̕\��
/*!
	@param hInstance [in] �A�v���P�[�V�����C���X�^���X�̃n���h��
	@param hwndParent [in] �I�[�i�[�E�B���h�E�̃n���h��
*/
INT_PTR CDialog::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam )
{
	m_bInited = FALSE;
	m_bModal = TRUE;
	m_hInstance = hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_lParam = lParam;
	return ::DialogBoxParam(
		m_hInstance,
		MAKEINTRESOURCE( nDlgTemplete ),
		m_hwndParent,
		(DLGPROC)MyDialogProc,
		(LPARAM)this
	);
}

//! ���[�h���X�_�C�A���O�̕\��
/*!
	@param hInstance [in] �A�v���P�[�V�����C���X�^���X�̃n���h��
	@param hwndParent [in] �I�[�i�[�E�B���h�E�̃n���h��
*/
HWND CDialog::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam, int nCmdShow )
{
	m_bInited = FALSE;
	m_bModal = FALSE;
	m_hInstance = hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_lParam = lParam;
	m_hWnd = ::CreateDialogParam(
		m_hInstance,
		MAKEINTRESOURCE( nDlgTemplete ),
		m_hwndParent,
		(DLGPROC)MyDialogProc,
		(LPARAM)this
	);
	if( NULL != m_hWnd ){
		::ShowWindow( m_hWnd, nCmdShow );
	}
	return m_hWnd;
}

void CDialog::CloseDialog( int nModalRetVal )
{
	if( NULL != m_hWnd ){
		if( m_bModal ){
			::EndDialog( m_hWnd, nModalRetVal );
		}else{
			::DestroyWindow( m_hWnd );
		}
		m_hWnd = NULL;
	}
	return;
}



BOOL CDialog::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;
	// Modified by KEITA for WIN64 2003.9.6
	::SetWindowLongPtr( m_hWnd, DWLP_USER, lParam );
	/* �_�C�A���O�f�[�^�̐ݒ� */
	SetData();

#if 0
	/* �_�C�A���O�̃T�C�Y�A�ʒu�̍Č� */
	if( -1 != m_xPos && -1 != m_yPos ){
		::SetWindowPos( m_hWnd, NULL, m_xPos, m_yPos, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
#ifdef _DEBUG
		MYTRACE_A( "CDialog::OnInitDialog() m_xPos=%d m_yPos=%d\n", m_xPos, m_yPos );
#endif
	}
	if( -1 != m_nWidth && -1 != m_nHeight ){
		::SetWindowPos( m_hWnd, NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
	}
#endif

	if( -1 != m_xPos && -1 != m_yPos ){
		/* �E�B���h�E�ʒu�E�T�C�Y���Č� */

		// 2006.06.09 ryoji
		// ���j�^�̃��[�N�̈�������E�㉺�ɂP�h�b�g�������̈���ɑS�̂����܂�悤�Ɉʒu��������
		//
		// note: �_�C�A���O�����[�N�̈拫�E�ɂ҂����荇�킹�悤�Ƃ���ƁA
		//       �����I�ɐe�̒����Ɉړ��������Ă��܂��Ƃ�������
		//      �i�}���`���j�^���Őe����v���C�}�����j�^�ɂ���ꍇ�����H�j
		//       �󋵂ɍ��킹�ď�����ς���͖̂��Ȃ̂ŁA�ꗥ�A�P�h�b�g�̋󂫂�����

		RECT rc;
		RECT rcWork;
		rc.left = m_xPos;
		rc.top = m_yPos;
		rc.right = m_xPos + m_nWidth;
		rc.bottom = m_yPos + m_nHeight;
		GetMonitorWorkRect(&rc, &rcWork);
		rcWork.top += 1;
		rcWork.bottom -= 1;
		rcWork.left += 1;
		rcWork.right -= 1;
		if( rc.bottom > rcWork.bottom ){
			rc.top -= (rc.bottom - rcWork.bottom);
			rc.bottom = rcWork.bottom;
		}
		if( rc.right > rcWork.right ){
			rc.left -= (rc.right - rcWork.right);
			rc.right = rcWork.right;
		}
		if( rc.top < rcWork.top ){
			rc.bottom += (rcWork.top - rc.top);
			rc.top = rcWork.top;
		}
		if( rc.left < rcWork.left ){
			rc.right += (rcWork.left - rc.left);
			rc.left = rcWork.left;
		}
		m_xPos = rc.left;
		m_yPos = rc.top;
		m_nWidth = rc.right - rc.left;
		m_nHeight = rc.bottom - rc.top;

		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( cWindowPlacement );
		cWindowPlacement.showCmd = m_nShowCmd;	//	�ő剻�E�ŏ���
		cWindowPlacement.rcNormalPosition.left = m_xPos;
		cWindowPlacement.rcNormalPosition.top = m_yPos;
		cWindowPlacement.rcNormalPosition.right = m_nWidth + m_xPos;
		cWindowPlacement.rcNormalPosition.bottom = m_nHeight + m_yPos;
		::SetWindowPlacement( m_hWnd, &cWindowPlacement );
	}
	m_bInited = TRUE;
	return TRUE;
}

BOOL CDialog::OnDestroy( void )
{
	/* �E�B���h�E�ʒu�E�T�C�Y���L�� */
	WINDOWPLACEMENT cWindowPlacement;
	cWindowPlacement.length = sizeof( cWindowPlacement );
	if (::GetWindowPlacement( m_hWnd, &cWindowPlacement )){
		m_nShowCmd = cWindowPlacement.showCmd;	//	�ő剻�E�ŏ���
		m_xPos = cWindowPlacement.rcNormalPosition.left;
		m_yPos = cWindowPlacement.rcNormalPosition.top;
		m_nWidth = cWindowPlacement.rcNormalPosition.right - cWindowPlacement.rcNormalPosition.left;
		m_nHeight = cWindowPlacement.rcNormalPosition.bottom - cWindowPlacement.rcNormalPosition.top;
	}
	/* �j�� */
	if( NULL != m_hwndSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
	m_hWnd = NULL;
	return TRUE;
}


BOOL CDialog::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	case IDOK:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}


BOOL CDialog::OnSize( WPARAM wParam, LPARAM lParam )
{
	RECT	rc;
	::GetWindowRect( m_hWnd, &rc );

	/* �_�C�A���O�̃T�C�Y�̋L�� */
	m_xPos = rc.left;
	m_yPos = rc.top;
	m_nWidth = rc.right - rc.left;
	m_nHeight = rc.bottom - rc.top;

	/* �T�C�Y�{�b�N�X�̈ړ� */
	if( NULL != m_hwndSizeBox ){
		::GetClientRect( m_hWnd, &rc );
//		::SetWindowPos( m_hwndSizeBox, NULL,
//	Sept. 17, 2000 JEPRO_16thdot �A�C�R����16dot�ڂ��\�������悤�Ɏ��s��ύX����K�v����H
//	Jan. 12, 2001 JEPRO (directed by stonee) 15��16�ɕύX����ƃA�E�g���C����͂̃_�C�A���O�̉E���ɂ���
//	�O���b�v�T�C�Y��`�V��'���ł��Ă��܂�(�ړ�����I)�A�_�C�A���O��傫���ł��Ȃ��Ƃ�����Q����������̂�
//	�ύX���Ȃ����Ƃɂ���(�v����Ɍ���łɖ߂�������)
//			rc.right - rc.left - 15, rc.bottom - rc.top - 15,
//			13, 13,
//			SWP_NOOWNERZORDER | SWP_NOZORDER
//		);

//	Jan. 12, 2001 Stonee (suggested by genta)
//		"13"�Ƃ����Œ�l�ł͂Ȃ��V�X�e������擾�����X�N���[���o�[�T�C�Y���g���悤�ɏC��
		::SetWindowPos( m_hwndSizeBox, NULL,
		rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
		rc.bottom - rc.top - GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
		GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
		GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
		SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		//	SizeBox���e�X�g
		if( wParam == SIZE_MAXIMIZED ){
			::ShowWindow( m_hwndSizeBox, SW_HIDE );
		}else{
			::ShowWindow( m_hwndSizeBox, SW_SHOW );
		}
		::InvalidateRect( m_hwndSizeBox, NULL, TRUE );
	}
	return FALSE;

}

BOOL CDialog::OnMove( WPARAM wParam, LPARAM lParam )
{

	/* �_�C�A���O�̈ʒu�̋L�� */
	if( !m_bInited ){
		return TRUE;
	}
	RECT	rc;
	::GetWindowRect( m_hWnd, &rc );

	/* �_�C�A���O�̃T�C�Y�̋L�� */
	m_xPos = rc.left;
	m_yPos = rc.top;
	m_nWidth = rc.right - rc.left;
	m_nHeight = rc.bottom - rc.top;
#ifdef _DEBUG
		MYTRACE_A( "CDialog::OnMove() m_xPos=%d m_yPos=%d\n", m_xPos, m_yPos );
#endif
	return TRUE;

}



void CDialog::CreateSizeBox( void )
{
	/* �T�C�Y�{�b�N�X */
	m_hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT,								/* no extended styles */
		_T("SCROLLBAR"),										/* scroll bar control class */
		NULL,												/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
		0,													/* horizontal position */
		0,													/* vertical position */
		0,													/* width of the scroll bar */
		0,													/* default height */
		m_hWnd/*hdlg*/, 									/* handle of main window */
		(HMENU) NULL,										/* no menu for a scroll bar */
		m_hInstance,										/* instance owning this window */
		(LPVOID) NULL										/* pointer not needed */
	);
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

}






/* �_�C�A���O�̃��b�Z�[�W���� */
INT_PTR CDialog::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//#ifdef _DEBUG
//	MYTRACE_A( "CDialog::DispatchEvent() uMsg == %xh\n", uMsg );
//#endif
	switch( uMsg ){
	case WM_INITDIALOG:	return OnInitDialog( hwndDlg, wParam, lParam );
	case WM_DESTROY:	return OnDestroy();
	case WM_COMMAND:	return OnCommand( wParam, lParam );
	case WM_NOTIFY:		return OnNotify( wParam, lParam );
	case WM_SIZE:
		m_hWnd = hwndDlg;
		return OnSize( wParam, lParam );
	case WM_MOVE:
		m_hWnd = hwndDlg;
		return OnMove( wParam, lParam );
	case WM_DRAWITEM:	return OnDrawItem( wParam, lParam );
	case WM_TIMER:		return OnTimer( wParam );
	case WM_KEYDOWN:	return OnKeyDown( wParam, lParam );
	case WM_KILLFOCUS:	return OnKillFocus( wParam, lParam );
	case WM_ACTIVATE:	return OnActivate( wParam, lParam );	//@@@ 2003.04.08 MIK
	case WM_VKEYTOITEM:	return OnVKeyToItem( wParam, lParam );
	case WM_CHARTOITEM:	return OnCharToItem( wParam, lParam );
	case WM_HELP:		return OnPopupHelp( wParam, lParam );	//@@@ 2002.01.18 add
	case WM_CONTEXTMENU:return OnContextMenu( wParam, lParam );	//@@@ 2002.01.18 add
	}
	return FALSE;
}

BOOL CDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
	wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
	hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
	switch( wNotifyCode ){

	/* �R���{�{�b�N�X�p���b�Z�[�W */
	case CBN_SELCHANGE:	return OnCbnSelChange( hwndCtl, wID );
	// @@2005.03.31 MIK �^�O�W�����vDialog�Ŏg���̂Œǉ�
	case CBN_EDITCHANGE:	return OnCbnEditChange( hwndCtl, wID );
//	case LBN_SELCHANGE:	return OnLbnSelChange( hwndCtl, wID );
//	case CBN_DROPDOWN:	return OnDbnDropDown( hwndCtl, wID );
//	case CBN_CLOSEUP:	return OnDbnCloseUp( hwndCtl, wID );
	case CBN_SELENDOK:	return OnCbnSelEndOk( hwndCtl, wID );
	case LBN_DBLCLK:	return OnLbnDblclk( wID );

	case EN_CHANGE:		return OnEditChange( hwndCtl, wID );

	/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
	case BN_CLICKED:	return OnBnClicked( wID );
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
BOOL CDialog::OnPopupHelp( WPARAM wPara, LPARAM lParam )
{
	HELPINFO *p = (HELPINFO *)lParam;
	MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)GetHelpIdTable() );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return TRUE;
}

BOOL CDialog::OnContextMenu( WPARAM wPara, LPARAM lParam )
{
	MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)GetHelpIdTable() );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return TRUE;
}

const DWORD p_helpids[] = {
	0, 0
};

LPVOID CDialog::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

BOOL CDialog::OnCbnSelEndOk( HWND hwndCtl, int wID )
{
	//�R���{�{�b�N�X�̃��X�g��\�������܂ܕ������ҏW���AEnter�L�[��
	//�����ƕ����񂪏����錻�ۂ̑΍�B
	//Enter�L�[�������Ă��̊֐��ɓ�������A���X�g���\���ɂ��Ă��܂��B
	::SendMessage( hwndCtl, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0 );
	return FALSE;
}

