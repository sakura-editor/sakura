/*!	@file
	@brief 文字コードセット設定ダイアログボックス

	@author Uchi
	@date 2010/6/14  新規作成
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgSetCharSet.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "env/DLLSHAREDATA.h"
#include "charset/CCodePage.h"
#include "sakura_rc.h"
#include "sakura.hh"

//文字コードセット設定 CDlgSetCharSet
const DWORD p_helpids[] = {
	IDOK,							HIDOK_GREP,							//検索
	IDCANCEL,						HIDCANCEL_GREP,						//キャンセル
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				//ヘルプ
	IDC_COMBO_CHARSET,				HIDC_OPENDLG_COMBO_CODE,			//文字コードセット
	IDC_CHECK_BOM,					HIDC_OPENDLG_CHECK_BOM,				//条件
	IDC_CHECK_CP,					HIDC_OPENDLG_CHECK_CP,				//CP
	0, 0
};



CDlgSetCharSet::CDlgSetCharSet()
{
	m_pnCharSet = NULL;			// 文字コードセット
	m_pbBom = NULL;				// 文字コードセット
	m_bCP = false;
}



/* モーダルダイアログの表示 */
int CDlgSetCharSet::DoModal( HINSTANCE hInstance, HWND hwndParent, ECodeType* pnCharSet, bool* pbBom)
{
	m_pnCharSet = pnCharSet;	// 文字コードセット
	m_pbBom = pbBom;			// BOM

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_SETCHARSET, (LPARAM)NULL );
}



BOOL CDlgSetCharSet::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	
	m_hwndCharSet = GetItemHwnd( IDC_COMBO_CHARSET );	// 文字コードセットコンボボックス
	m_hwndCheckBOM = GetItemHwnd( IDC_CHECK_BOM );		// BOMチェックボックス

	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI( m_hwndCharSet, TRUE );

	// 文字コードセット選択コンボボックス初期化
	int i;
	CCodeTypesForCombobox cCodeTypes;
	Combo_ResetContent( m_hwndCharSet );
	for (i = 1; i < cCodeTypes.GetCount(); ++i) {
		int idx = Combo_AddString( m_hwndCharSet, cCodeTypes.GetName(i) );
		Combo_SetItemData( m_hwndCharSet, idx, cCodeTypes.GetCode(i) );
	}

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}



BOOL CDlgSetCharSet::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_CHECK_CP:
		if( !m_bCP ){
			m_bCP = true;
			::EnableWindow( GetItemHwnd( IDC_CHECK_CP ), FALSE );
			CCodePage::AddComboCodePages( GetHwnd(), m_hwndCharSet, -1 );
		}
		return TRUE;
	case IDC_BUTTON_HELP:
		/* 「文字コードセット設定」のヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CHG_CHARSET) );
		return TRUE;
	case IDOK:
		/* ダイアログデータの取得 */
		if( GetData() ){
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}



// BOM の設定
void CDlgSetCharSet::SetBOM( void )
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	nIdx = Combo_GetCurSel( m_hwndCharSet );
	lRes = Combo_GetItemData( m_hwndCharSet, nIdx );
	CCodeTypeName	cCodeTypeName( lRes );
	if (cCodeTypeName.UseBom()) {
		::EnableWindow( m_hwndCheckBOM, TRUE );
		if (lRes == *m_pnCharSet) {
			fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
		}
		else{
			fCheck = cCodeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
		}
	}
	else {
		::EnableWindow( m_hwndCheckBOM, FALSE );
		fCheck = BST_UNCHECKED;
	}
	BtnCtl_SetCheck( m_hwndCheckBOM, fCheck );
}



// 文字コード選択時の処理
BOOL CDlgSetCharSet::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	switch (wID) {
	//	文字コードの変更をBOMチェックボックスに反映
	case IDC_COMBO_CHARSET:
		SetBOM();
		nIdx = Combo_GetCurSel( hwndCtl );
		lRes = Combo_GetItemData( hwndCtl, nIdx );
		CCodeTypeName	cCodeTypeName( lRes );
		if (cCodeTypeName.UseBom()) {
			::EnableWindow( m_hwndCheckBOM, TRUE );
			if (lRes == *m_pnCharSet) {
				fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
			}
			else{
				fCheck = cCodeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
			}
		}
		else {
			::EnableWindow( m_hwndCheckBOM, FALSE );
			fCheck = BST_UNCHECKED;
		}
		BtnCtl_SetCheck( m_hwndCheckBOM, fCheck );
		break;
	}
	return TRUE;
}



LPVOID CDlgSetCharSet::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}



/* ダイアログデータの設定 */
void CDlgSetCharSet::SetData( void )
{
	// 文字コードセット
	int		nIdx, nCurIdx, nIdxOld;
	ECodeType nCharSet;
	CCodeTypesForCombobox cCodeTypes;

	nIdxOld = Combo_GetCurSel( m_hwndCharSet );
	nCurIdx = -1;
	for (nIdx = 0; nIdx < Combo_GetCount( m_hwndCharSet ); nIdx++) {
		nCharSet = (ECodeType)Combo_GetItemData( m_hwndCharSet, nIdx );
		if (nCharSet == *m_pnCharSet) {
			nCurIdx = nIdx;
		}
	}
	if( -1 == nCurIdx ){
		m_bCP = true;
		::CheckDlgButton( GetHwnd(), IDC_CHECK_CP, TRUE );
		::EnableWindow( GetItemHwnd( IDC_CHECK_CP ), FALSE );
		nCurIdx = CCodePage::AddComboCodePages( GetHwnd(), m_hwndCharSet, *m_pnCharSet );
		if( nCurIdx == -1 ){
			nCurIdx = nIdxOld;
		}
	}
	Combo_SetCurSel( m_hwndCharSet, nCurIdx );

	// BOMを設定
	SetBOM();
}



/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー  */
int CDlgSetCharSet::GetData( void )
{
	// 文字コードセット
	int		nIdx;
	nIdx = Combo_GetCurSel( m_hwndCharSet );
	*m_pnCharSet = (ECodeType)Combo_GetItemData( m_hwndCharSet, nIdx );

	// BOM
	*m_pbBom = ( BtnCtl_GetCheck( m_hwndCheckBOM ) == BST_CHECKED );

	return TRUE;
}
