//	$Id$
/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include <windows.h>


#include "sakura_rc.h"
#include "CDlgGrep.h"
#include "debug.h"

#include "etc_uty.h"
#include "global.h"
#include "funccode.h"		// Stonee, 2001/03/12

//GREP CDlgGrep.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12000
	IDC_BUTTON_FOLDER,				HIDC_GREP_BUTTON_FOLDER,			//フォルダ
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_BUTTON_CURRENTFOLDER,		//現フォルダ
	IDOK,							HIDOK_GREP,							//検索
	IDCANCEL,						HIDCANCEL_GREP,						//キャンセル
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				//ヘルプ
	IDC_CHK_WORD,					HIDC_GREP_CHK_WORD,					//単語単位
	IDC_CHK_SUBFOLDER,				HIDC_GREP_CHK_SUBFOLDER,			//サブフォルダも検索
	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_CHK_FROMTHISTEXT,			//このファイルから
	IDC_CHK_LOHICASE,				HIDC_GREP_CHK_LOHICASE,				//大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_GREP_CHK_REGULAREXP,			//正規表現
	IDC_COMBO_CHARSET,				HIDC_GREP_COMBO_CHARSET,			//文字コードセット
	IDC_COMBO_TEXT,					HIDC_GREP_COMBO_TEXT,				//条件
	IDC_COMBO_FILE,					HIDC_GREP_COMBO_FILE,				//ファイル
	IDC_COMBO_FOLDER,				HIDC_GREP_COMBO_FOLDER,				//フォルダ
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_RADIO_OUTPUTLINE,			//結果出力：行単位
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_RADIO_OUTPUTMARKED,		//結果出力：該当部分
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_RADIO_OUTPUTSTYLE1,		//結果出力形式：ノーマル
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_RADIO_OUTPUTSTYLE2,		//結果出力形式：ファイル毎
	IDC_STATIC_JRE32VER,			HIDC_GREP_STATIC_JRE32VER,			//正規表現バージョン
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_CHK_DEFAULTFOLDER,		//フォルダの初期値をカレントフォルダにする
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgGrep::CDlgGrep()
{
	m_bSubFolder = FALSE;				/* サブフォルダからも検索する */
	m_bFromThisText = FALSE;			/* この編集中のテキストから検索する */
	m_bLoHiCase = FALSE;				/* 英大文字と英小文字を区別する */
	m_bRegularExp = FALSE;				/* 正規表現 */
	m_nGrepCharSet = CODE_SJIS;			/* 文字コードセット */
	m_bGrepOutputLine = TRUE;			/* 行を出力するか該当部分だけ出力するか */
	m_nGrepOutputStyle = 1;				/* Grep: 出力形式 */

	strcpy( m_szText, m_pShareData->m_szSEARCHKEYArr[0] );		/* 検索文字列 */
	strcpy( m_szFile, m_pShareData->m_szGREPFILEArr[0] );		/* 検索ファイル */
	strcpy( m_szFolder, m_pShareData->m_szGREPFOLDERArr[0] );	/* 検索フォルダ */
	return;
}



/* モーダルダイアログの表示 */
int CDlgGrep::DoModal( HINSTANCE hInstance, HWND hwndParent, const char* pszCurrentFilePath )
{
	m_bSubFolder = m_pShareData->m_Common.m_bGrepSubFolder;							/* Grep: サブフォルダも検索 */
	m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;							/* 1==正規表現 */
	m_nGrepCharSet = m_pShareData->m_Common.m_nGrepCharSet;							/* 文字コードセット */
	m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;								/* 1==大文字小文字の区別 */
	m_bGrepOutputLine = m_pShareData->m_Common.m_bGrepOutputLine;					/* 行を出力するか該当部分だけ出力するか */
	m_nGrepOutputStyle = m_pShareData->m_Common.m_nGrepOutputStyle;					/* Grep: 出力形式 */

	//2001/06/23 N.Nakatani add
	m_bWordOnly = m_pShareData->m_Common.m_bWordOnly;					/* 単語単位で検索 */

	lstrcpy( m_szCurrentFilePath, pszCurrentFilePath );

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_GREP, NULL );
}

//	/* モードレスダイアログの表示 */
//	HWND CDlgGrep::DoModeless( HINSTANCE hInstance, HWND hwndParent, const char* pszCurrentFilePath )
//	{
//		m_bSubFolder = m_pShareData->m_Common.m_bGrepSubFolder;							/* Grep: サブフォルダも検索 */
//		m_bRegularExp = m_pShareData->m_Common.m_bRegularExp;							/* 1==正規表現 */
//		m_nGrepCharSet = m_pShareData->m_Common.m_nGrepCharSet;							/* 文字コードセット */
//		m_bLoHiCase = m_pShareData->m_Common.m_bLoHiCase;								/* 1==英大文字小文字の区別 */
//		m_bGrepOutputLine = m_pShareData->m_Common.m_bGrepOutputLine;					/* 行を出力するか該当部分だけ出力するか */
//		m_nGrepOutputStyle = m_pShareData->m_Common.m_nGrepOutputStyle;					/* Grep: 出力形式 */
//		lstrcpy( m_szCurrentFilePath, pszCurrentFilePath );
//
//		return CDialog::DoModeless( hInstance, hwndParent, IDD_GREP, NULL );
//	}


BOOL CDlgGrep::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;

	/* ユーザーがコンボボックスのエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), CB_LIMITTEXT, (WPARAM)_MAX_PATH - 1, 0 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
//	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

	/* ダイアログのアイコン */
//	::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)::LoadIcon( m_hInstance, IDI_QUESTION ) );
//2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
	::SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessage( m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

	// 2002/09/22 Moca Add
	int i;
	/* 文字コードセット選択コンボボックス初期化 */
	for( i = 0; i < gm_nCodeComboNameArrNum; ++i ){
		int idx = ::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET ), CB_ADDSTRING,   0, (LPARAM)gm_pszCodeComboNameArr[i] );
		::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET ), CB_SETITEMDATA, idx, gm_nCodeComboValueArr[i] );
	}

	/* 基底クラスメンバ */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}




BOOL CDlgGrep::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「Grep」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_DIALOG) );
		return TRUE;
	case IDC_CHK_FROMTHISTEXT:	/* この編集中のテキストから検索する */
		if( 0 < (int)lstrlen(m_szCurrentFilePath ) ){
			if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_FROMTHISTEXT ) ){
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_FOLDER ), FALSE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_SUBFOLDER ), FALSE );
				::CheckDlgButton( m_hWnd, IDC_CHK_SUBFOLDER, 0 );
			}else{
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), TRUE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), TRUE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_FOLDER ), TRUE );
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_SUBFOLDER ), TRUE );
			}
			char	szWorkFolder[MAX_PATH];
			char	szWorkFile[MAX_PATH];
			// 2003.08.01 Moca ファイル名はスペースなどは区切り記号になるので、""で囲い、エスケープする
			szWorkFile[0] = '"';
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile + 1 );
			strcat( szWorkFile, "\"" ); // 2003.08.01 Moca
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FILE, szWorkFile );
		}
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	/* 現在編集中のファイルのフォルダ */
		/* ファイルを開いているか */
		if( 0 < lstrlen( m_szCurrentFilePath ) ){
			char	szWorkFolder[MAX_PATH];
			char	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
		}else{
			/* 現在のプロセスのカレントディレクトリを取得します */
			char	szWorkFolder[MAX_PATH];
			::GetCurrentDirectory( sizeof( szWorkFolder ) - 1, szWorkFolder );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
//			::MessageBeep( MB_ICONEXCLAMATION );
		}
		return TRUE;


//	case IDC_CHK_LOHICASE:	/* 英大文字と英小文字を区別する */
//		MYTRACE( "IDC_CHK_LOHICASE\n" );
//		return TRUE;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( "IDC_CHK_REGULAREXP ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) = %d\n", ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
			}else{
				//	To Here Jun. 26, 2001 genta
				/* 英大文字と英小文字を区別する */
				//	正規表現のときも選択できるように。
//				::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

				//2001/06/23 N.Nakatani
				/* 単語単位で検索 */
				::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* 英大文字と英小文字を区別する */
			//	正規表現のときも選択できるように。
//			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), TRUE );
//			::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 0 );


//2001/06/23 N.Nakatani
//単語単位のgrepが実装されたらコメントを外すと思います
//2002/03/07実装してみた。
			/* 単語単位で検索 */
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), TRUE );

		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		/* フォルダ参照ボタン */
		{
			char	szFolder[MAX_PATH];
			/* 検索フォルダ */
			::GetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1 );
			if( 0 == lstrlen( szFolder ) ){
				::GetCurrentDirectory( sizeof( szFolder ), szFolder );
			}
			if( SelectDir( m_hWnd, "検索するフォルダを選んでください", szFolder, szFolder ) ){
				::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szFolder );
			}
		}

		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		/* フォルダの初期値をカレントフォルダにする */
		{
			m_pShareData->m_Common.m_bGrepDefaultFolder = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_DEFAULTFOLDER );
		}
		return TRUE;
	case IDOK:
		/* ダイアログデータの取得 */
		if( GetData() ){
//			::EndDialog( hwndDlg, TRUE );
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog( hwndDlg, FALSE );
		CloseDialog( FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}



/* ダイアログデータの設定 */
void CDlgGrep::SetData( void )
{
	int		i;
	HWND	hwndCombo;
//	char	szWorkPath[_MAX_PATH + 1];
//	m_hWnd = hwndDlg;	/* このダイアログのハンドル */

	m_pShareData = CShareData::getInstance()->GetShareData();

	/* 検索文字列 */
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT );
	for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szSEARCHKEYArr[i] );
	}

	/* 検索ファイル */
	::SetDlgItemText( m_hWnd, IDC_COMBO_FILE, m_szFile );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_FILE );
	for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szGREPFILEArr[i] );
	}

	/* 検索フォルダ */
	::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, m_szFolder );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER );
	for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szGREPFOLDERArr[i] );
	}

	if((0 == lstrlen( m_pShareData->m_szGREPFOLDERArr[0] ) || m_pShareData->m_Common.m_bGrepDefaultFolder ) &&
		0 < lstrlen( m_szCurrentFilePath )
	){
		char	szWorkFolder[MAX_PATH];
		char	szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
		::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
	}

	/* この編集中のテキストから検索する */
	::CheckDlgButton( m_hWnd, IDC_CHK_FROMTHISTEXT, m_bFromThisText );
	if( 0 < lstrlen( m_szCurrentFilePath ) ){
		if( m_bFromThisText ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_FOLDER ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_SUBFOLDER ), FALSE );
			char	szWorkFolder[MAX_PATH];
			char	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, szWorkFolder );
			::SetDlgItemText( m_hWnd, IDC_COMBO_FILE, szWorkFile );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FILE ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_FOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON_FOLDER ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_SUBFOLDER ), TRUE );
		}
	}

	/* サブフォルダからも検索する */
	::CheckDlgButton( m_hWnd, IDC_CHK_SUBFOLDER, m_bSubFolder );

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, m_bLoHiCase );

	// 2001/06/23 N.Nakatani 現時点ではGrepでは単語単位の検索はサポートできていません
	// 2002/03/07 テストサポート
	/* 一致する単語のみ検索する */
	::CheckDlgButton( m_hWnd, IDC_CHK_WORD, m_bWordOnly );
//	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ) , false );	//チェックボックスを使用不可にすも


	/* 文字コード自動判別 */
//	::CheckDlgButton( m_hWnd, IDC_CHK_KANJICODEAUTODETECT, m_bKanjiCode_AutoDetect );

	// 2002/09/22 Moca Add
	/* 文字コードセット */
	{
		int		nIdx, nCurIdx, nCharSet;
		HWND	hWndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET );
		nCurIdx = ::SendMessage( hWndCombo , CB_GETCURSEL, 0, 0 );
		for( nIdx = 0; nIdx < gm_nCodeComboNameArrNum; nIdx++ ){
			nCharSet = ::SendMessage( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
			if( nCharSet == m_nGrepCharSet ){
				nCurIdx = nIdx;
			}
		}
		::SendMessage( hWndCombo, CB_SETCURSEL, (WPARAM)nCurIdx, 0 );
	}

	/* 行を出力するか該当部分だけ出力するか */
	if( m_bGrepOutputLine ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTLINE, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTMARKED, TRUE );
	}

	/* Grep: 出力形式 */
	if( 1 == m_nGrepOutputStyle ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}else
	if( 2 == m_nGrepOutputStyle ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE2, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( m_hWnd, IDC_STATIC_JRE32VER, false )
		&& m_bRegularExp){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 1 );
		//	正規表現のときも選択できるように。
//		::CheckDlgButton( m_hWnd, IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( m_hWnd, IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	if( 0 < lstrlen( m_szCurrentFilePath ) ){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_FROMTHISTEXT ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_CHK_FROMTHISTEXT ), FALSE );
	}

	// フォルダの初期値をカレントフォルダにする
	::CheckDlgButton( m_hWnd, IDC_CHK_DEFAULTFOLDER, m_pShareData->m_Common.m_bGrepDefaultFolder );
	if( m_pShareData->m_Common.m_bGrepDefaultFolder ) OnBnClicked( IDC_BUTTON_CURRENTFOLDER );

	return;
}




/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー  */
int CDlgGrep::GetData( void )
{
//	int			i;
//	int			j;
//	CMemory*	pcmWork;

	m_pShareData = CShareData::getInstance()->GetShareData();

	/* サブフォルダからも検索する*/
	m_bSubFolder = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_SUBFOLDER );

	m_pShareData->m_Common.m_bGrepSubFolder = m_bSubFolder;		/* Grep：サブフォルダも検索 */

	/* この編集中のテキストから検索する */
	m_bFromThisText = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_FROMTHISTEXT );
	/* 英大文字と英小文字を区別する */
	m_bLoHiCase = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_LOHICASE );

	//2001/06/23 N.Nakatani
	/* 単語単位で検索 */
	m_bWordOnly = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_WORD );

	/* 正規表現 */
	m_bRegularExp = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_REGULAREXP );

	/* 文字コード自動判別 */
//	m_bKanjiCode_AutoDetect = ::IsDlgButtonChecked( m_hWnd, IDC_CHK_KANJICODEAUTODETECT );

	/* 文字コードセット */
	{
		int		nIdx;
		HWND	hWndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_CHARSET );
		nIdx = ::SendMessage( hWndCombo, CB_GETCURSEL, 0, 0 );
		m_nGrepCharSet = ::SendMessage( hWndCombo, CB_GETITEMDATA, nIdx, 0 );
	}


	/* 行を出力するか該当部分だけ出力するか */
	m_bGrepOutputLine = ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTLINE );

	/* Grep: 出力形式 */
	if( TRUE == ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTSTYLE1 ) ){
		m_nGrepOutputStyle = 1;				/* Grep: 出力形式 */
	}
	if( TRUE == ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_OUTPUTSTYLE2 ) ){
		m_nGrepOutputStyle = 2;				/* Grep: 出力形式 */
	}



	/* 検索文字列 */
	::GetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szText, _MAX_PATH - 1 );
	/* 検索ファイル */
	::GetDlgItemText( m_hWnd, IDC_COMBO_FILE, m_szFile, _MAX_PATH - 1 );
	/* 検索フォルダ */
	::GetDlgItemText( m_hWnd, IDC_COMBO_FOLDER, m_szFolder, _MAX_PATH - 1 );

	m_pShareData->m_Common.m_bRegularExp = m_bRegularExp;							/* 1==正規表現 */
	m_pShareData->m_Common.m_nGrepCharSet = m_nGrepCharSet;								/* 文字コード自動判別 */
	m_pShareData->m_Common.m_bLoHiCase = m_bLoHiCase;								/* 1==英大文字小文字の区別 */
	m_pShareData->m_Common.m_bGrepOutputLine = m_bGrepOutputLine;					/* 行を出力するか該当部分だけ出力するか */
	m_pShareData->m_Common.m_nGrepOutputStyle = m_nGrepOutputStyle;					/* Grep: 出力形式 */
	//2001/06/23 N.Nakatani add
	m_pShareData->m_Common.m_bWordOnly = m_bWordOnly;		/* 1==単語のみ検索 */


//やめました
//	if( 0 == lstrlen( m_szText ) ){
//		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
//			"検索のキーワードを指定してください。"
//		);
//		return FALSE;
//	}
	/* この編集中のテキストから検索する */
	if( 0 == lstrlen( m_szFile ) ){
		//	Jun. 16, 2003 Moca
		//	検索パターンが指定されていない場合のメッセージ表示をやめ、
		//	「*.*」が指定されたものと見なす．
		strcpy( m_szFile, "*.*" );
	}
	if( 0 == lstrlen( m_szFolder ) ){
		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"検索対象フォルダを指定してください。"
		);
		return FALSE;
	}

	char szCurDirOld[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, szCurDirOld );
	/* 相対パス→絶対パス */
	if( 0 == ::SetCurrentDirectory( m_szFolder ) ){
		::MYMESSAGEBOX(	m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"検索対象フォルダが正しくありません。"
		);
		::SetCurrentDirectory( szCurDirOld );
		return FALSE;
	}
	::GetCurrentDirectory( MAX_PATH, m_szFolder );
	::SetCurrentDirectory( szCurDirOld );

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()追加に伴う変更
	/* 検索文字列 */
	if( 0 < lstrlen( m_szText ) ){
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0;
		nFlag |= m_bLoHiCase ? 0x01 : 0x00;
		if( m_bRegularExp  && !CheckRegexpSyntax( m_szText, m_hWnd, true, nFlag) ){
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え
		CShareData::getInstance()->AddToSearchKeyArr( m_szText );
	}

	/* 検索ファイル */
	CShareData::getInstance()->AddToGrepFileArr( m_szFile );

	/* 検索フォルダ */
	CShareData::getInstance()->AddToGrepFolderArr( m_szFolder );

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgGrep::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
