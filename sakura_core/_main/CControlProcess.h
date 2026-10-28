/*!	@file
	@brief コントロールプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#define SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#pragma once

#include "_main/CControlTray.h"
#include "_main/CProcess.h"
#include "_main/global.h"
#include "util/sync/MutexHandle.hpp"

#include <filesystem>
#include <memory>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはCControlTrayクラスのインスタンスを作る。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlProcess final : public CProcess {
private:
	using MutexHandle = cxx::MutexHandle;
	using CControlTrayHolder = std::unique_ptr<CControlTray>;

	using Base = CProcess;
	using Me = CControlProcess;

public:
	// コンストラクタは流用する
	using Base::Base;

	std::filesystem::path GetIniFileName() const override;

private:
	bool InitializeProcess(
		int nCmdShow [[maybe_unused]]
	) override;

	bool MainLoop() override;

	std::filesystem::path GetPrivateIniFileName(const std::wstring& exeIniPath, const std::wstring& filename) const;

	MutexHandle			m_hMutex{};		//!< アプリケーション実行検出用ミューテックス
	MutexHandle			m_hMutexCP{};	//!< コントロールプロセスミューテックス
	CControlTrayHolder	m_pcTray{};		//!< トレイウィンドウ
};

#endif /* SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_ */
