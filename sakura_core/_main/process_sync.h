/*!	@file
	@brief エディタープロセス起動時の同期待機ヘルパー

	@author Sakura Editor Organization
	@date 2026/07/27 作成
*/

/*
	Copyright (C) 2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_PROCESS_SYNC_5B8D56A4_D12C_4A6B_8C1B_88A53B98444B_H_
#define SAKURA_PROCESS_SYNC_5B8D56A4_D12C_4A6B_8C1B_88A53B98444B_H_
#pragma once

#include <windows.h>

//! 起動同期待機の判定結果
enum class ESyncWaitResult {
	Initialized,	//!< 初期化完了イベントがシグナル
	ChildExited,	//!< 子プロセスが先に終了
	SentMessage,	//!< 送信メッセージ到着（処理して継続すべき）
	Failed,			//!< タイムアウト/失敗
};

//! MsgWaitForMultipleObjectsEx の戻り値を分類する（handleCount は待機ハンドル数＝通常2）
inline ESyncWaitResult ClassifySyncWait(DWORD dwRet, DWORD handleCount) noexcept
{
	if (dwRet == WAIT_OBJECT_0)							return ESyncWaitResult::Initialized;
	if (handleCount >= 2 && dwRet == WAIT_OBJECT_0 + 1)	return ESyncWaitResult::ChildExited;
	if (dwRet == WAIT_OBJECT_0 + handleCount)			return ESyncWaitResult::SentMessage;
	return ESyncWaitResult::Failed;	// WAIT_TIMEOUT / WAIT_FAILED など
}

//! 残り待機時間(ms)を算出する。deadline を過ぎていれば 0
inline DWORD CalcRemainMs(ULONGLONG deadlineTick, ULONGLONG nowTick) noexcept
{
	return (nowTick < deadlineTick) ? DWORD(deadlineTick - nowTick) : 0u;
}

//! ハンドルが有効なときのみ SetEvent する。無効(nullptr)なら false を返し何もしない
inline bool SafeSetEvent(HANDLE hEvent) noexcept
{
	return (hEvent != nullptr) && (::SetEvent(hEvent) != FALSE);
}

//! 子エディタの初期化完了を待つ。
//! 待機中も送信メッセージ(sent message)は処理し、ポストメッセージはディスパッチしない。
//! @param hEvent   初期化完了イベント
//! @param hProcess 子プロセスハンドル（先に終了したら失敗扱い）
//! @param timeoutMs 全体タイムアウト(ms)
inline ESyncWaitResult WaitEditorInitialized(HANDLE hEvent, HANDLE hProcess, DWORD timeoutMs) noexcept
{
	HANDLE handles[2] = { hEvent, hProcess };
	const ULONGLONG deadline = ::GetTickCount64() + timeoutMs;
	for (;;) {
		const DWORD remain = CalcRemainMs(deadline, ::GetTickCount64());
		if (remain == 0) {
			// 期限超過。送信メッセージが届き続けていても打ち切る（ビジーループ回避）
			return ESyncWaitResult::Failed;
		}
		// 第5引数は 0。MWMO_INPUTAVAILABLE はキュー内の未読「入力」に対するフラグであり、
		// 送信メッセージ(QS_SENDMESSAGE)には作用しないため指定しない。
		const DWORD dwRet = ::MsgWaitForMultipleObjectsEx(
			2, handles, remain, QS_SENDMESSAGE, 0);
		const ESyncWaitResult eResult = ClassifySyncWait(dwRet, 2);
		if (eResult == ESyncWaitResult::SentMessage) {
			// 送信メッセージのみ処理し、ポストは残す
			MSG msg;
			::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE);
			continue;
		}
		return eResult;	// Initialized / ChildExited / Failed
	}
}

#endif /* SAKURA_PROCESS_SYNC_5B8D56A4_D12C_4A6B_8C1B_88A53B98444B_H_ */
