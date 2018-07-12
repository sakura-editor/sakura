/*!	@file
@brief CViewCommanderクラスのコマンド(支援)関数群

	2012/12/16	CViewCommander_Hokan.cppにcppCViewCommander.cppから支援関連を分離しCViewCommander_Support.cppに名称変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, kobake, ryoji
	Copyright (C) 2011, Moca
	Copyright (C) 2012, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <HtmlHelp.h>
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "CPropertyManager.h"
#include "CEditApp.h"
#include "dlg/CDlgAbout.h"	//Dec. 24, 2000 JEPRO 追加
#include "env/CHelpManager.h"
#include "util/module.h"
#include "util/shell.h"

/*!	入力補完
	Ctrl+Spaceでここに到着。
	CEditView::m_bHokan： 現在補完ウィンドウが表示されているかを表すフラグ。

    @date 2001/06/19 asa-o 英大文字小文字を同一視する
                     候補が1つのときはそれに確定する
	@date 2001/06/14 asa-o 参照データ変更
	                 開くプロパティシートをタイプ別に変更y
	@date 2000/09/15 JEPRO [Esc]キーと[x]ボタンでも中止できるように変更
	@date 2005/01/10 genta CEditView_Commandから移動
*/
void CViewCommander::Command_HOKAN( void )
{
#if 0
// 2011.06.24 Moca Plugin導入に従い未設定の確認をやめる
retry:;
	/* 補完候補一覧ファイルが設定されていないときは、設定するように促す。 */
	// 2003.06.22 Moca ファイル内から検索する場合には補完ファイルの設定は必須ではない
	if( m_pCommanderView->m_pTypeData->m_bUseHokanByFile == FALSE &&
		m_pCommanderView->m_pTypeData->m_bUseHokanByKeyword == false &&
		_T('\0') == m_pCommanderView->m_pTypeData->m_szHokanFile[0]
	){
		ConfirmBeep();
		if( IDYES == ::ConfirmMessage( GetMainWindow(),
			LS(STR_ERR_DLGEDITVWHOKAN1)
		) ){
			/* タイプ別設定 プロパティシート */
			if( !CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheetTypes( 2, GetDocument()->m_cDocType.GetDocumentType() ) ){
				return;
			}
			goto retry;
		}
	}
#endif
	CNativeW	cmemData;
	/* カーソル直前の単語を取得 */
	if( 0 < m_pCommanderView->GetParser().GetLeftWord( &cmemData, 100 ) ){
		m_pCommanderView->ShowHokanMgr( cmemData, TRUE );
	}else{
		InfoBeep(); //2010.04.03 Error→Info
		m_pCommanderView->SendStatusMessage(LS(STR_SUPPORT_NOT_COMPLITE)); // 2010.05.29 ステータスで表示
	}
	return;
}



/*! キャレット位置の単語を辞書検索ON-OFF

	@date 2006.03.24 fon 新規作成
*/
void CViewCommander::Command_ToggleKeySearch( int option )
{	/* 共通設定ダイアログの設定をキー割り当てでも切り替えられるように */
	if( option == 0 ){
		if( GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord ){
			GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = FALSE;
		}else{
			GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = TRUE;
		}
	}else if( option == 1 ){
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = TRUE;
	}else if( option == 2 ){
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = FALSE;
	}
}



/* ヘルプ目次 */
void CViewCommander::Command_HELP_CONTENTS( void )
{
	ShowWinHelpContents( m_pCommanderView->GetHwnd() );	//	目次を表示する
	return;
}



/* ヘルプキーワード検索 */
void CViewCommander::Command_HELP_SEARCH( void )
{
	MyWinHelp( m_pCommanderView->GetHwnd(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelpに変更に変更
	return;
}



/* コマンド一覧 */
void CViewCommander::Command_MENU_ALLFUNC( void )
{

	UINT	uFlags;
	POINT	po;
	RECT	rc;
	HMENU	hMenu;
	HMENU	hMenuPopUp;
	int		i;
	int		j;
	int		nId;

//	From Here Sept. 15, 2000 JEPRO
//	サブメニュー、特に「その他」のコマンドに対してステータスバーに表示されるキーアサイン情報が
//	メニューで隠れないように右にずらした
//	(本当はこの「コマンド一覧」メニューをダイアログに変更しバーをつまんで自由に移動できるようにしたい)
//	po.x = 0;
	po.x = 540;
//	To Here Sept. 15, 2000 (Oct. 7, 2000 300→500; Nov. 3, 2000 500→540)
	po.y = 0;

	CEditWnd*	pCEditWnd = GetEditWindow();	//	Sep. 10, 2002 genta
	::GetClientRect( pCEditWnd->GetHwnd(), &rc );
	po.x = t_min( po.x, rc.right );
	::ClientToScreen( pCEditWnd->GetHwnd(), &po );
	::GetWindowRect( pCEditWnd->m_cSplitterWnd.GetHwnd() , &rc );
	po.y = rc.top;

	pCEditWnd->GetMenuDrawer().ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更したことで1番(カーソル移動系)が前にシフトされた(この変更によって i=1→i=0 と変更)
	//	Oct. 3, 2001 genta
	for( i = 0; i < FuncLookup.GetCategoryCount(); i++ ){
		hMenuPopUp = ::CreatePopupMenu();
		for( j = 0; j < FuncLookup.GetItemCount(i); j++ ){
			//	Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode( i, j, false );	// 2007.11.02 ryoji 未登録マクロ非表示を明示指定
			if( code != 0 ){
				WCHAR	szLabel[300];
				FuncLookup.Pos2FuncName( i, j, szLabel, 256 );
				uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				//	Oct. 3, 2001 genta
				pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenuPopUp, uFlags, code, szLabel, L"" );
			}
		}
		//	Oct. 3, 2001 genta
		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , FuncLookup.Category2Name(i) , _T(""));
//		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i] );
	}

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow()/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu( hMenu );
	if( 0 != nId ){
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nFuncID, true, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
	}
	return;
}



/* 外部ヘルプ１
	@date 2012.09.26 Moca HTMLHELP対応
*/
void CViewCommander::Command_EXTHELP1( void )
{
retry:;
	if( CHelpManager().ExtWinHelpIsSet( &(GetDocument()->m_cDocType.GetDocumentAttribute()) ) == false){
//	if( 0 == wcslen( GetDllShareData().m_Common.m_szExtHelp1 ) ){
		ErrorBeep();
//From Here Sept. 15, 2000 JEPRO
//		[Esc]キーと[x]ボタンでも中止できるように変更
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
//To Here Sept. 15, 2000
			LS(STR_ERR_CEDITVIEW_CMD01)
		) ){
			/* 共通設定 プロパティシート */
			if( !CEditApp::getInstance()->OpenPropertySheet( ID_PROPCOM_PAGENUM_HELPER ) ){
				return;
			}
			goto retry;
		}
		//	Jun. 15, 2000 genta
		else{
			return;
		}
	}

	CNativeW		cmemCurText;
	const TCHAR*	helpfile = CHelpManager().GetExtWinHelp( &(GetDocument()->m_cDocType.GetDocumentAttribute()) );

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	m_pCommanderView->GetCurrentTextForSearch( cmemCurText, false );
	TCHAR path[_MAX_PATH];
	if( _IS_REL_PATH( helpfile ) ){
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		GetInidirOrExedir( path, helpfile );
	}else{
		auto_strcpy( path, helpfile );
	}
	// 2012.09.26 Moca HTMLHELP対応
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath( path, NULL, NULL, NULL, szExt );
	if( 0 == _tcsicmp(szExt, _T(".chi")) || 0 == _tcsicmp(szExt, _T(".chm")) || 0 == _tcsicmp(szExt, _T(".col")) ){
		std::wstring pathw = to_wchar(path);
		Command_EXTHTMLHELP( pathw.c_str(), cmemCurText.GetStringPtr() );
	}else{
		::WinHelp( m_pCommanderView->m_hwndParent, path, HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
	}
	return;
}



/*!
	外部HTMLヘルプ
	
	@param helpfile [in] HTMLヘルプファイル名．NULLのときはタイプ別に設定されたファイル．
	@param kwd [in] 検索キーワード．NULLのときはカーソル位置or選択されたワード
	@date 2002.07.05 genta 任意のファイル・キーワードの指定ができるよう引数追加
*/
void CViewCommander::Command_EXTHTMLHELP( const WCHAR* _helpfile, const WCHAR* kwd )
{
	std::tstring helpfile;
	if( _helpfile != NULL ){
		helpfile = to_tchar(_helpfile);
	}

	HWND		hwndHtmlHelp;
	int			nLen;

	DEBUG_TRACE( _T("helpfile=%ts\n"), helpfile.c_str() );

	//	From Here Jul. 5, 2002 genta
	const TCHAR *filename = NULL;
	if ( 0 == helpfile.length() ){
		while( !CHelpManager().ExtHTMLHelpIsSet( &(GetDocument()->m_cDocType.GetDocumentAttribute())) ){
			ErrorBeep();
	//	From Here Sept. 15, 2000 JEPRO
	//		[Esc]キーと[x]ボタンでも中止できるように変更
			if( IDYES != ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
	//	To Here Sept. 15, 2000
				LS(STR_ERR_CEDITVIEW_CMD02)
			) ){
				return;
			}
			/* 共通設定 プロパティシート */
			if( !CEditApp::getInstance()->OpenPropertySheet( ID_PROPCOM_PAGENUM_HELPER ) ){
				return;
			}
		}
		filename = CHelpManager().GetExtHTMLHelp( &(GetDocument()->m_cDocType.GetDocumentAttribute()) );
	}
	else {
		filename = helpfile.c_str();
	}
	//	To Here Jul. 5, 2002 genta

	//	Jul. 5, 2002 genta
	//	キーワードの外部指定を可能に
	CNativeW	cmemCurText;
	if( kwd != NULL && kwd[0] != _T('\0') ){
		cmemCurText.SetString( kwd );
	}
	else {
		/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText );
	}

	/* HtmlHelpビューアはひとつ */
	if( CHelpManager().HTMLHelpIsSingle( &(GetDocument()->m_cDocType.GetDocumentAttribute())) ){
		// タスクトレイのプロセスにHtmlHelpを起動させる
		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		TCHAR* pWork=GetDllShareData().m_sWorkBuffer.GetWorkBuffer<TCHAR>();
		if( _IS_REL_PATH( filename ) ){
			GetInidirOrExedir( pWork, filename );
		}else{
			_tcscpy( pWork, filename ); //	Jul. 5, 2002 genta
		}
		nLen = _tcslen( pWork );
		_tcscpy( &pWork[nLen + 1], cmemCurText.GetStringT() );
		hwndHtmlHelp = (HWND)::SendMessageAny(
			GetDllShareData().m_sHandles.m_hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)GetMainWindow(),
			0
		);
	}
	else{
		/* 自分でHtmlHelpを起動させる */
		HH_AKLINK	link;
		link.cbStruct = sizeof( link ) ;
		link.fReserved = FALSE ;
		link.pszKeywords = cmemCurText.GetStringT();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca 相対パスは実行ファイルからのパス
		// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( filename ) ){
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir( path, filename );
			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				path, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else{
			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				filename, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	//	Jul. 6, 2001 genta hwndHtmlHelpのチェックを追加
	if( hwndHtmlHelp != NULL ){
		::BringWindowToTop( hwndHtmlHelp );
	}

	return;
}



/* バージョン情報 */
void CViewCommander::Command_ABOUT( void )
{
	CDlgAbout cDlgAbout;
	cDlgAbout.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd() );
	return;
}
