/*! @file
	@brief ウィンドウの位置と大きさダイアログ

	@author Moca
	@date 2004/05/13 作成
*/
/*
	Copyright (C) 2004, genta, Moca
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
#include "dlg/CDlgWinSize.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,						HIDOK_WINSIZE,				// 閉じる
	IDC_BUTTON_HELP,			HIDC_BUTTON_WINSIZE_HELP,	// ヘルプ
	IDC_EDIT_WX,				HIDC_EDIT_WX,				// 幅
	IDC_EDIT_WY,				HIDC_EDIT_WY,				// 高さ
	IDC_EDIT_SX,				HIDC_EDIT_SX,				// X座標
	IDC_EDIT_SY,				HIDC_EDIT_SY,				// Y座標
//	IDC_CHECK_WINPOS,			HIDC_CHECK_WINPOS,
	IDC_RADIO_WINSIZE_DEF,		HIDC_RADIO_WINSIZE_DEF,		// 大きさ/指定しない
	IDC_RADIO_WINSIZE_SAVE,		HIDC_RADIO_WINSIZE_SAVE,	// 大きさ/継承する
	IDC_RADIO_WINSIZE_SET,		HIDC_RADIO_WINSIZE_SET,		// 大きさ/直接指定
	IDC_RADIO_WINPOS_DEF,		HIDC_RADIO_WINPOS_DEF,		// 位置/指定しない
	IDC_RADIO_WINPOS_SAVE,		HIDC_RADIO_WINPOS_SAVE, 	// 位置/継承する
	IDC_RADIO_WINPOS_SET,		HIDC_RADIO_WINPOS_SET,  	// 位置/直接指定
	IDC_COMBO_WINTYPE,			HIDC_COMBO_WINTYPE,
	0, 0
};

CDlgWinSize::CDlgWinSize()
{
	return;
}

CDlgWinSize::~CDlgWinSize()
{
	return;
}


// !モーダルダイアログの表示
int CDlgWinSize::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	EWinSizeMode&	eSaveWinSize,	//!< [in,out] ウィンドウ位置継承
	EWinSizeMode&	eSaveWinPos,	//!< [in,out] ウィンドウサイズ継承
	int&			nWinSizeType,	//!< [in,out] ウィンドウの実行時の大きさ
	RECT&			rc				//!< [in,out] 幅、高さ、左、上
)
{
	m_eSaveWinSize = eSaveWinSize;
	m_eSaveWinPos  = eSaveWinPos;
	m_nWinSizeType = nWinSizeType;
	m_rc = rc;
	(void)CDialog::DoModal( hInstance, hwndParent, IDD_WINPOSSIZE, (LPARAM)NULL );
	eSaveWinSize = m_eSaveWinSize;
	eSaveWinPos  = m_eSaveWinPos;
	nWinSizeType = m_nWinSizeType;
	rc = m_rc;
	return TRUE;
}

/*! 初期化処理
*/
BOOL CDlgWinSize::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	Combo_AddString( GetItemHwnd( IDC_COMBO_WINTYPE ), LSW( STR_DLGWINSZ_NORMAL ) );	//L"普通"
	Combo_AddString( GetItemHwnd( IDC_COMBO_WINTYPE ), LSW( STR_DLGWINSZ_MAXIMIZE ) );	//L"最大化"
	Combo_AddString( GetItemHwnd( IDC_COMBO_WINTYPE ), LSW( STR_DLGWINSZ_MINIMIZE ) );	//L"(最小化)"

	UpDown_SetRange( GetItemHwnd( IDC_SPIN_SX ), 30000, 0 );
	UpDown_SetRange( GetItemHwnd( IDC_SPIN_SY ), 30000, 0 );
	// ウィンドウの座標は、マイナス値も有効。
	UpDown_SetRange( GetItemHwnd( IDC_SPIN_WX ), 30000, -30000 );
	UpDown_SetRange( GetItemHwnd( IDC_SPIN_WY ), 30000, -30000 );

	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgWinSize::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:	// 2006/09/09 novice id修正
		MyWinHelp( GetHwnd(), HELP_CONTEXT, HLP000286 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDC_RADIO_WINPOS_DEF:
	case IDC_RADIO_WINPOS_SAVE:
	case IDC_RADIO_WINPOS_SET:
	case IDC_RADIO_WINSIZE_DEF:
	case IDC_RADIO_WINSIZE_SAVE:
	case IDC_RADIO_WINSIZE_SET:
		RenewItemState();
		return TRUE;
	case IDOK:
	case IDCANCEL:
		GetData();
	}
	return CDialog::OnBnClicked( wID );
}

/*! @brief ダイアログボックスにデータを設定
*/
void CDlgWinSize::SetData( void )
{
	switch( m_eSaveWinSize ){
	case 1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_SAVE, TRUE );
		break;
	case 2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_SET, TRUE );
		break;
	default:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINSIZE_DEF, TRUE );
	}

	switch( m_eSaveWinPos ){
	case 1:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_SAVE, TRUE );
		break;
	case 2:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_SET, TRUE );
		break;
	default:
		::CheckDlgButton( GetHwnd(), IDC_RADIO_WINPOS_DEF, TRUE );
	}

	int nCurIdx = 0;
	switch( m_nWinSizeType ){
	case SIZE_MINIMIZED:
		nCurIdx = 2;
		break;
	case SIZE_MAXIMIZED:
		nCurIdx = 1;
		break;
	default:
		nCurIdx = 0;
	}
	Combo_SetCurSel( GetItemHwnd( IDC_COMBO_WINTYPE ), nCurIdx );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_SX, m_rc.right,  TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_SY, m_rc.bottom, TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_WX, m_rc.top,  TRUE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_WY, m_rc.left, TRUE );
	RenewItemState();
}


/*! ダイアログボックスのデータを読み出す
*/
int CDlgWinSize::GetData( void )
{
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_DEF ) ){
		m_eSaveWinSize = WINSIZEMODE_DEF;
	}
	else if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SAVE ) ){
		m_eSaveWinSize = WINSIZEMODE_SAVE;
	}
	else if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SET ) ){
		m_eSaveWinSize = WINSIZEMODE_SET;
	}
	
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_DEF ) ){
		m_eSaveWinPos = WINSIZEMODE_DEF;
	}
	else if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SAVE ) ){
		m_eSaveWinPos = WINSIZEMODE_SAVE;
	}
	else if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SET ) ){
		m_eSaveWinPos = WINSIZEMODE_SET;
	}

	int nCurIdx;
	nCurIdx = Combo_GetCurSel( GetItemHwnd( IDC_COMBO_WINTYPE ) );
	switch( nCurIdx ){
	case 2:
		m_nWinSizeType = SIZE_MINIMIZED;
		break;
	case 1:
		m_nWinSizeType = SIZE_MAXIMIZED;
		break;
	default:
		m_nWinSizeType = SIZE_RESTORED;
	}
	m_rc.right  = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_SX, NULL, TRUE );
	m_rc.bottom = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_SY, NULL, TRUE );
	m_rc.top    = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_WX, NULL, TRUE );
	m_rc.left   = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_WY, NULL, TRUE );
	return TRUE;
}


/*! 利用可能・不可の状態を更新する
*/
void CDlgWinSize::RenewItemState( void )
{
	BOOL state;
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINPOS_SET ) ){
		state = TRUE;
	}else{
		state = FALSE;
	}
	::EnableWindow( GetItemHwnd( IDC_EDIT_WX ), state );
	::EnableWindow( GetItemHwnd( IDC_EDIT_WY ), state );

	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_WINSIZE_SET ) ){
		state = TRUE;
	}else{
		state = FALSE;
	}
	::EnableWindow( GetItemHwnd( IDC_COMBO_WINTYPE ), state );
	::EnableWindow( GetItemHwnd( IDC_EDIT_SX ), state );
	::EnableWindow( GetItemHwnd( IDC_EDIT_SY ), state );
}

LPVOID CDlgWinSize::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


