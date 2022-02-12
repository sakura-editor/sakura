/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include <gtest/gtest.h>

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
