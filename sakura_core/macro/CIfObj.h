/*!	@file
	@brief WSHインターフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.hから切り出し

*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CIFOBJ_12FF5D67_757D_47E3_A7B3_E9896AAA6BB4_H_
#define SAKURA_CIFOBJ_12FF5D67_757D_47E3_A7B3_E9896AAA6BB4_H_
#pragma once

#include <string>
#include <vector>
#include "_os/OleTypes.h"
class CEditView;

//COM一般

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
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override
	{ 
		return E_NOINTERFACE; 
	}
	ULONG STDMETHODCALLTYPE AddRef() override { ++ m_RefCount; return m_RefCount; }
	ULONG STDMETHODCALLTYPE Release() override { -- m_RefCount; int R = m_RefCount; if(m_RefCount == 0) delete this; return R; }
public:
	ImplementsIUnknown(): m_RefCount(0) {}
	virtual ~ImplementsIUnknown() = default;
};

//WSH一般

class CIfObj;
typedef HRESULT (CIfObj::*CIfObjMethod)(int ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);

//CIfObjが必要とするWSHClientのインターフェース
class IWSHClient
{
public:
	virtual void* GetData() const = 0;
};

//スクリプトに渡されるオブジェクト

class CIfObj
: public ImplementsIUnknown<IDispatch>
{
public:
	// 型定義
	struct CMethodInfo
	{
		FUNCDESC		Desc;
		wchar_t			Name[64];
		CIfObjMethod	Method;
		ELEMDESC		Arguments[9];
		int				ID;
	};
	typedef std::vector<CMethodInfo> CMethodInfoList;

	// コンストラクタ・デストラクタ
	CIfObj(const wchar_t* name, bool isGlobal);
	virtual ~CIfObj();

	// フィールド・アクセサ
	const std::wstring::value_type* Name() const { return this->m_sName.c_str(); } // インターフェースオブジェクト名
	bool IsGlobal() const { return this->m_isGlobal; } //オブジェクト名の省略可否
	IWSHClient* Owner() const { return this->m_Owner; } // オーナーIWSHClient
	std::wstring m_sName;
	bool m_isGlobal;
	IWSHClient *m_Owner;

	// 操作
	void AddMethod(const wchar_t* Name, int ID, VARTYPE *ArgumentTypes,
		int ArgumentCount, VARTYPE ResultType, CIfObjMethod Method);
	void ReserveMethods(int Count)
	{
		m_Methods.reserve(Count);
	}

	// 実装
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) override;
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(
					REFIID riid,
					OLECHAR FAR* FAR* rgszNames,
					UINT cNames,
					LCID lcid,
					DISPID FAR* rgdispid) override;
	HRESULT STDMETHODCALLTYPE Invoke(
					DISPID dispidMember,
					REFIID riid,
					LCID lcid,
					WORD wFlags,
					DISPPARAMS FAR* pdispparams,
					VARIANT FAR* pvarResult,
					EXCEPINFO FAR* pexcepinfo,
					UINT FAR* puArgErr) override;
	HRESULT STDMETHODCALLTYPE GetTypeInfo(
					/* [in] */ UINT iTInfo,
					/* [in] */ LCID lcid,
					/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo) override;
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
					/* [out] */ UINT __RPC_FAR *pctinfo) override;

private:
	// メンバ変数
	CMethodInfoList m_Methods;			//メソッド情報リスト
	ITypeInfo* m_TypeInfo;
};
#endif /* SAKURA_CIFOBJ_12FF5D67_757D_47E3_A7B3_E9896AAA6BB4_H_ */
