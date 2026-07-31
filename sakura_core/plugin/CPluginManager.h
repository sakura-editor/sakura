/*!	@file
	@brief プラグイン管理クラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CPLUGINMANAGER_CE705DAD_1876_4B21_9052_07A9BFD292DE_H_
#define SAKURA_CPLUGINMANAGER_CE705DAD_1876_4B21_9052_07A9BFD292DE_H_
#pragma once

#include <list>
#include <string>

#include "env/CSakuraEnvironment.h"	//env::ShareDataClient
#include "plugin/CPlugin.h"
#include "util/design_template.h"

class CPluginManager final : public TSakuraSingleton<CPluginManager>, private env::ShareDataClient {
public:
	CPluginManager();

	// 操作
	bool LoadAllPlugin(CommonSetting* common = nullptr);				//全プラグインを読み込む
	void UnloadAllPlugin();				//全プラグインを解放する
	bool SearchNewPlugin( CommonSetting& common, HWND hWndOwner );		//新規プラグインを導入する
	int InstallPlugin( CommonSetting& common, const WCHAR* pszPluginName, HWND hWndOwner, std::wstring& errorMsg, bool bUpdate = false );	//プラグインの初期導入をする
	bool InstZipPlugin( CommonSetting& common, HWND hWndOwner, const std::wstring& sZipName, bool bInSearch=false );		//Zipプラグインを追加する
	CPlugin* GetPlugin( int id );		//プラグインを取得する
	void UninstallPlugin( CommonSetting& common, int id );		//プラグインを削除する

private:
	CPlugin* LoadPlugin( const WCHAR* pszPluginDir, const WCHAR* pszPluginName, const WCHAR* pszLangName );	//プラグインを読み込む
	bool RegisterPlugin( CPlugin* plugin );	//プラグインをCJackManagerに登録する
	bool UnRegisterPlugin( CPlugin* plugin );	//プラグインのCJackManagerの登録を解除する

	//属性
public:
	//pluginsフォルダーのパス
	const std::wstring GetBaseDir() { return m_sBaseDir; }
	const std::wstring GetExePluginDir() { return m_sExePluginDir; }
	bool SearchNewPluginDir( CommonSetting& common, HWND hWndOwner, const std::wstring& sSearchDir, bool& bCancel );		//新規プラグインを追加する(下請け)
	bool SearchNewPluginZip( CommonSetting& common, HWND hWndOwner, const std::wstring& sSearchDir, bool& bCancel );		//新規プラグインを追加する(下請け)Zip File
	bool InstZipPluginSub( CommonSetting& common, HWND hWndOwner, const std::wstring& sZipName, const std::wstring& sDispName, bool bInSearch, bool& bCancel );		//Zipプラグインを導入する(下請け)

	// メンバ変数
private:
	CPlugin::List m_plugins;
	std::wstring m_sBaseDir;			//pluginsフォルダーのパス
	std::wstring m_sExePluginDir;		//Exeフォルダー配下pluginsフォルダーのパス
};

#endif /* SAKURA_CPLUGINMANAGER_CE705DAD_1876_4B21_9052_07A9BFD292DE_H_ */
