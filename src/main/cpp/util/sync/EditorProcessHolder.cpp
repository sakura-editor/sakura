/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/sync/EditorProcessHolder.hpp"

#include "_main/CCommandLine.h"
#include "config/system_constants.h"

BOOL BlockingHook(HWND hwndDlgCancel);

namespace cxx {

struct SEnumEditorWindowProcParam
{
	HWND hWnd = nullptr;
};

using LPSEnumEditorWindowProcParam = SEnumEditorWindowProcParam*;

/*!
 * @brief 編集ウィンドウを列挙するコールバック関数
 *
 * @param[in] hWnd 列挙されたウィンドウのハンドル
 * @param[in] lParam 列挙の呼び出し元から渡されたパラメータ（HWND* を期待）
 * @retval TRUE  列挙続行（編集ウィンドウではなかった。）
 * @retval FALSE 列挙停止（編集ウィンドウが見付かった。）
 */
BOOL CALLBACK EnumEditorWindowProc(
	_In_ HWND   hWnd,
	_In_ LPARAM lParam
)
{
	if (!hWnd || !lParam || !IsSakuraMainWindow(hWnd)) return TRUE;	// 検索続行

	auto param = LPSEnumEditorWindowProcParam(lParam);

	param->hWnd = hWnd;

	return FALSE;
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @param[in] optProfileName プロファイル名
 * @param[in] args コマンドライン引数
 * @param[in] sync 同期起動するかどうか
 * @return 起動したプロセスオブジェクト
 */
/* static */ EditorProcessHolder ProcessHolder::CreateEditorProcess(
	std::vector<std::wstring>& args,
	const std::optional<std::filesystem::path>& optWorkingDir,
	bool sync,
	const std::optional<std::wstring_view>& optProfileName
)
{
	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
	if (sync) dwCreationFlag |= CREATE_SUSPENDED;

	// エディタープロセスを起動する
	auto ep = CreateSakuraProcess(
		args,
		dwCreationFlag,
		optWorkingDir,
		si,
		optProfileName
	);

	if (!sync) return EditorProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId, HWND(nullptr) };

	const bool isGrepMode = std::ranges::any_of(args, [] (std::wstring_view opt) {
		return cxx::iequals(opt, L"-GREPMODE")
			|| cxx::iequals(opt, L"-GREPDLG");
	});

	// エディターのメインスレッドを開く
	HandleHolder hThread{ ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, ep.dwThreadId) };

	// 初期化完了イベントを作成する
	SFilePath initEventName{ std::format(GSTR_EVENT_SAKURA_EP_INITIALIZED, ep.dwThreadId) };
	cxx::HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, initEventName) };

	cxx::HandleHolder grepEvent{};

	if (isGrepMode) {
		// Grep完了イベントを作成する
		SFilePath grepEventName{ std::format(GSTR_EVENT_SAKURA_GREP_COMPLETED, ep.dwThreadId) };
		grepEvent = ::CreateEventW(nullptr, TRUE, FALSE, grepEventName);
	}

	// エディターのメインスレッドを再開する
	::ResumeThread(hThread);

	const auto startTick = ::GetTickCount64();
	const ULONGLONG timeoutMillis = 15000;	// タイムアウト時間

	// WaitForMultipleObjectsに渡す配列の個数
	constexpr auto count = 2;

	DWORD dwRet = WAIT_TIMEOUT;

	// エディター初期化完了を待つ
	do {
		// 残り時間を考慮して待機する
		const auto elapsed = ::GetTickCount64() - startTick;	// 経過時間
		if (timeoutMillis <= elapsed) break; // タイムアウト発生
		const auto remaining = DWORD(timeoutMillis - elapsed);			// 残り時間
		dwRet = cxx::MsgWaitForMultipleObjects(std::array{ hEvent.get(), ep.get()}, false, remaining, QS_SENDMESSAGE);

		// 自スレッドにメッセージが送られてきた場合
		if (WAIT_OBJECT_0 + count == dwRet) {
			BlockingHook(nullptr);	// 溜まったメッセージを処理する
		}
	}
	while (
		WAIT_OBJECT_0 != dwRet &&	// エディター初期化完了
		WAIT_OBJECT_0 + 1 != dwRet	// エディタープロセス終了
	);

	// メインウインドウを取得する
	SEnumEditorWindowProcParam param{};

	// スレッドに含まれるウインドウを列挙する
	::EnumThreadWindows(ep.dwThreadId, &EnumEditorWindowProc, LPARAM(&param));

	// プロセス起動正常、かつ、Grepモードの場合
	if (isGrepMode) {
		dwRet = WAIT_TIMEOUT;

		constexpr auto waitIntervals = 100; // 100ms

		while (
			WAIT_OBJECT_0 != dwRet &&	// Grep完了
			WAIT_OBJECT_0 + 1 != dwRet	// エディタープロセス終了
		) {
			dwRet = cxx::MsgWaitForMultipleObjects(std::array{ grepEvent.get(), ep.get() }, false, waitIntervals, QS_SENDMESSAGE);

			// 自スレッドにメッセージが送られてきた場合
			if (WAIT_OBJECT_0 + count == dwRet) {
				BlockingHook(nullptr);	// 溜まったメッセージを処理する
			}
		}
	}

	// プロセスオブジェクトを返す
	return EditorProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId, param.hWnd };
}

} // namespace cxx

