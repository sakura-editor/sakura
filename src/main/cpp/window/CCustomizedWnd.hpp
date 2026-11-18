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

	/*!
	 * @brief コンストラクタ
	 * 
	 * @param[in, opt] uIdSubclass サブクラスID。省略した場合はthisポインタのアドレスを数値として使う。
	 */
	explicit CCustomizedWnd(
		UINT_PTR uIdSubclass = 0
	)
		: m_IdSubclass{ uIdSubclass ? uIdSubclass : UINT_PTR(this) }
	{
	}

	virtual ~CCustomizedWnd() = default;

	virtual bool AttachWnd(
		HWND hWnd
	);

	virtual LRESULT DispatchEvent(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	virtual void	OnDestroy(HWND hWnd);

private:
	UINT_PTR	m_IdSubclass;
};

