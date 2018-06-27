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

#include "StdAfx.h"
#include <process.h> // _beginthreadex
#ifdef __MINGW32__
#define INITGUID 1
#endif
#include <ObjBase.h>
#include <InitGuid.h>
#include <ShlDisp.h>
#include "macro/CWSH.h"
#include "macro/CIfObj.h"
#include "window/CEditWnd.h"
#include "util/os.h"
#include "util/module.h"
#include "util/window.h"	// BlockingHook
#include "dlg/CDlgCancel.h"
#include "sakura_rc.h"
#ifndef SCRIPT_E_REPORTED
#define	SCRIPT_E_REPORTED	0x80020101L	// ActivScp.h(VS2012)と同じ様な形に変更
#endif

#ifdef USE_JSCRIPT9
const GUID CLSID_JSScript9 =
{
	0x16d51579, 0xa30b, 0x4c8b, { 0xa2, 0x76, 0x0f, 0xf4, 0xdc, 0x41, 0xe7, 0x55 } 
};
#endif

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
	CWSHSite(CWSHClient *AClient): m_Client(AClient), m_RefCount(0)
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
	}

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
	}

	virtual HRESULT STDMETHODCALLTYPE GetDocVersionString( 
	    /* [out] */ BSTR *pbstrVersion) 
	{ 
#ifdef TEST
		cout << "GetDocVersionString" << endl;
#endif
		return E_NOTIMPL; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate( 
	    /* [in] */ const VARIANT *pvarResult,
	    /* [in] */ const EXCEPINFO *pexcepinfo) 
	{ 
#ifdef TEST
		cout << "OnScriptTerminate" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnStateChange( 
	    /* [in] */ SCRIPTSTATE ssScriptState) 
	{ 
#ifdef TEST
		cout << "OnStateChange" << endl;
#endif
		return S_OK; 
	}

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
			if(Info.bstrDescription == NULL) {
				Info.bstrDescription = SysAllocString(LSW(STR_ERR_CWSH09));
			}
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
	}

	virtual HRESULT STDMETHODCALLTYPE OnEnterScript() {
#ifdef TEST
		cout << "OnEnterScript" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnLeaveScript() {
#ifdef TEST
		cout << "OnLeaveScript" << endl;
#endif
		return S_OK; 
	}

	//	Sep. 15, 2005 FILE IActiveScriptSiteWindow実装
	virtual HRESULT __stdcall GetWindow(
	    /* [out] */ HWND *phwnd)
	{
		*phwnd = CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd();
		return S_OK;
	}

	//	Sep. 15, 2005 FILE IActiveScriptSiteWindow実装
	virtual HRESULT __stdcall EnableModeless(
	    /* [in] */ BOOL fEnable)
	{
		return S_OK;
	}
};

//implementation

CWSHClient::CWSHClient(const wchar_t *AEngine, ScriptErrorHandler AErrorHandler, void *AData): 
				m_OnError(AErrorHandler), m_Data(AData), m_Valid(false), m_Engine(NULL)
{ 
	// 2010.08.28 DLL インジェクション対策としてEXEのフォルダに移動する
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();
	
	CLSID ClassID;
	if(CLSIDFromProgID(AEngine, &ClassID) != S_OK)
		Error(LSW(STR_ERR_CWSH01));
	else
	{
#ifdef USE_JSCRIPT9
		if( 0 == wcscmp( AEngine, LTEXT("JScript") ) ){
			ClassID = CLSID_JSScript9;
		}
#endif
		if(CoCreateInstance(ClassID, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, reinterpret_cast<void **>(&m_Engine)) != S_OK)
			Error(LSW(STR_ERR_CWSH02));
		else
		{
			IActiveScriptSite *Site = new CWSHSite(this);
			if(m_Engine->SetScriptSite(Site) != S_OK)
			{
				delete Site;
				Error(LSW(STR_ERR_CWSH03));
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

// AbortMacroProcのパラメータ構造体
typedef struct {
	HANDLE hEvent;
	IActiveScript *pEngine;				//ActiveScript
	int nCancelTimer;
	CEditView *view;
} SAbortMacroParam;

// WSHマクロ実行を中止するスレッド
static unsigned __stdcall AbortMacroProc( LPVOID lpParameter )
{
	SAbortMacroParam* pParam = (SAbortMacroParam*) lpParameter;

	//停止ダイアログ表示前に数秒待つ
	if(::WaitForSingleObject(pParam->hEvent, pParam->nCancelTimer * 1000) == WAIT_TIMEOUT){
		//停止ダイアログ表示
		DEBUG_TRACE(_T("AbortMacro: Show Dialog\n"));

		MSG msg;
		CDlgCancel cDlgCancel;
		HWND hwndDlg = cDlgCancel.DoModeless(G_AppInstance(), NULL, IDD_MACRORUNNING);	// エディタビジーでも表示できるよう、親を指定しない
		// ダイアログタイトルとファイル名を設定
		::SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)GSTR_APPNAME);
		::SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_CMD),
			WM_SETTEXT, 0, (LPARAM)pParam->view->GetDocument()->m_cDocFile.GetFilePath());
		
		bool bCanceled = false;
		for(;;){
			DWORD dwResult = MsgWaitForMultipleObjects( 1, &pParam->hEvent, FALSE, INFINITE, QS_ALLINPUT );
			if(dwResult == WAIT_OBJECT_0){
				::SendMessage( cDlgCancel.GetHwnd(), WM_CLOSE, 0, 0 );
			}else if(dwResult == WAIT_OBJECT_0+1){
				while(::PeekMessage(&msg , NULL , 0 , 0, PM_REMOVE )){
					if(cDlgCancel.GetHwnd() != NULL && ::IsDialogMessage(cDlgCancel.GetHwnd(), &msg)){
					}else{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
			}else{
				//MsgWaitForMultipleObjectsに与えたハンドルのエラー
				break;
			}
			if(!bCanceled && cDlgCancel.IsCanceled()){
				DEBUG_TRACE(_T("Canceld\n"));
				bCanceled = true;
				cDlgCancel.CloseDialog( 0 );
			}
			if(cDlgCancel.GetHwnd() == NULL){
				DEBUG_TRACE(_T("Close\n"));
				break;
			}
		}

		DEBUG_TRACE(_T("AbortMacro: Try Interrupt\n"));
		pParam->pEngine->InterruptScriptThread(SCRIPTTHREADID_BASE, NULL, 0);
		DEBUG_TRACE(_T("AbortMacro: Done\n"));
	}

	DEBUG_TRACE(_T("AbortMacro: Exit\n"));
	return 0;
}


bool CWSHClient::Execute(const wchar_t *AScript)
{
	bool bRet = false;
	IActiveScriptParse *Parser;
	if(m_Engine->QueryInterface(IID_IActiveScriptParse, reinterpret_cast<void **>(&Parser)) != S_OK)
		Error(LSW(STR_ERR_CWSH04));
	else 
	{
		if(Parser->InitNew() != S_OK)
			Error(LSW(STR_ERR_CWSH05));
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
					Error(LSW(STR_ERR_CWSH06));
					break;
				}
			}
			if( !bAddNamedItemError )
			{
				//マクロ停止スレッドの起動
				SAbortMacroParam sThreadParam;
				sThreadParam.pEngine = m_Engine;
				sThreadParam.nCancelTimer = GetDllShareData().m_Common.m_sMacro.m_nMacroCancelTimer;
				sThreadParam.view = (CEditView*)m_Data;

				HANDLE hThread = NULL;
				unsigned int nThreadId = 0;
				if( 0 < sThreadParam.nCancelTimer ){
					sThreadParam.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
					hThread = (HANDLE)_beginthreadex( NULL, 0, AbortMacroProc, (LPVOID)&sThreadParam, 0, &nThreadId );
					DEBUG_TRACE(_T("Start AbortMacroProc 0x%08x\n"), nThreadId);
				}

				//マクロ実行
				if(m_Engine->SetScriptState(SCRIPTSTATE_STARTED) != S_OK)
					Error(LSW(STR_ERR_CWSH07));
				else
				{
					HRESULT hr = Parser->ParseScriptText(AScript, 0, 0, 0, 0, 0, SCRIPTTEXT_ISVISIBLE, 0, 0);
					if (hr == SCRIPT_E_REPORTED) {
					/*
						IActiveScriptSite->OnScriptErrorに通知済み。
						中断メッセージが既に表示されてるはず。
					*/
					} else if(hr != S_OK) {
						Error(LSW(STR_ERR_CWSH08));
					} else {
						bRet = true;
					}
				}

				if( 0 < sThreadParam.nCancelTimer ){
					::SetEvent(sThreadParam.hEvent);

					//マクロ停止スレッドの終了待ち
					DEBUG_TRACE(_T("Waiting for AbortMacroProc to finish\n"));
					::WaitForSingleObject(hThread, INFINITE); 
					::CloseHandle(hThread);
					::CloseHandle(sThreadParam.hEvent);
				}
			}
		}
		Parser->Release();
	}
	m_Engine->Close();
	return bRet;
}

void CWSHClient::Error(BSTR Description, BSTR Source)
{
	if(m_OnError != NULL)
		m_OnError(Description, Source, m_Data);
}

void CWSHClient::Error(const wchar_t* Description)
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

