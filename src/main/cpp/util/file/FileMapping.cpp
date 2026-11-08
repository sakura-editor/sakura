/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/FileMapping.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

namespace cxx {

/*!
 * @brief ファイルマッピングを作る
 *
 * @param[in] hFile ファイルハンドル
 * @param[in] lpFileMappingAttributes セキュリティ属性
 * @param[in] flProtect 保護属性
 * @param[in] maximumSize 最大サイズ
 * @param[in] name ファイルマッピングの名前
 */
/* static */ FileMapping FileMapping::CreateFileMappingW(
	_In_ HANDLE hFile,
	_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
	_In_ DWORD flProtect,
	const ULARGE_INTEGER& maximumSize,
	std::wstring_view name
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Name{ name };

	// ファイルマッピングを作る
	const auto hFileMapping = Kernel32::getInstance()->CreateFileMappingW(
		hFile,
		lpFileMappingAttributes,
		flProtect,
		maximumSize.HighPart,
		maximumSize.LowPart,
		_Name.c_str()
	);

	// 戻り値が NULL だった場合
	if (!hFileMapping) {
		// システムエラー例外を発生させる
		throw cxx::make_system_error("CreateFileMappingW() failed");
	}

	return FileMapping{ hFileMapping };
}

/*!
 * @brief ファイルマッピングを開く
 *
 * @param[in] dwDesiredAccess 要求するアクセス権
 * @param[in] bInheritHandle ハンドルを継承するかどうか
 * @param[in] name ファイルマッピングの名前
 */
/* static */ FileMapping FileMapping::OpenFileMappingW(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOL bInheritHandle,
	std::wstring_view name
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Name{ name };

	// ファイルマッピングを開く
	const auto hFileMapping = Kernel32::getInstance()->OpenFileMappingW(
		dwDesiredAccess,
		bInheritHandle,
		_Name.c_str()
	);

	// 戻り値が NULL だった場合
	if (!hFileMapping) {
		// システムエラー例外を発生させる
		throw cxx::make_system_error("OpenFileMappingW() failed");
	}

	return FileMapping{ hFileMapping };
}

}	// namespace cxx
