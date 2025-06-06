﻿/*!	@file
	@brief タグファイル作成ダイアログボックス

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "dlg/CDlgTagsMake.h"

#include <Shlwapi.h>

#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "String_define.h"

const DWORD p_helpids[] = {	//13700
	IDC_EDIT_TAG_MAKE_FOLDER,	HIDC_EDIT_TAG_MAKE_FOLDER,	//タグ作成フォルダー
	IDC_BUTTON_TAG_MAKE_REF,	HIDC_BUTTON_TAG_MAKE_REF,	//参照
	IDC_BUTTON_FOLDER_UP,		HIDC_BUTTON_TAG_MAKE_FOLDER_UP,	// 上
	IDC_EDIT_TAG_MAKE_CMDLINE,	HIDC_EDIT_TAG_MAKE_CMDLINE,	//コマンドライン
	IDC_CHECK_TAG_MAKE_RECURSE,	HIDC_CHECK_TAG_MAKE_RECURSE,	//サブフォルダーも対象
	IDOK,						HIDC_TAG_MAKE_IDOK,
	IDCANCEL,					HIDC_TAG_MAKE_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_TAG_MAKE_HELP,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgTagsMake::CDlgTagsMake()
{
	m_szPath[0] = L'\0';
	m_szTagsCmdLine[0] = L'\0';
	m_nTagsOpt = 0;
	return;
}

/* モーダルダイアログの表示 */
int CDlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const WCHAR*	pszPath		//パス
)
{
	wcscpy( m_szPath, pszPath );

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TAG_MAKE, lParam );
}

BOOL CDlgTagsMake::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDC_BUTTON_TAG_MAKE_REF:	/* 参照 */
		SelectFolder( GetHwnd() );
		return TRUE;

	case IDC_BUTTON_FOLDER_UP:
		{
			WCHAR szDir[_MAX_PATH];
			HWND hwnd = GetItemHwnd( IDC_EDIT_TAG_MAKE_FOLDER );
			::GetWindowText( hwnd, szDir, _countof(szDir) );
			if( DirectoryUp( szDir ) ){
				::SetWindowText( hwnd, szDir );
			}
		}
		return TRUE;

	case IDOK:
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

/*!
	フォルダーを選択する
	
	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void CDlgTagsMake::SelectFolder( HWND hwndDlg )
{
	WCHAR	szPath[_MAX_PATH + 1];

	/* フォルダー */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH );

	if( SelectDir( hwndDlg, LS(STR_DLGTAGMAK_SELECTDIR), szPath, szPath ) )
	{
		//末尾に\\マークを追加する．
		::PathAddBackslashW( szPath );

		::DlgItem_SetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath );
	}
}

/* ダイアログデータの設定 */
void CDlgTagsMake::SetData( void )
{
	//作成フォルダー
	Combo_LimitText( GetItemHwnd( IDC_EDIT_TAG_MAKE_FOLDER ), _countof( m_szPath ) );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath );

	//オプション
	m_nTagsOpt = m_pShareData->m_nTagsOpt;
	if( m_nTagsOpt & 0x0001 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE, TRUE );

	//コマンドライン
	Combo_LimitText( GetItemHwnd( IDC_EDIT_TAG_MAKE_CMDLINE ), _countof( m_pShareData->m_szTagsCmdLine ) );
	wcscpy( m_szTagsCmdLine, m_pShareData->m_szTagsCmdLine );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_pShareData->m_szTagsCmdLine );

	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgTagsMake::GetData( void )
{
	//フォルダー
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath, _countof( m_szPath ) );
	int length = wcslen( m_szPath );
	if( length > 0 )
	{
		if( m_szPath[ length - 1 ] != L'\\' ) wcscat( m_szPath, L"\\" );
	}

	//CTAGSオプション
	m_nTagsOpt = 0;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE ) == BST_CHECKED ) m_nTagsOpt |= 0x0001;
	m_pShareData->m_nTagsOpt = m_nTagsOpt;

	//コマンドライン
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_szTagsCmdLine, _countof( m_szTagsCmdLine ) );
	wcscpy( m_pShareData->m_szTagsCmdLine, m_szTagsCmdLine );

	return TRUE;
}

LPVOID CDlgTagsMake::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}
