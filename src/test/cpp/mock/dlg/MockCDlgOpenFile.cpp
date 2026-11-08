/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "dlg/MockCDlgOpenFile.hpp"

/* static */ void MockCDlgOpenFile::_Cleanup([[maybe_unused]] const MockCDlgOpenFile*)
{
	gm_Files.clear();
}

/* static */ bool MockCDlgOpenFile::_GetOpenFileName(std::span<WCHAR> szPath, EFilter eAddFileter)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_GetSaveFileName(std::span<WCHAR> szPath)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFilenames,
	bool bOptions [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pLoadInfo->cFilePath = gm_Files.front().c_str();
		*pFilenames = gm_Files;
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pSaveInfo->cFilePath = gm_Files.front().c_str();
	}
	return result;
}

MockCDlgOpenFile::MockCDlgOpenFile()
{
	ON_CALL(*this, DoModal_GetOpenFileName(_, _)).WillByDefault(&_GetOpenFileName);
	ON_CALL(*this, DoModal_GetSaveFileName(_)).WillByDefault(&_GetSaveFileName);
	ON_CALL(*this, DoModalOpenDlg(_, _, _)).WillByDefault(&_DoModalOpenDlg);
	ON_CALL(*this, DoModalSaveDlg(_, _)).WillByDefault(&_DoModalSaveDlg);
}
