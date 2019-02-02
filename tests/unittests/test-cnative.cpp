/*! @file */
/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
#include "mem/CNativeW.h"
#include "mem/CNativeA.h"

/*!
	CNativeW::Clear のデータサイズのクリアをテストする

	0. バッファが空の状態でクリアする

	1-1. 固定データを追加する
	1-2. バッファの状態を取得する
	1-3. バッファの状態をチェックする

	2-1. CNativeW をクリアする
	2-2. クリア後のバッファの状態を取得する
	2-3. クリア後のバッファの状態をチェックする

	3-1. 固定データを再追加する
	3-2. バッファの状態を取得する
	3-3. バッファの状態をチェックする
*/
TEST(CNativeW, Clear)
{
	constexpr const WCHAR*	fixedPatternStr = L"abc";
	constexpr const int		fixedPatternLen = 3;
	
	CNativeW stringW;
	
	// 0. バッファが空の状態でクリアする
	stringW.Clear();

	// 1-1. 固定データを追加する

	stringW.AppendString(fixedPatternStr);			// 固定データを追加する

	// 1-2. バッファの状態を取得する
	
	auto orgCapacity = stringW.capacity();			// データ追加後にバッファサイズを取得する
	auto orgLength   = stringW.GetStringLength();	// Clear() 前にデータサイズを取得する

	// 1-3. バッファの状態をチェックする

	EXPECT_GT(orgCapacity, 0);						// データ追加後のバッファサイズを確認する
	EXPECT_EQ(orgLength, fixedPatternLen);			// データ追加後のデータサイズを確認する

	// 2-1. CNativeW をクリアする
	
	stringW.Clear();								// CNativeW をクリアする

	// 2-2. クリア後のバッファの状態を取得する

	auto newCapacity = stringW.capacity();			// Clear() 後にバッファサイズを取得する
	auto newLength   = stringW.GetStringLength();	// Clear() 後にデータサイズを取得する

	// 2-3. クリア後のバッファの状態をチェックする
	
	EXPECT_EQ(orgCapacity, newCapacity);			// Clear() 後にバッファサイズが変わっていないのを確認する
	EXPECT_EQ(newLength, 0);						// Clear() 後にデータが空なのを確認する

	// 3-1. 固定データを再追加する

	stringW.AppendString(fixedPatternStr);			// Clear() 後に固定データを再追加する

	// 3-2. バッファの状態を取得する
	
	auto newCapacity2 = stringW.capacity();			// 再追加後にバッファサイズを取得する
	auto newLength2   = stringW.GetStringLength();	// 再追加後にデータサイズを取得する

	// 3-3. バッファの状態をチェックする
	
	EXPECT_EQ(orgCapacity, newCapacity2);			// 再追加後にバッファサイズが変わっていないのを確認する
	EXPECT_EQ(newLength2, fixedPatternLen);			// 再追加後にデータサイズを確認する
}
