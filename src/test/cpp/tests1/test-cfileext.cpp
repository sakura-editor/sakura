/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <Windows.h>
#include "basis/CFileExt.h"


static size_t GetFilterLength(const WCHAR* filter)
{
	size_t length = 0;
	size_t i = 0;
	while (!(filter[i] == LCHAR('\0') && filter[i+1] == LCHAR('\0'))) {
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
	WCHAR result[] = LTEXT("ユーザー設定 (*.cpp;*.h;*.*)\0*.cpp;*.h;*.*\0テキスト (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0");

	CFileExt cFileExt;
	cFileExt.AppendExtRaw(LTEXT("ユーザー設定"), LTEXT("*.cpp;*.h;*.*"));
	cFileExt.AppendExtRaw(LTEXT("テキスト"), LTEXT("*.txt"));
	cFileExt.AppendExtRaw(LTEXT("すべてのファイル"), LTEXT("*.*"));

	const WCHAR* filter = cFileExt.GetExtFilter();
	size_t length = GetFilterLength(filter);
	sakura::wstring expected = {result, int(std::size(result))};
	sakura::wstring actual = {filter, length};
	EXPECT_EQ(expected, actual);
}

TEST(CFileExt, RawLongFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExtRaw(
		LTEXT("ユーザー設定"),
		LTEXT("*.extensin_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_longX")
	);
}

TEST(CFileExt, RawManyFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExtRaw(
		LTEXT("ユーザー指定"),
		LTEXT("*.extensin_50_0_long_long_long_long_long_long_long_l;*.extensin_50_1_long_long_long_long_long_long_long_l;*.extensin_50_2_long_long_long_long_long_long_long_l;*.extensin_50_3_long_long_long_long_long_long_long_l;*.extensin_50_4_long_long_long_long_long_long_long_l;*.extensin_50_5_long_long_long_long_long_long_long_l;*.extensin_50_6_long_long_long_long_long_long_long_l;*.extensin_50_7_long_long_long_long_long_long_long_l;*.extensin_50_8_long_long_long_long_long_long_long_l;*.extensin_50_9_long_long_long_long_long_long_long_l")
	);
}

TEST(CFileExt, LongFilter)
{
	CFileExt cFileExt;
	cFileExt.AppendExt(
		LTEXT("ユーザー設定"),
		LTEXT("*.extensin_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_longX")
	);
}
