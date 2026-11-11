/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <string_view>

#include <Windows.h>

namespace cxx {

/*!
 * @brief ファイル検索ハンドルをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class FileFind final : public  cxx::ResourceHolder<&::FindClose>
{
private:
	using Base = cxx::ResourceHolder<&::FindClose>;
	using Me = FileFind;

public:
	static Me FindFirstFileW(
		std::wstring_view searchPattern,
		WIN32_FIND_DATAW* pFindFileData
	);

	using Base::Base;

	bool FindNextFileW(
		WIN32_FIND_DATAW* pFindFileData
	) const;

	bool FindNext(
		WIN32_FIND_DATAW* pFindFileData
	) const;
};

}	// namespace cxx
