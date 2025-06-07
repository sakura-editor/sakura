/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <windowsx.h>
#include <Shlwapi.h>

#include <cstdlib>

#include "util/window.h"

/*!
 * @brief CFontAutoDeleterのテスト
 */
TEST( CFontAutoDeleter, test )
{
	CFontAutoDeleter deleter;
	ASSERT_EQ(nullptr, deleter.GetFont());

	if (const auto hGdiFont = GetStockFont(DEFAULT_GUI_FONT)) {
		if (LOGFONT lf = {};
			::GetObject(hGdiFont, sizeof(lf), &lf)) {
			if (const auto hFont = ::CreateFontIndirect(&lf)) {
				deleter.SetFont(nullptr, hFont, nullptr);
				ASSERT_EQ(hFont, deleter.GetFont());
			}
		}
	}

	ASSERT_NE(nullptr, deleter.GetFont());
	if (const auto hFont = deleter.GetFont()) {
		CFontAutoDeleter other(deleter);
		ASSERT_NE(hFont, other.GetFont());

		other.ReleaseOnDestroy();
		ASSERT_EQ(nullptr, other.GetFont());

		CFontAutoDeleter another(std::move(deleter));
		ASSERT_EQ(hFont, another.GetFont());
		ASSERT_EQ(nullptr, deleter.GetFont());
	}
}
