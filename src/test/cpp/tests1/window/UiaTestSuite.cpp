/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "window/UiaTestSuite.hpp"

#include "config/system_constants.h"
#include "cxx/type_of_Nth_lambda_arg.hpp"
#include "util/window.h"

namespace cxx {

template<typename TQuery, typename R = cxx::lambda_traits<TQuery>::return_type>
R WaitForQuery(
	TQuery query,
	std::stop_token st,
	ULONGLONG timeoutMillis,
	DWORD intervalMillis = 100
)
{
	const auto startTick = ::GetTickCount64();

	while (!st.stop_requested()) {
		// 脱出条件を満たしたら抜ける
		auto result = query();
		if (result) return result;

		// タイムアウトしたら抜ける
		const auto elapsed = ::GetTickCount64() - startTick;	// 経過時間
		if (timeoutMillis <= elapsed) return result;

		// 停止要求が来ていたら待機せずに抜ける
		if (st.stop_requested()) return result;

		// GUI待機なので残り時間を考慮して待機する
		const auto remaining = timeoutMillis - elapsed;			// 残り時間
		::Sleep(std::min(intervalMillis, DWORD(remaining)));
	}

	return {};
}

} // namespace cxx

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

HWND WaitForDialog(
	const std::wstring& title,
	std::stop_token st,
	ULONGLONG timeoutMillis
)
{
	return WaitForWindow(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), title, st, timeoutMillis);
}

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
	LPCWSTR pszTitle = optTitle.has_value() ? optTitle.value().c_str() : nullptr;

	// ウィンドウがVisibleかつEnabledになるのを待つ
	return cxx::WaitForQuery([targetClass, pszTitle] {
		// 全プロセスのトップレベルウィンドウを対象に検索する
		const auto hWndFound = ::FindWindowW(targetClass, pszTitle);
		if (!hWndFound) {
			std::clog << "window not found." << std::endl;
		} else if (!::IsWindowVisible(hWndFound)) {
			std::clog << "window is not visible." << std::endl;
			return HWND(nullptr);
		} else if (!::IsWindowEnabled(hWndFound)) {
			std::clog << "window is disabled." << std::endl;
			return HWND(nullptr);
		} else if (hWndFound != ::GetForegroundWindow()) {
			std::clog << "window is not foreground." << std::endl;
			return HWND(nullptr);
		}
		return hWndFound;
	}, st, timeoutMillis);
}

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::SetUpUia()
{
	// OLEを初期化する
	pcOleInit = std::make_unique<cxx::COleInit>();

	// 初期化に失敗した場合はテストを中止する
	ASSERT_TRUE(pcOleInit);
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void UiaTestSuite::TearDownUia()
{
	// OLEをシャットダウンする
	pcOleInit = nullptr;
}

/* static */ void UiaTestSuite::EmulateInvokeButton(
	IUIAutomation* pAutomation,
	_In_ HWND hWndDlg,
	std::wstring_view caption,
	std::stop_token st
)
{
	// ボタンの検索条件を構築する
	auto pControlTypeCondition = uia::CreatePropertyCondition(pAutomation, UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
	auto pNameCondition = uia::CreatePropertyCondition(pAutomation, UIA_NamePropertyId, _bstr_t{ ::SysAllocStringLen(std::data(caption), UINT(std::size(caption))) });
	auto pFinalCondition = uia::CreateAndCondition(pAutomation, pControlTypeCondition, pNameCondition);

	// ボタンを検索する
	auto pItem = uia::FindFirst(pAutomation, hWndDlg, TreeScope_Subtree, pFinalCondition, st);
	ASSERT_THAT(pItem, NotNull()) << "button not found: " << cxx::to_string(caption);

	if (st.stop_requested()) {
		return;
	}

	// ボタンを押下する
	EmulateInvoke(pItem);
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

/* static */ std::function<void(IUIAutomation*, HWND, std::stop_token)> UiaTestSuite::EmulateEnterFileName(
	const std::filesystem::path& path
)
{
	return [path] (IUIAutomation* pAutomation, HWND, std::stop_token st) {
		// ファイル名を入力する
		EmulateSetValue(uia::GetFocusedElement(pAutomation), path.native());

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// Enterキーを押下して閉じる
		EmulateHitEnter();
	};
}

/* static */ void UiaTestSuite::EmulateEnterOpenFileName(
	IUIAutomation* pAutomation,
	const std::filesystem::path& path,
	std::stop_token st
)
{
	const auto action = EmulateEnterFileName(path);

	if (const auto hWndDlgOpenFile = window::WaitForDialog(L"開く", st)) {
		action(pAutomation, hWndDlgOpenFile, st);
	}
}

/* static */ void UiaTestSuite::EmulateEnterSaveFileName(
	IUIAutomation* pAutomation,
	const std::filesystem::path& path,
	std::stop_token st
)
{
	const auto action = EmulateEnterFileName(path);

	if (const auto hWndDlgOpenFile = window::WaitForDialog(L"名前を付けて保存", st)) {
		action(pAutomation, hWndDlgOpenFile, st);
	}
}

/*!
 * ダイアログを閉じるスレッドを開始する
 *
 * @param dialogTitle タイトル
 * @param action 閉じるアクション
 * @param timeoutMillis タイムアウト時間（ミリ秒）
 * @return ダイアログを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartDialogCloser(
	std::wstring_view dialogTitle,
	const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
	ULONGLONG timeoutMillis
) const
{
	LPCWSTR targetClass = MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS);

	return StartWindowCloser(targetClass, std::wstring{ dialogTitle }, action, timeoutMillis);
}

/*!
 * ファイルを開くダイアログを閉じるスレッドを開始する
 *
 * @param path ファイルパス
 * @return ファイルを開くダイアログを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartEnterOpenFileName(
	const std::filesystem::path& path
) const
{
	return StartDialogCloser(L"開く", EmulateEnterFileName(path));
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
) const
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
 * UI Automationを利用するスレッドを開始する
 *
 * @param action 実行するアクション
 * @return スレッド
 */
std::jthread UiaTestSuite::StartUiaThread(
	const std::function<void(IUIAutomation*, std::stop_token)>& action
) const
{
	return std::jthread([this, action] (std::stop_token st) {
		// OLEを初期化する
		cxx::COleInit oleInit;
		ASSERT_TRUE(oleInit);

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// UI Automationオブジェクトを作成する
		IUIAutomationPtr pAutomation = nullptr;
		ASSERT_HRESULT_SUCCEEDED(pAutomation.CreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER));

		// 停止要求が来ていたら抜ける
		if (st.stop_requested()) {
			return;
		}

		// アクションを実行する
		RunGuiTest([action, pAutomation, st] {
			action(pAutomation, st);
		});
	});
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
) const
{
	return StartUiaThread([targetClass, optTitle, action, timeoutMillis] (IUIAutomation* pAutomation, std::stop_token st) {
		// テスト対象ウィンドウを検索する
		const auto hWndFound = window::WaitForWindow(targetClass, optTitle, st, timeoutMillis);
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
 * ダイアログを閉じるスレッドを開始する
 *
 * @param title タイトル
 * @param timeoutMillis タイムアウト時間（ミリ秒）
 * @return ダイアログを閉じるためのスレッド
 */
std::jthread UiaTestSuite::StartDialogCloser(
	std::wstring_view title,
	ULONGLONG timeoutMillis
) const
{
	return StartDialogCloser(title, [] (IUIAutomation* pUIAutomation, HWND hWndDlg, std::stop_token st) {
		// OKボタンを押下して閉じる
		EmulateInvokeButton(pUIAutomation, hWndDlg, IDOK, st);
	}, timeoutMillis);
}

} // namespace window
