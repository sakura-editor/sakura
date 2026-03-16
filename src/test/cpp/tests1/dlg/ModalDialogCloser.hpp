/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#pragma once

#include "cxx/ResourceHolder.hpp"

namespace dialog {

/*!
 * モーダルダイアログテスト用のクラス
 *
 * WindowsHookを使ってダイアログの初期表示を検出し自動的に閉じるようにするもの。
 */
struct ModalDialogCloser final {
	using CbtHookHolder = cxx::ResourceHolder<&::UnhookWindowsHookEx>;

	using Me = ModalDialogCloser;

	//! ダイアログボックスのウインドウクラス名
	static constexpr auto DIALOG_CLASS = 32770;

	static inline CbtHookHolder gm_CbtHook = nullptr;

	//! Windowsフック関数
	static LRESULT CALLBACK CBTProc(
		_In_ int    nCode,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
	);

	ModalDialogCloser() noexcept;
	ModalDialogCloser( const Me& ) = delete;
	Me& operator=(const Me&) = delete;
	~ModalDialogCloser() noexcept;
};

} // namespace dialog
