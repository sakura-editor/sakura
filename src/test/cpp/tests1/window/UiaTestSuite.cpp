/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/UiaTestSuite.hpp"

#include "config/system_constants.h"
#include "cxx/load_string.hpp"
#include "util/window.h"

using namespace std::literals::string_literals;

namespace uia {

inline cxx::com_pointer<IUIAutomationElement> FindFirst(
	const cxx::com_pointer<IUIAutomationElement>& pDlg,
	TreeScope scope,
	const cxx::com_pointer<IUIAutomationCondition>& pCondition
)
{
	cxx::com_pointer<IUIAutomationElement> pItem;
	if (FAILED(pDlg->FindFirst(scope, pCondition, &pItem))) return nullptr;
	return pItem;
}

cxx::com_pointer<IUIAutomationCondition> CreatePropertyCondition(
	IUIAutomation* pAutomation,
	PROPERTYID propertyId,
	const _variant_t& value
)
{
	cxx::com_pointer<IUIAutomationCondition> condition;
	_com_util::CheckError(pAutomation->CreatePropertyCondition(propertyId, value, &condition));
	return condition;
}

cxx::com_pointer<IUIAutomationElement> ElementFromHandle(
	IUIAutomation* pAutomation,
	_In_ HWND hWnd
)
{
	cxx::com_pointer<IUIAutomationElement> pElement;
	_com_util::CheckError(pAutomation->ElementFromHandle(hWnd, &pElement));
	return pElement;
}

cxx::com_pointer<IUIAutomationElement> FindFirst(
	IUIAutomation* pAutomation,
	_In_ HWND hWndDlg,
	TreeScope scope,
	const cxx::com_pointer<IUIAutomationCondition>& pCondition,
	std::stop_token st = {},
	ULONGLONG timeoutMillis = window::defaultTimeoutMillis
)
{
	const auto pDlg = ElementFromHandle(pAutomation, hWndDlg);
	return cxx::WaitForQuery([pDlg, scope, pCondition] {
		return uia::FindFirst(pDlg, scope, pCondition);
	}, st, timeoutMillis);
}

cxx::com_pointer<IUIAutomationElement> GetFocusedElement(
	IUIAutomation* pAutomation
)
{
	cxx::com_pointer<IUIAutomationElement> pFocusedElement;
	_com_util::CheckError(pAutomation->GetFocusedElement(&pFocusedElement));
	return pFocusedElement;
}

template<typename... Conditions>
	requires (1 < sizeof...(Conditions)) && (std::convertible_to<Conditions, IUIAutomationCondition*>&& ...)
inline cxx::com_pointer<IUIAutomationCondition> CreateAndCondition(
	IUIAutomation* pAutomation,
	Conditions... conditions
)
{
	std::array<IUIAutomationCondition*, sizeof...(Conditions)> arrayOfCondtions{
		static_cast<IUIAutomationCondition*>(conditions)...
	};

	cxx::com_pointer<IUIAutomationCondition> condition;
	_com_util::CheckError(pAutomation->CreateAndConditionFromNativeArray(std::data(arrayOfCondtions), int(std::size(arrayOfCondtions)), &condition));
	return condition;
}

} // namespace uia

namespace window {

bool IsDialog(_In_ HWND hWnd, _In_opt_ LPCWSTR pClassName);

HWND WaitForPopupMenu(
	std::stop_token st,
	ULONGLONG timeoutMillis
)
{
	LPCWSTR targetClass = MAKEINTRESOURCEW(32768);	// 検索対象はポップアップメニュー固定
	LPCWSTR pszTitle = nullptr;						// ウィンドウキャプションは指定しない

	return cxx::WaitForQuery([targetClass, pszTitle] {
		// 全プロセスのトップレベルウィンドウを対象に検索する
		const auto hWndFound = ::FindWindowW(targetClass, pszTitle);
		if (!hWndFound) {
			std::clog << "popup-menu not found." << std::endl;
		} else if (!::IsWindowVisible(hWndFound)) {
			std::clog << "popup-menu is not visible." << std::endl;
			return HWND(nullptr);
		} else if (!::IsWindowEnabled(hWndFound)) {
			std::clog << "popup-menu is disabled." << std::endl;
			return HWND(nullptr);
		}
		return hWndFound;
	}, st, timeoutMillis);
}

HWND WaitForWindow(
	LPCWSTR targetClass,
	const std::optional<std::wstring>& optTitle,
	std::stop_token st,
	ULONGLONG timeoutMillis
)
{
	const auto windowType = IS_INTRESOURCE(targetClass) && WC_DIALOG == targetClass ? "Dialog"s : "Window"s;
	const auto dispTitle = std::format("'{:s}'", cxx::to_string(optTitle.value_or(L""), CP_UTF8));

	// ウィンドウがVisibleかつEnabledになるのを待つ
	return cxx::WaitForQuery([targetClass, optTitle, windowType, dispTitle] {
		// 全プロセスのトップレベルウィンドウを対象に検索する
		const auto hWndFound = ::FindWindowW(targetClass, optTitle.has_value() ? std::data(optTitle.value()) : nullptr);
		if (!hWndFound) {
			std::clog << std::format("{:s} not found.", windowType) << (optTitle.has_value() ? std::format(" title: {:s}", dispTitle) : "") << std::endl;
		} else if (!::IsWindowVisible(hWndFound)) {
			std::clog << std::format("{:s} is not visible.", windowType) << (optTitle.has_value() ? std::format(" title: {:s}", dispTitle) : "") << std::endl;
			return HWND(nullptr);
		} else if (!::IsWindowEnabled(hWndFound)) {
			std::clog << std::format("{:s} is disabled.", windowType) << (optTitle.has_value() ? std::format(" title: {:s}", dispTitle) : "") << std::endl;
			return HWND(nullptr);
		} else if (hWndFound != ::GetForegroundWindow()) {
			std::clog << std::format("{:s} is not foreground.", windowType) << (optTitle.has_value() ? std::format(" title: {:s}", dispTitle) : "") << std::endl;
			::SetForegroundWindow(hWndFound);
		}
		return hWndFound;
	}, st, timeoutMillis);
}

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::SetUpUiaTestSuite()
{
	// OLEを初期化する
	pcOleInit = std::make_unique<cxx::COleInit>();

	// 初期化に失敗した場合はテストを中止する
	ASSERT_TRUE(pcOleInit);
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::TearDownUiaTestSuite()
{
	// OLEをシャットダウンする
	pcOleInit = nullptr;
}

/* static */ void UiaTestSuite::EmulateInvokeButton(
	IUIAutomation* pAutomation,
	_In_ HWND hWndDlg,
	int nIDDlgItem,
	std::stop_token st
)
{
	// コントロールIDを10進数文字列にする
	const auto automationId = std::to_wstring(nIDDlgItem);

	// ボタンの検索条件を構築する
	auto pControlTypeCondition = uia::CreatePropertyCondition(pAutomation, UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
	auto pAutomationIdCondition = uia::CreatePropertyCondition(pAutomation, UIA_AutomationIdPropertyId, _bstr_t{ ::SysAllocStringLen(std::data(automationId), UINT(std::size(automationId))) });
	auto pFinalCondition = uia::CreateAndCondition(pAutomation, pControlTypeCondition, pAutomationIdCondition);

	// ボタンを検索する
	auto pItem = uia::FindFirst(pAutomation, hWndDlg, TreeScope_Subtree, pFinalCondition, st);
	ASSERT_THAT(pItem, NotNull()) << "button not found: #" << cxx::to_string(automationId);

	if (st.stop_requested()) {
		return;
	}

	// ボタンを押下する
	EmulateInvoke(pItem);
}

/*!
 * @brief スレッド実行をブロックしているウィンドウを列挙するコールバック関数
 *
 * @param hWnd 列挙されたウィンドウのハンドル
 * @param lParam 列挙の呼び出し元から渡されたパラメータ（使用しない）
 */
/* static */ BOOL CALLBACK UiaTestSuite::CloseBlockingWindowProc(
	_In_ HWND   hWnd,
	_In_ LPARAM lParam [[maybe_unused]]
)
{
	if (hWnd && ::IsWindow(hWnd) && ::IsWindowVisible(hWnd)) {
		if (window::IsDialog(hWnd, nullptr)) {
			::EndDialog(hWnd, 0);
		} else {
			::DestroyWindow(hWnd);
		}
	}

	return TRUE;
}

/* static */ HWND UiaTestSuite::GetActivePage(
	HWND hWndDlg,
	std::stop_token st,
	ULONGLONG timeoutMillis
)
{
	return cxx::WaitForQuery([hWndDlg] {
		return HWND(::SendMessageW(hWndDlg, PSM_GETCURRENTPAGEHWND, 0L, 0L));
	}, st, timeoutMillis);
}

/* static */ void UiaTestSuite::EmulateInvokeMenuItem(
	IUIAutomation* pAutomation,
	_In_ HWND hWndPopupMenu,
	std::wstring_view caption,
	std::stop_token st
)
{
	// メニュー項目の検索条件を構築する
	auto pControlTypeCondition = uia::CreatePropertyCondition(pAutomation, UIA_ControlTypePropertyId, UIA_MenuItemControlTypeId);
	auto pNameCondition = uia::CreatePropertyCondition(pAutomation, UIA_NamePropertyId, _bstr_t{ ::SysAllocStringLen(std::data(caption), UINT(std::size(caption))) });
	auto pFinalCondition = uia::CreateAndCondition(pAutomation, pControlTypeCondition, pNameCondition);

	// メニュー項目を検索する
	auto pItem = uia::FindFirst(pAutomation, hWndPopupMenu, TreeScope_Subtree, pFinalCondition, st);
	ASSERT_THAT(pItem, NotNull()) << "menu-item not found: " << cxx::to_string(caption);

	if (st.stop_requested()) {
		return;
	}

	// メニュー項目を選択する
	EmulateInvoke(pItem);
}

/* static */ void UiaTestSuite::SendDlgCommand(
	_In_ HWND hWndDlg,
	int nIDDlgItem,
	int notifyCode
)
{
	const auto hWndCtrl = ::GetDlgItem(hWndDlg, nIDDlgItem);
	ASSERT_THAT(hWndCtrl, NotNull()) << "control not found: #" << nIDDlgItem;

	EXPECT_THAT(::SendMessageTimeoutW(hWndDlg, WM_COMMAND, MAKEWPARAM(nIDDlgItem, notifyCode), LPARAM(hWndCtrl), SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr), IsTrue());
}

/* static */ void UiaTestSuite::SendPsmPressButton(
	_In_ HWND hWndPropertySheet,
	UINT button
)
{
	EXPECT_THAT(::SendMessageTimeoutW(hWndPropertySheet, PSM_PRESSBUTTON, button, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr), IsTrue());
}

/*!
 * @brief UI Automationを使ったテストを実行する
 */
template <class TAction>
void UiaTestSuite::RunUiaAction(
	TAction&& action
) const
{
	const auto st = m_StopSource.get_token();

	RunGuiTest([action, st] {
		// UI Automationオブジェクトを作成する
		IUIAutomationPtr pAutomation = nullptr;
		ASSERT_HRESULT_SUCCEEDED(pAutomation.CreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER));

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		std::forward<TAction>(action)(pAutomation, st);
	});
}

/*!
 * UIAテストケースの開始前に自分で呼ぶ関数
 */
void UiaTestSuite::SetUpUia()
{
	const auto testThreadId = ::GetCurrentThreadId();

	m_StopSource = std::stop_source{};
	m_Thread = std::jthread([this, testThreadId] (std::stop_token st) {
		m_Timeout = true;	// 初期値は「タイムアウト発生」にする

		std::mutex mutex;
		std::condition_variable_any condition;
		std::unique_lock lock(mutex);
		condition.wait_for(lock, st, std::chrono::seconds(30), [] { return false; });

		if (st.stop_requested()) {
			m_Timeout = false;	// タイムアウトは発生しなかった
			return;
		}

		m_StopSource.request_stop();

		// スレッドブロックしているウィンドウを列挙して閉じる
		::EnumThreadWindows(testThreadId, CloseBlockingWindowProc, 0L);
	});
}

/*!
 * UIAテストケースの終了後に呼ばれる関数
 */
void UiaTestSuite::TearDownUia()
{
	m_StopSource.request_stop();

	m_Thread.request_stop();

	if (m_Thread.joinable()) {
		m_Thread.join();
	}

	if (m_Timeout) {
		ADD_FAILURE() << "test case timed out after 30 seconds";
	}
}

/*!
 * ダイアログを閉じるスレッドを開始する
 *
 * @param optTitle タイトル
 * @param action 閉じるアクション
 * @param timeoutMillis タイムアウト時間（ミリ秒）
 * @return ダイアログを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartDialogCloser(
	const std::optional<std::wstring>& optTitle,
	const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
	ULONGLONG timeoutMillis
)
{
	return StartWindowCloser(WC_DIALOG, optTitle, action, timeoutMillis);
}

/*!
 * ダイアログを閉じるスレッドを開始する
 *
 * @param titleResourceId タイトルのリソースID
 * @param action 閉じるアクション
 * @return ダイアログを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartDialogCloser(
	int titleResourceId,
	const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action
)
{
	const std::wstring buffer{ cxx::load_string(titleResourceId) };

	return StartDialogCloser(buffer, action);
}

/*!
 * ポップアップメニューを選択するスレッドを開始する
 *
 * @param menuLabel メニューラベル
 * @return ポップアップメニューを選択するためのスレッド
 */
std::jthread UiaTestSuite::StartPopupMenuSelector(
	std::wstring_view menuLabel,
	ULONGLONG timeoutMillis
)
{
	// 閉じるアクションを構築する
	const auto action = [menuLabel] (IUIAutomation* pAutomation, HWND hWndPopupMenu, std::stop_token st) {
		EmulateInvokeMenuItem(pAutomation, hWndPopupMenu, menuLabel, st);
	};

	return StartUiaThread([action, timeoutMillis] (IUIAutomation* pAutomation, std::stop_token st) {
		// ポップアップメニューを検索する
		const auto hWndFound = window::WaitForPopupMenu(st, timeoutMillis);
		ASSERT_THAT(hWndFound, NotNull());

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// 閉じるアクションを実行する
		action(pAutomation, hWndFound, st);
	});
}

/*!
 * プロパティーシートを閉じるスレッドを開始する
 *
 * @param titleResourceId タイトルのリソースID
 * @param psButtonId ボタンID
 * @return プロパティーシートを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartPropertySheetCloser(
	int titleResourceId,
	UINT psButtonId
)
{
	return StartDialogCloser(titleResourceId, [psButtonId] (IUIAutomation*, HWND hWndDlg, std::stop_token) {
		SendPsmPressButton(hWndDlg, psButtonId);
	});
}

/*!
 * UI Automationを利用するスレッドを開始する
 *
 * @param action 実行するアクション
 * @return スレッド
 */
std::jthread UiaTestSuite::StartUiaThread(
	const std::function<void(IUIAutomation*, std::stop_token)>& action
)
{
	SetUpUia();

	return std::jthread([this, action] (std::stop_token st) {
		// OLEを初期化する
		cxx::COleInit oleInit;
		ASSERT_TRUE(oleInit);

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// アクションを実行する
		RunUiaAction(action);

	}, m_StopSource.get_token());
}

/*!
 * ウィンドウを閉じるスレッドを開始する
 *
 * @param targetClass ウィンドウクラス名
 * @param optTitle タイトル
 * @param action 閉じるアクション
 * @param timeoutMillis タイムアウト時間（ミリ秒）
 * @return ウィンドウを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartWindowCloser(
	LPCWSTR targetClass,
	const std::optional<std::wstring>& optTitle,
	const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
	ULONGLONG timeoutMillis
)
{
	return StartUiaThread([targetClass, has_title = optTitle.has_value(), title = optTitle.value_or(L""), action, timeoutMillis] (IUIAutomation* pAutomation, std::stop_token st) {
		// テスト対象ウィンドウを検索する
		const auto hWndFound = window::WaitForWindow(targetClass, has_title ? std::optional(title) : std::nullopt, st, timeoutMillis);
		ASSERT_THAT(hWndFound, NotNull());

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// 閉じるアクションを実行する
		action(pAutomation, hWndFound, st);
	});
}

} // namespace window
