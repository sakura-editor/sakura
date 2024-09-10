/*!	@file
	@brief コントロールプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#define SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#pragma once

#include "_main/CProcess.h"
#include "_main/CControlTray.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはCControlTrayクラスのインスタンスを作る。
 */
class CControlProcess final : public CProcess {

	using Me = CControlProcess;
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;

public:
	explicit CControlProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine) noexcept;
	~CControlProcess() override;

protected:
	bool InitializeProcess() override;
	bool MainLoop() override;
	void OnExitProcess() override;

private:
	HANDLE			m_hMutex              = nullptr;    //!< アプリケーション実行検出用ミューテックス
	HANDLE			m_hMutexCP            = nullptr;    //!< コントロールプロセスミューテックス
	HANDLE			m_hEventCPInitialized = nullptr;    //!< コントロールプロセス初期化完了イベント 2006.04.10 ryoji
	CControlTray*	m_pcTray              = nullptr;
};

#endif /* SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_ */
