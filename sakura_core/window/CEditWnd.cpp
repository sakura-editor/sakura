/*!	@file
	@brief 編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, ao
	Copyright (C) 2001, MIK, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta, hor, aroka, minfu, 鬼, MIK, ai
	Copyright (C) 2003, genta, MIK, Moca, wmlhq, ryoji, KEITA
	Copyright (C) 2004, genta, Moca, yasu, MIK, novice, Kazika
	Copyright (C) 2005, genta, MIK, Moca, aroka, ryoji
	Copyright (C) 2006, genta, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji

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
#include "mymessage.h"
#include "CShareData.h"
#include "CPrint.h"
#include "charset/charcode.h"
#include "global.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgPrintPage.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "macro/CSMacroMgr.h" // Jun. 16, 2002 genta
#include "COsVersioninfo.h"	// Sep. 6, 2003 genta
#include "debug/CRunningTimer.h"
#include "KeyCode.h"
#include "charset/CharPointer.h"
#include "CEditApp.h"
#include "util/module.h"
#include "util/os.h"
#include "util/window.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/string_ex2.h"
#include <shlobj.h>
#include "CSakuraEnvironment.h"
#include "util/os.h" //WM_MOUSEWHEEL,WM_THEMECHANGED



//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたので
//	定義を削除

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif



#define		YOHAKU_X		4		/* ウィンドウ内の枠と紙の隙間最小値 */
#define		YOHAKU_Y		4		/* ウィンドウ内の枠と紙の隙間最小値 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたので
//	定義を削除





//	/* メッセージループ */
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

#pragma warning(disable:4355) //「thisポインタが初期化リストで使用されました」の警告を無効化

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditWnd::CEditWnd()
: m_hWnd( NULL )
, m_bDragMode( FALSE )
, m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ) // 20020331 aroka 再変換対応 for 95/NT
, m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
, m_pPrintPreview( NULL ) //@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
, m_pszWndClass( GSTR_EDITWINDOWNAME )
, m_pszLastCaption( NULL )
, m_nCurrentFocus( 0 )
, m_bIsActiveApp( false )
, m_IconClicked(icNone) //by 鬼(2)
, m_cToolbar(this)
, m_cStatusBar(this)
, m_nActivePaneIndex( 0 )
{
	g_pcEditWnd=this;
	for(int i=0;i<4;i++)
		m_pcEditViewArr[i]=new CEditView(this);

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
}

CEditWnd::~CEditWnd()
{
	g_pcEditWnd=NULL;

	delete[] m_pszLastCaption;

	for(int i=0;i<4;i++)
		delete m_pcEditViewArr[i];

	//	Dec. 4, 2002 genta
	/* キャレットの行桁位置表示用フォント */
	::DeleteObject( m_hFontCaretPosInfo );

	m_hWnd = NULL;
}


//! ドキュメントリスナ：セーブ後
// 2008.02.02 kobake
void CEditWnd::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	//ビュー再描画
	this->Views_RedrawAll();

	//キャプションの更新を行う
	UpdateCaption();

	/* キャレットの行桁位置を表示する */
	this->GetActiveView().GetCaret().ShowCaretPosInfo();
}

void CEditWnd::UpdateCaption()
{
	if( !GetActiveView().GetDrawSwitch() )return;

	//キャプション文字列の生成 -> pszCap
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

	//キャプション更新
	::SetWindowText( this->GetHwnd(), to_tchar(pszCap) );

	//@@@ From Here 2003.06.13 MIK
	//タブウインドウのファイル名を通知
	CSakuraEnvironment::ExpandParameter( GetDllShareData().m_Common.m_sTabBar.m_szTabWndCaption, pszCap, _countof( pszCap ));
	this->ChangeFileNameNotify( to_tchar(pszCap), GetListeningDoc()->m_cDocFile.GetFilePath(), CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode );	// 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
	//@@@ To Here 2003.06.13 MIK
}



//!< ウィンドウ生成用の矩形を取得
void CEditWnd::_GetWindowRectForInit(CMyRect* rcResult, int nGroup, const STabGroupInfo& sTabGroupInfo)
{
	/* ウィンドウサイズ継承 */
	int	nWinCX, nWinCY;
	//	2004.05.13 Moca m_Common.m_nSaveWindowSizeをBOOLからenumに変えたため
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_sWindow.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_sWindow.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* ウィンドウサイズ指定 */
	EditInfo fi;
	CCommandLine::Instance()->GetEditInfo(&fi);
	if( fi.m_nWindowSizeX >= 0 ){
		nWinCX = fi.m_nWindowSizeX;
	}
	if( fi.m_nWindowSizeY >= 0 ){
		nWinCY = fi.m_nWindowSizeY;
	}

	/* ウィンドウ位置指定 */
	int nWinOX, nWinOY;
	nWinOX = CW_USEDEFAULT;
	nWinOY = 0;
	// ウィンドウ位置固定
	//	2004.05.13 Moca 保存したウィンドウ位置を使う場合は共有メモリからセット
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_nSaveWindowPos ){
		nWinOX =  m_pShareData->m_Common.m_sWindow.m_nWinPosX;
		nWinOY =  m_pShareData->m_Common.m_sWindow.m_nWinPosY;
	}

	//	2004.05.13 Moca マルチディスプレイでは負の値も有効なので，
	//	未設定の判定方法を変更．(負の値→CW_USEDEFAULT)
	if( fi.m_nWindowOriginX != CW_USEDEFAULT ){
		nWinOX = fi.m_nWindowOriginX;
	}
	if( fi.m_nWindowOriginY != CW_USEDEFAULT ){
		nWinOY = fi.m_nWindowOriginY;
	}

	// 必要なら、タブグループにフィットするよう、変更
	if(sTabGroupInfo.IsValid()){
		RECT rcWork, rcMon;
		GetMonitorWorkRect( sTabGroupInfo.hwndTop, &rcWork, &rcMon );

		const WINDOWPLACEMENT& wpTop = sTabGroupInfo.wpTop;
		nWinCX = wpTop.rcNormalPosition.right  - wpTop.rcNormalPosition.left;
		nWinCY = wpTop.rcNormalPosition.bottom - wpTop.rcNormalPosition.top;
		nWinOX = wpTop.rcNormalPosition.left   + (rcWork.left - rcMon.left);
		nWinOY = wpTop.rcNormalPosition.top    + (rcWork.top - rcMon.top);
	}

	//結果
	rcResult->SetXYWH(nWinOX,nWinOY,nWinCX,nWinCY);
}

HWND CEditWnd::_CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo)
{
	// -- -- -- -- ウィンドウクラス登録 -- -- -- -- //
	WNDCLASSEX	wc;
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= CNormalProcess::Instance()->GetProcessInstance();
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	wc.hIcon			= GetAppIcon( CNormalProcess::Instance()->GetProcessInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= m_pszWndClass;

	//	Dec. 6, 2002 genta
	//	small icon指定のため RegisterClassExに変更
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( CNormalProcess::Instance()->GetProcessInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true );
	ATOM	atom = RegisterClassEx( &wc );
	if( 0 == atom ){
		//	2004.05.13 Moca return NULLを有効にした
		return NULL;
	}

	//矩形取得
	CMyRect rc;
	_GetWindowRectForInit(&rc, nGroup, sTabGroupInfo);

	//作成
	HWND hwndResult = ::CreateWindowEx(
		0,				 	// extended window style
		m_pszWndClass,		// pointer to registered class name
		m_pszWndClass,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		rc.left,			// horizontal position of window
		rc.top,				// vertical position of window
		rc.Width(),			// window width
		rc.Height(),		// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		CNormalProcess::Instance()->GetProcessInstance(),		// handle to application instance
		NULL				// pointer to window-creation data
	);
	return hwndResult;
}

void CEditWnd::_GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup)
{
	HWND hwndTop = NULL;
	WINDOWPLACEMENT	wpTop = {0};

	//From Here @@@ 2003.05.31 MIK
	//タブウインドウの場合は現状値を指定
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		if( nGroup < 0 )	// 不正なグループID
			nGroup = 0;	// グループ指定無し（最近アクティブのグループに入れる）
		EditNode*	pEditNode = CShareData::getInstance()->GetEditNodeAt( nGroup, 0 );	// グループの先頭ウィンドウ情報を取得	// 2007.06.20 ryoji
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if( hwndTop )
		{
			//	Sep. 11, 2003 MIK 新規TABウィンドウの位置が上にずれないように
			// 2007.06.20 ryoji 非プライマリモニタまたはタスクバーを動かした後でもずれないように

			wpTop.length = sizeof(wpTop);
			if( ::GetWindowPlacement( hwndTop, &wpTop ) ){	// 現在の先頭ウィンドウから位置を取得
				if( wpTop.showCmd == SW_SHOWMINIMIZED )
					wpTop.showCmd = pEditNode->m_showCmdRestore;
			}
			else{
				hwndTop = NULL;
			}
		}
	}
	//To Here @@@ 2003.05.31 MIK

	//結果
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void CEditWnd::_AdjustInMonitor(const STabGroupInfo& sTabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( GetHwnd(), &rcDesktop );
	::GetWindowRect( GetHwnd(), &rcOrg );

	// 2005.11.23 Moca マルチモニタ等で問題があったため計算方法変更
	/* ウィンドウ位置調整 */
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

	/* ウィンドウサイズ調整 */
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
		// 現在の先頭ウィンドウから WS_EX_TOPMOST 状態を引き継ぐ	// 2007.05.18 ryoji
		DWORD dwExStyle = (DWORD)::GetWindowLongPtr( sTabGroupInfo.hwndTop, GWL_EXSTYLE );
		::SetWindowPos( GetHwnd(), (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

		//タブウインドウ時は現状を維持
		/* ウィンドウサイズ継承 */
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

		/* ウィンドウサイズ継承 */
		if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize &&
			m_pShareData->m_Common.m_sWindow.m_nWinSizeType == SIZE_MAXIMIZED ){
			::ShowWindow( GetHwnd(), SW_SHOWMAXIMIZED );
		}else
		// 2004.05.14 Moca ウィンドウサイズを直接指定する場合は、最小化表示を受け入れる
		if( WINSIZEMODE_SET == m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize &&
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
	作成

	@date 2002.03.07 genta nDocumentType追加
	@date 2007.06.26 ryoji nGroup追加
*/
HWND CEditWnd::Create(
	int				nGroup			//!< [in] グループID
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	//ウィンドウ数制限
	if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return NULL;
	}

	//タブグループ情報取得
	STabGroupInfo sTabGroupInfo;
	_GetTabGroupInfo(&sTabGroupInfo, nGroup);


	// -- -- -- -- ウィンドウ作成 -- -- -- -- //
	HWND hWnd = _CreateMainWindow(nGroup, sTabGroupInfo);
	if(!hWnd)return NULL;
	m_hWnd = hWnd;

	//コモンコントロール初期化
	MyInitCommonControls();

	//イメージ、ヘルパなどの作成
	m_CMenuDrawer.Create( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd(), &CEditApp::Instance()->GetIcons() );


	// -- -- -- -- 子ウィンドウ作成 -- -- -- -- //

	/* 分割フレーム作成 */
	m_cSplitterWnd.Create( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd(), this );

	/* ビュー */
	m_pcEditViewArr[0]->Create( CNormalProcess::Instance()->GetProcessInstance(), m_cSplitterWnd.GetHwnd(), &GetDocument(), 0, TRUE  );
	m_pcEditViewArr[1]->Create( CNormalProcess::Instance()->GetProcessInstance(), m_cSplitterWnd.GetHwnd(), &GetDocument(), 1, FALSE );
	m_pcEditViewArr[2]->Create( CNormalProcess::Instance()->GetProcessInstance(), m_cSplitterWnd.GetHwnd(), &GetDocument(), 2, FALSE );
	m_pcEditViewArr[3]->Create( CNormalProcess::Instance()->GetProcessInstance(), m_cSplitterWnd.GetHwnd(), &GetDocument(), 3, FALSE );

	m_pcEditViewArr[0]->OnSetFocus();

	/* 子ウィンドウの設定 */
	HWND		hWndArr[4];
	hWndArr[0] = m_pcEditViewArr[0]->GetHwnd();
	hWndArr[1] = m_pcEditViewArr[1]->GetHwnd();
	hWndArr[2] = m_pcEditViewArr[2]->GetHwnd();
	hWndArr[3] = m_pcEditViewArr[3]->GetHwnd();
	m_cSplitterWnd.SetChildWndArr( hWndArr );

	MY_TRACETIME( cRunningTimer, "View created" );

	// -- -- -- -- ダイアログ作成 -- -- -- -- //

	/* 入力補完ウィンドウ作成 */
	m_cHokanMgr.DoModeless(
		CNormalProcess::Instance()->GetProcessInstance(),
		m_pcEditViewArr[0]->GetHwnd(),
		(LPARAM)m_pcEditViewArr[0]
	);


	// -- -- -- -- 各種バー作成 -- -- -- -- //

	/* ツールバー */
	LayoutToolBar();

	/* ステータスバー */
	LayoutStatusBar();

	/* ファンクションキー バー */
	LayoutFuncKey();

	/* タブウインドウ */
	LayoutTabBar();

	/* バーの配置終了 */
	EndLayoutBars( FALSE );


	// -- -- -- -- その他調整など -- -- -- -- //

	// 画面表示直前にDispatchEventを有効化する
	::SetWindowLongPtr( GetHwnd(), GWLP_USERDATA, (LONG_PTR)this );

	// デスクトップからはみ出さないようにする
	_AdjustInMonitor(sTabGroupInfo);

	// ドロップされたファイルを受け入れる
	::DragAcceptFiles( GetHwnd(), TRUE );

	//アクティブ情報
	m_bIsActiveApp = ( ::GetActiveWindow() == GetHwnd() );	// 2007.03.08 ryoji

	// エディタ－トレイ間でのUI特権分離の確認（Vista UIPI機能） 2007.06.07 ryoji
	if( COsVersionInfo().IsWinVista_or_later() ){
		m_bUIPI = FALSE;
		::SendMessage( m_pShareData->m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd() );
		if( !m_bUIPI ){	// 返事が返らない
			TopErrorMessage( GetHwnd(), 
				_T("エディタ間の対話に失敗しました。\n")
				_T("権限レベルの異なるエディタが既に起動している可能性があります。")
			);
			::DestroyWindow( GetHwnd() );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	/* 編集ウィンドウリストへの登録 */
	if( !CShareData::getInstance()->AddEditWndList( GetHwnd(), nGroup ) ){	// 2007.06.26 ryoji nGroup引数追加
		OkMessage( GetHwnd(), _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		::DestroyWindow( GetHwnd() );
		m_hWnd = hWnd = NULL;
		return hWnd;
	}
	CShareData::getInstance()->SetTraceOutSource( GetHwnd() );	// TraceOut()起動元ウィンドウの設定	// 2006.06.26 ryoji

	//	Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	/* タイマーを起動 */ // タイマーのIDと間隔を変更 20060128 aroka
	if( 0 == ::SetTimer( GetHwnd(), IDT_EDIT, 500, NULL ) ){
		WarningMessage( GetHwnd(), _T("CEditWnd::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。") );
	}
	// ツールバーのタイマーを分離した 20060128 aroka
	Timer_ONOFF( TRUE );

	//デフォルトのIMEモード設定
	GetDocument().m_cDocEditor.SetImeMode( m_pShareData->GetTypeSetting(CTypeConfig(0)).m_nImeState );

	return GetHwnd();
}



//! 起動時のファイルオープン処理
void CEditWnd::OpenDocumentWhenStart(
	const SLoadInfo& _sLoadInfo		//!< [in]
)
{
	if( _sLoadInfo.cFilePath.Length() ){
		::ShowWindow( GetHwnd(), SW_SHOW );
		//	Oct. 03, 2004 genta コード確認は設定に依存
		SLoadInfo	sLoadInfo = _sLoadInfo;
		bool		bReadResult = GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
		if( !bReadResult ){
			/* ファイルが既に開かれている */
			if( sLoadInfo.bOpened ){
				::PostMessageAny( GetHwnd(), WM_CLOSE, 0, 0 );
				// 2004.07.12 Moca return NULLだと、メッセージループを通らずにそのまま破棄されてしまい、タブの終了処理が抜ける
				//	この後は正常ルートでメッセージループに入った後WM_CLOSEを受信して直ちにCLOSE & DESTROYとなる．
				//	その中で編集ウィンドウの削除が行われる．
			}
		}
	}
}

void CEditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		//!< [in] 漢字コード
	bool			bViewMode,		//!< [in] ビューモードで開くかどうか
	CTypeConfig		nDocumentType	//!< [in] 文書タイプ．-1のとき強制指定無し．
)
{
	//	Mar. 7, 2002 genta 文書タイプの強制指定
	//	Jun. 4 ,2004 genta ファイル名指定が無くてもタイプ強制指定を有効にする
	if( nDocumentType.IsValid() ){
		GetDocument().m_cDocType.SetDocumentType( nDocumentType, true );
		//	2002/05/07 YAZAKI タイプ別設定一覧の一時適用のコードを流用
		GetDocument().m_cDocType.LockDocumentType();
		/* 設定変更を反映させる */
		GetDocument().OnChangeSetting();
	}

	//	Jun. 4 ,2004 genta ファイル名指定が無くてもビューモード強制指定を有効にする
	CAppMode::Instance()->SetViewMode(bViewMode);
}


/*! ツールバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutToolBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR ){	/* ツールバーを表示する */
		m_cToolbar.CreateToolBar();
	}else{
		m_cToolbar.DestroyToolBar();
	}
}

/*! ステータスバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutStatusBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR ){	/* ステータスバーを表示する */
		/* ステータスバー作成 */
		m_cStatusBar.CreateStatusBar();
	}
	else{
		/* ステータスバー破棄 */
		m_cStatusBar.DestroyStatusBar();
	}
}

/*! ファンクションキーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutFuncKey( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND ){	/* ファンクションキーを表示する */
		if( NULL == m_CFuncKeyWnd.GetHwnd() ){
			bool	bSizeBox;
			if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
				bSizeBox = false;
			}else{
				bSizeBox = true;
				/* ステータスバーがあるときはサイズボックスを表示しない */
				if( m_cStatusBar.GetStatusHwnd() ){
					bSizeBox = false;
				}
			}
			m_CFuncKeyWnd.Open( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd(), &GetDocument(), bSizeBox );
		}
	}else{
		m_CFuncKeyWnd.Close();
	}
}

/*! タブバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutTabBar( void )
{
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd ){	/* タブバーを表示する */
		if( NULL == m_cTabWnd.GetHwnd() ){
			m_cTabWnd.Open( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd() );
		}
	}else{
		m_cTabWnd.Close();
	}
}

/*! バーの配置終了処理
	@date 2006.12.19 ryoji 新規作成
	@data 2007.03.04 ryoji 印刷プレビュー時はバーを隠す
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
		::RedrawWindow( GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW );	// ステータスバーに必要？

		GetActiveView().SetIMECompFormPos();
	}
}

static BOOL MyIsDialogMessage(HWND hwnd, MSG* msg)
{
	if(hwnd==NULL)return FALSE;
	return ::IsDialogMessage(hwnd, msg);
}

//複数プロセス版
/* メッセージループ */
//2004.02.17 Moca GetMessageのエラーチェック
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	int ret;

	while(GetHwnd())
	{
		//メッセージ取得
		ret = GetMessage(&msg,NULL,0,0);
		if(ret== 0)break; //WM_QUIT
		if(ret==-1)break; //GetMessage失敗

		if(0){}
		//ダイアログメッセージ
		else if( MyIsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg ) ){}	//!< 印刷プレビュー 操作バー
		else if( MyIsDialogMessage( m_cDlgFind.GetHwnd(),								&msg ) ){}	//!<「検索」ダイアログ
		else if( MyIsDialogMessage( m_cDlgFuncList.GetHwnd(),							&msg ) ){}	//!<「アウトライン」ダイアログ
		else if( MyIsDialogMessage( m_cDlgReplace.GetHwnd(),							&msg ) ){}	//!<「置換」ダイアログ
		else if( MyIsDialogMessage( m_cDlgGrep.GetHwnd(),								&msg ) ){}	//!<「Grep」ダイアログ
		else if( MyIsDialogMessage( m_cHokanMgr.GetHwnd(),								&msg ) ){}	//!<「入力補完」
		else if( m_cToolbar.EatMessage(&msg ) ){ }													//!<ツールバー
		//アクセラレータ
		else if( m_pShareData->m_hAccel && TranslateAccelerator( msg.hwnd, m_pShareData->m_hAccel, &msg ) ){}
		//通常メッセージ
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

	UINT				idCtl;	/* コントロールのID */
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	/* 項目描画情報 */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	UINT				fuFlags;
	HMENU				hmenu;
	LRESULT				lRes;

	switch( uMsg ){
	case WM_PAINTICON:
//		MYTRACE_A( "WM_PAINTICON\n" );
		return 0;
	case WM_ICONERASEBKGND:
//		MYTRACE_A( "WM_ICONERASEBKGND\n" );
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
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );

	// 2007.09.09 Moca 互換BMPによる画面バッファ
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
			/* メニュー機能のテキストをセット */
			CNativeT	cmemWork;

			/* 機能に対応するキー名の取得(複数) */
			CNativeT**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				CNormalProcess::Instance()->GetProcessInstance(),
				m_pShareData->m_nKeyNameArrNum,
				(KEYDATA*)m_pShareData->m_pKeyNameArr,
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
		idCtl = (UINT) wParam;				/* コントロールのID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報 */
		if( IDW_STATUSBAR == idCtl ){
			if( 5 == lpdis->itemID ){ // 2003.08.26 Moca idがずれて作画されなかった
				int	nColor;
				if( m_pShareData->m_bRecordingKeyMacro	/* キーボードマクロの記録中 */
				 && m_pShareData->m_hwndRecordingKeyMacro == GetHwnd()	/* キーボードマクロを記録中のウィンドウ */
				){
					nColor = COLOR_BTNTEXT;
				}else{
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor( lpdis->hDC, ::GetSysColor( nColor ) );
				::SetBkMode( lpdis->hDC, TRANSPARENT );
				
				// 2003.08.26 Moca 上下中央位置に作画
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
			case ODT_MENU:	/* オーナー描画メニュー */
				/* メニューアイテム描画 */
				m_CMenuDrawer.DrawItem( lpdis );
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch( lpmis->CtlType ){
		case ODT_MENU:	/* オーナー描画メニュー */
//			CMenuDrawer* pCMenuDrawer;
//			pCMenuDrawer = (CMenuDrawer*)lpmis->itemData;


//			MYTRACE_A( "WM_MEASUREITEM  lpmis->itemID=%d\n", lpmis->itemID );
			/* メニューアイテムの描画サイズを計算 */
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
		m_bIsActiveApp = (wParam != 0);	// 自アプリがアクティブかどうか

		// アクティブ化なら編集ウィンドウリストの先頭に移動する		// 2007.04.08 ryoji WM_SETFOCUS から移動
		if( m_bIsActiveApp ){
			CShareData::getInstance()->AddEditWndList( GetHwnd() );	// リスト移動処理
		}

		// キャプション設定、タイマーON/OFF		// 2007.03.08 ryoji WM_ACTIVATEから移動
		UpdateCaption();
		m_CFuncKeyWnd.Timer_ONOFF( m_bIsActiveApp ); // 20060126 aroka
		this->Timer_ONOFF( m_bIsActiveApp ); // 20060128 aroka

		return 0L;

	case WM_WINDOWPOSCHANGED:
		// ポップアップウィンドウの表示切替指示をポストする	// 2007.10.22 ryoji
		// ・WM_SHOWWINDOWはすべての表示切替で呼ばれるわけではないのでWM_WINDOWPOSCHANGEDで処理
		//   （タブグループ解除などの設定変更時はWM_SHOWWINDOWは呼ばれない）
		// ・即時切替だとタブ切替に干渉して元のタブに戻ってしまうことがあるので後で切り替える
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
		/* WM_SIZE 処理 */
		if( SIZE_MINIMIZED == wParam ){
			this->UpdateCaption();
		}
		return OnSize( wParam, lParam );

	//From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca ウィンドウ位置継承
		//	最後の位置を復元するため，移動されるたびに共有メモリに位置を保存する．
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_nSaveWindowPos ){
			if( !::IsZoomed( GetHwnd() ) && !::IsIconic( GetHwnd() ) ){
				// 2005.11.23 Moca ワークエリア座標だとずれるのでスクリーン座標に変更
				RECT rcWork;
				::GetWindowRect( hwnd, &rcWork);
				m_pShareData->m_Common.m_sWindow.m_nWinPosX = rcWork.left;
				m_pShareData->m_Common.m_sWindow.m_nWinPosY = rcWork.top;
			}
		}
		// To Here 2004.05.13 Moca ウィンドウ位置継承
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	//To here 2003.05.31 MIK
	case WM_SYSCOMMAND:
		// タブまとめ表示では閉じる動作はオプション指定に従う	// 2006.02.13 ryoji
		//	Feb. 11, 2007 genta 動作を選べるように(MDI風と従来動作)
		// 2007.02.22 ryoji Alt+F4 のデフォルト機能でモード毎の動作が得られるようになった
		if( wParam == SC_CLOSE ){
			// 印刷プレビューモードでウィンドウを閉じる操作のときはプレビューを閉じる	// 2007.03.04 ryoji
			if( m_pPrintPreview ){
				PrintPreviewModeONOFF();	// 印刷プレビューモードのオン/オフ
				return 0L;
			}
			OnCommand( 0, CKeyBind::GetDefFuncCode( VK_F4, _ALT ), NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	case WM_IME_COMPOSITION:
		if ( lParam & GCS_RESULTSTR ) {
			/* メッセージの配送 */
			return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	//case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYUP:	// 2004.04.28 Moca ALT+キーのキーリピート処理のため追加
	case WM_ENTERMENULOOP:
	case MYWM_IME_REQUEST:   /*  再変換対応 by minfu 2002.03.27  */ // 20020331 aroka
		/* メッセージの配送 */
		return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_EXITMENULOOP:
//		MYTRACE_A( "WM_EXITMENULOOP\n" );
		if( NULL != m_cStatusBar.GetStatusHwnd() ){
			m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		/* メッセージの配送 */
		return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
	case WM_SETFOCUS:
//		MYTRACE_A( "WM_SETFOCUS\n" );

		// Aug. 29, 2003 wmlhq & ryojiファイルのタイムスタンプのチェック処理 OnTimer に移行
		m_nTimerCount = 9;

		// ビューにフォーカスを移動する	// 2007.10.16 ryoji
		::SetFocus( this->GetActiveView().GetHwnd() );
		lRes = 0;

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/* 印刷プレビューモードのときは、キー操作は全部PrintPreviewBarへ転送 */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//検索ボックスを更新
		m_cToolbar.AcceptSharedSearchKey();
		
		return lRes;

	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	ステータスバーのダブルクリックでモード切替ができるようにする
		if( m_cStatusBar.GetStatusHwnd() && pnmh->hwndFrom == m_cStatusBar.GetStatusHwnd() ){
			if( pnmh->code == NM_DBLCLK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 6 ){	//	上書き/挿入
					GetDocument().HandleCommand( F_CHGMOD_INS );
				}
				else if( mp->dwItemSpec == 5 ){	//	マクロの記録開始・終了
					GetDocument().HandleCommand( F_RECKEYMACRO );
				}
				else if( mp->dwItemSpec == 1 ){	//	桁位置→行番号ジャンプ
					GetDocument().HandleCommand( F_JUMP_DIALOG );
				}
			}
			else if( pnmh->code == NM_RCLICK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 2 ){	//	入力改行モード
					m_CMenuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						_T("入力改行コード指定(&CRLF)") ); // 入力改行コード指定(CRLF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						_T("入力改行コード指定(&LF)") ); // 入力改行コード指定(LF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						_T("入力改行コード指定(C&R)") ); // 入力改行コード指定(CR)
						
					//	mp->ptはステータスバー内部の座標なので，スクリーン座標への変換が必要
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
		// 2007.09.08 kobake TTN_NEEDTEXTの処理をA版とW版に分けて明示的に処理するようにしました。
		//                   ※テキストが80文字を超えそうならTOOLTIPTEXT::lpszTextを利用してください。
		case TTN_NEEDTEXT:
			{
				//ツールチップテキスト取得、設定
				LPTOOLTIPTEXT lptip = (LPTOOLTIPTEXT)pnmh;
				GetTooltipText(lptip->szText, _countof(lptip->szText), lptip->hdr.idFrom);
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu( pnmh->hwndFrom );
				if( nId != 0 ) OnCommand( (WORD)0 /*メニュー*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if( pnmh->hwndFrom == m_cToolbar.GetToolbarHwnd() ){
				//	ツールバーのOwner Draw
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
		/* ファイルがドロップされた */
		OnDropFiles( (HDROP) wParam );
		return 0L;
	case WM_QUERYENDSESSION:	//OSの終了
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
		
		if( m_pShareData->m_bRecordingKeyMacro ){					/* キーボードマクロの記録中 */
			if( m_pShareData->m_hwndRecordingKeyMacro == GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				m_pShareData->m_bRecordingKeyMacro = FALSE;			/* キーボードマクロの記録中 */
				m_pShareData->m_hwndRecordingKeyMacro = NULL;		/* キーボードマクロを記録中のウィンドウ */
			}
		}

		/* タイマーを削除 */
		::KillTimer( GetHwnd(), IDT_TOOLBAR );

		/* ドロップされたファイルを受け入れるのを解除 */
		::DragAcceptFiles( hwnd, FALSE );

		/* 編集ウィンドウリストからの削除 */
		CShareData::getInstance()->DeleteEditWndList( GetHwnd() );

		if( m_pShareData->m_hwndDebug == GetHwnd() ){
			m_pShareData->m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
		RELPRINT_A("post MYWM_DELETE_ME to 0x%08X", m_pShareData->m_hwndTray);
		::PostMessageAny( m_pShareData->m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows にスレッドの終了を要求します */
		::PostQuitMessage( 0 );

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// ビジュアルスタイル／クラシックスタイルが切り替わったらツールバーを再作成する
		// （ビジュアルスタイル: Rebar 有り、クラシックスタイル: Rebar 無し）
		if( m_cToolbar.GetToolbarHwnd() ){
			if( IsVisualStyle() == (NULL == m_cToolbar.GetRebarHwnd()) ){
				m_cToolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		/* エディタ－トレイ間でのUI特権分離の確認メッセージ */	// 2007.06.07 ryoji
		m_bUIPI = TRUE;	// トレイからの返事を受け取った
		return 0L;

	case MYWM_CLOSE:
		/* エディタへの終了要求 */
		if( FALSE != ( nRet = OnClose()) ){	// Jan. 23, 2002 genta 警告抑制
			// タブまとめ表示では閉じる動作はオプション指定に従う	// 2006.02.13 ryoji
			if( !(BOOL)wParam ){	// 全終了要求でない場合
				// タブまとめ表示で(無題)を残す指定の場合、残ウィンドウが１個なら新規エディタを起動して終了する
				if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd &&
					!m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin &&
					m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin
					){
					// 自グループ内の残ウィンドウ数を調べる	// 2007.06.20 ryoji
					int nGroup = CShareData::getInstance()->GetGroupId( GetHwnd() );
					if( 1 == CShareData::getInstance()->GetEditorWindowsNum( nGroup ) ){
						EditNode* pEditNode = CShareData::getInstance()->GetEditNode( GetHwnd() );
						if( pEditNode )
							pEditNode->m_bClosing = TRUE;	// 自分はタブ表示してもらわなくていい
						SLoadInfo sLoadInfo;
						sLoadInfo.cFilePath = _T("");
						sLoadInfo.eCharCode = CODE_DEFAULT;
						sLoadInfo.bViewMode = false;
						CControlTray::OpenNewEditor(
							CNormalProcess::Instance()->GetProcessInstance(),
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
		/* トレイからエディタへの編集ファイル名要求通知 */
		pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

		/* 編集ファイル情報を格納 */
		GetDocument().GetEditInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* 設定変更の通知 */
// Oct 10, 2000 ao
/* 設定変更時、ツールバーを再作成するようにする（バーの内容変更も反映） */
		m_cToolbar.DestroyToolBar();
		LayoutToolBar();
// Oct 10, 2000 ao ここまで

		// ファンクションキーを再作成する（バーの内容、位置、グループボタン数の変更も反映）	// 2006.12.19 ryoji
		m_CFuncKeyWnd.Close();
		LayoutFuncKey();

		// タブバーの表示／非表示切り替え	// 2006.12.19 ryoji
		LayoutTabBar();

		// ステータスバーの表示／非表示切り替え	// 2006.12.19 ryoji
		LayoutStatusBar();

		// 水平スクロールバーの表示／非表示切り替え	// 2006.12.19 ryoji
		{
			int i;
			bool b1;
			bool b2;
			b1 = (m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz == FALSE);
			for( i = 0; i < 4; i++ )
			{
				b2 = (m_pcEditViewArr[i]->m_hwndHScrollBar == NULL);
				if( b1 != b2 )		/* 水平スクロールバーを使う */
				{
					m_pcEditViewArr[i]->DestroyScrollBar();
					m_pcEditViewArr[i]->CreateScrollBar();
				}
			}
		}

		// バー変更で画面が乱れないように	// 2006.12.19 ryoji
		EndLayoutBars();

		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd )
		{
			// タブ表示のままグループ化する／しないが変更されていたらタブを更新する必要がある
			m_cTabWnd.Refresh( FALSE );
		}
		if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			if( CShareData::getInstance()->IsTopEditWnd( GetHwnd() ) )
			{
				if( !::IsWindowVisible( GetHwnd() ) )
				{
					// ::ShowWindow( GetHwnd(), SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
					::SetWindowPos( GetHwnd(), NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );

					// このウィンドウの WS_EX_TOPMOST 状態を全ウィンドウに反映する	// 2007.05.18 ryoji
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
				// ::ShowWindow( GetHwnd(), SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
				::SetWindowPos( GetHwnd(), NULL,0,0,0,0,
								SWP_SHOWWINDOW | SWP_NOACTIVATE
								| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
			}
		}

		//	Aug, 21, 2000 genta
		GetDocument().m_cAutoSaveAgent.ReloadAutoSaveParam();

		GetDocument().m_cDocType.SetDocumentIcon();	// Sep. 10, 2002 genta 文書アイコンの再設定
		GetDocument().OnChangeSetting();	/* ビューに設定変更を反映させる */
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


	case MYWM_SETCARETPOS:	/* カーソル位置変更通知 */
		{
			//	2006.07.09 genta LPARAMに新たな意味を追加
			//	bit 0 (MASK 1): (bit 1==0のとき) 0/選択クリア, 1/選択開始・変更
			//	bit 1 (MASK 2): 0: bit 0の設定に従う．1:現在の選択ロックs状態を継続
			//	既存の実装では どちらも0なので強制解除と解釈される．
			//	呼び出し時はe_PM_SETCARETPOS_SELECTSTATEの値を使うこと．
			bool bSelect = (0!= (lParam & 1));
			if( lParam & 2 ){
				// 現在の状態をKEEP
				bSelect = GetActiveView().GetSelectionInfo().m_bSelectingLock;
			}
			
			//	2006.07.09 genta 強制解除しない
			/*
			カーソル位置変換
			 物理位置(行頭からのバイト数、折り返し無し行位置)
			→
			 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLogicPoint* ppoCaret = m_pShareData->GetWorkBuffer<CLogicPoint>();
			CLayoutPoint ptCaretPos;
			GetDocument().m_cLayoutMgr.LogicToLayout(
				*ppoCaret,
				&ptCaretPos
			);
			// 改行の真ん中にカーソルが来ないように	// 2007.08.22 ryoji
			// Note. もとが改行単位の桁位置なのでレイアウト折り返しの桁位置を超えることはない。
			//       選択指定(bSelect==TRUE)の場合にはどうするのが妥当かよくわからないが、
			//       2007.08.22現在ではアウトライン解析ダイアログから桁位置0で呼び出される
			//       パターンしかないので実用上特に問題は無い。
			if( !bSelect ){
				const CDocLine *pTmpDocLine = GetDocument().m_cDocLineMgr.GetLine( ppoCaret->GetY2() );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x ) ptCaretPos.x--;
				}
			}
			//	2006.07.09 genta 選択範囲を考慮して移動
			//	MoveCursorの位置調整機能があるので，最終行以降への
			//	移動指示の調整もMoveCursorにまかせる
			GetActiveView().MoveCursorSelecting( ptCaretPos, bSelect, _CARETMARGINRATE / 3 );
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* カーソル位置取得要求 */
		/*
		カーソル位置変換
		 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		→
		物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		{
			CLogicPoint* ppoCaret = m_pShareData->GetWorkBuffer<CLogicPoint>();
			GetDocument().m_cLayoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos(),
				ppoCaret
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	/* 行(改行単位)データの要求 */
		pLine = GetDocument().m_cDocLineMgr.GetLine(CLogicInt(wParam))->GetDocLineStrWithEOL( &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > (int)m_pShareData->GetWorkBufferCount<EDIT_CHAR>() ){
			auto_memcpy( m_pShareData->GetWorkBuffer<EDIT_CHAR>(), pLine, m_pShareData->GetWorkBufferCount<EDIT_CHAR>() );
		}else{
			auto_memcpy( m_pShareData->GetWorkBuffer<EDIT_CHAR>(), pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
		{
			EDIT_CHAR* pWork = m_pShareData->GetWorkBuffer<EDIT_CHAR>();

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

	//タブウインドウ	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_cTabWnd.TabWindowNotify( wParam, lParam );
		return 0L;

	//バーの表示・非表示	//@@@ 2003.06.10 MIK
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
					// ::ShowWindow( hwnd, SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
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

	//by 鬼 (2) MYWM_CHECKSYSMENUDBLCLKは不要に, WM_LBUTTONDBLCLK追加
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
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（１）	// 2007.04.03 ryoji
		// 前面にいるのが編集ウィンドウならアクティブ状態を保持する
		if( m_pShareData->m_bEditWndChanging && CShareData::IsEditWnd(::GetForegroundWindow()) ){
			wParam = TRUE;	// アクティブ
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_SETTEXT:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（２）	// 2007.04.03 ryoji
		// タイマーを使用してタイトルの変更を遅延する
		if( m_pShareData->m_bEditWndChanging ){
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[ ::_tcslen((LPCTSTR)lParam) + 1 ];
			::_tcscpy( m_pszLastCaption, (LPCTSTR)lParam );	// 変更後のタイトルを記憶しておく
			::SetTimer( GetHwnd(), IDT_CAPTION, 50, NULL );
			return 0L;
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_TIMER:
		if( !OnTimer(wParam, lParam) )
			return 0L;
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	default:
// << 20020331 aroka 再変換対応 for 95/NT
		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
			return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
// >> by aroka
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

/*! 終了時の処理

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
int	CEditWnd::OnClose( void )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	int nRet = GetDocument().OnFileClose();
	if( !nRet ) return nRet;

	// 2005.09.01 ryoji タブまとめ表示の場合は次のウィンドウを前面に（終了時のウィンドウちらつきを抑制）

	// 2007.07.07 ryoji
	// Note. タブまとめでない場合にも以下の処理を通しているのは、以前、タブまとめ設定を同期していなかったときの名残です。
	//       （タブまとめ解除しても非表示のままになっているウィンドウを表示に戻す必要があった）
	//       同期にしている現在では、同期に不具合があって非表示になっている場合でも表示に戻す、という保険的な意味しかありません。

	int i, j;
	EditNode*	p = NULL;
	int nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, FALSE );
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
					// タブまとめ表示でこの画面が非表示から表示に変わってすぐ閉じる場合(タブの中クリック時等)、
					// 以前のウィンドウが消えるよりも先に一気にここまで処理が進んでしまうと
					// あとで画面がちらつくので、以前のウィンドウが消えるのをちょっとだけ待つ
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






//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行(F_HELP_CONTENTS,F_HELP_SEARCH)を修正・復活
void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta マウスから呼びだされた場合
		switch( wID ){
		case F_EXITALL:	//Dec. 26, 2000 JEPRO F_に変更
			/* サクラエディタの全終了 */
			CControlTray::TerminateApplication( GetHwnd() );	// 2006.12.25 ryoji 引数追加
			break;

		case F_HELP_CONTENTS:
			/* ヘルプ目次 */
			ShowWinHelpContents( GetHwnd(), CEditApp::Instance()->GetHelpFilePath() );	//	目次を表示する
			break;

		case F_HELP_SEARCH:
			/* ヘルプキーワード検索 */
			MyWinHelp( GetHwnd(), CEditApp::Instance()->GetHelpFilePath(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelpに変更に変更
			break;

		case F_ABOUT:	//Dec. 25, 2000 JEPRO F_に変更
			/* バージョン情報 */
			{
				CDlgAbout cDlgAbout;
				cDlgAbout.DoModal( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd() );
			}
			break;
		default:
			//ウィンドウ切り替え
			if( wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_nEditArrNum ){
				ActivateFrameWindow( m_pShareData->m_pEditArr[wID - IDM_SELWINDOW].GetHwnd() );
			}
			//最近使ったファイル
			else if( wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999){
				/* 指定ファイルが開かれているか調べる */
				CMRU cMRU;
				EditInfo checkEditInfo;
				cMRU.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
				GetDocument().m_cDocFileOperation.FileLoad( &SLoadInfo(checkEditInfo.m_szPath, checkEditInfo.m_nCharCode, false) );	//	Oct.  9, 2004 genta 共通関数化
			}
			//最近使ったフォルダ
			else if( wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999){
				//フォルダ取得
				CMRUFolder cMRUFolder;
				LPCTSTR pszFolderPath = cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER );

				//Stonee, 2001/12/21 UNCであれば接続を試みる
				NetConnect( pszFolderPath );

				//「ファイルを開く」ダイアログ
				SLoadInfo sLoadInfo(_T(""), CODE_AUTODETECT, false);
				CDocFileOperation& cDocOp = GetDocument().m_cDocFileOperation;
				if( cDocOp.OpenFileDialog(GetHwnd(), pszFolderPath, &sLoadInfo) ){
					//開く
					cDocOp.FileLoad( &sLoadInfo );
				}
			}
			//その他コマンド
			else{
				//ビューにフォーカスを移動しておく
				if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
					::SetFocus( GetActiveView().GetHwnd() );

				// コマンドコードによる処理振り分け
				GetDocument().HandleCommand( (EFunctionCode)MAKELONG( wID, wNotifyCode ));	//	May 19, 2006 genta 上位ビットを渡す
			}
			break;
		}
		break;
	/* アクセラレータからのメッセージ */
	case 1:
		{
			//ビューにフォーカスを移動しておく
			if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
				::SetFocus( GetActiveView().GetHwnd() );

			EFunctionCode nFuncCode = CKeyBind::GetFuncCode(
				wID,
				m_pShareData->m_nKeyNameArrNum,
				m_pShareData->m_pKeyNameArr
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

			//フォーカスがはずれたときに検索キーにしてしまう。

			//検索キーワードを取得
			wchar_t	szText[_MAX_PATH];
			if( m_cToolbar.GetSearchKey(szText,_countof(szText)) )	//キー文字列がある
			{
				//検索キーを登録
				CShareData::getInstance()->AddToSearchKeyArr( szText );
			}

		}
		break;

	/* コントロールからのメッセージには通知コード */
	default:
		break;
	}
	return;
}



//	キーワード：メニューバー順序
//	Sept.14, 2000 Jepro note: メニューバーの項目のキャプションや順番設定などは以下で行っているらしい
//	Sept.16, 2000 Jepro note: アイコンとの関連付けはCShareData_new2.cppファイルで行っている
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
			/* 「ファイル」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, _T("新規作成(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, _T("開く(&O)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, _T("上書き保存(&S)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, _T("名前を付けて保存(&A)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL		, _T("すべて上書き保存(&Z)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//	Feb. 28, 2003 genta 保存して閉じるを追加．閉じるメニューを近くに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVECLOSE	, _T("保存して閉じる(&E)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T("閉じる(&C)") );	//Feb. 18, 2001	JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, _T("閉じて(無題) (&R)") );	//Oct. 17, 2000 jepro キャプションを「閉じる」から変更	//Feb. 18, 2001 JEPRO アクセスキー変更(C→B; Blankの意味)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, _T("閉じて開く(&L)...") );

			// 「文字コードセット」ポップアップメニュー
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, _T("開き直す(&W)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp_2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, _T("SJISで開き直す(&S)") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, _T("JISで開き直す(&J)") );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, _T("EUCで開き直す(&E)") );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, _T("Unicodeで開き直す(&U)") );	//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, _T("UnicodeBEで開き直す(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, _T("UTF-8で開き直す(&8)") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, _T("UTF-7で開き直す(&7)") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp_2 , _T("開き直す(&W)") );//Oct. 11, 2000 JEPRO アクセスキー変更(M→H)

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, _T("印刷(&P)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, _T("印刷プレビュー(&V)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, _T("ページ設定(&U)...") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 「ファイル操作」ポップアップメニュー
			//hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, _T("ファイルのプロパティ(&T)") );		//Nov. 7, 2000 jepro キャプションに'ファイルの'を追加
			//m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("ファイル操作(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BROWSE						, _T("ブラウズ(&B)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			/* MRUリストのファイルのリストをメニューにする */
			{
				//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
				CMRU cMRU;
				hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	ファイルメニュー
				if ( cMRU.Length() > 0 ){
					//	アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("最近使ったファイル(&F)") );
				}
				else {
					//	非アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("最近使ったファイル(&F)") );
				}
			}

			/* 最近使ったフォルダのメニューを作成 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
				CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("最近使ったフォルダ(&D)") );
				}
				else {
					//	非アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp , _T("最近使ったフォルダ(&D)") );
				}
			}
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE, _T("グループを閉じる(&G)") );	// グループを閉じる	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T("編集の全終了(&Q)") );	//Feb/ 19, 2001 JEPRO 追加	// 2006.10.21 ryoji 表示文字列変更	// 2007.02.13 ryoji →F_EXITALLEDITORS
			//	Jun. 9, 2001 genta ソフトウェア名改称
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, _T("サクラエディタの全終了(&X)") );	//Sept. 11, 2000 jepro キャプションを「アプリケーション終了」から変更	//Dec. 26, 2000 JEPRO F_に変更
			break;

		case 1:
			/* 「編集」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, _T("元に戻す(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, _T("やり直し(&R)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, _T("切り取り(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, _T("コピー(&C)") );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, _T("全行引用コピー(&N)") );
			//	Sept. 14, 2000 JEPRO	キャプションに「記号付き」を追加、アクセスキー変更(N→.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, _T("選択範囲内全行引用符付きコピー(&.)") );
//			Sept. 30, 2000 JEPRO	引用符付きコピーのアイコンを作成したので上記メニューは重複を避けて「高度な操作」内におくだけにする
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, _T("貼り付け(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, _T("削除(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, _T("すべて選択(&A)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );							/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, _T("再変換(&R)") );		/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, _T("CRLF改行でコピー(&L)") );				//Nov. 9, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, _T("折り返し位置に改行をつけてコピー(&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, _T("矩形貼り付け(&X)") );					//Sept. 13, 2000 JEPRO 移動に伴いアクセスキー付与	//Oct. 22, 2000 JEPRO アクセスキー変更(P→X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, _T("カーソル前を削除(&B)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			// 「挿入」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, _T("日付(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, _T("時刻(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, _T("コントロールコード(&C)...") );	// 2004.05.06 MIK ...追加

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("挿入(&I)") );

			// 「高度な操作」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	_T("単語の左端まで削除(&L)") );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	_T("単語の右端まで削除(&R)") );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	_T("現在位置の単語選択(&W)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	_T("単語切り取り(&T)") );				//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	_T("単語削除(&D)") );					//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	_T("行頭まで切り取り(改行単位) (&U)") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	_T("行末まで切り取り(改行単位) (&K)") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	_T("行頭まで削除(改行単位) (&H)") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	_T("行末まで削除(改行単位) (&E)") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	_T("行切り取り(折り返し単位) (&X)") );	//Jan. 16, 2001 JEPRO 行(頭・末)関係の順序を入れ替えた
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	_T("行削除(折り返し単位) (&Y)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	_T("行の二重化(折り返し単位) (&2)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	_T("TABインデント(&A)") );				//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	_T("逆TABインデント(&B)") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	_T("SPACEインデント(&S)") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	_T("逆SPACEインデント(&P)") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, _T("選択範囲内全行コピー(&@)") );		//Sept. 14, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, _T("選択範囲内全行引用符付きコピー(&.)") );//Sept. 13, 2000 JEPRO キャプションから「記号付き」を追加、アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, _T("選択範囲内全行行番号付きコピー(&:)") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	_T("このファイル名をコピー(&-)") );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	_T("このファイルのパス名をコピー(&\\)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	_T("このファイルのパス名とカーソル位置をコピー(&^)") );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, _T("キー割り当て一覧をコピー(&Q)") );	//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, _T("単語リファレンス(&W)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("高度な操作(&V)") );

		//From Here Feb. 19, 2001 JEPRO [移動(M)], [選択(R)]メニューを[編集]のサブメニューとして移動
			// 「移動」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, _T("カーソル上移動(２行ごと) (&Q)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, _T("カーソル下移動(２行ごと) (&K)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, _T("単語の左端に移動(&L)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, _T("単語の右端に移動(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, _T("前の段落に移動(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, _T("次の段落に移動(&Z)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, _T("行頭に移動(折り返し単位) (&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, _T("行末に移動(折り返し単位) (&E)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, _T("１ページアップ(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, _T("１ページダウン(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, _T("ファイルの先頭に移動(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, _T("ファイルの最後に移動(&B)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, _T("カーソル行をウィンドウ中央へ(&C)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, _T("指定行へジャンプ(&J)...") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T("検索開始位置へ戻る(&I)") );	// 検索開始位置へ戻る 02/06/26 ai
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, _T("移動履歴: 前へ(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, _T("移動履歴: 次へ(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, _T("現在位置を移動履歴に登録(&S)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("移動(&O)") );

			// 「選択」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, _T("現在位置の単語選択(&W)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, _T("すべて選択(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, _T("範囲選択開始(&S)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, _T("(選択)カーソル上移動(２行ごと) (&Q)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, _T("(選択)カーソル下移動(２行ごと) (&K)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, _T("(選択)単語の左端に移動(&L)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, _T("(選択)単語の右端に移動(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, _T("(選択)前の段落に移動(&2)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, _T("(選択)次の段落に移動(&8)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, _T("(選択)行頭に移動(折り返し単位) (&H)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, _T("(選択)行末に移動(折り返し単位) (&T)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, _T("(選択)１ページアップ(&U)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, _T("(選択)１ページダウン(&D)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, _T("(選択)ファイルの先頭に移動(&1)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, _T("(選択)ファイルの最後に移動(&9)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			//	Mar. 11, 2004 genta 矩形選択メニューを選択メニューに統合
			//	今のところは1つしかないので
			// 「矩形選択」ポップアップメニュー
			// hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, _T("矩形範囲選択開始(&B)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP_BOX			, _T("(矩選)カーソル上移動(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN_BOX		, _T("(矩選)カーソル下移動(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LEFT_BOX		, _T("(矩選)カーソル左移動(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RIGHT_BOX		, _T("(矩選)カーソル右移動(&)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UP2_BOX			, _T("(矩選)カーソル上移動(２行ごと) (&Q)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DOWN2_BOX		, _T("(矩選)カーソル下移動(２行ごと) (&K)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDLEFT_BOX	, _T("(矩選)単語の左端に移動(&L)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_BOX	, _T("(矩選)単語の右端に移動(&R)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINETOP_BOX	, _T("(矩選)行頭に移動(折り返し単位) (&H)") x);
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOLINEEND_BOX	, _T("(矩選)行末に移動(折り返し単位) (&E)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageUp_Box	, _T("(選択)半ページアップ(&)") );
////			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HalfPageDown_Box, _T("(選択)半ページダウン(&)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageUp_Box		, _T("(矩選)１ページアップ(&U)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_1PageDown_Box	, _T("(矩選)１ページダウン(&D)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILETOP_BOX	, _T("(矩選)ファイルの先頭に移動(&T)") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GOFILEEND_BOX	, _T("(矩選)ファイルの最後に移動(&B)") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("選択(&S)") );

//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("矩形選択(&E)") );

			// 「整形」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, _T("左(先頭)の空白を削除(&L)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, _T("右(末尾)の空白を削除(&R)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, _T("選択行の昇順ソート(&A)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, _T("選択行の降順ソート(&D)") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, _T("連続した重複行の削除(uniq)(&U)") );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("整形(&K)") );

			break;
		//Feb. 19, 2001 JEPRO [移動][移動], [選択]を[編集]配下に移したので削除

//		case 4://case 2: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 2://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「変換」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, _T("小文字(&L)") );			//Sept. 10, 2000 jepro キャプションを英語から変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, _T("大文字(&U)") );			//Sept. 10, 2000 jepro キャプションを英語から変更
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, _T("全角→半角") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, _T("半角→全角カタカナ") );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, _T("半角→全角ひらがな") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, _T("全角→半角(&F)") );					//Sept. 13, 2000 JEPRO アクセスキー付与
	// From Here 2007.01.24 maru メニューの並び変更とアクセスキー追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, _T("半角＋全ひら→全角・カタカナ(&Z)") );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, _T("半角＋全カタ→全角・ひらがな(&N)") );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, _T("全角英数→半角英数(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, _T("半角英数→全角英数(&M)") );				//July. 29, 2001 Misaka アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, _T("全角カタカナ→半角カタカナ(&J)") );		//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru メニューの並び変更とアクセスキー追加
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKATA, _T("半角カタカナ→全角カタカナ(&K)") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENHIRA, _T("半角カタカナ→全角ひらがな(&H)") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, _T("TAB→空白(&S)") );	//Feb. 19, 2001 JEPRO 下から移動した
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, _T("空白→TAB(&T)") );	//---- Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//「文字コード変換」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, _T("自動判別→SJISコード変換(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, _T("E-Mail(JIS→SJIS)コード変換(&M)") );//Sept. 11, 2000 JEPRO キャプションに「E-Mail」を追加しアクセスキー変更(V→M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, _T("EUC→SJISコード変換(&W)") );		//Sept. 11, 2000 JEPRO アクセスキー変更(E→W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, _T("Unicode→SJISコード変換(&U)") );	//Sept. 11, 2000 JEPRO アクセスキー変更候補はI:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, _T("UnicodeBE→SJISコード変換(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, _T("UTF-8→SJISコード変換(&T)") );	//Sept. 11, 2000 JEPRO アクセスキー付与(T:uTF/shifT)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, _T("UTF-7→SJISコード変換(&F)") );	//Sept. 11, 2000 JEPRO アクセスキー付与(F:utF/shiFt)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, _T("SJIS→JISコード変換(&J)") );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, _T("SJIS→EUCコード変換(&E)") );		//Sept. 11, 2000 JEPRO アクセスキー付与
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, _T("SJIS→&Unicodeコード変換") );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, _T("SJIS→UTF-8コード変換(&8)") );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, _T("SJIS→UTF-7コード変換(&7)") );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, _T("Base64デコードして保存(&B)") );	//Oct. 6, 2000 JEPRO アクセスキー変更(6→B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, _T("uudecodeして保存(&D)") );			//Sept. 11, 2000 JEPRO アクセスキー変更(U→D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("文字コード変換(&C)") );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 3://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「検索」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, _T("検索(&F)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, _T("次を検索(&N)") );				//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, _T("前を検索(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, _T("置換(&R)...") );				//Oct. 7, 2000 JEPRO 下のセクションからここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, _T("検索マークの切替え(&C)") );	// "検索マークのクリア(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T("検索開始位置へ戻る(&I)") );	// 検索開始位置へ戻る 02/06/26 ai
		
			// Jan. 10, 2005 genta インクリメンタルサーチ
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_NEXT, _T("前方インクリメンタルサーチ(&F)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_PREV, _T("後方インクリメンタルサーチ(&B)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_NEXT, _T("正規表現前方インクリメンタルサーチ(&R)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_PREV, _T("正規表現後方インクリメンタルサーチ(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_NEXT, _T("MIGEMO前方インクリメンタルサーチ(&M)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_PREV, _T("MIGEMO後方インクリメンタルサーチ(&N)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("インクリメンタルサーチ(&S)") );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
// From Here 2001.12.03 hor
			// Jan. 10, 2005 genta 長すぎるのでブックマークをサブメニューに
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, _T("ブックマーク設定・解除(&S)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, _T("次のブックマークへ(&A)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, _T("前のブックマークへ(&Z)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, _T("ブックマークの全解除(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, _T("ブックマークの一覧(&V)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("ブックマーク(&M)") );
// To Here 2001.12.03 hor
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, _T("Grep(&G)...") );					//Oct. 7, 2000 JEPRO 下からここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, _T("指定行へジャンプ(&J)...") );	//Sept. 11, 2000 jepro キャプションに「 ジャンプ」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, _T("アウトライン解析(&L)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, _T("タグジャンプ(&T)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, _T("タグジャンプバック(&B)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, _T("タグファイルの作成...") );	//@@@ 2003.04.13 MIK // 2004.05.06 MIK ...追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, _T("ダイレクトタグジャンプ") );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP_KEYWORD	, _T("キーワードを指定してタグジャンプ") ); //@@ 2005.03.31 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC				, _T("同名のC/C++ヘッダ(ソース)を開く(&C)") );	//Feb. 7, 2001 JEPRO 追加
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, _T("ファイル内容比較(&@)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, _T("DIFF差分表示(&D)...") );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, _T("次の差分へ") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, _T("前の差分へ") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, _T("差分表示の全解除") );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	対括弧の検索をメニューに追加
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, _T("対括弧の検索(&[)") );
//	To Here Sept. 1, 2000

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 4://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「オプション」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( !m_pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, _T("キーマクロの記録開始(&R)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, _T("キーマクロの保存(&M)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, _T("キーマクロの読み込み(&A)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, _T("キーマクロの実行(&D)") );
			}else{
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECKEYMACRO	, _T("キーマクロの記録終了(&R)") );
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SAVEKEYMACRO, _T("キーマクロの記録終了&&保存(&M)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_LOADKEYMACRO, _T("キーマクロの記録終了&&読み込み(&A)") );
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECKEYMACRO, _T("キーマクロの記録終了&&実行(&D)") );
			}
			
			//	From Here Sep. 14, 2001 genta
			//「登録済みマクロ」ポップアップ
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
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("登録済みマクロ(&B)") );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, _T("外部コマンド実行(&X)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECMD_DIALOG, _T("外部コマンド実行(&X)...") );	//Mar. 10, 2001 JEPRO 機能しないのでメニューから隠した	//Mar.21, 2001 JEPRO 標準出力しないで復活 // 2004.05.06 MIK ...追加
			//To Here Sept. 20, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, _T("SQL*Plusをアクティブ表示(&P)") );	//Sept. 11, 2000 JEPRO アクセスキー付与	説明の「アクティブ化」を「アクティブ表示」に統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, _T("SQL*Plusで実行(&S)") );			//Sept. 11, 2000 JEPRO アクセスキー付与

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, _T("入力補完(&/)") );
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

			//「カスタムメニュー」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	右クリックメニュー
			if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0] );
			}
			//	カスタムメニュー
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i] );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("カスタムメニュー(&U)") );

//		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

			break;

		case 5://Feb. 28, 2004 genta 「設定」メニュー
			//	設定項目を「ツール」から独立させた
			m_CMenuDrawer.ResetContents();
			/* 「ウィンドウ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

//	From Here Sept. 17, 2000 JEPRO
//	やはりWin標準に合わせてチェックマークだけで表示／非表示を判断するようにした方がいいので変更
			if ( !m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				pszLabel = _T("ツールバーを表示(&T)");				//これのみ表示
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//これのみ
				pszLabel = _T("ファンクションキーを表示(&K)");		//これのみ表示
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//これのみ
				pszLabel = _T("タブバーを表示");		//これのみ表示	//@@@ 2003.06.10 MIK	// 2007.02.13 ryoji 「タブ」→「タブバー」
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//これのみ	//@@@ 2003.06.10 MIK
				pszLabel = _T("ステータスバーを表示(&S)");			//これのみ表示
//	To Here Sept.17, 2000 JEPRO
//	From Here Oct. 28, 2000 JEPRO
//	3つボタンのアイコンができたことに伴い表示／非表示のメッセージを変えるように再び変更
			}
			else{
				if( m_cToolbar.GetToolbarHwnd() == NULL ){
					pszLabel = _T("ツールバーを表示(&T)");			//これのみ表示
				}else{
					pszLabel = _T("表示中のツールバーを隠す(&T)");			//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTOOLBAR, pszLabel );	//これのみ
				if( NULL == m_CFuncKeyWnd.GetHwnd() ){
					pszLabel = _T("ファンクションキーを表示(&K)");	//これのみ表示
				}else{
					pszLabel = _T("表示中のファンクションキーを隠す(&K)");	//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWFUNCKEY, pszLabel );	//これのみ
				//@@@ 2003.06.10 MIK
				if( NULL == m_cTabWnd.GetHwnd() ){
					pszLabel = _T("タブバーを表示(&M)");	//これのみ表示	// 2007.02.13 ryoji 「タブ」→「タブバー」
				}else{
					pszLabel = _T("表示中のタブバーを隠す(&M)");	// 2007.02.13 ryoji 「タブ」→「タブバー」
				}
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWTAB, pszLabel );	//これのみ
				if( m_cStatusBar.GetStatusHwnd() == NULL ){
					pszLabel = _T("ステータスバーを表示(&S)");		//これのみ表示
				}else{
					pszLabel = _T("表示中のステータスバーを隠す(&S)");		//Sept. 9, 2000 jepro キャプションに「表示中の」を追加
				}
			}
//	To Here Oct. 28, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SHOWSTATUSBAR, pszLabel );

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, _T("タイプ別設定一覧(&L)...") );	//Sept. 13, 2000 JEPRO 設定より上に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, _T("タイプ別設定(&Y)...") );		//Sept. 13, 2000 JEPRO アクセスキー変更(S→Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, _T("共通設定(&C)...") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, _T("フォント設定(&F)...") );		//Sept. 17, 2000 jepro キャプションに「設定」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, _T("履歴の管理(&O)...") );	//履歴の管理	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
//@@@ 2002.01.14 YAZAKI 折り返さないコマンド追加
// 20051022 aroka タイプ別設定値に戻すコマンド追加
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			//	Jan.  8, 2006 genta 共通関数化
			{
				CLayoutInt ketas;
				CEditView::TOGGLE_WRAP_ACTION mode = this->GetActiveView().GetWrapMode( &ketas );
				if( mode == CEditView::TGWRAP_NONE ){
					pszLabel = _T("幅の変更は出来ません(&W)");
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , pszLabel );
				}
				else {
					if( mode == CEditView::TGWRAP_FULL ){
						pszLabel = _T("折り返さない(&W)");
					}
					else if( mode == CEditView::TGWRAP_WINDOW ){
						pszLabel = _T("現在のウィンドウ幅で折り返し(&W)");
					}
					else {	// TGWRAP_PROP
						pszLabel = _T("タイプ別設定の幅で折り返し(&W)");
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel );
				}
			}
			//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , _T("現在のウィンドウ幅で折り返し(&W)") );	//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			// 2003.06.08 Moca 追加
			// 「モード変更」ポップアップメニュー
			// Feb. 28, 2004 genta 編集メニューから移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, _T("挿入／上書きモード(&I)") );	//Nov. 9, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_VIEWMODE, _T("ビューモード(&R)") );
			if ( !m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				pszLabel = _T("キーワードヘルプ自動表示(&H)");
			}
			else if( IsFuncChecked( &GetDocument(), m_pShareData, F_TOGGLE_KEY_SEARCH ) ){
				pszLabel = _T("キーワードヘルプ自動表示しない(&H)");
			}
			else {
				pszLabel = _T("キーワードヘルプ自動表示する(&H)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOGGLE_KEY_SEARCH, pszLabel );
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, _T("入力改行コード指定(&CRLF)") ); // 入力改行コード指定(CRLF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF, _T("入力改行コード指定(&LF)") ); // 入力改行コード指定(LF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR, _T("入力改行コード指定(C&R)") ); // 入力改行コード指定(CR)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , _T("入力改行コード指定(&E)") );


			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
//		case 5://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
		case 6://Feb. 28, 2004 genta 「設定」の新設のため番号をずらした
			m_CMenuDrawer.ResetContents();
			/* 「ウィンドウ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			if( 1 == m_cSplitterWnd.GetAllSplitRows() ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = _T("上下に分割(&-)");	//Oct. 7, 2000 JEPRO アクセスキーを変更(T→-)
			}else{
				pszLabel = _T("上下分割の解除(&-)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_V , pszLabel );

			if( 1 == m_cSplitterWnd.GetAllSplitCols() ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = _T("左右に分割(&I)");	//Oct. 7, 2000 JEPRO アクセスキーを変更(Y→I)
			}else{
				pszLabel = _T("左右分割の解除(&I)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_H , pszLabel );
//	From Here Sept. 17, 2000 JEPRO	縦横分割の場合も状態によってメニューメッセージが変わるように変更
			if( (1 < m_cSplitterWnd.GetAllSplitRows()) && (1 < m_cSplitterWnd.GetAllSplitCols()) ){ // 2002/2/8 aroka メソッドを通じてアクセス
				pszLabel = _T("縦横分割の解除(&S)");	//Feb. 18, 2001 JEPRO アクセスキー変更(Q→S)
			}else{
				pszLabel = _T("縦横に分割(&S)");	//Sept. 17, 2000 jepro 説明に「に」を追加	//Oct. 7, 2000 JEPRO アクセスキーを変更(S→Q)	//Feb. 18, 2001 JEPRO アクセスキーを元に戻した(Q→S)
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPLIT_VH , pszLabel );
//	To Here Sept. 17, 2000
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T("閉じる(&C)") );			//Feb. 18, 2001 JEPRO アクセスキー変更(O→C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, _T("すべて閉じる(&Q)") );		//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, _T("次のウィンドウ(&N)") );	//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, _T("前のウィンドウ(&P)") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINLIST			, _T("ウィンドウ一覧(&W)...") );		// 2006.03.23 fon
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, _T("重ねて表示(&E)") );		//Oct. 7, 2000 JEPRO アクセスキー変更(C→E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, _T("上下に並べて表示(&H)") );	//Sept. 13, 2000 JEPRO 分割に合わせてメニューの左右と上下を入れ替えた //Oct. 7, 2000 JEPRO アクセスキー変更(V→H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, _T("左右に並べて表示(&T)") );	//Oct. 7, 2000 JEPRO アクセスキー変更(H→T)
			if( (DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST ){
				pszLabel = _T("常に手前を解除(&F)");
			}else{
				pszLabel = _T("常に手前に表示(&F)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOPMOST, pszLabel ); //2004.09.21 Moca	// 2007.06.20 ryoji アクセスキー追加

			hMenuPopUp = ::CreatePopupMenu();	// 2007.06.20 ryoji
			if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				pszLabel = _T("グループ化を解除(&B)");
			}else{
				pszLabel = _T("グループ化(&B)");
			}
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BIND_WINDOW, pszLabel );		//2004.07.14 Kazika 新規追加	// 2007.02.13 ryoji 「結合して表示」→「ひとつにまとめて表示」	// 2007.06.20 ryoji 「グループ化」
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE		, _T("グループを閉じる(&G)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenuSep( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_NEXTGROUP		, _T("次のグループ(&N)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PREVGROUP		, _T("前のグループ(&P)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVERIGHT	, _T("タブを右に移動(&R)") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVELEFT	, _T("タブを左に移動(&L)") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_SEPARATE	, _T("新規グループ(&E)") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTNEXT	, _T("次のグループに移動(&X)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTPREV	, _T("前のグループに移動(&V)") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("タブの操作(&B)") );		// 2007.06.20 ryoji

			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, _T("縦方向に最大化(&X)") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, _T("横方向に最大化(&Y)") );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, _T("すべて最小化(&M)") );		//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */				//Oct. 22, 2000 JEPRO 下の「再描画」復活に伴いセパレータを追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, _T("再描画(&R)") );			//Oct. 22, 2000 JEPRO コメントアウトされていたのを復活させた
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, _T("アウトプット(&U)") );		//Sept. 13, 2000 JEPRO アクセスキー変更(O→U)
// 2006.03.23 fon CHG-start>>
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );		/* セパレータ */
			EditNode*	pEditNodeArr;
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			delete [] pEditNodeArr;
//<< 2006.03.23 fon CHG-end
			break;

//		case 8://case 6: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
//		case 6://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
		case 7://Feb. 28, 2004 genta 「設定」の新設のため番号をずらした
			m_CMenuDrawer.ResetContents();
			/* 「ヘルプ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				bRet = ::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T("目次(&O)") );				//Sept. 7, 2000 jepro キャプションを「ヘルプ目次」から変更	Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(C→O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 _T("キーワード検索(&S)...") );	//Sept. 7, 2000 jepro キャプションを「ヘルプトピックの検索」から変更 //Nov. 25, 2000 jepro「トピックの」→「キーワード」に変更 // 2004.05.06 MIK ...追加
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, _T("コマンド一覧(&M)") );		//Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(L→M)
//Sept. 16, 2000 JEPRO ショートカットキーがうまく働かないので次行は殺して元に戻してある		//Dec. 25, 2000 復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, _T("キー割り当て一覧をコピー(&Q)") );			//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q) IDM_TEST→Fに変更			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, _T("外部ヘルプ１(&E)") );		//Sept. 7, 2000 JEPRO このメニューの順番をトップから下に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, _T("外部HTMLヘルプ(&H)") );	//Sept. 7, 2000 JEPRO このメニューの順番を２番目から下に移動
			m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, _T("バージョン情報(&A)") );	//Dec. 25, 2000 JEPRO F_に変更
			break;
		}
	}

end_of_func_IsEnable:;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if (m_pPrintPreview)	return;	//	印刷プレビューモードなら排除。（おそらく排除しなくてもいいと思うんだけど、念のため）

	/* 機能が利用可能かどうか、チェック状態かどうかを一括チェック */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		/* 機能が利用可能か調べる */
		//	Jan.  8, 2006 genta 機能が有効な場合には明示的に再設定しないようにする．
		if( ! IsFuncEnable( &GetDocument(), m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		/* 機能がチェック状態か調べる */
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




/* ファイルがドロップされた */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
	POINT		pt;
	WORD		cFiles, i;
	EditInfo*	pfi;
	HWND		hWndOwner;

	::DragQueryPoint( hDrop, &pt );
	cFiles = ::DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0);
	/* ファイルをドロップしたときは閉じて開く */
	if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
		cFiles = 1;
	}
	/* 一度にドロップ可能なファイル数 */
	if( cFiles > m_pShareData->m_Common.m_sFile.m_nDropFileNumMax ){
		cFiles = m_pShareData->m_Common.m_sFile.m_nDropFileNumMax;
	}

	for( i = 0; i < cFiles; i++ ) {
		//ファイルパス取得、解決。
		TCHAR		szFile[_MAX_PATH + 1];
		::DragQueryFile( hDrop, i, szFile, _countof(szFile) );
		CSakuraEnvironment::ResolvePath(szFile);

		/* 指定ファイルが開かれているか調べる */
		if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
			::SendMessageAny( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
			/* アクティブにする */
			ActivateFrameWindow( hWndOwner );
			/* MRUリストへの登録 */
			CMRU cMRU;
			cMRU.Add( pfi );
		}
		else{
			/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
			//	2005.06.24 Moca
			if( GetDocument().IsAcceptLoad() ){
				/* ファイル読み込み */
				SLoadInfo sLoadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
				hWndOwner = GetHwnd();
				/* アクティブにする */
				// 2007.06.17 maru すでに開いているかチェック済みだが
				// ドロップされたのはフォルダかもしれないので再チェック
				if(!sLoadInfo.bOpened) ActivateFrameWindow( hWndOwner );
			}
			else{
				/* ファイルをドロップしたときは閉じて開く */
				if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
					/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
					if( GetDocument().m_cDocFileOperation.FileClose() ){
						/* ファイル読み込み */
						SLoadInfo	sLoadInfo(szFile, CODE_AUTODETECT, false);
						GetDocument().m_cDocFileOperation.FileLoad(&sLoadInfo);
						hWndOwner = GetHwnd();
						/* アクティブにする */
						ActivateFrameWindow( hWndOwner );
					}
					goto end_of_drop_query;
				}
				else{
					/* 編集ウィンドウの上限チェック */
					if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
						TCHAR szMsg[512];
						auto_sprintf(
							szMsg,
							_T("編集ウィンドウ数の上限は%dです。\n")
							_T("これ以上は同時に開けません。"),
							MAX_EDITWINDOWS
						);
						::MessageBox( NULL, szMsg, GSTR_APPNAME, MB_OK );
						::DragFinish( hDrop );
						return;
					}
					/* 新たな編集ウィンドウを起動 */
					SLoadInfo sLoadInfo;
					sLoadInfo.cFilePath = szFile;
					sLoadInfo.eCharCode = CODE_AUTODETECT;
					sLoadInfo.bViewMode = false;
					CControlTray::OpenNewEditor(
						CNormalProcess::Instance()->GetProcessInstance(),
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

/*! WM_TIMER 処理 
	@date 2007.04.03 ryoji 新規
*/
LRESULT CEditWnd::OnTimer( WPARAM wParam, LPARAM lParam )
{
	// タイマー ID で処理を振り分ける
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
	default:
		return 1L;
	}

	return 0L;
}


/*! キャプション更新用タイマーの処理
	@date 2007.04.03 ryoji 新規
*/
void CEditWnd::OnCaptionTimer( void )
{
	// 編集画面の切替（タブまとめ時）が終わっていたらタイマーを終了してタイトルバーを更新する
	// まだ切替中ならタイマー継続
	if( !m_pShareData->m_bEditWndChanging ){
		::KillTimer( GetHwnd(), IDT_CAPTION );
		::SetWindowText( GetHwnd(), m_pszLastCaption );
	}
}

/*! システムメニュー表示用タイマーの処理
	@date 2007.04.03 ryoji パラメータ無しにした
	                       以前はコールバック関数でやっていたKillTimer()をここで行うようにした
*/
void CEditWnd::OnSysMenuTimer( void ) //by 鬼(2)
{
	::KillTimer( GetHwnd(), IDT_SYSMENU );	// 2007.04.03 ryoji

	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//システムメニュー表示
		// 2006.04.21 ryoji マルチモニタ対応の修正
		// 2007.05.13 ryoji 0x0313メッセージをポストする方式に変更（TrackPopupMenuだとメニュー項目の有効／無効状態が不正になる問題対策）
		RECT R;
		GetWindowRect(GetHwnd(), &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect( pt, &R );
		::PostMessageAny(
			GetHwnd(),
			0x0313, //右クリックでシステムメニューを表示する際に送信するモノらしい
			0,
			MAKELPARAM( (pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom )
		);
	}
	m_IconClicked = icNone;
}








//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更

/* 印刷プレビューモードのオン/オフ */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;
	HWND	hwndToolBar;

	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
	hwndToolBar = (NULL != m_cToolbar.GetRebarHwnd())? m_cToolbar.GetRebarHwnd(): m_cToolbar.GetToolbarHwnd();

	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	印刷プレビューモードを解除します。	*/
		delete m_pPrintPreview;	//	削除。
		m_pPrintPreview = NULL;	//	NULLか否かで、プリントプレビューモードか判断するため。

		/*	通常モードに戻す	*/
		::ShowWindow( this->m_cSplitterWnd.GetHwnd(), SW_SHOW );
		::ShowWindow( hwndToolBar, SW_SHOW );	// 2006.06.17 ryoji
		::ShowWindow( m_cStatusBar.GetStatusHwnd(), SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.GetHwnd(), SW_SHOW );
		::ShowWindow( m_cTabWnd.GetHwnd(), SW_SHOW );	//@@@ 2003.06.25 MIK

		::SetFocus( GetHwnd() );

		hMenu = ::LoadMenu( CNormalProcess::Instance()->GetProcessInstance(), MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( GetHwnd(), hMenu );
		::DrawMenuBar( GetHwnd() );

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	通常モードを隠す	*/
		hMenu = ::GetMenu( GetHwnd() );
		//	Jun. 18, 2001 genta Print Previewではメニューを削除
		::SetMenu( GetHwnd(), NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( GetHwnd() );

		::ShowWindow( this->m_cSplitterWnd.GetHwnd(), SW_HIDE );
		::ShowWindow( hwndToolBar, SW_HIDE );	// 2006.06.17 ryoji
		::ShowWindow( m_cStatusBar.GetStatusHwnd(), SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.GetHwnd(), SW_HIDE );
		::ShowWindow( m_cTabWnd.GetHwnd(), SW_HIDE );	//@@@ 2003.06.25 MIK

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview = new CPrintPreview( this );
		/* 現在の印刷設定 */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	プリンタの情報を取得。

		/* 現在のデフォルトプリンタの情報を取得 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( !bRes ){
			TopInfoMessage( GetHwnd(), _T("印刷プレビューを実行する前に、プリンタをインストールしてください。\n") );
			return;
		}

		/* 印刷設定の反映 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( GetHwnd(), NULL, TRUE );
		::UpdateWindow( GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}




/* WM_SIZE 処理 */
LRESULT CEditWnd::OnSize( WPARAM wParam, LPARAM lParam )
{
	HWND		hwndToolBar;
	int			cx;
	int			cy;
	int			nToolBarHeight;
	int			nStatusBarHeight;
	int			nFuncKeyWndHeight;
	int			nTabWndHeight;	//タブウインドウ	//@@@ 2003.05.31 MIK
	RECT		rc, rcClient;
	int			nCxHScroll;
	int			nCyHScroll;
	int			nCxVScroll;
	int			nCyVScroll;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる
//	変数削除

	RECT		rcWin;



	cx = LOWORD( lParam );
	cy = HIWORD( lParam );

	/* ウィンドウサイズ継承 */
	if( wParam != SIZE_MINIMIZED ){						/* 最小化は継承しない */
		//	2004.05.13 Moca m_nSaveWindowSizeの解釈追加のため
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize ){		/* ウィンドウサイズ継承をするか */
			if( wParam == SIZE_MAXIMIZED ){					/* 最大化はサイズを記録しない */
				if( m_pShareData->m_Common.m_sWindow.m_nWinSizeType != (int)wParam ){
					m_pShareData->m_Common.m_sWindow.m_nWinSizeType = wParam;
				}
			}else{
				::GetWindowRect( GetHwnd(), &rcWin );
				/* ウィンドウサイズに関するデータが変更されたか */
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

		// 元に戻すときのサイズ種別を記憶	// 2007.06.20 ryoji
		EditNode *p = CShareData::getInstance()->GetEditNode( GetHwnd() );
		if( p != NULL ){
			p->m_showCmdRestore = ::IsZoomed( p->GetHwnd() )? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	/* サイズ変更のタイプ */
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
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
	//タブウインドウ
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
		//	2カラム目に改行コードの表示を挿入
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0廃止に従い、適当に調整
		// 2004-02-28 yasu 文字列を出力時の書式に合わせる
		// 幅を変えた場合にはCEditView::ShowCaretPosInfo()での表示方法を見直す必要あり．
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 行 9999 列"), _T("CRLF"), _T("0000"), _T("Unicode"), _T("REC"), _T("上書") };	//Oct. 30, 2000 JEPRO 千万行も要らん
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth = rc.right - rc.left;
		int			nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // サイズボックスの幅
		int			nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // 境界の幅
		SIZE		sz;
		HDC			hdc;
		int			i;
		// 2004-02-28 yasu
		// 正確な幅を計算するために、表示フォントを取得してhdcに選択させる。
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
		//	初期状態ではすべての部分が「枠あり」だが，メッセージエリアは枠を描画しないようにしている
		//	ため，初期化時の枠が変な風に残ってしまう．初期状態で枠を描画させなくするため，
		//	最初に「枠無し」状態を設定した後でバーの分割を行う．
		m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));

		::SendMessageAny( m_cStatusBar.GetStatusHwnd(), SB_SETPARTS, nStArrNum, (LPARAM)nStArr );
		if (hFont != NULL)
		{
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC( m_cStatusBar.GetStatusHwnd(), hdc );

		::UpdateWindow( m_cStatusBar.GetStatusHwnd() );	// 2006.06.17 ryoji 即時描画でちらつきを減らす
		::GetWindowRect( m_cStatusBar.GetStatusHwnd(), &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( GetHwnd(), &rcClient );

	//@@@ From 2003.05.31 MIK
	//タブウインドウ追加に伴い，ファンクションキー表示位置も調整

	//タブウインドウ
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

	//	2005.04.23 genta ファンクションキー非表示の時は移動しない
	if( m_CFuncKeyWnd.GetHwnd() != NULL ){
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
		{	/* ファンクションキー表示位置／0:上 1:下 */
			::MoveWindow(
				m_CFuncKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE );
		}
		else if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 1 )
		{	/* ファンクションキー表示位置／0:上 1:下 */
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
		::UpdateWindow( m_CFuncKeyWnd.GetHwnd() );	// 2006.06.17 ryoji 即時描画でちらつきを減らす
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

	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if( !m_pPrintPreview ){
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}




/* WM_PAINT 描画処理 */
LRESULT CEditWnd::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	HDC				hdc;
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		PAINTSTRUCT		ps;
		hdc = ::BeginPaint( hwnd, &ps );
		::EndPaint( hwnd, &ps );
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

/* 印刷プレビュー 垂直スクロールバーメッセージ処理 WM_VSCROLL */
LRESULT CEditWnd::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		return 0;
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}




/* 印刷プレビュー 水平スクロールバーメッセージ処理 */
LRESULT CEditWnd::OnHScroll( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		return 0;
	}
	return m_pPrintPreview->OnHScroll( wParam, lParam );
}

LRESULT CEditWnd::OnLButtonDown( WPARAM wParam, LPARAM lParam )
{
	//by 鬼(2) キャプチャーして押されたら非クライアントでもこっちに来る
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
	//by 鬼 2002/04/18
	if(m_IconClicked != icNone)
	{
		if(m_IconClicked == icDown)
		{
			m_IconClicked = icClicked;
			//by 鬼(2) タイマー(IDは適当です)
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


LRESULT CEditWnd::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	//by 鬼
	if(m_IconClicked != icNone)
	{
		//by 鬼(2) 一回押された時だけ
		if(m_IconClicked == icDown)
		{
			POINT P;
			GetCursorPos(&P); //スクリーン座標
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

					//CMyString WPath(GetDocument().GetFilePath(), PathEnd-GetDocument().GetFilePath()); //wchar_tへの変換用
					wchar_t WPath[MAX_PATH];
					{
						//文字列抜き出し
						const TCHAR* p=GetDocument().m_cDocFile.GetFilePath();
						int n = PathEnd - p;
						CMyString tmp(p,n);

						//wchar_tに変換
						wcscpy(WPath,tmp);
					}

					//CMyString WFile(PathEnd+1); //wchar_tへの変換用
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
							//テキストでも持たせる…便利
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
							//移動は禁止
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

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
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

/** マウスホイール処理

	@date 2007.10.16 ryoji OnMouseWheel()から処理抜き出し
*/
BOOL CEditWnd::DoMouseWheel( WPARAM wParam, LPARAM lParam )
{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		// 2006.03.26 ryoji by assitance with John タブ上ならウィンドウ切り替え
		if( m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel && NULL != m_cTabWnd.m_hwndTab )
		{
			POINT pt;
			pt.x = (short)LOWORD( lParam );
			pt.y = (short)HIWORD( lParam );
			int nDelta = (short)HIWORD( wParam );
			HWND hwnd = ::WindowFromPoint( pt );
			if( (hwnd == m_cTabWnd.m_hwndTab || hwnd == m_cTabWnd.GetHwnd()) )
			{
				// 現在開いている編集窓のリストを得る
				EditNode* pEditNodeArr;
				int nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
				if(  nRowNum > 0 )
				{
					// 自分のウィンドウを調べる
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
							// 次のウィンドウ
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
							// 前のウィンドウ
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

						/* 次の（or 前の）ウィンドウをアクティブにする */
						if( i != j )
							ActivateFrameWindow( pEditNodeArr[j].GetHwnd() );
					}

					delete []pEditNodeArr;
				}
				return TRUE;	// 処理した
			}
		}
		return FALSE;	// 処理しなかった
	}
	return FALSE;	// 処理しなかった
}

/* 印刷ページ設定
	印刷プレビュー時にも、そうでないときでも呼ばれる可能性がある。
*/
BOOL CEditWnd::OnPrintPageSetting( void )
{
	/* 印刷設定（CANCEL押したときに破棄するための領域） */
	CDlgPrintSetting	cDlgPrintSetting;
	BOOL				bRes;
	PRINTSETTING		PrintSettingArr[MAX_PRINTSETTINGARR];
	int					i;
	int					nCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[i];
	}

//	cDlgPrintSetting.Create( CNormalProcess::Instance()->GetProcessInstance(), GetHwnd() );
	nCurrentPrintSetting = GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		CNormalProcess::Instance()->GetProcessInstance(),
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		GetHwnd(),
		&nCurrentPrintSetting, /* 現在選択している印刷設定 */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* 現在選択されているページ設定の番号が変更されたか */
		if( nCurrentPrintSetting !=
			GetDocument().m_cDocType.GetDocumentType()->m_nCurrentPrintSetting
		){
//			/* 変更フラグ(タイプ別設定) */
			GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
		}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		//	印刷プレビュー時のみ。
		if ( m_pPrintPreview ){
			/* 現在の印刷設定 */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[GetDocument().m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* 印刷プレビュー スクロールバー初期化 */
			m_pPrintPreview->InitPreviewScrollBar();

			/* 印刷設定の反映 */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( GetHwnd(), NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	::UpdateWindow( GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by 鬼

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
		//念のため
		ReleaseCapture();
		m_IconClicked = icNone;
		Result = 0;
	}
	else if(wp == HTSYSMENU)
		Result = 0;
	else{
		//	2004.05.23 Moca メッセージミス修正
		//	フレームのダブルクリック時後にウィンドウサイズ
		//	変更モードなっていた
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return Result;
}

LRESULT CEditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) //by 鬼(2)
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
		//	2004.05.23 Moca メッセージミス修正
		Result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return Result;
}

/*! ドロップダウンメニュー(開く) */	//@@@ 2002.06.15 MIK
int	CEditWnd::CreateFileDropDownMenu( HWND hwnd )
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;
	int			nIndex;

	// メニュー表示位置を決める	// 2007.03.25 ryoji
	// ※ TBN_DROPDOWN 時の NMTOOLBAR::iItem や NMTOOLBAR::rcButton にはドロップダウンメニュー(開く)ボタンが
	//    複数あるときはどれを押した時も１個目のボタン情報が入るようなのでマウス位置からボタン位置を求める
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

	/* MRUリストのファイルのリストをメニューにする */
	CMRU cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.Length() > 0 )
	{
		m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );
	}

	/* 最近使ったフォルダのメニューを作成 */
	CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.Length() > 0 )
	{
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp, _T("最近使ったフォルダ(&D)") );
	}
	else 
	{
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT)hMenuPopUp, _T("最近使ったフォルダ(&D)") );
	}

	m_CMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T("新規作成(&N)"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T("開く(&O)..."), FALSE );

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
	@brief ウィンドウのアイコン設定

	指定されたアイコンをウィンドウに設定する．
	以前のアイコンは破棄する．

	@param hIcon [in] 設定するアイコン
	@param flag [in] アイコン種別．ICON_BIGまたはICON_SMALL.
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
	標準アイコンの取得

	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta 新設した共通関数を使うように
*/
void CEditWnd::GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const
{
	*hIconBig   = GetAppIcon( CNormalProcess::Instance()->GetProcessInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false );
	*hIconSmall = GetAppIcon( CNormalProcess::Instance()->GetProcessInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true );
}

/*!
	アイコンの取得
	
	指定されたファイル名に対応するアイコン(大・小)を取得して返す．
	
	@param szFile     [in] ファイル名
	@param hIconBig   [out] 大きいアイコンのハンドル
	@param hIconSmall [out] 小さいアイコンのハンドル
	
	@retval true 関連づけられたアイコンが見つかった
	@retval false 関連づけられたアイコンが見つからなかった
	
	@author genta
	@date 2002.09.10
*/
bool CEditWnd::GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const
{
	if( NULL != szFile && szFile[0] != _T('\0') ){
		TCHAR szExt[_MAX_EXT];
		TCHAR FileType[1024];

		// (.で始まる)拡張子の取得
		_tsplitpath( szFile, NULL, NULL, NULL, szExt );
		
		if( ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, _countof(FileType) - 13)){
			_tcscat( FileType, _T("\\DefaultIcon") );
			if( ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)){
				// 関連づけられたアイコンを取得する
				SHFILEINFO shfi;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON );
				*hIconBig = shfi.hIcon;
				SHGetFileInfo( szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON );
				*hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	//	標準のアイコンを返す
	GetDefaultIcon( hIconBig, hIconSmall );
	return false;
}

/*
	@brief メニューバー表示用フォントの初期化
	
	メニューバー表示用フォントの初期化を行う．
	
	@date 2002.12.04 CEditViewのコンストラクタから移動
*/
void CEditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	LOGFONT	lf;
	HDC			hdc;
	HFONT		hFontOld;

	/* LOGFONTの初期化 */
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
	_tcscpy( lf.lfFaceName, _T("ＭＳ ゴシック") );
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
	@brief メニューバーにメッセージを表示する
	
	事前にメニューバー表示用フォントが初期化されていなくてはならない．
	指定できる文字数は最大30バイト．それ以上の場合はうち切って表示する．
	
	@author genta
	@date 2002.12.04
*/
void CEditWnd::PrintMenubarMessage( const TCHAR* msg ) const
{
	if( NULL == ::GetMenu( GetHwnd() ) )	// 2007.03.08 ryoji 追加
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
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	//	Sep. 6, 2003 genta Windows XP(Luna)の場合にはCOLOR_MENUBARを使わなくてはならない
	COLORREF bkColor =
		::GetSysColor( COsVersionInfo().IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU );
	::SetBkColor( hdc, bkColor );
	/*
	int			m_pnCaretPosInfoDx[64];	// 文字列描画用文字幅配列
	for( i = 0; i < _countof( m_pnCaretPosInfoDx ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	*/
	::ExtTextOut( hdc,rc.left,rc.top,ETO_OPAQUE,&rc,szText,nStrLen,NULL/*m_pnCaretPosInfoDx*/); //2007.10.17 kobake めんどいので今のところは文字間隔配列を使わない。
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( GetHwnd(), hdc );
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	ステータスバーが非表示の場合はメニューバーの右端に表示する．
	
	@param msg [in] 表示するメッセージ
	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta CEditViewより移動
*/
void CEditWnd::SendStatusMessage( const TCHAR* msg )
{
	if( NULL == m_cStatusBar.GetStatusHwnd() ){
		// メニューバーへ
		PrintMenubarMessage( msg );
	}
	else{
		// ステータスバーへ
		m_cStatusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! ファイル名変更通知

	@author MIK
	@date 2003.05.31 新規作成
	@date 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
*/
void CEditWnd::ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep )
{
	const TCHAR* pszFilePath = _pszFilePath;

	EditNode	*p;
	int		nIndex;

	if( NULL == pszTabCaption ) pszTabCaption = _T("");	//ガード
	if( NULL == pszFilePath )pszFilePath = _FT("");		//ガード 2006.01.28 ryoji

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

			// 2006.01.28 ryoji ファイル名、Grepモード追加
			size = _countof2( p->m_szFilePath ) - 1;
			_tcsncpy( p->m_szFilePath, pszFilePath, size );
			p->m_szFilePath[ size ] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	cRecentEditNode.Terminate();

	//ファイル名変更通知をブロードキャストする。
	int nGroup = CShareData::getInstance()->GetGroupId( GetHwnd() );
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TWNT_FILE,
		(LPARAM)GetHwnd(),
		GetHwnd(),
		nGroup
	);

	return;
}

/*! 常に手前に表示
	@param top  0:トグル動作 1:最前面 2:最前面解除 その他:なにもしない
	@date 2004.09.21 Moca
*/
void CEditWnd::WindowTopMost( int top )
{
	if( 0 == top ){
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr( GetHwnd(), GWL_EXSTYLE );
		if( dwExstyle & WS_EX_TOPMOST ){
			top = 2; // 最前面である -> 解除
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

	// タブまとめ時は WS_EX_TOPMOST 状態を全ウィンドウで同期する	// 2007.05.18 ryoji
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		HWND hwnd;
		int i;
		for( i = 0, hwndInsertAfter = GetHwnd(); i < m_pShareData->m_nEditArrNum; i++ ){
			hwnd = m_pShareData->m_pEditArr[i].GetHwnd();
			if( hwnd != GetHwnd() && CShareData::getInstance()->IsEditWnd( hwnd ) ){
				if( !CShareData::getInstance()->IsSameGroup( GetHwnd(), hwnd ) )
					continue;
				::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// タイマーの更新を開始／停止する。 20060128 aroka
// ツールバー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときにウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void CEditWnd::Timer_ONOFF( BOOL bStart )
{
	if( NULL != GetHwnd() ){
		if( bStart ){
			/* タイマーを起動 */
			if( 0 == ::SetTimer( GetHwnd(), IDT_TOOLBAR, 300, NULL ) ){
				WarningMessage( GetHwnd(), _T("CEditWnd::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。") );
			}
		} else {
			/* タCマーを削除 */
			::KillTimer( GetHwnd(), IDT_TOOLBAR );
		}
	}
	return;
}

/*!	@brief ウィンドウ一覧をポップアップ表示

	@param[in] bMousePos true: マウス位置にポップアップ表示する

	@date 2006.03.23 fon OnListBtnClickをベースに新規作成
	@date 2006.05.10 ryoji ポップアップ位置変更、その他微修正
	@data 2007.02.28 ryoji フルパス指定のパラメータを削除
*/
LRESULT CEditWnd::PopupWinList( bool bMousePos )
{
	POINT pt;

	// ポップアップ位置をアクティブビューの上辺に設定
	RECT rc;
	
	if( bMousePos ){
		::GetCursorPos( &pt );	// マウスカーソル位置に変更
	}
	else {
		::GetWindowRect( GetActiveView().GetHwnd(), &rc );
		pt.x = rc.right - 150;
		if( pt.x < rc.left )
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// ウィンドウ一覧メニューをポップアップ表示する
	if( NULL != m_cTabWnd.GetHwnd() ){
		m_cTabWnd.TabListMenu( pt );
	}
	else{
		EditNode*	pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();	// 2006.03.23 fon
		int nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
		WinListMenu( hMenu, pEditNodeArr, nRowNum, TRUE );
		// メニューを表示する
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
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

/*! @brief 現在開いている編集窓のリストをメニューにする 
	@date  2006.03.23 fon CEditWnd::InitMenuから移動。////が元からあるコメント。//>は追加コメントアウト。
*/
LRESULT CEditWnd::WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull )
{
	int			i;
	TCHAR		szMemu[280];
//>	EditNode*	pEditNodeArr;
	EditInfo*	pfi;

//>	int	nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
//>		/* セパレータ */
//>		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		CShareData::getInstance()->TransformFileName_MakeCache();
		for( i = 0; i < nRowNum; ++i ){
			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessageAny( pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
			if( pfi->m_bIsGrep ){
				/* データを指定バイト数以内に切り詰める */
				CNativeW	cmemDes;
				int			nDesLen;
				const wchar_t*	pszDes;
				LimitStringLengthW( pfi->m_szGrepKey, wcslen( pfi->m_szGrepKey ), 64, cmemDes );
				pszDes = cmemDes.GetStringPtr();
				nDesLen = wcslen( pszDes );
////	From Here Oct. 4, 2000 JEPRO commented out & modified	開いているファイル数がわかるように履歴とは違って1から数える
////		i >= 10 + 26 の時の考慮を省いた(に近い)が開くファイル数が36個を越えることはまずないので事実上OKでしょう
				auto_sprintf( szMemu, _T("&%tc 【Grep】\"%ls%ts\""),
					((1 + i) <= 9)?(_T('1') + i):(_T('A') + i - 9),
					pszDes,
					( (int)wcslen( pfi->m_szGrepKey ) > nDesLen ) ? _T("・・・"):_T("")
				);
			}
			else if( pEditNodeArr[i].GetHwnd() == m_pShareData->m_hwndDebug ){
////		i >= 10 + 26 の時の考慮を省いた(に近い)が出力ファイル数が36個を越えることはまずないので事実上OKでしょう
				auto_sprintf( szMemu, _T("&%tc アウトプット"), ((1 + i) <= 9)?(_T('1') + i):(_T('A') + i - 9) );

			}
			else{
////		From Here Jan. 23, 2001 JEPRO
////		ファイル名やパス名に'&'が使われているときに履歴等でキチンと表示されない問題を修正(&を&&に置換するだけ)
////<----- From Here Added
				TCHAR	szFile2[_MAX_PATH * 2];
				if( _T('\0') == pfi->m_szPath[0] ){
					_tcscpy( szFile2, _T("(無題)") );
				}else{
					TCHAR buf[_MAX_PATH];
					CShareData::getInstance()->GetTransformFileNameFast( pfi->m_szPath, buf, _MAX_PATH );
					
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
				// SJIS以外の文字コードの種別を表示する
				// gm_pszCodeNameArr_Bracket からコピーするように変更
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

//2007.09.08 kobake 追加
//!ツールチップのテキストを取得
void CEditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// 機能文字列の取得 -> tmp -> wszBuf
	WCHAR tmp[256];
	GetDocument().m_cFuncLookup.Funccode2Name( nID, tmp, _countof(tmp) );
	_wcstotcs(wszBuf, tmp, nBufCount);

	// 機能に対応するキー名の取得(複数)
	CNativeT**	ppcAssignedKeyList;
	int nAssignedKeyNum = CKeyBind::GetKeyStrList(
		CNormalProcess::Instance()->GetProcessInstance(),
		m_pShareData->m_nKeyNameArrNum,
		m_pShareData->m_pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBufへ結合
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



/*! タイマーの処理
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2003.08.29 wmlhq, ryoji nTimerCountの導入
	@date 2006.01.28 aroka ツールバー更新を OnToolbarTimerに移動した
	@date 2007.04.03 ryoji パラメータ無しにした
*/
void CEditWnd::OnEditTimer( void )
{
	//static	int	nLoopCount = 0; // wmlhq m_nTimerCountに移行
	// タイマーの呼び出し間隔を 500msに変更。300*10→500*6にする。 20060128 aroka
	IncrementTimerCount(6);

	// 2006.01.28 aroka ツールバー更新関連は OnToolbarTimerに移動した。
	
	//	Aug. 29, 2003 wmlhq, ryoji
	if( m_nTimerCount == 0 && GetCapture() == NULL ){ 
		// ファイルのタイムスタンプのチェック処理
		GetDocument().m_cAutoReloadAgent.CheckFileTimeStamp();

		// ファイル書込可能のチェック処理
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
//                        ビュー管理                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	CEditViewの画面バッファを削除
	@date 2007.09.09 Moca 新規作成
*/
void CEditWnd::Views_DeleteCompatibleBitmap()
{
	// CEditView群へ転送する
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
	//アクティブ以外を再描画してから…
	for( int v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v ){
			this->m_pcEditViewArr[v]->RedrawAll();
		}
	}
	//アクティブを再描画
	this->GetActiveView().RedrawAll();
}

void CEditWnd::Views_Redraw()
{
	//アクティブ以外を再描画してから…
	for( int v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v )
			m_pcEditViewArr[v]->Redraw();
	}
	//アクティブを再描画
	GetActiveView().Redraw();
}


/* アクティブなペインを設定 */
void  CEditWnd::SetActivePane( int nIndex )
{
	/* アクティブなビューを切り替える */
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;

	// フォーカスを移動する	// 2007.10.16 ryoji
	m_pcEditViewArr[nOldIndex]->CaretUnderLineOFF(TRUE);	//	2002/05/11 YAZAKI
	if( ::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != m_pcEditViewArr[m_nActivePaneIndex]->GetHwnd() )
	{
		// ::SetFocus()でフォーカスを切り替える
		::SetFocus( m_pcEditViewArr[m_nActivePaneIndex]->GetHwnd() );
	}else{
		// アクティブでないときに::SetFocus()するとアクティブになってしまう
		// （不可視なら可視になる）ので内部的に切り替えるだけにする
		m_pcEditViewArr[nOldIndex]->OnKillFocus();
		m_pcEditViewArr[m_nActivePaneIndex]->OnSetFocus();
	}

	this->GetActiveView().RedrawAll();	/* フォーカス移動時の再描画 */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_cDlgFind.GetHwnd() ){		/* 「検索」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_cDlgFind.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cDlgReplace.GetHwnd() ){	/* 「置換」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_cDlgReplace.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cHokanMgr.GetHwnd() ){	/* 「入力補完」ダイアログ */
		m_cHokanMgr.Hide();
		/* モードレス時：検索対象となるビューの変更 */
		m_cHokanMgr.ChangeView( (LPARAM)&this->GetActiveView() );
	}
	if( NULL != m_cDlgFuncList.GetHwnd() ){	/* 「アウトライン」ダイアログ */ // 20060201 aroka
		/* モードレス時：現在位置表示の対象となるビューの変更 */
		m_cDlgFuncList.ChangeView( (LPARAM)&this->GetActiveView() );
	}

	return;
}


/* アクティブなペインを取得 */
int CEditWnd::GetActivePane( void ) const
{
	return m_nActivePaneIndex;
}



/** 非アクティブなペインをRedrawする

	スクロールバーの状態更新はパラメータでフラグ制御 or 別関数にしたほうがいい？
	@date 2007.07.22 ryoji スクロールバーの状態更新を追加
*/
void CEditWnd::RedrawInactivePane(void)
{
	if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
		this->m_pcEditViewArr[m_nActivePaneIndex^1]->AdjustScrollBars();
		this->m_pcEditViewArr[m_nActivePaneIndex^1]->Redraw();
	}
	if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
		this->m_pcEditViewArr[m_nActivePaneIndex^2]->AdjustScrollBars();
		this->m_pcEditViewArr[m_nActivePaneIndex^2]->Redraw();
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			this->m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->AdjustScrollBars();
			this->m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->Redraw();
		}
	}
}

void CEditWnd::Views_DisableSelectArea(bool bRedraw)
{
	for( int i = 0; i < 4; ++i ){
		if( m_pcEditViewArr[i]->GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_pcEditViewArr[i]->GetSelectionInfo().DisableSelectArea( true );
		}
	}
}


/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
BOOL CEditWnd::DetectWidthOfLineNumberAreaAllPane( bool bRedraw )
{
	if ( this->GetActiveView().GetTextArea().DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePaneで計算したら、再設定・再描画が必要と判明した */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			this->m_pcEditViewArr[m_nActivePaneIndex^1]->GetTextArea().DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	表示されていないので再描画しない
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


/*!	レイアウトパラメータの変更

	具体的にはタブ幅と折り返し位置を変更する．
	現在のドキュメントのレイアウトのみを変更し，共通設定は変更しない．

	@date 2005.08.14 genta 新規作成
*/
void CEditWnd::ChangeLayoutParam( bool bShowProgress, CLayoutInt nTabSize, CLayoutInt nMaxLineKetas )
{
	HWND		hwndProgress = NULL;
	if( bShowProgress && NULL != this ){
		hwndProgress = this->m_cStatusBar.GetProgressHwnd();
		//	Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	//	座標の保存
	CLogicPoint* posSave = SavePhysPosOfAllView();

	//	レイアウトの更新
	GetDocument().m_cLayoutMgr.ChangeLayoutParam( nTabSize, nMaxLineKetas );

	//	座標の復元
	RestorePhysPosOfAllView( posSave );

	for( int i = 0; i < 4; i++ ){
		if( m_pcEditViewArr[i]->GetHwnd() ){
			InvalidateRect( m_pcEditViewArr[i]->GetHwnd(), NULL, TRUE );
		}
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}


/*!
	レイアウトの変更に先立って，全てのViewの座標を物理座標に変換して保存する．

	@return データを保存した配列へのポインタ

	@note 取得した値はレイアウト変更後にCEditDoc::RestorePhysPosOfAllViewへ渡す．
	渡し忘れるとメモリリークとなる．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 戻り値をCLogicPoint*に変更
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


/*!	座標の復元

	CEditDoc::SavePhysPosOfAllViewで保存したデータを元に座標値を再計算する．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 引数をCLogicPoint*に変更
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
