#include "StdAfx.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
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




/** フレームウィンドウをアクティブにする
	@date 2007.11.07 ryoji 対象がdisableのときは最近のポップアップをフォアグラウンド化する．
		（モーダルダイアログやメッセージボックスを表示しているようなとき）
*/
void ActivateFrameWindow( HWND hwnd )
{
	// 編集ウィンドウでタブまとめ表示の場合は表示位置を復元する
	DLLSHAREDATA* pShareData = &GetDllShareData();
	if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
		if( IsSakuraMainWindow( hwnd ) ){
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
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

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
