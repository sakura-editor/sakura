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
#include "dlg/CDialog.h"

/*
  * 勝手に閉じるダイアログ
  *
  * 自動テストで実行できるように作成したもの。
  * 初期表示後、勝手に閉じる仕様。
  */
template<typename TDialog, int IDC_DEFAULT>
class TAutoCloseDialog : public TDialog
{
private:
	static constexpr auto TIMERID_FIRST_IDLE = 1;

public:
	explicit TAutoCloseDialog(int DialogId_, std::shared_ptr<User32Dll> User32Dll_)
		: TDialog(DialogId_, std::move(User32Dll_))
	{
	}

	~TAutoCloseDialog() override = default;

	using TDialog::DispatchDlgEvent;

protected:
	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;
	BOOL    OnDlgDestroy(HWND hDlg) override;
	BOOL    OnDlgTimer(HWND hDlg, UINT id) override;

	virtual BOOL KillTimer(
		_In_opt_ HWND hWnd,
		_In_ UINT_PTR uIDEvent) const
	{
		return ::KillTimer(hWnd, uIDEvent);
	}

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
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
template<typename TDialog, int IDC_DEFAULT>
inline BOOL TAutoCloseDialog<TDialog, IDC_DEFAULT>::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnDlgInitDialog(hDlg, hWndFocus, lParam);

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
template<typename TDialog, int IDC_DEFAULT>
inline BOOL TAutoCloseDialog<TDialog, IDC_DEFAULT>::OnDlgDestroy(HWND hDlg)
{
	// タイマーを破棄する
	KillTimer(hDlg, TIMERID_FIRST_IDLE);

	return __super::OnDlgDestroy(hDlg);
}

/*!
 * WM_TIMER処理
 *
 * タイマーイベントを処理する。
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
template<typename TDialog, int IDC_DEFAULT>
inline BOOL TAutoCloseDialog<TDialog, IDC_DEFAULT>::OnDlgTimer(HWND hDlg, UINT id)
{
	if (id == TIMERID_FIRST_IDLE)
	{
		// フォーカスアウトしていたら最前面にする
		if (::GetFocus() != ::GetDlgItem(hDlg, IDC_DEFAULT))
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

	return __super::OnDlgTimer(hDlg, id);
}
