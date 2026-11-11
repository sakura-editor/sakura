/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "util/os/MockComdlg32.hpp"

/* static */ void MockComdlg32::_CleanUp(
	[[maybe_unused]] const MockComdlg32* /* pThis */
) noexcept
{
	gm_Files.clear();
}

/* static */ BOOL MockComdlg32::_GetOpenFileNameW(
	LPOPENFILENAMEW pOfn
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(pOfn->lpstrFile, pOfn->nMaxFile, std::data(gm_Files.front()));
	}
	return result;
}

/* static */ BOOL MockComdlg32::_GetSaveFileNameW(
	LPOPENFILENAMEW pOfn
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(pOfn->lpstrFile, pOfn->nMaxFile, std::data(gm_Files.front()));
	}
	return result;
}

MockComdlg32::MockComdlg32()
{
	ON_CALL(*this, GetOpenFileNameW(_)).WillByDefault(&_GetOpenFileNameW);
	ON_CALL(*this, GetSaveFileNameW(_)).WillByDefault(&_GetSaveFileNameW);

	ON_CALL(*this, CommDlgExtendedError()).WillByDefault([]() { return 0; });
}
