/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "window/CCustomizedWnd.hpp"

/*!
 * @brief 拡張ウィンドウのプロシージャ
 *
 * @param[in] hWnd 宛先ウィンドウのハンドル
 * @param[in] uMsg メッセージコード
 * @param[in, opt] wParam 第1パラメーター
 * @param[in, opt] lParam 第2パラメーター
 * @param[in] uIdSubclass サブクラスID
 * @param[in] dwRefData サブクラスデータ
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 */
/* static */ LRESULT CALLBACK CCustomizedWnd::SubclassProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	// クラスデータを取り出す。（格納方法は検討要。）
	auto pcWnd = MePtr(dwRefData);

	// クラスデータが無効な場合（防御コードなので到達不能）
	if (!pcWnd) {
		// サブクラスを解除する
		::RemoveWindowSubclass(hWnd, &SubclassProc, uIdSubclass);

		// DefSubclassProcにフォールバックする
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	if (WM_DESTROY == uMsg) {
		// 拡張WM_DESTROYハンドラを呼び出す
		pcWnd->OnDestroy(hWnd);

		// サブクラスを解除する
		::RemoveWindowSubclass(hWnd, &SubclassProc, uIdSubclass);

		// 元のウィンドウプロシージャへ転送する
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	// メッセージを配送する
	return pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief 指定したウィンドウをこのクラスに紐付ける
 *
 * @param[in] hWnd ウィンドウのハンドル
 *
 * @returns 処理結果
 * @retval true  サブクラス化に成功した
 * @retval false サブクラス化に失敗した
 */
bool CCustomizedWnd::AttachWnd(
	HWND hWnd
)
{
	// ウィンドウをサブクラス化する
	return ::SetWindowSubclass(
		hWnd,
		&SubclassProc,
		m_IdSubclass,
		DWORD_PTR(this)
	);
}

/*!
 * @brief 拡張ウィンドウのメッセージ配送
 *
 * @param[in] hWnd 宛先ウィンドウのハンドル
 * @param[in] uMsg メッセージコード
 * @param[in, opt] wParam 第1パラメーター
 * @param[in, opt] lParam 第2パラメーター
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 *
 * @note 戻り値0は「処理済み」を示すことが多いが、実装時に確認すること。
 */
LRESULT CCustomizedWnd::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	//あとはデフォルトに任せる
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 *
 * @param[in] hWnd 宛先ウィンドウのハンドル
 *
 * @returns このメッセージに戻り値はありません。
 */
void CCustomizedWnd::OnDestroy(HWND hWnd)
{
	if (!hWnd) return;
}

