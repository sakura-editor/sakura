#include "StdAfx.h"
#include "debug/Debug3.h"
#include "util/module.h"

#ifdef USE_DEBUGMON

static HINSTANCE hDll=NULL;
static void Load()
{
	if(!hDll){
		hDll=LoadLibraryExedir(_T("DebugMonitorLib.dll"));
	}
}

typedef __declspec(dllexport) int (*FN_DebugMonitor_Output)(const wchar_t* szInstanceId, const wchar_t* szText);
int DebugMonitor_Output(const wchar_t* szInstanceId, const wchar_t* szText)
{
	Load();
	if(!hDll)return -1;

	static FN_DebugMonitor_Output f=NULL;
	if(!f){
		f=(FN_DebugMonitor_Output)GetProcAddress(hDll,"DebugMonitor_Output");
	}
	if(!f)return -1;

	return f(NULL,to_wchar(szText));
}

typedef __declspec(dllexport) LPCWSTR (*FN_GetWindowsMessageName)(UINT msg);
LPCWSTR GetWindowsMessageName(UINT msg)
{
	Load();
	if(!hDll)return L"?";

	static FN_GetWindowsMessageName f=NULL;
	if(!f){
		f=(FN_GetWindowsMessageName)GetProcAddress(hDll,"GetWindowsMessageName");
	}
	if(!f)return L"?";

	return f(msg);
}

#endif

