/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "window/CDropTargetWnd.hpp"

#include "_os/CDropTarget.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"

#include "CSelectLang.h"

/*!
 * @brief クリーンアップ関数
 */
/* static */ void CDropTargetWnd::_CleanUp(
	CDropTargetWnd* pThis
) noexcept
{
	if (pThis) pThis->RevokeDropTarget();
}

 /*!
 * @brief 拡張ウィンドウのプロシージャ
 *
 * @param hWnd [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @param uIdSubclass [in] サブクラスID
 * @param dwRefData [in] サブクラスデータ
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 */
/* static */ LRESULT CALLBACK CDropTargetWnd::SubclassProc(
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
 * @brief 指定したウィンドウハンドルをこのクラスに紐付ける
 */
template <typename T>
bool CDropTargetWnd::AttachDropTarget(
	T* pThis
)
{
	if constexpr (std::is_same_v<T, CEditView>)
	{
		m_DropTarget = CDropTarget::make_instance(pThis);
	}
	else if constexpr (std::is_same_v<T, CEditWnd>)
	{
		m_DropTarget = CDropTarget::make_instance(pThis);
	}

	// ウィンドウハンドルを取得する
	const auto hWnd = pThis->GetHwnd();

	// ウィンドウをサブクラス化する
	if (!::SetWindowSubclass(hWnd, &SubclassProc, 0, DWORD_PTR(this))) return false;

	// ウィンドウハンドルを紐付ける
	m_hWnd = hWnd;

	// OLE D&Dのターゲットとして登録する
	return RegisterDropTarget();
}

/*!
 * @brief このウィンドウをOLE D&Dのターゲットとして登録する
 */
bool CDropTargetWnd::RegisterDropTarget()
{
	auto bRet = false;

	if (const auto hWnd = m_hWnd;
		!m_hWnd_DropTarget &&
		FAILED(::RegisterDragDrop(hWnd, m_DropTarget)))
	{
		// L"::RegisterDragDrop()\n失敗"
		TopWarningMessage(hWnd, LS(STR_ERR_DLGDRPTGT1));
	}
	else {
		m_hWnd_DropTarget = hWnd;

		bRet = true;
	}

	return bRet;
}

/*!
 * @brief OLE D&Dのウィンドウ登録を解除する
 */
void CDropTargetWnd::RevokeDropTarget() noexcept
{
	if (m_hWnd_DropTarget &&
		SUCCEEDED(::RevokeDragDrop(m_hWnd_DropTarget)))
	{
		m_hWnd_DropTarget = nullptr;
	}
}

/*!
 * @brief 拡張ウィンドウのメッセージ配送
 *
 * @param hWndDlg [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 *
 * @note 戻り値0は「処理済み」を示すことが多いが、実装時に確認すること。
 */
LRESULT CDropTargetWnd::DispatchEvent(
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
 * このメッセージに戻り値はありません。
 */
void CDropTargetWnd::OnDestroy(HWND hWnd)
{
	if (!hWnd) return;

	RevokeDropTarget();
}

template bool CDropTargetWnd::AttachDropTarget<CEditWnd>(CEditWnd* pThis);
template bool CDropTargetWnd::AttachDropTarget<CEditView>(CEditView* pThis);
