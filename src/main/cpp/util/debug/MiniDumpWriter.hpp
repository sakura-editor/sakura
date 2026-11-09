/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "extmodule/DbgHelp.hpp"

#include <optional>
#include <string_view>

#include <Windows.h>
#include <DbgHelp.h>

namespace cxx {

/*!
 * @brief ミニダンプを出力するクラス
 */
class MiniDumpWriter final : public DbgHelp
{
private:
	using Base = DbgHelp;
	using Me = MiniDumpWriter;

public:
	MiniDumpWriter() = default;

	void WriteMiniDump(
		std::wstring_view file,
		PEXCEPTION_POINTERS pExceptionParams
	) const;

private:
	bool _MiniDumpWriteDump(
        _In_ HANDLE hFile,
		_In_ MINIDUMP_TYPE DumpType,
		const std::optional<PMINIDUMP_EXCEPTION_INFORMATION>& optExceptionParams
    ) const;
};

}	// namespace cxx
