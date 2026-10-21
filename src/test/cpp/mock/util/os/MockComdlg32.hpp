/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "util/os/Comdlg32.hpp"

#include <gmock/gmock.h>

struct MockComdlg32;

struct MockComdlg32 final : public Comdlg32 {
	MOCK_CONST_METHOD1(ChooseColorW, BOOL(LPCHOOSECOLORW pCf));
	MOCK_CONST_METHOD1(ChooseFontW, BOOL(LPCHOOSEFONTW pCf));
	MOCK_CONST_METHOD0(CommDlgExtendedError, DWORD());
	MOCK_CONST_METHOD1(GetOpenFileNameW, BOOL(LPOPENFILENAMEW pOfn));
	MOCK_CONST_METHOD1(GetSaveFileNameW, BOOL(LPOPENFILENAMEW pOfn));
	MOCK_CONST_METHOD1(PrintDlgW, BOOL(LPPRINTDLGW pPD));

	static inline std::vector<std::wstring> gm_Files;

	static void _CleanUp(
		[[maybe_unused]] const MockComdlg32*
	) noexcept;

	static BOOL _GetOpenFileNameW(
		LPOPENFILENAMEW pOfn
	);

	static BOOL _GetSaveFileNameW(
		LPOPENFILENAMEW pOfn
	);

	MockComdlg32();

	using Holder = cxx::ResourceHolder<&_CleanUp>;
	Holder m_Holder{ this };
};
