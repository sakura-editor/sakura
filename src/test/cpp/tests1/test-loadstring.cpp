/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "CSelectLang.h"

#include "util/os/MockKernel32.hpp"

#include <thread>

namespace cxx {

void CheckLangId(
	WORD wLangId,
	WORD wId,
	HMODULE hModule
);

}	// namespace cxx

TEST(CSelectLang, test001)
{
	// 強制的に初期状態にする
	CSelectLang::UninitializeLanguageEnvironment();

	EXPECT_THAT(CSelectLang::getLangRsrcInstance(), NotNull());
	EXPECT_THAT(CSelectLang::getDefaultLangId(), 0x0411);
	EXPECT_THAT(CSelectLang::getDefaultLangString(), StrEq(L"Japanese"));
	EXPECT_THAT(CSelectLang::GetLangInfo().size(), Ge(size_t(1)));	// 未初期化状態のテストをやめて、初期状態をテストするよう仕様変更。
}

TEST(CSelectLang, test002)
{
	// 初期化する
	CSelectLang::InitializeLanguageEnvironment();

	EXPECT_THAT(CSelectLang::GetLangInfo().size(), Ge(size_t(1)));

	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");

	EXPECT_THAT(CSelectLang::getLangRsrcInstance(), NotNull());
	EXPECT_THAT(CSelectLang::getDefaultLangId(), 0x0409);
	EXPECT_THAT(CSelectLang::getDefaultLangString(), StrEq(L"English (United States)"));
	EXPECT_THAT(CSelectLang::GetLangInfo(1).GetDllName(), StrEq(L"sakura_lang_en_US.dll"));
	EXPECT_THAT(CSelectLang::GetLangInfo(1).GetLangName(), StrEq(L"English (United States)"));

	CSelectLang::ChangeLang(L"");

	EXPECT_THAT(CSelectLang::getLangRsrcInstance(), NotNull());
	EXPECT_THAT(CSelectLang::getDefaultLangId(), 0x0411);
	EXPECT_THAT(CSelectLang::getDefaultLangString(), StrEq(L"Japanese"));
	EXPECT_THAT(CSelectLang::GetLangInfo(0).GetDllName(), StrEq(L""));
	EXPECT_THAT(CSelectLang::GetLangInfo(0).GetLangName(), StrEq(L"Japanese"));

	// 強制的に初期状態にする
	CSelectLang::UninitializeLanguageEnvironment();
}

TEST(CSelectLang, ChangeLang101)
{
	CSelectLang::InitializeLanguageEnvironment();

	Kernel32::setInstance<MockKernel32>();
	auto pKernel32 = (MockKernel32*)Kernel32::getInstance();

	EXPECT_CALL(*pKernel32, LoadLibraryExW(_, _, _))
		.WillOnce(testing::Throw(std::system_error(ERROR_NOT_FOUND, std::system_category())));

	CSelectLang::ChangeLang(L"sakura_lang_zh_CN.dll");

	Kernel32::resetInstance();
}

TEST(CSelectLang, ChangeLang102)
{
	CSelectLang::InitializeLanguageEnvironment();

	CSelectLang::ChangeLang(L"sakura_lang_en_JP.dll");
}

TEST(CSelectLang, InitializeLanguageEnvironment101)
{
	Kernel32::setInstance<MockKernel32>();
	auto pKernel32 = (MockKernel32*)Kernel32::getInstance();

	EXPECT_CALL(*pKernel32, FindFirstFileW(_, _))
		.WillOnce(Return(INVALID_HANDLE_VALUE));

	CSelectLang::InitializeLanguageEnvironment();

	Kernel32::resetInstance();
}

TEST(CSelectLang, InitializeLanguageEnvironment102)
{
	Kernel32::setInstance<MockKernel32>();
	auto pKernel32 = (MockKernel32*)Kernel32::getInstance();

	EXPECT_CALL(*pKernel32, FindNextFileW(_, _))
		.WillOnce(testing::Throw(std::system_error(ERROR_NOT_FOUND, std::system_category())));

	CSelectLang::InitializeLanguageEnvironment();

	Kernel32::resetInstance();
}

TEST(CSelectLang, TryEmplace101)
{
	CSelectLang::InitializeLanguageEnvironment();

	Kernel32::setInstance<MockKernel32>();
	auto pKernel32 = (MockKernel32*)Kernel32::getInstance();

	EXPECT_CALL(*pKernel32, LoadLibraryExW(_, _, _))
		.WillOnce(testing::Throw(std::system_error(ERROR_NOT_FOUND, std::system_category())));

	CSelectLang::TryEmplace(L"sakura_lang_en_JP.dll", 0x411);

	Kernel32::resetInstance();
}

TEST(CheckLangId, test101)
{
	EXPECT_THROW(
		cxx::CheckLangId(0x0409, STR_SELLANG_LANGID, nullptr),
		std::system_error
	);
}

/*!
 * @brief リソース文字列の読込テスト
 *
 * tests1.exeに埋め込んだsakura_rc.rcの値を読み込めるかチェックするテスト。
 */
TEST(LoadStringW, LoadStringResource001)
{
	CSelectLang::InitializeLanguageEnvironment();

	// リソースから言語識別子のIDを読み取る
	// ここのリソース文字列値は、言語選択のキーなので変更してはならない。
	EXPECT_THAT(LS(STR_SELLANG_LANGID), StrEq(L"0x0411"));

	// リソースから選択中言語のラベル文字列を読み取る
	// これは共通設定の選択中言語のとこに表示するラベル文字列。
	// 古いWindows APIが言語表示名を提供してなかったことに起因するリソース。
	EXPECT_THAT(LS(STR_SELLANG_NAME), StrEq(L"Japanese"));
}

TEST(LoadStringW, LoadStringResource002)
{
	// ロケールを設定
	::SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));

	EXPECT_THAT(cxx::load_string_as_acp(STR_SELLANG_LANGID), StrEq("0x0411"));
	EXPECT_THAT(cxx::load_string_as_acp(STR_SELLANG_NAME), StrEq("Japanese"));
	EXPECT_THAT(cxx::load_string_as_acp(F_FILENEW), StrEq("新規作成"));
}

TEST(LoadStringW, LoadStringResource100)
{
	// ID範囲を越える値を指定した場合は例外が発生する
	EXPECT_ANY_THROW(cxx::load_string(std::numeric_limits<WORD>::max() + 1));
}

TEST(LoadStringW, LoadStringResource101)
{
	CSelectLang::InitializeLanguageEnvironment();

	// 対応する文字列リソースが存在しない機能IDを指定
	EXPECT_THAT(LS(F_EXPANDPARAMETER), StrEq(L""));
}

/*!
 * @brief LS() のバッファがスレッドごとに独立していること
 *
 * 別スレッドが LS() をバッファの個数(16)より多く呼んでも、
 * このスレッドで取得したポインタの内容は変わらない。
 * プロセス共有のバッファだと一巡して上書きされ、"0x0411" に変わる。
 */
TEST(LoadStringW, LoadStringStIsThreadLocal)
{
	CSelectLang::InitializeLanguageEnvironment();

	const auto pszName = LS(STR_SELLANG_NAME);
	ASSERT_THAT(pszName, StrEq(L"Japanese"));

	std::jthread worker([] {
		for (int i = 0; i < 32; ++i) {
			LS(STR_SELLANG_LANGID);
		}
	});
	worker.join();

	EXPECT_THAT(pszName, StrEq(L"Japanese"));
}
