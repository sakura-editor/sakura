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
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CPropTypes.h"
#include "util/shell.h"
using namespace std;


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
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
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
#define GEN_PROPTYPES_CALLBACK(FUNC,METHOD) \
INT_PTR CALLBACK FUNC(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) \
{ \
	return PropTypesCommonProc(hwndDlg,uMsg,wParam,lParam,&CPropTypes::METHOD); \
}
GEN_PROPTYPES_CALLBACK(PropTypesP1Proc,	DispatchEvent_Screen)
GEN_PROPTYPES_CALLBACK(PropTypesP2Proc,	DispatchEvent_Color)
GEN_PROPTYPES_CALLBACK(PropTypesP3Proc,	DispatchEvent_Support)
GEN_PROPTYPES_CALLBACK(PropTypesRegex,	DispatchEvent_Regex)
GEN_PROPTYPES_CALLBACK(PropTypesKeyHelp,DispatchEvent_KeyHelp)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CPropTypes::CPropTypes()
{
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更
	m_pCKeyWordSetMgr = &(GetDllShareData().m_CKeyWordSetMgr);

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
	m_szHelpFile = CEditApp::Instance()->GetHelpFilePath();
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
	memset_raw( &psp[nIdx], 0, sizeof_raw( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof_raw( psp[nIdx] );
	psp[nIdx].dwFlags     = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP1 );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesP1Proc;
	psp[nIdx].pszTitle    = _T("スクリーン");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	memset_raw( &psp[nIdx], 0, sizeof_raw( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof_raw( psp[nIdx] );
	psp[nIdx].dwFlags     = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_COLOR );
	psp[nIdx].pszIcon     = NULL /*MAKEINTRESOURCE( IDI_BORDER) */;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesP2Proc;
	psp[nIdx].pszTitle    = _T("カラー");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	// 2001/06/14 Start by asa-o: タイプ別設定に支援タブ追加
	memset_raw( &psp[nIdx], 0, sizeof_raw( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof_raw( psp[nIdx] );
	psp[nIdx].dwFlags     = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance   = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP2 );
	psp[nIdx].pszIcon     = NULL;
	psp[nIdx].pfnDlgProc  = (DLGPROC)PropTypesP3Proc;
	psp[nIdx].pszTitle    = _T("支援");
	psp[nIdx].lParam      = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001/06/14 End

	// 2001.11.17 add start MIK タイプ別設定に正規表現キーワードタブ追加
	memset_raw( &psp[nIdx], 0, sizeof_raw( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof_raw( psp[nIdx] );
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
	memset_raw( &psp[nIdx], 0, sizeof_raw( psp[nIdx] ) );
	psp[nIdx].dwSize      = sizeof_raw( psp[nIdx] );
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


	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	// JEPROtest Sept. 30, 2000 タイプ別設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = m_hInstance;
	psh.pszIcon    = NULL;
	psh.pszCaption = _T("タイプ別設定");	// Sept. 8, 2000 jepro 単なる「設定」から変更
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
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, _T("作者に教えて欲しいエラー"),
			_T("CPropTypes::DoPropertySheet()内でエラーが出ました。\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()失敗。\n")
			_T("\n")
			_T("%ts\n"),
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
	case IDD_PROPTYPESP1:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);			break;
	case IDD_PROP_COLOR:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);			break;
	case IDD_PROPTYPESP2:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);			break;
	case IDD_PROP_REGEX:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);	break;
	case IDD_PROP_KEYHELP:	nContextID = ::FuncID_To_HelpContextID(F_TYPE_KEYHELP);			break;
	default:				nContextID = -1;												break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, m_szHelpFile, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
}



