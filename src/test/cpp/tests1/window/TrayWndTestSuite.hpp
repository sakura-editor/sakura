/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/ShareDataTestSuite.hpp"
#include "window/UiaTestSuite.hpp"

#include "_main/CCommandLine.h"
#include "_main/CControlTray.h"

namespace window {

struct TrayWndTestSuite : public env::ShareDataTestSuite
{
	using CControlTrayHolder = std::unique_ptr<CControlTray>;

	static inline std::unique_ptr<CCommandLine> pCommandLine = nullptr;

	static inline CControlTrayHolder pcTrayWnd = nullptr;

	static void SetUpTrayWnd();
	static void TearDownTrayWnd();
};

} // namespace env
