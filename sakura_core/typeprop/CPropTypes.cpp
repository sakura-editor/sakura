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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "CEditApp.h"
#include "view/colors/EColorIndexType.h"
#include "util/shell.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      メッセージ処理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

typedef INT_PTR (CPropTypes::*DISPATCH_EVENT_TYPE)(HWND,UINT,WPARAM,LPARAM);

// 共通ダイアログプロシージャ
INT_PTR CALLBACK PropTypesCommonProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, DISPATCH_EVENT_TYPE pDispatch)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = reinterpret_cast<CPropTypes*>(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return (pCPropTypes->*pDispatch)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return (pCPropTypes->*pDispatch)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

// 各種ダイアログプロシージャ
typedef	INT_PTR (CPropTypes::*pDispatchPage)( HWND, UINT, WPARAM, LPARAM );
#define GEN_PROPTYPES_CALLBACK(FUNC,CLASS) \
INT_PTR CALLBACK FUNC(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ \
	return PropTypesCommonProc(hwndDlg,uMsg,wParam,lParam,reinterpret_cast<pDispatchPage>(&CLASS::DispatchEvent)); \
}
GEN_PROPTYPES_CALLBACK(PropTypesScreen,		CPropTypesScreen)
GEN_PROPTYPES_CALLBACK(PropTypesWindow,		CPropTypesWindow)
GEN_PROPTYPES_CALLBACK(PropTypesColor,		CPropTypesColor)
GEN_PROPTYPES_CALLBACK(PropTypesSupport,	CPropTypesSupport)
GEN_PROPTYPES_CALLBACK(PropTypesRegex,		CPropTypesRegex)
GEN_PROPTYPES_CALLBACK(PropTypesKeyHelp,	CPropTypesKeyHelp)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CPropTypes::CPropTypes()
{
	{
		assert( sizeof(CPropTypesScreen)  - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesWindow)  - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesColor)   - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesSupport) - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesRegex)   - sizeof(CPropTypes) == 0 );
		assert( sizeof(CPropTypesKeyHelp) - sizeof(CPropTypes) == 0 );
	}

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	// Mar. 31, 2003 genta メモリ削減のためポインタに変更
	m_pCKeyWordSetMgr = &m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;

	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = ID_PROPTYPE_PAGENUM_SCREEN;

	(static_cast<CPropTypesScreen*>(this))->CPropTypes_Screen();
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

struct TypePropSheetInfo {
	int m_nTabNameId;										//!< TABの表示名
	unsigned int resId;										//!< Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};

// キーワード：タイプ別設定タブ順序(プロパティシート)
/* プロパティシートの作成 */
INT_PTR CPropTypes::DoPropertySheet( int nPageNum )
{
	INT_PTR				nRet;
	int					nIdx;

	// 2001/06/14 Start by asa-o: タイプ別設定に支援タブ追加
	// 2001.11.17 add start MIK タイプ別設定に正規表現キーワードタブ追加
	// 2006.04.10 fon ADD-start タイプ別設定に「キーワードヘルプ」タブを追加
	// 2013.03.10 aroka ADD-start タイプ別設定に「ウィンドウ」タブを追加
	static const TypePropSheetInfo TypePropSheetInfoList[] = {
		{ STR_PROPTYPE_SCREEN,			IDD_PROP_SCREEN,	PropTypesScreen },
		{ STR_PROPTYPE_COLOR,			IDD_PROP_COLOR,		PropTypesColor },
		{ STR_PROPTYPE_WINDOW,			IDD_PROP_WINDOW,	PropTypesWindow },
		{ STR_PROPTYPE_SUPPORT,			IDD_PROP_SUPPORT,	PropTypesSupport },
		{ STR_PROPTYPE_REGEX_KEYWORD,	IDD_PROP_REGEX,		PropTypesRegex },
		{ STR_PROPTYPE_KEYWORD_HELP,	IDD_PROP_KEYHELP,	PropTypesKeyHelp }
	};

	// カスタム色を共有メモリから取得
	memcpy_raw( m_dwCustColors, m_pShareData->m_dwCustColors, sizeof(m_dwCustColors) );
	// 2005.11.30 Moca カスタム色の先頭にテキスト色を設定しておく
	m_dwCustColors[0] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cTEXT;
	m_dwCustColors[1] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_sColorAttr.m_cBACK;

	std::tstring		sTabname[_countof(TypePropSheetInfoList)];
	m_bChangeKeyWordSet = false;
	PROPSHEETPAGE		psp[_countof(TypePropSheetInfoList)];
	for( nIdx = 0; nIdx < _countof(TypePropSheetInfoList); nIdx++ ){
		sTabname[nIdx] = LS(TypePropSheetInfoList[nIdx].m_nTabNameId);

		PROPSHEETPAGE *p = &psp[nIdx];
		memset_raw( p, 0, sizeof_raw( *p ) );
		p->dwSize      = sizeof_raw( *p );
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = CSelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE( TypePropSheetInfoList[nIdx].resId );
		p->pszIcon     = NULL;
		p->pfnDlgProc  = TypePropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = NULL;
	}

	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );

	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/ | PSH_USEPAGELANG;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = CSelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS( STR_PROPTYPE );	//_T("タイプ別設定");	// Sept. 8, 2000 jepro 単なる「設定」から変更
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
	psh.ppsp = psp;
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
			LS(STR_PROPTYPE_ERR),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	// カスタム色を共有メモリに設定
	memcpy_raw( m_pShareData->m_dwCustColors, m_dwCustColors, sizeof(m_dwCustColors) );

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
	case IDD_PROP_WINDOW:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_WINDOW);			break;
	case IDD_PROP_SUPPORT:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
}



/*!	コントロールにフォント設定する
	@date 2013.04.24 Uchi
*/
HFONT CPropTypes::SetCtrlFont( HWND hwndDlg, int idc_ctrl, const LOGFONT& lf )
{
	HFONT	hFont;
	HWND	hCtrl;

	// 論理フォントを作成
	hCtrl = ::GetDlgItem( hwndDlg, idc_ctrl );
	hFont = ::CreateFontIndirect( &lf );
	if (hFont) {
		// フォントの設定
		::SendMessage( hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0) );
	}

	return hFont;
}



/*!	フォントラベルにフォントとフォント名設定する
	@date 2013.04.24 Uchi
*/
HFONT CPropTypes::SetFontLabel( HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps, bool bUse)
{
	HFONT	hFont;
	TCHAR	szFontName[80];
	LOGFONT lfTemp;
	lfTemp = lf;
	// 大きすぎるフォントは小さく表示
	if( lfTemp.lfHeight < -16 ){
		lfTemp.lfHeight = -16;
	}

	if (bUse) {
		hFont = SetCtrlFont( hwndDlg, idc_static, lfTemp );

		// フォント名の設定
		auto_sprintf( szFontName, nps % 10 ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
			lf.lfFaceName, double(nps)/10 );
		::DlgItem_SetText( hwndDlg, idc_static, szFontName );
	}
	else {
		hFont = NULL;
		::DlgItem_SetText( hwndDlg, idc_static, _T("") );
	}

	return hFont;
}
