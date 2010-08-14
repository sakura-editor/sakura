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
	Copyright (C) 2009, ryoji, nasukoji, Hidetaka Sakai
	Copyright (C) 2010, ryoji, Moca�AUchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <shlobj.h>

#include "CControlTray.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgAbout.h"
#include "dlg/CDlgPrintSetting.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "CPrintPreview.h"	/// 2002/2/3 aroka
#include "CCommandLine.h"	/// 2003/1/26 aroka
#include "debug/CRunningTimer.h"
#include "charset/CharPointer.h"
#include "CEditApp.h"
#include "util/module.h"
#include "util/os.h"		//WM_MOUSEWHEEL,WM_THEMECHANGED
#include "util/window.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "plugin/CJackManager.h"
#include "CGrepAgent.h"
#include "CAppMode.h"
#include "CMarkMgr.h"
#include "sakura_rc.h"


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


//	�󋵂ɂ�胁�j���[�̕\����ς���R�}���h���X�g(SetMenuFuncSel�Ŏg�p)
//		2010/5/19	Uchi
struct SFuncMenuName {
	EFunctionCode	eFunc;
	WCHAR*			sName[3];		// �I�𕶎���
};

static SFuncMenuName	sFuncMenuName[] = {
	{F_RECKEYMACRO,			{L"�L�[�}�N���̋L�^�J�n",			L"�L�[�}�N���̋L�^�I��"}},
	{F_SAVEKEYMACRO,		{L"�L�[�}�N���̕ۑ�",				L"�L�[�}�N���̋L�^�I��&&�ۑ�"}},
	{F_LOADKEYMACRO,		{L"�L�[�}�N���̓ǂݍ���",			L"�L�[�}�N���̋L�^�I��&&�ǂݍ���"}},
	{F_EXECKEYMACRO,		{L"�L�[�}�N���̎��s",				L"�L�[�}�N���̋L�^�I��&&���s"}},
	{F_SPLIT_V,				{L"�㉺�ɕ���",						L"�㉺�����̉���"}},
	{F_SPLIT_H,				{L"���E�ɕ���",						L"���E�����̉���"}},
	{F_SPLIT_VH,			{L"�c���ɕ���",						L"�c�������̉���"}},
	{F_TAB_CLOSEOTHER,		{L"���̃^�u�ȊO�����",			L"���̃E�B���h�E�ȊO�����"}},
	{F_TOPMOST,				{L"��Ɏ�O�ɕ\��",					L"��Ɏ�O������"}},
	{F_BIND_WINDOW,			{L"�O���[�v��",						L"�O���[�v��������"}},
	{F_SHOWTOOLBAR,			{L"�c�[���o�[��\��",				L"�c�[���o�[��\��",				L"�\�����̃c�[���o�[���B��"}},
	{F_SHOWFUNCKEY,			{L"�t�@���N�V�����L�[��\��",		L"�t�@���N�V�����L�[��\��",		L"�\�����̃t�@���N�V�����L�[���B��"}},
	{F_SHOWTAB,				{L"�^�u�o�[��\��",					L"�^�u�o�[��\��",					L"�\�����̃^�u�o�[���B��"}},
	{F_SHOWSTATUSBAR,		{L"�X�e�[�^�X�o�[��\��",			L"�X�e�[�^�X�o�[��\��",			L"�\�����̃X�e�[�^�X�o�[���B��"}},
	{F_TOGGLE_KEY_SEARCH,	{L"�L�[���[�h�w���v�����\��",		L"�L�[���[�h�w���v�����\������",	L"�L�[���[�h�w���v�����\�����Ȃ�"}},
};



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
, m_pszMenubarMessage( new TCHAR[MENUBAR_MESSAGE_MAX_LEN] )
, m_nSelectCountMode( SELECT_COUNT_TOGGLE )	//�����J�E���g���@�̏����l��SELECT_COUNT_TOGGLE�����ʐݒ�ɏ]��
{
	g_pcEditWnd=this;

	for( int i = 0; i < _countof(m_pcEditViewArr); i++ ){
		m_pcEditViewArr[i] = NULL;
	}
	// ���̂Ƃ���ő�l�͌Œ�
	m_nEditViewMaxCount = _countof(m_pcEditViewArr);
	m_nEditViewCount = 1;
	// [0] - [3] �܂ō쐬�E���������Ă������̂�[0]�������B�ق��͕��������܂ŉ������Ȃ�
	m_pcEditViewArr[0] = new CEditView(this);

	auto_memset( m_pszMenubarMessage, _T(' '), MENUBAR_MESSAGE_MAX_LEN );	// null�I�[�͕s�v

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_pcDropTarget = new CDropTarget( this );	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

	// 2009.01.17 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
	ClearMouseState();

	// 2009.08.15 Hidetaka Sakai, nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	if( m_pShareData->m_Common.m_sKeyBind.m_bCreateAccelTblEachWin ){
		m_hAccel = CKeyBind::CreateAccerelator(
						m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
						m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
				   );

		if( NULL == m_hAccel ){
			ErrorMessage(
				NULL,
				_T("CEditWnd::CEditWnd()\n")
				_T("�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n")
				_T("�V�X�e�����\�[�X���s�����Ă��܂��B")
			);
		}
	}else{
		m_hAccel = NULL;
	}
}

CEditWnd::~CEditWnd()
{
	g_pcEditWnd=NULL;

	for( int i = 0; i < m_nEditViewMaxCount; i++ ){
		delete m_pcEditViewArr[i];
		m_pcEditViewArr[i] = NULL;
	}
	delete[] m_pszMenubarMessage;
	delete[] m_pszLastCaption;

	//	Dec. 4, 2002 genta
	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;	// 2008.06.20 ryoji
	m_pcDropTarget = NULL;

	// 2009.08.15 Hidetaka Sakai, nasukoji	�E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u�����J������
	if( m_hAccel ){
		::DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
	}

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
	wc.lpszMenuName		= NULL;	// MAKEINTRESOURCE( IDR_MENU1 );	2010/5/16 Uchi
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

	// �v���O�C���R�}���h��o�^����
	RegisterPluginCommand();

	// -- -- -- -- �q�E�B���h�E�쐬 -- -- -- -- //

	/* �����t���[���쐬 */
	m_cSplitterWnd.Create( G_AppInstance(), GetHwnd(), this );

	/* �r���[ */
	GetView(0).Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), 0, TRUE  );
	GetView(0).OnSetFocus();

	/* �q�E�B���h�E�̐ݒ� */
	HWND        hWndArr[2];
	hWndArr[0] = GetView(0).GetHwnd();
	hWndArr[1] = NULL;
	m_cSplitterWnd.SetChildWndArr( hWndArr );

	MY_TRACETIME( cRunningTimer, "View created" );

	// -- -- -- -- �_�C�A���O�쐬 -- -- -- -- //

	/* ���͕⊮�E�B���h�E�쐬 */
	m_cHokanMgr.DoModeless(
		G_AppInstance(),
		GetView(0).GetHwnd(),
		(LPARAM)&GetView(0)
	);


	// -- -- -- -- �e��o�[�쐬 -- -- -- -- //

	// ���C�����j���[
	LayoutMainMenu();

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
		bool		bReadResult = GetDocument().m_cDocFileOperation.FileLoadWithoutAutoMacro(&sLoadInfo);	// �������s�}�N���͌�ŕʂ̏ꏊ�Ŏ��s�����
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


/*! ���C�����j���[�̔z�u����
	@date 20105/16 Uchi
*/
void CEditWnd::LayoutMainMenu()
{
	TCHAR		szLabel[300];
	TCHAR		szKey[10];
	CommonSetting_MainMenu*	pcMenu = &m_pShareData->m_Common.m_sMainMenu;
	CMainMenu*	cMainMenu;
	HWND		hWnd = GetHwnd();
	HMENU		hMenu;
	int			i;
	int 		j;
	int 		nCount;

	hMenu = ::CreateMenu();
	for (i = 0; i < MAX_MAINMENU_TOP && pcMenu->m_nMenuTopIdx[i] >= 0; i++) {
		nCount = ( i >= MAX_MAINMENU_TOP || pcMenu->m_nMenuTopIdx[i+1] < 0 ? pcMenu->m_nMainMenuNum : pcMenu->m_nMenuTopIdx[i+1] )
				- pcMenu->m_nMenuTopIdx[i];		// ���j���[���ڐ�
		cMainMenu = &pcMenu->m_cMainMenuTbl[pcMenu->m_nMenuTopIdx[i]];
		switch (cMainMenu->m_nType) {
		case T_NODE:
			::AppendMenu( hMenu, MF_POPUP | MFT_STRING | (nCount<=1 ? MF_GRAYED : 0), (UINT)CreatePopupMenu(), 
				CKeyBind::MakeMenuLabel( to_tchar(cMainMenu->m_sName), to_tchar(cMainMenu->m_sKey) ) );
			break;
		case T_LEAF:
			/* ���j���[���x���̍쐬 */
			szLabel[0] = _T('\0');
			auto_strcpy( szKey, to_tchar(cMainMenu->m_sKey));
			if (CKeyBind::GetMenuLabel(
				G_AppInstance(),
				m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
				m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
				cMainMenu->m_nFunc,
				szLabel,
				to_tchar(cMainMenu->m_sKey),
				FALSE) == NULL) {
				auto_strcpy( szLabel, _T("?") );
			}
			::AppendMenu( hMenu, MFT_STRING, cMainMenu->m_nFunc, szLabel );
			break;
		case T_SEPARATOR:
			::AppendMenu( hMenu, MFT_SEPARATOR, 0, NULL );
			break;
		case T_SPECIAL:
			nCount = 0;
			switch (cMainMenu->m_nFunc) {
			case F_WINDOW_LIST:				// �E�B���h�E���X�g
				EditNode*	pEditNodeArr;
				nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
				delete [] pEditNodeArr;
				break;
			case F_FILE_USED_RECENTLY:		// �ŋߎg�����t�@�C��
				{
					CRecentFile	cRecentFile;
					nCount = cRecentFile.GetViewCount();
				}
				break;
			case F_FOLDER_USED_RECENTLY:	// �ŋߎg�����t�H���_
				{
					CRecentFolder	cRecentFolder;
					nCount = cRecentFolder.GetViewCount();
				}
				break;
			case F_CUSTMENU_LIST:			// �J�X�^�����j���[���X�g
				//	�E�N���b�N���j���[
				if (m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[0] > 0) {
					nCount++;
				}
				//	�J�X�^�����j���[
				for (j = 1; j < MAX_CUSTOM_MENU; ++j) {
					if (m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[j] > 0) {
						nCount++;
					}
				}
				break;
			case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
				for (j = 0; j < MAX_CUSTMACRO; ++j) {
					MacroRec *mp = &m_pShareData->m_Common.m_sMacro.m_MacroTable[j];
					if (mp->IsEnabled()) {
						nCount++;
					}
				}
				break;
			case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
				//�v���O�C���R�}���h��񋟂���v���O�C����񋓂���
				{
					const CJackManager* pcJackManager = CJackManager::Instance();

					CPlug::Array plugs = pcJackManager->GetPlugs( PP_COMMAND );
					for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
						nCount++;
					}
				}
				break;
			}
			::AppendMenu( hMenu, MF_POPUP | MFT_STRING | (nCount<=0 ? MF_GRAYED : 0), (UINT)CreatePopupMenu(), 
				CKeyBind::MakeMenuLabel( to_tchar(cMainMenu->m_sName), to_tchar(cMainMenu->m_sKey) ) );
			break;
		}
	}
	SetMenu( hWnd, hMenu );

	DrawMenuBar( hWnd );
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
	if( NULL != m_cDlgFuncList.GetHwnd() && m_cDlgFuncList.IsDocking() )
		::ShowWindow( m_cDlgFuncList.GetHwnd(), nCmdShow );

	if( bAdjust )
	{
		RECT		rc;
		m_cSplitterWnd.DoSplit( -1, -1 );
		::GetClientRect( GetHwnd(), &rc );
		::SendMessage( GetHwnd(), WM_SIZE, m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
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
		else{
			// 2009.08.15 Hidetaka Sakai, nasukoji
			// Wine�ł͕ʃv���Z�X�ō쐬�����A�N�Z�����[�^�e�[�u�����g�p���邱�Ƃ��ł��Ȃ��B
			// m_bCreateAccelTblEachWin�I�v�V�����I���ɂ��v���Z�X���ɃA�N�Z�����[�^�e�[�u�����쐬�����悤�ɂȂ�
			// ���߁A�V���[�g�J�b�g�L�[��J�[�\���L�[������ɏ��������悤�ɂȂ�B
			HACCEL hAccel = m_pShareData->m_Common.m_sKeyBind.m_bCreateAccelTblEachWin ? m_hAccel : m_pShareData->m_sHandles.m_hAccel;

			if( hAccel && TranslateAccelerator( msg.hwnd, hAccel, &msg ) ){}
			//�ʏ탁�b�Z�[�W
			else{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
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
						cmemWork.AppendString(_T(" , "));
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

			// 2009.01.17 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
			ClearMouseState();
		}

		// �L���v�V�����ݒ�A�^�C�}�[ON/OFF		// 2007.03.08 ryoji WM_ACTIVATE����ړ�
		UpdateCaption();
		m_CFuncKeyWnd.Timer_ONOFF( m_bIsActiveApp ); // 20060126 aroka
		this->Timer_ONOFF( m_bIsActiveApp ); // 20060128 aroka

		return 0L;

	case WM_ENABLE:
		// �E�h���b�v�t�@�C���̎󂯓���ݒ�^����	// 2009.01.09 ryoji
		// Note: DragAcceptFiles��K�p�������h���b�v�ɂ��Ă� Enable/Disable �Ŏ����I�Ɏ󂯓���ݒ�^�������؂�ւ��
		if( (BOOL)wParam ){
			m_pcDropTarget->Register_DropTarget( m_hWnd );
		}else{
			m_pcDropTarget->Revoke_DropTarget();
		}
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
				// Aero Snap�ŏc�����ő剻�ŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������	// 2009.09.02 ryoji
				RECT rcWin;
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement( GetHwnd(), &wp );	// ���[�N�G���A���W
				rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect( GetHwnd(), &rcWork, &rcMon );
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
				m_pShareData->m_Common.m_sWindow.m_nWinPosX = rcWin.left;
				m_pShareData->m_Common.m_sWindow.m_nWinPosY = rcWin.top;
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
						_T("���͉��s�R�[�h�w��(&CRLF)"), _T("") ); // ���͉��s�R�[�h�w��(CRLF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						_T("���͉��s�R�[�h�w��(&LF)"), _T("") ); // ���͉��s�R�[�h�w��(LF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						_T("���͉��s�R�[�h�w��(C&R)"), _T("") ); // ���͉��s�R�[�h�w��(CR)
						
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
		// 2008.11.03 syat   ��`�͈͑I���J�n�̃c�[���`�b�v��80���������Ă����̂�lpszText�ɕύX�B
		case TTN_NEEDTEXT:
			{
				static TCHAR szText[256];
				memset(szText, 0, sizeof(szText));

				//�c�[���`�b�v�e�L�X�g�擾�A�ݒ�
				LPTOOLTIPTEXT lptip = (LPTOOLTIPTEXT)pnmh;
				GetTooltipText(szText, _countof(szText), lptip->hdr.idFrom);
				lptip->lpszText = szText;
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
						sLoadInfo.eCharCode = CODE_NONE;
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

		// ���C�����j���[	2010/5/16 Uchi
		LayoutMainMenu();

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
			for( i = 0; i < GetAllViewCount(); i++ )
			{
				b2 = (GetView(i).m_hwndHScrollBar == NULL);
				if( b1 != b2 )		/* �����X�N���[���o�[���g�� */
				{
					GetView(i).DestroyScrollBar();
					GetView(i).CreateScrollBar();
				}
			}
		}

		// �o�[�ύX�ŉ�ʂ�����Ȃ��悤��	// 2006.12.19 ryoji
		EndLayoutBars();

		// 2009.08.15 nasukoji	�A�N�Z�����[�^�e�[�u�����č쐬����(Wine�p)
		if( m_hAccel ){
			::DestroyAcceleratorTable( m_hAccel );		// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u�����J������
			m_hAccel = NULL;
		}

		if( m_pShareData->m_Common.m_sKeyBind.m_bCreateAccelTblEachWin ){		// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
			m_hAccel = CKeyBind::CreateAccerelator(
							m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
							m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
					   );

			if( NULL == m_hAccel ){
				ErrorMessage(
					NULL,
					_T("CEditWnd::DispatchEvent()\n")
					_T("�A�N�Z�����[�^ �e�[�u�����쐬�ł��܂���B\n")
					_T("�V�X�e�����\�[�X���s�����Ă��܂��B")
				);
			}
		}

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

		{	// �A�E�g���C����͉�ʏ���
			bool bAnalyzed = FALSE;
#if 0
			if( /* �K�v�Ȃ�ύX�����������ɋL�q����i�����p�j */ )
			{
				// �A�E�g���C����͉�ʂ̈ʒu�����݂̐ݒ�ɍ��킹��
				bAnalyzed = m_cDlgFuncList.ChangeLayout( OUTLINE_LAYOUT_BACKGROUND );	// �O������̕ύX�ʒm�Ɠ����̈���
			}
#endif
			if( m_cDlgFuncList.GetHwnd() && !bAnalyzed ){	// �A�E�g���C�����J���Ă���΍ĉ��
				// SHOW_NORMAL: ��͕��@���ω����Ă���΍ĉ�͂����B�����łȂ���Ε`��X�V�i�ύX���ꂽ�J���[�̓K�p�j�̂݁B
				EFunctionCode nFuncCode = (m_cDlgFuncList.m_nListType == OUTLINE_BOOKMARK)? F_BOOKMARK_VIEW: F_OUTLINE;
				GetActiveView().GetCommander().HandleCommand( nFuncCode, TRUE, SHOW_NORMAL, 0, 0, 0 );
			}
			if( MyGetAncestor( ::GetForegroundWindow(), GA_ROOTOWNER2 ) == GetHwnd() )
				::SetFocus( GetActiveView().GetHwnd() );	// �t�H�[�J�X��߂�
		}
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
			// TraceOut��MYWM_ADDSTRINGLEN_W�ɕύX�B�������g�p
			// ���L�f�[�^�\����ver���ύX���ꂽ��폜���Ă�OK�B����܂ł͓����N���p�ɂƂ��Ă���
			EDIT_CHAR* pWork = m_pShareData->m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
			// 2010.05.11 Moca wcslen��wcsnlen�ɕύX�Bm_sWorkBuffer�̑傫���𒴂��Ȃ��悤��
			int addSize = wcsnlen( pWork, m_pShareData->m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() );
			GetActiveView().GetCommander().HandleCommand( F_ADDTAIL_W, TRUE, (LPARAM)pWork, (LPARAM)addSize, 0, 0 );
			GetActiveView().GetCommander().HandleCommand( F_GOFILEEND, TRUE, 0, 0, 0, 0 );
		}
		return 0L;

	// 2010.05.11 Moca MYWM_ADDSTRINGLEN_W��ǉ� NUL�Z�[�t
	case MYWM_ADDSTRINGLEN_W:
		{
			EDIT_CHAR* pWork = m_pShareData->m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
			size_t addSize = t_min((size_t)wParam, m_pShareData->m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() );
			GetActiveView().GetCommander().HandleCommand( F_ADDTAIL_W, TRUE, (LPARAM)pWork, (LPARAM)addSize, 0, 0 );
			GetActiveView().GetCommander().HandleCommand( F_GOFILEEND, TRUE, 0, 0, 0, 0 );
		}
		return 0L;

	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_cTabWnd.TabWindowNotify( wParam, lParam );
		return 0L;

	//�A�E�g���C��	// 2010.06.06 ryoji
	case MYWM_OUTLINE_NOTIFY:
		m_cDlgFuncList.OnOutlineNotify( wParam, lParam );
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

	case WM_NCPAINT:
		DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_cStatusBar.GetStatusHwnd() ){
			PrintMenubarMessage( NULL );
		}
		return 0;

	case WM_NCACTIVATE:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�P�j	// 2007.04.03 ryoji
		// �O�ʂɂ���̂��ҏW�E�B���h�E�Ȃ�A�N�e�B�u��Ԃ�ێ�����
		if( m_pShareData->m_sFlags.m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow()) ){
			wParam = TRUE;	// �A�N�e�B�u
		}
		lRes = DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_cStatusBar.GetStatusHwnd() ){
			PrintMenubarMessage( NULL );
		}
		return lRes;

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
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
		&& !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
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
						int iWait = 0;
						while( ::IsWindowVisible( hwnd ) && iWait++ < 20 )
							::Sleep(1);
					}
					if( !::IsWindowVisible( hwnd ) )
					{
						ActivateFrameWindow( hwnd );
					}
				}
			}
		}
		if( p ) delete []p;
	}

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
				const CMRU cMRU;
				EditInfo checkEditInfo;
				cMRU.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
				GetDocument().m_cDocFileOperation.FileLoad( &SLoadInfo(checkEditInfo.m_szPath, checkEditInfo.m_nCharCode, false) );	//	Oct.  9, 2004 genta ���ʊ֐���
			}
			//�ŋߎg�����t�H���_
			else if( wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999){
				//�t�H���_�擾
				const CMRUFolder cMRUFolder;
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
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX ) {
					::SetFocus( GetActiveView().GetHwnd() );
					//�����{�b�N�X���X�V	// 2010/6/6 Uchi
					m_cToolbar.AcceptSharedSearchKey();
				}

				// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
				//	May 19, 2006 genta ��ʃr�b�g��n��
				//	Jul. 7, 2007 genta ��ʃr�b�g��萔��
				GetDocument().HandleCommand( (EFunctionCode)(wID | 0) );
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
			GetDocument().HandleCommand( (EFunctionCode)(nFuncCode | FA_FROMKEYBOARD) );
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
//	2010/5/16	Uchi	���I�ɍ쐬����l�ɕύX	
void CEditWnd::InitMenu( HMENU hMenu, UINT uPos, BOOL fSystemMenu )
{
	int			cMenuItems;
	int			nPos;
	UINT		fuFlags;
	int			i;
	BOOL		bRet;
	int			nRowNum;
	HMENU		hMenuPopUp;


	if( hMenu == ::GetSubMenu( ::GetMenu( GetHwnd() ), uPos )
		&& !fSystemMenu ){
		// ���擾
		CommonSetting_MainMenu*	pcMenu = &m_pShareData->m_Common.m_sMainMenu;
		CMainMenu*	cMainMenu;
		int			nIdxStr;
		int			nIdxEnd;
		int			nLv;
		int			j;
		std::vector<HMENU>	hSubMenu;
		WCHAR const*		psName;

		nIdxStr = pcMenu->m_nMenuTopIdx[uPos];
		nIdxEnd = (uPos < MAX_MAINMENU_TOP) ? pcMenu->m_nMenuTopIdx[uPos+1] : -1;
		if (nIdxEnd < 0) {
			nIdxEnd = pcMenu->m_nMainMenuNum;
		}

		// ���j���[ ������
		m_CMenuDrawer.ResetContents();
		cMenuItems = ::GetMenuItemCount( hMenu );
		for( i = cMenuItems - 1; i >= 0; i-- ){
			bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
		}

		// ���j���[�쐬
		hSubMenu.push_back( hMenu );
		nLv = 1;
		if (pcMenu->m_cMainMenuTbl[nIdxStr].m_nType == T_SPECIAL) {
			nLv = 0;
			nIdxStr--;
		}
		for (i = nIdxStr + 1; i < nIdxEnd; i++) {
			cMainMenu = &pcMenu->m_cMainMenuTbl[i];
			if (cMainMenu->m_nLevel != nLv) {
				nLv = cMainMenu->m_nLevel;
				if (hSubMenu.size() < (size_t)nLv) {
					// �ی�
					break;
				}
				hMenu = hSubMenu[nLv-1];
			}
			switch (cMainMenu->m_nType) {
			case T_NODE:
				hMenuPopUp = ::CreatePopupMenu();
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , 
					cMainMenu->m_sName, cMainMenu->m_sKey );
				if (hSubMenu.size() > (size_t)nLv) {
					hSubMenu[nLv] = hMenuPopUp;
				}
				else {
					hSubMenu.push_back( hMenuPopUp );
				}
				break;
			case T_LEAF:
				/* ���j���[���x���̍쐬 */
				// �J�X�^�����j���[
				if (cMainMenu->m_nFunc == F_MENU_RBUTTON
				  || cMainMenu->m_nFunc >= F_CUSTMENU_1 && cMainMenu->m_nFunc <= F_CUSTMENU_24) {
					//	�E�N���b�N���j���[
					if (cMainMenu->m_nFunc == F_MENU_RBUTTON) {
						j = 0;
					}
					else {
						j = cMainMenu->m_nFunc - F_CUSTMENU_BASE;
					}

					if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[j] > 0 ){
						 m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING,
					 		cMainMenu->m_nFunc, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[j], cMainMenu->m_sKey );
					}
					else {
						 m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
					 		cMainMenu->m_nFunc, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[j], cMainMenu->m_sKey );
					}
					break;
				}
				// �}�N��
				if (cMainMenu->m_nFunc >= F_USERMACRO_0 && cMainMenu->m_nFunc < F_USERMACRO_0+MAX_CUSTMACRO) {
					MacroRec *mp = &m_pShareData->m_Common.m_sMacro.m_MacroTable[cMainMenu->m_nFunc - F_USERMACRO_0];
					if (mp->IsEnabled()) {
						psName = to_wchar(mp->m_szName[0] ? mp->m_szName : mp->m_szFile);
						m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING,
							cMainMenu->m_nFunc, psName, cMainMenu->m_sKey );
					}
					else {
						psName = L"-- undefined macro --";
						m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
							cMainMenu->m_nFunc, psName, cMainMenu->m_sKey );
					}
					break;
				}
				// �v���O�C���R�}���h
				if (cMainMenu->m_nFunc >= F_PLUGCOMMAND_FIRST && cMainMenu->m_nFunc < F_PLUGCOMMAND_LAST) {
					const CJackManager* pcJackManager = CJackManager::Instance();
					const CPlugin* prevPlugin = NULL;

					CPlug::Array plugs = pcJackManager->GetPlugs( PP_COMMAND );
					j = -1;
					for (CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++) {
						const CPlugin* curPlugin = &(*it)->m_cPlugin;
						if ((*it)->GetFunctionCode() == cMainMenu->m_nFunc) {
							//�R�}���h��o�^
							j = cMainMenu->m_nFunc - F_PLUGCOMMAND_FIRST;
							m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING,
								(*it)->GetFunctionCode(), (*it)->m_sLabel.c_str(), cMainMenu->m_sKey,
								TRUE, (*it)->GetFunctionCode() );
						}
					}
					if (j == -1) {
						// not found
						psName = L"-- undefined plugin command --";
						m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
							cMainMenu->m_nFunc, psName, cMainMenu->m_sKey );
					}
					break;
				}
				switch (cMainMenu->m_nFunc) {
				case F_RECKEYMACRO:
				case F_SAVEKEYMACRO:
				case F_LOADKEYMACRO:
				case F_EXECKEYMACRO:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_sFlags.m_bRecordingKeyMacro);
					break;
				case F_SPLIT_V:	
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						m_cSplitterWnd.GetAllSplitRows() == 1 );
					break;
				case F_SPLIT_H:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						m_cSplitterWnd.GetAllSplitCols() == 1 );
					break;
				case F_SPLIT_VH:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						m_cSplitterWnd.GetAllSplitRows() == 1 || m_cSplitterWnd.GetAllSplitCols() == 1 );
					break;
				case F_TAB_CLOSEOTHER:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd != 0 );
					break;
				case F_TOPMOST:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						((DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST) == 0 );
					break;
				case F_BIND_WINDOW:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						(!m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd 
						|| m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin) );
					break;
				case F_SHOWTOOLBAR:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_Common.m_sWindow.m_bMenuIcon, m_cToolbar.GetToolbarHwnd() == NULL );
					break;
				case F_SHOWFUNCKEY:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_Common.m_sWindow.m_bMenuIcon, m_CFuncKeyWnd.GetHwnd() == NULL );
					break;
				case F_SHOWTAB:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_Common.m_sWindow.m_bMenuIcon, m_cTabWnd.GetHwnd() == NULL );
					break;
				case F_SHOWSTATUSBAR:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_Common.m_sWindow.m_bMenuIcon, m_cStatusBar.GetStatusHwnd() == NULL );
					break;
				case F_TOGGLE_KEY_SEARCH:
					SetMenuFuncSel( hMenu, cMainMenu->m_nFunc, cMainMenu->m_sKey, 
						!m_pShareData->m_Common.m_sWindow.m_bMenuIcon, !IsFuncChecked( &GetDocument(), m_pShareData, F_TOGGLE_KEY_SEARCH ) );
					break;
				case F_WRAPWINDOWWIDTH:
					{
						CLayoutInt ketas;
						WCHAR*	pszLabel;
						CEditView::TOGGLE_WRAP_ACTION mode = this->GetActiveView().GetWrapMode( &ketas );
						if( mode == CEditView::TGWRAP_NONE ){
							m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , L"", cMainMenu->m_sKey );
						}
						else {
							WCHAR szBuf[60];
							pszLabel = szBuf;
							if( mode == CEditView::TGWRAP_FULL ){
								auto_sprintf(
									szBuf,
									L"�܂�Ԃ�����: %d ���i�ő�j",
									MAXLINEKETAS
								);
							}
							else if( mode == CEditView::TGWRAP_WINDOW ){
								auto_sprintf(
									szBuf,
									L"�܂�Ԃ�����: %d ���i�E�[�j",
									this->GetActiveView().ViewColNumToWrapColNum(
										this->GetActiveView().GetTextArea().m_nViewColNum
									)
								);
							}
							else {
								auto_sprintf(
									szBuf,
									L"�܂�Ԃ�����: %d ���i�w��j",
									GetDocument().m_cDocType.GetDocumentAttribute().m_nMaxLineKetas
								);
							}
							m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, cMainMenu->m_sKey );
						}
					}
					break;
				default:
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, cMainMenu->m_nFunc, 
						cMainMenu->m_sName, cMainMenu->m_sKey );
					break;
				}
				break;
			case T_SEPARATOR:
				m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
				break;
			case T_SPECIAL:
				bool	bInList = false;		// ���X�g��1�ȏ゠��
				switch (cMainMenu->m_nFunc) {
				case F_WINDOW_LIST:				// �E�B���h�E���X�g
					EditNode*	pEditNodeArr;
					nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
					WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
					bInList = (nRowNum > 0);
					delete [] pEditNodeArr;
					break;
				case F_FILE_USED_RECENTLY:		// �ŋߎg�����t�@�C��
					/* MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ��� */
					{
						//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
						const CMRU cMRU;
						hMenuPopUp = cMRU.CreateMenu( hMenu, &m_CMenuDrawer );	//	�t�@�C�����j���[
						bInList = (cMRU.Length() > 0);
					}
					break;
				case F_FOLDER_USED_RECENTLY:	// �ŋߎg�����t�H���_
					/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
					{
						//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
						const CMRUFolder cMRUFolder;
						hMenuPopUp = cMRUFolder.CreateMenu( hMenu, &m_CMenuDrawer );
						bInList = (cMRUFolder.Length() > 0);;
					}
					break;
				case F_CUSTMENU_LIST:			// �J�X�^�����j���[���X�g
					//	�E�N���b�N���j���[
					if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[0] > 0 ){
						 m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING,
					 		F_MENU_RBUTTON, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], L"" );
						bInList = true;
					}
					//	�J�X�^�����j���[
					for( j = 1; j < MAX_CUSTOM_MENU; ++j ){
						if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[j] > 0 ){
							 m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING,
						 		F_CUSTMENU_BASE + j, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[j], L""  );
							bInList = true;
						}
					}
					break;
				case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
					for( j = 0; j < MAX_CUSTMACRO; ++j ){
						MacroRec *mp = &m_pShareData->m_Common.m_sMacro.m_MacroTable[j];
						if( mp->IsEnabled() ){
							if(  mp->m_szName[0] ){
								m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szName, _T("") );
							}
							else {
								m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szFile, _T("") );
							}
							bInList = true;
						}
					}
					break;
				case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
					//�v���O�C���R�}���h��񋟂���v���O�C����񋓂���
					{
						const CJackManager* pcJackManager = CJackManager::Instance();
						const CPlugin* prevPlugin = NULL;
						HMENU hMenuPlugin;

						CPlug::Array plugs = pcJackManager->GetPlugs( PP_COMMAND );
						for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
							const CPlugin* curPlugin = &(*it)->m_cPlugin;
							if( curPlugin != prevPlugin ){
								//�v���O�C�����ς������v���O�C���|�b�v�A�b�v���j���[��o�^
								hMenuPlugin = ::CreatePopupMenu();
								m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPlugin, curPlugin->m_sName.c_str(), L"" );
								prevPlugin = curPlugin;
							}

							//�R�}���h��o�^
							m_CMenuDrawer.MyAppendMenu( hMenuPlugin, MF_BYPOSITION | MF_STRING,
								(*it)->GetFunctionCode(), to_tchar( (*it)->m_sLabel.c_str() ), _T(""),
								TRUE, (*it)->GetFunctionCode() );
						}
						bInList = (prevPlugin != NULL);
					}
					break;
				}
				// ���X�g�������ꍇ�̏���
				if (!bInList) {
					//�������Ɉ͂܂�A�����X�g�Ȃ� �Ȃ�� ���̕��������X�L�b�v
					if ((i == nIdxStr + 1
						  || (pcMenu->m_cMainMenuTbl[i-1].m_nType == T_SEPARATOR 
							&& pcMenu->m_cMainMenuTbl[i-1].m_nLevel == cMainMenu->m_nLevel))
						&& i + 1 < nIdxEnd
						&& pcMenu->m_cMainMenuTbl[i+1].m_nType == T_SEPARATOR 
						&& pcMenu->m_cMainMenuTbl[i+1].m_nLevel == cMainMenu->m_nLevel) {
						i++;		// �X�L�b�v
					}
				}
				break;
			}
		}
		if (nLv > 0) {
			// ���x�����߂��Ă��Ȃ�
			hMenu = hSubMenu[0];
		}
		// �q�̖����ݒ�SubMenu��Desable
		CheckFreeSubMenu( GetHwnd(), hMenu, uPos );
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	if (m_pPrintPreview)	return;	//	����v���r���[���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

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



// ���j���[�o�[�̖�����������	2010/6/18 Uchi
void CEditWnd::CheckFreeSubMenu( HWND hWnd, HMENU hMenu, UINT uPos )
{
	int 	cMenuItems;

	cMenuItems = ::GetMenuItemCount( hMenu );
	if (cMenuItems == 0) {
		// ���������̂Ŗ�����
		::EnableMenuItem( ::GetMenu( hWnd ), uPos, MF_BYPOSITION | MF_GRAYED );
	}
	else {
		// ���ʃ��x��������
		CheckFreeSubMenuSub( hMenu, 1 );
	}
}

// ���j���[�o�[�̖�����������	2010/6/18 Uchi
void CEditWnd::CheckFreeSubMenuSub( HMENU hMenu, int nLv )
{
	HMENU	hSubMenu;
	int 	cMenuItems;
	int 	nPos;

	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		hSubMenu = ::GetSubMenu( hMenu, nPos );
		if (hSubMenu != NULL) {
			if ( ::GetMenuItemCount( hSubMenu ) == 0) {
				// ���������̂Ŗ�����
				::EnableMenuItem(hMenu, nPos, MF_BYPOSITION | MF_GRAYED);
			}
			else {
				// ���ʃ��x��������
				CheckFreeSubMenuSub( hSubMenu, nLv + 1 );
			}
		}
	}
}



//	�t���O�ɂ��\��������̑I��������B
//		2010/5/19	Uchi
void CEditWnd::SetMenuFuncSel( HMENU hMenu, EFunctionCode nFunc, WCHAR* sKey, bool flag )
{
	int				i;
	WCHAR*			sName;

	sName = L"";
	for (i = 0; i < _countof(sFuncMenuName) ;i++) {
		if (sFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? sFuncMenuName[i].sName[0] : sFuncMenuName[i].sName[1];
		}
	}
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey );
}

void CEditWnd::SetMenuFuncSel( HMENU hMenu, EFunctionCode nFunc, WCHAR* sKey, bool flag0, bool flag1 )
{
	int				i;
	WCHAR*			sName;

	sName = L"";
	for (i = 0; i < _countof(sFuncMenuName) ;i++) {
		if (sFuncMenuName[i].eFunc == nFunc) {
			sName = flag0 ? sFuncMenuName[i].sName[0] : (flag1 ? sFuncMenuName[i].sName[1] : sFuncMenuName[i].sName[2]);
		}
	}
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey );
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

	/* �A�N�e�B�u�ɂ��� */	// 2009.08.20 ryoji �����J�n�O�ɖ������ŃA�N�e�B�u��
	ActivateFrameWindow( GetHwnd() );

	for( i = 0; i < cFiles; i++ ) {
		//�t�@�C���p�X�擾�A�����B
		TCHAR		szFile[_MAX_PATH + 1];
		::DragQueryFile( hDrop, i, szFile, _countof(szFile) );
		CSakuraEnvironment::ResolvePath(szFile);

		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
			::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
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
			}
			else{
				/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
				if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
					/* �t�@�C���ǂݍ��� */
					SLoadInfo sLoadInfo(szFile, CODE_AUTODETECT, false);
					GetDocument().m_cDocFileOperation.FileCloseOpen(sLoadInfo);
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
		m_cDlgFuncList.m_bEditWndReady = true;	// �G�f�B�^��ʂ̏�������
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
		::ShowWindow( m_cDlgFuncList.GetHwnd(), SW_SHOW );	// 2010.06.25 ryoji

		// ���̑��̃��[�h���X�_�C�A���O���߂�	// 2010.06.25 ryoji
		::ShowWindow( m_cDlgFind.GetHwnd(), SW_SHOW );
		::ShowWindow( m_cDlgReplace.GetHwnd(), SW_SHOW );
		::ShowWindow( m_cDlgGrep.GetHwnd(), SW_SHOW );

		::SetFocus( GetHwnd() );

		// ���j���[�𓮓I�ɍ쐬����悤�ɕύX
		//hMenu = ::LoadMenu( G_AppInstance(), MAKEINTRESOURCE( IDR_MENU1 ) );
		//::SetMenu( GetHwnd(), hMenu );
		//::DrawMenuBar( GetHwnd() );
		LayoutMainMenu();				// 2010/5/16 Uchi

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
		::ShowWindow( m_cDlgFuncList.GetHwnd(), SW_HIDE );	// 2010.06.25 ryoji

		// ���̑��̃��[�h���X�_�C�A���O���B��	// 2010.06.25 ryoji
		::ShowWindow( m_cDlgFind.GetHwnd(), SW_HIDE );
		::ShowWindow( m_cDlgReplace.GetHwnd(), SW_HIDE );
		::ShowWindow( m_cDlgGrep.GetHwnd(), SW_HIDE );

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
				// Aero Snap�̏c�����ő剻��ԂŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������	// 2009.09.02 ryoji
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement( GetHwnd(), &wp );	// ���[�N�G���A���W
				rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect( GetHwnd(), &rcWork, &rcMon );
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
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
		::SendMessage( hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.GetHwnd() ){
		::SendMessage( m_CFuncKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.GetHwnd(), &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//�^�u�E�C���h�E
	nTabWndHeight = 0;
	if( NULL != m_cTabWnd.GetHwnd() )
	{
		::SendMessage( m_cTabWnd.GetHwnd(), WM_SIZE, wParam, lParam );
		::GetWindowRect( m_cTabWnd.GetHwnd(), &rc );
		nTabWndHeight = rc.bottom - rc.top;
	}
	//@@@ To Here 2003.05.31 MIK
	nStatusBarHeight = 0;
	if( NULL != m_cStatusBar.GetStatusHwnd() ){
		::SendMessage( m_cStatusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam );
		::GetClientRect( m_cStatusBar.GetStatusHwnd(), &rc );
		//	May 12, 2000 genta
		//	2�J�����ڂɉ��s�R�[�h�̕\����}��
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0�p�~�ɏ]���A�K���ɒ���
		// 2004-02-28 yasu ��������o�͎��̏����ɍ��킹��
		// ����ς����ꍇ�ɂ�CEditView::ShowCaretPosInfo()�ł̕\�����@���������K�v����D
		// ��pszLabel[3]: �X�e�[�^�X�o�[�����R�[�h�\���̈�͑傫�߂ɂƂ��Ă���
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 �s 9999 ��"), _T("CRLF"), _T("000000000000"), _T("Unicode"), _T("REC"), _T("�㏑") };	//Oct. 30, 2000 JEPRO �疜�s���v���	�����R�[�h�g���L���� 2008/6/21	Uchi
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
		HFONT hFont = (HFONT)::SendMessage(m_cStatusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
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

		StatusBar_SetParts( m_cStatusBar.GetStatusHwnd(), nStArrNum, nStArr );
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

	int nFuncListWidth = 0;
	int nFuncListHeight = 0;
	if( m_cDlgFuncList.GetHwnd() && m_cDlgFuncList.IsDocking() )
	{
		::SendMessageAny( m_cDlgFuncList.GetHwnd(), WM_SIZE, wParam, lParam );
		::GetWindowRect( m_cDlgFuncList.GetHwnd(), &rc );
		nFuncListWidth = rc.right - rc.left;
		nFuncListHeight = rc.bottom - rc.top;
	}

	EDockSide eDockSideFL = m_cDlgFuncList.GetDockSide();
	int nTop = nToolBarHeight + nTabWndHeight;
	if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0)
		nTop += nFuncKeyWndHeight;
	int nHeight = cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nStatusBarHeight;
	if( m_cDlgFuncList.GetHwnd() && m_cDlgFuncList.IsDocking() )
	{
		::MoveWindow(
			m_cDlgFuncList.GetHwnd(),
			(eDockSideFL == DOCKSIDE_RIGHT)? cx - nFuncListWidth: 0,
			(eDockSideFL == DOCKSIDE_BOTTOM)? nTop + nHeight - nFuncListHeight: nTop,
			(eDockSideFL == DOCKSIDE_LEFT || eDockSideFL == DOCKSIDE_RIGHT)? nFuncListWidth: cx,
			(eDockSideFL == DOCKSIDE_TOP || eDockSideFL == DOCKSIDE_BOTTOM)? nFuncListHeight: nHeight,
			TRUE
		);
	}
	::MoveWindow(
		m_cSplitterWnd.GetHwnd(),
		(eDockSideFL == DOCKSIDE_LEFT)? nFuncListWidth: 0,
		(eDockSideFL == DOCKSIDE_TOP)? nTop + nFuncListHeight: nTop,	//@@@ 2003.05.31 MIK
		(eDockSideFL == DOCKSIDE_LEFT || eDockSideFL == DOCKSIDE_RIGHT)? cx - nFuncListWidth: cx,
		(eDockSideFL == DOCKSIDE_TOP || eDockSideFL == DOCKSIDE_BOTTOM)? nHeight - nFuncListHeight: nHeight,	//@@@ 2003.05.31 MIK
		TRUE
	);
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
			if(SendMessage(GetHwnd(), WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
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

		SendMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

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
	nIndex = Toolbar_Hittest( hwnd, &po );
	if( nIndex < 0 ){
		return 0;
	}
	Toolbar_GetItemRect( hwnd, nIndex, &rc );
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
	const CMRU cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* �ŋߎg�����t�H���_�̃��j���[���쐬 */
	const CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	�A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("�ŋߎg�����t�H���_"),  _T("") );
	}
	else 
	{
		//	��A�N�e�B�u
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, __T("�ŋߎg�����t�H���_"),  _T("") );
	}

	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), FALSE );

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
		GetHwnd(),	// 2009.02.03 ryoji �A�N�Z�X�L�[�L�����̂��� hwnd -> GetHwnd() �ɕύX
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
	HICON hOld = (HICON)::SendMessage( GetHwnd(), WM_SETICON, flag, (LPARAM)hIcon );
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
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
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
void CEditWnd::PrintMenubarMessage( const TCHAR* msg )
{
	if( NULL == ::GetMenu( GetHwnd() ) )	// 2007.03.08 ryoji �ǉ�
		return;

	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL �̂Ƃ��͈ȑO�� m_pszMenubarMessage �ōĕ`��
	if( msg ){
		int len = _tcslen( msg );
		_tcsncpy( m_pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN );
		if( len < MENUBAR_MESSAGE_MAX_LEN ){
			auto_memset( m_pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len );	//  null�I�[�͕s�v
		}
	}

	HDC		hdc;
	hdc = ::GetWindowDC( GetHwnd() );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( GetHwnd(), &poFrame );
	::GetWindowRect( GetHwnd(), &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
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
	::ExtTextOut( hdc,rc.left,rc.top,ETO_CLIPPED | ETO_OPAQUE,&rc,m_pszMenubarMessage,nStrLen,NULL/*m_pnCaretPosInfoDx*/); //2007.10.17 kobake �߂�ǂ��̂ō��̂Ƃ���͕����Ԋu�z����g��Ȃ��B
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
	@date 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
	@date 2009.06.02 ryoji m_CMenuDrawer�̏������R��C��
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
		m_CMenuDrawer.ResetContents();	// 2009.06.02 ryoji �ǉ�
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
		::SendMessage( GetHwnd(), WM_COMMAND, (WPARAM)nId, (LPARAM)NULL );
	}

	return 0L;
}

/*! @brief ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� 
	@date  2006.03.23 fon CEditWnd::InitMenu����ړ��B////�������炠��R�����g�B//>�͒ǉ��R�����g�A�E�g�B
	@date 2009.06.02 ryoji �A�C�e�����������Ƃ��̓A�N�Z�X�L�[�� 1-9,A-Z �͈̔͂ōĎg�p����i�]����36����������j
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
			::SendMessage( pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
////	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
			TCHAR c = ((1 + i%35) <= 9)?(_T('1') + i%35):(_T('A') + i%35 - 9);	// 2009.06.02 ryoji �A�N�Z�X�L�[�� 1-9,A-Z �͈̔͂ōĎg�p
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			if( pfi->m_bIsGrep ){
				/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
				CNativeW	cmemDes;
				int			nDesLen;
				const wchar_t*	pszDes;
				LimitStringLengthW( pfi->m_szGrepKey, wcslen( pfi->m_szGrepKey ), 64, cmemDes );
				pszDes = cmemDes.GetStringPtr();
				nDesLen = wcslen( pszDes );
				auto_sprintf( szMemu, _T("&%tc �yGrep�z\"%ls%ts\""),
					c,
					pszDes,
					( (int)wcslen( pfi->m_szGrepKey ) > nDesLen ) ? _T("�E�E�E"):_T("")
				);
			}
			else if( pEditNodeArr[i].GetHwnd() == m_pShareData->m_sHandles.m_hwndDebug ){
				auto_sprintf( szMemu, _T("&%tc �A�E�g�v�b�g"), c );

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
					c,
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
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMemu, _T("") );
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
	size_t nLen;
	GetDocument().m_cFuncLookup.Funccode2Name( nID, tmp, _countof(tmp) );
	nLen = _wcstotcs(wszBuf, tmp, nBufCount);

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
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			int nKeyLen = _tcslen(pszKey);
			if ( nLen + 9 + nKeyLen < nBufCount ){
				_tcscat_s( wszBuf, nBufCount, _T("\n        ") );
				_tcscat_s( wszBuf, nBufCount, pszKey );
				nLen += 9 + nKeyLen;
			}
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
	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( GetView(i).GetHwnd() ){
			GetView(i).DeleteCompatibleBitmap();
		}
	}
}

LRESULT CEditWnd::Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		for( int i = 0; i < GetAllViewCount(); i++){
			GetView(i).DispatchEvent( hwnd, msg, wParam, lParam );
		}
		return 0L;
	default:
		return this->GetActiveView().DispatchEvent( hwnd, msg, wParam, lParam );
	}
}

/*
	�����w���B2�ڈȍ~�̃r���[�����
	@param nViewCount  �����̃r���[���܂߂��r���[�̍��v�v����
*/
bool CEditWnd::CreateEditViewBySplit(int nViewCount )
{
	if( m_nEditViewMaxCount < nViewCount ){
		return false;
	}
	if( GetAllViewCount() < nViewCount ){
		for( int i = GetAllViewCount(); i < nViewCount; i++ ){
			assert( NULL == m_pcEditViewArr[i] );
			m_pcEditViewArr[i] = new CEditView(this);
			m_pcEditViewArr[i]->Create( m_cSplitterWnd.GetHwnd(), &GetDocument(), i, FALSE );
		}
		m_nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for( int i = 0; i < nViewCount; i++ ){
			hWndArr.push_back( GetView(i).GetHwnd() );
		}
		hWndArr.push_back( NULL );

		m_cSplitterWnd.SetChildWndArr( &hWndArr[0] );
	}
	return true;
}

/*
	�r���[�̍ď�����
	@date 2010.04.10 CEditDoc::InitAllView����ړ�
*/
void CEditWnd::InitAllViews()
{
	/* �擪�փJ�[�\�����ړ� */
	for( int i = 0; i < GetAllViewCount(); ++i ){
		//	Apr. 1, 2001 genta
		// �ړ������̏���
		GetView(i).m_cHistory->Flush();

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		GetView(i).GetSelectionInfo().DisableSelectArea( FALSE );

		GetView(i).OnChangeSetting();
		GetView(i).GetCaret().MoveCursor( CLayoutPoint(0, 0), TRUE );
		GetView(i).GetCaret().m_nCaretPosX_Prev = CLayoutInt(0);
	}
}


void CEditWnd::Views_RedrawAll()
{
	//�A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for( int v = 0; v < GetAllViewCount(); ++v ){
		if( m_nActivePaneIndex != v ){
			this->GetView(v).RedrawAll();
		}
	}
	//�A�N�e�B�u���ĕ`��
	this->GetActiveView().RedrawAll();
}

void CEditWnd::Views_Redraw()
{
	//�A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for( int v = 0; v < GetAllViewCount(); ++v ){
		if( m_nActivePaneIndex != v )
			GetView(v).Redraw();
	}
	//�A�N�e�B�u���ĕ`��
	GetActiveView().Redraw();
}


/* �A�N�e�B�u�ȃy�C����ݒ� */
void  CEditWnd::SetActivePane( int nIndex )
{
	assert_warning( nIndex < GetAllViewCount() );
	DEBUG_TRACE( _T("CEditWnd::SetActivePane %d\n"), nIndex );

	/* �A�N�e�B�u�ȃr���[��؂�ւ��� */
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;

	// �t�H�[�J�X���ړ�����	// 2007.10.16 ryoji
	GetView(nOldIndex).CaretUnderLineOFF(TRUE);	//	2002/05/11 YAZAKI
	if( ::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != GetView(m_nActivePaneIndex).GetHwnd() )
	{
		// ::SetFocus()�Ńt�H�[�J�X��؂�ւ���
		::SetFocus( GetView(m_nActivePaneIndex).GetHwnd() );
	}else{
		// 2010.04.08 ryoji
		// �N���Ɠ����ɃG�f�B�b�g�{�b�N�X�Ƀt�H�[�J�X�̂���_�C�A���O��\������Ɠ��Y�G�f�B�b�g�{�b�N�X��
		// �L�����b�g���\������Ȃ����(*1)���C������̂��߁A�����I�Ȑ؂�ւ�������̂̓A�N�e�B�u�y�C����
		// �؂�ւ��Ƃ������ɂ����B�� CEditView::OnKillFocus()�͎��X���b�h�̃L�����b�g��j������̂�
		// (*1) -GREPDLG�I�v�V�����ɂ��GREP�_�C�A���O�\����J�t�@�C���㎩�����s�}�N���ł�InputBox�\��
		if( m_nActivePaneIndex != nOldIndex ){
			// �A�N�e�B�u�łȂ��Ƃ���::SetFocus()����ƃA�N�e�B�u�ɂȂ��Ă��܂�
			// �i�s���Ȃ���ɂȂ�j�̂œ����I�ɐ؂�ւ��邾���ɂ���
			GetView(nOldIndex).OnKillFocus();
			GetView(m_nActivePaneIndex).OnSetFocus();
		}
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

	for( i = 0; i < GetAllViewCount(); i++ ){
		GetView(i).SetDrawSwitch( bDraw );
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

	for( i = 0; i < GetAllViewCount(); i++ ){
		pcView = &GetView(i);
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
	for( int i = 0; i < GetAllViewCount(); ++i ){
		if( GetView(i).GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			GetView(i).GetSelectionInfo().DisableSelectArea( true );
		}
	}
}


/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
BOOL CEditWnd::DetectWidthOfLineNumberAreaAllPane( bool bRedraw )
{
	if( 1 == GetAllViewCount() ){
		return this->GetActiveView().GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
	}
	// �ȉ�2,4��������

	if ( this->GetActiveView().GetTextArea().DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePane�Ōv�Z������A�Đݒ�E�ĕ`�悪�K�v�Ɣ������� */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			this->GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	�\������Ă��Ȃ��̂ōĕ`�悵�Ȃ�
			this->GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea( FALSE );
		}
		if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
			this->GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
			if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
				this->GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
			}
		}
		else {
			this->GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea( FALSE );
			this->GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea( FALSE );
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
	CLayoutInt nWidth = GetView(nPane).ViewColNumToWrapColNum( GetView(nPane).GetTextArea().m_nViewColNum );
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
			for( int i = 0; i < GetAllViewCount(); i++ ){
				::UpdateWindow( GetView(i).GetHwnd() );
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

	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( GetView(i).GetHwnd() ){
			InvalidateRect( GetView(i).GetHwnd(), NULL, TRUE );
			GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}
	if( !GetDocument().m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){
		GetActiveView().GetCaret().ShowCaretPosInfo();	// 2009.07.25 ryoji
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
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;
	
	CLogicPoint* pptPosArray = new CLogicPoint[NUM_OF_VIEW * NUM_OF_POS];
	
	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		GetDocument().m_cLayoutMgr.LayoutToLogic(
			this->GetView(i).GetCaret().GetCaretLayoutPos(),
			&pptPosArray[i * NUM_OF_POS + 0]
		);
		if( this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 1]
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 2]
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelect.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->GetView(i).GetSelectionInfo().m_sSelect.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 3]
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelect.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				this->GetView(i).GetSelectionInfo().m_sSelect.GetTo(),
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
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;

	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		CLayoutPoint ptPosXY;
		GetDocument().m_cLayoutMgr.LogicToLayout(
			pptPosArray[i * NUM_OF_POS + 0],
			&ptPosXY
		);
		this->GetView(i).GetCaret().MoveCursor( ptPosXY, TRUE );
		this->GetView(i).GetCaret().m_nCaretPosX_Prev = this->GetView(i).GetCaret().GetCaretLayoutPos().GetX2();

		if( this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 1],
				this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetFromPointer()
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 2],
				this->GetView(i).GetSelectionInfo().m_sSelectBgn.GetToPointer()
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelect.GetFrom().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 3],
				this->GetView(i).GetSelectionInfo().m_sSelect.GetFromPointer()
			);
		}
		if( this->GetView(i).GetSelectionInfo().m_sSelect.GetTo().y >= 0 ){
			GetDocument().m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 4],
				this->GetView(i).GetSelectionInfo().m_sSelect.GetToPointer()
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

/*!
	@brief �}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j

	@note �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[���Ή��̂��߂ɒǉ��B
		  �y�[�W�X�N���[���E���X�N���[������t���O��OFF����B

	@date 2009.01.17 nasukoji	�V�K�쐬
*/
void CEditWnd::ClearMouseState( void )
{
	SetPageScrollByWheel( FALSE );		// �z�C�[������ɂ��y�[�W�X�N���[���L��
	SetHScrollByWheel( FALSE );			// �z�C�[������ɂ�鉡�X�N���[���L��
}

//�v���O�C���R�}���h���G�f�B�^�ɓo�^����
void CEditWnd::RegisterPluginCommand( int idCommand )
{
	CPlug* plug = CJackManager::Instance()->GetCommandById( idCommand );
	RegisterPluginCommand( plug );
}

//�v���O�C���R�}���h���G�f�B�^�ɓo�^����i�ꊇ�j
void CEditWnd::RegisterPluginCommand()
{
	const CPlug::Array& plugs = CJackManager::Instance()->GetPlugs( PP_COMMAND );
	for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ) {
		RegisterPluginCommand( *it );
	}
}

//�v���O�C���R�}���h���G�f�B�^�ɓo�^����
void CEditWnd::RegisterPluginCommand( CPlug* plug )
{
	int iBitmap = CMenuDrawer::TOOLBAR_BUTTON_F_PLUGCOMMAND - 1;
	if( !plug->m_sIcon.empty() ){
		iBitmap = m_CMenuDrawer.m_pcIcons->Add( to_tchar(plug->m_cPlugin.GetFilePath( to_tchar(plug->m_sIcon.c_str()) ).c_str()) );
	}

	m_CMenuDrawer.AddToolButton( iBitmap, plug->GetFunctionCode() );
}
