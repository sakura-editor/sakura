/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <Windows.h>
#include "CFileExt.h"


static size_t GetFilterLength(const wchar_t* filter)
{
	size_t length = 0;
	size_t i = 0;
	while (!(filter[i] == L'\0' && filter[i+1] == L'\0')) {
		++i;
	}
	return i + 2;
}

TEST(CFileExt, Construct)
{
	CFileExt cFileExt;
}

TEST(CFileExt, CreateFilter)
{
	wchar_t result[] = L"ユーザー設定 (*.cpp;*.h;*.*)\0*.cpp;*.h;*.*\0テキスト (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0";

	CFileExt cFileExt;
	cFileExt.AppendExtRaw(L"ユーザー設定", L"*.cpp;*.h;*.*");
	cFileExt.AppendExtRaw(L"テキスト", L"*.txt");
	cFileExt.AppendExtRaw(L"すべてのファイル", L"*.*");

	const wchar_t* filter = cFileExt.GetExtFilter();
	size_t length = GetFilterLength(filter);
	std::wstring expected = {result, _countof(result)};
	std::wstring actual = {filter, length};
	EXPECT_EQ(expected, actual);
}

TEST(CFileExt, RawLongFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExtRaw(
		L"ユーザー設定",
		L"*.extensin_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_longX"
	);
}

TEST(CFileExt, RawManyFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExtRaw(
		L"ユーザー指定",
		L"*.extensin_50_0_long_long_long_long_long_long_long_l;*.extensin_50_1_long_long_long_long_long_long_long_l;*.extensin_50_2_long_long_long_long_long_long_long_l;*.extensin_50_3_long_long_long_long_long_long_long_l;*.extensin_50_4_long_long_long_long_long_long_long_l;*.extensin_50_5_long_long_long_long_long_long_long_l;*.extensin_50_6_long_long_long_long_long_long_long_l;*.extensin_50_7_long_long_long_long_long_long_long_l;*.extensin_50_8_long_long_long_long_long_long_long_l;*.extensin_50_9_long_long_long_long_long_long_long_l"
	);
}

TEST(CFileExt, LongFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExt(
		L"ユーザー設定",
		L"*.extensin_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_longX"
	);
}
