/*! @file */
/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/StaticType.h"

#include "basis/CMyString.h"
#include "mem/CNativeW.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

static_assert(!basis::StringViewCompatible<LPCWSTR, WCHAR>);
static_assert(basis::StringViewCompatible<CNativeW, WCHAR>);
static_assert(basis::StringViewCompatible<CStringRef, WCHAR>);
static_assert(basis::StringViewCompatible<SFilePath, WCHAR>);
static_assert(!basis::StringViewCompatible<std::filesystem::path, WCHAR>);
static_assert(!basis::StringViewCompatible<std::nullptr_t, WCHAR>);
static_assert(basis::StringViewCompatible<std::wstring, WCHAR>);
static_assert(basis::StringViewCompatible<std::wstring_view, WCHAR>);
static_assert(!basis::StringViewCompatible<WCHAR[16], WCHAR>);
static_assert(!basis::StringViewCompatible<const WCHAR[16], WCHAR>);
static_assert(!basis::StringViewCompatible<std::array<WCHAR, 16>, WCHAR>);
static_assert(!basis::StringViewCompatible<std::vector<WCHAR>, WCHAR>);

static_assert(!basis::StringConstructible<LPCWSTR, WCHAR>);
static_assert(!basis::StringConstructible<CNativeW, WCHAR>);
static_assert(!basis::StringConstructible<CStringRef, WCHAR>);
static_assert(!basis::StringConstructible<SFilePath, WCHAR>);
static_assert(basis::StringConstructible<std::filesystem::path, WCHAR>);
static_assert(!basis::StringConstructible<std::nullptr_t, WCHAR>);
static_assert(!basis::StringConstructible<std::wstring, WCHAR>);
static_assert(!basis::StringConstructible<std::wstring_view, WCHAR>);
static_assert(!basis::StringConstructible<WCHAR[16], WCHAR>);
static_assert(!basis::StringConstructible<const WCHAR[16], WCHAR>);
static_assert(!basis::StringConstructible<std::array<WCHAR, 16>, WCHAR>);
static_assert(!basis::StringConstructible<std::vector<WCHAR>, WCHAR>);

static_assert(basis::NullTerminatedStringConstructible<WCHAR*, WCHAR>);
static_assert(basis::NullTerminatedStringConstructible<const WCHAR*, WCHAR>);
static_assert(!basis::NullTerminatedStringConstructible<void*, WCHAR>);
static_assert(!basis::NullTerminatedStringConstructible<int*, WCHAR>);
static_assert(basis::NullTerminatedStringConstructible<WCHAR[16], WCHAR>);
static_assert(basis::NullTerminatedStringConstructible<const WCHAR[16], WCHAR>);
static_assert(!basis::NullTerminatedStringConstructible<int[16], WCHAR>);
static_assert(!basis::NullTerminatedStringConstructible<volatile WCHAR[16], WCHAR>);

static_assert(!basis::WritableBuffer<LPCWSTR, WCHAR>);
static_assert(!basis::WritableBuffer<CNativeW, WCHAR>);
static_assert(!basis::WritableBuffer<CStringRef, WCHAR>);
static_assert(basis::WritableBuffer<SFilePath, WCHAR>);
static_assert(!basis::WritableBuffer<std::filesystem::path, WCHAR>);
static_assert(!basis::WritableBuffer<std::nullptr_t, WCHAR>);
static_assert(!basis::WritableBuffer<std::wstring, WCHAR>);
static_assert(!basis::WritableBuffer<std::wstring_view, WCHAR>);
static_assert(basis::WritableBuffer<WCHAR[16], WCHAR>);
static_assert(!basis::WritableBuffer<const WCHAR[16], WCHAR>);
static_assert(basis::WritableBuffer<std::array<WCHAR, 16>, WCHAR>);
static_assert(basis::WritableBuffer<std::vector<WCHAR>, WCHAR>);

namespace basis {

static_assert([] {
	const StaticVector<size_t, 3> vec{ 10u, 20u, 30u };
	return vec.size() == 3 && vec[0] == 10u && vec[1] == 20u && vec[2] == 30u;
}());

static_assert([] {
	const std::vector<size_t> source{ 10u, 20u, 30u };
	const StaticVector<size_t, 3> vec(source);
	return vec.size() == 3 && vec[0] == 10u && vec[1] == 20u && vec[2] == 30u;
}());

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
	EXPECT_THROW({ vec.push_back(0xffffff); }, std::out_of_range);

	// 追加できないので、サイズをカウントアップしてはいけない
	EXPECT_THAT(vec.size(), Eq(3));

	// 範囲外アクセス
	try {
		auto cv = vec;
		cv[3];

		FAIL() << "Expected std::out_of_range to be thrown";
	}
	catch (const std::out_of_range& e) {
		EXPECT_THAT(e.what(), StrEq("nIndex is out of range. (nIndex: 3, allowed: 2)"));
	}

	// 範囲外アクセス
	try {
		const auto& cv = vec;
		cv[3];

		FAIL() << "Expected std::out_of_range to be thrown";
	}
	catch (const std::out_of_range& e) {
		EXPECT_THAT(e.what(), StrEq("nIndex is out of range. (nIndex: 3, allowed: 2)"));
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
		EXPECT_THAT(e.what(), StrEq("source has too many elements. (elements: 4, allowed: 3)"));
	}

	// 長さ3の配列を用意する
	const std::array<size_t, 3> source{ 10u, 20u, 30u };

	vec = StaticVector<size_t, 3>(source);

	EXPECT_THAT(vec.size(), Eq(3));
	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(vec[2], Eq(30u));
	EXPECT_THAT(std::distance(vec.begin(), vec.end()), Eq(3));

	const std::vector<size_t> vectorSource{ 10u, 20u, 30u };
	vec = StaticVector<size_t, 3>(vectorSource);

	EXPECT_THAT(vec.size(), Eq(3));
	EXPECT_THAT(vec[0], Eq(10u));
	EXPECT_THAT(vec[1], Eq(20u));
	EXPECT_THAT(vec[2], Eq(30u));

	const std::vector<size_t> tooLargeVector{ 10u, 20u, 30u, 40u };
	EXPECT_THROW((StaticVector<size_t, 3>(tooLargeVector)), std::out_of_range);
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

// swprintf_sの確認
TEST(StaticString, swprintf_s001)
{
	std::wstring expected(259, L'a');

	SFilePath path{};
	EXPECT_THAT(swprintf_s(path, L"%s", std::data(expected)), Eq(259));
	EXPECT_THAT(path, StrEq(expected));
}

// swprintf_sの確認
TEST(StaticString, swprintf_s101)
{
	std::wstring expected(259, L'b');
	std::wstring dummy(260, L'b');

	SFilePath path{ std::wstring(259, L'a') };
	EXPECT_THAT(swprintf_s(path, L"%s", std::data(dummy)), Lt(0));
	EXPECT_THAT(path, StrEq(expected));
}

TEST(StaticString, test101)
{
	EXPECT_THAT(([] {
		// バッファサイズより文字列が長いと例外。
		StaticString<4> str{ L"test"}; }),
		ThrowsMessage<std::out_of_range>(Eq("source string is too long. (length: 4, allowed: 3)"))
	);
}

TEST(StaticString, test102)
{
	EXPECT_THAT(([] {
		// バッファサイズより文字列が長いと例外。
		StaticString<4> str{};
		str = L"test"; }),
		ThrowsMessage<std::out_of_range>(Eq("source string is too long. (length: 4, allowed: 3)"))
	);
}

TEST(StaticString, test103)
{
	EXPECT_THAT(([] {
		// 残りバッファサイズより文字列が長いと例外。
		StaticString<4> str{ L"te" };
		str += L"st"; }),
		ThrowsMessage<std::out_of_range>(Eq("source string is too long. (length: 2, allowed: 1)"))
	);
}

} // namespace basis

namespace cxx {

//! @brief NUL終端文字列と互換性のある型で初期化
TEST(NullTerminatedString, test001)
{
	auto testData = L"test"s;
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData.c_str()));
}

//! @brief std::filesystem::path型で初期化
TEST(NullTerminatedString, test002)
{
	auto testData = std::filesystem::path("test");
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData.c_str()));
}

//! @brief NULで初期化
TEST(NullTerminatedString, test003)
{
	auto value = cxx::NullTerminatedString(nullptr);
	EXPECT_THAT(static_cast<LPCWSTR>(value), IsNull());
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), IsNull());
}

//! @brief 配列で初期化
TEST(NullTerminatedString, test004)
{
	WCHAR testData[_MAX_PATH] = L"test";
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData));
}

//! @brief 配列で初期化
TEST(NullTerminatedString, test005)
{
	const auto& testData = L"test\0data";
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq<LPCWSTR>(testData));
}

//! @brief ポインタで初期化
TEST(NullTerminatedString, test006)
{
	LPCWSTR testData = L"test";
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData));
}

//! @brief ポインタで初期化
TEST(NullTerminatedString, test007)
{
	LPCWSTR testData = nullptr;
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), IsNull());
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), IsNull());
}

//! @brief 文字列として扱える型で初期化
TEST(NullTerminatedString, test008)
{
	auto testData = L""sv;
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L""));
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Ne(testData.data()));
}

//! @brief 文字列として扱える型で初期化
TEST(NullTerminatedString, test009)
{
	auto testData = L"test"sv;
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData.data()));
}

//! @brief 文字列として扱える型で初期化
TEST(NullTerminatedString, test00A)
{
	auto testData = L"testData"sv.substr(0, 4);
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsTrue());
	EXPECT_THAT(value.c_str(), Ne(testData.data()));
}

//! @brief 文字列表現を生成できる型で初期化
TEST(NullTerminatedString, test00B)
{
	struct Dummy {
		explicit operator std::wstring() const { return L"test"; }
	};

	auto testData = Dummy();
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L"test"));
	EXPECT_THAT(value, StrEq(L"test"));
	EXPECT_THAT(value.uses_buffer(), IsTrue());
}

//! @brief NUL終端文字列と互換性のある型で初期化
TEST(NullTerminatedString, test00C)
{
	auto testData = L""s;
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L""));
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Eq(testData.data()));
}

//! @brief 文字列として扱える型で初期化
TEST(NullTerminatedString, test00D)
{
	auto testData = L"testData"sv.substr(0, 0);
	auto value = cxx::NullTerminatedString(testData);
	EXPECT_THAT(static_cast<LPCWSTR>(value), StrEq(L""));
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
	EXPECT_THAT(value.c_str(), Ne(testData.data()));
}

TEST(NullTerminatedString, test101)
{
	auto value = cxx::NullTerminatedString(LPCWSTR(nullptr), 0);
	EXPECT_THAT(static_cast<LPCWSTR>(value), IsNull());
	EXPECT_THAT(value.c_str(), IsNull());
	EXPECT_THAT(value.uses_buffer(), IsFalse());
}

TEST(NullTerminatedString, test102)
{
	auto value = cxx::NullTerminatedString(L"", 0);
	EXPECT_THAT(static_cast<LPCWSTR>(value), NotNull());
	EXPECT_THAT(value, StrEq(L""));
	EXPECT_THAT(value.uses_buffer(), IsFalse());
}

//! @brief 配列で初期化
TEST(NullTerminatedString, test103)
{
	EXPECT_THAT(([] {
		WCHAR testData[5] = {};
		std::wstring dummy(5, L'a');
		std::ranges::copy_n(dummy.begin(), 5, std::begin(testData));
		auto value = cxx::NullTerminatedString(testData); }),
		ThrowsMessage<std::invalid_argument>(Eq("char array should be null-terminated."))
	);
}

} // namespace cxx
