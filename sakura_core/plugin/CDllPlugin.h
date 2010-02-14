/*!	@file
	@brief DLL�v���O�C���N���X

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
#ifndef SAKURA_CDLLPLUGIN_A62D7B59_C283_4C5A_887F_DA0138E09D2C_H_
#define SAKURA_CDLLPLUGIN_A62D7B59_C283_4C5A_887F_DA0138E09D2C_H_

#include "CPlugin.h"

#define	PII_DLL							L"Dll"			//DLL���
#define	PII_DLL_NAME					L"Name"			//���O

typedef void (*DllPlugHandler)();

class CDllPlug
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

class CDllPlugin
	: public CPlugin, public CDllImp
{
	//�R���X�g���N�^
public:
	CDllPlugin( tstring sBaseDir ) : CPlugin( sBaseDir ), CDllImp() {
	}

	//�f�X�g���N�^
public:
	~CDllPlugin(void);

	//����
public:
	bool ReadPluginDef( CDataProfile *cProfile );
	bool ReadPluginOption( CDataProfile *cProfile ) {
		return true;
	}
	CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel );
	CPlug::List GetPlugs() const{
		return m_plugs;
	}
	bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& params );

	bool InitDllImp() {
		return true;
	}
	LPCTSTR GetDllNameImp(int nIndex) {
		return _T("");
	}

	//�����o�ϐ�
private:
	wstring m_sDllName;

};

#endif /* SAKURA_CDLLPLUGIN_A62D7B59_C283_4C5A_887F_DA0138E09D2C_H_ */
/*[EOF]*/
