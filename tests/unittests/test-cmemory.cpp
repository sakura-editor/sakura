/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include "mem/CMemory.h"

#include "testing/GuiAwareTestSuite.hpp"

using CMemoryTest = testing::TGuiAware<::testing::Test>;

/*!
	_SetRawLength(0) を呼び出して落ちないことを確認する
*/
TEST(CMemory, SetRawLengthToZero)
{
	CMemory memory;

	// _SetRawLength(0) を呼び出して落ちないことを確認する
	memory._SetRawLength(0);
}

/*!
	CMemory を単にインスタンス化した状態ではバッファが確保されていないのを確認する。
*/
TEST(CMemory, CheckEmpty)
{
	CMemory memory;

	// インスタンス化しただけ
	// → バッファが確保されないことを確認する
	EXPECT_EQ(NULL, memory.GetRawPtr());

	// インスタンス化しただけ
	// → バッファサイズが 0 であることを確認する。
	EXPECT_EQ(0, memory.capacity());

	// インスタンス化しただけ
	// → データサイズが 0 であることを確認する。
	EXPECT_EQ(0, memory.GetRawLength());
}

/*!
	CMemoryのテスト
	比較用static関数。
 */
TEST(CMemory, StaticIsEqual)
{
	constexpr auto& v1 = u8"これはテストです。";
	constexpr auto& v2 = u8"これはテストですか？";
	constexpr auto& v3 = u8"これはテストです？";

	CMemory m1(v1, _countof(v1));
	CMemory m2(v2, _countof(v2));
	CMemory m3(v3, _countof(v3));
	CMemory m4(v1, _countof(v1));

	// 長さが違う場合、false
	ASSERT_FALSE(CMemory::IsEqual(m1, m2));

	// 長さが同じでも、内容が異なればfalse
	ASSERT_FALSE(CMemory::IsEqual(m1, m3));

	// 同内容なら、true
	ASSERT_TRUE(CMemory::IsEqual(m1, m4));
}

/*!
	CMemoryのテスト
	データをWORD配列とみなしてエンディアンを反転させる機能。
 */
TEST(CMemory, SwapHLByte)
{
	constexpr auto& source = "B+saci-";
	constexpr auto& expected = "+Basic-";

	CMemory cmem1(source, _countof(source) - 1);
	cmem1.SwapHLByte();
	ASSERT_TRUE(0 == memcmp(expected, cmem1.GetRawPtr(), cmem1.GetRawLength()));

	std::string buff(source);
	CMemory::SwapHLByte(buff.data(), buff.length());
	ASSERT_TRUE(0 == memcmp(expected, buff.data(), buff.length()));
}

/*!
	CMemoryのテスト
	ヒープに確保できる限界量を越えるサイズを要求した場合の挙動確認
 */
TEST_F(CMemoryTest, OverHeapMaxReq)
{
	CMemory cmem;
	const auto expected = strprintf(LS(STR_ERR_DLGMEM1), -31);	//FIXME: 確保サイズ == -31 はバグと思われる

	// _HEAP_MAXREQを越える値を指定すると、メモリは確保されない
	EXPECT_MSGBOX(cmem.AllocBuffer(static_cast<unsigned>(_HEAP_MAXREQ) + 1), GSTR_APPNAME, expected);
	ASSERT_TRUE(cmem.GetRawPtr() == nullptr);

	// 検証用のデータを入れる
	constexpr auto& data = L"テストデータ";
	cmem.SetRawData(data, (_countof(data) - 1) * sizeof(wchar_t));
	ASSERT_THAT(reinterpret_cast<wchar_t*>(cmem.GetRawPtr()), StrEq(data));
	ASSERT_THAT(cmem.GetRawLength(), (_countof(data) - 1) * sizeof(wchar_t));

	// メモリ確保失敗時は、メモリが解放される
	EXPECT_MSGBOX(cmem.AllocBuffer(static_cast<unsigned>(_HEAP_MAXREQ) + 1), GSTR_APPNAME, expected);
	ASSERT_TRUE(cmem.GetRawPtr() == nullptr);
}

/*!
	CMemoryのテスト
	仕様上の上限値を越えるサイズを要求した場合の挙動確認
 */
TEST_F(CMemoryTest, OverMaxSize)
{
	CMemory cmem;
	const auto expected = strprintf(LS(STR_ERR_DLGMEM1), -2147483648);	//FIXME: 確保サイズ == -2147483648 はバグと思われる

	// INT_MAXを越える値を指定すると、メモリは確保されない
	EXPECT_MSGBOX(cmem.AllocBuffer(static_cast<unsigned>(INT_MAX) + 1), GSTR_APPNAME, expected);
	ASSERT_TRUE(cmem.GetRawPtr() == nullptr);

	// 検証用のデータを入れる
	constexpr auto& data = L"テストデータ";
	cmem.SetRawData(data, (_countof(data) - 1) * sizeof(wchar_t));
	ASSERT_THAT(reinterpret_cast<wchar_t*>(cmem.GetRawPtr()), StrEq(data));
	ASSERT_THAT(cmem.GetRawLength(), (_countof(data) - 1) * sizeof(wchar_t));

	// メモリ確保失敗時は、メモリが解放される
	EXPECT_MSGBOX(cmem.AllocBuffer(static_cast<unsigned>(INT_MAX) + 1), GSTR_APPNAME, expected);
	ASSERT_TRUE(cmem.GetRawPtr() == nullptr);
}

/*!
	CMemory::AppendRawDataのテスト
	様々なサイズのデータ追加が正常に行える事の確認
 */
TEST(CMemory, AppendRawData)
{
	const size_t n = 2 * 5000;
	const size_t sumAnswer = n / 2 * (n + 1);
	std::vector<char> buff(n);
	for (size_t i = 0; i < n; ++i) {
		buff[i] = '0' + (i % 10);
	}
	const void* pData = &buff[0];
	CMemory cmem;
	cmem.AllocBuffer(sumAnswer);
	size_t sum = 0;
	for (size_t i = 0; i <= n; ++i) {
		{
			cmem.AppendRawData(pData, i);
			sum += i;
			ASSERT_TRUE(cmem.GetRawLength() == sum);
		}
		{
			// #1638 の修正に関連した試験
			CMemory cmemTmp;
			cmemTmp.AllocBuffer(1);
			cmemTmp.AppendRawData(pData, i);
			auto rawLen = cmemTmp.GetRawLength();
			ASSERT_TRUE(rawLen == i);
		}
	}
	ASSERT_TRUE(sum == sumAnswer);
}
