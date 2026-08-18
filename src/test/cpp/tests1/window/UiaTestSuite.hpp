/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"
#include "cxx/type_of_Nth_lambda_arg.hpp"
#include "dlg/ModalDialogCloser.hpp"
#include "util/tchar_convert.h"

// UI Automation経由でGUI操作を行う
#include <UIAutomation.h>

#include "testing/HResultEq.hpp"
#include "testing/StartEditorProcess.hpp"

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

template<typename TQuery, typename R = cxx::lambda_traits<TQuery>::return_type>
R WaitForQuery(
	TQuery query,
	std::stop_token st,
	ULONGLONG timeoutMillis,
	DWORD intervalMillis = 200
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

namespace window {

//! デフォルトの待機時間
static constexpr auto defaultTimeoutMillis = 5000;

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

	static BOOL CALLBACK CloseBlockingWindowProc(
		_In_ HWND   hWnd,
		_In_ LPARAM lParam [[maybe_unused]]
	);

	static void EmulateInvokeButton(
		IUIAutomation* pAutomation,
		_In_ HWND hWndDlg,
		int nIDDlgItem,
		std::stop_token st
	);

	static HWND GetActivePage(
		HWND hWndDlg,
		std::stop_token st = {},
		ULONGLONG timeoutMillis = 5000
	);

	//! UI Automationでメニュー項目選択を偽装
	static void EmulateInvokeMenuItem(
		IUIAutomation* pAutomation,
		_In_ HWND hWndPopupMenu,
		std::wstring_view caption,
		std::stop_token st
	);

	//! SendInputでEnterキー押下を偽装
	static void EmulateHitEnter()
	{
		std::vector<INPUT> inputs{};
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, false));
		inputs.emplace_back(MakeKeyboardInput(VK_RETURN, true));
		ASSERT_THAT(SendInput(inputs), Eq(std::size(inputs)));
	}

	//! SendInputでESCキー押下を偽装
	static void EmulateHitEscape()
	{
		std::vector<INPUT> inputs{};
		inputs.emplace_back(MakeKeyboardInput(VK_ESCAPE, false));
		inputs.emplace_back(MakeKeyboardInput(VK_ESCAPE, true));
		ASSERT_THAT(SendInput(inputs), Eq(std::size(inputs)));
	}

	//! UI Automationでボタン押下を偽装
	static void EmulateInvoke(
		const IUIAutomationElementPtr& pElement
	)
	{
		IUIAutomationInvokePatternPtr pInvokePattern;
		ASSERT_HRESULT_SUCCEEDED(pElement->GetCurrentPatternAs(UIA_InvokePatternId, IID_PPV_ARGS(&pInvokePattern)));
		ASSERT_HRESULT_EQ(pInvokePattern->Invoke(), S_OK);
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

	//! SendInputでマウス移動を偽装するためのデータを作る
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

	//! SendInputでマウスホイールを偽装するためのデータを作る
	static INPUT MakeMouseInputWheel(int delta)
	{
		INPUT input{};

		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		input.mi.mouseData = static_cast<DWORD>(delta);

		return input;
	}

	//! SendInputでキー押下を偽装するためのデータを作る
	static INPUT MakeKeyboardInput(WORD virtualKey, bool isKeyUp = false)
	{
		INPUT input{};

		input.type = INPUT_KEYBOARD;
		input.ki.wVk = virtualKey;
		input.ki.dwFlags = isKeyUp ? KEYEVENTF_KEYUP : 0;

		return input;
	}

	template <typename TAction>
	static void RunGuiTest(
		TAction&& action
	)
	{
#ifdef USE_STACK_TRACE
		__try
		{
#endif
			std::clog << "RunGuiTest() start" << std::endl;

			std::forward<TAction>(action)();

			std::clog << "RunGuiTest() end" << std::endl;

#ifdef USE_STACK_TRACE
		}
		__except (testing::OnUnhandledException(GetExceptionInformation()))
		{
			std::clog << "RunGuiTest() crashed." << std::endl;
		}
#endif
	}

	static void SendDlgCommand(
		_In_ HWND hWndDlg,
		int nIDDlgItem,
		int notifyCode = BN_CLICKED
	);

	template<std::ranges::range T>
	static UINT SendInput(T& inputs)
	{
		return ::SendInput(UINT(std::size(inputs)), std::data(inputs), sizeof(decltype(inputs[0])));
	}

	static void SendPsmPressButton(
		_In_ HWND hWndPropertySheet,
		UINT button
	);

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpUiaTestSuite();

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownUiaTestSuite();

	/*!
	 * UIAテストケースの開始前に自分で呼ぶ関数
	 */
	void SetUpUia();

	/*!
	 * UIAテストケースの終了後に呼ばれる関数
	 */
	void TearDownUia();

	/*!
	 * @brief UI Automationを使ったテストを実行する
	 */
	template <class TAction>
	void RunUiaAction(
		TAction&& action
	) const;

	/*!
	 * ダイアログを閉じるスレッドを開始する
	 *
	 * @param optTitle タイトル
	 * @param action 閉じるアクション
	 * @return ダイアログを閉じるためのスレッド
	 */
	std::jthread StartDialogCloser(
		const std::optional<std::wstring>& optTitle,
		const std::function<void(IUIAutomation*, HWND, std::stop_token)>& action,
		ULONGLONG timeoutMillis = defaultTimeoutMillis * 4
	);

	/*!
	 * ポップアップメニューを選択するスレッドを開始する
	 *
	 * @param menuLabel メニューラベル
	 * @return ポップアップメニューを選択するためのスレッド
	 */
	std::jthread StartPopupMenuSelector(
		std::wstring_view menuLabel,
		ULONGLONG timeoutMillis = defaultTimeoutMillis
	);

	/*!
	 * UI Automationを利用するスレッドを開始する
	 *
	 * @param action 実行するアクション
	 * @return スレッド
	 */
	std::jthread StartUiaThread(
		const std::function<void(IUIAutomation*, std::stop_token)>& action
	);

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
	);

	void RunMessageLoop(
		ULONGLONG timeoutMillis = defaultTimeoutMillis * 6
	);

	std::stop_source m_StopSource;
	std::jthread m_Thread;
	bool m_Timeout = false;
};

} // namespace env
