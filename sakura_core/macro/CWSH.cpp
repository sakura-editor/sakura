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
				Info.bstrDescription = SysAllocString(L"マクロの実行を中断しました。");
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

// AbortMacroProcのパラメータ構造体
typedef struct {
	CRITICAL_SECTION cs;				//同期用クリティカルセクション
	bool bIsMacroRunning;				//マクロ実行中フラグ
	bool bIsAbortThreadRunning;			//AbortMacroProc実行中フラグ
	IActiveScript *pEngine;				//ActiveScript
	HWND hwndDlgCancel;
	int nCancelTimer;
	CEditView *view;
} SAbortMacroParam;

// WSHマクロ実行を中止するスレッド
static unsigned __stdcall AbortMacroProc( LPVOID lpParameter )
{
	SAbortMacroParam* pParam = (SAbortMacroParam*) lpParameter;

	//停止ダイアログ表示前に数秒待つ
	int i;
	for( i=0; i < pParam->nCancelTimer * 10; i++ ){
		::EnterCriticalSection(&pParam->cs);
		if( ! pParam->bIsMacroRunning ){
			::LeaveCriticalSection(&pParam->cs);
			break;
		}
		::LeaveCriticalSection(&pParam->cs);
		::Sleep(100);
	}

	//停止ダイアログ表示
	if( pParam->bIsMacroRunning ){
		DEBUG_TRACE(_T("AbortMacro: Show Dialog\n"));

		MSG msg;
		CDlgCancel* pcDlgCancel = new CDlgCancel;
		pcDlgCancel->DoModeless(G_AppInstance(), NULL, IDD_MACRORUNNING);	// エディタビジーでも表示できるよう、親を指定しない
		pParam->hwndDlgCancel = pcDlgCancel->GetHwnd();
		// ダイアログタイトルとファイル名を設定
		::SendMessage(pParam->hwndDlgCancel, WM_SETTEXT, 0, (LPARAM)GSTR_APPNAME);
		::SendMessage(GetDlgItem(pParam->hwndDlgCancel, IDC_STATIC_CMD),
			WM_SETTEXT, 0, (LPARAM)pParam->view->GetDocument()->m_cDocFile.GetFilePath());
		
		while (GetMessage(&msg , NULL , 0 , 0 )) {
			if (pcDlgCancel->IsCanceled() || pcDlgCancel->m_nShowCmd != SW_SHOW) {
				break;
			}
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				::SendMessage( pcDlgCancel->GetHwnd(), WM_CLOSE, 0, 0 );
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		::EnterCriticalSection( &pParam->cs );
		if( pcDlgCancel->IsCanceled() ){
			if( pParam->bIsMacroRunning ){
				DEBUG_TRACE(_T("AbortMacro: Try Interrupt\n"));
				pParam->pEngine->InterruptScriptThread(SCRIPTTHREADID_BASE, NULL, 0);
				DEBUG_TRACE(_T("AbortMacro: Done\n"));
			}
		}
		pParam->hwndDlgCancel = NULL;
		::LeaveCriticalSection( &pParam->cs );
		pcDlgCancel->DeleteAsync();
	}

	DEBUG_TRACE(_T("AbortMacro: Exit\n"));
	pParam->bIsAbortThreadRunning = false;
	return 0;
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
				//マクロ停止スレッドの起動
				SAbortMacroParam sThreadParam;
				::InitializeCriticalSection(&sThreadParam.cs);
				sThreadParam.bIsMacroRunning = true;
				sThreadParam.bIsAbortThreadRunning = true;
				sThreadParam.pEngine = m_Engine;
				sThreadParam.hwndDlgCancel = NULL;
				sThreadParam.nCancelTimer = GetDllShareData().m_Common.m_sMacro.m_nMacroCancelTimer;
				sThreadParam.view = (CEditView*)m_Data;

				unsigned int nThreadId;
				HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, AbortMacroProc, (LPVOID)&sThreadParam, 0, &nThreadId );
				DEBUG_TRACE(_T("Start AbortMacroProc 0x%08x\n"), nThreadId);

				//マクロ実行
				if(m_Engine->SetScriptState(SCRIPTSTATE_STARTED) != S_OK)
					Error(L"状態変更エラー");
				else
				{
					if(Parser->ParseScriptText(AScript, 0, 0, 0, 0, 0, SCRIPTTEXT_ISVISIBLE, 0, 0) != S_OK)
						Error(L"実行に失敗しました");
				}

				::EnterCriticalSection(&sThreadParam.cs);
				sThreadParam.bIsMacroRunning = false;	//マクロ実行中フラグを落とす
				//マクロ停止ダイアログが表示されていれば閉じる
				if ( sThreadParam.hwndDlgCancel ) {
					::SendMessage( sThreadParam.hwndDlgCancel, WM_CLOSE, 0, 0 );
				}
				::LeaveCriticalSection(&sThreadParam.cs);

				//マクロ停止スレッドの終了待ち
				DEBUG_TRACE(_T("Waiting for AbortMacroProc to finish\n"));
				for (;;) {
					::EnterCriticalSection(&sThreadParam.cs);
					if( ! sThreadParam.bIsAbortThreadRunning ){
						::LeaveCriticalSection(&sThreadParam.cs);
						sThreadParam.pEngine = NULL;
						::DeleteCriticalSection(&sThreadParam.cs);
						break;
					}
					::LeaveCriticalSection(&sThreadParam.cs);
					::Sleep(50);
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

