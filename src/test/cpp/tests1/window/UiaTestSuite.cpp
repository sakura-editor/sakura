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
	// UI Automationオブジェクトを作成する
	_com_util::CheckError(m_pAutomation.CreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER));
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::TearDownUia()
{
	// UI Automationオブジェクトを破棄する
	m_pAutomation = nullptr;
}

} // namespace window
