/*!	@file
	@brief PPA Library Handler

	PPA.DLLを利用するためのインターフェース

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2001, YAZAKI, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, genta, Moca

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
/*
PPA(Poor-Pascal for Application)はDelphi/C++Builder用のPascalインタプリタコンポーネントです。
*/

#ifndef _DLL_CPPA_H_
#define _DLL_CPPA_H_

#include <ObjIdl.h>  // VARIANT等
#include <stdio.h>
#include "macro/CSMacroMgr.h"
#include "extmodule/CDllHandler.h"

#define PPADLL_VER 123

/*
PPA(Poor-Pascal for Application)はDelphi/C++Builder用の
Pascalインタプリタコンポーネントです。
アプリケーションにマクロ機能を搭載する事を目的に作成されています。
*/

/*!
	@brief PPA.DLL をサポートするクラス

	DLLの動的ロードを行うため、DllHandlerを継承している。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CPPA : public CDllImp {
public:
	CPPA();
	virtual ~CPPA();

	const char* GetVersion(){		//!< DLLのバージョン情報を取得。m_szMsgを壊す
		if ( IsAvailable() ){
			auto_sprintf(m_szMsg, "PPA.DLL Version %d.%d", m_fnGetVersion() / 100, m_fnGetVersion() % 100);
			return m_szMsg;
		}
		return "";
	}

	//! PPAメッセージを取得する
	const char* GetLastMessage(void) const { return m_szMsg; }

	//	Jun. 16, 2003 genta 引数追加
	static char* GetDeclarations( const MacroFuncInfo&, char* buf );

protected:
	//	Jul. 5, 2001 genta インターフェース変更に伴う引数追加
public:
	virtual LPCTSTR GetDllNameImp(int nIndex);
protected:
	virtual bool InitDllImp();

private:
	//	DLL Interfaceの受け皿
	typedef void (WINAPI *PPA_Execute)();
	typedef void (WINAPI *PPA_SetSource) (const char* ss);
	typedef void (WINAPI *PPA_SetDeclare)(const char* ss);
	typedef void (WINAPI *PPA_SetDefProc)(const char* ss);
	typedef void (WINAPI *PPA_SetDefine) (const char* ss);
	typedef void (WINAPI *PPA_AddIntVar) (const char*, int, int);
	typedef void (WINAPI *PPA_AddStrVar) (const char*, const char*, int);
	typedef void (WINAPI *PPA_SetIntFunc)(void* p);
	typedef void (WINAPI *PPA_SetStrFunc)(void* p);
	typedef void (WINAPI *PPA_SetProc)   (void* p);
	typedef void (WINAPI *PPA_SetErrProc)(void* p);
	typedef void (WINAPI *PPA_Abort)     ();
	typedef int  (WINAPI *PPA_GetVersion)();
	typedef void (WINAPI *PPA_DeleteVar) (const char*);
	typedef int  (WINAPI *PPA_GetArgInt) (int);
	typedef char*(WINAPI *PPA_GetArgStr) (int);
	typedef char*(WINAPI *PPA_GetArgBStr)(int);
	typedef void (WINAPI *PPA_SetStrObj) (void* proc);
	typedef void (WINAPI *PPA_SetIntObj) (void* proc);
	typedef void (WINAPI *PPA_AddIntObj) (const char*, int, BOOL, int);
	typedef void (WINAPI *PPA_AddStrObj) (const char*, const char*, BOOL, int);
	typedef int  (WINAPI *PPA_GetIntVar) (const char* ss);
	typedef char*(WINAPI *PPA_GetStrVar) (const char* ss);
	typedef char*(WINAPI *PPA_GetBStrVar)(const char* ss);
	typedef BOOL (WINAPI *PPA_SetIntVar) (const char*, int);
	typedef BOOL (WINAPI *PPA_SetStrVar) (const char*, const char*);

	// 以下は PPA.DLL Version 1.20 で追加された関数 --
	#if PPADLL_VER >= 120
	typedef void   (WINAPI *PPA_AddRealVar)(const char*, double, BOOL);
	typedef void   (WINAPI *PPA_SetRealObj)(void* p);
	typedef void   (WINAPI *PPA_AddRealObj)(const char*, double, BOOL, LONG);
	typedef double (WINAPI *PPA_GetRealVar)(const char*);
	typedef BOOL   (WINAPI *PPA_SetRealVar)(const char*, double);
	typedef void   (WINAPI *PPA_SetRealFunc)(void* p);
	typedef DWORD  (WINAPI *PPA_GetArgReal)(int);
	#endif // PPADLL_VER >= 120

	// 以下は PPA.DLL Version 1.23 で追加された関数 --
	#if PPADLL_VER >= 123
	typedef BYTE (WINAPI *PPA_IsRunning)();
	typedef void (WINAPI *PPA_SetFinishProc)(void* p);	//	2003.06.01 Moca
	#endif // PPADLL_VER >= 123

	PPA_Execute    m_fnExecute;
	PPA_SetSource  m_fnSetSource;
	PPA_SetDeclare m_fnSetDeclare;
	PPA_SetDefProc m_fnSetDefProc;
	PPA_SetDefine  m_fnSetDefine;
	PPA_AddIntVar  m_fnAddIntVar;
	PPA_AddStrVar  m_fnAddStrVar;
	PPA_SetIntFunc m_fnSetIntFunc;
	PPA_SetStrFunc m_fnSetStrFunc;
	PPA_SetProc    m_fnSetProc;
	PPA_SetErrProc m_fnSetErrProc;
	PPA_Abort      m_fnAbort;
	PPA_GetVersion m_fnGetVersion;
	PPA_DeleteVar  m_fnDeleteVar;
	PPA_GetArgInt  m_fnGetArgInt;
	PPA_GetArgStr  m_fnGetArgStr;
	PPA_GetArgBStr m_fnGetArgBStr;
	PPA_SetStrObj  m_fnSetStrObj;
	PPA_SetIntObj  m_fnSetIntObj;
	PPA_AddIntObj  m_fnAddIntObj;
	PPA_AddStrObj  m_fnAddStrObj;
	PPA_GetIntVar  m_fnGetIntVar;
	PPA_GetStrVar  m_fnGetStrVar;
	PPA_GetBStrVar m_fnGetBStrVar;
	PPA_SetIntVar  m_fnSetIntVar;
	PPA_SetStrVar  m_fnSetStrVar;

#if PPADLL_VER >= 120
	PPA_AddRealVar  m_fnAddRealVar;
	PPA_SetRealObj  m_fnSetRealObj;
	PPA_AddRealObj  m_fnAddRealObj;
	PPA_GetRealVar  m_fnGetRealVar;
	PPA_SetRealVar  m_fnSetRealVar;
	PPA_SetRealFunc m_fnSetRealFunc;
	PPA_GetArgReal  m_fnGetArgReal;
#endif

#if PPADLL_VER >= 123
	PPA_IsRunning m_fnIsRunning;
	PPA_SetFinishProc m_fnSetFinishProc;	//	2003.06.01 Moca
#endif

public:
	// exported
	//	2007.07.22 genta : flags追加
	bool Execute(class CEditView* pcEditView, int flags );
	void SetSource(const char* ss)
		{ m_fnSetSource(ss); }
	void SetDeclare(const char* ss)
		{ m_fnSetDeclare(ss); }
	void SetDefProc(const char* ss)
		{ m_fnSetDefProc(ss); }
	void SetDefine(const char* ss)
		{ m_fnSetDefine(ss); }
	void AddIntVar(const char* lpszDef, int nVal, int nCnst)
		{ m_fnAddIntVar(lpszDef, nVal, nCnst); }
	void AddStrVar(const char* lpszDef, const char* lpszVal, int nCnst)
		{ m_fnAddStrVar(lpszDef, lpszVal, nCnst); }
	void SetIntFunc(void* proc)
		{ m_fnSetIntFunc(proc); }
	void SetStrFunc(void* proc)
		{ m_fnSetStrFunc(proc); }
	void SetProc(void* proc)
		{ m_fnSetProc(proc); }
	void SetErrProc(void* proc)
		{ m_fnSetErrProc(proc); }
	void Abort()
		{ m_fnAbort(); }
//	int  GetVersion()
//		{ return m_fnGetVersion(); }
	void DeleteVar(const char* ss)
		{ m_fnDeleteVar(ss); }
	int  GetArgInt(int index)
		{ return m_fnGetArgInt(index); }
	char* GetArgStr(int index)
		{ return m_fnGetArgStr(index); }
	char* GetArgBStr(int index)
		{ return m_fnGetArgBStr(index); }
	void SetStrObj(void* proc)
		{ m_fnSetStrObj(proc); }
	void SetIntObj(void* proc)
		{ m_fnSetIntObj(proc); }
	void AddIntObj(const char* ss, int def, BOOL read, int index)
		{ m_fnAddIntObj(ss, def, read, index); }
	void AddStrObj(const char* ss, const char* def, BOOL read, int index)
		{ m_fnAddStrObj(ss, def, read, index); }
	int  GetIntVar(const char* ss)
		{ return m_fnGetIntVar(ss); }
	char* GetStrVar(const char* ss)
		{ return m_fnGetStrVar(ss); }
	char* GetBStrVar(const char* ss)
		{ return m_fnGetBStrVar(ss); }
	BOOL SetIntVar(const char* ss, int val)
		{ return m_fnSetIntVar(ss, val); }
	BOOL SetStrVar(const char* ss, const char* val)
		{ return m_fnSetStrVar(ss, val); }

#if PPADLL_VER >= 120
	void AddRealVar(const char* ss, double val, BOOL cnst)
		{ m_fnAddRealVar(ss, val, cnst); }
	void SetRealObj(void* proc)
		{ m_fnSetRealObj(proc); }
	void AddRealObj(const char* ss, double val, BOOL read, LONG index)
		{ m_fnAddRealObj(ss, val, read, index); }
	double GetRealVar(const char* ss)
		{ return m_fnGetRealVar(ss); }
	BOOL SetRealVar(const char* ss, double val)
		{ return m_fnSetRealVar(ss, val); }
	void SetRealFunc(void* proc)
		{ m_fnSetRealFunc(proc); }
	DWORD GetArgReal(int index)
		{ return m_fnGetArgReal(index); }
#endif

#if PPADLL_VER >= 123
	BOOL IsRunning()
		{ return (BOOL)m_fnIsRunning(); }
	void SetFinishProc(void* proc)	//	2003.06.01 Moca
		{ m_fnSetFinishProc(proc); }
#endif

private:
	// コールバックプロシージャ群
	static void __stdcall stdStrObj(const char*, int, BYTE, int*, char**);	//	2003.06.01 Moca

	static void __stdcall stdProc( const char* FuncName, const int Index, const char* Argument[], const int ArgSize, int* Err_CD);
	static void __stdcall stdIntFunc( const char* FuncName, const int Index,
		const char* Argument[], const int ArgSize, int* Err_CD, int* ResultValue); // 2002.02.24 Moca
	static void __stdcall stdStrFunc( const char* FuncName, const int Index, const char* Argument[], const int ArgSize, int* Err_CD, char** ResultValue);
	static bool CallHandleFunction( const int Index, const char* Arg[], int ArgSize, VARIANT* Result ); // 2002.02.24 Moca

	static void __stdcall stdError( int, const char* );	//	2003.06.01 Moca
	static void __stdcall stdFinishProc();	//	2003.06.01 Moca

	//	メンバ変数
	char		m_szMsg[80];		//!< CPPAからのメッセージを保持する

	//	2007.07.26 genta : PPAのネストを許容するために，別データ構造とする．
	
	struct PpaExecInfo {
		CNativeA		m_cMemRet;		//!< コールバックからDLLに渡す文字列を保持
		CEditView*		m_pcEditView;	//	2003.06.01 Moca
		DLLSHAREDATA*	m_pShareData;	//	2003.06.01 Moca
		bool			m_bError;		//!< エラーが2回表示されるのを防ぐ	2003.06.01 Moca
		CNativeA		m_cMemDebug;	//!< デバッグ用変数UserErrorMes 2003.06.01 Moca
		/** オプションフラグ
		
			CEditView::HandleCommand()にコマンドと一緒に渡すことで
			コマンドの素性を教える．
		*/
		int				m_commandflags;	//!< 
	};
	//	2007.07.26 genta : 現在実行中のインスタンス
	static PpaExecInfo* m_CurInstance;
	//PPAの多重起動禁止 2008.10.22 syat
	static bool				m_bIsRunning;	//!< PPAが同時実行されるのを防ぐ


/*	関数名はCMacroが持つ。
	static struct MacroFuncInfo	S_Table[];
	static int					m_nFuncNum;	//	SAKURAエディタ用関数の数
*/
};

#endif

