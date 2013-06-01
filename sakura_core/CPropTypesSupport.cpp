/*!	@file
	@brief タイプ別設定 - 支援

	@author Norio Nakatani
	@date 1998/12/24  新規作成
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, こおり, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "Debug.h"
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "CDlgKeywordSelect.h"
#include "etc_uty.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "Funccode.h"	//Stonee, 2001/05/18
#include "CDlgSameColor.h"	// 2006.04.26 ryoji
#include "CEditApp.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//単語ファイル名
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//入力補完 単語ファイル参照
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//入力補完の英大文字小文字
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//現在のファイルから入力補完

	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//外部ヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//外部ヘルプファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//外部HTMLヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//外部HTMLヘルプファイル参照	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//ビューアを複数起動しない	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//デフォルト文字コード
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//デフォルト改行コード	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//デフォルトBOM	// 2011.01.24 ryoji
//	IDC_STATIC,						-1,
	0, 0
};

static const TCHAR* aszCodeStr[] = {
	_T("SJIS"),
	_T("EUC"),
	_T("UTF-8"),
	_T("Unicode"),
	_T("UnicodeBE")
};
static const ECodeType aeCodeType[] = {
	CODE_SJIS,
	CODE_EUC,
	CODE_UTF8,
	CODE_UNICODE,
	CODE_UNICODEBE
};
static const BOOL abBomEnable[] = {
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE
};
static const TCHAR* aszEolStr[] = {
	_T("CR+LF"),
	_T("LF (UNIX)"),
	_T("CR (Mac)"),
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

/* メッセージ処理 */
INT_PTR CPropTypesSupport::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p2 */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 入力補完 単語ファイル */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// 文字コードの変更をBOMチェックボックスに反映
					i = ::SendMessage((HWND) lParam, CB_GETCURSEL, 0L, 0L);
					if( CB_ERR != i ){
						int nCheck = BST_UNCHECKED;
						if( abBomEnable[i] ){
							if( (aeCodeType[i] == CODE_UNICODE || aeCodeType[i] == CODE_UNICODEBE) )
								nCheck = BST_CHECKED;
						}
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, nCheck );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );
					}
					break;
				}
			}
			break;

		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完 単語ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szHokanFile ) ){
						GetInidirOrExedir( szPath, m_Types.m_szHokanFile );
					}else{
						_tcscpy( szPath, m_Types.m_szHokanFile );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szHokanFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* 外部ヘルプ１の「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szExtHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szExtHtmlHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHtmlHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHtmlHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHtmlHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:	//Jul. 03, 2001 JEPRO 支援タブのヘルプを有効化
			OnHelp( hwndDlg, IDD_PROP_SUPPORT );
			return TRUE;
		case PSN_KILLACTIVE:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = 2;
			return TRUE;
		}
		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* ダイアログデータの設定 */
void CPropTypesSupport::SetData( HWND hwndDlg )
{
	/* 入力補完 単語ファイル */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

//	2001/06/19 asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase );

	// 2003.06.25 Moca ファイルからの補完機能
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED );

	//@@@ 2002.2.2 YAZAKI
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED );

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// デフォルトコードタイプのコンボボックス設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		for( i = 0; i < _countof(aszCodeStr); ++i ){
			::SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)aszCodeStr[i] );
		}
		for( i = 0; i < _countof(aeCodeType); ++i ){
			if( m_Types.m_eDefaultCodetype == aeCodeType[i] ){
				break;
			}
		}
		if( i == _countof(aeCodeType) ){
			i = 0;
		}
		::SendMessage(hCombo, CB_SETCURSEL, (WPARAM)i, 0L);

		// BOM チェックボックス設定
		if( !abBomEnable[i] )
			m_Types.m_bDefaultBom = false;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (m_Types.m_bDefaultBom ? BST_CHECKED : BST_UNCHECKED) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), abBomEnable[i] );

		// デフォルト改行タイプのコンボボックス設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			::SendMessage( hCombo, CB_ADDSTRING, 0, (LPARAM)aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		::SendMessage(hCombo, CB_SETCURSEL, (WPARAM)i, 0L);
	}
}

/* ダイアログデータの取得 */
int CPropTypesSupport::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 2;

//	2001/06/19	asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE ) != 0;

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE ) != 0;

	/* 入力補完 単語ファイル */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, sizeof( m_Types.m_szHokanFile ));

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// m_Types.eDefaultCodetype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = ::SendMessage(hCombo, CB_GETCURSEL, 0L, 0L);
		if( CB_ERR != i ){
			m_Types.m_eDefaultCodetype = aeCodeType[i];
		}

		// m_Types.m_bDefaultBom を設定
		m_Types.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM ) != 0;

		// m_Types.eDefaultEoltype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = ::SendMessage(hCombo, CB_GETCURSEL, 0L, 0L);
		if( CB_ERR != i ){
			m_Types.m_eDefaultEoltype = aeEolType[i];
		}
	}

	//@@@ 2002.2.2 YAZAKI
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, sizeof( m_Types.m_szExtHelp ));
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, sizeof( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE ) != 0;

	return TRUE;
}

// 2001/06/13 End

/*[EOF]*/
