/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <HtmlHelp.h>
#include <ShlObj.h>
#include <shellapi.h>
#include <CdErr.h> // Nov. 3, 2005 genta	//CDERR_FINDRESFAILURE等
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/file.h"
#include "util/os.h"
#include "util/module.h"
#include "util/window.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "config/app_constants.h"
#include "cxx/com_pointer.hpp"
#include "cxx/ResourceHolder.hpp"

#pragma comment(lib, "htmlhelp.lib") 

BOOL SelectDir(HWND hWnd, const std::wstring& title, const std::filesystem::path& initialDirectory, WCHAR* strFolderName, size_t nMaxCount)
{
	return SelectDir(hWnd, title, initialDirectory, std::span(strFolderName, nMaxCount));
}

/* フォルダー選択ダイアログ */
BOOL SelectDir(
	HWND hWnd,
	const std::wstring& title,
	const std::filesystem::path& initialDirectory,
	std::span<WCHAR> buffer
)
{
	auto strFolderName = buffer.data();
	auto nMaxCount = buffer.size();

	if (!strFolderName || !nMaxCount) {
		return FALSE;
	}

	cxx::com_pointer<IFileDialog> pDialog;
	HRESULT hres;

	// インスタンスを作成
	hres = pDialog.CreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER);
	if ( FAILED(hres) ) {
		return FALSE;
	}

	// デフォルト設定を取得
	DWORD dwOptions = 0;
	hres = pDialog->GetOptions( &dwOptions );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	// オプションをフォルダーを選択可能に変更
	hres = pDialog->SetOptions( dwOptions | FOS_PICKFOLDERS | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	// 初期フォルダーを設定
	cxx::com_pointer<IShellItem> psiFolder;
	hres = SHCreateItemFromParsingName(initialDirectory.c_str(), nullptr, IID_PPV_ARGS(&psiFolder));
	if ( SUCCEEDED(hres) ) {
		pDialog->SetFolder(psiFolder);
	}

	// タイトル文字列を設定
	hres = pDialog->SetTitle( title.c_str() );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	// フォルダー選択ダイアログを表示
	hres = pDialog->Show( hWnd );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	// 選択結果を取得
	cxx::com_pointer<IShellItem> psiResult;
	hres = pDialog->GetResult( &psiResult );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	PWSTR pszResult;
	hres = psiResult->GetDisplayName( SIGDN_FILESYSPATH, &pszResult );
	if ( FAILED(hres) ) {
		return FALSE;
	}

	using CoTaskMemHolder = cxx::ResourceHolder<&::CoTaskMemFree>;
	CoTaskMemHolder taskMem = pszResult;

	BOOL bRet = TRUE;
	if ( 0 != wcsncpy_s( strFolderName, nMaxCount, pszResult, _TRUNCATE ) ) {
		wcsncpy_s( strFolderName, nMaxCount, L"", _TRUNCATE );
		bRet = FALSE;
	}

	return bRet;
}

///////////////////////////////////////////////////////////////////////
// From Here 2007.05.25 ryoji 独自拡張のプロパティシート関数群

/*!	独自拡張プロパティシートのウィンドウプロシージャ
	@author ryoji
	@date 2007.05.25 新規
*/
static LRESULT CALLBACK PropSheetWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, [[maybe_unused]] DWORD_PTR dwRefData )
{
	switch( uMsg ){
	case WM_SHOWWINDOW:
		// 追加ボタンの位置を調整する
		if( wParam ){
			HWND hwndBtn;
			RECT rcOk;
			RECT rcTab;
			POINT pt;

			hwndBtn = ::GetDlgItem( hwnd, 0x02000 );
			::GetWindowRect( ::GetDlgItem( hwnd, IDOK ), &rcOk );
			::GetWindowRect( PropSheet_GetTabControl( hwnd ), &rcTab );
			pt.x = rcTab.left;
			pt.y = rcOk.top;
			::ScreenToClient( hwnd, &pt );
			::MoveWindow( hwndBtn, pt.x, pt.y, DpiScaleX(140), rcOk.bottom - rcOk.top, FALSE );
		}
		break;

	case WM_COMMAND:
		// 追加ボタンが押された時はその処理を行う
		if( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == 0x02000 ){
			HWND hwndBtn = ::GetDlgItem( hwnd, 0x2000 );
			RECT rc;
			POINT pt;

			// メニューを表示する
			::GetWindowRect( hwndBtn, &rc );
			pt.x = rc.left;
			pt.y = rc.bottom;
			GetMonitorWorkRect( pt, &rc );	// モニタのワークエリア

			HMENU hMenu = ::CreatePopupMenu();
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, LS(STR_SHELL_MENU_OPEN) );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, LS(STR_SHELL_MENU_IMPEXP) );

			int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										( pt.x > rc.left )? pt.x: rc.left,
										( pt.y < rc.bottom )? pt.y: rc.bottom,
										0, hwnd, nullptr );
			::DestroyMenu( hMenu );

			// 選択されたメニューの処理
			switch( nId ){
			case 100:	// 設定フォルダを開く
				WCHAR szPath[_MAX_PATH];
				GetInidir( szPath );

				// フォルダの ITEMIDLIST を取得して ShellExecuteEx() で開く
				// Note. MSDN の ShellExecute() の解説にある方法でフォルダを開こうとした場合、
				//       フォルダと同じ場所に <フォルダ名>.exe があるとうまく動かない。
				//       verbが"open"やNULLではexeのほうが実行され"explore"では失敗する
				//       （フォルダ名の末尾に'\\'を付加してもWindows 2000では付加しないのと同じ動作になってしまう）
				LPSHELLFOLDER pDesktopFolder;
				if( SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder)) ){
					LPMALLOC pMalloc;
					if( SUCCEEDED(::SHGetMalloc(&pMalloc)) ){
						LPITEMIDLIST pIDL;
						WCHAR* pszDisplayName = szPath;
						if( SUCCEEDED(pDesktopFolder->ParseDisplayName(nullptr, nullptr, pszDisplayName, nullptr, &pIDL, nullptr)) ){
							SHELLEXECUTEINFO si;
							::ZeroMemory( &si, sizeof(si) );
							si.cbSize   = sizeof(si);
							si.fMask    = SEE_MASK_IDLIST;
							si.lpVerb   = L"open";
							si.lpIDList = pIDL;
							si.nShow    = SW_SHOWNORMAL;
							::ShellExecuteEx( &si );	// フォルダを開く
							pMalloc->Free( (void*)pIDL );
						}
						pMalloc->Release();
					}
					pDesktopFolder->Release();
				}
				break;

			case 101:	// インポート／エクスポートの起点リセット（起点を設定フォルダーにする）
			{
				int nMsgResult = MYMESSAGEBOX(
					hwnd,
					MB_OKCANCEL | MB_ICONINFORMATION,
					GSTR_APPNAME,
					LS(STR_SHELL_IMPEXPDIR)
				);
				if( IDOK == nMsgResult )
				{
					DLLSHAREDATA *pShareData = &GetDllShareData();
					GetInidir( pShareData->m_sHistory.m_szIMPORTFOLDER );
					AddLastChar( pShareData->m_sHistory.m_szIMPORTFOLDER, std::size(pShareData->m_sHistory.m_szIMPORTFOLDER), L'\\' );
				}
				break;
			}
			default:
				break;
			}
		}
		break;

	case WM_DESTROY:
		::RemoveWindowSubclass(hwnd, &PropSheetWndProc, uIdSubclass);
		return 0;
	default:
		break;
	}

	return ::DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/*!	独自拡張プロパティシートのコールバック関数
	@author ryoji
	@date 2007.05.25 新規
*/
static int CALLBACK PropSheetProc( HWND hwndDlg, UINT uMsg, [[maybe_unused]] LPARAM lParam )
{
	// プロパティシートの初期化時にシステムフォント設定、ボタン追加、プロパティシートのサブクラス化を行う
	if( uMsg == PSCB_INITIALIZED ){
		// システムフォント設定は言語設定に関係なく実施(force=TRUE)
		HFONT hFont = UpdateDialogFont( hwndDlg, TRUE );

		if( CShareData::getInstance()->IsPrivateSettings() ){
			// 個人設定フォルダーを使用するときは「設定フォルダー」ボタンを追加する
			::SetWindowSubclass(hwndDlg, &PropSheetWndProc, 0, 0);
			HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle( nullptr );
			HWND hwndBtn = ::CreateWindowEx( 0, WC_BUTTON, LS(STR_SHELL_INIFOLDER), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 140, 20, hwndDlg, (HMENU)0x02000, hInstance, nullptr );
			::SendMessage( hwndBtn, WM_SETFONT, (WPARAM)hFont, MAKELPARAM( FALSE, 0 ) );
			::SetWindowPos( hwndBtn, ::GetDlgItem( hwndDlg, IDHELP), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		}
	}
	return 0;
}

/*!	独自拡張プロパティシート（共通設定／タイプ別設定画面用）
	@author ryoji
	@date 2007.05.25 新規
*/
INT_PTR MyPropertySheet( LPPROPSHEETHEADER lppsph )
{
	lppsph->dwFlags |= PSH_USECALLBACK;
	lppsph->pfnCallback = PropSheetProc;

	return ::PropertySheet( lppsph );
}

// To Here 2007.05.25 ryoji 独自拡張のプロパティシート関数群
///////////////////////////////////////////////////////////////////////

/*	ヘルプの目次を表示
	目次タブを表示。問題があるバージョンでは、目次ページを表示。
*/
void ShowWinHelpContents( HWND hwnd )
{
	/* 目次タブを表示する */
	MyWinHelp( hwnd, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );	// 2006.10.10 ryoji MyWinHelpに変更
	return;
}

// Stonee, 2001/12/21
// NetWork上のリソースに接続するためのダイアログを出現させる
// NO_ERROR:成功 ERROR_CANCELLED:キャンセル それ以外:失敗
// プロジェクトの設定でリンクモジュールにMpr.libを追加のこと
DWORD NetConnect ( const WCHAR strNetWorkPass[] )
{
	//char sPassWord[] = "\0";	//パスワード
	//char sUser[] = "\0";		//ユーザー名
	DWORD dwRet;				//戻り値　エラーコードはWINERROR.Hを参照
	WCHAR sTemp[256];
	WCHAR sDrive[] = L"";
    int i;

	if (wcslen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  //UNCではない。
	if (strNetWorkPass[0] != L'\\' && strNetWorkPass[1] != L'\\')	return ERROR_BAD_NET_NAME;  //UNCではない。

	//3文字目から数えて最初の\の直前までを切り出す
	sTemp[0] = L'\\';
	sTemp[1] = L'\\';
	for (i = 2; strNetWorkPass[i] != L'\0'; i++) {
		if (strNetWorkPass[i] == L'\\') break;
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = L'\0';	//終端

	//NETRESOURCE作成
	NETRESOURCE nr;
	ZeroMemory( &nr, sizeof( nr ) );
	nr.dwScope       = RESOURCE_GLOBALNET;
	nr.dwType        = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage       = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName   = sDrive;
	nr.lpRemoteName  = sTemp;

	//ユーザー認証ダイアログを表示
	dwRet = WNetAddConnection3(nullptr, &nr, nullptr, nullptr, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);

	return dwRet;
}

/*!
 * HTML Helpを開く
 *
 * @param[in, opt] hWnd 呼び出し元ウィンドウのハンドル（省略可）
 * @param[in] szFile HTML Helpのファイル名。不等号(>)に続けてウィンドウタイプ名を指定可能
 * @param[in] uCmd HtmlHelpに渡すコマンド
 * @param[in] data コマンドに応じたデータ（省略可）
 *
 * @return 開いたヘルプウィンドウのハンドル。失敗時は nullptr
 *
 * @note Windows95以前ではHHCtrl.ocxが標準で入っていないバージョンが存在した。
 *       過去にはHHCtrl.ocx未検出時にエラーメッセージを表示していたが、
 *       現在はHHCtrl.ocxを静的リンクする方式に変更したため、チェック処理は削除した。
 *
 * @date 2001/06/26 genta 新規作成
 */
HWND OpenHtmlHelp(
	HWND		hWnd,	//!< [in] 呼び出し元ウィンドウのウィンドウハンドル
	LPCWSTR		szFile,	//!< [in] HTML Helpのファイル名。不等号に続けてウィンドウタイプ名を指定可能。
	UINT		uCmd,	//!< [in] HTML Help に渡すコマンド
	DWORD_PTR	data,	//!< [in] コマンドに応じたデータ
	[[maybe_unused]] bool		msgflag	//!< [in] エラーメッセージを表示するか。省略時はtrue。
)
{
	return ::HtmlHelpW( hWnd, szFile, uCmd, data );
}

/*! ショートカット(.lnk)の解決
	@date 2009.01.08 ryoji CoInitialize/CoUninitializeを削除（WinMainにOleInitialize/OleUninitializeを追加）
*/
bool ResolveShortcutLink(HWND hWnd, const std::filesystem::path& linkFile, std::span<WCHAR> szPath)
{
	szPath[0] = 0;

	SFilePath szAbsLongPath;
	if (!GetLongFileName(linkFile, szAbsLongPath)) {
		return false;	// ファイルが存在しない
	}

	// 2010.08.28 DLL インジェクション対策としてEXEのフォルダーに移動する
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	// Get a pointer to the IShellLink interface.
	cxx::com_pointer<IShellLink> pShellLink = nullptr;
	if (FAILED(pShellLink.CreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER))) {
		return false;
	}

	// Get a pointer to the IPersistFile interface.
	cxx::com_pointer<IPersistFile> pPersistFile = nullptr;
	if (FAILED(pShellLink->QueryInterface(&pPersistFile))) {
		return false;
	}

	// Load the shortcut.
	if (FAILED(pPersistFile->Load(szAbsLongPath, STGM_READ))) {
		return false;
	}

	// Resolve the link.
	if (FAILED(pShellLink->Resolve(hWnd, SLR_ANY_MATCH))) {
		return false;
	}

	// Get the path to the link target.
	SFilePath szGotPath;
	WIN32_FIND_DATA	wfd{};
	if (FAILED(pShellLink->GetPath(szGotPath, int(std::size(szGotPath)), &wfd, SLGP_SHORTPATH))) {
		return false;
	}

	// Get the description of the target.
	SFilePath szDescription;
	if (FAILED(pShellLink->GetDescription(szDescription, int(std::size(szDescription))))) {
		return false;
	}

	if (szGotPath.empty()) {
		return false;
	}

	// 正常終了
	if (STRUNCATE == ::wcsncpy_s(std::data(szPath), std::size(szPath), szGotPath, _TRUNCATE)) {
		return false;
	}

	return true;
}

/*! ヘルプファイルのフルパスを返す
 
    @return パスを格納したバッファのポインタ
 
    @note 実行ファイルと同じ位置の sakura.chm ファイルを返す。
        パスが UNC のときは _MAX_PATH に収まらない可能性がある。
 
    @date 2002/01/19 aroka ；nMaxLen 引数追加
	@date 2007/10/23 kobake 引数説明の誤りを修正(in→out)
	@date 2007/10/23 kobake CEditAppのメンバ関数に変更
	@date 2007/10/23 kobake シグニチャ変更。constポインタを返すだけのインターフェースにしました。
*/
static LPCWSTR GetHelpFilePath()
{
	static WCHAR szHelpFile[_MAX_PATH] = L"";
	if(szHelpFile[0]==L'\0'){
		GetExedir( szHelpFile, L"sakura.chm" );
	}
	return szHelpFile;
}

/*!	WinHelp のかわりに HtmlHelp を呼び出す

	@author ryoji
	@date 2006.07.22 ryoji 新規
*/
BOOL MyWinHelp(HWND hwndCaller, UINT uCommand, DWORD_PTR dwData)
{
	UINT uCommandOrg = uCommand;	// WinHelp のコマンド
	bool bDesktop = false;	// デスクトップを親にしてヘルプ画面を出すかどうか
	HH_POPUP hp;	// ポップアップヘルプ用の構造体

	// Note: HH_TP_HELP_CONTEXTMENU や HELP_WM_HELP ではヘルプコンパイル時の
	// トピックファイルを Cshelp.txt 以外にしている場合、
	// そのファイル名を .chm パス名に追加指定する必要がある。
	//     例）sakura.chm::/xxx.txt

	switch( uCommandOrg )
	{
	case HELP_COMMAND:	// [ヘルプ]-[目次]
	case HELP_CONTENTS:
		uCommand = HH_DISPLAY_TOC;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_KEY:	// [ヘルプ]-[キーワード検索]
		uCommand = HH_DISPLAY_INDEX;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXT:	// メニュー上での[F1]キー／ダイアログの[ヘルプ]ボタン
		uCommand = HH_HELP_CONTEXT;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXTMENU:	// コントロール上での右クリック
	case HELP_WM_HELP:		// [？]ボタンを押してコントロールをクリック／コントロールにフォーカスを置いて[F1]キー
		uCommand = HH_DISPLAY_TEXT_POPUP;
		{
			// ポップアップヘルプ用の構造体に値をセットする
			HWND hwndCtrl;	// ヘルプ表示対象のコントロール
			int nCtrlID;	// 対象コントロールの ID
			DWORD* pHelpIDs;	// コントロール ID とヘルプ ID の対応表へのポインタ

			memset(&hp, 0, sizeof(hp));	// 構造体をゼロクリア
			hp.cbStruct = sizeof(hp);
			hp.pszFont = L"ＭＳ Ｐゴシック, 9";
			hp.clrForeground = hp.clrBackground = COLORREF(-1);
			hp.rcMargins.left = hp.rcMargins.top = hp.rcMargins.right = hp.rcMargins.bottom = -1;
			if( uCommandOrg == HELP_CONTEXTMENU ){
				// マウスカーソル位置から対象コントロールと表示位置を求める
				if( !::GetCursorPos(&hp.pt) )
					return FALSE;
				hwndCtrl = ::WindowFromPoint(hp.pt);
			}
			else{
				// 対象コントロールは hwndCaller
				// [F1]キーの場合もあるので対象コントロールの位置から表示位置を決める
				RECT rc;
				hwndCtrl = hwndCaller;
				if( !::GetWindowRect( hwndCtrl, &rc ) )
					return FALSE;
				hp.pt.x = (rc.left + rc.right) / 2;
				hp.pt.y = rc.top;
			}
			// 対象コントロールに対応するヘルプ ID を探す
			nCtrlID = ::GetDlgCtrlID( hwndCtrl );
			if( nCtrlID <= 0 )
				return FALSE;
			pHelpIDs = (DWORD*)dwData;
			for (;;) {
				if( *pHelpIDs == 0 )
					return FALSE;	// 見つからなかった
				if( *pHelpIDs == (DWORD)nCtrlID )
					break;			// 見つかった
				pHelpIDs += 2;
			}
			hp.idString = *(pHelpIDs + 1);	// 見つけたヘルプ ID を設定する
			dwData = (DWORD_PTR)&hp;	// 引数をポップアップヘルプ用の構造体に差し替える
		}
		break;
	default:
		return FALSE;
	}

	LPCWSTR lpszHelp = GetHelpFilePath();
	if( IsFileExists( lpszHelp, true ) ){
		// HTML ヘルプを呼び出す
		HWND hWnd = OpenHtmlHelp( hwndCaller, lpszHelp, uCommand, dwData );
		if (bDesktop && hWnd != nullptr){
			::SetForegroundWindow( hWnd );	// ヘルプ画面を手前に出す
		}
	}
	else {
		if( uCommandOrg == HELP_CONTEXTMENU)
			return FALSE;	// 右クリックでは何もしないでおく

		// オンラインヘルプを呼び出す
		if( uCommandOrg != HELP_CONTEXT )
			dwData = 1;	// 目次ページ

		WCHAR buf[256];
		swprintf( buf, int(std::size(buf)), L"https://sakura-editor.github.io/help/HLP%06Iu.html", dwData );
		ShellExecute( ::GetActiveWindow(), nullptr, buf, nullptr, nullptr, SW_SHOWNORMAL );
	}

	return TRUE;
}

/*フォント選択ダイアログ
	@param plf [in,out]
	@param piPointSize [in,out] 1/10ポイント単位
	
	2008.04.27 kobake CEditDoc::SelectFont から分離
	2009.10.01 ryoji ポイントサイズ（1/10ポイント単位）引数追加
*/
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool FixedFontOnly )
{
	// 2004.02.16 Moca CHOOSEFONTをメンバから外す
	CHOOSEFONT cf;
	/* CHOOSEFONTの初期化 */
	::ZeroMemory( &cf, sizeof( cf ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = hwndDlgOwner;
	cf.hDC = nullptr;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	if( FixedFontOnly ){
		//FIXEDフォント
		cf.Flags |= CF_FIXEDPITCHONLY;
	}
	cf.lpLogFont = plf;
	if( !ChooseFont( &cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE( L"CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE( L"CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE( L"CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE( L"CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE( L"CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE( L"CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE( L"CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE( L"CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE( L"CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE( L"CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE( L"CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE( L"CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE( L"CFERR_NOFONTS \n" );			break;
		default:
			break;
		}
#endif
		return FALSE;
	}
	*piPointSize = cf.iPointSize;

	return TRUE;
}
