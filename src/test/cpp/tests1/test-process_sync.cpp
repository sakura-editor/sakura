/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization
	@brief エディター起動時の同期待機ヘルパー（process_sync.h）のテスト

	テスト方針:
	プロセス間の SendMessage 枯渇は「同一プロセス内のスレッド間 SendMessage」で再現する。
	そのため別プロセスを起動せず、実ウィンドウ・実イベント・別スレッドだけで実挙動を検証する。
*/

#include "pch.h"
#include <windows.h>
#include <thread>
#include <atomic>
#include "gtest/gtest.h"
#include "_main/process_sync.h"

namespace {

	//! 待機中に「送信」されるメッセージ。処理されればポンプが動いている証拠になる
	constexpr UINT WM_TEST_SENT   = WM_APP + 1;
	//! 待機中に「ポスト」されるメッセージ。処理されないことを確認するために使う
	constexpr UINT WM_TEST_POSTED = WM_APP + 2;

	//! WM_TEST_SENT がウィンドウプロシージャまで届いたかを記録する
	//! （ワーカースレッドと本スレッドの両方から触れるので atomic）
	std::atomic<bool> g_sentHandled{ false };

	LRESULT CALLBACK TestWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		if (msg == WM_TEST_SENT) {
			// ここに到達する ＝ 待機中に送信メッセージがディスパッチされた
			g_sentHandled = true;
			return 0;
		}
		return ::DefWindowProcW(hWnd, msg, wp, lp);
	}

	//! テスト用のメッセージ専用ウィンドウ（HWND_MESSAGE）を作る。
	//! 画面に出ず、送信・ポストの両方を受け取れるためテストに適する。
	HWND CreateTestWindow()
	{
		static const wchar_t* kClass = L"SakuraProcessSyncTestWnd";
		static bool registered = false;
		const HINSTANCE hInst = ::GetModuleHandleW(nullptr);
		if (!registered) {
			WNDCLASSEXW wc{};
			wc.cbSize = sizeof(wc);
			wc.lpfnWndProc = TestWndProc;
			wc.hInstance = hInst;
			wc.lpszClassName = kClass;
			::RegisterClassExW(&wc);	// 二重登録は失敗するだけで無害
			registered = true;
		}
		return ::CreateWindowExW(0, kClass, L"t", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, nullptr);
	}

} // namespace

/*!
	待機中に「送信は捌く／ポストは捌かない」、かつイベントで正常終了することを検証する。

	これが #2550 の再発防止そのもの。以下のいずれかが壊れると落ちる。
	 - 待機を ::WaitForMultipleObjects に戻した      → 送信が捌かれず worker がブロックし続けてタイムアウト
	 - 起床マスクから QS_SENDMESSAGE を外した        → 同上
	 - PeekMessage から PM_QS_SENDMESSAGE を外した   → ポストまで処理してしまい、残存チェックが落ちる

	同期は Sleep に頼らない。SendMessage は相手が捌くまで戻らない性質があるため、
	worker の Post → Send → SetEvent という並びだけで順序が確定する（フレークしにくい）。
*/
TEST(WaitEditorInitialized, PumpsSentButNotPosted)
{
	g_sentHandled = false;

	const HWND hWnd = CreateTestWindow();
	ASSERT_NE(nullptr, hWnd);

	// 初期化完了イベントの代役（手動リセット・初期非シグナル）
	HANDLE hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
	ASSERT_NE(nullptr, hEvent);

	// 子プロセスハンドルの代役。決してシグナルしないことで ChildExited 経路に入らないようにする
	HANDLE hProcStandin = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);
	ASSERT_NE(nullptr, hProcStandin);

	std::thread worker([&]{
		// 先にポストしておく。待機ループがこれを処理しないことを後で確認する
		::PostMessageW(hWnd, WM_TEST_POSTED, 0, 0);
		// 本スレッドが送信を捌くまでここでブロックする（＝ポンプ稼働の証明）
		::SendMessageW(hWnd, WM_TEST_SENT, 0, 0);
		// 送信が返った後に初期化完了を通知する
		::SetEvent(hEvent);
	});

	const ESyncWaitResult r = WaitEditorInitialized(hEvent, hProcStandin, 5000);
	worker.join();

	// イベントのシグナルで待機を抜けたこと
	EXPECT_EQ(ESyncWaitResult::Initialized, r);
	// 待機中に送信メッセージが処理されたこと（#2550 の確認）
	EXPECT_TRUE(g_sentHandled.load());

	// ポストは処理されずキューに残っているはず。
	// PeekMessage が true を返す ＝ 待機ループがポストをディスパッチしなかった証拠
	MSG msg{};
	EXPECT_TRUE(::PeekMessageW(&msg, hWnd, WM_TEST_POSTED, WM_TEST_POSTED, PM_REMOVE));

	::CloseHandle(hProcStandin);
	::CloseHandle(hEvent);
	::DestroyWindow(hWnd);
}

/*!
	子プロセスが初期化完了前に終了した場合、ChildExited として検出できることを検証する。

	ClassifySyncWait の WAIT_OBJECT_0 + 1 の判定を守る。
	handles の並び順（{ hEvent, hProcess }）を入れ替えると、
	子の異常終了を「初期化成功」と誤判定するようになり、ここで落ちる。
*/
TEST(WaitEditorInitialized, ChildExitReportsFailure)
{
	HANDLE hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);	// 初期化完了は通知されない
	HANDLE hProc  = ::CreateEventW(nullptr, TRUE, TRUE,  nullptr);	// 初期シグナル ＝ 子が終了済みの状態を模す
	ASSERT_NE(nullptr, hEvent);
	ASSERT_NE(nullptr, hProc);

	EXPECT_EQ(ESyncWaitResult::ChildExited, WaitEditorInitialized(hEvent, hProc, 5000));

	::CloseHandle(hProc);
	::CloseHandle(hEvent);
}

/*!
	何も起きないまま制限時間を過ぎたら Failed になることを検証する。

	CalcRemainMs が残り時間を正しく減らしていることの実挙動確認も兼ねる。
	残り時間の計算を誤って毎回フルの timeout を渡すと、待機が期限を超えて伸びる。

	注: 送信もポストも発生しない構成なので、100ms で確実に WAIT_TIMEOUT に落ちる。
	    このテストだけは意図的に約100ms かかる（異常ではない）。
*/
TEST(WaitEditorInitialized, TimeoutReportsFailure)
{
	HANDLE hEvent = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);	// シグナルしない
	HANDLE hProc  = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);	// シグナルしない
	ASSERT_NE(nullptr, hEvent);
	ASSERT_NE(nullptr, hProc);

	EXPECT_EQ(ESyncWaitResult::Failed, WaitEditorInitialized(hEvent, hProc, 100));

	::CloseHandle(hProc);
	::CloseHandle(hEvent);
}

/*!
	MsgWaitForMultipleObjectsEx の戻り値の分類を固定する（純粋関数・決定的）。

	この API の戻り値は待機ハンドル数 nCount に依存する off-by-one を作り込みやすい。
	nCount == 2 のとき、WAIT_OBJECT_0 + 2 は「3個目のハンドル」ではなく
	「送信メッセージの到着」を意味する。ここを取り違えると、
	送信到着を Failed と誤判定して待機が即座に失敗するようになる。

	CalcRemainMs は期限超過時に 0 を返すこと（負値へ回り込まないこと）を確認する。
	ULONGLONG の減算で回り込むと、事実上無限待ちになる。
*/
TEST(ProcessSyncClassify, Mapping)
{
	EXPECT_EQ(ESyncWaitResult::Initialized, ClassifySyncWait(WAIT_OBJECT_0,     2));	// 1個目 ＝ 初期化完了
	EXPECT_EQ(ESyncWaitResult::ChildExited, ClassifySyncWait(WAIT_OBJECT_0 + 1, 2));	// 2個目 ＝ 子プロセス終了
	EXPECT_EQ(ESyncWaitResult::SentMessage, ClassifySyncWait(WAIT_OBJECT_0 + 2, 2));	// nCount と同値 ＝ 送信到着
	EXPECT_EQ(ESyncWaitResult::Failed,      ClassifySyncWait(WAIT_TIMEOUT,      2));	// 時間切れ

	EXPECT_EQ(DWORD(0),   CalcRemainMs(1000, 1000));	// 期限ちょうど ＝ 残り 0
	EXPECT_EQ(DWORD(500), CalcRemainMs(1000, 500));		// 期限まで 500ms
}
