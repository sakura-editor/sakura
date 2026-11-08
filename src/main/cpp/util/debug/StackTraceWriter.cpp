/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/StackTraceWriter.hpp"

#include "util/os.h"

namespace cxx {

/* static */ void StackTraceWriter::_CleanUp(
	const StackTraceWriter* pThis
) noexcept
{
	// 引数がNULLなら何もしない
	if (!pThis) return;

	// DLL初期化に失敗していたら何もしない
	const auto SymCleanup = std::bit_cast<decltype(&::SymCleanup)>(pThis->m_Functions.at("SymCleanup"));
	if (!SymCleanup) return;

	SymCleanup(pThis->m_Process);
}

#pragma push_macro("USE_DBGHELP_API")

#define USE_DBGHELP_API(name) \
	const auto name = std::bit_cast<decltype(&:: name )>( m_Functions.at( #name ) ); \
	if (! name ) throw std::system_error(std::make_error_code(std::errc::not_supported), std::format("missing symbol '{}'", #name )); \
	0 /* ←セミコロンを付けないとエラーになるように。 */

// プロセスのシンボル ハンドラーを初期化します。
StackTraceWriter::StackTraceWriter()
{
	USE_DBGHELP_API(SymSetOptions);
	SymSetOptions(
		SYMOPT_LOAD_LINES | SYMOPT_UNDNAME
	);

	USE_DBGHELP_API(SymInitialize);
	PCSTR UserSearchPath = nullptr;
	BOOL  fInvadeProcess = TRUE;
	SymInitialize(
		m_Process,
		UserSearchPath,
		fInvadeProcess
	);
}

void StackTraceWriter::WriteStackTrace(
	CONTEXT& context,
	std::ostream* out
) const
{
	USE_DBGHELP_API(StackWalk64);
	USE_DBGHELP_API(SymFromAddr);
	USE_DBGHELP_API(SymFunctionTableAccess64);
	USE_DBGHELP_API(SymGetModuleBase64);

	if (!out) return;

#if defined(_M_AMD64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_IX86)
	constexpr auto machineType = IMAGE_FILE_MACHINE_I386;
#endif

	STACKFRAME64 stack     = {};
	stack.AddrPC.Mode      = AddrModeFlat;
	stack.AddrFrame.Mode   = AddrModeFlat;
	stack.AddrStack.Mode   = AddrModeFlat;

#ifdef _M_AMD64
	stack.AddrPC.Offset    = context.Rip;
	stack.AddrFrame.Offset = context.Rbp;
	stack.AddrStack.Offset = context.Rsp;
#elif defined(_M_ARM64)
	stack.AddrPC.Offset    = context.Pc;
	stack.AddrFrame.Offset = context.Fp;
	stack.AddrStack.Offset = context.Sp;
#else // _M_IX86
	stack.AddrPC.Offset    = context.Eip;
	stack.AddrFrame.Offset = context.Ebp;
	stack.AddrStack.Offset = context.Esp;
#endif

	std::array<BYTE, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME + 1) - 1> buffer = {};

	auto symbol          = PSYMBOL_INFO(buffer.data());
	symbol->SizeOfStruct = sizeof(*symbol);
	symbol->MaxNameLen   = MAX_SYM_NAME;

	*out << "PrintStackTrace()";

	// スタック トレースを取得します。
	while (StackWalk64(
		machineType,
		m_Process,
		m_Thread,
		&stack,
		&context,
		nullptr,
		SymFunctionTableAccess64,
		SymGetModuleBase64,
		nullptr))
	{
		if (const auto address = stack.AddrPC.Offset;
			SymFromAddr(m_Process, address, nullptr, symbol))
		{
			*out << std::format("\tat {} in {}", std::string(symbol->Name), symbol->Address) << std::endl;
		}
	}

	*out << std::endl;
}

#pragma pop_macro("USE_DBGHELP_API")

}	// namespace cxx
