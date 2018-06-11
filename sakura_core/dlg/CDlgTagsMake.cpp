/*!	@file
	@brief タグファイル作成ダイアログボックス

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
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
#include "dlg/CDlgTagsMake.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13700
	IDC_EDIT_TAG_MAKE_FOLDER,	HIDC_EDIT_TAG_MAKE_FOLDER,	//タグ作成フォルダ
	IDC_BUTTON_TAG_MAKE_REF,	HIDC_BUTTON_TAG_MAKE_REF,	//参照
	IDC_BUTTON_FOLDER_UP,		HIDC_BUTTON_TAG_MAKE_FOLDER_UP,	// 上
	IDC_EDIT_TAG_MAKE_CMDLINE,	HIDC_EDIT_TAG_MAKE_CMDLINE,	//コマンドライン
	IDC_CHECK_TAG_MAKE_RECURSE,	HIDC_CHECK_TAG_MAKE_RECURSE,	//サブフォルダも対象
	IDOK,						HIDC_TAG_MAKE_IDOK,
	IDCANCEL,					HIDC_TAG_MAKE_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_TAG_MAKE_HELP,
//	IDC_STATIC,						-1,
	0, 0
};

CDlgTagsMake::CDlgTagsMake()
{
	_tcscpy( m_szPath, _T("") );
	_tcscpy( m_szTagsCmdLine, _T("") );
	m_nTagsOpt = 0;
	return;
}

/* モーダルダイアログの表示 */
int CDlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath		//パス
)
{
	_tcscpy( m_szPath, pszPath );

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
			TCHAR szDir[_MAX_PATH];
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
	フォルダを選択する
	
	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void CDlgTagsMake::SelectFolder( HWND hwndDlg )
{
	TCHAR	szPath[_MAX_PATH + 1];

	/* フォルダ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH );

	if( SelectDir( hwndDlg, LS(STR_DLGTAGMAK_SELECTDIR), szPath, szPath ) )
	{
		//末尾に\\マークを追加する．
		int pos = _tcslen( szPath );
		if( pos > 0 && szPath[ pos - 1 ] != _T('\\') )
		{
			szPath[ pos     ] = _T('\\');
			szPath[ pos + 1 ] = _T('\0');
		}

		::DlgItem_SetText( hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath );
	}
}

/* ダイアログデータの設定 */
void CDlgTagsMake::SetData( void )
{
	//作成フォルダ
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER ), _countof( m_szPath ) );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath );

	//オプション
	m_nTagsOpt = m_pShareData->m_nTagsOpt;
	if( m_nTagsOpt & 0x0001 ) ::CheckDlgButton( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE, TRUE );

	//コマンドライン
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE ), _countof( m_pShareData->m_szTagsCmdLine ) );
	_tcscpy( m_szTagsCmdLine, m_pShareData->m_szTagsCmdLine );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_pShareData->m_szTagsCmdLine );

	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgTagsMake::GetData( void )
{
	//フォルダ
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_FOLDER, m_szPath, _countof( m_szPath ) );
	int length = _tcslen( m_szPath );
	if( length > 0 )
	{
		if( m_szPath[ length - 1 ] != _T('\\') ) _tcscat( m_szPath, _T("\\") );
	}

	//CTAGSオプション
	m_nTagsOpt = 0;
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_TAG_MAKE_RECURSE ) == BST_CHECKED ) m_nTagsOpt |= 0x0001;
	m_pShareData->m_nTagsOpt = m_nTagsOpt;

	//コマンドライン
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_TAG_MAKE_CMDLINE, m_szTagsCmdLine, _countof( m_szTagsCmdLine ) );
	_tcscpy( m_pShareData->m_szTagsCmdLine, m_szTagsCmdLine );

	return TRUE;
}

LPVOID CDlgTagsMake::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


