/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "charset/codechecker.h"

TEST(ConvertToUtf32, SurrogatePair)
{
	// 絵文字　男性のシンボル
	const auto& s = L"\U0001f6b9";
	EXPECT_EQ(0x1f6b9, ConvertToUtf32(s));
}

TEST(ConvertToUtf32, VariationSelector)
{
	// 異体字セレクタ　VS17
	const auto& s = L"\U000e0100";
	EXPECT_EQ(0xe0100, ConvertToUtf32(s));
}

TEST(ConvertToUtf32, BinaryOnSurrogate)
{
	// 独自仕様　変換できない文字は1byteずつ下位サロゲートに詰める
	const auto& s = L"\xdcff";
	EXPECT_EQ(0, ConvertToUtf32(s));
}

TEST(IsVariationSelector, VariationSelectorCheck)
{
	// IVS開始
	const auto& ivs1 = L"\U000E0100";
	EXPECT_TRUE(IsVariationSelector(ivs1, 2));

	// IVS終了
	const auto& ivs2 = L"\U000E01EF";
	EXPECT_TRUE(IsVariationSelector(ivs2, 2));

	// 長さ不足
	EXPECT_FALSE(IsVariationSelector(ivs1, 0));
	EXPECT_FALSE(IsVariationSelector(ivs1, 1));

	// IVS開始-1
	const auto& notivs1 = L"\U000E00FF";
	EXPECT_FALSE(IsVariationSelector(notivs1, 2));

	// IVS終了+1
	const auto& notivs2 = L"\U000E01F0";
	EXPECT_FALSE(IsVariationSelector(notivs2, 2));
}
