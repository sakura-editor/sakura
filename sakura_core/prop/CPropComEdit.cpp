/*!	@file
	@brief 共通設定ダイアログボックス、「編集」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10210
	IDC_CHECK_ADDCRLFWHENCOPY,			HIDC_CHECK_ADDCRLFWHENCOPY,				//折り返し行に改行を付けてコピー
	IDC_CHECK_COPYnDISABLESELECTEDAREA,	HIDC_CHECK_COPYnDISABLESELECTEDAREA,	//コピーしたら選択解除
	IDC_CHECK_bEnableNoSelectCopy,		HIDC_CHECK_bEnableNoSelectCopy,			//選択なしでコピーを可能にする	// 2007.11.18 ryoji
	IDC_CHECK_bEnableLineModePaste,		HIDC_CHECK_bEnableLineModePaste,		//ラインモード貼り付けを可能にする	// 2007.10.08 ryoji
	IDC_CHECK_DRAGDROP,					HIDC_CHECK_DRAGDROP,					//Drag&Drop編集する
	IDC_CHECK_DROPSOURCE,				HIDC_CHECK_DROPSOURCE,					//ドロップ元にする
	IDC_CHECK_bNotOverWriteCRLF,		HIDC_CHECK_bNotOverWriteCRLF,			//上書きモード
	IDC_CHECK_bOverWriteFixMode,		HIDC_CHECK_bOverWriteFixMode,			//文字幅に合わせてスペースを詰める
	IDC_CHECK_bOverWriteBoxDelete,		HIDC_CHECK_bOverWriteBoxDelete,			//矩形入力で選択範囲を削除する
	//	2007.02.11 genta クリッカブルURLをこのページに移動
	IDC_CHECK_bSelectClickedURL,	HIDC_CHECK_bSelectClickedURL,	//クリッカブルURL
	IDC_CHECK_CONVERTEOLPASTE,			HIDC_CHECK_CONVERTEOLPASTE,			//改行コードを変換して貼り付ける
	IDC_RADIO_CURDIR,					HIDC_RADIO_CURDIR,						//カレントフォルダ
	IDC_RADIO_MRUDIR,					HIDC_RADIO_MRUDIR,						//最近使ったフォルダ
	IDC_RADIO_SELDIR,					HIDC_RADIO_SELDIR,						//指定フォルダ
	IDC_EDIT_FILEOPENDIR,				HIDC_EDIT_FILEOPENDIR,					//指定フォルダパス
	IDC_BUTTON_FILEOPENDIR, 			HIDC_EDIT_FILEOPENDIR,					//指定フォルダパス
	IDC_CHECK_ENABLEEXTEOL,				HIDC_CHECK_ENABLEEXTEOL,				//改行コードNEL,PS,LSを有効にする
	IDC_CHECK_BOXSELECTLOCK,			HIDC_CHECK_BOXSELECTLOCK,				//矩形選択移動で選択をロックする
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropEdit::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropEdit::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* メッセージ処理 */
INT_PTR CPropEdit::DispatchEvent(
    HWND		hwndDlg,	// handle to dialog box
    UINT		uMsg,		// message
    WPARAM		wParam,		// first message parameter
    LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
//	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_FILEOPENDIR ), _MAX_PATH - 1 );
		/* ダイアログデータの設定 Edit */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_DRAGDROP:	/* タスクトレイを使う */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
				}
				else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
				}
				return TRUE;
			case IDC_RADIO_CURDIR:
			case IDC_RADIO_MRUDIR:
			case IDC_RADIO_SELDIR:
				EnableEditPropInput( hwndDlg );
				return TRUE;
			case IDC_BUTTON_FILEOPENDIR:
				{
					TCHAR szMetaPath[_MAX_PATH];
					TCHAR szPath[_MAX_PATH];
					::DlgItem_GetText( hwndDlg, IDC_EDIT_FILEOPENDIR, szMetaPath, _countof(szMetaPath) );
					CFileNameManager::ExpandMetaToFolder( szMetaPath, szPath, _countof(szPath) );
					if( SelectDir( hwndDlg, LS(STR_PROPEDIT_SELECT_DIR), szPath, szPath ) ){
						CNativeT cmem(szPath);
						cmem.Replace(_T("%"), _T("%%"));
						::DlgItem_SetText( hwndDlg, IDC_EDIT_FILEOPENDIR, cmem.GetStringPtr() );
					}
				}
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_EDIT );
			return TRUE;
		case PSN_KILLACTIVE:
			DEBUG_TRACE( _T("Edit PSN_KILLACTIVE\n") );

			/* ダイアログデータの取得 Edit */
			GetData( hwndDlg );
			return TRUE;

		case PSN_SETACTIVE: //@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			m_nPageNum = ID_PROPCOM_PAGENUM_EDIT;
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
void CPropEdit::SetData( HWND hwndDlg )
{
	/* ドラッグ & ドロップ編集 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DRAGDROP, m_Common.m_sEdit.m_bUseOLE_DragDrop );
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), TRUE );
	}
	else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DROPSOURCE ), FALSE );
	}

	/* DropSource */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DROPSOURCE, m_Common.m_sEdit.m_bUseOLE_DropSource );

	/* 折り返し行に改行を付けてコピー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_sEdit.m_bAddCRLFWhenCopy ? BST_CHECKED : BST_UNCHECKED );

	/* コピーしたら選択解除 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA, m_Common.m_sEdit.m_bCopyAndDisablSelection );

	/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_bEnableNoSelectCopy, m_Common.m_sEdit.m_bEnableNoSelectCopy );

	/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_bEnableLineModePaste, m_Common.m_sEdit.m_bEnableLineModePaste ? BST_CHECKED : BST_UNCHECKED );

	/* 改行は上書きしない */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bNotOverWriteCRLF, m_Common.m_sEdit.m_bNotOverWriteCRLF );

	// 文字幅に合わせてスペースを詰める
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_bOverWriteFixMode, m_Common.m_sEdit.m_bOverWriteFixMode );

	// 矩形入力で選択範囲を削除する
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_bOverWriteBoxDelete, m_Common.m_sEdit.m_bOverWriteBoxDelete );

	//	URLがクリックされたら選択するか */	// 2007.02.11 genta このページへ移動
	::CheckDlgButton( hwndDlg, IDC_CHECK_bSelectClickedURL, m_Common.m_sEdit.m_bSelectClickedURL );

	/*	改行コードを変換して貼り付ける */	// 2009.02.28 salarm
	::CheckDlgButton( hwndDlg, IDC_CHECK_CONVERTEOLPASTE, m_Common.m_sEdit.m_bConvertEOLPaste ? BST_CHECKED : BST_UNCHECKED );

	// ファイルダイアログの初期位置
	if( m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_CUR ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_CURDIR, TRUE );
	}
	if( m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_MRU ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_MRUDIR, TRUE );
	}
	if( m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_SEL ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_SELDIR, TRUE );
	}
	::DlgItem_SetText( hwndDlg, IDC_EDIT_FILEOPENDIR, m_Common.m_sEdit.m_OpenDialogSelDir );

	// 改行コードNEL,PS,LSを有効にする
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_ENABLEEXTEOL, m_Common.m_sEdit.m_bEnableExtEol );
	// 矩形選択移動で選択をロックする
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_BOXSELECTLOCK, m_Common.m_sEdit.m_bBoxSelectLock );

	EnableEditPropInput( hwndDlg );
}



/* ダイアログデータの取得 */
int CPropEdit::GetData( HWND hwndDlg )
{
	/* ドラッグ & ドロップ編集 */
	m_Common.m_sEdit.m_bUseOLE_DragDrop = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DRAGDROP );
	/* DropSource */
	m_Common.m_sEdit.m_bUseOLE_DropSource = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DROPSOURCE );

	/* 折り返し行に改行を付けてコピー */
	m_Common.m_sEdit.m_bAddCRLFWhenCopy = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY ));

	/* コピーしたら選択解除 */
	m_Common.m_sEdit.m_bCopyAndDisablSelection = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_COPYnDISABLESELECTEDAREA );

	/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
	m_Common.m_sEdit.m_bEnableNoSelectCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bEnableNoSelectCopy );

	/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
	m_Common.m_sEdit.m_bEnableLineModePaste = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bEnableLineModePaste ));

	/* 改行は上書きしない */
	m_Common.m_sEdit.m_bNotOverWriteCRLF = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bNotOverWriteCRLF );

	// 文字幅に合わせてスペースを詰める
	m_Common.m_sEdit.m_bOverWriteFixMode = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_bOverWriteFixMode );

	// 矩形入力で選択範囲を削除する
	m_Common.m_sEdit.m_bOverWriteBoxDelete = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_bOverWriteBoxDelete );

	/* URLがクリックされたら選択するか */	// 2007.02.11 genta このページへ移動
	m_Common.m_sEdit.m_bSelectClickedURL = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bSelectClickedURL );

	//	改行コードを変換して貼り付ける */	// 2009.02.28 salarm
	m_Common.m_sEdit.m_bConvertEOLPaste = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CONVERTEOLPASTE ));

	if( ::IsDlgButtonChecked(hwndDlg, IDC_RADIO_CURDIR) ){
		m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_CUR;
	}
	if( ::IsDlgButtonChecked(hwndDlg, IDC_RADIO_MRUDIR) ){
		m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_MRU;
	}
	if( ::IsDlgButtonChecked(hwndDlg, IDC_RADIO_SELDIR) ){
		m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_SEL;
	}
	::DlgItem_GetText( hwndDlg, IDC_EDIT_FILEOPENDIR, m_Common.m_sEdit.m_OpenDialogSelDir, _countof2(m_Common.m_sEdit.m_OpenDialogSelDir) );

	// 改行コードNEL,PS,LSを有効にする
	m_Common.m_sEdit.m_bEnableExtEol = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_ENABLEEXTEOL );
	// 矩形選択移動で選択をロックする
	m_Common.m_sEdit.m_bBoxSelectLock = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_BOXSELECTLOCK );

	return TRUE;
}

/*!	チェック状態に応じてダイアログボックス要素のEnable/Disableを
	適切に設定する

	@param hwndDlg プロパティシートのWindow Handle

	@date 2013.03.31 novice 新規作成
*/
void CPropEdit::EnableEditPropInput( HWND hwndDlg )
{
	// 指定フォルダ
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_SELDIR ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_FILEOPENDIR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_FILEOPENDIR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_FILEOPENDIR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_FILEOPENDIR ), FALSE );
	}
}


