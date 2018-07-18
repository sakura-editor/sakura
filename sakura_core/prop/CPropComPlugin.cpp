/*!	@file
	共通設定ダイアログボックス、「プラグイン」ページ

	@author syat
*/
/*
	Copyright (C) 2009, syat

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
#include <ShellAPI.h>
#include "prop/CPropCommon.h"
#include "CEditApp.h"
#include "plugin/CJackManager.h"
#include "uiparts/CMenuDrawer.h"
#include "util/shell.h"
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDlgPluginOption.h"	// 2010/3/22 Uchi
#include "io/CTextStream.h"
#include "io/CZipFile.h"
#include "sakura_rc.h"
#include "sakura.hh"

static void LoadPluginTemp(CommonSetting& common, CMenuDrawer& cMenuDrawer);

//! Popup Help用ID
static const DWORD p_helpids[] = {	//11700
	IDC_CHECK_PluginEnable,	HIDC_CHECK_PluginEnable,	//プラグインを有効にする
	IDC_PLUGINLIST,			HIDC_PLUGINLIST,			//プラグインリスト
	IDC_PLUGIN_INST_ZIP,	HIDC_PLUGIN_INST_ZIP,		//Zipプラグインを追加	// 2011/11/2 Uchi
	IDC_PLUGIN_SearchNew,	HIDC_PLUGIN_SearchNew,		//新規プラグインを追加
	IDC_PLUGIN_OpenFolder,	HIDC_PLUGIN_OpenFolder,		//フォルダを開く
	IDC_PLUGIN_Remove,		HIDC_PLUGIN_Remove,			//プラグインを削除
	IDC_PLUGIN_OPTION,		HIDC_PLUGIN_OPTION,			//プラグイン設定	// 2010/3/22 Uchi
	IDC_PLUGIN_README,		HIDC_PLUGIN_README,			//ReadMe表示		// 2011/11/2 Uchi
	IDC_PLUGIN_URL,			HIDC_PLUGIN_URL,			//配布先			// 2015/01/02 syat
	//	IDC_STATIC,			-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropPlugin::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropPlugin::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/*! Pluginページのメッセージ処理
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CPropPlugin::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	int			idCtrl;

	WORD		wNotifyCode;
	WORD		wID;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* ダイアログデータの設定 Plugin */
		InitDialog( hwndDlg );
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch( idCtrl ){
		case IDC_PLUGINLIST:
			switch( pNMHDR->code ){
			case LVN_ITEMCHANGED:
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 ){
						CPlugin* plugin = CPluginManager::getInstance()->GetPlugin(sel);
						if( plugin != NULL ){
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), to_tchar(plugin->m_sDescription.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), to_tchar(plugin->m_sAuthor.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), to_tchar(plugin->m_sVersion.c_str()) );
						}else{
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), _T("") );
						}
						// 2010.08.21 明らかに使えないときはDisableにする
						EPluginState state = m_Common.m_sPlugin.m_PluginTable[sel].m_state;
						BOOL bEdit = (state != PLS_DELETED && state != PLS_NONE);
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove ), bEdit );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ), state == PLS_LOADED && plugin && plugin->m_options.size() > 0 );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_README ), 
							(state == PLS_INSTALLED || state == PLS_UPDATED || state == PLS_LOADED || state == PLS_DELETED)
							&& !GetReadMeFile(to_tchar(m_Common.m_sPlugin.m_PluginTable[sel].m_szName)).empty());
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_PLUGIN_URL), state == PLS_LOADED && plugin && plugin->m_sUrl.size() > 0);
					}
				}
				break;
			case NM_DBLCLK:
				// リストビューへのダブルクリックで「プラグイン設定」を呼び出す
				if (::IsWindowEnabled(::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION )))
				{
					DispatchEvent( hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_PLUGIN_OPTION, BN_CLICKED), (LPARAM)::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ) );
				}
				break;
			}
			break;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_PLUGIN );
				return TRUE;
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 Plugin */
				GetData( hwndDlg );
				return TRUE;
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_PLUGIN;
				return TRUE;
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);			/* 項目ID､ コントロールID､ またはアクセラレータID */

		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_PLUGIN_SearchNew:		// 新規プラグインを追加
				GetData( hwndDlg );
				CPluginManager::getInstance()->SearchNewPlugin( m_Common, hwndDlg );
				if( m_bTrayProc ){
					LoadPluginTemp(m_Common, *m_pcMenuDrawer);
				}
				SetData_LIST( hwndDlg );	//リストの再構築
				break;
			case IDC_PLUGIN_INST_ZIP:		// ZIPプラグインを追加
				{
					static std::tstring	sTrgDir;
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					_tcscpy( szPath, (sTrgDir.empty() ? CPluginManager::getInstance()->GetBaseDir().c_str() : sTrgDir.c_str()));
					// ファイルオープンダイアログの初期化
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.zip"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						GetData( hwndDlg );
						CPluginManager::getInstance()->InstZipPlugin( m_Common, hwndDlg, szPath );
						if( m_bTrayProc ){
							LoadPluginTemp(m_Common, *m_pcMenuDrawer);
						}
						SetData_LIST( hwndDlg );	//リストの再構築
					}
					// フォルダを記憶
					TCHAR	szFolder[_MAX_PATH + 1];
					TCHAR	szFname[_MAX_PATH + 1];
					SplitPath_FolderAndFile(szPath, szFolder, szFname);
					sTrgDir = szFolder;
				}
				break;
			case IDC_CHECK_PluginEnable:	// プラグインを有効にする
				EnablePluginPropInput( hwndDlg );
				break;
			case IDC_PLUGIN_Remove:			// プラグインを削除
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 ){
						
						if( MYMESSAGEBOX( hwndDlg, MB_YESNO, GSTR_APPNAME, LS(STR_PROPCOMPLG_DELETE), m_Common.m_sPlugin.m_PluginTable[sel].m_szName ) == IDYES ){
							CPluginManager::getInstance()->UninstallPlugin( m_Common, sel );
							SetData_LIST( hwndDlg );
						}
					}
				}
				break;
			case IDC_PLUGIN_OPTION:		// プラグイン設定	// 2010/3/22 Uchi
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 && m_Common.m_sPlugin.m_PluginTable[sel].m_state == PLS_LOADED ){
						// 2010.08.21 プラグイン名(フォルダ名)の同一性の確認
						CPlugin* plugin = CPluginManager::getInstance()->GetPlugin(sel);
						wstring sDirName = to_wchar(plugin->GetFolderName().c_str());
						if( plugin && 0 == auto_stricmp(sDirName.c_str(), m_Common.m_sPlugin.m_PluginTable[sel].m_szName ) ){
							CDlgPluginOption cDlgPluginOption;
							cDlgPluginOption.DoModal( ::GetModuleHandle(NULL), hwndDlg, this, sel );
						}else{
							WarningMessage( hwndDlg, LS(STR_PROPCOMPLG_ERR1) );
						}
					}
				}
				break;
			case IDC_PLUGIN_OpenFolder:			// フォルダを開く
				{
					std::tstring sBaseDir = CPluginManager::getInstance()->GetBaseDir() + _T(".");
					if( ! IsDirectory(sBaseDir.c_str()) ){
						if( ::CreateDirectory(sBaseDir.c_str(), NULL) == 0 ){
							break;
						}
					}
					::ShellExecute( NULL, _T("open"), sBaseDir.c_str(), NULL, NULL, SW_SHOW );
				}
				break;
			case IDC_PLUGIN_README:		// ReadMe表示	// 2011/11/2 Uchi
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					std::tstring sName = to_tchar(m_Common.m_sPlugin.m_PluginTable[sel].m_szName);	// 個別フォルダ名
					std::tstring sReadMeName = GetReadMeFile(sName);
					if (!sReadMeName.empty()) {
						if (!BrowseReadMe(sReadMeName)) {
							WarningMessage( hwndDlg, LS(STR_PROPCOMPLG_ERR2) );
						}
					}else{
						WarningMessage( hwndDlg, LS(STR_PROPCOMPLG_ERR3) );
					}
				}
				break;
			case IDC_PLUGIN_URL:
				{
					HWND hListView = ::GetDlgItem(hwndDlg, IDC_PLUGINLIST);
					int sel = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
					if (sel >= 0){
						CPlugin* plugin = CPluginManager::getInstance()->GetPlugin(sel);
						if (plugin != NULL){
							::ShellExecute(NULL, _T("Open"), to_tchar(plugin->m_sUrl.c_str()), NULL, NULL, SW_SHOW);
						}
					}
				}
				break;
			}
			break;
		case CBN_DROPDOWN:
			//switch( wID ){
			//default:
			//	break;
			//}
			break;	/* CBN_DROPDOWN */
		case EN_KILLFOCUS:
			//switch( wID ){
			//default:
			//	break;
			//}
			break;
		}

		break;	/* WM_COMMAND */
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void CPropPlugin::SetData( HWND hwndDlg )
{
	//プラグインを有効にする
	::CheckDlgButton( hwndDlg, IDC_CHECK_PluginEnable, m_Common.m_sPlugin.m_bEnablePlugin );

	//プラグインリスト
	SetData_LIST( hwndDlg );
	
	EnablePluginPropInput( hwndDlg );
	return;
}

/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void CPropPlugin::SetData_LIST( HWND hwndDlg )
{
	int index;
	LVITEM sItem;
	PluginRec* plugin_table = m_Common.m_sPlugin.m_PluginTable;

	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_README ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_URL ), FALSE );

	//プラグインリスト
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );

	ListView_DeleteAllItems( hListView );

	for( index = 0; index < MAX_PLUGIN; ++index ){
		std::basic_string<TCHAR> sDirName;	//CPlugin.GetDirName()の結果保持変数
		CPlugin* plugin = CPluginManager::getInstance()->GetPlugin( index );

		//番号
		TCHAR buf[4];
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		sItem.iItem = index;
		sItem.iSubItem = 0;
		_itot( index, buf, 10 );
		sItem.pszText = buf;
		sItem.lParam = index;
		ListView_InsertItem( hListView, &sItem );

		//名前
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		if( plugin ){
			sItem.pszText = const_cast<LPTSTR>( to_tchar(plugin->m_sName.c_str()) );
		}else{
			sItem.pszText = const_cast<TCHAR*>(_T("-"));
		}
		ListView_SetItem( hListView, &sItem );

		//状態
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		switch( plugin_table[index].m_state ){
		case PLS_INSTALLED: sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE1)); break;
		case PLS_UPDATED:   sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE2)); break;
		case PLS_STOPPED:   sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE3)); break;
		case PLS_LOADED:    sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE4)); break;
		case PLS_DELETED:   sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE5)); break;
		case PLS_NONE:      sItem.pszText = const_cast<TCHAR*>(_T("")); break;
		default:            sItem.pszText = const_cast<TCHAR*>(LS(STR_PROPCOMPLG_STATE6)); break;
		}
		ListView_SetItem( hListView, &sItem );
		
		//読込
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 3;
		if( plugin_table[index].m_state != PLS_NONE ){
			sItem.pszText = const_cast<TCHAR*>(plugin ? LS(STR_PROPCOMPLG_LOAD) : _T(""));
		}else{
			sItem.pszText = const_cast<TCHAR*>(_T(""));
		}
		ListView_SetItem( hListView, &sItem );

		//フォルダ
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		switch( plugin_table[index].m_state ){
		case PLS_INSTALLED:
		case PLS_UPDATED:
		case PLS_STOPPED:
		case PLS_LOADED:
			if( plugin ){
				sDirName = plugin->GetFolderName();
				sItem.pszText = const_cast<LPTSTR>( sDirName.c_str() );
			}else{
				sItem.pszText = const_cast<LPTSTR>( to_tchar(plugin_table[index].m_szName) );
			}
			break;
		default:
			sItem.pszText = const_cast<TCHAR*>(_T(""));
		}
		ListView_SetItem( hListView, &sItem );
	}
	
	//	リストビューの行選択を可能にする．
	//	IE 3.x以降が入っている場合のみ動作する．
	//	これが無くても，番号部分しか選択できないだけで操作自体は可能．
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle( hListView );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hListView, dwStyle );

	return;
}

/*!
	ダイアログ上のコントロールからデータを取得してメモリに格納する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
int CPropPlugin::GetData( HWND hwndDlg )
{
	//プラグインを有効にする
	m_Common.m_sPlugin.m_bEnablePlugin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PluginEnable );

	//プラグインリストは今のところ変更できる部分がない
	//「新規プラグイン追加」はm_Commonに直接書き込むので、この関数ですることはない

	return TRUE;
}

struct ColumnData_CPropPlugin_Init {
	int titleId;
	int width;
};

/*!
	ダイアログ上のコントロールを初期化する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void CPropPlugin::InitDialog( HWND hwndDlg )
{
	const struct ColumnData_CPropPlugin_Init ColumnList[] = {
		{ STR_PROPCOMPLG_LIST1, 40 },
		{ STR_PROPCOMPLG_LIST2, 200 },
		{ STR_PROPCOMPLG_LIST3, 40 },
		{ STR_PROPCOMPLG_LIST4, 40 },
		{ STR_PROPCOMPLG_LIST5, 150 },
	};

	//	ListViewの初期化
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );

	LVCOLUMN sColumn;
	int pos;
	RECT rc;
	::GetWindowRect( hListView, &rc );
	int width = rc.right - rc.left;
	
	for( pos = 0; pos < _countof( ColumnList ); ++pos ){
		
		memset_raw( &sColumn, 0, sizeof( sColumn ));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = const_cast<TCHAR*>(LS(ColumnList[pos].titleId));
		sColumn.cx = ColumnList[pos].width * width / 499;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if( ListView_InsertColumn( hListView, pos, &sColumn ) < 0 ){
			PleaseReportToAuthor( hwndDlg, _T("PropComMacro::InitDlg::ColumnRegistrationFail") );
			return;	//	よくわからんけど失敗した
		}
	}

}

/*! 「プラグイン」シート上のアイテムの有効・無効を適切に設定する

	@date 2009.12.06 syat 新規作成
	@date 2010.08.21 Moca プラグイン無効状態でも削除操作などを可能にする
*/
void CPropPlugin::EnablePluginPropInput(HWND hwndDlg)
{
	if( !::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PluginEnable ) )
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_INST_ZIP          ), FALSE );
	}
	else
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), TRUE );
		CZipFile	cZipFile;
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_INST_ZIP          ), cZipFile.IsOk() );
	}
}

//	Readme ファイルの取得	2011/11/2 Uchi
std::tstring CPropPlugin::GetReadMeFile(const std::tstring& sName)
{
	std::tstring sReadMeName = CPluginManager::getInstance()->GetBaseDir()
		+ sName + _T("\\ReadMe.txt");
	CFile* fl = new CFile(sReadMeName.c_str());
	if (!fl->IsFileExist()) {
		sReadMeName = CPluginManager::getInstance()->GetBaseDir()
			+ sName + _T("\\") + sName + _T(".txt");
		delete fl;
		fl = new CFile(sReadMeName.c_str());
	}
	if (!fl->IsFileExist()) {
		// exeフォルダ配下
		sReadMeName = CPluginManager::getInstance()->GetExePluginDir()
			+ sName + _T("\\ReadMe.txt");
		delete fl;
		fl = new CFile(sReadMeName.c_str());
		if (!fl->IsFileExist()) {
			sReadMeName = CPluginManager::getInstance()->GetExePluginDir()
				+ sName + _T("\\") + sName + _T(".txt");
			delete fl;
			fl = new CFile(sReadMeName.c_str());
		}
	}

	if (!fl->IsFileExist()) {
		sReadMeName = _T("");
	}
	delete fl;
	return sReadMeName;
}

/*!
	Readme ファイルの表示

	@date 2011/11/2 Uchi

	@return true: 成功, false: 失敗
*/
bool CPropPlugin::BrowseReadMe(const std::tstring& sReadMeName)
{
	// -- -- -- -- コマンドライン文字列を生成 -- -- -- -- //
	CCommandLineString cCmdLineBuf;

	//アプリケーションパス
	TCHAR szExePath[MAX_PATH + 1];
	::GetModuleFileName( NULL, szExePath, _countof( szExePath ) );
	cCmdLineBuf.AppendF( _T("\"%ts\""), szExePath );

	// ファイル名
	cCmdLineBuf.AppendF( _T(" \"%ts\""), sReadMeName.c_str() );

	// コマンドラインオプション
	cCmdLineBuf.AppendF(_T(" -R -CODE=99"));

	// グループID
	int nGroup = GetDllShareData().m_sNodes.m_nGroupSequences;
	if( nGroup > 0 ){
		cCmdLineBuf.AppendF( _T(" -GROUP=%d"), nGroup+1 );
	}

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	::GetStartupInfo(&sui);

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	TCHAR	szCmdLine[1024];
	auto_strcpy_s(szCmdLine, _countof(szCmdLine), cCmdLineBuf.c_str());
	//リソースリーク対策
	BOOL bRet = ::CreateProcess( NULL, szCmdLine, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi );

	//プロセス作成に成功した場合
	if ( bRet )
	{
		//スレッドハンドルは使わないので閉じておく
		::CloseHandle( pi.hThread );
		//プロセスハンドルは使わないので閉じておく
		::CloseHandle( pi.hProcess );
	}

	return ( bRet != FALSE );
}

static void LoadPluginTemp(CommonSetting& common, CMenuDrawer& cMenuDrawer)
{
	{
		// 2013.05.31 コントロールプロセスなら即時読み込み
		CPluginManager::getInstance()->LoadAllPlugin( &common );
		// ツールバーアイコンの更新
		const CPlug::Array& plugs = CJackManager::getInstance()->GetPlugs( PP_COMMAND );
		cMenuDrawer.m_pcIcons->ResetExtend();
		for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ) {
			int iBitmap = CMenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
			const CPlug* plug = *it;
			if( !plug->m_sIcon.empty() ){
				iBitmap = cMenuDrawer.m_pcIcons->Add(
					to_tchar(plug->m_cPlugin.GetFilePath( to_tchar(plug->m_sIcon.c_str()) ).c_str()) );
			}
			cMenuDrawer.AddToolButton( iBitmap, plug->GetFunctionCode() );
		}
	}
}
