/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"

#include <oleidl.h>

/*!
 * @brief ドロップターゲットウィンドウ
 * 
 * CDropTargetに実装されていたものを分離。
 * CDropTargetはIDropTargetの実装に専念させる。
 */
class CDropTargetWnd
{
private:
	using IDropTargetPtr = cxx::com_pointer<IDropTarget>;

	using Me = CDropTargetWnd;
	using MePtr = Me*;

	static void _CleanUp(
		CDropTargetWnd* pThis
	) noexcept;

	using Holder = cxx::ResourceHolder<&_CleanUp>;

	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

public:
	virtual ~CDropTargetWnd() = default;

	template <typename T>
	bool AttachDropTarget(
		T* pThis
	);

	bool RegisterDropTarget();
	void RevokeDropTarget() noexcept;

	virtual LRESULT DispatchEvent(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	virtual void	OnDestroy(HWND hWnd);

private:
	HWND				m_hWnd = nullptr;
	IDropTargetPtr		m_DropTarget = nullptr;
	HWND				m_hWnd_DropTarget = nullptr;
	Holder				m_Holder{ this };
};
