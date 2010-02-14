/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X

	@date 2009.10.29 syat CWSH.h����؂�o��

*/
/*
	Copyright (C) 2002, �S, genta
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
#ifndef SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_
#define SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_

#include <string>
#include <list>
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
class CWSHIfObj;
enum EFunctionCode;
typedef HRESULT (CWSHIfObj::*CIfObjMethod)(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

//CWSHIfObj���K�v�Ƃ���WSHClient�̃C���^�t�F�[�X
class IWSHClient
{
public:
	virtual void* GetData() const = 0;
};

//�X�N���v�g�ɓn�����I�u�W�F�N�g
class CWSHIfObj: public ImplementsIUnknown<IDispatch>
{
	// �^��`
private:
	typedef std::wstring wstring;
public:
	typedef std::list<CWSHIfObj*> List;		//�C���^�t�F�[�X�I�u�W�F�N�g�̃��X�g
	typedef List::const_iterator ListIter;	//���̃C�e���[�^
	struct CMethodInfo
	{
		FUNCDESC				Desc;
		wchar_t					Name[64];
		CIfObjMethod			Method;
		ELEMDESC				Arguments[8];
		EFunctionCode			ID;
	};
	typedef std::vector<CMethodInfo> CMethodInfoList;

	// �R���X�g���N�^
public:
	CWSHIfObj(WCHAR* name);

	// �f�X�g���N�^
public:
	virtual ~CWSHIfObj();

	// ����
public:
	void AddMethod(wchar_t *Name, EFunctionCode ID, VARTYPE *ArgumentTypes,
		int ArgumentCount, VARTYPE ResultType, CIfObjMethod Method);
	void ReserveMethods(int Count)
	{
		m_Methods.reserve(Count);
	}
	virtual void ReadyMethods( int flags );	//�R�}���h�E�֐�����������
protected:
	//	2007.07.20 genta : flags�ǉ�
	//  2009.09.05 syat CWSHManager����ړ�
	void ReadyCommands(MacroFuncInfo *Info, int flags);
	HRESULT MacroCommand(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);

	// ����
public:
	virtual bool IsGlobalMember() { return false; }	//�I�u�W�F�N�g���̏ȗ���

	// ����
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
					REFIID riid,
					OLECHAR FAR* FAR* rgszNames,
					UINT cNames,
					LCID lcid,
					DISPID FAR* rgdispid);
	virtual HRESULT STDMETHODCALLTYPE Invoke(
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

	// �����o�ϐ�
public:
	CMethodInfoList m_Methods;			//���\�b�h��񃊃X�g
	IWSHClient *m_Owner;				//�I�[�i�[CWSHClient
	wstring m_sName;					//�C���^�t�F�[�X�I�u�W�F�N�g��
private:
	ITypeInfo* m_TypeInfo;

	// �������
protected:
	virtual bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result) = 0;	//�֐�����������
	virtual void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize) = 0;	//�R�}���h����������
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	//�R�}���h�����擾����
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	//�֐������擾����
};


#endif /* SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_ */
/*[EOF]*/
