/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include <Windows.h>

/*!
 * @brief 拡張ウィンドウ
 * 
 * 定義済みウィンドウのカスタマイズを実現する。
 */
class CCustomizedWnd
{
private:
	using Me = CCustomizedWnd;
	using MePtr = Me*;

public:
	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

	virtual ~CCustomizedWnd() = default;

	virtual bool AttachWnd(
		HWND hWnd,
		UINT_PTR uIdSubclass = 0
	);

	virtual LRESULT DispatchEvent(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	virtual void	OnDestroy(HWND hWnd);
};

