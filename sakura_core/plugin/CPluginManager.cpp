/*!	@file
	@brief プラグイン管理クラス

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
#include "stdafx.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "plugin/CWSHPlugin.h"
#include "plugin/CDllPlugin.h"
#include "util/module.h"

//コンストラクタ
CPluginManager::CPluginManager()
{

	//pluginsフォルダの場所を取得
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir( szPluginPath, _T("plugins\\") );	//iniと同じ階層のpluginsフォルダを検索
	m_sBaseDir.append(szPluginPath);
}

//全プラグインを解放する
void CPluginManager::UnloadAllPlugin()
{
	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		delete *it;
	}
}

//新規プラグインを追加する
bool CPluginManager::SearchNewPlugin( CommonSetting& common, HWND hWndOwner )
{
#if _DEBUG & _UNICODE
	DebugOut(L"Enter ControlProcessInit\n");
#endif

	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	HANDLE hFind;


	//プラグインフォルダの配下を検索
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//プラグインフォルダが存在しない
		InfoMessage( hWndOwner, _T("%s"), _T("プラグインフォルダがありません"));
		return true;
	}
	bool bFindNewDir = false;
	do {
		if( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			_tcscmp(wf.cFileName, _T("."))!=0 && _tcscmp(wf.cFileName, _T(".."))!=0 )
		{
			//インストール済みチェック。フォルダ名＝プラグインテーブルの名前ならインストールしない
			bool isNotInstalled = true;
			for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( _tcscmp( wf.cFileName, to_tchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if( !isNotInstalled ){ continue; }
			bFindNewDir = true;

			TCHAR msg[512];
			auto_snprintf_s( msg, _countof(msg), _T("プラグイン「%ts」をインストールしますか？"), wf.cFileName );
			if( ::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) == IDYES ){
				int pluginNo = InstallPlugin( common, wf.cFileName, hWndOwner );
				if( pluginNo < 0 ){
					auto_snprintf_s( msg, _countof(msg), _T("プラグイン「%ts」をインストールできませんでした\n理由：%ls"), wf.cFileName, m_sInstallError.c_str() );
					::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_OK | MB_ICONERROR );
				}
			}
		}
	} while( FindNextFile( hFind, &wf ));
	
	if(!bFindNewDir){
		InfoMessage( hWndOwner, _T("%s"), _T("新しいプラグインは見つかりませんでした"));
	}

	FindClose( hFind );
	return true;
}

//プラグインの初期導入をする
int CPluginManager::InstallPlugin( CommonSetting& common, TCHAR* pszPluginName, HWND hWndOwner )
{
	CDataProfile cProfDef;				//プラグイン定義ファイル

	//プラグイン定義ファイルを読み込む
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( (m_sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() ) ){
		m_sInstallError = L"プラグイン定義ファイル（plugin.def）がありません";
		return -1;
	}

	std::wstring sId;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_ID, sId );
	if( sId.length() == 0 ){
		m_sInstallError = L"Plugin.IDがありません";
		return -1;
	}

	//ID重複・テーブル空きチェック
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( nEmpty == -1 && plugin_table[iNo].m_state == PLS_NONE ){
			nEmpty = iNo;
		}
		if( wcscmp( sId.c_str(), plugin_table[iNo].m_szId ) == 0 ){	//ID一致
			TCHAR msg[512];
			auto_snprintf_s( msg, _countof(msg), _T("同じプラグインが別の名前でインストールされています。上書きしますか？\n　はい　→　新しい「%ts」を使用\n　いいえ→　インストール済みの「%ls」を使用"), pszPluginName, plugin_table[iNo].m_szName );
			if( ::MessageBox( hWndOwner, msg, GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) == IDYES ){
				nEmpty = iNo;
				isDuplicate = true;
				break;
			}else{
				m_sInstallError = L"ユーザキャンセル";
				return -1;
			}
		}
	}

	if( nEmpty == -1 ){
		m_sInstallError = L"プラグインをこれ以上登録できません";
		return -1;
	}

	wcsncpy( plugin_table[nEmpty].m_szName, to_wchar(pszPluginName), MAX_PLUGIN_NAME );
	plugin_table[nEmpty].m_szName[ MAX_PLUGIN_NAME-1 ] = '\0';
	wcsncpy( plugin_table[nEmpty].m_szId, sId.c_str(), MAX_PLUGIN_ID );
	plugin_table[nEmpty].m_szId[ MAX_PLUGIN_ID-1 ] = '\0';
	plugin_table[nEmpty].m_state = isDuplicate ? PLS_UPDATED : PLS_INSTALLED;

	return nEmpty;
}

//全プラグインを読み込む
bool CPluginManager::LoadAllPlugin()
{
#if _DEBUG & _UNICODE
	DebugOut(L"Enter LoadAllPlugin\n");
#endif

	if( ! GetDllShareData().m_Common.m_sPlugin.m_bEnablePlugin ) return true;

	//プラグインテーブルに登録されたプラグインを読み込む
	PluginRec* plugin_table = GetDllShareData().m_Common.m_sPlugin.m_PluginTable;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( plugin_table[iNo].m_szName[0] == '\0' ) continue;

		CPlugin* plugin = LoadPlugin( m_sBaseDir.c_str(), const_cast<TCHAR*>(to_tchar(plugin_table[iNo].m_szName)) );
		if( plugin ){
			plugin->m_id = iNo;		//プラグインテーブルの行番号をIDとする
			m_plugins.push_back( plugin );
			plugin_table[iNo].m_state = PLS_LOADED;
		}
	}

	m_plugins.sort();

	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		RegisterPlugin( *it );
	}
	
	return true;
}

//プラグインを読み込む
CPlugin* CPluginManager::LoadPlugin( const TCHAR* pszPluginDir, TCHAR* pszPluginName )
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	CDataProfile cProfDef;				//プラグイン定義ファイル
	CDataProfile cProfOption;			//オプションファイル
	CPlugin* plugin = NULL;

#if _DEBUG & _UNICODE
	DebugOut(L"Load Plugin %ts\n",  pszPluginName );
#endif
	//プラグイン定義ファイルを読み込む
	Concat_FolderAndFile( pszPluginDir, pszPluginName, pszBasePath );
	Concat_FolderAndFile( pszBasePath, PII_FILENAME, pszPath );
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( pszPath ) ){
		//プラグイン定義ファイルが存在しない
		return NULL;
	}
#if _DEBUG & _UNICODE
	DebugOut(L"  定義ファイル読込 %ts\n",  pszPath );
#endif

	WCHAR szPlugType[128];
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_PLUGTYPE, MakeStringBufferW(szPlugType) );

	if( wcsicmp( szPlugType, L"wsh" ) == 0 ){
		plugin = new CWSHPlugin( tstring(pszBasePath) );
	}else if( wcsicmp( szPlugType, L"dll" ) == 0 ){
		plugin = new CDllPlugin( tstring(pszBasePath) );
	}else{
		return NULL;
	}
	plugin->ReadPluginDef( &cProfDef );
#if _DEBUG & _UNICODE
	DebugOut(L"  プラグインタイプ %ls\n", szPlugType);
#endif

	//オプションファイルを読み込む
	_tcscpy( pszPath, pszBasePath );
	_tcscat( pszPath, PII_OPTFILEEXT );
	cProfOption.SetReadingMode();
	if( cProfOption.ReadProfile( pszPath ) ){
		//オプションファイルが存在する場合、読み込む
		plugin->ReadPluginOption( &cProfOption );
	}
#if _DEBUG & _UNICODE
	DebugOut(L"  オプションファイル読込 %ts\n",  pszPath );
#endif

	return plugin;
}

//プラグインをCJackManagerに登録する
bool CPluginManager::RegisterPlugin( CPlugin* plugin )
{
	CJackManager* pJackMgr = CJackManager::Instance();
	CPlug::Array plugs = plugin->GetPlugs();

	for( CPlug::ArrayIter plug = plugs.begin() ; plug != plugs.end(); plug++ ){
		pJackMgr->RegisterPlug( (*plug)->m_sJack.c_str(), *plug );
	}

	return true;
}

//プラグインを取得する
CPlugin* CPluginManager::GetPlugin( int id )
{
	for( CPlugin::ListIter plugin = m_plugins.begin() ; plugin != m_plugins.end(); plugin++ ){
		if( (*plugin)->m_id == id ) return *plugin;
	}
	return NULL;
}

//プラグインを削除する
void CPluginManager::UninstallPlugin( CommonSetting& common, int id )
{
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;

	plugin_table[id].m_szId[0] = '\0';
	plugin_table[id].m_szName[0] = '\0';
	plugin_table[id].m_state = PLS_DELETED;
}
