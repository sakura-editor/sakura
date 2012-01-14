/*! @file
	@brief 文字色／背景色統一ダイアログ

	@author ryoji
	@date 2006/04/26 作成
*/
/*
	Copyright (C) 2006, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "CDlgSameColor.h"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,						HIDOK_SAMECOLOR,						// OK
	IDCANCEL,					HIDCANCEL_SAMECOLOR,					// キャンセル
	IDC_BUTTON_HELP,			HIDC_BUTTON_SAMECOLOR_HELP,				// ヘルプ
	IDC_LIST_COLORS,			HIDC_LIST_SAMECOLOR_COLORS,				// 変更対象の色
	IDC_BUTTON_SELALL,			HIDC_BUTTON_SAMECOLOR_SELALL,			// 全チェック
	IDC_BUTTON_SELNOTING,		HIDC_BUTTON_SAMECOLOR_SELNOTING,		// 全解除
	IDC_LIST_ITEMINFO,			HIDC_LIST_SAMECOLOR_ITEMINFO,			// 選択中の色に対応する項目のリスト
	IDC_STATIC_COLOR,			HIDC_STATIC_COLOR,						// 統一色
	0, 0
};

LPVOID CDlgSameColor::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

CDlgSameColor::CDlgSameColor() :
	m_wpColorStaticProc(NULL),
	m_wpColorListProc(NULL),
	m_wID(0),
	m_pTypes(NULL),
	m_cr(0)
{
	return;
}

CDlgSameColor::~CDlgSameColor()
{
	return;
}

/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR CDlgSameColor::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		// 色選択リストボックスの選択が変更された場合の処理
		if( IDC_LIST_COLORS == LOWORD(wParam) && LBN_SELCHANGE == HIWORD(wParam) ){
			OnSelChangeListColors( (HWND)lParam );
		}
		break;

	case WM_CTLCOLORLISTBOX:
		{
			// 項目リストの背景色を設定する処理
			HWND hwndLB = (HWND)lParam;
			if( IDC_LIST_ITEMINFO == ::GetDlgCtrlID( hwndLB ) ){
				HDC hdcLB = (HDC)wParam;
				::SetTextColor( hdcLB, ::GetSysColor( COLOR_WINDOWTEXT ) );
				::SetBkMode( hdcLB, TRANSPARENT );
				return (INT_PTR)::GetSysColorBrush( COLOR_BTNFACE );
			}
		}
		break;

	default:
		break;
	}
	return result;
}

/*! モーダルダイアログの表示
	@param wID [in] タイプ別設定ダイアログで押されたボタンID
	@param pTypes  [in/out] タイプ別設定データ
	@param cr [in] 指定色

	@date 2006.04.26 ryoji 新規作成
*/
int CDlgSameColor::DoModal( HINSTANCE hInstance, HWND hwndParent, WORD wID, Types* pTypes, COLORREF cr )
{
	m_wID = wID;
	m_pTypes = pTypes;
	m_cr = cr;

	(void)CDialog::DoModal( hInstance, hwndParent, IDD_SAMECOLOR, NULL );

	return TRUE;
}

/*! WM_INITDIALOG 処理
	@date 2006.04.26 ryoji 新規作成
*/
BOOL CDlgSameColor::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL bRet = CDialog::OnInitDialog( hwndDlg, wParam, lParam );

	HWND hwndStatic = ::GetDlgItem( m_hWnd, IDC_STATIC_COLOR );
	HWND hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_COLORS );

	// 指定色スタティック、色選択リストをサブクラス化
	::SetWindowLongPtr( hwndStatic, GWLP_USERDATA, (LONG_PTR)this );
	m_wpColorStaticProc = (WNDPROC)::SetWindowLongPtr( hwndStatic, GWLP_WNDPROC, (LONG_PTR)ColorStatic_SubclassProc );
	::SetWindowLongPtr( hwndList, GWLP_USERDATA, (LONG_PTR)this );
	m_wpColorListProc = (WNDPROC)::SetWindowLongPtr( hwndList, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );


	TCHAR szText[30];
	int nItem;
	int i;

	switch( m_wID )	// タイプ別設定ダイアログで押されたボタンID
	{
	case IDC_BUTTON_SAMETEXTCOLOR:
		// タイプ別設定から文字色を重複しないように取り出す
		::SetWindowText( m_hWnd, _T("文字色統一") );
		for( i = 0; i < COLORIDX_LAST; ++i ){
			if( m_cr != m_pTypes->m_ColorInfoArr[i].m_colTEXT ){
				_ultot( m_pTypes->m_ColorInfoArr[i].m_colTEXT, szText, 10 );
				if( LB_ERR == ::SendMessage( hwndList, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szText ) ){
					nItem = ::SendMessage( hwndList, LB_ADDSTRING, (WPARAM)0, (LPARAM)szText );
					::SendMessage( hwndList, LB_SETITEMDATA, (WPARAM)nItem, (LPARAM)FALSE ); 
				}
			}
		}
		break;

	case IDC_BUTTON_SAMEBKCOLOR:
		// タイプ別設定から背景色を重複しないように取り出す
		::SetWindowText( m_hWnd, _T("背景色統一") );
		for( i = 0; i < COLORIDX_LAST; ++i ){
			if( 0 != (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
				continue;
			if( m_cr != m_pTypes->m_ColorInfoArr[i].m_colBACK ){
				_ultot( m_pTypes->m_ColorInfoArr[i].m_colBACK, szText, 10 );
				if( LB_ERR == ::SendMessage( hwndList, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szText ) ){
					nItem = ::SendMessage( hwndList, LB_ADDSTRING, (WPARAM)0, (LPARAM)szText );
					::SendMessage( hwndList, LB_SETITEMDATA, (WPARAM)nItem, (LPARAM)FALSE ); 
				}
			}
		}
		break;

	default:
		CloseDialog( IDCANCEL );
		break;
	}

	if( 0 < ::SendMessage( hwndList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0 ) ){
		::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
		OnSelChangeListColors( hwndList );
	}

	return bRet;
}

/*! BN_CLICKED 処理
	@date 2006.04.26 ryoji 新規作成
*/
BOOL CDlgSameColor::OnBnClicked( int wID )
{
	HWND hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_COLORS );
	int nItemNum = ::SendMessage( hwndList, LB_GETCOUNT, 0, 0 );
	BOOL bCheck;
	int i;
	int j;

	switch( wID ){
	case IDC_BUTTON_HELP:
		// ヘルプ	// 2006.10.07 ryoji
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, HLP000316 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDC_BUTTON_SELALL:
	case IDC_BUTTON_SELNOTING:
		// 全選択／全解除の処理
		bCheck = (wID == IDC_BUTTON_SELALL);
		for( i = 0; i < nItemNum; ++i ){
			::SendMessage( hwndList, LB_SETITEMDATA, (WPARAM)i, (LPARAM)bCheck );
		}
		::InvalidateRect( hwndList, NULL, TRUE );
		break;

	case IDOK:
		// タイプ別設定から選択色と同色のものを取り出して指定色に一括変更する
		TCHAR szText[30];
		LPTSTR pszStop;
		COLORREF cr;

		for( i = 0; i < nItemNum; ++i ){
			bCheck = (BOOL)::SendMessage( hwndList, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0 );
			if( bCheck ){
				::SendMessage( hwndList, LB_GETTEXT, (WPARAM)i, (LPARAM)szText );
				cr = _tcstoul( szText, &pszStop, 10 );

				switch( m_wID )
				{
				case IDC_BUTTON_SAMETEXTCOLOR:
					for( j = 0; j < COLORIDX_LAST; ++j ){
						if( cr == m_pTypes->m_ColorInfoArr[j].m_colTEXT ){
							m_pTypes->m_ColorInfoArr[j].m_colTEXT = m_cr;
						}
					}
					break;

				case IDC_BUTTON_SAMEBKCOLOR:
					for( j = 0; j < COLORIDX_LAST; ++j ){
						if( cr == m_pTypes->m_ColorInfoArr[j].m_colBACK ){
							m_pTypes->m_ColorInfoArr[j].m_colBACK = m_cr;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		break;

	case IDCANCEL:
		break;
	}
	return CDialog::OnBnClicked( wID );
}

/*! WM_DRAWITEM 処理
	@date 2006.04.26 ryoji 新規作成
*/
BOOL CDlgSameColor::OnDrawItem( WPARAM wParam, LPARAM lParam )
{
	LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;	// 項目描画情報
	if( IDC_LIST_COLORS != pDis->CtlID )	// オーナー描画にしているのは色選択リストだけ
		return TRUE;

	//
	// 色選択リストの描画処理
	//
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	RECT		rc;
	TCHAR		szText[30];
	LPTSTR		pszStop;
	COLORREF	cr;

	::SendMessage( pDis->hwndItem, LB_GETTEXT, pDis->itemID, (LPARAM)szText );
	cr = _tcstoul( szText, &pszStop, 10 );

	rc = pDis->rcItem;

	// アイテム矩形塗りつぶし
	::FillRect( pDis->hDC, &pDis->rcItem, ::GetSysColorBrush( COLOR_WINDOW ) );

	// アイテムが選択状態
	if( pDis->itemState & ODS_SELECTED ){
		rc = pDis->rcItem;
		rc.left += (rc.bottom - rc.top);
		::FillRect( pDis->hDC, &rc, ::GetSysColorBrush( COLOR_HIGHLIGHT ) );
	}

	// アイテムにフォーカスがある
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &pDis->rcItem );
	}

	// チェックボックス表示
	rc = pDis->rcItem;
	rc.top += 2;
	rc.bottom -= 2;
	rc.left += 2;
	rc.right = rc.left + (rc.bottom - rc.top);
	UINT uState =  DFCS_BUTTONCHECK | DFCS_FLAT;
	if( TRUE == (BOOL)pDis->itemData )
		uState |= DFCS_CHECKED;		// チェック状態
	::DrawFrameControl( pDis->hDC, &rc, DFC_BUTTON, uState );

	// 色見本矩形
	rc = pDis->rcItem;
	rc.left += rc.bottom - rc.top + 2;
	rc.top += 2;
	rc.bottom -= 2;
	rc.right -= 2;
	hBrush = ::CreateSolidBrush( cr );
	hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
	hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_3DSHADOW ) );
	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
	::RoundRect( pDis->hDC, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
	::SelectObject( pDis->hDC, hPenOld );
	::SelectObject( pDis->hDC, hBrushOld );
	::DeleteObject( hPen );
	::DeleteObject( hBrush );

	return TRUE;
}

/*! 色選択リストの LBN_SELCHANGE 処理
	@date 2006.05.01 ryoji 新規作成
*/
BOOL CDlgSameColor::OnSelChangeListColors( HWND hwndCtl )
{
	// 色選択リストで現在フォーカスのある色について
	// タイプ別設定から同色の項目を取り出して項目リストに表示する
	HWND hwndListInfo;
	COLORREF cr;
	TCHAR szText[30];
	LPTSTR pszStop;
	int i;
	int j;

	hwndListInfo = ::GetDlgItem( m_hWnd, IDC_LIST_ITEMINFO );
	::SendMessage( hwndListInfo, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );

	i = ::SendMessage( hwndCtl, LB_GETCARETINDEX, (WPARAM)0, (LPARAM)0 );
	if( LB_ERR != i ){
		::SendMessage( hwndCtl, LB_GETTEXT, (WPARAM)i, (LPARAM)szText );
		cr = _tcstoul( szText, &pszStop, 10 );

		switch( m_wID )
		{
		case IDC_BUTTON_SAMETEXTCOLOR:
			for( j = 0; j < COLORIDX_LAST; ++j ){
				if( cr == m_pTypes->m_ColorInfoArr[j].m_colTEXT ){
					::SendMessage( hwndListInfo, LB_ADDSTRING, (WPARAM)0, (LPARAM)m_pTypes->m_ColorInfoArr[j].m_szName);
				}
			}
			break;

		case IDC_BUTTON_SAMEBKCOLOR:
			for( j = 0; j < COLORIDX_LAST; ++j ){
			if( 0 != (g_ColorAttributeArr[j].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
					continue;
				if( cr == m_pTypes->m_ColorInfoArr[j].m_colBACK ){
					::SendMessage( hwndListInfo, LB_ADDSTRING, (WPARAM)0, (LPARAM)m_pTypes->m_ColorInfoArr[j].m_szName);
				}
			}
			break;

		default:
			break;
		}
	}

	return TRUE;
}

/*! サブクラス化された指定色スタティックのウィンドウプロシージャ
	@date 2006.04.26 ryoji 新規作成
*/
LRESULT CALLBACK CDlgSameColor::ColorStatic_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hDC;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	RECT		rc;

	CDlgSameColor* pCDlgSameColor;
	pCDlgSameColor = (CDlgSameColor*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch( uMsg ){
	case WM_PAINT:
		// ウィンドウ描画
		PAINTSTRUCT ps;

		hDC = ::BeginPaint( hwnd, &ps );

		// 色見本矩形
		::GetClientRect( hwnd, &rc );
		rc.left += 2;
		rc.top += 2;
		rc.right -=2;
		rc.bottom -= 2;
		hBrush = ::CreateSolidBrush( pCDlgSameColor->m_cr );
		hBrushOld = (HBRUSH)::SelectObject( hDC, hBrush );
		hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_3DSHADOW ) );
		hPenOld = (HPEN)::SelectObject( hDC, hPen );
		::RoundRect( hDC, rc.left, rc.top, rc.right, rc.bottom, 5, 5 );
		::SelectObject( hDC, hPenOld );
		::SelectObject( hDC, hBrushOld );
		::DeleteObject( hPen );
		::DeleteObject( hBrush );

		::EndPaint( hwnd, &ps );
		return (LRESULT)0;

	case WM_ERASEBKGND:
		// 背景描画
		hDC = (HDC)wParam;
		::GetClientRect( hwnd, &rc );

		// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
		hBrush = (HBRUSH)::SendMessage( GetParent( hwnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hwnd );
		hBrushOld = (HBRUSH)::SelectObject( hDC, hBrush );
		::FillRect( hDC, &rc, hBrush );
		::SelectObject( hDC, hBrushOld );

		return (LRESULT)1;

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)pCDlgSameColor->m_wpColorStaticProc );
		pCDlgSameColor->m_wpColorStaticProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return CallWindowProc( (WNDPROC)pCDlgSameColor->m_wpColorStaticProc, hwnd, uMsg, wParam, lParam );
}

/*! サブクラス化された色選択リストのウィンドウプロシージャ
	@date 2006.04.26 ryoji 新規作成
*/
LRESULT CALLBACK CDlgSameColor::ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT po;
	RECT rcItem;
	RECT rc;
	int nItemNum;
	int i;

	CDlgSameColor* pCDlgSameColor;
	pCDlgSameColor = (CDlgSameColor*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch( uMsg ){
	case WM_LBUTTONUP:
		// マウスボタン下にある項目の選択／選択解除をトグルする
		po.x = LOWORD(lParam);	// horizontal position of cursor
		po.y = HIWORD(lParam);	// vertical position of cursor
		nItemNum = ::SendMessage( hwnd, LB_GETCOUNT, 0, 0 );
		for( i = 0; i < nItemNum; ++i ){
			::SendMessage( hwnd, LB_GETITEMRECT, i, (LPARAM)&rcItem );
			rc = rcItem;
			rc.top += 2;
			rc.bottom -= 2;
			rc.left += 2;
			rc.right = rc.left + (rc.bottom - rc.top);
			if( ::PtInRect( &rc, po ) ){
				BOOL bCheck;
				bCheck = !(BOOL)::SendMessage( hwnd, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0 );
				::SendMessage( hwnd, LB_SETITEMDATA, (WPARAM)i, (LPARAM)bCheck );
				::InvalidateRect( hwnd, &rcItem, TRUE );
				break;
			}
		}
		break;

	case WM_KEYUP:
		// フォーカス項目の選択／選択解除をトグルする
		if( VK_SPACE == wParam ){
			BOOL bCheck;
			i = ::SendMessage( hwnd, LB_GETCARETINDEX, (WPARAM)0, (LPARAM)0 );
			if( LB_ERR != i ){
				bCheck = !(BOOL)::SendMessage( hwnd, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0 );
				::SendMessage( hwnd, LB_SETITEMDATA, (WPARAM)i, (LPARAM)bCheck );
				::InvalidateRect( hwnd, NULL, TRUE );
			}
		}
		break;

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)pCDlgSameColor->m_wpColorListProc );
		pCDlgSameColor->m_wpColorListProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return ::CallWindowProc( (WNDPROC)pCDlgSameColor->m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}

/*[EOF]*/
