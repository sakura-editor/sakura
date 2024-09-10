/*!	@file
	@brief エディタプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#define SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#pragma once

#include "_main/CProcess.h"
#include "extmodule/CMigemo.h"
#include "CEditApp.h"
#include "util/design_template.h"
class CEditWnd;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはCEditWndクラスのインスタンスを作る。
 */
class CNormalProcess final : public CProcess {

	using Me = CNormalProcess;
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;

public:
	//コンストラクタ・デストラクタ
	explicit CNormalProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow) noexcept;
	~CNormalProcess() override = default;


protected:
	//プロセスハンドラ
	bool InitializeProcess() override;
	bool MainLoop() override;
	void OnExitProcess() override;

protected:
	//実装補助
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka
	void OpenFiles(HWND hwnd);

private:
	CEditApp*	m_pcEditApp = nullptr;
	CMigemo		m_cMigemo;
};

using CEditorProcess = CNormalProcess;

#endif /* SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_ */
