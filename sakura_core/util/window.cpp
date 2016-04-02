#include "StdAfx.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "util/os.h"
#include <limits.h>
#include "window.h"

int CDPI::nDpiX = 96;
int CDPI::nDpiY = 96;
bool CDPI::bInitialized = false;

/**	指定したウィンドウの祖先のハンドルを取得する

	GetAncestor() APIがWin95で使えないのでそのかわり

	WS_POPUPスタイルを持たないウィンドウ（ex.CDlgFuncListダイアログ）だと、
	GA_ROOTOWNERでは編集ウィンドウまで遡れないみたい。GetAncestor() APIでも同様。
	本関数固有に用意したGA_ROOTOWNER2では遡ることができる。

	@author ryoji
	@date 2007.07.01 ryoji 新規
	@date 2007.10.22 ryoji フラグ値としてGA_ROOTOWNER2（本関数固有）を追加
	@date 2008.04.09 ryoji GA_ROOTOWNER2 は可能な限り祖先を遡るように動作修正
*/
HWND MyGetAncestor( HWND hWnd, UINT gaFlags )
{
	HWND hwndAncestor;
	HWND hwndDesktop = ::GetDesktopWindow();
	HWND hwndWk;

	if( hWnd == hwndDesktop )
		return NULL;

	switch( gaFlags )
	{
	case GA_PARENT:	// 親ウィンドウを返す（オーナーは返さない）
		hwndAncestor = ( (DWORD)::GetWindowLongPtr( hWnd, GWL_STYLE ) & WS_CHILD )? ::GetParent( hWnd ): hwndDesktop;
		break;

	case GA_ROOT:	// 親子関係を遡って直近上位のトップレベルウィンドウを返す
		hwndAncestor = hWnd;
		while( (DWORD)::GetWindowLongPtr( hwndAncestor, GWL_STYLE ) & WS_CHILD )
			hwndAncestor = ::GetParent( hwndAncestor );
		break;

	case GA_ROOTOWNER:	// 親子関係と所有関係をGetParent()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
		}while( hwndWk != NULL );
		break;

	case GA_ROOTOWNER2:	// 所有関係をGetWindow()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
			if( hwndWk == NULL )
				hwndWk = ::GetWindow( hwndAncestor, GW_OWNER );
		}while( hwndWk != NULL );
		break;

	default:
		hwndAncestor = NULL;
		break;
	}

	return hwndAncestor;
}


/*!
	処理中のユーザー操作を可能にする
	ブロッキングフック(?)（メッセージ配送

	@date 2003.07.04 genta 一回の呼び出しで複数メッセージを処理するように
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
	MSG		msg;
	BOOL	ret;
	//	Jun. 04, 2003 genta メッセージをあるだけ処理するように
	while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
		if ( msg.message == WM_QUIT ){
			return FALSE;
		}
		if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
		}else{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}
	return TRUE/*ret*/;
}




bool GetVirtualSnapRect( HWND hWnd, RECT* prcSnap/* = NULL*/, RECT* prcUnsnap/* = NULL*/ )
{
	RECT rcSnap, rcUnsnap;

	// Note. Unsnap サイズだけでなく Snap サイズも Window Byte に記憶している理由
	//   SetAeroSnap() 操作途中のウィンドウは一時的に Unsnap サイズにされるが、
	//   このタイミングで呼ばれた場合にも Snap サイズを返せるように。
	rcSnap.left = ::GetWindowLong(hWnd, GWL_SNAP_LEFT);
	rcSnap.top = ::GetWindowLong(hWnd, GWL_SNAP_TOP);
	rcSnap.right = ::GetWindowLong(hWnd, GWL_SNAP_RIGHT);
	rcSnap.bottom = ::GetWindowLong(hWnd, GWL_SNAP_BOTTOM);

	rcUnsnap.left = ::GetWindowLong(hWnd, GWL_UNSNAP_LEFT);
	rcUnsnap.top = ::GetWindowLong(hWnd, GWL_UNSNAP_TOP);
	rcUnsnap.right = ::GetWindowLong(hWnd, GWL_UNSNAP_RIGHT);
	rcUnsnap.bottom = ::GetWindowLong(hWnd, GWL_UNSNAP_BOTTOM);

	bool bRet = (!::IsRectEmpty(&rcSnap) && !::IsRectEmpty(&rcUnsnap));
	if (bRet)
	{
		if (prcSnap) *prcSnap = rcSnap;
		if (prcUnsnap) *prcUnsnap = rcUnsnap;
	}
	return bRet;
}

void SetVirtualSnapRect( HWND hWnd, const RECT* prcSnap, const RECT* prcUnsnap )
{
	::SetWindowLong(hWnd, GWL_SNAP_LEFT, prcSnap->left);
	::SetWindowLong(hWnd, GWL_SNAP_TOP, prcSnap->top);
	::SetWindowLong(hWnd, GWL_SNAP_RIGHT, prcSnap->right);
	::SetWindowLong(hWnd, GWL_SNAP_BOTTOM, prcSnap->bottom);

	::SetWindowLong(hWnd, GWL_UNSNAP_LEFT, prcUnsnap->left);
	::SetWindowLong(hWnd, GWL_UNSNAP_TOP, prcUnsnap->top);
	::SetWindowLong(hWnd, GWL_UNSNAP_RIGHT, prcUnsnap->right);
	::SetWindowLong(hWnd, GWL_UNSNAP_BOTTOM, prcUnsnap->bottom);
}

void SetVirtualSnapRectEmpty( HWND hWnd )
{
	RECT rcEmpty;
	::SetRectEmpty(&rcEmpty);
	SetVirtualSnapRect(hWnd, &rcEmpty, &rcEmpty);
}

bool GetAeroSnapRect( HWND hWnd, RECT* prcSnap/* = NULL*/, RECT* prcUnsnap/* = NULL*/, bool bRealOnly/* = false*/ )
{
	if (IsZoomed(hWnd) || IsIconic(hWnd))
		return false;

	if (!bRealOnly)
	{
		if (GetVirtualSnapRect(hWnd, prcSnap, prcUnsnap))
			return true;
	}

	RECT rcWnd, rcWork, rcMon;
	::GetWindowRect(hWnd, &rcWnd);
	GetMonitorWorkRect(hWnd, &rcWork, &rcMon);
	::OffsetRect(&rcWnd, rcMon.left - rcWork.left, rcMon.top - rcWork.top);	// ワークエリア座標に変換
	WINDOWPLACEMENT wp = {sizeof(wp)};
	::GetWindowPlacement(hWnd, &wp);
	if (!::EqualRect(&wp.rcNormalPosition, &rcWnd))
	{
		if (prcUnsnap) *prcUnsnap = wp.rcNormalPosition;
		if (prcSnap) *prcSnap = rcWnd;
		return true;
	}

	return false;
}

bool SetAeroSnap( HWND hWnd )
{
	DLLSHAREDATA* pShareData = &GetDllShareData();
	if( !::IsWindowVisible(hWnd) || ::IsZoomed(hWnd) ||
		!(pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin) ){
		// Aero Snap 状態への遷移が不要になった
		SetVirtualSnapRectEmpty(hWnd);
		return true;
	}

	RECT rcSnap, rcUnsnap;
	if (!GetVirtualSnapRect(hWnd, &rcSnap, &rcUnsnap))	// （注）rcSnap は現在の位置と同じのはず
		return true;

	if (GetWindowThreadProcessId(::GetForegroundWindow(), NULL) != GetWindowThreadProcessId(hWnd, NULL))
		return false;

	RECT rcWork, rcMon;
	GetMonitorWorkRect(hWnd, &rcWork, &rcMon);

	// Aero Snap 操作できるように、強制的にエディタウィンドウをアクティブ化する
	// （メッセージボックス表示中など Aero Snap 不可状態でも一時的に可能にする）
	HWND hWndActiveOld = ::GetActiveWindow();
	BOOL bEnableOld = ::IsWindowEnabled(hWnd);
	::EnableWindow(hWnd, TRUE);
	::SetActiveWindow(hWnd);

	// 一時的に Snap 解除後の位置に戻す
	::OffsetRect(&rcSnap, rcWork.left - rcMon.left, rcWork.top - rcMon.top);
	::OffsetRect(&rcUnsnap, rcWork.left - rcMon.left, rcWork.top - rcMon.top);
	::MoveWindow(hWnd, rcUnsnap.left, rcUnsnap.top, rcUnsnap.right - rcUnsnap.left, rcUnsnap.bottom - rcUnsnap.top, FALSE);

	// Shift, Ctrl, Alt キーは離す
	INPUT in[3 + 4];
	ULONG_PTR dwExtraInfo = ::GetMessageExtraInfo();
	in[0].ki.wVk = VK_SHIFT;
	in[1].ki.wVk = VK_CONTROL;
	in[2].ki.wVk = VK_MENU;
	int i;
	for (i = 0; i < 3; i++)
	{
		in[i].type = INPUT_KEYBOARD;
		in[i].ki.wScan = ::MapVirtualKey(in[i].ki.wVk, 0);
		in[i].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		in[i].ki.time = 0;
		in[i].ki.dwExtraInfo = dwExtraInfo;
	}
	// Aero Snap キー操作を注入する
	INPUT* pin = &in[i];
	pin[0].ki.wVk = pin[3].ki.wVk = VK_LWIN;
	pin[1].ki.wVk = pin[2].ki.wVk = ((rcSnap.right + rcSnap.left) < (rcWork.right + rcWork.left))? VK_LEFT: VK_RIGHT;
	pin[0].ki.dwFlags = pin[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	pin[2].ki.dwFlags = pin[3].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	for (i = 0; i < 4; i++)
	{
		pin[i].type = INPUT_KEYBOARD;
		pin[i].ki.wScan = ::MapVirtualKey(pin[i].ki.wVk, 0);
		pin[i].ki.time = 0;
		pin[i].ki.dwExtraInfo = dwExtraInfo;
	}
	::SendInput(_countof(in), in, sizeof(in[0]));
	BlockingHook(NULL);

	// Snap 位置に戻す
	::ShowWindow(hWnd, SW_HIDE);
	::MoveWindow(hWnd, rcSnap.left, rcSnap.top, rcSnap.right - rcSnap.left, rcSnap.bottom - rcSnap.top, FALSE);
	::ShowWindow(hWnd, SW_SHOW);
	::SetActiveWindow(hWndActiveOld);
	::EnableWindow(hWnd, bEnableOld);

	SetVirtualSnapRectEmpty(hWnd);

	// Win 10 では Aero Snap 操作で Foreground が外れるので強制的に戻す
	::Sleep(0);	// おまじない（Foreground 化が安定動作しますように）
	::SetForegroundWindow(hWnd);	// for Windows 10

	return true;
}

/** フレームウィンドウをアクティブにする
	@date 2007.11.07 ryoji 対象がdisableのときは最近のポップアップをフォアグラウンド化する．
		（モーダルダイアログやメッセージボックスを表示しているようなとき）
*/
void ActivateFrameWindow( HWND hwnd )
{
	// 編集ウィンドウでタブまとめ表示の場合は表示位置を復元する
	bool bAeroSnap = false;
	DLLSHAREDATA* pShareData = &GetDllShareData();
	if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
		if( IsSakuraMainWindow( hwnd ) ){
			// 既にアクティブなら何もしない
			// ※ 多重にアクティブ化（SetWindowPlacement）すると AeroSnap が解除されてしまう
			//    （例）「同名のC/C++ヘッダ（ソース）を開く」から不要なのに呼ばれている
			HWND hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
			if( hwndActivate == ::GetForegroundWindow() )
				return;

			if( pShareData->m_sFlags.m_bEditWndChanging )
				return;	// 切替の最中(busy)は要求を無視する
			pShareData->m_sFlags.m_bEditWndChanging = TRUE;	// 編集ウィンドウ切替中ON	2007.04.03 ryoji

			// 対象ウィンドウのスレッドに位置合わせを依頼する	// 2007.04.03 ryoji
			DWORD_PTR dwResult;
			::SendMessageTimeout(
				hwnd,
				MYWM_TAB_WINDOW_NOTIFY,
				TWNT_WNDPL_ADJUST,
				(LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK,
				10000,
				&dwResult
			);
			bAeroSnap = GetVirtualSnapRect(hwnd);
		}
	}

	// 対象がdisableのときは最近のポップアップをフォアグラウンド化する
	HWND hwndActivate;
	hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		// SW_SHOW -> SW_SHOWNAに変更（SW_SHOW だと既に Aero Snap 状態の場合に解除されてしまう）
		// （例）ファイル1を開いたエディタ1が Snap 状態で起動している状態で、
		//       更に起動パラメータにファイル1を指定してエディタ2を起動すると
		//       エディタ2がエディタ1をアクティブにする際、SW_SHOW だと Snap 解除されてしまうことがある
		//       ↑エディタ1から[ファイル]-[開く]操作でファイルダイアログのファイル名エディットボックス
		//         にファイル1、ファイル2の2ファイルを入力して[開く]ボタンを押すと再現
		::ShowWindow( hwnd, SW_SHOWNA );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

	// Aero Snap 状態の反映待ち
	// （まとめて閉じる場合でも Aero Snap が引き継がれるように）
	if( bAeroSnap ){
		DWORD dwTid = GetWindowThreadProcessId(hwnd, NULL);
		if( dwTid != GetCurrentThreadId() && dwTid == GetWindowThreadProcessId(::GetForegroundWindow(), NULL) ){
			for(int iRetry = 0; iRetry < 40; iRetry++){
				if( !GetVirtualSnapRect(hwnd) )
					break;
				::Sleep(50);
			}
		}
	}

	if( pShareData )
		pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中OFF	2007.04.03 ryoji

	return;
}


CTextWidthCalc::CTextWidthCalc(HWND hParent, int nID)
{
	assert_warning(hParent);

	hwnd = ::GetDlgItem(hParent, nID);
	hDC = ::GetDC( hwnd );
	assert(hDC);
	hFont = (HFONT)::SendMessageAny(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HWND hwndThis)
{
	assert_warning(hwndThis);

	hwnd = hwndThis;
	hDC = ::GetDC( hwnd );
	assert(hDC);
	hFont = (HFONT)::SendMessageAny(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HFONT font)
{
	hwnd = 0;
	HDC hDCTemp = ::GetDC( NULL ); // Desktop
	hDC = ::CreateCompatibleDC( hDCTemp );
	::ReleaseDC( NULL, hDCTemp );
	assert(hDC);
	hFont = font;
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HDC hdc)
{
	hwnd = 0;
	hDC = hdc;
	assert(hDC);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = true;
}

CTextWidthCalc::~CTextWidthCalc()
{
	if(hDC && !bFromDC){
		::SelectObject(hDC, hFontOld);
		if( bHDCComp ){
			::DeleteDC(hDC);
		}else{
			::ReleaseDC(hwnd, hDC);
		}
		hwnd = 0;
		hDC = 0;
	}
}


bool CTextWidthCalc::SetWidthIfMax(int width)
{
	return SetWidthIfMax(0, INT_MIN);
}

bool CTextWidthCalc::SetWidthIfMax(int width, int extCx)
{
	if( INT_MIN == extCx ){
		extCx = nExt;
	}
	if( nCx < width + extCx ){
		nCx = width + extCx;
		return true;
	}
	return false;
}

bool CTextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText)
{
	return SetTextWidthIfMax(pszText, INT_MIN);
}

bool CTextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText, int extCx)
{
	SIZE size;
	if( ::GetTextExtentPoint32( hDC, pszText, _tcslen(pszText), &size ) ){
		return SetWidthIfMax(size.cx, extCx);
	}
	return false;
}

int CTextWidthCalc::GetTextWidth(LPCTSTR pszText) const
{
	SIZE size;
	if( ::GetTextExtentPoint32( hDC, pszText, _tcslen(pszText), &size ) ){
		return size.cx;
	}
	return 0;
}

int CTextWidthCalc::GetTextHeight() const
{
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);
	return tm.tmHeight;
}

CFontAutoDeleter::CFontAutoDeleter()
	: m_hFontOld(NULL)
	, m_hFont(NULL)
	, m_hwnd(NULL)
{}

CFontAutoDeleter::~CFontAutoDeleter()
{
	if( m_hFont ){
		DeleteObject( m_hFont );
		m_hFont = NULL;
	}
}

void CFontAutoDeleter::SetFont( HFONT hfontOld, HFONT hfont, HWND hwnd )
{
	if( m_hFont ){
		::DeleteObject( m_hFont );
	}
	if( m_hFont != hfontOld ){
		m_hFontOld = hfontOld;
	}
	m_hFont = hfont;
	m_hwnd = hwnd;
}

/*! ウィンドウのリリース(WM_DESTROY用)
*/
void CFontAutoDeleter::ReleaseOnDestroy()
{
	if( m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	m_hFontOld = NULL;
}

/*! ウィンドウ生存中のリリース
*/
#if 0
void CFontAutoDeleter::Release()
{
	if( m_hwnd && m_hFont ){
		::SendMessageAny( m_hwnd, WM_SETFONT, (WPARAM)m_hFontOld, FALSE );
		::DeleteObject( m_hFont );
		m_hFont = NULL;
		m_hwnd = NULL;
	}
}
#endif
