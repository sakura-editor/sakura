/*
	タイプ別設定 - スクリーン

	2008.04.12 kobake CPropTypes.cppから分離
*/
#include "stdafx.h"
#include "CPropTypes.h"
#include "sakura.hh"
#include "util/module.h"
#include "util/shell.h"
#include <windowsx.h> //Edit_LimitText
#include "util/window.h"
#include "types/CType.h"

static const DWORD p_helpids1[] = {	//11300
	IDC_CHECK_WORDWRAP,				HIDC_CHECK_WORDWRAP,		//英文ワードラップ
	IDC_EDIT_TABSPACE,				HIDC_EDIT_TABSPACE,			//TAB幅 // Sep. 19, 2002 genta
	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,		//IMEのON/OFF状態
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,		//IMEの入力モード
	IDC_COMBO_SMARTINDENT,			HIDC_COMBO_SMARTINDENT,		//スマートインデント
	IDC_COMBO_OUTLINES,				HIDC_COMBO_OUTLINES,		//アウトライン解析方法
	IDC_EDIT_TYPENAME,				HIDC_EDIT_TYPENAME,			//設定の名前
	IDC_EDIT_TYPEEXTS,				HIDC_EDIT_TYPEEXTS,			//ファイル拡張子
	IDC_EDIT_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,		//折り返し桁数
	IDC_EDIT_CHARSPACE,				HIDC_EDIT_CHARSPACE,		//文字の間隔
	IDC_EDIT_LINESPACE,				HIDC_EDIT_LINESPACE,		//行の間隔
	IDC_EDIT_INDENTCHARS,			HIDC_EDIT_INDENTCHARS,		//その他のインデント対象文字
	IDC_EDIT_TABVIEWSTRING,			HIDC_EDIT_TABVIEWSTRING,	//TAB表示文字列
	IDC_CHECK_INS_SPACE,			HIDC_CHECK_INS_SPACE,		//スペースの挿入
	IDC_SPIN_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,
	IDC_SPIN_CHARSPACE,				HIDC_EDIT_CHARSPACE,
	IDC_SPIN_LINESPACE,				HIDC_EDIT_LINESPACE,
	IDC_CHECK_KINSOKUHEAD,			HIDC_CHECK_KINSOKUHEAD,		//行頭禁則	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKUTAIL,			HIDC_CHECK_KINSOKUTAIL,		//行末禁則	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKURET,			HIDC_CHECK_KINSOKURET,		//改行文字をぶら下げる	//@@@ 2002.04.14 MIK
	IDC_CHECK_KINSOKUKUTO,			HIDC_CHECK_KINSOKUKUTO,		//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	IDC_EDIT_KINSOKUHEAD,			HIDC_EDIT_KINSOKUHEAD,		//行頭禁則	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUTAIL,			HIDC_EDIT_KINSOKUTAIL,		//行末禁則	//@@@ 2002.04.08 MIK
	IDC_CHECK_TAB_ARROW,			HIDC_CHECK_TAB_ARROW,		//矢印表示	// 2006.08.06 ryoji
	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,			//自動インデント	// 2006.08.19 ryoji
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,	//全角空白もインデント	// 2006.08.19 ryoji
	IDC_COMBO_INDENTLAYOUT,			HIDC_COMBO_INDENTLAYOUT,	//折り返し行インデント	// 2006.08.06 ryoji
	IDC_CHECK_RTRIM_PREVLINE,		HIDC_CHECK_RTRIM_PREVLINE,	//改行時に末尾の空白を削除	// 2006.08.06 ryoji
	IDC_RADIO_OUTLINEDEFAULT,		HIDC_RADIO_OUTLINEDEFAULT,	//標準ルール	// 2006.08.06 ryoji
	IDC_RADIO_OUTLINERULEFILE,		HIDC_RADIO_OUTLINERULEFILE,	//ルールファイル	// 2006.08.06 ryoji
	IDC_EDIT_OUTLINERULEFILE,		HIDC_EDIT_OUTLINERULEFILE,	//ルールファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_RULEFILE_REF,		HIDC_BUTTON_RULEFILE_REF,	//ルールファイル参照	// 2006/09/09 novice
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,			//文書アイコンを使う	// 2006.08.06 ryoji
	IDC_COMBO_WRAPMETHOD,			HIDC_COMBO_WRAPMETHOD,		//テキストの折り返し方法		// 2008.05.30 nasukoji
//	IDC_STATIC,						-1,
	0, 0
};


//2007.11.29 kobake 変数の意味を明確にするため、nMethos を テンプレート化。
template <class TYPE>
struct TYPE_NAME {
	TYPE		nMethod;
	TCHAR*		pszName;
};

TYPE_NAME<EOutlineType> OlmArr[] = {
//	{ OUTLINE_C,		_T("C") },
	{ OUTLINE_CPP,		_T("C/C++") },
	{ OUTLINE_PLSQL,	_T("PL/SQL") },
	{ OUTLINE_JAVA,		_T("Java") },
	{ OUTLINE_COBOL,	_T("COBOL") },
	{ OUTLINE_PERL,		_T("Perl") },				//Sep. 8, 2000 genta
	{ OUTLINE_ASM,		_T("アセンブラ") },
	{ OUTLINE_VB,		_T("Visual Basic") },		// 2001/06/23 N.Nakatani
	{ OUTLINE_PYTHON,	_T("Python") },				//	2007.02.08 genta
	{ OUTLINE_WZTXT,	_T("WZ階層付テキスト") },	// 2003.05.20 zenryaku, 2003.06.23 Moca 名称変更
	{ OUTLINE_HTML,		_T("HTML") },				// 2003.05.20 zenryaku
	{ OUTLINE_TEX,		_T("TeX") },				// 2003.07.20 naoh
	{ OUTLINE_TEXT,		_T("テキスト") }			//Jul. 08, 2001 JEPRO 常に最後尾におく
};


TYPE_NAME<ESmartIndentType> SmartIndentArr[] = {
	{ SMARTINDENT_NONE,	_T("なし") },
	{ SMARTINDENT_CPP,	_T("C/C++") }
};

//	Nov. 20, 2000 genta
TYPE_NAME<int> ImeStateArr[] = {
	{ 0, _T("標準設定") },
	{ 1, _T("全角") },
	{ 2, _T("全角ひらがな") },
	{ 3, _T("全角カタカナ") },
	{ 4, _T("無変換") }
};

TYPE_NAME<int> ImeSwitchArr[] = {
	{ 0, _T("そのまま") },
	{ 1, _T("常にON") },
	{ 2, _T("常にOFF") },
};

/*!	2行目以降のインデント方法

	@sa CLayoutMgr::SetLayoutInfo()
	@date Oct. 1, 2002 genta 
*/
TYPE_NAME<int> IndentTypeArr[] = {
	{ 0, _T("なし") },
	{ 1, _T("tx2x") },
	{ 2, _T("論理行先頭") },
};

// 2008.05.30 nasukoji	テキストの折り返し方法
TYPE_NAME<int> WrapMethodArr[] = {
	{ WRAP_NO_TEXT_WRAP,	_T("折り返さない") },
	{ WRAP_SETTING_WIDTH,	_T("指定桁で折り返す") },
	{ WRAP_WINDOW_WIDTH,	_T("右端で折り返す") },
};



/* p1 メッセージ処理 */
INT_PTR CPropTypes::DispatchEvent_Screen(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	switch( uMsg ){

	case WM_INITDIALOG:
		m_hwndThis = hwndDlg;
		/* ダイアログデータの設定 p1 */
		SetData_p1( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		// エディットコントロールの入力文字数制限
		Edit_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME        ), _countof( m_Types.m_szTypeName      ) - 1 );
		Edit_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS        ), _countof( m_Types.m_szTypeExts      ) - 1 );
		Edit_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_INDENTCHARS     ), _countof( m_Types.m_szIndentChars   ) - 1 );
		Edit_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING   ), _countof( m_Types.m_szTabViewString ) - 1 );
		Edit_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), _countof2( m_Types.m_szOutlineRuleFilename ) - 1 );	//	Oct. 5, 2002 genta 画面上でも入力制限

		if( 0 == m_Types.m_nIdx ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), FALSE );	//設定の名前
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), FALSE );	//ファイル拡張子
		}

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			/*	2002.04.01 YAZAKI オートインデントを削除（もともと不要）
				アウトライン解析にルールファイル関連を追加
			*/
			case IDC_RADIO_OUTLINEDEFAULT:	/* アウトライン解析→標準ルール */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

				//::SendMessageAny( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), CB_SETCURSEL, 0, 0 );

				return TRUE;
			case IDC_RADIO_OUTLINERULEFILE:	/* アウトライン解析→ルールファイル */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );
				return TRUE;

			case IDC_BUTTON_RULEFILE_REF:	/* アウトライン解析→ルールファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szOutlineRuleFilename ) ){
						GetInidirOrExedir( szPath, m_Types.m_szOutlineRuleFilename );
					}else{
						_tcscpy( szPath, m_Types.m_szOutlineRuleFilename );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szOutlineRuleFilename, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );
					}
				}
				return TRUE;

			case IDC_CHECK_TAB_ARROW:
				// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), FALSE );
				}
				else {
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), TRUE );
				}
				return TRUE;

			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_SPIN_MAXLINELEN:
			/* 折り返し桁数 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < MINLINEKETAS ){
				nVal = MINLINEKETAS;
			}
			if( nVal > MAXLINEKETAS ){
				nVal = MAXLINEKETAS;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			/* 文字の隙間 */
//			MYTRACE_A( "IDC_SPIN_CHARSPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				nVal = COLUMNSPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LINESPACE:
			/* 行の隙間 */
//			MYTRACE_A( "IDC_SPIN_LINESPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO 行間も最小0まで設定できるように変更(昔に戻っただけ?)
//			if( nVal < 1 ){
//				nVal = 1;
//			}
			if( nVal < 0 ){
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if( nVal > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				nVal = LINESPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_REPEATEDSCROLLLINENUM:
			/* キーリピート時のスクロール行数 */
//			MYTRACE_A( "IDC_SPIN_REPEATEDSCROLLLINENUM\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 10 ){
				nVal = 10;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_TABSPACE:
			//	Sep. 22, 2002 genta
			/* TAB幅 */
//			MYTRACE_A( "IDC_SPIN_CHARSPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 64 ){
				nVal = 64;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, nVal, FALSE );
			return TRUE;

		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROPTYPESP1 );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 p1 */
				GetData_p1( hwndDlg );

				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = 0;
				return TRUE;
			}
			break;
		}

//		MYTRACE_A( "pNMHDR->hwndFrom	=%xh\n",	pNMHDR->hwndFrom );
//		MYTRACE_A( "pNMHDR->idFrom	=%xh\n",	pNMHDR->idFrom );
//		MYTRACE_A( "pNMHDR->code		=%xh\n",	pNMHDR->code );
//		MYTRACE_A( "pMNUD->iPos		=%d\n",		pMNUD->iPos );
//		MYTRACE_A( "pMNUD->iDelta		=%d\n",		pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids1 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids1 );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}



/* ダイアログデータの設定 p1 */
void CPropTypes::SetData_p1( HWND hwndDlg )
{
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName );	//設定の名前
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts );	//ファイル拡張子

	//レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WRAPMETHOD );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		int		nSelPos = 0;
		for( int i = 0; i < _countof( WrapMethodArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)WrapMethodArr[i].pszName );
			if( WrapMethodArr[i].nMethod == m_Types.m_nTextWrapMethod ){		// テキストの折り返し方法
				nSelPos = i;
			}
		}
		::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

		::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, (Int)m_Types.m_nMaxLineKetas, FALSE );	// 折り返し文字数
		::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, m_Types.m_nColmSpace, FALSE );			// 文字の間隔
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, m_Types.m_nLineSpace, FALSE );			// 行の間隔
		::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, (Int)m_Types.m_nTabSpace, FALSE );			// TAB幅	//	Sep. 22, 2002 genta
		::DlgItem_SetText( hwndDlg, IDC_EDIT_TABVIEWSTRING, m_Types.m_szTabViewString );		// TAB表示(8文字)
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), !m_Types.m_bTabArrow );	// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる
		::CheckDlgButton( hwndDlg, IDC_CHECK_TAB_ARROW, m_Types.m_bTabArrow );					// 矢印表示 [チェックボックス]	//@@@ 2003.03.26 MIK
		::CheckDlgButton( hwndDlg, IDC_CHECK_INS_SPACE, m_Types.m_bInsSpace );					// SPACEの挿入 [チェックボックス]	// From Here 2001.12.03 hor
	}

	//インデント
	{
		/* 自動インデント */
		::CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENT, m_Types.m_bAutoIndent );

		/* 日本語空白もインデント */
		::CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENT_WSPACE, m_Types.m_bAutoIndent_ZENSPACE );

		/* スマートインデント種別 */
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		int		nSelPos = 0;
		for( int i = 0; i < _countof( SmartIndentArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)SmartIndentArr[i].pszName );
			if( SmartIndentArr[i].nMethod == m_Types.m_eSmartIndent ){	/* スマートインデント種別 */
				nSelPos = i;
			}
		}
		::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

		// その他のインデント対象文字
		::DlgItem_SetText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars );

		//折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		nSelPos = 0;
		for( int i = 0; i < _countof( IndentTypeArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)IndentTypeArr[i].pszName );
			if( IndentTypeArr[i].nMethod == m_Types.m_nIndentLayout ){	/* 折り返しインデント種別 */
				nSelPos = i;
			}
		}
		::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

		// 改行時に末尾の空白を削除	//2005.10.11 ryoji
		::CheckDlgButton( hwndDlg, IDC_CHECK_RTRIM_PREVLINE, m_Types.m_bRTrimPrevLine );
	}

	//起動時のIME(日本語入力変換)	//Nov. 20, 2000 genta
	{
		int ime;
		// ON/OFF状態
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		ime = m_Types.m_nImeState & 3;
		int		nSelPos = 0;
		for( int i = 0; i < _countof( ImeSwitchArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeSwitchArr[i].pszName );
			if( ImeSwitchArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

		// 入力モード
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		ime = m_Types.m_nImeState >> 2;
		nSelPos = 0;
		for( int i = 0; i < _countof( ImeStateArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeStateArr[i].pszName );
			if( ImeStateArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	}

	//アウトライン解析方法
	//2002.04.01 YAZAKI ルールファイル関連追加
	{
		//標準ルールのコンボボックス初期化
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
		::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );
		int		nSelPos = 0;
		for( int i = 0; i < _countof( OlmArr ); ++i ){
			::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)OlmArr[i].pszName );
			if( OlmArr[i].nMethod == m_Types.m_eDefaultOutline ){	/* アウトライン解析方法 */
				nSelPos = i;
			}
		}

		//標準ルール
		if( m_Types.m_eDefaultOutline != OUTLINE_FILE ){
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, TRUE );
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, FALSE );

			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

			::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
		}
		//ルールファイル
		else{
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, FALSE );
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, TRUE );

			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );
		}

		//ルールファイル	// 2003.06.23 Moca ルールファイル名は使わなくてもセットしておく
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
		::DlgItem_SetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );
	}

	//その他
	{
		/* 英文ワードラップをする */
		::CheckDlgButton( hwndDlg, IDC_CHECK_WORDWRAP, m_Types.m_bWordWrap );
		
		/* 禁則処理 */
		{	//@@@ 2002.04.08 MIK start
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUHEAD, m_Types.m_bKinsokuHead ? TRUE : FALSE );
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUTAIL, m_Types.m_bKinsokuTail ? TRUE : FALSE );
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKURET,  m_Types.m_bKinsokuRet  ? TRUE : FALSE );	/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUKUTO, m_Types.m_bKinsokuKuto ? TRUE : FALSE );	/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), EM_LIMITTEXT, _countof(m_Types.m_szKinsokuHead) - 1, 0 );
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), EM_LIMITTEXT, _countof(m_Types.m_szKinsokuTail) - 1, 0 );
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead );
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail );
		}	//@@@ 2002.04.08 MIK end
		
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		::CheckDlgButton( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon  ? TRUE : FALSE );
	}
}



/* ダイアログデータの取得 p1 */
int CPropTypes::GetData_p1( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 0;

	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName, _countof( m_Types.m_szTypeName ) );	// 設定の名前
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts, _countof( m_Types.m_szTypeExts ) );	// ファイル拡張子

	//レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WRAPMETHOD );
		int		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_nTextWrapMethod = WrapMethodArr[nSelPos].nMethod;		// テキストの折り返し方法

		/* 折り返し桁数 */
		m_Types.m_nMaxLineKetas = CLayoutInt(::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE ));
		if( m_Types.m_nMaxLineKetas < CLayoutInt(MINLINEKETAS) ){
			m_Types.m_nMaxLineKetas = CLayoutInt(MINLINEKETAS);
		}
		if( m_Types.m_nMaxLineKetas > CLayoutInt(MAXLINEKETAS) ){
			m_Types.m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);
		}

		/* 文字の間隔 */
		m_Types.m_nColmSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
		if( m_Types.m_nColmSpace < 0 ){
			m_Types.m_nColmSpace = 0;
		}
		if( m_Types.m_nColmSpace > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
			m_Types.m_nColmSpace = COLUMNSPACE_MAX;
		}

		/* 行の間隔 */
		m_Types.m_nLineSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
		if( m_Types.m_nLineSpace < 0 ){
			m_Types.m_nLineSpace = 0;
		}
		if( m_Types.m_nLineSpace > LINESPACE_MAX ){	// Feb. 18, 2003 genta 最大値の定数化
			m_Types.m_nLineSpace = LINESPACE_MAX;
		}

		/* TAB幅 */
		m_Types.m_nTabSpace = CLayoutInt(::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE ));
		if( m_Types.m_nTabSpace < CLayoutInt(1) ){
			m_Types.m_nTabSpace = CLayoutInt(1);
		}
		if( m_Types.m_nTabSpace > CLayoutInt(64) ){
			m_Types.m_nTabSpace = CLayoutInt(64);
		}

		/* TAB表示文字列 */
		WIN_CHAR szTab[8+1+1]; /* +1. happy */
		::DlgItem_GetText( hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, _countof( szTab ) - 1 );
		wcscpy( m_Types.m_szTabViewString, L"^       " );
		for( int i = 0; i < 8; i++ ){
			if( !TCODE::IsTabAvailableCode(szTab[i]) )break;
			m_Types.m_szTabViewString[i] = szTab[i];
		}

		// タブ矢印表示	//@@@ 2003.03.26 MIK
		m_Types.m_bTabArrow = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW ));

		// SPACEの挿入
		m_Types.m_bInsSpace = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INS_SPACE );
	}

	//インデント
	{
		/* 自動インデント */
		m_Types.m_bAutoIndent = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENT );

		/* 日本語空白もインデント */
		m_Types.m_bAutoIndent_ZENSPACE = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENT_WSPACE );

		/* スマートインデント種別 */
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
		int		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_eSmartIndent = SmartIndentArr[nSelPos].nMethod;	/* スマートインデント種別 */

		/* その他のインデント対象文字 */
		::DlgItem_GetText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars, _countof( m_Types.m_szIndentChars ) );

		// 折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_nIndentLayout = IndentTypeArr[nSelPos].nMethod;	/* 折り返し部インデント種別 */

		// 改行時に末尾の空白を削除	//2005.10.11 ryoji
		m_Types.m_bRTrimPrevLine = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RTRIM_PREVLINE );
	}

	//起動時のIME(日本語入力変換)	Nov. 20, 2000 genta
	{
		//ON/OFF状態
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		int		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF

		//入力モード
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME入力モード
	}

	//アウトライン解析方法
	//2002.04.01 YAZAKI ルールファイル関連追加
	{
		//標準ルール
		if ( !::IsDlgButtonChecked( hwndDlg, IDC_RADIO_OUTLINERULEFILE) ){
			HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
			int		nSelPos = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
			m_Types.m_eDefaultOutline = OlmArr[nSelPos].nMethod;	/* アウトライン解析方法 */
		}
		//ルールファイル
		else {
			m_Types.m_eDefaultOutline = OUTLINE_FILE;
		}

		//ルールファイル	//2003.06.23 Moca ルールを使っていなくてもファイル名を保持
		::DlgItem_GetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename, _countof2( m_Types.m_szOutlineRuleFilename ));
	}

	//その他
	{
		/* 英文ワードラップをする */
		m_Types.m_bWordWrap = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WORDWRAP );

		/* 禁則処理 */
		{	//@@@ 2002.04.08 MIK start
			m_Types.m_bKinsokuHead = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUHEAD ) ? TRUE : FALSE;
			m_Types.m_bKinsokuTail = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUTAIL ) ? TRUE : FALSE;
			m_Types.m_bKinsokuRet  = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKURET  ) ? TRUE : FALSE;	/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
			m_Types.m_bKinsokuKuto = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUKUTO ) ? TRUE : FALSE;	/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
			::DlgItem_GetText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead, _countof( m_Types.m_szKinsokuHead ) );
			::DlgItem_GetText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail, _countof( m_Types.m_szKinsokuTail ) );
		}	//@@@ 2002.04.08 MIK end

		// 文書アイコンを使う	//Sep. 10, 2002 genta
		m_Types.m_bUseDocumentIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DOCICON ) ? TRUE : FALSE;
	}

	return TRUE;
}

