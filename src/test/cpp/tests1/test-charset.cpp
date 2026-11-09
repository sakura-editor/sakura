/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <array>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "charset/charset.h"
#include "charset/CCodePage.h"

//! テストで使う文字コードの一覧。いずれも msCodeSet に登録されている
constexpr std::array codes = {
	CODE_SJIS, CODE_JIS, CODE_EUC, CODE_LATIN1,
	CODE_UNICODE, CODE_UNICODEBE, CODE_UTF8, CODE_CESU8,
};

/*!
	@brief 登録済みの文字コードに対して3つの名前が取得できること
*/
TEST(CCodeTypeName, ReturnsNamesForKnownCode)
{
	for (const auto code : codes) {
		const CCodeTypeName name(code);
		const wchar_t* pszNormal = name.Normal();
		const wchar_t* pszShort = name.Short();
		const std::wstring bracket = name.Bracket();

		ASSERT_NE(nullptr, pszNormal);
		ASSERT_NE(nullptr, pszShort);
		ASSERT_FALSE(bracket.empty());

		// Bracket() は Short() を "  [" と "]" で囲んだもの
		EXPECT_THAT(bracket, std::wstring(L"  [") + pszShort + L"]");
	}
}

/*!
	@brief 未登録の文字コードでは Normal() / Short() は nullptr、Bracket() は空文字列が返ること

	@note CODE_AUTODETECT は表示順の一覧にだけ載せ、名前引きの対象からは
		  外してある(charset.cpp の InitCodeSet を参照)。
*/
TEST(CCodeTypeName, ReturnsNothingForUnknownCode)
{
	const CCodeTypeName name(CODE_AUTODETECT);

	EXPECT_THAT(name.Normal(), IsNull());
	EXPECT_THAT(name.Short(), IsNull());
	EXPECT_THAT(name.Bracket(), IsEmpty());
}

/*!
	@brief 文字コード名の同時取得

	@remark 複数スレッドから CCodeTypeName::Bracket() を同時に呼んでも、
			戻り値が "  [<短縮名>]" の形式を保つことを確認する。
			組み立て用バッファがプロセス共有だと、並列Grepのワーカー間で
			内容が混ざり "  [EUC]EUC]" のような文字列がGrep結果に出力される。
	@note	競合は確率的に顕在化するため反復回数を多めに取る。合格は「競合が無いこと」の
			証明にはならないが、共有バッファへ戻してしまった際の回帰検出には働く。
*/
TEST(CCodeTypeName, BracketFromMultipleThreads_NotCorrupted)
{
	constexpr int nIterations = 20000;

	// 期待値はメインスレッドで先に確定させる(ここで InitCodeSet() も済む)
	std::vector<std::wstring> expected;
	for (const auto code : codes) {
		const wchar_t* pszShort = CCodeTypeName(code).Short();
		ASSERT_NE(nullptr, pszShort);
		expected.emplace_back(std::wstring(L"  [") + pszShort + L"]");
	}

	std::atomic<int> nMismatch{ 0 };
	std::vector<std::wstring> firstBad(codes.size());	// 各スレッドは自分の添字だけを書く
	{
		std::vector<std::jthread> workers;
		workers.reserve(codes.size());
		for (size_t i = 0; i < codes.size(); ++i) {
			workers.emplace_back([&, i]() {
				for (int n = 0; n < nIterations; ++n) {
					// 値で受け取って比較する
					const std::wstring actual = CCodeTypeName(codes[i]).Bracket();
					if (actual != expected[i]) {
						if (firstBad[i].empty()) firstBad[i] = actual;
						nMismatch.fetch_add(1);
					}
				}
			});
		}
	}	// jthread のデストラクタで join される

	// gtest の失敗報告はワーカー内では行わず、join 後にメインスレッドでまとめて行う
	std::wstring samples;
	for (size_t i = 0; i < codes.size(); ++i) {
		if (!firstBad[i].empty()) {
			samples += L" [expected=" + expected[i] + L" actual=" + firstBad[i] + L"]";
		}
	}
	EXPECT_THAT(nMismatch.load(), 0) << "Bracket() の戻り値が壊れました";
	EXPECT_TRUE(samples.empty());
}

/*!
	@brief 登録済みの文字コードでは CCodeTypeName::Bracket() と同じ表記が返ること
*/
TEST(CCodePage, GetNameBracketReturnsBracketForKnownCode)
{
	for (const auto code : codes) {
		EXPECT_THAT(CCodePage::GetNameBracket(code), CCodeTypeName(code).Bracket());
	}
}

/*!
	@brief コードページでは "  [CP...]" の表記が返ること
*/
TEST(CCodePage, GetNameBracketReturnsCodePageName)
{
	EXPECT_THAT(CCodePage::GetNameBracket(CODE_CPACP), StrEq(L"  [CP_ACP]"));
	EXPECT_THAT(CCodePage::GetNameBracket(CODE_CPOEM), StrEq(L"  [CP_OEM]"));

	// それ以外のコードページは番号がそのまま表記される
	EXPECT_THAT(CCodePage::GetNameBracket(932), StrEq(L"  [CP932]"));
}

/*!
	@brief 出力引数版は値返し版と同じ表記を書き込み、種別を戻り値で返すこと
*/
TEST(CCodePage, GetNameBracketWritesToBuffer)
{
	std::array<wchar_t, 100> buffer{};

	EXPECT_THAT(CCodePage::GetNameBracket(buffer, CODE_EUC), 1);
	EXPECT_THAT(buffer.data(), StrEq(CCodeTypeName(CODE_EUC).Bracket()));

	EXPECT_THAT(CCodePage::GetNameBracket(buffer, CODE_CPACP), 2);
	EXPECT_THAT(buffer.data(), StrEq(L"  [CP_ACP]"));
}

/*!
	@brief 出力先が空のときは std::invalid_argument を投げること
*/
TEST(CCodePage, GetNameBracketThrowsForEmptyBuffer)
{
	EXPECT_THROW(CCodePage::GetNameBracket(std::span<WCHAR>(), CODE_EUC), std::invalid_argument);
}
