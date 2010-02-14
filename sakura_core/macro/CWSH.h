/*!	@file
	@brief WSH Handler

	@author �S
	@date 2002�N4��28��,5��3��,5��5��,5��6��,5��13��,5��16��
	@date 2002.08.25 genta �����N�G���[����̂���CWSHManager.h�ɃG�f�B�^��
		�}�N���C���^�[�t�F�[�X���𕪗��D
	@date 2009.10.29 syat �C���^�t�F�[�X�I�u�W�F�N�g������CWSHIfObj.h�ɕ���
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2009, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef __WSH_H__
#define __WSH_H__

//#include <windows.h>
//#include <winreg.h>
//#include <objbase.h>
//#include <initguid.h>
#include "activscp.h"
//��Microsoft Platform SDK ���
#include "CWSHIfObj.h"

/* 2009.10.29 syat �C���^�t�F�[�X�I�u�W�F�N�g������CWSHIfObj.h�ɕ���
template<class Base>
class ImplementsIUnknown: public Base

class CInterfaceObject: public ImplementsIUnknown<IDispatch>
 */

class CWSHClient : IWSHClient
{
private:
	IActiveScript *m_Engine;
	void *m_Data;
protected:
	ScriptErrorHandler m_OnError;
	CWSHIfObj::List m_IfObjArr;
public:
	CWSHClient(wchar_t const *AEngine, ScriptErrorHandler AErrorHandler, void *AData);
	~CWSHClient();
	bool m_Valid; //true�̏ꍇ�g�p�\
	void Execute(wchar_t const *AScript);
	void Error(BSTR Description, BSTR Source);
	void Error(wchar_t* Description);
	void* GetData() const { return m_Data; }
	void AddInterfaceObject( CWSHIfObj* obj );
	void AddInterfaceObject( CWSHIfObj::List& obj );
	const CWSHIfObj::List& GetInterfaceObject();
};


#endif
