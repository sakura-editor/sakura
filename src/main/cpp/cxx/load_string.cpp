/*
	Copyright (C) 2025, Sakura Editor Organization

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
 * @throw std::out_of_range 指定された文字列リソースが見付からなかった場合
 */
std::wstring_view load_string(UINT id, const std::optional<HMODULE>& optModule)
{
	if (std::numeric_limits<WORD>::max() < id) {
		throw std::invalid_argument("string resource id should be in WORD range!");
	}

	// RT_STRINGリソースのブロック番号を計算
	const auto block = WORD((id >> 4) + 1);
	const auto index = id & 0xF;

	// リソースを検索してロック
	return cxx::lock_resource<WCHAR>(
		block,
		[index] (std::span<const WCHAR> pResData) {
			// リソース内の文字列を検索
			for (size_t i = 0; i < index; ++i) {
				pResData = pResData.subspan(1 + pResData.front());
			}

			// 文字列の長さを取得
			const auto length = size_t(pResData.front());

			// 文字列をstd::wstring_viewに格納して返す
			return std::wstring_view{ &pResData[1], length };
		},
		RT_STRING,
		optModule
	);
}

/*!
 * リソース文字列をナロー文字列に変換する
 *
 * @param [in] id リソースID
 * @param [in, opt] optModule リソースDLLのハンドル（省略時はアプリリソースから取得）
 * @throw std::out_of_range 指定されたリソース見付からなかった場合
 * @throw std::invalid_argument ワイド文字列からナロー文字列への変換に失敗した場合
 */
std::string load_string_as_acp(UINT id, const std::optional<HMODULE>& optModule)
{
	return cxx::to_string(load_string(id, optModule));
}

} // namespace cxx
