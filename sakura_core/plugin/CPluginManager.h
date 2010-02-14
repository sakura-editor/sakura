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
	// 型定義
private:
	typedef std::basic_string<TCHAR> tstring;
public:
	friend class TSingleton<CPluginManager>;

	// コンストラクタ
protected:
	CPluginManager()
	{
		m_pShareData = &GetDllShareData();
	}

	// デストラクタ
	// Singletonのため呼ばれない。

	// 操作
public:
	bool LoadAllPlugin();				//全プラグインを読み込む
	void UnloadAllPlugin();				//全プラグインを解放する
	bool SearchNewPlugin( CommonSetting& common );		//新規プラグインを追加する
	bool InstallPlugin( CommonSetting& common, TCHAR* pszPluginDir, TCHAR* pszPluginName );	//プラグインの初期導入をする
	CPlugin* GetPlugin( int id );		//プラグインを取得する
	void UninstallPlugin( CommonSetting& common, int id );		//プラグインを削除する

private:
	CPlugin* LoadPlugin( TCHAR* pszPluginDir, TCHAR* pszPluginName );	//プラグインを読み込む
	bool RegisterPlugin( CPlugin* plugin );	//プラグインをCJackManagerに登録する

	// メンバ変数
private:
	DLLSHAREDATA* m_pShareData;
	CPlugin::List m_plugins;
	wstring m_sInstallError;			//InstallPluginのエラー理由

};

#endif /* SAKURA_CPLUGINMANAGER_1015EF83_3B11_47FB_BAAF_63ACAFE32FCAD_H_ */
/*[EOF]*/
