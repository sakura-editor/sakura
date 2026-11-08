/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "basis/make_initialization_error.hpp"

#include "util/tchar_convert.h"

namespace basis {

/*!
 * @brief 初期化失敗例外を生成する
 *
 * @param[in] message エラーメッセージ。
 * @param[in, opt] optExitCode 終了コード
 * @throws std::invalid_argument メッセージがSJISに変換できない文字を含んでいた場合
 */
initialization_error make_initialization_error(
	std::wstring_view message,
	std::optional<int> optExitCode
)
{
	// 初期化失敗例外を生成する
	return initialization_error(
		cxx::to_string(message, CP_ACP),
		message,
		optExitCode
	);
}

}	// namespace basis
