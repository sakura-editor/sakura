//	$Id$
/*!	@file
	@brief �풓��
	
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2001/06/03 N.Nakatani grep�P��P�ʂŌ�������������Ƃ��̂��߂ɃR�}���h���C���I�v�V�����̏����ǉ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, stonee, jepro, genta, aroka, hor

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#define ID_HOTKEY_TRAYMENU	0x1234

#include <windows.h>
//#include <stdio.h>
#include <io.h>
//#include <ras.h>
#include "debug.h"
#include "CEditApp.h"
#include "CEditView.h"		//Nov. 21, 2000 JEPROtest
#include "CEditDoc.h"		//Nov. 21, 2000 JEPROtest
#include "CEditWnd.h"		//Nov. 21, 2000 JEPROtest
#include "CDlgAbout.h"		//Nov. 21, 2000 JEPROtest
#include "CDlgTypeList.h"	//Nov. 21, 2000 JEPROtest
#include "sakura_rc.h"
#include "mymessage.h"
#include "CDlgOpenFile.h"
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include <htmlhelp.h>


/////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CEditAppWndProc( HWND, UINT, WPARAM, LPARAM );


CEditApp*	g_m_pCEditApp;

//Stonee, 2001/03/21
//Stonee, 2001/07/01  ���d�N�����ꂽ�ꍇ�͑O��̃_�C�A���O��O�ʂɏo���悤�ɂ����B
void CEditApp::DoGrep()
{
//	CDlgGrep	cDlgGrep;	// Jul. 2, 2001 genta
	char*			pCmdLine;
	char*			pOpt;
  	int				nDataLen;
	int				nRet;
	CMemory			cmWork1;
	CMemory			cmWork2;
	CMemory			cmWork3;

	/* Grep */
	/*Grep�_�C�A���O�̏������P */
//	cDlgGrep.Create( m_hInstance, /*m_hWnd*/NULL );

	//Stonee, 2001/06/30
	//�O��̃_�C�A���O������ΑO�ʂ� (suggested by genta)
	if ( ::IsWindow(m_cDlgGrep.m_hWnd) ){
		::OpenIcon(m_cDlgGrep.m_hWnd);
		::BringWindowToTop(m_cDlgGrep.m_hWnd);
		return;
	}

	strcpy( m_cDlgGrep.m_szText, m_pShareData->m_szSEARCHKEYArr[0] );

	/* Grep�_�C�A���O�̕\�� */
	nRet = m_cDlgGrep.DoModal( m_hInstance, /*m_hWnd*/NULL, "" );
//					MYTRACE( "nRet=%d\n", nRet );
	if( FALSE == nRet || m_hWnd == NULL ){
		return;
	}

//					MYTRACE( "cDlgGrep.m_szText  =[%s]\n", cDlgGrep.m_szText );
//					MYTRACE( "cDlgGrep.m_szFile  =[%s]\n", cDlgGrep.m_szFile );
//					MYTRACE( "cDlgGrep.m_szFolder=[%s]\n", cDlgGrep.m_szFolder );

	/*======= Grep�̎��s =============*/
	/* Grep���ʃE�B���h�E�̕\�� */

	pCmdLine = new char[1024];
	pOpt = new char[64];

	cmWork1.SetDataSz( m_cDlgGrep.m_szText );
	cmWork2.SetDataSz( m_cDlgGrep.m_szFile );
	cmWork3.SetDataSz( m_cDlgGrep.m_szFolder );
	cmWork1.Replace( "\"", "\"\"" );
	cmWork2.Replace( "\"", "\"\"" );
	cmWork3.Replace( "\"", "\"\"" );

	/*
	|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GOPT=S
	*/
	wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\"" ,
		cmWork1.GetPtr( &nDataLen ),
		cmWork2.GetPtr( &nDataLen ),
		cmWork3.GetPtr( &nDataLen )
	);

	pOpt[0] = '\0';
	if( m_cDlgGrep.m_bSubFolder ){			/* �T�u�t�H���_������������� */
		strcat( pOpt, "S" );
	}

	if( m_cDlgGrep.m_bLoHiCase ){				/* �p�啶���Ɖp����������ʂ��� */
		strcat( pOpt, "L" );
	}
	if( m_cDlgGrep.m_bRegularExp ){			/* ���K�\�� */
		strcat( pOpt, "R" );
	}
	if( m_cDlgGrep.m_bKanjiCode_AutoDetect ){	/* �����R�[�h�������� */
		strcat( pOpt, "K" );
	}
	if( m_cDlgGrep.m_bGrepOutputLine ){		/* �s���o�͂��邩�Y�����������o�͂��邩 */
		strcat( pOpt, "P" );
	}
	if( m_cDlgGrep.m_bWordOnly ){				/* �P��P�ʂŒT�� */
		strcat( pOpt, "W" );
	}
	if( 1 == m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
		strcat( pOpt, "1" );
	}
	if( 2 == m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
		strcat( pOpt, "2" );
	}


	if( 0 < lstrlen( pOpt ) ){
		strcat( pCmdLine, " -GOPT=" );
		strcat( pCmdLine, pOpt );
	}

	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
	CEditApp::OpenNewEditor( m_hInstance, m_pShareData->m_hwndTray, pCmdLine, 0, FALSE );

	delete [] pCmdLine;
	delete [] pOpt;
}

//	BOOL CALLBACK ExitingDlgProc(
//	  HWND hwndDlg,	// handle to dialog box
//	  UINT uMsg,	// message
//	  WPARAM wParam,// first message parameter
//	  LPARAM lParam	// second message parameter
//	)
//	{
//		switch( uMsg ){
//		case WM_INITDIALOG:
//			return TRUE;
//		}
//		return FALSE;
//	}


/* �E�B���h�E�v���V�[�W������ */
LRESULT CALLBACK CEditAppWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CEditApp* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CEditApp* )g_m_pCEditApp;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pSApp = ( CEditApp* )::GetWindowLong( hwnd, GWL_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CEditApp

CEditApp::CEditApp() :
	//	Apr. 24, 2001 genta
	m_uCreateTaskBarMsg( ::RegisterWindowMessage( TEXT("TaskbarCreated") ) ),
	m_bCreatedTrayIcon( FALSE ),	//�g���C�ɃA�C�R���������
	m_hInstance( NULL ),
	m_hWnd( NULL )
{
//	m_hAccel		= NULL;
//	m_nEditArrNum	= 0;
//	m_pEditArr		= NULL;
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
	if( m_pShareData->m_hAccel != NULL ){
		::DestroyAcceleratorTable( m_pShareData->m_hAccel );
		m_pShareData->m_hAccel = NULL;
	}
	m_pShareData->m_hAccel =
		CKeyBind::CreateAccerelator(
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr
		);
	if( NULL == m_pShareData->m_hAccel ){
		::MessageBox( NULL, "CEditApp::CEditApp()\n�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n�V�X�e�����\�[�X���s�����Ă��܂��B", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
	}

//	#ifdef _DEBUG
//		m_pszAppName = "CEditApp[�f�o�b�O�o�[�W����]";
//	#endif
//	#ifndef _DEBUG
		m_pszAppName = GSTR_CEDITAPP;
//	#endif

	return;
}


CEditApp::~CEditApp()
{
//	if( NULL != m_pEditArr ){
//		free( m_pEditArr );
//		m_pEditArr = NULL;
//		m_nEditArrNum = 0;
//	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CEditApp �����o�֐�




/* �쐬 */
HWND CEditApp::Create( HINSTANCE hInstance )
{
	WNDCLASS	wc;
//	HANDLE		hMutex;
	HWND		hWnd;
//	MSG			msg;
	ATOM		atom;
//	char		szMutexName[260];
//	HICON		hIcon;
//	HACCEL		hAccel;
//	int			nRet;
	HWND		hwndWork;
//	const char *pszTitle="sakura new UR1.2.20.0";	//Nov. 12, 2000 JEPROtestnow �g���C�Ƀo�[�W�������\������Ȃ��Ȃ��Ă��܂����̂ŏC�����Ă݂������s
//	const char *pszTips	="sakura tipsUR1.2.20.0";	//Nov. 12, 2000 JEPROtestnow �g���C�Ƀo�[�W�������\������Ȃ��Ȃ��Ă��܂����̂ŏC�����Ă݂������s

	m_hInstance = hInstance;
	hwndWork = ::FindWindow( m_pszAppName, m_pszAppName );
	if( NULL != hwndWork ){
		return NULL;
	}



//	wsprintf( szMutexName, "%sIsAlreadyExist", m_pszAppName );
//	hMutex = CreateMutex( NULL, TRUE, szMutexName );
//	if( ERROR_ALREADY_EXISTS != GetLastError() ){
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= CEditAppWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= m_pszAppName;
		atom = RegisterClass( &wc );
//	}else{
//		return NULL;
//	}
	g_m_pCEditApp = this;
	hWnd = ::CreateWindow(
		m_pszAppName,						// pointer to registered class name
//		pszTitle,							// pointer to registered class name	//Nov. 12, 2000 JEPROtestnow �g���C�Ƀo�[�W�������\������Ȃ��Ȃ��Ă��܂����̂ŏC�����Ă݂������s
		m_pszAppName,						// pointer to window name
//		pszTips,							// pointer to window name			//Nov. 12, 2000 JEPROtestnow �g���C�Ƀo�[�W�������\������Ȃ��Ȃ��Ă��܂����̂ŏC�����Ă݂������s
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		m_hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);
//	m_hWnd = hWnd;
//	MYMESSAGEBOX( "m_hWnd=%08xH \n", m_hWnd );

	//	Oct. 16, 2000 genta
	m_hIcons.Create( m_hInstance, m_hWnd );
	m_CMenuDrawer.Create( m_hInstance, m_hWnd, &m_hIcons );

	if( NULL != m_hWnd ){
		CreateTrayIcon( m_hWnd );
	}else{
	}
//	::ShowWindow( m_hWnd, SW_SHOW );

	/* Windows �A�N�Z�����[�^�̍쐬 */
//	m_CKeyBind.Create( m_hInstance );
//	m_hAccel = m_CKeyBind.CreateAccerelator();
	return m_hWnd;
}

//! �^�X�N�g���C�ɃA�C�R����o�^����
bool CEditApp::CreateTrayIcon( HWND hWnd )
{
	HICON hIcon;
//		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );
		/* �^�X�N�g���C�̃A�C�R������� */
		if( TRUE == m_pShareData->m_Common.m_bUseTaskTray ){	/* �^�X�N�g���C�̃A�C�R�����g�� */
#ifdef _DEBUG
//			hIcon =	::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
			hIcon=(HICON)::LoadImage(m_hInstance,MAKEINTRESOURCE(IDI_ICON_DEBUG),IMAGE_ICON,16,16,0);	//Jul. 02, 2001 JEPRO
#else
//			hIcon =	::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
			hIcon=(HICON)::LoadImage(m_hInstance,MAKEINTRESOURCE(IDI_ICON_STD),IMAGE_ICON,16,16,0);	//Jul. 02, 2001 JEPRO
#endif
//From Here Jan. 12, 2001 JEPRO �g���C�A�C�R���Ƀ|�C���g����ƃo�[�W����no.���\�������悤�ɏC��
//			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, GSTR_APPNAME );
			/* �o�[�W������� */
			//	UR version no.��ݒ� (cf. cDlgAbout.cpp)
			char	pszTips[64];
			char	pszTipsVerno[32];

			strcpy( pszTips, GSTR_APPNAME );
			wsprintf( pszTipsVerno, " %d.%d.%d.%d",		//Jul. 06, 2001 jepro UR �͂����t���Ȃ��Ȃ����̂�Y��Ă���
			HIWORD( m_pShareData->m_dwProductVersionMS ),
			LOWORD( m_pShareData->m_dwProductVersionMS ),
			HIWORD( m_pShareData->m_dwProductVersionLS ),
			LOWORD( m_pShareData->m_dwProductVersionLS )
			);
			strcat( pszTips, pszTipsVerno );
			TrayMessage( m_hWnd, NIM_ADD, 0,  hIcon, pszTips );
//To Here Jan. 12, 2001
			m_bCreatedTrayIcon = TRUE;	/* �g���C�ɃA�C�R��������� */
		}
	return true;
}




/* ���b�Z�[�W���[�v */
void CEditApp::MessageLoop( void )
{
//�����v���Z�X��
	MSG	msg;
	while ( m_hWnd != NULL && ::GetMessage(&msg, NULL, 0, 0 ) ){
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return;


//�V���O���v���Z�X��
//	MSG			msg;
//	MSG			msg2;
//	CEditWnd*	pCEditWnd;
//	char		szClassName[64];
//	BOOL		bFromEditWnd;
//	while ( m_hWnd != NULL && ::GetMessage(&msg, NULL/*m_hWnd*/, 0, 0 ) ){
//		bFromEditWnd = FALSE;
//		if( 0 != ::GetClassName( msg.hwnd, szClassName, sizeof(szClassName) - 1 ) ){
//			if( 0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
//				bFromEditWnd = TRUE;
//				pCEditWnd = ( CEditWnd* )::GetWindowLong( msg.hwnd, GWL_USERDATA );
//			}
//		}
//		if( bFromEditWnd ){
//			if( NULL != pCEditWnd->m_hwndPrintPreviewBar && ::IsDialogMessage( pCEditWnd->m_hwndPrintPreviewBar, &msg )  ){	/* ����v���r���[ ����o�[ */
//			}else{
//				if( NULL != pCEditWnd->m_pShareData->m_hAccel ){
//					msg2 = msg;
//					if( ::TranslateAccelerator( msg.hwnd, pCEditWnd->m_pShareData->m_hAccel, &msg ) ){
//					}else{
//						::TranslateMessage( &msg );
//						::DispatchMessage( &msg );
//					}
//				}else{
//					::TranslateMessage( &msg );
//					::DispatchMessage( &msg );
//				}
//			}
//		}else{
//			::TranslateMessage( &msg );
//			::DispatchMessage( &msg );
//		}
//	}
	return;
}






/* �^�X�N�g���C�̃A�C�R���Ɋւ��鏈�� */
BOOL CEditApp::TrayMessage( HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const char* pszTip )
{
	BOOL			res;
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof( NOTIFYICONDATA );
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if( pszTip ){
		lstrcpyn( tnd.szTip, pszTip, sizeof( tnd.szTip ) );
	}else{
		tnd.szTip[0] = '\0';
	}
	res = Shell_NotifyIcon( dwMessage, &tnd );
	if( hIcon ){
		DestroyIcon( hIcon );
	}
	return res;
}





/* ���b�Z�[�W���� */
LRESULT CEditApp::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	HMENU			hMenuTop;
//	HMENU			hMenu;
//	POINT			po;
	int				nId;
//	int				i;
//	int				j;
//	HGLOBAL			hgData;
//	char*			pData;
//	char			szMemu[300];
	HWND			hwndWork;
	//static CDlgGrep	cDlgGrep;  //Stonee, 2001/03/21 Grep�𑽏d�N�������Ƃ��G���[�ɂȂ�̂�Grep������ʊ֐��ɂ���
	//char*			pCmdLine;
	//char*			pOpt;
	CMemory			cmWork1;
	CMemory			cmWork2;
	CMemory			cmWork3;
	//int				nDataLen;
	//int				nRet;
	LPHELPINFO		lphi;
//	HWND			hwndExitingDlg;

//	CEditWnd*	pCEditWnd_Test;
//	int			i;
//	char*		pszCmdLine;
	int			nRowNum;
	EditNode*	pEditNodeArr;
//	HWND		hwndNew;
	static HWND	hwndHtmlHelp;
//	HWND		hwndHtmlHelpChild;



	int				idHotKey;
	UINT			fuModifiers;
	UINT			uVirtKey;
	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
	HWND			hwndFocused;
	char			szClassName[100];
	char			szText[256];
	UINT				idCtl;	/* �R���g���[����ID */
	MEASUREITEMSTRUCT*	lpmis;
//	char				szLabel[1024];
//	LPMEASUREITEMSTRUCT	lpmis;	/* ���ڃT�C�Y��� */
//	char*				pszwork;
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;

	switch ( uMsg ){
	case WM_MENUCHAR:
		/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );
	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* �R���g���[����ID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
			/* ���j���[�A�C�e���`�� */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
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

	/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
	case WM_HOTKEY:
		idHotKey = (int) wParam;				// identifier of hot key
		fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
		uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code

		hwndFocused = ::GetForegroundWindow();
		szClassName[0] = '\0';
		::GetClassName( hwndFocused, szClassName, sizeof( szClassName ) - 1 );
		::GetWindowText( hwndFocused, szText, sizeof( szText ) - 1 );
		if( 0 == strcmp( szText, "���ʐݒ�" ) ){
			return -1;
		}

		if( ID_HOTKEY_TRAYMENU == idHotKey
		 &&	( wHotKeyMods )  == fuModifiers
		 && wHotKeyCode == uVirtKey
		){
			::PostMessage( m_hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONDOWN );
		}
		return 0;

	case MYWM_HTMLHELP:
		{
			HH_AKLINK	link;
			char		szHtmlHelpFile[1024];
			int			nLen;
			int			nLenKey;
			char*		pszKey;
			HWND		hwndFrame;
			hwndFrame = (HWND)wParam;
			strcpy( szHtmlHelpFile, m_pShareData->m_szWork );
			nLen = lstrlen( szHtmlHelpFile );
			nLenKey = lstrlen( &m_pShareData->m_szWork[nLen + 1] );
			pszKey = new char[ nLenKey + 1 ];
			strcpy( pszKey, &m_pShareData->m_szWork[nLen + 1] );

//			if( NULL != hwndHtmlHelp ){
//				::BringWindowToTop( hwndHtmlHelp );
//			}

			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD)0,
				true
			);

			link.cbStruct		= sizeof(HH_AKLINK);
			link.fReserved		= FALSE;
			link.pszKeywords	= (char*)pszKey;
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;

//			if( NULL != hwndHtmlHelp ){
//				hwndHtmlHelpChild = ::FindWindow( "#32770", "Topics Found"/*NULL*/ );
//				if( NULL != hwndHtmlHelpChild ){
//					HWND	hwndWork;
//					hwndWork = ::GetParent( hwndHtmlHelpChild );
//					if( hwndHtmlHelp == hwndWork ){
//						hwndHtmlHelp = hwndHtmlHelpChild;
//					}
//				}else{
//				}
//			}
//			if( hwndHtmlHelp != NULL ){
//				DWORD	dwPID;
//				DWORD	dwTID = ::GetWindowThreadProcessId( hwndHtmlHelp, &dwPID );
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, TRUE );
//				::SetFocus( hwndHtmlHelp );
//				::AttachThreadInput( ::GetCurrentThreadId(), dwTID, FALSE );
//			}

			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				/*hwnd*/NULL/*hwndFrame*//*m_pShareData->m_hwndTray*/,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD)&link,
				false
			);
			delete [] pszKey;
		}
		return (LRESULT)hwndHtmlHelp;;


//�V���O���v���Z�X��
//	/* �V�����ҏW�E�B���h�E�̍쐬�˗�(�R�}���h���C����n��) */
//	case MYWM_OPENNEWEDITOR:
//		pszCmdLine = new char[lstrlen( m_pShareData->m_szWork ) + 1];
//		strcpy( pszCmdLine, m_pShareData->m_szWork );
//		hwndNew = OpenNewEditor3( m_hInstance, hwnd, pszCmdLine, FALSE );
//		delete [] pszCmdLine;
//		return (LONG)hwndNew;

//	/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃A�N�e�B�u�v�� */
//	case MYWM_ACTIVATE_ME:
//		{
//			CRunningTimer	cRunningTimer;
//			cRunningTimer.Reset();
//			while( 3000 > cRunningTimer.Read() ){
//				::BlockingHook();
//			}
//			/* �A�N�e�B�u�ɂ��� */
//			ActivateFrameWindow( (HWND)wParam );
//		}
//		return 0;

	/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
	case MYWM_DELETE_ME:
//�V���O���v���Z�X��
//		pCEditWnd_Test = (CEditWnd*)lParam;
//		delete pCEditWnd_Test;

		/* �^�X�N�g���C�̃A�C�R�����풓���Ȃ��ꍇ */
		if( FALSE == m_pShareData->m_Common.m_bStayTaskTray	/* �^�X�N�g���C�̃A�C�R�����풓 */
		 || FALSE == m_bCreatedTrayIcon						/* �g���C�ɃA�C�R��������Ă��Ȃ� */
		 ){
			/* ���݊J���Ă���ҏW���̃��X�g */
			nRowNum = m_cShareData.GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( 0 < nRowNum ){
				delete [] pEditNodeArr;
			}
			/* �ҏW�E�B���h�E�̐���0�ɂȂ�����I�� */
			if( 0 == nRowNum ){
				::SendMessage( hwnd, WM_CLOSE, 0, 0 );
			}
		}
		return 0;

//	case WM_RASDIALEVENT:
//		{
//			RASCONNSTATE	rasconnstate;
//			DWORD			dwError;
//
//			rasconnstate	= (RASCONNSTATE) wParam;	// connection state about to be entered
//			dwError			= (DWORD) lParam;			// error that may have occurred
//		}
//		break;
	case WM_CREATE:
		m_hWnd = hwnd;
		hwndHtmlHelp = NULL;
		::SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this );

//		for( i = 0; i < 16; ++i ){
//			HWND	hwndWork;
//			pCEditWnd_Test = NULL;
//			pCEditWnd_Test = new CEditWnd;
//			if( NULL == pCEditWnd_Test ){
//				MYTRACE( "�I�u�W�F�N�g�m�ێ��s\n" );
//				break;
//			}
//			hwndWork = pCEditWnd_Test->Create( m_hInstance, hwnd/*pShareData->m_hwndTray*/, NULL, 0, FALSE );
//			if( NULL == hwndWork ){
//				MYTRACE( "NULL == hwndWork\n" );
//				break;
//			}
//		}

		/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
		wHotKeyMods = 0;
		if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_SHIFT;
		}
		if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_CONTROL;
		}
		if( HOTKEYF_ALT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
			wHotKeyMods |= MOD_ALT;
		}
		wHotKeyCode = m_pShareData->m_Common.m_wTrayMenuHotKeyCode;
		::RegisterHotKey(
			m_hWnd,
			ID_HOTKEY_TRAYMENU,
			wHotKeyMods,
			wHotKeyCode
		);
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			CEditWnd::OnHelp_MenuItem( hwnd, lphi->iCtrlId );
			break;
		}
		return TRUE;
//		case WM_LBUTTONDOWN:
//			/* �|�b�v�A�b�v���j���[(�g���C���{�^��) */
//			nId = CreatePopUpMenu_L();
//			MYTRACE( "nId = %d\n", nId );
//			return 0L;
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
		case MYWM_CHANGESETTING:
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );
			/* �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^ */
			wHotKeyMods = 0;
			if( HOTKEYF_SHIFT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_SHIFT;
			}
			if( HOTKEYF_CONTROL & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_CONTROL;
			}
			if( HOTKEYF_ALT & m_pShareData->m_Common.m_wTrayMenuHotKeyMods ){
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = m_pShareData->m_Common.m_wTrayMenuHotKeyCode;
			::RegisterHotKey(
				m_hWnd,
				ID_HOTKEY_TRAYMENU,
				wHotKeyMods,
				wHotKeyCode
			);

//@@		/* ���L�f�[�^�̕ۑ� */
//@@		m_cShareData.SaveShareData();

			/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
			if( m_pShareData->m_hAccel != NULL ){
				::DestroyAcceleratorTable( m_pShareData->m_hAccel );
				m_pShareData->m_hAccel = NULL;
			}
			m_pShareData->m_hAccel =
				CKeyBind::CreateAccerelator(
					m_pShareData->m_nKeyNameArrNum,
					m_pShareData->m_pKeyNameArr
				);
			if( NULL == m_pShareData->m_hAccel ){
				::MessageBox( NULL, "CEditApp::DispatchEvent()\n�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n�V�X�e�����\�[�X���s�����Ă��܂��B", GSTR_APPNAME, MB_OK | MB_ICONSTOP );
			}

			return 0L;

		case MYWM_NOTIFYICON:
//			MYTRACE( "MYWM_NOTIFYICON\n" );
			switch (lParam){
//�L�[���[�h�F�g���C�E�N���b�N���j���[�ݒ�
//	From Here Oct. 12, 2000 JEPRO ���E�Ƃ����ꏈ���ɂȂ��Ă����̂�ʁX�ɏ�������悤�ɕύX
			case WM_RBUTTONDOWN:
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */
				nId = CreatePopUpMenu_R();
				switch( nId ){
// Nov. 21, 2000 JEPROtestnow
				case F_FONT:
					/* �t�H���g�ݒ� */
					{
//						HWND	hwndFrame;
//						hwndFrame = ::GetParent( m_hwndParent );
						/* �t�H���g�ݒ�_�C�A���O */
//						if( m_pcEditDoc->SelectFont( &(m_pShareData->m_Common.m_lf) ) ){
							if( m_pShareData->m_Common.m_lf.lfPitchAndFamily & FIXED_PITCH ){
								m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = TRUE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
							}else{
								m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = FALSE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
							}
							/* �ݒ�ύX�𔽉f������ */
							/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
//							m_cShareData.PostMessageToAllEditors(
//								MYWM_CHANGESETTING,
//								(WPARAM)0, (LPARAM)0, hwndFrame
//							);
//						}
					}
					break;
				case F_OPTION:
					/* ���ʐݒ� */
					{
						/* �ݒ�v���p�e�B�V�[�g �e�X�g�p */
//						m_pcEditDoc->bOpenPropertySheet( -1/*, -1*/ );
					}
					break;
				case F_OPTION_TYPE:
					/* �^�C�v�ʐݒ� */
					{
//						CEditDoc::OpenPropertySheetTypes( -1, m_nSettingType );
					}
					break;
				case F_TYPE_LIST:
					/* �^�C�v�ʐݒ�ꗗ */
					{
//						CDlgTypeList	cDlgTypeList;
//						int				nSettingType;
//						nSettingType = m_pcEditDoc->m_nSettingType;
//						if( cDlgTypeList.DoModal( m_hInstance, m_hWnd, &nSettingType ) ){
							/* �^�C�v�ʐݒ� */
//							m_pcEditDoc->OpenPropertySheetTypes( -1, nSettingType );
//						}
					}
					break;
				case F_HELP_CONTENTS:
					/* �w���v�ڎ� */
					{
						char	szHelp[_MAX_PATH + 1];
						::GetHelpFilePath( szHelp );
						/* �w���v�t�@�C���̃t���p�X��Ԃ� */
					//From Here Jan. 13, 2001 JEPRO HELP_FINDER�ł͑O��A�N�e�B�u�������g�s�b�N�̌����̃^�u�ɂȂ��Ă��܂�
					//��� HELP_CONTENTS (���邢�� HELP�QINDEX) ���Ɩڎ��y�[�W���o�Ă���B�����������...
					//	::WinHelp( m_hWnd, szHelp, HELP_FINDER, 0 );
					//	::WinHelp( m_hWnd, szHelp, HELP_COMMAND, (unsigned long)"CONTENTS()" );	//[�ڎ�]�^�u�̕\��
					//To Here Jan. 13, 2001
					// From Here 2001.12.03 hor
					//	WinNT 4 �ł͂Ȃɂ��\������Ȃ������̂ŃG���[�̏ꍇ�� HELP_CONTENTS �\������悤�ɕύX
						if( ::WinHelp( m_hWnd, szHelp, HELP_COMMAND, (unsigned long)"CONTENTS()" )){
							::WinHelp( m_hWnd, szHelp, HELP_CONTENTS , 0 );	//[�ڎ�]�^�u�̕\��
						}
					// To Here 2001.12.03 hor
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
//				case F_MENU_ALLFUNC:	//Jan. 12, 2001 JEPRO �R�}���h�ꗗ�͉E�N���b�N���j���[����ꉞ���O
//					/* �R�}���h�ꗗ */
//						CEditView::Command_MENU_ALLFUNC();
//					break;
				case F_EXTHELP1:
					/* �O���w���v�P */
//					{
//						CEditView::Command_EXTHELP1();
//					}
//					break;
					{
					retry:;
						if( 0 == strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
							::MessageBeep( MB_ICONHAND );
							if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
								"�O���w���v�P���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?"
							) ){
								/* ���ʐݒ� �v���p�e�B�V�[�g */
//								if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
//									break;
//								}
								goto retry;
							}
							else {
								break;
							}
						}

						CMemory		cmemCurText;
						/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
//						GetCurrentTextForSearch( cmemCurText );
//						::WinHelp( m_hwndParent, m_pShareData->m_Common.m_szExtHelp1, HELP_KEY, (DWORD)(char*)cmemCurText.GetPtr( NULL ) );
						break;
					}
					break;
				case F_EXTHTMLHELP:
					/* �O��HTML�w���v */
					{
//						CEditView::Command_EXTHTMLHELP();
					}
					break;
				case F_ABOUT:
					/* �o�[�W������� */
					{
						CDlgAbout cDlgAbout;
						cDlgAbout.DoModal( m_hInstance, m_hWnd );
					}
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
					/* �T�N���G�f�B�^�̑S�I�� */
					CEditApp::TerminateApplication();
					break;
				}
				return 0L;
//	To Here Oct. 12, 2000

			case WM_LBUTTONDOWN:
//				MYTRACE( "WM_LBUTTONDOWN\n" );
				::SetActiveWindow( m_hWnd );
				::SetForegroundWindow( m_hWnd );
				/* �|�b�v�A�b�v���j���[(�g���C���{�^��) */
//				MYTRACE( "CreatePopUpMenu_L START\n" );
				nId = CreatePopUpMenu_L();
//				MYTRACE( "CreatePopUpMenu_L END\n" );
				switch( nId ){
				case F_FILENEW:	/* �V�K�쐬 */
//					MYTRACE( "F_FILENEW\n" );
					/* �V�K�ҏW�E�B���h�E�̒ǉ� */
					OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
					break;
				case F_FILEOPEN:	/* �J�� */
//					MYTRACE( "F_FILEOPEN\n" );
					{
						CDlgOpenFile	cDlgOpenFile;
//						char*			pszMRU = NULL;;
						char			szPath[_MAX_PATH + 1];
						int				nCharCode;
						BOOL			bReadOnly;
						HWND			hWndOwner;
						FileInfo*		pfi;
						int				i;
						int				j;
						char**			ppszMRU;
						char**			ppszOPENFOLDER;

						/* MRU���X�g�̃t�@�C���̃��X�g */
						ppszMRU = NULL;
						j = 0;
						if( m_pShareData->m_nMRUArrNum > 0 ){
							for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
								if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
									break;
								}
								++j;
							}
						}
						ppszMRU = new char*[j + 1];
						if( j > 0 ){
							for( i = 0; i < j; ++i ){
					//			ppszMRU[i] = m_pShareData->m_Types[m_nSettingType].m_szMRUArr[i];
								ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
							}
						}
						ppszMRU[j] = NULL;

						/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
						ppszOPENFOLDER = NULL;
						j = 0;
						if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
							for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
								if( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= i ){
									break;
								}
								++j;
							}
						}
						ppszOPENFOLDER = new char*[j + 1];
						if( j > 0 ){
							for( i = 0; i < j; ++i ){
					//			ppszOPENFOLDER[i] = m_pShareData->m_Types[m_nSettingType].m_szOPENFOLDERArr[i];
								ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
							}
						}
						ppszOPENFOLDER[j] = NULL;



						/* �t�@�C���I�[�v���_�C�A���O�̏����� */
						strcpy( szPath, "" );
						nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
						bReadOnly = FALSE;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL/*m_hWnd*/,
							"*.*",
							m_pShareData->m_fiMRUArr[0].m_szPath,
							(const char **)ppszMRU,
							(const char **)ppszOPENFOLDER
						);
						if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							break;
						}
						if( NULL == m_hWnd ){
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							break;
						}
						delete [] ppszMRU;
						delete [] ppszOPENFOLDER;
						/* �w��t�@�C�����J����Ă��邩���ׂ� */
						if( m_cShareData.IsPathOpened( szPath, &hWndOwner ) ){
							::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//							pfi = (FileInfo*)m_pShareData->m_szWork;
							pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

							int nCharCodeNew;
							if( CODE_AUTODETECT == nCharCode ){	/* �����R�[�h�������� */
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
								nCharCodeNew = CMemory::CheckKanjiCodeOfFile( szPath );
								if( -1 == nCharCodeNew ){

								}else{
									nCharCode = nCharCodeNew;
								}
							}
							if( nCharCode != pfi->m_nCharCode ){	/* �����R�[�h��� */
								char*	pszCodeNameCur;
								char*	pszCodeNameNew;
								switch( pfi->m_nCharCode ){
								case CODE_SJIS:		/* SJIS */		pszCodeNameCur = "SJIS";break;	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
								case CODE_JIS:		/* JIS */		pszCodeNameCur = "JIS";break;
								case CODE_EUC:		/* EUC */		pszCodeNameCur = "EUC";break;
								case CODE_UNICODE:	/* Unicode */	pszCodeNameCur = "Unicode";break;
								case CODE_UTF8:		/* UTF-8 */		pszCodeNameCur = "UTF-8";break;
								case CODE_UTF7:		/* UTF-7 */		pszCodeNameCur = "UTF-7";break;
								}
								switch( nCharCode ){
								case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
								case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
								case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
								case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
								case CODE_UTF8:		/* UTF-8 */		pszCodeNameNew = "UTF-8";break;
								case CODE_UTF7:		/* UTF-7 */		pszCodeNameNew = "UTF-7";break;
								}
								::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
									"%s\n\n���ɊJ���Ă���t�@�C�����Ⴄ�����R�[�h�ŊJ���ꍇ�́A\n��U���Ă���łȂ��ƊJ���܂���B\n\n���݂̕����R�[�h�Z�b�g��%s\n�V���������R�[�h�Z�b�g��%s",
									szPath, pszCodeNameCur, pszCodeNameNew
								);
							}
							/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
							/* �A�N�e�B�u�ɂ��� */
							ActivateFrameWindow( hWndOwner );
						}else{
							if( strchr( szPath, ' ' ) ){
								char	szFile2[_MAX_PATH + 3];
								wsprintf( szFile2, "\"%s\"", szPath );
								strcpy( szPath, szFile2 );
							}
							/* �V���ȕҏW�E�B���h�E���N�� */
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly );
						}
					}
					break;
				case F_GREP:
					/* Grep */
					DoGrep();  //Stonee, 2001/03/21  Grep��ʊ֐���
					break;
				case F_WIN_CLOSEALL:	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
					/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
					CEditApp::CloseAllEditor();
					break;
//				case IDM_EXITALL:
				case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
					/* �T�N���G�f�B�^�̑S�I�� */
					CEditApp::TerminateApplication();
					break;
				default:
					if( nId - IDM_SELWINDOW  >= 0 &&
						nId - IDM_SELWINDOW  < m_pShareData->m_nEditArrNum ){
						hwndWork = m_pShareData->m_pEditArr[nId - IDM_SELWINDOW].m_hWnd;

						/* �A�N�e�B�u�ɂ��� */
						ActivateFrameWindow( hwndWork );
//						if( ::IsIconic( hwndWork ) ){
//							::ShowWindow( hwndWork, SW_RESTORE );
//						}else{
//							::ShowWindow( hwndWork, SW_SHOW );
//						}
//						::SetForegroundWindow( hwndWork );
//						::SetActiveWindow( hwndWork );
					}else
					if( nId - IDM_SELMRU  >= 0 &&
//						nId - IDM_SELMRU  < (( m_pShareData->m_nMRUArrNum < m_pShareData->m_Common.m_nMRUArrNum_MAX )?m_pShareData->m_nMRUArrNum :m_pShareData->m_Common.m_nMRUArrNum_MAX )
						nId - IDM_SELMRU  < 999
					){

//						char	szPath[_MAX_PATH + 3];
//						strcpy( szPath, &(m_pShareData->m_fiMRUArr[nId - IDM_SELMRU]) );
//						if( strchr( szPath, ' ' ) ){
//							char	pszFile2[_MAX_PATH + 3];
//							sprintf( pszFile2, "\"%s\"", szPath );
//							strcpy( szPath, pszFile2 );
//						}
						/* �V�����ҏW�E�B���h�E���J�� */
						//	From Here Oct. 27, 2000 genta	�J�[�\���ʒu�𕜌����Ȃ��@�\
						if( m_pShareData->m_Common.GetRestoreCurPosition() ){
							CEditApp::OpenNewEditor2( m_hInstance, m_hWnd, &(m_pShareData->m_fiMRUArr[nId - IDM_SELMRU]), FALSE );
						}
						else {
							CEditApp::OpenNewEditor( m_hInstance, m_hWnd,
								m_pShareData->m_fiMRUArr[nId - IDM_SELMRU].m_szPath,
								m_pShareData->m_fiMRUArr[nId - IDM_SELMRU].m_nCharCode,
								FALSE );

						}
						//	To Here Oct. 27, 2000 genta
					}else
					if( nId - IDM_SELOPENFOLDER  >= 0 &&
//						nId - IDM_SELOPENFOLDER  < (( m_pShareData->m_nOPENFOLDERArrNum < m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX )?m_pShareData->m_nOPENFOLDERArrNum:m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX )
						nId - IDM_SELOPENFOLDER  < 999
					){
						{
							CDlgOpenFile	cDlgOpenFile;
//							char*			pszMRU = NULL;;
							char			szPath[_MAX_PATH + 1];
							int				nCharCode;
							BOOL			bReadOnly;
							HWND			hWndOwner;
							FileInfo*		pfi;
							int				i;
							int				j;
							char**			ppszMRU;
							char**			ppszOPENFOLDER;

							/* MRU���X�g�̃t�@�C���̃��X�g */
							ppszMRU = NULL;
							j = 0;
							if( m_pShareData->m_nMRUArrNum > 0 ){
								for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
									if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
										break;
									}
									++j;
								}
							}
							ppszMRU = new char*[j + 1];
							if( j > 0 ){
								for( i = 0; i < j; ++i ){
						//			ppszMRU[i] = m_pShareData->m_Types[m_nSettingType].m_szMRUArr[i];
									ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
								}
							}
							ppszMRU[j] = NULL;

							/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
							ppszOPENFOLDER = NULL;
							j = 0;
							if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
								for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
									if( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= i ){
										break;
									}
									++j;
								}
							}
							ppszOPENFOLDER = new char*[j + 1];
							if( j > 0 ){
								for( i = 0; i < j; ++i ){
						//			ppszOPENFOLDER[i] = m_pShareData->m_Types[m_nSettingType].m_szOPENFOLDERArr[i];
									ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
								}
							}
							ppszOPENFOLDER[j] = NULL;

							/* �t�@�C���I�[�v���_�C�A���O�̏����� */
							strcpy( szPath, "" );
							nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
							bReadOnly = FALSE;
							cDlgOpenFile.Create(
								m_hInstance,
								NULL/*m_hWnd*/,
								"*.*",
								m_pShareData->m_szOPENFOLDERArr[nId - IDM_SELOPENFOLDER],
								(const char **)ppszMRU,
								(const char **)ppszOPENFOLDER
							);
							if( !cDlgOpenFile.DoModalOpenDlg( szPath, &nCharCode, &bReadOnly ) ){
								delete [] ppszMRU;
								delete [] ppszOPENFOLDER;
								break;
							}
							if( NULL == m_hWnd ){
								delete [] ppszMRU;
								delete [] ppszOPENFOLDER;
								break;
							}
							delete [] ppszMRU;
							delete [] ppszOPENFOLDER;
							/* �w��t�@�C�����J����Ă��邩���ׂ� */
							if( m_cShareData.IsPathOpened( szPath, &hWndOwner ) ){
								::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//								pfi = (FileInfo*)m_pShareData->m_szWork;
								pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

								int nCharCodeNew;
								if( CODE_AUTODETECT == nCharCode ){	/* �����R�[�h�������� */
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
									nCharCodeNew = CMemory::CheckKanjiCodeOfFile( szPath );
									if( -1 == nCharCodeNew ){

									}else{
										nCharCode = nCharCodeNew;
									}
								}
								if( nCharCode != pfi->m_nCharCode ){	/* �����R�[�h��� */
									char*	pszCodeNameCur;
									char*	pszCodeNameNew;
									switch( pfi->m_nCharCode ){
									case CODE_SJIS:		/* SJIS */		pszCodeNameCur = "SJIS";break;	//	Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
									case CODE_JIS:		/* JIS */		pszCodeNameCur = "JIS";break;
									case CODE_EUC:		/* EUC */		pszCodeNameCur = "EUC";break;
									case CODE_UNICODE:	/* Unicode */	pszCodeNameCur = "Unicode";break;
									case CODE_UTF8:		/* UTF-8 */		pszCodeNameCur = "UTF-8";break;
									case CODE_UTF7:		/* UTF-7 */		pszCodeNameCur = "UTF-7";break;
									}
									switch( nCharCode ){
									case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//	Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
									case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
									case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
									case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
									case CODE_UTF8:		/* UTF-8 */		pszCodeNameNew = "UTF-8";break;
									case CODE_UTF7:		/* UTF-7 */		pszCodeNameNew = "UTF-7";break;
									}
									::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
										"%s\n\n���ɊJ���Ă���t�@�C�����Ⴄ�����R�[�h�ŊJ���ꍇ�́A\n��U���Ă���łȂ��ƊJ���܂���B\n\n���݂̕����R�[�h�Z�b�g��%s\n�V���������R�[�h�Z�b�g��%s",
										szPath, pszCodeNameCur, pszCodeNameNew
									);
								}
								/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
								/* �A�N�e�B�u�ɂ��� */
								ActivateFrameWindow( hWndOwner );
//								if( ::IsIconic( hWndOwner ) ){
//									::ShowWindow( hWndOwner, SW_RESTORE );
//								}else{
//									::ShowWindow( hWndOwner, SW_SHOW );
//								}
//								::SetForegroundWindow( hWndOwner );
//								::SetActiveWindow( hWndOwner );
							}else{
								if( strchr( szPath, ' ' ) ){
									char	szFile2[_MAX_PATH + 3];
									wsprintf( szFile2, "\"%s\"", szPath );
									strcpy( szPath, szFile2 );
								}
								/* �V���ȕҏW�E�B���h�E���N�� */
								CEditApp::OpenNewEditor( m_hInstance, m_hWnd, szPath, nCharCode, bReadOnly );
							}
						}
					}else{

					}
					break;
				}
				return 0L;
			case WM_LBUTTONDBLCLK:
//				MYTRACE( "WM_LBUTTONDBLCLK\n" );
				/* �V�K�ҏW�E�B���h�E�̒ǉ� */
				OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
				return 0L;
			case WM_RBUTTONDBLCLK:
//				MYTRACE( "WM_RBUTTONDBLCLK\n" );
				return 0L;
			}
			break;

		case WM_QUERYENDSESSION:
			/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
			if( CloseAllEditor() ){
				//	Jan. 31, 2000 genta
				//	���̎��_�ł�Windows�̏I�����m�肵�Ă��Ȃ��̂ŏ풓�������ׂ��ł͂Ȃ��D
				//	DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
			if( CloseAllEditor() ){
				DestroyWindow( hwnd );
			}
			return 0L;

		//	From Here Jan. 31, 2000 genta	Windows�I�����̌㏈���D
		//	Windows�I������WM_CLOSE���Ă΂�Ȃ���CDestroyWindow��
		//	�Ăяo���K�v���Ȃ��D�܂��C���b�Z�[�W���[�v�ɖ߂�Ȃ��̂�
		//	���b�Z�[�W���[�v�̌��̏����������Ŋ���������K�v������D
		case WM_ENDSESSION:
			//	����Windows�̏I�������f���ꂽ�̂Ȃ牽�����Ȃ�
			if( wParam != TRUE )	return 0;

			//	�z�b�g�L�[�̔j��
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );

			//	�ǂ���Explorer���I������̂Ńg���C�A�C�R���͏������Ȃ��D

			//	�I���������ɐV�����E�B���h�E�����̂�����Ȋ����Ȃ̂�
			//	�I�v�V�����Ɋւ�炸�I���_�C�A���O�̕\���͍s��Ȃ�

			//	���L�f�[�^�̕ۑ�(�d�v)
			m_cShareData.SaveShareData();

			return 0;	//	�������̃v���Z�X�ɐ��䂪�߂邱�Ƃ͂Ȃ�
		//	To Here Jan. 31, 2000 genta
		case WM_DESTROY:
			::UnregisterHotKey( m_hWnd, ID_HOTKEY_TRAYMENU );



//			/* �I���_�C�A���O��\������ */
//			if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
//				/* �I�����_�C�A���O�̕\�� */
//				hwndExitingDlg = ::CreateDialog(
//					m_hInstance,
//					MAKEINTRESOURCE( IDD_EXITING ),
//					m_hWnd/*::GetDesktopWindow()*/,
//					(DLGPROC)ExitingDlgProc
//				);
//				::ShowWindow( hwndExitingDlg, SW_SHOW );
//			}
//
//			/* ���L�f�[�^�̕ۑ� */
//			m_cShareData.SaveShareData();
//
//			/* �I���_�C�A���O��\������ */
//			if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
//				/* �I�����_�C�A���O�̔j�� */
//				::DestroyWindow( hwndExitingDlg );
//			}

			if( m_bCreatedTrayIcon ){	/* �g���C�ɃA�C�R��������� */
				TrayMessage( hwnd, NIM_DELETE, 0, NULL, NULL );
			}
			m_hWnd = NULL;
			/* Windows �ɃX���b�h�̏I����v�����܂��B*/
			::PostQuitMessage( 0 );
			return 0L;
		default:
// << 20010412 by aroka
//	Apr. 24, 2001 genta RegisterWindowMessage���g���悤�ɏC��
			if( uMsg == m_uCreateTaskBarMsg ){
				/* TaskTray Icon�̍ēo�^��v�����郁�b�Z�[�W�D
					Explorer���ċN�������Ƃ��ɑ��o�����D*/
				CreateTrayIcon( m_hWnd ) ;
			}
// >> by aroka
	}
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}




/* WM_COMMAND���b�Z�[�W���� */
void CEditApp::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* ���j���[����̃��b�Z�[�W */
	case 0:
		break;
	}
	return;
}




/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
//	Oct. 24, 2000 genta
//	WinExec -> CreateProcess�D�����@�\��t��
bool CEditApp::OpenNewEditor( HINSTANCE hInstance, HWND hWndParent, char* pszPath, int nCharCode, BOOL bReadOnly, bool sync )
{
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
	char szCmdLineBuf[1024];	//	�R�}���h���C��
	char szEXE[MAX_PATH + 1];	//	�A�v���P�[�V�����p�X��
	int nPos = 0;				//	�R�}���h���C���\�z�p�|�C���^

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return false;
	}

	::GetModuleFileName( ::GetModuleHandle( NULL ), szEXE, sizeof( szEXE ) );
	nPos += wsprintf( szCmdLineBuf + nPos, "\"%s\"", szEXE );

	//	�t�@�C�������w�肳��Ă���ꍇ
	//	�R�}���h���C���������w�肳��Ă���̂ŁC�S�̂�""�ň͂�ł͂����Ȃ�
	if( pszPath != NULL ){
		nPos += wsprintf( szCmdLineBuf + nPos, " %s", pszPath );
	}
	//	�R�[�h�w�肪����ꍇ
	if( nCharCode != CODE_AUTODETECT ){
		nPos += wsprintf( szCmdLineBuf + nPos, " -CODE=%d", nCharCode );
	}
	//	�ǂݎ���p�w�肪����ꍇ		//From Here Feb. 26, 2001 JEPRO �ǉ� (direcited by genta)
	if( bReadOnly ){
		nPos += wsprintf( szCmdLineBuf + nPos, " -R" );
	}		//To Here Feb. 26, 2001

	//	DEBUG
	//	::MessageBox( NULL, szCmdLineBuf, "OpenNewEditor", MB_OK );

	//	�v���Z�X�̋N��
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;
	/*
	s.dwX = CW_USEDEFAULT;
	s.dwY = CW_USEDEFAULT;
	s.dwXSize = CW_USEDEFAULT;
	s.dwYSize = CW_USEDEFAULT;
	*/

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	if( CreateProcess( szEXE, szCmdLineBuf, NULL, NULL, FALSE,
		CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &s, &p ) == 0 ){
		//	���s
		LPVOID pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &pMsg,
						0,
						NULL
		);
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B\n%s",
			szEXE,
			(char*)pMsg
		);
		::LocalFree( (HLOCAL)pMsg );	//	�G���[���b�Z�[�W�o�b�t�@�����
		return false;
	}

	if( sync ){
		//	�N�������v���Z�X�����S�ɗ����オ��܂ł�����Ƒ҂D
		int nResult = WaitForInputIdle( p.hProcess, 10000 );	//	�ő�10�b�ԑ҂�
		if( nResult != 0 ){
			::MYMESSAGEBOX(
				hWndParent,
				MB_OK | MB_ICONSTOP,
				GSTR_APPNAME,
				"\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B",
				szEXE
			);
			CloseHandle( p.hThread );
			CloseHandle( p.hProcess );
			return false;
		}
	}

	CloseHandle( p.hThread );
	CloseHandle( p.hProcess );

	return true;
}


#if 0
/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 1 */
//void CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly )
{
	UINT			nRet;
	char*			pszCmdLine;
	char*			pszMsg;
	pszCmdLine = new char[1024];
	char			szPath[_MAX_PATH + 3];
	char			szEXE[MAX_PATH];
	char*			pszReadOnly;
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	::GetModuleFileName( ::GetModuleHandle( NULL ), szEXE, sizeof( szEXE ) );
	strcpy( szPath, pfi->m_szPath );
	if( strchr( szPath, ' ' ) ){
		char	pszFile2[_MAX_PATH + 3];
		wsprintf( pszFile2, "\"%s\"", szPath );
		strcpy( szPath, pszFile2 );
	}
	if( bReadOnly ){
		pszReadOnly = "-R";
	}else{
		pszReadOnly = "";
	}
	/* �s���w���1�J�n�ɂ��� */
	wsprintf( pszCmdLine, "%s %s -X=%d -Y=%d -VX=%d -VY=%d -CODE=%d %s",
		szEXE,
		szPath,
		pfi->m_nX + 1,
		pfi->m_nY + 1,
		pfi->m_nViewLeftCol + 1,
		pfi->m_nViewTopLine + 1,
		pfi->m_nCharCode,
		pszReadOnly
	);

//�����v���Z�X��
	if( 31 >= ( nRet = ::WinExec( pszCmdLine, SW_SHOW ) ) ){
		switch( nRet ){
		case 0:						pszMsg = "�V�X�e���Ƀ������܂��̓��\�[�X������܂���B";break;
		case ERROR_FILE_NOT_FOUND:	pszMsg = "�w�肳�ꂽ�t�@�C����������܂���ł����B";break;
		case ERROR_PATH_NOT_FOUND:	pszMsg = "�w�肳�ꂽ�p�X��������܂���ł����B";break;
		case ERROR_BAD_FORMAT:		pszMsg = ".EXE�t�@�C���������ł��B";break;
		default:					pszMsg = "�����s���ł��B";break;
		}
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B\n%s",
			szEXE,
			pszMsg
		);
	}

	delete [] pszCmdLine;
	return;
}
#endif

//	From Here Oct. 24, 2000 genta
//	�V�K�ҏW�E�B���h�E�̒ǉ� ver 2:
bool CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly, bool sync )
{
	char			pszCmdLine[1024];
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
	int				nPos = 0;		//	�����쐬�p�|�C���^

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
		char szMsg[512];
		wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
		return false;
	}

	if( pfi != NULL ){
		if( pfi->m_szPath != NULL ){
			if( strlen( pfi->m_szPath ) > 0 ){
				nPos += wsprintf( pszCmdLine + nPos, " \"%s\"", pfi->m_szPath );
			}
		}
		if( pfi->m_nX >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -X=%d", pfi->m_nX +1 );
		}
		if( pfi->m_nY >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -Y=%d", pfi->m_nY +1 );
		}
		if( pfi->m_nViewLeftCol >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -VX=%d", pfi->m_nViewLeftCol +1 );
		}
		if( pfi->m_nViewTopLine >= 0 ){
			nPos += wsprintf( pszCmdLine + nPos, " -VY=%d", pfi->m_nViewTopLine +1 );
		}
		if( pfi->m_nCharCode >= 0 && pfi->m_nCharCode != CODE_AUTODETECT ){
			nPos += wsprintf( pszCmdLine + nPos, " -CODE=%d", pfi->m_nCharCode );
		}
		if( bReadOnly ){
			nPos += wsprintf( pszCmdLine + nPos, " -R" );
		}
	}
	//	::MessageBox( NULL, pszCmdLine, "OpenNewEditor", MB_OK );
	return OpenNewEditor( hInstance, hWndParent, pszCmdLine, CODE_AUTODETECT, bReadOnly, sync );

}
//	To Here Oct. 24, 2000 genta


//�V���O���v���Z�X�ŗp
//	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 2 */
//	HWND CEditApp::OpenNewEditor3(
//		HINSTANCE hInstance, HWND hWndParent, const char* pszCommandLine, BOOL bActivate )
//	{
//		/* �V�X�e�����\�[�X�̃`�F�b�N */
//		if( FALSE == CheckSystemResources( GSTR_APPNAME ) ){
//			return NULL;
//		}
//
//		CShareData		m_cShareData;
//		DLLSHAREDATA*	pShareData;
//		CEditWnd*		pcEditWnd;
//		HWND			hWnd;
//		/* �R�}���h���C���I�v�V���� */
//		BOOL			bGrepMode;
//		CMemory			cmGrepKey;
//		CMemory			cmGrepFile;
//		CMemory			cmGrepFolder;
//		CMemory			cmWork;
//		BOOL			bGrepSubFolder;
//		BOOL			bGrepLoHiCase;
//		BOOL			bGrepRegularExp;
//		BOOL			bGrepKanjiCode_AutoDetect;
//		BOOL			bGrepOutputLine;
//		BOOL			bDebugMode;
//		BOOL			bNoWindow;
//		FileInfo		fi;
//		BOOL			bReadOnly;
//
//		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//		m_cShareData.Init();
//		pShareData = m_cShareData.GetShareData( NULL, NULL );
//
//		/* �R�}���h���C���̉�� */
//		ParseCommandLine(
//			pszCommandLine,
//			&bGrepMode,
//			&cmGrepKey,
//			&cmGrepFile,
//			&cmGrepFolder,
//			&bGrepSubFolder,
//			&bGrepLoHiCase,
//			&bGrepRegularExp,
//			&bGrepKanjiCode_AutoDetect,
//			&bGrepOutputLine,
//			&bDebugMode,
//			&bNoWindow,
//			&fi,
//			&bReadOnly
//		);
//
//		/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�́A*/
//		/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
//		if( 0 < lstrlen( fi.m_szPath ) ){
//			HWND hwndOwner;
//			/* �w��t�@�C�����J����Ă��邩���ׂ� */
//			if( TRUE == m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
//				if( bActivate ){
//					/* �A�N�e�B�u�ɂ��� */
//					ActivateFrameWindow( hwndOwner );
//				}
//				return hwndOwner;
//			}
//		}
//
//		pcEditWnd = new CEditWnd;
//		if( bDebugMode ){
//			hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//
//	#ifdef _DEBUG/////////////////////////////////////////////
//			/* �f�o�b�O���j�^���[�h�ɐݒ� */
//			pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////
//		}else
//		if( bGrepMode ){
//			hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//			/* �A�N�e�B�u�ɂ��� */
//			ActivateFrameWindow( hWnd );
//			/* GREP */
//			int			nHitCount;
//			GrepParam	GP;
//			GP.pCEditView				= (void*)&pcEditWnd->m_cEditDoc.m_cEditViewArr[0];
//			GP.pszGrepKey				= cmGrepKey.GetPtr( NULL );
//			GP.pszGrepFile				= cmGrepFile.GetPtr( NULL );
//			GP.pszGrepFolder			= cmGrepFolder.GetPtr( NULL );
//			GP.bGrepSubFolder			= bGrepSubFolder;
//			GP.bGrepLoHiCase			= bGrepLoHiCase;
//			GP.bGrepRegularExp			= bGrepRegularExp;
//			GP.bKanjiCode_AutoDetect	= bGrepKanjiCode_AutoDetect;
//			GP.bGrepOutputLine			= bGrepOutputLine;
//
//			/*nHitCount = */pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep_Thread( (DWORD)&GP );
//	//		nHitCount = pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
//	//			&cmGrepKey,
//	//			&cmGrepFile,
//	//			&cmGrepFolder,
//	//			bGrepSubFolder,
//	//			bGrepLoHiCase,
//	//			bGrepRegularExp,
//	//			bGrepKanjiCode_AutoDetect,
//	//			bGrepOutputLine
//	//		);
//		}else{
//			if( 0 < (int)lstrlen( fi.m_szPath ) ){
//				hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, fi.m_szPath, fi.m_nCharCode, bReadOnly/* �ǂݎ���p�� */ );
//				if( fi.m_nViewTopLine < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
//				}
//				/*
//				  �J�[�\���ʒu�ϊ�
//				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//
//				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//				*/
//				int		nPosX;
//				int		nPosY;
//				pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
//					fi.m_nX,
//					fi.m_nY,
//					&nPosX,
//					&nPosY
//				);
//				if( nPosY < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX = nPosX;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosY = nPosY;
//				}else{
//					int i;
//					i = pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
//					if( i < 0 ){
//						i = 0;
//					}
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX = 0;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
//					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosY = i;
//				}
//				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
//			}else{
//				hWnd = pcEditWnd->Create( hInstance, pShareData->m_hwndTray, NULL, 0, FALSE );
//			}
//		}
//		if( bActivate ){
//			/* �A�N�e�B�u�ɂ��� */
//			ActivateFrameWindow( hWnd );
//		}
//		return hWnd;
//	}





/* �T�N���G�f�B�^�̑S�I�� */
void CEditApp::TerminateApplication( void )
{
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
//	int				nSettingType;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	if( pShareData->m_Common.m_bExitConfirm ){	//�I�����̊m�F
		if( 0 < cShareData.GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				NULL,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				"���݊J���Ă���ҏW�p�̃E�B���h�E�����ׂĕ��ďI�����܂���?"
			) ){
				return;
			}
		}
	}
	/* �u���ׂẴE�B���h�E�����v�v�� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	if( CloseAllEditor() ){
		::PostMessage( pShareData->m_hwndTray, WM_CLOSE, 0, 0 );
	}
	return;
}




/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
BOOL CEditApp::CloseAllEditor( void )
{
//	int				i;
//	int				j;
//	HWND*			phWndArr;
//	DLLSHAREDATA*	pShareData;
//	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	pShareData = ::GetShareData();
	CShareData		cShareData;
	DLLSHAREDATA*	pShareData;
//	int				nSettingType;


	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	cShareData.Init();
	pShareData = cShareData.GetShareData( NULL, NULL );

	/* �S�ҏW�E�B���h�E�֏I���v�����o�� */
	if( !cShareData.RequestCloseAllEditor() ){
		return FALSE;
	}else{
		return TRUE;
	}
}




/*! �|�b�v�A�b�v���j���[(�g���C���{�^��) */
int	CEditApp::CreatePopUpMenu_L( void )
{
	int			i;
	int			j;
	int			nId;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	char		szMemu[300];
	POINT		po;
	RECT		rc;
	HWND		hwndDummy;
	int			nMenuNum;
	FileInfo*	pfi;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "�V�K�쐬(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "�J��(&O)...", FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP, "&Grep...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
	j = 0;
	if( m_pShareData->m_nMRUArrNum > 0 ){
		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			/* �w��t�@�C�����J����Ă��邩���ׂ� */
			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
				continue;
			}
			j++;
		}
	}
	if( j > 0 ){
		hMenuPopUp = ::CreateMenu();
		j = 0;
		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			if( j >= m_pShareData->m_Common.m_nMRUArrNum_MAX )
				break;

			/* �w��t�@�C�����J����Ă��邩���ׂ� */
			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
				continue;
			}

//	From Here Oct. 4, 2000 JEPRO added, commented out & modified
//		�t�@�C������p�X����'&'���g���Ă���Ƃ��ɗ��𓙂ŃL�`���ƕ\������Ȃ������C��(&��&&�ɒu�����邾��)
			char	szFile2[_MAX_PATH + 3];	//	'+1'���ȁH �悤�킩���̂ő��߂ɂ��Ƃ��B�킩��l�C���񍐂��������I
			char	*p;
			strcpy( szFile2, m_pShareData->m_fiMRUArr[i].m_szPath );
			if( (p = strchr( szFile2, '&' )) != NULL ){
				char	buf[_MAX_PATH + 3];	//	'+1'���ȁH �悤�킩���̂ő��߂ɂ��Ƃ��B�킩��l�C���񍐂��������I
				do{
					*p = '\0';
					strcpy( buf, p + strlen( "&" ) );
					strcat( szFile2, "&&" );
					strcat( szFile2, buf );
					p = strchr( p + strlen( "&&" ), '&' );
				} while( p != NULL );
			}

//			if( j < 10 ){
//				wsprintf( szMemu, "&%c %s", j + '0', m_pShareData->m_fiMRUArr[i].m_szPath );
//			}
//			else if( j < 10 + 26 ){
//				wsprintf( szMemu, "&%c %s", j - 10 + 'A', m_pShareData->m_fiMRUArr[i].m_szPath );
//			}else{
//				wsprintf( szMemu, "  %s", m_pShareData->m_fiMRUArr[i].m_szPath );
//			}
//		�C���̓s�����ƕϐ��ŏ����悤�ɕύX
//		j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�@�C���̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), szFile2 );
//		To Here Oct. 4, 2000
			if( 0 != m_pShareData->m_fiMRUArr[i].m_nCharCode ){		/* �����R�[�h��� */
				switch( m_pShareData->m_fiMRUArr[i].m_nCharCode ){
//	From Here Oct. 5, 2000 JEPRO commented out & modified
//				case 1:
//					strcat( szMemu, "  [JIS]" );
//					break;
//				case 2:
//					strcat( szMemu, "  [EUC]" );
//					break;
//				case 3:
//					strcat( szMemu, "  [Unicode]" );
//					break;
					case CODE_JIS:		/* JIS */
						strcat( szMemu, "  [JIS]" );
						break;
					case CODE_EUC:		/* EUC */
						strcat( szMemu, "  [EUC]" );
						break;
					case CODE_UNICODE:	/* Unicode */
						strcat( szMemu, "  [Unicode]" );
						break;
					case CODE_UTF8:		/* UTF-8 */
						strcat( szMemu, "  [UTF-8]" );
						break;
					case CODE_UTF7:		/* UTF-7 */
						strcat( szMemu, "  [UTF-7]" );
						break;
//	To Here Oct. 5, 2000
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu );
			j++;
//			if( m_cShareData.IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
//				if( 0 != _stricmp( m_cEditDoc.m_szFilePath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
//					::SetMenuItemBitmaps( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED, NULL, m_hbmpOPENED );
//					::CheckMenuItem( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED );
//				}else{
//					::SetMenuItemBitmaps( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED, NULL, m_hbmpOPENED_THIS );
//					::CheckMenuItem( hMenuPopUp, IDM_SELMRU + i, MF_BYCOMMAND | MF_CHECKED );
//				}
//			}
		}
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
	}else{
		hMenuPopUp = ::CreateMenu();
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�@�C��(&F)" );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
	if( m_pShareData->m_nOPENFOLDERArrNum > 0 ){
		hMenuPopUp = ::CreateMenu();
		j = 0;
		for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum ; ++i ){
			if( j >= m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX )
				break;

//	From Here Oct. 4, 2000 JEPRO added, commented out & modified
//		�t�@�C������p�X����'&'���g���Ă���Ƃ��ɗ��𓙂ŃL�`���ƕ\������Ȃ������C��(&��&&�ɒu�����邾��)
			char	szFolder2[_MAX_PATH + 3];	//	'+1'���ȁH �悤�킩���̂ő��߂ɂ��Ƃ��B�킩��l�C���񍐂��������I
			char	*p;
			strcpy( szFolder2, m_pShareData->m_szOPENFOLDERArr[i] );
			if( (p = strchr( szFolder2, '&' )) != NULL ){
				char	buf[_MAX_PATH + 3];	//	'+1'���ȁH �悤�킩���̂ő��߂ɂ��Ƃ��B�킩��l�C���񍐂��������I
				do{
					*p = '\0';
					strcpy( buf, p + strlen("&") );
					strcat( szFolder2, "&&" );
					strcat( szFolder2, buf );
					p = strchr( p + strlen( "&&" ), '&' );
				} while( p != NULL );
			}

//			if( j < 10 ){
//				wsprintf( szMemu, "&%c %s", j + '0', m_pShareData->m_szOPENFOLDERArr[i] );
//			}
//			else if( j < 10 + 26 ){
//				wsprintf( szMemu, "&%c %s", j - 10 + 'A', m_pShareData->m_szOPENFOLDERArr[i] );
//			}else{
//				wsprintf( szMemu, "  %s", m_pShareData->m_szOPENFOLDERArr[i] );
//			}
//		�C���̓s�����ƕϐ��ŏ����悤�ɕύX
//		j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�H���_�̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), szFolder2 );
//		To Here Oct. 4, 2000
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu );
			j++;
		}
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
	}else{
		hMenuPopUp = ::CreateMenu();
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , "�ŋߎg�����t�H���_(&D)" );
	}


	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL, "���ׂẴE�B���h�E�����(&Q)", FALSE );	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)



	/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			++j;
		}
	}
//	::InsertMenu( hMenu, nMenuNum, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	++nMenuNum;

	if( j > 0 ){
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
		j = 0;
		for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
			if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
				/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
				::SendMessage( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
//				pfi = (FileInfo*)m_pShareData->m_szWork;
				pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
					if( pfi->m_bIsGrep ){
						/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
						CMemory		cmemDes;
						int			nDesLen;
						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						pszDes = cmemDes.GetPtr( NULL );
						nDesLen = lstrlen( pszDes );
//	From Here Oct. 4, 2000 JEPRO commented out & modified
//						wsprintf( szMemu, "&%d �yGrep�z\"%s%s\"", ((i + 1) <= 9)? (i + 1):9,
//							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "�E�E�E":""
//						);
//					}else{
//						wsprintf( szMemu, "&%d %s %s", ((i + 1) <= 9)? (i + 1):9,
//							(0 < lstrlen( pfi->m_szPath ))?pfi->m_szPath:"�i����j",
//							pfi->m_bIsModified ? "*":" "
//						);
//		j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���J���t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
						wsprintf( szMemu, "&%c �yGrep�z\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							pszDes, ( (int)lstrlen( pfi->m_szGrepKey ) > nDesLen ) ? "�E�E�E":""
						);
					}else{
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							(0 < lstrlen( pfi->m_szPath ))?pfi->m_szPath:"�i����j",
							pfi->m_bIsModified ? "*":" "
						);
//		To Here Oct. 4, 2000
						if( 0 != pfi->m_nCharCode ){		/* �����R�[�h��� */
							switch( pfi->m_nCharCode ){
							case CODE_JIS:		/* JIS */
								strcat( szMemu, "  [JIS]" );
								break;
							case CODE_EUC:		/* EUC */
								strcat( szMemu, "  [EUC]" );
								break;
							case CODE_UNICODE:	/* Unicode */
								strcat( szMemu, "  [Unicode]" );
								break;
							case CODE_UTF8:		/* UTF-8 */
								strcat( szMemu, "  [UTF-8]" );
								break;
							case CODE_UTF7:		/* UTF-7 */
								strcat( szMemu, "  [UTF-7]" );
								break;
							}
						}
					}
//
//				if( j <= 9 ){
//					wsprintf( szMemu, "&%d %s %s", j,
//						(0 < lstrlen( pfi->m_szPath ))?pfi->m_szPath:"�i����j",
//						pfi->m_bIsModified ? "*":" " );
//				}else{
//					wsprintf( szMemu, "&%c %s %s", 'A' + j - 10,
//						(0 < lstrlen( pfi->m_szPath ))?pfi->m_szPath:"�i����j",
//						pfi->m_bIsModified ? "*":" " );
//				}
//				if( 0 != pfi->m_nCharCode ){		/* �����R�[�h��� */
//					switch( pfi->m_nCharCode ){
//					case 1:	/* JIS */
//						strcat( szMemu, "  [JIS]" );
//						break;
//					case 2:	/* EUC */
//						strcat( szMemu, "  [EUC]" );
//						break;
//					case 3:	/* Unicode */
//						strcat( szMemu, "  [Unicode]" );
//						break;
//					}
//				}


//				::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMemu, FALSE );
				++j;
			}
		}
		if( j > 0 ){
//			::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
			++nMenuNum;
		}
		nMenuNum += j;
	}
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_WIN_CLOSEALL, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	//	Jun. 9, 2001 genta �\�t�g�E�F�A������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "�T�N���G�f�B�^�̑S�I��(&X)", FALSE );	//Dec. 26, 2000 JEPRO F_�ɕύX

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}





//	Oct. 12, 2000 JEPRO ���̃R�[�h�𕔕��I�ɉ���ɂ�������S���R�����g�A�E�g����
//	���̉��ɐV���ɒǉ����邱�Ƃɂ����̂ł����͐G��Ȃ��I
/* �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */
//int	CEditApp::CreatePopUpMenu_R( void )
//{
//	int		i;
//	int		j;
//	int		nId;
//	HMENU	hMenuTop;
//	HMENU	hMenu;
//	char	szMemu[300];
//	POINT	po;
//	RECT	rc;
//
////	m_CMenuDrawer.ResetContents();
//
//
//	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );	//Oct. 12, 2000 jepro note: �����Ń��\�[�X��Menu�ɂ���u�g���C���{�^���|�b�v�A�b�v���j���[�v��ǂݍ���ł���
//	hMenu = ::GetSubMenu( hMenuTop, 0 );
//	if( m_pShareData->m_nMRUArrNum > 0 ){	//Oct. 12, 2000 jepro note: �����ōŋߎg�����t�@�C���̃��X�g���쐬���Ă���
//		j = 0;
//		::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
//			if( m_pShareData->m_Common.m_nMRUArrNum_MAX <= i ){
//				break;
//			}
////	From Here Oct. 4, 2000 JEPRO commented out & modified
////			if( j <= 9 ){
////				wsprintf( szMemu, "&%d %s", j, m_pShareData->m_fiMRUArr[i].m_szPath );
////			}else{
////				wsprintf( szMemu, "&%c %s", 'A' + j - 10, m_pShareData->m_fiMRUArr[i].m_szPath );
////			}
////		j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���t�@�C���̗���MAX��36�ɂ��Ă���̂Ŏ�����OK�ł��傤
//			wsprintf( szMemu, "&%c %s", (j < 10)?('0' + j):('A' + j - 10), m_pShareData->m_fiMRUArr[i].m_szPath );
////	To Here Oct. 4, 2000
//			::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
////			m_CMenuDrawer.MyAppendMenu(
////				hMenu, MF_BYPOSITION | MF_STRING | MF_ENABLED,
////				IDM_SELWINDOW + i , szMemu
////			);
//			++j;
//		}
//	}
//	::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL );
//	po.x = 0;
//	po.y = 0;
//	::GetCursorPos( &po );
//	po.y -= 4;
//
//	rc.left = 0;
//	rc.right = 0;
//	rc.top = 0;
//	rc.bottom = 0;
//
//	::SetForegroundWindow( m_hWnd );
//	nId = ::TrackPopupMenu(
//		hMenu,
//		TPM_BOTTOMALIGN
//		| TPM_RIGHTALIGN
//		| TPM_RETURNCMD
//		| TPM_LEFTBUTTON
//		/*| TPM_RIGHTBUTTON*/
//		,
//		po.x,
//		po.y,
//		0,
//		m_hWnd,
//		&rc
//	);
//	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
//	::DestroyMenu( hMenuTop );
////	MYTRACE( "nId=%d\n", nId );
//	return nId;
//}




//�L�[���[�h�F�g���C�E�N���b�N���j���[����
//	Oct. 12, 2000 JEPRO �|�b�v�A�b�v���j���[(�g���C���{�^��) ���Q�l�ɂ��ĐV���ɒǉ���������

/*! �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */
int	CEditApp::CreatePopUpMenu_R( void )
{
	int		i;
	int		nId;
	HMENU	hMenuTop;
	HMENU	hMenu;
	POINT	po;
	RECT	rc;
	int		nMenuNum;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	/* �g���C�E�N���b�N�́u�I�v�V�����v���j���[ */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_TYPE_LIST, "�^�C�v�ʐݒ�ꗗ(&L)...", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION_TYPE, "�^�C�v�ʐݒ�(&Y)...", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION, "���ʐݒ�(&C)...", FALSE );				//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_FONT, "�t�H���g�ݒ�(&F)...", FALSE );			//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );


	/* �g���C�E�N���b�N�́u�w���v�v���j���[ */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "�w���v�ڎ�(&O)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , "�w���v�L�[���[�h����(&S)", FALSE );	//Nov. 25, 2000 JEPRO �u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC , "�R�}���h�ꗗ(&M)", FALSE );	//Jan. 12, 2001 JEPRO �܂��R�����g�A�E�g��ꍆ (T_T)
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHELP1 , "�O���w���v�P(&E)", FALSE );			//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHTMLHELP , "�O��HTML�w���v(&H)", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_ABOUT, "�o�[�W�������(&A)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, "�o�[�W�������(&A)", FALSE );	//Dec. 25, 2000 JEPRO F_�ɕύX

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_EXITALL, "�e�L�X�g�G�f�B�^�̑S�I��(&X)", FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "�e�L�X�g�G�f�B�^�̑S�I��(&X)", FALSE );	//Dec. 26, 2000 JEPRO F_�ɕύX
	//	Jun. 18, 2001 genta �\�t�g�E�F�A������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, "�T�N���G�f�B�^�̑S�I��(&X)", FALSE );	//De

	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;

	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow( m_hWnd );
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		m_hWnd,
		&rc
	);
	::PostMessage( m_hWnd, WM_USER + 1, 0, 0 );
	::DestroyMenu( hMenuTop );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}



/*!
	�R�}���h���C���̃`�F�b�N���s���āA�I�v�V�����ԍ���
	����������ꍇ�͂��̐擪�A�h���X��Ԃ��B
	CEditApp::ParseCommandLine()�Ŏg����B

	@return �I�v�V�����̔ԍ��B�ǂ�ɂ��Y�����Ȃ��Ƃ���0��Ԃ��B

	@author genta
	@date Apr. 6, 2001
*/
int CEditApp::CheckCommandLine(
	char*  str, //!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
	char** arg	//!< [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
)
{

	/*!
		�R�}���h���C���I�v�V������͗p�\���̔z��
	*/
	struct _CmdLineOpt {
		const char *opt;	//!< �I�v�V����������
		int len;	//!< �I�v�V�����̕����񒷁i�v�Z���Ȃ����߁j
		int value;	//!< �ϊ���̒l
	};

	/*!
		�R�}���h���C���I�v�V����
		���Ɉ��������Ȃ�����
	*/
	static const _CmdLineOpt _COptWoA[] = {
		"CODE", 4, 1001,
		"R", 1, 1002,
		"NOWIN", 5, 1003,
		"GREPMODE", 8, 1100,
		"DEBUGMODE", 9, 1999,
		NULL, 0, 0
	};

	/*!
		�R�}���h���C���I�v�V����
		���Ɉ�����������
	*/
	static const _CmdLineOpt _COptWithA[] = {
		"X", 1, 1,
		"Y", 1, 2,
		"VX", 2, 3,
		"VY", 2, 4,
		"GKEY", 4, 101,
		"GFILE", 5, 102,
		"GFOLDER", 7, 103,
		"GOPT", 4, 104,
		NULL, 0, 0
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str );

	//	����������ꍇ���Ɋm�F
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ ){
		if( len >= ptr->len &&	//	����������Ă��邩
			//	�I�v�V���������̒����`�F�b�N
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&
			//	������̔�r
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			*arg = str + ptr->len + 1;
			return ptr->value;
		}
	}

	//	�������Ȃ��ꍇ
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ ){
		if( len == ptr->len &&	//	�����`�F�b�N
			//	������̔�r
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			return ptr->value;
		}
	}
	return 0;	//	�Y������
}

/*! �R�}���h���C���̉��

	WinMain()����Ăяo�����B
*/
void CEditApp::ParseCommandLine(
	const char*	pszCmdLineSrc,	//!< [in]�R�}���h���C��������
	BOOL*		pbGrepMode,	//!< [out] TRUE: Grep Mode
	CMemory*	pcmGrepKey,	//!< [out] Grep��Key
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	BOOL*		pbGrepSubFolder,
	BOOL*		pbGrepLoHiCase,
	BOOL*		pbGrepRegularExp,
	BOOL*		pbGrepKanjiCode_AutoDetect,
	BOOL*		pbGrepOutputLine,
	BOOL*		pbGrepWordOnly,
	int	*		pnGrepOutputStyle,
	BOOL*		pbDebugMode,
	BOOL*		pbNoWindow,	//!< [out] TRUE: �ҏWWindow���J���Ȃ�
	FileInfo*	pfi,
	BOOL*		pbReadOnly	//!< [out] TRUE: Read Only
)
{
	BOOL			bGrepMode;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	BOOL			bGrepSubFolder;
	BOOL			bGrepLoHiCase;
	BOOL			bGrepRegularExp;
	BOOL			bGrepKanjiCode_AutoDetect;
	BOOL			bGrepOutputLine;
	BOOL			bGrepWordOnly;
	int				nGrepOutputStyle;
	BOOL			bDebugMode;
	BOOL			bNoWindow;
	FileInfo		fi;
	BOOL			bReadOnly;
	char*			pszCmdLineWork;
	int				nCmdLineWorkLen;
	BOOL			bFind;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	char			szPath[_MAX_PATH + 1];
	int				i;
	int				j;
	int				nPos;
	char*			pszToken;
	CMemory			cmWork;
	//const char*		pszOpt;
	//int				nOptLen;

	bGrepMode = FALSE;
	bGrepSubFolder = FALSE;
	bGrepLoHiCase = FALSE;
	bGrepRegularExp = FALSE;
	bGrepKanjiCode_AutoDetect = FALSE;
	bGrepOutputLine = FALSE;
	bGrepWordOnly = FALSE;
	nGrepOutputStyle = 1;
	bDebugMode = FALSE;
	bNoWindow = FALSE;

	//	Oct. 19, 2001 genta �����l��-1�ɂ��āC�w��L��/�����𔻕ʉ\�ɂ��Ă݂�
	fi.m_nViewTopLine = -1;				/* �\����̈�ԏ�̍s(0�J�n) */
	fi.m_nViewLeftCol = -1;				/* �\����̈�ԍ��̌�(0�J�n) */
	fi.m_nX = -1;						/* �J�[�\�� �����ʒu(�s������̃o�C�g��) */
	fi.m_nY = -1;						/* �J�[�\�� �����ʒu(�܂�Ԃ������s�ʒu) */
	fi.m_bIsModified = 0;				/* �ύX�t���O */
	fi.m_nCharCode = CODE_AUTODETECT;	/* �����R�[�h��� *//* �����R�[�h�������� */
	fi.m_szPath[0] = '\0';				/* �t�@�C���� */
	bReadOnly = FALSE;					/* �ǂݎ���p�� */

	//	May 30, 2000 genta
	//	���s�t�@�C���������ƂɊ����R�[�h���Œ肷��D
	{
		char	exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int		len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '5' )
					fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}





	bFind = FALSE;
	if( pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(pszCmdLineSrc); ++i ){
			if( pszCmdLineSrc[i] != ' ' && pszCmdLineSrc[i] != '\0' ){
				szPath[j] = pszCmdLineSrc[i];
				++j;
				continue;
			}
			/* �t�@�C���̑��݂ƁA�t�@�C�����ǂ������`�F�b�N */
			if( -1 != _access( szPath, 0 ) ){
//? 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
//?				hFind = ::FindFirstFile( szPath, &w32fd );
//?				::FindClose( hFind );
//?				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
//?					w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
//?				}else{
					bFind = TRUE;
					break;
//?				}
			}
			szPath[j] = pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( fi.m_szPath, szPath );	/* �t�@�C���� */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	pszCmdLineWork = new char[lstrlen( pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, pszCmdLineSrc );
	nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
		if( !bFind && pszToken[0] != '-' ){
			if( pszToken[0] == '\"' ){
				cmWork.SetData( &pszToken[1],  lstrlen( pszToken ) - 2 );
				cmWork.Replace( "\"\"", "\"" );
				strcpy( fi.m_szPath, cmWork.GetPtr( NULL/*&nDummy*/ ) );	/* �t�@�C���� */
			}else{
				strcpy( fi.m_szPath, pszToken );							/* �t�@�C���� */
			}
		}else{
			++pszToken;	//	�擪��'-'��skip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case 1: //	X
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nX = atoi( arg ) - 1;
				break;
			case 2:	//	Y
				fi.m_nY = atoi( arg ) - 1;
				break;
			case 3:	// VX
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewLeftCol = atoi( arg ) - 1;
				break;
			case 4:	//	VY
				/* �s���w���1�J�n�ɂ��� */
				fi.m_nViewTopLine = atoi( arg ) - 1;
				break;
			case 1001:	//	CODE
				fi.m_nCharCode = atoi( arg );
				break;
			case 1002:	//	R
				bReadOnly = TRUE;
				break;
			case 1003:	//	NOWIN
				bNoWindow = TRUE;
				break;
			case 1100:	//	GREPMODE
				bGrepMode = TRUE;
				break;
			case 101:	//	GKEY
				//	�O���""����菜��
				cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case 102:	//	GFILE
				//	�O���""����菜��
				cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case 103:	//	GFOLDER
				cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case 104:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* �T�u�t�H���_������������� */
						bGrepSubFolder = TRUE;	break;
					case 'L':	/* �p�啶���Ɖp����������ʂ��� */
						bGrepLoHiCase = TRUE;	break;
					case 'R':	/* ���K�\�� */
						bGrepRegularExp = TRUE;	break;
					case 'K':	/* �����R�[�h�������� */
						bGrepKanjiCode_AutoDetect = TRUE;	break;
					case 'P':	/* �s���o�͂��邩�Y�����������o�͂��邩 */
						bGrepOutputLine = TRUE;	break;
					case 'W':	/* �P��P�ʂŒT�� */
						bGrepWordOnly = TRUE;	break;
					case '1':	/* Grep: �o�͌`�� */
						nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: �o�͌`�� */
						nGrepOutputStyle = 2;	break;
					}
				}
				break;
			case 1999:
				bDebugMode = TRUE;
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* �t�@�C���� */
	if( '\0' != fi.m_szPath[0] ){
		/* �V���[�g�J�b�g(.lnk)�̉��� */
		if( TRUE == ResolveShortcutLink( NULL, fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}
		/* �����O�t�@�C�������擾���� */
		if( TRUE == ::GetLongFileName( fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}

		/* MRU������擾 */

	}

	/* �������ʂ��i�[ */
	*pbGrepMode					= bGrepMode;
	*pcmGrepKey					= cmGrepKey;
	*pcmGrepFile				= cmGrepFile;
	*pcmGrepFolder				= cmGrepFolder;
	*pbGrepSubFolder			= bGrepSubFolder;
	*pbGrepLoHiCase				= bGrepLoHiCase;
	*pbGrepRegularExp			= bGrepRegularExp;
	*pbGrepKanjiCode_AutoDetect = bGrepKanjiCode_AutoDetect;
	*pbGrepOutputLine			= bGrepOutputLine;
	*pbGrepWordOnly				= bGrepWordOnly;
	*pnGrepOutputStyle			= nGrepOutputStyle;
	*pbDebugMode				= bDebugMode;
	*pbNoWindow					= bNoWindow;
	*pfi						= fi;
	*pbReadOnly					= bReadOnly;
	return;
}


/*[EOF]*/
