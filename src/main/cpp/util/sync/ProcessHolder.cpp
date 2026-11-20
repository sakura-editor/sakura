/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/sync/ProcessHolder.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

#include <system_error>

namespace cxx {

/*!
 * @brief プロセスを起動する
 *
 * @param[in] exePath 実行可能モジュールパス
 * @param[in] strCommandLine コマンドラインバッファ
 * @param[in] si スタートアップ情報
 * @param[in] dwCreationFlag 作成フラグ
 * @param[in] optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @return 起動したプロセスオブジェクト
 */
/* static */ ProcessHolder ProcessHolder::CreateProcessW(
	const std::filesystem::path& exePath,
	std::wstring& strCommandLine,
	DWORD dwCreationFlag,
	const std::optional<std::filesystem::path>& optWorkingDir,
	STARTUPINFO& si
)
{
	auto lpszCommandLine = std::data(strCommandLine);

	LPCWSTR lpszWorkingDir = nullptr;
	if (optWorkingDir.has_value()) {
		if (const auto attr = ::GetFileAttributesW(optWorkingDir->c_str());
			INVALID_FILE_ATTRIBUTES != attr && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			lpszWorkingDir = optWorkingDir->c_str();
		}
	}

	// プロセス情報（出力用構造体なので値は入れない）
	PROCESS_INFORMATION pi{};

	// コントロールプロセスを起動する
	if (!Kernel32::getInstance()->CreateProcessW(
		exePath.c_str(),	// 実行可能モジュールパス
		lpszCommandLine,	// コマンドラインバッファ
		nullptr,			// プロセスのセキュリティ記述子
		nullptr,			// スレッドのセキュリティ記述子
		FALSE,				// ハンドルの継承オプション(継承させない)
		dwCreationFlag,		// 作成のフラグ
		nullptr,			// 環境変数(変更しない)
		lpszWorkingDir,		// カレントディレクトリ(変更しない)
		&si,				// スタートアップ情報
		&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		throw cxx::make_system_error("CreateProcessW() failed.");
	}

	// 開いたハンドルは使わないので閉じておく
	::CloseHandle(pi.hThread);

	return ProcessHolder{ pi.hProcess, pi.dwProcessId, pi.dwThreadId };
}

} // namespace cxx

