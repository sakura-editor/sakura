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
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CPPA_FB41BBAE_DFBC_449D_9342_5D9424CFE086_H_
#define SAKURA_CPPA_FB41BBAE_DFBC_449D_9342_5D9424CFE086_H_
#pragma once

#include "extmodule/CPpaDllImpl.h"

#include <ObjIdl.h>  // VARIANT等
#include <stdio.h>

#include <string>

#include "macro/CSMacroMgr.h"
#include "extmodule/CDllHandler.h"
#include "mem/CNativeA.h"

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
class CPPA : public CPpaDllImpl {
public:
	CPPA();
	virtual ~CPPA();

	//! PPAメッセージを取得する
	const char* GetLastMessage(void) const { return m_szMsg; }

	static std::string& GetDeclarations( const MacroFuncInfo& cMacroFuncInfo, std::string& buffer );

	void SetDefProcByFuncInfo(const MacroFuncInfo& cMacroFuncInfo, std::string& buffer);

protected:
	bool InitDllImp() override;

public:
	// exported
	//	2007.07.22 genta : flags追加
	bool Execute(class CEditView* pcEditView, int flags );

	// SetSourceはpublicとして公開する
	using CPpaDllImpl::SetSource;

private:
	// コールバックプロシージャ群
	static void __stdcall stdStrObj(const char* ObjName, int Index, BYTE GS_Mode, int* Err_CD, char** Value);	//	2003.06.01 Moca

	static void __stdcall stdProc( const char* FuncName, const int Index, const char* Argument[], const int ArgSize, int* Err_CD);
	static void __stdcall stdIntFunc( const char* FuncName, const int Index,
		const char* Argument[], const int ArgSize, int* Err_CD, int* ResultValue); // 2002.02.24 Moca
	static void __stdcall stdStrFunc( const char* FuncName, const int Index, const char* Argument[], const int ArgSize, int* Err_CD, char** ResultValue);
	static bool CallHandleFunction( const int Index, const char* Arg[], int ArgSize, VARIANT* Result ); // 2002.02.24 Moca

public:
	static void __stdcall stdError( int Err_CD, const char* Err_Mes );	//	2003.06.01 Moca

private:
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
#endif /* SAKURA_CPPA_FB41BBAE_DFBC_449D_9342_5D9424CFE086_H_ */
