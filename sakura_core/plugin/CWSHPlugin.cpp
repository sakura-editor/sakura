/*!	@file
	@brief WSHプラグインクラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "plugin/CWSHPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CWSHManager.h"
#include "CSelectLang.h"
#include "String_define.h"

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
		CFilePath path( plug.m_cPlugin.GetFilePath( plug.m_sHandler ).c_str() );

		pWsh = (CWSHMacroManager*)CWSHMacroManager::Creator( path.GetExt( true ) );
		if( pWsh == NULL ){ return false; }

		BOOL bLoadResult = pWsh->LoadKeyMacro( G_AppInstance(), path );
		if ( !bLoadResult ){
			ErrorMessage( NULL, LS(STR_WSHPLUG_LOADMACRO), static_cast<const WCHAR*>(path) );
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
		// 終わったら解放
		delete pWsh;
	}

	return true;
}
