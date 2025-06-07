/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
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

TEST(StdControl, List_GetText) {
	std::wstring text = L"0123456789abcdef";
	HWND list = ::CreateWindow(L"LISTBOX", nullptr, 0, 1, 1, 1, 1, nullptr, nullptr, nullptr, nullptr);
	ApiWrap::List_AddString(list, text.c_str());
	ApiWrap::List_AddString(list, L"");

	std::wstring result1;
	ASSERT_TRUE(ApiWrap::List_GetText(list, 0, result1));
	ASSERT_EQ(result1, text);

	result1.clear();
	ASSERT_TRUE(ApiWrap::List_GetText(list, 1, result1));
	ASSERT_TRUE(result1.empty());

	ASSERT_FALSE(ApiWrap::List_GetText(list, 2, result1));

	wchar_t result2[15]{};
	ASSERT_EQ(ApiWrap::List_GetText(list, 0, result2), LB_ERRSPACE);

	wchar_t result3[16]{};
	ASSERT_EQ(ApiWrap::List_GetText(list, 0, result3), LB_ERRSPACE);

	wchar_t result4[17]{};
	ASSERT_EQ(ApiWrap::List_GetText(list, 0, result4), text.length());
	ASSERT_STREQ(result4, text.c_str());

	result4[0] = L'\0';
	ASSERT_EQ(ApiWrap::List_GetText(list, 2, result4), LB_ERR);

	::DestroyWindow(list);
}
