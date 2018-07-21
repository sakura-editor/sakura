/*!	@file
	@brief プロファイルマネージャ

	@author Moca
	@date 2013.12.31
*/
/*
	Copyright (C) 2013, Moca

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
#include "dlg/CDlgProfileMgr.h"
#include "dlg/CDlgInput1.h"
#include "CDataProfile.h"
#include "util/file.h"
#include "util/shell.h"
#include "util/window.h"
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

CDlgProfileMgr::CDlgProfileMgr()
: CDialog(false, false)
{
	return;
}

/*! モーダルダイアログの表示 */
int CDlgProfileMgr::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PROFILEMGR, lParam );
}


static std::tstring GetProfileMgrFileName(LPCTSTR profName = NULL)
{
	static TCHAR szPath[_MAX_PATH];
	static TCHAR szPath2[_MAX_PATH];
	static TCHAR* pszPath;
	static bool bSet = false;
	if( bSet == false ){
		pszPath = szPath;
		CFileNameManager::GetIniFileNameDirect( szPath, szPath2, _T("") );
		if( szPath[0] == _T('\0') ){
			pszPath = szPath2;
		}
		bSet = true;
	}
	
	TCHAR	szDir[_MAX_PATH];
	SplitPath_FolderAndFile( pszPath, szDir, NULL );


	TCHAR szIniFile[_MAX_PATH];
	if( profName == NULL ){
		TCHAR szExePath[_MAX_PATH];
		TCHAR szFname[_MAX_FNAME];
		::GetModuleFileName( NULL, szExePath, _countof(szExePath) );
		_tsplitpath( szExePath, NULL, NULL, szFname, NULL );
		auto_snprintf_s( szIniFile, _MAX_PATH - 1, _T("%ts\\%ts_prof%ts"), szDir, szFname, _T(".ini") );
	}else{
		auto_snprintf_s( szIniFile, _MAX_PATH - 1, _T("%ts\\%ts"), szDir, profName );
	}

	return std::tstring(szIniFile);
}


/*! ダイアログデータの設定 */
void CDlgProfileMgr::SetData()
{
	SetData( -1 );
}


void CDlgProfileMgr::SetData( int nSelIndex )
{
	int		nExtent = 0;
	HWND	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );

	List_ResetContent( hwndList );
	SProfileSettings settings;
	ReadProfSettings( settings );
	std::tstring strdef = _T("(default)");
	if( settings.m_nDefaultIndex == 0 ){
		strdef += _T("*");
	}
	List_AddString( hwndList, strdef.c_str() );
	CTextWidthCalc calc( hwndList );
	calc.SetDefaultExtend( CTextWidthCalc::WIDTH_MARGIN_SCROLLBER );
	int count = (int)settings.m_vProfList.size();
	for(int i = 0; i < count; i++){
		std::tstring str = settings.m_vProfList[i];
		if( settings.m_nDefaultIndex == i + 1 ){
			str += _T("*");
		}
		List_AddString( hwndList, str.c_str() );
		calc.SetTextWidthIfMax( str.c_str() );
	}
	List_SetHorizontalExtent( hwndList, calc.GetCx() );
	if( nSelIndex == -1 ){
		nSelIndex = settings.m_nDefaultIndex;
	}
	if( nSelIndex < 0 ){
		nSelIndex = 0;
	}
	List_SetCurSel( hwndList, nSelIndex );
	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DELETE, nSelIndex != 0 );
	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_RENAME, nSelIndex != 0 );

	DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, settings.m_nDefaultIndex != -1 );
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PROF_DEFSTART, settings.m_bDefaultSelect );
}



static bool MyList_GetText(HWND hwndList, int index, TCHAR* szText)
{
	List_GetText( hwndList, index, szText );
	TCHAR* pos = auto_strchr( szText, _T('*') );
	if( pos != NULL ){
		*pos = _T('\0');
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
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	int nCurIndex = List_GetCurSel(hwndList);
	TCHAR szText[_MAX_PATH];
	MyList_GetText( hwndList, nCurIndex, szText );
	m_strProfileName = szText;
	if( m_strProfileName == _T("(default)") ){
		m_strProfileName = _T("");
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
			HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
			int nSelIndex = List_GetCurSel( hwndList );
			SetDefaultProf(nSelIndex);
			UpdateIni();
			List_SetCurSel( hwndList, nSelIndex );
			DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DEFCLEAR, true );
		}
		break;

	case IDC_BUTTON_PROF_DEFCLEAR:
		{
			HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
			int nSelIndex = List_GetCurSel( hwndList );
			ClearDefaultProf();
			UpdateIni();
			List_SetCurSel( hwndList, nSelIndex );
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
					HWND hwndList = (HWND)lParam;
					int nIdx = List_GetCurSel( hwndList );
					DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_DELETE, nIdx != 0 );
					DlgItem_Enable( GetHwnd(), IDC_BUTTON_PROF_RENAME, nIdx != 0 );
					return TRUE;
				}
			}
		}
	}
	return result;
}


void CDlgProfileMgr::UpdateIni()
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	SProfileSettings settings;
	ReadProfSettings( settings );
	int nCount = List_GetCount( hwndList );
	settings.m_vProfList.clear();
	settings.m_nDefaultIndex = -1;
	for( int i = 0; i < nCount; i++ ){
		TCHAR szProfileName[_MAX_PATH];
		if( MyList_GetText( hwndList, i, szProfileName ) ){
			settings.m_nDefaultIndex = i;
		}
		if( 0 < i ){
			std::tstring str = szProfileName;
			settings.m_vProfList.push_back( str );
		}
	}
	settings.m_bDefaultSelect = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PROF_DEFSTART );

	if( !WriteProfSettings( settings ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_WRITE) );
	}
}


static bool IsProfileDuplicate(HWND hwndList, LPCTSTR szProfName, int skipIndex)
{
	int nCount = List_GetCount( hwndList );
	for( int i = 0; i < nCount; i++ ){
		if( skipIndex == i ){
			continue;
		}
		TCHAR szProfileName[_MAX_PATH];
		MyList_GetText( hwndList, i, szProfileName );
		if( 0 == auto_stricmp( szProfName, szProfileName ) ){
			return true;
		}
	}
	return false;
}

void CDlgProfileMgr::CreateProf()
{
	CDlgInput1 cDlgInput1;
	int max_size = _MAX_PATH;
	TCHAR szText[_MAX_PATH];
	std::tstring strTitle = LS(STR_DLGPROFILE_NEW_PROF_TITLE);
	std::tstring strMessage = LS(STR_DLGPROFILE_NEW_PROF_MSG);
	szText[0] = _T('\0');
	if( !cDlgInput1.DoModal(::GetModuleHandle(NULL), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size, szText) ){
		return;
	}
	if( szText[0] == _T('\0') ){
		return;
	}
	std::wstring strText = to_wchar(szText);
	static const WCHAR szReservedChars[] = L"/\\*?<>&|:\"'\t";
	for( int x = 0; x < _countof(szReservedChars); ++x ){
		if( strText.npos != strText.find(szReservedChars[x]) ){
			ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
			return;
		}
	}
	if( 0 == auto_strcmp( szText, _T("..") ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
		return;
	}
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	if( IsProfileDuplicate( hwndList, szText, -1 ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY) );
		return;
	}
	std::tstring strProfDir = GetProfileMgrFileName(szText);
	if( IsFileExists(strProfDir.c_str(), true) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_FILE) );
		return;
	}

	List_AddString( hwndList, szText );
	int sel = List_GetCurSel( hwndList );
	UpdateIni();
	SetData(sel);
}


void CDlgProfileMgr::DeleteProf()
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	int nCurIndex = List_GetCurSel(hwndList);
	List_DeleteString( hwndList, nCurIndex );
	UpdateIni();
	if( List_GetCount( hwndList ) <= nCurIndex ){
		nCurIndex--;
	}
	SetData(nCurIndex);
}


void CDlgProfileMgr::RenameProf()
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	CDlgInput1 cDlgInput1;
	int nCurIndex = List_GetCurSel(hwndList);
	TCHAR szText[_MAX_PATH];
	bool bDefault = MyList_GetText( hwndList, nCurIndex, szText );
	TCHAR szTextOld[_MAX_PATH];
	auto_strcpy( szTextOld, szText );
	std::tstring strTitle = LS(STR_DLGPROFILE_RENAME_TITLE);
	std::tstring strMessage = LS(STR_DLGPROFILE_RENAME_MSG);
	int max_size = _MAX_PATH;
	if( !cDlgInput1.DoModal(::GetModuleHandle(NULL), GetHwnd(), strTitle.c_str(), strMessage.c_str(), max_size, szText) ){
		return;
	}
	if( szText[0] == _T('\0') ){
		return;
	}
	if( 0 == auto_strcmp( szTextOld, szText ) ){
		return; // 未変更
	}
	std::wstring strText = to_wchar(szText);
	static const WCHAR szReservedChars[] = L"/\\*?<>&|:\"'\t";
	for( int x = 0; x < _countof(szReservedChars); ++x ){
		if( strText.npos != strText.find(szReservedChars[x]) ){
			ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
			return;
		}
	}
	if( 0 == auto_strcmp( szText, _T("..") ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_INVALID_CHAR) );
		return;
	}
	if( IsProfileDuplicate( hwndList, szText, nCurIndex ) ){
		ErrorMessage( GetHwnd(), LS(STR_DLGPROFILE_ERR_ALREADY) );
		return;
	}
	std::tstring strProfDirOld = GetProfileMgrFileName(szTextOld);
	std::tstring strProfDir = GetProfileMgrFileName(szText);
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
		auto_strcat(szText, _T("*"));
	}
	List_DeleteString( hwndList, nCurIndex );
	List_InsertString( hwndList, nCurIndex, szText );
	UpdateIni();
	SetData(nCurIndex);
}


void CDlgProfileMgr::SetDefaultProf(int index)
{
	ClearDefaultProf();
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	TCHAR szProfileName[_MAX_PATH];
	MyList_GetText( hwndList, index, szProfileName );
	List_DeleteString( hwndList, index );
	auto_strcat( szProfileName, _T("*") );
	List_InsertString( hwndList, index, szProfileName );
}


void CDlgProfileMgr::ClearDefaultProf()
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PROFILE );
	int nCount = List_GetCount( hwndList );
	for( int i = 0; i < nCount; i++ ){
		TCHAR szProfileName[_MAX_PATH];
		if( MyList_GetText( hwndList, i, szProfileName ) ){
			List_DeleteString( hwndList, i );
			List_InsertString( hwndList, i, szProfileName );
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
	std::tstring strIniName = GetProfileMgrFileName();
	if( !bWrite ){
		if( !cProf.ReadProfile( strIniName.c_str() ) ){
			return false;
		}
	}
	int nCount = (int)settings.m_vProfList.size();
	const wchar_t* const pSection = L"Profile";
	cProf.IOProfileData(pSection , L"nCount", nCount );
	for(int i = 0; i < nCount; i++){
		wchar_t szKey[64];
		std::tstring strProfName;
		swprintf( szKey, L"P[%d]", i + 1 ); // 1開始
		if( bWrite ){
			strProfName = settings.m_vProfList[i];
			std::wstring wstrProfName = to_wchar(strProfName.c_str());
			cProf.IOProfileData( pSection, szKey, wstrProfName );
		}else{
			std::wstring wstrProfName;
			cProf.IOProfileData( pSection, szKey, wstrProfName );
			strProfName = to_tchar(wstrProfName.c_str());
			settings.m_vProfList.push_back( strProfName );
		}
	}
	cProf.IOProfileData( pSection, L"nDefaultIndex", settings.m_nDefaultIndex );
	if( nCount < settings.m_nDefaultIndex ){
		settings.m_nDefaultIndex = -1;
	}
	if( settings.m_nDefaultIndex < -1 ){
		settings.m_nDefaultIndex = -1;
	}
	cProf.IOProfileData( pSection, L"szDllLanguage", StringBufferT(settings.m_szDllLanguage, _countof(settings.m_szDllLanguage)) );
	cProf.IOProfileData( pSection, L"bDefaultSelect", settings.m_bDefaultSelect );

	if( bWrite ){
		if( !cProf.WriteProfile( strIniName.c_str(), L"Sakura Profile ini" ) ){
			return false;
		}
	}
	return true;
}


bool CDlgProfileMgr::ReadProfSettings( SProfileSettings& settings )
{
	settings.m_szDllLanguage[0] = _T('\0');
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
