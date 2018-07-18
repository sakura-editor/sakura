/*!	@file
	@brief タイプ別設定インポート確認ダイアログ

	@author Uchi
	@date 2010/4/17 新規作成
*/
/*
	Copyright (C) 2010, Uchi

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
#include "CDlgTypeAscertain.h"
#include "env/CDocTypeManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura.hh"
#include "sakura_rc.h"

// タイプ別設定インポート確認 CDlgTypeAscertain.cpp
const DWORD p_helpids[] = {
	IDC_RADIO_TYPE_TO,		HIDC_RADIO_TYPE_TO,		//タイプ別名
	IDC_RADIO_TYPE_ADD,		HIDC_RADIO_TYPE_ADD,	//タイプ別追加
	IDC_COMBO_COLORS,		HIDC_COMBO_COLORS,		//色指定
	IDOK,					HIDOK_DTA,				//OK
	IDCANCEL,				HIDCANCEL_DTA,			//キャンセル
	IDC_BUTTON_HELP,		HIDC_DTA_BUTTON_HELP,	//ヘルプ
//	IDC_STATIC,				-1,
	0, 0
};

//  Constructors
CDlgTypeAscertain::CDlgTypeAscertain()
	: m_psi(NULL)
{
}

// モーダルダイアログの表示
int CDlgTypeAscertain::DoModal( HINSTANCE hInstance, HWND hwndParent, SAscertainInfo* psAscertainInfo )
{
	m_psi = psAscertainInfo;

	m_psi->nColorType = -1;

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPE_ASCERTAIN, (LPARAM)NULL );
}

// ボタンクリック
BOOL CDlgTypeAscertain::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「タイプ別設定インポート」のヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, HLP000338 );
		return TRUE;
	case IDOK:
		TCHAR	buff1[_MAX_PATH + 20];
		wchar_t	buff2[_MAX_PATH + 20];

		m_psi->bAddType = IsDlgButtonCheckedBool( GetHwnd(), IDC_RADIO_TYPE_ADD );
		m_psi->sColorFile = L"";
		m_psi->nColorType = Combo_GetCurSel( GetDlgItem( GetHwnd(), IDC_COMBO_COLORS ) ) - 1;
		if (m_psi->nColorType >= MAX_TYPES && Combo_GetLBText( ::GetDlgItem( GetHwnd(), IDC_COMBO_COLORS ), m_psi->nColorType + 1, buff1)) {
			if (_stscanf( buff1, _T("File -- %ls"), buff2 ) > 0) {
				m_psi->sColorFile = buff2;
				m_psi->nColorType = MAX_TYPES;
			}
		}
		::EndDialog( GetHwnd(), TRUE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}


/* ダイアログデータの設定 */
void CDlgTypeAscertain::SetData( void )
{
	// タイプ名設定
	std::wstring typeNameTo = m_psi->sTypeNameTo + L"(&B)";
	::SetWindowText( ::GetDlgItem( GetHwnd(), IDC_RADIO_TYPE_TO    ), to_tchar(typeNameTo.c_str()) );
	::SetWindowText( ::GetDlgItem( GetHwnd(), IDC_STATIC_TYPE_FILE ), to_tchar(m_psi->sTypeNameFile.c_str()) );

	::CheckDlgButton( GetHwnd(), IDC_RADIO_TYPE_ADD, TRUE );

	int		nIdx;
	HWND	hwndCombo;
	TCHAR	szText[_MAX_PATH + 10];
	hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_COLORS );
	/* コンボボックスを空にする */
	Combo_ResetContent( hwndCombo );
	/* 一行目はそのまま */
	Combo_AddString( hwndCombo, LSW(STR_DLGTYPEASC_IMPORT) );

	// エディタ内の設定
	for (nIdx = 0; nIdx < GetDllShareData().m_nTypesCount; ++nIdx) {
		const STypeConfigMini* type;
		CDocTypeManager().GetTypeConfigMini(CTypeConfig(nIdx), &type);
		if (type->m_szTypeExts[0] != _T('\0') ) {		/* タイプ属性：拡張子リスト */
			auto_sprintf( szText, _T("%ts (%ts)"),
				type->m_szTypeName,	/* タイプ属性：名称 */
				type->m_szTypeExts	/* タイプ属性：拡張子リスト */
			);
		}
		else{
			auto_sprintf( szText, _T("%ts"),
				type->m_szTypeName	/* タイプ属性：拡称 */
			);
		}
		::Combo_AddString( hwndCombo, szText );
	}
	// 読込色設定ファイル設定
	HANDLE	hFind;
	WIN32_FIND_DATA	wf;
	BOOL	bFind;
	TCHAR	sTrgCol[_MAX_PATH + 1];

	::SplitPath_FolderAndFile( m_psi->sImportFile.c_str(), sTrgCol, NULL );
	_tcscat( sTrgCol, _T("\\*.col") );
	for (bFind = ( ( hFind = FindFirstFile( sTrgCol, &wf ) ) != INVALID_HANDLE_VALUE );
		bFind;
		bFind = FindNextFile( hFind, &wf )) {
		if ( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			// 読込色設定ファイル発見
			auto_sprintf( szText, _T("File -- %ts"), wf.cFileName );
			::Combo_AddString( hwndCombo, szText );
		}
	}
	FindClose( hFind );

	// コンボボックスのデフォルト選択
	Combo_SetCurSel( hwndCombo, 0 );
	return;
}

LPVOID CDlgTypeAscertain::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
