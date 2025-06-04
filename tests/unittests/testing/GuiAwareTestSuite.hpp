/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */

#pragma once

// UI Automation経由でGUI操作を行う
#include <UIAutomation.h>

// メッセージボックスのデフォルトタイトルGSTR_APPNAMEを使うために必要
#include "config/app_constants.h"

// 文字列リソース読み込みマクロ LS(id) を使うたｍめに必要
#include "CSelectLang.h"

// リソースヘッダーファイル
#include "sakura_rc.h"
#include "String_define.h"

// COMスマートポインタの定義（追加するときは昇順で。）
DEFINE_COM_SMARTPTR(IUIAutomation);
DEFINE_COM_SMARTPTR(IUIAutomationCondition);
DEFINE_COM_SMARTPTR(IUIAutomationElement);
DEFINE_COM_SMARTPTR(IUIAutomationElementArray);
DEFINE_COM_SMARTPTR(IUIAutomationInvokePattern);
DEFINE_COM_SMARTPTR(IUIAutomationLegacyIAccessiblePattern);
DEFINE_COM_SMARTPTR(IUIAutomationSelectionItemPattern);
DEFINE_COM_SMARTPTR(IUIAutomationValuePattern);

namespace basis {

/*!
 * メッセージエラー
 * 
 * ワイド文字列でインスタンス化できるエラー。
 */
class message_error : public std::runtime_error {
private:
	std::wstring _Message;

public:
	explicit message_error(std::wstring_view message);

	std::wstring_view message() const noexcept { return _Message; }
};

} // namespace basis

namespace testing {

template<typename BaseTestSuiteType>
struct TGuiAware : public BaseTestSuiteType {
	using Base = BaseTestSuiteType;

	//! デフォルトの待機時間
	static constexpr auto defaultTimeoutMillis = 5000;

	//! テストスイートの開始時にOLE初期化が成功したかどうか
	static inline bool gm_OleInitialized = false;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static bool SetUpGuiTestSuite() {
		// OLEを初期化する
		const auto oleInitialized = OleInitialize(nullptr);
		gm_OleInitialized = SUCCEEDED(oleInitialized);

		return gm_OleInitialized;
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownGuiTestSuite() {
		// OLEの初期化を解除する
		if (gm_OleInitialized) {
			OleUninitialize();
		}
	}

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(SetUpGuiTestSuite());
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// OLEの初期化を解除する
		TearDownGuiTestSuite();
	}
	
	static void EmulateInvoke(IUIAutomationElement* pElement)
	{
		// UI Automationでボタン押下
		IUIAutomationInvokePatternPtr pInvokePattern;
		_com_util::CheckError(pElement->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&pInvokePattern)));
		_com_util::CheckError(pInvokePattern->Invoke());
	}

	static void EmulateSetValue(IUIAutomationElement* pElement, std::wstring_view text)
	{
		// UI Automationで値を設定
		IUIAutomationValuePatternPtr pValuePattern;
		_com_util::CheckError(pElement->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&pValuePattern)));
		pValuePattern->SetValue(_bstr_t(text.data()));
	}

	static void EmulateSelectItem(IUIAutomationElement* pElement)
	{
		// UI Automationで要素を選択
		IUIAutomationSelectionItemPatternPtr pSelectPattern;
		_com_util::CheckError(pElement->GetCurrentPatternAs(UIA_SelectionItemPatternId, IID_PPV_ARGS(&pSelectPattern)));		
		_com_util::CheckError(pSelectPattern->Select());
	}
	/*!
	 * UI Automationオブジェクト
	 *
	 * UI操作に使うCOMオブジェクト。
	 */
	 IUIAutomationPtr m_pAutomation;

	 /*!
	  * テストが起動される直前に毎回呼ばれる関数
	  */
	void SetUpGuiTestCase() {
		// UI Automationオブジェクトを作成する
		_com_util::CheckError(m_pAutomation.CreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER));
	}
 
	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDownGuiTestCase() {
		// UI Automationオブジェクトを解放する
		m_pAutomation = nullptr;
	}
	 
	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// UI Automationオブジェクトを作成する
		SetUpGuiTestCase();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// UI Automationオブジェクトを解放する
		TearDownGuiTestCase();
	}

	auto CreatePropertyCondition(
		PROPERTYID propertyId,
		const _variant_t& value
	) const
	{
		IUIAutomationConditionPtr condition;
		_com_util::CheckError(m_pAutomation->CreatePropertyCondition(propertyId, value, &condition));
		return condition;
	}

	template<typename... Conditions>
	auto CreateAndCondition(Conditions... conditions) const
	{
		static_assert(1 < sizeof...(Conditions), "At least two conditions are required");
		static_assert((std::is_convertible_v<Conditions, IUIAutomationCondition*> && ...),
			"すべての引数は IUIAutomationCondition* でなければなりません");

		std::array arrayOfCondtions = { static_cast<IUIAutomationCondition*>(conditions)... };

		IUIAutomationConditionPtr condition;
		_com_util::CheckError(m_pAutomation->CreateAndConditionFromNativeArray(arrayOfCondtions.data(), int(std::size(arrayOfCondtions)), &condition));
		return condition;
	}

	auto GetFocusedElement() const
	{
		IUIAutomationElementPtr pFocusedElement;
		_com_util::CheckError(m_pAutomation->GetFocusedElement(&pFocusedElement));
		return pFocusedElement;
	}

	auto ElementFromHandle(
		_In_ HWND hWnd
	) const
	{
		IUIAutomationElementPtr pElement;
		_com_util::CheckError(m_pAutomation->ElementFromHandle(hWnd, &pElement));
		return pElement;
	}

	template<typename T>
	void ForEachItems(
		_In_ HWND hWndDlg,
		IUIAutomationCondition* conditon,
		T action
	) const
	{
		const auto pDlg = ElementFromHandle(hWndDlg);

		IUIAutomationElementArrayPtr pItems;
		_com_util::CheckError(pDlg->FindAll(TreeScope_Subtree, conditon, &pItems));
		int count = 0;
		pItems->get_Length(&count);

		for (int i = 0; i < count; ++i)
		{
			IUIAutomationElementPtr pItem;
			_com_util::CheckError(pItems->GetElement(i, &pItem));
			action(pItem);
		}
	}

	auto FindFirst(
		_In_ HWND hWndDlg,
		TreeScope scope,
		IUIAutomationCondition* pCondition
	) const
	{
		const auto pDlg = ElementFromHandle(hWndDlg);

		IUIAutomationElementPtr pItem;
		const auto startTick = GetTickCount64();
		while (GetTickCount64() - startTick < defaultTimeoutMillis) {
			_com_util::CheckError(pDlg->FindFirst(scope, pCondition, &pItem));
			if (pItem) {
				break;
			}
	
			Sleep(10);  // 10msスリープしてリトライ
		}

		return pItem;
	}

	auto GetItemAt(
		_In_ HWND hWndDlg,
		TreeScope scope,
		IUIAutomationCondition* pCondition,
		int index
	) const
	{
		const auto pDlg = ElementFromHandle(hWndDlg);

		IUIAutomationElementPtr pItem;

		const auto startTick = GetTickCount64();
		while (GetTickCount64() - startTick < defaultTimeoutMillis) {
			IUIAutomationElementArrayPtr pItems;
			_com_util::CheckError(pDlg->FindAll(scope, pCondition, &pItems));

			if (!pItems) {
				Sleep(10);
				continue;
			}

			int count = 0;
			_com_util::CheckError(pItems->get_Length(&count));

			if (count <= index) {
				Sleep(10);
				continue;
			}

			_com_util::CheckError(pItems->GetElement(index, &pItem));

			if (pItem) {
				break;
			}

			Sleep(10);  // 10msスリープしてリトライ
		}

		return pItem;
	}

	_Success_(return != nullptr)
	HWND WaitForWindow(
		_In_ LPCWSTR targetClass,
		std::optional<LPCWSTR> windowName = std::nullopt,
		_Outptr_opt_result_maybenull_ IUIAutomationElement * *ppWnd = nullptr
	) const
	{
		HWND hWndFound = nullptr;

		if (ppWnd) {
			*ppWnd = nullptr;
		}

		// ウィンドウがVisibleかつEnabledになるのを待つ
		const auto startTick = GetTickCount64();
		while (!hWndFound && GetTickCount64() - startTick < defaultTimeoutMillis) {
			hWndFound = FindWindowW(targetClass, windowName.value_or(nullptr));
			if (!hWndFound) {
				//do nothing
			}
			else if (!IsWindowVisible(hWndFound) || !IsWindowEnabled(hWndFound)) {
				hWndFound = nullptr;
			}
			else if (const auto pFocusedElement = GetFocusedElement(); !pFocusedElement) {
				hWndFound = nullptr;
			}
			else {
				break;
			}
	
			Sleep(10);  // 10msスリープしてリトライ
		}

		if (hWndFound && ppWnd) {
			*ppWnd = ElementFromHandle(hWndFound);
		}

		return hWndFound;
	}

	_Success_(return != nullptr)
	HWND WaitForContextMenu() const
	{
		return WaitForWindow(MAKEINTRESOURCE(32768), std::nullopt);
	}

	_Success_(return != nullptr)
	HWND WaitForDialog(
		std::optional<std::wstring_view> caption = std::nullopt
	) const
	{
		const auto windowName = caption.has_value() ? caption.value().data() : nullptr;

		const auto hWndDlg = WaitForWindow(MAKEINTRESOURCE(32770), windowName);

		EXPECT_THAT(hWndDlg, Ne(nullptr));

		return hWndDlg;
	}

	void EmulateInvokeButton(_In_ HWND hWndDlg, std::wstring_view caption) const {
		// ボタンの検索条件を構築する
		auto pButtonCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
		auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, caption.data());
		auto pFinalCondition = CreateAndCondition(pButtonCondition, pNameCondition);

		// ボタンを検索する
		auto pItem = FindFirst(hWndDlg, TreeScope_Subtree, pFinalCondition);

		// ボタンを押下する
		EmulateInvoke(pItem);
	}
	
	void EmulateInvokeMenuItem(_In_ HWND hWndDlg, int index) const {
		// メニュー項目を選択する
		const auto pMenuItemCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_MenuItemControlTypeId);
		const auto pItem = GetItemAt(hWndDlg, TreeScope_Children, pMenuItemCondition, index);
		EXPECT_THAT(pItem, Ne(nullptr));

		EmulateInvoke(pItem);
	}

	template <typename T>
	void WaitForClose(HWND hWnd, T action) const
	{
		// ウインドウを閉じる。
		action();

		const auto startTick = GetTickCount64();

		while (IsWindow(hWnd) && GetTickCount64() - startTick < defaultTimeoutMillis) {
			Sleep(10);  // 10msスリープしてリトライ
		}
	}

	template<typename T>
	void ExpectMsgBox(const T& action, std::wstring_view caption, std::optional<std::wstring_view> expected, std::optional<std::wstring_view> button = std::nullopt)
	{
		HWND hWndMsgBox = nullptr;

		std::thread t([&] {
			// メッセージボックスが表示されるのを待つ
			const auto hWndMsgBox = WaitForDialog(caption);

			if (expected.has_value()) {
				// スタティックコントロールを取得する
				const auto hWndMsg = FindWindowExW(hWndMsgBox, nullptr, WC_STATIC, expected.value().data());
				EXPECT_THAT(hWndMsg, Ne(nullptr));
			}

			// ボタンの検索条件を構築
			auto pButtonCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
			IUIAutomationConditionPtr pFinalCondition;
			if (button.has_value()) {
				auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, button.value().data());
				pFinalCondition = CreateAndCondition(pButtonCondition, pNameCondition);
			} else {
				pFinalCondition = pButtonCondition;
			}

			WaitForClose(hWndMsgBox, [this, hWndMsgBox, pFinalCondition] {
				// ボタンを押下
				auto pButton = FindFirst(hWndMsgBox, TreeScope_Children, pFinalCondition);
				EmulateInvoke(pButton);
			});
		});

		action();

		t.join();
	}
};

} // end of namespace testing

#define EXPECT_MSGBOX(action, title, expected) \
	ExpectMsgBox([&]() { action; }, title, expected)

#define EXPECT_MSGBOX2(action, title, expected, button) \
	ExpectMsgBox([&]() { action; }, title, expected, button)
