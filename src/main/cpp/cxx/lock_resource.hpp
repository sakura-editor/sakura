/*
	Copyright (C) 2025-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "cxx/type_of_Nth_lambda_arg.hpp"

#include <span>

namespace cxx {

/*!
 * @brief リソースを検索する
 */
HRSRC FindResourceW(
	_In_opt_ HMODULE hModule,
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType
);

/*!
 * @brief リソースを読み込む
 */
HGLOBAL LoadResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
);

/*!
 * @brief リソースデータのポインターを取得する
 */
LPVOID LockResource(
	_In_ HGLOBAL hResData
);

/*!
 * @brief リソースのバイト数を取得する
 */
DWORD SizeofResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
);

template <typename T>
std::span<T> LoadResourceAs(
	HINSTANCE hInstance,
	HRSRC hResInfo
);

template<typename T, typename TAction, typename R = lambda_traits<TAction>::return_type>
R lock_resource(WORD id, TAction action, _In_ LPCWSTR resourceType, const std::optional<HMODULE>& optModule = std::nullopt) {
	const auto hInstance = HINSTANCE(optModule.value_or(nullptr));

	// リソースを検索
	const auto hResInfo = cxx::FindResourceW(hInstance, MAKEINTRESOURCE(id), resourceType);

	// リソースをロード
	return action(cxx::LoadResourceAs<T>(hInstance, hResInfo));
}


template <typename T>
std::span<T> LoadResourceAs(
	HINSTANCE hInstance,
	HRSRC hResInfo
)
{
	// リソースをロード
	const auto hResData = cxx::LoadResource(hInstance, hResInfo);

	// リソースのサイズを取得
	const auto cbSize = cxx::SizeofResource(hInstance, hResInfo);

	// リソースをロック
	auto pResData = static_cast<T*>(cxx::LockResource(hResData));

	return std::span(pResData, cbSize / sizeof(T));
}

} // end of namespace cxx
