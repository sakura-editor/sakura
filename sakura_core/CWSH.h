/*!	@file
	@brief WSH Handler

	@author �S
	@date 2002�N4��28��,5��3��,5��5��,5��6��,5��13��,5��16��
	@date 2002.08.25 genta �����N�G���[����̂���CWSHManager.h�ɃG�f�B�^��
		�}�N���C���^�[�t�F�[�X���𕪗��D

	@par TODO
	@li GetIDsOfNames�̍œK�� �� std::map���g���Ίy�����c����������STL�ɑa���̂�(;_;
*/
/*
	Copyright (C) 2002, �S, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef __WSH_H__
#define __WSH_H__

#include <windows.h>
#include <winreg.h>
#include <objbase.h>
#include <initguid.h>
#include "activscp.h"
//��Microsoft Platform SDK ���

#include <string>
#include <vector>

//COM���

template<class Base>
class ImplementsIUnknown: public Base
{
private:
	int m_RefCount;
	ImplementsIUnknown(const ImplementsIUnknown &);
	ImplementsIUnknown& operator = (const ImplementsIUnknown &);
public:
	#ifdef __BORLANDC__
	#pragma argsused
	#endif
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) 
	{ 
		return E_NOINTERFACE; 
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() { ++ m_RefCount; return m_RefCount; };
	virtual ULONG STDMETHODCALLTYPE Release() { -- m_RefCount; int R = m_RefCount; if(m_RefCount == 0) delete this; return R; };
public:
	ImplementsIUnknown(): m_RefCount(0) {};
	virtual ~ImplementsIUnknown(){};
};

//WSH���
enum EFunctionCode;
typedef HRESULT (*CInterfaceObjectMethod)(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

class CWSHClient;

//�X�N���v�g�ɓn�����I�u�W�F�N�g
class CInterfaceObject: public ImplementsIUnknown<IDispatch>
{
private:
	ITypeInfo* m_TypeInfo;
public:
	struct CMethodInfo
	{
		FUNCDESC				Desc;
		wchar_t					Name[64];
		CInterfaceObjectMethod	Method;
		ELEMDESC				Arguments[8];
		EFunctionCode			ID;
	};
	typedef std::vector<CMethodInfo> CMethodInfoList;
	CMethodInfoList m_Methods;
	CWSHClient *m_Owner;
	
	CInterfaceObject(CWSHClient *AOwner);
	~CInterfaceObject();
	void AddMethod(wchar_t *Name, EFunctionCode ID, VARTYPE *ArgumentTypes, int ArgumentCount, VARTYPE ResultType, 
					CInterfaceObjectMethod Method);
	void ReserveMethods(int Count)
	{
		m_Methods.reserve(Count);
	}
	
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
					REFIID riid,
					OLECHAR FAR* FAR* rgszNames,
					UINT cNames,
					LCID lcid,
					DISPID FAR* rgdispid);
	virtual HRESULT STDMETHODCALLTYPE CInterfaceObject::Invoke(
					DISPID dispidMember,
					REFIID riid,
					LCID lcid,
					WORD wFlags,
					DISPPARAMS FAR* pdispparams,
					VARIANT FAR* pvarResult,
					EXCEPINFO FAR* pexcepinfo,
					UINT FAR* puArgErr);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
					/* [in] */ UINT iTInfo,
					/* [in] */ LCID lcid,
					/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
					/* [out] */ UINT __RPC_FAR *pctinfo);
};

class CWSHClient
{
private:
	IActiveScript *m_Engine;
protected:
	ScriptErrorHandler m_OnError;
public:
	CWSHClient(wchar_t const *AEngine, ScriptErrorHandler AErrorHandler, void *AData);
	~CWSHClient();
	CInterfaceObject *m_InterfaceObject;
	void *m_Data;
	bool m_Valid; //true�̏ꍇ�g�p�\
	void Execute(wchar_t const *AScript);
	void Error(BSTR Description, BSTR Source);
	void Error(wchar_t* Description);
};
#endif
