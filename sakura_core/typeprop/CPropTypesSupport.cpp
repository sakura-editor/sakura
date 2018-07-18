/*! @file
	@brief タイプ別設定 - 支援

	@date 2008.04.12 kobake CPropTypes.cppから分離
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
#include "dlg/CDlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/file.h" // _IS_REL_PATH
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//単語ファイル名
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//入力補完 単語ファイル参照
	IDC_COMBO_HOKAN_TYPE,			HIDC_COMBO_HOKAN_TYPE,				//入力補完タイプ
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//入力補完の英大文字小文字
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//現在のファイルから入力補完
	IDC_CHECK_HOKANBYKEYWORD,		HIDC_CHECK_HOKANBYKEYWORD,			//強調キーワードから入力補完

	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//外部ヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//外部ヘルプファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//外部HTMLヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//外部HTMLヘルプファイル参照	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//ビューアを複数起動しない	// 2006.08.06 ryoji

	IDC_CHECK_CHKENTERATEND,		HIDC_CHECK_CHKENTERATEND,			//保存時に改行コードの混在を警告する	// 2013/4/14 Uchi
	IDC_CHECK_INDENTCPPSTR,			HIDC_CHECK_INDENTCPPSTR,
	IDC_CHECK_INDENTCPPCMT,			HIDC_CHECK_INDENTCPPCMT,
	IDC_CHECK_INDENTCPPUNDO,		HIDC_CHECK_INDENTCPPUNDO,
	//	IDC_STATIC,						-1,
	0, 0
};


struct SHokanMethod{
	int nMethod;
	std::wstring name;
};

static std::vector<SHokanMethod>* GetHokanMethodList()
{
	static std::vector<SHokanMethod> methodList;
	return &methodList;
}


// 2001/06/13 Start By asa-o: タイプ別設定の支援タブに関する処理

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
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), _MAX_PATH - 1 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完 単語ファイルの「参照...」ボタン */
				{
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_HOKANFILE), _T("*.kwd"), true, EFITER_TEXT);
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* 外部ヘルプ１の「参照...」ボタン */
				{
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTHELP), _T("*.hlp;*.chm;*.col"), true, EFITER_NONE);
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
				{
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP), _T("*.chm;*.col"), true, EFITER_NONE);
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
			m_nPageNum = ID_PROPTYPE_PAGENUM_SUPPORT;
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
	::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		std::vector<SHokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString( hCombo, LS(STR_SMART_INDENT_NONE) );
		Combo_SetCurSel( hCombo, 0 );
		size_t nSize = pMedothList->size();
		for( size_t i = 0; i < nSize; i++ ){
			ApiWrap::Combo_AddString( hCombo, (*pMedothList)[i].name.c_str() );
			if( m_Types.m_nHokanType == (*pMedothList)[i].nMethod ){
				Combo_SetCurSel( hCombo, i + 1 );
			}
		}
	}

//	2001/06/19 asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase ? BST_CHECKED : BST_UNCHECKED);

	// 2003.06.25 Moca ファイルからの補完機能
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_HOKANBYKEYWORD, m_Types.m_bUseHokanByKeyword );

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED);

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	::CheckDlgButton( hwndDlg, IDC_CHECK_CHKENTERATEND, m_Types.m_bChkEnterAtEnd ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENTCPPSTR,  m_Types.m_bIndentCppStringIgnore );
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENTCPPCMT,  m_Types.m_bIndentCppCommentIgnore );
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENTCPPUNDO, m_Types.m_bIndentCppUndoSep );
}

/* ダイアログデータの取得 */
int CPropTypesSupport::GetData( HWND hwndDlg )
{
//	2001/06/19	asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE ) != 0;

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE ) != 0;
	m_Types.m_bUseHokanByKeyword = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_HOKANBYKEYWORD );

	/* 入力補完 単語ファイル */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, _countof2( m_Types.m_szHokanFile ));

	// 入力補完種別
	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		int i = Combo_GetCurSel( hCombo );
		if( 0 == i ){
			m_Types.m_nHokanType = 0;
		}else if( CB_ERR != i ){
			m_Types.m_nHokanType = (*GetHokanMethodList())[i - 1].nMethod;
		}
	}

	//@@@ 2002.2.2 YAZAKI
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, _countof2( m_Types.m_szExtHelp ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, _countof2( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE ) != 0;

	// 保存時に改行コードの混在を警告する	2013/4/14 Uchi
	m_Types.m_bChkEnterAtEnd = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CHKENTERATEND ) != 0;


	m_Types.m_bIndentCppStringIgnore =  IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENTCPPSTR );
	m_Types.m_bIndentCppCommentIgnore = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENTCPPCMT );
	m_Types.m_bIndentCppUndoSep = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENTCPPUNDO );
	return TRUE;
}

// 2001/06/13 End

/*! 補完種別の追加
/*/
void CPropTypesSupport::AddHokanMethod(int nMethod, const WCHAR* szName)
{
	SHokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

void CPropTypesSupport::RemoveHokanMethod(int nMethod, const WCHAR* szName)
{
	int nSize = GetHokanMethodList()->size();
	for(int i = 0; i < nSize; i++ ){
		if( (*GetHokanMethodList())[i].nMethod == (EOutlineType)nMethod ){
			GetHokanMethodList()->erase( GetHokanMethodList()->begin() + i );
			break;
		}
	}
}
