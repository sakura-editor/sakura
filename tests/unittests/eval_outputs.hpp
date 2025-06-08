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
#include "pch.h"

#include "util/string_ex.h"

// 標準エラー出力に吐き出されたメッセージを評価します
#define EXPECT_ERROUT(statementExpression, expected) \
	testing::internal::CaptureStderr(); \
	statementExpression; \
	EXPECT_STREQ(strprintf(L"%s\n", expected).data(), u8stowcs(testing::internal::GetCapturedStderr()).data())
