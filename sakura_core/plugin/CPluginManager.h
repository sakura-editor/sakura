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
#ifndef SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_
#define SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_

#include "plugin/CPlugin.h"
#include <list>
#include <string>

class CPluginManager : public TSingleton<CPluginManager>{
	friend class TSingleton<CPluginManager>;
	CPluginManager();

	// 型定義
private:
	typedef std::wstring wstring;
	typedef std::string string;

	// 操作
public:
	bool LoadAllPlugin(CommonSetting* common = NULL);				//全プラグインを読み込む
	void UnloadAllPlugin();				//全プラグインを解放する
	bool SearchNewPlugin( CommonSetting& common, HWND hWndOwner );		//新規プラグインを導入する
	int InstallPlugin( CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, wstring& errorMsg, bool bUpdate = false );	//プラグインの初期導入をする
	bool InstZipPlugin( CommonSetting& common, HWND hWndOwner, const tstring& sZipName, bool bInSearch=false );		//Zipプラグインを追加する
	CPlugin* GetPlugin( int id );		//プラグインを取得する
	void UninstallPlugin( CommonSetting& common, int id );		//プラグインを削除する

private:
	CPlugin* LoadPlugin( const TCHAR* pszPluginDir, const TCHAR* pszPluginName, const TCHAR* pszLangName );	//プラグインを読み込む
	bool RegisterPlugin( CPlugin* plugin );	//プラグインをCJackManagerに登録する
	bool UnRegisterPlugin( CPlugin* plugin );	//プラグインのCJackManagerの登録を解除する

	//属性
public:
	//pluginsフォルダのパス
	const tstring GetBaseDir() { return m_sBaseDir; }
	const tstring GetExePluginDir() { return m_sExePluginDir; }
	bool SearchNewPluginDir( CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel );		//新規プラグインを追加する(下請け)
	bool SearchNewPluginZip( CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel );		//新規プラグインを追加する(下請け)Zip File
	bool InstZipPluginSub( CommonSetting& common, HWND hWndOwner, const tstring& sZipName, const tstring& sDispName, bool bInSearch, bool& bCancel );		//Zipプラグインを導入する(下請け)

	// メンバ変数
private:
	CPlugin::List m_plugins;
	tstring m_sBaseDir;					//pluginsフォルダのパス
	tstring m_sExePluginDir;			//Exeフォルダ配下pluginsフォルダのパス

};

#endif /* SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_ */
/*[EOF]*/
