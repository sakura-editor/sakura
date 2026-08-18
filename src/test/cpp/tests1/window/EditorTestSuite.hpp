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

struct MockUser32 final : public User32
{
	MOCK_CONST_METHOD5(MessageBoxExW, int(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	));

	MOCK_CONST_METHOD7(TrackPopupMenu, BOOL(
		_In_ HMENU hMenu,
		_In_ UINT uFlags,
		_In_ int x,
		_In_ int y,
		_Reserved_ int nReserved,
		_In_ HWND hWnd,
		_Reserved_ CONST RECT* prcRect
	));

	static int _MessageBoxExW(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	);

	MockUser32();
};

struct MockComdlg32 final : public Comdlg32 {
	MOCK_CONST_METHOD1(ChooseColorW, BOOL(LPCHOOSECOLORW pCf));
	MOCK_CONST_METHOD1(ChooseFontW, BOOL(LPCHOOSEFONTW pCf));
	MOCK_CONST_METHOD0(CommDlgExtendedError, DWORD());
	MOCK_CONST_METHOD1(GetOpenFileNameW, BOOL(LPOPENFILENAMEW pOfn));
	MOCK_CONST_METHOD1(GetSaveFileNameW, BOOL(LPOPENFILENAMEW pOfn));
	MOCK_CONST_METHOD1(PrintDlgW, BOOL(LPPRINTDLGW pPD));

	static inline std::vector<std::wstring> gm_Files;

	static void _Cleanup([[maybe_unused]] const MockComdlg32*);

	static BOOL _GetOpenFileNameW(
		LPOPENFILENAMEW pOfn
	);

	static BOOL _GetSaveFileNameW(
		LPOPENFILENAMEW pOfn
	);

	MockComdlg32();

	using Holder = cxx::ResourceHolder<&_Cleanup>;
	Holder m_Holder{ this };
};

