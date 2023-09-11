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
#include "dlg/CCustomDialog.hpp"

#include "MockUser32Dll.hpp"

#include <functional>

#include "sakura_rc.h"

extern HINSTANCE GetLanguageResourceLibrary();

/*
 * ダイアログクラステンプレートをテストするためのクラス

 * 自動テストで実行できるように作成したもの。
 * 初期表示後、勝手に閉じる仕様。
 */
class CDialog3 : public CCustomDialog
{
private:
	static constexpr auto TIMERID_FIRST_IDLE = 1;

	DLGPROC _pfnDlgProc = nullptr;

public:
	explicit CDialog3(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>()) noexcept;
	~CDialog3() override = default;

	INT_PTR CallDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

	INT_PTR DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

protected:
	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;

	virtual BOOL    OnDlgDestroy(HWND hDlg);
	virtual BOOL    OnDlgTimer(HWND hDlg, UINT id);

	BOOL KillTimer(
		_In_opt_ HWND hWnd,
		_In_ UINT_PTR uIDEvent) const
	{
		return ::KillTimer(hWnd, uIDEvent);
	}

	UINT_PTR SetTimer(
		_In_opt_ HWND hWnd,
		_In_ UINT_PTR nIDEvent,
		_In_ UINT uElapse,
		_In_opt_ TIMERPROC lpTimerFunc) const
	{
		return ::SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
	}
};

/*!
 * コンストラクター
 */
CDialog3::CDialog3(std::shared_ptr<User32Dll> User32Dll_) noexcept
	: CCustomDialog(IDD_INPUT1, std::move(User32Dll_))
{
}

/*!
 * ダイアログプロシージャを呼び出します。
 *
 * ポインタ変数は初期表示時に設定するので、一度開いてから使います。
 */
INT_PTR CDialog3::CallDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return _pfnDlgProc ? _pfnDlgProc(hDlg, uMsg, wParam, lParam) : FALSE;
}

/*!
 * ダイアログのメッセージ配送(旧関数)
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
INT_PTR CDialog3::DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
// clang-format off
	case WM_DESTROY:	 return OnDlgDestroy(hDlg);
	case WM_TIMER:		 return OnDlgTimer(hDlg, static_cast<UINT>(wParam));
// clang-format on

	default:
		break;
	}

	return __super::DispatchDlgEvent(hDlg, uMsg, wParam, lParam);
}

/*!
 * WM_INITDIALOG処理
 *
 * ダイアログ構築後、最初に受け取るメッセージを処理する。
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
BOOL CDialog3::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	// hDlgはこの時点で設定済み
	assert(hDlg == GetHwnd());

	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnDlgInitDialog(hDlg, hWndFocus, lParam);

	// デフォルト実装は0を返す
	assert(!GetDlgData(hDlg));

	// ダイアログプロシージャをメンバー変数に格納する
	_pfnDlgProc = std::bit_cast<DLGPROC>(GetWindowLongPtrW(hDlg, DWLP_DLGPROC));

	// タイマーを起動する
	SetTimer(hDlg, TIMERID_FIRST_IDLE, 0, nullptr);

	// 派生元クラスが返した戻り値をそのまま返す
	return ret;
}

/*!
 * WM_DESTROYハンドラ
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CDialog3::OnDlgDestroy(HWND hDlg)
{
	// タイマーを破棄する
	KillTimer(hDlg, TIMERID_FIRST_IDLE);

	return FALSE;
}

/*!
 * WM_TIMER処理
 *
 * タイマーイベントを処理する。
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CDialog3::OnDlgTimer(HWND hDlg, UINT id)
{
	if (id == TIMERID_FIRST_IDLE)
	{
		// フォーカスアウトしていたら最前面にする
		if (::GetFocus() != ::GetDlgItem(hDlg, IDC_EDIT_INPUT1))
		{
			::SetForegroundWindow(hDlg);
			return TRUE;
		}

		// プログラム的に「Enterキー押下」を発生させる
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

class mock_dialog_3 : public CDialog3
{
public:
	explicit mock_dialog_3(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>())
		: CDialog3(std::move(User32Dll_))
	{
	}

	MOCK_METHOD3(OnDlgInitDialog, BOOL(HWND, HWND, LPARAM));
};

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

/*!
 * モーダルダイアログ表示、正常系テスト
 */
TEST(CCustomDialog, SimpleDoModal)
{
	CDialog3 dlg;
	EXPECT_EQ(IDOK, dlg.Box(GetLanguageResourceLibrary(), nullptr));
	EXPECT_FALSE(dlg.CallDialogProc(nullptr, WM_NULL, 0, 0));
}

/*!
 * モーダルダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CCustomDialog, MockedDoModal)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, DialogBoxParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, _)).WillOnce(Return(IDCANCEL));

	CDialog3 mock(std::move(pUser32Dll));
	EXPECT_EQ(IDCANCEL, mock.Box(GetLanguageResourceLibrary(), hWndParent));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CCustomDialog, SimpleShow1)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = static_cast<HWND>(nullptr);

	CDialog3 dlg;
	EXPECT_NE(nullptr, dlg.Create(hLangRsrcInstance, hWndParent));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CCustomDialog, MockedShow1)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, CreateDialogParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, _)).WillOnce(Return(hDlg));

	CDialog3 dlg(std::move(pUser32Dll));
	EXPECT_EQ(hDlg, dlg.Create(hLangRsrcInstance, hWndParent));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CCustomDialog, SimpleShow2)
{
	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = static_cast<HWND>(nullptr);

	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	CDialog3 dlg;
	const auto hDlg = dlg.CreateIndirect(hLangRsrcInstance, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, hWndParent);
	EXPECT_TRUE(hDlg);
	CloseWindow(hDlg);
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CCustomDialog, MockedShow2)
{
	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, FindResourceW(_, _, _)).WillOnce(Invoke(::FindResourceW));
	EXPECT_CALL(*pUser32Dll, LoadResource(_, _)).WillOnce(Invoke(::LoadResource));
	EXPECT_CALL(*pUser32Dll, LockResource(_)).WillOnce(Invoke(::LockResource));
	EXPECT_CALL(*pUser32Dll, SizeofResource(_, _)).WillOnce(Invoke(::SizeofResource));
	EXPECT_CALL(*pUser32Dll, CreateDialogIndirectParamW(_, _, hWndParent, _, _)).WillOnce(Return(hDlg));

	CDialog3 dlg(std::move(pUser32Dll));
	EXPECT_EQ(hDlg, dlg.CreateIndirect(hLangRsrcInstance, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, hWndParent));
}

TEST(CCustomDialog, MockedDispachDlgEvent_OnInitDialog)
{
	mock_dialog_3 mock;

	const auto hDlg      = (HWND)0x4321;
	const auto hWndFocus = (HWND)0x1234;
	const auto wParam    = (WPARAM)hWndFocus;
	const auto lParam    = std::bit_cast<LPARAM>(&mock);

	EXPECT_CALL(mock, OnDlgInitDialog(hDlg, hWndFocus, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_INITDIALOG, wParam, lParam));
}
