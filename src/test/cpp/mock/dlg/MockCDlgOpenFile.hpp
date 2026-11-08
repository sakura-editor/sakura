/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "dlg/CDlgOpenFile.h"

#include <gmock/gmock.h>

struct MockCDlgOpenFile final : public CDlgOpenFile {
	MOCK_METHOD2(DoModal_GetOpenFileName, bool(std::span<WCHAR>, EFilter));
	MOCK_METHOD1(DoModal_GetSaveFileName, bool(std::span<WCHAR>));
	MOCK_METHOD3(DoModalOpenDlg, bool(SLoadInfo*, std::vector<std::wstring>*, bool));
	MOCK_METHOD2(DoModalSaveDlg, bool(SSaveInfo*, bool));

	static inline std::vector<std::wstring> gm_Files;

	static void _Cleanup([[maybe_unused]] const MockCDlgOpenFile*);

	static bool _GetOpenFileName(
		std::span<WCHAR> szPath,
		EFilter eAddFileter
	);

	static bool _GetSaveFileName(
		std::span<WCHAR> szPath
	);

	static bool _DoModalOpenDlg(
		SLoadInfo* pLoadInfo,
		std::vector<std::wstring>* pFilenames,
		bool bOptions [[maybe_unused]]
	);

	static bool _DoModalSaveDlg(
		SSaveInfo* pSaveInfo,
		bool bSimpleMode [[maybe_unused]]
	);

	explicit MockCDlgOpenFile();

	using Holder = cxx::ResourceHolder<&_Cleanup>;
	Holder m_Holder{ this };
};
