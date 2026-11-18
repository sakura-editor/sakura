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
 * @brief 指定したウィンドウハンドルをこのクラスに紐付ける
 */
template <typename T>
bool CDropTargetWnd::AttachDropTarget(
	T* pThis
)
{
	if constexpr (std::is_same_v<T, CEditView>)
	{
		m_DropTarget = CDropTarget::make_instance<CDropTarget>(pThis);
	}
	else if constexpr (std::is_same_v<T, CEditWnd>)
	{
		m_DropTarget = CDropTarget::make_instance<CDropTarget_CEditWnd>();
	}

	// ウィンドウハンドルを取得する
	const auto hWnd = pThis->GetHwnd();

	// ウィンドウをサブクラス化する
	if (!AttachWnd(hWnd)) return false;

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
 * @brief WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 *
 * @param[in] hWnd 宛先ウィンドウのハンドル
 *
 * @returns このメッセージに戻り値はありません。
 */
void CDropTargetWnd::OnDestroy(HWND hWnd)
{
	if (!hWnd) return;

	RevokeDropTarget();

	Base::OnDestroy(hWnd);
}

template bool CDropTargetWnd::AttachDropTarget<CEditWnd>(CEditWnd* pThis);
template bool CDropTargetWnd::AttachDropTarget<CEditView>(CEditView* pThis);
