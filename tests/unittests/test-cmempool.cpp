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
#include <stdint.h>
#include <Windows.h>

#define is_aligned(POINTER, BYTE_COUNT) (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

template <typename T>
void testPool(size_t sz)
{
	CMemPool<T> pool;
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
TEST(CMemPool, default_constructor)
{
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
		testPool<int8_t>(sz);
		testPool<int16_t>(sz);
		testPool<int32_t>(sz);
		testPool<int64_t>(sz);
		testPool<float>(sz);
		testPool<double>(sz);
		testPool<FILETIME>(sz);
		testPool<TestStruct>(sz);
		testPool<std::wstring>(sz);
	}
}

// 引数付きコンストラクタ
TEST(CMemPool, parameterized_constructor)
{
	CMemPool<std::wstring> pool;
	
	std::wstring* p0 = nullptr;
	std::wstring* p1 = nullptr;
	std::wstring* p2 = nullptr;
	std::wstring* p3 = nullptr;
	
	p0 = pool.Construct(L"あいうえお");
	p1 = pool.Construct(L"nullptr");
	p2 = pool.Construct(12345, '\u4A3B');
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

// ブロックサイズ
TEST(CMemPool, BlockSize)
{
	CMemPool<std::array<uint8_t, 2048>> pool;
	CMemPool<std::array<uint8_t, 4096>, 8192> pool2;
}

