/*!	@file
	@brief 共通設定ダイアログボックス、「ステータスバー」ページ

	@author Uchi
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta
	Copyright (C) 2007, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "prop/CPropCommon.h"
#include "debug/Debug.h" // 2002/2/10 aroka
#include "util/shell.h"


#include "sakura.hh"
static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJISで文字コード値をUnicodeで出力する
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JISで文字コード値をUnicodeで出力する
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUCで文字コード値をUnicodeで出力する
	IDC_CHECK_DISP_UTF8_BYTE,			HIDC_CHECK_DISP_UTF8_BYTE,				// UTF-8で表示をバイトコードで行う
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// サロゲートペアをコードポイントで表示
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_STATUSBAR(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_PROP_STATUSBAR, hwndDlg, uMsg, wParam, lParam );
}

/* メッセージ処理 */
INT_PTR CPropCommon::DispatchEvent_PROP_STATUSBAR(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 */
		SetData_PROP_STATUSBAR( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_DRAGDROP:	/* タスクトレイを使う */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
				}
				else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
				}
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_STATUSBAR );
			return TRUE;
		case PSN_KILLACTIVE:
			DBPRINT_A( "statusbar PSN_KILLACTIVE\n" );

			/* ダイアログデータの取得 */
			GetData_PROP_STATUSBAR( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			m_nPageNum = ID_PAGENUM_STATUSBAR;	//Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更(参照しているのはCPropCommon.cppのみの1箇所)
			return TRUE;
		}
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/* ダイアログデータの設定 */
void CPropCommon::SetData_PROP_STATUSBAR( HWND hwndDlg )
{
	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, m_Common.m_sStatusbar.m_bDispUniInSjis );
	// JISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  m_Common.m_sStatusbar.m_bDispUniInJis );
	// EUCで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  m_Common.m_sStatusbar.m_bDispUniInEuc );
	// UTF-8で表示をバイトコードで行う
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UTF8_BYTE,       m_Common.m_sStatusbar.m_bDispUtf8Byte );
	// サロゲートペアをコードポイントで表示
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    m_Common.m_sStatusbar.m_bDispSPCodepoint );
	return;
}


/* ダイアログデータの取得 */
int CPropCommon::GetData_PROP_STATUSBAR( HWND hwndDlg )
{
	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInSjis   = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS );
	// JISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInJis    = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS );
	// EUCで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInEuc    = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC );
	// UTF-8で表示をバイトコードで行う
	m_Common.m_sStatusbar.m_bDispUtf8Byte    = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UTF8_BYTE );
	// サロゲートペアをコードポイントで表示
	m_Common.m_sStatusbar.m_bDispSPCodepoint = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT );

	return TRUE;
}
