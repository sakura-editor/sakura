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
#include "mem/CMemPool.h"
#include <vector>
#include <array>
#include <string>
#include <type_traits>
#include <stdint.h>
#include <Windows.h>

template <typename T>
class CMemPoolTest : public ::testing::Test{};

using test_types = ::testing::Types<
	std::integral_constant<std::size_t, 512>,
	std::integral_constant<std::size_t, 1024>, 
	std::integral_constant<std::size_t, 2048>,
	std::integral_constant<std::size_t, 4096>,
	std::integral_constant<std::size_t, 8192>,
	std::integral_constant<std::size_t, 16384>,
	std::integral_constant<std::size_t, 32768>,
	std::integral_constant<std::size_t, 65536>
>;

TYPED_TEST_CASE(CMemPoolTest, test_types);

#define is_aligned(POINTER, BYTE_COUNT) (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

template <typename T, size_t BlockSize>
void testPool(size_t sz)
{
	CMemPool<T, BlockSize> pool;
	std::vector<T*> ptrs(sz);
	for (size_t i=0; i<sz; ++i) {
		T* p = pool.Construct();
		ptrs[i] = p;
		ASSERT_TRUE(p != nullptr);
		ASSERT_TRUE(is_aligned(p, alignof(T)));
	}
	for (size_t i=0; i<sz; ++i) {
		pool.Destruct(ptrs[i]);
	}
}

// デフォルトコンストラクタ
TYPED_TEST(CMemPoolTest, default_constructor)
{
	constexpr size_t BlockSize = TypeParam::value;
	struct TestStruct
	{
		int* ptr;
		char buff[32];
		long long ll;
		double f64;
	};
		
	/*!
		指定回数呼び出して落ちない事を確認する
	*/
	size_t sz = 1;
	for (size_t i=0; i<10; ++i, sz*=2) {
		testPool<int8_t, BlockSize>(sz);
		testPool<int16_t, BlockSize>(sz);
		testPool<int32_t, BlockSize>(sz);
		testPool<int64_t, BlockSize>(sz);
		testPool<float, BlockSize>(sz);
		testPool<double, BlockSize>(sz);
		testPool<FILETIME, BlockSize>(sz);
		testPool<TestStruct, BlockSize>(sz);
		testPool<std::wstring, BlockSize>(sz);
	}
}

// 引数付きコンストラクタ
TYPED_TEST(CMemPoolTest, parameterized_constructor)
{
	constexpr size_t BlockSize = TypeParam::value;
	CMemPool<std::wstring, BlockSize> pool;
	
	std::wstring* p0 = nullptr;
	std::wstring* p1 = nullptr;
	std::wstring* p2 = nullptr;
	std::wstring* p3 = nullptr;
	
	p0 = pool.Construct(L"あいうえお");
	p1 = pool.Construct(L"nullptr");
	p2 = pool.Construct(12345, '㌍');
	std::wstring s{L"令和"};
	p3 = pool.Construct(s);

	ASSERT_TRUE(p0 != nullptr);
	ASSERT_TRUE(p1 != nullptr);
	ASSERT_TRUE(p2 != nullptr);
	ASSERT_TRUE(p3 != nullptr);
	
	ASSERT_TRUE(p0->size() == 5);
	ASSERT_TRUE(p1->size() == 7);
	ASSERT_TRUE(p2->size() == 12345);
	ASSERT_TRUE(p3->size() == 2);
	ASSERT_TRUE(*p3 == s);
	
	pool.Destruct(p0);
	pool.Destruct(p1);
	pool.Destruct(p2);
	pool.Destruct(p3);
}

// ブロックサイズは要素が２つ以上入る大きさにする確認
TEST(CMemPool, BlockSize)
{
	CMemPool<std::array<uint8_t, 1024>, 2048> pool0;
	CMemPool<std::array<uint8_t, 1025>, 4096> pool1;
	CMemPool<std::array<uint8_t, 2048>, 4096> pool2;
	CMemPool<std::array<uint8_t, 2047>, 8192> pool3;
	CMemPool<std::array<uint8_t, 4096>, 8192> pool4;
}

