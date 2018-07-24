/*
	タイプ別設定 - ウィンドウ

	2008.04.12 kobake CPropTypes.cppから分離
	2009.02.22 ryoji
*/
#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "dlg/CDlgOpenFile.h"
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "view/colors/EColorIndexType.h"
#include "charset/CCodePage.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids2[] = {	//11400
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,				//文書アイコンを使う	// 2006.08.06 ryoji

	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,		//IMEのON/OFF状態
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,		//IMEの入力モード

	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//デフォルト文字コード
	IDC_CHECK_CP,					HIDC_CHECK_TYPE_SUPPORT_CP,			//コードページ
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//デフォルト改行コード	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//デフォルトBOM	// 2011.01.24 ryoji
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,				//自動判別時にCESU-8を優先する

	IDC_EDIT_BACKIMG_PATH,			HIDC_EDIT_BACKIMG_PATH,			//背景画像
	IDC_BUTTON_BACKIMG_PATH_SEL,	HIDC_BUTTON_BACKIMG_PATH_SEL,	//背景画像ボタン
	IDC_COMBO_BACKIMG_POS,			HIDC_COMBO_BACKIMG_POS,			//背景画像位置
	IDC_CHECK_BACKIMG_SCR_X,		HIDC_CHECK_BACKIMG_SCR_X,		//背景画像ScrollX
	IDC_CHECK_BACKIMG_SCR_Y,		HIDC_CHECK_BACKIMG_SCR_Y,		//背景画像ScrollY
	IDC_CHECK_BACKIMG_REP_X,		HIDC_CHECK_BACKIMG_REP_X,		//背景画像RepeatX
	IDC_CHECK_BACKIMG_REP_Y,		HIDC_CHECK_BACKIMG_REP_Y,		//背景画像RepeatY
	IDC_EDIT_BACKIMG_OFFSET_X,		HIDC_EDIT_BACKIMG_OFFSET_X,		//背景画像OffsetX
	IDC_EDIT_BACKIMG_OFFSET_Y,		HIDC_EDIT_BACKIMG_OFFSET_Y,		//背景画像OffsetY
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		//行番号の表示（折り返し単位）
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		//行番号の表示（改行単位）
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		//行番号区切り（なし）
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		//行番号区切り（縦線）
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		//行番号区切り（任意）
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			//行番号区切り
	IDC_EDIT_LINENUMWIDTH,			HIDC_EDIT_LINENUMWIDTH,			//行番号の最小桁数 2014.08.02 katze
//	IDC_STATIC,						-1,
	0, 0
};



TYPE_NAME_ID<int> ImeSwitchArr[] = {
	{ 0, STR_IME_SWITCH_DONTSET },
	{ 1, STR_IME_SWITCH_ON },
	{ 2, STR_IME_SWITCH_OFF },
};

//	Nov. 20, 2000 genta
TYPE_NAME_ID<int> ImeStateArr[] = {
	{ 0, STR_IME_STATE_DEF },
	{ 1, STR_IME_STATE_FULL },
	{ 2, STR_IME_STATE_FULLHIRA },
	{ 3, STR_IME_STATE_FULLKATA },
	{ 4, STR_IME_STATE_NO }
};

static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
	L"NEL",
	L"LS",
	L"PS",
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
	EOL_NEL,
	EOL_LS,
	EOL_PS,
};

/*! window メッセージ処理 */
INT_PTR CPropTypesWindow::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;		// 追加 2014.08.02 katze

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ダイアログデータの設定 color */
		SetData( hwndDlg );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), 1 );

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */

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
			switch( wID ){
			case IDC_BUTTON_BACKIMG_PATH_SEL:
				{
					CDlgOpenFile::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH),
						_T("*.bmp;*.jpg;*.jpeg"), true, EFITER_NONE );
				}
				return TRUE;
			//	From Here Sept. 10, 2000 JEPRO
			//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
			case IDC_RADIO_LINETERMTYPE0: /* 行番号区切り 0=なし 1=縦線 2=任意 */
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			case IDC_CHECK_CP:
				{
					::CheckDlgButton( hwndDlg, IDC_CHECK_CP, TRUE );
					::EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_CP ), FALSE );
					CCodePage::AddComboCodePages( hwndDlg, ::GetDlgItem(hwndDlg, IDC_COMBO_DEFAULT_CODETYPE), -1 );
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//				OnHelp( hwndDlg, IDD_PROP1P3 );
			OnHelp( hwndDlg, IDD_PROP_WINDOW );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( _T("color PSN_KILLACTIVE\n") );
			/* ダイアログデータの取得 window */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PROPTYPE_PAGENUM_WINDOW;
			return TRUE;
		}

		// switch文追加 2014.08.02 katze
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( (int)wParam ) {
		case IDC_SPIN_LINENUMWIDTH:
			/* 行番号の最小桁数 */
//			MYTRACE( _T("IDC_SPIN_LINENUMWIDTH\n") );
			int nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < LINENUMWIDTH_MIN ){
				nVal = LINENUMWIDTH_MIN;
			}
			if( nVal > LINENUMWIDTH_MAX ){
				nVal = LINENUMWIDTH_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINENUMWIDTH, nVal, FALSE );
			return TRUE;
		}

		break;	/* WM_NOTIFY */

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}


void CPropTypesWindow::SetCombobox(HWND hwndWork, const int* nIds, int nCount, int select)
{
	Combo_ResetContent(hwndWork);
	for(int i = 0; i < nCount; ++i ){
		Combo_AddString( hwndWork, LS(nIds[i]) );
	}
	Combo_SetCurSel(hwndWork, select);
}



/* ダイアログデータの設定 window */
void CPropTypesWindow::SetData( HWND hwndDlg )
{
	{
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		::CheckDlgButtonBool( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon );
	}

	//起動時のIME(日本語入力変換)	//Nov. 20, 2000 genta
	{
		int ime;
		// ON/OFF状態
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState & 3;
		int		nSelPos = 0;
		for( int i = 0; i < _countof( ImeSwitchArr ); ++i ){
			Combo_InsertString( hwndCombo, i, LS(ImeSwitchArr[i].nNameId) );
			if( ImeSwitchArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		// 入力モード
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState >> 2;
		nSelPos = 0;
		for( int i = 0; i < _countof( ImeStateArr ); ++i ){
			Combo_InsertString( hwndCombo, i, LS(ImeStateArr[i].nNameId) );
			if( ImeStateArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );
	}

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// 「自動認識時にCESU-8を優先」m_Types.m_encoding.m_bPriorCesu8 をチェック
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_encoding.m_bPriorCesu8 );

		// デフォルトコードタイプのコンボボックス設定
		int		nSel= -1;
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
		if( nSel == -1 ){
			::CheckDlgButton( hwndDlg, IDC_CHECK_CP, TRUE );
			::EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_CP ), FALSE );
			int nIdx = CCodePage::AddComboCodePages( hwndDlg, hCombo, m_Types.m_encoding.m_eDefaultCodetype );
			if( nIdx == -1 ){
				nSel = 0;
			}else{
				nSel = nIdx;
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

	/* 行番号の表示 false=折り返し単位／true=改行単位 */
	if( !m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}

	{
		// 行番号の最小桁数	// 追加 2014.08.02 katze
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINENUMWIDTH, (Int)m_Types.m_nLineNumWidth, FALSE );
	}

	// 背景画像
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH), _countof2(m_Types.m_szBackImgPath));
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X), 5);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y), 5);

	DlgItem_SetText( hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath );
	{
		static const int posNameId[] ={
			STR_IMAGE_POS1,
			STR_IMAGE_POS2,
			STR_IMAGE_POS3,
			STR_IMAGE_POS4,
			STR_IMAGE_POS5,
			STR_IMAGE_POS6,
			STR_IMAGE_POS7,
			STR_IMAGE_POS8,
			STR_IMAGE_POS9,
		};
		/*BGIMAGE_TOP_LEFT .. */
		int nCount = _countof(posNameId);
		SetCombobox( ::GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS), posNameId, nCount, m_Types.m_backImgPos);
	}
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X, m_Types.m_backImgRepeatX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y, m_Types.m_backImgRepeatY);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X, m_Types.m_backImgScrollX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y, m_Types.m_backImgScrollY);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, m_Types.m_backImgPosOffset.x, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, m_Types.m_backImgPosOffset.y, TRUE);

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( 0 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 1 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 2 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE );
	}

	/* 行番号区切り文字 */
	wchar_t	szLineTermChar[2];
	auto_sprintf( szLineTermChar, L"%lc", m_Types.m_cLineTermChar );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000


	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* ダイアログデータの取得 color */
int CPropTypesWindow::GetData( HWND hwndDlg )
{
	{
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		m_Types.m_bUseDocumentIcon = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_DOCICON );
	}

	//起動時のIME(日本語入力変換)	Nov. 20, 2000 genta
	{
		//入力モード
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		int		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME入力モード

		//ON/OFF状態
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	}

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

	/* 行番号の表示 false=折り返し単位／true=改行単位 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = false;
	}else{
		m_Types.m_bLineNumIsCRLF = true;
	}

	DlgItem_GetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath, _countof2(m_Types.m_szBackImgPath));
	m_Types.m_backImgPos = static_cast<EBackgroundImagePos>(Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS)));
	m_Types.m_backImgRepeatX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X);
	m_Types.m_backImgRepeatY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y);
	m_Types.m_backImgScrollX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X);
	m_Types.m_backImgScrollY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y);
	m_Types.m_backImgPosOffset.x = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, NULL, TRUE);
	m_Types.m_backImgPosOffset.y = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, NULL, TRUE);

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE0 ) ){
		m_Types.m_nLineTermType = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE1 ) ){
		m_Types.m_nLineTermType = 1;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		m_Types.m_nLineTermType = 2;
	}
	
	/* 行番号区切り文字 */
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];

	/* 行番号の最小桁数 */	// 追加 2014.08.02 katze
	m_Types.m_nLineNumWidth = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINENUMWIDTH, NULL, FALSE );
	if( m_Types.m_nLineNumWidth < LINENUMWIDTH_MIN ){
		m_Types.m_nLineNumWidth = LINENUMWIDTH_MIN;
	}
	if( m_Types.m_nLineNumWidth > LINENUMWIDTH_MAX ){
		m_Types.m_nLineNumWidth = LINENUMWIDTH_MAX;
	}

	return TRUE;
}




//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropTypesWindow::EnableTypesPropInput( HWND hwndDlg )
{
	//	行番号区切りを任意の半角文字にするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), FALSE );
	}
}
//	To Here Sept. 10, 2000

