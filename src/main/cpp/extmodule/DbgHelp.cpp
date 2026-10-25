/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "extmodule/DbgHelp.hpp"

#include "util/os/make_system_error.hpp"

#include <ranges>

// ローカルマクロを定義するため、既存マクロを退避する
#pragma push_macro("DECLARE_API_FUNCTION")

/*!
 * @brief API関数を宣言するためのマクロ
 *
 * @param name API関数名
 */
#define DECLARE_API_FUNCTION(name) \
	#name

/*!
 * @brief 利用する DbgHelp APIの関数のリスト
 */
constexpr std::array g_DbgHelpFunctions{
	DECLARE_API_FUNCTION(MiniDumpWriteDump),
	DECLARE_API_FUNCTION(StackWalk64),
	DECLARE_API_FUNCTION(SymCleanup),
	DECLARE_API_FUNCTION(SymFromAddr),
	DECLARE_API_FUNCTION(SymFunctionTableAccess64),
	DECLARE_API_FUNCTION(SymGetModuleBase64),
	DECLARE_API_FUNCTION(SymInitialize),
	DECLARE_API_FUNCTION(SymSetOptions),
};

// 既存マクロを復元する
#pragma pop_macro("DECLARE_API_FUNCTION")

/*!
 * @brief コンストラクタ
 *
 * DbgHelp APIの関数ポインタをロードする。
 * DLLが存在しなかったり、必要なAPI関数が存在しなかったりすると例外を投げる。
 *
 * @throws std::system_error DbgHelp APIのロードに失敗した場合（≒正規品Windowsでない場合）
 */
DbgHelp::DbgHelp()
{
	// DbgHelp.DLLをロードできなかった場合は例外を投げる
	if (!m_DbgHelp) throw std::system_error(std::make_error_code(std::errc::not_supported), "missing DbgHelp module");

	// DbgHelp APIの関数ポインタをロードする
	std::ranges::for_each(
		g_DbgHelpFunctions,
		[this, hModule = m_DbgHelp.get()] (const auto& name)
	{
		const auto addr = ::GetProcAddress(
			hModule,
			name
		);

		if (!addr) throw std::system_error(std::make_error_code(std::errc::not_supported), std::format("missing symbol '{}'", name));

		m_Functions.try_emplace(name, addr);
	});
}
