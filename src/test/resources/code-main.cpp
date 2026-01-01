/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"
#include "testing/StartEditorProcess.hpp"

/*!
 * テストモジュールのエントリポイント
 */
int wmain(int argc, wchar_t **argv)
{
	// 言語ID(日本語、日本)を導出する
	const auto langId = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);

	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	if (std::wstring command(GetCommandLineW());
		std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		// コマンドライン文字列の先頭に入っているアプリパスを除去する
		if (std::wsmatch m; std::regex_match(command, m, std::wregex(LR"(^(?:".+?"|\S+)\s+(.+))"))) {
			command = m[1];
		}

		// テスト実行時のロケールは日本語に固定する
		::SetThreadUILanguage(langId);	// スレッドのUI言語を変更

		// wWinMainを起動して結果を返して抜ける
		return testing::StartEditorProcess(command);
	}

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	printf("Running main() from %s\n", std::source_location::current().file_name());
	testing::InitGoogleMock(&argc, argv);

	// テスト実行時のロケールは日本語に固定する
	::SetThreadUILanguage(langId);	// スレッドのUI言語を変更

	// コードページを導出
	UINT codePage = CP_SJIS;
	const auto lcid = MAKELCID(langId, SORT_DEFAULT);

	std::wstring localeName{ LOCALE_NAME_MAX_LENGTH, L'\0' };
	const auto len = ::LCIDToLocaleName(lcid, std::data(localeName), LOCALE_NAME_MAX_LENGTH, 0);
	localeName.resize(len);

	const auto count = ::GetLocaleInfoEx(localeName.c_str(), LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, LPWSTR(&codePage), sizeof(DWORD) / sizeof(WCHAR));
	assert(sizeof(DWORD) / sizeof(WCHAR) == count);

	// Cロケールも日本語に固定
	::_wsetlocale(LC_ALL, std::format(L"{}.{}", localeName, codePage).c_str());

	return RUN_ALL_TESTS();
}
