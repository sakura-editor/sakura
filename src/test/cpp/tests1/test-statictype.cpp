/*! @file */
/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/StaticType.h"

namespace basis {

/*!
	@brief StaticVectorのテスト
 */
TEST(StaticVector, test001)
{
	StaticVector<size_t, 3> vec{ 10u, 20u, 30u };

	EXPECT_THAT(vec.size(), Eq(3));
	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(vec[2], Eq(30u));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));

	// 追加しようとしてもできないことを確認する
	EXPECT_THROW({ vec.emplace_back(40u); }, std::out_of_range);

	// 追加できないので、サイズをカウントアップしてはいけない
	EXPECT_THAT(vec.size(), Eq(3));

	// 追加しようとしてもできないことを確認する

#ifdef _DEBUG
	// デバッグビルドでは、正常にクラッシュする
	EXPECT_DEATH({ vec.push_back(40u); }, "");
#else
	// リリースビルドでもクラッシュする
	EXPECT_THROW({ vec.push_back(0xffffff); }, std::out_of_range);
#endif

	// 追加できないので、サイズをカウントアップしてはいけない
	EXPECT_THAT(vec.size(), Eq(3));

	// 範囲外アクセス
	try {
		const auto& cv = vec;
		cv[3];

		FAIL() << "Expected std::out_of_range to be thrown";
	}
	catch (const std::out_of_range& e) {
		EXPECT_STREQ(e.what(), "nIndex is out of range.");
	}

	auto* ptr = vec.dataPtr();
	ptr[0] = 10u;
	ptr[1] = 20u;

	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));

	EXPECT_THROW({ vec.resize(4); }, std::out_of_range);
	EXPECT_THAT(vec.size(), Eq(3));

	vec.resize(2);
	EXPECT_THAT(vec.size(), Eq(2));

	vec.resize(1);
	EXPECT_THAT(vec.size(), Eq(1));
	EXPECT_THAT(vec[0], Eq(10u));

	vec.push_back(20u);
	vec.emplace_back(30u);

	EXPECT_THAT(vec.size(), Eq(3));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(vec[2], Eq(30u));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));

	vec.clear();
	EXPECT_THAT(vec.size(), Eq(0));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));

	auto& sizeRef = vec._GetSizeRef();

	sizeRef = 2;
	vec.SetSizeLimit();
	EXPECT_THAT(vec.size(), Eq(2));

	sizeRef = 99;
	vec.SetSizeLimit();
	EXPECT_THAT(vec.size(), Eq(3));

	sizeRef = -1;
	vec.SetSizeLimit();
	EXPECT_THAT(vec.size(), Eq(0));

	// 引数足りない
	vec = StaticVector<size_t, 3>{ 10u, 20u };

	EXPECT_THAT(vec.size(), Eq(2));
	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));

	const auto& cv = vec;
	EXPECT_THAT(std::distance(cv.begin(), cv.end()), Eq(2));

	// 引数多過ぎ
	try {
		vec = StaticVector<size_t, 3>{ 10u, 20u, 30u, 40u };

		FAIL() << "Expected std::out_of_range to be thrown";
	}
	catch (const std::out_of_range& e) {
		EXPECT_STREQ(e.what(), "source is out of range.");
	}

	// 長さ3の配列を用意する
	const std::array<size_t, 3> source{ 10u, 20u, 30u };

	vec = StaticVector<size_t, 3>(source);

	EXPECT_THAT(vec.size(), Eq(3));
	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(vec[2], Eq(30u));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));
}

/*!
	@brief StaticStringのテスト
 */
TEST(StaticString, test001)
{
	StaticString<_MAX_PATH> path{};

	const std::wstring tooLong(_MAX_PATH * 2, L'a');

	// 初期状態（空）の確認
	EXPECT_THAT(path.empty(), IsTrue());
	EXPECT_THAT(path.length(), Eq(0u));
	EXPECT_THAT(path.GetBufferCount(), Eq(260u));

	// 代入メソッドの確認
	EXPECT_THAT(path.assign(L"C:\\work"), Eq(0));
	EXPECT_THAT(path, StrEq(L"C:\\work"));

	// 追記メソッドの確認
	EXPECT_THAT(path.append(L"\\sample.txt"), Eq(0));
	EXPECT_THAT(path, StrEq(L"C:\\work\\sample.txt"));

	// 代入メソッドの確認（オーバーフロー）
	EXPECT_THAT(path.assign(tooLong), Eq(STRUNCATE));
	EXPECT_THAT(path.length(), Eq(_MAX_PATH - 1));
	EXPECT_THAT(path.c_str()[_MAX_PATH - 1], Eq(L'\0'));

	EXPECT_THAT(path.assign(L"abc"), Eq(0));

	EXPECT_THAT(path.append(tooLong), Eq(STRUNCATE));
	EXPECT_THAT(path.length(), Eq(_MAX_PATH - 1));
	EXPECT_THAT(path[0], Eq(L'a'));
	EXPECT_THAT(path[1], Eq(L'b'));
	EXPECT_THAT(path[2], Eq(L'c'));
	for (size_t i = 3; i < _MAX_PATH - 1; ++i) {
		EXPECT_THAT(path[3], Eq(L'a'));
	}
	EXPECT_THAT(path.c_str()[_MAX_PATH - 1], Eq(L'\0'));

	// lengthがおかしいときの挙動確認
	::wmemcpy_s(std::data(path), std::size(path), std::data(tooLong), std::size(path));
	EXPECT_THAT(path.length(), Eq(_MAX_PATH - 1));

	EXPECT_THAT(wcsncpy_s(path, tooLong, _TRUNCATE), Eq(STRUNCATE));
	EXPECT_THAT(path.data()[_MAX_PATH - 1], Eq(0));

	EXPECT_THAT(wcscpy_s(path, L"abc"), Eq(0));
	EXPECT_THAT(wcsncat_s(path, L"DEFxyz", 3), Eq(0));
	EXPECT_THAT(path.c_str(), StrEq(L"abcDEF"));

	EXPECT_THAT(wcsncat_s(path, tooLong, _TRUNCATE), Eq(STRUNCATE));
	EXPECT_THAT(path.length(), Eq(_MAX_PATH - 1));

	path.assign(L"C:\\work");

	const std::wstring_view sv{ path };
	EXPECT_THAT(sv, StrEq(L"C:\\work"));

	const std::filesystem::path p{ path };
	EXPECT_THAT(p, StrEq(L"C:\\work"));

	const std::span<WCHAR> s{ path };
	EXPECT_THAT(std::data(s), StrEq(L"C:\\work"));

	// 既存メソッドの呼出確認
	path.Assign(L"C:\\work");
	EXPECT_THAT(path, StrEq(L"C:\\work"));

	// 呼ぶだけ
	EXPECT_THAT(path.GetBufferPointer(), path.c_str());
	EXPECT_THAT(path.At(0), path[0]);

	// begin/endの確認（非const版）
	path.assign(L"abc");
	EXPECT_THAT(static_cast<size_t>(std::distance(path.begin(), path.end())), Eq(path.GetBufferCount() - 1));
	*(path.begin() + 0) = L'X';
	*(path.begin() + 1) = L'Y';
	*(path.begin() + 2) = L'\0';
	EXPECT_THAT(path.c_str(), StrEq(L"XY"));

	// begin/endの確認（const版）
	const auto& constPath = path;
	EXPECT_THAT(static_cast<size_t>(std::distance(constPath.begin(), constPath.end())), Eq(constPath.length()));
}

} // namespace basis
