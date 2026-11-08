/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "_main/CProcess.h"	// EAppExitCode

#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>

namespace basis {

/*!
 * @brief 初期化失敗例外
 *
 * アプリ初期化中に発生したエラーを wWinMain に伝えるための例外クラス。
 */
class initialization_error final : public std::runtime_error {
private:
	using Base = std::runtime_error;
	using Me = initialization_error;

public:
	explicit initialization_error(
		std::string_view what,
		std::wstring_view message,
		std::optional<int> optExitCode = std::nullopt
	);

	auto message() const -> std::wstring { return m_Message; }

	auto exitCode() const -> std::optional<EAppExitCode> { return m_ExitCode; }

private:
	std::wstring m_Message;
	std::optional<EAppExitCode> m_ExitCode;
};

}	// namespace basis
