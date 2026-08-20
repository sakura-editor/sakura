/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#pragma once

#include "cxx/ResourceHolder.hpp"
#include "dlg/CDlgOpenFile.h"

#include "CSelectLang.h"

#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace dialog {

struct ModalDialogCloserTestPeer;

/*!
 * モーダルダイアログテスト用のクラス
 *
 * WindowsHookを使ってダイアログの初期表示を検出して閉じるもの。
 */
class ModalDialogCloser {
private:
	friend struct ModalDialogCloserTestPeer;

	using CbtHookHolder = cxx::ResourceHolder<&::UnhookWindowsHookEx>;

	using Me = ModalDialogCloser;

	static constexpr UINT TIMER_ID_FIRST_IDLE = 9999;
	static constexpr UINT SNOOZE_INTERVAL = 10;

	enum class State {
		Pending,
		Created,
		Running,
		Handled,
		Failed,
		Detached,
	};

	static inline CbtHookHolder gm_CbtHook = nullptr;
	static inline std::mutex gm_Mutex;

	static inline std::deque<Me*> gm_Entries;

	static inline std::map<HWND, Me*> gm_HwndMap;

	//! Windowsフック関数
	static LRESULT CALLBACK CBTProc(
		_In_ int    nCode,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
	);

	static bool ExecuteAction(HWND hWnd) noexcept;
	static void CALLBACK TimerProc(HWND hWnd, UINT, UINT_PTR idEvent, DWORD);

public:
	static bool IsHandled() noexcept;

	ModalDialogCloser(const std::optional<std::wstring>& optTitle, const std::function<void(HWND)>& action);

	ModalDialogCloser(int dialogTitleResourceId, const std::function<void(HWND)>& action);
	explicit ModalDialogCloser(const std::optional<std::wstring>& optTitle = std::nullopt, int nIDDlgItem = IDCANCEL);

	ModalDialogCloser(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	~ModalDialogCloser() noexcept;

private:
	std::optional<std::wstring> m_DialogTitle;
	std::function<void(HWND)> m_Action;

	State m_State = State::Pending;
	HWND m_hWnd = nullptr;
	std::exception_ptr m_Exception;
	DWORD m_HookError = ERROR_SUCCESS;
	bool m_TimerActive = false;
};

/*!
 * プロパティシートテスト用のクラス
 */
class PropertySheetCloser : public ModalDialogCloser
{
public:
	explicit PropertySheetCloser(const std::function<void(HWND, HWND)>& action)
		: ModalDialogCloser(std::nullopt, [action] (HWND hWndDlg) {
			// アクティブなプロパティーシートのハンドルを取得する 
			const auto hWndPage = HWND(::SendMessageW(hWndDlg, PSM_GETCURRENTPAGEHWND, 0L, 0L));

			action(hWndDlg, hWndPage);
		})
	{
	}

	explicit PropertySheetCloser(int button = PSBTN_CANCEL)
		: PropertySheetCloser([button] (HWND hWndDlg, HWND) {
			::SendMessageW(hWndDlg, PSM_PRESSBUTTON, button, 0L);
		})
	{
	}
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
