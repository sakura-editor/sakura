/*!	@file
	@brief Dialog Boxの基底クラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDialog.h"
#include "CEditApp.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "recent/CRecent.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/module.h"

/* ダイアログプロシージャ */
INT_PTR CALLBACK MyDialogProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
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
		// Modified by KEITA for WIN64 2003.9.6
		pCDialog = ( CDialog* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCDialog ){
			return pCDialog->DispatchEvent( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}


/*!	コンストラクタ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CDialog::CDialog(bool bSizable, bool bCheckShareData)
{
//	MYTRACE( _T("CDialog::CDialog()\n") );
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData(bCheckShareData);

	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hWnd  = NULL;			/* このダイアログのハンドル */
	m_hwndSizeBox = NULL;
	m_bSizable = bSizable;
	m_lParam = (LPARAM)NULL;
	m_nShowCmd = SW_SHOW;
	m_xPos = -1;
	m_yPos = -1;
	m_nWidth = -1;
	m_nHeight = -1;

	return;

}
CDialog::~CDialog()
{
//	MYTRACE( _T("CDialog::~CDialog()\n") );
	CloseDialog( 0 );
	return;
}

//! モーダルダイアログの表示
/*!
	@param hInstance [in] アプリケーションインスタンスのハンドル
	@param hwndParent [in] オーナーウィンドウのハンドル

	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
*/
INT_PTR CDialog::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam )
{
	m_bInited = FALSE;
	m_bModal = TRUE;
	m_hInstance = hInstance;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_lParam = lParam;
	m_hLangRsrcInstance = CSelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル
	return ::DialogBoxParam(
		m_hLangRsrcInstance,
		MAKEINTRESOURCE( nDlgTemplete ),
		m_hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
}

//! モードレスダイアログの表示
/*!
	@param hInstance [in] アプリケーションインスタンスのハンドル
	@param hwndParent [in] オーナーウィンドウのハンドル

	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
*/
HWND CDialog::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam, int nCmdShow )
{
	m_bInited = FALSE;
	m_bModal = FALSE;
	m_hInstance = hInstance;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_lParam = lParam;
	m_hLangRsrcInstance = CSelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル
	m_hWnd = ::CreateDialogParam(
		m_hLangRsrcInstance,
		MAKEINTRESOURCE( nDlgTemplete ),
		m_hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if( NULL != m_hWnd ){
		::ShowWindow( m_hWnd, nCmdShow );
	}
	return m_hWnd;
}

HWND CDialog::DoModeless( HINSTANCE hInstance, HWND hwndParent, LPCDLGTEMPLATE lpTemplate, LPARAM lParam, int nCmdShow )
{
	m_bInited = FALSE;
	m_bModal = FALSE;
	m_hInstance = hInstance;	/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_lParam = lParam;
	m_hWnd = ::CreateDialogIndirectParam(
		m_hInstance,
		lpTemplate,
		m_hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if( NULL != m_hWnd ){
		::ShowWindow( m_hWnd, nCmdShow );
	}
	return m_hWnd;
}

void CDialog::CloseDialog( INT_PTR nModalRetVal )
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
	// Modified by KEITA for WIN64 2003.9.6
	::SetWindowLongPtr( m_hWnd, DWLP_USER, lParam );

	/* ダイアログデータの設定 */
	SetData();

	SetDialogPosSize();

	m_bInited = TRUE;
	return TRUE;
}

void CDialog::SetDialogPosSize()
{
#if 0
	/* ダイアログのサイズ、位置の再現 */
	if( -1 != m_xPos && -1 != m_yPos ){
		::SetWindowPos( m_hWnd, NULL, m_xPos, m_yPos, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		DEBUG_TRACE( _T("CDialog::OnInitDialog() m_xPos=%d m_yPos=%d\n"), m_xPos, m_yPos );
	}
	if( -1 != m_nWidth && -1 != m_nHeight ){
		::SetWindowPos( m_hWnd, NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
	}
#endif

	if( -1 != m_xPos && -1 != m_yPos ){
		/* ウィンドウ位置・サイズを再現 */
		// 2014.11.28 フォント変更対応
		if( m_nWidth == -1 && m_nHeight == -1 ){
			RECT	rc;
			::GetWindowRect( m_hWnd, &rc );
			m_nWidth = rc.right - rc.left;
			m_nHeight = rc.bottom - rc.top;
		}

		if( !(::GetWindowLongPtr( m_hWnd, GWL_STYLE ) & WS_CHILD) ){
			// 2006.06.09 ryoji
			// モニタのワーク領域よりも左右上下に１ドット小さい領域内に全体が収まるように位置調整する
			//
			// note: ダイアログをワーク領域境界にぴったり合わせようとすると、
			//       強制的に親の中央に移動させられてしまうときがある
			//      （マルチモニタ環境で親が非プライマリモニタにある場合だけ？）
			//       状況に合わせて処理を変えるのは厄介なので、一律、１ドットの空きを入れる

			RECT rc;
			RECT rcWork;
			rc.left = m_xPos;
			rc.top = m_yPos;
			rc.right = m_xPos + m_nWidth;
			rc.bottom = m_yPos + m_nHeight;
			GetMonitorWorkRect(&rc, &rcWork);
			rcWork.top += 1;
			rcWork.bottom -= 1;
			rcWork.left += 1;
			rcWork.right -= 1;
			if( rc.bottom > rcWork.bottom ){
				rc.top -= (rc.bottom - rcWork.bottom);
				rc.bottom = rcWork.bottom;
			}
			if( rc.right > rcWork.right ){
				rc.left -= (rc.right - rcWork.right);
				rc.right = rcWork.right;
			}
			if( rc.top < rcWork.top ){
				rc.bottom += (rcWork.top - rc.top);
				rc.top = rcWork.top;
			}
			if( rc.left < rcWork.left ){
				rc.right += (rcWork.left - rc.left);
				rc.left = rcWork.left;
			}
			m_xPos = rc.left;
			m_yPos = rc.top;
			m_nWidth = rc.right - rc.left;
			m_nHeight = rc.bottom - rc.top;
		}

		WINDOWPLACEMENT cWindowPlacement;
		cWindowPlacement.length = sizeof( cWindowPlacement );
		cWindowPlacement.showCmd = m_nShowCmd;	//	最大化・最小化
		cWindowPlacement.rcNormalPosition.left = m_xPos;
		cWindowPlacement.rcNormalPosition.top = m_yPos;
		cWindowPlacement.rcNormalPosition.right = m_nWidth + m_xPos;
		cWindowPlacement.rcNormalPosition.bottom = m_nHeight + m_yPos;
		::SetWindowPlacement( m_hWnd, &cWindowPlacement );
	}
}

BOOL CDialog::OnDestroy( void )
{
	/* ウィンドウ位置・サイズを記憶 */
	WINDOWPLACEMENT cWindowPlacement;
	cWindowPlacement.length = sizeof( cWindowPlacement );
	if (::GetWindowPlacement( m_hWnd, &cWindowPlacement )){
		m_nShowCmd = cWindowPlacement.showCmd;	//	最大化・最小化
		m_xPos = cWindowPlacement.rcNormalPosition.left;
		m_yPos = cWindowPlacement.rcNormalPosition.top;
		m_nWidth = cWindowPlacement.rcNormalPosition.right - cWindowPlacement.rcNormalPosition.left;
		m_nHeight = cWindowPlacement.rcNormalPosition.bottom - cWindowPlacement.rcNormalPosition.top;
	}
	if( !m_bSizable ){
		m_nWidth = -1;
		m_nHeight = -1;
	}
	/* 破棄 */
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
	case IDCANCEL:	// Fall through.
	case IDOK:
		CloseDialog( wID );
		return TRUE;
	}
	return FALSE;
}

BOOL CDialog::OnSize()
{
	return CDialog::OnSize( 0, 0 );
}

BOOL CDialog::OnSize( WPARAM wParam, LPARAM lParam )
{
	RECT	rc;
	::GetWindowRect( m_hWnd, &rc );

	/* ダイアログのサイズの記憶 */
	m_xPos = rc.left;
	m_yPos = rc.top;
	m_nWidth = rc.right - rc.left;
	m_nHeight = rc.bottom - rc.top;

	/* サイズボックスの移動 */
	if( NULL != m_hwndSizeBox ){
		::GetClientRect( m_hWnd, &rc );
//		::SetWindowPos( m_hwndSizeBox, NULL,
//	Sept. 17, 2000 JEPRO_16thdot アイコンの16dot目が表示されるように次行を変更する必要ある？
//	Jan. 12, 2001 JEPRO (directed by stonee) 15を16に変更するとアウトライン解析のダイアログの右下にある
//	グリップサイズに`遊び'ができてしまい(移動する！)、ダイアログを大きくできないという障害が発生するので
//	変更しないことにした(要するに原作版に戻しただけ)
//			rc.right - rc.left - 15, rc.bottom - rc.top - 15,
//			13, 13,
//			SWP_NOOWNERZORDER | SWP_NOZORDER
//		);

//	Jan. 12, 2001 Stonee (suggested by genta)
//		"13"という固定値ではなくシステムから取得したスクロールバーサイズを使うように修正
		::SetWindowPos( m_hwndSizeBox, NULL,
		rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
		rc.bottom - rc.top - GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
		GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
		GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
		SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		//	SizeBox問題テスト
		if( wParam == SIZE_MAXIMIZED ){
			::ShowWindow( m_hwndSizeBox, SW_HIDE );
		}else{
			::ShowWindow( m_hwndSizeBox, SW_SHOW );
		}
		::InvalidateRect( m_hwndSizeBox, NULL, TRUE );
	}
	return FALSE;

}

BOOL CDialog::OnMove( WPARAM wParam, LPARAM lParam )
{

	/* ダイアログの位置の記憶 */
	if( !m_bInited ){
		return TRUE;
	}
	RECT	rc;
	::GetWindowRect( m_hWnd, &rc );

	/* ダイアログのサイズの記憶 */
	m_xPos = rc.left;
	m_yPos = rc.top;
	m_nWidth = rc.right - rc.left;
	m_nHeight = rc.bottom - rc.top;
	DEBUG_TRACE( _T("CDialog::OnMove() m_xPos=%d m_yPos=%d\n"), m_xPos, m_yPos );
	return TRUE;

}



void CDialog::CreateSizeBox( void )
{
	/* サイズボックス */
	m_hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT,								/* no extended styles */
		_T("SCROLLBAR"),									/* scroll bar control class */
		NULL,												/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
		0,													/* horizontal position */
		0,													/* vertical position */
		0,													/* width of the scroll bar */
		0,													/* default height */
		m_hWnd/*hdlg*/, 									/* handle of main window */
		(HMENU) NULL,										/* no menu for a scroll bar */
		CSelectLang::getLangRsrcInstance(),					/* instance owning this window */
		(LPVOID) NULL										/* pointer not needed */
	);
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

}






/* ダイアログのメッセージ処理 */
INT_PTR CDialog::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	DEBUG_TRACE( _T("CDialog::DispatchEvent() uMsg == %xh\n"), uMsg );
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
	case WM_ACTIVATE:	return OnActivate( wParam, lParam );	//@@@ 2003.04.08 MIK
	case WM_VKEYTOITEM:	return OnVKeyToItem( wParam, lParam );
	case WM_CHARTOITEM:	return OnCharToItem( wParam, lParam );
	case WM_HELP:		return OnPopupHelp( wParam, lParam );	//@@@ 2002.01.18 add
	case WM_CONTEXTMENU:return OnContextMenu( wParam, lParam );	//@@@ 2002.01.18 add
	}
	return FALSE;
}

BOOL CDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	wNotifyCode = HIWORD(wParam);	/* 通知コード */
	wID			= LOWORD(wParam);	/* 項目ID、 コントロールID、 またはアクセラレータID */
	hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
	TCHAR	szClass[32];

	// IDOK と IDCANCEL はボタンからでなくても同じ扱い
	// MSDN [Windows Management] "Dialog Box Programming Considerations"
	if( wID == IDOK || wID == IDCANCEL ){
		return OnBnClicked( wID );
	}

	// 通知元がコントロールだった場合の処理
	if( hwndCtl ){
		::GetClassName(hwndCtl, szClass, _countof(szClass));
		if( ::lstrcmpi(szClass, _T("Button")) == 0 ){
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:	return OnBnClicked( wID );
			}
		}else if( ::lstrcmpi(szClass, _T("Static")) == 0 ){
			switch( wNotifyCode ){
			case STN_CLICKED:	return OnStnClicked( wID );
			}
		}else if( ::lstrcmpi(szClass, _T("Edit")) == 0 ){
			switch( wNotifyCode ){
			case EN_CHANGE:		return OnEnChange( hwndCtl, wID );
			case EN_KILLFOCUS:	return OnEnKillFocus( hwndCtl, wID );
			}
		}else if( ::lstrcmpi(szClass, _T("ListBox")) == 0 ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:	return OnLbnSelChange( hwndCtl, wID );
			case LBN_DBLCLK:	return OnLbnDblclk( wID );
			}
		}else if( ::lstrcmpi(szClass, _T("ComboBox")) == 0 ){
			switch( wNotifyCode ){
			/* コンボボックス用メッセージ */
			case CBN_SELCHANGE:	return OnCbnSelChange( hwndCtl, wID );
			// @@2005.03.31 MIK タグジャンプDialogで使うので追加
			case CBN_EDITCHANGE:	return OnCbnEditChange( hwndCtl, wID );
			case CBN_DROPDOWN:	return OnCbnDropDown( hwndCtl, wID );
		//	case CBN_CLOSEUP:	return OnCbnCloseUp( hwndCtl, wID );
			case CBN_SELENDOK:	return OnCbnSelEndOk( hwndCtl, wID );
			}
		}
	}

	return FALSE;
}

//@@@ 2002.01.18 add start
BOOL CDialog::OnPopupHelp( WPARAM wPara, LPARAM lParam )
{
	HELPINFO *p = (HELPINFO *)lParam;
	MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)GetHelpIdTable() );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return TRUE;
}

BOOL CDialog::OnContextMenu( WPARAM wPara, LPARAM lParam )
{
	MyWinHelp( m_hWnd, HELP_CONTEXTMENU, (ULONG_PTR)GetHelpIdTable() );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return TRUE;
}

const DWORD p_helpids[] = {
	0, 0
};

LPVOID CDialog::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

BOOL CDialog::OnCbnSelEndOk( HWND hwndCtl, int wID )
{
	//コンボボックスのリストを表示したまま文字列を編集し、Enterキーを
	//押すと文字列が消える現象の対策。
	//Enterキーを押してこの関数に入ったら、リストを非表示にしてしまう。

	//リストを非表示にすると前方一致する文字列を選んでしまうので、
	//事前に文字列を退避し、リスト非表示後に復元する。

	int nLength;
	LPTSTR sBuf;

	//文字列を退避
	nLength = ::GetWindowTextLength( hwndCtl );
	sBuf = new TCHAR[nLength + 1];
	::GetWindowText( hwndCtl, sBuf, nLength+1 );
	sBuf[nLength] = _T('\0');

	//リストを非表示にする
	Combo_ShowDropdown( hwndCtl, FALSE );

	//文字列を復元・全選択
	::SetWindowText( hwndCtl, sBuf );
	Combo_SetEditSel( hwndCtl, 0, -1 );
	delete[] sBuf;

	return TRUE;
}

BOOL CDialog::OnCbnDropDown( HWND hwndCtl, int wID )
{
	return OnCbnDropDown( hwndCtl, false );
}
/** コンボボックスのドロップダウン時処理

	コンボボックスがドロップダウンされる時に
	ドロップダウンリストの幅をアイテム文字列の最大表示幅に合わせる

	@param hwndCtl [in]		コンボボックスのウィンドウハンドル
	@param wID [in]			コンボボックスのID

	@author ryoji
	@date 2009.03.29 新規作成
*/
BOOL CDialog::OnCbnDropDown( HWND hwndCtl, bool scrollBar )
{
	HDC hDC;
	HFONT hFont;
	LONG nWidth;
	RECT rc;
	SIZE sizeText;
	int nTextLen;
	int iItem;
	int nItem;
	const int nMargin = 8;
	int nScrollWidth = scrollBar ? ::GetSystemMetrics( SM_CXVSCROLL ) + 2 : 2;

	hDC = ::GetDC( hwndCtl );
	if( NULL == hDC )
		return FALSE;
	hFont = (HFONT)::SendMessageAny( hwndCtl, WM_GETFONT, 0, (LPARAM)NULL );
	hFont = (HFONT)::SelectObject( hDC, hFont );
	nItem = Combo_GetCount( hwndCtl );
	::GetWindowRect( hwndCtl, &rc );
	nWidth = rc.right - rc.left - nMargin + nScrollWidth;
	for( iItem = 0; iItem < nItem; iItem++ ){
		nTextLen = Combo_GetLBTextLen( hwndCtl, iItem );
		if( 0 < nTextLen ) {
			TCHAR* pszText = new TCHAR[nTextLen + 1];
			Combo_GetLBText( hwndCtl, iItem, pszText );
			if( ::GetTextExtentPoint32( hDC, pszText, nTextLen, &sizeText ) ){
				if ( nWidth < sizeText.cx + nScrollWidth )
					nWidth = sizeText.cx + nScrollWidth;
			}
			delete []pszText;
		}
	}
	Combo_SetDroppedWidth( hwndCtl, nWidth + nMargin );
	::SelectObject( hDC, hFont );
	::ReleaseDC( hwndCtl, hDC );
	return TRUE;
}

// static
bool CDialog::DirectoryUp( TCHAR* szDir )
{
	size_t nLen = auto_strlen( szDir );
	if( 3 < nLen ){
		// X:\ や\\. より長い
		CutLastYenFromDirectoryPath( szDir );
		const TCHAR *p = GetFileTitlePointer(szDir);
		if( 0 < p - szDir){
			if( 3 < p - szDir ){
				szDir[p - szDir - 1] = '\0'; // \を削るので-1
			}else{
				// 「C:\」の\を残す
				szDir[p - szDir] = '\0';
			}
		}
		return true;
	}
	return false;
}

// コントロールに画面のフォントを設定	2012/11/27 Uchi
HFONT CDialog::SetMainFont( HWND hTarget )
{
	if (hTarget == NULL)	return NULL;

	HFONT	hFont;
	LOGFONT	lf;

	// 設定するフォントの高さを取得
	hFont = (HFONT)::SendMessage(hTarget, WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(lf), &lf);
	LONG nfHeight = lf.lfHeight;

	// LOGFONTの作成
	lf = m_pShareData->m_Common.m_sView.m_lf;
	lf.lfHeight			= nfHeight;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= FW_NORMAL;
	lf.lfItalic			= FALSE;
	lf.lfUnderline		= FALSE;
	lf.lfStrikeOut		= FALSE;
	//lf.lfCharSet		= lf.lfCharSet;
	lf.lfOutPrecision	= OUT_TT_ONLY_PRECIS;		// Raster Font を使わないように
	//lf.lfClipPrecision	= lf.lfClipPrecision;
	//lf.lfQuality		= lf.lfQuality;
	//lf.lfPitchAndFamily	= lf.lfPitchAndFamily;
	//_tcsncpy( lf.lfFaceName, lf.lfFaceName, _countof(lf.lfFaceName));	// 画面のフォントに設定	2012/11/27 Uchi

	// フォントを作成
	hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// フォントの設定
		::SendMessage(hTarget, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}
	return hFont;
}

void CDialog::ResizeItem( HWND hTarget, const POINT& ptDlgDefault, const POINT& ptDlgNew, const RECT& rcItemDefault, EAnchorStyle anchor, bool bUpdate)
{
	POINT pt;
	int height, width;
	pt.x = rcItemDefault.left;
	pt.y = rcItemDefault.top;
	width = rcItemDefault.right - rcItemDefault.left;
	height = rcItemDefault.bottom - rcItemDefault.top;
	if( (anchor & (ANCHOR_LEFT | ANCHOR_RIGHT)) == ANCHOR_LEFT ){
		// なし
	}
	else if( (anchor & (ANCHOR_LEFT | ANCHOR_RIGHT)) == ANCHOR_RIGHT ){
		/*
			[<- rcItemDefault.left ->[   ]      ]
			[<- rcItemDefault.right  [ ->]      ]
			[<-    ptDlgDefault.x             ->]
			[<-    ptDlgNew.x             [   ]    ->]
			[<-    pt.x                 ->[   ]      ]
		*/
		pt.x = rcItemDefault.left + (ptDlgNew.x - ptDlgDefault.x);
	}
	else if( (anchor & (ANCHOR_LEFT | ANCHOR_RIGHT)) == (ANCHOR_LEFT | ANCHOR_RIGHT) ){
		/*
			[<-    ptDlgNew.x        [   ]         ->]
			[                        [<-width->]     ]
		*/
		width = ptDlgNew.x - rcItemDefault.left - (ptDlgDefault.x - rcItemDefault.right);
	}
	
	if( (anchor & (ANCHOR_TOP | ANCHOR_BOTTOM) ) == ANCHOR_TOP ){
		// なし
	}
	else if( (anchor & (ANCHOR_TOP | ANCHOR_BOTTOM) ) == ANCHOR_BOTTOM ){
		pt.y = rcItemDefault.top + (ptDlgNew.y - ptDlgDefault.y);
	}
	else if( (anchor & (ANCHOR_TOP | ANCHOR_BOTTOM) ) == (ANCHOR_TOP | ANCHOR_BOTTOM) ){
		height = ptDlgNew.y - rcItemDefault.top - (ptDlgDefault.y - rcItemDefault.bottom);
	}
//	::MoveWindow( hTarget, pt.x, pt.y, width, height, FALSE );
	::SetWindowPos( hTarget, NULL, pt.x, pt.y, width, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER );
	if( bUpdate ){
		::InvalidateRect( hTarget, NULL, TRUE );
	}
}

void CDialog::GetItemClientRect( int wID, RECT& rc )
{
	POINT po;
	::GetWindowRect( GetItemHwnd(wID), &rc );
	po.x = rc.left;
	po.y = rc.top;
	::ScreenToClient( GetHwnd(), &po );
	rc.left = po.x;
	rc.top  = po.y;
	po.x = rc.right;
	po.y = rc.bottom;
	::ScreenToClient( GetHwnd(), &po );
	rc.right  = po.x;
	rc.bottom = po.y;
}


static const TCHAR* TSTR_SUBCOMBOBOXDATA = _T("SubComboBoxData");

static void DeleteItem(HWND hwnd, CRecent* pRecent)
{
	int nIndex = Combo_GetCurSel(hwnd);
	if( 0 <= nIndex ){
		std::vector<TCHAR> szText;
		szText.resize(Combo_GetLBTextLen(hwnd, nIndex) + 1);
		Combo_GetLBText(hwnd, nIndex, &szText[0]);
		Combo_DeleteString(hwnd, nIndex);
		int nRecentIndex = pRecent->FindItemByText(&szText[0]);
		if( 0 <= nRecentIndex ){
			pRecent->DeleteItem(nRecentIndex);
		}
	}
}

LRESULT CALLBACK SubEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SComboBoxItemDeleter* data = (SComboBoxItemDeleter*)::GetProp( hwnd, TSTR_SUBCOMBOBOXDATA );
	switch( uMsg ){
	case WM_KEYDOWN:
	{
		if( wParam == VK_DELETE ){
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if( bShow ){
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pEditWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pEditWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pEditWndProc, hwnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK SubListBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SComboBoxItemDeleter* data = (SComboBoxItemDeleter*)::GetProp( hwnd, TSTR_SUBCOMBOBOXDATA );
	switch( uMsg ){
	case WM_KEYDOWN:
	{
		if( wParam == VK_DELETE ){
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if( bShow ){
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pListBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pListBoxWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pListBoxWndProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK SubComboBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SComboBoxItemDeleter* data = (SComboBoxItemDeleter*)::GetProp( hwnd, TSTR_SUBCOMBOBOXDATA );
	switch( uMsg ){
	case WM_CTLCOLOREDIT:
	{
		if( NULL == data->pEditWndProc ){
			HWND hwndCtl = (HWND)lParam;
			data->pEditWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubEditProc);
		}
		break;
	}
	case WM_CTLCOLORLISTBOX:
	{
		if( NULL == data->pListBoxWndProc ){
			HWND hwndCtl = (HWND)lParam;
			data->pListBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubListBoxProc);
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pComboBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pComboBoxWndProc = NULL;
		break;
	}

	default:
		break;
	}
	return CallWindowProc(data->pComboBoxWndProc, hwnd, uMsg, wParam, lParam);
}


void CDialog::SetComboBoxDeleter( HWND hwndCtl, SComboBoxItemDeleter* data )
{
	if( NULL == data->pRecent ){
		return;
	}
	data->hwndCombo = hwndCtl;
	data->pComboBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
	::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
	::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubComboBoxProc);
}
