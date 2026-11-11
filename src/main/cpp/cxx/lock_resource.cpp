/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "cxx/lock_resource.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

#include <string_view>

namespace cxx {

/*!
 * @brief リソースを検索する
 */
HRSRC FindResourceW(
	_In_opt_ HMODULE hModule,
	_In_ LPCWSTR lpName,
	_In_ LPCWSTR lpType
)
{
	const auto resource = Kernel32::getInstance()->FindResourceW(hModule, lpName, lpType);

	if (!resource) {
		throw cxx::make_system_error("FindResourceW() failed");
	}

	return resource;
}

/*!
 * @brief リソースを読み込む
 */
HGLOBAL LoadResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
)
{
	const auto resource = Kernel32::getInstance()->LoadResource(hModule, hResInfo);

	if (!resource) {
		throw cxx::make_system_error("LoadResource() failed");
	}

	return resource;
}

/*!
 * @brief リソースデータのポインターを取得する
 */
LPVOID LockResource(
	_In_ HGLOBAL hResData
)
{
	const auto data = Kernel32::getInstance()->LockResource(hResData);

	if (!data) {
		// LockResource は GetLastError に対応していない。
		throw std::system_error(
			ERROR_RESOURCE_DATA_NOT_FOUND,
			std::system_category(),
			"LockResource() failed"
		);
	}

	return data;
}

/*!
 * @brief リソースのバイト数を取得する
 */
DWORD SizeofResource(
	_In_opt_ HMODULE hModule,
	_In_ HRSRC hResInfo
)
{
	const auto size = Kernel32::getInstance()->SizeofResource(hModule, hResInfo);

	if (!size) throw cxx::make_system_error("SizeofResource() failed");

	return size;
}

} // namespace cxx
