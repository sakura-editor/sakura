/*!	@file
	@brief DLLプラグインクラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "plugin/CDllPlugin.h"
#include "view/CEditView.h"
#include "util/tchar_convert.h"
#include "CSelectLang.h"
#include "String_define.h"

// デストラクタ
CDllPlugin::~CDllPlugin(void)
{
	for( CPlug::ArrayIter it = m_plugs.begin(); it != m_plugs.end(); it++ ){
		delete (CDllPlug*)(*it);
	}
}

// プラグの生成
// CPlugの代わりにCDllPlugを作成する
CPlug* CDllPlugin::CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
{
	CDllPlug *newPlug =  new CDllPlug( plugin, id, sJack, sHandler, sLabel );
	return newPlug;
}

// プラグイン定義ファイルの読み込み
bool CDllPlugin::ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang )
{
	ReadPluginDefCommon( cProfile, cProfileMlang );

	//DLL名の読み込み
	cProfile->IOProfileData( PII_DLL, PII_DLL_NAME, m_sDllName );

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

// プラグ実行
bool CDllPlugin::InvokePlug( CEditView* view, CPlug& plug_raw, CWSHIfObj::List& params )
{
	wstring dllPath = GetFilePath( m_sDllName );
	EDllResult resInit = InitDll( dllPath.c_str() );
	if( resInit != DLL_SUCCESS ){
		::MYMESSAGEBOX( view->m_hwndParent, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR1), dllPath.c_str(), m_sName.c_str() );
		return false;
	}

	CDllPlug& plug = *(static_cast<CDllPlug*>(&plug_raw));
	if( ! plug.m_handler ){
		//DLL関数の取得
		ImportTable imp[2] = {
			{ &plug.m_handler, to_achar( plug.m_sHandler.c_str() ) },
			{ nullptr, nullptr }
		};
		if( ! RegisterEntries( imp ) ){
//			DWORD err = GetLastError();
			::MYMESSAGEBOX( NULL, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR2) );
			return false;
		}
	}
	CMacroBeforeAfter ba;
	int flags = FA_NONRECORD | FA_FROMMACRO;
	ba.ExecKeyMacroBefore(view, flags);
	//DLL関数の呼び出し
	plug.m_handler();
	ba.ExecKeyMacroAfter(view, flags, true);
	
	return true;
}
