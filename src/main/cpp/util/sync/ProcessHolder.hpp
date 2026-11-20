/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/sync/HandleHolder.hpp"

#include <type_traits>

#include <Windows.h>

namespace cxx {

/*!
 * @brief 起動したプロセスをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class ProcessHolder : public cxx::HandleHolder
{
private:
	using Base = cxx::HandleHolder;
	using Me = ProcessHolder;

public:
	static Me CreateControlProcess(
		const std::optional<std::wstring_view>& optProfileName = std::nullopt
	);

	static Me CreateProcessW(
		const std::filesystem::path& exePath,
		std::wstring& strCommandLine,
		DWORD dwCreationFlag,
		const std::optional<std::filesystem::path>& optWorkingDir,
		STARTUPINFO& si
	);

	static Me CreateSakuraProcess(
		std::vector<std::wstring>& args,
		DWORD dwCreationFlag,
		const std::optional<std::filesystem::path>& optWorkingDir,
		STARTUPINFO& si,
		const std::optional<std::wstring_view>& optProfileName = std::nullopt
	);

	ProcessHolder(
		HANDLE hProcess,
		DWORD dwProcessId,
		DWORD dwThreadId
	)
		: Base(hProcess)
		, dwProcessId(dwProcessId)
		, dwThreadId(dwThreadId)
	{
	}

	ProcessHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	ProcessHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	DWORD dwProcessId;
	DWORD dwThreadId;
};

} // namespace cxx

