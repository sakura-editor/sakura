/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "apiwrap/StdControl.h"

/*!
*/
TEST(StdControl, Wnd_GetText)
{
	CNativeW tempText;
	ASSERT_FALSE(Wnd_GetText(NULL, tempText));
}

// GitHub #1528 の退行防止テストケース。
// 取得する文字列の長さが basic_string::capacity と同じだった場合に一文字取りこぼしていた。
TEST(StdControl, Wnd_GetText2)
{
	wchar_t text[] = L"0123456789012345678901234567890123456789";

	std::wstring s;
	text[s.capacity()] = L'\0';

	HINSTANCE hinstance = GetModuleHandleW(nullptr);
	HWND hwnd = CreateWindowExW(0, L"STATIC", text, 0, 1, 1, 1, 1, nullptr, nullptr, hinstance, nullptr);
	Wnd_GetText(hwnd, s);
	DestroyWindow(hwnd);
	ASSERT_STREQ(s.c_str(), text);
}
