/*!	@file
	@brief タイプ別設定ダイアログボックス

	@author Norio Nakatani
	@date 1998/12/24  新規作成
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
#include "Debug.h"
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "CDlgKeywordSelect.h"
#include "etc_uty.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "Funccode.h"	//Stonee, 2001/05/18
#include "CDlgSameColor.h"	// 2006.04.26 ryoji
#include "CEditApp.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids1[] = {	//11300
	IDC_EDIT_TYPENAME,				HIDC_EDIT_TYPENAME,			//設定の名前
	IDC_EDIT_TYPEEXTS,				HIDC_EDIT_TYPEEXTS,			//ファイル拡張子

	IDC_COMBO_WRAPMETHOD,			HIDC_COMBO_WRAPMETHOD,		//テキストの折り返し方法		// 2008.05.30 nasukoji
	IDC_EDIT_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,		//折り返し桁数
	IDC_SPIN_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,
	IDC_EDIT_CHARSPACE,				HIDC_EDIT_CHARSPACE,		//文字の間隔
	IDC_SPIN_CHARSPACE,				HIDC_EDIT_CHARSPACE,
	IDC_EDIT_LINESPACE,				HIDC_EDIT_LINESPACE,		//行の間隔
	IDC_SPIN_LINESPACE,				HIDC_EDIT_LINESPACE,
	IDC_EDIT_TABSPACE,				HIDC_EDIT_TABSPACE,			//TAB幅 // Sep. 19, 2002 genta
	IDC_SPIN_TABSPACE,				HIDC_EDIT_TABSPACE,
	IDC_EDIT_TABVIEWSTRING,			HIDC_EDIT_TABVIEWSTRING,	//TAB表示文字列
	IDC_CHECK_TAB_ARROW,			HIDC_CHECK_TAB_ARROW,		//矢印表示	// 2006.08.06 ryoji
	IDC_CHECK_INS_SPACE,			HIDC_CHECK_INS_SPACE,		//スペースの挿入

	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,			//自動インデント	// 2006.08.19 ryoji
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,	//全角空白もインデント	// 2006.08.19 ryoji
	IDC_COMBO_SMARTINDENT,			HIDC_COMBO_SMARTINDENT,		//スマートインデント
	IDC_EDIT_INDENTCHARS,			HIDC_EDIT_INDENTCHARS,		//その他のインデント対象文字
	IDC_COMBO_INDENTLAYOUT,			HIDC_COMBO_INDENTLAYOUT,	//折り返し行インデント	// 2006.08.06 ryoji
	IDC_CHECK_RTRIM_PREVLINE,		HIDC_CHECK_RTRIM_PREVLINE,	//改行時に末尾の空白を削除	// 2006.08.06 ryoji

	IDC_RADIO_OUTLINEDEFAULT,		HIDC_RADIO_OUTLINEDEFAULT,	//標準ルール	// 2006.08.06 ryoji
	IDC_COMBO_OUTLINES,				HIDC_COMBO_OUTLINES,		//アウトライン解析方法
	IDC_RADIO_OUTLINERULEFILE,		HIDC_RADIO_OUTLINERULEFILE,	//ルールファイル	// 2006.08.06 ryoji
	IDC_EDIT_OUTLINERULEFILE,		HIDC_EDIT_OUTLINERULEFILE,	//ルールファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_RULEFILE_REF,		HIDC_BUTTON_RULEFILE_REF,	//ルールファイル参照	// 2006/09/09 novice

	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,		//IMEのON/OFF状態
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,		//IMEの入力モード
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,			//文書アイコンを使う	// 2006.08.06 ryoji

	IDC_CHECK_WORDWRAP,				HIDC_CHECK_WORDWRAP,		//英文ワードラップ
	IDC_CHECK_KINSOKURET,			HIDC_CHECK_KINSOKURET,		//改行文字をぶら下げる	//@@@ 2002.04.14 MIK
	IDC_CHECK_KINSOKUKUTO,			HIDC_CHECK_KINSOKUKUTO,		//句読点をぶら下げる	//@@@ 2002.04.17 MIK
	IDC_EDIT_KINSOKUKUTO,			HIDC_EDIT_KINSOKUKUTO,		//句読点ぶら下げ文字	// 2009.08.07 ryoji
	IDC_CHECK_KINSOKUHEAD,			HIDC_CHECK_KINSOKUHEAD,		//行頭禁則	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUHEAD,			HIDC_EDIT_KINSOKUHEAD,		//行頭禁則	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKUTAIL,			HIDC_CHECK_KINSOKUTAIL,		//行末禁則	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUTAIL,			HIDC_EDIT_KINSOKUTAIL,		//行末禁則	//@@@ 2002.04.08 MIK
//	IDC_STATIC,						-1,
	0, 0
};
static const DWORD p_helpids2[] = {	//11400
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			//文字色
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			//背景色
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		//文字色統一
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		//背景色統一
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		//エクスポート
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				//色分け表示
	IDC_CHECK_FAT,					HIDC_CHECK_FAT,					//太字
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			//下線
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				//桁指定１
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				//桁指定２
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			//強調キーワード１セット名
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	//ブロックコメント１開始
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		//ブロックコメント１終了
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			//行コメント１
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			//行コメント２
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		//桁数１
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		//桁数２
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			//行番号区切り
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	//ブロックコメント２開始
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		//ブロックコメント２終了
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			//行コメント３
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				//色指定
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				//桁指定３
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		//桁数３
	IDC_RADIO_ESCAPETYPE_1,			HIDC_RADIO_ESCAPETYPE_1,		//文字列エスケープ（C言語風）
	IDC_RADIO_ESCAPETYPE_2,			HIDC_RADIO_ESCAPETYPE_2,		//文字列エスケープ（PL/SQL風）
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		//行番号の表示（折り返し単位）
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		//行番号の表示（改行単位）
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		//行番号区切り（なし）
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		//行番号区切り（縦線）
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		//行番号区切り（任意）
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		//強調キーワード2～10	// 2006.08.06 ryoji
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				//縦線の桁指定	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//単語ファイル名
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//入力補完 単語ファイル参照
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//入力補完の英大文字小文字
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//現在のファイルから入力補完
	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//外部ヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//外部ヘルプファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//外部HTMLヘルプファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//外部HTMLヘルプファイル参照	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//ビューアを複数起動しない	// 2006.08.06 ryoji
	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//デフォルト文字コード
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//デフォルト改行コード	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//デフォルトBOM	// 2011.01.24 ryoji
//	IDC_STATIC,						-1,
	0, 0
};

static const TCHAR* aszCodeStr[] = {
	_T("SJIS"),
	_T("EUC"),
	_T("UTF-8"),
	_T("Unicode"),
	_T("UnicodeBE")
};
static const ECodeType aeCodeType[] = {
	CODE_SJIS,
	CODE_EUC,
	CODE_UTF8,
	CODE_UNICODE,
	CODE_UNICODEBE
};
static const BOOL abBomEnable[] = {
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE
};
static const TCHAR* aszEolStr[] = {
	_T("CR+LF"),
	_T("LF (UNIX)"),
	_T("CR (Mac)"),
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      メッセージ処理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* Screen ダイアログプロシージャ */
INT_PTR CALLBACK PropTypesScreen(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypesScreen* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypesScreen* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypesScreen* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* color ダイアログプロシージャ */
INT_PTR CALLBACK PropTypesColor(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypesColor*		pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypesColor* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypesColor* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* support ダイアログプロシージャ */
INT_PTR CALLBACK PropTypesSupport(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypesSupport* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypesSupport* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypesSupport* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* 正規表現キーワード ダイアログプロシージャ */
INT_PTR CALLBACK PropTypesRegex(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypesRegex* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypesRegex* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}
		break;
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypesRegex* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, lParam );
		}
		break;
	}
	return FALSE;
}

/*! キーワード辞書ファイル設定 ダイアログプロシージャ

	@date 2006.04.10 fon 新規作成
*/
INT_PTR CALLBACK PropTypesKeyHelp(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypesKeyHelp* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypesKeyHelp* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}break;
	default:
		pCPropTypes = ( CPropTypesKeyHelp* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->DispatchEvent)( hwndDlg, uMsg, wParam, lParam );
		}break;
	}
	return FALSE;
}

CPropTypes::CPropTypes()
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更
	m_pCKeyWordSetMgr = &(m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr);

	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = 0;

	// 2005.11.30 Moca カスタム色を設定・保持
	int i;
	for( i = 0; i < _countof(m_dwCustColors); i++ ){
		m_dwCustColors[i] = RGB( 255, 255, 255 );
	}
	
	/* ヘルプファイルのフルパスを返す */
	m_pszHelpFile = CEditApp::GetHelpFilePath();
}

CPropTypes::~CPropTypes()
{
}

/* 初期化 */
void CPropTypes::Create( HINSTANCE hInstApp, HWND hwndParent )
{
	m_hInstance = hInstApp;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
}

// キーワード：タイプ別設定タブ順序(プロパティシート)
/* プロパティシートの作成 */
int CPropTypes::DoPropertySheet( int nPageNum )
{
	int					nRet;
	PROPSHEETPAGE		psp[16];
	int					nIdx;

	// 2005.11.30 Moca カスタム色の先頭にテキスト色を設定しておく
	m_dwCustColors[0] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT;
	m_dwCustColors[1] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	nIdx = 0;
	memset( &psp[nIdx], 0, sizeof( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof( psp[nIdx] );
	psp[nIdx].dwFlags     = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_SCREEN );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesScreen;
	psp[nIdx].pszTitle    = _T("スクリーン");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	memset( &psp[nIdx], 0, sizeof( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof( psp[nIdx] );
	psp[nIdx].dwFlags     = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_COLOR );
	psp[nIdx].pszIcon     = NULL /*MAKEINTRESOURCE( IDI_BORDER) */;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesColor;
	psp[nIdx].pszTitle    = _T("カラー");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	// 2001/06/14 Start by asa-o: タイプ別設定に支援タブ追加
	memset( &psp[nIdx], 0, sizeof( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof( psp[nIdx] );
	psp[nIdx].dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_SUPPORT );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesSupport;
	psp[nIdx].pszTitle    = _T("支援");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001/06/14 End

	// 2001.11.17 add start MIK タイプ別設定に正規表現キーワードタブ追加
	memset( &psp[nIdx], 0, sizeof( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof( psp[nIdx] );
	psp[nIdx].dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_REGEX );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesRegex;
	psp[nIdx].pszTitle    = _T("正規表現キーワード");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001.11.17 add end MIK

	// 2006.04.10 fon ADD-start タイプ別設定に「キーワードヘルプ」タブを追加
	memset( &psp[nIdx], 0, sizeof( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof( psp[nIdx] );
	psp[nIdx].dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_KEYHELP );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesKeyHelp;
	psp[nIdx].pszTitle    = _T("キーワードヘルプ");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2006.04.10 fon ADD-end

	PROPSHEETHEADER	psh;
	memset( &psh, 0, sizeof( psh ) );
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = m_hInstance;
	psh.pszIcon    = NULL;
	psh.pszCaption = (LPSTR)_T("タイプ別設定");	// Sept. 8, 2000 jepro 単なる「設定」から変更
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage は unsigned なので負にならない
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}
	else if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}
	else{
		psh.nStartPage = nPageNum;
	}

	if( psh.nPages - 1 < psh.nStartPage ){
		psh.nStartPage = psh.nPages - 1;
	}
	psh.ppsp = (LPCPROPSHEETPAGE)psp;
	psh.pfnCallback = NULL;

	nRet = MyPropertySheet( &psh );	// 2007.05.24 ryoji 独自拡張プロパティシート

	if( -1 == nRet ){
		TCHAR*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			_T("CPropTypes::DoPropertySheet()内でエラーが出ました。\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()失敗。\n")
			_T("\n")
			_T("%s\n"),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ヘルプ */
//2001.05.18 Stonee 機能番号からヘルプトピック番号を調べるようにした
//2001.07.03 JEPRO  支援タブのヘルプを有効化
//2001.11.17 MIK    IDD_PROP_REGEX
void CPropTypes::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP_SCREEN:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);			break;
	case IDD_PROP_COLOR:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);			break;
	case IDD_PROP_SUPPORT:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, m_pszHelpFile, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
}

/*[EOF]*/
