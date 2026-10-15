/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/FileFind.hpp"

#include "basis/primitive.h"
#include "util/os/Kernel32.hpp"

#include "util/os/make_system_error.hpp"

#include <string_view>
#include <system_error>

namespace cxx {

FileFind FileFind::FindFirstFileW(
	std::wstring_view searchPattern,
	WIN32_FIND_DATAW* pFindFileData
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Pattern{ searchPattern };

	const auto hFindFile = Kernel32::getInstance()->FindFirstFileW(
		_Pattern.c_str(),
		pFindFileData
	);

	if (INVALID_HANDLE_VALUE == hFindFile) {
		throw cxx::make_system_error("FindFirstFileW failed.");
	}

	return FileFind{ hFindFile };
}

bool FileFind::FindNextFileW(
	WIN32_FIND_DATAW* pFindFileData
) const
{
	const auto result = Kernel32::getInstance()->FindNextFileW(get(), pFindFileData);

	if (!result) {
		throw cxx::make_system_error("FindNextFileW failed.");
	}

	return result;
}

bool FileFind::FindNext(
	WIN32_FIND_DATAW* pFindFileData
) const
{
	bool result = false;

	try {
		result = FindNextFileW(pFindFileData);
	}
	// エラーが発生した場合
	catch (const std::system_error& e) {
		if (ERROR_NO_MORE_FILES != e.code().value()) {
			throw;
		}
	}

	return result;
}

}	// namespace cxx
