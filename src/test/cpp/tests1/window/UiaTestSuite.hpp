/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"
#include "dlg/ModalDialogCloser.hpp"

// UI Automation経由でGUI操作を行う
#include <UIAutomation.h>

namespace window {

struct UiaTestSuite
{
	using IUIAutomationPtr = cxx::com_pointer<IUIAutomation>;
	using IUIAutomationConditionPtr = cxx::com_pointer<IUIAutomationCondition>;
	using IUIAutomationElementPtr = cxx::com_pointer<IUIAutomationElement>;
	using IUIAutomationElementArrayPtr = cxx::com_pointer<IUIAutomationElementArray>;
	using IUIAutomationInvokePatternPtr = cxx::com_pointer<IUIAutomationInvokePattern>;
	using IUIAutomationValuePatternPtr = cxx::com_pointer<IUIAutomationValuePattern>;

	//! デフォルトの待機時間
	static constexpr auto defaultTimeoutMillis = 5000;

	/*!
	 * UI Automationオブジェクト
	 *
	 * UI操作に使うCOMオブジェクト。
	 */
	static inline IUIAutomationPtr m_pAutomation = nullptr;

	//! UI Automationでボタン押下
	static void EmulateInvoke(const IUIAutomationElementPtr& pElement)
	{
		IUIAutomationInvokePatternPtr pInvokePattern;
		_com_util::CheckError(pElement->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&pInvokePattern)));
		_com_util::CheckError(pInvokePattern->Invoke());
	}

	//! UI Automationで値設定
	static void EmulateSetValue(const IUIAutomationElementPtr& pElement, const _bstr_t& val)
	{
		IUIAutomationValuePatternPtr pValuePattern;
		_com_util::CheckError(pElement->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&pValuePattern)));
		_com_util::CheckError(pValuePattern->SetValue(val));
	}

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpUia();

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownUia();

	IUIAutomationConditionPtr CreatePropertyCondition(
		PROPERTYID propertyId,
		const _variant_t& value
	) const
	{
		IUIAutomationConditionPtr condition;
		_com_util::CheckError(m_pAutomation->CreatePropertyCondition(propertyId, value, &condition));
		return condition;
	}

	IUIAutomationElementPtr ElementFromHandle(_In_ HWND hWnd) const
	{
		IUIAutomationElementPtr pElement;
		_com_util::CheckError(m_pAutomation->ElementFromHandle(hWnd, &pElement));
		return pElement;
	}

	void EmulateInvokeButton(_In_ HWND hWndDlg, const _bstr_t& caption) const
	{
		// ボタンの検索条件を構築する
		auto pControlTypeCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
		auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, caption);
		auto pFinalCondition = CreateAndCondition(pControlTypeCondition, pNameCondition);

		// ボタンを検索する
		auto pItem = FindFirst(hWndDlg, TreeScope_Subtree, pFinalCondition);

		// ボタンを押下する
		EmulateInvoke(pItem);
	}

	void EmulateEnterOpenFileName(const std::filesystem::path& exportPath) const
	{
		if (const auto hWndDlgOpenFile = WaitForDialog(L"開く")) {
			EmulateSetValue(GetFocusedElement(), exportPath.filename().c_str());
			EmulateHitEnter();
		}
	}

	void EmulateEnterSaveFileName(const std::filesystem::path& exportPath) const
	{
		if (const auto hWndDlgSaveAs = WaitForDialog(L"名前を付けて保存")) {
			EmulateSetValue(GetFocusedElement(), exportPath.filename().c_str());
			EmulateHitEnter();
		}
	}

	void EmulateHitEnter() const
	{
		std::vector<INPUT> inputs{};
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, false));
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, true));
		EXPECT_THAT(SendInput(inputs), Eq(std::size(inputs)));
	}

	IUIAutomationElementPtr FindFirst(
		_In_ HWND hWndDlg,
		TreeScope scope,
		const IUIAutomationConditionPtr& pCondition
	) const
	{
		const auto pDlg = ElementFromHandle(hWndDlg);

		IUIAutomationElementPtr pItem;
		const auto startTick = ::GetTickCount64();
		while (::GetTickCount64() - startTick < defaultTimeoutMillis) {
			_com_util::CheckError(pDlg->FindFirst(scope, pCondition, &pItem));
			if (pItem) {
				break;
			}
	
			::Sleep(10);  // 10msスリープしてリトライ
		}

		return pItem;
	}

	IUIAutomationElementPtr GetFocusedElement() const
	{
		IUIAutomationElementPtr pFocusedElement;
		_com_util::CheckError(m_pAutomation->GetFocusedElement(&pFocusedElement));
		return pFocusedElement;
	}

	INPUT MakeMouseInputMove(LONG x, LONG y) const
	{
		const auto vx = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
		const auto vy = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
		const auto vw = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		const auto vh = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

		INPUT input{};

		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_MOVE
			| MOUSEEVENTF_ABSOLUTE
			| MOUSEEVENTF_VIRTUALDESK;
		input.mi.dx = ::MulDiv(x - vx, 65535, vw - 1);
		input.mi.dy = ::MulDiv(y - vy, 65535, vh - 1);

		return input;
	}

	INPUT MakeMouseInputWheel(int delta) const
	{
		INPUT input{};

		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		input.mi.mouseData = static_cast<DWORD>(delta);

		return input;
	}

	INPUT MakeKeyboardInput(WORD virtualKey, bool isKeyUp = false) const
	{
		INPUT input{};

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = virtualKey;
		input.ki.dwFlags = isKeyUp ? KEYEVENTF_KEYUP : 0;

		return input;
	}

	HWND WaitForDialog(const std::wstring& title) const
	{
		return WaitForWindow(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), title);
	}

	IUIAutomationElementPtr WaitForFocus(ULONGLONG startTick, ULONGLONG timeoutMillis) const
	{
		IUIAutomationElementPtr pFocusedElement = nullptr;

		do {
			_com_util::CheckError(m_pAutomation->GetFocusedElement(&pFocusedElement));

			if (pFocusedElement) {
				break;
			}

			::Sleep(10);  // 10msスリープしてリトライ
		}
		while (::GetTickCount64() - startTick < timeoutMillis);

		return pFocusedElement;
	}

	HWND WaitForWindow(LPCWSTR targetClass, const std::optional<std::wstring>& title = std::nullopt, bool waitCaret = true, ULONGLONG timeoutMillis = defaultTimeoutMillis) const
	{
		const auto startTick = ::GetTickCount64();

		do {
			// ウィンドウがVisibleかつEnabledになるのを待つ
			if (const auto hWndFound = ::FindWindowW(targetClass, title.has_value() ? title.value().c_str() : nullptr); hWndFound && ::IsWindowEnabled(hWndFound) && ::IsWindowVisible(hWndFound)) {
				if (!waitCaret) {
					return hWndFound;
				}

				WaitForFocus(startTick, timeoutMillis);

				return hWndFound;
			}

			Sleep(10);  // 10msスリープしてリトライ
		}
		while (::GetTickCount64() - startTick < timeoutMillis);

		return nullptr;
	}

	template<typename... Conditions>
	requires (1 < sizeof...(Conditions)) && (std::convertible_to<Conditions, IUIAutomationCondition*>&& ...)
	IUIAutomationConditionPtr CreateAndCondition(Conditions... conditions) const
	{
		std::array<IUIAutomationCondition*, sizeof...(Conditions)> arrayOfCondtions{
			static_cast<IUIAutomationCondition*>(conditions)...
		};

		IUIAutomationConditionPtr condition;
		_com_util::CheckError(m_pAutomation->CreateAndConditionFromNativeArray(std::data(arrayOfCondtions), int(std::size(arrayOfCondtions)), &condition));
		return condition;
	}

	template<typename T>
		requires std::ranges::range<T>
	UINT SendInput(T& inputs) const
	{
		return ::SendInput(UINT(std::size(inputs)), std::data(inputs), sizeof(decltype(inputs[0])));
	}
};

} // namespace env
