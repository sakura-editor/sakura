/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/ShareDataTestSuite.hpp"

#include "doc/CEditDoc.h"
#include "macro/CSMacroMgr.h"
#include "window/CEditWnd.h"

namespace window {

struct EditorTestSuite : public env::ShareDataTestSuite
{
	static inline std::unique_ptr<CEditDoc> pcEditDoc = nullptr;
	static inline std::unique_ptr<CEditWnd> pcEditWnd = nullptr;
	static inline std::unique_ptr<CSMacroMgr> pcSMacroMgr = nullptr;

	static void SetUpEditor();
	static void TearDownEditor();
};

} // namespace env
