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
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理

#define NEVER_USED_PARAM(p) ((void)p)

CEditView*		CPPA::m_pcEditView = NULL;
DLLSHAREDATA*	CPPA::m_pShareData = NULL;

CPPA::CPPA()
{
}

CPPA::~CPPA()
{
	//	Apr. 15, 2002 genta cleanup処理追加
	if( IsAvailable()){
		DeinitDll();
	}
}


//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
void CPPA::Execute(CEditView* pcEditView )
{
	m_pcEditView = pcEditView;
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

	//	Apr. 15, 2002 genta constを付けた
	//	アドレスの入れ場所はオブジェクトに依存するので
	//	static配列にはできない。
	const ImportTable table[] = 
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

	//	Apr. 15, 2002 genta
	//	CDllHandlerの共通関数化した
	if( ! RegisterEntries(table) )
		return 1;

	SetStrFunc((void*)CPPA::stdStrFunc);
	SetProc((void*)CPPA::stdProc);

	/* SAKURAエディタ用独自関数を準備 */

	int i;
	// コマンドに置き換えられない関数 ＝ PPA無しでは使えない。。。
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName) + 256 ];	//	256文字分プラス
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i] );	//	m_pszDataを作成する
		SetDefProc(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData);
	}

	// コマンドに置き換えられる関数 ＝ PPA無しでも使える。
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName) + 256 ];	//	256文字分プラス
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoArr[i] );	//	m_pszDataを作成する
		SetDefProc(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData);
	}
	return 0; 
}

void CPPA::GetDeclarations( MacroFuncInfo& cMacroFuncInfo )
{
	char szBuffer[1024];	//	最長1024でどうだ？

	char szType[20];			//	procedure/function用バッファ
	char szReturn[20];			//	戻り値型用バッファ
	if (cMacroFuncInfo.m_varResult == VT_EMPTY){
		strcpy( szType, "procedure" );
		szReturn[0] = '\0';
	}
	else {
		strcpy( szType, "function" );
		if (cMacroFuncInfo.m_varResult == VT_BSTR){
			strcpy( szReturn, ": string" );
		}
		else {
			szReturn[0] = '\0';
		}
	}
	
	char szArguments[4][20];	//	引数用バッファ
	int i;
	for (i=0; i<4; i++){
		if ( cMacroFuncInfo.m_varArguments[i] == VT_EMPTY ){
			break;
		}
		if ( cMacroFuncInfo.m_varArguments[i] == VT_BSTR ){
			strcpy( szArguments[i], "s0: string" );
			szArguments[i][1] = '0' + i;
		}
		else if ( cMacroFuncInfo.m_varArguments[i] == VT_I4 ){
			strcpy( szArguments[i], "i0: Integer" );
			szArguments[i][1] = '0' + i;
		}
		else {
			strcpy( szArguments[i], "u0: Unknown" );
		}
	}
	if ( i > 0 ){	//	引数があったとき
		int j;
		char szArgument[80];
		szArgument[0] = '\0';
		for (j=0; j<i; j++){
			strcat( szArgument, szArguments[j] );
			if ( j < i-1 ){
				strcat( szArgument, "; " );
			}
		}
		sprintf( szBuffer, "%s S_%s(%s)%s; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szArgument,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	else {
		sprintf( szBuffer, "%s S_%s%s; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	cMacroFuncInfo.m_pszData = new char [ lstrlen( szBuffer ) + 1 ];
	strcpy( cMacroFuncInfo.m_pszData, szBuffer );
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
