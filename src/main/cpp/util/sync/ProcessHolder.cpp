/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/sync/ProcessHolder.hpp"

#include "_main/CCommandLine.h"
#include "config/system_constants.h"
#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

#include <system_error>

namespace cxx {

/*!
 * @brief コマンドライン引数を引用符で囲む
 *
 * @param arg [in] コマンドライン引数
 * @return 引用符で囲まれたコマンドライン引数
 */
std::wstring QuoteArg(const std::wstring_view arg)
{
	const auto endsWithQuote = arg.ends_with(LR"(")");
	const auto containsQuotes = std::wstring_view::npos != arg.find_first_of(LR"(")");
	if (const auto needsEscape = std::wstring_view::npos != arg.find_first_of(L"\t ");
		!endsWithQuote && containsQuotes)
	{
		return std::format(LR"("{:s}")", std::regex_replace(arg.data(), std::wregex(LR"(")"), LR"("")"));
	}
	else if (!endsWithQuote && needsEscape)
	{
		return std::format(LR"("{:s}")", arg);
	}
	else
	{
		return std::wstring(arg);
	}
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param[in] profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 */
/* static */ ProcessHolder ProcessHolder::CreateControlProcess(
	const std::optional<std::wstring_view>& optProfileName
)
{
	// 初期化完了イベントの名前を決める
	SFilePath initEventName{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
	if (optProfileName.has_value()) {
		initEventName += *optProfileName;
	}

	// プロセス起動前に初期化完了イベントを作成する
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);
	if (!hEvent || ERROR_ALREADY_EXISTS == ::GetLastError()) {
		throw cxx::make_system_error("create event failed.");
	}

	std::wstring title{ L"sakura control process" };

	auto commandArgs = std::vector<std::wstring>{ LR"(-NOWIN)" };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.lpTitle = std::data(title);
	si.wShowWindow = SW_SHOWDEFAULT;

	// コントロールプロセスを起動する
	auto cp = Me::CreateSakuraProcess(
		commandArgs,
		CREATE_DEFAULT_ERROR_MODE,
		std::nullopt,
		si,
		optProfileName
	);

	// 初期化完了を待つ
	cxx::WaitForMultipleObjects(std::array{ hEvent.get(), cp.get() }, false, std::chrono::seconds(15));

	// プロセスオブジェクトを返す
	return ProcessHolder{ cp.release(), cp.dwProcessId, cp.dwThreadId };
}

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

/*!
 * @brief サクラエディタのプロセスを起動する
 *
 * @param[in] si スタートアップ情報
 * @param[in] args コマンドライン引数
 * @param[in] optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @param[in] optProfileName プロファイル名（省略した場合は指定なし）
 * @return 起動したプロセスオブジェクト
 */
/* static */ ProcessHolder ProcessHolder::CreateSakuraProcess(
	std::vector<std::wstring>& args,
	DWORD dwCreationFlag,
	const std::optional<std::filesystem::path>& optWorkingDir,
	STARTUPINFO& si,
	const std::optional<std::wstring_view>& optProfileName
)
{
	// プロファイル名が指定されている場合は、そのまま使う
	if (optProfileName.has_value()) {
		args.emplace(args.begin(), std::format(LR"(-PROF="{}")", *optProfileName));
	}
	// 省略時は、プロセスのプロファイル名を使う
	else if (const auto pCommandLine = CCommandLine::getInstance();
		pCommandLine &&
		pCommandLine->IsSetProfile())
	{
		args.emplace(args.begin(), std::format(LR"(-PROF="{}")", GetProfileName()));
	}

	const auto exePath = GetExeFileName();

	auto strCommandLine = std::accumulate(
		args.begin(),
		args.end(),
		QuoteArg(exePath.native()),
		[](const std::wstring& a, std::wstring_view b) {
			return std::format(LR"({} {})", a, QuoteArg(b));
		}
	);

	strCommandLine.erase(strCommandLine.cbegin());	// 先頭のスペースを削除

	return Me::CreateProcessW(
		exePath,
		strCommandLine,
		dwCreationFlag,
		optWorkingDir,
		si
	);
}

} // namespace cxx

