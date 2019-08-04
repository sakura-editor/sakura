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

// BlockSize テンプレート引数のバリエーション用意
using test_types = ::testing::Types<
	std::integral_constant<std::size_t, 1>,
	std::integral_constant<std::size_t, 2>,
	std::integral_constant<std::size_t, 3>,
	std::integral_constant<std::size_t, 4>,
	std::integral_constant<std::size_t, 8>,
	std::integral_constant<std::size_t, 16>,
	std::integral_constant<std::size_t, 32>,
	std::integral_constant<std::size_t, 64>,
	std::integral_constant<std::size_t, 128>,
	std::integral_constant<std::size_t, 256>,
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

// ポインタのアライメントが取れているか確認
#define is_aligned(POINTER, BYTE_COUNT) (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

// デフォルトコンストラクタを指定回数呼び出し
template <typename T, size_t BlockSize>
void testPool(size_t sz)
{
	CMemPool<T, BlockSize> pool;
	std::vector<T*> ptrs(sz);
	// 要素構築
	for (size_t i=0; i<sz; ++i) {
		T* p = pool.Construct();
		ptrs[i] = p;
		ASSERT_TRUE(p != nullptr); // 領域を確保できたか確認
		ASSERT_TRUE(is_aligned(p, alignof(T))); // アライメントが取れているか確認
	}
	// 要素破棄
	for (size_t i=0; i<sz; ++i) {
		pool.Destruct(ptrs[i]);
	}
	// 要素破棄後に要素を再度構築
	for (size_t i=0; i<sz; ++i) {
		T* p = pool.Construct();
		ptrs[i] = p;
		ASSERT_TRUE(p != nullptr); // 領域を確保できたか確認
		ASSERT_TRUE(is_aligned(p, alignof(T))); // アライメントが取れているか確認
	}
	// 再度要素を破棄
	for (size_t i=0; i<sz; ++i) {
		pool.Destruct(ptrs[i]);
	}
}

// 色々な型やBlockSizeでデフォルトコンストラクタをたくさん呼び出す
TYPED_TEST(CMemPoolTest, default_constructor)
{
	// BlockSize を取得
	constexpr size_t BlockSize = TypeParam::value;
	
	// ユーザー定義型の確認用
	struct TestStruct
	{
		int* ptr;
		char buff[32];
		long long ll;
		double f64;
	};
		
	/*!
		色々なケースの確認
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
	
	std::wstring s0{L"あいうえお"};
	std::wstring s1{L"nullptr"};
	wchar_t c2{'㌍'};
	size_t len2 = 12345;
	std::wstring s2(len2, c2);
	std::wstring s3{L"令和"};
	
	// 構築と破棄を繰り返して動作するか確認
	for (size_t i=0; i<8; ++i) {
		// 要素構築
		p0 = pool.Construct(s0);
		p1 = pool.Construct(s1);
		p2 = pool.Construct(len2, c2);
		p3 = pool.Construct(s3);

		// 領域を構築できたか確認
		ASSERT_TRUE(p0 != nullptr);
		ASSERT_TRUE(p1 != nullptr);
		ASSERT_TRUE(p2 != nullptr);
		ASSERT_TRUE(p3 != nullptr);
		
		// コンストラクタに指定した値と同一か確認
		ASSERT_TRUE(*p0 == s0);
		ASSERT_TRUE(*p1 == s1);
		ASSERT_TRUE(*p2 == s2);
		ASSERT_TRUE(*p3 == s3);
		
		// 要素破棄
		pool.Destruct(p0);
		pool.Destruct(p1);
		pool.Destruct(p2);
		pool.Destruct(p3);
	}
}

