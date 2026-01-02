/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "util/string_ex.h"

// 標準エラー出力に吐き出されたメッセージを評価します
#define EXPECT_ERROUT(statementExpression, expected) \
	testing::internal::CaptureStderr(); \
	statementExpression; \
	EXPECT_STREQ(strprintf(L"%s\n", expected).data(), u8stowcs(testing::internal::GetCapturedStderr()).data())
