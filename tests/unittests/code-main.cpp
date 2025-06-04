/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */

#include "pch.h"
#include "testing/StartEditorProcess.hpp"

#include "util/StaticType.h"

// MSVCデバッグビルド専用コードを隔離するためのCマクロ
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_WIN64)
#define USE_STACK_TRACE
#endif

#ifdef USE_STACK_TRACE

// デバッグ出力の独自関数を使う
#include "debug/Debug1.h"	// DebugOutW, TRACEマクロ等

// Windows SDKのデバッグヘルパーとリンクする
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

#endif

namespace testing {

#ifdef USE_STACK_TRACE

void PrintStackTrace(
    CONTEXT& context)
{
    const auto process = GetCurrentProcess();
    const auto thread  = GetCurrentThread();

    // Dbghelp.dll
    // プロセスのシンボル ハンドラーを初期化します。
    PCSTR UserSearchPath = nullptr;
    BOOL  fInvadeProcess = TRUE;
    SymInitialize(process, UserSearchPath, fInvadeProcess);

    STACKFRAME64 stack     = {};
    stack.AddrPC.Offset    = context.Rip;
    stack.AddrPC.Mode      = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rbp;
    stack.AddrFrame.Mode   = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode   = AddrModeFlat;

    std::array<BYTE, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME + 1) * sizeof(TCHAR) - 1> buffer = {};

    auto symbol          = PSYMBOL_INFO(buffer.data());
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO); // ULONG(buffer.size());
    symbol->MaxNameLen   = MAX_SYM_NAME;

    TRACE("PrintStackTrace()");

    // Dbghelp.dll
    // スタック トレースを取得します。
    while (StackWalk64(
        IMAGE_FILE_MACHINE_AMD64,
        process,
        thread,
        &stack,
        &context,
        nullptr,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        nullptr))
    {
        if (const auto address = stack.AddrPC.Offset;
            SymFromAddr(process, address, nullptr, symbol))
        {
			DebugOutW(L"\tat %s in %0p\n", symbol->Name, symbol->Address);
        }
    }

	DebugOutW(L"\n");

	// Dbghelp.dll
    // プロセス ハンドルに関連付けられているすべてのリソースの割り当てを解除します。
    SymCleanup(process);
}

LONG WINAPI MyUnhandledExceptionFilter(
    int                 nExceptionCode,
    EXCEPTION_POINTERS* pExceptionInfo)
{
    UNREFERENCED_PARAMETER(nExceptionCode);

	auto& context = *pExceptionInfo->ContextRecord;

    PrintStackTrace(context);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif

int CallWinMain(
	_In_z_ LPWSTR lpCmdLine
)
{
#ifdef USE_STACK_TRACE
	__try
    {
#endif

		// 実行中モジュールのインスタンスハンドルを取得する
		const auto hInstance = GetModuleHandleW(nullptr);

		return wWinMain(hInstance, nullptr, lpCmdLine, SW_SHOWDEFAULT);

#ifdef USE_STACK_TRACE
    }
    __except (MyUnhandledExceptionFilter(GetExceptionCode(),  GetExceptionInformation()))
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
	std::wcout << strprintf(L"%hs(%d): launching process [%s]", __FILE__, __LINE__, command.data()) << std::endl;

	assert(std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase)));

	// wWinMainに渡すためのコマンドライン
	std::wstring buffer(command);

	// コマンドラインに -CODE 指定がない場合は付与する
	if (!std::regex_search(command, std::wregex(LR"(-NOWIN\b)", std::wregex::icase)) &&
		!std::regex_search(command, std::wregex(LR"(-CODE\b)", std::wregex::icase)))
	{
		buffer += L" -CODE=99"; // 指定しないとファイル名から文字コードを判定する仕様によりJIS指定になってしまう。
	}

	// wWinMainを起動する(戻り値は0が正常)
	const auto ret = CallWinMain(buffer.data());

	// ログ出力
	std::wcout << strprintf(L"%hs(%d): leaving process   [%s] => %d\n", __FILE__, __LINE__, command.data(), ret) << std::endl;

	return ret;
}

} // namespace testing

/*!
 * テストモジュールのエントリポイント
 */
int wmain(int argc, wchar_t **argv) {

	// テスト実行時のロケールは日本語に固定する
	const LCID lcid = 0x0411;
	SetThreadUILanguage(lcid);	// スレッドのUI言語を変更

	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	if (std::wstring command(GetCommandLineW());
		std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		// コマンドライン文字列の先頭に入っているアプリパスを除去する
		if (std::wsmatch m; std::regex_match(command, m, std::wregex(LR"(^(?:".+?"|\S+)\s+(.+))"))) {
			command = m[1];
		}

		// wWinMainを起動して結果を返して抜ける
		return testing::StartEditorProcess(command);
	}

	// LCIDからロケール名を取得（"ja-JP"が取れる）
	SString<LOCALE_NAME_MAX_LENGTH> szLocaleName;
	LCIDToLocaleName(lcid, szLocaleName, int(std::size(szLocaleName)), NULL);

	// TODO: 以下をコメントインする
	// szLocaleName += L".UTF-8";	// UCRTのutf8サポートを有効にする

	// Cロケールも変更
	_wsetlocale(LC_ALL, szLocaleName);

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	wprintf(L"Running main() from %hs\n", __FILE__);
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
