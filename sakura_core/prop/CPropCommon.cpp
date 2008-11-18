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

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_rc.h"
#include "prop/CPropCommon.h"
#include "debug/Debug.h"
#include <windows.h>
#include <commctrl.h>
#include "dlg/CDlgOpenFile.h"
#include "global.h"
#include "dlg/CDlgInput1.h"
#include "dlg/CDlgDebug.h"
#include "window/CSplitBoxWnd.h"
#include "CMenuDrawer.h"
#include "func/Funccode.h"	//Stonee, 2001/05/18
#include "CEditApp.h"
#include "util/shell.h"
#include "CNormalProcess.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		HIDC_BUTTON_CLEAR_MRU_FILE,			//履歴をクリア（ファイル）
	IDC_BUTTON_CLEAR_MRU_FOLDER,	HIDC_BUTTON_CLEAR_MRU_FOLDER,		//履歴をクリア（フォルダ）
	IDC_CHECK_FREECARET,			HIDC_CHECK_FREECARET,				//フリーカーソル
//DEL	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,					//自動インデント ：タイプ別へ移動
//DEL	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,			//全角空白もインデント ：タイプ別へ移動
	IDC_CHECK_USETRAYICON,			HIDC_CHECK_USETRAYICON,				//タスクトレイを使う
	IDC_CHECK_STAYTASKTRAY,			HIDC_CHECK_STAYTASKTRAY,			//タスクトレイに常駐
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	HIDC_CHECK_REPEATEDSCROLLSMOOTH,	//少し滑らかにする
	IDC_CHECK_CLOSEALLCONFIRM,		HIDC_CHECK_CLOSEALLCONFIRM,			//[すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	IDC_CHECK_EXITCONFIRM,			HIDC_CHECK_EXITCONFIRM,				//終了の確認
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, HIDC_CHECK_STOPS_WORD, //単語単位で移動するときに単語の両端に止まる
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, HIDC_CHECK_STOPS_PARAGRAPH, // 段落単位で移動するときに段落の両端に止まる
	IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE,	// マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない 2007.10.08 genta
	IDC_HOTKEY_TRAYMENU,			HIDC_HOTKEY_TRAYMENU,				//左クリックメニューのショートカットキー
	IDC_EDIT_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,	//スクロール行数
	IDC_EDIT_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,				//ファイル履歴の最大数
	IDC_EDIT_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,			//フォルダ履歴の最大数
	IDC_RADIO_CARETTYPE0,			HIDC_RADIO_CARETTYPE0,				//カーソル形状（Windows風）
	IDC_RADIO_CARETTYPE1,			HIDC_RADIO_CARETTYPE1,				//カーソル形状（MS-DOS風）
	IDC_SPIN_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,
	IDC_SPIN_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,
	IDC_SPIN_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,
	IDC_CHECK_MEMDC,				HIDC_CHECK_MEMDC,					//画面キャッシュを使う
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		10900,	//履歴をクリア（ファイル）
	IDC_BUTTON_CLEAR_MRU_FOLDER,	10901,	//履歴をクリア（フォルダ）
	IDC_CHECK_FREECARET,			10910,	//フリーカーソル
	IDC_CHECK_INDENT,				10911,	//自動インデント
	IDC_CHECK_INDENT_WSPACE,		10912,	//全角空白もインデント
	IDC_CHECK_USETRAYICON,			10913,	//タスクトレイを使う
	IDC_CHECK_STAYTASKTRAY,			10914,	//タスクトレイに常駐
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	10915,	//少し滑らかにする
	IDC_CHECK_EXITCONFIRM,			10916,	//終了の確認
	IDC_HOTKEY_TRAYMENU,			10940,	//左クリックメニューのショートカットキー
	IDC_EDIT_REPEATEDSCROLLLINENUM,	10941,	//スクロール行数
	IDC_EDIT_MAX_MRU_FILE,			10942,	//ファイル履歴の最大数
	IDC_EDIT_MAX_MRU_FOLDER,		10943,	//フォルダ履歴の最大数
	IDC_RADIO_CARETTYPE0,			10960,	//カーソル形状（Windows風）
	IDC_RADIO_CARETTYPE1,			10961,	//カーソル形状（MS-DOS風）
	IDC_SPIN_REPEATEDSCROLLLINENUM,	-1,
	IDC_SPIN_MAX_MRU_FILE,			-1,
	IDC_SPIN_MAX_MRU_FOLDER,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End






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

//	From Here Jun. 2, 2001 genta
//	Dialog procedureの処理を共通化し、各ページのDialog Procedureでは
//	真の処理メソッドを指定して共通関数を呼ぶだけにした．
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_GENERAL(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p1, hwndDlg, uMsg, wParam, lParam );
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

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CPropCommon::CPropCommon()
{
//	int		i;
//	long	lPathLen;

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hwndThis  = NULL;		/* このダイアログのハンドル */
	m_nPageNum = ID_PAGENUM_GENERAL;

	/* ヘルプファイルのフルパスを返す */
	m_szHelpFile = CEditApp::Instance()->GetHelpFilePath();

	return;
}





CPropCommon::~CPropCommon()
{
}





/* 初期化 */
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
void CPropCommon::Create( HWND hwndParent, CImageListMgr* cIcons, CSMacroMgr* pMacro, CMenuDrawer* pMenuDrawer )
{
	m_hwndParent = hwndParent;	/* オーナーウィンドウのハンドル */
	m_pcIcons = cIcons;
	m_pcSMacro = pMacro;

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
	const TCHAR* szTabname;								//!< TABの表示名
	unsigned int resId;										//!< Property sheetに対応するDialog resource
	INT_PTR (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);	//!< Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	キーワード：共通設定タブ順序(プロパティシート)
/*! プロパティシートの作成
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
int CPropCommon::DoPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
//	m_nActiveItem = nActiveItem;

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	int					nRet;
	int					nIdx;
	int					i;

	//	From Here Jun. 2, 2001 genta
	//	Feb. 11, 2007 genta URLをTABと入れ換え	// 2007.02.13 順序変更（TABをWINの次に）
	//!	「共通設定」プロパティシートの作成時に必要な情報の配列．
	//	順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に	2008/6/22 Uchi 
	static ComPropSheetInfo ComPropSheetInfoList[] = {
		{ _T("全般"), 				IDD_PROP1P1,		DlgProc_PROP_GENERAL },
		{ _T("ウィンドウ"),			IDD_PROP_WIN,		DlgProc_PROP_WIN },
		{ _T("ツールバー"),			IDD_PROP_TOOLBAR,	DlgProc_PROP_TOOLBAR },
		{ _T("タブバー"),			IDD_PROP_TAB,		DlgProc_PROP_TAB },
		{ _T("ステータスバー"),		IDD_PROP_STATUSBAR,	DlgProc_PROP_STATUSBAR},	// 文字コード表示指定	2008/6/21	Uchi
		{ _T("編集"),				IDD_PROP_EDIT,		DlgProc_PROP_EDIT },
		{ _T("ファイル"),			IDD_PROP_FILE,		DlgProc_PROP_FILE },
		{ _T("ファイル名表示"),		IDD_PROP_FNAME,		DlgProc_PROP_FILENAME},
		{ _T("バックアップ"),		IDD_PROP_BACKUP,	DlgProc_PROP_BACKUP },
		{ _T("書式"),				IDD_PROP_FORMAT,	DlgProc_PROP_FORMAT },
		{ _T("検索"),				IDD_PROP_GREP,		DlgProc_PROP_GREP },	// 2006.08.23 ryoji タイトル変更（Grep -> 検索）
		{ _T("キー割り当て"),		IDD_PROP_KEYBIND,	DlgProc_PROP_KEYBIND },
		{ _T("カスタムメニュー"),	IDD_PROP_CUSTMENU,	DlgProc_PROP_CUSTMENU },
		{ _T("強調キーワード"),		IDD_PROP_KEYWORD,	DlgProc_PROP_KEYWORD },
		{ _T("支援"),				IDD_PROP_HELPER,	DlgProc_PROP_HELPER },
		{ _T("マクロ"),				IDD_PROP_MACRO,		DlgProc_PROP_MACRO },
	};

	PROPSHEETPAGE		psp[32];
	for( nIdx = 0, i = 0; i < _countof(ComPropSheetInfoList) && nIdx < 32 ; i++ ){
		if( ComPropSheetInfoList[i].szTabname != NULL ){
			PROPSHEETPAGE *p = &psp[nIdx];
			memset_raw( p, 0, sizeof_raw( *p ) );
			p->dwSize      = sizeof_raw( *p );
			p->dwFlags     = PSP_USETITLE | PSP_HASHELP;
			p->hInstance   = G_AppInstance();
			p->pszTemplate = MAKEINTRESOURCE( ComPropSheetInfoList[i].resId );
			p->pszIcon     = NULL;
			p->pfnDlgProc  = (DLGPROC)(ComPropSheetInfoList[i].DProc);
			p->pszTitle    = ComPropSheetInfoList[i].szTabname;
			p->lParam      = (LPARAM)this;
			p->pfnCallback = NULL;
			nIdx++;
		}
	}
	//	To Here Jun. 2, 2001 genta

	PROPSHEETHEADER		psh;
	memset_raw( &psh, 0, sizeof_raw( psh ) );
	
	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	psh.dwSize = sizeof_old_PROPSHEETHEADER;

	//	JEPROtest Sept. 30, 2000 共通設定の隠れ[適用]ボタンの正体はここ。行頭のコメントアウトを入れ替えてみればわかる
	psh.dwFlags    = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	psh.hwndParent = m_hwndParent;
	psh.hInstance  = G_AppInstance();
	psh.pszIcon    = NULL;
	psh.pszCaption = _T("共通設定");
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
		::MYMESSAGEBOX(
			NULL, 
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			_T("作者に教えて欲しいエラー"),
			_T("CPropCommon::DoPropertySheet()内でエラーが出ました。\n")
			_T("psh.nStartPage=[%d]\n")
			_T("::PropertySheet()失敗\n")
			_T("\n")
			_T("%ts\n"),
			psh.nStartPage,
			pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

	return nRet;
}



/*!	ShareDataから一時領域へ設定をコピーする
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::InitData( void )
{
	m_Common = m_pShareData->m_Common;

	//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			m_Types_nKeyWordSetIdx[i][j] = CTypeConfig(i)->m_nKeyWordSetIdx[j];
		}
	}
}

/*!	ShareData に 設定を適用・コピーする
	@note ShareDataにコピーするだけなので，更新要求などは，利用する側で処理してもらう
	@date 2002.12.11 Moca CEditDoc::OpenPropertySheetから移動
*/
void CPropCommon::ApplyData( void )
{
	m_pShareData->m_Common = m_Common;

	int i;
	for( i = 0; i < MAX_TYPES; ++i ){
		//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
		/* 変更された設定値のコピー */
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			CTypeConfig(i)->m_nKeyWordSetIdx[j] = m_Types_nKeyWordSetIdx[i][j];
		}
	}
}




/* p1 メッセージ処理 */
INT_PTR CPropCommon::DispatchEvent_p1(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 p1 */
		SetData_p1( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){

			case IDC_CHECK_USETRAYICON:	/* タスクトレイを使う */
			// From Here 2001.12.03 hor
			//		操作しにくいって評判だったのでタスクトレイ関係のEnable制御をやめました
			//@@@ YAZAKI 2001.12.31 IDC_CHECKSTAYTASKTRAYのアクティブ、非アクティブのみ制御。
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
				}else{
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
				}
			// To Here 2001.12.03 hor
				return TRUE;

			case IDC_CHECK_STAYTASKTRAY:	/* タスクトレイに常駐 */
				return TRUE;

			case IDC_BUTTON_CLEAR_MRU_FILE:
				/* ファイルの履歴をクリア */
				if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					_T("最近使ったファイルの履歴を削除します。\nよろしいですか？\n") ) ){
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
//				m_pShareData->m_sHistory.m_nMRUArrNum = 0;
				{
					CMRU cMRU;
					cMRU.ClearAll();
				}
				InfoMessage( hwndDlg, _T("最近使ったファイルの履歴を削除しました。\n") );
				return TRUE;
			case IDC_BUTTON_CLEAR_MRU_FOLDER:
				/* フォルダの履歴をクリア */
				if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					_T("最近使ったフォルダの履歴を削除します。\nよろしいですか？\n") ) ){
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
//				m_pShareData->m_sHistory.m_nOPENFOLDERArrNum = 0;
				{
					CMRUFolder cMRUFolder;	//	MRUリストの初期化。ラベル内だと問題あり？
					cMRUFolder.ClearAll();
				}
				InfoMessage( hwndDlg, _T("最近使ったフォルダの履歴を削除しました。\n") );
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
		case IDC_SPIN_MAX_MRU_FILE:
			/* ファイルの履歴MAX */
//			MYTRACE_A( "IDC_SPIN_MAX_MRU_FILE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > MAX_MRU ){
				nVal = MAX_MRU;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_MAX_MRU_FOLDER:
			/* フォルダの履歴MAX */
//			MYTRACE_A( "IDC_SPIN_MAX_MRU_FOLDER\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > MAX_OPENFOLDER ){
				nVal = MAX_OPENFOLDER;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, nVal, FALSE );
			return TRUE;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP1P1 );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE_A( "p1 PSN_KILLACTIVE\n" );
				/* ダイアログデータの取得 p1 */
				GetData_p1( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_GENERAL;	//Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更(参照しているのはCPropCommon.cppのみの1箇所)
				return TRUE;
			}
			break;
		}

//		MYTRACE_A( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE_A( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE_A( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE_A( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE_A( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}





/* ダイアログデータの設定 p1 */
void CPropCommon::SetData_p1( HWND hwndDlg )
{
	BOOL	bRet;

	/* カーソルのタイプ 0=win 1=dos  */
	if( 0 == m_Common.m_sGeneral.GetCaretType() ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, TRUE );
	}


	/* フリーカーソルモード */
	::CheckDlgButton( hwndDlg, IDC_CHECK_FREECARET, m_Common.m_sGeneral.m_bIsFreeCursorMode ? 1 : 0 );

	/* 単語単位で移動するときに、単語の両端で止まるか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord );

	/* 段落単位で移動するときに、段落の両端で止まるか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph );

	//	2007.10.08 genta マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない (2007.10.02 by nasukoji)
	::CheckDlgButton( hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, m_Common.m_sGeneral.m_bNoCaretMoveByActivation );

	/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	::CheckDlgButton( hwndDlg, IDC_CHECK_CLOSEALLCONFIRM, m_Common.m_sGeneral.m_bCloseAllConfirm );

	/* 終了時の確認をする */
	::CheckDlgButton( hwndDlg, IDC_CHECK_EXITCONFIRM, m_Common.m_sGeneral.m_bExitConfirm );

	/* キーリピート時のスクロール行数 */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, (Int)m_Common.m_sGeneral.m_nRepeatedScrollLineNum, FALSE );

	/* キーリピート時のスクロールを滑らかにするか */
	::CheckDlgButton( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH, m_Common.m_sGeneral.m_nRepeatedScroll_Smooth );

	// 2008.10.06 nasukoji	マウスの中ボタン押下中のホイールスクロールではページスクロールする
	::CheckDlgButton( hwndDlg, IDC_CHECK_PAGESCROLL_BY_MBUTTONWHEEL, m_Common.m_sGeneral.m_bPageScroolByMButtonWheel );

	// 2007.09.09 Moca 画面キャッシュ設定追加
	// 画面キャッシュを使う
	::CheckDlgButton( hwndDlg, IDC_CHECK_MEMDC, m_Common.m_sWindow.m_bUseCompotibleBMP );

	/* ファイルの履歴MAX */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, m_Common.m_sGeneral.m_nMRUArrNum_MAX, FALSE );

	/* フォルダの履歴MAX */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX, FALSE );

	/* タスクトレイを使う */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USETRAYICON, m_Common.m_sGeneral.m_bUseTaskTray );
// From Here 2001.12.03 hor
//@@@ YAZAKI 2001.12.31 ここは制御する。
	if( m_Common.m_sGeneral.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
// To Here 2001.12.03 hor
	/* タスクトレイに常駐 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STAYTASKTRAY, m_Common.m_sGeneral.m_bStayTaskTray );

	/* タスクトレイ左クリックメニューのショートカット */
	::SendMessageAny( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ), HKM_SETHOTKEY, MAKEWORD( m_Common.m_sGeneral.m_wTrayMenuHotKeyCode, m_Common.m_sGeneral.m_wTrayMenuHotKeyMods ), 0 );

	return;
}





/* ダイアログデータの取得 p1 */
int CPropCommon::GetData_p1( HWND hwndDlg )
{
	/* カーソルのタイプ 0=win 1=dos  */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE0 ) ){
		m_Common.m_sGeneral.SetCaretType(0);
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE1 ) ){
		m_Common.m_sGeneral.SetCaretType(1);
	}

	/* フリーカーソルモード */
	m_Common.m_sGeneral.m_bIsFreeCursorMode = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FREECARET ) != 0;

	/* 単語単位で移動するときに、単語の両端で止まるか */
	m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD );
	//	2007.10.08 genta マウスクリックでアクティブになったときはカーソルをクリック位置に移動しない (2007.10.02 by nasukoji)
	m_Common.m_sGeneral.m_bNoCaretMoveByActivation = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE );

	/* 段落単位で移動するときに、段落の両端で止まるか */
	m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH );

	/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	m_Common.m_sGeneral.m_bCloseAllConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CLOSEALLCONFIRM );

	/* 終了時の確認をする */
	m_Common.m_sGeneral.m_bExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXITCONFIRM );

	/* キーリピート時のスクロール行数 */
	m_Common.m_sGeneral.m_nRepeatedScrollLineNum = (CLayoutInt)::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nRepeatedScrollLineNum < CLayoutInt(1) ){
		m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(1);
	}
	if( m_Common.m_sGeneral.m_nRepeatedScrollLineNum > CLayoutInt(10) ){
		m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(10);
	}

	/* キーリピート時のスクロールを滑らかにするか */
	m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH );

	// 2008.10.06 nasukoji	マウスの中ボタン押下中のホイールスクロールではページスクロールする
	m_Common.m_sGeneral.m_bPageScroolByMButtonWheel = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PAGESCROLL_BY_MBUTTONWHEEL );

	// 2007.09.09 Moca 画面キャッシュ設定追加
	// 画面キャッシュを使う
	m_Common.m_sWindow.m_bUseCompotibleBMP = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_MEMDC );

	/* ファイルの履歴MAX */
	m_Common.m_sGeneral.m_nMRUArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nMRUArrNum_MAX < 0 ){
		m_Common.m_sGeneral.m_nMRUArrNum_MAX = 0;
	}
	if( m_Common.m_sGeneral.m_nMRUArrNum_MAX > MAX_MRU ){
		m_Common.m_sGeneral.m_nMRUArrNum_MAX = MAX_MRU;
	}

	{	//履歴の管理	//@@@ 2003.04.09 MIK
		CRecentFile	cRecentFile;
		cRecentFile.UpdateView();
		cRecentFile.Terminate();
	}

	/* フォルダの履歴MAX */
	m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
	if( m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX < 0 ){
		m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 0;
	}
	if( m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX > MAX_OPENFOLDER ){
		m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = MAX_OPENFOLDER;
	}

	{	//履歴の管理	//@@@ 2003.04.09 MIK
		CRecentFolder	cRecentFolder;
		cRecentFolder.UpdateView();
		cRecentFolder.Terminate();
	}

	/* タスクトレイを使う */
	m_Common.m_sGeneral.m_bUseTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON );
//@@@ YAZAKI 2001.12.31 m_bUseTaskTrayに引きづられるように。
	if( m_Common.m_sGeneral.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
	/* タスクトレイに常駐 */
	m_Common.m_sGeneral.m_bStayTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STAYTASKTRAY );

	/* タスクトレイ左クリックメニューのショートカット */
	LRESULT	lResult;
	lResult = ::SendMessageAny( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ), HKM_GETHOTKEY, 0, 0 );
	m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = LOBYTE( lResult );
	m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HIBYTE( lResult );

	return TRUE;
}



/* ヘルプ */
//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
void CPropCommon::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP1P1:
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

	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		MyWinHelp( hwndParent, m_szHelpFile, HELP_CONTEXT, nContextID );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	}
	return;
}





