/*
	タイプ別設定 - 支援

	2008.04.12 kobake CPropTypes.cppから分離
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
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//入力補完 単語ファイル参照
	IDC_COMBO_HOKAN_TYPE,			HIDC_COMBO_HOKAN_TYPE,				//入力補完タイプ
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//入力補完の英大文字小文字
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//現在のファイルから入力補完
	IDC_CHECK_HOKANBYKEYWORD,		HIDC_CHECK_HOKANBYKEYWORD,			//強調キーワードから入力補完
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//単語ファイル名
	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//外部ヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//外部ヘルプファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//外部HTMLヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//外部HTMLヘルプファイル参照	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//ビューアを複数起動しない	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//デフォルト文字コード
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//デフォルト改行コード	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//デフォルトBOM	// 2011.01.24 ryoji
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,				//自動判別時にCESU-8を優先する
//	IDC_STATIC,						-1,
	0, 0
};

//static const wchar_t* aszCodeStr[] = {
//	L"SJIS",
//	L"EUC",
//	L"UTF-8",
//	L"CESU-8",
//	L"Unicode",
//	L"UnicodeBE"
//};
//static const ECodeType aeCodeType[] = {
//	CODE_SJIS,
//	CODE_EUC,
//	CODE_UTF8,
//	CODE_CESU8,
//	CODE_UNICODE,
//	CODE_UNICODEBE
//};
//static const BOOL abBomEnable[] = {
//	FALSE,
//	FALSE,
//	TRUE,
//	TRUE,
//	TRUE,
//	TRUE
//};
static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
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
INT_PTR CPropSupport::DispatchEvent(
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
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// 文字コードの変更をBOMチェックボックスに反映
					i = Combo_GetCurSel( (HWND) lParam );
					if( CB_ERR != i ){
						CCodeTypeName	cCodeTypeName( Combo_GetItemData( (HWND)lParam, i ) );
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (cCodeTypeName.IsBomDefOn() ?  BST_CHECKED : BST_UNCHECKED) );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), cCodeTypeName.UseBom() );
					}
					break;
				}
			}
			break;

		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完 単語ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szHokanFile ) ){
						GetInidirOrExedir( szPath, m_Types.m_szHokanFile );
					}else{
						_tcscpy( szPath, m_Types.m_szHokanFile );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szHokanFile, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* 外部ヘルプ１の「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szExtHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szExtHtmlHelp ) ){
						GetInidirOrExedir( szPath, m_Types.m_szExtHtmlHelp, TRUE );
					}else{
						_tcscpy( szPath, m_Types.m_szExtHtmlHelp );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szExtHtmlHelp, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
					}
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
			m_nPageNum = 2;
			return TRUE;
		}
		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* ダイアログデータの設定 */
void CPropSupport::SetData( HWND hwndDlg )
{
	/* 入力補完 単語ファイル */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		std::vector<SHokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString( hCombo, L"なし" );
		Combo_SetCurSel( hCombo, 0 );
		for( size_t i = 0; i < pMedothList->size(); i++ ){
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
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle );

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// 「自動認識時にCESU-8を優先」m_Types.m_encoding.m_bPriorCesu8 をチェック
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_encoding.m_bPriorCesu8 );

		// デフォルトコードタイプのコンボボックス設定
		int		nSel= 0;
		int		j = 0;
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		CCodeTypesForCombobox cCodeTypes;
		for (i = 0; i < cCodeTypes.GetCount(); i++) {
			if (CCodeTypeName( cCodeTypes.GetCode(i) ).CanDefault()) {
				int idx = Combo_AddString( hCombo, cCodeTypes.GetName(i) );
				Combo_SetItemData( hCombo, idx, cCodeTypes.GetCode(i) );
				if (m_Types.m_encoding.m_eDefaultCodetype == cCodeTypes.GetCode(i)) {
					nSel = j;
				}
				j++;
			}
		}
		Combo_SetCurSel( hCombo, nSel );

		// BOM チェックボックス設定
		CCodeTypeName	cCodeTypeName(m_Types.m_encoding.m_eDefaultCodetype);
		if( !cCodeTypeName.UseBom() )
			m_Types.m_encoding.m_bDefaultBom = false;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (m_Types.m_encoding.m_bDefaultBom ? BST_CHECKED : BST_UNCHECKED) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), (int)cCodeTypeName.UseBom() );

		// デフォルト改行タイプのコンボボックス設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_encoding.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		Combo_SetCurSel( hCombo, i );
	}
}

/* ダイアログデータの取得 */
int CPropSupport::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 2;

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

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// m_Types.m_bPriorCesu8 を設定
		m_Types.m_encoding.m_bPriorCesu8 = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PRIOR_CESU8 ) != 0;

		// m_Types.eDefaultCodetype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultCodetype = ECodeType( Combo_GetItemData( hCombo, i ) );
		}

		// m_Types.m_bDefaultBom を設定
		m_Types.m_encoding.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM ) != 0;

		// m_Types.eDefaultEoltype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultEoltype = aeEolType[i];
		}
	}


	return TRUE;
}

// 2001/06/13 End

/*! 補完種別の追加
/*/
void CPropSupport::AddHokanMethod(int nMethod, const WCHAR* szName)
{
	SHokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

