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

#include <filesystem>

#include "global.h"
#include "CProcess.h"

class CControlTray;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはCControlTrayクラスのインスタンスを作る。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlProcess final : public CProcess {
public:
	CControlProcess( HINSTANCE hInstance, LPCWSTR lpCmdLine ) : 
		CProcess( hInstance, lpCmdLine ),
		// 2006.04.10 ryoji 同期オブジェクトのハンドルを初期化
		m_hMutex( NULL ),
		m_hMutexCP( NULL ),
		m_hEventCPInitialized( NULL ),
		m_pcTray( nullptr )
	{}

	~CControlProcess() override;

protected:
	bool InitializeProcess() override;
	bool MainLoop() override;
	void OnExitProcess() override;

private:
	HANDLE			m_hMutex;				//!< アプリケーション実行検出用ミューテックス
	HANDLE			m_hMutexCP;				//!< コントロールプロセスミューテックス
	HANDLE			m_hEventCPInitialized;	//!< コントロールプロセス初期化完了イベント 2006.04.10 ryoji
	CControlTray*	m_pcTray;
};

#endif /* SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_ */
