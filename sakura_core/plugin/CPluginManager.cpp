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
#include "StdAfx.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "plugin/CWSHPlugin.h"
#include "plugin/CDllPlugin.h"
#include "util/module.h"
#include "io/CZipFile.h"

//コンストラクタ
CPluginManager::CPluginManager()
{

	//pluginsフォルダの場所を取得
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir( szPluginPath, _T("plugins\\") );	//iniと同じ階層のpluginsフォルダを検索
	m_sBaseDir.append(szPluginPath);

	//Exeフォルダ配下pluginsフォルダのパスを取得
	TCHAR	szPath[_MAX_PATH];
	TCHAR	szFolder[_MAX_PATH];
	TCHAR	szFname[_MAX_PATH];

	::GetModuleFileName( NULL, szPath, _countof(szPath)	);
	SplitPath_FolderAndFile(szPath, szFolder, szFname);
	Concat_FolderAndFile(szFolder, _T("plugins\\"), szPluginPath);

	m_sExePluginDir.append(szPluginPath);
}

//全プラグインを解放する
void CPluginManager::UnloadAllPlugin()
{
	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		UnRegisterPlugin( *it );
	}

	for( CPlugin::ListIter it = m_plugins.begin(); it != m_plugins.end(); it++ ){
		delete *it;
	}
	
	// 2010.08.04 Moca m_plugins.claerする
	m_plugins.clear();
}

//新規プラグインを追加する
bool CPluginManager::SearchNewPlugin( CommonSetting& common, HWND hWndOwner )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPlugin\n"));
#endif

	HANDLE hFind;
	CZipFile	cZipFile;


	//プラグインフォルダの配下を検索
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//プラグインフォルダが存在しない
		if (!CreateDirectory(m_sBaseDir.c_str(), NULL)) {
			InfoMessage( hWndOwner, _T("%ts"), LS(STR_PLGMGR_FOLDER));
			return true;
		}
	}
	::FindClose(hFind);

	bool	bCancel = false;
	//プラグインフォルダの配下を検索
	bool bFindNewDir = SearchNewPluginDir(common, hWndOwner, m_sBaseDir, bCancel);
	if (!bCancel && m_sBaseDir != m_sExePluginDir) {
		bFindNewDir |= SearchNewPluginDir(common, hWndOwner, m_sExePluginDir, bCancel);
	}
	if (!bCancel && cZipFile.IsOk()) {
		bFindNewDir |= SearchNewPluginZip(common, hWndOwner, m_sBaseDir, bCancel);
		if (!bCancel && m_sBaseDir != m_sExePluginDir) {
			bFindNewDir |= SearchNewPluginZip(common, hWndOwner, m_sExePluginDir, bCancel);
		}
	}

	if (bCancel) {
		InfoMessage( hWndOwner, _T("%ts"), LS(STR_PLGMGR_CANCEL));
	}
	else if (!bFindNewDir) {
		InfoMessage( hWndOwner, _T("%ts"), LS(STR_PLGMGR_NEWPLUGIN));
	}

	return true;
}


//新規プラグインを追加する(下請け)
bool CPluginManager::SearchNewPluginDir( CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginDir\n"));
#endif

	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	HANDLE hFind;

	WIN32_FIND_DATA wf;
	hFind = FindFirstFile( (sSearchDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		//プラグインフォルダが存在しない
		return false;
	}
	bool bFindNewDir = false;
	do {
		if( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			(wf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 &&
			_tcscmp(wf.cFileName, _T("."))!=0 && _tcscmp(wf.cFileName, _T(".."))!=0 &&
			auto_stricmp(wf.cFileName, _T("unuse")) !=0 )
		{
			//インストール済みチェック。フォルダ名＝プラグインテーブルの名前ならインストールしない
			// 2010.08.04 大文字小文字同一視にする
			bool isNotInstalled = true;
			for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( auto_stricmp( wf.cFileName, to_tchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if( !isNotInstalled ){ continue; }

			// 2011.08.20 syat plugin.defが存在しないフォルダは飛ばす
			if( ! IsFileExists( (sSearchDir + wf.cFileName + _T("\\") + PII_FILENAME).c_str(), true ) ){
				continue;
			}

			bFindNewDir = true;
			int nRes = Select3Message( hWndOwner, LS(STR_PLGMGR_INSTALL), wf.cFileName );
			if (nRes == IDYES) {
				std::wstring errMsg;
				int pluginNo = InstallPlugin( common, wf.cFileName, hWndOwner, errMsg );
				if( pluginNo < 0 ){
					WarningMessage( hWndOwner, LS(STR_PLGMGR_INSTALL_ERR),
						wf.cFileName, errMsg.c_str() );
				}
			}
			else if (nRes == IDCANCEL) {
				bCancel = true;
				break;	// for loop
			}
		}
	} while( FindNextFile( hFind, &wf ));

	FindClose( hFind );
	return bFindNewDir;
}


//新規プラグインを追加する(下請け)Zip File
bool CPluginManager::SearchNewPluginZip( CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginZip\n"));
#endif

	HANDLE hFind;

	WIN32_FIND_DATA wf;
	bool		bNewPlugin = false;
	bool		bFound;
	CZipFile	cZipFile;

	hFind = INVALID_HANDLE_VALUE;

	// Zip File 検索解凍
	if (cZipFile.IsOk()) {
		hFind = FindFirstFile( (sSearchDir + _T("*.zip")).c_str(), &wf );

		for (bFound = (hFind != INVALID_HANDLE_VALUE); bFound; bFound = (FindNextFile( hFind, &wf ) != 0)) {
			if( (wf.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) == 0)
			{
				bNewPlugin |= InstZipPluginSub(common, hWndOwner, sSearchDir + wf.cFileName, wf.cFileName, true, bCancel);
				if (bCancel) {
					break;
				}
			}
		}
	}

	FindClose( hFind );
	return bNewPlugin;
}


//Zipプラグインを導入する
bool CPluginManager::InstZipPlugin( CommonSetting& common, HWND hWndOwner, const tstring& sZipFile, bool bInSearch )
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Entry InstZipPlugin\n"));
#endif

	CZipFile		cZipFile;
	TCHAR			msg[512];

	// ZIPファイルが扱えるか
	if (!cZipFile.IsOk()) {
		auto_snprintf_s( msg, _countof(msg), LS(STR_PLGMGR_ERR_ZIP) );
		InfoMessage( hWndOwner, _T("%ts"), msg);
		return false;
	}

	//プラグインフォルダの存在を確認
	WIN32_FIND_DATA wf;
	HANDLE		hFind;
	if ((hFind = ::FindFirstFile( (m_sBaseDir + _T("*")).c_str(), &wf )) == INVALID_HANDLE_VALUE) {
		//プラグインフォルダが存在しない
		if (m_sBaseDir == m_sExePluginDir) {
			InfoMessage( hWndOwner, LS(STR_PLGMGR_ERR_FOLDER));
			::FindClose(hFind);
			return false;
		}
		else {
			if (!CreateDirectory(m_sBaseDir.c_str(), NULL)) {
				WarningMessage( hWndOwner, LS(STR_PLGMGR_ERR_CREATEDIR) );
				::FindClose(hFind);
				return false;
			}
		}
	}
	::FindClose(hFind);

	bool	bCancel;
	return CPluginManager::InstZipPluginSub( common, hWndOwner, sZipFile, sZipFile, false, bCancel );
}

//Zipプラグインを導入する(下請け)
bool CPluginManager::InstZipPluginSub( CommonSetting& common, HWND hWndOwner, const tstring& sZipFile, const tstring& sDispName, bool bInSearch, bool& bCancel )
{
	PluginRec*		plugin_table = common.m_sPlugin.m_PluginTable;
	CZipFile		cZipFile;
	std::tstring	sFolderName;
	TCHAR			msg[512];
	std::wstring	errMsg;
	bool			bOk = true;
	bool			bSkip = false;
	bool			bNewPlugin = false;

	// Plugin フォルダ名の取得,定義ファイルの確認
	if (bOk && !cZipFile.SetZip(sZipFile)) {
		auto_snprintf_s( msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ACCESS), sDispName.c_str() );
		bOk = false;
		bSkip = bInSearch;
	}

	// Plgin フォルダ名の取得,定義ファイルの確認
	if (bOk && !cZipFile.ChkPluginDef(PII_FILENAME, sFolderName)) {
		auto_snprintf_s( msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_DEF), sDispName.c_str() );
		bOk = false;
		bSkip = bInSearch;
	}

	if (!bInSearch) {
		// 単独インストール
		//インストール済みチェック。
		bool	isNotInstalled = true;
		int		iNo;
		if (bOk) {
			for( iNo=0; iNo < MAX_PLUGIN; iNo++ ){
				if( auto_stricmp( to_wchar( sFolderName.c_str()), to_wchar( plugin_table[iNo].m_szName ) ) == 0 ){
					isNotInstalled = false;
					break;
				}
			}
			if (isNotInstalled) {
				bNewPlugin = true;
			}
			else {
				if( ConfirmMessage( hWndOwner, LS(STR_PLGMGR_INST_ZIP_ALREADY),
						sDispName.c_str() ) != IDYES ){
					// Yesで無いなら終了
					return false;
				}
			}
		}
	}
	else {
		// pluginsフォルダ検索中
		// フォルダ チェック。すでに解凍されていたならインストールしない(前段でインストール済み或は可否を確認済み)
		if (bOk && (fexist(to_tchar((m_sBaseDir + to_tchar(sFolderName.c_str())).c_str()))
			|| fexist(to_tchar((m_sExePluginDir + to_tchar(sFolderName.c_str())).c_str()))) ) {
			bOk = false;
			bSkip = true;
		}
		if (bOk) {
			bNewPlugin= true;
			int nRes = Select3Message( hWndOwner, LS(STR_PLGMGR_INST_ZIP_INST),
				sDispName.c_str(), sFolderName.c_str() );
			switch (nRes) {
			case IDCANCEL:
				bCancel = true;
				// through
			case IDNO:
				bOk = false;
				bSkip = true;
				break;
			}
		}
	}

	// Zip解凍
	if (bOk && !cZipFile.Unzip(m_sBaseDir)) {
		auto_snprintf_s( msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_UNZIP), sDispName.c_str() );
		bOk = false;
	}
	if (bOk) {
		int pluginNo = InstallPlugin( common, to_tchar(sFolderName.c_str()), hWndOwner, errMsg, true );
		if( pluginNo < 0 ){
			auto_snprintf_s( msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ERR), sDispName.c_str(), errMsg.c_str() );
			bOk = false;
		}
	}

	if (!bOk && !bSkip) {
		// エラーメッセージ出力
		WarningMessage( hWndOwner, _T("%s"), msg);
	}

	return bNewPlugin;
}

//プラグインの初期導入をする
//	common			共有設定変数
//	pszPluginName	プラグイン名
//	hWndOwner		
//	errorMsg		エラーメッセージを返す
//	bUodate			すでに登録していた場合、確認せず上書きする
int CPluginManager::InstallPlugin( CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, std::wstring& errorMsg, bool bUpdate )
{
	CDataProfile cProfDef;				//プラグイン定義ファイル

	//プラグイン定義ファイルを読み込む
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( (m_sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() )
		&& !cProfDef.ReadProfile( (m_sExePluginDir + pszPluginName + _T("\\") + PII_FILENAME).c_str() ) ){
		errorMsg = LSW(STR_PLGMGR_INST_DEF);
		return -1;
	}

	std::wstring sId;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_ID, sId );
	if( sId.length() == 0 ){
		errorMsg = LSW(STR_PLGMGR_INST_ID);
		return -1;
	}
	//2010.08.04 ID使用不可の文字を確認
	//  後々ファイル名やiniで使うことを考えていくつか拒否する
	static const WCHAR szReservedChars[] = L"/\\,[]*?<>&|;:=\" \t";
	for( int x = 0; x < _countof(szReservedChars); ++x ){
		if( sId.npos != sId.find(szReservedChars[x]) ){
			errorMsg = std::wstring(LSW(STR_PLGMGR_INST_RESERVE1)) + szReservedChars + LSW(STR_PLGMGR_INST_RESERVE2);
			return -1;
		}
	}
	if( WCODE::Is09(sId[0]) ){
		errorMsg = LSW(STR_PLGMGR_INST_IDNUM);
		return -1;
	}

	//ID重複・テーブル空きチェック
	PluginRec* plugin_table = common.m_sPlugin.m_PluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( nEmpty == -1 && plugin_table[iNo].m_state == PLS_NONE ){
			nEmpty = iNo;
			// break してはいけない。後ろで同一IDがあるかも
		}
		if( wcscmp( sId.c_str(), plugin_table[iNo].m_szId ) == 0 ){	//ID一致
			if (!bUpdate) {
				const TCHAR* msg = LS(STR_PLGMGR_INST_NAME);
				// 2010.08.04 削除中のIDは元の位置へ追加(復活させる)
				if( plugin_table[iNo].m_state != PLS_DELETED &&
				  ConfirmMessage( hWndOwner, msg, static_cast<const TCHAR*>(pszPluginName), static_cast<const WCHAR*>(plugin_table[iNo].m_szName) ) != IDYES ){
					errorMsg = LSW(STR_PLGMGR_INST_USERCANCEL);
					return -1;
				}
			}
			nEmpty = iNo;
			isDuplicate = plugin_table[iNo].m_state != PLS_DELETED;
			break;
		}
	}

	if( nEmpty == -1 ){
		errorMsg = LSW(STR_PLGMGR_INST_MAX);
		return -1;
	}

	wcsncpy( plugin_table[nEmpty].m_szName, to_wchar(pszPluginName), MAX_PLUGIN_NAME );
	plugin_table[nEmpty].m_szName[ MAX_PLUGIN_NAME-1 ] = '\0';
	wcsncpy( plugin_table[nEmpty].m_szId, sId.c_str(), MAX_PLUGIN_ID );
	plugin_table[nEmpty].m_szId[ MAX_PLUGIN_ID-1 ] = '\0';
	plugin_table[nEmpty].m_state = isDuplicate ? PLS_UPDATED : PLS_INSTALLED;

	// コマンド数の設定	2010/7/11 Uchi
	int			i;
	WCHAR		szPlugKey[10];
	wstring		sPlugCmd;

	plugin_table[nEmpty].m_nCmdNum = 0;
	for (i = 1; i < MAX_PLUG_CMD; i++) {
		auto_sprintf( szPlugKey, L"C[%d]", i);
		sPlugCmd.clear();
		cProfDef.IOProfileData( PII_COMMAND, szPlugKey, sPlugCmd );
		if (sPlugCmd == L"") {
			break;
		}
		plugin_table[nEmpty].m_nCmdNum = i;
	}

	return nEmpty;
}

//全プラグインを読み込む
bool CPluginManager::LoadAllPlugin(CommonSetting* common)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter LoadAllPlugin\n"));
#endif
	CommonSetting_Plugin& pluginSetting = (common ? common->m_sPlugin : GetDllShareData().m_Common.m_sPlugin);

	if( ! pluginSetting.m_bEnablePlugin ) return true;

	std::tstring szLangName;
	{
		std::tstring szDllName = GetDllShareData().m_Common.m_sWindow.m_szLanguageDll;
		if( szDllName == _T("") ){
			szLangName = _T("ja_JP");
		}else{
			// "sakura_lang_*.dll"
			int nStartPos = 0;
			int nEndPos = szDllName.length();
			if( szDllName.substr( 0, 12 ) == _T("sakura_lang_") ){
				nStartPos = 12;
			}
			if( 4 < szDllName.length() && szDllName.substr( szDllName.length() - 4, 4 ) == _T(".dll") ){
				nEndPos = szDllName.length() - 4;
			}
			szLangName = szDllName.substr( nStartPos, nEndPos - nStartPos );
		}
		DEBUG_TRACE( _T("lang = %ts\n"), szLangName.c_str() );
	}

	//プラグインテーブルに登録されたプラグインを読み込む
	PluginRec* plugin_table = pluginSetting.m_PluginTable;
	for( int iNo=0; iNo < MAX_PLUGIN; iNo++ ){
		if( plugin_table[iNo].m_szName[0] == '\0' ) continue;
		// 2010.08.04 削除状態を見る(今のところ保険)
		if( plugin_table[iNo].m_state == PLS_DELETED ) continue;
		if( NULL != GetPlugin( iNo ) ) continue; // 2013.05.31 読み込み済み
		std::tstring name = to_tchar(plugin_table[iNo].m_szName);
		CPlugin* plugin = LoadPlugin( m_sBaseDir.c_str(), name.c_str(), szLangName.c_str() );
		if( !plugin ){
			plugin = LoadPlugin( m_sExePluginDir.c_str(), name.c_str(), szLangName.c_str() );
		}
		if( plugin ){
			// 要検討：plugin.defのidとsakuraw.iniのidの不一致処理
			assert_warning( 0 == auto_strcmp( plugin_table[iNo].m_szId, plugin->m_sId.c_str() ) );
			plugin->m_id = iNo;		//プラグインテーブルの行番号をIDとする
			m_plugins.push_back( plugin );
			plugin_table[iNo].m_state = PLS_LOADED;
			// コマンド数設定
			plugin_table[iNo].m_nCmdNum = plugin->GetCommandCount();

			RegisterPlugin( plugin );
		}
	}
	
	return true;
}

//プラグインを読み込む
CPlugin* CPluginManager::LoadPlugin( const TCHAR* pszPluginDir, const TCHAR* pszPluginName, const TCHAR* pszLangName )
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	std::tstring strMlang;
	CDataProfile cProfDef;				//プラグイン定義ファイル
	CDataProfile cProfDefMLang;			//プラグイン定義ファイル(L10N)
	CDataProfile* pcProfDefMLang = &cProfDefMLang; 
	CDataProfile cProfOption;			//オプションファイル
	CPlugin* plugin = NULL;

#ifdef _UNICODE
	DEBUG_TRACE(_T("Load Plugin %ts\n"),  pszPluginName );
#endif
	//プラグイン定義ファイルを読み込む
	Concat_FolderAndFile( pszPluginDir, pszPluginName, pszBasePath );
	Concat_FolderAndFile( pszBasePath, PII_FILENAME, pszPath );
	cProfDef.SetReadingMode();
	if( !cProfDef.ReadProfile( pszPath ) ){
		//プラグイン定義ファイルが存在しない
		return NULL;
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  定義ファイル読込 %ts\n"),  pszPath );
#endif

	//L10N定義ファイルを読む
	//プラグイン定義ファイルを読み込む base\pluginname\local\plugin_en_us.def
	strMlang = std::tstring(pszBasePath) + _T("\\") + PII_L10NDIR + _T("\\") + PII_L10NFILEBASE + pszLangName + PII_L10NFILEEXT;
	cProfDefMLang.SetReadingMode();
	if( !cProfDefMLang.ReadProfile( strMlang.c_str() ) ){
		//プラグイン定義ファイルが存在しない
		pcProfDefMLang = NULL;
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N定義ファイル読込 %ts Not Found\n"),  strMlang.c_str() );
#endif
	}else{
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N定義ファイル読込 %ts\n"),  strMlang.c_str() );
#endif
	}

	std::wstring sPlugType;
	cProfDef.IOProfileData( PII_PLUGIN, PII_PLUGIN_PLUGTYPE, sPlugType );

	if( wcsicmp( sPlugType.c_str(), L"wsh" ) == 0 ){
		plugin = new CWSHPlugin( tstring(pszBasePath) );
	}else if( wcsicmp( sPlugType.c_str(), L"dll" ) == 0 ){
		plugin = new CDllPlugin( tstring(pszBasePath) );
	}else{
		return NULL;
	}
	plugin->m_sOptionDir = m_sBaseDir + pszPluginName;
	plugin->m_sLangName = pszLangName;
	plugin->ReadPluginDef( &cProfDef, pcProfDefMLang );
#ifdef _UNICODE
	DEBUG_TRACE(_T("  プラグインタイプ %ls\n"), sPlugType.c_str() );
#endif

	//オプションファイルを読み込む
	cProfOption.SetReadingMode();
	if( cProfOption.ReadProfile( plugin->GetOptionPath().c_str() ) ){
		//オプションファイルが存在する場合、読み込む
		plugin->ReadPluginOption( &cProfOption );
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  オプションファイル読込 %ts\n"),  plugin->GetOptionPath().c_str() );
#endif

	return plugin;
}

//プラグインをCJackManagerに登録する
bool CPluginManager::RegisterPlugin( CPlugin* plugin )
{
	CJackManager* pJackMgr = CJackManager::getInstance();
	CPlug::Array plugs = plugin->GetPlugs();

	for( CPlug::ArrayIter plug = plugs.begin() ; plug != plugs.end(); plug++ ){
		pJackMgr->RegisterPlug( (*plug)->m_sJack.c_str(), *plug );
	}

	return true;
}

//プラグインのCJackManagerの登録を解除する
bool CPluginManager::UnRegisterPlugin( CPlugin* plugin )
{
	CJackManager* pJackMgr = CJackManager::getInstance();
	CPlug::Array plugs = plugin->GetPlugs();

	for( CPlug::ArrayIter plug = plugs.begin() ; plug != plugs.end(); plug++ ){
		pJackMgr->UnRegisterPlug( (*plug)->m_sJack.c_str(), *plug );
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

	// 2010.08.04 ここではIDを保持する。後で再度追加するときに同じ位置に追加
	// PLS_DELETEDのm_szId/m_szNameはiniを保存すると削除されます
//	plugin_table[id].m_szId[0] = '\0';
	plugin_table[id].m_szName[0] = '\0';
	plugin_table[id].m_state = PLS_DELETED;
	plugin_table[id].m_nCmdNum = 0;
}
