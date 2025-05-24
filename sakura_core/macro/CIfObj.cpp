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
class CIfObjTypeInfo: public ImplementsIUnknown<ITypeInfo>
{
private:
	const CIfObj::CMethodInfoList& m_MethodsRef;
	const std::wstring& m_sName;
	TYPEATTR m_TypeAttr;
public:
	CIfObjTypeInfo(const CIfObj::CMethodInfoList& methods, const std::wstring& sName);

	HRESULT STDMETHODCALLTYPE GetTypeAttr(
					/* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *ppTypeAttr) override
	{
#ifdef TEST
		DEBUG_TRACE( L"GetTypeAttr\n" );
#endif
		*ppTypeAttr = &m_TypeAttr;
		return S_OK;
	}
        
	HRESULT STDMETHODCALLTYPE GetTypeComp(
					/* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *ppTComp) override
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
	    /* [in] */ UINT index,
	    /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppVarDesc) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetNames(
	    /* [in] */ MEMBERID memid,
	    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
	    /* [in] */ UINT cMaxNames,
	    /* [out] */ UINT __RPC_FAR *pcNames) override;

	HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType(
	    /* [in] */ UINT index,
	    /* [out] */ HREFTYPE __RPC_FAR *pRefType) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetImplTypeFlags(
	    /* [in] */ UINT index,
	    /* [out] */ INT __RPC_FAR *pImplTypeFlags) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetIDsOfNames(
	    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
	    /* [in] */ UINT cNames,
	    /* [size_is][out] */ MEMBERID __RPC_FAR *pMemId) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE Invoke(
	    /* [in] */ PVOID pvInstance,
	    /* [in] */ MEMBERID memid,
	    /* [in] */ WORD wFlags,
	    /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
	    /* [out] */ VARIANT __RPC_FAR *pVarResult,
	    /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
	    /* [out] */ UINT __RPC_FAR *puArgErr) override
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
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ BSTR __RPC_FAR *pBstrDllName,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ WORD __RPC_FAR *pwOrdinal) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetRefTypeInfo(
	    /* [in] */ HREFTYPE hRefType,
	    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE AddressOfMember(
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ PVOID __RPC_FAR *ppv) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance(
	    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
	    /* [in] */ REFIID riid,
	    /* [iid_is][out] */ PVOID __RPC_FAR *ppvObj) override
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetMops(
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrMops) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ HRESULT STDMETHODCALLTYPE GetContainingTypeLib(
	    /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *ppTLib,
	    /* [out] */ UINT __RPC_FAR *pIndex) override
	{
		return E_NOTIMPL;
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseTypeAttr(
					/* [in] */ TYPEATTR __RPC_FAR *pTypeAttr) override
	{
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseFuncDesc(
					/* [in] */ FUNCDESC __RPC_FAR *pFuncDesc) override
	{
	}

	/* [local] */ void STDMETHODCALLTYPE ReleaseVarDesc(
				/* [in] */ VARDESC __RPC_FAR *pVarDesc) override
	{
	}
};

CIfObjTypeInfo::CIfObjTypeInfo(const CIfObj::CMethodInfoList& methods, const std::wstring& sName)
				: ImplementsIUnknown<ITypeInfo>(), m_MethodsRef(methods), m_sName(sName)
{ 
	ZeroMemory(&m_TypeAttr, sizeof(m_TypeAttr));
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
CIfObj::CIfObj(const wchar_t* name, bool isGlobal)
: ImplementsIUnknown<IDispatch>(), m_sName(name), m_isGlobal(isGlobal), m_Owner(nullptr), m_Methods(), m_TypeInfo(nullptr)
{ 
};

//デストラクタ
CIfObj::~CIfObj()
{
	if(m_TypeInfo != NULL)
		m_TypeInfo->Release();
}
	
//IUnknown実装
HRESULT STDMETHODCALLTYPE CIfObj::QueryInterface(REFIID iid, void ** ppvObject) 
{
	if(ppvObject == NULL) 
		return E_POINTER;
	else if(IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDispatch))
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}

//IDispatch実装
HRESULT STDMETHODCALLTYPE CIfObj::Invoke(
				DISPID dispidMember,
				REFIID riid,
				LCID lcid,
				WORD wFlags,
				DISPPARAMS FAR* pdispparams,
				VARIANT FAR* pvarResult,
				EXCEPINFO FAR* pexcepinfo,
				UINT FAR* puArgErr)
{
	if((unsigned)dispidMember < m_Methods.size())
		return (this->* (m_Methods[dispidMember].Method))( m_Methods[dispidMember].ID, pdispparams, pvarResult, m_Owner->GetData() );
	else
		return E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE CIfObj::GetTypeInfo( 
				/* [in] */ UINT iTInfo,
				/* [in] */ LCID lcid,
				/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	if(m_TypeInfo == NULL)
	{
		m_TypeInfo = new CIfObjTypeInfo(this->m_Methods, this->m_sName);
		m_TypeInfo->AddRef();
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
  REFIID riid,
  OLECHAR FAR* FAR* rgszNames,
  UINT cNames,
  LCID lcid,
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
				rgdispid[i] = j;
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
	assert( wcslen(Name)<_countof(Info->Name) );
	wcscpy(Info->Name, Name);
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
