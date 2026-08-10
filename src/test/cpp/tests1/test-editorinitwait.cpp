/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <thread>

#include <Windows.h>

#include "cxx/ResourceHolder.hpp"
#include "util/window.h"

/*
	エディター起動時の初期化完了待ちのテスト。

	対象は CControlTray::OpenNewEditor の if (sync) ブロックから切り出した
	WaitForEditorInitialized。

	 - #2561 : 待機中に送信メッセージを捌かないと、送信元がブロックしてハングする
	 - #2565 : 残り時間の算出が期限超過時にラップアラウンドし、待機が復帰しなくなり得る

	テスト方針:
	プロセス間の SendMessage 枯渇は「同一プロセス内のスレッド間 SendMessage」で再現できるため、
	別プロセスは起動せず、実ウィンドウ・実イベント・別スレッドだけで実挙動を検証する。
	子プロセスハンドルは待機にしか使われないため、イベントハンドルで代用する。
*/

namespace {

	//! 待機中に「送信」されるメッセージ。処理されればポンプが動いている証拠になる
	constexpr UINT WM_TEST_SENT   = WM_APP + 1;
	//! 待機中に「ポスト」されるメッセージ。BlockingHook がこれも処理することの確認に使う
	constexpr UINT WM_TEST_POSTED = WM_APP + 2;

	using HandleHolder = cxx::ResourceHolder<&::CloseHandle>;
	using WindowHolder = cxx::ResourceHolder<&::DestroyWindow>;

	//! メッセージがウィンドウプロシージャまで届いたかを記録する
	//! （複数スレッドから触れるので atomic）
	std::atomic<bool> g_sentHandled{ false };
	std::atomic<bool> g_postedHandled{ false };

	LRESULT CALLBACK TestWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg) {
		case WM_TEST_SENT:
			// ここに到達する ＝ 待機中に送信メッセージがディスパッチされた
			g_sentHandled = true;
			return 0;
		case WM_TEST_POSTED:
			g_postedHandled = true;
			return 0;
		default:
			return ::DefWindowProcW(hWnd, msg, wp, lp);
		}
	}

	//! テスト用のメッセージ専用ウィンドウ（HWND_MESSAGE）を作る。
	//! 画面に出ず、送信・ポストの両方を受け取れるためテストに適する。
	//! 呼び出したスレッドがオーナーになるため、待機するスレッド上で呼ぶこと。
	HWND CreateTestWindow()
	{
		static const wchar_t* kClass = L"SakuraEditorInitWaitTestWnd";
		static std::once_flag registerOnce;
		static ATOM atom = 0;

		const HINSTANCE hInst = ::GetModuleHandleW(nullptr);
		std::call_once(registerOnce, [hInst]{
			WNDCLASSEXW wc{};
			wc.cbSize = sizeof(wc);
			wc.lpfnWndProc = TestWndProc;
			wc.hInstance = hInst;
			wc.lpszClassName = kClass;
			atom = ::RegisterClassExW(&wc);
		});
		if (atom == 0) {
			return nullptr;	// 呼び出し側の ASSERT で検出させる
		}
		return ::CreateWindowExW(0, kClass, L"t", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, nullptr);
	}

	//! 自スレッドのメッセージキューを空にする
	void DrainThreadQueue()
	{
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// 破棄のみ
		}
	}

	//! 待機のタイムアウト値。CI での揺らぎに耐えつつテスト時間を抑えた値
	constexpr ULONGLONG TEST_TIMEOUT_MS = 300;

} // namespace

/*!
	待機中に送信メッセージを捌き、かつ初期化完了イベントで正常終了することを検証する。

	これが #2561 の再発防止そのもの。以下のいずれかが壊れると落ちる。
	 - 待機を ::WaitForMultipleObjects に戻した   → 送信が捌かれず worker がブロックし続ける
	 - 起床マスクから QS_SENDMESSAGE を外した     → 同上
	 - BlockingHook の呼び出しを外した            → 同上
	 - 送信到着の判定（WAIT_OBJECT_0 + count）を誤った → 同上

	同期は Sleep に頼らない。SendMessage は相手が捌くまで戻らない性質があるため、
	worker の Post → Send → SetEvent という並びだけで順序が確定する（フレークしにくい）。

	なお worker の送信はタイムアウト付きで行う。回帰が起きたとき、テストがハングせず
	「失敗」として表面化させるため（gtest にはテスト単位のタイムアウトがない）。
*/
TEST(WaitForEditorInitialized, PumpsMessagesWhileWaiting)
{
	g_sentHandled = false;
	g_postedHandled = false;

	// 前テスト・前回実行の残留メッセージで誤判定しないようキューを空にする
	DrainThreadQueue();

	WindowHolder hWnd{ CreateTestWindow() };
	ASSERT_NE(nullptr, hWnd.get());

	// 初期化完了イベントの代役（手動リセット・初期非シグナル）
	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };
	ASSERT_NE(nullptr, hEvent.get());

	// 子プロセスハンドルの代役。決してシグナルしないことでプロセス終了経路に入らないようにする
	HandleHolder hProcStandin{ ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };
	ASSERT_NE(nullptr, hProcStandin.get());

	const HWND hWndTarget = hWnd.get();
	const HANDLE hEventToSignal = hEvent.get();
	std::thread worker([hWndTarget, hEventToSignal]{
		// 先にポストしておく
		::PostMessageW(hWndTarget, WM_TEST_POSTED, 0, 0);
		// 本スレッドが送信を捌くまでここでブロックする（＝ポンプ稼働の証明）
		DWORD_PTR sendResult = 0;
		::SendMessageTimeoutW(hWndTarget, WM_TEST_SENT, 0, 0, SMTO_NORMAL, 10000, &sendResult);
		// 送信が返った後に初期化完了を通知する
		::SetEvent(hEventToSignal);
	});

	const DWORD dwRet = WaitForEditorInitialized(hEvent.get(), hProcStandin.get(), 10000);

	// 判定値は排水より前に確定させる
	const bool sentHandledDuringWait   = g_sentHandled.load();
	const bool postedHandledDuringWait = g_postedHandled.load();

	// 回帰時に worker を解放してハングを防ぐ（正常時は何も残っていない）
	MSG drain{};
	::PeekMessageW(&drain, nullptr, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE);

	worker.join();

	// イベントのシグナルで待機を抜けたこと
	EXPECT_EQ(DWORD(WAIT_OBJECT_0), dwRet);
	// 待機中に送信メッセージが処理されたこと（#2561 の確認）
	EXPECT_TRUE(sentHandledDuringWait);
	// BlockingHook は送信・ポストを区別せず処理する。現状挙動の固定。
	// 送信のみを処理する実装（PM_QS_SENDMESSAGE 指定）へ変更する場合はこの期待値も変わる
	EXPECT_TRUE(postedHandledDuringWait);

	DrainThreadQueue();
}

/*!
	エディタープロセスが初期化完了前に終了した場合、それを区別して検出できることを検証する。

	handles の並び順（{ hEvent, hProcess }）を入れ替えると、
	プロセスの異常終了を「初期化成功」と誤判定するようになり、ここで落ちる。
*/
TEST(WaitForEditorInitialized, DetectsProcessExit)
{
	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };	// 初期化完了は通知されない
	HandleHolder hProc { ::CreateEventW(nullptr, TRUE, TRUE,  nullptr) };	// 初期シグナル ＝ 終了済みを模す
	ASSERT_NE(nullptr, hEvent.get());
	ASSERT_NE(nullptr, hProc.get());

	EXPECT_EQ(DWORD(WAIT_OBJECT_0 + 1), WaitForEditorInitialized(hEvent.get(), hProc.get(), TEST_TIMEOUT_MS));
}

/*!
	初期化完了とプロセス終了が同時にシグナルした場合、初期化完了を優先することを固定する。

	::MsgWaitForMultipleObjects は添字の小さいハンドルを返すため、
	handles の並び順に依存する仕様である。並び替えの事故をここで検知する。
*/
TEST(WaitForEditorInitialized, PrefersInitializedWhenBothSignaled)
{
	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, TRUE, nullptr) };
	HandleHolder hProc { ::CreateEventW(nullptr, TRUE, TRUE, nullptr) };
	ASSERT_NE(nullptr, hEvent.get());
	ASSERT_NE(nullptr, hProc.get());

	EXPECT_EQ(DWORD(WAIT_OBJECT_0), WaitForEditorInitialized(hEvent.get(), hProc.get(), TEST_TIMEOUT_MS));
}

/*!
	何も起きないまま制限時間を過ぎたら WAIT_TIMEOUT になることを検証する。

	注: 送信もポストも発生しない構成なので、待機は一度で時間切れになる。
	    このテストだけは意図的に約 300ms かかる（異常ではない）。
*/
TEST(WaitForEditorInitialized, ReturnsTimeout)
{
	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };	// シグナルしない
	HandleHolder hProc { ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };	// シグナルしない
	ASSERT_NE(nullptr, hEvent.get());
	ASSERT_NE(nullptr, hProc.get());

	const ULONGLONG begin = ::GetTickCount64();
	const DWORD dwRet = WaitForEditorInitialized(hEvent.get(), hProc.get(), TEST_TIMEOUT_MS);
	const ULONGLONG elapsed = ::GetTickCount64() - begin;

	EXPECT_EQ(DWORD(WAIT_TIMEOUT), dwRet);
	EXPECT_GE(elapsed, TEST_TIMEOUT_MS);	// 期限より早く諦めていないこと
}

/*!
	制限時間 0 の場合、一度も待機せず即座に WAIT_TIMEOUT を返すことを検証する。

	この経路では do-while の先頭で break するため、::MsgWaitForMultipleObjects は
	一度も呼ばれず、dwRet は初期値のまま返る。
	つまり dwRet の初期値が結果を決める。

	初期値を 0 にすると 0 == WAIT_OBJECT_0 であるため、
	「待機していないのに初期化完了」という誤った成功を返すようになる。
	初期値が WAIT_TIMEOUT であることを、ここで固定する。

	なお、初期化完了・プロセス終了の両ハンドルはシグナル済みにしてある。
	期限判定より先に待機してしまう実装では WAIT_OBJECT_0 が返るため、
	break の位置がループ先頭にあることも同時に確認できる。
*/
TEST(WaitForEditorInitialized, ReturnsTimeoutWhenAlreadyExpired)
{
	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, TRUE, nullptr) };	// シグナル済み
	HandleHolder hProc { ::CreateEventW(nullptr, TRUE, TRUE, nullptr) };	// シグナル済み
	ASSERT_NE(nullptr, hEvent.get());
	ASSERT_NE(nullptr, hProc.get());

	const ULONGLONG begin = ::GetTickCount64();
	const DWORD dwRet = WaitForEditorInitialized(hEvent.get(), hProc.get(), 0);
	const ULONGLONG elapsed = ::GetTickCount64() - begin;

	EXPECT_EQ(DWORD(WAIT_TIMEOUT), dwRet);
	EXPECT_LT(elapsed, 100ULL);	// 待機せず即座に返ること
}

/*!
	送信メッセージで繰り返し起床させても、待機が制限時間を大きく超えて伸びないことを検証する（#2565 の実挙動側）。

	期限の判定をループ先頭から外したり、残り時間の算出を誤ったりすると、
	起床のたびに期限が延長され、待機が復帰しなくなる。
	特に、期限判定を do-while の継続条件だけに戻すと、
	条件判定から次の残り時間算出までの間に期限をまたいだ場合に
	ULONGLONG の減算がラップアラウンドし、事実上の無限待ちになる。

	あわせて、戻り値の off-by-one も検知する。
	::MsgWaitForMultipleObjects の戻り値は待機ハンドル数 count に依存する。
	count == 2 のとき WAIT_OBJECT_0 + 2 は「3個目のハンドル」ではなく「送信メッセージの到着」を意味する。
	ここを取り違えると、送信の到着を初期化完了やプロセス終了と誤判定して待機が即座に抜けてしまう。
	この構成ではどちらのハンドルもシグナルしないため、
	 - 起床が実際に発生したこと（g_sentHandled）
	 - それでも初期化完了・プロセス終了とは判定されないこと（dwRet）
	 - 期限まで待ってから抜けること（elapsed）
	の3点を同時に確認できる。

	待機は別スレッドで行う。復帰しなくなった場合でも本スレッドは進めるため、
	テストはハングせず「失敗」として表面化する。
*/
TEST(WaitForEditorInitialized, TimeoutIsNotExtendedByRepeatedWakeups)
{
	g_sentHandled = false;

	HandleHolder hEvent{ ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };	// シグナルしない
	HandleHolder hProc { ::CreateEventW(nullptr, TRUE, FALSE, nullptr) };	// シグナルしない
	ASSERT_NE(nullptr, hEvent.get());
	ASSERT_NE(nullptr, hProc.get());

	std::promise<HWND> hwndPromise;
	auto hwndFuture = hwndPromise.get_future();
	std::atomic<ULONGLONG> elapsed{ 0 };

	const HANDLE hEventArg = hEvent.get();
	const HANDLE hProcArg  = hProc.get();
	auto waitTask = std::async(std::launch::async, [&hwndPromise, &elapsed, hEventArg, hProcArg]{
		// 待機するスレッド自身がウィンドウのオーナーになる必要がある
		WindowHolder hWnd{ CreateTestWindow() };
		hwndPromise.set_value(hWnd.get());

		const ULONGLONG begin = ::GetTickCount64();
		const DWORD dwRet = WaitForEditorInitialized(hEventArg, hProcArg, TEST_TIMEOUT_MS);
		elapsed = ::GetTickCount64() - begin;
		return dwRet;
	});

	const HWND hWndWaiter = hwndFuture.get();
	ASSERT_NE(nullptr, hWndWaiter);

	// 待機スレッドを送信メッセージで繰り返し起床させる（2000ms で自走終了するのでハングしない）
	const ULONGLONG pokeUntil = ::GetTickCount64() + 2000;
	while (::GetTickCount64() < pokeUntil) {
		DWORD_PTR sendResult = 0;
		::SendMessageTimeoutW(hWndWaiter, WM_TEST_SENT, 0, 0, SMTO_NORMAL, 100, &sendResult);
		::Sleep(20);
	}

	const bool finished = (waitTask.wait_for(std::chrono::seconds(3)) == std::future_status::ready);
	if (!finished) {
		// 復帰しない場合でもテストをハングさせない。
		// std::async の future はデストラクターでブロックするため、必ず解放してから抜ける
		::SetEvent(hEvent.get());
		ADD_FAILURE() << "待機が制限時間を超えても復帰しない（期限判定が機能していない疑い）";
	}
	const DWORD dwRet = waitTask.get();

	if (finished) {
		// 送信メッセージによる起床が実際に発生していること。
		// これが false だと以下の判定が「起床していないから素通りした」だけになり、意味を持たない
		EXPECT_TRUE(g_sentHandled.load());
		// 起床の直後に期限切れとなった場合、戻り値は WAIT_TIMEOUT ではなく
		// 「送信メッセージ到着」になり得る。呼び出し側は WAIT_OBJECT_0 以外を失敗として扱うため、
		// ここでは「初期化完了・プロセス終了ではないこと」を確認する
		EXPECT_NE(DWORD(WAIT_OBJECT_0), dwRet);
		EXPECT_NE(DWORD(WAIT_OBJECT_0 + 1), dwRet);
		// 正常なら約 300ms。期限が延長されるバグでは 2000ms 以上になる
		EXPECT_LT(elapsed.load(), 1500ULL);
	}

	DrainThreadQueue();
}
