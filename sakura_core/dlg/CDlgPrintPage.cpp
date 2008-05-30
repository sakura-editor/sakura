/*!	@file
	@brief 印刷ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, aroka, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "dlg/CDlgPrintPage.h"
#include "sakura_rc.h"
#include "debug/Debug.h"// 2002/2/10 aroka

// 印刷 CDlgPrintPage.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12400
	IDOK,							HIDOK_PP,				//OK
	IDCANCEL,						HIDCANCEL_PP,			//キャンセル
	IDC_EDIT_FROM,					HIDC_PP_EDIT_FROM,		//ページから
	IDC_EDIT_TO,					HIDC_PP_EDIT_TO,		//ページまで
	IDC_RADIO_ALL,					HIDC_PP_RADIO_ALL,		//すべて
	IDC_RADIO_FROMTO,				HIDC_PP_RADIO_FROMTO,	//指定
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgPrintPage::CDlgPrintPage()
{
	return;
}




/* モーダルダイアログの表示 */
int CDlgPrintPage::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTPAGE, lParam );
}




/* ダイアログデータの設定 */
void CDlgPrintPage::SetData( void )
{
	TCHAR szText[100];
//	From Here Sept. 12, 2000 JEPRO スタイルを少し変更(指定を選んだ時だけ印刷範囲を入力できるようにした)
//	一度設定したページ設定値を保存できるようにしたいがまだできてない
//	::CheckDlgButton( GetHwnd(), IDC_RADIO_ALL, m_bAllPage?BST_CHECKED:BST_UNCHECKED );
//	auto_sprintf( szText, "%d ～ %d 頁", m_nPageMin, m_nPageMax );
//	::DlgItem_SetText( GetHwnd(), IDC_STATIC_ALL, szText );
//
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, m_nPageFrom, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_TO, m_nPageTo, FALSE );
//
	if( TRUE == m_bAllPage ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALL, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_FROMTO, BST_CHECKED );
	}
	auto_sprintf( szText, _T("%d ～ %d ページ"), m_nPageMin, m_nPageMax );
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_ALL, szText );

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, m_nPageFrom, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_TO, m_nPageTo, FALSE );

	//	印刷範囲を指定するかどうか
	if( !m_bAllPage ){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), FALSE );
	}
//	To Here Sept. 12, 2000

	return;
}




/* ダイアログデータの取得 */
int CDlgPrintPage::GetData( void )
{
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALL ) ){
		m_bAllPage = TRUE;
	}else{
		m_bAllPage = FALSE;
	}
	m_nPageFrom = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, NULL, FALSE );
	m_nPageTo	= ::GetDlgItemInt( GetHwnd(), IDC_EDIT_TO  , NULL, FALSE );
	/* 頁範囲チェック */
	if( !m_bAllPage ){
		if( m_nPageMin <= m_nPageFrom && m_nPageFrom <= m_nPageMax &&
			m_nPageMin <= m_nPageTo   && m_nPageTo <= m_nPageMax &&
			m_nPageFrom <= m_nPageTo
		){
		}else{
			::MYMESSAGEBOX(	GetHwnd(), MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("入力エラー"), _T("ページ範囲指定が正しくありません。") );
			return FALSE;
		}
	}
	return TRUE;
}




BOOL CDlgPrintPage::OnBnClicked( int wID )
{
	switch( wID ){
//	From Here Sept. 12, 2000 JEPRO スタイルを少し変更(指定を選んだ時だけ印刷範囲を入力できるようにした)
//	一度設定したページ設定値を保存できるようにしたいがまだできてない
	case IDC_RADIO_ALL:
	case IDC_RADIO_FROMTO:
	//	印刷範囲を指定するかどうか
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_FROMTO ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), FALSE );
		}
		return TRUE;
//	To Here Sept. 12, 2000

	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		if( GetData() ){
			CloseDialog( 1 );
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgPrintPage::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


