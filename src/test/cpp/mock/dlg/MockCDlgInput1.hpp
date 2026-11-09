/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "dlg/CDlgInput1.h"

#include <gmock/gmock.h>

struct MockCDlgInput1 final : public CDlgInput1
{
	MOCK_METHOD5(DoModal, BOOL(
		_In_opt_ HWND hWndOwner,
		_In_z_ LPCWSTR pszTitle,
		_In_z_ LPCWSTR pszMessage,
		_Out_writes_z_(cchBuffer) LPWSTR pBuffer,
		size_t cchBuffer
	));
};
