/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"
#include "window/CCustomizedWnd.hpp"

#include <oleidl.h>

/*!
 * @brief ドロップターゲットウィンドウ
 * 
 * CDropTargetに実装されていたものを分離。
 * CDropTargetはIDropTargetの実装に専念させる。
 */
class CDropTargetWnd : public CCustomizedWnd
{
private:
	using IDropTargetPtr = cxx::com_pointer<IDropTarget>;

	using Base = CCustomizedWnd;
	using Me = CDropTargetWnd;

	static void _CleanUp(
		CDropTargetWnd* pThis
	) noexcept;

	using Holder = cxx::ResourceHolder<&_CleanUp>;

public:
	template <typename T>
	bool AttachDropTarget(
		T* pThis
	);

	bool RegisterDropTarget();
	void RevokeDropTarget() noexcept;

	void	OnDestroy(HWND hWnd) override;

private:
	HWND				m_hWnd = nullptr;
	IDropTargetPtr		m_DropTarget = nullptr;
	HWND				m_hWnd_DropTarget = nullptr;
	Holder				m_Holder{ this };
};
