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
	Copyright (C) 2007, ryoji, maru, genta, Moca
	Copyright (C) 2008, ryoji, nasukoji, novice, syat
	Copyright (C) 2009, ryoji, nasukoji, Hidetaka Sakai, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"

#include <windows.h>
#include <io.h>
#include <mbctype.h>
#include <mbstring.h>
#include <assert.h>
#include "CEditApp.h"
#include "CShareData.h"
#include "CRunningTimer.h"
#include "CControlTray.h"
#include "CEditWnd.h"
#include "CEditDoc.h"
#include "CDocLine.h"
#include "Debug.h"
#include "CDlgAbout.h"
#include "mymessage.h"
#include "CPrint.h"
#include "etc_uty.h"
#include "os.h"
#include "charcode.h"
#include "global.h"
#include "CDlgPrintSetting.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "CPrintPreview.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "CCommandLine.h" /// 2003/1/26 aroka
#include "CSMacroMgr.h" // Jun. 16, 2002 genta
#include "COsVersionInfo.h"	// Sep. 6, 2003 genta
#include "sakura_rc.h"


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


//	状況によりメニューの表示を変えるコマンドリスト(SetMenuFuncSelで使用)
//		2010/5/19	Uchi
struct SFuncMenuName {
	int				eFunc;
	const TCHAR*	sName[2];		// 選択文字列
};

static const SFuncMenuName	sFuncMenuName[] = {
	{F_RECKEYMACRO,			{_T("キーマクロの記録開始"),			_T("キーマクロの記録終了")}},
	{F_SAVEKEYMACRO,		{_T("キーマクロの保存"),				_T("キーマクロの記録終了&&保存")}},
	{F_LOADKEYMACRO,		{_T("キーマクロの読み込み"),			_T("キーマクロの記録終了&&読み込み")}},
	{F_EXECKEYMACRO,		{_T("キーマクロの実行"),				_T("キーマクロの記録終了&&実行")}},
	{F_SPLIT_V,				{_T("上下に分割"),						_T("上下分割の解除")}},
	{F_SPLIT_H,				{_T("左右に分割"),						_T("左右分割の解除")}},
	{F_SPLIT_VH,			{_T("縦横に分割"),						_T("縦横分割の解除")}},
	{F_TAB_CLOSEOTHER,		{_T("このタブ以外を閉じる"),			_T("このウィンドウ以外を閉じる")}},
	{F_TOPMOST,				{_T("常に手前に表示"),					_T("常に手前を解除")}},
	{F_BIND_WINDOW,			{_T("グループ化"),						_T("グループ化を解除")}},
	{F_SHOWTOOLBAR,			{_T("ツールバーを表示"),				_T("表示中のツールバーを隠す")}},
	{F_SHOWFUNCKEY,			{_T("ファンクションキーを表示"),		_T("表示中のファンクションキーを隠す")}},
	{F_SHOWTAB,				{_T("タブバーを表示"),					_T("表示中のタブバーを隠す")}},
	{F_SHOWSTATUSBAR,		{_T("ステータスバーを表示"),			_T("表示中のステータスバーを隠す")}},
	{F_TOGGLE_KEY_SEARCH,	{_T("キーワードヘルプ自動表示する"),	_T("キーワードヘルプ自動表示しない")}},
};

/*! サブクラス化したツールバーのウィンドウプロシージャ
	@author ryoji
	@date 2006.09.06 ryoji
*/
static WNDPROC g_pOldToolBarWndProc;	// ツールバーの本来のウィンドウプロシージャ

static LRESULT CALLBACK ToolBarWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	// WinXP Visual Style のときにツールバー上でのマウス左右ボタン同時押しで無応答になる
	//（マウスをキャプチャーしたまま放さない） 問題を回避するために右ボタンを無視する
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// 右ボタンの UP/DOWN は本来のウィンドウプロシージャに渡さない

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc );
		break;
	}
	return ::CallWindowProc( g_pOldToolBarWndProc, hWnd, msg, wParam, lParam );
}


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

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditWnd::CEditWnd()
: m_hWnd( NULL )
, m_hwndParent( NULL )
, m_hwndReBar( NULL )	// 2006.06.17 ryoji
, m_hwndToolBar( NULL )
, m_hwndStatusBar( NULL )
, m_hwndProgressBar( NULL )
, m_pPrintPreview( NULL ) //@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
, m_pcDragSourceView( NULL )
, m_nActivePaneIndex( 0 )
, m_nEditViewCount( 1 )
, m_nEditViewMaxCount( _countof(m_pcEditViewArr) )	// 今のところ最大値は固定
, m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ) // 20020331 aroka 再変換対応 for 95/NT
, m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
, m_hwndSearchBox( NULL )
, m_hFontSearchBox( NULL )
, m_bIsActiveApp( false )
, m_pszLastCaption( NULL )
, m_pszMenubarMessage( new TCHAR[MENUBAR_MESSAGE_MAX_LEN] )
, m_nCurrentFocus( 0 )
, m_hAccelWine( NULL )
, m_hAccel( NULL )
, m_bDragMode( false )
, m_IconClicked(icNone) //by 鬼(2)
{
}

CEditWnd::~CEditWnd()
{
	delete m_pPrintPreview;
	m_pPrintPreview = NULL;

	for( int i = 0; i < m_nEditViewMaxCount; i++ ){
		delete m_pcEditViewArr[i];
		m_pcEditViewArr[i] = NULL;
	}
	m_pcEditView = NULL;

	delete m_pcViewFont;
	m_pcViewFont = NULL;

	delete[] m_pszMenubarMessage;
	delete[] m_pszLastCaption;

	//	Dec. 4, 2002 genta
	/* キャレットの行桁位置表示用フォント */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;	// 2008.06.20 ryoji
	m_pcDropTarget = NULL;

	// ウィンドウ毎に作成したアクセラレータテーブルを破棄する(Wine用)
	DeleteAccelTbl();

	m_hWnd = NULL;
}





/*! 親ウィンドウのタイトルを更新

	@date 2007.03.08 ryoji bKillFocusパラメータを除去
*/
void CEditWnd::UpdateCaption()
{
	if( !GetActiveView().m_bDrawSWITCH )return;

	//キャプション文字列の生成 -> pszCap
	char	pszCap[1024];
	const CommonSetting_Window& setting = m_pShareData->m_Common.m_sWindow;
	const char* pszFormat = NULL;
	if( !IsActiveApp() )	pszFormat = setting.m_szWindowCaptionInactive;
	else					pszFormat = setting.m_szWindowCaptionActive;
	m_pcEditDoc->ExpandParameter(
		pszFormat,
		pszCap,
		_countof( pszCap )
	);

	//キャプション更新
	::SetWindowText( m_hWnd, pszCap );

	//@@@ From Here 2003.06.13 MIK
	//タブウインドウのファイル名を通知
	m_pcEditDoc->ExpandParameter( m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption, pszCap, _countof( pszCap ));
	ChangeFileNameNotify( pszCap, m_pcEditDoc->GetFilePath(), m_pcEditDoc->m_bGrepMode );	// 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
	//@@@ To Here 2003.06.13 MIK
}



//!< ウィンドウ生成用の矩形を取得
void CEditWnd::_GetWindowRectForInit(int& nWinOX, int& nWinOY, int& nWinCX, int& nWinCY, int nGroup, const STabGroupInfo& sTabGroupInfo)
{
	/* ウィンドウサイズ継承 */

	//	2004.05.13 Moca m_Common.m_eSaveWindowSizeをBOOLからenumに変えたため
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize ){
		nWinCX = m_pShareData->m_Common.m_sWindow.m_nWinSizeCX;
		nWinCY = m_pShareData->m_Common.m_sWindow.m_nWinSizeCY;
	}else{
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	/* ウィンドウサイズ指定 */
	EditInfo fi;
	CCommandLine::getInstance()->GetEditInfo(&fi);
	if( fi.m_nWindowSizeX >= 0 ){
		nWinCX = fi.m_nWindowSizeX;
	}
	if( fi.m_nWindowSizeY >= 0 ){
		nWinCY = fi.m_nWindowSizeY;
	}

	/* ウィンドウ位置指定 */

	nWinOX = CW_USEDEFAULT;
	nWinOY = 0;
	// ウィンドウ位置固定
	//	2004.05.13 Moca 保存したウィンドウ位置を使う場合は共有メモリからセット
	if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos ){
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
	wc.hInstance		= m_hInstance;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	wc.hIcon			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );

	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_ARROW )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= MAKEINTRESOURCE( IDR_MENU1 );
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	//	Dec. 6, 2002 genta
	//	small icon指定のため RegisterClassExに変更
	wc.cbSize			= sizeof( wc );
	wc.hIconSm			= GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
	ATOM	atom = RegisterClassEx( &wc );
	if( 0 == atom ){
		//	2004.05.13 Moca return NULLを有効にした
		return NULL;
	}

	//矩形取得
	int nWinOX, nWinOY, nWinCX, nWinCY;
	_GetWindowRectForInit(nWinOX, nWinOY, nWinCX, nWinCY, nGroup, sTabGroupInfo);

	//作成
	HWND hwndResult = ::CreateWindowEx(
		0,				 	// extended window style
		GSTR_EDITWINDOWNAME,		// pointer to registered class name
		GSTR_EDITWINDOWNAME,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		nWinOX,				// horizontal position of window
		nWinOY,				// vertical position of window
		nWinCX,				// window width
		nWinCY,				// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		m_hInstance,		// handle to application instance
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
		hwndTop = pEditNode? pEditNode->m_hWnd: NULL;

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
	::GetMonitorWorkRect( m_hWnd, &rcDesktop );
	::GetWindowRect( m_hWnd, &rcOrg );

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
		::SetWindowPos( m_hWnd, (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

		//タブウインドウ時は現状を維持
		/* ウィンドウサイズ継承 */
		// Vista 以降の初回表示アニメーション効果を抑止する
		if( !IsWinVista_or_later() ){
			if( sTabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED )
			{
				::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
			}
			else
			{
				::ShowWindow( m_hWnd, SW_SHOW );
			}
		}
		else
		{
			// 初回表示のアニメーション効果を抑止する

			// 先頭ウィンドウの背後で画面描画してから手前に出す（ツールバーやビューのちらつきを抑える）
			// ここでは、あとで正式に適用されるはずのドキュメントタイプを仮設定して一時描画しておく（ビューの配色切替によるちらつきを抑える）
			// さらに、タイプを戻して画面を無効化だけしておく（何らかの原因で途中停止した場合にはもとのタイプ色で再描画されるように ← 例えばファイルサイズが大きすぎる警告を出すときなど）
			// ※ 正攻法とはいえないかもしれないがあちこち手を入れることなく簡潔に済ませられるのでこうしておく
			int cTypeOld, cTypeNew = -1;
			cTypeOld = m_pcEditDoc->GetDocumentType();	// 現在のタイプ
			{
				EditInfo ei, mruei;
				CCommandLine::getInstance()->GetEditInfo( &ei );
				if( ei.m_szDocType[0] != '\0' ){
					cTypeNew = CShareData::getInstance()->GetDocumentTypeOfExt( ei.m_szDocType );
				}else{
					if( CMRUFile().GetEditInfo( ei.m_szPath, &mruei ) ){
						cTypeNew = mruei.m_nType;
					}
					if( !(cTypeNew>=0 && cTypeNew<MAX_TYPES) ){
						if( ei.m_szPath[0] ){
							cTypeNew = CShareData::getInstance()->GetDocumentTypeOfPath( ei.m_szPath );
						}else{
							cTypeNew = cTypeOld;
						}
					}
				}
			}
			m_pcEditDoc->SetDocumentType( cTypeNew, true, true );	// 仮設定

			// 可能な限り画面描画の様子が見えないよう一時的に先頭ウィンドウの後ろに配置
			::SetWindowPos( m_hWnd, sTabGroupInfo.hwndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

			// アニメーション効果は一時的に OFF にする
			ANIMATIONINFO ai = {sizeof(ANIMATIONINFO)};
			::SystemParametersInfo( SPI_GETANIMATION, sizeof(ANIMATIONINFO), &ai, 0 );
			int iMinAnimateOld = ai.iMinAnimate;
			ai.iMinAnimate = 0;
			::SystemParametersInfo( SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0 );

			// 可視化する（最大化のときは次の ::ShowWindow() で手前に出てしまうので、アニメーション除去効果はあるがクライアント領域のちらつきは抑えきれない）
			int nCmdShow = ( sTabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED )? SW_SHOWMAXIMIZED: SW_SHOWNOACTIVATE;
			::ShowWindow( m_hWnd, nCmdShow );
			::UpdateWindow( m_hWnd );	// 画面更新
			::BringWindowToTop( m_hWnd );
			::ShowWindow( sTabGroupInfo.hwndTop , SW_HIDE );	// 以前の先頭ウィンドウはここで消しておかないと消えるアニメーションが見える場合がある

			// アニメーション効果を戻す
			ai.iMinAnimate = iMinAnimateOld;
			::SystemParametersInfo( SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0 );

			// アイドリング開始時にその時点のタイプ別設定色で再描画されるようにしておく
			m_pcEditDoc->SetDocumentType( cTypeOld, true, true );	// タイプ戻し
			::InvalidateRect( m_hWnd, NULL, TRUE );	// 画面無効化
		}
	}
	else
	{
		::SetWindowPos(
			m_hWnd, 0,
			rcOrg.left, rcOrg.top,
			rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		/* ウィンドウサイズ継承 */
		if( WINSIZEMODE_DEF != m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_sWindow.m_nWinSizeType == SIZE_MAXIMIZED ){
			::ShowWindow( m_hWnd, SW_SHOWMAXIMIZED );
		}else
		// 2004.05.14 Moca ウィンドウサイズを直接指定する場合は、最小化表示を受け入れる
		if( WINSIZEMODE_SET == m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize &&
			m_pShareData->m_Common.m_sWindow.m_nWinSizeType == SIZE_MINIMIZED ){
			::ShowWindow( m_hWnd, SW_SHOWMINIMIZED );
		}
		else{
			::ShowWindow( m_hWnd, SW_SHOW );
		}
	}
	//To Here @@@ 2003.06.13 MIK
}

/*!
	作成

	@date 2002.03.07 genta nDocumentType追加
	@date 2007.06.26 ryoji nGroup追加
	@date 2008.04.19 ryoji 初回アイドリング検出用ゼロ秒タイマーのセット処理を追加
*/
HWND CEditWnd::Create(
	HINSTANCE		hInstance,	//!< [in] Instance Handle
	HWND			hwndParent,	//!< [in] 親ウィンドウのハンドル
	CEditDoc*		pcEditDoc,
	CImageListMgr*	pcIcons,	//!< [in] Image List
	int				nGroup		//!< [in] グループID
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditWnd::Create" );

	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_pcEditDoc = pcEditDoc;

	for( int i = 0; i < _countof(m_pcEditViewArr); i++ ){
		m_pcEditViewArr[i] = NULL;
	}
	// [0] - [3] まで作成・初期化していたものを[0]だけ作る。ほかは分割されるまで何もしない
	m_pcEditViewArr[0] = new CEditView( this );
	m_pcEditView = m_pcEditViewArr[0];

	m_pcViewFont = new CViewFont(&m_pShareData->m_Common.m_sView.m_lf);

	memset( m_pszMenubarMessage, ' ', MENUBAR_MESSAGE_MAX_LEN );	// null終端は不要

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	m_pcDropTarget = new CDropTarget( this );	// 右ボタンドロップ用	// 2008.06.20 ryoji

	// 2009.01.12 nasukoji	ホイールスクロール有無状態をクリア
	ClearMouseState();

	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	CreateAccelTbl();

	//ウィンドウ数制限
	if( m_pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( m_hWnd, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return NULL;
	}

	//タブグループ情報取得
	STabGroupInfo sTabGroupInfo;
	_GetTabGroupInfo(&sTabGroupInfo, nGroup);


	// -- -- -- -- ウィンドウ作成 -- -- -- -- //
	HWND hWnd = _CreateMainWindow(nGroup, sTabGroupInfo);
	if(!hWnd)return NULL;
	m_hWnd = hWnd;

	// 初回アイドリング検出用のゼロ秒タイマーをセットする	// 2008.04.19 ryoji
	// ゼロ秒タイマーが発動（初回アイドリング検出）したら MYWM_FIRST_IDLE を起動元プロセスにポストする。
	// ※起動元での起動先アイドリング検出については CEditApp::OpenNewEditor を参照
	::SetTimer( m_hWnd, IDT_FIRST_IDLE, 0, NULL );

	/* 編集ウィンドウリストへの登録 */
	// 2011.01.12 ryoji この処理は以前はウィンドウ可視化よりも後の位置にあった
	// Vista/7 での初回表示アニメーション抑止（rev1868）とのからみで、ウィンドウが可視化される時点でタブバーに全タブが揃っていないと見苦しいのでここに移動。
	// AddEditWndList() で自ウィンドウにポストされる MYWM_TAB_WINDOW_NOTIFY(TWNT_ADD) はタブバー作成後の初回アイドリング時に処理されるので特に問題は無いはず。
	if( !CShareData::getInstance()->AddEditWndList( m_hWnd, nGroup ) ){	// 2007.06.26 ryoji nGroup引数追加
		OkMessage( m_hWnd, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		::DestroyWindow( m_hWnd );
		m_hWnd = hWnd = NULL;
		return hWnd;
	}

	//コモンコントロール初期化
	MyInitCommonControls();

	//イメージ、ヘルパなどの作成
	m_pcIcons = pcIcons;
	m_CMenuDrawer.Create( m_hInstance, m_hWnd, pcIcons );

	/* 分割フレーム作成 */
	m_cSplitterWnd.Create( m_hInstance, m_hWnd, this );

	/* ビュー */
	m_pcEditViewArr[0]->Create( m_hInstance, m_cSplitterWnd.m_hWnd, m_pcEditDoc, 0, TRUE  );
	m_pcEditViewArr[0]->OnSetFocus();

	/* 子ウィンドウの設定 */
	HWND        hWndArr[2];
	hWndArr[0] = m_pcEditViewArr[0]->m_hWnd;
	hWndArr[1] = NULL;
	m_cSplitterWnd.SetChildWndArr( hWndArr );

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
	::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)this );

	// デスクトップからはみ出さないようにする
	_AdjustInMonitor(sTabGroupInfo);

	// ドロップされたファイルを受け入れる
	::DragAcceptFiles( m_hWnd, TRUE );
	m_pcDropTarget->Register_DropTarget( m_hWnd );	// 右ボタンドロップ用	// 2008.06.20 ryoji

	//アクティブ情報
	m_bIsActiveApp = ( ::GetActiveWindow() == m_hWnd );	// 2007.03.08 ryoji

	// エディタ－トレイ間でのUI特権分離の確認（Vista UIPI機能） 2007.06.07 ryoji
	if( IsWinVista_or_later() ){
		m_bUIPI = FALSE;
		::SendMessage( m_pShareData->m_sHandles.m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)m_hWnd );
		if( !m_bUIPI ){	// 返事が返らない
			TopErrorMessage( m_hWnd,
				_T("エディタ間の対話に失敗しました。\n")
				_T("権限レベルの異なるエディタが既に起動している可能性があります。")
			);
			::DestroyWindow( m_hWnd );
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	CShareData::getInstance()->SetTraceOutSource( m_hWnd );	// TraceOut()起動元ウィンドウの設定	// 2006.06.26 ryoji

	//	Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	/* タイマーを起動 */ // タイマーのIDと間隔を変更 20060128 aroka
	if( 0 == ::SetTimer( m_hWnd, IDT_EDIT, 500, NULL ) ){
		WarningMessage( m_hWnd, _T("CEditWnd::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。") );
	}
	// ツールバーのタイマーを分離した 20060128 aroka
	Timer_ONOFF( true );

	//デフォルトのIMEモード設定
	m_pcEditDoc->SetImeMode( m_pShareData->m_Types[0].m_nImeState );

	return m_hWnd;
}



//! 起動時のファイルオープン処理
void CEditWnd::OpenDocumentWhenStart(
	const char*	pszPath,		//!< [in] 最初に開くファイルのパス．NULLのとき開くファイル無し．
	ECodeType	nCharCode,		//!< [in] 漢字コード
	bool		bReadOnly		//!< [in] 読み取り専用で開くかどうか
)
{
	if( pszPath ){
		char*	pszPathNew = new char[_MAX_PATH];
		strcpy( pszPathNew, pszPath );
		//	Oct. 03, 2004 genta コード確認は設定に依存
		BOOL		bOpened;
		BOOL		bReadResult = m_pcEditDoc->FileRead( pszPathNew, &bOpened, nCharCode, bReadOnly, m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange );
		if( !bReadResult ){
			/* ファイルが既に開かれている */
			if( bOpened ){
				::PostMessage( m_hWnd, WM_CLOSE, 0, 0 );
				// 2004.07.12 Moca return NULLだと、メッセージループを通らずにそのまま破棄されてしまい、タブの終了処理が抜ける
				//	この後は正常ルートでメッセージループに入った後WM_CLOSEを受信して直ちにCLOSE & DESTROYとなる．
				//	その中で編集ウィンドウの削除が行われる．
			}
		}
		delete [] pszPathNew;
	}
}

void CEditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		//!< [in] 漢字コード
	bool			bReadOnly,		//!< [in] 読み取り専用で開くかどうか
	int				nDocumentType	//!< [in] 文書タイプ．-1のとき強制指定無し．
)
{
	//	Mar. 7, 2002 genta 文書タイプの強制指定
	//	Jun. 4 ,2004 genta ファイル名指定が無くてもタイプ強制指定を有効にする
	if( nDocumentType >= 0 ){
		m_pcEditDoc->SetDocumentType( nDocumentType, true );
		//	2002/05/07 YAZAKI タイプ別設定一覧の一時適用のコードを流用
		m_pcEditDoc->LockDocumentType();
	}

	// 文字コードの指定	2008/6/14 Uchi
	if( IsValidCodeType( nCharCode ) || nDocumentType >= 0 ){
		STypeConfig& types = m_pcEditDoc->GetDocumentAttribute();
		ECodeType eDefaultCharCode = static_cast<ECodeType>(types.m_encoding.m_eDefaultCodetype);
		if( !IsValidCodeType( nCharCode ) ){
			nCharCode = eDefaultCharCode;	// 直接コード指定がなければタイプ指定のデフォルト文字コードを使用
		}
		m_pcEditDoc->m_nCharCode = nCharCode;
		if( nCharCode == eDefaultCharCode ){	// デフォルト文字コードと同じ文字コードが選択されたとき
			m_pcEditDoc->m_bBomExist = types.m_encoding.m_bDefaultBom;
			m_pcEditDoc->SetNewLineCode( static_cast<EEolType>( types.m_encoding.m_eDefaultEoltype ) );
		}
		else{
			m_pcEditDoc->m_bBomExist = ( nCharCode == CODE_UNICODE || nCharCode == CODE_UNICODEBE );
			m_pcEditDoc->SetNewLineCode( EOL_CRLF );
		}
	}

	//	Jun. 4 ,2004 genta ファイル名指定が無くても読み取り専用強制指定を有効にする
	m_pcEditDoc->m_bReadOnly = bReadOnly;

	if( nDocumentType >= 0 ){
		/* 設定変更を反映させる */
		m_pcEditDoc->OnChangeSetting();
	}
}


//	キーワード：ステータスバー順序
/* ステータスバー作成 */
void CEditWnd::CreateStatusBar( void )
{
	if( m_hwndStatusBar )return;

	/* ステータスバー */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE 除去
		_T(""),
		m_hWnd,
		IDW_STATUSBAR
	);

	/* プログレスバー */
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

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		m_CFuncKeyWnd.SizeBox_ONOFF( false );
	}

	//スプリッターの、サイズボックスの位置を変更
	m_cSplitterWnd.DoSplit( -1, -1);
}


/* ステータスバー破棄 */
void CEditWnd::DestroyStatusBar( void )
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		bool bSizeBox;
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
			/* サイズボックスの表示／非表示切り替え */
			bSizeBox = false;
		}
		else{
			bSizeBox = true;
			/* ステータスパーを表示している場合はサイズボックスを表示しない */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = false;
			}
		}
		m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//スプリッターの、サイズボックスの位置を変更
	m_cSplitterWnd.DoSplit( -1, -1 );
}

/* ツールバー作成
	@date @@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2005.08.29 aroka ツールバーの折り返し
	@date 2006.06.17 ryoji ビジュアルスタイルが有効の場合はツールバーを Rebar に入れてサイズ変更時のちらつきを無くす
*/
void CEditWnd::CreateToolBar( void )
{
	if( m_hwndToolBar )return;

	REBARINFO		rbi;
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	LONG_PTR		lToolType;
	nFlag = 0;

	// 2006.06.17 ryoji
	// Rebar ウィンドウの作成
	if( IsVisualStyle() ){	// ビジュアルスタイル有効
		m_hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, //レバーコントロール
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |	// 2007.03.08 ryoji WS_VISIBLE 除去
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			m_hWnd,
			NULL,
			m_hInstance,
			NULL
		);

		if( NULL == m_hwndReBar ){
			TopWarningMessage( m_hWnd, _T("Rebar の作成に失敗しました。") );
			return;
		}

		if( m_pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			PreventVisualStyle( m_hwndReBar );	// ビジュアルスタイル非適用のフラットな Rebar にする
		}

		::ZeroMemory(&rbi, sizeof(rbi));
		rbi.cbSize = sizeof(rbi);
		::SendMessage(m_hwndReBar, RB_SETBARINFO, 0, (LPARAM)&rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// ツールバーへの追加スタイル
	}

	/* ツールバーウィンドウの作成 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */	// 2006.06.17 ryoji WS_CLIPCHILDREN 追加	// 2007.03.08 ryoji WS_VISIBLE 除去
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
		if( m_pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			m_pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat = FALSE;
		}
		TopWarningMessage( m_hWnd, _T("ツールバーの作成に失敗しました。") );
		DestroyToolBar();	// 2006.06.17 ryoji
	}
	else{
		// 2006.09.06 ryoji ツールバーをサブクラス化する
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			m_hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		::SendMessage( m_hwndToolBar, TB_SETBUTTONSIZE, 0, (LPARAM)MAKELONG(DpiScaleX(22), DpiScaleY(22)) );	// 2009.10.01 ryoji 高DPI対応スケーリング
		::SendMessage( m_hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0 );
		//	Oct. 12, 2000 genta
		//	既に用意されているImage Listをアイコンとして登録
		m_pcIcons->SetToolBarImages( m_hwndToolBar );
		/* ツールバーにボタンを追加 */
		int count = 0;	//@@@ 2002.06.15 MIK
		int nToolBarButtonNum = 0;// 2005/8/29 aroka
		//	From Here 2005.08.29 aroka
		// はじめにツールバー構造体の配列を作っておく
		TBBUTTON *pTbbArr = new TBBUTTON[m_pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum];
		for( i = 0; i < m_pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
			nIdx = m_pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = m_CMenuDrawer.getButton(nIdx);
			// セパレータが続くときはひとつにまとめる
			// 折り返しボタンもTBSTYLE_SEP属性を持っているので
			// 折り返しの前のセパレータは全て削除される．
			if( (pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum!=0)){
				if( (pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP) ){
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					nToolBarButtonNum--;
				}
			}
			// 仮想折返しボタンがきたら直前のボタンに折返し属性を付ける
			if( pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP ){
				if( nToolBarButtonNum!=0 ){
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			nToolBarButtonNum++;
		}
		//	To Here 2005.08.29 aroka

		for( i = 0; i < nToolBarButtonNum; ++i ){
			tbb = pTbbArr[i];

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//ドロップダウン
				//拡張スタイルに設定
				::SendMessage( m_hwndToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//コンボボックス
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT			lf;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//セパレータ作る
						memset( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  //ボタンにしないと描画が乱れる 2005/8/29 aroka
						my_tbb.idCommand = tbb.idCommand;	//同じIDにしておく
						if( tbb.fsState & TBSTATE_WRAP ){   //折り返し 2005/8/29 aroka
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&my_tbb );
						count++;

						//サイズを設定する
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = DpiScaleX(160);	//ボックスの幅	// 2009.10.01 ryoji 高DPI対応スケーリング
						::SendMessage( m_hwndToolBar, TB_SETBUTTONINFO, (WPARAM)(tbb.idCommand), (LPARAM)&tbi );

						//位置とサイズを取得する
						rc.right = rc.left = rc.top = rc.bottom = 0;
						::SendMessage( m_hwndToolBar, TB_GETITEMRECT, (WPARAM)(count-1), (LPARAM)&rc );

						//コンボボックスを作る
						//	Mar. 8, 2003 genta 検索ボックスを1ドット下にずらした
						m_hwndSearchBox = CreateWindow( _T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, m_hInstance, NULL );
						if( m_hwndSearchBox )
						{
							m_nCurrentFocus = 0;

							memset( &lf, 0, sizeof(lf) );
							lf.lfHeight			= DpiPointsToPixels(-9); // Jan. 14, 2003 genta ダイアログにあわせてちょっと小さく	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
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
							_tcscpy( lf.lfFaceName, _T("ＭＳ Ｐゴシック") );
							m_hFontSearchBox = ::CreateFontIndirect( &lf );
							if( m_hFontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (WPARAM)m_hFontSearchBox, MAKELONG (TRUE, 0) );
							}

							//入力長制限
							::SendMessage( m_hwndSearchBox, CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

							//検索ボックスを更新	// 関数化 2010/6/6 Uchi
							AcceptSharedSearchKey();
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//ボタン
			case TBSTYLE_SEP:		//セパレータ
			default:
				::SendMessage( m_hwndToolBar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( m_pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			lToolType = ::GetWindowLongPtr(m_hwndToolBar, GWL_STYLE);
			lToolType |= (TBSTYLE_FLAT);
			::SetWindowLongPtr(m_hwndToolBar, GWL_STYLE, lToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
		delete []pTbbArr;// 2005/8/29 aroka
	}

	// 2006.06.17 ryoji
	// ツールバーを Rebar に入れる
	if( m_hwndReBar && m_hwndToolBar ){
		// ツールバーの高さを取得する
		DWORD dwBtnSize = ::SendMessage( m_hwndToolBar, TB_GETBUTTONSIZE, 0, 0 );
		DWORD dwRows = ::SendMessage( m_hwndToolBar, TB_GETROWS, 0, 0 );

		// バンド情報を設定する
		// 以前のプラットフォームに _WIN32_WINNT >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
		rbBand.cbSize = CCSIZEOF_STRUCT( REBARBANDINFO, wID );
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = m_hwndToolBar;	// ツールバー
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// バンドを追加する
		::SendMessage( m_hwndReBar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
	}

	return;
}

void CEditWnd::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_hFontSearchBox )
			{
				::DeleteObject( m_hFontSearchBox );
				m_hFontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_nCurrentFocus = 0;
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;

		//if( m_cTabWnd.m_hWnd ) ::UpdateWindow( m_cTabWnd.m_hWnd );
		//if( m_CFuncKeyWnd.m_hWnd ) ::UpdateWindow( m_CFuncKeyWnd.m_hWnd );
	}

	// 2006.06.17 ryoji Rebar を破棄する
	if( m_hwndReBar )
	{
		::DestroyWindow( m_hwndReBar );
		m_hwndReBar = NULL;
	}

	return;
}

/*! ツールバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutToolBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR ){	/* ツールバーを表示する */
		CreateToolBar();
	}else{
		DestroyToolBar();
	}
}

/*! ステータスバーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutStatusBar( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR ){	/* ステータスバーを表示する */
		/* ステータスバー作成 */
		CreateStatusBar();
	}
	else{
		/* ステータスバー破棄 */
		DestroyStatusBar();
	}
}

/*! ファンクションキーの配置処理
	@date 2006.12.19 ryoji 新規作成
*/
void CEditWnd::LayoutFuncKey( void )
{
	if( m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND ){	/* ファンクションキーを表示する */
		if( NULL == m_CFuncKeyWnd.m_hWnd ){
			bool	bSizeBox;
			if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
				bSizeBox = false;
			}else{
				bSizeBox = true;
				/* ステータスバーがあるときはサイズボックスを表示しない */
				if( m_hwndStatusBar ){
					bSizeBox = false;
				}
			}
			m_CFuncKeyWnd.Open( m_hInstance, m_hWnd, m_pcEditDoc, bSizeBox );
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
		if( NULL == m_cTabWnd.m_hWnd ){
			m_cTabWnd.Open( m_hInstance, m_hWnd );
		}
	}else{
		m_cTabWnd.Close();
	}
}

/*! バーの配置終了処理
	@date 2006.12.19 ryoji 新規作成
	@date 2007.03.04 ryoji 印刷プレビュー時はバーを隠す
*/
void CEditWnd::EndLayoutBars( BOOL bAdjust/* = TRUE*/ )
{
	int nCmdShow = m_pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;
	if( NULL != hwndToolBar )
		::ShowWindow( hwndToolBar, nCmdShow );
	if( NULL != m_hwndStatusBar )
		::ShowWindow( m_hwndStatusBar, nCmdShow );
	if( NULL != m_CFuncKeyWnd.m_hWnd )
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, nCmdShow );
	if( NULL != m_cTabWnd.m_hWnd )
		::ShowWindow( m_cTabWnd.m_hWnd, nCmdShow );

	if( bAdjust )
	{
		RECT		rc;
		m_cSplitterWnd.DoSplit( -1, -1 );
		::GetClientRect( m_hWnd, &rc );
		::SendMessage( m_hWnd, WM_SIZE, m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		::RedrawWindow( m_hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW );	// ステータスバーに必要？

		GetActiveView().SetIMECompFormPos();
	}
}


//複数プロセス版
/* メッセージループ */
//2004.02.17 Moca GetMessageのエラーチェック
void CEditWnd::MessageLoop( void )
{
	MSG	msg;
	int ret;
	
	while( m_hWnd )
	{
		//メッセージ取得
		ret = GetMessage( &msg, NULL, 0, 0 );
		if(ret== 0)break; //WM_QUIT
		if(ret==-1)break; //GetMessage失敗

		//ダイアログメッセージ
		if( m_pPrintPreview && NULL != m_pPrintPreview->GetPrintPreviewBarHANDLE() && ::IsDialogMessage( m_pPrintPreview->GetPrintPreviewBarHANDLE(), &msg ) ){}	//!< 印刷プレビュー 操作バー
		else if( m_pcEditDoc->m_cDlgFind.m_hWnd && ::IsDialogMessage( m_pcEditDoc->m_cDlgFind.m_hWnd, &msg ) ){}	//!<「検索」ダイアログ
		else if( m_pcEditDoc->m_cDlgFuncList.m_hWnd && ::IsDialogMessage( m_pcEditDoc->m_cDlgFuncList.m_hWnd, &msg ) ){}	//!<「アウトライン」ダイアログ
		else if( m_pcEditDoc->m_cDlgReplace.m_hWnd && ::IsDialogMessage( m_pcEditDoc->m_cDlgReplace.m_hWnd, &msg ) ){}	//!<「置換」ダイアログ
		else if( m_pcEditDoc->m_cDlgGrep.m_hWnd && ::IsDialogMessage( m_pcEditDoc->m_cDlgGrep.m_hWnd, &msg ) ){}	//!<「Grep」ダイアログ
		else if( m_pcEditDoc->m_cHokanMgr.m_hWnd && ::IsDialogMessage( m_pcEditDoc->m_cHokanMgr.m_hWnd, &msg ) ){}	//!<「入力補完」
		else if( m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, &msg ) ){	/* 「検索ボックス」 */
			ProcSearchBox( &msg );
		}
		//アクセラレータ
		else{
			if( m_hAccel && TranslateAccelerator( msg.hwnd, m_hAccel, &msg ) ){}
			//通常メッセージ
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
	LPNMHDR				pnmh;
	LPTOOLTIPTEXT		lptip;
	int					nPane;
	EditInfo*			pfi;
	int					nCaretPosX;
	int					nCaretPosY;
	POINT*				ppoCaret;
	LPHELPINFO			lphi;
	const char*			pLine;
	int					nLineLen;

	UINT				idCtl;	/* コントロールのID */
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	/* 項目描画情報 */
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
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
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_CMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );

	// 2007.09.09 Moca 互換BMPによる画面バッファ
	case WM_SHOWWINDOW:
		if( !wParam ){
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_MENUSELECT:
		if( NULL == m_hwndStatusBar ){
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		{
			/* メニュー機能のテキストをセット */
			CMemory		cmemWork;

			/* 機能に対応するキー名の取得(複数) */
			CMemory**	ppcAssignedKeyList;
			int			nAssignedKeyNum;
			int			j;
			nAssignedKeyNum = CKeyBind::GetKeyStrList(
				m_hInstance,
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

			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)pszItemStr );


		}
		return 0;


	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				/* コントロールのID */
		lpdis = (DRAWITEMSTRUCT*) lParam;	/* 項目描画情報 */
		if( IDW_STATUSBAR == idCtl ){
			if( 5 == lpdis->itemID ){ // 2003.08.26 Moca idがずれて作画されなかった
				int	nColor;
				if( m_pShareData->m_sFlags.m_bRecordingKeyMacro	/* キーボードマクロの記録中 */
				 && m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == m_hWnd	/* キーボードマクロを記録中のウィンドウ */
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


//			MYTRACE( _T("WM_MEASUREITEM  lpmis->itemID=%d\n"), lpmis->itemID );
			/* メニューアイテムの描画サイズを計算 */
			nItemWidth = m_CMenuDrawer.MeasureItem( lpmis->itemID, &nItemHeight );
			if( 0 < nItemWidth ){
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;




	case WM_PAINT:
		return OnPaint( hwnd, uMsg, wParam, lParam );

	case WM_PASTE:
		return GetActiveView().HandleCommand( F_PASTE, true, 0, 0, 0, 0 );

	case WM_COPY:
		return GetActiveView().HandleCommand( F_COPY, true, 0, 0, 0, 0 );

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch( lphi->iContextType ){
		case HELPINFO_MENUITEM:
			MyWinHelp( hwnd, HELP_CONTEXT, FuncID_To_HelpContextID( lphi->iCtrlId ) );
			break;
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		m_bIsActiveApp = (wParam != 0);	// 自アプリがアクティブかどうか

		// アクティブ化なら編集ウィンドウリストの先頭に移動する		// 2007.04.08 ryoji WM_SETFOCUS から移動
		if( m_bIsActiveApp ){
			CShareData::getInstance()->AddEditWndList( m_hWnd );	// リスト移動処理

			// 2009.01.12 nasukoji	ホイールスクロール有無状態をクリア
			ClearMouseState();
		} else {
			// 非アクティブになるときだけキャプション設定(アクティブ時はほかで呼び出される)
			UpdateCaption();
		}

		// タイマーON/OFF		// 2007.03.08 ryoji WM_ACTIVATEから移動
		m_CFuncKeyWnd.Timer_ONOFF( m_bIsActiveApp ); // 20060126 aroka
		this->Timer_ONOFF( m_bIsActiveApp ); // 20060128 aroka

		return 0L;

	case WM_ENABLE:
		// 右ドロップファイルの受け入れ設定／解除	// 2009.01.09 ryoji
		// Note: DragAcceptFilesを適用した左ドロップについては Enable/Disable で自動的に受け入れ設定／解除が切り替わる
		if( (BOOL)wParam ){
			m_pcDropTarget->Register_DropTarget( m_hWnd );
		}else{
			m_pcDropTarget->Revoke_DropTarget();
		}
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
//		MYTRACE( _T("WM_SIZE\n") );
		/* WM_SIZE 処理 */
		if( SIZE_MINIMIZED == wParam ){
			UpdateCaption();
		}
		return OnSize( wParam, lParam );

	//From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca ウィンドウ位置継承
		//	最後の位置を復元するため，移動されるたびに共有メモリに位置を保存する．
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos ){
			if( !::IsZoomed( m_hWnd ) && !::IsIconic( m_hWnd ) ){
				// 2005.11.23 Moca ワークエリア座標だとずれるのでスクリーン座標に変更
				RECT rcWin;
				::GetWindowRect( hwnd, &rcWin);
				m_pShareData->m_Common.m_sWindow.m_nWinPosX = rcWin.left;
				m_pShareData->m_Common.m_sWindow.m_nWinPosY = rcWin.top;
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
//		MYTRACE( _T("WM_EXITMENULOOP\n") );
		if( NULL != m_hwndStatusBar ){
			::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)_T(""));
		}
		/* メッセージの配送 */
		return Views_DispatchEvent( hwnd, uMsg, wParam, lParam );

	case WM_SETFOCUS:
//		MYTRACE( _T("WM_SETFOCUS\n") );

		// Aug. 29, 2003 wmlhq & ryojiファイルのタイムスタンプのチェック処理 OnTimer に移行
		m_nTimerCount = 9;

		// ビューにフォーカスを移動する	// 2007.10.16 ryoji
		::SetFocus( GetActiveView().m_hWnd );
		lRes = 0;

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/* 印刷プレビューモードのときは、キー操作は全部PrintPreviewBarへ転送 */
		if( m_pPrintPreview ){
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}

		//検索ボックスを更新
		AcceptSharedSearchKey();
		
		return lRes;

	case WM_NOTIFY:
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	ステータスバーのダブルクリックでモード切替ができるようにする
		if( m_hwndStatusBar && pnmh->hwndFrom == m_hwndStatusBar ){
			if( pnmh->code == NM_DBLCLK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 6 ){	//	上書き/挿入
					m_pcEditDoc->HandleCommand( F_CHGMOD_INS );
				}
				else if( mp->dwItemSpec == 5 ){	//	マクロの記録開始・終了
					m_pcEditDoc->HandleCommand( F_RECKEYMACRO );
				}
				else if( mp->dwItemSpec == 1 ){	//	桁位置→行番号ジャンプ
					m_pcEditDoc->HandleCommand( F_JUMP_DIALOG );
				}
			}
			else if( pnmh->code == NM_RCLICK ){
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if( mp->dwItemSpec == 2 ){	//	入力改行モード
					m_CMenuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						_T("入力改行コード指定(&CRLF)"), _T("") ); // 入力改行コード指定(CRLF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						_T("入力改行コード指定(&LF)"), _T("") ); // 入力改行コード指定(LF)
					m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						_T("入力改行コード指定(C&R)"), _T("") ); // 入力改行コード指定(CR)
						
					//	mp->ptはステータスバー内部の座標なので，スクリーン座標への変換が必要
					POINT	po = mp->pt;
					::ClientToScreen( m_hwndStatusBar, &po );
					int nId = ::TrackPopupMenu(
						hMenuPopUp,
						TPM_CENTERALIGN
						| TPM_BOTTOMALIGN
						| TPM_RETURNCMD
						| TPM_LEFTBUTTON
						,
						po.x,
						po.y,
						0,
						m_hWnd,
						NULL
					);
					::DestroyMenu( hMenuPopUp );
					m_pcEditDoc->HandleCommand( nId );
				}
			}
			return 0L;
		}
		//	To Here Feb. 15, 2004 genta 

		switch( pnmh->code ){
		case TTN_NEEDTEXT:
			lptip = (LPTOOLTIPTEXT)pnmh;
			{
				/* ツールバーのツールチップのテキストをセット */
				CMemory**	ppcAssignedKeyList;
				int			nAssignedKeyNum;
				int			j;
				char*		pszKey;
				char		szLabel[1024];


				// From Here Oct. 15, 2001 genta
				// 機能文字列の取得にLookupを使うように変更
				m_pcEditDoc->m_cFuncLookup.Funccode2Name( lptip->hdr.idFrom, szLabel, 1024 );

				// To Here Oct. 15, 2001 genta
				/* 機能に対応するキー名の取得(複数) */
				nAssignedKeyNum = CKeyBind::GetKeyStrList(
					m_hInstance, m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
					(KEYDATA*)m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr, &ppcAssignedKeyList, lptip->hdr.idFrom
				);
				if( 0 < nAssignedKeyNum ){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						strcat( szLabel, "\n        " );
						pszKey = ppcAssignedKeyList[j]->GetStringPtr();
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
				if( nId != 0 ) OnCommand( (WORD)0 /*メニュー*/, (WORD)nId, (HWND)0 );
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if( pnmh->hwndFrom == m_hwndToolBar ){
				//	ツールバーのOwner Draw
				return ToolBarOwnerDraw( (LPNMCUSTOMDRAW)pnmh );
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
			::DestroyWindow( hwnd );
			return TRUE;
		}
		else{
			return FALSE;
		}
	case WM_CLOSE:
		if( OnClose() ){
			::DestroyWindow( hwnd );
		}
		return 0L;
	case WM_DESTROY:
		if( m_pShareData->m_sFlags.m_bRecordingKeyMacro ){					/* キーボードマクロの記録中 */
			if( m_pShareData->m_sFlags.m_hwndRecordingKeyMacro == m_hWnd ){	/* キーボードマクロを記録中のウィンドウ */
				m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;			/* キーボードマクロの記録中 */
				m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;		/* キーボードマクロを記録中のウィンドウ */
			}
		}

		/* タイマーを削除 */
		::KillTimer( m_hWnd, IDT_TOOLBAR );

		/* ドロップされたファイルを受け入れるのを解除 */
		::DragAcceptFiles( hwnd, FALSE );
		m_pcDropTarget->Revoke_DropTarget();	// 右ボタンドロップ用	// 2008.06.20 ryoji

		/* 編集ウィンドウリストからの削除 */
		CShareData::getInstance()->DeleteEditWndList( m_hWnd );

		if( m_pShareData->m_sHandles.m_hwndDebug == m_hWnd ){
			m_pShareData->m_sHandles.m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
		::PostMessage( m_pShareData->m_sHandles.m_hwndTray, MYWM_DELETE_ME, 0, 0 );

		/* Windows にスレッドの終了を要求します */
		::PostQuitMessage( 0 );

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// ビジュアルスタイル／クラシックスタイルが切り替わったらツールバーを再作成する
		// （ビジュアルスタイル: Rebar 有り、クラシックスタイル: Rebar 無し）
		if( m_hwndToolBar ){
			if( IsVisualStyle() == (NULL == m_hwndReBar) ){
				DestroyToolBar();
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
					int nGroup = CShareData::getInstance()->GetGroupId( m_hWnd );
					if( 1 == CShareData::getInstance()->GetEditorWindowsNum( nGroup ) ){
						EditNode* pEditNode = CShareData::getInstance()->GetEditNode( m_hWnd );
						if( pEditNode )
							pEditNode->m_bClosing = TRUE;	// 自分はタブ表示してもらわなくていい
						CControlTray::OpenNewEditor(
							m_hInstance,
							m_hWnd,
							(char*)NULL,
							0,
							false,
							TRUE
						);
					}
				}
			}
			::DestroyWindow( hwnd );
		}
		return nRet;


	case MYWM_GETFILEINFO:
		/* トレイからエディタへの編集ファイル名要求通知 */
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

		/* 編集ファイル情報を格納 */
		m_pcEditDoc->GetEditInfo( pfi );
		return 0L;
	case MYWM_CHANGESETTING:
		/* 設定変更の通知 */
		switch( (e_PM_CHANGESETTING_SELECT)lParam ){
		case PM_CHANGESETTING_ALL:
			// Oct 10, 2000 ao
			/* 設定変更時、ツールバーを再作成するようにする（バーの内容変更も反映） */
			DestroyToolBar();
			LayoutToolBar();
			// Oct 10, 2000 ao ここまで

			// 2008.09.23 nasukoji	非アクティブなウィンドウのツールバーを更新する
			// アクティブなウィンドウはタイマにより更新されるが、それ以外のウィンドウは
			// タイマを停止させており設定変更すると全部有効となってしまうため、ここで
			// ツールバーを更新する
			if( !m_bIsActiveApp )
				UpdateToolbar();

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
				for( i = 0; i < GetAllViewCount(); i++ )
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
				if( CShareData::getInstance()->IsTopEditWnd( m_hWnd ) )
				{
					if( !::IsWindowVisible( m_hWnd ) )
					{
						// ::ShowWindow( m_hWnd, SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
						::SetWindowPos( m_hWnd, NULL,0,0,0,0,
										SWP_SHOWWINDOW | SWP_NOACTIVATE
										| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );

						// このウィンドウの WS_EX_TOPMOST 状態を全ウィンドウに反映する	// 2007.05.18 ryoji
						WindowTopMost( ((DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST)? 1: 2 );
					}
				}
				else
				{
					if( ::IsWindowVisible( m_hWnd ) )
					{
						::ShowWindow( m_hWnd, SW_HIDE );
					}
				}
			}
			else
			{
				if( !::IsWindowVisible( m_hWnd ) )
				{
					// ::ShowWindow( m_hWnd, SW_SHOWNA ) だと非表示から表示に切り替わるときに Z-order がおかしくなることがあるので ::SetWindowPos を使う
					::SetWindowPos( m_hWnd, NULL,0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
				}
			}

			//	Aug, 21, 2000 genta
			m_pcEditDoc->ReloadAutoSaveParam();

			m_pcEditDoc->SetDocumentIcon();	// Sep. 10, 2002 genta 文書アイコンの再設定
			m_pcEditDoc->OnChangeSetting();	// ビューに設定変更を反映させる
			break;
		case PM_CHANGESETTING_FONT:
			// フォント更新
			m_pcViewFont->UpdateFont(&m_pShareData->m_Common.m_sView.m_lf);
			m_pcEditDoc->OnChangeSetting( true );	// フォントで文字幅が変わるので、レイアウト再構築
			break;
		case PM_CHANGESETTING_FONTSIZE:
			// フォント更新
			m_pcViewFont->UpdateFont(&m_pShareData->m_Common.m_sView.m_lf);
			m_pcEditDoc->OnChangeSetting( false );	// ビューに設定変更を反映させる(レイアウト情報の再作成しない)
			break;
		default:
			break;
		}
		return 0L;
	case MYWM_SETACTIVEPANE:
		if( -1 == (int)wParam ){
			if( 0 == lParam ){
				nPane = m_cSplitterWnd.GetFirstPane();
			}else{
				nPane = m_cSplitterWnd.GetLastPane();
			}
			SetActivePane( nPane );
		}
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
				bSelect = GetActiveView().m_bSelectingLock;
			}
			
			ppoCaret = (POINT*)m_pShareData->m_sWorkBuffer.m_szWork;
			//	2006.07.09 genta 強制解除しない
			/*
			カーソル位置変換
			 物理位置(行頭からのバイト数、折り返し無し行位置)
			→
			 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				ppoCaret->x,
				ppoCaret->y,
				&nCaretPosX,
				&nCaretPosY
			);
			// 改行の真ん中にカーソルが来ないように	// 2007.08.22 ryoji
			// Note. もとが改行単位の桁位置なのでレイアウト折り返しの桁位置を超えることはない。
			//       選択指定(bSelect==TRUE)の場合にはどうするのが妥当かよくわからないが、
			//       2007.08.22現在ではアウトライン解析ダイアログから桁位置0で呼び出される
			//       パターンしかないので実用上特に問題は無い。
			if( !bSelect ){
				const CDocLine *pTmpDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( ppoCaret->y );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x ) nCaretPosX--;
				}
			}
			//	2006.07.09 genta 選択範囲を考慮して移動
			//	MoveCursorの位置調整機能があるので，最終行以降への
			//	移動指示の調整もMoveCursorにまかせる
			GetActiveView().MoveCursorSelecting( nCaretPosX, nCaretPosY, bSelect, _CARETMARGINRATE / 3 );
		}
		return 0L;


	case MYWM_GETCARETPOS:	/* カーソル位置取得要求 */
		ppoCaret = (POINT*)m_pShareData->m_sWorkBuffer.m_szWork;
		/*
		カーソル位置変換
		 レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		→
		物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				GetActiveView().m_ptCaretPos.x,
				GetActiveView().m_ptCaretPos.y,
				(int*)&ppoCaret->x,
				(int*)&ppoCaret->y
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	/* 行(改行単位)データの要求 */
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( (int)wParam, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( nLineLen > sizeof( m_pShareData->m_sWorkBuffer.m_szWork ) ){
			memcpy( m_pShareData->m_sWorkBuffer.m_szWork, pLine, sizeof( m_pShareData->m_sWorkBuffer.m_szWork ) );
		}else{
			memcpy( m_pShareData->m_sWorkBuffer.m_szWork, pLine, nLineLen );
		}
		return nLineLen;


	case MYWM_ADDSTRING:
		GetActiveView().HandleCommand( F_ADDTAIL, true, (LPARAM)m_pShareData->m_sWorkBuffer.m_szWork, (LPARAM)lstrlen( m_pShareData->m_sWorkBuffer.m_szWork ), 0, 0 );
		GetActiveView().HandleCommand( F_GOFILEEND, true, 0, 0, 0, 0 );
		return 0L;

	//タブウインドウ	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_cTabWnd.TabWindowNotify( wParam, lParam );
		return 0L;

	//バーの表示・非表示	//@@@ 2003.06.10 MIK
	case MYWM_BAR_CHANGE_NOTIFY:
		if( m_hWnd != (HWND)lParam )
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
					::ShowWindow(m_hWnd, SW_HIDE);
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
			GetActiveView().ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_NCPAINT:
		DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_hwndStatusBar ){
			PrintMenubarMessage( NULL );
		}
		return 0;

	case WM_NCACTIVATE:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（１）	// 2007.04.03 ryoji
		// 前面にいるのが編集ウィンドウならアクティブ状態を保持する
		if( m_pShareData->m_sFlags.m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow()) ){
			wParam = TRUE;	// アクティブ
		}
		lRes = DefWindowProc( hwnd, uMsg, wParam, lParam );
		if( NULL == m_hwndStatusBar ){
			PrintMenubarMessage( NULL );
		}
		return lRes;

	case WM_SETTEXT:
		// 編集ウィンドウ切替中（タブまとめ時）はタイトルバーのアクティブ／非アクティブ状態をできるだけ変更しないように（２）	// 2007.04.03 ryoji
		// タイマーを使用してタイトルの変更を遅延する
		if( m_pShareData->m_sFlags.m_bEditWndChanging ){
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[ ::_tcslen((LPCTSTR)lParam) + 1 ];
			::_tcscpy( m_pszLastCaption, (LPCTSTR)lParam );	// 変更後のタイトルを記憶しておく
			::SetTimer( m_hWnd, IDT_CAPTION, 50, NULL );
			return 0L;
		}
		::KillTimer( m_hWnd, IDT_CAPTION );	// タイマーが残っていたら削除する（遅延タイトルを破棄）
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
int	CEditWnd::OnClose()
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	int nRet = m_pcEditDoc->OnFileClose();
	if( !nRet ) return nRet;

	// 2005.09.01 ryoji タブまとめ表示の場合は次のウィンドウを前面に（終了時のウィンドウちらつきを抑制）
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
		&& !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		int i, j;
		EditNode*	p = NULL;
		int nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, FALSE );
		if( nCount > 1 )
		{
			for( i = 0; i < nCount; i++ )
			{
				if( p[ i ].m_hWnd == m_hWnd )
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
					HWND hwnd = p[ j ].m_hWnd;
					{
						// 2006.01.28 ryoji
						// タブまとめ表示でこの画面が非表示から表示に変わってすぐ閉じる場合(タブの中クリック時等)、
						// 以前のウィンドウが消えるよりも先に一気にここまで処理が進んでしまうと
						// あとで画面がちらつくので、以前のウィンドウが消えるのをちょっとだけ待つ
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






/*! WM_COMMAND処理
	@date 2000.11.15 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行(F_HELP_CONTENTS,F_HELP_SEARCH)を修正・復活
	@date 2013.05.09 novice 重複するメッセージ処理削除
*/
void CEditWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{

	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta マウスから呼びだされた場合
		//ウィンドウ切り替え
		if( wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_sNodes.m_nEditArrNum ){
			ActivateFrameWindow( m_pShareData->m_sNodes.m_pEditArr[wID - IDM_SELWINDOW].m_hWnd );
		}
		//最近使ったファイル
		else if( wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999){
			/* 指定ファイルが開かれているか調べる */
			const CMRUFile cMRU;
			EditInfo checkEditInfo;
			cMRU.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
			m_pcEditDoc->OpenFile( checkEditInfo.m_szPath, checkEditInfo.m_nCharCode);
		}
		//最近使ったフォルダ
		else if( wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999){
			//フォルダ取得
			const CMRUFolder cMRUFolder;
			LPCTSTR pszFolderPath = cMRUFolder.GetPath( wID - IDM_SELOPENFOLDER );

			//Stonee, 2001/12/21 UNCであれば接続を試みる
			NetConnect( pszFolderPath );

			//「ファイルを開く」ダイアログ
			ECodeType nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
			bool bReadOnly = false;
			char		szPath[_MAX_PATH + 3];
			szPath[0] = '\0';
			if( !m_pcEditDoc->OpenFileDialog( m_hWnd, cMRUFolder.GetPath(wID - IDM_SELOPENFOLDER), szPath, &nCharCode, &bReadOnly ) ){
				return;
			}
			//	Oct.  9, 2004 genta 共通関数化
			m_pcEditDoc->OpenFile( szPath, nCharCode, bReadOnly );
		}
		//その他コマンド
		else{
			//ビューにフォーカスを移動しておく
			if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX ) {
				::SetFocus( GetActiveView().m_hWnd );
				//検索ボックスを更新	// 2010/6/6 Uchi
				AcceptSharedSearchKey();
			}

			// コマンドコードによる処理振り分け
			//	May 19, 2006 genta 上位ビットを渡す
			//	Jul. 7, 2007 genta 上位ビットを定数に
			m_pcEditDoc->HandleCommand( wID | 0 );
		}
		break;
	/* アクセラレータからのメッセージ */
	case 1:
		{
			//ビューにフォーカスを移動しておく
			if( wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX )
				::SetFocus( GetActiveView().m_hWnd );

			int nFuncCode = CKeyBind::GetFuncCode(
				wID,
				m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
				m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
			);
			m_pcEditDoc->HandleCommand( nFuncCode | FA_FROMKEYBOARD );
		}
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

			//フォーカスがはずれたときに検索キーにしてしまう。
			//検索キーワードを取得
			char	szText[_MAX_PATH];
			memset( szText, 0, sizeof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//キー文字列がある
			{
				//検索キーを登録
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );
			}

		}
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
	int			nRowNum;
	HMENU		hMenuPopUp;
	HMENU		hMenuPopUp_2;
	const char*	pszLabel;


	if( hMenu == ::GetSubMenu( ::GetMenu( m_hWnd ), uPos )
		&& !fSystemMenu ){
		switch( uPos ){
		case 0:
			/* 「ファイル」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW			, _T(""), _T("N") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("M") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN		, _T(""), _T("O") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVE		, _T(""), _T("S") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEAS_DIALOG	, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL		, _T(""), _T("Z") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			//	Feb. 28, 2003 genta 保存して閉じるを追加．閉じるメニューを近くに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVECLOSE	, _T(""), _T("E") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T(""), _T("C") );	//Feb. 18, 2001	JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE		, _T(""), _T("R") );	//Oct. 17, 2000 jepro キャプションを「閉じる」から変更	//Feb. 18, 2001 JEPRO アクセスキー変更(C→B; Blankの意味)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILECLOSE_OPEN	, _T(""), _T("L") );

			// 「文字コードセット」ポップアップメニュー
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			hMenuPopUp_2 = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	Dec. 4, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN	, _T(""), _T("W") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_SJIS, _T(""), _T("S") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_JIS, _T(""), _T("J") );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_EUC, _T(""), _T("E") );			//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODE, _T(""), _T("U") );	//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UNICODEBE, _T(""), _T("N") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF8, _T(""), _T("8") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp_2, MF_BYPOSITION | MF_STRING, F_FILE_REOPEN_UTF7, _T(""), _T("7") );		//Nov. 7, 2000 jepro キャプションに'で開き直す'を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp_2 , _T("開き直す"), _T("W") );//Oct. 11, 2000 JEPRO アクセスキー変更(M→H)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT				, _T(""), _T("P") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PREVIEW		, _T(""), _T("V") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PRINT_PAGESETUP		, _T(""), _T("U") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			// 「ファイル操作」ポップアップメニュー
			//hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PROPERTY_FILE				, _T(""), _T("T") );		//Nov. 7, 2000 jepro キャプションに'ファイルの'を追加
			//m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "ファイル操作(&R)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BROWSE						, _T(""), _T("B") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			/* MRUリストのファイルのリストをメニューにする */
			{
				//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
				const CMRUFile cMRU;
				hMenuPopUp = cMRU.CreateMenu( &m_CMenuDrawer );	//	ファイルメニュー
				if ( cMRU.Length() > 0 ){
					//	アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "最近使ったファイル", _T("F") );
				}
				else {
					//	非アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT_PTR)hMenuPopUp , "最近使ったファイル", _T("F") );
				}
			}

			/* 最近使ったフォルダのメニューを作成 */
			{
				//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
				const CMRUFolder cMRUFolder;
				hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
				if (cMRUFolder.Length() > 0){
					//	アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "最近使ったフォルダ", _T("D") );
				}
				else {
					//	非アクティブ
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT_PTR)hMenuPopUp , "最近使ったフォルダ", _T("D") );
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE, _T(""), _T("G") );	// グループを閉じる	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T(""), _T("Q") );	//Feb/ 19, 2001 JEPRO 追加	// 2006.10.21 ryoji 表示文字列変更	// 2007.02.13 ryoji →F_EXITALLEDITORS
			//	Jun. 9, 2001 genta ソフトウェア名改称
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL			, _T(""), _T("X") );	//Sept. 11, 2000 jepro キャプションを「アプリケーション終了」から変更	//Dec. 26, 2000 JEPRO F_に変更
			break;

		case 1:
			/* 「編集」メニュー */
			m_CMenuDrawer.ResetContents();
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_UNDO		, _T(""), _T("U") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDO		, _T(""), _T("R") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CUT			, _T(""), _T("T") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY		, _T(""), _T("C") );
			//	Jul, 3, 2000 genta
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "全行引用コピー(&N)" );
			//	Sept. 14, 2000 JEPRO	キャプションに「記号付き」を追加、アクセスキー変更(N→.)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE, "選択範囲内全行引用符付きコピー(&.)" );
//			Sept. 30, 2000 JEPRO	引用符付きコピーのアイコンを作成したので上記メニューは重複を避けて「高度な操作」内におくだけにする
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTE		, _T(""), _T("P") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE		, _T(""), _T("D") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SELECTALL	, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );							/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_RECONVERT	, _T(""), _T("R") );		/* メニューからの再変換対応 minfu 2002.04.09 */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_CRLF	, _T(""), _T("L") );				//Nov. 9, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COPY_ADDCRLF	, _T(""), _T("H") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PASTEBOX	, _T(""), _T("X") );					//Sept. 13, 2000 JEPRO 移動に伴いアクセスキー付与	//Oct. 22, 2000 JEPRO アクセスキー変更(P→X)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DELETE_BACK	, _T(""), _T("B") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			// 「挿入」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_DATE, _T(""), _T("D") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INS_TIME, _T(""), _T("T") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CTRL_CODE_DIALOG, _T(""), _T("C") );	// 2004.05.06 MIK ...追加

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , _T("挿入"), _T("I") );

			// 「高度な操作」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToStart	,	_T(""), _T("L") );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDeleteToEnd	,	_T(""), _T("R") );			//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD			,	_T(""), _T("W") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordCut			,	_T(""), _T("T") );				//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WordDelete			,	_T(""), _T("D") );					//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToStart		,	_T(""), _T("U") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineCutToEnd		,	_T(""), _T("K") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToStart	,	_T(""), _T("H") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LineDeleteToEnd	,	_T(""), _T("E") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CUT_LINE			,	_T(""), _T("X") );	//Jan. 16, 2001 JEPRO 行(頭・末)関係の順序を入れ替えた
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DELETE_LINE		,	_T(""), _T("Y") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DUPLICATELINE		,	_T(""), _T("2") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_TAB			,	_T(""), _T("A") );				//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_TAB		,	_T(""), _T("B") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_INDENT_SPACE		,	_T(""), _T("S") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UNINDENT_SPACE		,	_T(""), _T("P") );			//Oct. 22, 2000 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINES				, _T(""), _T("@") );		//Sept. 14, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESASPASSAGE		, _T(""), _T(".") );//Sept. 13, 2000 JEPRO キャプションから「記号付き」を追加、アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYLINESWITHLINENUMBER, _T(""), _T(":") );//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYFNAME			,	_T(""), _T("-") );	// 2002/2/3 aroka
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYPATH			,	_T(""), _T("\\") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_COPYTAG			,	_T(""), _T("^") );
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_TEST_CREATEKEYBINDLIST	, "キー割り当て一覧をコピー(&Q)" );	//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q)
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WORDSREFERENCE, "単語リファレンス(&W)" );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , _T("高度な操作"), _T("V") );

		//From Here Feb. 19, 2001 JEPRO [移動(M)], [選択(R)]メニューを[編集]のサブメニューとして移動
			// 「移動」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2		, _T(""), _T("Q") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2		, _T(""), _T("K") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT	, _T(""), _T("L") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT	, _T(""), _T("R") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH	, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH	, _T(""), _T("Z") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP	, _T(""), _T("H") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND	, _T(""), _T("E") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp	, _T(""), _T("U") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown	, _T(""), _T("D") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP	, _T(""), _T("T") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND	, _T(""), _T("B") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CURLINECENTER, _T(""), _T("C") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG, _T(""), _T("J") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T(""), _T("I") );	// 検索開始位置へ戻る 02/06/26 ai
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL,_T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_PREV	, _T(""), _T("P") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_NEXT	, _T(""), _T("N") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_JUMPHIST_SET	, _T(""), _T("S") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "移動", _T("O") );

			// 「選択」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTWORD		, _T(""), _T("W") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SELECTALL		, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_SEL		, _T(""), _T("S") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UP2_SEL		, _T(""), _T("Q") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_DOWN2_SEL		, _T(""), _T("K") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDLEFT_SEL	, _T(""), _T("L") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_WORDRIGHT_SEL	, _T(""), _T("R") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOPREVPARAGRAPH_SEL	, _T(""), _T("2") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GONEXTPARAGRAPH_SEL	, _T(""), _T("8") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINETOP_SEL	, _T(""), _T("H") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOLINEEND_SEL	, _T(""), _T("T") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageUp_Sel	, _T(""), _T("U") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_1PageDown_Sel	, _T(""), _T("D") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILETOP_SEL	, _T(""), _T("1") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GOFILEEND_SEL	, _T(""), _T("9") );

			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			//	Mar. 11, 2004 genta 矩形選択メニューを選択メニューに統合
			//	今のところは1つしかないので
			// 「矩形選択」ポップアップメニュー
			// hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BEGIN_BOX	, _T(""), _T("B") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "選択", _T("S") );

//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , "矩形選択(&E)" );

			// 「整形」ポップアップメニュー
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_LTRIM, _T(""), _T("L") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_RTRIM, _T(""), _T("R") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_ASC, _T(""), _T("A") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_SORT_DESC, _T(""), _T("D") );	// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_MERGE, _T(""), _T("U") );			// 2001.12.06 hor
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "整形", _T("K") );

			break;
		//Feb. 19, 2001 JEPRO [移動][移動], [選択]を[編集]配下に移したので削除

//		case 4://case 2: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 2://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「変換」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOLOWER				, _T(""), _T("L") );			//Sept. 10, 2000 jepro キャプションを英語から変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOUPPER				, _T(""), _T("U") );			//Sept. 10, 2000 jepro キャプションを英語から変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
//	From Here Sept. 18, 2000 JEPRO
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, "全角→半角" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, "半角→全角カタカナ" );
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, "半角→全角ひらがな" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKAKU			, _T(""), _T("F") );					//Sept. 13, 2000 JEPRO アクセスキー付与
	// From Here 2007.01.24 maru メニューの並び変更とアクセスキー追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUKATA		, _T(""), _T("Z") );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENKAKUHIRA		, _T(""), _T("N") );	//Sept. 13, 2000 JEPRO キャプション変更 & アクセスキー付与 //Oct. 11, 2000 JEPRO キャプション変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANEI				, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOZENEI				, _T(""), _T("M") );				//July. 29, 2001 Misaka アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TOHANKATA			, _T(""), _T("J") );		//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru メニューの並び変更とアクセスキー追加
//	To Here Sept. 18, 2000
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENKATA, _T(""), _T("K") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HANKATATOZENHIRA, _T(""), _T("H") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TABTOSPACE			, _T(""), _T("S") );	//Feb. 19, 2001 JEPRO 下から移動した
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SPACETOTAB			, _T(""), _T("T") );	//#### Stonee, 2001/05/27
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			//「文字コード変換」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_AUTO2SJIS		, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EMAIL			, _T(""), _T("M") );//Sept. 11, 2000 JEPRO キャプションに「E-Mail」を追加しアクセスキー変更(V→M:Mail)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_EUC2SJIS		, _T(""), _T("W") );		//Sept. 11, 2000 JEPRO アクセスキー変更(E→W:Work Station)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODE2SJIS	, _T(""), _T("U") );	//Sept. 11, 2000 JEPRO アクセスキー変更候補はI:shIft
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UNICODEBE2SJIS	, _T(""), _T("N") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF82SJIS		, _T(""), _T("T") );	//Sept. 11, 2000 JEPRO アクセスキー付与(T:uTF/shifT)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_UTF72SJIS		, _T(""), _T("F") );	//Sept. 11, 2000 JEPRO アクセスキー付与(F:utF/shiFt)	//Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2JIS		, _T(""), _T("J") );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2EUC		, _T(""), _T("E") );		//Sept. 11, 2000 JEPRO アクセスキー付与
//			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UNICODE	, "SJIS→&Unicodeコード変換" );		//Sept. 11, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF8		, _T(""), _T("8") );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CODECNV_SJIS2UTF7		, _T(""), _T("7") );	//Sept. 11, 2000 JEPRO アクセスキー付与 //Oct. 6, 2000 簡潔表示にした
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BASE64DECODE			, _T(""), _T("B") );	//Oct. 6, 2000 JEPRO アクセスキー変更(6→B)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_UUDECODE				, _T(""), _T("D") );			//Sept. 11, 2000 JEPRO アクセスキー変更(U→D)

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp, "文字コード変換", _T("C") );

			break;

//		case 5://case 3: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 3://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「検索」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_DIALOG	, _T(""), _T("F") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_NEXT		, _T(""), _T("N") );				//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_PREV		, _T(""), _T("P") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REPLACE_DIALOG	, _T(""), _T("R") );				//Oct. 7, 2000 JEPRO 下のセクションからここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_SEARCH_CLEARMARK, _T(""), _T("C") );	// "検索マークのクリア(&C)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_SRCHSTARTPOS, _T(""), _T("I") );	// 検索開始位置へ戻る 02/06/26 ai
		
			// Jan. 10, 2005 genta インクリメンタルサーチ
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_NEXT, _T(""), _T("F") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_PREV, _T(""), _T("B") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_NEXT, _T(""), _T("R") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_REGEXP_PREV, _T(""), _T("X") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_NEXT, _T(""), _T("M") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_ISEARCH_MIGEMO_PREV, _T(""), _T("N") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp, _T("インクリメンタルサーチ"), _T("S") );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
// From Here 2001.12.03 hor
			// Jan. 10, 2005 genta 長すぎるのでブックマークをサブメニューに
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_SET	, _T(""), _T("S") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_NEXT	, _T(""), _T("A") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_PREV	, _T(""), _T("Z") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_RESET	, _T(""), _T("X") );
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_BOOKMARK_VIEW	, _T(""), _T("V") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp, _T("ブックマーク"), _T("M") );
// To Here 2001.12.03 hor
			//	Aug. 19. 2003 genta アクセスキー表記を統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG		, _T(""), _T("G") );					//Oct. 7, 2000 JEPRO 下からここに移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_JUMP_DIALOG		, _T(""), _T("J") );	//Sept. 11, 2000 jepro キャプションに「 ジャンプ」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OUTLINE			, _T(""), _T("L") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP			, _T(""), _T("T") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMPBACK		, _T(""), _T("B") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGS_MAKE		, _T(""), _T("") );	//@@@ 2003.04.13 MIK // 2004.05.06 MIK ...追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIRECT_TAGJUMP	, _T(""), _T("") );	//@@@ 2003.04.13 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TAGJUMP_KEYWORD	, _T(""), _T("") ); //@@ 2005.03.31 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPEN_HfromtoC	, _T(""), _T("C") );	//Feb. 7, 2001 JEPRO 追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_COMPARE			, _T(""), _T("@") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_DIALOG		, _T(""), _T("D") );	//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_NEXT		, _T(""), _T("") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_PREV		, _T(""), _T("") );		//@@@ 2002.05.25 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_DIFF_RESET		, _T(""), _T("") );		//@@@ 2002.05.25 MIK
//	From Here Sept. 1, 2000 JEPRO	対括弧の検索をメニューに追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_BRACKETPAIR		, _T(""), _T("[") );
//	To Here Sept. 1, 2000

			break;

//		case 6://case 4: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
		case 4://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
			m_CMenuDrawer.ResetContents();
			/* 「オプション」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			SetMenuFuncSel( hMenu, F_RECKEYMACRO, _T("R"),
				!m_pShareData->m_sFlags.m_bRecordingKeyMacro );

			SetMenuFuncSel( hMenu, F_SAVEKEYMACRO, _T("M"),
				!m_pShareData->m_sFlags.m_bRecordingKeyMacro );

			SetMenuFuncSel( hMenu, F_LOADKEYMACRO, _T("A"),
				!m_pShareData->m_sFlags.m_bRecordingKeyMacro );

			SetMenuFuncSel( hMenu, F_EXECKEYMACRO, _T("D"),
				!m_pShareData->m_sFlags.m_bRecordingKeyMacro );

			//	From Here Sep. 14, 2001 genta
			//「登録済みマクロ」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			
			for( i = 0; i < MAX_CUSTMACRO; ++i ){
				MacroRec *mp = &m_pShareData->m_Common.m_sMacro.m_MacroTable[i];
				if( mp->IsEnabled() ){
					if(  mp->m_szName[0] ){
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szName, _T("") );
					}
					else {
						m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + i, mp->m_szFile, _T("") );
					}
				}
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "登録済みマクロ", _T("B") );
			//	To Here Sep. 14, 2001 genta

			if( m_pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
				::CheckMenuItem( hMenu, F_RECKEYMACRO, MF_BYCOMMAND | MF_CHECKED );
			}
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECEXTMACRO, _T(""), _T("E") ); // 2008.10.22 syat 追加

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			//From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECCMMAND, "外部コマンド実行(&X)" );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXECMD_DIALOG, _T(""), _T("X") );	//Mar. 10, 2001 JEPRO 機能しないのでメニューから隠した	//Mar.21, 2001 JEPRO 標準出力しないで復活 // 2004.05.06 MIK ...追加
			//To Here Sept. 20, 2000

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ACTIVATE_SQLPLUS			, _T(""), _T("P") );	//Sept. 11, 2000 JEPRO アクセスキー付与	説明の「アクティブ化」を「アクティブ表示」に統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PLSQL_COMPILE_ON_SQLPLUS	, _T(""), _T("S") );			//Sept. 11, 2000 JEPRO アクセスキー付与

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HOKAN			, _T(""), _T("/") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			//「カスタムメニュー」ポップアップ
			hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
			//	右クリックメニュー
			if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[0] > 0 ){
				 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
				 	F_MENU_RBUTTON, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], _T("") );
			}
			//	カスタムメニュー
			for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
				if( m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] > 0 ){
					 m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING,
					 	F_CUSTMENU_BASE + i, m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i], _T("") );
				}
			}

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "カスタムメニュー", _T("U") );

//		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

			break;

		case 5://Feb. 28, 2004 genta 「設定」メニュー
			//	設定項目を「ツール」から独立させた
			m_CMenuDrawer.ResetContents();
			/* 「ウィンドウ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			SetMenuFuncSel( hMenu, F_SHOWTOOLBAR, _T("T"),
				!m_pShareData->m_Common.m_sWindow.m_bMenuIcon | !m_hwndToolBar );

			SetMenuFuncSel( hMenu, F_SHOWFUNCKEY, _T("K"),
				!m_pShareData->m_Common.m_sWindow.m_bMenuIcon | !m_CFuncKeyWnd.m_hWnd );

			SetMenuFuncSel( hMenu, F_SHOWTAB, _T("M"),
				!m_pShareData->m_Common.m_sWindow.m_bMenuIcon | !m_cTabWnd.m_hWnd );

			SetMenuFuncSel( hMenu, F_SHOWSTATUSBAR, _T("S"),
				!m_pShareData->m_Common.m_sWindow.m_bMenuIcon | !m_cTabWnd.m_hWnd );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST		, _T(""), _T("L") );	//Sept. 13, 2000 JEPRO 設定より上に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION_TYPE		, _T(""), _T("Y") );		//Sept. 13, 2000 JEPRO アクセスキー変更(S→Y)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION			, _T(""), _T("C") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FONT			, _T(""), _T("F") );		//Sept. 17, 2000 jepro キャプションに「設定」を追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE		, _T(""), _T("O") );	//履歴の管理	//@@@ 2003.04.08 MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );

			// 2008.05.30 nasukoji	テキストの折り返し方法の変更（一時設定）を追加
			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPNOWRAP, _T(""), _T("X") );		// 折り返さない（一時設定）
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPSETTING, _T(""), _T("S") );	// 指定桁で折り返す（一時設定）
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TMPWRAPWINDOW, _T(""), _T("W") );		// 右端で折り返す（一時設定）
			// 折り返し方法に一時設定を適用中
			if( m_pcEditDoc->m_bTextWrapMethodCurTemp )
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "折り返し方法（一時設定適用中）", _T("X") );
			else
				m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "折り返し方法", _T("X") );

//@@@ 2002.01.14 YAZAKI 折り返さないコマンド追加
// 20051022 aroka タイプ別設定値に戻すコマンド追加
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			//	Jan.  8, 2006 genta 共通関数化
			{
				int width;
				CEditView::TOGGLE_WRAP_ACTION mode = GetActiveView().GetWrapMode( width );
				if( mode == CEditView::TGWRAP_NONE ){
					pszLabel = "折り返し桁数";
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , pszLabel, _T("W") );
				}
				else {
					char szBuf[60];
					pszLabel = szBuf;
					if( mode == CEditView::TGWRAP_FULL ){
						sprintf(
							szBuf,
							"折り返し桁数: %d 桁（最大）",
							MAXLINEKETAS
						);
					}
					else if( mode == CEditView::TGWRAP_WINDOW ){
						sprintf(
							szBuf,
							"折り返し桁数: %d 桁（右端）",
							GetActiveView().ViewColNumToWrapColNum(
								GetActiveView().m_nViewColNum
							)
						);
					}
					else {	// TGWRAP_PROP
						sprintf(
							szBuf,
							"折り返し桁数: %d 桁（指定）",
							m_pcEditDoc->GetDocumentAttribute().m_nMaxLineKetas
						);
					}
					m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, _T("W") );
				}
			}
			//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
//			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , "現在のウィンドウ幅で折り返し(&W)" );	//Sept. 13, 2000 JEPRO アクセスキー付与	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			// 2003.06.08 Moca 追加
			// 「モード変更」ポップアップメニュー
			// Feb. 28, 2004 genta 編集メニューから移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CHGMOD_INS	, _T(""), _T("I") );	//Nov. 9, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_READONLY	, _T(""), _T("R") );

			SetMenuFuncSel( hMenu, F_TOGGLE_KEY_SEARCH, _T("H"),
				!m_pShareData->m_Common.m_sWindow.m_bMenuIcon | !IsFuncChecked( m_pcEditDoc, m_pShareData, F_TOGGLE_KEY_SEARCH ) );

			hMenuPopUp = ::CreatePopupMenu();
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, "入力改行コード指定(&CRLF)", _T("") ); // 入力改行コード指定(CRLF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF, "入力改行コード指定(&LF)", _T("") ); // 入力改行コード指定(LF)
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR, "入力改行コード指定(C&R)", _T("") ); // 入力改行コード指定(CR)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , "入力改行コード指定", _T("E") );


			break;

//		case 7://case 5: (Oct. 22, 2000 JEPRO [移動]と[選択]を新設したため番号を2つシフトした)
//		case 5://Feb. 19, 2001 JEPRO [移動]と[選択]を[編集]配下に移動したため番号を元に戻した
		case 6://Feb. 28, 2004 genta 「設定」の新設のため番号をずらした
			m_CMenuDrawer.ResetContents();
			/* 「ウィンドウ」メニュー */
			cMenuItems = ::GetMenuItemCount( hMenu );
			for( i = cMenuItems - 1; i >= 0; i-- ){
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}

			SetMenuFuncSel( hMenu, F_SPLIT_V, _T("-"),
				m_cSplitterWnd.GetAllSplitRows() == 1 );

			SetMenuFuncSel( hMenu, F_SPLIT_H, _T("I"),
				m_cSplitterWnd.GetAllSplitCols() == 1 );

			SetMenuFuncSel( hMenu, F_SPLIT_VH, _T("S"),
				m_cSplitterWnd.GetAllSplitRows() == 1 || m_cSplitterWnd.GetAllSplitCols() == 1 );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINCLOSE		, _T(""), _T("C") );			//Feb. 18, 2001 JEPRO アクセスキー変更(O→C)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_CLOSEALL	, _T(""), _T("Q") );		//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)

			SetMenuFuncSel( hMenu, F_TAB_CLOSEOTHER, _T("O"),
				m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd != 0 );

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_NEXTWINDOW		, _T(""), _T("N") );	//Sept. 11, 2000 JEPRO "次"を"前"の前に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_PREVWINDOW		, _T(""), _T("P") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WINLIST			, _T(""), _T("W") );		// 2006.03.23 fon
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CASCADE			, _T(""), _T("E") );		//Oct. 7, 2000 JEPRO アクセスキー変更(C→E)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_V			, _T(""), _T("H") );	//Sept. 13, 2000 JEPRO 分割に合わせてメニューの左右と上下を入れ替えた //Oct. 7, 2000 JEPRO アクセスキー変更(V→H)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TILE_H			, _T(""), _T("T") );	//Oct. 7, 2000 JEPRO アクセスキー変更(H→T)

			SetMenuFuncSel( hMenu, F_TOPMOST, _T("F"),
				((DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST) == 0 );

			hMenuPopUp = ::CreatePopupMenu();	// 2007.06.20 ryoji

			SetMenuFuncSel( hMenu, F_BIND_WINDOW, _T("B"),
				!m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd || m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin );

			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_GROUPCLOSE		, _T(""), _T("G") );	// 2007.06.20 ryoji
			//2009.12.26 syat 「このタブ以外を閉じる」は「このウィンドウ以外を閉じる」と兼用とし、ウィンドウメニュー直下へ移動。
			//m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSEOTHER	, "このタブ以外を閉じる(&O)" );	// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSELEFT	, _T(""), _T("H") );		// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_CLOSERIGHT	, _T(""), _T("M") );		// 2009.07.20 syat
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_NEXTGROUP		, _T(""), _T("N") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_PREVGROUP		, _T(""), _T("P") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVERIGHT	, _T(""), _T("R") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_MOVELEFT	, _T(""), _T("L") );		// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_SEPARATE	, _T(""), _T("E") );			// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTNEXT	, _T(""), _T("X") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, F_TAB_JOINTPREV	, _T(""), _T("V") );	// 2007.06.20 ryoji
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp, "タブの操作", _T("B") );		// 2007.06.20 ryoji

			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_V		, _T(""), _T("X") );	//Sept. 13, 2000 JEPRO アクセスキー付与
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MAXIMIZE_H		, _T(""), _T("Y") );	//2001.02.10 by MIK
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MINIMIZE_ALL	, _T(""), _T("M") );		//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */				//Oct. 22, 2000 JEPRO 下の「再描画」復活に伴いセパレータを追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_REDRAW			, _T(""), _T("R") );			//Oct. 22, 2000 JEPRO コメントアウトされていたのを復活させた
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );	/* セパレータ */
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_WIN_OUTPUT		, _T(""), _T("U") );		//Sept. 13, 2000 JEPRO アクセスキー変更(O→U)
// 2006.03.23 fon CHG-start>>
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );		/* セパレータ */
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
				::DeleteMenu( hMenu, i, MF_BYPOSITION );
			}
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T(""), _T("O") );				//Sept. 7, 2000 jepro キャプションを「ヘルプ目次」から変更	Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(C→O)
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH	,	 _T(""), _T("S") );	//Sept. 7, 2000 jepro キャプションを「ヘルプトピックの検索」から変更 //Nov. 25, 2000 jepro「トピックの」→「キーワード」に変更 // 2004.05.06 MIK ...追加
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_MENU_ALLFUNC	, _T(""), _T("M") );		//Oct. 13, 2000 JEPRO アクセスキーを「トレイ右ボタン」のために変更(L→M)
//Sept. 16, 2000 JEPRO ショートカットキーがうまく働かないので次行は殺して元に戻してある		//Dec. 25, 2000 復活
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_CREATEKEYBINDLIST	, _T(""), _T("Q") );			//Sept. 15, 2000 JEPRO キャプションの「...リスト」、アクセスキー変更(K→Q) IDM_TEST→Fに変更			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHELP1		, _T(""), _T("E") );		//Sept. 7, 2000 JEPRO このメニューの順番をトップから下に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXTHTMLHELP		, _T(""), _T("H") );	//Sept. 7, 2000 JEPRO このメニューの順番を２番目から下に移動
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T("") );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT			, _T(""), _T("A") );	//Dec. 25, 2000 JEPRO F_に変更
			break;
		}
	}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if (m_pPrintPreview)	return;	//	印刷プレビューモードなら排除。（おそらく排除しなくてもいいと思うんだけど、念のため）

	/* 機能が利用可能かどうか、チェック状態かどうかを一括チェック */
	cMenuItems = ::GetMenuItemCount( hMenu );
	for (nPos = 0; nPos < cMenuItems; nPos++) {
		UINT id = ::GetMenuItemID(hMenu, nPos);
		/* 機能が利用可能か調べる */
		//	Jan.  8, 2006 genta 機能が有効な場合には明示的に再設定しないようにする．
		if( ! IsFuncEnable( m_pcEditDoc, m_pShareData, id ) ){
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		/* 機能がチェック状態か調べる */
		if( IsFuncChecked( m_pcEditDoc, m_pShareData, id ) ){
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



//	フラグにより表示文字列の選択をする。
//		2010/5/19	Uchi
void CEditWnd::SetMenuFuncSel( HMENU hMenu, int nFunc, const TCHAR* sKey, bool flag )
{
	int				i;
	const TCHAR*	sName;

	sName = _T("");
	for (i = 0; i < _countof(sFuncMenuName) ;i++) {
		if (sFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? sFuncMenuName[i].sName[0] : sFuncMenuName[i].sName[1];
		}
	}
	assert( _tcslen(sName) );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey );
}




STDMETHODIMP CEditWnd::DragEnter(  LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
	if( pDataObject == NULL || pdwEffect == NULL ){
		return E_INVALIDARG;
	}

	// 右ボタンファイルドロップの場合だけ処理する
	if( !((MK_RBUTTON & dwKeyState) && IsDataAvailable(pDataObject, CF_HDROP)) ){
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// 印刷プレビューでは受け付けない
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

	// ファイルドロップをアクティブビューで処理する
	*pdwEffect &= DROPEFFECT_LINK;
	return GetActiveView().PostMyDropFiles( pDataObject );
}

/* ファイルがドロップされた */
void CEditWnd::OnDropFiles( HDROP hDrop )
{
	POINT		pt;
	WORD		cFiles, i;
	BOOL		bOpened;
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
		// 2008.07.28 nasukoji	長すぎるパスをドロップされると異常終了することへの対策
		if( ::DragQueryFile( hDrop, i, NULL, 0 ) >= _MAX_PATH ){
			ErrorBeep();
			TopErrorMessage( m_hWnd,
				_T("ファイルパスが長すぎます。 ANSI 版では %d バイト以上の絶対パスを扱えません。"),
				_MAX_PATH );
			continue;
		}

		::DragQueryFile( hDrop, i, szFile, _countof(szFile) );
		ResolvePath(szFile);

		/* 指定ファイルが開かれているか調べる */
		if( CShareData::getInstance()->IsPathOpened( szFile, &hWndOwner ) ){
			::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			/* アクティブにする */
			ActivateFrameWindow( hWndOwner );
			/* MRUリストへの登録 */
			CMRUFile cMRU;
			cMRU.Add( pfi );
		}
		else{
			/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
			//	2005.06.24 Moca
			if( m_pcEditDoc->IsFileOpenInThisWindow() ){
				/* ファイル読み込み */
				BOOL bRet = m_pcEditDoc->FileRead(
					szFile,
					&bOpened,
					CODE_AUTODETECT,	/* 文字コード自動判別 */
					false,				/* 読み取り専用か */
					//	Oct. 03, 2004 genta コード確認は設定に依存
					m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange
				);
				hWndOwner = m_hWnd;
				/* アクティブにする */
				// 2007.06.17 maru すでに開いているかチェック済みだが
				// ドロップされたのはフォルダかもしれないので再チェック
				if(FALSE==bOpened) ActivateFrameWindow( hWndOwner );

				// 2006.09.01 ryoji オープン後自動実行マクロを実行する
				// 2007.06.27 maru すでに編集ウィンドウは開いているのでFileReadがキャンセルされたときは開くマクロを実行する必要なし
				if(TRUE==bRet) m_pcEditDoc->RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );
			}
			else{
				/* ファイルをドロップしたときは閉じて開く */
				if( m_pShareData->m_Common.m_sFile.m_bDropFileAndClose ){
					/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
					if( m_pcEditDoc->OnFileClose() ){
						/* 既存データのクリア */
						m_pcEditDoc->InitDoc();

						/* 全ビューの初期化 */
						m_pcEditDoc->InitAllView();

						/* 親ウィンドウのタイトルを更新 */
						UpdateCaption();

						/* ファイル読み込み */
							m_pcEditDoc->FileRead(
							szFile,
							&bOpened,
							CODE_AUTODETECT,	/* 文字コード自動判別 */
							false,				/* 読み取り専用か */
							//	Oct. 03, 2004 genta コード確認は設定に依存
							m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange
							//true
						);
						hWndOwner = m_hWnd;
						/* アクティブにする */
						ActivateFrameWindow( hWndOwner );

						// 2006.09.01 ryoji オープン後自動実行マクロを実行する
						m_pcEditDoc->RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );
					}
					goto end_of_drop_query;
				}
				else{
					/* 編集ウィンドウの上限チェック */
					if( m_pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
						OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
						::DragFinish( hDrop );
						return;
					}
					char	szFile2[_MAX_PATH + 3];
					if( strchr( szFile, ' ' ) ){
						wsprintf( szFile2, "\"%s\"", szFile );
						strcpy( szFile, szFile2 );
					}
					/* 新たな編集ウィンドウを起動 */
					CControlTray::OpenNewEditor(
						m_hInstance,
						m_hWnd,
						szFile,
						CODE_NONE,
						false				/* 読み取り専用か */
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
	@date 2008.04.19 ryoji IDT_FIRST_IDLE での MYWM_FIRST_IDLE ポスト処理を追加
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
		OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		CShareData::getInstance()->PostMessageToAllEditors( MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL );	// プロセスの初回アイドリング通知	// 2008.04.19 ryoji
		::KillTimer( m_hWnd, wParam );
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
	if( !m_pShareData->m_sFlags.m_bEditWndChanging ){
		::KillTimer( m_hWnd, IDT_CAPTION );
		::SetWindowText( m_hWnd, m_pszLastCaption );
	}
}

/*! システムメニュー表示用タイマーの処理
	@date 2007.04.03 ryoji パラメータ無しにした
	                       以前はコールバック関数でやっていたKillTimer()をここで行うようにした
*/
void CEditWnd::OnSysMenuTimer( void ) //by 鬼(2)
{
	::KillTimer( m_hWnd, IDT_SYSMENU );	// 2007.04.03 ryoji

	if(m_IconClicked == icClicked)
	{
		ReleaseCapture();

		//システムメニュー表示
		// 2006.04.21 ryoji マルチモニタ対応の修正
		// 2007.05.13 ryoji 0x0313メッセージをポストする方式に変更（TrackPopupMenuだとメニュー項目の有効／無効状態が不正になる問題対策）
		RECT R;
		GetWindowRect(m_hWnd, &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect( pt, &R );
		::PostMessage(
			m_hWnd,
			0x0313, //右クリックでシステムメニューを表示する際に送信するモノらしい
			0,
			MAKELPARAM( (pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom )
		);
	}
	m_IconClicked = icNone;
}

/*! タイマーの処理
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2003.08.29 wmlhq, ryoji nTimerCountの導入
	@date 2006.01.28 aroka ツールバー更新を OnToolbarTimerに移動した
	@date 2007.04.03 ryoji パラメータ無しにした
	@date 2012.11.29 aroka 呼び出し間隔のバグ修正
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
		m_pcEditDoc->CheckFileTimeStamp() ;
	}

	m_pcEditDoc->CheckAutoSave();
}

/*! ツールバー更新用タイマーの処理
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2003.08.29 wmlhq, ryoji nTimerCountの導入
	@date 2006.01.28 aroka OnTimerから分離
	@date 2007.04.03 ryoji パラメータ無しにした
	@date 2008.09.23 nasukoji ツールバー更新部分を外に出した
	@date 2012.11.29 aroka OnTimerから分離したときのバグ修正
*/
void CEditWnd::OnToolbarTimer( void )
{
	// 2012.11.29 aroka ここではカウントアップ不要
	//m_nTimerCount++;
	//if( 10 < m_nTimerCount ){
	//	m_nTimerCount = 0;
	//}

	UpdateToolbar();	// 2008.09.23 nasukoji	ツールバーの表示を更新する

	return;
}

/*!
	@brief ツールバーの表示を更新する
	
	@note 他から呼べるようにOnToolbarTimer()より切り出した
	
	@date 2008.09.23 nasukoji
*/
void CEditWnd::UpdateToolbar( void )
{
	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューなら、何もしない。そうでなければ、ツールバーの状態更新 */
	if( !m_pPrintPreview && NULL != m_hwndToolBar ){
		for( int i = 0; i < m_pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
			TBBUTTON tbb = m_CMenuDrawer.getButton(m_pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i]);

			/* 機能が利用可能か調べる */
			::PostMessage(
				m_hwndToolBar, TB_ENABLEBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( (IsFuncEnable( m_pcEditDoc, m_pShareData, tbb.idCommand ) ) , 0 )
			);

			/* 機能がチェック状態か調べる */
			::PostMessage(
				m_hwndToolBar, TB_CHECKBUTTON, tbb.idCommand,
				(LPARAM) MAKELONG( IsFuncChecked( m_pcEditDoc, m_pShareData, tbb.idCommand ), 0 )
			);
		}
	}
}

//検索ボックスを更新
void CEditWnd::AcceptSharedSearchKey()
{
	if( m_hwndSearchBox )
	{
		int	i;
		::SendMessage( m_hwndSearchBox, CB_RESETCONTENT, 0, 0 );
		for( i = 0; i < m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum; i++ )
		{
			::SendMessage( m_hwndSearchBox, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i] );
		}
		::SendMessage( m_hwndSearchBox, CB_SETCURSEL, 0, 0 );
	}
}

/* デバッグモニタモードに設定 */
void CEditWnd::SetDebugModeON()
{
	if( NULL != m_pShareData->m_sHandles.m_hwndDebug ){
		if( IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug ) ){
			return;
		}
	}
	m_pShareData->m_sHandles.m_hwndDebug = m_hWnd;
	m_pcEditDoc->m_bDebugMode = false;

// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAILが抑止されるのでとりあえず読み取り専用モードは辞めました
	m_pcEditDoc->m_bReadOnly = false;		/* 読み取り専用モード */
	/* 親ウィンドウのタイトルを更新 */
	UpdateCaption();
}

// 2005.06.24 Moca
//! デバックモニタモードの解除
void CEditWnd::SetDebugModeOFF()
{
	if( m_pShareData->m_sHandles.m_hwndDebug == m_hWnd ){
		m_pShareData->m_sHandles.m_hwndDebug = NULL;
		m_pcEditDoc->m_bDebugMode = false;
		UpdateCaption();
	}
}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更

/* 印刷プレビューモードのオン/オフ */
void CEditWnd::PrintPreviewModeONOFF( void )
{
	HMENU	hMenu;
	HWND	hwndToolBar;

	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
	hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;

	/* 印刷プレビューモードか */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	if( m_pPrintPreview ){
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	印刷プレビューモードを解除します。	*/
		delete m_pPrintPreview;	//	削除。
		m_pPrintPreview = NULL;	//	NULLか否かで、プリントプレビューモードか判断するため。

		/*	通常モードに戻す	*/
		::ShowWindow( m_cSplitterWnd.m_hWnd, SW_SHOW );
		::ShowWindow( hwndToolBar, SW_SHOW );	// 2006.06.17 ryoji
		::ShowWindow( m_hwndStatusBar, SW_SHOW );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_SHOW );
		::ShowWindow( m_cTabWnd.m_hWnd, SW_SHOW );	//@@@ 2003.06.25 MIK

		::SetFocus( m_hWnd );

		hMenu = ::LoadMenu( m_hInstance, MAKEINTRESOURCE( IDR_MENU1 ) );
		::SetMenu( m_hWnd, hMenu );
		::DrawMenuBar( m_hWnd );

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		/*	通常モードを隠す	*/
		hMenu = ::GetMenu( m_hWnd );
		//	Jun. 18, 2001 genta Print Previewではメニューを削除
		::SetMenu( m_hWnd, NULL );
		::DestroyMenu( hMenu );
		::DrawMenuBar( m_hWnd );

		::ShowWindow( m_cSplitterWnd.m_hWnd, SW_HIDE );
		::ShowWindow( hwndToolBar, SW_HIDE );	// 2006.06.17 ryoji
		::ShowWindow( m_hwndStatusBar, SW_HIDE );
		::ShowWindow( m_CFuncKeyWnd.m_hWnd, SW_HIDE );
		::ShowWindow( m_cTabWnd.m_hWnd, SW_HIDE );	//@@@ 2003.06.25 MIK

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview = new CPrintPreview( this );
		/* 現在の印刷設定 */
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_PrintSettingArr[
				m_pcEditDoc->GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		//	プリンタの情報を取得。

		/* 現在のデフォルトプリンタの情報を取得 */
		BOOL bRes;
		bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if( !bRes ){
			TopInfoMessage( m_hWnd, _T("印刷プレビューを実行する前に、プリンタをインストールしてください。\n") );
			return;
		}

		/* 印刷設定の反映 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect( m_hWnd, NULL, TRUE );
		::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

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
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる
//	変数削除

	RECT		rcWin;



	cx = LOWORD( lParam );
	cy = HIWORD( lParam );

	/* ウィンドウサイズ継承 */
	if( wParam != SIZE_MINIMIZED ){						/* 最小化は継承しない */
		//	2004.05.13 Moca m_eSaveWindowSizeの解釈追加のため
		if( WINSIZEMODE_SAVE == m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize ){		/* ウィンドウサイズ継承をするか */
			if( wParam == SIZE_MAXIMIZED ){					/* 最大化はサイズを記録しない */
				if( m_pShareData->m_Common.m_sWindow.m_nWinSizeType != (int)wParam ){
					m_pShareData->m_Common.m_sWindow.m_nWinSizeType = wParam;
				}
			}else{
				::GetWindowRect( m_hWnd, &rcWin );
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
		EditNode *p = CShareData::getInstance()->GetEditNode( m_hWnd );
		if( p != NULL ){
			p->m_showCmdRestore = ::IsZoomed( p->m_hWnd )? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	/* サイズ変更のタイプ */

	// 2006.06.17 ryoji Rebar があればそれをツールバー扱いする
	hwndToolBar = (NULL != m_hwndReBar)? m_hwndReBar: m_hwndToolBar;
	nToolBarHeight = 0;
	if( NULL != hwndToolBar ){
		::SendMessage( hwndToolBar, WM_SIZE, wParam, lParam );
		::GetWindowRect( hwndToolBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	nFuncKeyWndHeight = 0;
	if( NULL != m_CFuncKeyWnd.m_hWnd ){
		::SendMessage( m_CFuncKeyWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_CFuncKeyWnd.m_hWnd, &rc );
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//タブウインドウ
	nTabWndHeight = 0;
	if( NULL != m_cTabWnd.m_hWnd )
	{
		::SendMessage( m_cTabWnd.m_hWnd, WM_SIZE, wParam, lParam );
		::GetWindowRect( m_cTabWnd.m_hWnd, &rc );
		nTabWndHeight = rc.bottom - rc.top;
	}
	//@@@ To Here 2003.05.31 MIK
	nStatusBarHeight = 0;
	if( NULL != m_hwndStatusBar ){
		::SendMessage( m_hwndStatusBar, WM_SIZE, wParam, lParam );
		::GetClientRect( m_hwndStatusBar, &rc );
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
		hdc = ::GetDC( m_hwndStatusBar );
		HFONT hFont = (HFONT)::SendMessage(m_hwndStatusBar, WM_GETFONT, 0, 0);
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
		::SendMessage( m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)_T(""));

		::SendMessage( m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
		if (hFont != NULL)
		{
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC( m_hwndStatusBar, hdc );

		::UpdateWindow( m_hwndStatusBar );	// 2006.06.17 ryoji 即時描画でちらつきを減らす
		::GetWindowRect( m_hwndStatusBar, &rc );
		nStatusBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_hWnd, &rcClient );

	//@@@ From 2003.05.31 MIK
	//タブウインドウ追加に伴い，ファンクションキー表示位置も調整

	//タブウインドウ
	if( m_cTabWnd.m_hWnd )
	{
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
		{
			::MoveWindow( m_cTabWnd.m_hWnd, 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
		}
		else
		{
			::MoveWindow( m_cTabWnd.m_hWnd, 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
		}
	}

	//	2005.04.23 genta ファンクションキー非表示の時は移動しない
	if( m_CFuncKeyWnd.m_hWnd != NULL ){
		if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
		{	/* ファンクションキー表示位置／0:上 1:下 */
			::MoveWindow(
				m_CFuncKeyWnd.m_hWnd,
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE );
		}
		else if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 1 )
		{	/* ファンクションキー表示位置／0:上 1:下 */
			::MoveWindow(
				m_CFuncKeyWnd.m_hWnd,
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool	bSizeBox = true;
			if( NULL != m_hwndStatusBar ){
				bSizeBox = false;
			}
			if( wParam == SIZE_MAXIMIZED ){
				bSizeBox = false;
			}
			m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
		}
		::UpdateWindow( m_CFuncKeyWnd.m_hWnd );	// 2006.06.17 ryoji 即時描画でちらつきを減らす
	}

	if( m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 )
	{
		::MoveWindow(
			m_cSplitterWnd.m_hWnd,
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
			m_cSplitterWnd.m_hWnd,
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
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	/* 印刷プレビューモードか */
	if( !m_pPrintPreview ){
		PAINTSTRUCT		ps;
		::BeginPaint( hwnd, &ps );
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
	SetCapture( m_hWnd );

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
			SetTimer(m_hWnd, IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	m_bDragMode = false;
	ReleaseCapture();
	::InvalidateRect( m_hWnd, NULL, TRUE );
	return 0;
}


/*!	WM_MOUSEMOVE処理
	@date 2008.05.05 novice メモリリーク修正
*/
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
			if(SendMessage(m_hWnd, WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU)
			{
				ReleaseCapture();
				m_IconClicked = icNone;

				if(m_pcEditDoc->IsValidPath())
				{
					const char *PathEnd = m_pcEditDoc->GetFilePath();
					for(const char* I = m_pcEditDoc->GetFilePath(); *I != 0; ++I)
					{
						//by 鬼(2): DBCS処理
						if(IsDBCSLeadByte(*I))
							++I;
						else if(*I == '\\')
							PathEnd = I;
					}

					wchar_t WPath[MAX_PATH];
					int c = MultiByteToWideChar(CP_ACP, 0, m_pcEditDoc->GetFilePath(), PathEnd - m_pcEditDoc->GetFilePath(), WPath, MAX_PATH);
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
							//テキストでも持たせる…便利
							{
								FORMATETC F;
								F.cfFormat = CF_TEXT;
								F.ptd      = NULL;
								F.dwAspect = DVASPECT_CONTENT;
								F.lindex   = -1;
								F.tymed    = TYMED_HGLOBAL;

								STGMEDIUM M;
								const TCHAR* pFilePath = m_pcEditDoc->GetFilePath();
								int Len = _tcslen(pFilePath);
								M.tymed          = TYMED_HGLOBAL;
								M.pUnkForRelease = NULL;
								M.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len+1)*sizeof(TCHAR));
								void* p = GlobalLock(M.hGlobal);
								CopyMemory(p, pFilePath, (Len+1)*sizeof(TCHAR));
								GlobalUnlock(M.hGlobal);

								DataObject->SetData(&F, &M, TRUE);
							}
#endif
							//移動は禁止
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
	return Views_DispatchEvent( m_hWnd, WM_MOUSEWHEEL, wParam, lParam );
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
			if( (hwnd == m_cTabWnd.m_hwndTab || hwnd == m_cTabWnd.m_hWnd) )
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
						if( m_hWnd == pEditNodeArr[i].m_hWnd )
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
							ActivateFrameWindow( pEditNodeArr[j].m_hWnd );
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

//	cDlgPrintSetting.Create( m_hInstance, m_hWnd );
	nCurrentPrintSetting = m_pcEditDoc->GetDocumentAttribute().m_nCurrentPrintSetting;
	bRes = cDlgPrintSetting.DoModal(
		m_hInstance,
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		m_hWnd,
		&nCurrentPrintSetting, /* 現在選択している印刷設定 */
		PrintSettingArr
	);

	if( TRUE == bRes ){
		/* 現在選択されているページ設定の番号が変更されたか */
		if( nCurrentPrintSetting !=
			m_pShareData->m_Types[m_pcEditDoc->GetDocumentType()].m_nCurrentPrintSetting
		){
//			/* 変更フラグ(タイプ別設定) */
			m_pcEditDoc->GetDocumentAttribute().m_nCurrentPrintSetting = nCurrentPrintSetting;
		}

		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = PrintSettingArr[i];
		}

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
		//	印刷プレビュー時のみ。
		if ( m_pPrintPreview ){
			/* 現在の印刷設定 */
			m_pPrintPreview->SetPrintSetting( &m_pShareData->m_PrintSettingArr[m_pcEditDoc->GetDocumentAttribute().m_nCurrentPrintSetting] );

			/* 印刷プレビュー スクロールバー初期化 */
			m_pPrintPreview->InitPreviewScrollBar();

			/* 印刷設定の反映 */
			m_pPrintPreview->OnChangePrintSetting( );

			::InvalidateRect( m_hWnd, NULL, TRUE );
		}
	}
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	::UpdateWindow( m_hWnd /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by 鬼

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
		Result = DefWindowProc(m_hWnd, WM_NCLBUTTONUP, wp, lp);
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

		SendMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}
	else {
		//	2004.05.23 Moca メッセージミス修正
		Result = DefWindowProc(m_hWnd, WM_LBUTTONDBLCLK, wp, lp);
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
	nIndex = ::SendMessage( hwnd, TB_HITTEST, (WPARAM)0, (LPARAM)&po );
	if( nIndex < 0 ){
		return 0;
	}
	::SendMessage( hwnd, TB_GETITEMRECT, (WPARAM)nIndex, (LPARAM)&rc );
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
	const CMRUFile cMRU;
	hMenu = cMRU.CreateMenu( &m_CMenuDrawer );
	if( cMRU.MenuLength() > 0 )
	{
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );
	}

	/* 最近使ったフォルダのメニューを作成 */
	const CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_CMenuDrawer );
	if ( cMRUFolder.MenuLength() > 0 )
	{
		//	アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp, _T("最近使ったフォルダ"), _T("D") );
	}
	else 
	{
		//	非アクティブ
		m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED, (UINT_PTR)hMenuPopUp, _T("最近使ったフォルダ"), _T("D") );
	}

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, _T(""), FALSE );

	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("M"), FALSE );
	m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T(""), FALSE );

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
		m_hWnd,	// 2009.02.03 ryoji アクセスキー有効化のため hwnd -> m_hWnd に変更
		NULL
	);

	::DestroyMenu( hMenu );

	return nId;
}

/*! 検索ボックスでの処理 */
void CEditWnd::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( (TCHAR)msg->wParam == VK_RETURN )  //リターンキー
		{
			//検索キーワードを取得
			char	szText[_MAX_PATH];
			memset( szText, 0, _countof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//キー文字列がある
			{
				//検索キーを登録
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)szText );

				//検索ボックスを更新	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus( m_hWnd );	//先にフォーカスを移動しておかないとキャレットが消える
				::SetFocus( GetActiveView().m_hWnd );

				// 検索開始時のカーソル位置登録条件を変更 02/07/28 ai start
				GetActiveView().m_ptSrchStartPos_PHY = GetActiveView().m_ptCaretPos_PHY;
				// 02/07/28 ai end

				//次を検索
				OnCommand( (WORD)0 /*メニュー*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( (TCHAR)msg->wParam == VK_TAB )	//タブキー
		{
			//フォーカスを移動
			//	2004.10.27 MIK IME表示位置のずれ修正
			::SetFocus( m_hWnd  );
		}
	}
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
	HICON hOld = (HICON)::SendMessage( m_hWnd, WM_SETICON, flag, (LPARAM)hIcon );
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
	*hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	*hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
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
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	/* LOGFONTの初期化 */
	memset( &lf, 0, sizeof( lf ) );
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
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
	for( i = 0; i < ( sizeof( m_pnCaretPosInfoDx ) / sizeof( m_pnCaretPosInfoDx[0] ) ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
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
void CEditWnd::PrintMenubarMessage( const TCHAR* msg )
{
	if( NULL == ::GetMenu( m_hWnd ) )	// 2007.03.08 ryoji 追加
		return;

	POINT	po,poFrame;
	RECT	rc,rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL のときは以前の m_pszMenubarMessage で再描画
	if( msg ){
		int len = _tcslen( msg );
		_tcsncpy( m_pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN );
		if( len < MENUBAR_MESSAGE_MAX_LEN ){
			memset( m_pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len );	//  null終端は不要
		}
	}

	HDC		hdc;
	hdc = ::GetWindowDC( m_hWnd );
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen( m_hWnd, &poFrame );
	::GetWindowRect( m_hWnd, &rcFrame );
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - ( ::GetSystemMetrics( SM_CXSIZEFRAME ) + 2 );
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
	//	Sep. 6, 2003 genta Windows XP(Luna)の場合にはCOLOR_MENUBARを使わなくてはならない
	COLORREF bkColor =
		::GetSysColor( IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU );
	::SetBkColor( hdc, bkColor );
	::ExtTextOut( hdc,rc.left,rc.top,ETO_OPAQUE,&rc,m_pszMenubarMessage,nStrLen,m_pnCaretPosInfoDx);
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
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
	if( NULL == m_hwndStatusBar ){
		// メニューバーへ
		PrintMenubarMessage( msg );
	}
	else{
		// ステータスバーへ
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}

/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	メニューバー右端に入らないものや，桁位置表示を隠したくないものに使う
	
	呼び出し前にSendStatusMessage2IsEffective()で処理の有無を
	確認することで無駄な処理を省くことが出来る．

	@param msg [in] 表示するメッセージ
	@date 2005.07.09 genta 新規作成
	
	@sa SendStatusMessage2IsEffective
*/
void CEditWnd::SendStatusMessage2( const char* msg )
{
	if( NULL != m_hwndStatusBar ){
		// ステータスバーへ
		::SendMessage( m_hwndStatusBar,SB_SETTEXT,0 | SBT_NOBORDERS,(LPARAM) (LPINT)msg );
	}
}

/*! ファイル名変更通知

	@author MIK
	@date 2003.05.31 新規作成
	@date 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
*/
void CEditWnd::ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep )
{
	CRecentEditNode	cRecentEditNode;
	EditNode	*p;
	int		nIndex;

	if( NULL == pszTabCaption ) pszTabCaption = _T("");	//ガード
	if( NULL == pszFilePath )pszFilePath = "";		//ガード 2006.01.28 ryoji

	nIndex = cRecentEditNode.FindItem( (const char*)&m_hWnd );
	if( -1 != nIndex )
	{
		p = (EditNode*)cRecentEditNode.GetItem( nIndex );
		if( p )
		{
			int	size = _countof( p->m_szTabCaption ) - 1;
			_tcsncpy( p->m_szTabCaption, pszTabCaption, size );
			p->m_szTabCaption[ size ] = _T('\0');

			// 2006.01.28 ryoji ファイル名、Grepモード追加
			size = sizeof( p->m_szFilePath ) - 1;
			_tcsncpy( p->m_szFilePath, pszFilePath, size );
			p->m_szFilePath[ size ] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	cRecentEditNode.Terminate();

	//ファイル名変更通知をブロードキャストする。
	int nGroup = CShareData::getInstance()->GetGroupId( m_hWnd );
	CShareData::getInstance()->PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TWNT_FILE,
		(LPARAM)m_hWnd,
		m_hWnd,
		nGroup
	);

	return;
}
/*!	@brief ToolBarのOwnerDraw

	@param pnmh [in] Owner Draw情報

	@note Common Control V4.71以降はNMTBCUSTOMDRAWを送ってくるが，
	Common Control V4.70はLPNMCUSTOMDRAWしか送ってこないので
	安全のため小さい方に合わせて処理を行う．
	
	@author genta
	@date 2003.07.21 作成

*/
LPARAM CEditWnd::ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh )
{
	switch( pnmh->dwDrawStage ){
	case CDDS_PREPAINT:
		//	描画開始前
		//	アイテムを自前で描画する旨を通知する
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		//	面倒くさいので，枠はToolbarに描いてもらう
		//	アイコンが登録されていないので中身は何も描かれない
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			//	描画
			// コマンド番号（pnmh->dwItemSpec）からアイコン番号を取得する	// 2007.11.02 ryoji
			int nIconId = ::SendMessage( pnmh->hdr.hwndFrom, TB_GETBITMAP, (WPARAM)pnmh->dwItemSpec, 0 );

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - m_pcIcons->cy()) / 2;		// アイテム矩形からの画像のオフセット	// 2007.03.25 ryoji
			int shift = pnmh->uItemState & ( CDIS_SELECTED | CDIS_CHECKED ) ? 1 : 0;	//	Aug. 30, 2003 genta ボタンを押されたらちょっと画像をずらす
			int color = pnmh->uItemState & CDIS_CHECKED ? COLOR_3DHILIGHT : COLOR_3DFACE;

			//	Sep. 6, 2003 genta 押下時は右だけでなく下にもずらす
			m_pcIcons->Draw( nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED ) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}

/*! 常に手前に表示
	@param top  0:トグル動作 1:最前面 2:最前面解除 その他:なにもしない
	@date 2004.09.21 Moca
*/
void CEditWnd::WindowTopMost( int top )
{
	if( 0 == top ){
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr( m_hWnd, GWL_EXSTYLE );
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

	::SetWindowPos( m_hWnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	// タブまとめ時は WS_EX_TOPMOST 状態を全ウィンドウで同期する	// 2007.05.18 ryoji
	if( m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		HWND hwnd;
		int i;
		for( i = 0, hwndInsertAfter = m_hWnd; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ ){
			hwnd = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
			if( hwnd != m_hWnd && IsSakuraMainWindow( hwnd ) ){
				if( !CShareData::getInstance()->IsSameGroup( m_hWnd, hwnd ) )
					continue;
				::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
				hwndInsertAfter = hwnd;
			}
		}
	}
}

/*!
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void CEditWnd::SetFocusSearchBox( void ) const
{
	if( m_hwndSearchBox ){
		::SetFocus(m_hwndSearchBox);
	}
}


// タイマーの更新を開始／停止する。 20060128 aroka
// ツールバー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときにウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void CEditWnd::Timer_ONOFF( bool bStart )
{
	if( NULL != m_hWnd ){
		if( bStart ){
			/* タイマーを起動 */
			if( 0 == ::SetTimer( m_hWnd, IDT_TOOLBAR, 300, NULL ) ){
				WarningMessage( m_hWnd, _T("CEditWnd::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。") );
			}
		} else {
			/* タCマーを削除 */
			::KillTimer( m_hWnd, IDT_TOOLBAR );
		}
	}
	return;
}

/*!	@brief ウィンドウ一覧をポップアップ表示

	@param[in] bMousePos true: マウス位置にポップアップ表示する

	@date 2006.03.23 fon OnListBtnClickをベースに新規作成
	@date 2006.05.10 ryoji ポップアップ位置変更、その他微修正
	@date 2007.02.28 ryoji フルパス指定のパラメータを削除
	@date 2009.06.02 ryoji m_CMenuDrawerの初期化漏れ修正
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
		::GetWindowRect( GetActiveView().m_hWnd, &rc );
		pt.x = rc.right - 150;
		if( pt.x < rc.left )
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// ウィンドウ一覧メニューをポップアップ表示する
	if( NULL != m_cTabWnd.m_hWnd ){
		m_cTabWnd.TabListMenu( pt );
	}
	else{
		m_CMenuDrawer.ResetContents();	// 2009.06.02 ryoji 追加
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
									0, m_hWnd, NULL);
		delete [] pEditNodeArr;
		::DestroyMenu( hMenu );
		::SendMessage( m_hWnd, WM_COMMAND, (WPARAM)nId, (LPARAM)NULL );
	}

	return 0L;
}

/*! @brief 現在開いている編集窓のリストをメニューにする 
	@date  2006.03.23 fon CEditWnd::InitMenuから移動。////が元からあるコメント。//>は追加コメントアウト。
	@date 2009.06.02 ryoji アイテム数が多いときはアクセスキーを 1-9,A-Z の範囲で再使用する（従来は36個未満を仮定）
*/
LRESULT CEditWnd::WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull )
{
	int			i;
	TCHAR		szMenu[_MAX_PATH * 2 + 3];
	const EditInfo*	pfi;

	if( nRowNum > 0 ){
		CShareData::getInstance()->TransformFileName_MakeCache();
		for( i = 0; i < nRowNum; ++i ){
			/* トレイからエディタへの編集ファイル名要求通知 */
			::SendMessage( pEditNodeArr[i].m_hWnd, MYWM_GETFILEINFO, 0, 0 );
////	From Here Oct. 4, 2000 JEPRO commented out & modified	開いているファイル数がわかるように履歴とは違って1から数える
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			CShareData::getInstance()->GetMenuFullLabel_WinList( szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, i );
			m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMenu, _T("") );
			if( m_hWnd == pEditNodeArr[i].m_hWnd ){
				::CheckMenuItem( hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED );
			}
		}
	}
	return 0L;
}


/*!
	CEditViewの画面バッファを削除
	@date 2007.09.09 Moca 新規作成
*/
void CEditWnd::Views_DeleteCompatibleBitmap()
{
	// CEditView群へ転送する
	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( m_pcEditViewArr[i]->m_hWnd ){
			m_pcEditViewArr[i]->DeleteCompatibleBitmap();
		}
	}
}

LRESULT CEditWnd::Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		{
			int i;
			for( i = 0; i < GetAllViewCount(); i++ ){
				m_pcEditViewArr[i]->DispatchEvent( hwnd, msg, wParam, lParam );
			}
		}
		return 0L;
	default:
		return GetActiveView().DispatchEvent( hwnd, msg, wParam, lParam );
	}
}

/*
	分割指示。2つ目以降のビューを作る
	@param nViewCount  既存のビューも含めたビューの合計要求数
*/
bool CEditWnd::CreateEditViewBySplit(int nViewCount )
{
	if( m_nEditViewMaxCount < nViewCount ){
		return false;
	}
	if( GetAllViewCount() < nViewCount ){
		int i;
		for( i = GetAllViewCount(); i < nViewCount; i++ ){
			assert( NULL == m_pcEditViewArr[i] );
			m_pcEditViewArr[i] = new CEditView( this );
			m_pcEditViewArr[i]->Create( m_hInstance, m_cSplitterWnd.m_hWnd, m_pcEditDoc, i, FALSE );
		}
		m_nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for( i = 0; i < nViewCount; i++ ){
			hWndArr.push_back( m_pcEditViewArr[i]->m_hWnd );
		}
		hWndArr.push_back( NULL );

		m_cSplitterWnd.SetChildWndArr( &hWndArr[0] );
	}
	return true;
}

/*
	ビューの再初期化
	@date 2010.04.10 CEditDoc::InitAllViewから移動
*/
void CEditWnd::InitAllViews()
{
	int i;

	/* 先頭へカーソルを移動 */
	for( i = 0; i < GetAllViewCount(); ++i ){
		//	Apr. 1, 2001 genta
		// 移動履歴の消去
		m_pcEditViewArr[i]->m_cHistory->Flush();

		/* 現在の選択範囲を非選択状態に戻す */
		m_pcEditViewArr[i]->DisableSelectArea( false );

		m_pcEditViewArr[i]->OnChangeSetting();
		m_pcEditViewArr[i]->MoveCursor( 0, 0, true );
		m_pcEditViewArr[i]->m_nCaretPosX_Prev = 0;
	}
}

void CEditWnd::Views_Redraw()
{
	//アクティブ以外を再描画してから…
	for( int v = 0; v < GetAllViewCount(); ++v ){
		if( m_nActivePaneIndex != v ){
			m_pcEditViewArr[v]->Redraw();
		}
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
	m_pcEditView = m_pcEditViewArr[m_nActivePaneIndex];

	// フォーカスを移動する	// 2007.10.16 ryoji
	m_pcEditViewArr[nOldIndex]->m_cUnderLine.CaretUnderLineOFF( true );	//	2002/05/11 YAZAKI
	if( ::GetActiveWindow() == m_hWnd
		&& ::GetFocus() != m_pcEditViewArr[m_nActivePaneIndex]->m_hWnd )
	{
		// ::SetFocus()でフォーカスを切り替える
		::SetFocus( m_pcEditViewArr[m_nActivePaneIndex]->m_hWnd );
	}else{
		// 2010.04.08 ryoji
		// 起動直後にエディットボックスにフォーカスのあるダイアログを表示する場合にキャレットが消える問題の修正
		// （例: -GREPDLGオプションで起動するとGREPダイアログのキャレットが消えている）
		// この問題を修正するのため、内部的な切り替え動作をするのはアクティブペインが替わるときだけにした．
		if( m_nActivePaneIndex != nOldIndex ){
			// アクティブでないときに::SetFocus()するとアクティブになってしまう
			// （不可視なら可視になる）ので内部的に切り替えるだけにする
			m_pcEditViewArr[nOldIndex]->OnKillFocus();
			m_pcEditViewArr[m_nActivePaneIndex]->OnSetFocus();
		}
	}

	GetActiveView().RedrawAll();	/* フォーカス移動時の再描画 */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_pcEditDoc->m_cDlgFind.m_hWnd ){		/* 「検索」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_pcEditDoc->m_cDlgFind.ChangeView( (LPARAM)&GetActiveView() );
	}
	if( NULL != m_pcEditDoc->m_cDlgReplace.m_hWnd ){	/* 「置換」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_pcEditDoc->m_cDlgReplace.ChangeView( (LPARAM)&GetActiveView() );
	}
	if( NULL != m_pcEditDoc->m_cHokanMgr.m_hWnd ){	/* 「入力補完」ダイアログ */
		m_pcEditDoc->m_cHokanMgr.Hide();
		/* モードレス時：検索対象となるビューの変更 */
		m_pcEditDoc->m_cHokanMgr.ChangeView( (LPARAM)&GetActiveView() );
	}
	if( NULL != m_pcEditDoc->m_cDlgFuncList.m_hWnd ){	/* 「アウトライン」ダイアログ */ // 20060201 aroka
		/* モードレス時：現在位置表示の対象となるビューの変更 */
		m_pcEditDoc->m_cDlgFuncList.ChangeView( (LPARAM)&GetActiveView() );
	}

	return;
}

/** すべてのペインの描画スイッチを設定する

	@param bDraw [in] 描画スイッチの設定値

	@date 2008.06.08 ryoji 新規作成
*/
void CEditWnd::SetDrawSwitchOfAllViews( bool bDraw )
{
	int i;
	CEditView* pView;

	for( i = 0; i < GetAllViewCount(); i++ ){
		pView = m_pcEditViewArr[i];
		pView->m_bDrawSWITCH = bDraw;
	}
}


/** すべてのペインをRedrawする

	スクロールバーの状態更新はパラメータでフラグ制御 or 別関数にしたほうがいい？

	@param pcViewExclude [in] Redrawから除外するビュー

	@date 2007.07.22 ryoji スクロールバーの状態更新を追加
	@date 2008.06.08 ryoji pcViewExclude パラメータ追加
*/
void CEditWnd::RedrawAllViews( CEditView* pcViewExclude )
{
	int i;
	CEditView* pcView;

	for( i = 0; i < GetAllViewCount(); i++ ){
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


/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
BOOL CEditWnd::DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw )
{
	if( 1 == GetAllViewCount() ){
		return m_pcEditViewArr[m_nActivePaneIndex]->DetectWidthOfLineNumberArea( bRedraw );
	}
	// 以下2,4分割限定

	if ( m_pcEditViewArr[m_nActivePaneIndex]->DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePaneで計算したら、再設定・再描画が必要と判明した */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			m_pcEditViewArr[m_nActivePaneIndex^1]->DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	表示されていないので再描画しない
			m_pcEditViewArr[m_nActivePaneIndex^1]->DetectWidthOfLineNumberArea( FALSE );
		}
		if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
			m_pcEditViewArr[m_nActivePaneIndex^2]->DetectWidthOfLineNumberArea( bRedraw );
			if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
				m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->DetectWidthOfLineNumberArea( bRedraw );
			}
		}
		else {
			m_pcEditViewArr[m_nActivePaneIndex^2]->DetectWidthOfLineNumberArea( FALSE );
			m_pcEditViewArr[(m_nActivePaneIndex^1)^2]->DetectWidthOfLineNumberArea( FALSE );
		}
		return TRUE;
	}
	return FALSE;
}



/** 右端で折り返す
	@param nViewColNum	[in] 右端で折り返すペインの番号
	@retval 折り返しを変更したかどうか
	@date 2008.06.08 ryoji 新規作成
*/
BOOL CEditWnd::WrapWindowWidth( int nPane )
{
	// 右端で折り返す
	int nWidth = m_pcEditViewArr[nPane]->ViewColNumToWrapColNum( m_pcEditViewArr[nPane]->m_nViewColNum );
	if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() != nWidth ){
		ChangeLayoutParam( false, m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), nWidth );
		return TRUE;
	}
	return FALSE;
}

/** 折り返し方法関連の更新
	@retval 画面更新したかどうか
	@date 2008.06.10 ryoji 新規作成
*/
BOOL CEditWnd::UpdateTextWrap( void )
{
	// この関数はコマンド実行ごとに処理の最終段階で利用する
	// （アンドゥ登録＆全ビュー更新のタイミング）
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH ){
		BOOL bWrap = WrapWindowWidth( 0 );	// 右端で折り返す
		if( bWrap ){
			// WrapWindowWidth() で追加した更新リージョンで画面更新する
			for( int i = 0; i < GetAllViewCount(); i++ ){
				::UpdateWindow( m_pcEditViewArr[i]->m_hWnd );
			}
		}
		return bWrap;	// 画面更新＝折り返し変更
	}
	return FALSE;	// 画面更新しなかった
}

/*!	レイアウトパラメータの変更

	具体的にはタブ幅と折り返し位置を変更する．
	現在のドキュメントのレイアウトのみを変更し，共通設定は変更しない．

	@date 2005.08.14 genta 新規作成
	@date 2008.06.18 ryoji レイアウト変更途中はカーソル移動の画面スクロールを見せない（画面のちらつき抑止）
*/
void CEditWnd::ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineKetas )
{
	HWND		hwndProgress = NULL;
	if( bShowProgress && NULL != this ){
		hwndProgress = this->m_hwndProgressBar;
		//	Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	//	座標の保存
	CLogicPoint* posSave = SavePhysPosOfAllView();

	//	レイアウトの更新
	m_pcEditDoc->m_cLayoutMgr.ChangeLayoutParam( NULL, nTabSize, nMaxLineKetas );

	//	座標の復元
	//	レイアウト変更途中はカーソル移動の画面スクロールを見せない	// 2008.06.18 ryoji
	SetDrawSwitchOfAllViews( false );
	RestorePhysPosOfAllView( posSave );
	SetDrawSwitchOfAllViews( true );

	for( int i = 0; i < GetAllViewCount(); i++ ){
		if( m_pcEditViewArr[i]->m_hWnd ){
			InvalidateRect( m_pcEditViewArr[i]->m_hWnd, NULL, TRUE );
			m_pcEditViewArr[i]->AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}
	if( !m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
		GetActiveView().ShowCaretPosInfo();	// 2009.07.25 ryoji
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}


/*!
	レイアウトの変更に先立って，全てのViewの座標を物理座標に変換して保存する．

	@return データを保存した配列へのポインタ

	@note 取得した値はレイアウト変更後にCEditWnd::RestorePhysPosOfAllViewへ渡す．
	渡し忘れるとメモリリークとなる．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 戻り値をCLogicPoint*に変更
*/
CLogicPoint* CEditWnd::SavePhysPosOfAllView()
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;
	
	CLogicPoint* pptPosArray = new CLogicPoint[NUM_OF_VIEW * NUM_OF_POS];
	
	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			m_pcEditViewArr[i]->m_ptCaretPos.x,
			m_pcEditViewArr[i]->m_ptCaretPos.y,
			&pptPosArray[i * NUM_OF_POS + 0].x,
			&pptPosArray[i * NUM_OF_POS + 0].y
		);
		if( m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.x,
				m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.y,
				&pptPosArray[i * NUM_OF_POS + 1].x,
				&pptPosArray[i * NUM_OF_POS + 1].y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.x,
				m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.y,
				&pptPosArray[i * NUM_OF_POS + 2].x,
				&pptPosArray[i * NUM_OF_POS + 2].y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelect.m_ptFrom.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_pcEditViewArr[i]->m_sSelect.m_ptFrom.x,
				m_pcEditViewArr[i]->m_sSelect.m_ptFrom.y,
				&pptPosArray[i * NUM_OF_POS + 3].x,
				&pptPosArray[i * NUM_OF_POS + 3].y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelect.m_ptTo.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				m_pcEditViewArr[i]->m_sSelect.m_ptTo.x,
				m_pcEditViewArr[i]->m_sSelect.m_ptTo.y,
				&pptPosArray[i * NUM_OF_POS + 4].x,
				&pptPosArray[i * NUM_OF_POS + 4].y
			);
		}
	}
	return pptPosArray;
}


/*!	座標の復元

	CEditWnd::SavePhysPosOfAllViewで保存したデータを元に座標値を再計算する．

	@date 2005.08.11 genta  新規作成
	@date 2007.09.06 kobake 引数をCLogicPoint*に変更
*/
void CEditWnd::RestorePhysPosOfAllView( CLogicPoint* pptPosArray )
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 5;

	for( int i = 0; i < NUM_OF_VIEW; ++i ){
		CLayoutPoint tmp;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
			pptPosArray[i * NUM_OF_POS + 0].x,
			pptPosArray[i * NUM_OF_POS + 0].y,
			&tmp.x,
			&tmp.y
		);
		m_pcEditViewArr[i]->MoveCursor( tmp.x, tmp.y, true );
		m_pcEditViewArr[i]->m_nCaretPosX_Prev = m_pcEditViewArr[i]->m_ptCaretPos.x;

		if( m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 1].x,
				pptPosArray[i * NUM_OF_POS + 1].y,
				&m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.x,
				&m_pcEditViewArr[i]->m_sSelectBgn.m_ptFrom.y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 2].x,
				pptPosArray[i * NUM_OF_POS + 2].y,
				&m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.x,
				&m_pcEditViewArr[i]->m_sSelectBgn.m_ptTo.y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelect.m_ptFrom.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 3].x,
				pptPosArray[i * NUM_OF_POS + 3].y,
				&m_pcEditViewArr[i]->m_sSelect.m_ptFrom.x,
				&m_pcEditViewArr[i]->m_sSelect.m_ptFrom.y
			);
		}
		if( m_pcEditViewArr[i]->m_sSelect.m_ptTo.y >= 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
				pptPosArray[i * NUM_OF_POS + 4].x,
				pptPosArray[i * NUM_OF_POS + 4].y,
				&m_pcEditViewArr[i]->m_sSelect.m_ptTo.x,
				&m_pcEditViewArr[i]->m_sSelect.m_ptTo.y
			);
		}
	}
	delete[] pptPosArray;
}

/*!
	@brief マウスの状態をクリアする（ホイールスクロール有無状態をクリア）

	@note ホイール操作によるページスクロール・横スクロール対応のために追加。
		  ページスクロール・横スクロールありフラグをOFFする。

	@date 2009.01.12 nasukoji	新規作成
*/
void CEditWnd::ClearMouseState( void )
{
	SetPageScrollByWheel( FALSE );		// ホイール操作によるページスクロール有無
	SetHScrollByWheel( FALSE );			// ホイール操作による横スクロール有無
}

/*! ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)
	@date 2009.08.15 Hidetaka Sakai, nasukoji
	@date 2013.10.19 novice 共有メモリの代わりにWine実行判定処理を呼び出す

	@note Wineでは別プロセスで作成したアクセラレータテーブルを使用することができない。
	      IsWine()によりプロセス毎にアクセラレータテーブルが作成されるようになる
	      ため、ショートカットキーやカーソルキーが正常に処理されるようになる。
*/
void CEditWnd::CreateAccelTbl( void )
{
	if( IsWine() ){
		m_hAccelWine = CKeyBind::CreateAccerelator(
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
		);

		if( NULL == m_hAccelWine ){
			ErrorMessage(
				NULL,
				_T("CEditWnd::CEditWnd()\n")
				_T("アクセラレータ テーブルが作成できません。\n")
				_T("システムリソースが不足しています。")
			);
		}
	}

	m_hAccel = m_hAccelWine ? m_hAccelWine : m_pShareData->m_sHandles.m_hAccel;
}

/*! ウィンドウ毎に作成したアクセラレータテーブルを破棄する
	@datet 2009.08.15 Hidetaka Sakai, nasukoji
*/
void CEditWnd::DeleteAccelTbl( void )
{
	m_hAccel = NULL;

	if( m_hAccelWine ){
		::DestroyAcceleratorTable( m_hAccelWine );
		m_hAccelWine = NULL;
	}
}

/*[EOF]*/
