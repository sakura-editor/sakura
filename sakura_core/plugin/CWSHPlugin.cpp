/*!	@file
	@brief WSHプラグインクラス

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
#include "plugin/CWSHPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CWSHManager.h"

// デストラクタ
CWSHPlugin::~CWSHPlugin(void)
{
	for( CPlug::ArrayIter it = m_plugs.begin(); it != m_plugs.end(); it++ ){
		delete *it;
	}
}

//プラグイン定義ファイルを読み込む
bool CWSHPlugin::ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang )
{
	ReadPluginDefCommon( cProfile, cProfileMlang );

	//WSHセクションの読み込み
	cProfile->IOProfileData<bool>( PII_WSH, PII_WSH_USECACHE, m_bUseCache );

	//プラグの読み込み
	ReadPluginDefPlug( cProfile, cProfileMlang );

	//コマンドの読み込み
	ReadPluginDefCommand( cProfile, cProfileMlang );

	//オプション定義の読み込み	// 2010/3/24 Uchi
	ReadPluginDefOption( cProfile, cProfileMlang );

	//文字列定義の読み込み
	ReadPluginDefString( cProfile, cProfileMlang );

	return true;
}

//オプションファイルを読み込む
bool CWSHPlugin::ReadPluginOption( CDataProfile *cProfile )
{
	return true;
}

//プラグを実行する
bool CWSHPlugin::InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params )
{
	CWSHPlug& wshPlug = static_cast<CWSHPlug&>( plug );
	CWSHMacroManager* pWsh = NULL;

	if( !m_bUseCache || wshPlug.m_Wsh == NULL ){
		CFilePath path( plug.m_cPlugin.GetFilePath( to_tchar(plug.m_sHandler.c_str()) ).c_str() );

		pWsh = (CWSHMacroManager*)CWSHMacroManager::Creator( path.GetExt( true ) );
		if( pWsh == NULL ){ return false; }

		BOOL bLoadResult = pWsh->LoadKeyMacro( G_AppInstance(), path );
		if ( !bLoadResult ){
			ErrorMessage( NULL, LS(STR_WSHPLUG_LOADMACRO), static_cast<const TCHAR*>(path) );
			delete pWsh;
			return false;
		}

	}else{
		pWsh = wshPlug.m_Wsh;
	}

	CPluginIfObj cPluginIfo(*this);		//Pluginオブジェクトを追加
	cPluginIfo.AddRef();
	cPluginIfo.SetPlugIndex( plug.m_id );	//実行中プラグ番号を提供
	pWsh->AddParam( &cPluginIfo );

	pWsh->AddParam( params );			//パラメータを追加

	pWsh->ExecKeyMacro2( view, FA_NONRECORD | FA_FROMMACRO );

	pWsh->ClearParam();

	if( m_bUseCache ){
		wshPlug.m_Wsh = pWsh;
	}else{
		// 終わったら開放
		delete pWsh;
	}

	return true;
}
