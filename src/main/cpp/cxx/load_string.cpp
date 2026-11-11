/*
	Copyright (C) 2025-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "cxx/load_string.hpp"

#include "cxx/lock_resource.hpp"

#include "util/tchar_convert.h"

namespace cxx {

/*!
 * リソース文字列を読み込む
 *
 * @param [in] id リソースID
 * @param [in, opt] optModule リソースDLLのハンドル（省略時はアプリリソースから取得）
 * @throw std::system_error 指定された文字列リソースが見付からなかった場合
 */
std::wstring_view load_string(UINT id, const std::optional<HMODULE>& optModule)
{
	if (std::numeric_limits<WORD>::max() < id) {
		throw std::invalid_argument("string resource id should be in WORD range!");
	}

	// 文字列リソースを検索して参照を返す
#if 0
	// リソースを格納するモジュールを特定する(省略時はアプリリソースから取得)
	const auto hModule = optModule.value_or(::GetModuleHandleW(nullptr));

	// 文字列の先頭を受け取るためのポインター
	LPCWSTR lpBuffer = nullptr;

	// バッファサイズに0を指定してLoadStringWを呼び出すと、文字列の長さが返る
	const auto cchResStr = ::LoadStringW(hModule, id, LPWSTR(&lpBuffer), 0);

#if 0 // 新実装では↓の処理を行わない
	if (!cchResStr) {
		throw std::system_error("missing resource!");
	}
#endif

	// 先頭ポインターと文字列長で文字列参照を構築して返す
	return std::wstring_view(lpBuffer, cchResStr);

#else
	// RT_STRINGリソースのブロック番号を計算
	const auto block = WORD((id >> 4) + 1);
	const auto index = id & 0xF;

	// リソースを検索してロック
	return cxx::lock_resource<WCHAR>(
		block,
		[index] (std::span<const WCHAR> resData) {
			// リソースブロックが空の場合は例外を投げる
			if (resData.empty()) throw std::system_error(std::make_error_code(std::errc::not_supported), "String resource block is empty.");

			// リソース内の文字列を検索
			for (size_t i = 0; i < index; ++i) {
				// 文字列エントリのサイズを取得する
				const auto entrySize = 1 + size_t(resData.front());

				// 残りデータがエントリサイズに満たない場合は例外を投げる
				if (resData.size() < entrySize) throw std::system_error(std::make_error_code(std::errc::not_supported), "String resource entry is truncated.");

				// 次の文字列の先頭に移動する
				resData = resData.subspan(entrySize);

				// 移動した先に残りデータがない場合は例外を投げる
				if (resData.empty()) throw std::system_error(std::make_error_code(std::errc::not_supported), "String resource block does not contain the requested entry.");
			}

			// 文字列の長さを取得
			const auto length = size_t(resData.front());

			// 文字列の長さがゼロな場合は例外を投げる
			if (!length) throw std::system_error(std::make_error_code(std::errc::not_supported), "String resource entry is empty.");

			// 残りデータが取得サイズに満たない場合は例外を投げる
			if (std::size(resData) < 1 + length) throw std::system_error(std::make_error_code(std::errc::not_supported), "String resource entry is truncated.");

			// 文字列をstd::wstring_viewに格納して返す
			return std::wstring_view(resData.data() + 1, length);
		},
		RT_STRING,
		optModule
	);

#endif
}

/*!
 * リソース文字列をナロー文字列に変換する
 *
 * @param [in] id リソースID
 * @param [in, opt] optModule リソースDLLのハンドル（省略時はアプリリソースから取得）
 * @throw std::system_error 指定されたリソース見付からなかった場合
 * @throw std::invalid_argument ワイド文字列からナロー文字列への変換に失敗した場合
 */
std::string load_string_as_acp(UINT id, const std::optional<HMODULE>& optModule)
{
	return cxx::to_string(load_string(id, optModule));
}

} // namespace cxx
