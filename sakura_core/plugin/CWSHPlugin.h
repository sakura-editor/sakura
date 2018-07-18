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
#ifndef SAKURA_CWSHPLUGIN_EBA87CBD_779A_4AD3_AB4D_4BFE8E7E339AD_H_
#define SAKURA_CWSHPLUGIN_EBA87CBD_779A_4AD3_AB4D_4BFE8E7E339AD_H_

#include "plugin/CPlugin.h"
#include "macro/CWSHManager.h"

#define PII_WSH						L"Wsh"			//WSHセクション
#define PII_WSH_USECACHE			L"UseCache"		//読み込んだスクリプトを再利用する


class CWSHPlug :
	public CPlug
{
public:
	CWSHPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel ) :
		CPlug( plugin, id, sJack, sHandler, sLabel )
	{
		m_Wsh = NULL;
	}
	virtual ~CWSHPlug() {
		if( m_Wsh ){
			delete m_Wsh;
			m_Wsh = NULL;
		}
	}
	CWSHMacroManager* m_Wsh;
};

class CWSHPlugin :
	public CPlugin
{
	//コンストラクタ
public:
	CWSHPlugin( const tstring& sBaseDir ) : CPlugin( sBaseDir ) {
		m_bUseCache = false;
	}

	//デストラクタ
public:
	~CWSHPlugin(void);

	//操作
	//CPlugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
	{
		return new CWSHPlug( plugin, id, sJack, sHandler, sLabel );
	}

	//実装
public:
	bool ReadPluginDef( CDataProfile *cProfile, CDataProfile *cProfileMlang );
	bool ReadPluginOption( CDataProfile *cProfile );
	CPlug::Array GetPlugs() const{
		return m_plugs;
	}
	bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params );

	//メンバ変数
private:
	bool m_bUseCache;

};

#endif /* SAKURA_CWSHPLUGIN_EBA87CBD_779A_4AD3_AB4D_4BFE8E7E339AD_H_ */
/*[EOF]*/
