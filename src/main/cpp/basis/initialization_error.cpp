/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "basis/initialization_error.hpp"

namespace basis {

/*!
 * @brief 初期化失敗例外を生成する
 *
 * @param[in] what 例外の説明(ナロー文字列)
 * @param[in] message エラーメッセージ(ワイド文字列)
 * @param[in, opt] optExitCode 終了コード
 */
initialization_error::initialization_error(
	std::string_view what,
	std::wstring_view message,
	std::optional<int> optExitCode
)
	: Base{ std::data(what) }
	, m_Message{ message }
	, m_ExitCode{ optExitCode.has_value() ? std::make_optional(EAppExitCode(*optExitCode)) : std::nullopt }
{
}

}	// namespace basis
