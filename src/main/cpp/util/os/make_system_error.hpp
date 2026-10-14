/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/primitive.h"

#include <system_error>

namespace cxx {

/*!
 * @brief システムエラー例外を作る
 *
 * @tparam A エラーメッセージの文字列型（NUL終端文字列を生成できる型）
 * @param[in] message エラーメッセージ
 */
template <basis::NullTerminatedStringConstructible<ACHAR> A>
std::system_error make_system_error(
	const A& message
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<ACHAR> msg{ message };

	// システムエラー例外を作る
	return std::system_error(
		int(::GetLastError()),
		std::system_category(),
		msg.c_str()
	);
}

} // namespace cxx
