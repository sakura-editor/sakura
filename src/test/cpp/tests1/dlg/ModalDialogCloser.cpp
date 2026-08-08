/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "dlg/ModalDialogCloser.hpp"

#include "cxx/load_string.hpp"
#include "doc/CDocListener.h"

namespace window {

std::wstring GetClassNameW(_In_ HWND hWnd)
{
	// ウィンドウハンドルが有効かどうかチェックする
	if (!IsWindow(hWnd)) return {};

	// クラス名を受け取るバッファ配列を用意する
	constexpr auto maxWindowClassName = 256;
	std::array<wchar_t, maxWindowClassName + 1> buffer{};

	// APIを呼び出してクラス名を取得する
	const auto actual = ::GetClassNameW(hWnd, std::data(buffer), int(std::size(buffer)));
	if (actual <= 0) return {};

	// バッファと文字列長からstd::wstringを生成して返す
	return std::wstring(std::data(buffer), static_cast<size_t>(actual));
}

bool IsDialog(_In_ HWND hWnd, _In_opt_ LPCWSTR pClassName)
{
	if (pClassName && IS_INTRESOURCE(pClassName) && WC_DIALOG == pClassName) return true;

	const auto DIALOG_CLASS = std::format(L"#{:d}", LOWORD(WC_DIALOG));
	return DIALOG_CLASS == window::GetClassNameW(hWnd);
}

} // namespace window

namespace dialog {

bool ModalDialogCloser::ExecuteAction(HWND hWnd) noexcept
{
	Me* entry = nullptr;
	if (hWnd) {
		std::unique_lock lock{ gm_Mutex };

		const auto found = gm_HwndMap.find(hWnd);
		if (found == gm_HwndMap.end()) return false;

		entry = found->second;

		::KillTimer(hWnd, TIMER_ID_FIRST_IDLE);
		entry->m_TimerActive = false;

		entry->m_State = State::Running;

		gm_HwndMap.erase(found);
	}

	try {
		if (entry) {
			entry->m_Action(hWnd);

			entry->m_State = State::Handled;

			return true;
		}
	}
	catch (...) {
		std::unique_lock lock{ gm_Mutex };

		const auto exception = std::current_exception();
		if (entry) {
			entry->m_Exception = exception;
			entry->m_State = State::Failed;
		}

		try {
			std::rethrow_exception(exception);
		}
		catch (const std::exception& e) {
			std::clog << "ModalDialogCloser action failed: " << e.what() << std::endl;
		}
		catch (...) {
			std::clog << "ModalDialogCloser action failed with an unknown exception." << std::endl;
		}
	}

	return false;
}

void CALLBACK ModalDialogCloser::TimerProc(HWND hWnd, UINT, UINT_PTR idEvent, DWORD)
{
	ExecuteAction(hWnd);
}

/*!
 * CBTフックプロシージャ
 *
 * CBT(Computer-Based Training)フックを使ってウインドウイベントを捕捉する。
 */
LRESULT CALLBACK ModalDialogCloser::CBTProc(
  _In_ int    nCode,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
)
{
	const auto hWnd = std::bit_cast<HWND>(wParam);

	// ダイアログボックスの作成イベントを捕捉する
	if (const auto pCreateWnd = LPCBT_CREATEWND(lParam);
		HCBT_CREATEWND == nCode &&
		pCreateWnd &&
		pCreateWnd->lpcs &&
		window::IsDialog(hWnd, pCreateWnd->lpcs->lpszClass))
	{
		std::unique_lock lock{ gm_Mutex };

		if (!gm_Entries.empty())
		{
			const auto& entry = gm_Entries.front();
			if (const auto pDialogTitle = pCreateWnd->lpcs->lpszName;
				!entry->m_DialogTitle.has_value() ||
				(pDialogTitle && !IS_INTRESOURCE(pDialogTitle) && *entry->m_DialogTitle == pDialogTitle)) {
				entry->m_State = State::Created;
				entry->m_hWnd = hWnd;

				// ウィンドウハンドル監視エントリを入れる
				gm_HwndMap.try_emplace(hWnd, entry);

				// タイトル監視エントリを除去する
				gm_Entries.pop_front();

				// タイマーをセットする
				entry->m_TimerActive = 0 != ::SetTimer(hWnd, TIMER_ID_FIRST_IDLE, FALLBACK_DELAY_MILLIS, &TimerProc);
			}
		}
	}
	else if (HCBT_ACTIVATE == nCode)
	{
		if (ExecuteAction(hWnd)) {
			std::unique_lock lock{ gm_Mutex };

			if (gm_Entries.empty() && gm_HwndMap.empty()) {
				const auto ret = ::CallNextHookEx(nullptr, nCode, wParam, lParam);
				CbtHookHolder hook = std::move(gm_CbtHook);
				return ret;
			}
		}
	}
	return ::CallNextHookEx(nullptr, nCode, wParam, lParam);
}

ModalDialogCloser::ModalDialogCloser(
	const std::optional<std::wstring>& optTitle,
	const std::function<void(HWND)>& action
)
	: m_DialogTitle(optTitle)
	, m_Action(action)
{
	std::unique_lock lock{ gm_Mutex };

	gm_Entries.emplace_back(this);

	if (!gm_CbtHook) {

		::SetLastError(ERROR_SUCCESS);
		gm_CbtHook = ::SetWindowsHookExW(WH_CBT, &CBTProc, nullptr, ::GetCurrentThreadId());
		if (!gm_CbtHook) {
			m_HookError = ::GetLastError();
		}
	}
}

ModalDialogCloser::ModalDialogCloser(int dialogTitleResourceId, const std::function<void(HWND)>& action)
	: ModalDialogCloser(std::wstring{ cxx::load_string(dialogTitleResourceId) }, action)
{
}

ModalDialogCloser::ModalDialogCloser(const std::optional<std::wstring>& optTitle, int nIDDlgItem)
	: ModalDialogCloser(optTitle, [nIDDlgItem] (HWND hWndDlg)
		{
			FORWARD_WM_COMMAND(hWndDlg, nIDDlgItem, ::GetDlgItem(hWndDlg, nIDDlgItem), BN_CLICKED, ::SendMessageW);
		})
{
}

ModalDialogCloser::~ModalDialogCloser() noexcept
{
	const auto state = m_State;
	const auto hWnd = m_hWnd;
	const auto timerActive = m_TimerActive;
	const auto hookError = m_HookError;
	const auto exception = m_Exception;

	if (ERROR_SUCCESS != hookError) {
		ADD_FAILURE() << "SetWindowsHookExW failed with error " << hookError << ".";

		return;
	}

	if (State::Pending == state) {
		ADD_FAILURE() << (m_DialogTitle.has_value()
			? testing::Message() << "A dialog box named '" << m_DialogTitle.value() << "' was not created."
			: testing::Message() << "No dialog box was created.");
	}

	if (timerActive) {
		ADD_FAILURE() << "The timer for FIRST_IDLE is still active.";
	}

	if (State::Handled != state) {
		ADD_FAILURE() << "m_Action was not handled.";
	}

	const auto reportException = [] (const char* source, const std::exception_ptr& captured) {
		if (!captured) return;
		try {
			std::rethrow_exception(captured);
		}
		catch (const std::exception& e) {
			ADD_FAILURE() << source << " failed: " << e.what();
		}
		catch (...) {
			ADD_FAILURE() << source << " failed with an unknown exception.";
		}
	};
	reportException("ModalDialogCloser action", exception);

	{
		std::unique_lock lock{ gm_Mutex };

		std::erase(gm_Entries, this);

		if (const auto found = gm_HwndMap.find(hWnd);
			found != gm_HwndMap.end() && found->second == this)
		{
			gm_HwndMap.erase(found);
		}
	}

	m_TimerActive = false;

	m_hWnd = nullptr;

	m_State = State::Detached;
}

} // namespace dialog


/* static */ void MockCDlgOpenFile::_Cleanup([[maybe_unused]] const MockCDlgOpenFile*)
{
	gm_Files.clear();
}

/* static */ bool MockCDlgOpenFile::_GetOpenFileName(std::span<WCHAR> szPath, EFilter eAddFileter)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_GetSaveFileName(std::span<WCHAR> szPath)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFilenames,
	bool bOptions [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pLoadInfo->cFilePath = gm_Files.front().c_str();
		*pFilenames = gm_Files;
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pSaveInfo->cFilePath = gm_Files.front().c_str();
	}
	return result;
}

MockCDlgOpenFile::MockCDlgOpenFile()
{
	ON_CALL(*this, DoModal_GetOpenFileName(_, _)).WillByDefault(&_GetOpenFileName);
	ON_CALL(*this, DoModal_GetSaveFileName(_)).WillByDefault(&_GetSaveFileName);
	ON_CALL(*this, DoModalOpenDlg(_, _, _)).WillByDefault(&_DoModalOpenDlg);
	ON_CALL(*this, DoModalSaveDlg(_, _)).WillByDefault(&_DoModalSaveDlg);
}
