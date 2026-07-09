/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/UiaTestSuite.hpp"

namespace window {

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::SetUpUia()
{
	// OLEを初期化する
	EXPECT_HRESULT_SUCCEEDED(::OleInitialize(nullptr));

	// UI Automationオブジェクトを作成する
	EXPECT_HRESULT_SUCCEEDED(m_pAutomation.CreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER));
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::TearDownUia()
{
	// UI Automationオブジェクトを破棄する
	m_pAutomation = nullptr;

	// OLEをシャットダウンする
	::OleUninitialize();
}

} // namespace window
