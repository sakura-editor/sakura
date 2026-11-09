/*!	@file
	@brief プロファイルマネージャ

	@author Moca
	@date 2013.12.31
*/
/*
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "dlg/CDlgProfileMgr.h"

#include "_main/CCommandLine.h"

#include "dlg/CDlgInput1.h"
#include "env/CDataProfile.h"
#include "util/file.h"
#include "util/shell.h"
#include "util/window.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "func/Funccode.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_LIST_PROFILE,				HIDC_LIST_PROFILE,				//プロファイル一覧
	IDC_CHECK_PROF_DEFSTART,		HIDC_CHECK_PROF_DEFSTART,		//デフォルト設定にして起動
	IDOK,							HIDOK_PROFILEMGR,				//起動
	IDCANCEL,						HIDCANCEL_PROFILEMGR,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_PROFILEMGR_BUTTON_HELP,	//ヘルプ
	IDC_BUTTON_PROF_CREATE,			HIDC_BUTTON_PROF_CREATE,		//新規作成
	IDC_BUTTON_PROF_RENAME,			HIDC_BUTTON_PROF_RENAME,		//名前変更
	IDC_BUTTON_PROF_DELETE,			HIDC_BUTTON_PROF_DELETE,		//削除
	IDC_BUTTON_PROF_DEFSET,			HIDC_BUTTON_PROF_DEFSET,		//デフォルト設定
	IDC_BUTTON_PROF_DEFCLEAR,		HIDC_BUTTON_PROF_DEFCLEAR,		//デフォルト解除
	0, 0
};

//! コマンドラインだけでプロファイルが確定するか調べる
bool CDlgProfileMgr::TrySelectProfile( CCommandLine* pcCommandLine ) noexcept
{
	SProfileSettings settings;
	bool bSettingLoaded = ReadProfSettings( settings );

	bool bDialog;
	if( pcCommandLine->IsProfileMgr() ){		// コマンドラインでプロファイルマネージャの表示が指定されている
		bDialog = true;
	}else if( pcCommandLine->IsSetProfile() ){	// コマンドラインでプロファイル名が指定されている
		bDialog = false;
	}else if( !bSettingLoaded ){				// プロファイル設定がなかった
		bDialog = false;
	}else if( 0 < settings.m_nDefaultIndex && settings.m_nDefaultIndex <= static_cast<int>(settings.m_vProfList.size()) ){
		// プロファイル設定のデフォルトインデックス値から該当のプロファイル名が指定されたものとして動作する
		pcCommandLine->SetProfileName(settings.m_vProfList[settings.m_nDefaultIndex - 1]);
		bDialog = false;
	}else{
		// プロファイル設定のデフォルトインデックス値が不正なのでプロファイルマネージャを表示して設定更新を促す
		bDialog = true;
	}
	if( bDialog ){
		if( bSettingLoaded ){
			// 設定が読めた場合のみ、日本語以外の設定言語を適用する
			CSelectLang::ChangeLang( settings.m_szDllLanguage );
		}
	}
	return !bDialog;
}

CDlgProfileMgr::CDlgProfileMgr()
: CDialog(false, false)
{
	return;
}

/*! モーダルダイアログの表示 */
int CDlgProfileMgr::DoModal(
	HWND hWndParent,
	SProfileName& profileName
)
{
	const HINSTANCE unusedArg = nullptr;

	const auto ret = (int)CDialog::DoModal(unusedArg, hWndParent, IDD_PROFILEMGR, 0L);

	if (ret)
	{
		profileName = m_ProfileName;
	}

	return ret;
}

/*!
	@brief プロファイルマネージャ設定ファイルパスを取得する
 */
std::filesystem::path GetProfileMgrFileName()
{
	auto privateIniPath = GetIniFileName();
	if (const auto* pCommandLine = CCommandLine::getInstance(); pCommandLine->IsSetProfile() && *pCommandLine->GetProfileName()) {
		auto filename = privateIniPath.filename();
		privateIniPath = privateIniPath.parent_path().parent_path().append(filename.c_str());
	}
	return privateIniPath.replace_extension().concat(LTEXT("_prof.ini"));
}

/*!
	指定したプロファイルの設定保存先ディレクトリを取得する
 */
std::filesystem::path GetProfileDirectory(sakura::wstring_view name)
{
	auto privateIniDir = GetIniFileName().parent_path();
	if (const auto profileName = GetProfileName(); *profileName) {
		privateIniDir = privateIniDir.parent_path();
	}
	return privateIniDir.append(name).append(LTEXT("a.txt")).remove_filename();
}

/*!
	既存コード互換用関数
	指定したプロファイルの設定保存先ディレクトリを返す。
 */
[[nodiscard]] sakura::wstring GetProfileMgrFileName(sakura::wstring_view name)
{
	return GetProfileDirectory(name);
}

/*! ダイアログデータの設定 */
void CDlgProfileMgr::SetData()
{
	SetData( -1 );
}

void CDlgProfileMgr::SetData( int nSelIndex )
{
	HWND	hwndList = GetItemHwnd( IDC_LIST_PROFILE );

	ApiWrap::List_ResetContent( hwndList );
	SProfileSettings settings;
	ReadProfSettings( settings );
	sakura::wstring strdef = LTEXT("(default)");
	if( settings.m_nDefaultIndex == 0 ){
		strdef += LTEXT("*");
	}
	ApiWrap::List_AddString( hwndList, strdef.c_str() );
	CTextWidthCalc calc( hwndList );
	calc.SetDefaultExtend( CTextWidthCalc::WIDTH_MARGIN_SCROLLBER );
	int count = (int)settings.m_vProfList.size();
	for(int i = 0; i < count; i++){
		sakura::wstring str = settings.m_vProfList[i];
		if( settings.m_nDefaultIndex == i + 1 ){
			str += LTEXT("*");
		}
		ApiWrap::List_AddString( hwndList, str.c_str() );
		calc.SetTextWidthIfMax( str.c_str() );
	}
	ApiWrap::List_SetHorizontalExtent( hwndList, calc.GetCx() );
	if( nSelIndex == -1 ){
		nSelIndex = settings.m_nDefaultIndex;
	}
	if( nSelIndex < 0 ){
		nSelIndex = 0;
	}
	ApiWrap::List_SetCurSel( hwndList, nSelIndex );
	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DELETE, nSelIndex != 0 );
	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_RENAME, nSelIndex != 0 );

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, settings.m_nDefaultIndex != -1 );
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PROF_DEFSTART, settings.m_bDefaultSelect );
}

template <size_t cchText>
static bool MyList_GetText(HWND hwndList, int index, WCHAR(&szText)[cchText])
{
	ApiWrap::List_GetText( hwndList, index, szText );
	WCHAR* pos = wcschr( szText, LCHAR('*') );
	if( pos != nullptr ){
		*pos = LCHAR('\0');
		return true;
	}
	return false;
}

/*! ダイアログデータの取得 */
int CDlgProfileMgr::GetData()
{
	return GetData(true);
}

int CDlgProfileMgr::GetData(bool bStart)
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	int nCurIndex = ApiWrap::List_GetCurSel(hwndList);
	WCHAR szText[_MAX_PATH];
	MyList_GetText( hwndList, nCurIndex, szText );
	m_ProfileName = szText;
	if( m_ProfileName == LTEXT("(default)") ){
		m_ProfileName = LTEXT("");
	}
	bool bDefaultSelect = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PROF_DEFSTART );
	SProfileSettings settings;
	ReadProfSettings( settings );
	bool bWrtie = false;
	if( settings.m_bDefaultSelect != bDefaultSelect ){
		bWrtie = true;
	}
	if( bDefaultSelect && bStart ){
		bWrtie = true;
		SetDefaultProf( nCurIndex );
	}
	if( bWrtie ){
		UpdateIni();
	}
	return 1;
}

BOOL CDlgProfileMgr::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_PROF_CREATE:
		CreateProf();
		break;

	case IDC_BUTTON_PROF_RENAME:
		RenameProf();
		break;

	case IDC_BUTTON_PROF_DELETE:
		DeleteProf();
		break;

	case IDC_BUTTON_PROF_DEFSET:
		{
			HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
			int nSelIndex = ApiWrap::List_GetCurSel( hwndList );
			SetDefaultProf(nSelIndex);
			UpdateIni();
			ApiWrap::List_SetCurSel( hwndList, nSelIndex );
			DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, true );
		}
		break;

	case IDC_BUTTON_PROF_DEFCLEAR:
		{
			HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
			int nSelIndex = ApiWrap::List_GetCurSel( hwndList );
			ClearDefaultProf();
			UpdateIni();
			ApiWrap::List_SetCurSel( hwndList, nSelIndex );
			DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, false );
		}
		break;

	case IDC_BUTTON_HELP:
		/* 「検索」のヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROFILEMGR) );
		break;

	case IDOK:
		GetData();
		CloseDialog( 1 );
		return TRUE;
	case IDCANCEL:
		GetData(false);
		CloseDialog( 0 );
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

INT_PTR CDlgProfileMgr::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		{
			if( LOWORD(wParam) == IDC_LIST_PROFILE ){
				switch( HIWORD(wParam) ){
				case LBN_SELCHANGE:
				{
					HWND hwndList = (HWND)lParam;
					int nIdx = ApiWrap::List_GetCurSel( hwndList );
					DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DELETE, nIdx != 0 );
					DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_RENAME, nIdx != 0 );
					return TRUE;
				}
				default:
					break;
				}
			}
		}
	default:
		break;
	}
	return result;
}

void CDlgProfileMgr::UpdateIni()
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	SProfileSettings settings;
	ReadProfSettings( settings );
	int nCount = ApiWrap::List_GetCount( hwndList );
	settings.m_vProfList.clear();
	settings.m_nDefaultIndex = -1;
	for( int i = 0; i < nCount; i++ ){
		WCHAR szProfileName[_MAX_PATH];
		if( MyList_GetText( hwndList, i, szProfileName ) ){
			settings.m_nDefaultIndex = i;
		}
		if( 0 < i ){
			sakura::wstring str = szProfileName;
			settings.m_vProfList.push_back( str );
		}
	}
	settings.m_bDefaultSelect = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PROF_DEFSTART );

	if( !WriteProfSettings( settings ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_WRITE) );
	}
}

static bool IsProfileDuplicate(HWND hwndList, LPCWSTR szProfName, int skipIndex)
{
	int nCount = ApiWrap::List_GetCount( hwndList );
	for( int i = 0; i < nCount; i++ ){
		if( skipIndex == i ){
			continue;
		}
		WCHAR szProfileName[_MAX_PATH];
		MyList_GetText( hwndList, i, szProfileName );
		if( 0 == wmemicmp( szProfName, szProfileName ) ){
			return true;
		}
	}
	return false;
}

void CDlgProfileMgr::CreateProf()
{
	auto& cDlgInput1 = *CDlgInput1::getInstance();
	int max_size = _MAX_PATH;
	WCHAR szText[_MAX_PATH];
	sakura::wstring strTitle = LS(STR_DLGPROFILE_NEW_PROF_TITLE);
	sakura::wstring strMessage = LS(STR_DLGPROFILE_NEW_PROF_MSG);
	szText[0] = LCHAR('\0');
	if( !cDlgInput1.DoModal(::GetModuleHandle(nullptr), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size - 1, szText) ){
		return;
	}
	if( szText[0] == LCHAR('\0') ){
		return;
	}
	sakura::wstring strText = szText;
	static const WCHAR szReservedChars[] = LTEXT("/\\*?<>&|:\"'\t");
	for( int x = 0; x < int(std::size(szReservedChars)); ++x ){
		if( strText.npos != strText.find(szReservedChars[x]) ){
			ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
			return;
		}
	}
	if( 0 == wcscmp( szText, LTEXT("..") ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
		return;
	}
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	if( IsProfileDuplicate( hwndList, szText, -1 ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY) );
		return;
	}
	sakura::wstring strProfDir = GetProfileMgrFileName(szText);
	if( IsFileExists(strProfDir.c_str(), true) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_FILE) );
		return;
	}

	ApiWrap::List_AddString( hwndList, szText );
	int sel = ApiWrap::List_GetCurSel( hwndList );
	UpdateIni();
	SetData(sel);
}

void CDlgProfileMgr::DeleteProf()
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	int nCurIndex = ApiWrap::List_GetCurSel(hwndList);
	ApiWrap::List_DeleteString( hwndList, nCurIndex );
	UpdateIni();
	if( ApiWrap::List_GetCount( hwndList ) <= nCurIndex ){
		nCurIndex--;
	}
	SetData(nCurIndex);
}

void CDlgProfileMgr::RenameProf()
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	auto& cDlgInput1 = *CDlgInput1::getInstance();
	int nCurIndex = ApiWrap::List_GetCurSel(hwndList);
	WCHAR szText[_MAX_PATH];
	bool bDefault = MyList_GetText( hwndList, nCurIndex, szText );
	WCHAR szTextOld[_MAX_PATH];
	wcscpy( szTextOld, szText );
	sakura::wstring strTitle = LS(STR_DLGPROFILE_RENAME_TITLE);
	sakura::wstring strMessage = LS(STR_DLGPROFILE_RENAME_MSG);
	int max_size = _MAX_PATH;
	if( !cDlgInput1.DoModal(::GetModuleHandle(nullptr), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size - 1, szText) ){
		return;
	}
	if( szText[0] == LCHAR('\0') ){
		return;
	}
	if( 0 == wcscmp( szTextOld, szText ) ){
		return; // 未変更
	}
	sakura::wstring strText = szText;
	static const WCHAR szReservedChars[] = LTEXT("/\\*?<>&|:\"'\t");
	for( int x = 0; x < int(std::size(szReservedChars)); ++x ){
		if( strText.npos != strText.find(szReservedChars[x]) ){
			ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
			return;
		}
	}
	if( 0 == wcscmp( szText, LTEXT("..") ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
		return;
	}
	if( IsProfileDuplicate( hwndList, szText, nCurIndex ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY) );
		return;
	}
	sakura::wstring strProfDirOld = GetProfileMgrFileName(szTextOld);
	sakura::wstring strProfDir = GetProfileMgrFileName(szText);
	if( IsFileExists(strProfDirOld.c_str(), false) ){
		if( !IsFileExists(strProfDirOld.c_str(), true) ){
			// プロファイル名はディレクトリ
			if( FALSE == ::MoveFile( strProfDirOld.c_str(), strProfDir.c_str() ) ){
				ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_RENAME) );
				return;
			}
		}else{
			// 旧プロファイル名はファイルだったので新規プロファイルとして作成確認
			if( IsFileExists(strProfDir.c_str(), true) ){
				ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_FILE) );
				return;
			}
		}
	}
	if( bDefault ){
		wcscat(szText, LTEXT("*"));
	}
	ApiWrap::List_DeleteString( hwndList, nCurIndex );
	ApiWrap::List_InsertString( hwndList, nCurIndex, szText );
	UpdateIni();
	SetData(nCurIndex);
}

void CDlgProfileMgr::SetDefaultProf(int index)
{
	ClearDefaultProf();
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	WCHAR szProfileName[_MAX_PATH];
	MyList_GetText( hwndList, index, szProfileName );
	ApiWrap::List_DeleteString( hwndList, index );
	wcscat( szProfileName, LTEXT("*") );
	ApiWrap::List_InsertString( hwndList, index, szProfileName );
}

void CDlgProfileMgr::ClearDefaultProf()
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PROFILE );
	int nCount = ApiWrap::List_GetCount( hwndList );
	for( int i = 0; i < nCount; i++ ){
		WCHAR szProfileName[_MAX_PATH];
		if( MyList_GetText( hwndList, i, szProfileName ) ){
			ApiWrap::List_DeleteString( hwndList, i );
			ApiWrap::List_InsertString( hwndList, i, szProfileName );
		}
	}
}

static bool IOProfSettings( SProfileSettings& settings, bool bWrite )
{
	CDataProfile cProf;
	if( bWrite ){
		cProf.SetWritingMode();
	}else{
		cProf.SetReadingMode();
	}
	sakura::wstring strIniName = GetProfileMgrFileName();
	if( !bWrite ){
		if( !cProf.ReadProfile( strIniName.c_str() ) ){
			return false;
		}
	}
	int nCount = (int)settings.m_vProfList.size();
	const WCHAR* const pSection = LTEXT("Profile");
	cProf.IOProfileData(pSection , LTEXT("nCount"), nCount );
	for(int i = 0; i < nCount; i++){
		WCHAR szKey[64];
		sakura::wstring strProfName;
		_swprintf( szKey, LTEXT("P[%d]"), i + 1 ); // 1開始
		if( bWrite ){
			strProfName = settings.m_vProfList[i];
			cProf.IOProfileData( pSection, szKey, strProfName );
		}else{
			cProf.IOProfileData( pSection, szKey, strProfName );
			settings.m_vProfList.push_back( strProfName );
		}
	}
	cProf.IOProfileData( pSection, LTEXT("nDefaultIndex"), settings.m_nDefaultIndex );
	if( nCount < settings.m_nDefaultIndex ){
		settings.m_nDefaultIndex = -1;
	}
	if( settings.m_nDefaultIndex < -1 ){
		settings.m_nDefaultIndex = -1;
	}
	cProf.IOProfileData(pSection, LTEXT("szDllLanguage"), StringBufferW(settings.m_szDllLanguage));
	cProf.IOProfileData( pSection, LTEXT("bDefaultSelect"), settings.m_bDefaultSelect );

	if( bWrite ){
		if( !cProf.WriteProfile( strIniName.c_str(), LTEXT("Sakura Profile ini") ) ){
			return false;
		}
	}
	return true;
}

bool CDlgProfileMgr::ReadProfSettings( SProfileSettings& settings )
{
	settings.m_szDllLanguage[0] = LCHAR('\0');
	settings.m_nDefaultIndex = 0;
	settings.m_vProfList.clear();
	settings.m_bDefaultSelect = false;

	return IOProfSettings( settings, false );
}

bool CDlgProfileMgr::WriteProfSettings( SProfileSettings& settings )
{
	return IOProfSettings( settings, true );
}

LPVOID CDlgProfileMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
