/*!	@file
	@brief WSH Handler

	@author 鬼
	@date 2002年4月28日,5月3日,5月5日,5月6日,5月13日,5月16日
	@date 2002.08.25 genta リンクエラー回避のためCWSHManager.hにエディタの
		マクロインターフェース部を分離．
	@date 2009.10.29 syat インタフェースオブジェクト部分をCWSHIfObj.hに分離
*/
/*
	Copyright (C) 2002, 鬼, genta
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
//↑Microsoft Platform SDK より
#include "CWSHIfObj.h"

/* 2009.10.29 syat インタフェースオブジェクト部分をCWSHIfObj.hに分離
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
	bool m_Valid; //trueの場合使用可能
	void Execute(wchar_t const *AScript);
	void Error(BSTR Description, BSTR Source);
	void Error(wchar_t* Description);
	void* GetData() const { return m_Data; }
	void AddInterfaceObject( CWSHIfObj* obj );
	void AddInterfaceObject( CWSHIfObj::List& obj );
	const CWSHIfObj::List& GetInterfaceObject();
};


#endif
