/*!	@file
	@brief WSHインターフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.cppから切り出し

	@par TODO
	@li GetIDsOfNamesの最適化 → std::mapを使えば楽そう…しかし私はSTLに疎いので(;_;
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "macro/CIfObj.h"

#include "debug/Debug1.h"
#include "debug/Debug2.h"

//トレースメッセージ有無
#if defined( _DEBUG )
#define TEST
#endif

/////////////////////////////////////////////
//スクリプトに渡されるオブジェクトの型情報
class CIfObjTypeInfo : public cxx::TComImpl<ITypeInfo>
{
private:
	using Base = cxx::TComImpl<ITypeInfo>;
	using Me = CIfObjTypeInfo;

	const CIfObj::CMethodInfoList& m_MethodsRef;
	std::wstring m_sName;
	TYPEATTR m_TypeAttr{};

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<CIfObjTypeInfo, Args...>
	{
		return Base::template make_instance<CIfObjTypeInfo>(std::forward<Args>(args)...);
	}

	CIfObjTypeInfo(const CIfObj::CMethodInfoList& methods, std::wstring_view sName);

	HRESULT STDMETHODCALLTYPE GetTypeAttr(
					/* [out] */ [[maybe_unused]] TYPEATTR __RPC_FAR *__RPC_FAR *ppTypeAttr) override
	{
#ifdef TEST
		DEBUG_TRACE( L"GetTypeAttr\n" );
#endif
		*ppTypeAttr = &m_TypeAttr;
		return S_OK;
	}
        
	HRESULT STDMETHODCALLTYPE GetTypeComp(
					/* [out] */ [[maybe_unused]] ITypeComp __RPC_FAR *__RPC_FAR *ppTComp) override
	{
#ifdef TEST
		DEBUG_TRACE( L"GetTypeComp\n" );
#endif
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetFuncDesc(
				/* [in] */ UINT index,
				/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc) override;

	/* [local] */ HRESULT STDMETHODCALLTYPE GetVarDesc(
	    /* [in] */ [[maybe_unused]] UINT index,
	    /* [out] */ [[maybe_unused]] VARDESC __RPC_FAR *__RPC_FAR *ppVarDesc) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetNames(
	    /* [in] */ MEMBERID memid,
	    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
	    /* [in] */ UINT cMaxNames,
	    /* [out] */ UINT __RPC_FAR *pcNames) override;

	HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType(
	    /* [in] */ [[maybe_unused]] UINT index,
	    /* [out] */ [[maybe_unused]] HREFTYPE __RPC_FAR *pRefType) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetImplTypeFlags(
	    /* [in] */ [[maybe_unused]] UINT index,
	    /* [out] */ [[maybe_unused]] INT __RPC_FAR *pImplTypeFlags) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetIDsOfNames(
	    /* [size_is][in] */ [[maybe_unused]] LPOLESTR __RPC_FAR *rgszNames,
	    /* [in] */ [[maybe_unused]] UINT cNames,
	    /* [size_is][out] */ [[maybe_unused]] MEMBERID __RPC_FAR *pMemId) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE Invoke(
	    /* [in] */ [[maybe_unused]] PVOID pvInstance,
	    /* [in] */ [[maybe_unused]] MEMBERID memid,
	    /* [in] */ [[maybe_unused]] WORD wFlags,
	    /* [out][in] */ [[maybe_unused]] DISPPARAMS __RPC_FAR *pDispParams,
	    /* [out] */ [[maybe_unused]] VARIANT __RPC_FAR *pVarResult,
	    /* [out] */ [[maybe_unused]] EXCEPINFO __RPC_FAR *pExcepInfo,
	    /* [out] */ [[maybe_unused]] UINT __RPC_FAR *puArgErr) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetDocumentation(
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ BSTR __RPC_FAR *pBstrDocString,
	    /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
	    /* [out] */ BSTR __RPC_FAR *pBstrHelpFile) override
	{
		//	Feb. 08, 2004 genta
		//	とりあえず全部NULLを返す (情報無し)
		//	2014.02.12 各パラメータを設定するように
		if( memid == -1 ){
			if( pBstrName ){
				*pBstrName = SysAllocString( m_sName.c_str() );
			}
		}else if( 0 <= memid && memid < (int)m_MethodsRef.size() ){
			if( pBstrName ){
				*pBstrName = SysAllocString( m_MethodsRef[memid].Name );
			}
		}else{
			return TYPE_E_ELEMENTNOTFOUND;
		}
		if( pBstrDocString ){
			*pBstrDocString = SysAllocString(L"");
		}
		if( pdwHelpContext ){
			*pdwHelpContext = 0;
		}
		if( pBstrHelpFile ){
			*pBstrHelpFile = SysAllocString(L"");
		}
		return S_OK;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetDllEntry(
	    /* [in] */ [[maybe_unused]] MEMBERID memid,
	    /* [in] */ [[maybe_unused]] INVOKEKIND invKind,
	    /* [out] */ [[maybe_unused]] BSTR __RPC_FAR *pBstrDllName,
	    /* [out] */ [[maybe_unused]] BSTR __RPC_FAR *pBstrName,
	    /* [out] */ [[maybe_unused]] WORD __RPC_FAR *pwOrdinal) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetRefTypeInfo(
	    /* [in] */ [[maybe_unused]] HREFTYPE hRefType,
	    /* [out] */ [[maybe_unused]] ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE AddressOfMember(
	    /* [in] */ [[maybe_unused]] MEMBERID memid,
	    /* [in] */ [[maybe_unused]] INVOKEKIND invKind,
	    /* [out] */ [[maybe_unused]] PVOID __RPC_FAR *ppv) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance(
	    /* [in] */ [[maybe_unused]] IUnknown __RPC_FAR *pUnkOuter,
	    /* [in] */ [[maybe_unused]] REFIID riid,
	    /* [iid_is][out] */ [[maybe_unused]] PVOID __RPC_FAR *ppvObj) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetMops(
	    /* [in] */ [[maybe_unused]] MEMBERID memid,
	    /* [out] */ [[maybe_unused]] BSTR __RPC_FAR *pBstrMops) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetContainingTypeLib(
	    /* [out] */ [[maybe_unused]] ITypeLib __RPC_FAR *__RPC_FAR *ppTLib,
	    /* [out] */ [[maybe_unused]] UINT __RPC_FAR *pIndex) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseTypeAttr(
					/* [in] */ [[maybe_unused]] TYPEATTR __RPC_FAR *pTypeAttr) override
	{
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseFuncDesc(
					/* [in] */ [[maybe_unused]] FUNCDESC __RPC_FAR *pFuncDesc) override
	{
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseVarDesc(
				/* [in] */ [[maybe_unused]] VARDESC __RPC_FAR *pVarDesc) override
	{
	}
};

CIfObjTypeInfo::CIfObjTypeInfo(const CIfObj::CMethodInfoList& methods, std::wstring_view sName)
	: m_MethodsRef(methods)
	, m_sName(sName)
{
	m_TypeAttr.cImplTypes = 0; //親クラスのITypeInfoの数
	m_TypeAttr.cFuncs = (WORD)m_MethodsRef.size();
}

HRESULT STDMETHODCALLTYPE CIfObjTypeInfo::GetFuncDesc( 
			/* [in] */ UINT index,
			/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc)
{
#ifdef TEST
	DEBUG_TRACE( L"GetFuncDesc\n" );
#endif
	*ppFuncDesc = const_cast<FUNCDESC __RPC_FAR *>(&(m_MethodsRef[index].Desc));
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIfObjTypeInfo::GetNames( 
    /* [in] */ MEMBERID memid,
    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
    /* [in] */ UINT cMaxNames,
    /* [out] */ UINT __RPC_FAR *pcNames)
{
#ifdef TEST
		DEBUG_TRACE( L"GetNames\n" );
#endif
	*pcNames = 1;
	if(cMaxNames > 0)
		*rgBstrNames = SysAllocString(m_MethodsRef[memid].Name);
	return S_OK;
}

/////////////////////////////////////////////
//インターフェースオブジェクト

//コンストラクタ
CIfObj::CIfObj(std::wstring_view name, bool isGlobal)
	: m_sName(name)
	, m_isGlobal(isGlobal)
{
}

//デストラクタ
CIfObj::~CIfObj() = default;
	
//IDispatch実装
HRESULT STDMETHODCALLTYPE CIfObj::Invoke(
				DISPID dispidMember,
				[[maybe_unused]] REFIID riid,
				[[maybe_unused]] LCID lcid,
				[[maybe_unused]] WORD wFlags,
				DISPPARAMS FAR* pdispparams,
				VARIANT FAR* pvarResult,
				[[maybe_unused]] EXCEPINFO FAR* pexcepinfo,
				[[maybe_unused]] UINT FAR* puArgErr)
{
	if((unsigned)dispidMember < m_Methods.size())
		return (this->* (m_Methods[dispidMember].Method))( m_Methods[dispidMember].ID, pdispparams, pvarResult, m_Owner->GetData() );
	else
		return E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE CIfObj::GetTypeInfo( 
				/* [in] */ [[maybe_unused]] UINT iTInfo,
				/* [in] */ [[maybe_unused]] LCID lcid,
				/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	if (!m_TypeInfo) {
		m_TypeInfo = CIfObjTypeInfo::make_instance(m_Methods, m_sName);
	}
		
	(*ppTInfo) = m_TypeInfo;
	(*ppTInfo)->AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIfObj::GetTypeInfoCount( 
				/* [out] */ UINT __RPC_FAR *pctinfo)
{
	*pctinfo = 1;
	return S_OK;
}

#ifdef __BORLANDC__
#pragma argsused
#endif
HRESULT STDMETHODCALLTYPE CIfObj::GetIDsOfNames(
  [[maybe_unused]] REFIID riid,
  OLECHAR FAR* FAR* rgszNames,
  UINT cNames,
  [[maybe_unused]] LCID lcid,
  DISPID FAR* rgdispid)
{
	for(unsigned i = 0; i < cNames; ++i)
	{
#ifdef TEST
		//大量にメッセージが出るので注意。
		//DEBUG_TRACE( L"GetIDsOfNames: %ls\n", rgszNames[i] );
#endif
		size_t nSize = m_Methods.size();
		for(size_t j = 0; j < nSize; ++j)
		{
			//	Nov. 10, 2003 FILE Win9Xでは、[lstrcmpiW]が無効のため、[_wcsicmp]に修正
			if(_wcsicmp(rgszNames[i], m_Methods[j].Name) == 0)
			{
				rgdispid[i] = (int)j;
				goto Found;
			}
		}
		return DISP_E_UNKNOWNNAME;
		Found:
		;
	}
	return S_OK;
}

//型情報にメソッドを追加する
void CIfObj::AddMethod(
	const wchar_t*	Name,
	int				ID,
	VARTYPE*		ArgumentTypes,
	int				ArgumentCount,
	VARTYPE			ResultType,
	CIfObjMethod	Method
)
{
	/*
		this->m_TypeInfoが NULLでなければ AddMethod()は反映されない。
	*/
	m_Methods.push_back(CMethodInfo());
	CMethodInfo *Info = &m_Methods[m_Methods.size() - 1];
	ZeroMemory(Info, sizeof(CMethodInfo));
	Info->Desc.invkind = INVOKE_FUNC;
	Info->Desc.cParams = (SHORT)ArgumentCount + 1; //戻り値の分
	Info->Desc.lprgelemdescParam = Info->Arguments;
	//	Nov. 10, 2003 FILE Win9Xでは、[lstrcpyW]が無効のため、[wcscpy]に修正
	assert( wcslen(Name)<int(std::size(Info->Name)) );
	::wcsncpy_s(Info->Name, Name, _TRUNCATE);
	Info->Method = Method;
	Info->ID = ID;
	for(int i = 0; i < ArgumentCount; ++i)
	{
		Info->Arguments[i].tdesc.vt = ArgumentTypes[ArgumentCount - i - 1];
		Info->Arguments[i].paramdesc.wParamFlags = PARAMFLAG_FIN;
	}
	Info->Arguments[ArgumentCount].tdesc.vt = ResultType;
	Info->Arguments[ArgumentCount].paramdesc.wParamFlags = PARAMFLAG_FRETVAL;
}
