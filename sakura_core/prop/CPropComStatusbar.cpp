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

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUCで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UTF8_CODEPOINT,		HIDC_CHECK_DISP_UTF8_CODEPOINT,			// UTF-8をコードポイントで表示する
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// サロゲートペアをコードポイントで表示する
	IDC_CHECK_DISP_SELCOUNT_BY_BYTE,	HIDC_CHECK_DISP_SELCOUNT_BY_BYTE,		// 選択文字数を文字単位ではなくバイト単位で表示する
	IDC_CHECK_DISP_COL_BY_CHAR,			HIDC_CHECK_DISP_COL_BY_CHAR,			// 現在桁をルーラー単位ではなく文字単位で表示する
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropStatusbar::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropStatusbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/* メッセージ処理 */
INT_PTR CPropStatusbar::DispatchEvent(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	NMHDR*		pNMHDR;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		return TRUE;
	case WM_COMMAND:
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_STATUSBAR );
			return TRUE;
		case PSN_KILLACTIVE:
			DEBUG_TRACE( _T("statusbar PSN_KILLACTIVE\n") );

			/* ダイアログデータの取得 */
			GetData( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			m_nPageNum = ID_PROPCOM_PAGENUM_STATUSBAR;
			return TRUE;
		}
		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/* ダイアログデータの設定 */
void CPropStatusbar::SetData( HWND hwndDlg )
{
	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, m_Common.m_sStatusbar.m_bDispUniInSjis );
	// JISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  m_Common.m_sStatusbar.m_bDispUniInJis );
	// EUCで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  m_Common.m_sStatusbar.m_bDispUniInEuc );
	// UTF-8で表示をバイトコードで行う
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT,  m_Common.m_sStatusbar.m_bDispUtf8Codepoint );
	// サロゲートペアをコードポイントで表示
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    m_Common.m_sStatusbar.m_bDispSPCodepoint );
	// 選択文字数を文字単位ではなくバイト単位で表示する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE,m_Common.m_sStatusbar.m_bDispSelCountByByte );
	// 現在桁をルーラー単位ではなく文字単位で表示する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_COL_BY_CHAR,     m_Common.m_sStatusbar.m_bDispColByChar );
	return;
}


/* ダイアログデータの取得 */
int CPropStatusbar::GetData( HWND hwndDlg )
{
	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInSjis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS );
	// JISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInJis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS );
	// EUCで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInEuc		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC );
	// UTF-8で表示をバイトコードで行う
	m_Common.m_sStatusbar.m_bDispUtf8Codepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT );
	// サロゲートペアをコードポイントで表示
	m_Common.m_sStatusbar.m_bDispSPCodepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT );
	// 選択文字数を文字単位ではなくバイト単位で表示する
	m_Common.m_sStatusbar.m_bDispSelCountByByte	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE );
	// 現在桁をルーラー単位ではなく文字単位で表示する
	m_Common.m_sStatusbar.m_bDispColByChar		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_COL_BY_CHAR );

	return TRUE;
}
