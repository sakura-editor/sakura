/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/ShareDataTestSuite.hpp"
#include "window/UiaTestSuite.hpp"

#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"

#include "CEditApp.h"

#include "agent/CGrepAgent.h"
#include "agent/CLoadAgent.h"
#include "agent/CSaveAgent.h"
#include "env/CPropertyManager.h"
#include "macro/CSMacroMgr.h"
#include "recent/CMruListener.h"
#include "uiparts/CVisualProgress.h"

namespace window {

struct EditorTestSuite : public env::ShareDataTestSuite
{
	static inline std::unique_ptr<CEditDoc> pcEditDoc = nullptr;
	static inline std::unique_ptr<CEditWnd> pcEditWnd = nullptr;
	static inline std::unique_ptr<CSMacroMgr> pcSMacroMgr = nullptr;

	static inline std::unique_ptr<CLoadAgent> pcLoadAgent = nullptr;
	static inline std::unique_ptr<CSaveAgent> pcSaveAgent = nullptr;
	static inline std::unique_ptr<CVisualProgress> pcVisualProgress = nullptr;
	static inline std::unique_ptr<CGrepAgent> pcGrepAgent = nullptr;
	static inline std::unique_ptr<CMruListener> pcMruListener = nullptr;
	static inline std::unique_ptr<CPropertyManager> pcPropertyManager = nullptr;

	static void SetUpEditor();
	static void TearDownEditor();
};

} // namespace env

struct MockBcrypt final : public Bcrypt
{
	MOCK_CONST_METHOD4(BCryptGenRandom, NTSTATUS(
		_In_opt_                        BCRYPT_ALG_HANDLE   hAlgorithm,
		_Out_writes_bytes_(cbBuffer)    PUCHAR  pbBuffer,
		_In_                            ULONG   cbBuffer,
		_In_                            ULONG   dwFlags
	));
};
