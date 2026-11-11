/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "util/os.h"
#include "util/tchar_convert.h"
#include "window/EditorTestSuite.hpp"

// 標準エラー出力に吐き出されたメッセージを評価します
#define EXPECT_ERROUT(statementExpression, expected) \
	testing::internal::CaptureStderr(); \
	statementExpression; \
	EXPECT_THAT(cxx::to_wstring(testing::internal::GetCapturedStderr(), CP_UTF8), StrEq(std::format(L"{:s}\n", expected)))
