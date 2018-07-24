/*!	@file
	@brief 共通設定ダイアログボックス、「全般」ページ

	@author Norio Nakatani
	@date 1998/12/24 新規作成
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, hor, Stonee, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, MIK, Moca, こおり
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Uchi
	Copyright (C) 2013, Uchi

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
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CDocTypeManager.h"
#include "CEditApp.h"
#include "util/shell.h"
#include "sakura_rc.h"

int	CPropCommon::SearchIntArr( int nKey, int* pnArr, int nArrNum )
{
	int i;
	for( i = 0; i < nArrNum; ++i ){
		if( nKey == pnArr[i] ){
			return i;
		}
	}
	return -1;
}


/*!
	プロパティページごとのWindow Procedureを引数に取ることで
	処理の共通化を狙った．

	@param DispatchPage 真のWindow Procedureのメンバ関数ポインタ
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CPropCommon::DlgProc(
	INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE*	pPsp;
	CPropCommon*	pCPropCommon;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropCommon = ( CPropCommon* )(pPsp->lParam);
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropCommon = ( CPropCommon* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}
//	To Here Jun. 2, 2001 genta

// 独立ウィンドウ用 2013.3.14 aroka
INT_PTR CPropCommon::DlgProc2(
	INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	CPropCommon*	pCPropCommon;
	switch( uMsg ){
	case WM_INITDIALOG:
		pCPropCommon = ( CPropCommon* )(lParam);
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, IDOK, lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropCommon = ( CPropCommon* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CPropCommon::CPropCommon()
{
	{
		assert( sizeof(CPropGeneral)   - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropWin)       - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropMainMenu)  - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropToolbar)   - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropTab)       - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropStatusbar) - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropEdit)      - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropFile)      - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropFileName)  - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropBackup)    - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropFormat)    - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropGrep)      - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropKeybind)   - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropCustmenu)  - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropKeyword)   - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropHelper)    - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropMacro)     - sizeof(CPropCommon) == 0 );
		assert( sizeof(CPropPlugin)    - sizeof(CPropCommon) == 0 );
	}

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = ID_PROPCOM_PAGENUM_GENERAL;
	m_nKeywordSet1 = -1;

	return;
}





CPropCommon::~CPropCommon()
{
}





/* 初期化 */
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
void CPropCommon::Create( HWND hwndParent, CImageListMgr* pcIcons, CMenuDrawer* pMenuDrawer )
{
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_pcIcons = pcIcons;

	// 2007.11.02 ryoji マクロ設定を変更したあと、画面を閉じないでカスタムメニュー、ツールバー、
	//                  キー割り当ての画面に切り替えた時に各画面でマクロ設定の変更が反映されるよう、
	//                  m_Common.m_sMacro.m_MacroTable（ローカルメンバ）でm_cLookupを初期化する
	m_cLookup.Init( m_Common.m_sMacro.m_MacroTable, &m_Common );	//	機能名・番号resolveクラス．

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	m_pcMenuDrawer = pMenuDrawer;

	return;
}






//	From Here Jun. 2, 2001 genta
/*!
	「共通設定」プロパティシートの作成時に必要な情報を
	保持する構造体
*/
struct ComPropSheetInfo {
	int m_nTabNameId;										//!< TABの表示名
	unsigned int resId;										//!< Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	キーワード：共通設定タブ順序(プロパティシート)
/*! プロパティシートの作成
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
INT_PTR CPropCommon::DoPropertySheet( int nPageNum, bool bTrayProc )
{
	INT_PTR				nRet;
	int					nIdx;

	m_bTrayProc = bTrayProc;

	//	From Here Jun. 2, 2001 genta
	//	Feb. 11, 2007 genta URLをTABと入れ換え	// 2007.02.13 順序変更（TABをWINの次に）
	//!	「共通設定」プロパティシートの作成時に必要な情報の配列．
	//	順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に	2008/6/22 Uchi 
	//	DProcの変更	2010/5/9 Uchi
	static const ComPropSheetInfo ComPropSheetInfoList[] = {
		{ STR_PROPCOMMON_GENERAL,	IDD_PROP_GENERAL,	CPropGeneral::DlgProc_page },
		{ STR_PROPCOMMON_WINDOW,	IDD_PROP_WIN,		CPropWin::DlgProc_page },
		{ STR_PROPCOMMON_MAINMENU,	IDD_PROP_MAINMENU,	CPropMainMenu::DlgProc_page },	// 2010/5/8 Uchi
		{ STR_PROPCOMMON_TOOLBAR,	IDD_PROP_TOOLBAR,	CPropToolbar::DlgProc_page },
		{ STR_PROPCOMMON_TABS,		IDD_PROP_TAB,		CPropTab::DlgProc_page },
		{ STR_PROPCOMMON_STATBAR,	IDD_PROP_STATUSBAR,	CPropStatusbar::DlgProc_page },	// 文字コード表示指定	2008/6/21	Uchi
		{ STR_PROPCOMMON_EDITING,	IDD_PROP_EDIT,		CPropEdit::DlgProc_page },
		{ STR_PROPCOMMON_FILE,		IDD_PROP_FILE,		CPropFile::DlgProc_page },
		{ STR_PROPCOMMON_FILENAME,	IDD_PROP_FNAME,		CPropFileName::DlgProc_page },
		{ STR_PROPCOMMON_BACKUP,	IDD_PROP_BACKUP,	CPropBackup::DlgProc_page },
		{ STR_PROPCOMMON_FORMAT,	IDD_PROP_FORMAT,	CPropFormat::DlgProc_page },
		{ STR_PROPCOMMON_SEARCH,	IDD_PROP_GREP,		CPropGrep::DlgProc_page },	// 2006.08.23 ryoji タイトル変更（Grep -> 検索）
		{ STR_PROPCOMMON_KEYS,		IDD_PROP_KEYBIND,	CPropKeybind::DlgProc_page },
		{ STR_PROPCOMMON_CUSTMENU,	IDD_PROP_CUSTMENU,	CPropCustmenu::DlgProc_page },
		{ STR_PROPCOMMON_KEYWORD,	IDD_PROP_KEYWORD,	CPropKeyword::DlgProc_page },
		{ STR_PROPCOMMON_SUPPORT,	IDD_PROP_HELPER,	CPropHelper::DlgProc_page },
		{ STR_PROPCOMMON_MACRO,		IDD_PROP_MACRO,		CPropMacro::DlgProc_page },
		{ STR_PROPCOMMON_PLUGIN,	IDD_PROP_PLUGIN,	CPropPlugin::DlgProc_page },
	};

	std::tstring		sTabname[_countof(ComPropSheetInfoList)];
	PROPSHEETPAGE		psp[_countof(ComPropSheetInfoList)];
	for( nIdx = 0; nIdx < _countof(ComPropSheetInfoList); nIdx++ ){
		sTabname[nIdx] = LS(ComPropSheetInfoList[nIdx].m_nTabNameId);

		PROPSHEETPAGE *p = &psp[nIdx];
		memset_raw( p, 0, sizeof_raw( *p ) );
		p->dwSize      = sizeof_raw( *p );
		p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
		p->hInstance   = CSelectLang::getLangRsrcInstance();
		p->pszTemplate = MAKEINTRESOURCE( ComPropSheetInfoList[nIdx].resId );
		p->pszIcon     = NULL;
		p->pfnDlgProc  = ComPropSheetInfoList[nIdx].DProc;
		p->pszTitle    = sTabname[nIdx].c_str();
		p->lParam      = (LPARAM)this;
		p->pfnCallback = NULL;
	}
	//	To Here Jun. 2, 2001 genta

	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	JEPROtest Sept. 30, 2000 共通設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEPAGELANG;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = CSelectLang::getLangRsrcInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = LS( STR_PROPCOMMON );	//_T("共通設定");
	psh.nPages     = nIdx;

	//- 20020106 aroka # psh.nStartPage は unsigned なので負にならない
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}else
	if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}else{
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
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// デフォルト言語
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		PleaseReportToAuthor(
			NULL,
			LS(STR_ERR_DLGPROPCOMMON24),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}

/*!	ShareDataから一時領域へ設定をコピーする
	@param[in] tempTypeKeywordSet キーワードセット
	@param[in] name	タイプ属性：名称
	@param[in] exts	タイプ属性：拡張子リスト

	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::InitData( const int* tempTypeKeywordSet, const TCHAR* name, const TCHAR* exts )
{
	m_Common = m_pShareData->m_Common;
	m_tempTypeName[0] = _T('\0');
	m_tempTypeExts[0] = _T('\0');

	//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
	if( tempTypeKeywordSet ){
		m_nKeywordSet1 = tempTypeKeywordSet[0];
		auto_strcpy(m_tempTypeName, name);
		auto_strcpy(m_tempTypeExts, exts);
		SKeywordSetIndex indexs;
		indexs.typeId = -1;
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			indexs.index[j] = tempTypeKeywordSet[j];
		}
		m_Types_nKeyWordSetIdx.push_back(indexs);
	}
	int i;
	for( i = 0; i < GetDllShareData().m_nTypesCount; ++i ){
		SKeywordSetIndex indexs;
		STypeConfig type;
		CDocTypeManager().GetTypeConfig(CTypeConfig(i), type);
		indexs.typeId = type.m_id;
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			indexs.index[j] = type.m_nKeyWordSetIdx[j];
		}
		m_Types_nKeyWordSetIdx.push_back(indexs);
	}
}

/*!	ShareData に 設定を適用・コピーする
	@param[out] tempTypeKeywordSet キーワードセット
	@note ShareDataにコピーするだけなので，更新要求などは，利用する側で処理してもらう
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::ApplyData( int* tempTypeKeywordSet )
{
	m_pShareData->m_Common = m_Common;

	int i;
	const int nSize = (int)m_Types_nKeyWordSetIdx.size();
	int nBegin = 0;
	if( tempTypeKeywordSet ){
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			tempTypeKeywordSet[j] = m_Types_nKeyWordSetIdx[0].index[j];
		}
		nBegin = 1;
	}
	for( i = nBegin; i < nSize; ++i ){
		CTypeConfig configIdx = CDocTypeManager().GetDocumentTypeOfId( m_Types_nKeyWordSetIdx[i].typeId );
		if( configIdx.IsValidType() ){
			STypeConfig type;
			CDocTypeManager().GetTypeConfig(configIdx, type);
			//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
			/* 変更された設定値のコピー */
			for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
				type.m_nKeyWordSetIdx[j] = m_Types_nKeyWordSetIdx[i].index[j];
			}
			CDocTypeManager().SetTypeConfig(configIdx, type);
		}
	}
}



/* ヘルプ */
//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
void CPropCommon::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP_GENERAL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
//	Sept. 10, 2000 JEPRO ID名を実際の名前に変更するため以下の行はコメントアウト
//	変更は少し後の行(Sept. 9, 2000)で行っている
//	case IDD_PROP1P5:
//		nContextID = 84;
//		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	// From Here Sept. 9, 2000 JEPRO 共通設定のヘルプボタンが効かなくなっていた部分を以下の追加によって修正
	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_TAB:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TAB);
		break;
	case IDD_PROP_STATUSBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_STATUSBAR);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	// To Here Sept. 9, 2000
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;
	case IDD_PROP_FNAME:	// 2002.12.09 Moca FNAME追加
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FNAME);
		break;
	case IDD_PROP_PLUGIN:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_PLUGIN);
		break;
	case IDD_PROP_MAINMENU:	//@@@ 2010/6/2 Uchi
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MAINMENU);
		break;

	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
	return;
}



/*!	コントロールにフォント設定する
	@date 2013.04.24 Uchi
*/
HFONT CPropCommon::SetCtrlFont( HWND hwndDlg, int idc_ctrl, const LOGFONT& lf )
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
HFONT CPropCommon::SetFontLabel( HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps )
{
	HFONT	hFont;
	TCHAR	szFontName[80];
	LOGFONT lfTemp;
	lfTemp = lf;
	// 大きすぎるフォントは小さく表示
	if( lfTemp.lfHeight < -16 ){
		lfTemp.lfHeight = -16;
	}

	hFont = SetCtrlFont( hwndDlg, idc_static, lfTemp );

	// フォント名の設定
	auto_sprintf( szFontName, nps % 10 ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
		lf.lfFaceName, double(nps)/10 );
	::DlgItem_SetText( hwndDlg, idc_static, szFontName );

	return hFont;
}
