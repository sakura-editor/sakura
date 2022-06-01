﻿/*!	@file
	@brief DLLプラグインクラス

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

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

	bool InitDllImp() {
		return true;
	}
	LPCWSTR GetDllNameImp(int nIndex) {
		return L"";
	}

	//メンバ変数
private:
	wstring m_sDllName;
};
#endif /* SAKURA_CDLLPLUGIN_7F8E705E_8E61_4B89_BA48_936CD5429E1D_H_ */
