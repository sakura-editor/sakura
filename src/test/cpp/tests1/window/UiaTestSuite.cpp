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
	pcOleInit = std::make_unique<cxx::COleInit>();

	// 初期化に失敗した場合はテストを中止する
	ASSERT_TRUE(pcOleInit);

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
	pcOleInit = nullptr;
}

} // namespace window
