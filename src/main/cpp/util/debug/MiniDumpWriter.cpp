/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/MiniDumpWriter.hpp"

#include "util/file/FileHandle.hpp"
#include "util/os/make_system_error.hpp"
#include "util/os.h"

namespace cxx {

#pragma push_macro("USE_DBGHELP_API")

#define USE_DBGHELP_API(name) \
	const auto name = std::bit_cast<decltype(&:: name )>( m_Functions.at( #name ) ); \
	if (!name) throw std::system_error(std::make_error_code(std::errc::not_supported), #name " is not available."); \
	0 /* ←セミコロンを付けないとエラーになるように。 */

/*!
 * @brief クラッシュダンプ
 *
 * @author ryoji
 * @date 2009/01/21 新規作成
 * @date 2026/10/16 berryzplus 復刻
 */
void MiniDumpWriter::WriteMiniDump(
	std::wstring_view file,
	PEXCEPTION_POINTERS pExceptionParams
) const
{
	USE_DBGHELP_API(MiniDumpWriteDump);

	// ファイルを作る
	const auto hFile = FileHandle::CreateFileW(
		file,
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		nullptr
	);

	MINIDUMP_EXCEPTION_INFORMATION exceptionParams{};
	exceptionParams.ThreadId = m_ThreadId;
	exceptionParams.ExceptionPointers = pExceptionParams;
	exceptionParams.ClientPointers = FALSE;

	const auto optExceptionParams = pExceptionParams
		? std::make_optional(&exceptionParams)
		: std::nullopt;

	// ミニダンプを出力する
	_MiniDumpWriteDump(
		hFile.get(),
		MiniDumpNormal,
		optExceptionParams
	);
}

bool MiniDumpWriter::_MiniDumpWriteDump(
	_In_ HANDLE hFile,
	_In_ MINIDUMP_TYPE DumpType,
	const std::optional<PMINIDUMP_EXCEPTION_INFORMATION>& optExceptionParams
) const
{
	USE_DBGHELP_API(MiniDumpWriteDump);
	if (!MiniDumpWriteDump) throw std::system_error(std::make_error_code(std::errc::not_supported), "MiniDumpWriteDump is not available.");

	const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam = nullptr;
	const PMINIDUMP_CALLBACK_INFORMATION CallbackParam = nullptr;

	return MiniDumpWriteDump(
		m_Process,
		m_ProcessId,
		hFile,
		DumpType,
		optExceptionParams.value_or(nullptr),
		UserStreamParam,
		CallbackParam
	);
}

#pragma pop_macro("USE_DBGHELP_API")

}	// namespace cxx
