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
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#define ID_HOTKEY_TRAYMENU	0x1234

#include <windows.h>
#include <io.h>
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
//  	int				nDataLen;
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
	|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	*/
	wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d",
		cmWork1.GetPtr(),
		cmWork2.GetPtr(),
		cmWork3.GetPtr(),
		m_cDlgGrep.m_nGrepCharSet
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
//	2002/09/20 Moca �����R�[�h�Z�b�g�I�v�V�����ɓ���
//	if( m_cDlgGrep.m_bKanjiCode_AutoDetect ){	/* �����R�[�h�������� */
//		strcat( pOpt, "K" );
//	}
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
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditApp::CEditApp() :
	//	Apr. 24, 2001 genta
	m_uCreateTaskBarMsg( ::RegisterWindowMessage( TEXT("TaskbarCreated") ) ),
	m_bCreatedTrayIcon( FALSE ),	//�g���C�ɃA�C�R���������
	m_hInstance( NULL ),
	m_hWnd( NULL )
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
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

	m_bUseTrayMenu = false;

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
	MY_RUNNINGTIMER( cRunningTimer, "CEditApp::Create" );

	WNDCLASS	wc;
//	HANDLE		hMutex;
//	HWND		hWnd;
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
		if( 0 == atom ){
			::MessageBox( NULL, "CEditApp::Create()\n�E�B���h�E�N���X��o�^�ł��܂���ł����B",
					GSTR_APPNAME, MB_OK | MB_ICONSTOP );
		}
//	}else{
//		return NULL;
//	}
	g_m_pCEditApp = this;

	/*hWnd =*/ ::CreateWindow(
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
	MY_TRACETIME( cRunningTimer, "Window is created" );

	//	Oct. 16, 2000 genta
	m_hIcons.Create( m_hInstance, m_hWnd );
	
	MY_TRACETIME( cRunningTimer, "Icons are created" );
	
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
			//	Dec. 02, 2002 genta
			hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
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
//	return;
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
	int				nId;
//	int				i;
//	int				j;
	HWND			hwndWork;
	//static CDlgGrep	cDlgGrep;  //Stonee, 2001/03/21 Grep�𑽏d�N�������Ƃ��G���[�ɂȂ�̂�Grep������ʊ֐��ɂ���
	LPHELPINFO		lphi;
//	HWND			hwndExitingDlg;

//	CEditWnd*	pCEditWnd_Test;
//	char*		pszCmdLine;
	int			nRowNum;
	EditNode*	pEditNodeArr;
//	HWND		hwndNew;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
//	UINT				idCtl;	/* �R���g���[����ID */
	LPMEASUREITEMSTRUCT	lpmis;	/* ���ڃT�C�Y��� */
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;
	static bool			bLDClick = false;	/* ���_�u���N���b�N�������� 03/02/20 ai */

	switch ( uMsg ){
	case WM_MENUCHAR:
		/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );
	case WM_DRAWITEM:
//		idCtl = (UINT) wParam;				/* �R���g���[����ID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		switch( lpdis->CtlType ){
		case ODT_MENU:	/* �I�[�i�[�`�惁�j���[ */
			/* ���j���[�A�C�e���`�� */
			m_CMenuDrawer.DrawItem( lpdis );
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
//		idCtl = (UINT) wParam;					// control identifier
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
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			char	szClassName[100];
			char	szText[256];

			hwndWork = ::GetForegroundWindow();
			szClassName[0] = '\0';
			::GetClassName( hwndWork, szClassName, sizeof( szClassName ) - 1 );
			::GetWindowText( hwndWork, szText, sizeof( szText ) - 1 );
			if( 0 == strcmp( szText, "���ʐݒ�" ) ){
				return -1;
			}

			if( ID_HOTKEY_TRAYMENU == idHotKey
			 &&	( wHotKeyMods )  == fuModifiers
			 && wHotKeyCode == uVirtKey
			){
				// Jan. 1, 2003 AROKA
				// �^�X�N�g���C���j���[�̕\���^�C�~���O��LBUTTONDOWN��LBUTTONUP�ɕύX�������Ƃɂ��
				::PostMessage( m_hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONUP );
			}
		}
		return 0;

	case MYWM_HTMLHELP:
		{
			HH_AKLINK	link;
			char		szHtmlHelpFile[1024];
			int			nLen;
			int			nLenKey;
			char*		pszKey;
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
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
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
			case WM_RBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
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
//				case F_OPTION:
					/* ���ʐݒ� */
//					{
						/* �ݒ�v���p�e�B�V�[�g �e�X�g�p */
//						m_pcEditDoc->bOpenPropertySheet( -1/*, -1*/ );
//					}
//					break;
//				case F_OPTION_TYPE:
					/* �^�C�v�ʐݒ� */
//					{
//						CEditDoc::OpenPropertySheetTypes( -1, m_nSettingType );
//					}
//					break;
//				case F_TYPE_LIST:
					/* �^�C�v�ʐݒ�ꗗ */
//					{
//						CDlgTypeList	cDlgTypeList;
//						int				nSettingType;
//						nSettingType = m_pcEditDoc->m_nSettingType;
//						if( cDlgTypeList.DoModal( m_hInstance, m_hWnd, &nSettingType ) ){
//							/* �^�C�v�ʐݒ� */
//							m_pcEditDoc->OpenPropertySheetTypes( -1, nSettingType );
//						}
//					}
//					break;
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
					do{
						if( CShareData::getInstance()->ExtWinHelpIsSet() ) {	//	���ʐݒ�̂݊m�F
//						if( 0 != strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
							break;
						}else
						{
							::MessageBeep( MB_ICONHAND );
						}
					}while(IDYES == ::MYMESSAGEBOX( 
							NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
							GSTR_APPNAME,
							"�O���w���v�P���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?")
					);/*do-while*/
#if 0
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

//						CMemory		cmemCurText;
						/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
//						GetCurrentTextForSearch( cmemCurText );
//						::WinHelp( m_hwndParent, m_pShareData->m_Common.m_szExtHelp1, HELP_KEY, (DWORD)(char*)cmemCurText.GetPtr() );
//						break;
					}
#endif
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
				//	Mar. 29, 2003 genta �O�̂��߃t���O�N���A
				bLDClick = false;
				return 0L;
			case WM_LBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
//				MYTRACE( "WM_LBUTTONDOWN\n" );
				/* 03/02/20 ���_�u���N���b�N��̓��j���[��\�����Ȃ� ai Start */
				if( bLDClick ){
					bLDClick = false;
					return 0L;
				}
				/* 03/02/20 ai End */
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
//						int				i;
//						int				j;
						char**			ppszMRU;
						char**			ppszOPENFOLDER;

						/* MRU���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�ɂ��ׂĈ˗�����
						CMRU cMRU;
//						ppszMRU = NULL;
						ppszMRU = new char*[ cMRU.Length() + 1 ];
						cMRU.GetPathList(ppszMRU);

						/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
						CMRUFolder cMRUFolder;
//						ppszOPENFOLDER = NULL;
						ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
						cMRUFolder.GetPathList(ppszOPENFOLDER);

						/* �t�@�C���I�[�v���_�C�A���O�̏����� */
						strcpy( szPath, "" );
						nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
						bReadOnly = FALSE;
						cDlgOpenFile.Create(
							m_hInstance,
							NULL/*m_hWnd*/,
							"*.*",
							ppszMRU[0],//@@@ 2001.12.26 YAZAKI m_fiMRUArr�ɂ̓A�N�Z�X���Ȃ�
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
						if( CShareData::getInstance()->IsPathOpened( szPath, &hWndOwner ) ){
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
								char*	pszCodeNameCur = "";
								char*	pszCodeNameNew = "";
								// gm_pszCodeNameArr_1 ���g���悤�ɕύX Moca. 2002/05/26
								if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
									pszCodeNameCur = (char *)gm_pszCodeNameArr_1[pfi->m_nCharCode];
								}
								if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
									pszCodeNameNew = (char *)gm_pszCodeNameArr_1[nCharCode];
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
				case F_GREP_DIALOG:
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
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
						CMRU cMRU;
						FileInfo openFileInfo;
						cMRU.GetFileInfo(nId - IDM_SELMRU, &openFileInfo);

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
					}else
					if( nId - IDM_SELOPENFOLDER  >= 0 &&
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
//							int				i;
//							int				j;
							char**			ppszMRU;
							char**			ppszOPENFOLDER;

							/* MRU���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
							CMRU cMRU;
							ppszMRU = NULL;
							ppszMRU = new char*[ cMRU.Length() + 1 ];
							cMRU.GetPathList(ppszMRU);
							/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
							CMRUFolder cMRUFolder;
//							ppszOPENFOLDER = NULL;
							ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
							cMRUFolder.GetPathList(ppszOPENFOLDER);

							//Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
							NetConnect( cMRUFolder.GetPath( nId - IDM_SELOPENFOLDER ) );

							/* �t�@�C���I�[�v���_�C�A���O�̏����� */
							strcpy( szPath, "" );
							nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
							bReadOnly = FALSE;
							cDlgOpenFile.Create(
								m_hInstance,
								NULL/*m_hWnd*/,
								"*.*",
								ppszOPENFOLDER[ nId - IDM_SELOPENFOLDER ],
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
							if( CShareData::getInstance()->IsPathOpened( szPath, &hWndOwner ) ){
								::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
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
									char*	pszCodeNameCur = "";
									char*	pszCodeNameNew = "";
									// gm_pszCodeNameArr_1 ���g���悤�ɕύX Moca. 2002/05/26
									if( -1 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
										pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
									}
									if( -1 < nCharCode && nCharCode < CODE_CODEMAX ){
										pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
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
				bLDClick = true;		/* 03/02/20 ai */
				/* �V�K�ҏW�E�B���h�E�̒ǉ� */
				OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
				// Apr. 1, 2003 genta ���̌�ŕ\�����ꂽ���j���[�͕���
				::PostMessage( m_hWnd, WM_CANCELMODE, 0, 0 );
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
			CShareData::getInstance()->SaveShareData();

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
			break;	/* default */
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




/*!	�V�K�ҏW�E�B���h�E�̒ǉ� ver 0

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date Oct. 24, 2000 genta WinExec -> CreateProcess�D�����@�\��t��
*/
bool CEditApp::OpenNewEditor( HINSTANCE hInstance, HWND hWndParent, char* pszPath, int nCharCode, BOOL bReadOnly, bool sync )
{
	DLLSHAREDATA*	pShareData;
	char szCmdLineBuf[1024];	//	�R�}���h���C��
	char szEXE[MAX_PATH + 1];	//	�A�v���P�[�V�����p�X��
	int nPos = 0;				//	�R�}���h���C���\�z�p�|�C���^

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	pShareData = CShareData::getInstance()->GetShareData();

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
//: do error check nPos

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


/*!	�V�K�ҏW�E�B���h�E�̒ǉ� ver 2:

	@date Oct. 24, 2000 genta create.
*/
bool CEditApp::OpenNewEditor2( HINSTANCE hInstance, HWND hWndParent, FileInfo* pfi, BOOL bReadOnly, bool sync )
{
	char			pszCmdLine[1024];
	DLLSHAREDATA*	pShareData;
	int				nPos = 0;		//	�����쐬�p�|�C���^

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	pShareData = CShareData::getInstance()->GetShareData();

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
//: do error check nPos
	}
	//	::MessageBox( NULL, pszCmdLine, "OpenNewEditor", MB_OK );
	return OpenNewEditor( hInstance, hWndParent, pszCmdLine, CODE_AUTODETECT, bReadOnly, sync );

}
//	To Here Oct. 24, 2000 genta





/*!	�T�N���G�f�B�^�̑S�I��

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
void CEditApp::TerminateApplication( void )
{
	DLLSHAREDATA*	pShareData;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	pShareData = CShareData::getInstance()->GetShareData();

	/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	if( pShareData->m_Common.m_bExitConfirm ){	//�I�����̊m�F
		if( 0 < CShareData::getInstance()->GetEditorWindowsNum() ){
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




/*!	���ׂẴE�B���h�E�����
	
	@date Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
BOOL CEditApp::CloseAllEditor( void )
{
	/* �S�ҏW�E�B���h�E�֏I���v�����o�� */
	if( !CShareData::getInstance()->RequestCloseAllEditor() ){
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
	char		szMemu[100 + MAX_PATH * 2];	//	Jan. 19, 2001 genta
	char		szMenu2[MAX_PATH * 2];	//	Jan. 19, 2001 genta
	POINT		po;
	RECT		rc;
//	HWND		hwndDummy;
	int			nMenuNum;
	FileInfo*	pfi;

	//�{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();
	CShareData::getInstance()->TransformFileName_MakeCash();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, "�V�K�쐬(&N)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, "�J��(&O)...", FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, "&Grep...", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
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

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 ){
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, "�ŋߎg�����t�H���_(&D)" );
	}
	else {
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, "�ŋߎg�����t�H���_(&D)" );
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
//						int			nDesLen;
//						const char*	pszDes;
						LimitStringLengthB( pfi->m_szGrepKey, lstrlen( pfi->m_szGrepKey ), 64, cmemDes );
						//	Jan. 19, 2002 genta
						//	���j���[�������&���l��
						dupamp( cmemDes.GetPtr(), szMenu2 );
//	From Here Oct. 4, 2000 JEPRO commented out & modified
//		j >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���J���t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
						//	Jan. 19, 2002 genta
						//	&�̏d��������ǉ��������ߌp��������኱�ύX
						wsprintf( szMemu, "&%c �yGrep�z\"%s%s\"", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							szMenu2, ( (int)lstrlen( pfi->m_szGrepKey ) > cmemDes.GetLength() ) ? "�c":""
						);
					}else{
						char szFileName[_MAX_PATH];
						// 2003/01/27 Moca �t�@�C�����̊ȈՕ\��
						CShareData::getInstance()->GetTransformFileNameFast( pfi->m_szPath, szFileName, MAX_PATH );
						//	Jan. 19, 2002 genta
						//	���j���[�������&���l��
						dupamp( szFileName, szMenu2 );
						wsprintf( szMemu, "&%c %s %s", ((1 + i) <= 9)?('1' + i):('A' + i - 9),
							(0 < lstrlen( szMenu2 ))? szMenu2:"�i����j",
							pfi->m_bIsModified ? "*":" "
						);
//		To Here Oct. 4, 2000
						// gm_pszCodeNameArr_3 ����R�s�[����悤�ɕύX
						if( 0 < pfi->m_nCharCode && pfi->m_nCharCode < CODE_CODEMAX ){
							strcat( szMemu, gm_pszCodeNameArr_3[pfi->m_nCharCode] );
						}
					}

//				::InsertMenu( hMenu, IDM_EXITALL, MF_BYCOMMAND | MF_STRING, IDM_SELWINDOW + i, szMemu );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMemu, FALSE );
				++j;
			}
		}
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

	m_bUseTrayMenu = false;

	return nId;
}

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

	//�{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if( m_bUseTrayMenu ) return -1;
	m_bUseTrayMenu = true;

	m_CMenuDrawer.ResetContents();

	hMenuTop = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU_L ) );
	hMenu = ::GetSubMenu( hMenuTop, 0 );
	nMenuNum = ::GetMenuItemCount( hMenu )/* - 1*/;
	for( i = nMenuNum - 1; i >= 0; i-- ){
		::DeleteMenu( hMenu, i, MF_BYPOSITION );
	}

#if 0
	2002/04/26 YAZAKI �g���Ȃ����͕̂\�����Ȃ�

	/* �g���C�E�N���b�N�́u�I�v�V�����v���j���[ */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_TYPE_LIST, "�^�C�v�ʐݒ�ꗗ(&L)...", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION_TYPE, "�^�C�v�ʐݒ�(&Y)...", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_OPTION, "���ʐݒ�(&C)...", FALSE );				//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_FONT, "�t�H���g�ݒ�(&F)...", FALSE );			//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
#endif

	/* �g���C�E�N���b�N�́u�w���v�v���j���[ */
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , "�w���v�ڎ�(&O)", FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , "�w���v�L�[���[�h����(&S)", FALSE );	//Nov. 25, 2000 JEPRO �u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
//	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC , "�R�}���h�ꗗ(&M)", FALSE );	//Jan. 12, 2001 JEPRO �܂��R�����g�A�E�g��ꍆ (T_T)
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

#if 0
	2002/04/26 YAZAKI �g���Ȃ����͕̂\�����Ȃ�

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHELP1 , "�O���w���v�P(&E)", FALSE );			//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_EXTHTMLHELP , "�O��HTML�w���v(&H)", FALSE );	//Jan. 12, 2001 JEPRO ���̃��j���[���ڂ𖳌�������
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
#endif

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

	m_bUseTrayMenu = false;

	return nId;
}

/*[EOF]*/
