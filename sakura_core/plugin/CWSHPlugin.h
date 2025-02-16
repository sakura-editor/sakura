/*!	@file
	@brief WSHプラグインクラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CWSHPLUGIN_C424AA2D_B0A3_4170_9566_7F316368EDAF_H_
#define SAKURA_CWSHPLUGIN_C424AA2D_B0A3_4170_9566_7F316368EDAF_H_
#pragma once

#include "plugin/CPlugin.h"
#include "macro/CWSHManager.h"

#define PII_WSH						L"Wsh"			//WSHセクション
#define PII_WSH_USECACHE			L"UseCache"		//読み込んだスクリプトを再利用する

class CWSHPlug final :
	public CPlug
{
	using Me = CWSHPlug;

public:
	CWSHPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel ) :
		CPlug( plugin, id, sJack, sHandler, sLabel )
	{
		m_Wsh = NULL;
	}
	CWSHPlug(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWSHPlug(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CWSHPlug() {
		if( m_Wsh ){
			delete m_Wsh;
			m_Wsh = NULL;
		}
	}
	CWSHMacroManager* m_Wsh;
};

class CWSHPlugin final :
	public CPlugin
{
	using Me = CWSHPlugin;

	//コンストラクタ
public:
	CWSHPlugin( const wstring& sBaseDir ) : CPlugin( sBaseDir ) {
		m_bUseCache = false;
	}
	CWSHPlugin(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWSHPlugin(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;

	//デストラクタ
public:
	~CWSHPlugin(void);

	//操作
	//CPlugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel ) override
	{
		return new CWSHPlug( plugin, id, sJack, sHandler, sLabel );
	}

	//実装
public:
	bool ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang ) override;
	bool ReadPluginOption( CDataProfile *cProfile ) override;
	CPlug::Array GetPlugs() const override{
		return m_plugs;
	}
	bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params ) override;

	//メンバ変数
private:
	bool m_bUseCache;
};
#endif /* SAKURA_CWSHPLUGIN_C424AA2D_B0A3_4170_9566_7F316368EDAF_H_ */
