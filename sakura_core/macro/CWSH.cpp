/*!	@file
	@brief WSH Handler

	@author 鬼
	@date 2002年4月28日
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#include "stdafx.h"
#include "macro/CWSH.h"
#include "macro/CIfObj.h"
#include "window/CEditWnd.h"
#include "util/os.h"
#include "util/module.h"

/* 2009.10.29 syat インタフェースオブジェクト部分をCWSHIfObj.hに分離
class CInterfaceObjectTypeInfo: public ImplementsIUnknown<ITypeInfo>
 */

//IActiveScriptSite, IActiveScriptSiteWindow
/*!
	@date Sep. 15, 2005 FILE IActiveScriptSiteWindow実装．
		マクロでMsgBoxを使用可能にする．
*/
class CWSHSite: public IActiveScriptSite, public IActiveScriptSiteWindow
{
private:
	CWSHClient *m_Client;
	ITypeInfo *m_TypeInfo;
	ULONG m_RefCount;
public:
	CWSHSite(CWSHClient *AClient): m_RefCount(0), m_Client(AClient)
	{
	}

	virtual ULONG _stdcall AddRef() {
		return ++m_RefCount;
	}

	virtual ULONG _stdcall Release() {
		if(--m_RefCount == 0)
		{
			delete this;
			return 0;
		}
		return m_RefCount;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
	    /* [in] */ REFIID iid,
	    /* [out] */ void ** ppvObject)
	{
		*ppvObject = NULL;

		if(iid == IID_IActiveScriptSiteWindow){
			*ppvObject = static_cast<IActiveScriptSiteWindow*>(this);
			++m_RefCount;
			return S_OK;
		}

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetLCID( 
	    /* [out] */ LCID *plcid) 
	{ 
#ifdef TEST
		cout << "GetLCID" << endl;
#endif
		return E_NOTIMPL; //システムデフォルトを使用
	};

	virtual HRESULT STDMETHODCALLTYPE GetItemInfo( 
	    /* [in] */ LPCOLESTR pstrName,
	    /* [in] */ DWORD dwReturnMask,
	    /* [out] */ IUnknown **ppiunkItem,
	    /* [out] */ ITypeInfo **ppti) 
	{
#ifdef TEST
		wcout << L"GetItemInfo:" << pstrName << endl;
#endif
		//指定された名前のインタフェースオブジェクトを検索
		const CWSHClient::List& objects = m_Client->GetInterfaceObjects();
		for( CWSHClient::ListIter it = objects.begin(); it != objects.end(); it++ )
		{
			//	Nov. 10, 2003 FILE Win9Xでは、[lstrcmpiW]が無効のため、[_wcsicmp]に修正
			if( _wcsicmp( pstrName, (*it)->m_sName.c_str() ) == 0 )
			{
				if(dwReturnMask & SCRIPTINFO_IUNKNOWN)
				{
					(*ppiunkItem) = *it;
					(*ppiunkItem)->AddRef();
				}
				if(dwReturnMask & SCRIPTINFO_ITYPEINFO)
				{
					(*it)->GetTypeInfo(0, 0, ppti);
				}
				return S_OK;
			}
		}
		return TYPE_E_ELEMENTNOTFOUND;
	};

	virtual HRESULT STDMETHODCALLTYPE GetDocVersionString( 
	    /* [out] */ BSTR *pbstrVersion) 
	{ 
#ifdef TEST
		cout << "GetDocVersionString" << endl;
#endif
		return E_NOTIMPL; 
	};

	virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate( 
	    /* [in] */ const VARIANT *pvarResult,
	    /* [in] */ const EXCEPINFO *pexcepinfo) 
	{ 
#ifdef TEST
		cout << "OnScriptTerminate" << endl;
#endif
		return S_OK; 
	};

	virtual HRESULT STDMETHODCALLTYPE OnStateChange( 
	    /* [in] */ SCRIPTSTATE ssScriptState) 
	{ 
#ifdef TEST
		cout << "OnStateChange" << endl;
#endif
		return S_OK; 
	};

	//	Nov. 3, 2002 鬼
	//	エラー行番号表示対応
	virtual HRESULT STDMETHODCALLTYPE OnScriptError(
	  /* [in] */ IActiveScriptError *pscripterror)
	{ 
		EXCEPINFO Info;
		if(pscripterror->GetExceptionInfo(&Info) == S_OK)
		{
			DWORD Context;
			ULONG Line;
			LONG Pos;
			if(pscripterror->GetSourcePosition(&Context, &Line, &Pos) == S_OK)
			{
				wchar_t *Message = new wchar_t[SysStringLen(Info.bstrDescription) + 128];
				//	Nov. 10, 2003 FILE Win9Xでは、[wsprintfW]が無効のため、[auto_sprintf]に修正
				const wchar_t* szDesc=Info.bstrDescription;
				auto_sprintf(Message, L"[Line %d] %ls", Line + 1, szDesc);
				SysReAllocString(&Info.bstrDescription, Message);
				delete[] Message;
			}
			m_Client->Error(Info.bstrDescription, Info.bstrSource);
			SysFreeString(Info.bstrSource);
			SysFreeString(Info.bstrDescription);
			SysFreeString(Info.bstrHelpFile);
		}
		return S_OK;
	};

	virtual HRESULT STDMETHODCALLTYPE OnEnterScript() {
#ifdef TEST
		cout << "OnEnterScript" << endl;
#endif
		return S_OK; 
	};

	virtual HRESULT STDMETHODCALLTYPE OnLeaveScript() {
#ifdef TEST
		cout << "OnLeaveScript" << endl;
#endif
		return S_OK; 
	};

	//	Sep. 15, 2005 FILE IActiveScriptSiteWindow実装
	virtual HRESULT __stdcall GetWindow(
	    /* [out] */ HWND *phwnd)
	{
		*phwnd = CEditWnd::Instance()->m_cSplitterWnd.GetHwnd();
		return S_OK;
	}

	//	Sep. 15, 2005 FILE IActiveScriptSiteWindow実装
	virtual HRESULT __stdcall EnableModeless(
	    /* [in] */ BOOL fEnable)
	{
		return S_OK;
	};
};

//implementation

CWSHClient::CWSHClient(wchar_t const *AEngine, ScriptErrorHandler AErrorHandler, void *AData): 
				m_Engine(NULL), m_Data(AData), m_OnError(AErrorHandler), m_Valid(false)
{ 
	// 2010.08.28 DLL インジェクション対策としてEXEのフォルダに移動する
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();
	
	CLSID ClassID;
	if(CLSIDFromProgID(AEngine, &ClassID) != S_OK)
		Error(L"指名のスクリプトエンジンが見つかりません");
	else
	{
		if(CoCreateInstance(ClassID, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, reinterpret_cast<void **>(&m_Engine)) != S_OK)
			Error(L"指名のスクリプトエンジンが作成できません");
		else
		{
			IActiveScriptSite *Site = new CWSHSite(this);
			if(m_Engine->SetScriptSite(Site) != S_OK)
			{
				delete Site;
				Error(L"サイトを登録できません");
			}
			else
			{
				m_Valid = true;
			}
		}
	}
}

CWSHClient::~CWSHClient()
{
	//インタフェースオブジェクトを解放
	for( ListIter it = m_IfObjArr.begin(); it != m_IfObjArr.end(); it++ ){
		(*it)->Release();
	}
	
	if(m_Engine != NULL) 
		m_Engine->Release();
}

void CWSHClient::Execute(wchar_t const *AScript)
{
	IActiveScriptParse *Parser;
	if(m_Engine->QueryInterface(IID_IActiveScriptParse, reinterpret_cast<void **>(&Parser)) != S_OK)
		Error(L"パーサを取得できません");
	else 
	{
		if(Parser->InitNew() != S_OK)
			Error(L"初期化できません");
		else
		{
			bool bAddNamedItemError = false;

			for( ListIter it = m_IfObjArr.begin(); it != m_IfObjArr.end(); it++ )
			{
				DWORD dwFlag = SCRIPTITEM_ISVISIBLE;

				if( (*it)->IsGlobal() ){ dwFlag |= SCRIPTITEM_GLOBALMEMBERS; }

				if(m_Engine->AddNamedItem( (*it)->Name(), dwFlag ) != S_OK)
				{
					bAddNamedItemError = true;
					Error(L"オブジェクトを渡せなかった");
					break;
				}
			}
			if( !bAddNamedItemError )
			{
				if(m_Engine->SetScriptState(SCRIPTSTATE_STARTED) != S_OK)
					Error(L"状態変更エラー");
				else
				{
					if(Parser->ParseScriptText(AScript, 0, 0, 0, 0, 0, SCRIPTTEXT_ISVISIBLE, 0, 0) != S_OK)
						Error(L"実行に失敗しました");
				}
			}
		}
		Parser->Release();
	}
	m_Engine->Close();
}

void CWSHClient::Error(BSTR Description, BSTR Source)
{
	if(m_OnError != NULL)
		m_OnError(Description, Source, m_Data);
}

void CWSHClient::Error(wchar_t* Description)
{
	BSTR S = SysAllocString(L"WSH");
	BSTR D = SysAllocString(Description);
	Error(D, S);
	SysFreeString(S);
	SysFreeString(D);
}

//インタフェースオブジェクトの追加
void CWSHClient::AddInterfaceObject( CIfObj* obj )
{
	if( !obj ) return;
	m_IfObjArr.push_back( obj );
	obj->m_Owner = this;
	obj->AddRef();
}


/////////////////////////////////////////////
/*!
	MacroCommand→CWSHIfObj.cppへ移動
	CWSHMacroManager →　CWSHManager.cppへ移動

*/

