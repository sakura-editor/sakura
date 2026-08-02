/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#pragma once

#include "cxx/ResourceHolder.hpp"
#include "dlg/CDlgOpenFile.h"

#include <functional>

namespace dialog {

/*!
 * モーダルダイアログテスト用のクラス
 *
 * WindowsHookを使ってダイアログの初期表示を検出して閉じるようにするもの。
 */
struct ModalDialogCloser final : public TSingleInstance<ModalDialogCloser> {
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

	static void CloseDialogByCancel(HWND hWndDlg);

	ModalDialogCloser() noexcept;
	explicit ModalDialogCloser(const std::function<void(HWND)>& action) noexcept;
	ModalDialogCloser(const Me&) = delete;
	Me& operator=(const Me&) = delete;
	~ModalDialogCloser() noexcept override;

private:
	std::function<void(HWND)> m_Action;
};

} // namespace dialog

struct MockCDlgOpenFile final : public CDlgOpenFile {
	MOCK_METHOD2(DoModal_GetOpenFileName, bool(std::span<WCHAR>, EFilter));
	MOCK_METHOD1(DoModal_GetSaveFileName, bool(std::span<WCHAR>));
	MOCK_METHOD3(DoModalOpenDlg, bool(SLoadInfo*, std::vector<std::wstring>*, bool));
	MOCK_METHOD2(DoModalSaveDlg, bool(SSaveInfo*, bool));

	static inline std::vector<std::wstring> gm_Files;

	static void _Cleanup([[maybe_unused]] const MockCDlgOpenFile*);

	static bool _GetOpenFileName(
		std::span<WCHAR> szPath,
		EFilter eAddFileter
	);

	static bool _GetSaveFileName(
		std::span<WCHAR> szPath
	);

	static bool _DoModalOpenDlg(
		SLoadInfo* pLoadInfo,
		std::vector<std::wstring>* pFilenames,
		bool bOptions [[maybe_unused]]
	);

	static bool _DoModalSaveDlg(
		SSaveInfo* pSaveInfo,
		bool bSimpleMode [[maybe_unused]]
	);

	explicit MockCDlgOpenFile();

	using Holder = cxx::ResourceHolder<&_Cleanup>;
	Holder m_Holder{ this };
};
