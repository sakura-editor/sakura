/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X

	@date 2009.10.29 syat CWSH.cpp����؂�o��

	@par TODO
	@li GetIDsOfNames�̍œK�� �� std::map���g���Ίy�����c����������STL�ɑa���̂�(;_;
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
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

#include "stdafx.h"
#include "CWSHIfObj.h"
#include "OleTypes.h"

//�g���[�X���b�Z�[�W�L��
#if _DEBUG && _UNICODE
#define TEST
#endif

/////////////////////////////////////////////
//�X�N���v�g�ɓn�����I�u�W�F�N�g�̌^���
class CIfObjTypeInfo: public ImplementsIUnknown<ITypeInfo>
{
private:
	CWSHIfObj *m_Object;
	TYPEATTR m_TypeAttr;
public:
	CIfObjTypeInfo(CWSHIfObj *AObject);

	virtual HRESULT STDMETHODCALLTYPE GetTypeAttr(
					/* [out] */ TYPEATTR __RPC_FAR *__RPC_FAR *ppTypeAttr)
	{
#ifdef TEST
		DebugOut( L"GetTypeAttr\n" );
#endif
		*ppTypeAttr = &m_TypeAttr;
		return S_OK;
	}
        
	virtual HRESULT STDMETHODCALLTYPE GetTypeComp( 
					/* [out] */ ITypeComp __RPC_FAR *__RPC_FAR *ppTComp)
	{
#ifdef TEST
		DebugOut( L"GetTypeComp\n" );
#endif
		return E_NOTIMPL;
	}
        
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetFuncDesc( 
				/* [in] */ UINT index,
				/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc);

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetVarDesc( 
	    /* [in] */ UINT index,
	    /* [out] */ VARDESC __RPC_FAR *__RPC_FAR *ppVarDesc)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetNames( 
	    /* [in] */ MEMBERID memid,
	    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
	    /* [in] */ UINT cMaxNames,
	    /* [out] */ UINT __RPC_FAR *pcNames);

	virtual HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType( 
	    /* [in] */ UINT index,
	    /* [out] */ HREFTYPE __RPC_FAR *pRefType)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetImplTypeFlags( 
	    /* [in] */ UINT index,
	    /* [out] */ INT __RPC_FAR *pImplTypeFlags)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
	    /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
	    /* [in] */ UINT cNames,
	    /* [size_is][out] */ MEMBERID __RPC_FAR *pMemId)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( 
	    /* [in] */ PVOID pvInstance,
	    /* [in] */ MEMBERID memid,
	    /* [in] */ WORD wFlags,
	    /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
	    /* [out] */ VARIANT __RPC_FAR *pVarResult,
	    /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
	    /* [out] */ UINT __RPC_FAR *puArgErr)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDocumentation( 
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ BSTR __RPC_FAR *pBstrDocString,
	    /* [out] */ DWORD __RPC_FAR *pdwHelpContext,
	    /* [out] */ BSTR __RPC_FAR *pBstrHelpFile)
	{
		//	Feb. 08, 2004 genta
		//	�Ƃ肠�����S��NULL��Ԃ� (��񖳂�)
		pBstrName = NULL;
		pBstrDocString = NULL;
		pdwHelpContext = NULL;
		pBstrHelpFile = NULL;
		return S_OK ;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDllEntry( 
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ BSTR __RPC_FAR *pBstrDllName,
	    /* [out] */ BSTR __RPC_FAR *pBstrName,
	    /* [out] */ WORD __RPC_FAR *pwOrdinal)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetRefTypeInfo( 
	    /* [in] */ HREFTYPE hRefType,
	    /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE AddressOfMember( 
	    /* [in] */ MEMBERID memid,
	    /* [in] */ INVOKEKIND invKind,
	    /* [out] */ PVOID __RPC_FAR *ppv)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
	    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
	    /* [in] */ REFIID riid,
	    /* [iid_is][out] */ PVOID __RPC_FAR *ppvObj)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetMops( 
	    /* [in] */ MEMBERID memid,
	    /* [out] */ BSTR __RPC_FAR *pBstrMops)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetContainingTypeLib( 
	    /* [out] */ ITypeLib __RPC_FAR *__RPC_FAR *ppTLib,
	    /* [out] */ UINT __RPC_FAR *pIndex)
	{
		return E_NOTIMPL;
	}
        
	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseTypeAttr( 
					/* [in] */ TYPEATTR __RPC_FAR *pTypeAttr)
	{
	}
        
	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseFuncDesc( 
					/* [in] */ FUNCDESC __RPC_FAR *pFuncDesc)
	{
	}
        
	virtual /* [local] */ void STDMETHODCALLTYPE ReleaseVarDesc(
				/* [in] */ VARDESC __RPC_FAR *pVarDesc)
	{
	}
};

CIfObjTypeInfo::CIfObjTypeInfo(CWSHIfObj *AObject)
				: ImplementsIUnknown<ITypeInfo>(), m_Object(AObject)
{ 
	ZeroMemory(&m_TypeAttr, sizeof(m_TypeAttr));
	m_TypeAttr.cImplTypes = 0; //�e�N���X��ITypeInfo�̐�
	m_TypeAttr.cFuncs = m_Object->m_Methods.size();
}

HRESULT STDMETHODCALLTYPE CIfObjTypeInfo::GetFuncDesc( 
			/* [in] */ UINT index,
			/* [out] */ FUNCDESC __RPC_FAR *__RPC_FAR *ppFuncDesc)
{
#ifdef TEST
	DebugOut( L"GetFuncDesc\n" );
#endif
	*ppFuncDesc = &(m_Object->m_Methods[index].Desc);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CIfObjTypeInfo::GetNames( 
    /* [in] */ MEMBERID memid,
    /* [length_is][size_is][out] */ BSTR __RPC_FAR *rgBstrNames,
    /* [in] */ UINT cMaxNames,
    /* [out] */ UINT __RPC_FAR *pcNames)
{
#ifdef TEST
		DebugOut( L"GetNames\n" );
#endif
	*pcNames = 1;
	if(cMaxNames > 0)
		*rgBstrNames = SysAllocString(m_Object->m_Methods[memid].Name);
	return S_OK;
}


/////////////////////////////////////////////
//�C���^�t�F�[�X�I�u�W�F�N�g

//�R���X�g���N�^
CWSHIfObj::CWSHIfObj(WCHAR* name)
	: ImplementsIUnknown<IDispatch>(), m_Methods(), m_TypeInfo(NULL), m_sName(name)
{ 
};

//�f�X�g���N�^
CWSHIfObj::~CWSHIfObj()
{
	if(m_TypeInfo != NULL)
		m_TypeInfo->Release();
}
	
//IUnknown����
HRESULT STDMETHODCALLTYPE CWSHIfObj::QueryInterface(REFIID iid, void ** ppvObject) 
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

//IDispatch����
HRESULT STDMETHODCALLTYPE CWSHIfObj::Invoke(
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

HRESULT STDMETHODCALLTYPE CWSHIfObj::GetTypeInfo( 
				/* [in] */ UINT iTInfo,
				/* [in] */ LCID lcid,
				/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	if(m_TypeInfo == NULL)
	{
		m_TypeInfo = new CIfObjTypeInfo(this);
		m_TypeInfo->AddRef();
	}
		
	(*ppTInfo) = m_TypeInfo;
	(*ppTInfo)->AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CWSHIfObj::GetTypeInfoCount( 
				/* [out] */ UINT __RPC_FAR *pctinfo)
{
	*pctinfo = 1;
	return S_OK;
}

#ifdef __BORLANDC__
#pragma argsused
#endif
HRESULT STDMETHODCALLTYPE CWSHIfObj::GetIDsOfNames(
  REFIID riid,
  OLECHAR FAR* FAR* rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID FAR* rgdispid)
{
	for(unsigned I = 0; I < cNames; ++I)
	{
#ifdef TEST
		//��ʂɃ��b�Z�[�W���o��̂Œ��ӁB
		//DebugOut( L"GetIDsOfNames: %ls\n", rgszNames[I] );
#endif
		for(unsigned J = 0; J < m_Methods.size(); ++J)
		{
			//	Nov. 10, 2003 FILE Win9X�ł́A[lstrcmpiW]�������̂��߁A[_wcsicmp]�ɏC��
			if(_wcsicmp(rgszNames[I], m_Methods[J].Name) == 0)
			{
				rgdispid[I] = J;
				goto Found;
			}
		}
		return DISP_E_UNKNOWNNAME;
		Found:
		;
	}
	return S_OK;		
}

//�^���Ƀ��\�b�h��ǉ�����
void CWSHIfObj::AddMethod(
	wchar_t*				Name,
	EFunctionCode			ID,
	VARTYPE*				ArgumentTypes,
	int						ArgumentCount,
	VARTYPE					ResultType,
	CIfObjMethod	Method
)
{
	m_Methods.push_back(CMethodInfo());
	CMethodInfo *Info = &m_Methods[m_Methods.size() - 1];
	ZeroMemory(Info, sizeof(CMethodInfo));
	Info->Desc.invkind = INVOKE_FUNC;
	Info->Desc.cParams = ArgumentCount + 1; //�߂�l�̕�
	Info->Desc.lprgelemdescParam = Info->Arguments;
	//	Nov. 10, 2003 FILE Win9X�ł́A[lstrcpyW]�������̂��߁A[wcscpy]�ɏC��
	wcscpy(Info->Name, Name);
	Info->Method = Method;
	Info->ID = ID;
	for(int I = 0; I < ArgumentCount; ++I)
	{
		Info->Arguments[I].tdesc.vt = ArgumentTypes[ArgumentCount - I - 1];
		Info->Arguments[I].paramdesc.wParamFlags = PARAMFLAG_FIN;
	}
	Info->Arguments[ArgumentCount].tdesc.vt = ResultType;
	Info->Arguments[ArgumentCount].paramdesc.wParamFlags = PARAMFLAG_FRETVAL;
}

//�R�}���h�E�֐�����������
void CWSHIfObj::ReadyMethods( int flags )
{
	//	 2007.07.20 genta : �R�}���h�ɍ������ރt���O��n��
	ReadyCommands(GetMacroCommandInfo(), flags | FA_FROMMACRO );
	ReadyCommands(GetMacroFuncInfo(), 0);
	/* CWSHIfObj���p�������T�u�N���X����ReadyMethods���Ăяo�����ꍇ�A
	 * �T�u�N���X��GetMacroCommandInfo,GetMacroFuncInfo���Ăяo�����B */
}

/** WSH�}�N���G���W���փR�}���h�o�^���s��

	@date 2007.07.20 genta flags�ǉ��Dflag�̓R�}���h�o�^�i�K�ō����Ă����D
*/
void CWSHIfObj::ReadyCommands(MacroFuncInfo *Info, int flags)
{
	while(Info->m_nFuncID != -1)	// Aug. 29, 2002 genta �Ԑl�̒l���ύX���ꂽ�̂ł������ύX
	{
		wchar_t FuncName[256];
		wcscpy(FuncName, Info->m_pszFuncName);

		int ArgCount = 0;
		for(int I = 0; I < 4; ++I)
			if(Info->m_varArguments[I] != VT_EMPTY) 
				++ArgCount;
		
		//	2007.07.21 genta : flag���������l��o�^����
		this->AddMethod(
			FuncName,
			(EFunctionCode)(Info->m_nFuncID | flags),
			Info->m_varArguments,
			ArgCount,
			Info->m_varResult,
			&CWSHIfObj::MacroCommand
			/* CWSHIfObj���p�������T�u�N���X����ReadyCommands���Ăяo�����ꍇ�A
			 * �T�u�N���X��MacroCommand���Ăяo�����B */
		);
		
		++Info;
	}
}

/*!
	�}�N���R�}���h�̎��s

	@date 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��G���[�ɂ����Ɋ֐������s����
*/
HRESULT CWSHIfObj::MacroCommand(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	CEditView *View = reinterpret_cast<CEditView*>(Data);

	//	2007.07.22 genta : �R�}���h�͉���16�r�b�g�̂�
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��֐������s����
		VariantInit(&ret);
		
		// 2009.9.5 syat HandleFunction�̓T�u�N���X�ŃI�[�o�[���C�h����
		bool r = HandleFunction(View, ID, Arguments->rgvarg, Arguments->cArgs, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		return r ? S_OK : E_FAIL;
	}
	else
	{
		int ArgCount = Arguments->cArgs;
		if(ArgCount > 4) ArgCount = 4;

		//	Nov. 29, 2005 FILE �����𕶎���Ŏ擾����
		WCHAR *StrArgs[4] = {NULL, NULL, NULL, NULL};	// �������K�{
		WCHAR *S = NULL;								// �������K�{
		Variant varCopy;							// VT_BYREF���ƍ���̂ŃR�s�[�p
		int Len;
		for(int I = 0; I < ArgCount; ++I)
		{
			if(VariantChangeType(&varCopy.Data, &(Arguments->rgvarg[I]), 0, VT_BSTR) == S_OK)
			{
				Wrap(&varCopy.Data.bstrVal)->GetW(&S, &Len);
			}
			else
			{
				S = new WCHAR[1];
				S[0] = 0;
			}
			StrArgs[ArgCount - I - 1] = S;			// DISPPARAMS�͈����̏������t�]���Ă��邽�ߐ��������ɒ���
		}

		// 2009.10.29 syat HandleCommand�̓T�u�N���X�ŃI�[�o�[���C�h����
		HandleCommand(View, ID, const_cast<WCHAR const **>(StrArgs), ArgCount);

		//	Nov. 29, 2005 FILE �z��̔j���Ȃ̂ŁA[����]��ǉ�
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}
