/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"
#include "dlg/ModalDialogCloser.hpp"
#include "util/tchar_convert.h"

// UI Automation経由でGUI操作を行う
#include <UIAutomation.h>

namespace cxx {

/*!
 * @brief COMライブラリの初期化状態をRAIIで管理するクラス
 */
class COleInit final
{
private:
	/*!
	 * @brief COMライブラリのクリーンアップ
	 */
	static void Cleanup([[maybe_unused]] const COleInit*)
	{
		// OLEをシャットダウンする
		::OleUninitialize();
	}

	using CleanupHolder = cxx::ResourceHolder<&COleInit::Cleanup>;

	using Me = COleInit;

	/*!
	 * @brief COMライブラリの初期化状態
	 * 
	 * クラスの生成と同時にSTAモードでCOMライブラリを初期化する。
	 * 初期化に成功した場合、クリーンアップ用のリソースホルダーを生成する。
	 * 初期化失敗した場合、リソースホルダーはnullptrとなり、クリーンアップは行われない。
	 */
	CleanupHolder m_Initialized = SUCCEEDED(::OleInitialize(nullptr)) ? this : nullptr;

public:
	COleInit() = default;
	~COleInit() noexcept = default;

	COleInit(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	explicit operator bool() const noexcept {
		return m_Initialized;
	}
};

} // namespace cxx

namespace window {

//! デフォルトの待機時間
static constexpr auto defaultTimeoutMillis = 5000;

HWND WaitForDialog(
	const std::wstring& title,
	std::stop_token st = {},
	ULONGLONG timeoutMillis = defaultTimeoutMillis
);

HWND WaitForPopupMenu(
	std::stop_token st,
	ULONGLONG timeoutMillis = defaultTimeoutMillis
);

HWND WaitForWindow(
	LPCWSTR targetClass,
	const std::optional<std::wstring>& optTitle = std::nullopt,
	std::stop_token st = {},
	ULONGLONG timeoutMillis = defaultTimeoutMillis
);

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

	static inline std::unique_ptr<cxx::COleInit> pcOleInit = nullptr;

	static void EmulateInvokeButton(
		IUIAutomation* pAutomation,
		_In_ HWND hWndDlg,
		std::wstring_view caption,
		std::stop_token st
	);

	static void EmulateInvokeButton(
		IUIAutomation* pAutomation,
		_In_ HWND hWndDlg,
		int nIDDlgItem,
		std::stop_token st
	);

	static void EmulateInvokeMenuItem(
		IUIAutomation* pAutomation,
		_In_ HWND hWndPopupMenu,
		std::wstring_view caption,
		std::stop_token st
	);

	static std::function<void(IUIAutomation*, HWND, std::stop_token)> EmulateEnterFileName(
		const std::filesystem::path& path
	);

	static void EmulateEnterOpenFileName(
		IUIAutomation* pAutomation,
		const std::filesystem::path& path,
		std::stop_token st = {}
	);

	static void EmulateEnterSaveFileName(
		IUIAutomation* pAutomation,
		const std::filesystem::path& path,
		std::stop_token st = {}
	);

	//! UI AutomationでEnterキー押下を偽装
	static void EmulateHitEnter()
	{
		std::vector<INPUT> inputs{};
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, false));
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, true));
		EXPECT_THAT(SendInput(inputs), Eq(std::size(inputs)));
	}

	//! UI Automationでボタン押下を偽装
	static void EmulateInvoke(
		const IUIAutomationElementPtr& pElement
	)
	{
		IUIAutomationInvokePatternPtr pInvokePattern;
		ASSERT_HRESULT_SUCCEEDED(pElement->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&pInvokePattern)));
		ASSERT_HRESULT_SUCCEEDED(pInvokePattern->Invoke());
	}

	//! UI Automationで値設定
	static void EmulateSetValue(
		const IUIAutomationElementPtr& pElement,
		std::wstring_view val
	)
	{
		IUIAutomationValuePatternPtr pValuePattern;
		ASSERT_HRESULT_SUCCEEDED(pElement->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&pValuePattern)));
		ASSERT_HRESULT_SUCCEEDED(pValuePattern->SetValue(_bstr_t{ ::SysAllocStringLen(std::data(val), UINT(std::size(val))) }));
	}

	static INPUT MakeMouseInputMove(LONG x, LONG y)
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

	static INPUT MakeMouseInputWheel(int delta)
	{
		INPUT input{};

		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		input.mi.mouseData = static_cast<DWORD>(delta);

		return input;
	}

	static INPUT MakeKeyboardInput(WORD virtualKey, bool isKeyUp = false)
	{
		INPUT input{};

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = virtualKey;
		input.ki.dwFlags = isKeyUp ? KEYEVENTF_KEYUP : 0;

		return input;
	}

	template <typename Func>
	static void RunGuiTest(Func&& f)
	{
		try {
			std::forward<Func>(f)();
		}
		catch (const std::exception& e) {
			FAIL() << "std::exception: " << typeid(e).name()
				<< ": " << e.what();
		}
		catch (const _com_error& e) {
			FAIL() << "_com_error: "
				<< ": " << cxx::to_string(e.ErrorMessage());
		}
		catch (...) {
			FAIL() << "unknown non-std exception";
		}
	}

	template<std::ranges::range T>
	static UINT SendInput(T& inputs)
	{
		return ::SendInput(UINT(std::size(inputs)), std::data(inputs), sizeof(decltype(inputs[0])));
	}

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpUia();

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownUia();

	/*!
	 * ダイアログを閉じるスレッドを開始する
	 *
	 * @param title タイトル
	 * @param action 閉じるアクション
	 * @return ダイアログを閉じるためのスレッド
	 */
	std::jthread StartDialogCloser(
		std::wstring_view title,
		const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
		ULONGLONG timeoutMillis = defaultTimeoutMillis
	) const;

	/*!
	 * ファイルを開くダイアログを閉じるスレッドを開始する
	 *
	 * @param path ファイルパス
	 * @return ファイルを開くダイアログを閉じるためのスレッド
	 */
	std::jthread StartEnterOpenFileName(
		const std::filesystem::path& path
	) const;

	/*!
	 * ポップアップメニューを選択するスレッドを開始する
	 *
	 * @param menuLabel メニューラベル
	 * @return ポップアップメニューを選択するためのスレッド
	 */
	std::jthread StartPopupMenuSelector(
		std::wstring_view menuLabel,
		ULONGLONG timeoutMillis = defaultTimeoutMillis
	) const;

	/*!
	 * UI Automationを利用するスレッドを開始する
	 *
	 * @param action 実行するアクション
	 * @return スレッド
	 */
	std::jthread StartUiaThread(
		const std::function<void(IUIAutomation*, std::stop_token)>& action
	) const;

	/*!
	 * ウィンドウを閉じるスレッドを開始する
	 *
	 * @param targetClass ウィンドウクラス名
	 * @param optTitle タイトル
	 * @param action 閉じるアクション
	 * @return ウィンドウを閉じるためのスレッド
	 */
	std::jthread StartWindowCloser(
		LPCWSTR targetClass,
		const std::optional<std::wstring>& optTitle,
		const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
		ULONGLONG timeoutMillis = defaultTimeoutMillis
	) const;

	/*!
	 * ダイアログを閉じるスレッドを開始する
	 *
	 * @param dialogTitle タイトル
	 * @return ダイアログを閉じるためのスレッド
	 */
	std::jthread StartDialogCloser(
		std::wstring_view dialogTitle,
		ULONGLONG timeoutMillis = defaultTimeoutMillis
	) const;
};

} // namespace env
