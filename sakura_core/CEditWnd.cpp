//	$Id$
/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, mik, hor
	Copyright (C) 2002, YAZAKI, genta, aroka, MIK
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <io.h>
#include <mbctype.h>
#include <mbstring.h>

#include "CEditApp.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "CEditDoc.h"
#include "debug.h"
#include "CDlgAbout.h"
#include "mymessage.h"
#include "CShareData.h"
#include "CPrint.h"
#include "etc_uty.h"
#include "charcode.h"
#include "global.h"
#include "CDlgPrintSetting.h"
#include "CDlgPrintPage.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "CSMacroMgr.h" // Jun. 16, 2002 genta
#include "CRunningTimer.h"


#define IDT_TOOLBAR		456
#define ID_TOOLBAR		100


//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ��������̂�
//	��`���폜

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif

#ifndef	WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif

#define		YOHAKU_X		4		/* �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l */
#define		YOHAKU_Y		4		/* �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ��������̂�
//	��`���폜

//	/* ���b�Z�[�W���[�v */
//	DWORD MessageLoop_Thread( DWORD pCEditWndObject );

LRESULT CALLBACK CEditWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CEditWnd* pSEWnd;
	pSEWnd = ( CEditWnd* )::GetWindowLong( hwnd, GWL_USERDATA );
	if( NULL != pSEWnd ){
		return pSEWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}


/*
||  �^�C�}�[���b�Z�[�W�̃R�[���o�b�N�֐�
||
||	�c�[���o�[�̏�ԍX�V�̂��߂Ƀ^�C�}�[���g�p���Ă��܂�
*/
VOID CALLBACK CEditWndTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditWnd*	pCEdit;
	pCEdit = ( CEditWnd* )::GetWindowLong( hwnd, GWL_USERDATA );
	if( NULL != pCEdit ){
		pCEdit->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}

//by �S(2)
void CALLBACK SysMenuTimer(HWND Wnd, UINT Msg, UINT Event, DWORD Time)
{
	CEditWnd *WndObj;
	WndObj = (CEditWnd*)GetWindowLong(Wnd, GWL_USERDATA);
	if(WndObj != NULL)
		WndObj->OnSysMenuTimer();

	KillTimer(Wnd, Event);
}

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditWnd::CEditWnd() :
	m_hWnd( NULL ),
	m_bDragMode( FALSE ),
	m_hwndParent( NULL ),
	m_hwndToolBar( NULL ),
	m_hwndStatusBar( NULL ),
	m_hwndProgressBar( NULL ),
	m_hdcCompatDC( NULL ),			/* �ĕ`��p�R���p�`�u���c�b */
	m_hbmpCompatBMP( NULL ),		/* �ĕ`��p�������a�l�o */
	m_hbmpCompatBMPOld( NULL ),		/* �ĕ`��p�������a�l�o(OLD) */
// 20020331 aroka �ĕϊ��Ή� for 95/NT
	m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ),
	m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ),

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	m_pPrintPreview( NULL ),
	m_pszAppName( GSTR_EDITWINDOWNAME ),
	m_hwndSearchBox( NULL ),
	m_fontSearchBox( NULL ),
	m_nCurrentFocus( 0 ),
	m_IconClicked(icNone) //by �S(2)
{
	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	// Sep. 10, 2002 genta
	m_cEditDoc.m_pcEditWnd = this;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

//	MYTRACE( "CEditWnd::CEditWnd()�����\n" );
	return;
}




CEditWnd::~CEditWnd()
{
	//	Dec. 4, 2002 genta
	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	::DeleteObject( m_hFontCaretPosInfo );

	/* �ĕ`��p�������a�l�o */
	if( m_hbmpCompatBMP != NULL ){
		/* �ĕ`��p�������a�l�o(OLD) */
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
	}
	/* �ĕ`��p�R���p�`�u���c�b */
	if( m_hdcCompatDC != NULL ){
		::DeleteDC( m_hdcCompatDC );
	}

	if( NULL != m_hWnd ){
		m_hWnd = NULL;
	}
	return;
}





/*!	�쐬

	@param hInstance [in] Instance Handle
	@param hwndParent [in] �e�E�B���h�E�̃n���h��
	@param pszPath [in] �ŏ��ɊJ���t�@�C���̃p�X�DNULL�̂Ƃ��J���t�@�C�������D
	@param nCharCode [in] �����R�[�h
	@param bReadOnly [in] �ǂ݂Ƃ��p�ŊJ�����ǂ���
	@param nDocumentType [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
	
	@date 2002.03.07 genta nDocumentType�ǉ�
*/
HWND CEditWnd::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	const char*	pszPath,
	int			nCharCode,
	BOOL		bReadOnly,
	int			nDocumentType
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	WNDCLASSEX	wc;
	HWND		hWnd;
	ATOM		atom;
	BOOL		bOpened;
	char szMsg[512];

	if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return NULL;
	}


	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= m_hInstance;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	wc.hIcon			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= m_pszAppName;

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
	if( 0 == ( atom = RegisterClassEx( &wc ) ) ){
//		return NULL;
	}

	/* �E�B���h�E�T�C�Y�p�� */
	int	nWinCX, nWinCY;
	if( m_pShareData->m_Common.m_bSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* �E�B���h�E�T�C�Y�w�� */
	FileInfo fi;
	CCommandLine::Instance()->GetFileInfo(fi);
	if( fi.m_nWindowSizeX >= 0 ){
		nWinCX = fi.m_nWindowSizeX;
	}
	if( fi.m_nWindowSizeY >= 0 ){
		nWinCY = fi.m_nWindowSizeY;
	}

	/* �E�B���h�E�ʒu�w�� */
	int nWinOX, nWinOY;
	nWinOX = CW_USEDEFAULT;
	nWinOY = 0;
	if( fi.m_nWindowOriginX >= 0 ){
		nWinOX = fi.m_nWindowOriginX;
	}
	if( fi.m_nWindowOriginY >= 0 ){
		nWinOY = fi.m_nWindowOriginY;
	}

	hWnd = ::CreateWindowEx(
		0 	// extended window style
//		| WS_EX_CLIENTEDGE
		,
		m_pszAppName,		// pointer to registered class name
		m_pszAppName,		// pointer to window name
//		WS_VISIBLE |
		WS_OVERLAPPEDWINDOW |
		WS_CLIPCHILDREN	|
//		WS_HSCROLL | WS_VSCROLL	|
		0,	// window style

		nWinOX,				// horizontal position of window
		nWinOY,				// vertical position of window
		nWinCX,				// window width
		nWinCY,				// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		m_hInstance,		// handle to application instance
		NULL				// pointer to window-creation data
	);
	m_hWnd = hWnd;

	m_cIcons.Create( m_hInstance, m_hWnd );	//	CreateImage List

	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_cIcons );

	if( NULL != m_hWnd ){
		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );

		/* �ĕ`��p�R���p�`�u���c�b */
		HDC hdc = ::GetDC( m_hWnd );
		m_hdcCompatDC = ::CreateCompatibleDC( hdc );
		::ReleaseDC( m_hWnd, hdc );

	}

	if( m_pShareData->m_Common.m_bDispTOOLBAR ){	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
 		/* �c�[���o�[�쐬 */
		CreateToolBar();
	}

	/* �X�e�[�^�X�o�[ */
	if( m_pShareData->m_Common.m_bDispSTATUSBAR ){	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
		/* �X�e�[�^�X�o�[�쐬 */
		CreateStatusBar();
	}

	/* �t�@���N�V�����L�[ �o�[ */
	if( m_pShareData->m_Common.m_bDispFUNCKEYWND ){	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
		BOOL bSizeBox;
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			bSizeBox = FALSE;
		}else{
			bSizeBox = TRUE;
			/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
		}
		m_CFuncKeyWnd.Open( hInstance, m_hWnd, &m_cEditDoc, bSizeBox );
	}


	if( FALSE == m_cEditDoc.Create( m_hInstance, m_hWnd, &m_cIcons/*, 1, 1, 0, 0*/ ) ){
		::MessageBox(
			m_hWnd,
			"�N���C�A���g�E�B���h�E�̍쐬�Ɏ��s���܂���", GSTR_APPNAME,
			MB_OK
		);
	}
	/* �f�X�N�g�b�v����͂ݏo���Ȃ��悤�ɂ��� */
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;
	::SystemParametersInfo( SPI_GETWORKAREA, NULL, &rcDesktop, 0 );
	::GetWindowRect( m_hWnd, &rcOrg );
	/* �E�B���h�E�ʒu���� */
	if( rcOrg.bottom >= rcDesktop.bottom ){
		if( 0 > rcOrg.top - (rcOrg.bottom - rcDesktop.bottom ) ){
			rcOrg.top = 0;
		}else{
			rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		}
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right >= rcDesktop.right ){
		if( 0 > rcOrg.left - (rcOrg.right - rcDesktop.right ) ){
			rcOrg.left = 0;
		}else{
			rcOrg.left -= rcOrg.right - rcDesktop.right;
		}
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	/* �E�B���h�E�T�C�Y���� */
	if( rcOrg.top < rcDesktop.top ){
		rcOrg.top = rcDesktop.top;
	}
	if( rcOrg.left < rcDesktop.left ){
		rcOrg.left = rcDesktop.left;
	}
	if( rcOrg.bottom >= rcDesktop.bottom ){
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right >= rcDesktop.right ){
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	::SetWindowPos(
		m_hWnd, 0,
		rcOrg.left, rcOrg.top,
		rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);

	/* �E�B���h�E�T�C�Y�p�� */
	if( m_pShareData->m_Common.m_bSaveWindowSize &&
		m_pShareData->m_Common.m_nWinSizeType == SIZE_MAXIMIZED ){
		::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
	}else{
		::ShowWindow( m_hWnd, SW_SHOW );
	}
	if( NULL != m_hWnd ){
		/* �h���b�v���ꂽ�t�@�C�����󂯓���� */
		::DragAcceptFiles( m_hWnd, TRUE );
		/* �ҏW�E�B���h�E���X�g�ւ̓o�^ */
		if( FALSE == CShareData::getInstance()->AddEditWndList( m_hWnd ) ){
			wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
			::MessageBox( m_hWnd, szMsg, GSTR_APPNAME, MB_OK );
			::DestroyWindow( m_hWnd );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
		/* �^�C�}�[���N�� */
		if( 0 == ::SetTimer( m_hWnd, IDT_TOOLBAR, 300, (TIMERPROC)CEditWndTimerProc ) ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"CEditWnd::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B"
			);
		}
	}
	::InvalidateRect( m_hWnd, NULL, TRUE );
	if( NULL != pszPath ){
		char*	pszPathNew = new char[_MAX_PATH];
		strcpy( pszPathNew, pszPath );
		::ShowWindow( m_hWnd, SW_SHOW );
		if( !m_cEditDoc.FileRead( pszPathNew, &bOpened, nCharCode, bReadOnly, TRUE ) ){
			/* �t�@�C�������ɊJ����Ă��� */
			if( bOpened ){
				::PostMessage( m_hWnd, WM_CLOSE, 0, 0 );
				delete [] pszPathNew;
				return NULL;
			}
			else {
				//	Nov. 20, 2000 genta
				m_cEditDoc.SetImeMode( m_pShareData->m_Types[0].m_nImeState );
			}
		}
		//	Mar. 7, 2002 genta �����^�C�v�̋����w��
		if( nDocumentType >= 0 ){
			m_cEditDoc.SetDocumentType( nDocumentType, true );
			//	2002/05/07 YAZAKI �^�C�v�ʐݒ�ꗗ�̈ꎞ�K�p�̃R�[�h�𗬗p
			m_cEditDoc.LockDocumentType();
			/* �ݒ�ύX�𔽉f������ */
			m_cEditDoc.OnChangeSetting();
		}
		delete [] pszPathNew;
	}
	else {
		//	Nov. 20, 2000 genta
		m_cEditDoc.SetImeMode( m_pShareData->m_Types[0].m_nImeState );
	}
	//	YAZAKI 2002/05/30 IME�E�B���h�E�̈ʒu�����������̂��C���B
	m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].SetIMECompFormPos();
	return m_hWnd;
}


//	�L�[���[�h�F�X�e�[�^�X�o�[����
/* �X�e�[�^�X�o�[�쐬 */
void CEditWnd::CreateStatusBar( void )
{
//	int		nStArr[] = { 300, 400, 500. -1 };
//	int		nStArrNum = sizeof( nStArr ) / sizeof( nStArr[0] );

	/* �X�e�[�^�X�o�[ */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | WS_EX_RIGHT | SBARS_SIZEGRIP,
		"",
		m_hWnd,
		IDW_STATUSBAR
	);

	/* �v���O���X�o�[ */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		(LPSTR) NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		m_hInstance,
		0
	);
//	::ShowWindow( m_hwndProgressBar, SW_SHOW );

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		m_CFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}
	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1);
	return;
}


/* �X�e�[�^�X�o�[�j�� */
void CEditWnd::DestroyStatusBar( void )
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		BOOL bSizeBox;
		if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
			bSizeBox = FALSE;
		}else{
			bSizeBox = TRUE;
			/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = FALSE;
			}
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//�X�v���b�^�[�́A�T�C�Y�{�b�N�X�̈ʒu��ύX
	m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1 );

	return;
}

/* �c�[���o�[�쐬
	@date @@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
void CEditWnd::CreateToolBar( void )
{
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	UINT			uToolType;
	nFlag = 0;

	/* �c�[���o�[�E�B���h�E�̍쐬 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD | WS_VISIBLE | /*WS_BORDER | */
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		m_hWnd,
		(HMENU)ID_TOOLBAR,
		m_hInstance,
		NULL
	);
	if( NULL == m_hwndToolBar ){
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			m_pShareData->m_Common.m_bToolBarIsFlat = FALSE;
		}
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
			"�c�[���o�[�̍쐬�Ɏ��s���܂����B"
		);
	}else{
		::SendMessage( m_hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
		//	Oct. 12, 2000 genta
		//	���ɗp�ӂ���Ă���Image List���A�C�R���Ƃ��ēo�^
		m_cIcons.SetToolBarImages( m_hwndToolBar );
		/* �c�[���o�[�Ƀ{�^����ǉ� */
		int count = 0;	//@@@ 2002.06.15 MIK
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			nIdx = m_pShareData->m_Common.m_nToolBarButtonIdxArr[i];
			tbb = m_CMenuDrawer.m_tbMyButton[m_pShareData->m_Common.m_nToolBarButtonIdxArr[i]];
			//::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//�h���b�v�_�E��
				//�g���X�^�C���ɐݒ�
				::SendMessage( m_hwndToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//�R���{�{�b�N�X
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					//HWND			my_hwnd;
					TBBUTTON		my_tbb;
					//int			width;
					LOGFONT			lf;
					//HFONT			my_font;
					int				my_i;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//�Z�p���[�^���
						memset( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_SEP;
						my_tbb.idCommand = tbb.idCommand;	//����ID�ɂ��Ă���
						::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&my_tbb );
						count++;

						//�T�C�Y��ݒ肷��
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = 160;	//�{�b�N�X�̕�
						::SendMessage( m_hwndToolBar, TB_SETBUTTONINFO, (WPARAM)(tbb.idCommand), (LPARAM)&tbi );

						//�T�C�Y���擾����
						rc.right = rc.left = rc.top = rc.bottom = 0;
						::SendMessage( m_hwndToolBar, TB_GETITEMRECT, (WPARAM)(count-1), (LPARAM)&rc );

						//�R���{�{�b�N�X�����
						//	Mar. 8, 2003 genta �����{�b�N�X��1�h�b�g���ɂ��炵��
						m_hwndSearchBox = CreateWindow( "COMBOBOX", "Combo",
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, m_hInstance, NULL );
						if( m_hwndSearchBox )
						{
							m_nCurrentFocus = 0;

							memset( &lf, 0, sizeof(LOGFONT) );
							lf.lfHeight			= 12; // Jan. 14, 2003 genta �_�C�A���O�ɂ��킹�Ă�����Ə�����
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							lf.lfCharSet		= SHIFTJIS_CHARSET;
							lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
							lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
							lf.lfQuality		= DEFAULT_QUALITY;
							lf.lfPitchAndFamily	= FF_MODERN | DEFAULT_PITCH;
							//strcpy( lf.lfFaceName, "�l�r �S�V�b�N" );
							strcpy( lf.lfFaceName, "�l�r �o�S�V�b�N" );
							m_fontSearchBox = ::CreateFontIndirect( &lf );
							if( m_fontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (UINT_PTR)m_fontSearchBox, MAKELONG (TRUE, 0) );
							}

							//���͒�����
							::SendMessage( m_hwndSearchBox, CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

							//�g���C���^�t�F�[�X
							//::SendMessage( m_hwndSearchBox, CB_SETEXTENDEDUI, (WPARAM)(BOOL)TRUE, 0 );

							//�����{�b�N�X���X�V
							::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
							for( my_i = 0; my_i < m_pShareData->m_nSEARCHKEYArrNum; my_i++ )
							{
								::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[my_i] );
							}
							::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
						}
						break;

					default:
						//width = 0;
						//my_hwnd = NULL;
						//my_font = NULL;
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//�{�^��
			case TBSTYLE_SEP:		//�Z�p���[�^
			default:
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( m_pShareData->m_Common.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			uToolType = (UINT)::GetWindowLong(m_hwndToolBar, GWL_STYLE);
			uToolType |= (TBSTYLE_FLAT);
			::SetWindowLong(m_hwndToolBar, GWL_STYLE, (LONG)uToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
	}
	return;
}

void CEditWnd::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_fontSearchBox )
			{
				::DeleteObject( m_fontSearchBox );
				m_fontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_nCurrentFocus = 0;
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;
	}

	return;
}


//�����v���Z�X��
/* ���b�Z�[�W���[�v */
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	while ( NULL != m_hWnd && GetMessage( &msg, NULL, 0, 0 ) ){
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		if( m_pPrintPreview && NULL != m_pPrintPreview->GetPrintPreviewBarHANDLE() && ::IsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE(), &msg ) ){	/* ����v���r���[ ����o�[ */
		}else
		if( NULL != m_cEditDoc.m_cDlgFind.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFind.m_hWnd, &msg ) ){	/* �u�����v�_�C�A���O */
		}else
		if( NULL != m_cEditDoc.m_cDlgFuncList.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgFuncList.m_hWnd, &msg ) ){	/* �u�A�E�g���C���v�_�C�A���O */
		}else
		if( NULL != m_cEditDoc.m_cDlgReplace.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgReplace.m_hWnd, &msg ) ){	/* �u�u���v�_�C�A���O */
		}else
		if( NULL != m_cEditDoc.m_cDlgGrep.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cDlgGrep.m_hWnd, &msg ) ){	/* �uGrep�v�_�C�A���O */
		}else
		if( NULL != m_cEditDoc.m_cHokanMgr.m_hWnd && ::IsDialogMessage( m_cEditDoc.m_cHokanMgr.m_hWnd, &msg ) ){	/* �u���͕⊮�v */
		}else
		if( NULL != m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, &msg ) ){	/* �u�����{�b�N�X�v */
			ProcSearchBox( &msg );
		}else
		{
			if( NULL != m_pShareData->m_hAccel ){
				if( TranslateAccelerator( msg.hwnd, m_pShareData->m_hAccel, &msg ) ){
				}else{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}else{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	return;
}


LRESULT CEditWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int					nRet;
	int					idCtrl;
	LPNMHDR				pnmh;
	LPTOOLTIPTEXT		lptip;
	int					nPane;
	FileInfo*			pfi;
	WORD				fActive;
	BOOL				fMinimized;
	HWND				hwndTarget;
	HWND				hwndActive;
	BOOL				bIsActive;
	int					nCaretPosX;
	int					nCaretPosY;
	POINT*				ppoCaret;
	LPHELPINFO			lphi;
	const char*			pLine;
	int					nLineLen;

	UINT				idCtl;	/* �R���g���[����ID */
	MEASUREITEMSTRUCT*	lpmis;
	char				szLabel[1024];
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	UINT				fuFlags;
	HMENU				hmenu;
	const char*			pszItemStr;
	LRESULT				lRes;

	switch( uMsg ){
	case WM_PAINTICON:
//		MYTRACE( "WM_PAINTICON\n" );
		return 0;
	case WM_ICONERASEBKGND:
//		MYTRACE( "WM_ICONERASEBKGND\n" );
		return 0;
	case WM_LBUTTONDOWN:
		return OnLButtonDown( wParam, lParam );
	case WM_MOUSEMOVE:
		return OnMouseMove( wParam, lParam );
	case WM_LBUTTONUP:
		return OnLButtonUp( wParam, lParam );
	case WM_MOUSEWHEEL:
		return OnMouseWheel( wParam, lParam );
	case WM_HSCROLL:
		return OnHScroll( wParam, lParam );
	case WM_VSCROLL:
		return OnVScroll( wParam, lParam );


	case WM_MENUCHAR:
		/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );






	case WM_MENUSELECT:
		if( NULL == m_hwndStatusBar ){
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		fuFlags = (UINT) HIWORD(wParam);	// menu flags
		hmenu = (HMENU) lParam;				// handle to menu clicked
		{
			/* ���j���[�@�\�̃e�L�X�g���Z�b�g */
			CMemory		cmemWork;
			pszItemStr = "";

			/* �@�\�ɑΉ�����L�[���̎擾(����) */
			CMemory**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				m_hInstance, m_pShareData->m_nKeyNameArrNum,
				(KEYDATA*)m_pShareData->m_pKeyNameArr, &ppcAssignedKeyList, uItem
			);
			if( 0 < nAssignedKeyNum ){
				for( j = 0; j < nAssignedKeyNum; ++j ){
					if( j > 0 ){
						cmemWork.AppendSz( " , " );
					}
					cmemWork.Append( ppcAssignedKeyList[j] );
					delete ppcAssignedKeyList[j];
				}
				delete [] ppcAssignedKeyList;
			}
			pszItemStr = cmemWork.GetPtr();


			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)pszItemStr );


		}
		return 0;


	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* �R���g���[����ID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		if( IDW_STATUSBAR == idCtl ){
			if( 4 == lpdis->itemID ){
				int	nColor;
				if( m_pShareData->m_bRecordingKeyMacro	/* �L�[�{�[�h�}�N���̋L�^�� */
				 && m_pShareData->m_hwndRecordingKeyMacro == m_hWnd	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				){
					nColor = COLOR_BTNTEXT;
				}else{
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor( lpdis->hDC, ::GetSysColor( nColor ) );
				::SetBkMode( lpdis->hDC, TRANSPARENT );
				::TextOut( lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top + 2, "REC", lstrlen( "REC" ) );
				if( COLOR_BTNTEXT == nColor ){
					::TextOut( lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 2, "REC", lstrlen( "REC" ) );
				}
			}
			return 0;
		}else{
			switch( lpdis->CtlType ){
			case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
				/* ���j���[�A�C�e���`�� */
				m_CMenuDrawer.DrawItem( lpdis );
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
//			CMenuDrawer* pCMenuDrawer;
//			pCMenuDrawer = (CMenuDrawer*)lpmis->itemData;


//			MYTRACE( "WM_MEASUREITEM  lpmis->itemID=%d\n", lpmis->itemID );
			/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( -1 == nItemWidth ){
			}else{
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;




	case WM_PAINT:
		return OnPaint( hwnd, uMsg, wParam, lParam );

	case WM_PASTE:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_PASTE, TRUE, 0, 0, 0, 0 );

	case WM_COPY:
		return m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_COPY, TRUE, 0, 0, 0, 0 );

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			OnHelp_MenuItem( hwnd, lphi->iCtrlId );
			break;
		}
		return TRUE;

		//	Jun. 2, 2000 genta
	case WM_ACTIVATEAPP:
		fActive = LOWORD(wParam);				// activation flag
		if( fActive ){
			::SetFocus( m_hWnd );
		}
		return 0;	//	should return zero. / Jun. 23, 2000 genta
	case WM_ACTIVATE:
		fActive = LOWORD( wParam );				// activation flag
		fMinimized = (BOOL) HIWORD( wParam );	// minimized flag
		hwndTarget = (HWND) lParam;				// window handle

//		MYTRACE( "WM_ACTIVATE " );
 		switch( fActive ){
		case WA_ACTIVE:
//			MYTRACE( " WA_ACTIVE\n" );
			bIsActive = TRUE;
			break;
		case WA_CLICKACTIVE:
//			MYTRACE( " WA_CLICKACTIVE\n" );
			bIsActive = TRUE;
			break;
		case WA_INACTIVE:
//			MYTRACE( " WA_INACTIVE\n" );
			bIsActive = FALSE;
			break;
		}
		if( !bIsActive ){
			hwndActive = hwndTarget;
			while( hwndActive != NULL ){
				hwndActive = ::GetParent( hwndActive );
				if( hwndActive == m_hWnd ){
					bIsActive = TRUE;
					break;
				}
			}
		}
		if( !bIsActive ){
			if( m_hWnd == ::GetWindow( hwndTarget, GW_OWNER ) ){
				bIsActive = TRUE;
			}
		}
		m_cEditDoc.SetParentCaption( !bIsActive );


		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_SIZE:
//		MYTRACE( "WM_SIZE\n" );
		/* WM_SIZE ���� */
		if( SIZE_MINIMIZED == wParam ){
			m_cEditDoc.SetParentCaption();
		}
		return OnSize( wParam, lParam );

	case WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR ) {
			/* ���b�Z�[�W�̔z�� */
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_ENTERMENULOOP:
	case MYWM_IME_REQUEST:   /*  �ĕϊ��Ή� by minfu 2002.03.27  */ // 20020331 aroka
		/* ���b�Z�[�W�̔z�� */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_EXITMENULOOP:
//		MYTRACE( "WM_EXITMENULOOP\n" );
		if( NULL != m_hwndStatusBar ){
			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		}

		/* ���b�Z�[�W�̔z�� */
		return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
	case WM_SETFOCUS:
//		MYTRACE( "WM_SETFOCUS\n" );

		/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
		m_cEditDoc.CheckFileTimeStamp();


		/* �ҏW�E�B���h�E���X�g�ւ̓o�^ */
		CShareData::getInstance()->AddEditWndList( m_hWnd );
		/* ���b�Z�[�W�̔z�� */
		lRes = m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/* ����v���r���[���[�h�̂Ƃ��́A�L�[����͑S��PrintPreviewBar�֓]�� */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//�����{�b�N�X���X�V
		if( m_hwndSearchBox )
		{
			int	i;
			::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
			for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; i++ )
			{
				::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
			}
			::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
		}
		
		return lRes;


	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pnmh = (LPNMHDR) lParam;
		switch( pnmh->code ){
		case TTN_NEEDTEXT:
			lptip = (LPTOOLTIPTEXT)pnmh;
			{
				/* �c�[���o�[�̃c�[���`�b�v�̃e�L�X�g���Z�b�g */
				CMemory**	ppcAssignedKeyList;
				int			nAssignedKeyNum;
				int			j;
				char*		pszKey;

				// From Here Oct. 15, 2001 genta
				// �@�\������̎擾��Lookup���g���悤�ɕύX
				if( !m_cEditDoc.m_cFuncLookup.Funccode2Name( lptip->hdr.idFrom, szLabel, 1024 )){
					szLabel[0] = '\0';
				}
				// To Here Oct. 15, 2001 genta
				/* �@�\�ɑΉ�����L�[���̎擾(����) */
				nAssignedKeyNum = CKeyBind::GetKeyStrList(
					m_hInstance, m_pShareData->m_nKeyNameArrNum,
					(KEYDATA*)m_pShareData->m_pKeyNameArr, &ppcAssignedKeyList, lptip->hdr.idFrom
				);
				if( 0 < nAssignedKeyNum ){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						strcat( szLabel, "\n        " );
						pszKey = ppcAssignedKeyList[j]->GetPtr();
						strcat( szLabel, pszKey );
						delete ppcAssignedKeyList[j];
					}
					delete [] ppcAssignedKeyList;
				}
				lptip->hinst = NULL;
				lptip->lpszText	= szLabel;
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu( pnmh->hwndFrom );
				if( nId != 0 ) OnCommand( (WORD)0 /*���j���[*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		}
		return 0L;
	case WM_COMMAND:
		OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
		return 0L;
	case WM_INITMENUPOPUP:
		InitMenu( (HMENU)wParam, (UINT)LOWORD( lParam ), (BOOL)HIWORD( lParam ) );
		return 0L;
	case WM_DROPFILES:
		/* �t�@�C�����h���b�v���ꂽ */
		OnDropFiles( (HDROP) wParam );
		return 0L;
	case WM_QUERYENDSESSION:
		if( OnClose() ){
			DestroyWindow( hwnd );
			return TRUE;
		}else{
			return FALSE;
		}
	case WM_CLOSE:
		if( OnClose() ){
			DestroyWindow( hwnd );
		}
		return 0L;
	case WM_DESTROY:
		if( m_pShareData->m_bRecordingKeyMacro ){					/* �L�[�{�[�h�}�N���̋L�^�� */
			if( m_pShareData->m_hwndRecordingKeyMacro == m_hWnd ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				m_pShareData->m_bRecordingKeyMacro = FALSE;			/* �L�[�{�[�h�}�N���̋L�^�� */
				m_pShareData->m_hwndRecordingKeyMacro = NULL;		/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
			}
		}

		/* �^�C�}�[���폜 */
		::KillTimer( m_hWnd, IDT_TOOLBAR );

		/* �h���b�v���ꂽ�t�@�C�����󂯓����̂����� */
		::DragAcceptFiles( hwnd, FALSE );
		/* �ҏW�E�B���h�E���X�g����̍폜 */
		CShareData::getInstance()->DeleteEditWndList( m_hWnd );

		if( m_pShareData->m_hwndDebug == m_hWnd ){
			m_pShareData->m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
		::PostMessage( m_pShareData->m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows �ɃX���b�h�̏I����v�����܂� */
		::PostQuitMessage( 0 );

		return 0L;
	case MYWM_CLOSE:
		/* �G�f�B�^�ւ̑S�I���v�� */
		if( FALSE != ( nRet = OnClose()) ){	// Jan. 23, 2002 genta �x���}��
			DestroyWindow( hwnd );
		}
		return nRet;


	case MYWM_GETFILEINFO:
		/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
		pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

		/* �ҏW�t�@�C�������i�[ */
		m_cEditDoc.SetFileInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* �ݒ�ύX�̒ʒm */
// Oct 10, 2000 ao
/* �ݒ�ύX���A�c�[���o�[���č쐬����悤�ɂ��� */
		if( NULL != m_hwndToolBar ){
			DestroyToolBar();
			CreateToolBar();
		}
// Oct 10, 2000 ao �����܂�

		if( NULL != m_CFuncKeyWnd.m_hWnd ){
			BOOL bSizeBox;
			if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
				/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
				bSizeBox = FALSE;
			}else{
				bSizeBox = TRUE;
				/* �X�e�[�^�X�p�[��\�����Ă���ꍇ�̓T�C�Y�{�b�N�X��\�����Ȃ� */
				if( NULL != m_hwndStatusBar ){
					bSizeBox = FALSE;
				}
			}
			m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
		}

		//	Aug, 21, 2000 genta
		m_cEditDoc.ReloadAutoSaveParam();

//1999.09.03 �ȁ[�񂩂��܂�������̂ŁA��߂��B�^�p�ŃJ�o�[���Ăق����B
//
//�t�@���N�V�����L�[���ʒu�ύX�����Ƃ��A�T�C�Y�{�b�N�X�̈ʒu��ύX
//		m_cEditDoc.m_cSplitterWnd.DoSplit( -1, -1);
//�t�@���N�V�����L�[���ʒu�ύX�����Ƃ��A�t�@���N�V�����L�[���ʒu�ύX
//		::GetWindowRect( hwnd, &rc );
//		::SetWindowPos( hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top + 1, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//		::SetWindowPos( hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

		m_cEditDoc.SetDocumentIcon();	// Sep. 10, 2002 genta �����A�C�R���̍Đݒ�
		m_cEditDoc.OnChangeSetting();	/* �r���[�ɐݒ�ύX�𔽉f������ */
		return 0L;
	case MYWM_SETACTIVEPANE:
		if( -1 == (int)wParam ){
			if( 0 == lParam ){
				nPane = m_cEditDoc.m_cSplitterWnd.GetFirstPane();
			}else{
				nPane = m_cEditDoc.m_cSplitterWnd.GetLastPane();
			}
		}
		m_cEditDoc.SetActivePane( nPane );
		return 0L;


	case MYWM_SETCARETPOS:	/* �J�[�\���ʒu�ύX�ʒm */
		ppoCaret = (POINT*)m_pShareData->m_szWork;
		/* �͈͑I�𒆂� */
		if( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].DisableSelectArea( TRUE );
		}
		/*
		�J�[�\���ʒu�ϊ�
		 �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		��
		 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
			ppoCaret->x,
			ppoCaret->y,
			&nCaretPosX,
			&nCaretPosY
		);
		/* �J�[�\���ړ� */
		if( nCaretPosY >= m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_GOFILEEND, FALSE, 0, 0, 0, 0 );
		}else{
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE, _CARETMARGINRATE / 3 );
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX_Prev =
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX;
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* �J�[�\���ʒu�擾�v�� */
		ppoCaret = (POINT*)m_pShareData->m_szWork;
		/*
		�J�[�\���ʒu�ϊ�
		 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		��
		�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		m_cEditDoc.m_cLayoutMgr.CaretPos_Log2Phys(
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX,
			m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY,
			(int*)&ppoCaret->x,
			(int*)&ppoCaret->y
		);
		return 0L;

	case MYWM_GETLINEDATA:	/* �s(���s�P��)�f�[�^�̗v�� */
		pLine = m_cEditDoc.m_cDocLineMgr.GetLineStr( (int)wParam, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > sizeof( m_pShareData->m_szWork ) ){
			memcpy( m_pShareData->m_szWork, pLine, sizeof( m_pShareData->m_szWork ) );
		}else{
			memcpy( m_pShareData->m_szWork, pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
//		MYTRACE( "MYWM_ADDSTRING[%s]\n", m_pShareData->m_szWork );
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_ADDTAIL, TRUE, (LPARAM)m_pShareData->m_szWork, (LPARAM)lstrlen( m_pShareData->m_szWork ), 0, 0 );
		m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].HandleCommand( F_GOFILEEND, TRUE, 0, 0, 0, 0 );
		return 0L;
	case MYWM_SETREFERER:
		ppoCaret = (POINT*)m_pShareData->m_szWork;

		m_cEditDoc.SetReferer( (HWND)wParam, ppoCaret->x, ppoCaret->y );
		return 0L;

	//by �S (2) MYWM_CHECKSYSMENUDBLCLK�͕s�v��, WM_LBUTTONDBLCLK�ǉ�
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
			return m_cEditDoc.DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
// >> by aroka
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

/*! �I�����̏���

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
int	CEditWnd::OnClose( void )
{
	/* �E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( m_hWnd );
//	if( ::IsIconic( m_hWnd ) ){
//		::ShowWindow( m_hWnd, SW_RESTORE );
//	}else{
//		::ShowWindow( m_hWnd, SW_SHOW );
//	}
//	::SetForegroundWindow( m_hWnd );
//	::SetActiveWindow( m_hWnd );
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	return m_cEditDoc.OnFileClose();
}






void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
//	MYTRACE( "CEditWnd::OnCommand()\n" );

	static			CHOOSEFONT cf;
	CMemory			cMemKeyList;
	int				nFuncCode;
	HWND			hwndWork;
	BOOL			bOpened;
	FileInfo*		pfi;
	HWND			hWndOwner;
	static char		szURL[1024];

	switch( wNotifyCode ){
	/* ���j���[����̃��b�Z�[�W */
	case 0:
		switch( wID ){
		case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
			/* �T�N���G�f�B�^�̑S�I�� */
			CEditApp::TerminateApplication();
			break;
//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
		case F_HELP_CONTENTS:
			/* �w���v�ڎ� */
			{
				char	szHelp[_MAX_PATH + 1];
				/* �w���v�t�@�C���̃t���p�X��Ԃ� */
				::GetHelpFilePath( szHelp );
				ShowWinHelpContents( m_hWnd, szHelp );	//	�ڎ���\������
			}
			break;
		case F_HELP_SEARCH:
			/* �w���v�L�[���[�h���� */
			{
				char	szHelp[_MAX_PATH + 1];
				/* �w���v�t�@�C���̃t���p�X��Ԃ� */
				::GetHelpFilePath( szHelp );
				::WinHelp( m_hWnd, szHelp, HELP_KEY, (unsigned long)"" );
			}
			break;
		case F_ABOUT:	//Dec. 25, 2000 JEPRO F_�ɕύX
			/* �o�[�W������� */
			{
				CDlgAbout cDlgAbout;
				cDlgAbout.DoModal( m_hInstance, m_hWnd );

#ifdef _DEBUG
				HRESULT hres;
				char szPath[_MAX_PATH + 1];
				/* �V���[�g�J�b�g(.lnk)�̉��� */
				hres = ::ResolveShortcutLink( m_hWnd, "C:\\WINDOWS\\All Users\\�޽�į��\\Outlook Express.LNK", szPath );
				MYTRACE( "hres=%xh, szPath=[%s]\n", hres, szPath );

				/* �V���[�g�J�b�g(.lnk)�̉��� */
				hres = ::ResolveShortcutLink( m_hWnd, "C:\\My Documents\\develop\\EDIT\\sakura\\CDlgCompare.cpp", szPath );
				MYTRACE( "hres=%xh, szPath=[%s]\n", hres, szPath );

				/* �p�X���ɑ΂���A�C�e���h�c���X�g���쐬���� */
				BOOL bRes;
				ITEMIDLIST* pIDL;
				pIDL = CreateItemIDList( "C:\\WINDOWS\\ALLUSE~1\\�޽�į��\\OUTLOO~1.LNK" );
//				pIDL = CreateItemIDList( "\\\\NPSV-NT5\\newpat-src\\nwmsc4\\ZENBUN~1\\ZENBUN~1.RC" );
//				pIDL = CreateItemIDList( "C:\\MYDOCU~1\\DEVELOP\\ZENBUN~1\\CDLGSE~2.CPP" );

				// SHGetPathFromIDList()�֐��̓A�C�e���h�c���X�g�̕����p�X��T���Ă����
				bRes = ::SHGetPathFromIDList( pIDL, szPath );
				MYTRACE( "szPath=[%s]\n", szPath );

				/* �A�C�e���h�c���X�g���폜���� */
				bRes = DeleteItemIDList( pIDL );
				MYTRACE( "bRes=%d\n", bRes );
#endif
			}
			break;
		default:
			if( wID - IDM_SELWINDOW >= 0 &&
				wID - IDM_SELWINDOW < m_pShareData->m_nEditArrNum ){
				hwndWork = m_pShareData->m_pEditArr[wID - IDM_SELWINDOW].m_hWnd;
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( hwndWork );
			}else
			if( wID - IDM_SELMRU >= 0 &&
				wID - IDM_SELMRU < 999
			){
				/* �w��t�@�C�����J����Ă��邩���ׂ� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
				CMRU cMRU;
				FileInfo checkFileInfo;
				cMRU.GetFileInfo(wID - IDM_SELMRU, &checkFileInfo);
				if( CShareData::getInstance()->IsPathOpened( checkFileInfo.m_szPath, &hWndOwner ) ){

					::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hWndOwner );
					/* MRU���X�g�ւ̓o�^ */
					cMRU.Add( pfi );
				}else{
					/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
//@@@ 2002.01.08 YAZAKI Grep���ʂŖ����ꍇ���܂߂�B
					if( !m_cEditDoc.IsModified() &&
						!m_cEditDoc.IsFilePathAvailable() && 	/* ���ݕҏW���̃t�@�C���̃p�X */
						!m_cEditDoc.m_bGrepMode	//	����ɁAGrep���[�h����Ȃ��B
					){
						/* �t�@�C���ǂݍ��� */
						m_cEditDoc.FileRead(
							checkFileInfo.m_szPath,
							&bOpened,
							checkFileInfo.m_nCharCode,
							FALSE,	/* �ǂݎ���p�� */
							TRUE	/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
						);
					}else{
						/* �V���ȕҏW�E�B���h�E���N�� */
						//	From Here Oct. 27, 2000 genta	�J�[�\���ʒu�𕜌����Ȃ��@�\
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
						CMRU cMRU;
						FileInfo openFileInfo;
						cMRU.GetFileInfo(wID - IDM_SELMRU, &openFileInfo);
						if( m_pShareData->m_Common.GetRestoreCurPosition() ){
							CEditApp::OpenNewEditor2( m_hInstance, m_hWnd, &openFileInfo, FALSE );
						}
						else {
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd,
								openFileInfo.m_szPath,
								openFileInfo.m_nCharCode,
								FALSE );

						}
						//	To Here Oct. 27, 2000 genta
					}
				}
			}else
			if( wID - IDM_SELOPENFOLDER >= 0 &&
				wID - IDM_SELOPENFOLDER < 999
			){
				{
					char*		pszPath = new char[_MAX_PATH];
					BOOL		bOpened;
					int			nCharCode;
					BOOL		bReadOnly;
					FileInfo*	pfi;
					HWND		hWndOwner;

					strcpy( pszPath, "" );

					//Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
					CMRUFolder cMRUFolder;
					NetConnect( cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER ) );

					/* �u�t�@�C�����J���v�_�C�A���O */
					nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
					bReadOnly = FALSE;
					if( !m_cEditDoc.OpenFileDialog( m_hWnd, cMRUFolder.GetPath(wID - IDM_SELOPENFOLDER), pszPath, &nCharCode, &bReadOnly ) ){
						return;
					}
					/* �w��t�@�C�����J����Ă��邩���ׂ� */
					if( CShareData::getInstance()->IsPathOpened( pszPath, &hWndOwner ) ){
						::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
						pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

						if( CODE_AUTODETECT == nCharCode ){	/* �����R�[�h�������� */
							int nCharCodeNew; 
							/*
							|| �t�@�C���̓��{��R�[�h�Z�b�g����
							||
							|| �y�߂�l�z
							||	SJIS	0
							||	JIS		1
							||	EUC		2
							||	Unicode	3
							||	�G���[	-1
							*/
							nCharCodeNew = CMemory::CheckKanjiCodeOfFile( pszPath );
							if( -1 == nCharCodeNew ){

							}else{
								nCharCode = nCharCodeNew;
							}
						}
						if( nCharCode != pfi->m_nCharCode ){	/* �����R�[�h��� */
							char*	pszCodeNameCur;
							char*	pszCodeNameNew;
							// gm_pszCodeNameArr_1 ���g���悤�ɕύX Moca. 2002/05/26
							if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
								pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
							}
							if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
								pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
							}
							::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
								"%s\n\n\n���ɊJ���Ă���t�@�C�����Ⴄ�����R�[�h�ŊJ���ꍇ�́A\n��U���Ă���J���Ă��������B\n\n���݂̕����R�[�h�Z�b�g=[%s]\n�V���������R�[�h�Z�b�g=[%s]",
								pszPath, pszCodeNameCur, pszCodeNameNew
							);
						}
						/* �������J���Ă��邩 */
						if( 0 == strcmp( m_cEditDoc.GetFilePath(), pszPath ) ){
							/* �������Ȃ� */
						}else{
							/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
							/* �A�N�e�B�u�ɂ��� */
							ActivateFrameWindow( hWndOwner );
						}
					}else{
						/* �t�@�C�����J����Ă��Ȃ� */
						/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
						if( !m_cEditDoc.IsModified() &&
							!m_cEditDoc.IsFilePathAvailable() &&		/* ���ݕҏW���̃t�@�C���̃p�X */
							//	Jun. 13, 2003 Moca Grep�E�B���h�E��Drop�����Ƃ���Grep�E�B���h�E�̂܂�
							//	�����E�B���h�E�ŊJ����Ă��܂������C��
							!m_cEditDoc.m_bGrepMode					/* Grep���ʂł͂Ȃ� */
						){
							/* �t�@�C���ǂݍ��� */
							m_cEditDoc.FileRead( pszPath, &bOpened, nCharCode, bReadOnly,
								TRUE	/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
							);
						}else{
							if( strchr( pszPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", pszPath );
								strcpy( pszPath, szFile2 );
							}
							/* �V���ȕҏW�E�B���h�E���N�� */
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd, pszPath, nCharCode, bReadOnly );
						}
					}
					delete [] pszPath;
				}
			}else{
				//�r���[�Ƀt�H�[�J�X���ړ����Ă���
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
					::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

				/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
				m_cEditDoc.HandleCommand( wID );
			}
			break;
		}
		break;
	/* �A�N�Z�����[�^����̃��b�Z�[�W */
	case 1:
		//�r���[�Ƀt�H�[�J�X���ړ����Ă���
		if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
			::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

		nFuncCode = CKeyBind::GetFuncCode(
			wID,
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
		);
//		MYTRACE( "CEditWnd::OnCommand()  nFuncCode=%d\n", nFuncCode );
		m_cEditDoc.HandleCommand( nFuncCode );
		break;

	case CBN_SETFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = F_SEARCH_BOX;
		}
		break;

	case CBN_KILLFOCUS:
		if( NULL != m_hwndSearchBox && hwndCtl == m_hwndSearchBox )
		{
			m_nCurrentFocus = 0;

			//�t�H�[�J�X���͂��ꂽ�Ƃ��Ɍ����L�[�ɂ��Ă��܂��B
			//�����L�[���[�h���擾
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );
			}

		}
		break;

	/* �R���g���[������̃��b�Z�[�W�ɂ͒ʒm�R�[�h */
	default:
		break;
	}
	return;
}



//	�L�[���[�h�F���j���[�o�[����
//	Sept.14, 2000 Jepro note: ���j���[�o�[�̍��ڂ̃L���v�V�����⏇�Ԑݒ�Ȃǂ͈ȉ��ōs���Ă���炵��
//	Sept.16, 2000 Jepro note: �A�C�R���Ƃ̊֘A�t����CShareData_new2.cpp�t�@�C���ōs���Ă���
void CEditWnd::InitMenu( HMENU hMenu, UINT uPos, BOOL fSystemMenu )
{
	int			cMenuItems;
	int			nPos;
	UINT		id;
	UINT		fuFlags;
	int			i;
	BOOL		bRet;
	char		szMemu[280];
	int			nRowNum;
	EditNode*	pEditNodeArr;
	FileInfo*	pfi;

	HMENU		hMenuPopUp;
	HMENU		hMenuPopUp_2;
	const char*	pszLabel;


	if( hMenu != ::GetSubMenu( ::GetMenu( m_hWnd ), uPos ) ){
		goto end_of_func_IsEnable;
	}


	if( fSystemMenu ){
	}else{
//		MYTRACE( "hMenu=%08xh uPos=%d, fSystemMenu=%s\n", hMenu, uPos, fSystemMenu ? "TRUE":"FALSE" );
		switch( uPos ){
		case 0:
			/* �u�t�@�C���v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, "�V�K�쐬(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, "�J��(&O)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, "�㏑���ۑ�(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, "���O��t���ĕۑ�(&A)..." );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, "����(����) (&B)" );	//Oct. 17, 2000 jepro �L���v�V�������u����v����ύX	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(C��B; Blank�̈Ӗ�)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, "���ĊJ��(&L)..." );
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, "�J������(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "�E�B���h�E�����(&C)" );	//Feb. 18, 2001	JEPRO �ǉ�

			// �u�����R�[�h�Z�b�g�v�|�b�v�A�b�v���j���[
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, "&SJIS�ŊJ������" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, "&JIS�ŊJ������" );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, "&EUC�ŊJ������" );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, "&Unicode�ŊJ������" );	//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, "U&nicodeBE�ŊJ������" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, "UTF-&8�ŊJ������" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, "UTF-&7�ŊJ������" );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp_2 , "�����R�[�h�Z�b�g(&H)" );//Oct. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(M��H)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, "���(&P)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, "����v���r���[(&V)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, "�y�[�W�ݒ�(&U)..." );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�t�@�C������v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC				, "������C/C++�w�b�_(�\�[�X)���J��(&C)" );	//Feb. 7, 2001 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, "SQL*Plus���A�N�e�B�u�\��(&A)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^	�����́u�A�N�e�B�u���v���u�A�N�e�B�u�\���v�ɓ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, "SQL*Plus�Ŏ��s(&X)" );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BROWSE						, "�u���E�Y(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_READONLY					, "�ǂݎ���p(&O)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, "�t�@�C���̃v���p�e�B(&R)" );		//Nov. 7, 2000 jepro �L���v�V������'�t�@�C����'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�t�@�C������(&R)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
			{
				//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
				CMRU cMRU;
				hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	�t�@�C�����j���[
				if ( cMRU.Length() > 0 ){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
				}
			}

			/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
				CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, "���ׂẴE�B���h�E�����(&Q)" );	//Feb/ 19, 2001 JEPRO �ǉ�
			//	Jun. 9, 2001 genta �\�t�g�E�F�A������
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, "�T�N���G�f�B�^�̑S�I��(&X)" );	//Sept. 11, 2000 jepro �L���v�V�������u�A�v���P�[�V�����I���v����ύX	//Dec. 26, 2000 JEPRO F_�ɕύX
			break;

		case 1:
			/* �u�ҏW�v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, "���ɖ߂�(&Undo)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, "��蒼��(&Redo)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, "�؂���(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, "�R�s�[(&C)" );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "�S�s���p�R�s�[(&N)" );
			//	Sept. 14, 2000 JEPRO	�L���v�V�����Ɂu�L���t���v��ǉ��A�A�N�Z�X�L�[�ύX(N��.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "�I��͈͓��S�s���p���t���R�s�[(&.)" );
//			Sept. 30, 2000 JEPRO	���p���t���R�s�[�̃A�C�R�����쐬�����̂ŏ�L���j���[�͏d��������āu���x�ȑ���v���ɂ��������ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, "�\��t��(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, "�폜(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, "���ׂđI��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );							/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, "�ĕϊ�(&R)" );		/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, "�}���^�㏑�����[�h�؂�ւ�(&M)" );	//Nov. 9, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, "CR&LF���s�ŃR�s�[" );				//Nov. 9, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, "�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, "��`�\��t��(&X)" );					//Sept. 13, 2000 JEPRO �ړ��ɔ����A�N�Z�X�L�[�t�^	//Oct. 22, 2000 JEPRO �A�N�Z�X�L�[�ύX(P��X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, "�J�[�\���O���폜(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�}���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, "���t(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, "����(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, "�R���g���[���R�[�h(&C)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�}��(&I)" );

			// �u���x�ȑ���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	"�P��̍��[�܂ō폜(&L)" );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	"�P��̉E�[�܂ō폜(&R)" );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	"���݈ʒu�̒P��I��(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	"�P��؂���(&T)" );				//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	"�P��폜(&D)" );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	"�s���܂Ő؂���(���s�P��) (&U)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	"�s���܂Ő؂���(���s�P��) (&K)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	"�s���܂ō폜(���s�P��) (&H)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	"�s���܂ō폜(���s�P��) (&E)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	"�s�؂���(�܂�Ԃ��P��) (&X)" );	//Jan. 16, 2001 JEPRO �s(���E��)�֌W�̏��������ւ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	"�s�폜(�܂�Ԃ��P��) (&Y)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	"�s�̓�d��(�܂�Ԃ��P��) (&2)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	"TAB�C���f���g(&A)" );				//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	"�tTAB�C���f���g(&B)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	"SPACE�C���f���g(&S)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	"�tSPACE�C���f���g(&P)" );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, "�I��͈͓��S�s�R�s�[(&@)" );		//Sept. 14, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, "�I��͈͓��S�s���p���t���R�s�[(&.)" );//Sept. 13, 2000 JEPRO �L���v�V��������u�L���t���v��ǉ��A�A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, "�I��͈͓��S�s�s�ԍ��t���R�s�[(&:)" );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	"���̃t�@�C�������R�s�[(&-)" );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	"���̃t�@�C���̃p�X�����R�s�[(&\\)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	"���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[(&^)" );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, "�L�[���蓖�Ĉꗗ���R�s�[(&Q)" );	//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q)
//Sept. 16, 2000 JEPRO �V���[�g�J�b�g�L�[�����܂������Ȃ��̂Ŏ��s�͎E���Č��ɖ߂��Ă���		//Dec. 25, 2000 ����
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, "�L�[���蓖�Ĉꗗ���R�s�[(&Q)" );			//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q) IDM_TEST��F�ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, "�P�ꃊ�t�@�����X(&W)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "���x�ȑ���(&V)" );

		//From Here Feb. 19, 2001 JEPRO [�ړ�(M)], [�I��(R)]���j���[��[�ҏW]�̃T�u���j���[�Ƃ��Ĉړ�
			// �u�ړ��v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, "�J�[�\����ړ�(�Q�s����) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, "�J�[�\�����ړ�(�Q�s����) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, "�P��̍��[�Ɉړ�(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, "�P��̉E�[�Ɉړ�(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, "�O�̒i���Ɉړ�(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, "���̒i���Ɉړ�(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, "�s���Ɉړ�(�܂�Ԃ��P��) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, "�s���Ɉړ�(�܂�Ԃ��P��) (&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, "�P�y�[�W�A�b�v(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, "�P�y�[�W�_�E��(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, "�t�@�C���̐擪�Ɉړ�(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, "�t�@�C���̍Ō�Ɉړ�(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, "�J�[�\���s���E�B���h�E������(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, "�w��s�փW�����v(&J)..." );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "�����J�n�ʒu�֖߂�(&I)" );	// �����J�n�ʒu�֖߂� 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, "�ړ�����: �O��(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, "�ړ�����: ����(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, "���݈ʒu���ړ������ɓo�^(&S)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ړ�(&O)" );

			// �u�I���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, "���݈ʒu�̒P��I��(&W)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, "���ׂđI��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, "�͈͑I���J�n(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, "(�I��)�J�[�\����ړ�(�Q�s����) (&Q)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, "(�I��)�J�[�\�����ړ�(�Q�s����) (&K)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, "(�I��)�P��̍��[�Ɉړ�(&L)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, "(�I��)�P��̉E�[�Ɉړ�(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, "(�I��)�O�̒i���Ɉړ�(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, "(�I��)���̒i���Ɉړ�(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, "(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&H)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, "(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&E)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, "(�I��)�P�y�[�W�A�b�v(&U)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, "(�I��)�P�y�[�W�_�E��(&D)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, "(�I��)�t�@�C���̐擪�Ɉړ�(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, "(�I��)�t�@�C���̍Ō�Ɉړ�(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�I��(&S)" );

			// �u��`�I���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, "��`�͈͑I���J�n(&S)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP_BOX			, "(��I)�J�[�\����ړ�(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN_BOX		, "(��I)�J�[�\�����ړ�(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LEFT_BOX		, "(��I)�J�[�\�����ړ�(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RIGHT_BOX		, "(��I)�J�[�\���E�ړ�(&)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP2_BOX			, "(��I)�J�[�\����ړ�(�Q�s����) (&Q)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN2_BOX		, "(��I)�J�[�\�����ړ�(�Q�s����) (&K)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDLEFT_BOX	, "(��I)�P��̍��[�Ɉړ�(&L)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_BOX	, "(��I)�P��̉E�[�Ɉړ�(&R)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINETOP_BOX	, "(��I)�s���Ɉړ�(�܂�Ԃ��P��) (&H)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINEEND_BOX	, "(��I)�s���Ɉړ�(�܂�Ԃ��P��) (&E)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageUp_Box	, "(�I��)���y�[�W�A�b�v(&)" );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageDown_Box, "(�I��)���y�[�W�_�E��(&)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageUp_Box		, "(��I)�P�y�[�W�A�b�v(&U)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageDown_Box	, "(��I)�P�y�[�W�_�E��(&D)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILETOP_BOX	, "(��I)�t�@�C���̐擪�Ɉړ�(&T)" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILEEND_BOX	, "(��I)�t�@�C���̍Ō�Ɉړ�(&B)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "��`�I��(&E)" );

			// �u���`�v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, "��(�擪)�̋󔒂��폜(&L)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, "�E(����)�̋󔒂��폜(&R)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, "�I���s�̏����\�[�g(&A)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, "�I���s�̍~���\�[�g(&D)" );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, "�I���s�̃}�[�W(&M)" );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "���`(&K)" );

			break;
		//Feb. 19, 2001 JEPRO [�ړ�][�ړ�], [�I��]��[�ҏW]�z���Ɉڂ����̂ō폜

//		case 4://case 2: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 2://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�ϊ��v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, "�p�啶�����p������(&L)" );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, "�p���������p�啶��(&U)" );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "�S�p�����p" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "���p���S�p�J�^�J�i" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "���p���S�p�Ђ炪��" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "�S�p�����p(&F)" );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, "�S�p�J�^�J�i�����p�J�^�J�i" );		//Aug. 29, 2002 ai
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, "�S�p�p�������p�p��" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, "���p�p�����S�p�p��" );				//July. 29, 2001 Misaka �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "���p�{�S�Ђ灨�S�p�E�J�^�J�i(&Z)" );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "���p�{�S�J�^���S�p�E�Ђ炪��(&N)" );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKAKUKATA, "���p�J�^�J�i���S�p�J�^�J�i(&K)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKAKUHIRA, "���p�J�^�J�i���S�p�Ђ炪��(&H)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, "TAB����(&S)" );	//Feb. 19, 2001 JEPRO ������ړ�����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, "�󔒁�TAB(&T)" );	//#### Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//�u�����R�[�h�ϊ��v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, "�������ʁ�SJIS�R�[�h�ϊ�(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, "E-Mail(JIS��SJIS)�R�[�h�ϊ�(&M)" );//Sept. 11, 2000 JEPRO �L���v�V�����ɁuE-Mail�v��ǉ����A�N�Z�X�L�[�ύX(V��M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, "EUC��SJIS�R�[�h�ϊ�(&W)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(E��W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, "Unicode��SJIS�R�[�h�ϊ�(&U)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX����I:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, "UnicodeBE��SJIS�R�[�h�ϊ�(&N)" );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, "UTF-8��SJIS�R�[�h�ϊ�(&T)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(T:uTF/shifT)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, "UTF-7��SJIS�R�[�h�ϊ�(&F)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(F:utF/shiFt)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, "SJIS��JIS�R�[�h�ϊ�(&J)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, "SJIS��EUC�R�[�h�ϊ�(&E)" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, "SJIS��&Unicode�R�[�h�ϊ�" );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, "SJIS��UTF-8�R�[�h�ϊ�(&8)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, "SJIS��UTF-7�R�[�h�ϊ�(&7)" );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, "Base64�f�R�[�h���ĕۑ�(&B)" );	//Oct. 6, 2000 JEPRO �A�N�Z�X�L�[�ύX(6��B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, "uudecode���ĕۑ�(&D)" );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(U��D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�����R�[�h�ϊ�(&C)" );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 3://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, "����(&F)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, "��������(&N)" );				//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, "�O������(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, "�u��(&R)..." );				//Oct. 7, 2000 JEPRO ���̃Z�N�V�������炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, "�����}�[�N�̐ؑւ�(&C)" );	// "�����}�[�N�̃N���A(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, "�����J�n�ʒu�֖߂�(&I)" );	// �����J�n�ʒu�֖߂� 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, "&Grep..." );					//Oct. 7, 2000 JEPRO �����炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, "�w��s�փW�����v(&J)..." );	//Sept. 11, 2000 jepro �L���v�V�����Ɂu �W�����v�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, "�A�E�g���C�����(&L)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, "�^�O�W�����v(&T)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, "�^�O�W�����v�o�b�N(&B)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, "�^�O�t�@�C���̍쐬" );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, "�_�C���N�g�^�O�W�����v" );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, "���͕⊮(&/)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, "�t�@�C�����e��r(&@)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, "DIFF�����\��(&D)..." );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, "���̍�����" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, "�O�̍�����" );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, "�����\���̑S����" );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	�Ί��ʂ̌��������j���[�ɒǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, "�Ί��ʂ̌���(&[)" );
//	To Here Sept. 1, 2000
// From Here 2001.12.03 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, "�u�b�N�}�[�N�ݒ�E����(&S)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, "���̃u�b�N�}�[�N��(&A)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, "�O�̃u�b�N�}�[�N��(&Z)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, "�u�b�N�}�[�N�̑S����(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, "�u�b�N�}�[�N�̈ꗗ(&V)" );
// To Here 2001.12.03 hor

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 4://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�I�v�V�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//	From Here Sept. 17, 2000 JEPRO
//	��͂�Win�W���ɍ��킹�ă`�F�b�N�}�[�N�����ŕ\���^��\���𔻒f����悤�ɂ������������̂ŕύX
			if ( FALSE == m_pShareData->m_Common.m_bMenuIcon ){
				pszLabel = "�c�[���o�[��\��(&T)";				//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				pszLabel = "�t�@���N�V�����L�[��\��(&K)";		//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				pszLabel = "�X�e�[�^�X�o�[��\��(&S)";			//����̂ݕ\��
//	To Here Sept.17, 2000 JEPRO
//	From Here Oct. 28, 2000 JEPRO
//	3�{�^���̃A�C�R�����ł������Ƃɔ����\���^��\���̃��b�Z�[�W��ς���悤�ɍĂѕύX
			}else{
				if( m_hwndToolBar == NULL ){
					pszLabel = "�c�[���o�[��\��(&T)";			//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃c�[���o�[���B��(&T)";			//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				if( NULL == m_CFuncKeyWnd.m_hWnd ){
					pszLabel = "�t�@���N�V�����L�[��\��(&K)";	//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃t�@���N�V�����L�[���B��(&K)";	//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				if( m_hwndStatusBar == NULL ){
					pszLabel = "�X�e�[�^�X�o�[��\��(&S)";		//����̂ݕ\��
				}else{
					pszLabel = "�\�����̃X�e�[�^�X�o�[���B��(&S)";		//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
			}
//	To Here Oct. 28, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWSTATUSBAR, pszLabel );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, "�^�C�v�ʐݒ�ꗗ(&L)..." );	//Sept. 13, 2000 JEPRO �ݒ����Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, "�^�C�v�ʐݒ�(&Y)..." );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(S��Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, "���ʐݒ�(&C)..." );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, "�t�H���g�ݒ�(&F)..." );		//Sept. 17, 2000 jepro �L���v�V�����Ɂu�ݒ�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, "���C�ɓ���̐ݒ�(&O)..." );	//���C�ɓ���	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//@@@ 2002.01.14 YAZAKI �܂�Ԃ��Ȃ��R�}���h�ǉ�
			if( m_cEditDoc.GetDocumentAttribute().m_nMaxLineSize == m_cEditDoc.ActiveView().m_nViewColNum ){
				pszLabel = "�܂�Ԃ��Ȃ�(&W)";
			}else{
				pszLabel = "���݂̃E�B���h�E���Ő܂�Ԃ�(&W)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , "���݂̃E�B���h�E���Ő܂�Ԃ�(&W)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			if( !m_pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "�L�[�}�N���̋L�^�J�n(&R)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "�L�[�}�N���̕ۑ�(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "�L�[�}�N���̓ǂݍ���(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "�L�[�}�N���̎��s(&D)" );
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, "�L�[�}�N���̋L�^�I��(&R)" );
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, "�L�[�}�N���̋L�^�I��&&�ۑ�(&M)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, "�L�[�}�N���̋L�^�I��&&�ǂݍ���(&A)" );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, "�L�[�}�N���̋L�^�I��&&���s(&D)" );
			}
			
			//	From Here Sep. 14, 2001 genta
			//�u�o�^�ς݃}�N���v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			
			for( i = 0; i < MAX_CUSTMACRO; ++i ){
				MacroRec *mp = &m_pShareData->m_MacroTable[i];
				if( mp->IsEnabled() ){
					if(  mp->m_szName[0] ){
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szName );
					}
					else {
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szFile );
					}
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�o�^�ς݃}�N��(&B)" );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, "�O���R�}���h���s(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCOMMAND_DIALOG, "�O���R�}���h���s(&X)" );	//Mar. 10, 2001 JEPRO �@�\���Ȃ��̂Ń��j���[����B����	//Mar.21, 2001 JEPRO �W���o�͂��Ȃ��ŕ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//To Here Sept. 20, 2000

			//�u�J�X�^�����j���[�v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	�E�N���b�N���j���[
			if( m_pShareData->m_Common.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_szCustMenuNameArr[0] );
			}
			//	�J�X�^�����j���[
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_szCustMenuNameArr[i] );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�J�X�^�����j���[(&U)" );

//		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 5://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�E�B���h�E�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitRows() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "�㉺�ɕ���(&-)";	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(T��-)
			}else{
				pszLabel = "�㉺�����̉���(&-)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_V , pszLabel );

			if( 1 == m_cEditDoc.m_cSplitterWnd.GetAllSplitCols() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "���E�ɕ���(&I)";	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(Y��I)
			}else{
				pszLabel = "���E�����̉���(&I)";
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_H , pszLabel );
//	From Here Sept. 17, 2000 JEPRO	�c�������̏ꍇ����Ԃɂ���ă��j���[���b�Z�[�W���ς��悤�ɕύX
			if( (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitRows()) && (1 < m_cEditDoc.m_cSplitterWnd.GetAllSplitCols()) ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = "�c�������̉���(&S)";	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(Q��S)
			}else{
				pszLabel = "�c���ɕ���(&S)";	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(S��Q)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�����ɖ߂���(Q��S)
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_VH , pszLabel );
//	To Here Sept. 17, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, "����(&C)" );			//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(O��C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, "���ׂĕ���(&Q)" );		//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, "���̃E�B���h�E(&N)" );	//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, "�O�̃E�B���h�E(&P)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, "�d�˂ĕ\��(&E)" );		//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(C��E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, "�㉺�ɕ��ׂĕ\��(&H)" );	//Sept. 13, 2000 JEPRO �����ɍ��킹�ă��j���[�̍��E�Ə㉺�����ւ��� //Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(V��H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, "���E�ɕ��ׂĕ\��(&T)" );	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(H��T)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, "�c�����ɍő剻(&X)" );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, "�������ɍő剻(&Y)" );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, "���ׂčŏ���(&M)" );		//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */				//Oct. 22, 2000 JEPRO ���́u�ĕ`��v�����ɔ����Z�p���[�^��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, "�ĕ`��(&R)" );			//Oct. 22, 2000 JEPRO �R�����g�A�E�g����Ă����̂𕜊�������
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, "�A�E�g�v�b�g(&U)" );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(O��U)

			/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( nRowNum > 0 ){
				/* �Z�p���[�^ */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
				for( i = 0; i < nRowNum; ++i ){
					/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
					::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					if( pfi->m_bIsGrep ){
						/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
						CMemory		cmemDes;
						int			nDesLen;
						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						pszDes = cmemDes.GetPtr();
						nDesLen = lstrlen( pszDes );
//	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
//		i >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���J���t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
						wsprintf( szMemu, "&%c �yGrep�z\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "�E�E�E":""
						);
					}else
					if( pEditNodeArr[i].m_hWnd == m_pShareData->m_hwndDebug ){
//		i >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���o�̓t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
						wsprintf( szMemu, "&%c �A�E�g�v�b�g", ((1 + i) <= 9)?('1' + i):('A' + i - 9) );

					}else{
//		From Here Jan. 23, 2001 JEPRO
//		�t�@�C������p�X����'&'���g���Ă���Ƃ��ɗ��𓙂ŃL�`���ƕ\������Ȃ������C��(&��&&�ɒu�����邾��)
//<----- From Here Added
						char	szFile2[_MAX_PATH * 2];
						if( '\0' == pfi->m_szPath[0] ){
							strcpy( szFile2, "(����)" );
						}else{
							char buf[_MAX_PATH];
							CShareData::getInstance()->GetTransformFileName( pfi->m_szPath, buf, _MAX_PATH );
							dupamp( buf, szFile2 );
						}
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							szFile2,
							pfi->m_bIsModified ? "*":" "
						);
//-----> To Here Added
//		To Here Jan. 23, 2001

//	To Here Oct. 4, 2000
						// SJIS�ȊO�̕����R�[�h�̎�ʂ�\������
						// gm_pszCodeNameArr_3 ����R�s�[����悤�ɕύX
						if( 0 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
							strcat( szMemu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
						}
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMemu );
					if( m_hWnd == pEditNodeArr[i].m_hWnd ){
						::CheckMenuItem( hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED );
					}
				}
				delete [] pEditNodeArr;
			}

			break;

//		case 8://case 6: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 6://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�w���v�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "�ڎ�(&O)" );				//Sept. 7, 2000 jepro �L���v�V�������u�w���v�ڎ��v����ύX	Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(C��O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 "�L�[���[�h����(&S)" );	//Sept. 7, 2000 jepro �L���v�V�������u�w���v�g�s�b�N�̌����v����ύX //Nov. 25, 2000 jepro�u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, "�R�}���h�ꗗ(&M)" );		//Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(L��M)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, "�O���w���v�P(&E)" );		//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��g�b�v���牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, "�O��HTML�w���v(&H)" );	//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��Q�Ԗڂ��牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, "�o�[�W�������(&A)" );	//Dec. 25, 2000 JEPRO F_�ɕύX
			break;
		}
	}

end_of_func_IsEnable:;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if (m_pPrintPreview)	return;	//	����v���r���[���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

	/* �@�\�����p�\���ǂ����A�`�F�b�N��Ԃ��ǂ������ꊇ�`�F�b�N */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		id = ::GetMenuItemID(hMenu, nPos);
		/* �@�\�����p�\�����ׂ� */
		if( IsFuncEnable( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_ENABLED;
		}else{
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
		}
		::EnableMenuItem(hMenu, id, fuFlags);

		/* �@�\���`�F�b�N��Ԃ����ׂ� */
		if( IsFuncChecked( &m_cEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
		/* else{
			fuFlags = MF_BYCOMMAND | MF_UNCHECKED;
		}
		*/
	}
	return;
}




/* �t�@�C�����h���b�v���ꂽ */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
		POINT		pt;
		WORD		cFiles, i;
		char		szFile[_MAX_PATH + 1];
		char		szWork[_MAX_PATH + 1];
		BOOL		bOpened;
		FileInfo*	pfi;
		HWND		hWndOwner;

		::DragQueryPoint( hDrop, &pt );
		cFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, (LPSTR) NULL, 0);
		/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
		if( m_pShareData->m_Common.m_bDropFileAndClose ){
			cFiles = 1;
		}
		/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
		if( cFiles > m_pShareData->m_Common.m_nDropFileNumMax ){
				cFiles = m_pShareData->m_Common.m_nDropFileNumMax;
		}
		for( i = 0; i < cFiles; i++ ) {
				::DragQueryFile( hDrop, i, szFile, sizeof(szFile) );
				/* �V���[�g�J�b�g(.lnk)�̉��� */
				if( TRUE == ResolveShortcutLink( NULL, szFile, szWork ) ){
					strcpy( szFile, szWork );
				}
				/* �����O�t�@�C�������擾���� */
				if( TRUE == ::GetLongFileName( szFile, szWork ) ){
						strcpy( szFile, szWork );
				}

				/* �w��t�@�C�����J����Ă��邩���ׂ� */
				if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
					::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hWndOwner );
					/* MRU���X�g�ւ̓o�^ */
					CMRU cMRU;
					cMRU.Add( pfi );
				}else{
						/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
						if( !m_cEditDoc.IsModified() &&	//	Jan. 22, 2002 genta
							!m_cEditDoc.IsFilePathAvailable() &&	/* ���ݕҏW���̃t�@�C���̃p�X */
							//	Jun. 13, 2003 Moca Grep�E�B���h�E��Drop�����Ƃ���Grep�E�B���h�E�̂܂�
							//	�����E�B���h�E�ŊJ����Ă��܂������C��
							!m_cEditDoc.m_bGrepMode					/* Grep���ʂł͂Ȃ� */  // 2003.06.13 Add
						){
								/* �t�@�C���ǂݍ��� */
								m_cEditDoc.FileRead(
										szFile,
										&bOpened,
										CODE_AUTODETECT,	/* �����R�[�h�������� */
										FALSE,				/* �ǂݎ���p�� */
										TRUE				/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
								);
								hWndOwner = m_hWnd;
								/* �A�N�e�B�u�ɂ��� */
								ActivateFrameWindow( hWndOwner );
						}else{
								/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
								if( m_pShareData->m_Common.m_bDropFileAndClose ){
										/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
										if( m_cEditDoc.OnFileClose() ){
												/* �����f�[�^�̃N���A */
												m_cEditDoc.Init();

												/* �S�r���[�̏����� */
												m_cEditDoc.InitAllView();

												/* �e�E�B���h�E�̃^�C�g�����X�V */
												m_cEditDoc.SetParentCaption();

												/* �t�@�C���ǂݍ��� */
														m_cEditDoc.FileRead(
														szFile,
														&bOpened,
														CODE_AUTODETECT,	/* �����R�[�h�������� */
														FALSE,				/* �ǂݎ���p�� */
														TRUE				/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
												);
												hWndOwner = m_hWnd;
												/* �A�N�e�B�u�ɂ��� */
												ActivateFrameWindow( hWndOwner );
										}
										goto end_of_drop_query;
								}else{
										/* �ҏW�E�B���h�E�̏���`�F�b�N */
										if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
												char szMsg[512];
												wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
												::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
												::DragFinish( hDrop );
												return;
										}
										char	szFile2[_MAX_PATH + 3];
										if( strchr( szFile, ' ' ) ){
												wsprintf( szFile2, "\"%s\"", szFile );
												strcpy( szFile, szFile2 );
										}
										/* �V���ȕҏW�E�B���h�E���N�� */
										CEditApp::OpenNewEditor(
												m_hInstance,
												m_hWnd,
												szFile,
												CODE_AUTODETECT,	/* �����R�[�h�������� */
												FALSE				/* �ǂݎ���p�� */
										);
								}
						}
				}
		}
end_of_drop_query:;
		::DragFinish( hDrop );
		return;
}


/* �^�C�}�[�̏���
	@datey 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
void CEditWnd::OnTimer(
	HWND		hwnd,		// handle of window for timer messages
	UINT		uMsg,		// WM_TIMER message
	UINT		idEvent,	// timer identifier
	DWORD		dwTime 		// current system time
)
{
	static	int	nLoopCount = 0;
	int			i;
	TBBUTTON	tbb;
	nLoopCount++;
	if( 10 < nLoopCount ){
		nLoopCount = 0;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[�Ȃ�A�������Ȃ��B�����łȂ���΁A�c�[���o�[�̏�ԍX�V */
	if( !m_pPrintPreview && NULL != m_hwndToolBar ){
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			tbb = m_CMenuDrawer.m_tbMyButton[m_pShareData->m_Common.m_nToolBarButtonIdxArr[i]];
			/* �@�\�����p�\�����ׂ� */
			::PostMessage(
				m_hwndToolBar, TB_ENABLEBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( (IsFuncEnable( &m_cEditDoc, m_pShareData, tbb.idCommand ) ) , 0 )
			);
			/* �@�\���`�F�b�N��Ԃ����ׂ� */
			::PostMessage(
				m_hwndToolBar, TB_CHECKBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( IsFuncChecked( &m_cEditDoc, m_pShareData, tbb.idCommand ), 0 )
			);
		}
	}

	if( nLoopCount == 0 ){
		/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
		m_cEditDoc.CheckFileTimeStamp() ;
	}

	m_cEditDoc.CheckAutoSave();
	return;
}

/* �@�\���`�F�b�N��Ԃ����ׂ� */
int CEditWnd::IsFuncChecked( CEditDoc* pcEditDoc, DLLSHAREDATA*	pShareData, int nId )
{
	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( pcEditDoc->m_hwndParent, GWL_USERDATA );
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��A�v���r���[����폜
	switch( nId ){
	case F_FILE_REOPEN_SJIS:
		if( CODE_SJIS == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_JIS:
		if( CODE_JIS == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_EUC:
		if( CODE_EUC == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UNICODE:
		if( CODE_UNICODE == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UNICODEBE:
		if( CODE_UNICODEBE == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UTF8:
		if( CODE_UTF8 == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_FILE_REOPEN_UTF7:
		if( CODE_UTF7 == pcEditDoc->m_nCharCode ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_RECKEYMACRO:	/* �L�[�}�N���̋L�^�J�n�^�I�� */
		if( pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return FALSE;
		}
	case F_SHOWTOOLBAR:
		if( pCEditWnd->m_hwndToolBar != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SHOWFUNCKEY:
		if( pCEditWnd->m_CFuncKeyWnd.m_hWnd != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SHOWSTATUSBAR:
		if( pCEditWnd->m_hwndStatusBar != NULL ){
			return TRUE;
		}else{
			return FALSE;
		}
	// Mar. 6, 2002 genta
	case F_READONLY://�ǂݎ���p
		if( pcEditDoc->m_bReadOnly ){ /* �ύX�t���O */
			return TRUE;
		}else{
			return FALSE;
		}
	}
	return FALSE;
}




/* �@�\�����p�\�����ׂ� */
int CEditWnd::IsFuncEnable( CEditDoc* pcEditDoc, DLLSHAREDATA* pShareData, int nId )
{
	/* ���������֎~�̂Ƃ����ꊇ�`�F�b�N */
	if( pcEditDoc->IsModificationForbidden( nId ) )
		return FALSE;

	switch( nId ){
	case F_RECKEYMACRO:	/* �L�[�}�N���̋L�^�J�n�^�I�� */
		if( pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return TRUE;
		}
	case F_SAVEKEYMACRO:	/* �L�[�}�N���̕ۑ� */
		//	Jun. 16, 2002 genta
		//	�L�[�}�N���G���W���ȊO�̃}�N����ǂݍ���ł���Ƃ���
		//	���s�͂ł��邪�ۑ��͂ł��Ȃ��D
		if( pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return pcEditDoc->m_pcSMacroMgr->IsSaveOk();
		}
	case F_EXECKEYMACRO:	/* �L�[�}�N���̎��s */
		if( pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			//@@@ 2002.1.24 YAZAKI m_szKeyMacroFileName�Ƀt�@�C�������R�s�[����Ă��邩�ǂ����B
			if (pShareData->m_szKeyMacroFileName[0] ) {
				return TRUE;
			}else{
				return FALSE;
			}
		}
	case F_LOADKEYMACRO:	/* �L�[�}�N���̓ǂݍ��� */
		if( pShareData->m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
			if( pShareData->m_hwndRecordingKeyMacro == pcEditDoc->m_hwndParent ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				return TRUE;
			}else{
				return FALSE;
			}
		}else{
			return TRUE;
		}

	case F_SEARCH_CLEARMARK:	//�����}�[�N�̃N���A
		return TRUE;

	// 02/06/26 ai Start
	case F_JUMP_SRCHSTARTPOS:	// �����J�n�ʒu�֖߂�
		if( 0 <= pcEditDoc->m_cEditViewArr[pcEditDoc->m_nActivePaneIndex].m_nSrchStartPosX_PHY &&
			0 <= pcEditDoc->m_cEditViewArr[pcEditDoc->m_nActivePaneIndex].m_nSrchStartPosY_PHY) {
			return TRUE;
		}else{
			return FALSE;
		}
	// 02/06/26 ai End

	case F_COMPARE:	/* �t�@�C�����e��r */
		if( 2 <= pShareData->m_nEditArrNum ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_DIFF_NEXT:	/* ���̍����� */	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:	/* �O�̍����� */	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:	/* �����̑S���� */	//@@@ 2002.05.25 MIK
		if( ! pcEditDoc->m_cDocLineMgr.IsDiffUse() ) return FALSE;
		return TRUE;
	case F_DIFF_DIALOG:	/* DIFF�����\�� */	//@@@ 2002.05.25 MIK
		//if( pcEditDoc->IsModified() ) return FALSE;
		//if( ! pcEditDoc->IsFilePathAvailable() ) return FALSE;
		return TRUE;

	case F_BEGIN_BOX:	//��`�͈͑I���J�n
		if( TRUE == pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
			return TRUE;
		}else{
			return FALSE;
		}
	case F_PASTEBOX:
		/* �N���b�v�{�[�h����\��t���\���H */
		if( pcEditDoc->IsEnablePaste() &&
			TRUE == pShareData->m_Common.m_bFontIs_FIXED_PITCH
		){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_PASTE:
		/* �N���b�v�{�[�h����\��t���\���H */
		if( pcEditDoc->IsEnablePaste() ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_FILENEW:	/* �V�K�쐬 */
	case F_GREP_DIALOG:	/* Grep */
		/* �ҏW�E�B���h�E�̏���`�F�b�N */
		if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
			return FALSE;
		}else{
			return TRUE;
		}

	case F_FILESAVE:	/* �㏑���ۑ� */
		if( !pcEditDoc->m_bReadOnly ){	/* �ǂݎ���p���[�h */
			if( TRUE == pcEditDoc->IsModified() ){	/* �ύX�t���O */
				return TRUE;
			}else{
				/* ���ύX�ł��㏑�����邩 */
				if( FALSE == pShareData->m_Common.m_bEnableUnmodifiedOverwrite ){
					return FALSE;
				}else{
					return TRUE;
				}
			}
		}else{
			return FALSE;
		}
	case F_COPYLINES:				//�I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE:		//�I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER:	//�I��͈͓��S�s�s�ԍ��t���R�s�[
		if( pcEditDoc->IsTextSelected( ) ){/* �e�L�X�g���I������Ă��邩 */
			return TRUE;
		}else{
			return FALSE;
		}

	case F_TOLOWER:					/* �p�啶�����p������ */
	case F_TOUPPER:					/* �p���������p�啶�� */
	case F_TOHANKAKU:				/* �S�p�����p */
	case F_TOHANKATA:				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	case F_TOZENEI:					/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOHANEI:					/* �S�p�p�������p�p�� */
	case F_TOZENKAKUKATA:			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA:			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA:	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA:	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TABTOSPACE:				/* TAB���� */
	case F_SPACETOTAB:				/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_CODECNV_EMAIL:			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	case F_CODECNV_EUC2SJIS:		/* EUC��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODE2SJIS:	/* Unicode��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODEBE2SJIS:	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF82SJIS:		/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:		/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:		/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC:		/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:		/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:		/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_BASE64DECODE:			/* Base64�f�R�[�h���ĕۑ� */
	case F_UUDECODE:				//uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

		/* �e�L�X�g���I������Ă��邩 */
		if( pcEditDoc->IsTextSelected( ) ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_SELECTWORD:	/* ���݈ʒu�̒P��I�� */
	case F_CUT_LINE:	//�s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE:	//�s�폜(�܂�Ԃ��P��)
		/* �e�L�X�g���I������Ă��邩 */
		if( pcEditDoc->IsTextSelected( ) ){
			return FALSE;
		}else{
			return TRUE;
		}
	case F_UNDO:
		/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
		if( pcEditDoc->IsEnableUndo() ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_REDO:
		/* Redo(��蒼��)�\�ȏ�Ԃ��H */
		if( pcEditDoc->IsEnableRedo() ){
			return TRUE;
		}else{
			return FALSE;
		}

	case F_COPYPATH:
	case F_COPYTAG:
	case F_COPYFNAME:					// 2002/2/3 aroka
	case F_OPEN_HfromtoC:				//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	case F_OPEN_HHPP:					//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	case F_OPEN_CCPP:					//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	case F_PLSQL_COMPILE_ON_SQLPLUS:	/* Oracle SQL*Plus�Ŏ��s */
	case F_BROWSE:						//�u���E�Y
	//case F_READONLY:					//�ǂݎ���p	//	Sep. 10, 2002 genta ��Ɏg����悤��
	case F_PROPERTY_FILE:
		/* ���ݕҏW���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[�ł��邩 */
		if( pcEditDoc->IsFilePathAvailable() ){
			return TRUE;
		}else{
			return FALSE;
		}
	case F_JUMPHIST_PREV:	//	�ړ�����: �O��
		if( pcEditDoc->ActiveView().m_cHistory->CheckPrev() )
			return TRUE;
		else
			return FALSE;
	case F_JUMPHIST_NEXT:	//	�ړ�����: ����
		if( pcEditDoc->ActiveView().m_cHistory->CheckNext() )
			return TRUE;
		else
			return FALSE;
	case F_JUMPHIST_SET:	//	���݈ʒu���ړ������ɓo�^
		return TRUE;
	case F_DIRECT_TAGJUMP:	//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.15 MIK
	//	2003.05.12 MIK �^�O�t�@�C���쐬���I�ׂ�悤�ɂ����̂ŁA��ɍ쐬�\�Ƃ���
//	case F_TAGS_MAKE:	//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
		if( pcEditDoc->IsFilePathAvailable() ){
			return TRUE;
		}else{
			return FALSE;
		}
		
	}
	return TRUE;
}

//#ifdef _DEBUG
	/* �f�o�b�O���j�^���[�h�ɐݒ� */
	void CEditWnd::SetDebugModeON( void )
	{
		if( NULL != m_pShareData->m_hwndDebug ){
			if( CShareData::IsEditWnd( m_pShareData->m_hwndDebug ) ){
				return;
			}
		}
		m_pShareData->m_hwndDebug = m_hWnd;
		m_cEditDoc.m_bDebugMode = TRUE;

// 2001/06/23 N.Nakatani �A�E�g�v�b�g���ւ̏o�̓e�L�X�g�̒ǉ�F_ADDTAIL���}�~�����̂łƂ肠�����ǂݎ���p���[�h�͎��߂܂���
		m_cEditDoc.m_bReadOnly = FALSE;		/* �ǂݎ���p���[�h */
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		m_cEditDoc.SetParentCaption();
		return;
	}
//#endif


/* �@�\ID�ɑΉ�����w���v�R���e�L�X�g�ԍ���Ԃ� */
// Modified by Stonee, 2001/02/23
// etc_uty.cpp���ɒ��g���ړ�
/*	2002/04/17 YAZAKI �R�����g�A�E�g
int CEditWnd::FuncID_To_HelpContextID( int nFuncID )
{
	return ::FuncID_To_HelpContextID(nFuncID);
}
*/

/* ���j���[�A�C�e���ɑΉ�����w���v��\�� */
void CEditWnd::OnHelp_MenuItem( HWND hwndParent, int nFuncID )
{
	char	szHelpFile[_MAX_PATH + 1];
	int		nHelpContextID;

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( szHelpFile );

	/* �@�\ID�ɑΉ�����w���v�R���e�L�X�g�ԍ���Ԃ� */
	nHelpContextID = FuncID_To_HelpContextID( nFuncID );
	if( 0 != nHelpContextID ){
		::WinHelp( hwndParent, szHelpFile, HELP_CONTEXT, nHelpContextID );
	}
	return;
}


//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX

/* ����v���r���[���[�h�̃I��/�I�t */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;

	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	����v���r���[���[�h���������܂��B	*/
		delete m_pPrintPreview;	//	�폜�B
		m_pPrintPreview = NULL;	//	NULL���ۂ��ŁA�v�����g�v���r���[���[�h�����f���邽�߁B

		/*	�ʏ탂�[�h�ɖ߂�	*/
		::ShowWindow( m_cEditDoc.m_hWnd, SW_SHOW );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
		::ShowWindow( m_hwndStatusBar, SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_SHOW );

		::SetFocus( m_hWnd );

		hMenu = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( m_hWnd, hMenu );
		::DrawMenuBar( m_hWnd );

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	�ʏ탂�[�h���B��	*/
		hMenu = ::GetMenu( m_hWnd );
		//	Jun. 18, 2001 genta Print Preview�ł̓��j���[���폜
		::SetMenu( m_hWnd, NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( m_hWnd );

		::ShowWindow( m_cEditDoc.m_hWnd, SW_HIDE );
		::ShowWindow( m_hwndToolBar, SW_HIDE );
		::ShowWindow( m_hwndStatusBar, SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_HIDE );

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview = new CPrintPreview( this );
		/* ���݂̈���ݒ� */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	�v�����^�̏����擾�B

		/* ���݂̃f�t�H���g�v�����^�̏����擾 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( FALSE == bRes ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
				"����v���r���[�����s����O�ɁA�v�����^���C���X�g�[�����Ă��������B\n"
			);
			return;
		}

		/* ����ݒ�̔��f */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( m_hWnd, NULL, TRUE );
		::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}




/* WM_SIZE ���� */
LRESULT CEditWnd::OnSize( WPARAM wParam, LPARAM lParam )
{
	int			cx;
	int			cy;
	int			nToolBarHeight;
	int			nStatusBarHeight;
	int			nFuncKeyWndHeight;
	RECT		rc, rcClient;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��
//	�ϐ��폜

	RECT		rcWin;



	cx = LOWORD( lParam );
	cy = HIWORD( lParam );

	/* �E�B���h�E�T�C�Y�p�� */
	if( wParam != SIZE_MINIMIZED &&						/* �ŏ����͌p�����Ȃ� */
		m_pShareData->m_Common.m_bSaveWindowSize		/* �E�B���h�E�T�C�Y�p�������邩 */
	){
		if( wParam == SIZE_MAXIMIZED ){					/* �ő剻�̓T�C�Y���L�^���Ȃ� */
			if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ){
				m_pShareData->m_Common.m_nWinSizeType = wParam;
			}
		}else{
			::GetWindowRect( m_hWnd, &rcWin );
			/* �E�B���h�E�T�C�Y�Ɋւ���f�[�^���ύX���ꂽ�� */
			if( m_pShareData->m_Common.m_nWinSizeType != (int)wParam ||
				m_pShareData->m_Common.m_nWinSizeCX != rcWin.right - rcWin.left ||
				m_pShareData->m_Common.m_nWinSizeCY != rcWin.bottom - rcWin.top
			){
				m_pShareData->m_Common.m_nWinSizeType = wParam;
				m_pShareData->m_Common.m_nWinSizeCX = rcWin.right - rcWin.left;
				m_pShareData->m_Common.m_nWinSizeCY = rcWin.bottom - rcWin.top;
			}
		}
	}

	m_nWinSizeType = wParam;	/* �T�C�Y�ύX�̃^�C�v */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );


	nToolBarHeight = 0;
	if( NULL != m_hwndToolBar ){
		::SendMessage( m_hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		::SendMessage( m_CFuncKeyWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.m_hWnd, &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	nStatusBarHeight = 0;
	if( NULL != m_hwndStatusBar ){
		::SendMessage( m_hwndStatusBar, WM_SIZE, wParam, lParam );
		::GetClientRect( m_hwndStatusBar, &rc );
		//	May 12, 2000 genta
		//	2�J�����ڂɉ��s�R�[�h�̕\����}��
		//	From Here
		int			nStArr[8];
		const char*	pszLabel[7] = { "", "999999�s 99999��", "RR0RR0","FFFFFFFF", "SJIS  ", "REC", "�㏑" };	//Oct. 30, 2000 JEPRO �疜�s���v���
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth;
		SIZE		sz;
		HDC			hdc;
		int			i;
		nAllWidth = rc.right - rc.left;
		hdc = ::GetDC( m_hwndStatusBar );
		nStArr[nStArrNum - 1] = nAllWidth;
		if( wParam != SIZE_MAXIMIZED ){
			nStArr[nStArrNum - 1] -= 16;
		}
		for( i = nStArrNum - 1; i > 0; i-- ){
			::GetTextExtentPoint32( hdc, pszLabel[i], lstrlen( pszLabel[i] ), &sz );
			nStArr[i - 1] = nStArr[i] - ( sz.cx + ::GetSystemMetrics( SM_CXEDGE ) );
		}
		nStArr[0] += 16;	//Nov. 2, 2000 JEPRO �悭�킩��Ȃ����Ƃ����������𑝂₵�Ă݂�

		::SendMessage( m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
		::ReleaseDC( m_hwndStatusBar, hdc );


		::GetWindowRect( m_hwndStatusBar, &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_hWnd, &rcClient );

	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_cEditDoc.OnMove(
			0,
			nToolBarHeight + nFuncKeyWndHeight,
			cx,
			cy - nToolBarHeight - nFuncKeyWndHeight - nStatusBarHeight
		);
		::MoveWindow( m_CFuncKeyWnd.m_hWnd, 0, nToolBarHeight, cx, nFuncKeyWndHeight, TRUE );
	}else
	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 1 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_cEditDoc.OnMove( 0, nToolBarHeight, cx, cy - nToolBarHeight - nFuncKeyWndHeight - nStatusBarHeight );
		::MoveWindow(
			m_CFuncKeyWnd.m_hWnd,
			0,
			cy - nFuncKeyWndHeight - nStatusBarHeight,
			cx,
			nFuncKeyWndHeight, TRUE
		);
		BOOL	bSizeBox = TRUE;
		if( NULL != m_hwndStatusBar ){
			bSizeBox = FALSE;
		}
		if( wParam == SIZE_MAXIMIZED ){
			bSizeBox = FALSE;
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if( !m_pPrintPreview ){
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}




/* WM_PAINT �`�揈�� */
LRESULT CEditWnd::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	HDC				hdc;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		PAINTSTRUCT		ps;
		hdc = ::BeginPaint( hwnd, &ps );
		::EndPaint( hwnd, &ps );
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

/* ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL */
LRESULT CEditWnd::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		return 0;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}




/* ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� */
LRESULT CEditWnd::OnHScroll( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		return 0;
	}
	return m_pPrintPreview->OnHScroll( wParam, lParam );
}

LRESULT CEditWnd::OnLButtonDown( WPARAM wParam, LPARAM lParam )
{
	//by �S(2) �L���v�`���[���ĉ����ꂽ���N���C�A���g�ł��������ɗ���
	if(m_IconClicked != icNone)
		return 0;

	WPARAM		fwKeys;
	int			xPos;
	int			yPos;
	fwKeys = wParam;		// key flags
	xPos = LOWORD(lParam);	// horizontal position of cursor
	yPos = HIWORD(lParam);	// vertical position of cursor
	m_nDragPosOrgX = xPos;
	m_nDragPosOrgY = yPos;
	SetCapture( m_hWnd );
	m_bDragMode = TRUE;

	return 0;
}
#define IDT_SYSMENU 1357
LRESULT CEditWnd::OnLButtonUp( WPARAM wParam, LPARAM lParam )
{
	//by �S 2002/04/18
	if(m_IconClicked != icNone)
	{
		if(m_IconClicked == icDown)
		{
			m_IconClicked = icClicked;
			//by �S(2) �^�C�}�[(ID�͓K���ł�)
			SetTimer(m_hWnd, IDT_SYSMENU, GetDoubleClickTime(), SysMenuTimer);
		}
		return 0;
	}

	m_bDragMode = FALSE;
	ReleaseCapture();
	::InvalidateRect( m_hWnd, NULL, TRUE );
	return 0;
}


LRESULT CEditWnd::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	//by �S
	if(m_IconClicked != icNone)
	{
		//by �S(2) ��񉟂��ꂽ������
		if(m_IconClicked == icDown)
		{
			POINT P;
			GetCursorPos(&P); //�X�N���[�����W
			if(SendMessage(m_hWnd, WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
			{
				ReleaseCapture();
				m_IconClicked = icNone;

				if(m_cEditDoc.IsFilePathAvailable())
				{
					const char *PathEnd = m_cEditDoc.GetFilePath();
					for(const char* I = m_cEditDoc.GetFilePath(); *I != 0; ++I)
					{
						//by �S(2): DBCS����
						if(IsDBCSLeadByte(*I))
							++I;
						else if(*I == '\\')
							PathEnd = I;
					}

					wchar_t WPath[MAX_PATH];
					int c = MultiByteToWideChar(CP_ACP, 0, m_cEditDoc.GetFilePath(), PathEnd - m_cEditDoc.GetFilePath(), WPath, MAX_PATH);
					WPath[c] = 0;
					wchar_t WFile[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, PathEnd + 1, -1, WFile, MAX_PATH);

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if(SUCCEEDED(Desktop->ParseDisplayName(0, NULL, WPath, &Eaten, &PathID, &Attribs)))
					{
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if(SUCCEEDED(Folder->ParseDisplayName(0, NULL, WFile, &Eaten, &ItemID, &Attribs)))
						{
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							//�e�L�X�g�ł���������c�֗�
							int Len = lstrlen(m_cEditDoc.GetFilePath());
							FORMATETC F;
							STGMEDIUM M;

							F.cfFormat = CF_TEXT;
							F.ptd      = NULL;
							F.dwAspect = DVASPECT_CONTENT;
							F.lindex   = -1;
							F.tymed    = TYMED_HGLOBAL;

							M.tymed = TYMED_HGLOBAL;
							M.pUnkForRelease = NULL;
							M.hGlobal = GlobalAlloc(GMEM_MOVEABLE, Len+1);

							void* P = GlobalLock(M.hGlobal);
							CopyMemory(P, m_cEditDoc.GetFilePath(), Len+1);
							GlobalUnlock(M.hGlobal);

							DataObject->SetData(&F, &M, TRUE);
#endif
							//�ړ��͋֎~
							DWORD R;
							DoDragDrop(DataObject, new CDropSource(TRUE), DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
							DataObject->Release();
						}
						Folder->Release();
					}
					Desktop->Release();
					Malloc->Release();
				}
			}
		}
		return 0;
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if (!m_pPrintPreview){
		return 0;
	}
	else {
		return m_pPrintPreview->OnMouseMove( wParam, lParam );
	}
}




LRESULT CEditWnd::OnMouseWheel( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		/* ���b�Z�[�W�̔z�� */
		return m_cEditDoc.DispatchEvent( m_hWnd, WM_MOUSEWHEEL, wParam, lParam );
	}
	return m_pPrintPreview->OnMouseWheel( wParam, lParam );
}

/* ����y�[�W�ݒ�
	����v���r���[���ɂ��A�����łȂ��Ƃ��ł��Ă΂��\��������B
*/
BOOL CEditWnd::OnPrintPageSetting( void )
{
	/* ����ݒ�iCANCEL�������Ƃ��ɔj�����邽�߂̗̈�j */
	CDlgPrintSetting	cDlgPrintSetting;
	BOOL				bRes;
	PRINTSETTING		PrintSettingArr[MAX_PRINTSETTINGARR];
	int					i;
	int					nCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[i];
	}

//	cDlgPrintSetting.Create( m_hInstance, m_hWnd );
	nCurrentPrintSetting = m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		m_hInstance,
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_hWnd,
		&nCurrentPrintSetting, /* ���ݑI�����Ă������ݒ� */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* ���ݑI������Ă���y�[�W�ݒ�̔ԍ����ύX���ꂽ�� */
		if( nCurrentPrintSetting !=
			m_pShareData->m_Types[m_cEditDoc.GetDocumentType()].m_nCurrentPrintSetting
		){
//			/* �ύX�t���O(�^�C�v�ʐݒ�) */
			m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

//		/* ���ꂼ��̃y�[�W�ݒ�̓��e���ύX���ꂽ�� */
//		if( 0 != memcmp(
//			&m_pShareData->m_PrintSettingArr,
//			&PrintSettingArr,
//			sizeof( m_pShareData->m_PrintSettingArr ) )
//		){
//			m_pShareData->m_bPrintSettingModify = TRUE;	/* �ύX�t���O(����̑S��) */
			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//				if( 0 != memcmp(
//					&m_pShareData->m_PrintSettingArr[i],
//					&PrintSettingArr[i],
//					sizeof( m_pShareData->m_PrintSettingArr[i] ) )
//				){
//					/* �ύX�t���O(����ݒ育��) */
//					m_pShareData->m_bPrintSettingModifyArr[i] = TRUE;
					m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
//				}
			}
//		}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		//	����v���r���[���̂݁B
		if ( m_pPrintPreview ){
			/* ���݂̈���ݒ� */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[m_cEditDoc.GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* ����v���r���[ �X�N���[���o�[������ */
			m_pPrintPreview->InitPreviewScrollBar();

			/* ����ݒ�̔��f */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( m_hWnd, NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by �S

LRESULT CEditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(wp == HTSYSMENU)
	{
		SetCapture(m_hWnd);
		m_IconClicked = icDown;
		Result = 0;
	}
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT CEditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		//�O�̂���
		ReleaseCapture();
		m_IconClicked = icNone;
		Result = 0;
	}
	else if(wp == HTSYSMENU)
		Result = 0;
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT CEditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) //by �S(2)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		ReleaseCapture();
		m_IconClicked = icDoubleClicked;

		SendMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}
	else
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

void CEditWnd::OnSysMenuTimer() //by �S(2)
{
	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//�V�X�e�����j���[�\��
		HMENU SysMenu = GetSystemMenu(m_hWnd, FALSE);
		RECT R;
		GetWindowRect(m_hWnd, &R);
		DWORD Cmd = TrackPopupMenu(SysMenu, TPM_RETURNCMD | TPM_LEFTBUTTON |
						TPM_LEFTALIGN | TPM_TOPALIGN,
						(R.left + GetSystemMetrics(SM_CXFRAME) > 0)? R.left + GetSystemMetrics(SM_CXFRAME) : 0,
						R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME),
						0, m_hWnd, NULL);
		if(Cmd != 0)
			SendMessage(m_hWnd, WM_SYSCOMMAND, Cmd, 0);
	}
	m_IconClicked = icNone;
}

/*! �h���b�v�_�E�����j���[(�J��) */	//@@@ 2002.06.15 MIK
int	CEditWnd::CreateFileDropDownMenu( HWND hwnd )
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;

	m_CMenuDrawer.ResetContents();

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
	CMRU cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�ŋߎg�����t�H���_(&D)" );
	}
	else 
	{
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, "�ŋߎg�����t�H���_(&D)" );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "�V�K�쐬(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "�J��(&O)...", FALSE );

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );

	rc.left   = 0;
	rc.right  = 0;
	rc.top    = 0;
	rc.bottom = 0;
	::GetWindowRect( hwnd, &rc );
	po.x = po.x /*rc.left*/;
	po.y = rc.bottom - 2;

	rc.left   = 0;
	rc.right  = 0;
	rc.top    = 0;
	rc.bottom = 0;

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		hwnd,
		&rc
	);

	::DestroyMenu( hMenu );

	return nId;
}

/*! �����{�b�N�X�ł̏��� */
void CEditWnd::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( (TCHAR)msg->wParam == VK_RETURN )  //���^�[���L�[
		{
			//�����L�[���[�h���擾
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );

				//::SetFocus( m_hWnd );	//��Ƀt�H�[�J�X���ړ����Ă����Ȃ��ƃL�����b�g��������
				::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );

				// �����J�n���̃J�[�\���ʒu�o�^������ύX 02/07/28 ai start
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosX_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosX_PHY;
				m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nSrchStartPosY_PHY = m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_nCaretPosY_PHY;
				// 02/07/28 ai end

				//��������
				OnCommand( (WORD)0 /*���j���[*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( (TCHAR)msg->wParam == VK_TAB )	//�^�u�L�[
		{
			//�t�H�[�J�X���ړ�
			::SetFocus( m_cEditDoc.m_cEditViewArr[m_cEditDoc.m_nActivePaneIndex].m_hWnd );
		}
	}

	return;
}

/*!
	@brief �E�B���h�E�̃A�C�R���ݒ�

	�w�肳�ꂽ�A�C�R�����E�B���h�E�ɐݒ肷��D
	�ȑO�̃A�C�R���͔j������D

	@param hIcon [in] �ݒ肷��A�C�R��
	@param flag [in] �A�C�R����ʁDICON_BIG�܂���ICON_SMALL.
	@author genta
	@date 2002.09.10
*/
void CEditWnd::SetWindowIcon(HICON hIcon, int flag)
{
	HICON hOld = (HICON)::SendMessage( m_hWnd, WM_SETICON, flag, (LPARAM)hIcon );
	if( hOld != NULL ){
		::DestroyIcon( hOld );
	}
}

/*!
	�W���A�C�R���̎擾

	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta �V�݂������ʊ֐����g���悤��
*/
void CEditWnd::GetDefaultIcon( HICON& hIconBig, HICON& hIconSmall ) const
{
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
}

/*!
	�A�C�R���̎擾
	
	�w�肳�ꂽ�t�@�C�����ɑΉ�����A�C�R��(��E��)���擾���ĕԂ��D
	
	@param szFile     [in] �t�@�C����
	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��
	
	@retval true �֘A�Â���ꂽ�A�C�R������������
	@retval false �֘A�Â���ꂽ�A�C�R����������Ȃ�����
	
	@author genta
	@date 2002.09.10
*/
bool CEditWnd::GetRelatedIcon(const char* szFile, HICON& hIconBig, HICON& hIconSmall) const
{
	if( NULL != szFile && szFile[0] != '\0' ){
		char szExt[_MAX_EXT];
		char FileType[1024];

		// (.�Ŏn�܂�)�g���q�̎擾
		_splitpath( szFile, NULL, NULL, NULL, szExt );
		
		if( ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, sizeof(FileType) - 13)){
			lstrcat( FileType, "\\DefaultIcon" );
			if( ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)){
				// �֘A�Â���ꂽ�A�C�R�����擾����
				SHFILEINFO shfi;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON );
				hIconBig = shfi.hIcon;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON );
				hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	//	�W���̃A�C�R����Ԃ�
	GetDefaultIcon( hIconBig, hIconSmall );
	return false;
}

/*
	@brief ���j���[�o�[�\���p�t�H���g�̏�����
	
	���j���[�o�[�\���p�t�H���g�̏��������s���D
	
	@date 2002.12.04 CEditView�̃R���X�g���N�^����ړ�
*/
void CEditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	/* LOGFONT�̏����� */
	memset( &lf, 0, sizeof( LOGFONT ) );
	lf.lfHeight			= -12;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	strcpy( lf.lfFaceName, "�l�r �S�V�b�N" );
	m_hFontCaretPosInfo = ::CreateFontIndirect( &lf );

	hdc = ::GetDC( ::GetDesktopWindow() );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	::GetTextMetrics( hdc, &tm );
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	for( i = 0; i < ( sizeof( m_pnCaretPosInfoDx ) / sizeof( m_pnCaretPosInfoDx[0] ) ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( ::GetDesktopWindow(), hdc );
}

/*
	@brief ���j���[�o�[�Ƀ��b�Z�[�W��\������
	
	���O�Ƀ��j���[�o�[�\���p�t�H���g������������Ă��Ȃ��Ă͂Ȃ�Ȃ��D
	�w��ł��镶�����͍ő�30�o�C�g�D����ȏ�̏ꍇ�͂����؂��ĕ\������D
	
	@author genta
	@date 2002.12.04
*/
void CEditWnd::PrintMenubarMessage( const char* msg )
{
	HDC		hdc;
	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;
	const int MAX_LEN = 30;
	char	szText[MAX_LEN + 1];
	
	int len = strlen( msg );
	strncpy( szText, msg, MAX_LEN );
	
	if( len < MAX_LEN ){
		memset( szText + len, ' ', MAX_LEN - len );
	}
	szText[MAX_LEN] = '\0';
	
	hdc = ::GetWindowDC( m_hWnd );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( m_hWnd, &poFrame );
	::GetWindowRect( m_hWnd, &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - 5;
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	::SetBkColor( hdc, ::GetSysColor( COLOR_MENU ) );
	::ExtTextOut( hdc,rc.left,rc.top,ETO_OPAQUE,&rc,szText,nStrLen,m_pnCaretPosInfoDx);
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	�X�e�[�^�X�o�[����\���̏ꍇ�̓��j���[�o�[�̉E�[�ɕ\������D
	
	@param msg [in] �\�����郁�b�Z�[�W
	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta CEditView���ړ�
*/
void CEditWnd::SendStatusMessage( const char* msg )
{
	if( NULL == m_hwndStatusBar ){
		// ���j���[�o�[��
		PrintMenubarMessage( msg );
	}else{
		// �X�e�[�^�X�o�[��
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}
/*[EOF]*/
