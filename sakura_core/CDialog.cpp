//	$Id$
/************************************************************************

	CDialog.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
#include "CDialog.h"
#include "etc_uty.h"

/* �_�C�A���O�v���V�[�W�� */
BOOL CALLBACK MyDialogProc(
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
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
		pCDialog = ( CDialog* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCDialog ){
			return pCDialog->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}



CDialog::CDialog()
{
//	MYTRACE( "CDialog::CDialog()\n" );
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;		/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hWnd  = NULL;		/* ���̃_�C�A���O�̃n���h�� */
	m_hwndSizeBox = NULL;
	m_lParam = NULL;
	m_xPos = -1;
	m_yPos = -1;		
	m_nWidth = -1;
	m_nHeight = -1;

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( m_szHelpFile );

	return;

}
CDialog::~CDialog()
{
//	MYTRACE( "CDialog::~CDialog()\n" );
	CloseDialog( 0 );
	return;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDialog::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam )
{
	m_bInited = FALSE;
	m_bModal = TRUE;
	m_hInstance = hInstance;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
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

/* ���[�h���X�_�C�A���O�̕\�� */
HWND CDialog::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam, int nCmdShow  )
{
	m_bInited = FALSE;
	m_bModal = FALSE;
	m_hInstance = hInstance;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
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
		::ShowWindow( m_hWnd, nCmdShow  );
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
	::SetWindowLong( m_hWnd, DWL_USER, (LONG)lParam );
	/* �_�C�A���O�f�[�^�̐ݒ� */
	SetData();

	/* �_�C�A���O�̃T�C�Y�A�ʒu�̍Č� */
	if( -1 != m_xPos && -1 != m_yPos ){
		::SetWindowPos( m_hWnd, NULL, m_xPos, m_yPos, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
#ifdef _DEBUG
		MYTRACE( "CDialog::OnInitDialog() m_xPos=%d m_yPos=%d\n", m_xPos, m_yPos );
#endif
	}
	if( -1 != m_nWidth && -1 != m_nHeight ){
		::SetWindowPos( m_hWnd, NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
	}
	m_bInited = TRUE;
	return TRUE;
}

BOOL CDialog::OnDestroy( void )
{
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
	m_nWidth = rc.right - rc.left;
	m_nHeight = rc.bottom - rc.top;

	/* �T�C�Y�{�b�N�X�̈ړ� */
	if( NULL != m_hwndSizeBox ){
		::GetClientRect( m_hWnd, &rc );
		::SetWindowPos( m_hwndSizeBox, NULL, 
//	Sept. 17,2000 JEPRO_16thdot �A�C�R����16dot�ڂ��\�������悤�Ɏ��s��ύX����K�v����H
//	�Ƃ肠�������̂܂܂Ŗ��Ȃ������Ȃ̂ŕύX���Ă��Ȃ�
//	Sept. 28, 2000 JEPRO_16thdot ��͂�O�̂��߂������ύX���Ă݂�
//			rc.right - rc.left - 15, rc.bottom - rc.top - 15, 
//			13, 13, 
//			SWP_NOOWNERZORDER | SWP_NOZORDER 
//		);
			rc.right - rc.left - 16, rc.bottom - rc.top - 16, 
			13, 13, 
			SWP_NOOWNERZORDER | SWP_NOZORDER 
		);

		::InvalidateRect( m_hwndSizeBox, NULL, TRUE );
	}
	return FALSE;

}

BOOL CDialog::OnMove( WPARAM wParam, LPARAM lParam )
{
	
	/* �_�C�A���O�̈ʒu�̋L�� */
	if( FALSE == m_bInited ){
		return TRUE;
	}
//	WINDOWPLACEMENT	wpl;
	RECT	rc;
//	wpl.length = sizeof( WINDOWPLACEMENT );
//	::GetWindowPlacement( m_hWnd, &wpl );
//	m_xPos = wpl.rcNormalPosition.left;
//	m_yPos = wpl.rcNormalPosition.top;
//#ifdef _DEBUG
//		MYTRACE( "CDialog::OnMove() m_xPos=%d m_yPos=%d\n", m_xPos, m_yPos );
//#endif
	::GetWindowRect( m_hWnd, &rc ); 
	m_xPos = rc.left;
	m_yPos = rc.top;
#ifdef _DEBUG
		MYTRACE( "CDialog::OnMove() m_xPos=%d m_yPos=%d\n", m_xPos, m_yPos );
#endif

//	m_xPos = (int)(short) LOWORD(lParam);    // horizontal position 
//	m_yPos = (int)(short) HIWORD(lParam);    // vertical position 
	return TRUE;

}



void CDialog::CreateSizeBox( void )
{
	/* �T�C�Y�{�b�N�X */
	m_hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT, 						/* no extended styles			*/
		"SCROLLBAR",				/* scroll bar control class		*/
		(LPSTR) NULL,				/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
		0,							/* horizontal position			*/
		0,							/* vertical position			*/
		0,						/* width of the scroll bar		*/
		0,				/* default height				*/
		m_hWnd/*hdlg*/, 				/* handle of main window		*/
		(HMENU) NULL,				/* no menu for a scroll bar 	*/
		m_hInstance,				/* instance owning this window	*/
		(LPVOID) NULL			/* pointer not needed				*/
	);
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

}






/* �_�C�A���O�̃��b�Z�[�W���� */
BOOL CDialog::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//#ifdef _DEBUG
//	MYTRACE( "CDialog::DispatchEvent() uMsg == %xh\n", uMsg );
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
	case WM_VKEYTOITEM:	return OnVKeyToItem( wParam, lParam );
	case WM_CHARTOITEM:	return OnCharToItem( wParam, lParam );
//	case WM_NEXTDLGCTL:	return OnNextDlgCtl( wParam, lParam );
	}
	return FALSE;
}

BOOL CDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
	wID         = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
	hwndCtl     = (HWND) lParam;	/* �R���g���[���̃n���h�� */
	switch( wNotifyCode ){

	/* �R���{�{�b�N�X�p���b�Z�[�W */
	case CBN_SELCHANGE:	return OnCbnSelChange( hwndCtl, wID );
	case LBN_DBLCLK:	return OnLbnDblclk( wID );



	/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
	case BN_CLICKED:	return OnBnClicked( wID );
	}
	return FALSE;
}

