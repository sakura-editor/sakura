/*!	@file
	@brief PPA Library Handler

	PPA.DLLを利用するためのインターフェース

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2001, YAZAKI

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

#include "CPPA.h"
#include "CEditView.h"
#include "Funccode.h"
#include "debug.h"
#include "CMacro.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "CShareData.h"
#include "etc_uty.h"
#include "CEditApp.h"

#define NEVER_USED_PARAM(p) ((void)p)

CEditView*		CPPA::m_pcEditView = NULL;
DLLSHAREDATA*	CPPA::m_pShareData = NULL;
//CShareData*		CPPA::m_cShareData = NULL;

CPPA::CPPA()
{
}

CPPA::~CPPA()
{
}


void CPPA::Execute(CEditView* pcEditView )
{
	m_pcEditView = pcEditView;
//	m_cShareData = &m_pcEditView->m_cShareData;
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_fnExecute();
}

char *
CPPA::GetDllName( char* str )
{
	return "PPA.DLL";
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval 0 成功
	@retval 1 アドレス取得に失敗
*/
int CPPA::InitDll()
{
	/* PPA.DLLが持っている関数を準備 */
	//	アドレス取得
	struct ImportTable 
	{
		void* proc;
		const char* name;
	};

	ImportTable table[] = 
	{
		{ &m_fnExecute,		"Execute" },
		{ &m_fnSetDeclare,	"SetDeclare" },
		{ &m_fnSetSource,	"SetSource" },
		{ &m_fnSetDefProc,	"SetDefProc" },
		{ &m_fnSetDefine,	"SetDefine" },
		{ &m_fnSetIntFunc,	"SetIntFunc" },
		{ &m_fnSetStrFunc,	"SetStrFunc" },
		{ &m_fnSetProc,		"SetProc" },
		{ &m_fnSetErrProc,	"SetErrProc" },
		{ &m_fnAbort,		"ppaAbort" },
		{ &m_fnGetVersion,	"GetVersion" },
		{ &m_fnDeleteVar,	"DeleteVar" },
		{ &m_fnGetArgInt,	"GetArgInt" },
		{ &m_fnGetArgStr,	"GetArgStr" },
		{ &m_fnGetArgBStr,	"GetArgBStr" },
		{ &m_fnGetIntVar,	"GetIntVar" },
		{ &m_fnGetStrVar,	"GetStrVar" },
		{ &m_fnGetBStrVar,	"GetBStrVar" },
		{ &m_fnSetIntVar,	"SetIntVar" },
		{ &m_fnSetStrVar,	"SetStrVar" },
		{ &m_fnAddIntObj,	"AddIntObj" },
		{ &m_fnAddStrObj,	"AddStrObj" },
		{ &m_fnAddIntVar,	"AddIntVar" },
		{ &m_fnAddStrVar,	"AddStrVar" },
		{ &m_fnSetIntObj,	"SetIntObj" },
		{ &m_fnSetStrObj,	"SetStrObj" },

#if PPADLL_VER >= 120
		{ &m_fnAddRealVar,	"AddRealVar" },
		{ &m_fnSetRealObj,	"SetRealObj" },
		{ &m_fnAddRealObj,	"AddRealObj" },
		{ &m_fnGetRealVar,	"GetRealVar" },
		{ &m_fnSetRealVar,	"SetRealVar" },
		{ &m_fnSetRealFunc,	"SetRealFunc" },
		{ &m_fnGetArgReal,	"GetArgReal" },
#endif

#if PPADLL_VER >= 123
		{ &m_fnIsRunning, "IsRunning" },
#endif

		{ NULL, 0 }
	};

	int i;
	for (i=0; table[i].proc!=NULL; i++) 
	{
		FARPROC proc;
		if ((proc = ::GetProcAddress(GetInstance(), table[i].name)) == NULL) 
		{
//			Free();
			return 1;
		}
		*((FARPROC*)table[i].proc) = proc;
	}

	SetStrFunc((void*)CPPA::stdStrFunc);
	SetProc((void*)CPPA::stdProc);

	/* SAKURAエディタ用独自関数を準備 */

	// コマンドに置き換えられない関数 ＝ PPA無しでは使えない。。。
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName) + 30 ];	//	30文字分プラス
		sprintf(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData, "%s index %d;", CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName, CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_nFuncID );
		SetDefProc(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData);
	}

	// コマンドに置き換えられる関数 ＝ PPA無しでも使える。
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName) + strlen(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncParam) + 30 ];	//	30文字分プラス
		sprintf(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData, "procedure %s%s; index %d;", CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName, CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncParam, CSMacroMgr::m_MacroFuncInfoArr[i].m_nFuncID);
		SetDefProc(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData);
	}
	return 0; 
}

/*!	
*/
int CPPA::DeinitDll( void )
{
	int i;
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++){
		if (CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData)
			delete CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData;
	}
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		if (CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData)
			delete CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData;
	}
	return 0;
}

//----------------------------------------------------------------------
void __stdcall CPPA::stdProc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD)
{
	NEVER_USED_PARAM(FuncName);

	*Err_CD = 0;
	CMacro::HandleCommand( m_pcEditView, Index, Argument, ArgSize );
}

//----------------------------------------------------------------------
static char g_ResultStr[4096];	//	作業用

void __stdcall CPPA::stdStrFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	char** ResultValue)
{
	NEVER_USED_PARAM(FuncName);

	*Err_CD = 0;
	switch ( Index ){
	case F_GETFILENAME:	//	ファイル名を返す
		strcpy(g_ResultStr, m_pcEditView->m_pcEditDoc->m_szFilePath);
		*ResultValue = g_ResultStr;
		break;
	}
}
