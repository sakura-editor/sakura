/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include <gmock/gmock.h>

#include "dlg/CDlgInput1.h"

#include "apiwrap/StdControl.h"

#include "tests1_rc.h"
#include "sakura_rc.h"

#include "CSelectLang.h"

#include <functional>

/*
 * ダイアログクラステンプレートをテストするためのクラス
 
 * 自動テストで実行できるように、初期表示後、勝手に閉じる仕様。
 */
class CDlgInput1ForTest : public CDlgInput1
{
private:
	static constexpr auto TIMERID_1 = 1;

public:
	~CDlgInput1ForTest() override = default;

	using CDlgInput1::GetHelpIdTable;

	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;
	BOOL    OnDlgTimer(HWND hDlg, UINT id) override;

protected:
	virtual UINT_PTR SetTimer(
		_In_opt_ HWND hWnd,
		_In_ UINT_PTR nIDEvent,
		_In_ UINT uElapse,
		_In_opt_ TIMERPROC lpTimerFunc) const
	{
		return ::SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
	}
};

/*!
 * WM_INITDIALOG処理
 *
 * ダイアログ構築後、最初に受け取るメッセージを処理する。
 */
BOOL CDlgInput1ForTest::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnDlgInitDialog(hDlg, hWndFocus, lParam);

	// タイマーを起動する
	SetTimer(hDlg, TIMERID_1, 0, nullptr);

	// 派生元クラスが返した戻り値をそのまま返す
	return ret;
}

/*!
 * WM_TIMER処理
 *
 * タイマーイベントを処理する。
 */
BOOL CDlgInput1ForTest::OnDlgTimer(HWND hDlg, UINT id)
{
	if (id == TIMERID_1)
	{
		// プログラム的に「Enterキー押下」のイベントを発生させる
		INPUT input = {};

		// WM_KEYDOWNを発生させる
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_RETURN;  // Enterキーの仮想キーコード
		input.ki.dwFlags = 0;      // キーを押す
		SendInput(1, &input, sizeof(INPUT));

		// WM_KEYUPを発生させる
		input.ki.dwFlags = KEYEVENTF_KEYUP;  // キーを離す
		SendInput(1, &input, sizeof(INPUT));

		return TRUE;
	}

	return FALSE;
}

using ::testing::Return;

/*!
 * モーダルダイアログ表示、正常系テスト
 */
TEST(CDlgInput1, SimpleDoModal)
{
	auto buffer = std::wstring(L"test");
	buffer.resize(256);

	CDlgInput1ForTest dlg;
	HINSTANCE hInstance  = nullptr;
	const auto hWndParent = static_cast<HWND>(nullptr);
	EXPECT_EQ(IDOK, dlg.DoModal(hInstance, hWndParent, L"title", L"message", static_cast<int>(buffer.length()), buffer.data()));
	EXPECT_STREQ(L"test", buffer.c_str());
}

/*!
 * 入力テキストがトリムされる確認
 */
TEST(CDlgInput1, TrimTest)
{
	auto buffer = std::wstring(L"test");
	buffer.resize(256);

	CDlgInput1ForTest dlg;
	HINSTANCE hInstance  = nullptr;
	const auto hWndParent = static_cast<HWND>(nullptr);
	EXPECT_EQ(IDOK, dlg.DoModal(hInstance, hWndParent, L"title", L"message", 2, buffer.data()));
	EXPECT_STREQ(L"te", buffer.c_str());
}

TEST(CDlgInput1, GetHelpIdTable)
{
	CDlgInput1ForTest dlg;
	EXPECT_TRUE(dlg.GetHelpIdTable());
}
