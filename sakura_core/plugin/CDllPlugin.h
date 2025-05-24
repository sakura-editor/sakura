/*!	@file
	@brief DLLプラグインクラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDLLPLUGIN_7F8E705E_8E61_4B89_BA48_936CD5429E1D_H_
#define SAKURA_CDLLPLUGIN_7F8E705E_8E61_4B89_BA48_936CD5429E1D_H_
#pragma once

#include "CPlugin.h"

#define	PII_DLL							L"Dll"			//DLL情報
#define	PII_DLL_NAME					L"Name"			//名前

typedef void (*DllPlugHandler)();

class CDllPlug final
	: public CPlug
{
public:
	CDllPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
		: CPlug( plugin, id, sJack, sHandler, sLabel )
		, m_handler( NULL )
	{
	}
public:
	DllPlugHandler m_handler;
};

class CDllPlugin final
	: public CPlugin, public CDllImp
{
	//コンストラクタ
public:
	CDllPlugin( const wstring& sBaseDir ) : CPlugin( sBaseDir ), CDllImp() {
	}

	//デストラクタ
public:
	~CDllPlugin(void);

	//実装
public:
	bool ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang ) override;
	bool ReadPluginOption( CDataProfile *cProfile ) override{
		return true;
	}
	CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel ) override;
	CPlug::Array GetPlugs() const override{
		return m_plugs;
	}
	bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params ) override;

	bool InitDllImp() override {
		return true;
	}
	LPCWSTR GetDllNameImp(int nIndex) override {
		return L"";
	}

	//メンバ変数
private:
	wstring m_sDllName;
};
#endif /* SAKURA_CDLLPLUGIN_7F8E705E_8E61_4B89_BA48_936CD5429E1D_H_ */
