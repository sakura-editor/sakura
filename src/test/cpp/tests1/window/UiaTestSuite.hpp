/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"
 
// UI Automation経由でGUI操作を行う
#include <UIAutomation.h>

#include <comdef.h>

namespace window {

struct UiaTestSuite
{
	using IUIAutomationPtr = cxx::com_pointer<IUIAutomation>;
	using IUIAutomationConditionPtr = cxx::com_pointer<IUIAutomationCondition>;
	using IUIAutomationElementPtr = cxx::com_pointer<IUIAutomationElement>;
	using IUIAutomationElementArrayPtr = cxx::com_pointer<IUIAutomationElementArray>;
	using IUIAutomationInvokePatternPtr = cxx::com_pointer<IUIAutomationInvokePattern>;

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
		auto pButtonCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_ButtonControlTypeId);
		auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, caption);
		auto pFinalCondition = CreateAndCondition(pButtonCondition, pNameCondition);

		// ボタンを検索する
		auto pItem = FindFirst(hWndDlg, TreeScope_Subtree, pFinalCondition);

		// ボタンを押下する
		EmulateInvoke(pItem);
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

	HWND WaitForFocus(HWND hWndTarget, ULONGLONG startTick, ULONGLONG timeoutMillis) const
	{
		do {
			IUIAutomationElementPtr pFocusedElement = nullptr;
			_com_util::CheckError(m_pAutomation->GetFocusedElement(&pFocusedElement));

			if (pFocusedElement) {
				return hWndTarget;
			}

			::Sleep(10);  // 10msスリープしてリトライ
		}
		while (::GetTickCount64() - startTick < timeoutMillis);

		return nullptr;
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

				return WaitForFocus(hWndFound, startTick, timeoutMillis);
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
};

} // namespace env
