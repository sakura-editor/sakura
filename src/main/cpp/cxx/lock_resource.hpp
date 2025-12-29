/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "cxx/type_of_Nth_lambda_arg.hpp"

namespace cxx {

template<typename T, typename TAction, typename R = lambda_traits<TAction>::return_type>
R lock_resource(WORD id, TAction action, _In_ LPCWSTR resourceType, const std::optional<HMODULE>& optModule = std::nullopt) {
	const auto hInstance = HINSTANCE(optModule.value_or(nullptr));

	// リソースを検索
	const auto hResInfo = ::FindResourceW(hInstance, MAKEINTRESOURCE(id), resourceType);
	if (!hResInfo) {
		throw std::out_of_range("missing resource!");
	}

	// リソースをロード
	const auto hResData = ::LoadResource(hInstance, hResInfo);
	if (!hResData) {
		throw std::system_error(int(::GetLastError()), std::system_category(), "cannot load resource!");
	}

	// リソースのサイズを取得
	const auto cbSize = ::SizeofResource(hInstance, hResInfo);
	if (cbSize < 1) {
		throw std::system_error(int(::GetLastError()), std::system_category(), "invalid resource size!");
	}

	// リソースをロック
	auto pResData = static_cast<T*>(::LockResource(hResData));
	if (!pResData) {
		throw std::system_error(int(::GetLastError()), std::system_category(), "cannot lock resource!");
	}

	return action(std::span(pResData, cbSize / sizeof(T)));
}

} // end of namespace cxx
