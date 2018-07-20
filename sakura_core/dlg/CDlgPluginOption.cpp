/*!	@file
	@brief プラグイン設定ダイアログボックス

	@author Uchi
	@date 2010/3/22
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
#include <limits.h>
#include "dlg/CDlgPluginOption.h"
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// BOOL変数の表示
#ifdef _UNICODE
#define	BOOL_DISP_TRUE	_T("\u2611")
#define	BOOL_DISP_FALSE	_T("\u2610")
#else
#define	BOOL_DISP_TRUE	_T("<True>")
#define	BOOL_DISP_FALSE	_T("<False>")
#endif

// 編集領域を表示、非表示にする
static inline void CtrlShow(HWND hwndDlg, int id, BOOL bShow)
{
	HWND hWnd = ::GetDlgItem( hwndDlg, id );
	::ShowWindow( hWnd, bShow? SW_SHOW: SW_HIDE );
	::EnableWindow( hWnd, bShow );
}

const DWORD p_helpids[] = {
	IDC_LIST_PLUGIN_OPTIONS,		HIDC_LIST_PLUGIN_OPTIONS,		// オプションリスト
	IDC_EDIT_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_EDIT_PLUGIN_OPTION_DIR,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_BUTTON_PLUGIN_OPTION_DIR,	HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_EDIT_PLUGIN_OPTION_NUM,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_SPIN_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_CHECK_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDC_COMBO_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// オプション編集
	IDOK,							HIDC_FAVORITE_IDOK,				//OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			//キャンセル
	IDC_PLUGIN_README,				HIDC_PLUGIN_README,				//ReadMe
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		//ヘルプ
//	IDC_STATIC,						-1,
	0, 0
};

CDlgPluginOption::CDlgPluginOption()
{

}

CDlgPluginOption::~CDlgPluginOption()
{

}

/* モーダルダイアログの表示 */
int CDlgPluginOption::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	CPropPlugin*	cPropPlugin,
	int 		ID
)
{
	// プラグイン番号（エディタがふる番号）
	m_ID = ID;
	m_cPlugin = CPluginManager::getInstance()->GetPlugin( m_ID );
	m_cPropPlugin = cPropPlugin;

	if( m_cPlugin == NULL ){
		::ErrorMessage( hwndParent, LS(STR_DLGPLUGINOPT_LOAD) );
		return 0;
	}

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PLUGIN_OPTION, (LPARAM)NULL );
}

/* ダイアログデータの設定 */
void CDlgPluginOption::SetData( void )
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	TCHAR	buf[MAX_LENGTH_VALUE+1];
	bool bLoadDefault = false;

	// タイトル
	auto_sprintf( buf, LS(STR_DLGPLUGINOPT_TITLE), m_cPlugin->m_sName.c_str());
	::SetWindowText( GetHwnd(), buf );

	// リスト
	hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );
	ListView_DeleteAllItems( hwndList );	// リストを空にする
	m_Line = -1;							// 行非選択

	std::auto_ptr<CDataProfile> cProfile( new CDataProfile );
	cProfile->SetReadingMode();
	cProfile->ReadProfile( m_cPlugin->GetOptionPath().c_str() );

	CPluginOption* cOpt;
	CPluginOption::ArrayIter it;
	for( i=0, it = m_cPlugin->m_options.begin(); it != m_cPlugin->m_options.end(); i++, it++ ){
		cOpt = *it;

		auto_snprintf_s( buf, _countof(buf), _T("%ls"), cOpt->GetLabel().c_str());
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = buf;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = cOpt->GetIndex();
		ListView_InsertItem( hwndList, &lvi );

		wstring sSection;
		wstring sKey;
		wstring sValue;
		wstring sType;

		cOpt->GetKey(&sSection, &sKey);
		if( sSection.empty() || sKey.empty() ) {
			sValue = L"";
		}
		else {
			if( !cProfile->IOProfileData( sSection.c_str(), sKey.c_str(), sValue ) ){
				// Optionが見つからなかったらDefault値を設定
				sValue = cOpt->GetDefaultVal();
				if( sValue != wstring(L"") ){
					bLoadDefault = true;
					cProfile->SetWritingMode();
					cProfile->IOProfileData( sSection.c_str(), sKey.c_str(), sValue );
					cProfile->SetReadingMode();
				}
			}
		}

		if (cOpt->GetType() == OPTION_TYPE_BOOL) {
			_tcscpy( buf, sValue == wstring( L"0") || sValue == wstring( L"") ? BOOL_DISP_FALSE : BOOL_DISP_TRUE );
		}
		else if (cOpt->GetType() == OPTION_TYPE_INT) {
			// 数値へ正規化
			auto_sprintf( buf, _T("%d"), _wtoi(sValue.c_str()));
		}
		else if (cOpt->GetType() == OPTION_TYPE_SEL) {
			// 値から表示へ
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = cOpt->GetSelects();

			_tcscpy( buf, _T("") );
			for (auto it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sTrg);
				if (sValue == sTrg) {
					auto_snprintf_s( buf, _countof(buf), _T("%ls"), sView.c_str());
					break;
				}
			}
		}
		else {
			auto_snprintf_s( buf, _countof(buf), _T("%ls"), sValue.c_str());
		}
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem( hwndList, &lvi );
		ListView_SetItemState( hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED );
	}

	if( bLoadDefault ){
		cProfile->SetWritingMode();
		cProfile->WriteProfile( m_cPlugin->GetOptionPath().c_str() ,(m_cPlugin->m_sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str());
	}

	if (i ==0) {
		// オプションが無い
		::EnableWindow( GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS    ), FALSE );
		::EnableWindow( GetItemHwnd( IDOK                       ), FALSE );
	
		::DlgItem_SetText( GetHwnd(), IDC_STATIC_MSG, LS(STR_DLGPLUGINOPT_OPTION) );
	}

	// ReadMe Button
	m_sReadMeName = m_cPropPlugin->GetReadMeFile(to_tchar(m_pShareData->m_Common.m_sPlugin.m_PluginTable[m_ID].m_szName));
	::EnableWindow( GetItemHwnd( IDC_PLUGIN_README ), !m_sReadMeName.empty() );
	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgPluginOption::GetData( void )
{
	// .ini ファイルへの書き込み
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;

	// リスト
	hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );

	std::auto_ptr<CDataProfile> cProfile( new CDataProfile );
	cProfile->SetReadingMode();
	cProfile->ReadProfile( m_cPlugin->GetOptionPath().c_str() );
	cProfile->SetWritingMode();

	CPluginOption* cOpt;
	TCHAR	buf[MAX_LENGTH_VALUE+1];
	CPluginOption::ArrayIter it;
	for( i=0, it = m_cPlugin->m_options.begin(); it != m_cPlugin->m_options.end(); i++, it++ ){
		cOpt = *it;

		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = i;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE+1;
		ListView_GetItem( hwndList, &lvi );

		if (cOpt->GetType() == OPTION_TYPE_BOOL) {
			if (_tcscmp(buf,  BOOL_DISP_FALSE) == 0) {
				_tcscpy (buf, _T("0"));
			}
			else {
				_tcscpy (buf, _T("1"));
			}
		}
		else if (cOpt->GetType() == OPTION_TYPE_SEL) {
			// 表示から値へ
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = cOpt->GetSelects();
			wstring sWbuf = to_wchar(buf);

			for (auto it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sTrg);
				if (sView == sWbuf) {
					auto_sprintf( buf, _T("%ls"), sTrg.c_str());
					break;
				}
			}
		}

		wstring sSection;
		wstring sKey;
		wstring sValue;

		cOpt->GetKey(&sSection, &sKey);
		if( sSection.empty() || sKey.empty() ) {
			continue;
		}

		sValue = to_wchar(buf);

		cProfile->IOProfileData( sSection.c_str(), sKey.c_str(), sValue );
	}

	cProfile->WriteProfile( m_cPlugin->GetOptionPath().c_str() ,(m_cPlugin->m_sName + LSW(STR_DLGPLUGINOPT_INIHEAD)).c_str());

	return TRUE;
}

BOOL CDlgPluginOption::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;

	_SetHwnd( hwndDlg );

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_PLUGIN_OPTIONS );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 40 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST1));
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 55 / 100;
	col.pszText  = const_cast<TCHAR*>(LS(STR_DLGPLUGINOPT_LIST2));
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	/* 行選択 */
	lngStyle = ListView_GetExtendedListViewStyle( hwndList );
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndList, lngStyle );

	// 編集領域の非アクティブ化
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION ), FALSE );
	CtrlShow( hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR,  FALSE );
	CtrlShow( hwndDlg, IDC_BUTTON_PLUGIN_OPTION_DIR,FALSE );
	CtrlShow( hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM,  FALSE );
	CtrlShow( hwndDlg, IDC_SPIN_PLUGIN_OPTION,      FALSE );
	CtrlShow( hwndDlg, IDC_CHECK_PLUGIN_OPTION,     FALSE );
	CtrlShow( hwndDlg, IDC_COMBO_PLUGIN_OPTION,     FALSE );

	// 桁数制限
	EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION     ), MAX_LENGTH_VALUE );
	EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION_DIR ), _MAX_PATH );
	EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM ), 11 );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgPluginOption::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	int			idCtrl;

	idCtrl = (int)wParam;
	switch( idCtrl ){
	case IDC_LIST_PLUGIN_OPTIONS:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case LVN_ITEMCHANGED:
			ChangeListPosition( );
			break;
		case NM_DBLCLK:
			// リストビューへのダブルクリックで編集領域へ移動	2013/5/23 Uchi
			MoveFocusToEdit();
			break;
		}
		return TRUE;

	case IDC_SPIN_PLUGIN_OPTION:
		int			nVal;
		NM_UPDOWN*	pMNUD;
		
		pMNUD  = (NM_UPDOWN*)lParam;

		nVal = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE );
		if( pMNUD->iDelta < 0 ){
			if (nVal < INT_MAX)		++nVal;
		}else
		if( pMNUD->iDelta > 0 ){
			// INT_MINは SetDlgItemInt で扱えない
			if (nVal > -INT_MAX)	--nVal;
		}
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, nVal, TRUE );

		// 編集中のデータの戻し
		SetFromEdit( m_Line );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}



BOOL CDlgPluginOption::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_CHECK_PLUGIN_OPTION:
		// 編集中のデータの戻し
		SetFromEdit( m_Line );
		return TRUE;

	case IDC_BUTTON_PLUGIN_OPTION_DIR:	// 2013/05/22 Uchi
		// ディレクトリ選択
		SelectDirectory( m_Line );
		return TRUE;

	case IDC_PLUGIN_README:		// 2012/12/22 Uchi
		// ReadMe
		{
			if (!m_sReadMeName.empty()) {
				if (!m_cPropPlugin->BrowseReadMe(m_sReadMeName)) {
					WarningMessage( GetHwnd(), LS(STR_PROPCOMPLG_ERR2) );
				}
			}else{
				WarningMessage( GetHwnd(), LS(STR_PROPCOMPLG_ERR3) );
			}
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, HLP000153 );	// 『プラグイン設定』Helpの指定 	2011/11/26 Uchi
		return TRUE;

	case IDOK:
		// 編集中のデータの戻し
		SetFromEdit( m_Line );
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgPluginOption::OnCbnSelChange( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_COMBO_PLUGIN_OPTION:
		// 編集中のデータの戻し
		SetFromEdit( m_Line );

		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}


BOOL CDlgPluginOption::OnEnChange( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_EDIT_PLUGIN_OPTION:
	case IDC_EDIT_PLUGIN_OPTION_DIR:
	case IDC_EDIT_PLUGIN_OPTION_NUM:
		// 編集中のデータの戻し
		SetFromEdit( m_Line );

		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnEnChange( hwndCtl, wID );
}


BOOL CDlgPluginOption::OnActivate( WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD( wParam ) )
	{
	case WA_INACTIVE:
		SetFromEdit( m_Line );
		break;

	case WA_ACTIVE:
	case WA_CLICKACTIVE:
	default:
		break;
	}

	/* 基底クラスメンバ */
	return CDialog::OnActivate( wParam, lParam );
}


LPVOID CDlgPluginOption::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


void CDlgPluginOption::ChangeListPosition( void )
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );

	//	現在のFocus取得
	int current = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED);

	if( current == -1 || current == m_Line ){
		return;
	}

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	LVITEM	lvi;

// 戻し
	if (m_Line >= 0) {
		SetFromEdit( m_Line );
	}

	m_Line = current;

// 編集領域に書き込み
	SetToEdit(current);

	memset_raw( &lvi, 0, sizeof( lvi ));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = current;
	lvi.iSubItem   = 1;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE+1;

	ListView_GetItem( hwndList, &lvi );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf );
}

void CDlgPluginOption::MoveFocusToEdit( void )
{
	//	現在のFocus取得
	int		iLine = ListView_GetNextItem( GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS ), -1, LVNI_SELECTED);
	wstring	sType;
	HWND	hwndCtrl;

	if (iLine >= 0) {
		// Focusの切り替え
		sType = m_cPlugin->m_options[iLine]->GetType();
		transform( sType.begin(), sType.end(), sType.begin(), my_towlower2 );
		if (sType == OPTION_TYPE_BOOL) {
			hwndCtrl = GetItemHwnd( IDC_CHECK_PLUGIN_OPTION );
			::SetFocus( hwndCtrl );
		}
		else if (sType == OPTION_TYPE_INT) {
			hwndCtrl = GetItemHwnd( IDC_EDIT_PLUGIN_OPTION_NUM );
			::SetFocus( hwndCtrl );
		}
		else if (sType == OPTION_TYPE_SEL) {
			hwndCtrl = GetItemHwnd( IDC_COMBO_PLUGIN_OPTION );
			::SetFocus( hwndCtrl );
		}
		else if (sType == OPTION_TYPE_DIR) {
			hwndCtrl = GetItemHwnd( IDC_EDIT_PLUGIN_OPTION_DIR );
			::SetFocus( hwndCtrl );
		}
		else {
			hwndCtrl = GetItemHwnd( IDC_EDIT_PLUGIN_OPTION );
			::SetFocus( hwndCtrl );
		}
	}
}


// 編集領域に書き込み
void CDlgPluginOption::SetToEdit( int iLine )
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = iLine;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE+1;
		ListView_GetItem( hwndList, &lvi );

		sType = m_cPlugin->m_options[iLine]->GetType();
		transform( sType.begin(), sType.end(), sType.begin(), my_towlower2 );
		if (sType == OPTION_TYPE_BOOL) {
			::CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PLUGIN_OPTION, _tcscmp(buf,  BOOL_DISP_FALSE) != 0 );
			::DlgItem_SetText( GetHwnd(), IDC_CHECK_PLUGIN_OPTION, m_cPlugin->m_options[iLine]->GetLabel().c_str() );

			// 編集領域の切り替え
			SelectEdit(IDC_CHECK_PLUGIN_OPTION);
		}
		else if (sType == OPTION_TYPE_INT) {
			::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, buf );

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_NUM);
		}
		else if (sType == OPTION_TYPE_SEL) {
			// CONBO 設定
			std::vector<wstring>	selects;
			selects = m_cPlugin->m_options[iLine]->GetSelects();

			HWND	hwndCombo;
			hwndCombo = GetItemHwnd( IDC_COMBO_PLUGIN_OPTION );
			Combo_ResetContent( hwndCombo );

			int		nSelIdx;
			int		i;
			int		nItemIdx;
			wstring	sView;
			wstring	sValue;
			wstring	sWbuf = to_wchar(buf);
			nSelIdx = -1;		// 選択
			i = 0;
			for (auto it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sValue);
				nItemIdx = Combo_AddString( hwndCombo, sView.c_str() );
				if (sView == sWbuf) {
					nSelIdx = i;
				}
				Combo_SetItemData( hwndCombo, nItemIdx, i++ );
			}
			Combo_SetCurSel( hwndCombo, nSelIdx );

			// 編集領域の切り替え
			SelectEdit(IDC_COMBO_PLUGIN_OPTION);
		}
		else if (sType == OPTION_TYPE_DIR) {
			::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR, buf );

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_DIR);
		}
		else {
			::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf );

			// 編集領域の切り替え
			SelectEdit(IDC_EDIT_PLUGIN_OPTION);
		}
	}
}

// 編集領域の切り替え
void CDlgPluginOption::SelectEdit( int IDCenable )
{
	CtrlShow( GetHwnd(), IDC_EDIT_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION) );
	CtrlShow( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR) );
	CtrlShow( GetHwnd(), IDC_BUTTON_PLUGIN_OPTION_DIR,  (IDCenable == IDC_EDIT_PLUGIN_OPTION_DIR) );
	CtrlShow( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM,    (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM) );
	CtrlShow( GetHwnd(), IDC_SPIN_PLUGIN_OPTION,        (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM) );
	CtrlShow( GetHwnd(), IDC_CHECK_PLUGIN_OPTION,       (IDCenable == IDC_CHECK_PLUGIN_OPTION)  );
	CtrlShow( GetHwnd(), IDC_COMBO_PLUGIN_OPTION,       (IDCenable == IDC_COMBO_PLUGIN_OPTION)  );
}

// 編集領域から戻し
void CDlgPluginOption::SetFromEdit( int iLine )
{
	HWND hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	int		nVal;
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		sType = m_cPlugin->m_options[iLine]->GetType();
		transform(sType.begin (), sType.end (), sType.begin (), my_towlower2);
		if (sType == OPTION_TYPE_BOOL) {
			if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PLUGIN_OPTION ) ) {
				_tcscpy( buf, BOOL_DISP_TRUE );
			}
			else {
				_tcscpy( buf, BOOL_DISP_FALSE );
			}
			lvi.mask     = LVIF_TEXT;
			lvi.iItem    = iLine;
			lvi.iSubItem = 1;
			lvi.pszText  = buf;
			ListView_SetItem( hwndList, &lvi );
		}
		else if (sType == OPTION_TYPE_INT) {
			nVal = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE );
			auto_sprintf( buf, _T("%d"), nVal);
		}
		else if (sType == OPTION_TYPE_SEL) {
			::DlgItem_GetText( GetHwnd(), IDC_COMBO_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		}
		else if (sType == OPTION_TYPE_DIR) {
			::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR, buf, MAX_LENGTH_VALUE+1);
		}
		else {
			::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		}
		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = iLine;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem( hwndList, &lvi );
	}
}

// 選択用文字列分解
void CDlgPluginOption::SepSelect( wstring sTrg, wstring* spView, wstring* spValue )
{
	int		ix;
	ix = sTrg.find(L':');
	if ((std::wstring::size_type)ix == std::wstring::npos) {
		*spView = *spValue = sTrg;
	}
	else {
#ifdef _DEBUG
		*spView  = sTrg;
#else
		*spView  = sTrg.substr( 0, ix );
#endif
		*spValue = sTrg.substr( ix + 1 );
	}
}

// ディレクトリを選択する
void CDlgPluginOption::SelectDirectory( int iLine )
{
	TCHAR	szDir[_MAX_PATH+1];

	/* 検索フォルダ */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR, szDir, _countof(szDir) );

	if (_IS_REL_PATH( szDir )) {
		TCHAR	folder[_MAX_PATH];
		_tcscpy( folder, szDir );
		GetInidirOrExedir( szDir, folder );
	}

	// 項目名の取得
	HWND	hwndList = GetItemHwnd( IDC_LIST_PLUGIN_OPTIONS );
	LVITEM	lvi;
	TCHAR	buf[MAX_LENGTH_VALUE+1];
	memset_raw( &lvi, 0, sizeof( lvi ));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = iLine;
	lvi.iSubItem   = 0;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE+1;
	ListView_GetItem( hwndList, &lvi );

	TCHAR	sTitle[MAX_LENGTH_VALUE+10];
	auto_sprintf( sTitle, LS(STR_DLGPLUGINOPT_SELECT), buf);
	if (SelectDir( GetHwnd(), (const TCHAR*)sTitle /*_T("ディレクトリの選択")*/, szDir, szDir )) {
		//	末尾に\マークを追加する．
		AddLastChar( szDir, _countof(szDir), _T('\\') );
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_DIR, szDir );
	}
}
