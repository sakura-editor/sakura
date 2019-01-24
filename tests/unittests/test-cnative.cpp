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

TEST(CNativeW, Clear)
{
	CNativeW stringW;
	stringW.AppendString(L"abc");
	
	/* Clear() 前にバッファサイズを取得する */
	auto orgCapacity = stringW.capacity();

	/* Clear() 前にデータサイズを取得する */
	auto orgLength   = stringW.GetStringLength();
	EXPECT_EQ(orgLength, 3);

	stringW.Clear();

	/* Clear() 後にバッファサイズを取得する */
	auto newCapacity = stringW.capacity();

	/* Clear() 後にデータサイズを取得する */
	auto newLength   = stringW.GetStringLength();

	/* Clear() 後にバッファサイズが変わっていないのを確認する */
	EXPECT_EQ(orgCapacity, newCapacity);

	/* Clear() 後にデータが空なのを確認する */
	EXPECT_EQ(newLength, 0);
}
