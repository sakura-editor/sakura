/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "debug/Debug3.h"
#include "util/module.h"

#ifdef USE_DEBUGMON

static HINSTANCE hDll=NULL;
static void Load()
{
	if(!hDll){
		hDll=LoadLibraryExedir(L"DebugMonitorLib.dll");
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

	return f(NULL,szText);
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
