//	$Id$
/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDlgOpenFile.h"
//#include <stdio.h>
#include "debug.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
#include "funccode.h"	//Stonee, 2001/05/18

// オープンファイル CDlgOpenFile.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDCANCEL,				HIDCANCEL_OPENDLG,		//Winのヘルプで勝手に出てくる
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		//ヘルプボタン
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		//文字コードセット
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			//最近のファイル
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	//最近のフォルダ
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			//改行コード
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

WNDPROC			m_wpOpenDialogProc;

const char**	m_ppszMRU;
const char**	m_ppszOPENFOLDER;
char			m_szHelpFile[_MAX_PATH + 1];
int				m_nHelpTopicID;
BOOL			m_bReadOnly;		/* 読み取り専用か */
BOOL			m_bIsSaveDialog;	/* 保存のダイアログか */





/*
|| 	開くダイアログのフックプロシージャ
*/
LRESULT APIENTRY OFNHookProcMain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WPARAM					wCtlFocus;	/* フォーカスを持つコントロールのID */
//	BOOL					fHandle;	/* wParamが処理するフラグ */
	int						idCtrl;
	OFNOTIFY*				pofn;
//	int						nIdx;
//	char*					pszWork;
	WORD					wNotifyCode;
	WORD					wID;
	HWND					hwndCtl;
//	HWND					hwndFrame;
//	static CShareData		cShareData;
	static DLLSHAREDATA*	pShareData;
	switch( uMsg ){
	case WM_MOVE:
		/* 「開く」ダイアログのサイズと位置 */
//		cShareData.Init();
		pShareData = CShareData::getInstance()->GetShareData();
		::GetWindowRect( hwnd, &pShareData->m_Common.m_rcOpenDialog );
//		MYTRACE( "WM_MOVE 1\n" );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND) lParam;		// handle of control
		switch( wNotifyCode ){
//			break;
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case pshHelp:
				/* ヘルプ */
				::WinHelp( hwnd, m_szHelpFile, HELP_CONTEXT, m_nHelpTopicID );
				break;
			case chx1:	// The read-only check box
				m_bReadOnly = ::IsDlgButtonChecked( hwnd , chx1 );
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE( "=========WM_NOTIFY=========\n" );
//		MYTRACE( "pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE( "pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( "pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );
		break;
	}
//	return ::CallWindowProc( (int (__stdcall *)( void ))(WNDPROC)m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );
	return ::CallWindowProc( (WNDPROC)m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );
}




/*!
	開くダイアログのフックプロシージャ
*/
UINT APIENTRY OFNHookProc(
	HWND hdlg,		// handle to child dialog window
	UINT uiMsg,		// message identifier
	WPARAM wParam,	// message parameter
	LPARAM lParam 	// message parameter
)
{
	HFONT					hFont;
	POINT					po;
//	RECT					rc0;
	RECT					rc;
	static HWND				hwndEdit1;
	static HWND				hwndExplororList;
	static OPENFILENAME*	pOf;
	static HWND				hwndOpenDlg;
	static HWND				hwndComboMRU;
	static HWND				hwndComboOPENFOLDER;
	static HWND				hwndComboCODES;
	static HWND				hwndComboEOL;	//	Feb. 9, 2001 genta
	static CDlgOpenFile*	pcDlgOpenFile;
	int						i;
	OFNOTIFY*				pofn;
	int						idCtrl;
//	HWND					hwndCtrl;
	char					szFolder[_MAX_PATH];
	LRESULT					lRes;
	const int				nExtraSize = 100;
	const int				nControls = 9;
	char					szWork[_MAX_PATH + 1];
	WORD					wNotifyCode;
	WORD					wID;
	HWND					hwndCtl;
	int						nIdx;
	int						nIdxSel;
	int						fwSizeType;
	int						nWidth;
	int						nHeight;
	int						nCodeValueArr[] = {
		CODE_AUTODETECT,	/* 文字コード自動判別 */
		CODE_SJIS,
		CODE_JIS,
		CODE_EUC,
		CODE_UNICODE,
		CODE_UTF8,
		CODE_UTF7
	};
	char*					pCodeNameArr[] = {
		"自動選択",
		"SJIS",
		"JIS",
		"EUC",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
	int nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );
	//	From Here	Feb. 9, 2001 genta
	int						nEolValueArr[] = {
		EOL_NONE,
		EOL_CRLF,
		EOL_LF,
		EOL_CR,
	};
	//	文字列はResource内に入れる
	char*					pEolNameArr[] = {
		"変換なし",
		"CR+LF",
		"LF (UNIX)",
		"CR (Mac)",
	};
	int nEolNameArrNum = sizeof( pEolNameArr ) / sizeof( pEolNameArr[0] );
//	To Here	Feb. 9, 2001 genta
	int	Controls[nControls] = {
		stc3, stc2,		// The two label controls
		edt1, cmb1,		// The edit control and the drop-down box
		IDOK, IDCANCEL,
		pshHelp,		// The Help command button (push button)
		lst1,			// The Explorer window
		chx1			// The read-only check box
	};
	int	nRightMargin = 24;
	HWND	hwndFrame;

	switch( uiMsg ){
	case WM_MOVE:
//		MYTRACE( "WM_MOVE 2\n" );
		break;
	case WM_SIZE:
		fwSizeType = wParam;		// resizing flag
		nWidth = LOWORD(lParam);	// width of client area
		nHeight = HIWORD(lParam);	// height of client area

		/* 「開く」ダイアログのサイズと位置 */
		hwndFrame = ::GetParent( hdlg );
		::GetWindowRect( hwndFrame, &pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog );

		::GetWindowRect( hwndEdit1, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( hwndOpenDlg, &po );
//		::GetWindowRect( hwndExplororList, &rc2 );
//		po2.x = rc2.right;
//		po2.y = rc2.bottom;
//		::ScreenToClient( hwndOpenDlg, &po2 );
		::SetWindowPos( hwndComboMRU, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		::SetWindowPos( hwndComboOPENFOLDER, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		return 0;
	case WM_INITDIALOG:
//From Here jeprotest Oct. 12, 2000 JEPRO デフォルトボタン名変更実験
//		CommDlg_OpenSave_SetControlText( GetParent(hdlg), IDOK, "このファイルを開く" );
//To Here jeorotest

		// Save off the long pointer to the OPENFILENAME structure.
		::SetWindowLong(hdlg, DWL_USER, lParam);
		pOf = (OPENFILENAME*)lParam;
		pcDlgOpenFile = (CDlgOpenFile*)pOf->lCustData;


		/* Explorerスタイルの「開く」ダイアログのハンドル */
		hwndOpenDlg = ::GetParent( hdlg );
		/* コントロールのハンドル */
		hwndComboCODES = ::GetDlgItem( hdlg, IDC_COMBO_CODE );
		hwndComboMRU = ::GetDlgItem( hdlg, IDC_COMBO_MRU );
		hwndComboOPENFOLDER = ::GetDlgItem( hdlg, IDC_COMBO_OPENFOLDER );
		hwndComboEOL = ::GetDlgItem( hdlg, IDC_COMBO_EOL );

		/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
		::SendMessage( hwndComboCODES, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
		::SendMessage( hwndComboMRU, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
		::SendMessage( hwndComboOPENFOLDER, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );
		::SendMessage( hwndComboEOL, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

		//	From Here Feb. 9, 2001 genta
		//	改行コードの選択コンボボックス初期化
		//	必要なときのみ利用する
		if( pcDlgOpenFile->m_bUseEol ){
			//	値の設定
			for( i = 0; i < nEolNameArrNum; ++i ){
				nIdx = ::SendMessage( hwndComboEOL, CB_ADDSTRING, 0, (LPARAM)pEolNameArr[i] );
				::SendMessage( hwndComboEOL, CB_SETITEMDATA, nIdx, nEolValueArr[i] );
			}
			//	使うときは先頭の要素を選択状態にする
			::SendMessage( hwndComboEOL, CB_SETCURSEL, (WPARAM)0, 0 );
		}
		else {
			//	使わないときは隠す
			::ShowWindow( ::GetDlgItem( hdlg, IDC_STATIC_EOL ), SW_HIDE );
			::ShowWindow( hwndComboEOL, SW_HIDE );
		}
		//	To Here Feb. 9, 2001 genta

		/* Explorerスタイルの「開く」ダイアログをフック */
		m_wpOpenDialogProc = (WNDPROC) ::SetWindowLong( hwndOpenDlg, GWL_WNDPROC, (LONG) OFNHookProcMain );

//		/* Explorerスタイルの「開く」ダイアログのスタイル */
//		lStyle = ::GetWindowLong( hwndOpenDlg, GWL_STYLE );
//		if( lStyle & WS_BORDER )	lStyle = WS_BORDER;
//		if( lStyle & WS_DLGFRAME )	lStyle = WS_DLGFRAME;
//		if( lStyle & WS_SYSMENU )	lStyle = WS_SYSMENU;
//		::SetWindowLong( hwndOpenDlg, GWL_STYLE, lStyle );

//		lStyle = ::GetWindowLong( hwndOpenDlg, GWL_EXSTYLE );
//		lStyle =~ WS_EX_DLGMODALFRAME;
//		::SetWindowLong( hwndOpenDlg, GWL_EXSTYLE, lStyle );


		/* 文字コード選択コンボボックス初期化 */
		nIdxSel = 0;
		if( m_bIsSaveDialog ){	/* 保存のダイアログか */
			i = 1;
		}else{
			i = 0;
		}
		for( /*i = 0*/; i < nCodeNameArrNum; ++i ){
			nIdx = ::SendMessage( hwndComboCODES, CB_ADDSTRING, 0, (LPARAM)pCodeNameArr[i] );
			::SendMessage( hwndComboCODES, CB_SETITEMDATA, nIdx, nCodeValueArr[i] );
			if( nCodeValueArr[i] == pcDlgOpenFile->m_nCharCode ){
				nIdxSel = nIdx;
			}
		}
		::SendMessage( hwndComboCODES, CB_SETCURSEL, (WPARAM)nIdxSel, 0 );



//		::GetWindowRect( hwndOpenDlg, &rc0 );
//		for( i = 0; i < nControls; ++i ){
//			hwndCtrl = ::GetDlgItem( hwndOpenDlg, Controls[i] );
//			::GetWindowRect( hwndCtrl, &rc );
//			po.x = rc.left;
//			po.y = rc.top;
//			::ScreenToClient( hwndOpenDlg, &po );
//			rc.left = po.x;
//			rc.top  = po.y;
//			po.x = rc.right;
//			po.y = rc.bottom;
//			::ScreenToClient( hwndOpenDlg, &po );
//			rc.right = po.x;
//			rc.bottom  = po.y;
//			if( Controls[i] != lst1 ){
//				::SetWindowPos( hwndCtrl, NULL, rc.left, rc.top + nExtraSize, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//			}else{
//				::SetWindowPos( hwndCtrl, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top + nExtraSize, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
//			}
//		}
//		::SetWindowPos(
//			hwndOpenDlg,
//			HWND_TOP,
//			0, 0,
//			rc0.right - rc0.left,
//			(rc0.bottom - rc0.top)+ nExtraSize,
//			SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER
//		);


		/* 読み取り専用の初期値セット */
		hwndEdit1 = ::GetDlgItem( hwndOpenDlg, edt1 );
		hwndExplororList = ::GetDlgItem( hwndOpenDlg, lst1 );

		::CheckDlgButton( hwndOpenDlg, chx1, m_bReadOnly );

//		::ShowWindow( hwndEdit1, SW_HIDE );


		::GetWindowRect( hwndEdit1, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( hwndOpenDlg, &po );
		rc.left = po.x;
		rc.top = po.y;

		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( hwndOpenDlg, &po );
		rc.right = po.x;
		rc.bottom = po.y;

		hFont = (HFONT)::SendMessage( hdlg, WM_GETFONT, 0, 0 );

		/* 最近開いたファイル コンボボックス初期値設定 */
		for( i = 0; m_ppszMRU[i] != NULL; ++i ){
			::SendMessage( hwndComboMRU, CB_ADDSTRING, 0, (LPARAM)m_ppszMRU[i] );
		}
		::SetWindowPos( hwndComboMRU, 0, rc.left, rc.top, rc.right - rc.left + 100, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

		/* 最近開いたフォルダ コンボボックス初期値設定 */
		for( i = 0; m_ppszOPENFOLDER[i] != NULL; ++i ){
			::SendMessage( hwndComboOPENFOLDER, CB_ADDSTRING, 0, (LPARAM)m_ppszOPENFOLDER[i] );
		}
		::SetWindowPos( hwndComboOPENFOLDER, 0, rc.left, rc.top, rc.right - rc.left + 100, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );


//		::SetWindowLong( hwndOpenDlg, DWL_USER, (LONG)pcDlgOpenFile );

		/* サイズボックス */
//		m_hwndSizeBox = ::CreateWindowEx(
//			WS_EX_CONTROLPARENT, 								/* no extended styles */
//			"SCROLLBAR",										/* scroll bar control class */
//			(LPSTR) NULL,										/* text for window title bar */
//			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP,	/* scroll bar styles */
//			0,													/* horizontal position */
//			0,													/* vertical position */
//			0,													/* width of the scroll bar */
//			0,													/* default height */
//			hwndOpenDlg/*hdlg*/, 								/* handle of main window */
//			(HMENU) NULL,										/* no menu for a scroll bar */
//			pcDlgOpenFile->m_hInstance,							/* instance owning this window */
//			(LPVOID) NULL										/* pointer not needed */
//		);
//		::ShowWindow( m_hwndSizeBox, SW_SHOW );

//NG  1999.11.21
//NG		/* 「開く」ダイアログのサイズと位置 */
//NG		if( 0 != pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.left
//NG		 && 0 != pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.top
//NG		 && 0 != pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.right
//NG		 && 0 != pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.bottom
//NG		){
//NG			hwndFrame = ::GetParent( hdlg );
//NG			::SetWindowPos(
//NG				hwndFrame,
//NG				0,
//NG				pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.left,
//NG				pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.top,
//NG				pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.right - pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.left,
//NG				pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.bottom - pcDlgOpenFile->m_pShareData->m_Common.m_rcOpenDialog.top,
//NG				/*SWP_NOMOVE |*/ SWP_NOOWNERZORDER | SWP_NOZORDER
//NG			);
//NG		}
//NG
		break;


	case WM_DESTROY:
		/* フック解除 */
		::SetWindowLong( hwndOpenDlg, GWL_WNDPROC, (LONG) m_wpOpenDialogProc );
		return FALSE;

	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE( "=========WM_NOTIFY=========\n" );
//		MYTRACE( "pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE( "pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE( "pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );

		switch( pofn->hdr.code ){
		case CDN_FILEOK:
			/* 文字コード選択コンボボックス 値を取得 */
			nIdx = ::SendMessage( hwndComboCODES, CB_GETCURSEL, 0, 0 );
			lRes = ::SendMessage( hwndComboCODES, CB_GETITEMDATA, nIdx, 0 );
//			lRes = ::SendMessage( hwndComboCODES, CB_GETCURSEL, 0, 0 );
			pcDlgOpenFile->m_nCharCode = lRes;	/* 文字コード */
			//	Feb. 9, 2001 genta
			if( pcDlgOpenFile->m_bUseEol ){
				nIdx = ::SendMessage( hwndComboEOL, CB_GETCURSEL, 0, 0 );
				lRes = ::SendMessage( hwndComboEOL, CB_GETITEMDATA, nIdx, 0 );
//			lRes = ::SendMessage( hwndComboCODES, CB_GETCURSEL, 0, 0 );
				pcDlgOpenFile->m_cEol = (enumEOLType)lRes;	/* 文字コード */
			}

//			MYTRACE( "文字コード  lRes=%d\n", lRes );
//			MYTRACE( "pofn->hdr.code=CDN_FILEOK        \n" );break;
		case CDN_FOLDERCHANGE  :
//			MYTRACE( "pofn->hdr.code=CDN_FOLDERCHANGE  \n" );
			lRes = ::SendMessage( hwndOpenDlg, CDM_GETFOLDERPATH, sizeof( szFolder ), (long)(char*)szFolder );
//			MYTRACE( "\tlRes=%d\tszFolder=[%s]\n", lRes, szFolder );

			break;
//		case CDN_HELP			:	MYTRACE( "pofn->hdr.code=CDN_HELP          \n" );break;
//		case CDN_INITDONE		:	MYTRACE( "pofn->hdr.code=CDN_INITDONE      \n" );break;
//		case CDN_SELCHANGE		:	MYTRACE( "pofn->hdr.code=CDN_SELCHANGE     \n" );break;
//		case CDN_SHAREVIOLATION	:	MYTRACE( "pofn->hdr.code=CDN_SHAREVIOLATION\n" );break;
//		case CDN_TYPECHANGE		:	MYTRACE( "pofn->hdr.code=CDN_TYPECHANGE    \n" );break;
//		default:					MYTRACE( "pofn->hdr.code=???\n" );break;
		case CDN_TYPECHANGE:	//@@@ 2002.1.24 YAZAKI 名前を付けて保存するときに「テキストファイル（*.txt）」を選択したら、拡張子txtを補うように変更
			switch (pOf->nFilterIndex){
			case 1:
			 	pOf->lpstrDefExt = NULL;
				break;
			case 2:
			 	pOf->lpstrDefExt = "txt";
				break;
			case 3:
			 	pOf->lpstrDefExt = NULL;
				break;
			}
			::SendMessage( hwndOpenDlg, CDM_SETDEFEXT, 0, (long)pOf->lpstrDefExt );
			break;
		}

//		MYTRACE( "=======================\n" );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND) lParam;		// handle of control
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			switch( (int) LOWORD(wParam) ){
			case IDC_COMBO_MRU:
				nIdx = ::SendMessage( (HWND) lParam, CB_GETCURSEL, 0, 0 );
				if( CB_ERR != ::SendMessage( (HWND) lParam, CB_GETLBTEXT, nIdx, (LPARAM) (LPCSTR)szWork ) ){
//					MYTRACE( "szWork=%s\n", szWork );
					lRes = ::SendMessage( hwndOpenDlg, CDM_SETCONTROLTEXT, edt1,(LPARAM)(LPSTR)szWork );
				}
				break;
			case IDC_COMBO_OPENFOLDER:
				nIdx = ::SendMessage( (HWND) lParam, CB_GETCURSEL, 0, 0 );
				if( CB_ERR != ::SendMessage( (HWND) lParam, CB_GETLBTEXT, nIdx, (LPARAM) (LPCSTR)szWork ) ){
//					MYTRACE( "szWork=%s\n", szWork );
					lRes = ::SendMessage( hwndOpenDlg, CDM_SETCONTROLTEXT, edt1,(LPARAM)(LPSTR)szWork );
					::PostMessage( ::GetDlgItem( hwndOpenDlg, edt1), WM_KEYDOWN, VK_RETURN, (LPARAM)0 );
				}
				break;
			}
		}
		break;

	//@@@ 2002.01.08 add start
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hdlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids );
		return TRUE;
	//@@@ 2002.01.08 add end

	default:
		return FALSE;
	}
	return TRUE;
}





CDlgOpenFile::CDlgOpenFile()
{
	/* メンバの初期化 */
	long	lPathLen;
	char	szFile[_MAX_PATH + 1];
//	int		nCharChars;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];

	m_nCharCode = CODE_AUTODETECT;	/* 文字コード *//* 文字コード自動判別 */


	m_hInstance = NULL;		/* アプリケーションインスタンスのハンドル */
	m_hwndParent = NULL;	/* オーナーウィンドウのハンドル */
	m_hWnd = NULL;			/* このダイアログのハンドル */
//	m_pShareData = NULL;
//	m_cShareData = NULL;

	/* 共有データ構造体のアドレスを返す */
//	m_cShareData.Init();
	m_pShareData = CShareData::getInstance()->GetShareData();

	/* OPENFILENAMEの初期化 */
	memset( &m_ofn, 0, sizeof( OPENFILENAME ) );
	m_ofn.lStructSize = sizeof( OPENFILENAME );
//	m_ofn.nFilterIndex = 3;
	m_ofn.nFilterIndex = 1;	//Jul. 09, 2001 JEPRO		/* 「開く」での最初のワイルドカード */

//	::GetCurrentDirectory( _MAX_PATH, m_szInitialDir );	/* 「開く」での初期ディレクトリ */
	lPathLen = ::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szFile, sizeof( szFile )
	);
	_splitpath( szFile, szDrive, szDir, NULL, NULL );
	strcpy( m_szInitialDir, szDrive );
	strcat( m_szInitialDir, szDir );



	strcpy( m_szDefaultWildCard, "*.*" );	/*「開く」での最初のワイルドカード */

	/* ヘルプファイルのフルパスを返す */
	::GetHelpFilePath( m_szHelpFile );
	m_nHelpTopicID = 0;

	return;
}





CDlgOpenFile::~CDlgOpenFile()
{
	return;
}





/* 初期化 */
void CDlgOpenFile::Create(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	const char*		pszUserWildCard,
	const char*		pszDefaultPath,
	const char**	ppszMRU,
	const char**	ppszOPENFOLDER
)
{
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* ユーザー定義ワイルドカード */
	if( NULL != pszUserWildCard ){
		strcpy( m_szDefaultWildCard, pszUserWildCard );
	}
	/* 「開く」での初期フォルダ */
	if( 0 < lstrlen( pszDefaultPath ) ){	//現在編集中のファイルのパス
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		_splitpath( pszDefaultPath, szDrive, szDir, NULL, NULL );
		wsprintf( m_szInitialDir, "%s%s", szDrive, szDir );
	}
	m_ppszMRU = ppszMRU;
	m_ppszOPENFOLDER = ppszOPENFOLDER;
	return;
}




/*! 「開く」ダイアログ モーダルダイアログの表示 */
BOOL CDlgOpenFile::DoModal_GetOpenFileName( char* pszPath )
{
	DWORD	dwError;
	int		i;
	char	szWork[256];
	char	szFilter[1024];
	char*	pszFilterArr[] = {
			"ユーザー指定",	m_szDefaultWildCard,	//Jul. 09, 2001 JEPRO これがリストの先頭に来るように変更
			"テキストファイル", "*.txt",
			"すべてのファイル", "*.*"
	};
	int		nFilterArrNum = sizeof( pszFilterArr ) / sizeof( pszFilterArr[0] );
	/* 拡張子フィルタの作成 */
	strcpy( szFilter, "" );
	for( i = 0; i < nFilterArrNum; i += 2 ){
		wsprintf( szWork, "%s (%s)|%s|", pszFilterArr[i], pszFilterArr[i + 1], pszFilterArr[i + 1] );
		strcat( szFilter, szWork );
	}
	strcat( szFilter, "|" );
//	MYTRACE( "%s\n", szFilter );
	for (i = 0; szFilter[i] != '\0'; i++){
		if (szFilter[i] == '|' ){
			szFilter[i] = '\0';
		}
	}
	/* 構造体の初期化 */
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = szFilter;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter = 0;
//	m_ofn.nFilterIndex = 3;
	if( 0 == lstrlen( pszPath ) ){
		m_ofn.lpstrFile = strcpy( pszPath, pszFilterArr[(m_ofn.nFilterIndex - 1) * 2 + 1]);
	}else{
		m_ofn.lpstrFile = pszPath;
	}
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.lpstrTitle = NULL;
//	m_ofn.Flags =
//		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
//		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
	m_ofn.Flags =
		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
		;
	m_ofn.nFileOffset = 0;
	m_ofn.nFileExtension = 0;
	m_ofn.lpstrDefExt = NULL;
	m_ofn.lCustData = (DWORD)this;
	m_ofn.lpfnHook = OFNHookProc;
	m_ofn.lpTemplateName = "IDD_FILEOPEN";
	if( ::GetOpenFileName( &m_ofn ) ){
		return TRUE;
	}else{
//		MYTRACE( "FALSE == GetOpenFileName( .... )\n" );
		BOOL		bCancel;
		const char*	pszError;
		bCancel = FALSE;
		dwError = ::CommDlgExtendedError();
		switch( dwError ){
		case CDERR_DIALOGFAILURE	: pszError = "CDERR_DIALOGFAILURE  "; break;
		case CDERR_FINDRESFAILURE	: pszError = "CDERR_FINDRESFAILURE "; break;
		case CDERR_NOHINSTANCE		: pszError = "CDERR_NOHINSTANCE    "; break;
		case CDERR_INITIALIZATION	: pszError = "CDERR_INITIALIZATION "; break;
		case CDERR_NOHOOK			: pszError = "CDERR_NOHOOK         "; break;
		case CDERR_LOCKRESFAILURE	: pszError = "CDERR_LOCKRESFAILURE "; break;
		case CDERR_NOTEMPLATE		: pszError = "CDERR_NOTEMPLATE     "; break;
		case CDERR_LOADRESFAILURE	: pszError = "CDERR_LOADRESFAILURE "; break;
		case CDERR_STRUCTSIZE		: pszError = "CDERR_STRUCTSIZE     "; break;
		case CDERR_LOADSTRFAILURE	: pszError = "CDERR_LOADSTRFAILURE "; break;
		case FNERR_BUFFERTOOSMALL	: pszError = "FNERR_BUFFERTOOSMALL "; break;
		case CDERR_MEMALLOCFAILURE	: pszError = "CDERR_MEMALLOCFAILURE"; break;
		case FNERR_INVALIDFILENAME	: pszError = "FNERR_INVALIDFILENAME"; break;
		case CDERR_MEMLOCKFAILURE	: pszError = "CDERR_MEMLOCKFAILURE "; break;
		case FNERR_SUBCLASSFAILURE	: pszError = "FNERR_SUBCLASSFAILURE"; break;
		default: bCancel = TRUE; break;
		}
		if( !bCancel ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( m_hwndParent, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"ダイアログが開けません。\n\nエラー:%s", pszError
			);
		}
		return FALSE;
	}
}


/*! 保存ダイアログ モーダルダイアログの表示 */
BOOL CDlgOpenFile::DoModal_GetSaveFileName( char* pszPath )
{
	DWORD	dwError;
	int		i;
	char	szWork[256];
	char	szFilter[1024];
	char*	pszFilterArr[] = {
			"ユーザー指定", m_szDefaultWildCard,	//Jul. 09, 2001 JEPRO これがリストの先頭に来るように変更
			"テキストファイル",	"*.txt",
			"すべてのファイル", "*.*"
	};
	int		nFilterArrNum = sizeof( pszFilterArr ) / sizeof( pszFilterArr[0] );
	/* 拡張子フィルタの作成 */
	strcpy( szFilter, "" );
	for( i = 0; i < nFilterArrNum; i += 2 ){
		wsprintf( szWork, "%s (%s)|%s|", pszFilterArr[i], pszFilterArr[i + 1], pszFilterArr[i + 1] );
		strcat( szFilter, szWork );
	}
	strcat( szFilter, "|" );
//	MYTRACE( "%s\n", szFilter );
	for (i = 0; szFilter[i] != '\0'; i++){
		if (szFilter[i] == '|' ){
			szFilter[i] = '\0';
		}
	}
	/* 構造体の初期化 */
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = szFilter;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter = 0;
//	m_ofn.nFilterIndex = 3;
	if( 0 == lstrlen( pszPath ) ){
		m_ofn.lpstrFile = strcpy( pszPath, pszFilterArr[(m_ofn.nFilterIndex - 1) * 2 + 1]);
	}else{
		m_ofn.lpstrFile = pszPath;
	}
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.lpstrTitle = NULL;
//	m_ofn.Flags =
//		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
//		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
//	m_ofn.Flags =
//		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
//		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
//		;
	m_ofn.Flags =
		OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
		 /*| OFN_ENABLETEMPLATE | OFN_ENABLEHOOK*/;
	m_ofn.nFileOffset = 0;
	m_ofn.nFileExtension = 0;
	m_ofn.lpstrDefExt = NULL;
	m_ofn.lCustData = (DWORD)this;
	m_ofn.lpfnHook = OFNHookProc;
	m_ofn.lpTemplateName = "IDD_FILEOPEN";
	if( ::GetSaveFileName( &m_ofn ) ){
		return TRUE;
	}else{
//		MYTRACE( "FALSE == GetSaveFileName( .... )\n" );
		BOOL		bCancel;
		const char*	pszError;
		bCancel = FALSE;
		dwError = ::CommDlgExtendedError();
		switch( dwError ){
		case CDERR_DIALOGFAILURE  : pszError = "CDERR_DIALOGFAILURE  "; break;
		case CDERR_FINDRESFAILURE : pszError = "CDERR_FINDRESFAILURE "; break;
		case CDERR_NOHINSTANCE    : pszError = "CDERR_NOHINSTANCE    "; break;
		case CDERR_INITIALIZATION : pszError = "CDERR_INITIALIZATION "; break;
		case CDERR_NOHOOK         : pszError = "CDERR_NOHOOK         "; break;
		case CDERR_LOCKRESFAILURE : pszError = "CDERR_LOCKRESFAILURE "; break;
		case CDERR_NOTEMPLATE     : pszError = "CDERR_NOTEMPLATE     "; break;
		case CDERR_LOADRESFAILURE : pszError = "CDERR_LOADRESFAILURE "; break;
		case CDERR_STRUCTSIZE     : pszError = "CDERR_STRUCTSIZE     "; break;
		case CDERR_LOADSTRFAILURE : pszError = "CDERR_LOADSTRFAILURE "; break;
		case FNERR_BUFFERTOOSMALL : pszError = "FNERR_BUFFERTOOSMALL "; break;
		case CDERR_MEMALLOCFAILURE: pszError = "CDERR_MEMALLOCFAILURE"; break;
		case FNERR_INVALIDFILENAME: pszError = "FNERR_INVALIDFILENAME"; break;
		case CDERR_MEMLOCKFAILURE : pszError = "CDERR_MEMLOCKFAILURE "; break;
		case FNERR_SUBCLASSFAILURE: pszError = "FNERR_SUBCLASSFAILURE"; break;
		default: bCancel = TRUE; break;
		}
		if( !bCancel ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( m_hwndParent, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"ダイアログが開けません。\n\nエラー:%s", pszError
			);
		}
		return FALSE;
	}
}





/*! 「開く」ダイアログ モーダルダイアログの表示 */
BOOL CDlgOpenFile::DoModalOpenDlg( char* pszPath, int* pnCharCode, BOOL* pbReadOnly )
{
	m_bIsSaveDialog = FALSE;	/* 保存のダイアログか */

	DWORD	dwError;
	int		i;
	char	szWork[256];
	char	szFilter[1024];
	char*	pszFilterArr[] = {
//			"テキストファイル", "*.txt",
			"すべてのファイル", "*.*",
			"テキストファイル",	"*.txt",
//			"ユーザー指定",		m_szDefaultWildCard,					//Jul. 09, 2001 JEPRO	これは使わない
			"C/C++ファイル",	"*.c;*.cpp;*.cxx;*.h;*.rc",				//Jul. 08, 2001 JEPRO
			"HTMLファイル",		"*.html;*.htm;*.shtml",					//Jul. 08, 2001 JEPRO
			"Perlファイル",		"*.cgi;*.pl;*.pm",						//Jul. 08, 2001 JEPRO
			"VBファイル",		"*.bas;*.frm;*.cls,*.ctl;*.pag",		//Jul. 08, 2001 JEPRO
			"TeXファイル",		"*.tex;*.ltx;*.sty",					//Jul. 08, 2001 JEPRO
			"Winヘルプ関連",	"*.rtf;*.cnt;*.hpj;*.hh;*.hm"			//Jul. 08, 2001 JEPRO
	};
	int		nFilterArrNum = sizeof( pszFilterArr ) / sizeof( pszFilterArr[0] );
	/* 拡張子フィルタの作成 */
	strcpy( szFilter, "" );
	for( i = 0; i < nFilterArrNum; i += 2 ){
		wsprintf( szWork, "%s (%s)|%s|", pszFilterArr[i], pszFilterArr[i + 1], pszFilterArr[i + 1] );
		strcat( szFilter, szWork );
	}
	strcat( szFilter, "|" );
//	MYTRACE( "%s\n", szFilter );
	for (i = 0; szFilter[i] != '\0'; i++){
		if( szFilter[i] == '|' ){
			szFilter[i] = '\0';
		}
	}
	/* 構造体の初期化 */
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = szFilter;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter = 0;
//	m_ofn.nFilterIndex = 3;
	if( 0 == lstrlen( pszPath ) ){
		m_ofn.lpstrFile = strcpy( pszPath, pszFilterArr[(m_ofn.nFilterIndex - 1) * 2 + 1] );
	}else{
		m_ofn.lpstrFile = pszPath;
	}
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.lpstrTitle = NULL;
//	m_ofn.Flags =
//		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
//		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
	m_ofn.Flags =
		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST  /*| OFN_HIDEREADONLY*/
		 | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK /*| OFN_READONLY*/ | OFN_SHOWHELP | OFN_ENABLESIZING /*| OFN_ALLOWMULTISELECT*/
		;
	if( NULL != pbReadOnly ){
		m_bReadOnly = *pbReadOnly;
		if( TRUE == *pbReadOnly ){
			m_ofn.Flags |= OFN_READONLY;
		}
	}
	m_ofn.nFileOffset = 0;
	m_ofn.nFileExtension = 0;
	m_ofn.lpstrDefExt = NULL;
	m_ofn.lCustData = (DWORD)this;
	m_ofn.lpfnHook = OFNHookProc;
	m_ofn.lpTemplateName = "IDD_FILEOPEN";
//	m_ofn.lpTemplateName = MAKEINTRESOURCE( 149 );

	m_nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);
	m_bUseEol = false;	//	Feb. 9, 2001 genta


	if( ::GetOpenFileName( &m_ofn ) ){

//		MYTRACE( "m_nCharCode = %d\n", m_nCharCode );	/* 文字コード */

		if( NULL != pnCharCode ){
			*pnCharCode = m_nCharCode;
		}
		if( NULL != pbReadOnly ){
			*pbReadOnly = m_bReadOnly;
		}
		return TRUE;
	}else{
//		MYTRACE( "FALSE == GetOpenFileName( .... )\n" );
		BOOL		bCancel;
		const char*	pszError;
		bCancel = FALSE;
		dwError = ::CommDlgExtendedError();
		switch( dwError ){
		case CDERR_DIALOGFAILURE  : pszError = "CDERR_DIALOGFAILURE  "; break;
		case CDERR_FINDRESFAILURE : pszError = "CDERR_FINDRESFAILURE "; break;
		case CDERR_NOHINSTANCE    : pszError = "CDERR_NOHINSTANCE    "; break;
		case CDERR_INITIALIZATION : pszError = "CDERR_INITIALIZATION "; break;
		case CDERR_NOHOOK         : pszError = "CDERR_NOHOOK         "; break;
		case CDERR_LOCKRESFAILURE : pszError = "CDERR_LOCKRESFAILURE "; break;
		case CDERR_NOTEMPLATE     : pszError = "CDERR_NOTEMPLATE     "; break;
		case CDERR_LOADRESFAILURE : pszError = "CDERR_LOADRESFAILURE "; break;
		case CDERR_STRUCTSIZE     : pszError = "CDERR_STRUCTSIZE     "; break;
		case CDERR_LOADSTRFAILURE : pszError = "CDERR_LOADSTRFAILURE "; break;
		case FNERR_BUFFERTOOSMALL : pszError = "FNERR_BUFFERTOOSMALL "; break;
		case CDERR_MEMALLOCFAILURE: pszError = "CDERR_MEMALLOCFAILURE"; break;
		case FNERR_INVALIDFILENAME: pszError = "FNERR_INVALIDFILENAME"; break;
		case CDERR_MEMLOCKFAILURE : pszError = "CDERR_MEMLOCKFAILURE "; break;
		case FNERR_SUBCLASSFAILURE: pszError = "FNERR_SUBCLASSFAILURE"; break;
		default: bCancel = TRUE; break;
		}
		if( !bCancel ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( m_hwndParent, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"ダイアログが開けません。\n\nエラー:%s", pszError
			);
		}
		return FALSE;
	}
}

/*! 保存ダイアログ モーダルダイアログの表示 */
BOOL CDlgOpenFile::DoModalSaveDlg( char* pszPath, int* pnCharCode, CEOL* pcEol )
{
	m_bIsSaveDialog = TRUE;	/* 保存のダイアログか */

	DWORD	dwError;
	int		i;
	char	szWork[256];
	char	szFilter[1024];
	char*	pszFilterArr[] = {
			"ユーザー指定", m_szDefaultWildCard,	//Jul. 09, 2001 JEPRO これがリストの先頭に来るように変更
			"テキストファイル",	"*.txt",
			"すべてのファイル", "*.*"

	};
	int		nFilterArrNum = sizeof( pszFilterArr ) / sizeof( pszFilterArr[0] );
	/* 拡張子フィルタの作成 */
	strcpy( szFilter, "" );
	for( i = 0; i < nFilterArrNum; i+=2 ){
		wsprintf( szWork, "%s (%s)|%s|", pszFilterArr[i], pszFilterArr[i + 1], pszFilterArr[i + 1] );
		strcat( szFilter, szWork );
	}
	strcat( szFilter, "|" );
//	MYTRACE( "%s\n", szFilter );
	for (i = 0; szFilter[i] != '\0'; i++){
		if( szFilter[i] == '|' ){
			szFilter[i] = '\0';
		}
	}
	/* 構造体の初期化 */
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = szFilter;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter = 0;
//	m_ofn.nFilterIndex = 3;
	if( 0 == lstrlen( pszPath ) ){
		m_ofn.lpstrFile = strcpy( pszPath, pszFilterArr[(m_ofn.nFilterIndex - 1) * 2 + 1] );
	}else{
		m_ofn.lpstrFile = pszPath;
	}
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.lpstrTitle = NULL;
//	m_ofn.Flags =
//		/*OFN_CREATEPROMPT |*/ OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
//		/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK */
	m_ofn.Flags =
		OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
		 /*| OFN_ENABLETEMPLATE | OFN_ENABLEHOOK*/ | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( NULL != pnCharCode || NULL != pcEol ){
		m_ofn.Flags = m_ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	}
	m_ofn.nFileOffset = 0;
	m_ofn.nFileExtension = 0;
	m_ofn.lpstrDefExt = NULL;
	m_ofn.lCustData = (DWORD)this;
	m_ofn.lpfnHook = OFNHookProc;
	m_ofn.lpTemplateName = "IDD_FILEOPEN";

	if( NULL != pnCharCode ){
		m_nCharCode = *pnCharCode;
	}
	//	From Here Feb. 9, 2001 genta
	if( NULL != pcEol ){
		m_cEol = EOL_NONE;	//	初期値は「改行コードを保存」に固定
		m_bUseEol = true;
	}
	else{
		m_bUseEol = false;
	}
	//	To Here Feb. 9, 2001 genta
	m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 機能番号からヘルプトピック番号を調べるようにした
	if( ::GetSaveFileName( &m_ofn ) ){

//		MYTRACE( "m_nCharCode = %d\n", m_nCharCode );	/* 文字コード */

		if( NULL != pnCharCode ){
			*pnCharCode = m_nCharCode;
		}
		//	Feb. 9, 2001 genta
		if( m_bUseEol ){
			*pcEol = m_cEol;
		}
		return TRUE;
	}else{
//		MYTRACE( "FALSE == GetOpenFileName( .... )\n" );
		BOOL		bCancel;
		const char*	pszError;
		bCancel = FALSE;
		dwError = ::CommDlgExtendedError();
		switch( dwError ){
		case CDERR_DIALOGFAILURE  : pszError = "CDERR_DIALOGFAILURE  "; break;
		case CDERR_FINDRESFAILURE : pszError = "CDERR_FINDRESFAILURE "; break;
		case CDERR_NOHINSTANCE    : pszError = "CDERR_NOHINSTANCE    "; break;
		case CDERR_INITIALIZATION : pszError = "CDERR_INITIALIZATION "; break;
		case CDERR_NOHOOK         : pszError = "CDERR_NOHOOK         "; break;
		case CDERR_LOCKRESFAILURE : pszError = "CDERR_LOCKRESFAILURE "; break;
		case CDERR_NOTEMPLATE     : pszError = "CDERR_NOTEMPLATE     "; break;
		case CDERR_LOADRESFAILURE : pszError = "CDERR_LOADRESFAILURE "; break;
		case CDERR_STRUCTSIZE     : pszError = "CDERR_STRUCTSIZE     "; break;
		case CDERR_LOADSTRFAILURE : pszError = "CDERR_LOADSTRFAILURE "; break;
		case FNERR_BUFFERTOOSMALL : pszError = "FNERR_BUFFERTOOSMALL "; break;
		case CDERR_MEMALLOCFAILURE: pszError = "CDERR_MEMALLOCFAILURE"; break;
		case FNERR_INVALIDFILENAME: pszError = "FNERR_INVALIDFILENAME"; break;
		case CDERR_MEMLOCKFAILURE : pszError = "CDERR_MEMLOCKFAILURE "; break;
		case FNERR_SUBCLASSFAILURE: pszError = "FNERR_SUBCLASSFAILURE"; break;
		default: bCancel = TRUE; break;
		}
		if( !bCancel ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( m_hwndParent, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"ダイアログが開けません。\n\nエラー:%s", pszError
			);
		}
		return FALSE;
	}
}


/*[EOF]*/
