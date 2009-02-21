/*!	@file
	@brief 外部コマンド実行ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, maru

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "dlg/CDlgExec.h"
#include "func/Funccode.h"	//Stonee, 2001/03/12  コメントアウトされてたのを有効にした
#include "sakura_rc.h"
#include <windows.h>		//Mar. 28, 2001 JEPRO (一応入れたが不要？)
#include <stdio.h>			//Mar. 28, 2001 JEPRO (一応入れたが不要？)
#include <commctrl.h>		//Mar. 28, 2001 JEPRO
#include "dlg/CDlgOpenFile.h"	//Mar. 28, 2001 JEPRO
#include "debug/Debug.h"// 2002/2/10 aroka ヘッダ整理
#include "util/shell.h"

//外部コマンド CDlgExec.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		//参照
	IDOK,							HIDOK_EXEC,						//実行
	IDCANCEL,						HIDCANCEL_EXEC,					//キャンセル
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			//ヘルプ
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		//標準出力を得る
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	//コマンド
	IDC_RADIO_OUTPUT,				HIDC_RADIO_OUTPUT,				//標準出力リダイレクト先：アウトプットウィンドウ
	IDC_RADIO_EDITWINDOW,			HIDC_RADIO_EDITWINDOW,			//標準出力リダイレクト先：編集中のウィンドウ
	IDC_CHECK_SENDSTDIN,			HIDC_CHECK_SENDSTDIN,			//標準入力に送る
	IDC_CHECK_UNICODE_GET,			HIDC_CHECK_UNICODE_GET,			//Unicodeで標準出力	// 2008/6/17 Uchi
	IDC_CHECK_UNICODE_SEND,			HIDC_CHECK_UNICODE_SEND,		//Unicodeで標準入力	// 2008/6/20 Uch
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgExec::CDlgExec()
{
	m_szCommand[0] = _T('\0');	/* コマンドライン */
//	m_bGetStdout = /*FALSE*/TRUE;	// 標準出力を得る	//Mar. 21, 2001 JEPRO [得ない]をデフォルトに変更	//Jul. 03, 2001 JEPRO [得る]がデフォルトとなるように戻した

	return;
}




/* モーダルダイアログの表示 */
int CDlgExec::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_szCommand[0] = _T('\0');	/* コマンドライン */
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_EXEC, lParam );
}




/* ダイアログデータの設定 */
void CDlgExec::SetData( void )
{
//	MYTRACE_A( "CDlgExec::SetData()" );
	int		i;
	HWND	hwndCombo;

	/*****************************
	*           初期             *
	*****************************/
	/* ユーザーがコンボ ボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand ), CB_LIMITTEXT, _countof( m_szCommand ) - 1, 0 );
	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessageAny( ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand ), CB_SETEXTENDEDUI, TRUE, 0 );


	// 標準出力を得る
//	From Here Sept. 12, 2000 jeprotest
//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動

	//	::CheckDlgButton( GetHwnd(), IDC_CHECK_GETSTDOUT, m_pShareData->m_bGetStdout/*m_bGetStdout*/ ? BST_CHECKED : BST_UNCHECKED );
	//	::CheckDlgButton( GetHwnd(), IDC_CHECK_GETSTDOUT, TRUE );

//	To Here Sept. 12, 2000 	うまくいかないので元に戻してある

	{	//	From Here 2007.01.02 maru 引数を拡張のため
		//	マクロからの呼び出しではShareDataに保存させないように，ShareDataとの受け渡しはExecCmdの外で
		int nExecFlgOpt;
		nExecFlgOpt = m_pShareData->m_nExecFlgOpt;
		
		::CheckDlgButton( GetHwnd(), IDC_CHECK_GETSTDOUT, nExecFlgOpt & 0x01 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUT, nExecFlgOpt & 0x02 ? BST_UNCHECKED : BST_CHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_EDITWINDOW, nExecFlgOpt & 0x02 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_CHECK_SENDSTDIN, nExecFlgOpt & 0x04 ? BST_CHECKED : BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_CHECK_UNICODE_GET, nExecFlgOpt & 0x08 ? BST_CHECKED : BST_UNCHECKED );		// 2008/6/17 Uchi
		::CheckDlgButton( GetHwnd(), IDC_CHECK_UNICODE_SEND, nExecFlgOpt & 0x10 ? BST_CHECKED : BST_UNCHECKED );	// 2008/6/20 Uchi

		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_OUTPUT ), nExecFlgOpt & 0x01 ? TRUE : FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_EDITWINDOW ), nExecFlgOpt & 0x01 ? TRUE : FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_UNICODE_GET ), nExecFlgOpt & 0x01 ? TRUE : FALSE );		// 標準出力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_UNICODE_SEND ), nExecFlgOpt & 0x04 ? TRUE : FALSE );		// 標準入力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
	}	//	To Here 2007.01.02 maru 引数を拡張のため

	/*****************************
	*         データ設定         *
	*****************************/
	_tcscpy( m_szCommand, m_pShareData->m_sHistory.m_aCommands[0] );
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_m_szCommand );
	::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_szCommand );
	for( i = 0; i < m_pShareData->m_sHistory.m_aCommands.size(); ++i ){
		Combo_AddString( hwndCombo, m_pShareData->m_sHistory.m_aCommands[i] );
	}
	::SendMessageAny( hwndCombo, CB_SETCURSEL, 0, 0 );
	return;
}




/* ダイアログデータの取得 */
int CDlgExec::GetData( void )
{
	DlgItem_GetText( GetHwnd(), IDC_COMBO_m_szCommand, m_szCommand, _countof( m_szCommand ));
	{	//	From Here 2007.01.02 maru 引数を拡張のため
		//	マクロからの呼び出しではShareDataに保存させないように，ShareDataとの受け渡しはExecCmdの外で
		int nFlgOpt = 0;
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT ) ) ? 0x01 : 0;	// 標準出力を得る
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_EDITWINDOW ) ) ? 0x02 : 0;	// 標準出力を編集中のウインドウへ
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SENDSTDIN ) ) ? 0x04 : 0;	// 編集中ファイルを標準入力へ
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_UNICODE_GET ) ) ? 0x08 : 0;	// Unicodeで標準出力	2008/6/17 Uchi
		nFlgOpt |= ( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_UNICODE_SEND ) ) ? 0x10 : 0;	// Unicodeで標準入力	2008/6/20 Uchi
		m_pShareData->m_nExecFlgOpt = nFlgOpt;
	}	//	To Here 2007.01.02 maru 引数を拡張のため
	return 1;
}



BOOL CDlgExec::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_CHECK_GETSTDOUT:
		{	//	From Here 2007.01.02 maru 引数を拡張のため
			BOOL bEnabled;
			bEnabled = (BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT)) ? TRUE : FALSE;
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_OUTPUT ), bEnabled );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_EDITWINDOW ), bEnabled );
		}	//	To Here 2007.01.02 maru 引数を拡張のため

		// 標準出力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
		::EnableWindow(
			::GetDlgItem( GetHwnd(), IDC_CHECK_UNICODE_GET ), 
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_GETSTDOUT )
		);
		break;
	case IDC_CHECK_SENDSTDIN:	// 標準入力Off時、Unicodeを使用するをDesableする	2008/6/20 Uchi
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_UNICODE_SEND ), 
			BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_SENDSTDIN ) );
		break;

	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECMD_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		break;

	//From Here Mar. 28, 2001 JEPRO
	case IDC_BUTTON_REFERENCE:	/* ファイル名の「参照...」ボタン */
		{
			CDlgOpenFile	cDlgOpenFile;
			TCHAR			szPath[_MAX_PATH + 1];
			_tcscpy( szPath, m_szCommand );
			/* ファイルオープンダイアログの初期化 */
			cDlgOpenFile.Create(
				m_hInstance,
				GetHwnd(),
				_T("*.com;*.exe;*.bat"),
				m_szCommand
			);
			if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
				_tcscpy( m_szCommand, szPath );
				::DlgItem_SetText( GetHwnd(), IDC_COMBO_m_szCommand, m_szCommand );
			}
		}
		return TRUE;
	//To Here Mar. 28, 2001

	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		GetData();
		CloseDialog( 1 );
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgExec::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


