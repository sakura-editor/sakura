/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "util/os.h"
#include "util/tchar_convert.h"

struct MockUser32 final : public User32
{
	MOCK_CONST_METHOD5(MessageBoxExW, int(
		_In_opt_ HWND hWnd,
		_In_opt_ LPCWSTR lpText,
		_In_opt_ LPCWSTR lpCaption,
		_In_ UINT uType,
		_In_ WORD wLanguageId
	));
};

// 標準エラー出力に吐き出されたメッセージを評価します
#define EXPECT_ERROUT(statementExpression, expected) \
	testing::internal::CaptureStderr(); \
	statementExpression; \
	EXPECT_THAT(cxx::to_wstring(testing::internal::GetCapturedStderr(), CP_UTF8), StrEq(std::format(L"{:s}\n", expected)))
