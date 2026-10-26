/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/sync/MutexHandle.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

#include <string_view>
#include <system_error>

namespace cxx {

/*!
 * @brief ミューテックスハンドルを作成する
 *
 * @param[in] lpMutexAttributes セキュリティ属性
 * @param[in] bInitialOwner 初期所有者フラグ
 * @param[in] name ミューテックス名
 */
/* static */ MutexHandle MutexHandle::CreateMutexW(
	_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
	bool bInitialOwner,
	std::wstring_view name
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Name{ name };

	// ミューテックスハンドルを作成する
	const auto hMutex = Kernel32::getInstance()->CreateMutexW(
		lpMutexAttributes,
		bInitialOwner,
		_Name.c_str()
	);

	// 戻り値が NULL だった場合
	if (!hMutex) {
		// システムエラー例外を発生させる
		throw cxx::make_system_error("CreateMutexW() failed");
	}

	auto ret = MutexHandle{ hMutex };

	if (bInitialOwner && ERROR_ALREADY_EXISTS == ::GetLastError()) {
		// システムエラー例外を発生させる
		throw std::system_error(
			ERROR_ALREADY_EXISTS,
			std::system_category(),
			"CreateMutexW() failed"
		);
	}

	return ret;
}

} // namespace cxx
