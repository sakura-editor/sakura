/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, ao
	Copyright (C) 2001, MIK, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta, hor, aroka, minfu, �S, MIK, ai
	Copyright (C) 2003, genta, MIK, Moca, wmlhq, ryoji, KEITA
	Copyright (C) 2004, genta, Moca, yasu, MIK, novice, Kazika
	Copyright (C) 2005, genta, MIK, Moca, aroka, ryoji
	Copyright (C) 2006, genta, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <mbctype.h>
#include <mbstring.h>

#include "CControlTray.h"
#include "window/CEditWnd.h"
#include "sakura_rc.h"
#include "doc/CEditDoc.h"
#include "doc/CDocLine.h"
#include "debug/Debug.h"
#include "dlg/CDlgAbout.h"
#include "env/CShareData.h"
#include "CPrint.h"
#include "charset/charcode.h"
#include "global.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgPrintPage.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "macro/CSMacroMgr.h" // Jun. 16, 2002 genta
#include "COsVersioninfo.h"	// Sep. 6, 2003 genta
#include "debug/CRunningTimer.h"
#include "charset/CharPointer.h"
#include "CEditApp.h"
#include "util/module.h"
#include "util/os.h"
#include "util/window.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/string_ex2.h"
#include <shlobj.h>
#include "env/CSakuraEnvironment.h"
#include "util/os.h" //WM_MOUSEWHEEL,WM_THEMECHANGED
#include "env/CSakuraEnvironment.h"



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
	CEditWnd* pcWnd = ( CEditWnd* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
	if( pcWnd ){
		return pcWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}

#pragma warning(disable:4355) //�uthis�|�C���^�����������X�g�Ŏg�p����܂����v�̌x���𖳌���

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditWnd::CEditWnd()
: m_hWnd( NULL )
, m_bDragMode( FALSE )
, m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ) // 20020331 aroka �ĕϊ��Ή� for 95/NT
, m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
, m_pPrintPreview( NULL ) //@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
, m_pszLastCaption( NULL )
, m_nCurrentFocus( 0 )
, m_bIsActiveApp( false )
, m_IconClicked(icNone) //by �S(2)
, m_cToolbar(this)
, m_cStatusBar(this)
, m_nActivePaneIndex( 0 )
, m_pcDragSourceView( NULL )
{
	g_pcEditWnd=this;
	for(int i=0;i<4;i++)
		m_pcEditViewArr[i]=new CEditView(this);

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_pcDropTarget = new CDropTarget( this );	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji
}

CEditWnd::~CEditWnd()
{
	g_pcEditWnd=NULL;

	delete[] m_pszLastCaption;

	for(int i=0;i<4;i++)
		delete m_pcEditViewArr[i];

	//	Dec. 4, 2002 genta
	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;	// 2008.06.20 ryoji
	m_pcDropTarget = NULL;

	m_hWnd = NULL;
}


//! �h�L�������g���X�i�F�Z�[�u��
// 2008.02.02 kobake
void CEditWnd::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	//�r���[�ĕ`��
	this->Views_RedrawAll();

	//�L���v�V�����̍X�V���s��
	UpdateCaption();

	/* �L�����b�g�̍s���ʒu��\������ */
	this->GetActiveView().GetCaret().ShowCaretPosInfo();
}

void CEditWnd::UpdateCaption()
{
	if( !GetActiveView().GetDrawSwitch() )return;

	//�L���v�V����������̐��� -> pszCap
	wchar_t	pszCap[1024];
	const CommonSetting_Window& setting = GetDllShareData().m_Common.m_sWindow;
	const wchar_t* pszFormat = NULL;
	if( !this->IsActiveApp() )	pszFormat = to_wchar(setting.m_szWindowCaptionInactive);
	else						pszFormat = to_wchar(setting.m_szWindowCaptionActive);
	CSakuraEnvironment::ExpandParameter(
		pszFormat,
		pszCap,
		_countof( pszCap )
	);

	//�L���v�V�����X�V
	::SetWindowText( this->GetHwnd(), to_tchar(pszCap) );

	//@@@ From Here 2003.06.13 MIK
	//�^�u�E�C���h�E�̃t�@�C������ʒm
	CSakuraEnvironment::ExpandParameter( GetDllShareData().m_Common.m_sTabBar.m_szTabWndCaption, pszCap, _countof( pszCap ));
	this->ChangeFileNameNotify( to_tchar(pszCap), GetListeningDoc()->m_cDocFile.GetFilePath(), CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode );	// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	//@@@ To Here 2003.06.13 MIK
}



//!< �E�B���h�E�����p�̋�`���擾
void CEditWnd::_GetWindowRectForInit(CMyRect* rcResult, int nGroup, const STabGroupInfo& sTabGroupInfo)
{
	/* �E�B���h�E�T�C�Y�p�� */
	int	nWinCX, nWinCY;
	//	2004.05.13 Moca m_Common.m_eSaveWindowSize��BOOL����enum�ɕς�������
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_sWindow.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_sWindow.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* �E�B���h�E�T�C�Y�w�� */
	EditInfo fi;
	CCommandLine::Instance()->GetEditInfo(&fi);
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
	// �E�B���h�E�ʒu�Œ�
	//	2004.05.13 Moca �ۑ������E�B���h�E�ʒu���g���ꍇ�͋��L����������Z�b�g
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos ){
		nWinOX =  m_pShareData->m_Common.m_sWindow.m_nWinPosX;
		nWinOY =  m_pShareData->m_Common.m_sWindow.m_nWinPosY;
	}

	//	2004.05.13 Moca �}���`�f�B�X�v���C�ł͕��̒l���L���Ȃ̂ŁC
	//	���ݒ�̔�����@��ύX�D(���̒l��CW_USEDEFAULT)
	if( fi.m_nWindowOriginX != CW_USEDEFAULT ){
		nWinOX = fi.m_nWindowOriginX;
	}
	if( fi.m_nWindowOriginY != CW_USEDEFAULT ){
		nWinOY = fi.m_nWindowOriginY;
	}

	// �K�v�Ȃ�A�^�u�O���[�v�Ƀt�B�b�g����悤�A�ύX
	if(sTabGroupInfo.IsValid()){
		RECT rcWork, rcMon;
		GetMonitorWorkRect( sTabGroupInfo.hwndTop, &rcWork, &rcMon );

		const WINDOWPLACEMENT& wpTop = sTabGroupInfo.wpTop;
		nWinCX = wpTop.rcNormalPosition.right  - wpTop.rcNormalPosition.left;
		nWinCY = wpTop.rcNormalPosition.bottom - wpTop.rcNormalPosition.top;
		nWinOX = wpTop.rcNormalPosition.left   + (rcWork.left - rcMon.left);
		nWinOY = wpTop.rcNormalPosition.top    + (rcWork.top - rcMon.top);
	}

	//����
	rcResult->SetXYWH(nWinOX,nWinOY,nWinCX,nWinCY);
}

HWND CEditWnd::_CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo)
{
	// -- -- -- -- �E�B���h�E�N���X�o�^ -- -- -- -- //
	WNDCLASSEX	wc;
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= G_AppInstance();
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	wc.hIcon			= GetAppIcon( G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true );
	ATOM	atom = RegisterClassEx( &wc );
	if( 0 == atom ){
		//	2004.05.13 Moca return NULL��L���ɂ���
		return NULL;
	}

	//��`�擾
	CMyRect rc;
	_GetWindowRectForInit(&rc, nGroup, sTabGroupInfo);

	//�쐬
	HWND hwndResult = ::CreateWindowEx(
		0,				 	// extended window style
		GSTR_EDITWINDOWNAME,		// pointer to registered class name
		GSTR_EDITWINDOWNAME,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		rc.left,			// horizontal position of window
		rc.top,				// vertical position of window
		rc.Width(),			// window width
		rc.Height(),		// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		G_AppInstance(),		// handle to application instance
		NULL				// pointer to window-creation data
	);
	return hwndResult;
}

void CEditWnd::_GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup)
{
	HWND hwndTop = NULL;
	WINDOWPLACEMENT	wpTop = {0};

	//From Here @@@ 2003.05.31 MIK
	//�^�u�E�C���h�E�̏ꍇ�͌���l���w��
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		if( nGroup < 0 )	// �s���ȃO���[�vID
			nGroup = 0;	// �O���[�v�w�薳���i�ŋ߃A�N�e�B�u�̃O���[�v�ɓ����j
		EditNode*	pEditNode = CAppNodeGroupHandle(nGroup).GetEditNodeAt(0);	// �O���[�v�̐擪�E�B���h�E�����擾	// 2007.06.20 ryoji
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if( hwndTop )
		{
			//	Sep. 11, 2003 MIK �V�KTAB�E�B���h�E�̈ʒu����ɂ���Ȃ��悤��
			// 2007.06.20 ryoji ��v���C�}�����j�^�܂��̓^�X�N�o�[�𓮂�������ł�����Ȃ��悤��

			wpTop.length = sizeof(wpTop);
			if( ::GetWindowPlacement( hwndTop, &wpTop ) ){	// ���݂̐擪�E�B���h�E����ʒu���擾
				if( wpTop.showCmd == SW_SHOWMINIMIZED )
					wpTop.showCmd = pEditNode->m_showCmdRestore;
			}
			else{
				hwndTop = NULL;
			}
		}
	}
	//To Here @@@ 2003.05.31 MIK

	//����
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void CEditWnd::_AdjustInMonitor(const STabGroupInfo& sTabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( GetHwnd(), &rcDesktop );
	::GetWindowRect( GetHwnd(), &rcOrg );

	// 2005.11.23 Moca �}���`���j�^���Ŗ�肪���������ߌv�Z���@�ύX
	/* �E�B���h�E�ʒu���� */
	if( rcOrg.bottom > rcDesktop.bottom ){
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right > rcDesktop.right ){
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	
	if( rcOrg.top < rcDesktop.top ){
		rcOrg.bottom += rcDesktop.top - rcOrg.top;
		rcOrg.top = rcDesktop.top;
	}
	if( rcOrg.left < rcDesktop.left ){
		rcOrg.right += rcDesktop.left - rcOrg.left;
		rcOrg.left = rcDesktop.left;
	}

	/* �E�B���h�E�T�C�Y���� */
	if( rcOrg.bottom > rcDesktop.bottom ){
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if( rcOrg.right > rcDesktop.right ){
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}

	//From Here @@@ 2003.06.13 MIK
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
		&& !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin
		&& sTabGroupInfo.hwndTop )
	{
		// ���݂̐擪�E�B���h�E���� WS_EX_TOPMOST ��Ԃ������p��	// 2007.05.18 ryoji
		DWORD dwExStyle = (DWORD)::GetWindowLongPtr( sTabGroupInfo.hwndTop, GWL_EXSTYLE );
		::SetWindowPos( GetHwnd(), (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

		//�^�u�E�C���h�E���͌�����ێ�
		/* �E�B���h�E�T�C�Y�p�� */
		if( sTabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED )
		{
			::ShowWindow( GetHwnd(), SW_SHOWMAXIMIZED );
		}
		else
		{
			::ShowWindow( GetHwnd(), SW_SHOW );
		}
	}
	else
	{
		::SetWindowPos(
			GetHwnd(), 0,
			rcOrg.left, rcOrg.top,
			rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		/* �E�B���h�E�T�C�Y�p�� */
		if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_sWindow.m_nWinSizeType == SIZE_MAXIMIZED ){
			::ShowWindow( GetHwnd(), SW_SHOWMAXIMIZED );
		}else
		// 2004.05.14 Moca �E�B���h�E�T�C�Y�𒼐ڎw�肷��ꍇ�́A�ŏ����\�����󂯓����
		if( WINSIZEMODE_SET == m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_sWindow.m_nWinSizeType == SIZE_MINIMIZED ){
			::ShowWindow( GetHwnd(), SW_SHOWMINIMIZED );
		}
		else{
			::ShowWindow( GetHwnd(), SW_SHOW );
		}
	}
	//To Here @@@ 2003.06.13 MIK
}

/*!
	�쐬

	@date 2002.03.07 genta nDocumentType�ǉ�
	@date 2007.06.26 ryoji nGroup�ǉ�
	@date 2008.04.19 ryoji ����A�C�h�����O���o�p�[���b�^�C�}�[�̃Z�b�g������ǉ�
*/
HWND CEditWnd::Create(
	int				nGroup			//!< [in] �O���[�vID
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	//�E�B���h�E������
	if( m_pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		return NULL;
	}

	//�^�u�O���[�v���擾
	STabGroupInfo sTabGroupInfo;
	_GetTabGroupInfo(&sTabGroupInfo, nGroup);


	// -- -- -- -- �E�B���h�E�쐬 -- -- -- -- //
	HWND hWnd = _CreateMainWindow(nGroup, sTabGroupInfo);
	if(!hWnd)return NULL;
	m_hWnd = hWnd;

	// ����A�C�h�����O���o�p�̃[���b�^�C�}�[���Z�b�g����	// 2008.04.19 ryoji
	// �[���b�^�C�}�[�������i����A�C�h�����O���o�j������ MYWM_FIRST_IDLE ���N�����v���Z�X�Ƀ|�X�g����B
	// ���N�����ł̋N����A�C�h�����O���o�ɂ��Ă� CControlTray::OpenNewEditor ���Q��
	::SetTimer( GetHwnd(), IDT_FIRST_IDLE, 0, NULL );

	//�R�����R���g���[��������
	MyInitCommonControls();

	//�C���[�W�A�w���p�Ȃǂ̍쐬
	m_CMenuDrawer.Create( G_AppInstance(), GetHwnd(), &CEditApp::Instance()->GetIcons() );


	// -- -- -- -- �q�E�B���h�E�쐬 -- -- -- -- //

	/* �����t���[���쐬 */
	m_cSplitterWnd.Create( G_AppInstance(), GetHwnd(), this );

	/* �r���[ */
	m_pcEditViewArr[0]->Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), 0, TRUE  );
	m_pcEditViewArr[1]->Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), 1, FALSE );
	m_pcEditViewArr[2]->Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), 2, FALSE );
	m_pcEditViewArr[3]->Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), 3, FALSE );

	m_pcEditViewArr[0]->OnSetFocus();

	/* �q�E�B���h�E�̐ݒ� */
	HWND		hWndArr[4];
	hWndArr[0] = m_pcEditViewArr[0]->GetHwnd();
	hWndArr[1] = m_pcEditViewArr[1]->GetHwnd();
	hWndArr[2] = m_pcEditViewArr[2]->GetHwnd();
	hWndArr[3] = m_pcEditViewArr[3]->GetHwnd();
	m_cSplitterWnd.SetChildWndArr( hWndArr );

	MY_TRACETIME( cRunningTimer, "View created" );

	// -- -- -- -- �_�C�A���O�쐬 -- -- -- -- //

	/* ���͕⊮�E�B���h�E�쐬 */
	m_cHokanMgr.DoModeless(
		G_AppInstance(),
		m_pcEditViewArr[0]->GetHwnd(),
		(LPARAM)m_pcEditViewArr[0]
	);


	// -- -- -- -- �e��o�[�쐬 -- -- -- -- //

	/* �c�[���o�[ */
	LayoutToolBar();

	/* �X�e�[�^�X�o�[ */
	LayoutStatusBar();

	/* �t�@���N�V�����L�[ �o�[ */
	LayoutFuncKey();

	/* �^�u�E�C���h�E */
	LayoutTabBar();

	/* �o�[�̔z�u�I�� */
	EndLayoutBars( FALSE );


	// -- -- -- -- ���̑������Ȃ� -- -- -- -- //

	// ��ʕ\�����O��DispatchEvent��L��������
	::SetWindowLongPtr( GetHwnd(), GWLP_USERDATA, (LONG_PTR)this );

	// �f�X�N�g�b�v����͂ݏo���Ȃ��悤�ɂ���
	_AdjustInMonitor(sTabGroupInfo);

	// �h���b�v���ꂽ�t�@�C�����󂯓����
	::DragAcceptFiles( GetHwnd(), TRUE );
	m_pcDropTarget->Register_DropTarget( m_hWnd );	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

	//�A�N�e�B�u���
	m_bIsActiveApp = ( ::GetActiveWindow() == GetHwnd() );	// 2007.03.08 ryoji

	// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F�iVista UIPI�@�\�j 2007.06.07 ryoji
	if( COsVersionInfo().IsWinVista_or_later() ){
		m_bUIPI = FALSE;
		::SendMessage( m_pShareData->m_sHandles.m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd() );
		if( !m_bUIPI ){	// �Ԏ����Ԃ�Ȃ�
			TopErrorMessage( GetHwnd(), 
				_T("�G�f�B�^�Ԃ̑Θb�Ɏ��s���܂����B\n")
				_T("�������x���̈قȂ�G�f�B�^�����ɋN�����Ă���\��������܂��B")
			);
			::DestroyWindow( GetHwnd() );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	/* �ҏW�E�B���h�E���X�g�ւ̓o�^ */
	if( !CAppNodeGroupHandle(nGroup).AddEditWndList( GetHwnd() ) ){	// 2007.06.26 ryoji nGroup�����ǉ�
		OkMessage( GetHwnd(), _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		::DestroyWindow( GetHwnd() );
		m_hWnd = hWnd = NULL;
		return hWnd;
	}
	CShareData::getInstance()->SetTraceOutSource( GetHwnd() );	// TraceOut()�N�����E�B���h�E�̐ݒ�	// 2006.06.26 ryoji

	//	Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	/* �^�C�}�[���N�� */ // �^�C�}�[��ID�ƊԊu��ύX 20060128 aroka
	if( 0 == ::SetTimer( GetHwnd(), IDT_EDIT, 500, NULL ) ){
		WarningMessage( GetHwnd(), _T("CEditWnd::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
	}
	// �c�[���o�[�̃^�C�}�[�𕪗����� 20060128 aroka
	Timer_ONOFF( TRUE );

	//�f�t�H���g��IME���[�h�ݒ�
	GetDocument().m_cDocEditor.SetImeMode( CDocTypeManager().GetTypeSetting(CTypeConfig(0)).m_nImeState );

	return GetHwnd();
}



//! �N�����̃t�@�C���I�[�v������
void CEditWnd::OpenDocumentWhenStart(
	const SLoadInfo& _sLoadInfo		//!< [in]
)
{
	if( _sLoadInfo.cFilePath.Length() ){
		::ShowWindow( GetHwnd(), SW_SHOW );
		//	Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
		SLoadInfo	sLoadInfo = _sLoadInfo;
		bool		bReadResult = GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
		if( !bReadResult ){
			/* �t�@�C�������ɊJ����Ă��� */
			if( sLoadInfo.bOpened ){
				::PostMessageAny( GetHwnd(), WM_CLOSE, 0, 0 );
				// 2004.07.12 Moca return NULL���ƁA���b�Z�[�W���[�v��ʂ炸�ɂ��̂܂ܔj������Ă��܂��A�^�u�̏I��������������
				//	���̌�͐��탋�[�g�Ń��b�Z�[�W���[�v�ɓ�������WM_CLOSE����M���Ē�����CLOSE & DESTROY�ƂȂ�D
				//	���̒��ŕҏW�E�B���h�E�̍폜���s����D
			}
		}
	}
}

void CEditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		//!< [in] �����R�[�h
	bool			bViewMode,		//!< [in] �r���[���[�h�ŊJ�����ǂ���
	CTypeConfig		nDocumentType	//!< [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
)
{
	// �����R�[�h�̎w��	2008/6/14 Uchi
	if (IsValidCodeType(nCharCode)) {
		GetDocument().SetDocumentEncoding(nCharCode);
	}

	//	Mar. 7, 2002 genta �����^�C�v�̋����w��
	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��^�C�v�����w���L���ɂ���
	if( nDocumentType.IsValid() ){
		GetDocument().m_cDocType.SetDocumentType( nDocumentType, true );
		//	2002/05/07 YAZAKI �^�C�v�ʐݒ�ꗗ�̈ꎞ�K�p�̃R�[�h�𗬗p
		GetDocument().m_cDocType.LockDocumentType();
		/* �ݒ�ύX�𔽉f������ */
		GetDocument().OnChangeSetting();
	}

	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��r���[���[�h�����w���L���ɂ���
	CAppMode::Instance()->SetViewMode(bViewMode);
}


/*! �c�[���o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutToolBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR ){	/* �c�[���o�[��\������ */
		m_cToolbar.CreateToolBar();
	}else{
		m_cToolbar.DestroyToolBar();
	}
}

/*! �X�e�[�^�X�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutStatusBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR ){	/* �X�e�[�^�X�o�[��\������ */
		/* �X�e�[�^�X�o�[�쐬 */
		m_cStatusBar.CreateStatusBar();
	}
	else{
		/* �X�e�[�^�X�o�[�j�� */
		m_cStatusBar.DestroyStatusBar();
	}
}

/*! �t�@���N�V�����L�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutFuncKey( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND ){	/* �t�@���N�V�����L�[��\������ */
		if( NULL == m_CFuncKeyWnd.GetHwnd() ){
			bool	bSizeBox;
			if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
				bSizeBox = false;
			}else{
				bSizeBox = true;
				/* �X�e�[�^�X�o�[������Ƃ��̓T�C�Y�{�b�N�X��\�����Ȃ� */
				if( m_cStatusBar.GetStatusHwnd() ){
					bSizeBox = false;
				}
			}
			m_CFuncKeyWnd.Open( G_AppInstance(), GetHwnd(), &GetDocument(), bSizeBox );
		}
	}else{
		m_CFuncKeyWnd.Close();
	}
}

/*! �^�u�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditWnd::LayoutTabBar( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd ){	/* �^�u�o�[��\������ */
		if( NULL == m_cTabWnd.GetHwnd() ){
			m_cTabWnd.Open( G_AppInstance(), GetHwnd() );
		}
	}else{
		m_cTabWnd.Close();
	}
}

/*! �o�[�̔z�u�I������
	@date 2006.12.19 ryoji �V�K�쐬
	@data 2007.03.04 ryoji ����v���r���[���̓o�[���B��
*/
void CEditWnd::EndLayoutBars( BOOL bAdjust/* = TRUE*/ )
{
	int nCmdShow = m_pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = (NULL != m_cToolbar.GetRebarHwnd())? m_cToolbar.GetRebarHwnd(): m_cToolbar.GetToolbarHwnd();
	if( NULL != hwndToolBar )
		::ShowWindow( hwndToolBar, nCmdShow );
	if( m_cStatusBar.GetStatusHwnd() )
		::ShowWindow( m_cStatusBar.GetStatusHwnd(), nCmdShow );
	if( NULL != m_CFuncKeyWnd.GetHwnd() )
		::ShowWindow( m_CFuncKeyWnd.GetHwnd(), nCmdShow );
	if( NULL != m_cTabWnd.GetHwnd() )
		::ShowWindow( m_cTabWnd.GetHwnd(), nCmdShow );

	if( bAdjust )
	{
		RECT		rc;
		m_cSplitterWnd.DoSplit( -1, -1 );
		::GetClientRect( GetHwnd(), &rc );
		::SendMessageAny( GetHwnd(), WM_SIZE, m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		::RedrawWindow( GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW );	// �X�e�[�^�X�o�[�ɕK�v�H

		GetActiveView().SetIMECompFormPos();
	}
}

static BOOL MyIsDialogMessage(HWND hwnd, MSG* msg)
{
	if(hwnd==NULL)return FALSE;
	return ::IsDialogMessage(hwnd, msg);
}

//�����v���Z�X��
/* ���b�Z�[�W���[�v */
//2004.02.17 Moca GetMessage�̃G���[�`�F�b�N
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	int ret;

	while(GetHwnd())
	{
		//���b�Z�[�W�擾
		ret = GetMessage(&msg,NULL,0,0);
		if(ret== 0)break; //WM_QUIT
		if(ret==-1)break; //GetMessage���s

		if(0){}
		//�_�C�A���O���b�Z�[�W
		else if( MyIsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg ) ){}	//!< ����v���r���[ ����o�[
		else if( MyIsDialogMessage( m_cDlgFind.GetHwnd(),								&msg ) ){}	//!<�u�����v�_�C�A���O
		else if( MyIsDialogMessage( m_cDlgFuncList.GetHwnd(),							&msg ) ){}	//!<�u�A�E�g���C���v�_�C�A���O
		else if( MyIsDialogMessage( m_cDlgReplace.GetHwnd(),							&msg ) ){}	//!<�u�u���v�_�C�A���O
		else if( MyIsDialogMessage( m_cDlgGrep.GetHwnd(),								&msg ) ){}	//!<�uGrep�v�_�C�A���O
		else if( MyIsDialogMessage( m_cHokanMgr.GetHwnd(),								&msg ) ){}	//!<�u���͕⊮�v
		else if( m_cToolbar.EatMessage(&msg ) ){ }													//!<�c�[���o�[
		//�A�N�Z�����[�^
		else if( m_pShareData->m_sHandles.m_hAccel && TranslateAccelerator( msg.hwnd, m_pShareData->m_sHandles.m_hAccel, &msg ) ){}
		//�ʏ탁�b�Z�[�W
		else{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
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
	int					nPane;
	EditInfo*			pfi;
	LPHELPINFO			lphi;
	const wchar_t*		pLine;
	CLogicInt			nLineLen;

	UINT				idCtl;	/* �R���g���[����ID */
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	/* ���ڕ`���� */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	UINT				fuFlags;
	HMENU				hmenu;
	LRESULT				lRes;

	switch( uMsg ){
	case WM_PAINTICON:
		return 0;
	case WM_ICONERASEBKGND:
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

	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		if( !wParam ){
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_MENUSELECT:
		if( NULL == m_cStatusBar.GetStatusHwnd() ){
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		fuFlags = (UINT) HIWORD(wParam);	// menu flags
		hmenu = (HMENU) lParam;				// handle to menu clicked
		{
			/* ���j���[�@�\�̃e�L�X�g���Z�b�g */
			CNativeT	cmemWork;

			/* �@�\�ɑΉ�����L�[���̎擾(����) */
			CNativeT**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				G_AppInstance(),
				m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
				(KEYDATA*)m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if( 0 < nAssignedKeyNum ){
				for( j = 0; j < nAssignedKeyNum; ++j ){
					if( j > 0 ){
						cmemWork.AppendString(_T(" , L"));
					}
					cmemWork.AppendNativeData( *ppcAssignedKeyList[j] );
					delete ppcAssignedKeyList[j];
				}
				delete [] ppcAssignedKeyList;
			}

			const TCHAR* pszItemStr = cmemWork.GetStringPtr();

			m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, pszItemStr);


		}
		return 0;


	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* �R���g���[����ID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* ���ڕ`���� */
		if( IDW_STATUSBAR == idCtl ){
			if( 5 == lpdis->itemID ){ // 2003.08.26 Moca id������č�悳��Ȃ�����
				int	nColor;
				if( m_pShareData->m_sFlags.m_bRecordingKeyMacro	/* �L�[�{�[�h�}�N���̋L�^�� */
				 && m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetHwnd()	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				){
					nColor = COLOR_BTNTEXT;
				}else{
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor( lpdis->hDC, ::GetSysColor( nColor ) );
				::SetBkMode( lpdis->hDC, TRANSPARENT );
				
				// 2003.08.26 Moca �㉺�����ʒu�ɍ��
				TEXTMETRIC tm;
				::GetTextMetrics( lpdis->hDC, &tm );
				int y = ( lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1 ) / 2 + lpdis->rcItem.top;
				::TextOut( lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen( _T("REC") ) );
				if( COLOR_BTNTEXT == nColor ){
					::TextOut( lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen( _T("REC") ) );
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


//			MYTRACE_A( "WM_MEASUREITEM  lpmis->itemID=%d\n", lpmis->itemID );
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
		return GetActiveView().GetCommander().HandleCommand( F_PASTE, TRUE, 0, 0, 0, 0 );

	case WM_COPY:
		return GetActiveView().GetCommander().HandleCommand( F_COPY, TRUE, 0, 0, 0, 0 );

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			CEditApp::Instance()->ShowFuncHelp( hwnd, (EFunctionCode)lphi->iCtrlId );
			break;
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		m_bIsActiveApp = (wParam != 0);	// ���A�v�����A�N�e�B�u���ǂ���

		// �A�N�e�B�u���Ȃ�ҏW�E�B���h�E���X�g�̐擪�Ɉړ�����		// 2007.04.08 ryoji WM_SETFOCUS ����ړ�
		if( m_bIsActiveApp ){
			CAppNodeGroupHandle(0).AddEditWndList( GetHwnd() );	// ���X�g�ړ�����
		}

		// �L���v�V�����ݒ�A�^�C�}�[ON/OFF		// 2007.03.08 ryoji WM_ACTIVATE����ړ�
		UpdateCaption();
		m_CFuncKeyWnd.Timer_ONOFF( m_bIsActiveApp ); // 20060126 aroka
		this->Timer_ONOFF( m_bIsActiveApp ); // 20060128 aroka

		return 0L;

	case WM_WINDOWPOSCHANGED:
		// �|�b�v�A�b�v�E�B���h�E�̕\���ؑ֎w�����|�X�g����	// 2007.10.22 ryoji
		// �EWM_SHOWWINDOW�͂��ׂĂ̕\���ؑւŌĂ΂��킯�ł͂Ȃ��̂�WM_WINDOWPOSCHANGED�ŏ���
		//   �i�^�u�O���[�v�����Ȃǂ̐ݒ�ύX����WM_SHOWWINDOW�͌Ă΂�Ȃ��j
		// �E�����ؑւ��ƃ^�u�ؑւɊ����Č��̃^�u�ɖ߂��Ă��܂����Ƃ�����̂Ō�Ő؂�ւ���
		WINDOWPOS* pwp;
		pwp = (WINDOWPOS*)lParam;
		if( pwp->flags & SWP_SHOWWINDOW )
			::PostMessage( hwnd, MYWM_SHOWOWNEDPOPUPS, TRUE, 0 );
		else if( pwp->flags & SWP_HIDEWINDOW )
			::PostMessage( hwnd, MYWM_SHOWOWNEDPOPUPS, FALSE, 0 );

		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );

	case MYWM_SHOWOWNEDPOPUPS:
		::ShowOwnedPopups( m_hWnd, (BOOL)wParam );	// 2007.10.22 ryoji
		return 0L;

	case WM_SIZE:
//		MYTRACE_A( "WM_SIZE\n" );
		/* WM_SIZE ���� */
		if( SIZE_MINIMIZED == wParam ){
			this->UpdateCaption();
		}
		return OnSize( wParam, lParam );

	//From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		//	�Ō�̈ʒu�𕜌����邽�߁C�ړ�����邽�тɋ��L�������Ɉʒu��ۑ�����D
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos ){
			if( !::IsZoomed( GetHwnd() ) && !::IsIconic( GetHwnd() ) ){
				// 2005.11.23 Moca ���[�N�G���A���W���Ƃ����̂ŃX�N���[�����W�ɕύX
				RECT rcWork;
				::GetWindowRect( hwnd, &rcWork);
				m_pShareData->m_Common.m_sWindow.m_nWinPosX = rcWork.left;
				m_pShareData->m_Common.m_sWindow.m_nWinPosY = rcWork.top;
			}
		}
		// To Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	//To here 2003.05.31 MIK
	case WM_SYSCOMMAND:
		// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
		//	Feb. 11, 2007 genta �����I�ׂ�悤��(MDI���Ə]������)
		// 2007.02.22 ryoji Alt+F4 �̃f�t�H���g�@�\�Ń��[�h���̓��삪������悤�ɂȂ���
		if( wParam == SC_CLOSE ){
			// ����v���r���[���[�h�ŃE�B���h�E����鑀��̂Ƃ��̓v���r���[�����	// 2007.03.04 ryoji
			if( m_pPrintPreview ){
				PrintPreviewModeONOFF();	// ����v���r���[���[�h�̃I��/�I�t
				return 0L;
			}
			OnCommand( 0, CKeyBind::GetDefFuncCode( VK_F4, _ALT ), NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	case WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR ) {
			/* ���b�Z�[�W�̔z�� */
			return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	//case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYUP:	// 2004.04.28 Moca ALT+�L�[�̃L�[���s�[�g�����̂��ߒǉ�
	case WM_ENTERMENULOOP:
	case MYWM_IME_REQUEST:   /*  �ĕϊ��Ή� by minfu 2002.03.27  */ // 20020331 aroka
		/* ���b�Z�[�W�̔z�� */
		return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_EXITMENULOOP:
//		MYTRACE_A( "WM_EXITMENULOOP\n" );
		if( NULL != m_cStatusBar.GetStatusHwnd() ){
			m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		/* ���b�Z�[�W�̔z�� */
		return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
	case WM_SETFOCUS:
//		MYTRACE_A( "WM_SETFOCUS\n" );

		// Aug. 29, 2003 wmlhq & ryoji�t�@�C���̃^�C���X�^���v�̃`�F�b�N���� OnTimer �Ɉڍs
		m_nTimerCount = 9;

		// �r���[�Ƀt�H�[�J�X���ړ�����	// 2007.10.16 ryoji
		::SetFocus( this->GetActiveView().GetHwnd() );
		lRes = 0;

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/* ����v���r���[���[�h�̂Ƃ��́A�L�[����͑S��PrintPreviewBar�֓]�� */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//�����{�b�N�X���X�V
		m_cToolbar.AcceptSharedSearchKey();
		
		return lRes;

	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	�X�e�[�^�X�o�[�̃_�u���N���b�N�Ń��[�h�ؑւ��ł���悤�ɂ���
		if( m_cStatusBar.GetStatusHwnd() && pnmh->hwndFrom == m_cStatusBar.GetStatusHwnd() ){
			if( pnmh->code == NM_DBLCLK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 6 ){	//	�㏑��/�}��
					GetDocument().HandleCommand( F_CHGMOD_INS );
				}
				else if( mp->dwItemSpec == 5 ){	//	�}�N���̋L�^�J�n�E�I��
					GetDocument().HandleCommand( F_RECKEYMACRO );
				}
				else if( mp->dwItemSpec == 1 ){	//	���ʒu���s�ԍ��W�����v
					GetDocument().HandleCommand( F_JUMP_DIALOG );
				}
			}
			else if( pnmh->code == NM_RCLICK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 2 ){	//	���͉��s���[�h
					m_CMenuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						_T("���͉��s�R�[�h�w��(&CRLF)") ); // ���͉��s�R�[�h�w��(CRLF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						_T("���͉��s�R�[�h�w��(&LF)") ); // ���͉��s�R�[�h�w��(LF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						_T("���͉��s�R�[�h�w��(C&R)") ); // ���͉��s�R�[�h�w��(CR)
						
					//	mp->pt�̓X�e�[�^�X�o�[�����̍��W�Ȃ̂ŁC�X�N���[�����W�ւ̕ϊ����K�v
					POINT	po = mp->pt;
					::ClientToScreen( m_cStatusBar.GetStatusHwnd(), &po );
					EFunctionCode nId = (EFunctionCode)::TrackPopupMenu(
						hMenuPopUp,
						TPM_CENTERALIGN
						| TPM_BOTTOMALIGN
						| TPM_RETURNCMD
						| TPM_LEFTBUTTON
						,
						po.x,
						po.y,
						0,
						GetHwnd(),
						NULL
					);
					::DestroyMenu( hMenuPopUp );
					GetDocument().HandleCommand( nId );
				}
			}
			return 0L;
		}
		//	To Here Feb. 15, 2004 genta 

		switch( pnmh->code ){
		// 2007.09.08 kobake TTN_NEEDTEXT�̏�����A�ł�W�łɕ����Ė����I�ɏ�������悤�ɂ��܂����B
		//                   ���e�L�X�g��80�����𒴂������Ȃ�TOOLTIPTEXT::lpszText�𗘗p���Ă��������B
		case TTN_NEEDTEXT:
			{
				//�c�[���`�b�v�e�L�X�g�擾�A�ݒ�
				LPTOOLTIPTEXT lptip = (LPTOOLTIPTEXT)pnmh;
				GetTooltipText(lptip->szText, _countof(lptip->szText), lptip->hdr.idFrom);
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu( pnmh->hwndFrom );
				if( nId != 0 ) OnCommand( (WORD)0 /*���j���[*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if( pnmh->hwndFrom == m_cToolbar.GetToolbarHwnd() ){
				//	�c�[���o�[��Owner Draw
				return m_cToolbar.ToolBarOwnerDraw( (LPNMCUSTOMDRAW)pnmh );
			}
			break;
		//	To Here Jul. 21, 2003 genta
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
	case WM_QUERYENDSESSION:	//OS�̏I��
		if( OnClose() ){
			DestroyWindow( hwnd );
			return TRUE;
		}
		else{
			return FALSE;
		}
	case WM_CLOSE:
		if( OnClose() ){
			DestroyWindow( hwnd );
		}
		return 0L;
	case WM_DESTROY:
		RELPRINT_A("CEditWnd WM_DESTROY");
		
		if( m_pShareData->m_sFlags.m_bRecordingKeyMacro ){					/* �L�[�{�[�h�}�N���̋L�^�� */
			if( m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetHwnd() ){	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
				m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;			/* �L�[�{�[�h�}�N���̋L�^�� */
				m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;		/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
			}
		}

		/* �^�C�}�[���폜 */
		::KillTimer( GetHwnd(), IDT_TOOLBAR );

		/* �h���b�v���ꂽ�t�@�C�����󂯓����̂����� */
		::DragAcceptFiles( hwnd, FALSE );
		m_pcDropTarget->Revoke_DropTarget();	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

		/* �ҏW�E�B���h�E���X�g����̍폜 */
		CAppNodeGroupHandle(GetHwnd()).DeleteEditWndList( GetHwnd() );

		if( m_pShareData->m_sHandles.m_hwndDebug == GetHwnd() ){
			m_pShareData->m_sHandles.m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
		RELPRINT_A("post MYWM_DELETE_ME to 0x%08X", m_pShareData->m_sHandles.m_hwndTray);
		::PostMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows �ɃX���b�h�̏I����v�����܂� */
		::PostQuitMessage( 0 );

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// �r�W���A���X�^�C���^�N���V�b�N�X�^�C�����؂�ւ������c�[���o�[���č쐬����
		// �i�r�W���A���X�^�C��: Rebar �L��A�N���V�b�N�X�^�C��: Rebar �����j
		if( m_cToolbar.GetToolbarHwnd() ){
			if( IsVisualStyle() == (NULL == m_cToolbar.GetRebarHwnd()) ){
				m_cToolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		/* �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W */	// 2007.06.07 ryoji
		m_bUIPI = TRUE;	// �g���C����̕Ԏ����󂯎����
		return 0L;

	case MYWM_CLOSE:
		/* �G�f�B�^�ւ̏I���v�� */
		if( FALSE != ( nRet = OnClose()) ){	// Jan. 23, 2002 genta �x���}��
			// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
			if( !(BOOL)wParam ){	// �S�I���v���łȂ��ꍇ
				// �^�u�܂Ƃߕ\����(����)���c���w��̏ꍇ�A�c�E�B���h�E���P�Ȃ�V�K�G�f�B�^���N�����ďI������
				if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd &&
					!m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin &&
					m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin
					){
					// ���O���[�v���̎c�E�B���h�E���𒲂ׂ�	// 2007.06.20 ryoji
					int nGroup = CAppNodeManager::Instance()->GetEditNode( GetHwnd() )->GetGroup();
					if( 1 == CAppNodeGroupHandle(nGroup).GetEditorWindowsNum() ){
						EditNode* pEditNode = CAppNodeManager::Instance()->GetEditNode( GetHwnd() );
						if( pEditNode )
							pEditNode->m_bClosing = TRUE;	// �����̓^�u�\�����Ă����Ȃ��Ă���
						SLoadInfo sLoadInfo;
						sLoadInfo.cFilePath = _T("");
						sLoadInfo.eCharCode = CODE_DEFAULT;
						sLoadInfo.bViewMode = false;
						CControlTray::OpenNewEditor(
							G_AppInstance(),
							GetHwnd(),
							sLoadInfo,
							NULL,
							true
						);
					}
				}
			}
			DestroyWindow( hwnd );
		}
		return nRet;


	case MYWM_GETFILEINFO:
		/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

		/* �ҏW�t�@�C�������i�[ */
		GetDocument().GetEditInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* �ݒ�ύX�̒ʒm */
		// Font�ύX�̒ʒm 2008/5/17 Uchi
		InitCharWidthCache(m_pShareData->m_Common.m_sView.m_lf);
// Oct 10, 2000 ao
/* �ݒ�ύX���A�c�[���o�[���č쐬����悤�ɂ���i�o�[�̓��e�ύX�����f�j */
		m_cToolbar.DestroyToolBar();
		LayoutToolBar();
// Oct 10, 2000 ao �����܂�

		// 2008.10.05 nasukoji	��A�N�e�B�u�ȃE�B���h�E�̃c�[���o�[���X�V����
		// �A�N�e�B�u�ȃE�B���h�E�̓^�C�}�ɂ��X�V����邪�A����ȊO�̃E�B���h�E��
		// �^�C�}���~�����Ă���ݒ�ύX����ƑS���L���ƂȂ��Ă��܂����߁A������
		// �c�[���o�[���X�V����
		if( !m_bIsActiveApp )
			m_cToolbar.UpdateToolbar();

		// �t�@���N�V�����L�[���č쐬����i�o�[�̓��e�A�ʒu�A�O���[�v�{�^�����̕ύX�����f�j	// 2006.12.19 ryoji
		m_CFuncKeyWnd.Close();
		LayoutFuncKey();

		// �^�u�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		LayoutTabBar();

		// �X�e�[�^�X�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		LayoutStatusBar();

		// �����X�N���[���o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
		{
			int i;
			bool b1;
			bool b2;
			b1 = (m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz == FALSE);
			for( i = 0; i < 4; i++ )
			{
				b2 = (m_pcEditViewArr[i]->m_hwndHScrollBar == NULL);
				if( b1 != b2 )		/* �����X�N���[���o�[���g�� */
				{
					m_pcEditViewArr[i]->DestroyScrollBar();
					m_pcEditViewArr[i]->CreateScrollBar();
				}
			}
		}

		// �o�[�ύX�ŉ�ʂ�����Ȃ��悤��	// 2006.12.19 ryoji
		EndLayoutBars();

		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd )
		{
			// �^�u�\���̂܂܃O���[�v������^���Ȃ����ύX����Ă�����^�u���X�V����K�v������
			m_cTabWnd.Refresh( FALSE );
		}
		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			if( CAppNodeManager::Instance()->GetEditNode( GetHwnd() )->IsTopInGroup() )
			{
				if( !::IsWindowVisible( GetHwnd() ) )
				{
					// ::ShowWindow( GetHwnd(), SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos( GetHwnd(), NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );

					// ���̃E�B���h�E�� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�ɔ��f����	// 2007.05.18 ryoji
					WindowTopMost( ((DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST)? 1: 2 );
				}
			}
			else
			{
				if( ::IsWindowVisible( GetHwnd() ) )
				{
					::ShowWindow( GetHwnd(), SW_HIDE );
				}
			}
		}
		else
		{
			if( !::IsWindowVisible( GetHwnd() ) )
			{
				// ::ShowWindow( GetHwnd(), SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
				::SetWindowPos( GetHwnd(), NULL,0,0,0,0,
								SWP_SHOWWINDOW | SWP_NOACTIVATE
								| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
			}
		}

		//	Aug, 21, 2000 genta
		GetDocument().m_cAutoSaveAgent.ReloadAutoSaveParam();

		GetDocument().m_cDocType.SetDocumentIcon();	// Sep. 10, 2002 genta �����A�C�R���̍Đݒ�
		GetDocument().OnChangeSetting();	/* �r���[�ɐݒ�ύX�𔽉f������ */
		return 0L;
	case MYWM_SETACTIVEPANE:
		if( -1 == (int)wParam ){
			if( 0 == lParam ){
				nPane = m_cSplitterWnd.GetFirstPane();
			}else{
				nPane = m_cSplitterWnd.GetLastPane();
			}
		}
		this->SetActivePane( nPane );
		return 0L;


	case MYWM_SETCARETPOS:	/* �J�[�\���ʒu�ύX�ʒm */
		{
			//	2006.07.09 genta LPARAM�ɐV���ȈӖ���ǉ�
			//	bit 0 (MASK 1): (bit 1==0�̂Ƃ�) 0/�I���N���A, 1/�I���J�n�E�ύX
			//	bit 1 (MASK 2): 0: bit 0�̐ݒ�ɏ]���D1:���݂̑I�����b�Ns��Ԃ��p��
			//	�����̎����ł� �ǂ����0�Ȃ̂ŋ��������Ɖ��߂����D
			//	�Ăяo������e_PM_SETCARETPOS_SELECTSTATE�̒l���g�����ƁD
			bool bSelect = (0!= (lParam & 1));
			if( lParam & 2 ){
				// ���݂̏�Ԃ�KEEP
				bSelect = GetActiveView().GetSelectionInfo().m_bSelectingLock;
			}
			
			//	2006.07.09 genta �����������Ȃ�
			/*
			�J�[�\���ʒu�ϊ�
			 �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			��
			 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			CLogicPoint* ppoCaret = m_pShareData->m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
			CLayoutPoint ptCaretPos;
			GetDocument().m_cLayoutMgr.LogicToLayout(
				*ppoCaret,
				&ptCaretPos
			);
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��	// 2007.08.22 ryoji
			// Note. ���Ƃ����s�P�ʂ̌��ʒu�Ȃ̂Ń��C�A�E�g�܂�Ԃ��̌��ʒu�𒴂��邱�Ƃ͂Ȃ��B
			//       �I���w��(bSelect==TRUE)�̏ꍇ�ɂ͂ǂ�����̂��Ó����悭�킩��Ȃ����A
			//       2007.08.22���݂ł̓A�E�g���C����̓_�C�A���O���猅�ʒu0�ŌĂяo�����
			//       �p�^�[�������Ȃ��̂Ŏ��p����ɖ��͖����B
			if( !bSelect ){
				const CDocLine *pTmpDocLine = GetDocument().m_cDocLineMgr.GetLine( ppoCaret->GetY2() );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x ) ptCaretPos.x--;
				}
			}
			//	2006.07.09 genta �I��͈͂��l�����Ĉړ�
			//	MoveCursor�̈ʒu�����@�\������̂ŁC�ŏI�s�ȍ~�ւ�
			//	�ړ��w���̒�����MoveCursor�ɂ܂�����
			GetActiveView().MoveCursorSelecting( ptCaretPos, bSelect, _CARETMARGINRATE / 3 );
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* �J�[�\���ʒu�擾�v�� */
		/*
		�J�[�\���ʒu�ϊ�
		 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		��
		�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		{
			CLogicPoint* ppoCaret = m_pShareData->m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos(),
				ppoCaret
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	/* �s(���s�P��)�f�[�^�̗v�� */
		pLine = GetDocument().m_cDocLineMgr.GetLine(CLogicInt(wParam))->GetDocLineStrWithEOL( &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > (int)m_pShareData->m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
			auto_memcpy( m_pShareData->m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, m_pShareData->m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() );
		}else{
			auto_memcpy( m_pShareData->m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
		{
			EDIT_CHAR* pWork = m_pShareData->m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();

			GetActiveView().GetCommander().HandleCommand(
				F_ADDTAIL_W,
				TRUE,
				(LPARAM)pWork,
				(LPARAM)wcslen( pWork ),
				0,
				0
			);
			GetActiveView().GetCommander().HandleCommand(
				F_GOFILEEND,
				TRUE,
				0,
				0,
				0,
				0
			);
		}
		return 0L;

	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_cTabWnd.TabWindowNotify( wParam, lParam );
		return 0L;

	//�o�[�̕\���E��\��	//@@@ 2003.06.10 MIK
	case MYWM_BAR_CHANGE_NOTIFY:
		if( GetHwnd() != (HWND)lParam )
		{
			switch( wParam )
			{
			case MYBCN_TOOLBAR:
				LayoutToolBar();	// 2006.12.19 ryoji
				break;
			case MYBCN_FUNCKEY:
				LayoutFuncKey();	// 2006.12.19 ryoji
				break;
			case MYBCN_TAB:
				LayoutTabBar();		// 2006.12.19 ryoji
				if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
					&& !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
				{
					::ShowWindow(GetHwnd(), SW_HIDE);
				}
				else
				{
					// ::ShowWindow( hwnd, SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos( hwnd, NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
				}
				break;
			case MYBCN_STATUSBAR:
				LayoutStatusBar();		// 2006.12.19 ryoji
				break;
			}
			EndLayoutBars();	// 2006.12.19 ryoji
		}
		return 0L;

	//by �S (2) MYWM_CHECKSYSMENUDBLCLK�͕s�v��, WM_LBUTTONDBLCLK�ǉ�
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			this->GetActiveView().GetCaret().ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_NCACTIVATE:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�P�j	// 2007.04.03 ryoji
		// �O�ʂɂ���̂��ҏW�E�B���h�E�Ȃ�A�N�e�B�u��Ԃ�ێ�����
		if( m_pShareData->m_sFlags.m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow()) ){
			wParam = TRUE;	// �A�N�e�B�u
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_SETTEXT:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�Q�j	// 2007.04.03 ryoji
		// �^�C�}�[���g�p���ă^�C�g���̕ύX��x������
		if( m_pShareData->m_sFlags.m_bEditWndChanging ){
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[ ::_tcslen((LPCTSTR)lParam) + 1 ];
			::_tcscpy( m_pszLastCaption, (LPCTSTR)lParam );	// �ύX��̃^�C�g�����L�����Ă���
			::SetTimer( GetHwnd(), IDT_CAPTION, 50, NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_TIMER:
		if( !OnTimer(wParam, lParam) )
			return 0L;
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
			return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
// >> by aroka
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

/*! �I�����̏���

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
int	CEditWnd::OnClose()
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	int nRet = GetDocument().OnFileClose();
	if( !nRet ) return nRet;

	// 2005.09.01 ryoji �^�u�܂Ƃߕ\���̏ꍇ�͎��̃E�B���h�E��O�ʂɁi�I�����̃E�B���h�E�������}���j

	// 2007.07.07 ryoji
	// Note. �^�u�܂Ƃ߂łȂ��ꍇ�ɂ��ȉ��̏�����ʂ��Ă���̂́A�ȑO�A�^�u�܂Ƃߐݒ�𓯊����Ă��Ȃ������Ƃ��̖��c�ł��B
	//       �i�^�u�܂Ƃ߉������Ă���\���̂܂܂ɂȂ��Ă���E�B���h�E��\���ɖ߂��K�v���������j
	//       �����ɂ��Ă��錻�݂ł́A�����ɕs��������Ĕ�\���ɂȂ��Ă���ꍇ�ł��\���ɖ߂��A�Ƃ����ی��I�ȈӖ���������܂���B

	int i, j;
	EditNode*	p = NULL;
	int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &p, FALSE );
	if( nCount > 1 )
	{
		for( i = 0; i < nCount; i++ )
		{
			if( p[ i ].GetHwnd() == GetHwnd() )
				break;
		}
		if( i < nCount )
		{
			for( j = i + 1; j < nCount; j++ )
			{
				if( p[ j ].m_nGroup == p[ i ].m_nGroup )
					break;
			}
			if( j >= nCount )
			{
				for( j = 0; j < i; j++ )
				{
					if( p[ j ].m_nGroup == p[ i ].m_nGroup )
						break;
				}
			}
			if( j != i )
			{
				HWND hwnd = p[ j ].GetHwnd();
				{
					// 2006.01.28 ryoji
					// �^�u�܂Ƃߕ\���ł��̉�ʂ���\������\���ɕς���Ă�������ꍇ(�^�u�̒��N���b�N����)�A
					// �ȑO�̃E�B���h�E�������������Ɉ�C�ɂ����܂ŏ������i��ł��܂���
					// ���Ƃŉ�ʂ�������̂ŁA�ȑO�̃E�B���h�E��������̂�������Ƃ����҂�
					if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
						&& !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
					{
						int iWait = 0;
						while( ::IsWindowVisible( hwnd ) && iWait++ < 20 )
							::Sleep(1);
					}
				}
				if( !::IsWindowVisible( hwnd ) )
				{
					ActivateFrameWindow( hwnd );
				}
			}
		}
	}
	if( p ) delete []p;

	return nRet;
}






//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s(F_HELP_CONTENTS,F_HELP_SEARCH)���C���E����
void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* ���j���[����̃��b�Z�[�W */
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta �}�E�X����Ăт����ꂽ�ꍇ
		switch( wID ){
		case F_EXITALL:	//Dec. 26, 2000 JEPRO F_�ɕύX
			/* �T�N���G�f�B�^�̑S�I�� */
			CControlTray::TerminateApplication( GetHwnd() );	// 2006.12.25 ryoji �����ǉ�
			break;

		case F_HELP_CONTENTS:
			/* �w���v�ڎ� */
			ShowWinHelpContents( GetHwnd(), CEditApp::Instance()->GetHelpFilePath() );	//	�ڎ���\������
			break;

		case F_HELP_SEARCH:
			/* �w���v�L�[���[�h���� */
			MyWinHelp( GetHwnd(), CEditApp::Instance()->GetHelpFilePath(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
			break;

		case F_ABOUT:	//Dec. 25, 2000 JEPRO F_�ɕύX
			/* �o�[�W������� */
			{
				CDlgAbout cDlgAbout;
				cDlgAbout.DoModal( G_AppInstance(), GetHwnd() );
			}
			break;
		default:
			//�E�B���h�E�؂�ւ�
			if( wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_sNodes.m_nEditArrNum ){
				ActivateFrameWindow( m_pShareData->m_sNodes.m_pEditArr[wID - IDM_SELWINDOW].GetHwnd() );
			}
			//�ŋߎg�����t�@�C��
			else if( wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999){
				/* �w��t�@�C�����J����Ă��邩���ׂ� */
				CMRU cMRU;
				EditInfo checkEditInfo;
				cMRU.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
				GetDocument().m_cDocFileOperation.FileLoad( &SLoadInfo(checkEditInfo.m_szPath, checkEditInfo.m_nCharCode, false) );	//	Oct.  9, 2004 genta ���ʊ֐���
			}
			//�ŋߎg�����t�H���_
			else if( wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999){
				//�t�H���_�擾
				CMRUFolder cMRUFolder;
				LPCTSTR pszFolderPath = cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER );

				//Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
				NetConnect( pszFolderPath );

				//�u�t�@�C�����J���v�_�C�A���O
				SLoadInfo sLoadInfo(_T(""), CODE_AUTODETECT, false);
				CDocFileOperation& cDocOp = GetDocument().m_cDocFileOperation;
				if( cDocOp.OpenFileDialog(GetHwnd(), pszFolderPath, &sLoadInfo) ){
					//�J��
					cDocOp.FileLoad( &sLoadInfo );
				}
			}
			//���̑��R�}���h
			else{
				//�r���[�Ƀt�H�[�J�X���ړ����Ă���
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
					::SetFocus( GetActiveView().GetHwnd() );

				// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
				GetDocument().HandleCommand( (EFunctionCode)MAKELONG( wID, wNotifyCode ));	//	May 19, 2006 genta ��ʃr�b�g��n��
			}
			break;
		}
		break;
	/* �A�N�Z�����[�^����̃��b�Z�[�W */
	case 1:
		{
			//�r���[�Ƀt�H�[�J�X���ړ����Ă���
			if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
				::SetFocus( GetActiveView().GetHwnd() );

			EFunctionCode nFuncCode = CKeyBind::GetFuncCode(
				wID,
				m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
				m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
			);
			GetDocument().HandleCommand( (EFunctionCode)MAKELONG( nFuncCode, wNotifyCode ) );
		}
		break;

	case CBN_SETFOCUS:
		if( NULL != m_cToolbar.GetSearchHwnd() && hwndCtl == m_cToolbar.GetSearchHwnd() )
		{
			m_nCurrentFocus = F_SEARCH_BOX;
		}
		break;

	case CBN_KILLFOCUS:
		if( NULL != m_cToolbar.GetSearchHwnd() && hwndCtl == m_cToolbar.GetSearchHwnd() )
		{
			m_nCurrentFocus = 0;

			//�t�H�[�J�X���͂��ꂽ�Ƃ��Ɍ����L�[�ɂ��Ă��܂��B

			//�����L�[���[�h���擾
			wchar_t	szText[_MAX_PATH];
			if( m_cToolbar.GetSearchKey(szText,_countof(szText)) )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CSearchKeywordManager().AddToSearchKeyArr( szText );
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
	UINT		fuFlags;
	int			i;
	BOOL		bRet;
	int			nRowNum;

	HMENU		hMenuPopUp;
	HMENU		hMenuPopUp_2;
	const TCHAR*	pszLabel;


	if( hMenu != ::GetSubMenu( ::GetMenu( GetHwnd() ), uPos ) ){
		goto end_of_func_IsEnable;
	}


	if( fSystemMenu ){
	}else{
//		MYTRACE_A( "hMenu=%08xh uPos=%d, fSystemMenu=%ls\n", hMenu, uPos, fSystemMenu ? "TRUE":"FALSE" );
		switch( uPos ){
		case 0:
			/* �u�t�@�C���v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, _T("�V�K�쐬(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, _T("�J��(&O)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, _T("�㏑���ۑ�(&S)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, _T("���O��t���ĕۑ�(&A)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL		, _T("���ׂď㏑���ۑ�(&Z)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//	Feb. 28, 2003 genta �ۑ����ĕ����ǉ��D���郁�j���[���߂��Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVECLOSE	, _T("�ۑ����ĕ���(&E)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T("����(&C)") );	//Feb. 18, 2001	JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, _T("����(����) (&R)") );	//Oct. 17, 2000 jepro �L���v�V�������u����v����ύX	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(C��B; Blank�̈Ӗ�)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, _T("���ĊJ��(&L)...") );

			// �u�����R�[�h�Z�b�g�v�|�b�v�A�b�v���j���[
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, _T("�J������(&W)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp_2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, _T("SJIS�ŊJ������(&S)") );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, _T("JIS�ŊJ������(&J)") );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, _T("EUC�ŊJ������(&E)") );			//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, _T("Unicode�ŊJ������(&U)") );	//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, _T("UnicodeBE�ŊJ������(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, _T("UTF-8�ŊJ������(&8)") );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, _T("UTF-7�ŊJ������(&7)") );		//Nov. 7, 2000 jepro �L���v�V������'�ŊJ������'��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp_2 , _T("�J������(&W)") );//Oct. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(M��H)

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, _T("���(&P)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, _T("����v���r���[(&V)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, _T("�y�[�W�ݒ�(&U)...") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�t�@�C������v�|�b�v�A�b�v���j���[
			//hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, _T("�t�@�C���̃v���p�e�B(&T)") );		//Nov. 7, 2000 jepro �L���v�V������'�t�@�C����'��ǉ�
			//m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�t�@�C������(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BROWSE						, _T("�u���E�Y(&B)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
			{
				//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
				CMRU cMRU;
				hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	�t�@�C�����j���[
				if ( cMRU.Length() > 0 ){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�ŋߎg�����t�@�C��(&F)") );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("�ŋߎg�����t�@�C��(&F)") );
				}
			}

			/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
				CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	�A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�ŋߎg�����t�H���_(&D)") );
				}
				else {
					//	��A�N�e�B�u
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("�ŋߎg�����t�H���_(&D)") );
				}
			}
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE, _T("�O���[�v�����(&G)") );	// �O���[�v�����	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T("�ҏW�̑S�I��(&Q)") );	//Feb/ 19, 2001 JEPRO �ǉ�	// 2006.10.21 ryoji �\��������ύX	// 2007.02.13 ryoji ��F_EXITALLEDITORS
			//	Jun. 9, 2001 genta �\�t�g�E�F�A������
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, _T("�T�N���G�f�B�^�̑S�I��(&X)") );	//Sept. 11, 2000 jepro �L���v�V�������u�A�v���P�[�V�����I���v����ύX	//Dec. 26, 2000 JEPRO F_�ɕύX
			break;

		case 1:
			/* �u�ҏW�v���j���[ */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, _T("���ɖ߂�(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, _T("��蒼��(&R)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, _T("�؂���(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, _T("�R�s�[(&C)") );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, _T("�S�s���p�R�s�[(&N)") );
			//	Sept. 14, 2000 JEPRO	�L���v�V�����Ɂu�L���t���v��ǉ��A�A�N�Z�X�L�[�ύX(N��.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, _T("�I��͈͓��S�s���p���t���R�s�[(&.)") );
//			Sept. 30, 2000 JEPRO	���p���t���R�s�[�̃A�C�R�����쐬�����̂ŏ�L���j���[�͏d��������āu���x�ȑ���v���ɂ��������ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, _T("�\��t��(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, _T("�폜(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, _T("���ׂđI��(&A)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );							/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, _T("�ĕϊ�(&R)") );		/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, _T("CRLF���s�ŃR�s�[(&L)") );				//Nov. 9, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, _T("�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, _T("��`�\��t��(&X)") );					//Sept. 13, 2000 JEPRO �ړ��ɔ����A�N�Z�X�L�[�t�^	//Oct. 22, 2000 JEPRO �A�N�Z�X�L�[�ύX(P��X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, _T("�J�[�\���O���폜(&B)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// �u�}���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, _T("���t(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, _T("����(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, _T("�R���g���[���R�[�h(&C)...") );	// 2004.05.06 MIK ...�ǉ�

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�}��(&I)") );

			// �u���x�ȑ���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	_T("�P��̍��[�܂ō폜(&L)") );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	_T("�P��̉E�[�܂ō폜(&R)") );			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	_T("���݈ʒu�̒P��I��(&W)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	_T("�P��؂���(&T)") );				//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	_T("�P��폜(&D)") );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	_T("�s���܂Ő؂���(���s�P��) (&U)") );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	_T("�s���܂Ő؂���(���s�P��) (&K)") );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	_T("�s���܂ō폜(���s�P��) (&H)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	_T("�s���܂ō폜(���s�P��) (&E)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	_T("�s�؂���(�܂�Ԃ��P��) (&X)") );	//Jan. 16, 2001 JEPRO �s(���E��)�֌W�̏��������ւ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	_T("�s�폜(�܂�Ԃ��P��) (&Y)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	_T("�s�̓�d��(�܂�Ԃ��P��) (&2)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	_T("TAB�C���f���g(&A)") );				//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	_T("�tTAB�C���f���g(&B)") );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	_T("SPACE�C���f���g(&S)") );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	_T("�tSPACE�C���f���g(&P)") );			//Oct. 22, 2000 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, _T("�I��͈͓��S�s�R�s�[(&@)") );		//Sept. 14, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, _T("�I��͈͓��S�s���p���t���R�s�[(&.)") );//Sept. 13, 2000 JEPRO �L���v�V��������u�L���t���v��ǉ��A�A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, _T("�I��͈͓��S�s�s�ԍ��t���R�s�[(&:)") );//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	_T("���̃t�@�C�������R�s�[(&-)") );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	_T("���̃t�@�C���̃p�X�����R�s�[(&\\)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	_T("���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[(&^)") );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, _T("�L�[���蓖�Ĉꗗ���R�s�[(&Q)") );	//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, _T("�P�ꃊ�t�@�����X(&W)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("���x�ȑ���(&V)") );

		//From Here Feb. 19, 2001 JEPRO [�ړ�(M)], [�I��(R)]���j���[��[�ҏW]�̃T�u���j���[�Ƃ��Ĉړ�
			// �u�ړ��v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, _T("�J�[�\����ړ�(�Q�s����) (&Q)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, _T("�J�[�\�����ړ�(�Q�s����) (&K)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, _T("�P��̍��[�Ɉړ�(&L)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, _T("�P��̉E�[�Ɉړ�(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, _T("�O�̒i���Ɉړ�(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, _T("���̒i���Ɉړ�(&Z)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, _T("�s���Ɉړ�(�܂�Ԃ��P��) (&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, _T("�s���Ɉړ�(�܂�Ԃ��P��) (&E)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, _T("�P�y�[�W�A�b�v(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, _T("�P�y�[�W�_�E��(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, _T("�t�@�C���̐擪�Ɉړ�(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, _T("�t�@�C���̍Ō�Ɉړ�(&B)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, _T("�J�[�\���s���E�B���h�E������(&C)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, _T("�w��s�փW�����v(&J)...") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T("�����J�n�ʒu�֖߂�(&I)") );	// �����J�n�ʒu�֖߂� 02/06/26 ai
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, _T("�ړ�����: �O��(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, _T("�ړ�����: ����(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, _T("���݈ʒu���ړ������ɓo�^(&S)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�ړ�(&O)") );

			// �u�I���v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, _T("���݈ʒu�̒P��I��(&W)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, _T("���ׂđI��(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, _T("�͈͑I���J�n(&S)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, _T("(�I��)�J�[�\����ړ�(�Q�s����) (&Q)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, _T("(�I��)�J�[�\�����ړ�(�Q�s����) (&K)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, _T("(�I��)�P��̍��[�Ɉړ�(&L)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, _T("(�I��)�P��̉E�[�Ɉړ�(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, _T("(�I��)�O�̒i���Ɉړ�(&2)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, _T("(�I��)���̒i���Ɉړ�(&8)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, _T("(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, _T("(�I��)�s���Ɉړ�(�܂�Ԃ��P��) (&T)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, _T("(�I��)�P�y�[�W�A�b�v(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, _T("(�I��)�P�y�[�W�_�E��(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, _T("(�I��)�t�@�C���̐擪�Ɉړ�(&1)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, _T("(�I��)�t�@�C���̍Ō�Ɉړ�(&9)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Mar. 11, 2004 genta ��`�I�����j���[��I�����j���[�ɓ���
			//	���̂Ƃ����1�����Ȃ��̂�
			// �u��`�I���v�|�b�v�A�b�v���j���[
			// hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, _T("��`�͈͑I���J�n(&B)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP_BOX			, _T("(��I)�J�[�\����ړ�(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN_BOX		, _T("(��I)�J�[�\�����ړ�(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LEFT_BOX		, _T("(��I)�J�[�\�����ړ�(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RIGHT_BOX		, _T("(��I)�J�[�\���E�ړ�(&)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP2_BOX			, _T("(��I)�J�[�\����ړ�(�Q�s����) (&Q)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN2_BOX		, _T("(��I)�J�[�\�����ړ�(�Q�s����) (&K)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDLEFT_BOX	, _T("(��I)�P��̍��[�Ɉړ�(&L)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_BOX	, _T("(��I)�P��̉E�[�Ɉړ�(&R)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINETOP_BOX	, _T("(��I)�s���Ɉړ�(�܂�Ԃ��P��) (&H)") x);
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINEEND_BOX	, _T("(��I)�s���Ɉړ�(�܂�Ԃ��P��) (&E)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageUp_Box	, _T("(�I��)���y�[�W�A�b�v(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageDown_Box, _T("(�I��)���y�[�W�_�E��(&)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageUp_Box		, _T("(��I)�P�y�[�W�A�b�v(&U)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageDown_Box	, _T("(��I)�P�y�[�W�_�E��(&D)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILETOP_BOX	, _T("(��I)�t�@�C���̐擪�Ɉړ�(&T)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILEEND_BOX	, _T("(��I)�t�@�C���̍Ō�Ɉړ�(&B)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�I��(&S)") );

//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("��`�I��(&E)") );

			// �u���`�v�|�b�v�A�b�v���j���[
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, _T("��(�擪)�̋󔒂��폜(&L)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, _T("�E(����)�̋󔒂��폜(&R)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, _T("�I���s�̏����\�[�g(&A)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, _T("�I���s�̍~���\�[�g(&D)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, _T("�A�������d���s�̍폜(uniq)(&U)") );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("���`(&K)") );

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
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, _T("������(&L)") );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, _T("�啶��(&U)") );			//Sept. 10, 2000 jepro �L���v�V�������p�ꂩ��ύX
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, _T("�S�p�����p") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, _T("���p���S�p�J�^�J�i") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, _T("���p���S�p�Ђ炪��") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, _T("�S�p�����p(&F)") );					//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
	// From Here 2007.01.24 maru ���j���[�̕��ѕύX�ƃA�N�Z�X�L�[�ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, _T("���p�{�S�Ђ灨�S�p�E�J�^�J�i(&Z)") );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, _T("���p�{�S�J�^���S�p�E�Ђ炪��(&N)") );	//Sept. 13, 2000 JEPRO �L���v�V�����ύX & �A�N�Z�X�L�[�t�^ //Oct. 11, 2000 JEPRO �L���v�V�����ύX
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, _T("�S�p�p�������p�p��(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, _T("���p�p�����S�p�p��(&M)") );				//July. 29, 2001 Misaka �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, _T("�S�p�J�^�J�i�����p�J�^�J�i(&J)") );		//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru ���j���[�̕��ѕύX�ƃA�N�Z�X�L�[�ǉ�
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKATA, _T("���p�J�^�J�i���S�p�J�^�J�i(&K)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENHIRA, _T("���p�J�^�J�i���S�p�Ђ炪��(&H)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, _T("TAB����(&S)") );	//Feb. 19, 2001 JEPRO ������ړ�����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, _T("�󔒁�TAB(&T)") );	//---- Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//�u�����R�[�h�ϊ��v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, _T("�������ʁ�SJIS�R�[�h�ϊ�(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, _T("E-Mail(JIS��SJIS)�R�[�h�ϊ�(&M)") );//Sept. 11, 2000 JEPRO �L���v�V�����ɁuE-Mail�v��ǉ����A�N�Z�X�L�[�ύX(V��M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, _T("EUC��SJIS�R�[�h�ϊ�(&W)") );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(E��W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, _T("Unicode��SJIS�R�[�h�ϊ�(&U)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX����I:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, _T("UnicodeBE��SJIS�R�[�h�ϊ�(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, _T("UTF-8��SJIS�R�[�h�ϊ�(&T)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(T:uTF/shifT)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, _T("UTF-7��SJIS�R�[�h�ϊ�(&F)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^(F:utF/shiFt)	//Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, _T("SJIS��JIS�R�[�h�ϊ�(&J)") );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, _T("SJIS��EUC�R�[�h�ϊ�(&E)") );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, _T("SJIS��&Unicode�R�[�h�ϊ�") );		//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, _T("SJIS��UTF-8�R�[�h�ϊ�(&8)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, _T("SJIS��UTF-7�R�[�h�ϊ�(&7)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^ //Oct. 6, 2000 �Ȍ��\���ɂ���
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, _T("Base64�f�R�[�h���ĕۑ�(&B)") );	//Oct. 6, 2000 JEPRO �A�N�Z�X�L�[�ύX(6��B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, _T("uudecode���ĕۑ�(&D)") );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�ύX(U��D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�����R�[�h�ϊ�(&C)") );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 3://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, _T("����(&F)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, _T("��������(&N)") );				//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, _T("�O������(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, _T("�u��(&R)...") );				//Oct. 7, 2000 JEPRO ���̃Z�N�V�������炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, _T("�����}�[�N�̐ؑւ�(&C)") );	// "�����}�[�N�̃N���A(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T("�����J�n�ʒu�֖߂�(&I)") );	// �����J�n�ʒu�֖߂� 02/06/26 ai
		
			// Jan. 10, 2005 genta �C���N�������^���T�[�`
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_NEXT, _T("�O���C���N�������^���T�[�`(&F)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_PREV, _T("����C���N�������^���T�[�`(&B)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_NEXT, _T("���K�\���O���C���N�������^���T�[�`(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_PREV, _T("���K�\������C���N�������^���T�[�`(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_NEXT, _T("MIGEMO�O���C���N�������^���T�[�`(&M)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_PREV, _T("MIGEMO����C���N�������^���T�[�`(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�C���N�������^���T�[�`(&S)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
// From Here 2001.12.03 hor
			// Jan. 10, 2005 genta ��������̂Ńu�b�N�}�[�N���T�u���j���[��
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, _T("�u�b�N�}�[�N�ݒ�E����(&S)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, _T("���̃u�b�N�}�[�N��(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, _T("�O�̃u�b�N�}�[�N��(&Z)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, _T("�u�b�N�}�[�N�̑S����(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, _T("�u�b�N�}�[�N�̈ꗗ(&V)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�u�b�N�}�[�N(&M)") );
// To Here 2001.12.03 hor
			//	Aug. 19. 2003 genta �A�N�Z�X�L�[�\�L�𓝈�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, _T("Grep(&G)...") );					//Oct. 7, 2000 JEPRO �����炱���Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, _T("�w��s�փW�����v(&J)...") );	//Sept. 11, 2000 jepro �L���v�V�����Ɂu �W�����v�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, _T("�A�E�g���C�����(&L)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, _T("�^�O�W�����v(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, _T("�^�O�W�����v�o�b�N(&B)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, _T("�^�O�t�@�C���̍쐬...") );	//@@@ 2003.04.13 MIK // 2004.05.06 MIK ...�ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, _T("�_�C���N�g�^�O�W�����v") );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP_KEYWORD	, _T("�L�[���[�h���w�肵�ă^�O�W�����v") ); //@@ 2005.03.31 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC				, _T("������C/C++�w�b�_(�\�[�X)���J��(&C)") );	//Feb. 7, 2001 JEPRO �ǉ�
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, _T("�t�@�C�����e��r(&@)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, _T("DIFF�����\��(&D)...") );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, _T("���̍�����") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, _T("�O�̍�����") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, _T("�����\���̑S����") );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	�Ί��ʂ̌��������j���[�ɒǉ�
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, _T("�Ί��ʂ̌���(&[)") );
//	To Here Sept. 1, 2000

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
		case 4://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
			m_CMenuDrawer.ResetContents();
			/* �u�I�v�V�����v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( !m_pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, _T("�L�[�}�N���̋L�^�J�n(&R)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, _T("�L�[�}�N���̕ۑ�(&M)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, _T("�L�[�}�N���̓ǂݍ���(&A)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, _T("�L�[�}�N���̎��s(&D)") );
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, _T("�L�[�}�N���̋L�^�I��(&R)") );
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, _T("�L�[�}�N���̋L�^�I��&&�ۑ�(&M)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, _T("�L�[�}�N���̋L�^�I��&&�ǂݍ���(&A)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, _T("�L�[�}�N���̋L�^�I��&&���s(&D)") );
			}
			
			//	From Here Sep. 14, 2001 genta
			//�u�o�^�ς݃}�N���v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			
			for( i = 0; i < MAX_CUSTMACRO; ++i ){
				MacroRec *mp = &m_pShareData->m_Common.m_sMacro.m_MacroTable[i];
				if( mp->IsEnabled() ){
					if(  mp->m_szName[0] ){
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szName );
					}
					else {
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szFile );
					}
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�o�^�ς݃}�N��(&B)") );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* �L�[�{�[�h�}�N���̋L�^�� */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, _T("�O���R�}���h���s(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECMD_DIALOG, _T("�O���R�}���h���s(&X)...") );	//Mar. 10, 2001 JEPRO �@�\���Ȃ��̂Ń��j���[����B����	//Mar.21, 2001 JEPRO �W���o�͂��Ȃ��ŕ��� // 2004.05.06 MIK ...�ǉ�
			//To Here Sept. 20, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, _T("SQL*Plus���A�N�e�B�u�\��(&P)") );	//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^	�����́u�A�N�e�B�u���v���u�A�N�e�B�u�\���v�ɓ���
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, _T("SQL*Plus�Ŏ��s(&S)") );			//Sept. 11, 2000 JEPRO �A�N�Z�X�L�[�t�^

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, _T("���͕⊮(&/)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//�u�J�X�^�����j���[�v�|�b�v�A�b�v
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	�E�N���b�N���j���[
			if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0] );
			}
			//	�J�X�^�����j���[
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i] );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�J�X�^�����j���[(&U)") );

//		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

			break;

		case 5://Feb. 28, 2004 genta �u�ݒ�v���j���[
			//	�ݒ荀�ڂ��u�c�[���v����Ɨ�������
			m_CMenuDrawer.ResetContents();
			/* �u�E�B���h�E�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

//	From Here Sept. 17, 2000 JEPRO
//	��͂�Win�W���ɍ��킹�ă`�F�b�N�}�[�N�����ŕ\���^��\���𔻒f����悤�ɂ������������̂ŕύX
			if ( !m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				pszLabel = _T("�c�[���o�[��\��(&T)");				//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				pszLabel = _T("�t�@���N�V�����L�[��\��(&K)");		//����̂ݕ\��
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				pszLabel = _T("�^�u�o�[��\��");		//����̂ݕ\��	//@@@ 2003.06.10 MIK	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//����̂�	//@@@ 2003.06.10 MIK
				pszLabel = _T("�X�e�[�^�X�o�[��\��(&S)");			//����̂ݕ\��
//	To Here Sept.17, 2000 JEPRO
//	From Here Oct. 28, 2000 JEPRO
//	3�{�^���̃A�C�R�����ł������Ƃɔ����\���^��\���̃��b�Z�[�W��ς���悤�ɍĂѕύX
			}
			else{
				if( m_cToolbar.GetToolbarHwnd() == NULL ){
					pszLabel = _T("�c�[���o�[��\��(&T)");			//����̂ݕ\��
				}else{
					pszLabel = _T("�\�����̃c�[���o�[���B��(&T)");			//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//����̂�
				if( NULL == m_CFuncKeyWnd.GetHwnd() ){
					pszLabel = _T("�t�@���N�V�����L�[��\��(&K)");	//����̂ݕ\��
				}else{
					pszLabel = _T("�\�����̃t�@���N�V�����L�[���B��(&K)");	//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//����̂�
				//@@@ 2003.06.10 MIK
				if( NULL == m_cTabWnd.GetHwnd() ){
					pszLabel = _T("�^�u�o�[��\��(&M)");	//����̂ݕ\��	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				}else{
					pszLabel = _T("�\�����̃^�u�o�[���B��(&M)");	// 2007.02.13 ryoji �u�^�u�v���u�^�u�o�[�v
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//����̂�
				if( m_cStatusBar.GetStatusHwnd() == NULL ){
					pszLabel = _T("�X�e�[�^�X�o�[��\��(&S)");		//����̂ݕ\��
				}else{
					pszLabel = _T("�\�����̃X�e�[�^�X�o�[���B��(&S)");		//Sept. 9, 2000 jepro �L���v�V�����Ɂu�\�����́v��ǉ�
				}
			}
//	To Here Oct. 28, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWSTATUSBAR, pszLabel );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, _T("�^�C�v�ʐݒ�ꗗ(&L)...") );	//Sept. 13, 2000 JEPRO �ݒ����Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, _T("�^�C�v�ʐݒ�(&Y)...") );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(S��Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, _T("���ʐݒ�(&C)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, _T("�t�H���g�ݒ�(&F)...") );		//Sept. 17, 2000 jepro �L���v�V�����Ɂu�ݒ�v��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, _T("�����̊Ǘ�(&O)...") );	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX�i�ꎞ�ݒ�j��ǉ�
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPNOWRAP, _T("�܂�Ԃ��Ȃ�(&X)") );		// �܂�Ԃ��Ȃ��i�ꎞ�ݒ�j
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPSETTING, _T("�w�茅�Ő܂�Ԃ�(&S)") );	// �w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPWINDOW, _T("�E�[�Ő܂�Ԃ�(&W)") );		// �E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j
			// �܂�Ԃ����@�Ɉꎞ�ݒ��K�p��
			if( GetDocument().m_bTextWrapMethodCurTemp )
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�܂�Ԃ����@�i�ꎞ�ݒ�K�p���j(&X)") );
			else
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("�܂�Ԃ����@(&X)") );

//@@@ 2002.01.14 YAZAKI �܂�Ԃ��Ȃ��R�}���h�ǉ�
// 20051022 aroka �^�C�v�ʐݒ�l�ɖ߂��R�}���h�ǉ�
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			//	Jan.  8, 2006 genta ���ʊ֐���
			{
				CLayoutInt ketas;
				CEditView::TOGGLE_WRAP_ACTION mode = this->GetActiveView().GetWrapMode( &ketas );
				if( mode == CEditView::TGWRAP_NONE ){
					pszLabel = _T("�܂�Ԃ�����(&W)");
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , pszLabel );
				}
				else {
					TCHAR szBuf[60];
					pszLabel = szBuf;
					if( mode == CEditView::TGWRAP_FULL ){
						_stprintf(
							szBuf,
							_T("�܂�Ԃ�����: %d ���i�ő�j(&W)"),
							MAXLINEKETAS
						);
					}
					else if( mode == CEditView::TGWRAP_WINDOW ){
						_stprintf(
							szBuf,
							_T("�܂�Ԃ�����: %d ���i�E�[�j(&W)"),
							this->GetActiveView().ViewColNumToWrapColNum(
								this->GetActiveView().GetTextArea().m_nViewColNum
							)
						);
					}
					else {	// TGWRAP_PROP
						_stprintf(
							szBuf,
							_T("�܂�Ԃ�����: %d ���i�w��j(&W)"),
							GetDocument().m_cDocType.GetDocumentAttribute().m_nMaxLineKetas
						);
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel );
				}
			}
			//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , _T("���݂̃E�B���h�E���Ő܂�Ԃ�(&W)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			// 2003.06.08 Moca �ǉ�
			// �u���[�h�ύX�v�|�b�v�A�b�v���j���[
			// Feb. 28, 2004 genta �ҏW���j���[����ړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, _T("�}���^�㏑�����[�h(&I)") );	//Nov. 9, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_VIEWMODE, _T("�r���[���[�h(&R)") );
			if ( !m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				pszLabel = _T("�L�[���[�h�w���v�����\��(&H)");
			}
			else if( IsFuncChecked( &GetDocument(), m_pShareData, F_TOGGLE_KEY_SEARCH ) ){
				pszLabel = _T("�L�[���[�h�w���v�����\�����Ȃ�(&H)");
			}
			else {
				pszLabel = _T("�L�[���[�h�w���v�����\������(&H)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOGGLE_KEY_SEARCH, pszLabel );
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, _T("���͉��s�R�[�h�w��(&CRLF)") ); // ���͉��s�R�[�h�w��(CRLF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF, _T("���͉��s�R�[�h�w��(&LF)") ); // ���͉��s�R�[�h�w��(LF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR, _T("���͉��s�R�[�h�w��(C&R)") ); // ���͉��s�R�[�h�w��(CR)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("���͉��s�R�[�h�w��(&E)") );


			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
//		case 5://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
		case 6://Feb. 28, 2004 genta �u�ݒ�v�̐V�݂̂��ߔԍ������炵��
			m_CMenuDrawer.ResetContents();
			/* �u�E�B���h�E�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( 1 == m_cSplitterWnd.GetAllSplitRows() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = _T("�㉺�ɕ���(&-)");	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(T��-)
			}else{
				pszLabel = _T("�㉺�����̉���(&-)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_V , pszLabel );

			if( 1 == m_cSplitterWnd.GetAllSplitCols() ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = _T("���E�ɕ���(&I)");	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(Y��I)
			}else{
				pszLabel = _T("���E�����̉���(&I)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_H , pszLabel );
//	From Here Sept. 17, 2000 JEPRO	�c�������̏ꍇ����Ԃɂ���ă��j���[���b�Z�[�W���ς��悤�ɕύX
			if( (1 < m_cSplitterWnd.GetAllSplitRows()) && (1 < m_cSplitterWnd.GetAllSplitCols()) ){ // 2002/2/8 aroka ���\�b�h��ʂ��ăA�N�Z�X
				pszLabel = _T("�c�������̉���(&S)");	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(Q��S)
			}else{
				pszLabel = _T("�c���ɕ���(&S)");	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[��ύX(S��Q)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�����ɖ߂���(Q��S)
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_VH , pszLabel );
//	To Here Sept. 17, 2000
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T("����(&C)") );			//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(O��C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, _T("���ׂĕ���(&Q)") );		//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, _T("���̃E�B���h�E(&N)") );	//Sept. 11, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, _T("�O�̃E�B���h�E(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINLIST			, _T("�E�B���h�E�ꗗ(&W)...") );		// 2006.03.23 fon
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, _T("�d�˂ĕ\��(&E)") );		//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(C��E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, _T("�㉺�ɕ��ׂĕ\��(&H)") );	//Sept. 13, 2000 JEPRO �����ɍ��킹�ă��j���[�̍��E�Ə㉺�����ւ��� //Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(V��H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, _T("���E�ɕ��ׂĕ\��(&T)") );	//Oct. 7, 2000 JEPRO �A�N�Z�X�L�[�ύX(H��T)
			if( (DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST ){
				pszLabel = _T("��Ɏ�O������(&F)");
			}else{
				pszLabel = _T("��Ɏ�O�ɕ\��(&F)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOPMOST, pszLabel ); //2004.09.21 Moca	// 2007.06.20 ryoji �A�N�Z�X�L�[�ǉ�

			hMenuPopUp = ::CreatePopupMenu();	// 2007.06.20 ryoji
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				pszLabel = _T("�O���[�v��������(&B)");
			}else{
				pszLabel = _T("�O���[�v��(&B)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BIND_WINDOW, pszLabel );		//2004.07.14 Kazika �V�K�ǉ�	// 2007.02.13 ryoji �u�������ĕ\���v���u�ЂƂɂ܂Ƃ߂ĕ\���v	// 2007.06.20 ryoji �u�O���[�v���v
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE		, _T("�O���[�v�����(&G)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_NEXTGROUP		, _T("���̃O���[�v(&N)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PREVGROUP		, _T("�O�̃O���[�v(&P)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVERIGHT	, _T("�^�u���E�Ɉړ�(&R)") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVELEFT	, _T("�^�u�����Ɉړ�(&L)") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_SEPARATE	, _T("�V�K�O���[�v(&E)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTNEXT	, _T("���̃O���[�v�Ɉړ�(&X)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTPREV	, _T("�O�̃O���[�v�Ɉړ�(&V)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�^�u�̑���(&B)") );		// 2007.06.20 ryoji

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, _T("�c�����ɍő剻(&X)") );	//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�t�^
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, _T("�������ɍő剻(&Y)") );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, _T("���ׂčŏ���(&M)") );		//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */				//Oct. 22, 2000 JEPRO ���́u�ĕ`��v�����ɔ����Z�p���[�^��ǉ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, _T("�ĕ`��(&R)") );			//Oct. 22, 2000 JEPRO �R�����g�A�E�g����Ă����̂𕜊�������
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* �Z�p���[�^ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, _T("�A�E�g�v�b�g(&U)") );		//Sept. 13, 2000 JEPRO �A�N�Z�X�L�[�ύX(O��U)
// 2006.03.23 fon CHG-start>>
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );		/* �Z�p���[�^ */
			EditNode*	pEditNodeArr;
			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			delete [] pEditNodeArr;
//<< 2006.03.23 fon CHG-end
			break;

//		case 8://case 6: (Oct. 22, 2000 JEPRO [�ړ�]��[�I��]��V�݂������ߔԍ���2�V�t�g����)
//		case 6://Feb. 19, 2001 JEPRO [�ړ�]��[�I��]��[�ҏW]�z���Ɉړ��������ߔԍ������ɖ߂���
		case 7://Feb. 28, 2004 genta �u�ݒ�v�̐V�݂̂��ߔԍ������炵��
			m_CMenuDrawer.ResetContents();
			/* �u�w���v�v���j���[ */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T("�ڎ�(&O)") );				//Sept. 7, 2000 jepro �L���v�V�������u�w���v�ڎ��v����ύX	Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(C��O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 _T("�L�[���[�h����(&S)...") );	//Sept. 7, 2000 jepro �L���v�V�������u�w���v�g�s�b�N�̌����v����ύX //Nov. 25, 2000 jepro�u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX // 2004.05.06 MIK ...�ǉ�
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, _T("�R�}���h�ꗗ(&M)") );		//Oct. 13, 2000 JEPRO �A�N�Z�X�L�[���u�g���C�E�{�^���v�̂��߂ɕύX(L��M)
//Sept. 16, 2000 JEPRO �V���[�g�J�b�g�L�[�����܂������Ȃ��̂Ŏ��s�͎E���Č��ɖ߂��Ă���		//Dec. 25, 2000 ����
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, _T("�L�[���蓖�Ĉꗗ���R�s�[(&Q)") );			//Sept. 15, 2000 JEPRO �L���v�V�����́u...���X�g�v�A�A�N�Z�X�L�[�ύX(K��Q) IDM_TEST��F�ɕύX			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, _T("�O���w���v�P(&E)") );		//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��g�b�v���牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, _T("�O��HTML�w���v(&H)") );	//Sept. 7, 2000 JEPRO ���̃��j���[�̏��Ԃ��Q�Ԗڂ��牺�Ɉړ�
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, _T("�o�[�W�������(&A)") );	//Dec. 25, 2000 JEPRO F_�ɕύX
			break;
		}
	}

end_of_func_IsEnable:;
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if (m_pPrintPreview)	return;	//	����v���r���[���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

	/* �@�\�����p�\���ǂ����A�`�F�b�N��Ԃ��ǂ������ꊇ�`�F�b�N */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		/* �@�\�����p�\�����ׂ� */
		//	Jan.  8, 2006 genta �@�\���L���ȏꍇ�ɂ͖����I�ɍĐݒ肵�Ȃ��悤�ɂ���D
		if( ! IsFuncEnable( &GetDocument(), m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		/* �@�\���`�F�b�N��Ԃ����ׂ� */
		if( IsFuncChecked( &GetDocument(), m_pShareData, id ) ){
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




STDMETHODIMP CEditWnd::DragEnter(  LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pDataObject == NULL || pdwEffect == NULL ){
		return E_INVALIDARG;
	}

	// �E�{�^���t�@�C���h���b�v�̏ꍇ������������
	if( !((MK_RBUTTON & dwKeyState) && IsDataAvailable(pDataObject, CF_HDROP)) ){
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// ����v���r���[�ł͎󂯕t���Ȃ�
	if( m_pPrintPreview ){
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP CEditWnd::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pdwEffect == NULL )
		return E_INVALIDARG;

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP CEditWnd::DragLeave( void )
{
	return S_OK;
}

STDMETHODIMP CEditWnd::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;

	// �t�@�C���h���b�v���A�N�e�B�u�r���[�ŏ�������
	*pdwEffect &= DROPEFFECT_LINK;
	return GetActiveView().PostMyDropFiles( pDataObject );
}

/* �t�@�C�����h���b�v���ꂽ */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
	POINT		pt;
	WORD		cFiles, i;
	EditInfo*	pfi;
	HWND		hWndOwner;

	::DragQueryPoint( hDrop, &pt );
	cFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0);
	/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
		cFiles = 1;
	}
	/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	if( cFiles > m_pShareData->m_Common.m_sFile.m_nDropFileNumMax ){
		cFiles = m_pShareData->m_Common.m_sFile.m_nDropFileNumMax;
	}

	for( i = 0; i < cFiles; i++ ) {
		//�t�@�C���p�X�擾�A�����B
		TCHAR		szFile[_MAX_PATH + 1];
		::DragQueryFile( hDrop, i, szFile, _countof(szFile) );
		CSakuraEnvironment::ResolvePath(szFile);

		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
			::SendMessageAny( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( hWndOwner );
			/* MRU���X�g�ւ̓o�^ */
			CMRU cMRU;
			cMRU.Add( pfi );
		}
		else{
			/* �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ */
			//	2005.06.24 Moca
			if( GetDocument().IsAcceptLoad() ){
				/* �t�@�C���ǂݍ��� */
				SLoadInfo sLoadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
				hWndOwner = GetHwnd();
				/* �A�N�e�B�u�ɂ��� */
				// 2007.06.17 maru ���łɊJ���Ă��邩�`�F�b�N�ς݂���
				// �h���b�v���ꂽ�̂̓t�H���_��������Ȃ��̂ōă`�F�b�N
				if(!sLoadInfo.bOpened) ActivateFrameWindow( hWndOwner );
			}
			else{
				/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
				if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
					/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
					if( GetDocument().m_cDocFileOperation.FileClose() ){
						/* �t�@�C���ǂݍ��� */
						SLoadInfo	sLoadInfo(szFile, CODE_AUTODETECT, false);
						GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
						hWndOwner = GetHwnd();
						/* �A�N�e�B�u�ɂ��� */
						ActivateFrameWindow( hWndOwner );
					}
					goto end_of_drop_query;
				}
				else{
					/* �ҏW�E�B���h�E�̏���`�F�b�N */
					if( m_pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
						TCHAR szMsg[512];
						auto_sprintf(
							szMsg,
							_T("�ҏW�E�B���h�E���̏����%d�ł��B\n")
							_T("����ȏ�͓����ɊJ���܂���B"),
							MAX_EDITWINDOWS
						);
						::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
						::DragFinish( hDrop );
						return;
					}
					/* �V���ȕҏW�E�B���h�E���N�� */
					SLoadInfo sLoadInfo;
					sLoadInfo.cFilePath = szFile;
					sLoadInfo.eCharCode = CODE_AUTODETECT;
					sLoadInfo.bViewMode = false;
					CControlTray::OpenNewEditor(
						G_AppInstance(),
						GetHwnd(),
						sLoadInfo
					);
				}
			}
		}
	}
end_of_drop_query:;
	::DragFinish( hDrop );
	return;
}

/*! WM_TIMER ���� 
	@date 2007.04.03 ryoji �V�K
	@date 2008.04.19 ryoji IDT_FIRST_IDLE �ł� MYWM_FIRST_IDLE �|�X�g������ǉ�
*/
LRESULT CEditWnd::OnTimer( WPARAM wParam, LPARAM lParam )
{
	// �^�C�}�[ ID �ŏ�����U�蕪����
	switch( wParam )
	{
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		m_cToolbar.OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		CAppNodeGroupHandle(0).PostMessageToAllEditors( MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL );	// �v���Z�X�̏���A�C�h�����O�ʒm	// 2008.04.19 ryoji
		::KillTimer( m_hWnd, wParam );
		break;
	default:
		return 1L;
	}

	return 0L;
}


/*! �L���v�V�����X�V�p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �V�K
*/
void CEditWnd::OnCaptionTimer( void )
{
	// �ҏW��ʂ̐ؑցi�^�u�܂Ƃߎ��j���I����Ă�����^�C�}�[���I�����ă^�C�g���o�[���X�V����
	// �܂��ؑ֒��Ȃ�^�C�}�[�p��
	if( !m_pShareData->m_sFlags.m_bEditWndChanging ){
		::KillTimer( GetHwnd(), IDT_CAPTION );
		::SetWindowText( GetHwnd(), m_pszLastCaption );
	}
}

/*! �V�X�e�����j���[�\���p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	                       �ȑO�̓R�[���o�b�N�֐��ł���Ă���KillTimer()�������ōs���悤�ɂ���
*/
void CEditWnd::OnSysMenuTimer( void ) //by �S(2)
{
	::KillTimer( GetHwnd(), IDT_SYSMENU );	// 2007.04.03 ryoji

	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//�V�X�e�����j���[�\��
		// 2006.04.21 ryoji �}���`���j�^�Ή��̏C��
		// 2007.05.13 ryoji 0x0313���b�Z�[�W���|�X�g��������ɕύX�iTrackPopupMenu���ƃ��j���[���ڂ̗L���^������Ԃ��s���ɂȂ���΍�j
		RECT R;
		GetWindowRect(GetHwnd(), &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect( pt, &R );
		::PostMessageAny(
			GetHwnd(),
			0x0313, //�E�N���b�N�ŃV�X�e�����j���[��\������ۂɑ��M���郂�m�炵��
			0,
			MAKELPARAM( (pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom )
		);
	}
	m_IconClicked = icNone;
}








//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX

/* ����v���r���[���[�h�̃I��/�I�t */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;
	HWND	hwndToolBar;

	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	hwndToolBar = (NULL != m_cToolbar.GetRebarHwnd())? m_cToolbar.GetRebarHwnd(): m_cToolbar.GetToolbarHwnd();

	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	����v���r���[���[�h���������܂��B	*/
		delete m_pPrintPreview;	//	�폜�B
		m_pPrintPreview = NULL;	//	NULL���ۂ��ŁA�v�����g�v���r���[���[�h�����f���邽�߁B

		/*	�ʏ탂�[�h�ɖ߂�	*/
		::ShowWindow( this->m_cSplitterWnd.GetHwnd(), SW_SHOW );
		::ShowWindow( hwndToolBar, SW_SHOW );	// 2006.06.17 ryoji
		::ShowWindow( m_cStatusBar.GetStatusHwnd(), SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.GetHwnd(), SW_SHOW );
		::ShowWindow( m_cTabWnd.GetHwnd(), SW_SHOW );	//@@@ 2003.06.25 MIK

		::SetFocus( GetHwnd() );

		hMenu = ::LoadMenu( G_AppInstance(), MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( GetHwnd(), hMenu );
		::DrawMenuBar( GetHwnd() );

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		/*	�ʏ탂�[�h���B��	*/
		hMenu = ::GetMenu( GetHwnd() );
		//	Jun. 18, 2001 genta Print Preview�ł̓��j���[���폜
		::SetMenu( GetHwnd(), NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( GetHwnd() );

		::ShowWindow( this->m_cSplitterWnd.GetHwnd(), SW_HIDE );
		::ShowWindow( hwndToolBar, SW_HIDE );	// 2006.06.17 ryoji
		::ShowWindow( m_cStatusBar.GetStatusHwnd(), SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.GetHwnd(), SW_HIDE );
		::ShowWindow( m_cTabWnd.GetHwnd(), SW_HIDE );	//@@@ 2003.06.25 MIK

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview = new CPrintPreview( this );
		/* ���݂̈���ݒ� */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	�v�����^�̏����擾�B

		/* ���݂̃f�t�H���g�v�����^�̏����擾 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( !bRes ){
			TopInfoMessage( GetHwnd(), _T("����v���r���[�����s����O�ɁA�v�����^���C���X�g�[�����Ă��������B\n") );
			return;
		}

		/* ����ݒ�̔��f */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( GetHwnd(), NULL, TRUE );
		::UpdateWindow( GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}




/* WM_SIZE ���� */
LRESULT CEditWnd::OnSize( WPARAM wParam, LPARAM lParam )
{
	HWND		hwndToolBar;
	int			cx;
	int			cy;
	int			nToolBarHeight;
	int			nStatusBarHeight;
	int			nFuncKeyWndHeight;
	int			nTabWndHeight;	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
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
	if( wParam != SIZE_MINIMIZED ){						/* �ŏ����͌p�����Ȃ� */
		//	2004.05.13 Moca m_eSaveWindowSize�̉��ߒǉ��̂���
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize ){		/* �E�B���h�E�T�C�Y�p�������邩 */
			if( wParam == SIZE_MAXIMIZED ){					/* �ő剻�̓T�C�Y���L�^���Ȃ� */
				if( m_pShareData->m_Common.m_sWindow.m_nWinSizeType != (int)wParam ){
					m_pShareData->m_Common.m_sWindow.m_nWinSizeType = wParam;
				}
			}else{
				::GetWindowRect( GetHwnd(), &rcWin );
				/* �E�B���h�E�T�C�Y�Ɋւ���f�[�^���ύX���ꂽ�� */
				if( m_pShareData->m_Common.m_sWindow.m_nWinSizeType != (int)wParam ||
					m_pShareData->m_Common.m_sWindow.m_nWinSizeCX != rcWin.right - rcWin.left ||
					m_pShareData->m_Common.m_sWindow.m_nWinSizeCY != rcWin.bottom - rcWin.top
				){
					m_pShareData->m_Common.m_sWindow.m_nWinSizeType = wParam;
					m_pShareData->m_Common.m_sWindow.m_nWinSizeCX = rcWin.right - rcWin.left;
					m_pShareData->m_Common.m_sWindow.m_nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// ���ɖ߂��Ƃ��̃T�C�Y��ʂ��L��	// 2007.06.20 ryoji
		EditNode *p = CAppNodeManager::Instance()->GetEditNode( GetHwnd() );
		if( p != NULL ){
			p->m_showCmdRestore = ::IsZoomed( p->GetHwnd() )? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	/* �T�C�Y�ύX�̃^�C�v */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	hwndToolBar = (NULL != m_cToolbar.GetRebarHwnd())? m_cToolbar.GetRebarHwnd(): m_cToolbar.GetToolbarHwnd();
	nToolBarHeight = 0;
	if( NULL != hwndToolBar ){
		::SendMessageAny( hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.GetHwnd() ){
		::SendMessageAny( m_CFuncKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.GetHwnd(), &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//�^�u�E�C���h�E
	nTabWndHeight = 0;
	if( NULL != m_cTabWnd.GetHwnd() )
	{
		::SendMessageAny( m_cTabWnd.GetHwnd(), WM_SIZE, wParam, lParam );
		::GetWindowRect( m_cTabWnd.GetHwnd(), &rc );
		nTabWndHeight = rc.bottom - rc.top;
	}
	//@@@ To Here 2003.05.31 MIK
	nStatusBarHeight = 0;
	if( NULL != m_cStatusBar.GetStatusHwnd() ){
		::SendMessageAny( m_cStatusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam );
		::GetClientRect( m_cStatusBar.GetStatusHwnd(), &rc );
		//	May 12, 2000 genta
		//	2�J�����ڂɉ��s�R�[�h�̕\����}��
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0�p�~�ɏ]���A�K���ɒ���
		// 2004-02-28 yasu ��������o�͎��̏����ɍ��킹��
		// ����ς����ꍇ�ɂ�CEditView::ShowCaretPosInfo()�ł̕\�����@���������K�v����D
		// ��pszLabel[3]: �X�e�[�^�X�o�[�����R�[�h�\���̈�͑傫�߂ɂƂ��Ă���
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 �s 9999 ��"), _T("CRLF"), _T("00000000"), _T("Unicode"), _T("REC"), _T("�㏑") };	//Oct. 30, 2000 JEPRO �疜�s���v���	�����R�[�h�g���L���� 2008/6/21	Uchi
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth = rc.right - rc.left;
		int			nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // �T�C�Y�{�b�N�X�̕�
		int			nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // ���E�̕�
		SIZE		sz;
		HDC			hdc;
		int			i;
		// 2004-02-28 yasu
		// ���m�ȕ����v�Z���邽�߂ɁA�\���t�H���g���擾����hdc�ɑI��������B
		hdc = ::GetDC( m_cStatusBar.GetStatusHwnd() );
		HFONT hFont = (HFONT)::SendMessageAny(m_cStatusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
		if (hFont != NULL)
		{
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if( wParam != SIZE_MAXIMIZED ){
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for( i = nStArrNum - 1; i > 0; i-- ){
			::GetTextExtentPoint32( hdc, pszLabel[i], _tcslen( pszLabel[i] ), &sz );
			nStArr[i - 1] = nStArr[i] - ( sz.cx + nBdrWidth );
		}

		//	Nov. 8, 2003 genta
		//	������Ԃł͂��ׂĂ̕������u�g����v�����C���b�Z�[�W�G���A�͘g��`�悵�Ȃ��悤�ɂ��Ă���
		//	���߁C���������̘g���ςȕ��Ɏc���Ă��܂��D������ԂŘg��`�悳���Ȃ����邽�߁C
		//	�ŏ��Ɂu�g�����v��Ԃ�ݒ肵����Ńo�[�̕������s���D
		m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));

		::SendMessageAny( m_cStatusBar.GetStatusHwnd(), SB_SETPARTS, nStArrNum, (LPARAM)nStArr );
		if (hFont != NULL)
		{
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC( m_cStatusBar.GetStatusHwnd(), hdc );

		::UpdateWindow( m_cStatusBar.GetStatusHwnd() );	// 2006.06.17 ryoji �����`��ł���������炷
		::GetWindowRect( m_cStatusBar.GetStatusHwnd(), &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( GetHwnd(), &rcClient );

	//@@@ From 2003.05.31 MIK
	//�^�u�E�C���h�E�ǉ��ɔ����C�t�@���N�V�����L�[�\���ʒu������

	//�^�u�E�C���h�E
	if( m_cTabWnd.GetHwnd() )
	{
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
		{
			::MoveWindow( m_cTabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
		}
		else
		{
			::MoveWindow( m_cTabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
		}
	}

	//	2005.04.23 genta �t�@���N�V�����L�[��\���̎��͈ړ����Ȃ�
	if( m_CFuncKeyWnd.GetHwnd() != NULL ){
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
		{	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			::MoveWindow(
				m_CFuncKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE );
		}
		else if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 1 )
		{	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
			::MoveWindow(
				m_CFuncKeyWnd.GetHwnd(),
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool	bSizeBox = true;
			if( NULL != m_cStatusBar.GetStatusHwnd() ){
				bSizeBox = false;
			}
			if( wParam == SIZE_MAXIMIZED ){
				bSizeBox = false;
			}
			m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
		}
		::UpdateWindow( m_CFuncKeyWnd.GetHwnd() );	// 2006.06.17 ryoji �����`��ł���������炷
	}

	if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
	{
		::MoveWindow(
			m_cSplitterWnd.GetHwnd(),
			0,
			nToolBarHeight + nFuncKeyWndHeight + nTabWndHeight,	//@@@ 2003.05.31 MIK
			cx,
			cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nStatusBarHeight,	//@@@ 2003.05.31 MIK
			TRUE
		);
	}
	else
	{
		::MoveWindow(
			m_cSplitterWnd.GetHwnd(),
			0,
			nToolBarHeight + nTabWndHeight,
			cx,
			cy - nToolBarHeight - nTabWndHeight - nFuncKeyWndHeight - nStatusBarHeight,	//@@@ 2003.05.31 MIK
			TRUE
		);
	}
	//@@@ To 2003.05.31 MIK

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

	m_ptDragPosOrg.x = LOWORD(lParam);	// horizontal position of cursor
	m_ptDragPosOrg.y = HIWORD(lParam);	// vertical position of cursor
	m_bDragMode      = true;
	SetCapture( GetHwnd() );

	return 0;
}

LRESULT CEditWnd::OnLButtonUp( WPARAM wParam, LPARAM lParam )
{
	//by �S 2002/04/18
	if(m_IconClicked != icNone)
	{
		if(m_IconClicked == icDown)
		{
			m_IconClicked = icClicked;
			//by �S(2) �^�C�}�[(ID�͓K���ł�)
			SetTimer(GetHwnd(), IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	m_bDragMode = FALSE;
//	MYTRACE_A("m_bDragMode = FALSE (OnLButtonUp)\n");
	ReleaseCapture();
	::InvalidateRect( GetHwnd(), NULL, TRUE );
	return 0;
}


/*!	WM_MOUSEMOVE����
	@date 2008.05.05 novice ���������[�N�C��
*/
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
			if(SendMessageAny(GetHwnd(), WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
			{
				ReleaseCapture();
				m_IconClicked = icNone;

				if(GetDocument().m_cDocFile.GetFilePathClass().IsValidPath())
				{
					const TCHAR *PathEnd = GetDocument().m_cDocFile.GetFilePath();
					for(CharPointerT I = GetDocument().m_cDocFile.GetFilePath(); *I != 0; ++I)
					{
						if(*I == _T('\\'))
							PathEnd = I.GetPointer();
					}

					//CMyString WPath(GetDocument().GetFilePath(), PathEnd-GetDocument().GetFilePath()); //wchar_t�ւ̕ϊ��p
					wchar_t WPath[MAX_PATH];
					{
						//�����񔲂��o��
						const TCHAR* p=GetDocument().m_cDocFile.GetFilePath();
						int n = PathEnd - p;
						CMyString tmp(p,n);

						//wchar_t�ɕϊ�
						wcscpy(WPath,tmp);
					}

					//CMyString WFile(PathEnd+1); //wchar_t�ւ̕ϊ��p
					wchar_t WFile[MAX_PATH];
					_tcstowcs(WFile,PathEnd+1,_countof(WFile));

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
							{
								FORMATETC F;
								F.cfFormat = CF_UNICODETEXT;
								F.ptd      = NULL;
								F.dwAspect = DVASPECT_CONTENT;
								F.lindex   = -1;
								F.tymed    = TYMED_HGLOBAL;

								STGMEDIUM M;
								const wchar_t* pFilePath = to_wchar(GetDocument().m_cDocFile.GetFilePath());
								int Len = wcslen(pFilePath);
								M.tymed          = TYMED_HGLOBAL;
								M.pUnkForRelease = NULL;
								M.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len+1)*sizeof(wchar_t));
								void* p = GlobalLock(M.hGlobal);
								CopyMemory(p, pFilePath, (Len+1)*sizeof(wchar_t));
								GlobalUnlock(M.hGlobal);

								DataObject->SetData(&F, &M, TRUE);
							}
#endif
							//�ړ��͋֎~
							DWORD R;
							CDropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
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
	if( m_pPrintPreview ){
		return m_pPrintPreview->OnMouseWheel( wParam, lParam );
	}
	return Views_DispatchEvent( GetHwnd(), WM_MOUSEWHEEL, wParam, lParam );
}

/** �}�E�X�z�C�[������

	@date 2007.10.16 ryoji OnMouseWheel()���珈�������o��
*/
BOOL CEditWnd::DoMouseWheel( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	/* ����v���r���[���[�h�� */
	if( !m_pPrintPreview ){
		// 2006.03.26 ryoji by assitance with John �^�u��Ȃ�E�B���h�E�؂�ւ�
		if( m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel && NULL != m_cTabWnd.m_hwndTab )
		{
			POINT pt;
			pt.x = (short)LOWORD( lParam );
			pt.y = (short)HIWORD( lParam );
			int nDelta = (short)HIWORD( wParam );
			HWND hwnd = ::WindowFromPoint( pt );
			if( (hwnd == m_cTabWnd.m_hwndTab || hwnd == m_cTabWnd.GetHwnd()) )
			{
				// ���݊J���Ă���ҏW���̃��X�g�𓾂�
				EditNode* pEditNodeArr;
				int nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
				if(  nRowNum > 0 )
				{
					// �����̃E�B���h�E�𒲂ׂ�
					int i, j;
					int nGroup = 0;
					for( i = 0; i < nRowNum; ++i )
					{
						if( GetHwnd() == pEditNodeArr[i].GetHwnd() )
						{
							nGroup = pEditNodeArr[i].m_nGroup;
							break;
						}
					}
					if( i < nRowNum )
					{
						if( nDelta < 0 )
						{
							// ���̃E�B���h�E
							for( j = i + 1; j < nRowNum; ++j )
							{
								if( nGroup == pEditNodeArr[j].m_nGroup )
									break;
							}
							if( j >= nRowNum )
							{
								for( j = 0; j < i; ++j )
								{
									if( nGroup == pEditNodeArr[j].m_nGroup )
										break;
								}
							}
						}
						else
						{
							// �O�̃E�B���h�E
							for( j = i - 1; j >= 0; --j )
							{
								if( nGroup == pEditNodeArr[j].m_nGroup )
									break;
							}
							if( j < 0 )
							{
								for( j = nRowNum - 1; j > i; --j )
								{
									if( nGroup == pEditNodeArr[j].m_nGroup )
										break;
								}
							}
						}

						/* ���́ior �O�́j�E�B���h�E���A�N�e�B�u�ɂ��� */
						if( i != j )
							ActivateFrameWindow( pEditNodeArr[j].GetHwnd() );
					}

					delete []pEditNodeArr;
				}
				return TRUE;	// ��������
			}
		}
		return FALSE;	// �������Ȃ�����
	}
	return FALSE;	// �������Ȃ�����
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

//	cDlgPrintSetting.Create( G_AppInstance(), GetHwnd() );
	nCurrentPrintSetting = GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		G_AppInstance(),
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		GetHwnd(),
		&nCurrentPrintSetting, /* ���ݑI�����Ă������ݒ� */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* ���ݑI������Ă���y�[�W�ݒ�̔ԍ����ύX���ꂽ�� */
		if( nCurrentPrintSetting !=
			GetDocument().m_cDocType.GetDocumentType()->m_nCurrentPrintSetting
		){
//			/* �ύX�t���O(�^�C�v�ʐݒ�) */
			GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
		}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
		//	����v���r���[���̂݁B
		if ( m_pPrintPreview ){
			/* ���݂̈���ݒ� */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* ����v���r���[ �X�N���[���o�[������ */
			m_pPrintPreview->InitPreviewScrollBar();

			/* ����ݒ�̔��f */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( GetHwnd(), NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	::UpdateWindow( GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by �S

LRESULT CEditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if(wp == HTSYSMENU)
	{
		SetCapture(GetHwnd());
		m_IconClicked = icDown;
		Result = 0;
	}
	else
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONDOWN, wp, lp);

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
	else{
		//	2004.05.23 Moca ���b�Z�[�W�~�X�C��
		//	�t���[���̃_�u���N���b�N����ɃE�B���h�E�T�C�Y
		//	�ύX���[�h�Ȃ��Ă���
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return Result;
}

LRESULT CEditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) //by �S(2)
{
	LRESULT Result;
	if(m_IconClicked != icNone)
	{
		ReleaseCapture();
		m_IconClicked = icDoubleClicked;

		SendMessageCmd(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}
	else {
		//	2004.05.23 Moca ���b�Z�[�W�~�X�C��
		Result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return Result;
}

/*! �h���b�v�_�E�����j���[(�J��) */	//@@@ 2002.06.15 MIK
int	CEditWnd::CreateFileDropDownMenu( HWND hwnd )
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;
	int			nIndex;

	// ���j���[�\���ʒu�����߂�	// 2007.03.25 ryoji
	// �� TBN_DROPDOWN ���� NMTOOLBAR::iItem �� NMTOOLBAR::rcButton �ɂ̓h���b�v�_�E�����j���[(�J��)�{�^����
	//    ��������Ƃ��͂ǂ�������������P�ڂ̃{�^����񂪓���悤�Ȃ̂Ń}�E�X�ʒu����{�^���ʒu�����߂�
	::GetCursorPos( &po );
	::ScreenToClient( hwnd, &po );
	nIndex = ::SendMessageAny( hwnd, TB_HITTEST, (WPARAM)0, (LPARAM)&po );
	if( nIndex < 0 ){
		return 0;
	}
	::SendMessageAny( hwnd, TB_GETITEMRECT, (WPARAM)nIndex, (LPARAM)&rc );
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen( hwnd, &po );
	GetMonitorWorkRect( po, &rc );
	if( po.x < rc.left )
		po.x = rc.left;
	if( po.y < rc.top )
		po.y = rc.top;


	m_CMenuDrawer.ResetContents();

	/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
	CMRU cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_(&D)") );
	}
	else 
	{
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_(&D)") );
	}

	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T("�V�K�쐬(&N)"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T("�J��(&O)..."), FALSE );

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
		NULL
	);

	::DestroyMenu( hMenu );

	return nId;
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
	HICON hOld = (HICON)::SendMessageAny( GetHwnd(), WM_SETICON, flag, (LPARAM)hIcon );
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
void CEditWnd::GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const
{
	*hIconBig   = GetAppIcon( G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false );
	*hIconSmall = GetAppIcon( G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true );
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
bool CEditWnd::GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const
{
	if( NULL != szFile && szFile[0] != _T('\0') ){
		TCHAR szExt[_MAX_EXT];
		TCHAR FileType[1024];

		// (.�Ŏn�܂�)�g���q�̎擾
		_tsplitpath( szFile, NULL, NULL, NULL, szExt );
		
		if( ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, _countof(FileType) - 13)){
			_tcscat( FileType, _T("\\DefaultIcon") );
			if( ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)){
				// �֘A�Â���ꂽ�A�C�R�����擾����
				SHFILEINFO shfi;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON );
				*hIconBig = shfi.hIcon;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON );
				*hIconSmall = shfi.hIcon;
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
	LOGFONT	lf;
	HDC			hdc;
	HFONT		hFontOld;

	/* LOGFONT�̏����� */
	memset_raw( &lf, 0, sizeof( lf ) );
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
	_tcscpy( lf.lfFaceName, _T("�l�r �S�V�b�N") );
	m_hFontCaretPosInfo = ::CreateFontIndirect( &lf );

	hdc = ::GetDC( ::GetDesktopWindow() );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	::GetTextMetrics( hdc, &tm );
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
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
void CEditWnd::PrintMenubarMessage( const TCHAR* msg ) const
{
	if( NULL == ::GetMenu( GetHwnd() ) )	// 2007.03.08 ryoji �ǉ�
		return;

	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;
	const int MAX_LEN = 30;
	TCHAR	szText[MAX_LEN + 1];
	
	int len = _tcslen( msg );
	_tcsncpy( szText, msg, MAX_LEN );
	
	if( len < MAX_LEN ){
		auto_memset( szText + len, _T(' '), MAX_LEN - len );
	}
	szText[MAX_LEN] = _T('\0');
	
	HDC		hdc;
	hdc = ::GetWindowDC( GetHwnd() );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( GetHwnd(), &poFrame );
	::GetWindowRect( GetHwnd(), &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - ( ::GetSystemMetrics( SM_CXSIZEFRAME ) + 2 );
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth + 2;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	//	Sep. 6, 2003 genta Windows XP(Luna)�̏ꍇ�ɂ�COLOR_MENUBAR���g��Ȃ��Ă͂Ȃ�Ȃ�
	COLORREF bkColor =
		::GetSysColor( COsVersionInfo().IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU );
	::SetBkColor( hdc, bkColor );
	/*
	int			m_pnCaretPosInfoDx[64];	// ������`��p�������z��
	for( i = 0; i < _countof( m_pnCaretPosInfoDx ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	*/
	::ExtTextOut( hdc,rc.left,rc.top,ETO_CLIPPED | ETO_OPAQUE,&rc,szText,nStrLen,NULL/*m_pnCaretPosInfoDx*/); //2007.10.17 kobake �߂�ǂ��̂ō��̂Ƃ���͕����Ԋu�z����g��Ȃ��B
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( GetHwnd(), hdc );
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	�X�e�[�^�X�o�[����\���̏ꍇ�̓��j���[�o�[�̉E�[�ɕ\������D
	
	@param msg [in] �\�����郁�b�Z�[�W
	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta CEditView���ړ�
*/
void CEditWnd::SendStatusMessage( const TCHAR* msg )
{
	if( NULL == m_cStatusBar.GetStatusHwnd() ){
		// ���j���[�o�[��
		PrintMenubarMessage( msg );
	}
	else{
		// �X�e�[�^�X�o�[��
		m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! �t�@�C�����ύX�ʒm

	@author MIK
	@date 2003.05.31 �V�K�쐬
	@date 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
*/
void CEditWnd::ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep )
{
	const TCHAR* pszFilePath = _pszFilePath;

	EditNode	*p;
	int		nIndex;

	if( NULL == pszTabCaption ) pszTabCaption = _T("");	//�K�[�h
	if( NULL == pszFilePath )pszFilePath = _FT("");		//�K�[�h 2006.01.28 ryoji

	CRecentEditNode	cRecentEditNode;
	nIndex = cRecentEditNode.FindItemByHwnd( GetHwnd() );
	if( -1 != nIndex )
	{
		p = cRecentEditNode.GetItem( nIndex );
		if( p )
		{
			int	size = _countof( p->m_szTabCaption ) - 1;
			_tcsncpy( p->m_szTabCaption, pszTabCaption, size );
			p->m_szTabCaption[ size ] = _T('\0');

			// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�ǉ�
			size = _countof2( p->m_szFilePath ) - 1;
			_tcsncpy( p->m_szFilePath, pszFilePath, size );
			p->m_szFilePath[ size ] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	cRecentEditNode.Terminate();

	//�t�@�C�����ύX�ʒm���u���[�h�L���X�g����B
	int nGroup = CAppNodeManager::Instance()->GetEditNode( GetHwnd() )->GetGroup();
	CAppNodeGroupHandle(nGroup).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TWNT_FILE,
		(LPARAM)GetHwnd(),
		GetHwnd()
	);

	return;
}

/*! ��Ɏ�O�ɕ\��
	@param top  0:�g�O������ 1:�őO�� 2:�őO�ʉ��� ���̑�:�Ȃɂ����Ȃ�
	@date 2004.09.21 Moca
*/
void CEditWnd::WindowTopMost( int top )
{
	if( 0 == top ){
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE );
		if( dwExstyle & WS_EX_TOPMOST ){
			top = 2; // �őO�ʂł��� -> ����
		}else{
			top = 1;
		}
	}

	HWND hwndInsertAfter;
	switch( top ){
	case 1:
		hwndInsertAfter = HWND_TOPMOST;
		break;
	case 2:
		hwndInsertAfter = HWND_NOTOPMOST;
		break;
	default:
		return;
	}

	::SetWindowPos( GetHwnd(), hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	// �^�u�܂Ƃߎ��� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�œ�������	// 2007.05.18 ryoji
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		HWND hwnd;
		int i;
		for( i = 0, hwndInsertAfter = GetHwnd(); i < m_pShareData->m_sNodes.m_nEditArrNum; i++ ){
			hwnd = m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd();
			if( hwnd != GetHwnd() && IsSakuraMainWindow( hwnd ) ){
				if( !CAppNodeManager::IsSameGroup( GetHwnd(), hwnd ) )
					continue;
				::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// �^�C�}�[�̍X�V���J�n�^��~����B 20060128 aroka
// �c�[���o�[�\���̓^�C�}�[�ɂ��X�V���Ă��邪�A
// �A�v���̃t�H�[�J�X���O�ꂽ�Ƃ��ɃE�B���h�E����ON/OFF��
//	�Ăяo���Ă��炤���Ƃɂ��A�]�v�ȕ��ׂ��~�������B
void CEditWnd::Timer_ONOFF( BOOL bStart )
{
	if( NULL != GetHwnd() ){
		if( bStart ){
			/* �^�C�}�[���N�� */
			if( 0 == ::SetTimer( GetHwnd(), IDT_TOOLBAR, 300, NULL ) ){
				WarningMessage( GetHwnd(), _T("CEditWnd::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B") );
			}
		} else {
			/* �^C�}�[���폜 */
			::KillTimer( GetHwnd(), IDT_TOOLBAR );
		}
	}
	return;
}

/*!	@brief �E�B���h�E�ꗗ���|�b�v�A�b�v�\��

	@param[in] bMousePos true: �}�E�X�ʒu�Ƀ|�b�v�A�b�v�\������

	@date 2006.03.23 fon OnListBtnClick���x�[�X�ɐV�K�쐬
	@date 2006.05.10 ryoji �|�b�v�A�b�v�ʒu�ύX�A���̑����C��
	@data 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
*/
LRESULT CEditWnd::PopupWinList( bool bMousePos )
{
	POINT pt;

	// �|�b�v�A�b�v�ʒu���A�N�e�B�u�r���[�̏�ӂɐݒ�
	RECT rc;
	
	if( bMousePos ){
		::GetCursorPos( &pt );	// �}�E�X�J�[�\���ʒu�ɕύX
	}
	else {
		::GetWindowRect( GetActiveView().GetHwnd(), &rc );
		pt.x = rc.right - 150;
		if( pt.x < rc.left )
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// �E�B���h�E�ꗗ���j���[���|�b�v�A�b�v�\������
	if( NULL != m_cTabWnd.GetHwnd() ){
		m_cTabWnd.TabListMenu( pt );
	}
	else{
		EditNode*	pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();	// 2006.03.23 fon
		int nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
		WinListMenu( hMenu, pEditNodeArr, nRowNum, TRUE );
		// ���j���[��\������
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
		int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
									0, GetHwnd(), NULL);
		delete [] pEditNodeArr;
		::DestroyMenu( hMenu );
		::SendMessageCmd( GetHwnd(), WM_COMMAND, (WPARAM)nId, (LPARAM)NULL );
	}

	return 0L;
}

/*! @brief ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� 
	@date  2006.03.23 fon CEditWnd::InitMenu����ړ��B////�������炠��R�����g�B//>�͒ǉ��R�����g�A�E�g�B
*/
LRESULT CEditWnd::WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull )
{
	int			i;
	TCHAR		szMemu[280];
//>	EditNode*	pEditNodeArr;
	EditInfo*	pfi;

//>	int	nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
//>		/* �Z�p���[�^ */
//>		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		CFileNameManager::Instance()->TransformFileName_MakeCache();
		for( i = 0; i < nRowNum; ++i ){
			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessageAny( pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			if( pfi->m_bIsGrep ){
				/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
				CNativeW	cmemDes;
				int			nDesLen;
				const wchar_t*	pszDes;
				LimitStringLengthW( pfi->m_szGrepKey, wcslen( pfi->m_szGrepKey ), 64, cmemDes );
				pszDes = cmemDes.GetStringPtr();
				nDesLen = wcslen( pszDes );
////	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
////		i >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���J���t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
				auto_sprintf( szMemu, _T("&%tc �yGrep�z\"%ls%ts\""),
					((1 + i) <= 9)?(_T('1') + i):(_T('A') + i - 9),
					pszDes,
					( (int)wcslen( pfi->m_szGrepKey ) > nDesLen ) ? _T("�E�E�E"):_T("")
				);
			}
			else if( pEditNodeArr[i].GetHwnd() == m_pShareData->m_sHandles.m_hwndDebug ){
////		i >= 10 + 26 �̎��̍l�����Ȃ���(�ɋ߂�)���o�̓t�@�C������36���z���邱�Ƃ͂܂��Ȃ��̂Ŏ�����OK�ł��傤
				auto_sprintf( szMemu, _T("&%tc �A�E�g�v�b�g"), ((1 + i) <= 9)?(_T('1') + i):(_T('A') + i - 9) );

			}
			else{
////		From Here Jan. 23, 2001 JEPRO
////		�t�@�C������p�X����'&'���g���Ă���Ƃ��ɗ��𓙂ŃL�`���ƕ\������Ȃ������C��(&��&&�ɒu�����邾��)
////<----- From Here Added
				TCHAR	szFile2[_MAX_PATH * 2];
				if( _T('\0') == pfi->m_szPath[0] ){
					_tcscpy( szFile2, _T("(����)") );
				}else{
					TCHAR buf[_MAX_PATH];
					CFileNameManager::Instance()->GetTransformFileNameFast( pfi->m_szPath, buf, _MAX_PATH );
					
					dupamp( buf, szFile2 );
				}
				auto_sprintf(
					szMemu,
					_T("&%tc %ts %ts"),
					((1 + i) <= 9)?(_T('1') + i):(_T('A') + i - 9),
					szFile2,
					pfi->m_bIsModified ? _T("*"):_T(" ")
				);
////-----> To Here Added
////		To Here Jan. 23, 2001

////	To Here Oct. 4, 2000
				// SJIS�ȊO�̕����R�[�h�̎�ʂ�\������
				// gm_pszCodeNameArr_Bracket ����R�s�[����悤�ɕύX
				if(IsValidCodeTypeExceptSJIS(pfi->m_nCharCode)){
					_tcscat( szMemu, CCodeTypeName(pfi->m_nCharCode).Bracket() );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMemu );
			if( GetHwnd() == pEditNodeArr[i].GetHwnd() ){
				::CheckMenuItem( hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED );
			}
		}
//>		delete [] pEditNodeArr;
	}
	return 0L;
}

//2007.09.08 kobake �ǉ�
//!�c�[���`�b�v�̃e�L�X�g���擾
void CEditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// �@�\������̎擾 -> tmp -> wszBuf
	WCHAR tmp[256];
	GetDocument().m_cFuncLookup.Funccode2Name( nID, tmp, _countof(tmp) );
	_wcstotcs(wszBuf, tmp, nBufCount);

	// �@�\�ɑΉ�����L�[���̎擾(����)
	CNativeT**	ppcAssignedKeyList;
	int nAssignedKeyNum = CKeyBind::GetKeyStrList(
		G_AppInstance(),
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBuf�֌���
	if( 0 < nAssignedKeyNum ){
		for( int j = 0; j < nAssignedKeyNum; ++j ){
			_tcscat_s( wszBuf, nBufCount, _T("\n        ") );
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			_tcscat_s( wszBuf, nBufCount, pszKey );
			delete ppcAssignedKeyList[j];
		}
		delete [] ppcAssignedKeyList;
	}
}



/*! �^�C�}�[�̏���
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka �c�[���o�[�X�V�� OnToolbarTimer�Ɉړ�����
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
*/
void CEditWnd::OnEditTimer( void )
{
	//static	int	nLoopCount = 0; // wmlhq m_nTimerCount�Ɉڍs
	// �^�C�}�[�̌Ăяo���Ԋu�� 500ms�ɕύX�B300*10��500*6�ɂ���B 20060128 aroka
	IncrementTimerCount(6);

	// 2006.01.28 aroka �c�[���o�[�X�V�֘A�� OnToolbarTimer�Ɉړ������B
	
	//	Aug. 29, 2003 wmlhq, ryoji
	if( m_nTimerCount == 0 && GetCapture() == NULL ){ 
		// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����
		GetDocument().m_cAutoReloadAgent.CheckFileTimeStamp();

		// �t�@�C�������\�̃`�F�b�N����
		if(GetDocument().m_cAutoReloadAgent._ToDoChecking()){
			bool bOld = GetDocument().m_cDocLocker.IsDocWritable();
			GetDocument().m_cDocLocker.CheckWritable(false);
			if(bOld != GetDocument().m_cDocLocker.IsDocWritable()){
				this->UpdateCaption();
			}
		}
	}

	GetDocument().m_cAutoSaveAgent.CheckAutoSave();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �r���[�Ǘ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	CEditView�̉�ʃo�b�t�@���폜
	@date 2007.09.09 Moca �V�K�쐬
*/
void CEditWnd::Views_DeleteCompatibleBitmap()
{
	// CEditView�Q�֓]������
	for( int i = 0; i < 4; i++ ){
		if( m_pcEditViewArr[i]->GetHwnd() ){
			m_pcEditViewArr[i]->DeleteCompatibleBitmap();
		}
	}
}

LRESULT CEditWnd::Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		m_pcEditViewArr[0]->DispatchEvent( hwnd, msg, wParam, lParam );
		m_pcEditViewArr[1]->DispatchEvent( hwnd, msg, wParam, lParam );
		m_pcEditViewArr[2]->DispatchEvent( hwnd, msg, wParam, lParam );
		m_pcEditViewArr[3]->DispatchEvent( hwnd, msg, wParam, lParam );
		return 0L;
	default:
		return this->GetActiveView().DispatchEvent( hwnd, msg, wParam, lParam );
	}
}

void CEditWnd::Views_RedrawAll()
{
	//�A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for( int v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v ){
			this->m_pcEditViewArr[v]->RedrawAll();
		}
	}
	//�A�N�e�B�u���ĕ`��
	this->GetActiveView().RedrawAll();
}

void CEditWnd::Views_Redraw()
{
	//�A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for( int v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v )
			m_pcEditViewArr[v]->Redraw();
	}
	//�A�N�e�B�u���ĕ`��
	GetActiveView().Redraw();
}


/* �A�N�e�B�u�ȃy�C����ݒ� */
void  CEditWnd::SetActivePane( int nIndex )
{
	/* �A�N�e�B�u�ȃr���[��؂�ւ��� */
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;

	// �t�H�[�J�X���ړ�����	// 2007.10.16 ryoji
	m_pcEditViewArr[nOldIndex]->CaretUnderLineOFF(TRUE);	//	2002/05/11 YAZAKI
	if( ::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != m_pcEditViewArr[m_nActivePaneIndex]->GetHwnd() )
	{
		// ::SetFocus()�Ńt�H�[�J�X��؂�ւ���
		::SetFocus( m_pcEditViewArr[m_nActivePaneIndex]->GetHwnd() );
	}else{
		// �A�N�e�B�u�łȂ��Ƃ���::SetFocus()����ƃA�N�e�B�u�ɂȂ��Ă��܂�
		// �i�s���Ȃ���ɂȂ�j�̂œ����I�ɐ؂�ւ��邾���ɂ���
		m_pcEditViewArr[nOldIndex]->OnKillFocus();
		m_pcEditViewArr[m_nActivePaneIndex]->OnSetFocus();
	}

	this->GetActiveView().RedrawAll();	/* �t�H�[�J�X�ړ����̍ĕ`�� */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_cDlgFind.GetHwnd() ){		/* �u�����v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgFind.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cDlgReplace.GetHwnd() ){	/* �u�u���v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgReplace.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cHokanMgr.GetHwnd() ){	/* �u���͕⊮�v�_�C�A���O */
		m_cHokanMgr.Hide();
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cHokanMgr.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cDlgFuncList.GetHwnd() ){	/* �u�A�E�g���C���v�_�C�A���O */ // 20060201 aroka
		/* ���[�h���X���F���݈ʒu�\���̑ΏۂƂȂ�r���[�̕ύX */
		m_cDlgFuncList.ChangeView( (LPARAM)&this->GetActiveView() );
	}

	return;
}


/* �A�N�e�B�u�ȃy�C�����擾 */
int CEditWnd::GetActivePane( void ) const
{
	return m_nActivePaneIndex;
}


/** ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��

	@param bDraw [in] �`��X�C�b�`�̐ݒ�l

	@date 2008.06.08 ryoji �V�K�쐬
*/
void CEditWnd::SetDrawSwitchOfAllViews( bool bDraw )
{
	int i;
	CEditView* pcView;

	for( i = 0; i < _countof( m_pcEditViewArr ); i++ ){
		pcView = m_pcEditViewArr[i];
		pcView->SetDrawSwitch( bDraw );
	}
}


/** ���ׂẴy�C����Redraw����

	�X�N���[���o�[�̏�ԍX�V�̓p�����[�^�Ńt���O���� or �ʊ֐��ɂ����ق��������H
	@date 2007.07.22 ryoji �X�N���[���o�[�̏�ԍX�V��ǉ�

	@param pcViewExclude [in] Redraw���珜�O����r���[
	@date 2008.06.08 ryoji pcViewExclude �p�����[�^�ǉ�
*/
void CEditWnd::RedrawAllViews( CEditView* pcViewExclude )
{
	int i;
	CEditView* pcView;

	for( i = 0; i < _countof( m_pcEditViewArr ); i++ ){
		pcView = m_pcEditViewArr[i];
		if( pcView == pcViewExclude )
			continue;
		if( i == m_nActivePaneIndex ){
			pcView->RedrawAll();
		}else{
			pcView->Redraw();
			pcView->AdjustScrollBars();
		}
	}
}


void CEditWnd::Views_DisableSelectArea(bool bRedraw)
{
	for( int i = 0; i < 4; ++i ){
		if( m_pcEditViewArr[i]->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pcEditViewArr[i]->GetSelectionInfo().DisableSelectArea( true );
		}
	}
}


/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
BOOL CEditWnd::DetectWidthOfLineNumberAreaAllPane( bool bRedraw )
{
	if ( this->GetActiveView().GetTextArea().DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePane�Ōv�Z������A�Đݒ�E�ĕ`�悪�K�v�Ɣ������� */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			this->m_pcEditViewArr[m_nActivePaneIndex^1]->GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	�\������Ă��Ȃ��̂ōĕ`�悵�Ȃ�
			this->m_pcEditViewArr[m_nActivePaneIndex^1]->GetTextArea().DetectWidthOfLineNumberArea( FALSE );
		}
		if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
			this->m_pcEditViewArr[m_nActivePaneIndex^2]->GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
			if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
				this->m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
			}
		}
		else {
			this->m_pcEditViewArr[m_nActivePaneIndex^2]->GetTextArea().DetectWidthOfLineNumberArea( FALSE );
			this->m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->GetTextArea().DetectWidthOfLineNumberArea( FALSE );
		}
		return TRUE;
	}
	return FALSE;
}



/** �E�[�Ő܂�Ԃ�
	@param nViewColNum	[in] �E�[�Ő܂�Ԃ��y�C���̔ԍ�
	@retval �܂�Ԃ���ύX�������ǂ���
	@date 2008.06.08 ryoji �V�K�쐬
*/
BOOL CEditWnd::WrapWindowWidth( int nPane )
{
	// �E�[�Ő܂�Ԃ�
	CLayoutInt nWidth = m_pcEditViewArr[nPane]->ViewColNumToWrapColNum( m_pcEditViewArr[nPane]->GetTextArea().m_nViewColNum );
	if( GetDocument().m_cLayoutMgr.GetMaxLineKetas() != nWidth ){
		ChangeLayoutParam( false, GetDocument().m_cLayoutMgr.GetTabSpace(), nWidth );
		return TRUE;
	}
	return FALSE;
}

/** �܂�Ԃ����@�֘A�̍X�V
	@retval ��ʍX�V�������ǂ���
	@date 2008.06.10 ryoji �V�K�쐬
*/
BOOL CEditWnd::UpdateTextWrap( void )
{
	// ���̊֐��̓R�}���h���s���Ƃɏ����̍ŏI�i�K�ŗ��p����
	// �i�A���h�D�o�^���S�r���[�X�V�̃^�C�~���O�j
	if( GetDocument().m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH ){
		BOOL bWrap = WrapWindowWidth( 0 );	// �E�[�Ő܂�Ԃ�
		if( bWrap ){
			// WrapWindowWidth() �Œǉ������X�V���[�W�����ŉ�ʍX�V����
			for( int i = 0; i < _countof(m_pcEditViewArr); i++ ){
				::UpdateWindow( m_pcEditViewArr[i]->GetHwnd() );
			}
		}
		return bWrap;	// ��ʍX�V���܂�Ԃ��ύX
	}
	return FALSE;	// ��ʍX�V���Ȃ�����
}

/*!	���C�A�E�g�p�����[�^�̕ύX

	��̓I�ɂ̓^�u���Ɛ܂�Ԃ��ʒu��ύX����D
	���݂̃h�L�������g�̃��C�A�E�g�݂̂�ύX���C���ʐݒ�͕ύX���Ȃ��D

	@date 2005.08.14 genta �V�K�쐬
	@date 2008.06.18 ryoji ���C�A�E�g�ύX�r���̓J�[�\���ړ��̉�ʃX�N���[���������Ȃ��i��ʂ̂�����}�~�j
*/
void CEditWnd::ChangeLayoutParam( bool bShowProgress, CLayoutInt nTabSize, CLayoutInt nMaxLineKetas )
{
	HWND		hwndProgress = NULL;
	if( bShowProgress && NULL != this ){
		hwndProgress = this->m_cStatusBar.GetProgressHwnd();
		//	Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	//	���W�̕ۑ�
	CLogicPoint* posSave = SavePhysPosOfAllView();

	//	���C�A�E�g�̍X�V
	GetDocument().m_cLayoutMgr.ChangeLayoutParam( nTabSize, nMaxLineKetas );

	//	���W�̕���
	//	���C�A�E�g�ύX�r���̓J�[�\���ړ��̉�ʃX�N���[���������Ȃ�	// 2008.06.18 ryoji
	SetDrawSwitchOfAllViews( false );
	RestorePhysPosOfAllView( posSave );
	SetDrawSwitchOfAllViews( true );

	for( int i = 0; i < _countof(m_pcEditViewArr); i++ ){
		if( m_pcEditViewArr[i]->GetHwnd() ){
			InvalidateRect( m_pcEditViewArr[i]->GetHwnd(), NULL, TRUE );
			m_pcEditViewArr[i]->AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}


/*!
	���C�A�E�g�̕ύX�ɐ旧���āC�S�Ă�View�̍��W�𕨗����W�ɕϊ����ĕۑ�����D

	@return �f�[�^��ۑ������z��ւ̃|�C���^

	@note �擾�����l�̓��C�A�E�g�ύX���CEditDoc::RestorePhysPosOfAllView�֓n���D
	�n���Y���ƃ��������[�N�ƂȂ�D

	@date 2005.08.11 genta  �V�K�쐬
	@date 2007.09.06 kobake �߂�l��CLogicPoint*�ɕύX
*/
CLogicPoint* CEditWnd::SavePhysPosOfAllView()
{
	const int NUM_OF_VIEW = 4;
	const int NUM_OF_POS = 5;
	
	CLogicPoint* pptPosArray = new CLogicPoint[NUM_OF_VIEW * NUM_OF_POS];
	
	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		GetDocument().m_cLayoutMgr.LayoutToLogic(
			this->m_pcEditViewArr[i]->GetCaret().GetCaretLayoutPos(),
			&pptPosArray[i * NUM_OF_POS + 0]
		);
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 1]
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 2]
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 3]
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 4]
			);
		}
	}
	return pptPosArray;
}


/*!	���W�̕���

	CEditDoc::SavePhysPosOfAllView�ŕۑ������f�[�^�����ɍ��W�l���Čv�Z����D

	@date 2005.08.11 genta  �V�K�쐬
	@date 2007.09.06 kobake ������CLogicPoint*�ɕύX
*/
void CEditWnd::RestorePhysPosOfAllView( CLogicPoint* pptPosArray/*int* posary*/ )
{
	const int NUM_OF_VIEW = 4;
	const int NUM_OF_POS = 5;

	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		CLayoutPoint ptPosXY;
		GetDocument().m_cLayoutMgr.LogicToLayout(
			pptPosArray[i * NUM_OF_POS + 0],
			&ptPosXY
		);
		this->m_pcEditViewArr[i]->GetCaret().MoveCursor( ptPosXY, TRUE );
		this->m_pcEditViewArr[i]->GetCaret().m_nCaretPosX_Prev = this->m_pcEditViewArr[i]->GetCaret().GetCaretLayoutPos().GetX2();

		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 1],
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetFromPointer()
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 2],
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelectBgn.GetToPointer()
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 3],
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetFromPointer()
			);
		}
		if( this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 4],
				this->m_pcEditViewArr[i]->GetSelectionInfo().m_sSelect.GetToPointer()
			);
		}
	}
	delete[] pptPosArray;
}

CEditDoc& CEditWnd::GetDocument()
{
	return CEditApp::Instance()->GetDocument();
}
const CEditDoc& CEditWnd::GetDocument() const
{
	return CEditApp::Instance()->GetDocument();
}
