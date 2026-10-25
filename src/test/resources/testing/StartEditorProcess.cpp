/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"
#include "testing/StartEditorProcess.hpp"

#include "_main/CProcess.h"
#include "util/debug/StackTraceWriter.hpp"
#include "util/StaticType.h"
#include "util/tchar_convert.h"
#include "env/CDataProfile.h" // StringBufferW

// デバッグ出力の独自関数を使う
#include "debug/Debug1.h"	// DebugOutW, TRACEマクロ等

// Windows SDKのデバッグヘルパーを参照する
#include <DbgHelp.h>

// Windows SDKのデバッグヘルパーとリンクする
#pragma comment(lib, "dbghelp.lib")

namespace testing {

using namespace cxx;

// SEH例外コードを人間が読める文字列に変換する
std::string_view SehExceptionCodeName(NTSTATUS code) noexcept
{
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:         return "access violation (seg fault)";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "array bounds exceeded";
	case EXCEPTION_DATATYPE_MISALIGNMENT:    return "datatype misalignment";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "float: divide by zero";
	case EXCEPTION_FLT_OVERFLOW:             return "float: overflow";
	case EXCEPTION_FLT_STACK_CHECK:          return "float: stack check";
	case EXCEPTION_FLT_UNDERFLOW:            return "float: underflow";
	case EXCEPTION_ILLEGAL_INSTRUCTION:      return "illegal instruction";
	case EXCEPTION_IN_PAGE_ERROR:            return "in-page error";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "integer: divide by zero";
	case EXCEPTION_INT_OVERFLOW:             return "integer: overflow";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "non-continuable exception";
	case EXCEPTION_PRIV_INSTRUCTION:         return "privileged instruction";
	case EXCEPTION_STACK_OVERFLOW:           return "stack overflow";
	default:                                 return {};
	}
}

void DumpSehException(EXCEPTION_POINTERS* ep)
{
	if (!ep || !ep->ExceptionRecord) {
		std::cerr << "no SEH exception info\n";
		return;
	}

	const auto code = ep->ExceptionRecord->ExceptionCode;
	if (const auto name = SehExceptionCodeName(code); !name.empty()) {
		std::cerr << "SEH exception: " << name << "\n";
	}
	std::cerr << "SEH code: 0x"
		<< std::hex << code << std::dec << "\n";
	std::cerr << "SEH address: 0x"
		<< std::hex
		<< uint64_t(ep->ExceptionRecord->ExceptionAddress)
		<< std::dec << "\n";
}

// 未処理例外フィルター: SEH例外またはC++例外をダンプし、スタックトレースを出力
// コンパイルオプション(/EHaなど)によってはC++例外もこのフィルターで捕捉される
LONG OnUnhandledException(EXCEPTION_POINTERS* ep)
{
	DumpSehException(ep);
	if (ep && ep->ContextRecord) {
		CProcess::WriteStackTraceIgnoringExceptions(
			*ep->ContextRecord,
			&std::cerr
		);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

int CallWinMain(
	std::wstring& cmdLine
)
{
#ifdef USE_STACK_TRACE
	__try
	{
#endif

		// 実行中モジュールのインスタンスハンドルを取得する
		const auto hInstance = ::GetModuleHandleW(nullptr);

		return wWinMain(hInstance, nullptr, std::data(cmdLine), SW_SHOWDEFAULT);

#ifdef USE_STACK_TRACE
	}
	__except (OnUnhandledException(GetExceptionInformation()))
	{
		return 1;
	}
#endif
}

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 * コマンドラインには -PROF 指定が含まれている必要があります。
 *
 * @param[in] command コマンドライン文字列(exeパスを含まない)
 * @retval 0		正常終了
 * @retval 1		異常終了(asset失敗でabortした場合はこの値)
 * @retval その他	おそらくバグ。サクラエディタは0以外の終了コードを定義していない。
 */
int StartEditorProcess(const std::wstring& command)
{
	// ログ出力
	std::cout << std::format("{}({}): launching process [{}]", std::source_location::current().file_name(), std::source_location::current().line(), cxx::to_string(command)) << std::endl;

	assert(std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase)));

	// wWinMainに渡すためのコマンドライン
	std::wstring buffer(command);

	// コマンドラインに -CODE 指定がない場合は付与する
	if (!std::regex_search(command, std::wregex(LR"(-CODE\b)", std::wregex::icase)) &&
		!std::regex_search(command, std::wregex(LR"(-NOWIN\b)", std::wregex::icase)))
	{
		buffer += std::format(LR"( -CODE={})", static_cast<int>(CODE_AUTODETECT)); // 指定しないとファイル名から文字コードを判定する仕様によりJIS指定になってしまう。
	}

	// wWinMainを起動する(戻り値は0が正常)
	const auto ret = CallWinMain(buffer);

	// ログ出力
	std::cout << std::format("{}({}): leaving process   [{}] => {}", std::source_location::current().file_name(), std::source_location::current().line(), cxx::to_string(command), ret) << std::endl << std::endl;

	return ret;
}

} // namespace testing
